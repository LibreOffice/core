/*************************************************************************
 *
 *  $RCSfile: checkdll.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mhu $ $Date: 2003-04-16 10:19:57 $
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
#include <CoreFoundation/CoreFoundation.h>
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
    CFStringRef     bundlePath;
    CFURLRef        bundleURL;
    CFBundleRef     bundle;
    CFStringRef     symbolName;
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

    /* Convert char pointers to CFStringRefs */
    bundlePath = CFStringCreateWithCStringNoCopy(NULL, argv[1],
        CFStringGetSystemEncoding(), kCFAllocatorNull);
    symbolName = CFStringCreateWithCStringNoCopy(NULL, psymbol,
        CFStringGetSystemEncoding(), kCFAllocatorNull);

    /* Get the framework's URL using its path */
    if ((bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
        bundlePath, kCFURLPOSIXPathStyle, true)) != NULL) {
            /* Load the framework */
            if ((bundle = CFBundleCreate( kCFAllocatorDefault,
                bundleURL)) != NULL) {
                    /* Load the shared library */
                    if (CFBundleLoadExecutable(bundle)) {
                        if ((pfun = CFBundleGetFunctionPointerForName(bundle,
                            symbolName)) != NULL) {
                                printf(": ok\n");
                                CFRelease(bundlePath);
                                CFRelease(bundleURL);
                                CFRelease(bundle);
                                CFRelease(symbolName);
                                return 0;
                        }
                        else
                            printf(": ERROR: symbol %s not found\n", psymbol);
                    }
                    /* No message printed since CFLog prints its own message */
            }
            else
                printf(": ERROR: %s is not a bundle\n", argv[1]);
    }
    else
        printf(": ERROR: %s is not a valid bundle name\n", argv[1]);

    CFRelease(bundlePath);
    if (bundleURL != NULL) CFRelease(bundleURL);
    if (bundle != NULL) CFRelease(bundle);
    CFRelease(symbolName);
    return 3;

#else /* MACOSX */

    if ( (phandle = dlopen(argv[1], RTLD_NOW)) != NULL )
    {
        if  ( (pfun = (char *(*)(void))dlsym(phandle, psymbol)) != NULL )
        {
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

