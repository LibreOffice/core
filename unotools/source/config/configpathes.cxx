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
#include "precompiled_unotools.hxx"

#include "unotools/configpathes.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

//----------------------------------------------------------------------------
namespace utl
{
//----------------------------------------------------------------------------

    using ::rtl::OUString;
    using ::rtl::OUStringBuffer;

//----------------------------------------------------------------------------

static
void lcl_resolveCharEntities(OUString & aLocalString)
{
    sal_Int32 nEscapePos=aLocalString.indexOf('&');
    if (nEscapePos < 0) return;

    OUStringBuffer aResult;
    sal_Int32 nStart = 0;

    do
    {
        sal_Unicode ch = 0;
        if (aLocalString.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("&amp;"),nEscapePos))
            ch = '&';

        else if (aLocalString.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("&apos;"),nEscapePos))
            ch = '\'';

        else if (aLocalString.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("&quot;"),nEscapePos))
            ch = '"';

        OSL_ENSURE(ch,"Configuration path contains '&' that is not part of a valid character escape");
        if (ch)
        {
            aResult.append(aLocalString.copy(nStart,nEscapePos-nStart)).append(ch);

            sal_Int32 nEscapeEnd=aLocalString.indexOf(';',nEscapePos);
            nStart = nEscapeEnd+1;
            nEscapePos=aLocalString.indexOf('&',nStart);
        }
        else
        {
            nEscapePos=aLocalString.indexOf('&',nEscapePos+1);
        }
    }
    while ( nEscapePos > 0);

    aResult.append(aLocalString.copy(nStart));

    aLocalString = aResult.makeStringAndClear();
}

//----------------------------------------------------------------------------
sal_Bool splitLastFromConfigurationPath(OUString const& _sInPath,
                                        OUString& _rsOutPath,
                                        OUString& _rsLocalName)
{
    sal_Int32 nStart,nEnd;

    sal_Int32 nPos = _sInPath.getLength()-1;

    // strip trailing slash
    if (nPos > 0 && _sInPath[ nPos ] == sal_Unicode('/'))
    {
        OSL_FAIL("Invalid config path: trailing '/' is not allowed");
        --nPos;
    }

    // check for predicate ['xxx'] or ["yyy"]
    if (nPos  > 0 && _sInPath[ nPos ] == sal_Unicode(']'))
    {
        sal_Unicode chQuote = _sInPath[--nPos];

        if (chQuote == '\'' || chQuote == '\"')
        {
            nEnd = nPos;
            nPos = _sInPath.lastIndexOf(chQuote,nEnd);
            nStart = nPos + 1;
            --nPos; // nPos = rInPath.lastIndexOf('[',nPos);
        }
        else // allow [xxx]
        {
            nEnd = nPos + 1;
            nPos = _sInPath.lastIndexOf('[',nEnd);
            nStart = nPos + 1;
        }

        OSL_ENSURE(nPos >= 0 && _sInPath[nPos] == '[', "Invalid config path: unmatched quotes or brackets");
        if (nPos >= 0 && _sInPath[nPos] == '[')
        {
            nPos =  _sInPath.lastIndexOf('/',nPos);
        }
        else // defined behavior for invalid pathes
        {
            nStart = 0, nEnd = _sInPath.getLength();
            nPos = -1;
        }

    }
    else
    {
        nEnd = nPos+1;
        nPos = _sInPath.lastIndexOf('/',nEnd);
        nStart = nPos + 1;
    }
    OSL_ASSERT( -1 <= nPos &&
                nPos < nStart &&
                nStart < nEnd &&
                nEnd <= _sInPath.getLength() );

    OSL_ASSERT(nPos == -1 || _sInPath[nPos] == '/');
    OSL_ENSURE(nPos != 0 , "Invalid config child path: immediate child of root");

    _rsLocalName = _sInPath.copy(nStart, nEnd-nStart);
    _rsOutPath = (nPos > 0) ? _sInPath.copy(0,nPos) : OUString();
    lcl_resolveCharEntities(_rsLocalName);

    return nPos >= 0;
}

//----------------------------------------------------------------------------
OUString extractFirstFromConfigurationPath(OUString const& _sInPath, OUString* _sOutPath)
{
    sal_Int32 nSep      = _sInPath.indexOf('/');
    sal_Int32 nBracket  = _sInPath.indexOf('[');

    sal_Int32 nStart    = nBracket + 1;
    sal_Int32 nEnd      = nSep;

    if (0 <= nBracket) // found a bracket-quoted relative path
    {
        if (nSep < 0 || nBracket < nSep) // and the separator comes after it
        {
            sal_Unicode chQuote = _sInPath[nStart];
            if (chQuote == '\'' || chQuote == '\"')
            {
                ++nStart;
                nEnd      = _sInPath.indexOf(chQuote, nStart+1);
                nBracket  = nEnd+1;
            }
            else
            {
                nEnd = _sInPath.indexOf(']',nStart);
                nBracket = nEnd;
            }
            OSL_ENSURE(nEnd > nStart && _sInPath[nBracket] == ']', "Invalid config path: improper mismatch of quote or bracket");
            OSL_ENSURE((nBracket+1 == _sInPath.getLength() && nSep == -1) || (_sInPath[nBracket+1] == '/' && nSep == nBracket+1), "Invalid config path: brackets not followed by slash");
        }
        else // ... but our initial element name is in simple form
            nStart = 0;
    }

    OUString sResult = (nEnd >= 0) ? _sInPath.copy(nStart, nEnd-nStart) : _sInPath;
    lcl_resolveCharEntities(sResult);

    if (_sOutPath != 0)
    {
        *_sOutPath = (nSep >= 0) ? _sInPath.copy(nSep + 1) : OUString();
    }

    return sResult;
}

//----------------------------------------------------------------------------

// find the position after the prefix in the nested path
static inline
sal_Int32 lcl_findPrefixEnd(OUString const& _sNestedPath, OUString const& _sPrefixPath)
{
    // TODO: currently handles only exact prefix matches
    sal_Int32 nPrefixLength = _sPrefixPath.getLength();

    OSL_ENSURE(nPrefixLength == 0 || _sPrefixPath[nPrefixLength-1] != '/',
                "Cannot handle slash-terminated prefix pathes");

    sal_Bool bIsPrefix;
    if (_sNestedPath.getLength() > nPrefixLength)
    {
        bIsPrefix = _sNestedPath[nPrefixLength] == '/' &&
                    _sNestedPath.compareTo(_sPrefixPath,nPrefixLength) == 0;
        ++nPrefixLength;
    }
    else if (_sNestedPath.getLength() == nPrefixLength)
    {
        bIsPrefix = _sNestedPath.equals(_sPrefixPath);
    }
    else
    {
        bIsPrefix = false;
    }

    return bIsPrefix ? nPrefixLength : 0;
}

//----------------------------------------------------------------------------
sal_Bool isPrefixOfConfigurationPath(OUString const& _sNestedPath,
                                     OUString const& _sPrefixPath)
{
    return _sPrefixPath.getLength() == 0 || lcl_findPrefixEnd(_sNestedPath,_sPrefixPath) != 0;
}

//----------------------------------------------------------------------------
OUString dropPrefixFromConfigurationPath(OUString const& _sNestedPath,
                                         OUString const& _sPrefixPath)
{
    if ( sal_Int32 nPrefixEnd = lcl_findPrefixEnd(_sNestedPath,_sPrefixPath) )
    {
        return _sNestedPath.copy(nPrefixEnd);
    }
    else
    {
        OSL_ENSURE(_sPrefixPath.getLength() == 0,  "Path does not start with expected prefix");

        return _sNestedPath;
    }
}

//----------------------------------------------------------------------------
static
OUString lcl_wrapName(const OUString& _sContent, const OUString& _sType)
{
    const sal_Unicode * const pBeginContent = _sContent.getStr();
    const sal_Unicode * const pEndContent   = pBeginContent + _sContent.getLength();

    OSL_PRECOND(_sType.getLength(), "Unexpected config type name: empty");
    OSL_PRECOND(pBeginContent <= pEndContent, "Invalid config name: empty");

    if (pBeginContent == pEndContent)
        return _sType;

    rtl::OUStringBuffer aNormalized(_sType.getLength() + _sContent.getLength() + 4); // reserve approximate size initially

    // prefix: type, opening bracket and quote
    aNormalized.append( _sType ).appendAscii( RTL_CONSTASCII_STRINGPARAM("['") );

    // content: copy over each char and handle escaping
    for(const sal_Unicode* pCur = pBeginContent; pCur != pEndContent; ++pCur)
    {
        // append (escape if needed)
        switch(*pCur)
        {
        case sal_Unicode('&') : aNormalized.appendAscii( RTL_CONSTASCII_STRINGPARAM("&amp;") ); break;
        case sal_Unicode('\''): aNormalized.appendAscii( RTL_CONSTASCII_STRINGPARAM("&apos;") ); break;
        case sal_Unicode('\"'): aNormalized.appendAscii( RTL_CONSTASCII_STRINGPARAM("&quot;") ); break;

        default: aNormalized.append( *pCur );
        }
    }

    // suffix: closing quote and bracket
    aNormalized.appendAscii( RTL_CONSTASCII_STRINGPARAM("']") );

    return aNormalized.makeStringAndClear();
}

//----------------------------------------------------------------------------

OUString wrapConfigurationElementName(OUString const& _sElementName)
{
    return lcl_wrapName(_sElementName, OUString(RTL_CONSTASCII_USTRINGPARAM("*")) );
}

//----------------------------------------------------------------------------

OUString wrapConfigurationElementName(OUString const& _sElementName,
                                      OUString const& _sTypeName)
{
    // todo: check that _sTypeName is valid
    return lcl_wrapName(_sElementName, _sTypeName);
}

//----------------------------------------------------------------------------
} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
