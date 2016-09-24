httpserv 
============
Simple http server for testing of Certificate Revocation Lists (CRL).

Netbeans 6.7.1 project. httpserv.jar requires java 6.

Run with

java -jar httpserv/dist/httpserv.jar --accept portnumber

portnumber: an arbitrary port number to which the server should listen.

httpserv looks for the requested file relativ to the current directory. For
example, you are in
$(SRC_ROOT)/xmlsecurity/$(INPATH)/httpserv/dist/javadoc,
a directory which contains the index.html. From there you run httpserv.jar:

java -jar <path_to_wherever_it_is>/httpserv.jar --accept 8000

now enter in the Browser:

localhost:8000/index.html


