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

#include <tools/fsys.hxx>
#include <tools/stream.hxx>
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
