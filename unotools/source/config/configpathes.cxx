/*************************************************************************
 *
 *  $RCSfile: configpathes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 16:57:31 $
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

#include "unotools/configpathes.hxx"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
        OSL_ENSURE(false, "Invalid config path: trailing '/' is not allowed");
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
OUString extractFirstFromConfigurationPath(OUString const& _sInPath)
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
            OSL_DEBUG_ONLY(nSep = nBracket+1);
            OSL_ENSURE(nSep == _sInPath.getLength() || _sInPath[nSep] == '/', "Invalid config path: brackets not followed by slash");
        }
        else // ... but our initial element name is in simple form
            nStart = 0;
    }

    OUString sResult = (nEnd >= 0) ? _sInPath.copy(nStart, nEnd-nStart) : _sInPath;
    lcl_resolveCharEntities(sResult);
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
