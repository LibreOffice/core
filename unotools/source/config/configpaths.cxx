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

#include <sal/config.h>

#include <string_view>

#include <unotools/configpaths.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

namespace utl
{

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
        if (aLocalString.match("&amp;",nEscapePos))
            ch = '&';

        else if (aLocalString.match("&apos;",nEscapePos))
            ch = '\'';

        else if (aLocalString.match("&quot;",nEscapePos))
            ch = '"';

        OSL_ENSURE(ch,"Configuration path contains '&' that is not part of a valid character escape");
        if (ch)
        {
            aResult.append(std::u16string_view(aLocalString).substr(nStart,nEscapePos-nStart)).append(ch);

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

    aResult.append(std::u16string_view(aLocalString).substr(nStart));

    aLocalString = aResult.makeStringAndClear();
}

bool splitLastFromConfigurationPath(OUString const& _sInPath,
                                        OUString& _rsOutPath,
                                        OUString& _rsLocalName)
{
    sal_Int32 nStart,nEnd;

    sal_Int32 nPos = _sInPath.getLength()-1;

    // strip trailing slash
    if (nPos > 0 && _sInPath[ nPos ] == '/')
    {
        OSL_FAIL("Invalid config path: trailing '/' is not allowed");
        --nPos;
    }

    // check for predicate ['xxx'] or ["yyy"]
    if (nPos  > 0 && _sInPath[ nPos ] == ']')
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
        else // defined behavior for invalid paths
        {
            nStart = 0;
            nEnd = _sInPath.getLength();
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

    if (_sOutPath != nullptr)
    {
        *_sOutPath = (nSep >= 0) ? _sInPath.copy(nSep + 1) : OUString();
    }

    return sResult;
}

// find the position after the prefix in the nested path
static sal_Int32 lcl_findPrefixEnd(OUString const& _sNestedPath, OUString const& _sPrefixPath)
{
    // TODO: currently handles only exact prefix matches
    sal_Int32 nPrefixLength = _sPrefixPath.getLength();

    OSL_ENSURE(nPrefixLength == 0 || _sPrefixPath[nPrefixLength-1] != '/',
                "Cannot handle slash-terminated prefix paths");

    bool bIsPrefix;
    if (_sNestedPath.getLength() > nPrefixLength)
    {
        bIsPrefix = _sNestedPath[nPrefixLength] == '/' &&
                    _sNestedPath.startsWith(_sPrefixPath);
        ++nPrefixLength;
    }
    else if (_sNestedPath.getLength() == nPrefixLength)
    {
        bIsPrefix = _sNestedPath == _sPrefixPath;
    }
    else
    {
        bIsPrefix = false;
    }

    return bIsPrefix ? nPrefixLength : 0;
}

bool isPrefixOfConfigurationPath(OUString const& _sNestedPath,
                                     OUString const& _sPrefixPath)
{
    return _sPrefixPath.isEmpty() || lcl_findPrefixEnd(_sNestedPath,_sPrefixPath) != 0;
}

OUString dropPrefixFromConfigurationPath(OUString const& _sNestedPath,
                                         OUString const& _sPrefixPath)
{
    if ( sal_Int32 nPrefixEnd = lcl_findPrefixEnd(_sNestedPath,_sPrefixPath) )
    {
        return _sNestedPath.copy(nPrefixEnd);
    }
    else
    {
        OSL_ENSURE(_sPrefixPath.isEmpty(),  "Path does not start with expected prefix");

        return _sNestedPath;
    }
}

static
OUString lcl_wrapName(const OUString& _sContent, const OUString& _sType)
{
    const sal_Unicode * const pBeginContent = _sContent.getStr();
    const sal_Unicode * const pEndContent   = pBeginContent + _sContent.getLength();

    OSL_PRECOND(!_sType.isEmpty(), "Unexpected config type name: empty");
    OSL_PRECOND(pBeginContent <= pEndContent, "Invalid config name: empty");

    if (pBeginContent == pEndContent)
        return _sType;

    OUStringBuffer aNormalized(_sType.getLength() + _sContent.getLength() + 4); // reserve approximate size initially

    // prefix: type, opening bracket and quote
    aNormalized.append( _sType ).append( "['" );

    // content: copy over each char and handle escaping
    for(const sal_Unicode* pCur = pBeginContent; pCur != pEndContent; ++pCur)
    {
        // append (escape if needed)
        switch(*pCur)
        {
        case u'&' : aNormalized.append( "&amp;" ); break;
        case u'\'': aNormalized.append( "&apos;" ); break;
        case u'\"': aNormalized.append( "&quot;" ); break;

        default: aNormalized.append( *pCur );
        }
    }

    // suffix: closing quote and bracket
    aNormalized.append( "']" );

    return aNormalized.makeStringAndClear();
}

OUString wrapConfigurationElementName(OUString const& _sElementName)
{
    return lcl_wrapName(_sElementName, "*" );
}

OUString wrapConfigurationElementName(OUString const& _sElementName,
                                      OUString const& _sTypeName)
{
    // todo: check that _sTypeName is valid
    return lcl_wrapName(_sElementName, _sTypeName);
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
