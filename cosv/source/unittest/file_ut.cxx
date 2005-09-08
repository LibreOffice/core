/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file_ut.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:09:36 $
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



