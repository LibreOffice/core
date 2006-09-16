/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sspretty.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:49:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include "iparser.hxx"
#include "geninfo.hxx"
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

