/^\/\* RCS/ { print "/* RCS  $Id: rcsclean.awk,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $"; next }
/^-- LOG/,/^\*\// {
   if( ! flag ) {
      print "-- LOG";
      print "--      Use cvs log to obtain detailed change logs.";
      print "*/";
      flag = 1;
   }

   next;
}
/^-- SYNOPSIS --/ {
     print "--";
     print "-- SYNOPSIS";
     printf "--      %s%s\n", toupper(substr($0, 16,1)), substr($0,17);
     next;
}
/^-- WWW/,/^--$/ {
   if( !wflag ) {
     print "-- WWW";
     print "--      http://dmake.wticorp.com/";
     print "--";
     wflag = 1;
   }
   next;
}
/^-- AUTHOR/,/^--$/ {
   if( !aflag ) {
     print "-- AUTHOR";
     print "--      Dennis Vadura, dvadura@dmake.wticorp.com";
     print "--";
     aflag = 1;
   }
   next;
}
/^-- COPYRIGHT/,/^--$/ {
   if( !wflag ) {
     print "-- WWW";
     print "--      http://dmake.wticorp.com/";
     print "--";
     wflag = 1;
   }
     
   if( !cflag ) {
print "-- COPYRIGHT";
print "--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.";
print "-- ";
print "--      This program is NOT free software; you can redistribute it and/or";
print "--      modify it under the terms of the Software License Agreement Provided";
print "--      in the file <distribution-root>/readme/license.txt.";
print "--";
cflag = 1;
   }
   next;
}

{ print; }
