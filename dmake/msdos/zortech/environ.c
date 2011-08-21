/* RCS  $Id: environ.c,v 1.1.1.1 2000-09-22 15:33:29 hr Exp $
--
-- SYNOPSIS
--      environment routines.
--
-- DESCRIPTION
--      Someone thought that Zortech needs this.
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
/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alloc.h"

/* ZTC++ doesn't have environ, so we have to create one. */

extern char *_envptr;
char **environ = { NULL };

void
make_env()
{
    int     i;
    char    *cp;

    for (i = 0, cp = _envptr; *cp; i++, cp += strlen(cp)+1)
        ;

    TALLOC(environ, i+1, char*);

    for (i = 0, cp = _envptr; *cp; i++, cp += strlen(cp)+1)
        environ[i] = cp;

    return;
}

void
free_env()
{
    FREE(environ);

    return;
}
