/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <liblibreoffice.hxx>

int main (int argc, char **argv)
{
    if( argc < 2 )
        return -1;
    LibLibreOffice *pOffice = lo_init( argv[1] );
    if( !pOffice )
        return -1;
    // This separate init is lame I think.
    if( !pOffice->initialize( argv[1] ) )
    {
        fprintf( stderr, "failed to initialize\n" );
        return -1;
    }
    fprintf( stderr, "start to load document '%s'\n", argv[2] );
    LODocument *pDocument = pOffice->documentLoad( argv[2] );
    if( !pDocument )
    {
        char *pError = pOffice->getError();
        fprintf( stderr, "failed to load document '%s': '%s'\n",
                 argv[2], pError );
        free (pError);
        return -1;
    }

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
            fprintf( stderr, "Save succeeded\n" );
    }
    fprintf( stderr, "all tests passed." );

    delete pDocument;
    delete pOffice;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
