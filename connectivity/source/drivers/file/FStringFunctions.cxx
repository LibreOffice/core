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
#include "precompiled_connectivity.hxx"

#include "file/FStringFunctions.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

using namespace connectivity;
using namespace connectivity::file;
//------------------------------------------------------------------
ORowSetValue OOp_Upper::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Upper::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiUpperCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Lower::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Lower::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().toAsciiLowerCase();
}
//------------------------------------------------------------------
ORowSetValue OOp_Ascii::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Ascii::operate" );
    if ( lhs.isNull() )
        return lhs;
    ::rtl::OString sStr(::rtl::OUStringToOString(lhs,RTL_TEXTENCODING_ASCII_US));
    sal_Int32 nAscii = sStr.toChar();
    return nAscii;
}
//------------------------------------------------------------------
ORowSetValue OOp_CharLength::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_CharLength::operate" );
    if ( lhs.isNull() )
        return lhs;

    return lhs.getString().getLength();
}
//------------------------------------------------------------------
ORowSetValue OOp_Char::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Char::operate" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Concat::operate" );
    if ( lhs.empty() )
        return ORowSetValue();

    ::rtl::OUStringBuffer sRet;
    ::std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    ::std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if ( aIter->isNull() )
            return ORowSetValue();

        sRet.append(aIter->operator ::rtl::OUString());
    }

    return sRet.makeStringAndClear();
}
//------------------------------------------------------------------
ORowSetValue OOp_Locate::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Locate::operate" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_SubString::operate" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_LTrim::operate" );
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet = lhs;
    ::rtl::OUString sNew = sRet.trim();
    return sRet.copy(sRet.indexOf(sNew));
}
//------------------------------------------------------------------
ORowSetValue OOp_RTrim::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_RTrim::operate" );
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet = lhs;
    ::rtl::OUString sNew = sRet.trim();
    return sRet.copy(0,sRet.lastIndexOf(sNew.getStr()[sNew.getLength()-1])+1);
}
//------------------------------------------------------------------
ORowSetValue OOp_Space::operate(const ORowSetValue& lhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Space::operate" );
    if ( lhs.isNull() )
        return lhs;

    const sal_Char c = ' ';
    ::rtl::OUStringBuffer sRet;
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Replace::operate" );
    if ( lhs.size() != 3 )
        return ORowSetValue();

    ::rtl::OUString sStr  = lhs[2];
    ::rtl::OUString sFrom = lhs[1];
    ::rtl::OUString sTo   = lhs[0];
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Repeat::operate" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Insert::operate" );
    if ( lhs.size() != 4 )
        return ORowSetValue();

    ::rtl::OUString sStr = lhs[3];

    sal_Int32 nStart = static_cast<sal_Int32>(lhs[2]);
    if ( nStart < 1 )
        nStart = 1;
    return sStr.replaceAt(nStart-1,static_cast<sal_Int32>(lhs[1]),lhs[0]);
}
//------------------------------------------------------------------
ORowSetValue OOp_Left::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Left::operate" );
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    ::rtl::OUString sRet = lhs;
    sal_Int32 nCount = rhs;
    if ( nCount < 0 )
        return ORowSetValue();
    return sRet.copy(0,nCount);
}
//------------------------------------------------------------------
ORowSetValue OOp_Right::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OOp_Right::operate" );
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    sal_Int32 nCount = rhs;
    ::rtl::OUString sRet = lhs;
    if ( nCount < 0 || nCount >= sRet.getLength() )
        return ORowSetValue();

    return sRet.copy(sRet.getLength()-nCount,nCount);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
