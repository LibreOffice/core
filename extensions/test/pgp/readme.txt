The program TestPGP instantiates a SimplePGPMailerFactory and adds
 it to the ServiceManager obtained from a running StarOffice.
Then PGP can be used from within StarOffice.

Prerequisites:
StarOffice needs an entry in Office/user/sofficerc under the Common
 section:

 Port2=socket:hamburg-11070:1111 // old style
as of 569 m
Connection=socket,hamburg-11070,port=1111;iiop;

Please note the semicolons!

 hamburg-11070 is the host running the office and 1111 is a
 freely choosable port number.


 For the program to run make sure that the office has access
 to classes.zip, unoil.jar, jurt.jar, sandbox.jar, pgp.jar
 and swingall.jar.

 Therefore do the necessary entries in
 Office/user/config/javarc
 under SystemClasspath.

There is a bug with jdk1.1.8, therfore use 1.2 instead. 

The TestPGP is called without ;iiop; :

java TestPGP "socket,host=localhost,port=1111"


 
