#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define BUFF_LEN 20
#define INPUT_LEN 30

#define SERVERADDR "34.241.4.235"
#define PORT 8080


int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char buffer[BUFF_LEN];
    char username[INPUT_LEN];
    char password[INPUT_LEN];
    char *response_line;
    char *cookie = NULL;
    char *token = NULL;
    char *h = calloc(BUFLEN, sizeof(char)); //helper

    while(1){
        
        sockfd = open_connection(SERVERADDR, PORT, AF_INET, SOCK_STREAM, 0);
        printf("\nType command:\n");
        scanf("%s", buffer);

        if(!strcmp(buffer, "register")){

            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *body_data = NULL;
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            body_data = json_serialize_to_string_pretty(value);



            message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/register", "application/json", &body_data, 1, NULL, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            json_free_serialized_string(body_data);
            json_value_free(value);

            //extragem prima linie din raspuns si verificam daca apare cuvantul cheie "Created" care ne indica succesul inregistrarii
            char *response_line;
            response_line = strtok(response, "\r\n");


            if(strstr(response_line, "Created")){
                printf("User %s registered successfully", username);
            }
            else{
                printf("The username %s is taken.", username);
                continue;
            }

            
        }else if(!strcmp(buffer, "login")){

            if(cookie != NULL){
                printf("\nYou are already logged in!\n");
                continue;
            }

            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *body_data = NULL;
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            body_data = json_serialize_to_string_pretty(value);


            message = compute_post_request(SERVERADDR, "/api/v1/tema/auth/login", "application/json", &body_data, 1, NULL, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            json_free_serialized_string(body_data);
            json_value_free(value);


            strcpy(h, response);


            //extragem prima linie din raspuns si verificam daca apare cuvantul cheie "OK" care ne indica succesul inregistrarii
            response_line = strtok(response, "\r\n");

            if(strstr(response_line, "OK")){


                //salvez cookie-ul din raspunsul primit
                h = strstr(h, "connect.sid=");
                cookie = calloc(BUFLEN, sizeof(char));
                strcpy(cookie, strtok(h,";"));


                printf("\nUser %s logged in successfully", username);
                continue;
            }
            else{
                printf("\nCredentials for user %s are not good!", username);
                continue;
            }

        }else if(!strcmp(buffer, "enter_library")){
            
            if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }
                
            message = compute_get_request(SERVERADDR, "/api/v1/tema/library/access", NULL, NULL, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            strcpy(h, response);

            
            response_line = strtok(response, "\r\n");

            if(strstr(response_line, "OK")){
                //extrag token-ul din raspunsul primit
                h = strstr(h, "token");
                if(h == NULL){
                    continue;
                }
                h = strstr(h, ":");
                if(h == NULL){
                    continue;
                }
                h = h + 1;
                token = calloc(BUFLEN, sizeof(char));
                strcpy(token, strtok(h, "\""));
                printf("\nUser %s entered library successfully!", username);
            }else{
                printf("\nCannot enter library! Please try again!");
            }

        }else if(!strcmp(buffer, "get_books")){

            if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }

            if(token == NULL){
                printf("You do not have access to the library!");
                continue;
            }

            message = compute_get_request(SERVERADDR, "/api/v1/tema/library/books", NULL, token, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            if(strstr(response, "OK")){
                char *books;
                books = strstr(response, "[");
                printf("The books from the library are: \n %s", books);
            }else{
                printf("Error while trying to get the books from the library");
            }

           
        }else if(!strcmp(buffer, "get_book")){

            if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }

            if(token == NULL){
                printf("You do not have access to the library!");
                continue;
            }

            char id[INPUT_LEN];
            int id_is_number;

            printf("id=");
            scanf("%s", id);

            id_is_number = atoi(id);
            if(id_is_number == 0){
                printf("Invalid id!");
            }
            
            //cream calea dorita, de forma /api/v1/tema/library/books/bookId
            char url[50] = "/api/v1/tema/library/books/";
            strcat(url, id);

            message = compute_get_request(SERVERADDR, url, NULL, token, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            strcpy(h, response);

            response_line = strtok(response, "\r\n");

            if(strstr(response_line, "OK")){
                char *book_details;
                book_details = strstr(h, "{");
                book_details = strtok(book_details, "}");
                printf("\nThe details for the book with id %s are:\n %s}", id, book_details);
            }else{
                printf("\nThe book with id %s does not exist!", id);
            }

            
        }else if(!strcmp(buffer, "add_book")){

            if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }

            if(token == NULL){
                printf("You do not have access to the library!");
                continue;
            }

            char title[INPUT_LEN];
            char author[INPUT_LEN];
            char genre[INPUT_LEN];
            int page_count;
            char pages[INPUT_LEN];
            char publisher[INPUT_LEN];
            int len;
            int is_number;

            //-----------title---------

            //ramane de la scanf un "\n" pe care fgets ul il citeste prima oara
            fgets(title, INPUT_LEN, stdin);
            printf("title=");
            fgets(title, INPUT_LEN, stdin);
            len = strlen(title);
            //inlocuiesc \n de la sfarsitul cuvantului citit cu fgets cu \0, pentru a marca sfarsitul stringului
            title[len -1] = '\0';

            if(!strcmp(title, "\0")){
                printf("\nTitle is mandatory!");
                continue;
            }

            //ne asiguram ca titlul introdus nu este un numar
            is_number = atoi(title);
            //functia atoi intoarce zero daca nu poate converti stringul in int
            //functia atoi intoarce stringul convertit in numar(diferit de zero) daca primeste ca parametru un string ce contine de fapt un numar
            if(is_number != 0){
                //atoi a avut succes, deci titul introdus a fost un numar
                printf("\nThe title cannot be a number!");
                continue;
            }

            //------author-------

            printf("author=");
            fgets(author, INPUT_LEN, stdin);
            len = strlen(author);
            author[len -1] = '\0';

            if(!strcmp(author, "\0")){
                printf("\nThe author is mandatory!");
                continue;
            }

            //ne asiguram ca autorul introdus nu este un numar
            is_number = atoi(author);
            if(is_number != 0){
                printf("\nThe author cannot be a number!");
                continue;
            }


            //------genre-------

            printf("genre=");
            fgets(genre, INPUT_LEN, stdin);
            len = strlen(genre);
            genre[len -1] = '\0';

            if(!strcmp(genre, "\0")){
                printf("\nThe genre is mandatory!");
                continue;
            }

            //ne asiguram ca genul introdus nu este un numar
            is_number = atoi(genre);
            if(is_number != 0){
                printf("\nThe genre cannot be a number!");
                continue;
            }

            //------page_count-------

            printf("page_count=");
            fgets(pages, INPUT_LEN, stdin);
            len = strlen(pages);
            pages[len -1] = '\0';

            if(!strcmp(pages, "\0")){
                printf("\nThe number of pages is mandatory!");
                continue;
            }

            //ne asiguram ca numarul de pagini introdus nu este un string
            is_number = atoi(pages);
            if(is_number == 0){
                printf("\nThe number of pages cannot be a string!");
                continue;
            }
            page_count = is_number;
            


            //------publisher-------

            printf("publisher=");
            fgets(publisher, INPUT_LEN, stdin);
            len = strlen(publisher);
            publisher[len -1] = '\0';

            if(!strcmp(publisher, "")){
                printf("\nThe publisher is mandatory!");
                continue;
            }

            //ne asiguram ca redactorul introdus nu este un numar
            is_number = atoi(publisher);
            if(is_number != 0){
                printf("\nThe publisher cannot be a number!");
                continue;
            }

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *body_data = NULL;
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "genre", genre);
            json_object_set_number(object, "page_count", page_count);
            json_object_set_string(object, "publisher", publisher);
            body_data = json_serialize_to_string_pretty(value);


            message = compute_post_request(SERVERADDR, "/api/v1/tema/library/books", "application/json", &body_data, 1, token, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            json_free_serialized_string(body_data);
            json_value_free(value);

            printf("%s", response);

           response_line = strtok(response, "\r\n");

            if(strstr(response_line, "OK")){
                printf("\nUser %s added a book to the library!", username);
            }else{
                printf("\nError while trying to add a book to the library!");
            }

        }else if(!strcmp(buffer, "delete_book")){

             if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }

            if(token == NULL){
                printf("You do not have access to the library!");
            }

            char id[INPUT_LEN];
            int id_is_number;

            printf("id=");
            scanf("%s", id);

            id_is_number = atoi(id);
            if(id_is_number == 0){
                printf("Invalid id!");
            }
            
            //cream calea dorita, de forma /api/v1/tema/library/books/bookId
            char url[50] = "/api/v1/tema/library/books/";
            strcat(url, id);

            message = compute_delete_request(SERVERADDR, url, NULL, token, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            response_line = strtok(response, "\r\n");

            if(strstr(response_line, "OK")){
                printf("The book with id %s was deleted!", id);
            }else{
                printf("Error while trying to delete the book with id %s !", id);
            }


        }else if(!strcmp(buffer, "logout")){
            if(cookie == NULL){
                printf("You are not log in!");
                continue;
            }

            if(cookie == NULL){
                printf("You are not logged in!");
                continue;
            }

            message = compute_get_request(SERVERADDR, "/api/v1/tema/auth/logout", NULL, token, &cookie, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            response_line = strtok(response, "\r\n");

            free(cookie);
            free(token);
            cookie = NULL;
            token = NULL;

            if(strstr(response, "OK")){
                printf("\nUser %s logged out!", username);
            }else{
                printf("\nError while trying to log out!");
            }

        }else if(!strcmp(buffer, "exit")){
            close_connection(sockfd);
            break;
        }

    }


    return 0;
}
