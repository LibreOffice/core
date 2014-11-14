/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <math.h>

#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>



#ifdef _WIN32
//#include <Windows.h>   // come from LibreOfficeKitInit.h
    long getTimeMS()
    {
        return GetTickCount();
    }

    bool IsAbsolutePath(char *pPath)
    {
        if (pPath[1] != ':')
        {
            fprintf( stderr, "Absolute path required to libreoffice install\n" );
            return false;
        }

        return true;
    }

#else
#include <sys/time.h>
#include <sal/types.h>
    long getTimeMS()
    {
        struct timeval t;
        gettimeofday(&t, NULL);
        return t.tv_sec*1000 + t.tv_usec/1000;
    }

    bool IsAbsolutePath(char *pPath)
    {
        if (pPath[0] != '/')
        {
            fprintf( stderr, "Absolute path required to libreoffice install\n" );
            return false;
        }

        return true;
    }
#endif


using namespace ::lok;


static int help()
{
    fprintf( stderr, "Usage: libtest <absolute-path-to-libreoffice-install> [path to load document] [path to save document].\n" );
    return 1;
}

int main (int argc, char **argv)
{
    long start, end;

    start = getTimeMS();

    if( argc < 2 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();


    if( !IsAbsolutePath(argv[1]) )
        return 1;

    // coverity[tainted_string] - build time test tool
    Office *pOffice = lok_cpp_init( argv[1] );
    if( !pOffice )
    {
        fprintf( stderr, "Failed to initialize\n" );
        return -1;
    }

    end = getTimeMS();
    fprintf( stderr, "init time: %ld ms\n", (end-start) );
    start = end;

    fprintf( stderr, "start to load document '%s'\n", argv[2] );
    Document *pDocument = pOffice->documentLoad( argv[2] );
    if( !pDocument )
    {
        char *pError = pOffice->getError();
        fprintf( stderr, "failed to load document '%s': '%s'\n",
                 argv[2], pError );
        free (pError);
        return -1;
    }

    end = getTimeMS();
    fprintf( stderr, "load time: %ld ms\n", (end-start) );
    start = end;

    if( argc > 3 )
    {
        const char *pFilter = NULL;
        if( argc > 4 )
            pFilter = argv[4];
        fprintf( stderr, "save document as '%s' (%s)\n", argv[3], pFilter ? pFilter : "<null>" );
        if( !pDocument->saveAs( argv[3], pFilter ) )
        {
            char *pError = pOffice->getError();
            fprintf( stderr, "failed to save document '%s'\n", pError);
            free (pError);
        }
        else
        {
            fprintf( stderr, "Save succeeded\n" );
            end = getTimeMS();
            fprintf( stderr, "save time: %ld ms\n", (end-start) );
        }
    }
    fprintf( stderr, "all tests passed.\n" );

    delete pDocument;
    delete pOffice;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
