/*************************************************************************
 *
 *  $RCSfile: string_ut.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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


