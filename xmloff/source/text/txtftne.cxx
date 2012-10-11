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



/** @#file
 *
 * This file implements XMLTextParagraphExport methods to export
 * - footnotes
 * - endnotes
 * - footnote configuration elements
 * - endnote configuration elements
 */
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnume.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/txtparae.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;


void XMLTextParagraphExport::exportTextFootnote(
    const Reference<XPropertySet> & rPropSet,
    const OUString& rText,
    sal_Bool bAutoStyles, sal_Bool bIsProgress )
{
    // get footnote and associated text
    Any aAny;
    aAny = rPropSet->getPropertyValue(sFootnote);
    Reference<XFootnote> xFootnote;
    aAny >>= xFootnote;
    Reference<XText> xText(xFootnote, UNO_QUERY);

    // are we an endnote?
    Reference<XServiceInfo> xServiceInfo( xFootnote, UNO_QUERY );
    sal_Bool bIsEndnote = xServiceInfo->supportsService(sTextEndnoteService);

    if (bAutoStyles)
    {
        // handle formatting of citation mark
        Add( XML_STYLE_FAMILY_TEXT_TEXT, rPropSet );

        // handle formatting within footnote
        exportTextFootnoteHelper(xFootnote, xText, rText,
                                 bAutoStyles, bIsEndnote, bIsProgress );
    }
    else
    {
        // create span (for citation mark) if necessary; footnote content
        // wil be handled via exportTextFootnoteHelper, exportText
        sal_Bool bHasHyperlink;
        sal_Bool bIsUICharStyle = sal_False;
        sal_Bool bHasAutoStyle = sal_False;

        OUString sStyle = FindTextStyleAndHyperlink( rPropSet, bHasHyperlink,
                                                     bIsUICharStyle, bHasAutoStyle );

        // export hyperlink (if we have one)
        Reference < XPropertySetInfo > xPropSetInfo;
        if( bHasHyperlink )
        {
            Reference<XPropertyState> xPropState( rPropSet, UNO_QUERY );
            xPropSetInfo = rPropSet->getPropertySetInfo();
            bHasHyperlink =
                addHyperlinkAttributes( rPropSet, xPropState, xPropSetInfo );
        }
        SvXMLElementExport aHyperlink( GetExport(), bHasHyperlink,
                                       XML_NAMESPACE_TEXT, XML_A,
                                       sal_False, sal_False );

        if( bHasHyperlink )
        {
            // export events (if supported)
            OUString sHyperLinkEvents(RTL_CONSTASCII_USTRINGPARAM(
                "HyperLinkEvents"));
            if (xPropSetInfo->hasPropertyByName(sHyperLinkEvents))
            {
                Any a = rPropSet->getPropertyValue(sHyperLinkEvents);
                Reference<XNameReplace> xName;
                a >>= xName;
                GetExport().GetEventExport().Export(xName, sal_False);
            }
        }

        {
            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle &&
                             aCharStyleNamesPropInfoCache.hasProperty(
                                                    rPropSet ), bHasAutoStyle,
                rPropSet, sCharStyleNames );
            if( !sStyle.isEmpty() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          GetExport().EncodeStyleName( sStyle ) );
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                          XML_SPAN, sal_False, sal_False );
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
    sal_Bool bAutoStyles,
    sal_Bool bIsEndnote,
    sal_Bool bIsProgress )
{
    if (bAutoStyles)
    {
        exportText(rText, bAutoStyles, bIsProgress, sal_True );
    }
    else
    {
        // export reference Id (for reference fields)
        Reference<XPropertySet> xPropSet(rFootnote, UNO_QUERY);
        Any aAny = xPropSet->getPropertyValue(sReferenceId);
        sal_Int32 nNumber = 0;
        aAny >>= nNumber;
        OUStringBuffer aBuf;
        aBuf.appendAscii("ftn");
        aBuf.append(nNumber);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 aBuf.makeStringAndClear());
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
                                 GetXMLToken( bIsEndnote ? XML_ENDNOTE
                                                          : XML_FOOTNOTE ) );

        SvXMLElementExport aNote(GetExport(), XML_NAMESPACE_TEXT,
                                 XML_NOTE, sal_False, sal_False);
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
                                     XML_NOTE_CITATION, sal_False, sal_False);
            GetExport().Characters(rTextString);
        }

        {
            SvXMLElementExport aBody(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_NOTE_BODY, sal_False, sal_False);
            exportText(rText, bAutoStyles, bIsProgress, sal_True );
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
    exportTextFootnoteConfigurationHelper(aFootnoteConfiguration, sal_False);

    // endnote settings
    Reference<XEndnotesSupplier> aEndnotesSupplier(GetExport().GetModel(),
                                                   UNO_QUERY);
    Reference<XPropertySet> aEndnoteConfiguration(
        aEndnotesSupplier->getEndnoteSettings());
    exportTextFootnoteConfigurationHelper(aEndnoteConfiguration, sal_True);
}


static void lcl_exportString(
    SvXMLExport& rExport,
    const Reference<XPropertySet> & rPropSet,
    const OUString& sProperty,
    sal_uInt16 nPrefix,
    enum XMLTokenEnum eElement,
    sal_Bool bEncodeName,
    sal_Bool bOmitIfEmpty)
{
    DBG_ASSERT( eElement != XML_TOKEN_INVALID, "need element token");

    Any aAny = rPropSet->getPropertyValue(sProperty);
    OUString sTmp;
    aAny >>= sTmp;
    if (!bOmitIfEmpty || !sTmp.isEmpty())
    {
        if( bEncodeName )
            sTmp = rExport.EncodeStyleName( sTmp );
        rExport.AddAttribute(nPrefix, eElement, sTmp);
    }
}

void XMLTextParagraphExport::exportTextFootnoteConfigurationHelper(
    const Reference<XPropertySet> & rFootnoteConfig,
    sal_Bool bIsEndnote)
{
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
                             GetXMLToken( bIsEndnote ? XML_ENDNOTE
                                                     : XML_FOOTNOTE ) );
    // default/paragraph style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sParaStyleName,
                      XML_NAMESPACE_TEXT, XML_DEFAULT_STYLE_NAME,
                      sal_True, sal_True);

    // citation style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_STYLE_NAME,
                      sal_True, sal_True);

    // citation body style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sAnchorCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_BODY_STYLE_NAME,
                      sal_True, sal_True);

    // page style
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sPageStyleName,
                      XML_NAMESPACE_TEXT, XML_MASTER_PAGE_NAME,
                      sal_True, sal_True );

    // prefix
    lcl_exportString( GetExport(), rFootnoteConfig, sPrefix,
                      XML_NAMESPACE_STYLE, XML_NUM_PREFIX, sal_False, sal_True);

    // suffix
    lcl_exportString( GetExport(), rFootnoteConfig, sSuffix,
                      XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, sal_False, sal_True);



    Any aAny;

    // numbering style
    OUStringBuffer sBuffer;
    aAny = rFootnoteConfig->getPropertyValue(sNumberingType);
    sal_Int16 nNumbering = 0;
    aAny >>= nNumbering;
    GetExport().GetMM100UnitConverter().convertNumFormat( sBuffer, nNumbering);
    GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                             sBuffer.makeStringAndClear() );
    GetExport().GetMM100UnitConverter().convertNumLetterSync( sBuffer, nNumbering);
    if (sBuffer.getLength() )
    {
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                                 sBuffer.makeStringAndClear());
    }

    // StartAt / start-value
    aAny = rFootnoteConfig->getPropertyValue(sStartAt);
    sal_Int16 nOffset = 0;
    aAny >>= nOffset;
    ::sax::Converter::convertNumber(sBuffer, (sal_Int32)nOffset);
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_START_VALUE,
                             sBuffer.makeStringAndClear());

    // some properties are for footnotes only
    if (!bIsEndnote)
    {
        // footnotes position
        aAny = rFootnoteConfig->getPropertyValue(
            sPositionEndOfDoc);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_FOOTNOTES_POSITION,
                                 ( (*(sal_Bool *)aAny.getValue()) ?
                                        XML_DOCUMENT : XML_PAGE ) );

        aAny = rFootnoteConfig->getPropertyValue(sFootnoteCounting);
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
        sal_True, sal_True);

    // two element for footnote content
    if (!bIsEndnote)
    {
        OUString sTmp;

        // end notice / quo vadis
        aAny = rFootnoteConfig->getPropertyValue(sEndNotice);
        aAny >>= sTmp;

        if (!sTmp.isEmpty())
        {
            SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD,
                                     sal_True, sal_False);
            GetExport().Characters(sTmp);
        }

        // begin notice / ergo sum
        aAny = rFootnoteConfig->getPropertyValue(sBeginNotice);
        aAny >>= sTmp;

        if (!sTmp.isEmpty())
        {
            SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD,
                                     sal_True, sal_False);
            GetExport().Characters(sTmp);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
