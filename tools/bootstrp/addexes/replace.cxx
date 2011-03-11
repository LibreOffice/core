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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdio.h>
#include <tools/string.hxx>


/****************************************************************************/
#if defined UNX
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/****************************************************************************/
{
    if ( argc < 4 )
    {
        fprintf( stderr, "ERROR: too few parameters. \n\n");
        fprintf( stderr, "usage: txtrep.exe EnvironmentVariable Searchstring replacestring\n");
        return 1;
    }
    ByteString aText( getenv( argv[ 1 ] ));
    if ( aText.Len() == 0 )
    {
        fprintf( stderr, "ERROR: Variable not set. \n\n");
        fprintf( stderr, "usage: txtrep.exe EnvironmentVariable Searchstring replacestring\n");
        return 2;
    }
    ByteString aSearch( argv[ 2 ] );
    ByteString aReplace( argv[ 3 ] );

    ByteString aUpperText( aText );
    aUpperText.ToUpperAscii();


    sal_uIntPtr nIndex;
    aSearch.ToUpperAscii();

    nIndex = aUpperText.Search( aSearch.GetBuffer(), 0);
    while ( nIndex != STRING_NOTFOUND )
    {
        aText.Replace( nIndex, aSearch.Len(), aReplace.GetBuffer());
        aUpperText.Replace( nIndex, aSearch.Len(), aReplace.GetBuffer());
        nIndex = aUpperText.Search( aSearch.GetBuffer(), nIndex + aReplace.Len());
    }

    fprintf( stdout, "%s\n", aText.GetBuffer());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
