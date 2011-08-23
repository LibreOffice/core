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

#include <tools/urlobj.hxx>
#include <tools/table.hxx>
#include <urihelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/ucb/Command.hpp"
#include <com/sun/star/ucb/FileSystemNotation.hpp>
#include "com/sun/star/ucb/IllegalIdentifierException.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/ucb/XCommandProcessor.hpp"
#include "com/sun/star/ucb/XContent.hpp"
#include "com/sun/star/ucb/XContentIdentifierFactory.hpp"
#include "com/sun/star/ucb/XContentProvider.hpp"
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriReferenceFactory.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/processfactory.hxx"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
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
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include "rtl/instance.hxx"
#endif

namespace css = com::sun::star;
using namespace com::sun::star;

namespace binfilter
{

//============================================================================
//
//  SmartRel2Abs
//
//============================================================================

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
            && (aAbsURIRef.GetProtocol() == INET_PROT_FILE) )
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

UniString
SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
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

}

namespace { struct MaybeFileHdl : public rtl::Static< Link, MaybeFileHdl > {}; }

namespace binfilter
{

void SetMaybeFileHdl(Link const & rTheMaybeFileHdl)
{
    MaybeFileHdl::get() = rTheMaybeFileHdl;
}

//============================================================================
//
//  GetMaybeFileHdl
//
//============================================================================

Link GetMaybeFileHdl()
{
    return MaybeFileHdl::get();
}

namespace {

bool isAbsoluteHierarchicalUriReference(
    css::uno::Reference< css::uri::XUriReference > const & uriReference)
{
    return uriReference.is() && uriReference->isAbsolute()
        && uriReference->isHierarchical() && !uriReference->hasRelativePath();
}

// To improve performance, assume that if for any prefix URL of a given
// hierarchical URL either a UCB content cannot be created, or the UCB content
// does not support the getCasePreservingURL command, then this will hold for
// any other prefix URL of the given URL, too:
enum Result { Success, GeneralFailure, SpecificFailure };

Result normalizePrefix(
    css::uno::Reference< css::ucb::XContentProvider > const & broker,
    rtl::OUString const & uri, rtl::OUString * normalized)
{
    OSL_ASSERT(broker.is() && normalized != 0);
    css::uno::Reference< css::ucb::XContent > content;
    try {
        content = broker->queryContent(
            css::uno::Reference< css::ucb::XContentIdentifierFactory >(
                broker, css::uno::UNO_QUERY_THROW)->createContentIdentifier(
                    uri));
    } catch (css::ucb::IllegalIdentifierException &) {}
    if (!content.is()) {
        return GeneralFailure;
    }
    try {
        #if OSL_DEBUG_LEVEL > 0
        bool ok =
        #endif
            (css::uno::Reference< css::ucb::XCommandProcessor >(
                   content, css::uno::UNO_QUERY_THROW)->execute(
                       css::ucb::Command(
                           rtl::OUString(
                               RTL_CONSTASCII_USTRINGPARAM(
                                   "getCasePreservingURL")),
                           -1, css::uno::Any()),
                       0,
                       css::uno::Reference< css::ucb::XCommandEnvironment >())
               >>= *normalized);
        OSL_ASSERT(ok);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::ucb::UnsupportedCommandException &) {
        return GeneralFailure;
    } catch (css::uno::Exception &) {
        return SpecificFailure;
    }
    return Success;
}

rtl::OUString normalize(
    css::uno::Reference< css::ucb::XContentProvider > const & broker,
    css::uno::Reference< css::uri::XUriReferenceFactory > const & uriFactory,
    rtl::OUString const & uriReference)
{
    // normalizePrefix can potentially fail (a typically example being a file
    // URL that denotes a non-existing resource); in such a case, try to
    // normalize as long a prefix of the given URL as possible (i.e., normalize
    // all the existing directories within the path):
    rtl::OUString normalized;
    sal_Int32 n = uriReference.indexOf('#');
    normalized = n == -1 ? uriReference : uriReference.copy(0, n);
    switch (normalizePrefix(broker, normalized, &normalized)) {
    case Success:
        return n == -1 ? normalized : normalized + uriReference.copy(n);
    case GeneralFailure:
        return uriReference;
    case SpecificFailure:
    default:
        break;
    }
    css::uno::Reference< css::uri::XUriReference > ref(
        uriFactory->parse(uriReference));
    if (!isAbsoluteHierarchicalUriReference(ref)) {
        return uriReference;
    }
    sal_Int32 count = ref->getPathSegmentCount();
    if (count < 2) {
        return uriReference;
    }
    rtl::OUStringBuffer head(ref->getScheme());
    head.append(static_cast< sal_Unicode >(':'));
    if (ref->hasAuthority()) {
        head.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
        head.append(ref->getAuthority());
    }
    for (sal_Int32 i = count - 1; i > 0; --i) {
        rtl::OUStringBuffer buf(head);
        for (sal_Int32 j = 0; j < i; ++j) {
            buf.append(static_cast< sal_Unicode >('/'));
            buf.append(ref->getPathSegment(j));
        }
        normalized = buf.makeStringAndClear();
        if (normalizePrefix(broker, normalized, &normalized) != SpecificFailure)
        {
            buf.append(normalized);
            css::uno::Reference< css::uri::XUriReference > preRef(
                uriFactory->parse(normalized));
            if (!isAbsoluteHierarchicalUriReference(preRef)) {
                // This could only happen if something is inconsistent:
                break;
            }
            sal_Int32 preCount = preRef->getPathSegmentCount();
            // normalizePrefix may have added or removed a final slash:
            if (preCount != i) {
                if (preCount == i - 1) {
                    buf.append(static_cast< sal_Unicode >('/'));
                } else if (preCount - 1 == i && buf.getLength() > 0
                           && buf.charAt(buf.getLength() - 1) == '/')
                {
                    buf.setLength(buf.getLength() - 1);
                } else {
                    // This could only happen if something is inconsistent:
                    break;
                }
            }
            for (sal_Int32 j = i; j < count; ++j) {
                buf.append(static_cast< sal_Unicode >('/'));
                buf.append(ref->getPathSegment(j));
            }
            if (ref->hasQuery()) {
                buf.append(static_cast< sal_Unicode >('?'));
                buf.append(ref->getQuery());
            }
            if (ref->hasFragment()) {
                buf.append(static_cast< sal_Unicode >('#'));
                buf.append(ref->getFragment());
            }
            return buf.makeStringAndClear();
        }
    }
    return uriReference;
}

}

css::uno::Reference< css::uri::XUriReference >
normalizedMakeRelative(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    rtl::OUString const & baseUriReference, rtl::OUString const & uriReference)
{
    OSL_ASSERT(context.is());
    css::uno::Reference< css::lang::XMultiComponentFactory > componentFactory(
        context->getServiceManager());
    if (!componentFactory.is()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "component context has no service manager")),
            css::uno::Reference< css::uno::XInterface >());
    }
    css::uno::Sequence< css::uno::Any > args(2);
    args[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    args[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    css::uno::Reference< css::ucb::XContentProvider > broker;
    try {
        broker = css::uno::Reference< css::ucb::XContentProvider >(
            componentFactory->createInstanceWithArgumentsAndContext(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.ucb.UniversalContentBroker")),
                args, context),
            css::uno::UNO_QUERY_THROW);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception &) {
        css::uno::Any exception(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "creating com.sun.star.ucb.UniversalContentBroker failed")),
            css::uno::Reference< css::uno::XInterface >(),
            exception);
    }
    css::uno::Reference< css::uri::XUriReferenceFactory > uriFactory(
        css::uri::UriReferenceFactory::create(context));
    return uriFactory->makeRelative(
        uriFactory->parse(normalize(broker, uriFactory, baseUriReference)),
        uriFactory->parse(normalize(broker, uriFactory, uriReference)), true,
        true, false);
}

rtl::OUString simpleNormalizedMakeRelative(
    rtl::OUString const & baseUriReference, rtl::OUString const & uriReference)
{
    com::sun::star::uno::Reference< com::sun::star::uri::XUriReference > rel(
        normalizedMakeRelative(
            com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >(
                (com::sun::star::uno::Reference<
                 com::sun::star::beans::XPropertySet >(
                    comphelper::getProcessServiceFactory(),
                    com::sun::star::uno::UNO_QUERY_THROW)->
                 getPropertyValue(
                     rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM("DefaultContext")))),
                com::sun::star::uno::UNO_QUERY_THROW),
            baseUriReference, uriReference));
    return rel.is() ? rel->getUriReference() : uriReference;
}

//============================================================================
//
//  FindFirstURLInText
//
//============================================================================

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
    *pPos = sal::static_int_cast< xub_StrLen >(p - pBuffer);
    return nLabels;
}

UniString
FindFirstURLInText(UniString const & rText,
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
                    while (rText.GetChar(i++) != ':') ;
                    xub_StrLen nPrefixEnd = i;
                    xub_StrLen nUriEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd, true,
                                         true)) ;
                    if (i != nPrefixEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
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
                    while (rText.GetChar(i++) != ':') ;
                    xub_StrLen nPrefixEnd = i;
                    xub_StrLen nUriEnd = i;
                    while (i != rEnd
                           && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
                    if (i != nPrefixEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
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
                               && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
                    }
                    if (i != rEnd && rText.GetChar(i) == '#')
                    {
                        ++i;
                        while (i != rEnd
                               && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
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
                           && checkWChar(rCharClass, rText, &i, &nUriEnd)) ;
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
                                         true)) ;
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
removePassword(UniString const & rURI,
                          INetURLObject::EncodeMechanism eEncodeMechanism,
                          INetURLObject::DecodeMechanism eDecodeMechanism,
                          rtl_TextEncoding eCharset)
{
    INetURLObject aObj(rURI, eEncodeMechanism, eCharset);
    return aObj.HasError() ?
               rURI :
               String(aObj.GetURLNoPass(eDecodeMechanism, eCharset));
}

}
