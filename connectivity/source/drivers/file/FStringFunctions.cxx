/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "file/FStringFunctions.hxx"
#include <rtl/ustrbuf.hxx>

using namespace connectivity;
using namespace connectivity::file;
//------------------------------------------------------------------
ORowSetValue OOp_Upper::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Upper::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiUpperCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Lower::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Lower::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiLowerCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Ascii::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Ascii::operate" );
    if ( lhs.isNull() )
        return lhs;
    OString sStr(OUStringToOString(lhs,RTL_TEXTENCODING_ASCII_US));
    sal_Int32 nAscii = sStr.toChar();
    return nAscii;
}
//------------------------------------------------------------------
ORowSetValue OOp_CharLength::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_CharLength::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().getLength();
}
//------------------------------------------------------------------
ORowSetValue OOp_Char::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Char::operate" );
    if ( lhs.empty() )
        return ORowSetValue();

    OUString sRet;
    ::std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    ::std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if ( !aIter->isNull() )
        {
            sal_Char c = static_cast<sal_Char>(static_cast<sal_Int32>(*aIter));

            sRet += OUString(&c,1,RTL_TEXTENCODING_ASCII_US);
        }
    }

    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Concat::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Concat::operate" );
    if ( lhs.empty() )
        return ORowSetValue();

    OUStringBuffer sRet;
    ::std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    ::std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();

        sRet.append(aIter->operator OUString());
    }

    return sRet.makeStringAndClear();
}
//------------------------------------------------------------------
ORowSetValue OOp_Locate::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Locate::operate" );
    ::std::vector<ORowSetValue>::const_iterator aIter = lhs.begin();
    ::std::vector<ORowSetValue>::const_iterator aEnd = lhs.end();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();
    }
    if ( lhs.size() == 2 )
        return OUString::number(lhs[0].getString().indexOf(lhs[1].getString())+1);

    else if ( lhs.size() != 3 )
        return ORowSetValue();

    return lhs[1].getString().indexOf(lhs[2].getString(),lhs[0]) + 1;
}
//------------------------------------------------------------------
ORowSetValue OOp_SubString::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_SubString::operate" );
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
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_LTrim::operate" );
    if ( lhs.isNull() )
        return lhs;

    OUString sRet = lhs;
    OUString sNew = sRet.trim();
    return sRet.copy(sRet.indexOf(sNew));
}
//------------------------------------------------------------------
ORowSetValue OOp_RTrim::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_RTrim::operate" );
    if ( lhs.isNull() )
        return lhs;

    OUString sRet = lhs;
    OUString sNew = sRet.trim();
    return sRet.copy(0,sRet.lastIndexOf(sNew.getStr()[sNew.getLength()-1])+1);
}
//------------------------------------------------------------------
ORowSetValue OOp_Space::operate(const ORowSetValue& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Space::operate" );
    if ( lhs.isNull() )
        return lhs;

    const sal_Char c = ' ';
    OUStringBuffer sRet;
    sal_Int32 nCount = lhs;
    for (sal_Int32 i=0; i < nCount; ++i)
    {
        sRet.appendAscii(&c,1);
    }
    return sRet.makeStringAndClear();
}
//------------------------------------------------------------------
ORowSetValue OOp_Replace::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Replace::operate" );
    if ( lhs.size() != 3 )
        return ORowSetValue();

    OUString sStr  = lhs[2];
    OUString sFrom = lhs[1];
    OUString sTo   = lhs[0];
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
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Repeat::operate" );
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    OUString sRet;
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
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Insert::operate" );
    if ( lhs.size() != 4 )
        return ORowSetValue();

    OUString sStr = lhs[3];

    sal_Int32 nStart = static_cast<sal_Int32>(lhs[2]);
    if ( nStart < 1 )
        nStart = 1;
    return sStr.replaceAt(nStart-1,static_cast<sal_Int32>(lhs[1]),lhs[0]);
}
//------------------------------------------------------------------
ORowSetValue OOp_Left::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Left::operate" );
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    OUString sRet = lhs;
    sal_Int32 nCount = rhs;
    if ( nCount < 0 )
        return ORowSetValue();
    return sRet.copy(0,nCount);
}
//------------------------------------------------------------------
ORowSetValue OOp_Right::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OOp_Right::operate" );
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    sal_Int32 nCount = rhs;
    OUString sRet = lhs;
    if ( nCount < 0 || nCount >= sRet.getLength() )
        return ORowSetValue();

    return sRet.copy(sRet.getLength()-nCount,nCount);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
