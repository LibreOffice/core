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
#include <cosv/string.hxx>

// NOT FULLY DECLARED SERVICES
#include <ut.hxx>

using csv::SimpleString;


/** @file
    UnitTests for class String.
*/

inline bool
check_value( const String & a, const char * b )
{
     return strcmp( a.c_str(), b ) == 0;
}


bool
classtest_String( csv::SimpleString & rSimpleString )
{
    bool ret = true;

    String x1;
    UT_CHECK( String(), check_value(x1,"") )

    const char * s2a = "";
    String x2a(s2a);
    UT_CHECK( String(const char*), check_value(x2a,s2a) )

    const char * s2b = "_zluoadninger  prrg chi‰ﬂi(/%$##@\\\"'''fh  kl";
    String x2b(s2b);
    UT_CHECK( String(const char*), check_value(x2b,s2b) )


    bool ret = ftest_Read( rSimpleString );
    ret = ftest_Write( rSimpleString ) AND ret;
    ret = ftest_SeekBack( rSimpleString ) AND ret;
    ret = ftest_SeekRelative( rSimpleString ) AND ret;

    rSimpleString.Close();

    return ret;
}




#if 0
FUT_DECL( SimpleString, Ctor_Def );
FUT_DECL( SimpleString, Seek );
FUT_DECL( SimpleString, SeekBack );
FUT_DECL( SimpleString, SeekRelative );



FUT_DECL( SimpleString, Read )
{
    bool ret = true;

    rSimpleString.SeekBack(0);
    uintt nSourceSize = rSimpleString.Position();
    rSimpleString.Seek(0);

    char * pBuf = new char[nSourceSize+1];
    uintt nCount = rSimpleString.Read(pBuf,nSourceSize);

    UT_CHECK( Read, nCount == nSourceSize );

    return ret;
}

FUT_DECL( SimpleString, Write )
{
    bool ret = true;



    return ret;
}

FUT_DECL( SimpleString, Seek )
{
    bool ret = true;



    return ret;
}

FUT_DECL( SimpleString, SeekBack )
{
    bool ret = true;



    return ret;
}

FUT_DECL( SimpleString, SeekRelative )
{
    bool ret = true;



    return ret;
}

#endif


