#include <iostream>

#include "msg_list.h"

msg_list::msg_list(){
	front = back = NULL;
}

void msg_list::display() {
	msg_list_node *tmp;  int i;
	
	if(front==NULL) {
		cout << "** List is empty. **\n";
		return;
	}
	
	tmp = front;  i = 1;
	while(tmp!=NULL) {
		cout << "Datagram " << i++ << ":  \n";
		(tmp->d)->display();
		cout << "\n";
		tmp = tmp->next;
	}
	
}

void msg_list::append(datagram *x) {
	msg_list_node* tmp = new msg_list_node;
	tmp->next = NULL;
	tmp->d = x;
    
	if(front==NULL)
        front = tmp;
	else
        back->next = tmp;
	back = tmp;
}

datagram* msg_list::returnFront(){
    if (front == NULL) {
        return NULL;
    }
    else if (front == back){
        msg_list_node* tmp = front;
        datagram* tmpd = tmp->d;
        front = NULL;
        back = NULL;
        delete tmp;
        return tmpd;
    }
    else{
        msg_list_node* tmp = front;
        datagram* tmpd = tmp->d;
        front = front->next;
        delete tmp;
        return tmpd;
    }
}

void msg_list::deleteList(){
    msg_list_node* tmp = front;
    msg_list_node* prev = NULL;
    while (tmp != NULL){
        delete tmp->d;
        prev = tmp;
        tmp = tmp->next;
    }
    front = NULL;
    back = NULL;
}