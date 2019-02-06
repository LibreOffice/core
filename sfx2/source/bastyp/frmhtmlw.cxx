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

#include <svtools/htmlkywd.hxx>

#include <rtl/tencinfo.h>
#include <sal/log.hxx>

#include <unotools/configmgr.hxx>
#include <svl/urihelper.hxx>
#include <tools/datetime.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <unotools/resmgr.hxx>

#include <sfx2/frmhtmlw.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <strings.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <sax/tools/converter.hxx>

using namespace ::com::sun::star;

static sal_Char const sHTML_SC_yes[] =  "YES";
static sal_Char const sHTML_SC_no[] =       "NO";

void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                                  const sal_Char *pIndent,
                                  const OUString& rName,
                                  const OUString& rContent,
                                  bool bHTTPEquiv,
                                  rtl_TextEncoding eDestEnc,
                                  OUString *pNonConvertableChars  )
{
    rStrm.WriteCharPtr( SAL_NEWLINE_STRING );
    if( pIndent )
        rStrm.WriteCharPtr( pIndent );

    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_meta).append(' ')
        .append(bHTTPEquiv ? OOO_STRING_SVTOOLS_HTML_O_httpequiv : OOO_STRING_SVTOOLS_HTML_O_name).append("=\"");
    rStrm.WriteCharPtr( sOut.makeStringAndClear().getStr() );

    HTMLOutFuncs::Out_String( rStrm, rName, eDestEnc, pNonConvertableChars );

    sOut.append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_content).append("=\"");
    rStrm.WriteCharPtr( sOut.makeStringAndClear().getStr() );

    HTMLOutFuncs::Out_String( rStrm, rContent, eDestEnc, pNonConvertableChars ).WriteCharPtr( "\"/>" );
}

void SfxFrameHTMLWriter::Out_DocInfo( SvStream& rStrm, const OUString& rBaseURL,
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        const sal_Char *pIndent,
        rtl_TextEncoding eDestEnc,
        OUString *pNonConvertableChars    )
{
    const sal_Char *pCharSet =
                rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );

    if( pCharSet )
    {
        OUString aContentType = "text/html; charset=" + OUString(pCharSet, strlen(pCharSet), RTL_TEXTENCODING_UTF8);
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_content_type, aContentType, true,
                 eDestEnc, pNonConvertableChars );
    }

    // Title (regardless if empty)
    rStrm.WriteCharPtr( SAL_NEWLINE_STRING );
    if( pIndent )
        rStrm.WriteCharPtr( pIndent );
    HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title );
    if( i_xDocProps.is() )
    {
        const OUString& rTitle = i_xDocProps->getTitle();
        if( !rTitle.isEmpty() )
            HTMLOutFuncs::Out_String( rStrm, rTitle, eDestEnc, pNonConvertableChars );
    }
    HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title, false );

    // Target-Frame
    if( i_xDocProps.is() )
    {
        const OUString& rTarget = i_xDocProps->getDefaultTarget();
        if( !rTarget.isEmpty() )
        {
            rStrm.WriteCharPtr( SAL_NEWLINE_STRING );
            if( pIndent )
                rStrm.WriteCharPtr( pIndent );

            OStringBuffer sOut;
            sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_base).append(' ')
                .append(OOO_STRING_SVTOOLS_HTML_O_target).append("=\"");
            rStrm.WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rStrm, rTarget, eDestEnc, pNonConvertableChars )
               .WriteCharPtr( "\">" );
        }
    }

    // Who we are
    OUString sGenerator(Translate::ExpandVariables(STR_HTML_GENERATOR));
    OUString os( "$_OS" );
    ::rtl::Bootstrap::expandMacros(os);
    sGenerator = sGenerator.replaceFirst( "%1", os );
    OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_generator, sGenerator, false, eDestEnc, pNonConvertableChars );

    if( !i_xDocProps.is() )
        return;

    // Reload
    if( (i_xDocProps->getAutoloadSecs() != 0) ||
        !i_xDocProps->getAutoloadURL().isEmpty() )
    {
        OUString sContent = OUString::number(
                            i_xDocProps->getAutoloadSecs() );

        const OUString &rReloadURL = i_xDocProps->getAutoloadURL();
        if( !rReloadURL.isEmpty() )
        {
            sContent += ";URL=";
            sContent += URIHelper::simpleNormalizedMakeRelative(
                          rBaseURL, rReloadURL);
        }

        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_refresh, sContent, true,
                 eDestEnc, pNonConvertableChars );
    }

    // Author
    const OUString& rAuthor = i_xDocProps->getAuthor();
    if( !rAuthor.isEmpty() )
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_author, rAuthor, false,
                 eDestEnc, pNonConvertableChars );

    // created
    ::util::DateTime uDT = i_xDocProps->getCreationDate();
    OUStringBuffer aBuffer;
    ::sax::Converter::convertTimeOrDateTime(aBuffer, uDT);

    OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_created, aBuffer.makeStringAndClear(), false,
             eDestEnc, pNonConvertableChars );

    // changedby
    const OUString& rChangedBy = i_xDocProps->getModifiedBy();
    if( !rChangedBy.isEmpty() )
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changedby, rChangedBy, false,
                 eDestEnc, pNonConvertableChars );

    // changed
    uDT = i_xDocProps->getModificationDate();
    ::sax::Converter::convertTimeOrDateTime(aBuffer, uDT);

    OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changed, aBuffer.makeStringAndClear(), false,
             eDestEnc, pNonConvertableChars );

    // Subject
    const OUString& rTheme = i_xDocProps->getSubject();
    if( !rTheme.isEmpty() )
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_classification, rTheme, false,
                 eDestEnc, pNonConvertableChars );

    // Description
    const OUString& rComment = i_xDocProps->getDescription();
    if( !rComment.isEmpty() )
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_description, rComment, false,
                 eDestEnc, pNonConvertableChars);

    // Keywords
    OUString Keywords = ::comphelper::string::convertCommaSeparated(
        i_xDocProps->getKeywords());
    if( !Keywords.isEmpty() )
        OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_keywords, Keywords, false,
                 eDestEnc, pNonConvertableChars);

    uno::Reference < script::XTypeConverter > xConverter( script::Converter::create(
        ::comphelper::getProcessComponentContext() ) );
    uno::Reference<beans::XPropertySet> xUserDefinedProps(
        i_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropInfo =
        xUserDefinedProps->getPropertySetInfo();
    DBG_ASSERT(xPropInfo.is(), "UserDefinedProperties Info is null");
    uno::Sequence<beans::Property> props = xPropInfo->getProperties();
    for (sal_Int32 i = 0; i < props.getLength(); ++i)
    {
        try
        {
            OUString name = props[i].Name;
            uno::Any aStr = xConverter->convertToSimpleType(
                    xUserDefinedProps->getPropertyValue(name),
                    uno::TypeClass_STRING);
            OUString str;
            aStr >>= str;
            OUString valstr(comphelper::string::stripEnd(str, ' '));
            OutMeta( rStrm, pIndent, name, valstr, false,
                     eDestEnc, pNonConvertableChars );
        }
        catch (const uno::Exception&)
        {
            // may happen with concurrent modification...
            SAL_INFO("sfx", "SfxFrameHTMLWriter::Out_DocInfo: exception");
        }
    }
}

void SfxFrameHTMLWriter::Out_FrameDescriptor(
    SvStream& rOut, const OUString& rBaseURL, const uno::Reference < beans::XPropertySet >& xSet,
    rtl_TextEncoding eDestEnc, OUString *pNonConvertableChars )
{
    try
    {
        OStringBuffer sOut;
        OUString aStr;
        uno::Any aAny = xSet->getPropertyValue("FrameURL");
        if ( (aAny >>= aStr) && !aStr.isEmpty() )
        {
            OUString aURL = INetURLObject( aStr ).GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
            if( !aURL.isEmpty() )
            {
                aURL = URIHelper::simpleNormalizedMakeRelative(
                    rBaseURL, aURL );
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_src)
                    .append("=\"");
                rOut.WriteCharPtr( sOut.makeStringAndClear().getStr() );
                HTMLOutFuncs::Out_String( rOut, aURL, eDestEnc, pNonConvertableChars );
                sOut.append('\"');
            }
        }

        aAny = xSet->getPropertyValue("FrameName");
        if ( (aAny >>= aStr) && !aStr.isEmpty() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_name)
                .append("=\"");
            rOut.WriteCharPtr( sOut.makeStringAndClear().getStr() );
            HTMLOutFuncs::Out_String( rOut, aStr, eDestEnc, pNonConvertableChars );
            sOut.append('\"');
        }

        sal_Int32 nVal = SIZE_NOT_SET;
        aAny = xSet->getPropertyValue("FrameMarginWidth");
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_marginwidth)
                .append('=').append(nVal);
        }
        aAny = xSet->getPropertyValue("FrameMarginHeight");
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_marginheight)
                .append('=').append(nVal);
        }

        bool bVal = true;
        aAny = xSet->getPropertyValue("FrameIsAutoScroll");
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue("FrameIsScrollingMode");
            if ( aAny >>= bVal )
            {
                const sal_Char *pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_scrolling)
                    .append(pStr);
            }
        }

        // frame border (MS+Netscape-Extension)
        aAny = xSet->getPropertyValue("FrameIsAutoBorder");
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue("FrameIsBorder");
            if ( aAny >>= bVal )
            {
                const char* pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_frameborder)
                    .append('=').append(pStr);
            }
        }
        rOut.WriteCharPtr( sOut.getStr() );
    }
    catch (const uno::Exception&)
    {
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
