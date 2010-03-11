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

#include <precomp.h>
#include <cosv/file.hxx>

// NOT FULLY DECLARED SERVICES
#include <ut.hxx>

using csv::File;


/** @file
    UnitTests for class File.
*/

FUT_DECL( File, Read );
FUT_DECL( File, Write );
FUT_DECL( File, Seek );
FUT_DECL( File, SeekBack );
FUT_DECL( File, SeekRelative );


bool
classtest_File()
{
    csv::File aFile( "bigfile.txt", csv::CFM_RW, 0 );
    csv::File & rFile = aFile;

    rFile.Open();

    bool ret = ftest_Read( rFile );
    ret = ftest_Write( rFile ) AND ret;
    ret = ftest_SeekBack( rFile ) AND ret;
    ret = ftest_SeekRelative( rFile ) AND ret;

    rFile.Close();

    return ret;
}



FUT_DECL( File, Read )
{
    bool ret = true;

    rFile.SeekBack(0);
    uintt nSourceSize = rFile.Position();
    rFile.Seek(0);

    char * pBuf = new char[nSourceSize+1];
    uintt nCount = rFile.Read(pBuf,nSourceSize);

    UT_CHECK( Read, nCount == nSourceSize );

    return ret;
}

FUT_DECL( File, Write )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, Seek )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, SeekBack )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, SeekRelative )
{
    bool ret = true;



    return ret;
}



