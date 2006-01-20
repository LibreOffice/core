/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checkdll.c,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-20 13:28:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

/*
 * NOTE: Since no one is really interested in correct unload behavior I've
 * disabled the shared library unload check. If you want to reenable it comment
 * the following line out
 */
#define NO_UNLOAD_CHECK

static const char *pprog_name   = "checkdll";
static const char *psymbol      = "GetVersionInfo";

void usage()
{
    fprintf(stderr, "usage: %s <dllname>\n", pprog_name);
    return;
}

int main(int argc, char *argv[])
{
    int     rc;
    void    *phandle;
    char    *(*pfun)(void);

    if ( argc < 2 || argc > 3) {
        usage();
        return 1;
    }

    if ( (rc = access( argv[1], R_OK )) == -1 ) {
        fprintf(stderr, "%s: ERROR: %s: %s\n",
                pprog_name, argv[1], strerror(errno));
        return 2;
    }

    printf("Checking DLL %s ...", argv[1]);
    fflush(stdout);

    if ( (phandle = dlopen(argv[1], RTLD_NOW)) != NULL ) {
        if  ( (pfun = (char *(*)(void))dlsym(phandle, psymbol)) != NULL ) {
            printf(": ok\n");
        }
        else
        {
            printf(": WARNING: %s\n", dlerror());
        }
#ifdef NO_UNLOAD_CHECK
        _exit(0);
#else
        dlclose(phandle);
#endif
        return 0;
    }

    printf(": ERROR: %s\n", dlerror());
    return 3;
}


