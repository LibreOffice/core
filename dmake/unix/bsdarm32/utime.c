/* RCS  $Id: utime.c,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      utime
--
-- DESCRIPTION
--      Provide our own utime function.
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>

int
utime(name, timep)
char*   name;
time_t  timep[2];
{
    struct timeval tv[2], *tvp;
    struct stat buf;
    int fil;
    char    data;

    if (timep!=0)
    {
        tvp = tv, tv[0].tv_sec = timep[0], tv[1].tv_sec = timep[1];
        if (utimes(name, tvp)==0)
            return (0);
    }

    if (stat(name, &buf) != 0)
        return (-1);
    if (buf.st_size != 0)  {
        if ((fil = open(name, O_RDWR, 0666)) < 0)
            return (-1);
        if (read(fil, &data, 1) < 1) {
            close(fil);
            return (-1);
        }
        lseek(fil, 0L, 0);
        if (write(fil, &data, 1) < 1) {
            close(fil);
            return (-1);
        }
        close(fil);
        return (0);
    } else  if ((fil = creat(name, 0666)) < 0) {
        return (-1);
    } else {
        close(fil);
        return (0);
    }
}
