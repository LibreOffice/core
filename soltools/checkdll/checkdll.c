/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
    fprintf(stderr, "usage: %s [-s] <dllname>\n", pprog_name);
    return;
}

int main(int argc, char *argv[])
{
    int     rc;
    int     silent=0;
    void    *phandle;
    char    *(*pfun)(void);

    if ( argc < 2 || argc > 4) {
        usage();
        return 1;
    }

    if ( !strcmp(argv[1],"-s") ) {
        silent = 1;
        ++argv, --argc;
    }

    if ( (rc = access( argv[1], R_OK )) == -1 ) {
        fprintf(stderr, "%s: ERROR: %s: %s\n",
                pprog_name, argv[1], strerror(errno));
        return 2;
    }

    if (!silent) printf("Checking DLL %s ...", argv[1]);
    fflush(stdout);

    if ( (phandle = dlopen(argv[1], RTLD_NOW)) != NULL ) {
        if  ( (pfun = (char *(*)(void))dlsym(phandle, psymbol)) != NULL ) {
            if (!silent) printf(": ok\n");
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
