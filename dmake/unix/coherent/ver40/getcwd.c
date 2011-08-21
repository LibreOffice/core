/* RCS  $Id: getcwd.c,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      getcwd
--
-- DESCRIPTION
--      Wrapper for getcwd.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

#if __STDC__
char *getcwd(char *buffer, int length)
#else
char *getcwd (buffer, length)
char *buffer;
int length;
#endif
{
  extern char *getwd();

  char *dir;
  dir = getwd();
  if (dir)
  {
    strncpy(buffer,dir,length);
    return buffer;
  }
  else
  {
    *buffer = 0;
    return (char *) 0;
  }
}

