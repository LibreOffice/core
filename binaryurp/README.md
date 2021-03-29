# UNO Remote Protocol (URP)

UNO provides a binary protocol called the UNO Remote Protocol (URP) that provides
a bridge between UNO environments. This bridge allows processes and objects
to send method calls and to receive return values. UNO objects in different 
environments are connected by way of this interprocess bridge. The underlying 
connection is made through a socket or pipe. Remote UNO objects are connected
by means of TCP/IP using the high-level protocol of the URP.
