#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress(){
    return this->my_address;
}
//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++)
    {
        if (network[i] != NULL)
        {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else               {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else               {cout << "\n"; outgoing->display(); }
    
}

void laptop::consumeDatagram(){
    this->incoming = NULL;
}

int laptop::canReceiveDatagram(){
    if (this->incoming == NULL){
        return 1;
    }
    else {
        return 0;
    }
}
/**************new*************/
laptop::~laptop()
{
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}
/**************new*************/

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}

int server::myType() {
    return SERVER;
}

int  server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void server::transferDatagram(){
    extern node* network[MAX_MACHINES];
    msg_list* temp = new msg_list;
    datagram* currDg;
    IPAddress destinationIP;
    int absdiffacc = 0; //variable for absolute value of difference of WAN octads
    int WANid = -1; //variable to keep track of which WAN to send the datagram to
    
    currDg = dlist->returnFront(); //set the current datagram to the front of the dlist
    while (currDg != NULL){ //outer loop that runs until we reach the end of the server's dlist
        destinationIP = currDg->destinationAddress(); //set the destination IP to the destination address of the current datagram
        if (destinationIP.firstOctad() == my_address.firstOctad()){ //if the datagram has a LANid equal to that of the server
            for (int j = 0; j < MAX_MACHINES; j++){ //loop through the network array to find the corresponding laptop
                if (network[j] != NULL && network[j]->amIThisComputer(destinationIP)){
                    if (((laptop*)network[j])->canReceiveDatagram()){ //if the laptop can receieve a datagram
                        ((laptop*)network[j])->receiveDatagram(currDg); //have that laptop receieve the datagram
                    }
                    else{ //if the laptop can't receive datagram, append currDg to temp list
                        temp->append(currDg);
                    }
                }
            }
        }
        else { //if the datagram has a LANid NOT equal to that of the server
            if (number_of_wans == 0){
                temp->append(currDg);
            }
            else{
                for (int i = 0; i < number_of_wans; i++){ //search through the wan_list to find the appropriate WAN to send the datagram
                    if (i == 0){
                        absdiffacc = abs((WAN_list[0]).firstOctad() - destinationIP.firstOctad()); //initialize the LANid absolute difference
                        WANid = i;
                    }
                    else{ //for all other values of i, determine whether the WAN is closer, if so, grab it
                        if (abs(WAN_list[i].firstOctad() - destinationIP.firstOctad()) < absdiffacc){
                            absdiffacc = abs(WAN_list[i].firstOctad() - destinationIP.firstOctad());
                            WANid = i;
                        }
                    }
                }
                for (int i = 0; i < MAX_MACHINES; i++){ //search through the network array to find the WAN
                    if (network[i] != NULL && network[i]->amIThisComputer(WAN_list[WANid])){
                        ((WAN*)network[i])->receiveDatagram(currDg);
                    }
                }
            }
        }
        currDg = dlist->returnFront(); //set the current datagram to the next node of the dlist
    }
    delete dlist;
    dlist = temp;
}

/**************new*************/
server::~server()
{
    dlist->deleteList();
}
/**************new*************/

//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void WAN::transferDatagram(){
    extern node* network[MAX_MACHINES];
    msg_list* temp = new msg_list;
    //temp = NULL;
    datagram* currDg;
    IPAddress destinationIP;
    int transferred = 0;
    int absdiffacc = -1; //variable for absolute value of difference of WAN octads
    int WANid = -1; //variable to keep track of which WAN to send the datagram to
    currDg = dlist->returnFront(); //set the current datagram to the front of the dlist
    while (currDg != NULL){ //outer loop that runs until we reach the end of the server's dlist
        transferred = 0;
        destinationIP = currDg->destinationAddress(); //set the destination IP to the destination address of the current datagram
        for (int i = 0; i < number_of_servers; i++){    //if the destination IP has the same LANid as a connected server
            if (number_of_servers > 0 && server_list[i].firstOctad() == destinationIP.firstOctad()){
                for (int j = 0; j < MAX_MACHINES; j++){ //loop through the network array to find the corresponding laptop
                    if (network[j] != NULL && network[j]->amIThisComputer(server_list[i])){
                        ((server*)network[j])->receiveDatagram(currDg);
                        currDg = NULL; //**********
                        transferred = 1;
                        break;
                    }
                }
            }
        }
        if (transferred == 0 && number_of_wans > 0){
            for (int i = 0; i < number_of_wans; i++){
                if (i == 0){
                    absdiffacc = abs(destinationIP.firstOctad() - WAN_list[0].firstOctad());
                    WANid = 0;
                }
                else if ((abs(destinationIP.firstOctad() - WAN_list[i].firstOctad())) < absdiffacc){
                    absdiffacc = abs(destinationIP.firstOctad() - WAN_list[i].firstOctad());
                    WANid = i;
                }
            }
            for (int i = 0; i < MAX_MACHINES; i++){ //search through the network array to find the WAN
                if (network[i] != NULL && network[i]->amIThisComputer(WAN_list[WANid])){
                    ((WAN*)network[i])->receiveDatagram(currDg);
                    currDg = NULL; //***************
                    transferred = 1;
                    break;
                }
            }
        }
        else if (transferred == 0) {
            temp->append(currDg);
        }
        currDg = dlist->returnFront();
    }
    delete dlist;
    dlist = temp;
}



/**************new*************/
WAN::~WAN()
{
    dlist->deleteList();
}
/**************new*************/





















