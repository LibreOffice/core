/*************************************************************************
 *
 *  $RCSfile: urihelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-09 15:38:15 $
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
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
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

    bool bWasAbsolute;
    INetURLObject
        aAbsURIRef(rTheBaseURIRef.smartRel2Abs(rTheRelURIRef, bWasAbsolute,
                                               bIgnoreFragment,
                                               eEncodeMechanism, eCharset,
                                               bRelativeNonURIs, eStyle));
    if (bCheckFileExists && !bWasAbsolute
        && aAbsURIRef.GetProtocol() == INET_PROT_FILE)
    {
        INetURLObject aNonFileURIRef;
        aNonFileURIRef.SetSmartURL(rTheRelURIRef, eEncodeMechanism, eCharset,
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
            {
                sal_Bool bExists = false;
                try
                {
                    ::ucb::Content(
                                aAbsURIRef.
                                    GetMainURL(INetURLObject::NO_DECODE),
                                uno::Reference<
                                    com::sun::star::ucb::XCommandEnvironment >
                                        ()).
                            getPropertyValue(rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "Exists")))
                        >>= bExists;
                }
                catch (::ucb::ContentCreationException const &)
                {
                    DBG_ERROR("URIHelper::SmartRel2Abs(): UCB Exception");
                }
                catch (uno::Exception const &)
                {
                    DBG_ERROR("URIHelper::SmartRel2Abs(): UCB Exception");
                }
                if (!bExists)
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

inline xub_StrLen prevChar(UniString const & rStr, xub_StrLen nPos)
{
    return INetMIME::isLowSurrogate(rStr.GetChar(nPos - 1))
           && nPos >= 2
           && INetMIME::isHighSurrogate(rStr.GetChar(nPos - 2)) ?
               nPos - 2 : nPos - 1;
}

inline xub_StrLen nextChar(UniString const & rStr, xub_StrLen nPos)
{
    return INetMIME::isHighSurrogate(rStr.GetChar(nPos))
           && rStr.Len() - nPos >= 2
           && INetMIME::isLowSurrogate(rStr.GetChar(nPos + 1)) ?
               nPos + 2 : nPos + 1;
}

inline bool isWLetter(CharClass const & rCharClass,
                      UniString const & rStr, xub_StrLen nPos)
{
    if (rCharClass.isLetterNumeric(rStr, nPos))
        return true;
    sal_Unicode c = rStr.GetChar(nPos);
    return c == '$' || c == '%' || c == '&' || c == '-' || c == '/'
           || c == '@' || c == '\\';
}

inline bool checkWChar(CharClass const & rCharClass, UniString const & rStr,
                       xub_StrLen * pPos, xub_StrLen * pEnd,
                       bool bBackslash = false, bool bPipe = false)
{
    sal_Unicode c = rStr.GetChar(*pPos);
    if (INetMIME::isUSASCII(c))
    {
        static sal_uInt8 const aMap[128]
            = { 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 0, 1, 1, 1, 1,   //  !"#$%&'
                1, 1, 1, 1, 1, 1, 1, 1,   // ()*+,-./
                4, 4, 4, 4, 4, 4, 4, 4,   // 01234567
                4, 4, 1, 1, 0, 1, 0, 1,   // 89:;<=>?
                1, 4, 4, 4, 4, 4, 4, 4,   // @ABCDEFG
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

            case 4: // letter, ...
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

inline sal_uInt32
scanDomain(UniString const & rStr, xub_StrLen * pPos, xub_StrLen nEnd)
{
    sal_Unicode const * pBuffer = rStr.GetBuffer();
    sal_Unicode const * p = pBuffer + *pPos;
    sal_uInt32 nLabels = INetURLObject::scanDomain(p, pBuffer + nEnd, false);
    *pPos = p - pBuffer;
    return nLabels;
}

inline bool
scanIPv6reference(UniString const & rStr, xub_StrLen * pPos, xub_StrLen nEnd)
{
    sal_Unicode const * pBuffer = rStr.GetBuffer();
    sal_Unicode const * p = pBuffer + *pPos;
    bool bSuccess
        = INetURLObject::scanIPv6reference(p, pBuffer + nEnd, false);
    *pPos = p - pBuffer;
    return bSuccess;
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

    // Search for the first (longest possible) substring of [pBegin..pEnd[
    // that matches any of the following productions (for which the
    // appropriate style bit is set in eStyle, if applicable).  "\W" stands
    // for a word break, i.e., the begin or end of the block of text, or a
    // character that is neither a letter nor a digit (according to
    // rCharClass).  The productions use the auxiliary rules
    //
    //    domain = label *("." label)
    //    label = alphanum [*(alphanum / "-") alphanum]
    //    alphanum = ALPHA / DIGIT
    //    IPv6reference = "[" IPv6address "]"
    //    IPv6address = hexpart [":" IPv4address]
    //    IPv4address = 1*3DIGIT 3("." 1*3DIGIT)
    //    hexpart = (hexseq ["::" [hexseq]]) / ("::" [hexseq])
    //    hexseq = hex4 *(":" hex4)
    //    hex4 = 1*4HEXDIG
    //    wchar = <any uric character (ignoring the escaped rule), or "%", or
    //             a letter or digit (according to rCharClass)>
    //
    // 1st Production (file):
    //    \W "file:" 1*(wchar / "\" / "|") ["#" 1*wchar] \W
    //
    // 2nd Production (known scheme):
    //    \W <one of the known schemes, ignoring case> ":" 1*wchar
    //        ["#" 1*wchar] \W
    //
    // 3rd Production (mailto):
    //    \W domain "@" domain \W
    //
    // 4th Production (ftp):
    //    \W "ftp" 2*("." label) ["/" *wchar] ["#" 1*wchar] \W
    //
    // 5th Production (http):
    //    \W label 2*("." label) ["/" *wchar] ["#" 1*wchar] \W
    //
    // 6th Production (file):
    //    \W "//" (domain / IPv6reference) ["/" *wchar] ["#" 1*wchar] \W
    //
    // 7th Production (Unix file; FSYS_UNX only): @@@ DISABLED @@@
    //    \W "/" 1*wchar \W
    //
    // 8th Production (UNC file; FSYS_DOS only):
    //    \W "\\" domain ["\" *(wchar / "\")] \W
    //
    // 9th Production (Unix-like DOS file; FSYS_DOS only):
    //    \W ALPHA ":/" *wchar \W
    //
    // 10th Production (DOS file; FSYS_DOS only):
    //    \W ALPHA ":\" *(wchar / "\") \W

    for (xub_StrLen nPos = rBegin; nPos != rEnd;)
    {
        sal_Unicode c = rText.GetChar(nPos);
        if ((INetMIME::isAlpha(c) || c == '/' || c == '\\')
            && (nPos == rBegin
                || !isWLetter(rCharClass, rText, prevChar(rText, nPos))))
        {
            xub_StrLen nURIEnd = STRING_NOTFOUND;

            if (INetMIME::isAlpha(c))
            {
                xub_StrLen i = nPos;
                INetProtocol eScheme
                    = INetURLObject::CompareProtocolScheme(UniString(rText, i,
                                                                     rEnd));
                if (eScheme != INET_PROT_NOT_VALID) // 1st, 2nd
                {
                    while (rText.GetChar(i++) != ':');
                    xub_StrLen nPrefixEnd = i;
                    if (eScheme == INET_PROT_FILE)
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nURIEnd,
                                             true, true));
                    else
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i,
                                             &nURIEnd));
                    if (i != nPrefixEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i,
                                             &nURIEnd));
                    }
                }
                else if (eStyle & INetURLObject::FSYS_DOS
                         && rEnd - i >= 3
                         && rText.GetChar(i + 1) == ':'
                         && rText.GetChar(i + 2) == '/') // 9th
                {
                    i += 3;
                    nURIEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nURIEnd));
                }
                else if (eStyle & INetURLObject::FSYS_DOS
                         && rEnd - i >= 3
                         && rText.GetChar(i + 1) == ':'
                         && rText.GetChar(i + 2) == '\\') // 10th
                {
                    i += 3;
                    nURIEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nURIEnd,
                                         true));
                }
                else
                {
                    sal_uInt32 nLabels = scanDomain(rText, &i, rEnd);
                    if (nLabels > 0 && i != rEnd && rText.GetChar(i) == '@')
                        // 3rd
                    {
                        ++i;
                        if (scanDomain(rText, &i, rEnd) > 0)
                            nURIEnd = i;
                    }
                    else if (nLabels >= 3) // 4th, 5th
                    {
                        nURIEnd = i;
                        if (i != rEnd && rText.GetChar(i) == '/')
                        {
                            nURIEnd = ++i;
                            while (i != rEnd
                                   && checkWChar(rCharClass, rText, &i,
                                                 &nURIEnd));
                        }
                        if (i != rEnd && rText.GetChar(i) == '#')
                        {
                            ++i;
                            while (i != rEnd
                                   && checkWChar(rCharClass, rText, &i,
                                                 &nURIEnd));
                        }
                    }
                }
            }
            else if (c == '/')
            {
                xub_StrLen i = nPos;
                if (rEnd - i >= 2)
                    if (rText.GetChar(i + 1) == '/') // 6th
                    {
                        i += 2;
                        if (scanDomain(rText, &i, rEnd) > 0
                            || scanIPv6reference(rText, &i, rEnd))
                        {
                            nURIEnd = i;
                            if (i != rEnd && rText.GetChar(i) == '/')
                            {
                                nURIEnd = ++i;
                                while (i != rEnd
                                       && checkWChar(rCharClass, rText, &i,
                                                     &nURIEnd));
                            }
                            if (i != rEnd && rText.GetChar(i) == '#')
                            {
                                ++i;
                                while (i != rEnd
                                       && checkWChar(rCharClass, rText, &i,
                                                     &nURIEnd));
                            }
                        }
                    }
//                  else if (eStyle & INetURLObject::FSYS_UNX) // 7th
//                  {
//                      ++i;
//                      while (i != rEnd
//                             && checkWChar(rCharClass, rText, &i,
//                                           &nURIEnd));
//                  }
            }
            else if (eStyle & INetURLObject::FSYS_DOS && c == '\\') // 8th
            {
                xub_StrLen i = nPos;
                if (rEnd - i >= 2 && rText.GetChar(i + 1) == '\\')
                {
                    i += 2;
                    if (scanDomain(rText, &i, rEnd) > 0)
                    {
                        nURIEnd = i;
                        if (i != rEnd && rText.GetChar(i) == '\\')
                        {
                            nURIEnd = ++i;
                            while (i != rEnd
                                   && checkWChar(rCharClass, rText, &i,
                                                 &nURIEnd, true));
                        }
                    }
                }
            }

            if (nURIEnd != STRING_NOTFOUND
                && (nURIEnd == rEnd
                    || !isWLetter(rCharClass, rText, nURIEnd)))
            {
                INetURLObject aURI(UniString(rText, nPos, nURIEnd - nPos),
                                   INET_PROT_HTTP, eMechanism, eCharset,
                                   eStyle);
                if (!aURI.HasError())
                {
                    rBegin = nPos;
                    rEnd = nURIEnd;
                    return aURI.GetMainURL();
                }
            }

            ++nPos;
            while (nPos != rEnd && isWLetter(rCharClass, rText, nPos))
                nPos = nextChar(rText, nPos);
        }
        else
            nPos = nextChar(rText, nPos);
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
