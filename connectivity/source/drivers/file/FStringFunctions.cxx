/*************************************************************************
 *
 *  $RCSfile: FStringFunctions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:25:52 $
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

#include "file/FStringFunctions.hxx"

using namespace connectivity;
using namespace connectivity::file;
//------------------------------------------------------------------
ORowSetValue OOp_Upper::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiUpperCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Lower::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiLowerCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Ascii::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;
    ::rtl::OString sStr(::rtl::OUStringToOString(lhs,RTL_TEXTENCODING_ASCII_US));
    sal_Int32 nAscii = sStr.toChar();
    return nAscii;
}
//------------------------------------------------------------------
ORowSetValue OOp_CharLength::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().getLength();
}
//------------------------------------------------------------------
ORowSetValue OOp_Char::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() )
        return ORowSetValue();

    ::rtl::OUString sRet;
    ::std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    ::std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if ( !aIter->isNull() )
        {
            sal_Char c = static_cast<sal_Char>(static_cast<sal_Int32>(*aIter));

            sRet += ::rtl::OUString(&c,1,RTL_TEXTENCODING_ASCII_US);
        }
    }

    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Concat::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() )
        return ORowSetValue();

    ::rtl::OUString sRet;
    ::std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    ::std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();

        sRet +=  *aIter;
    }

    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Locate::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    ::std::vector<ORowSetValue>::const_iterator aIter = lhs.begin();
    ::std::vector<ORowSetValue>::const_iterator aEnd = lhs.end();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();
    }
    if ( lhs.size() == 2 )
        return ::rtl::OUString::valueOf(lhs[0].getString().indexOf(lhs[1].getString())+1);

    else if ( lhs.size() != 3 )
        return ORowSetValue();

    return lhs[1].getString().indexOf(lhs[2].getString(),lhs[0]) + 1;
}
//------------------------------------------------------------------
ORowSetValue OOp_SubString::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    ::std::vector<ORowSetValue>::const_iterator aIter = lhs.begin();
    ::std::vector<ORowSetValue>::const_iterator aEnd = lhs.end();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();
    }
    if ( lhs.size() == 2 && static_cast<sal_Int32>(lhs[0]) >= sal_Int32(0) )
        return lhs[1].getString().copy(static_cast<sal_Int32>(lhs[0])-1);

    else if ( lhs.size() != 3 || static_cast<sal_Int32>(lhs[1]) < sal_Int32(0))
        return ORowSetValue();

    return lhs[2].getString().copy(static_cast<sal_Int32>(lhs[1])-1,lhs[0]);
}
//------------------------------------------------------------------
ORowSetValue OOp_LTrim::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet(lhs);
    ::rtl::OUString sNew = sRet.trim();
    return sRet.copy(sRet.indexOf(sNew));
}
//------------------------------------------------------------------
ORowSetValue OOp_RTrim::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet(lhs);
    ::rtl::OUString sNew = sRet.trim();
    return sRet.copy(0,sRet.lastIndexOf(sNew.getStr()[sNew.getLength()-1])+1);
}
//------------------------------------------------------------------
ORowSetValue OOp_Space::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    const sal_Char c = ' ';
    ::rtl::OUString sRet;
    sal_Int32 nCount = lhs;
    for (sal_Int32 i=0; i < nCount; ++i)
    {
        sRet += ::rtl::OUString(&c,1,RTL_TEXTENCODING_ASCII_US);
    }
    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Replace::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.size() != 3 )
        return ORowSetValue();

    ::rtl::OUString sStr(lhs[2]);
    ::rtl::OUString sFrom(lhs[1]);
    ::rtl::OUString sTo(lhs[0]);
    sal_Int32 nIndexOf = sStr.indexOf(sFrom);
    while( nIndexOf != -1 )
    {
        sStr = sStr.replaceAt(nIndexOf,sFrom.getLength(),sTo);
        nIndexOf = sStr.indexOf(sFrom,nIndexOf + sTo.getLength());
    }

    return sStr;
}
//------------------------------------------------------------------
ORowSetValue OOp_Repeat::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    ::rtl::OUString sRet;
    sal_Int32 nCount = rhs;
    for (sal_Int32 i=0; i < nCount; ++i)
    {
        sRet += lhs;
    }
    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Insert::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.size() != 4 )
        return ORowSetValue();

    ::rtl::OUString sStr(lhs[3]);

    sal_Int32 nStart = static_cast<sal_Int32>(lhs[2]);
    if ( nStart < 1 )
        nStart = 1;
    return sStr.replaceAt(nStart-1,static_cast<sal_Int32>(lhs[1]),lhs[0]);
}
//------------------------------------------------------------------
ORowSetValue OOp_Left::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    ::rtl::OUString sRet(lhs);
    sal_Int32 nCount = rhs;
    if ( nCount < 0 )
        return ORowSetValue();
    return sRet.copy(0,nCount);
}
//------------------------------------------------------------------
ORowSetValue OOp_Right::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    sal_Int32 nCount = rhs;
    ::rtl::OUString sRet(lhs);
    if ( nCount < 0 || nCount >= sRet.getLength() )
        return ORowSetValue();

    return sRet.copy(sRet.getLength()-nCount,nCount);
}

