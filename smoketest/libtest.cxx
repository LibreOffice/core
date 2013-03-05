/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
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
        fprintf( stderr, "failed to load document '%s'\n", argv[2] );
        return -1;
    }
    fprintf( stderr, "all tests passed." );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
