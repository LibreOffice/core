/*************************************************************************
 *
 *  $RCSfile: checkdll.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mh $ $Date: 2003-07-17 10:26:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifdef MACOSX
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif

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
#ifdef MACOSX
        struct mach_header *pLib;
#else
    void    *phandle;
#endif
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

#ifdef MACOSX

        // Check if library is already loaded
        pLib = NSAddImage(argv[1], NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
        if (!pLib) {
                // Check DYLD_LIBRARY_PATH
                pLib = NSAddImage(argv[1], NSADDIMAGE_OPTION_WITH_SEARCHING);
        }
        if (pLib) {
                // Prefix symbol name with '_'
                char *name = malloc(1+strlen(psymbol)+1);
                NSSymbol *symbol;
                void *address = NULL;
                strcpy(name, "_");
                strcat(name, psymbol);
                symbol = NSLookupSymbolInImage(pLib, name, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND);
                free(name);
                if (symbol) address = NSAddressOfSymbol(symbol);
                if (address != NULL) {
                        printf(": ok\n");
#ifdef NO_UNLOAD_CHECK
                        _exit(0);
#else
                        // Mac OS X can't unload dylibs
#endif
                } else {
                        printf(": WARNING: symbol %s not found\n", psymbol);
                }
                return 0;
        } else {
                printf(": WARNING: %s is not a valid dylib name\n", argv[1]);
        }
        return 3;

        // fixme use NSLinkEditError() for better error messages

#else /* MACOSX */

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

#endif /* MACOSX */
}


