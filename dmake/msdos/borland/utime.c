/* RCS  $Id: utime.c,v 1.1.1.1 2000-09-22 15:33:28 hr Exp $
--
-- SYNOPSIS
--      utime
--
-- DESCRIPTION
--      chage the last modified time on a file.
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
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>

int
utime(name, timep)/*
====================
    Broken for turbo C it only sets the file time to the current time by
    touching a character in the file */
char*   name;
time_t  timep[2];
{
    struct  stat buf;
    int fil;
    char    data;

    if (stat(name, &buf) != 0)
        return (-1);
    if (buf.st_size != 0)  {
        if ((fil = open(name, O_RDWR, S_IWRITE)) < 0)
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
    } else  if ((fil = creat(name, S_IWRITE)) < 0) {
        return (-1);
    } else {
        close(fil);
        return (0);
    }
}
