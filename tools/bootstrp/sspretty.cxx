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
#include <tools/iparser.hxx>
#include <tools/geninfo.hxx>
#include <stdio.h>


/*****************************************************************************/
#ifdef UNX
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( argc != 2 ) {
        fprintf( stdout, "\nsspretty.exe v1.0 (c) 2001\n\n" );
        fprintf( stdout, "Syntax:  sspretty filename\n" );
    }
    else {
        String aFileName( argv[ 1 ], RTL_TEXTENCODING_ASCII_US );
        InformationParser aParser;
        GenericInformationList *pList = aParser.Execute( aFileName );
        if ( pList )
            aParser.Save( aFileName, pList );
        else {
            fprintf( stderr, "Error reading input file!\n" );
            return 1;
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
