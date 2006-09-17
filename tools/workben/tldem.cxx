/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tldem.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:10:04 $
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

#include <fsys.hxx>
#include <stream.hxx>
#include <sstring.hxx>

int
#ifdef WNT
__cdecl
#endif
main( int argc, char **argv )
{
    String aString;
    aString = "*.*";
    Dir aDir(aString);
    SvFileStream aFile;
    SvFileStream aSortedFile;

    StringList *pList = new StringList;
    SStringList *pSortedList = new SStringList;
    ULONG nCount = aDir.Count();

    for ( ULONG i=0; i<nCount; i++ )
    {
        aString = aDir[i].GetName();
        pList->Insert( new String( aString ), LIST_APPEND );
        pSortedList->PutString( new String( aString ));
    }

    aFile.Open( "test.dir", STREAM_WRITE );
    for ( ULONG j=0; j<nCount; j++ )
    {
        aFile.WriteLine( *pList->GetObject(j) );
    }
    aFile.Close();


    aSortedFile.Open( "stest.dir", STREAM_WRITE );
    for ( ULONG k=0; k<nCount; k++ )
    {
        aSortedFile.WriteLine( *pSortedList->GetObject(k) );
    }
    if ( pSortedList->IsString( new String("bloedString")) != NOT_THERE )
        aSortedFile.WriteLine( "Fehler !" );
    if ( pSortedList->IsString( new String(".")) == NOT_THERE )
        aSortedFile.WriteLine( "Fehler ?!?" );
    aSortedFile.Close();

    delete pList;
    delete pSortedList;
    return 0;
}


