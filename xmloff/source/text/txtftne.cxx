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


/** @#file
 *
 * This file implements XMLTextParagraphExport methods to export
 * - footnotes
 * - endnotes
 * - footnote configuration elements
 * - endnote configuration elements
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <o3tl/any.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/families.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/txtparae.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


void XMLTextParagraphExport::exportTextFootnote(
    const Reference<XPropertySet> & rPropSet,
    const OUString& rText,
    bool bAutoStyles, bool bIsProgress )
{
    // get footnote and associated text
    Any aAny = rPropSet->getPropertyValue(gsFootnote);
    Reference<XFootnote> xFootnote;
    aAny >>= xFootnote;
    Reference<XText> xText(xFootnote, UNO_QUERY);

    // are we an endnote?
    Reference<XServiceInfo> xServiceInfo( xFootnote, UNO_QUERY );
    bool bIsEndnote = xServiceInfo->supportsService(gsTextEndnoteService);

    if (bAutoStyles)
    {
        // handle formatting of citation mark
        Add( XmlStyleFamily::TEXT_TEXT, rPropSet );

        // handle formatting within footnote
        exportTextFootnoteHelper(xFootnote, xText, rText,
                                 bAutoStyles, bIsEndnote, bIsProgress );
    }
    else
    {
        // create span (for citation mark) if necessary; footnote content
        // will be handled via exportTextFootnoteHelper, exportText
        bool bIsUICharStyle = false;
        bool bHasAutoStyle = false;

        OUString sStyle = FindTextStyle( rPropSet, bIsUICharStyle, bHasAutoStyle );

        {
            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle &&
                             m_aCharStyleNamesPropInfoCache.hasProperty(
                                                    rPropSet ), bHasAutoStyle,
                rPropSet, gsCharStyleNames );
            if( !sStyle.isEmpty() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          GetExport().EncodeStyleName( sStyle ) );
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                          XML_SPAN, false, false );
                exportTextFootnoteHelper(xFootnote, xText, rText,
                                         bAutoStyles, bIsEndnote, bIsProgress );
            }
            else
            {
                exportTextFootnoteHelper(xFootnote, xText, rText,
                                         bAutoStyles, bIsEndnote, bIsProgress );
            }
        }
    }
}


void XMLTextParagraphExport::exportTextFootnoteHelper(
    const Reference<XFootnote> & rFootnote,
    const Reference<XText> & rText,
    const OUString& rTextString,
    bool bAutoStyles,
    bool bIsEndnote,
    bool bIsProgress )
{
    if (bAutoStyles)
    {
        exportText(rText, bAutoStyles, bIsProgress, true );
    }
    else
    {
        // export reference Id (for reference fields)
        Reference<XPropertySet> xPropSet(rFootnote, UNO_QUERY);
        Any aAny = xPropSet->getPropertyValue(gsReferenceId);
        sal_Int32 nNumber = 0;
        aAny >>= nNumber;
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 "ftn" + OUString::number(nNumber));
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
                                 GetXMLToken( bIsEndnote ? XML_ENDNOTE
                                                          : XML_FOOTNOTE ) );

        SvXMLElementExport aNote(GetExport(), XML_NAMESPACE_TEXT,
                                 XML_NOTE, false, false);
        {
            // handle label vs. automatic numbering
            OUString sLabel = rFootnote->getLabel();
            if (!sLabel.isEmpty())
            {
                GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_LABEL,
                                         sLabel);
            }
            // else: automatic numbering -> no attribute

            SvXMLElementExport aCite(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_NOTE_CITATION, false, false);
            GetExport().Characters(rTextString);
        }

        {
            SvXMLElementExport aBody(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_NOTE_BODY, false, false);
            exportText(rText, bAutoStyles, bIsProgress, true );
        }
    }
}


void XMLTextParagraphExport::exportTextFootnoteConfiguration()
{
    // footnote settings
    Reference<XFootnotesSupplier> aFootnotesSupplier(GetExport().GetModel(),
                                                     UNO_QUERY);
    Reference<XPropertySet> aFootnoteConfiguration(
        aFootnotesSupplier->getFootnoteSettings());
    exportTextFootnoteConfigurationHelper(aFootnoteConfiguration, false);

    // endnote settings
    Reference<XEndnotesSupplier> aEndnotesSupplier(GetExport().GetModel(),
                                                   UNO_QUERY);
    Reference<XPropertySet> aEndnoteConfiguration(
        aEndnotesSupplier->getEndnoteSettings());
    exportTextFootnoteConfigurationHelper(aEndnoteConfiguration, true);
}


static void lcl_exportString(
    SvXMLExport& rExport,
    const Reference<XPropertySet> & rPropSet,
    const OUString& sProperty,
    sal_uInt16 nPrefix,
    enum XMLTokenEnum eElement,
    bool bEncodeName)
{
    SAL_WARN_IF( eElement == XML_TOKEN_INVALID, "xmloff", "need element token");

    Any aAny = rPropSet->getPropertyValue(sProperty);
    OUString sTmp;
    aAny >>= sTmp;
    if (!sTmp.isEmpty())
    {
        if( bEncodeName )
            sTmp = rExport.EncodeStyleName( sTmp );
        rExport.AddAttribute(nPrefix, eElement, sTmp);
    }
}

void XMLTextParagraphExport::exportTextFootnoteConfigurationHelper(
    const Reference<XPropertySet> & rFootnoteConfig,
    bool bIsEndnote)
{
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
                             GetXMLToken( bIsEndnote ? XML_ENDNOTE
                                                     : XML_FOOTNOTE ) );
    // default/paragraph style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      gsParaStyleName,
                      XML_NAMESPACE_TEXT, XML_DEFAULT_STYLE_NAME,
                      true);

    // citation style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      gsCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_STYLE_NAME,
                      true);

    // citation body style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      gsAnchorCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_BODY_STYLE_NAME,
                      true);

    // page style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      gsPageStyleName,
                      XML_NAMESPACE_TEXT, XML_MASTER_PAGE_NAME,
                      true );

    // prefix
    lcl_exportString( GetExport(), rFootnoteConfig, gsPrefix,
                      XML_NAMESPACE_STYLE, XML_NUM_PREFIX, false);

    // suffix
    lcl_exportString( GetExport(), rFootnoteConfig, gsSuffix,
                      XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, false);


    Any aAny;

    // numbering style
    OUStringBuffer sBuffer;
    aAny = rFootnoteConfig->getPropertyValue(gsNumberingType);
    sal_Int16 nNumbering = 0;
    aAny >>= nNumbering;
    GetExport().GetMM100UnitConverter().convertNumFormat( sBuffer, nNumbering);
    GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                             sBuffer.makeStringAndClear() );
    SvXMLUnitConverter::convertNumLetterSync( sBuffer, nNumbering);
    if (!sBuffer.isEmpty() )
    {
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                                 sBuffer.makeStringAndClear());
    }

    // StartAt / start-value
    aAny = rFootnoteConfig->getPropertyValue(gsStartAt);
    sal_Int16 nOffset = 0;
    aAny >>= nOffset;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_START_VALUE,
                             OUString::number(nOffset));

    // some properties are for footnotes only
    if (!bIsEndnote)
    {
        // footnotes position
        aAny = rFootnoteConfig->getPropertyValue(
            gsPositionEndOfDoc);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_FOOTNOTES_POSITION,
                                 ( (*o3tl::doAccess<bool>(aAny)) ?
                                        XML_DOCUMENT : XML_PAGE ) );

        aAny = rFootnoteConfig->getPropertyValue(gsFootnoteCounting);
        sal_Int16 nTmp = 0;
        aAny >>= nTmp;
        enum XMLTokenEnum eElement;
        switch (nTmp)
        {
            case FootnoteNumbering::PER_PAGE:
                eElement = XML_PAGE;
                break;
            case FootnoteNumbering::PER_CHAPTER:
                eElement = XML_CHAPTER;
                break;
            case FootnoteNumbering::PER_DOCUMENT:
            default:
                eElement = XML_DOCUMENT;
                break;
        }
        GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                 XML_START_NUMBERING_AT, eElement);
    }

    // element
    SvXMLElementExport aFootnoteConfigElement(
        GetExport(), XML_NAMESPACE_TEXT,
        XML_NOTES_CONFIGURATION,
        true, true);

    // two element for footnote content
    if (bIsEndnote)
        return;

    OUString sTmp;

    // end notice / quo vadis
    aAny = rFootnoteConfig->getPropertyValue(gsEndNotice);
    aAny >>= sTmp;

    if (!sTmp.isEmpty())
    {
        SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT,
                                 XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD,
                                 true, false);
        GetExport().Characters(sTmp);
    }

    // begin notice / ergo sum
    aAny = rFootnoteConfig->getPropertyValue(gsBeginNotice);
    aAny >>= sTmp;

    if (!sTmp.isEmpty())
    {
        SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT,
                                 XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD,
                                 true, false);
        GetExport().Characters(sTmp);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
