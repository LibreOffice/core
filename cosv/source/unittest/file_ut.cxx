/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



