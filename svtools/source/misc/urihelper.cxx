/*************************************************************************
 *
 *  $RCSfile: urihelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:20 $
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

#ifndef SVTOOLS_URIHELPER_HXX
#include <urihelper.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_FILESYSTEMNOTATION_HPP_
#include <com/sun/star/ucb/FileSystemNotation.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_INETMIME_HXX
#include <tools/inetmime.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

namespace unnamed_svtools_urihelper {}
using namespace unnamed_svtools_urihelper;
    // unnamed namespaces don't work well yet...

using namespace com::sun;
using namespace com::sun::star;

//============================================================================
//
//  SmartRel2Abs
//
//============================================================================

namespace unnamed_svtools_urihelper {

inline UniString toUniString(ByteString const & rString)
{
    return UniString(rString, RTL_TEXTENCODING_ISO_8859_1);
}

inline UniString toUniString(UniString const & rString)
{
    return rString;
}

template< typename Str >
inline UniString SmartRel2Abs_Impl(INetURLObject const & rTheBaseURIRef,
                                   Str const & rTheRelURIRef,
                                   Link const & rMaybeFileHdl,
                                   bool bCheckFileExists,
                                   bool bIgnoreFragment,
                                   INetURLObject::EncodeMechanism
                                       eEncodeMechanism,
                                   INetURLObject::DecodeMechanism
                                       eDecodeMechanism,
                                   rtl_TextEncoding eCharset,
                                   bool bRelativeNonURIs,
                                   INetURLObject::FSysStyle eStyle)
{
    // Backwards compatibility:
    if (rTheRelURIRef.Len() != 0 && rTheRelURIRef.GetChar(0) == '#')
        return toUniString(rTheRelURIRef);

    INetURLObject aAbsURIRef;
    if (rTheBaseURIRef.HasError())
        aAbsURIRef.
            SetSmartURL(rTheRelURIRef, eEncodeMechanism, eCharset, eStyle);
    else
    {
        bool bWasAbsolute;
        aAbsURIRef = rTheBaseURIRef.smartRel2Abs(rTheRelURIRef,
                                                 bWasAbsolute,
                                                 bIgnoreFragment,
                                                 eEncodeMechanism,
                                                 eCharset,
                                                 bRelativeNonURIs,
                                                 eStyle);
        if (bCheckFileExists
            && !bWasAbsolute
            && (aAbsURIRef.GetProtocol() == INET_PROT_FILE
                || aAbsURIRef.GetProtocol() == INET_PROT_VND_SUN_STAR_WFS))
        {
            INetURLObject aNonFileURIRef;
            aNonFileURIRef.SetSmartURL(rTheRelURIRef,
                                       eEncodeMechanism,
                                       eCharset,
                                       eStyle);
            if (!aNonFileURIRef.HasError()
                && aNonFileURIRef.GetProtocol() != INET_PROT_FILE)
            {
                bool bMaybeFile = false;
                if (rMaybeFileHdl.IsSet())
                {
                    UniString aFilePath(toUniString(rTheRelURIRef));
                    bMaybeFile = rMaybeFileHdl.Call(&aFilePath) != 0;
                }
                if (!bMaybeFile)
                    aAbsURIRef = aNonFileURIRef;
            }
        }
    }
    return aAbsURIRef.GetMainURL(eDecodeMechanism, eCharset);
}

}

UniString
URIHelper::SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
                        ByteString const & rTheRelURIRef,
                        Link const & rMaybeFileHdl,
                        bool bCheckFileExists,
                        bool bIgnoreFragment,
                        INetURLObject::EncodeMechanism eEncodeMechanism,
                        INetURLObject::DecodeMechanism eDecodeMechanism,
                        rtl_TextEncoding eCharset,
                        bool bRelativeNonURIs,
                        INetURLObject::FSysStyle eStyle)
{
    return SmartRel2Abs_Impl(rTheBaseURIRef, rTheRelURIRef, rMaybeFileHdl,
                             bCheckFileExists, bIgnoreFragment,
                             eEncodeMechanism, eDecodeMechanism, eCharset,
                             bRelativeNonURIs, eStyle);
}

UniString
URIHelper::SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
                        UniString const & rTheRelURIRef,
                        Link const & rMaybeFileHdl,
                        bool bCheckFileExists,
                        bool bIgnoreFragment,
                        INetURLObject::EncodeMechanism eEncodeMechanism,
                        INetURLObject::DecodeMechanism eDecodeMechanism,
                        rtl_TextEncoding eCharset,
                        bool bRelativeNonURIs,
                        INetURLObject::FSysStyle eStyle)
{
    return SmartRel2Abs_Impl(rTheBaseURIRef, rTheRelURIRef, rMaybeFileHdl,
                             bCheckFileExists, bIgnoreFragment,
                             eEncodeMechanism, eDecodeMechanism, eCharset,
                             bRelativeNonURIs, eStyle);
}

//============================================================================
//
//  SetMaybeFileHdl
//
//============================================================================

namespace unnamed_svtools_urihelper {

static Link aMaybeFileHdl;

}

void URIHelper::SetMaybeFileHdl(Link const & rTheMaybeFileHdl)
{
    aMaybeFileHdl = rTheMaybeFileHdl;
}

//============================================================================
//
//  GetMaybeFileHdl
//
//============================================================================

Link URIHelper::GetMaybeFileHdl()
{
    return aMaybeFileHdl;
}

//============================================================================
//
//  FindFirstURLInText
//
//============================================================================

namespace unnamed_svtools_urihelper {

inline xub_StrLen nextChar(UniString const & rStr, xub_StrLen nPos)
{
    return INetMIME::isHighSurrogate(rStr.GetChar(nPos))
           && rStr.Len() - nPos >= 2
           && INetMIME::isLowSurrogate(rStr.GetChar(nPos + 1)) ?
               nPos + 2 : nPos + 1;
}

bool isBoundary1(CharClass const & rCharClass, UniString const & rStr,
                 xub_StrLen nPos, xub_StrLen nEnd)
{
    if (nPos == nEnd)
        return true;
    if (rCharClass.isLetterNumeric(rStr, nPos))
        return false;
    switch (rStr.GetChar(nPos))
    {
    case '$':
    case '%':
    case '&':
    case '-':
    case '/':
    case '@':
    case '\\':
        return false;
    default:
        return true;
    }
}

bool isBoundary2(CharClass const & rCharClass, UniString const & rStr,
                 xub_StrLen nPos, xub_StrLen nEnd)
{
    if (nPos == nEnd)
        return true;
    if (rCharClass.isLetterNumeric(rStr, nPos))
        return false;
    switch (rStr.GetChar(nPos))
    {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '/':
    case '=':
    case '?':
    case '@':
    case '^':
    case '_':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
        return false;
    default:
        return true;
    }
}

bool checkWChar(CharClass const & rCharClass, UniString const & rStr,
                xub_StrLen * pPos, xub_StrLen * pEnd, bool bBackslash = false,
                bool bPipe = false)
{
    sal_Unicode c = rStr.GetChar(*pPos);
    if (INetMIME::isUSASCII(c))
    {
        static sal_uInt8 const aMap[128]
            = { 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 0, 4, 4, 4, 1,   //  !"#$%&'
                1, 1, 1, 1, 1, 4, 1, 4,   // ()*+,-./
                4, 4, 4, 4, 4, 4, 4, 4,   // 01234567
                4, 4, 1, 1, 0, 1, 0, 1,   // 89:;<=>?
                4, 4, 4, 4, 4, 4, 4, 4,   // @ABCDEFG
                4, 4, 4, 4, 4, 4, 4, 4,   // HIJKLMNO
                4, 4, 4, 4, 4, 4, 4, 4,   // PQRSTUVW
                4, 4, 4, 1, 2, 1, 0, 1,   // XYZ[\]^_
                0, 4, 4, 4, 4, 4, 4, 4,   // `abcdefg
                4, 4, 4, 4, 4, 4, 4, 4,   // hijklmno
                4, 4, 4, 4, 4, 4, 4, 4,   // pqrstuvw
                4, 4, 4, 0, 3, 0, 1, 0 }; // xyz{|}~
        switch (aMap[c])
        {
            default: // not uric
                return false;

            case 1: // uric
                ++(*pPos);
                return true;

            case 2: // "\"
                if (bBackslash)
                {
                    *pEnd = ++(*pPos);
                    return true;
                }
                else
                    return false;

            case 3: // "|"
                if (bPipe)
                {
                    *pEnd = ++(*pPos);
                    return true;
                }
                else
                    return false;

            case 4: // alpha, digit, "$", "%", "&", "-", "/", "@" (see
                    // isBoundary1)
                *pEnd = ++(*pPos);
                return true;
        }
    }
    else if (rCharClass.isLetterNumeric(rStr, *pPos))
    {
        *pEnd = *pPos = nextChar(rStr, *pPos);
        return true;
    }
    else
        return false;
}

sal_uInt32 scanDomain(UniString const & rStr, xub_StrLen * pPos,
                      xub_StrLen nEnd)
{
    sal_Unicode const * pBuffer = rStr.GetBuffer();
    sal_Unicode const * p = pBuffer + *pPos;
    sal_uInt32 nLabels = INetURLObject::scanDomain(p, pBuffer + nEnd, false);
    *pPos = p - pBuffer;
    return nLabels;
}

}

UniString
URIHelper::FindFirstURLInText(UniString const & rText,
                              xub_StrLen & rBegin,
                              xub_StrLen & rEnd,
                              CharClass const & rCharClass,
                              INetURLObject::EncodeMechanism eMechanism,
                              rtl_TextEncoding eCharset,
                              INetURLObject::FSysStyle eStyle)
{
    if (!(rBegin <= rEnd && rEnd <= rText.Len()))
        return UniString();

    // Search for the first substring of [rBegin..rEnd[ that matches any of the
    // following productions (for which the appropriate style bit is set in
    // eStyle, if applicable).
    //
    // 1st Production (known scheme):
    //    \B1 <one of the known schemes, except file> ":" 1*wchar ["#" 1*wchar]
    //        \B1
    //
    // 2nd Production (file):
    //    \B1 "FILE:" 1*(wchar / "\" / "|") ["#" 1*wchar] \B1
    //
    // 3rd Production (ftp):
    //    \B1 "FTP" 2*("." label) ["/" *wchar] ["#" 1*wchar] \B1
    //
    // 4th Production (http):
    //    \B1 "WWW" 2*("." label) ["/" *wchar] ["#" 1*wchar] \B1
    //
    // 5th Production (mailto):
    //    \B2 local-part "@" domain \B1
    //
    // 6th Production (UNC file):
    //    \B1 "\\" domain "\" *(wchar / "\") \B1
    //
    // 7th Production (DOS file):
    //    \B1 ALPHA ":\" *(wchar / "\") \B1
    //
    // 8th Production (Unix-like DOS file):
    //    \B1 ALPHA ":/" *(wchar / "\") \B1
    //
    // The productions use the following auxiliary rules.
    //
    //    local-part = atom *("." atom)
    //    atom = 1*(alphanum / "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+"
    //              / "-" / "/" / "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}"
    //              / "~")
    //    domain = label *("." label)
    //    label = alphanum [*(alphanum / "-") alphanum]
    //    alphanum = ALPHA / DIGIT
    //    wchar = <any uric character (ignoring the escaped rule), or "%", or
    //             a letter or digit (according to rCharClass)>
    //
    // "\B1" (boundary 1) stands for the beginning or end of the block of text,
    // or a character that is neither (a) a letter or digit (according to
    // rCharClass), nor (b) any of "$", "%", "&", "-", "/", "@", or "\".
    // (FIXME:  What was the rationale for this set of punctuation characters?)
    //
    // "\B2" (boundary 2) stands for the beginning or end of the block of text,
    // or a character that is neither (a) a letter or digit (according to
    // rCharClass), nor (b) any of "!", "#", "$", "%", "&", "'", "*", "+", "-",
    // "/", "=", "?", "@", "^", "_", "`", "{", "|", "}", or "~" (i.e., an RFC
    // 822 <atom> character, or "@" from \B1's set above).
    //
    // Productions 1--4, and 6--8 try to find a maximum-length match, but they
    // stop at the first <wchar> character that is a "\B1" character which is
    // only followed by "\B1" characters (taking "\" and "|" characters into
    // account appropriately).  Production 5 simply tries to find a maximum-
    // length match.
    //
    // Productions 1--4 use the given eMechanism and eCharset.  Productions 5--9
    // use ENCODE_ALL.
    //
    // Productions 6--9 are only applicable if the FSYS_DOS bit is set in
    // eStyle.

    bool bBoundary1 = true;
    bool bBoundary2 = true;
    for (xub_StrLen nPos = rBegin; nPos != rEnd; nPos = nextChar(rText, nPos))
    {
        sal_Unicode c = rText.GetChar(nPos);
        if (bBoundary1)
        {
            if (INetMIME::isAlpha(c))
            {
                xub_StrLen i = nPos;
                INetProtocol eScheme
                    = INetURLObject::CompareProtocolScheme(UniString(rText, i,
                                                                     rEnd));
                if (eScheme == INET_PROT_FILE) // 2nd
                {
                    while (rText.GetChar(i++) != ':');
                    xub_StrLen nPrefixEnd = i;
                    xub_StrLen nUriEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd, true,
                                         true));
                    if (i != nPrefixEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    }
                    if (nUriEnd != nPrefixEnd
                        && isBoundary1(rCharClass, rText, nUriEnd, rEnd))
                    {
                        INetURLObject aUri(UniString(rText, nPos,
                                                     nUriEnd - nPos),
                                           INET_PROT_FILE, eMechanism, eCharset,
                                           eStyle);
                        if (!aUri.HasError())
                        {
                            rBegin = nPos;
                            rEnd = nUriEnd;
                            return
                                aUri.GetMainURL(INetURLObject::DECODE_TO_IURI);
                        }
                    }
                }
                else if (eScheme != INET_PROT_NOT_VALID) // 1st
                {
                    while (rText.GetChar(i++) != ':');
                    xub_StrLen nPrefixEnd = i;
                    xub_StrLen nUriEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    if (i != nPrefixEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    }
                    if (nUriEnd != nPrefixEnd
                        && (isBoundary1(rCharClass, rText, nUriEnd, rEnd)
                            || rText.GetChar(nUriEnd) == '\\'))
                    {
                        INetURLObject aUri(UniString(rText, nPos,
                                                     nUriEnd - nPos),
                                           INET_PROT_HTTP, eMechanism,
                                           eCharset);
                        if (!aUri.HasError())
                        {
                            rBegin = nPos;
                            rEnd = nUriEnd;
                            return
                                aUri.GetMainURL(INetURLObject::DECODE_TO_IURI);
                        }
                    }
                }

                // 3rd, 4th:
                i = nPos;
                sal_uInt32 nLabels = scanDomain(rText, &i, rEnd);
                if (nLabels >= 3
                    && rText.GetChar(nPos + 3) == '.'
                    && (((rText.GetChar(nPos) == 'w'
                          || rText.GetChar(nPos) == 'W')
                         && (rText.GetChar(nPos + 1) == 'w'
                             || rText.GetChar(nPos + 1) == 'W')
                         && (rText.GetChar(nPos + 2) == 'w'
                             || rText.GetChar(nPos + 2) == 'W'))
                        || ((rText.GetChar(nPos) == 'f'
                             || rText.GetChar(nPos) == 'F')
                            && (rText.GetChar(nPos + 1) == 't'
                                || rText.GetChar(nPos + 1) == 'T')
                            && (rText.GetChar(nPos + 2) == 'p'
                                || rText.GetChar(nPos + 2) == 'P'))))
                    // (note that rText.GetChar(nPos + 3) is guaranteed to be
                    // valid)
                {
                    xub_StrLen nUriEnd = i;
                    if (i != rEnd && rText.GetChar(i) == '/')
                    {
                        nUriEnd = ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    }
                    if (i != rEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    }
                    if (isBoundary1(rCharClass, rText, nUriEnd, rEnd)
                        || rText.GetChar(nUriEnd) == '\\')
                    {
                        INetURLObject aUri(UniString(rText, nPos,
                                                     nUriEnd - nPos),
                                           INET_PROT_HTTP, eMechanism,
                                           eCharset);
                        if (!aUri.HasError())
                        {
                            rBegin = nPos;
                            rEnd = nUriEnd;
                            return
                                aUri.GetMainURL(INetURLObject::DECODE_TO_IURI);
                        }
                    }
                }

                if ((eStyle & INetURLObject::FSYS_DOS) != 0 && rEnd - nPos >= 3
                    && rText.GetChar(nPos + 1) == ':'
                    && (rText.GetChar(nPos + 2) == '/'
                        || rText.GetChar(nPos + 2) == '\\')) // 7th, 8th
                {
                    i = nPos + 3;
                    xub_StrLen nUriEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd));
                    if (isBoundary1(rCharClass, rText, nUriEnd, rEnd))
                    {
                        INetURLObject aUri(UniString(rText, nPos,
                                                     nUriEnd - nPos),
                                           INET_PROT_FILE,
                                           INetURLObject::ENCODE_ALL,
                                           RTL_TEXTENCODING_UTF8,
                                           INetURLObject::FSYS_DOS);
                        if (!aUri.HasError())
                        {
                            rBegin = nPos;
                            rEnd = nUriEnd;
                            return
                                aUri.GetMainURL(INetURLObject::DECODE_TO_IURI);
                        }
                    }
                }
            }
            else if ((eStyle & INetURLObject::FSYS_DOS) != 0 && rEnd - nPos >= 2
                     && rText.GetChar(nPos) == '\\'
                     && rText.GetChar(nPos + 1) == '\\') // 6th
            {
                xub_StrLen i = nPos + 2;
                sal_uInt32 nLabels = scanDomain(rText, &i, rEnd);
                if (nLabels >= 1 && i != rEnd && rText.GetChar(i) == '\\')
                {
                    xub_StrLen nUriEnd = ++i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd,
                                         true));
                    if (isBoundary1(rCharClass, rText, nUriEnd, rEnd))
                    {
                        INetURLObject aUri(UniString(rText, nPos,
                                                     nUriEnd - nPos),
                                           INET_PROT_FILE,
                                           INetURLObject::ENCODE_ALL,
                                           RTL_TEXTENCODING_UTF8,
                                           INetURLObject::FSYS_DOS);
                        if (!aUri.HasError())
                        {
                            rBegin = nPos;
                            rEnd = nUriEnd;
                            return
                                aUri.GetMainURL(INetURLObject::DECODE_TO_IURI);
                        }
                    }
                }
            }
        }
        if (bBoundary2 && INetMIME::isAtomChar(c)) // 5th
        {
            bool bDot = false;
            for (xub_StrLen i = nPos + 1; i != rEnd; ++i)
            {
                sal_Unicode c2 = rText.GetChar(i);
                if (INetMIME::isAtomChar(c2))
                    bDot = false;
                else if (bDot)
                    break;
                else if (c2 == '.')
                    bDot = true;
                else
                {
                    if (c2 == '@')
                    {
                        ++i;
                        sal_uInt32 nLabels = scanDomain(rText, &i, rEnd);
                        if (nLabels >= 1
                            && isBoundary1(rCharClass, rText, i, rEnd))
                        {
                            INetURLObject aUri(UniString(rText, nPos, i - nPos),
                                               INET_PROT_MAILTO,
                                               INetURLObject::ENCODE_ALL);
                            if (!aUri.HasError())
                            {
                                rBegin = nPos;
                                rEnd = i;
                                return aUri.GetMainURL(
                                           INetURLObject::DECODE_TO_IURI);
                            }
                        }
                    }
                    break;
                }
            }
        }
        bBoundary1 = isBoundary1(rCharClass, rText, nPos, rEnd);
        bBoundary2 = isBoundary2(rCharClass, rText, nPos, rEnd);
    }
    rBegin = rEnd;
    return UniString();
}

//============================================================================
//
//  removePassword
//
//============================================================================

UniString
URIHelper::removePassword(UniString const & rURI,
                          INetURLObject::EncodeMechanism eEncodeMechanism,
                          INetURLObject::DecodeMechanism eDecodeMechanism,
                          rtl_TextEncoding eCharset)
{
    INetURLObject aObj(rURI, eEncodeMechanism, eCharset);
    return aObj.HasError() ?
               rURI :
               aObj.GetURLNoPass(eDecodeMechanism, eCharset);
}

//============================================================================
//
//  queryFSysStyle
//
//============================================================================

INetURLObject::FSysStyle URIHelper::queryFSysStyle(UniString const & rFileUrl,
                                                   bool bAddConvenienceStyles)
    throw (uno::RuntimeException)
{
    ::ucb::ContentBroker const * pBroker = ::ucb::ContentBroker::get();
    uno::Reference< star::ucb::XContentProviderManager > xManager;
    if (pBroker)
        xManager = pBroker->getContentProviderManagerInterface();
    uno::Reference< beans::XPropertySet > xProperties;
    if (xManager.is())
        xProperties
            = uno::Reference< beans::XPropertySet >(
                  xManager->queryContentProvider(rFileUrl), uno::UNO_QUERY);
    sal_Int32 nNotation = star::ucb::FileSystemNotation::UNKNOWN_NOTATION;
    if (xProperties.is())
        try
        {
            xProperties->getPropertyValue(rtl::OUString(
                                              RTL_CONSTASCII_USTRINGPARAM(
                                                  "FileSystemNotation")))
                >>= nNotation;
        }
        catch (beans::UnknownPropertyException const &) {}
        catch (lang::WrappedTargetException const &) {}

    // The following code depends on the fact that the
    // com::sun::star::ucb::FileSystemNotation constants range from UNKNOWN to
    // MAC, without any holes.  The table below has two entries per notation,
    // the first is used if bAddConvenienceStyles == false, while the second
    // is used if bAddConvenienceStyles == true:
    static INetURLObject::FSysStyle const aMap[][2]
        = { { INetURLObject::FSysStyle(0),
              INetURLObject::FSYS_DETECT },
                // UNKNOWN
            { INetURLObject::FSYS_UNX,
              INetURLObject::FSysStyle(INetURLObject::FSYS_VOS
                                           | INetURLObject::FSYS_UNX) },
                // UNIX
            { INetURLObject::FSYS_DOS,
              INetURLObject::FSysStyle(INetURLObject::FSYS_VOS
                                           | INetURLObject::FSYS_UNX
                                           | INetURLObject::FSYS_DOS) },
                // DOS
            { INetURLObject::FSYS_MAC,
              INetURLObject::FSysStyle(INetURLObject::FSYS_VOS
                                           | INetURLObject::FSYS_UNX
                                           | INetURLObject::FSYS_MAC) } };
    return aMap[nNotation < star::ucb::FileSystemNotation::UNKNOWN_NOTATION
                || nNotation > star::ucb::FileSystemNotation::MAC_NOTATION ?
                        0 :
                        nNotation
                            - star::ucb::FileSystemNotation::UNKNOWN_NOTATION]
                   [bAddConvenienceStyles];
}
