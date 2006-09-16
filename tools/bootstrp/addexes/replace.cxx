/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: replace.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:50:16 $
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

#include <stdio.h>
#include "string.hxx"


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


    ULONG nIndex;
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
