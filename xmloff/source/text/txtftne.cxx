/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnume.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/txtparae.hxx>


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
    
    Any aAny;
    aAny = rPropSet->getPropertyValue(sFootnote);
    Reference<XFootnote> xFootnote;
    aAny >>= xFootnote;
    Reference<XText> xText(xFootnote, UNO_QUERY);

    
    Reference<XServiceInfo> xServiceInfo( xFootnote, UNO_QUERY );
    sal_Bool bIsEndnote = xServiceInfo->supportsService(sTextEndnoteService);

    if (bAutoStyles)
    {
        
        Add( XML_STYLE_FAMILY_TEXT_TEXT, rPropSet );

        
        exportTextFootnoteHelper(xFootnote, xText, rText,
                                 bAutoStyles, bIsEndnote, bIsProgress );
    }
    else
    {
        
        
        sal_Bool bHasHyperlink;
        sal_Bool bIsUICharStyle = sal_False;
        sal_Bool bHasAutoStyle = sal_False;

        OUString sStyle = FindTextStyleAndHyperlink( rPropSet, bHasHyperlink,
                                                     bIsUICharStyle, bHasAutoStyle );

        
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
            
            OUString sHyperLinkEvents("HyperLinkEvents");
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
            
            OUString sLabel = rFootnote->getLabel();
            if (!sLabel.isEmpty())
            {
                GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_LABEL,
                                         sLabel);
            }
            

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
    
    Reference<XFootnotesSupplier> aFootnotesSupplier(GetExport().GetModel(),
                                                     UNO_QUERY);
    Reference<XPropertySet> aFootnoteConfiguration(
        aFootnotesSupplier->getFootnoteSettings());
    exportTextFootnoteConfigurationHelper(aFootnoteConfiguration, sal_False);

    
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
    bool bEncodeName,
    bool bOmitIfEmpty)
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
    
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sParaStyleName,
                      XML_NAMESPACE_TEXT, XML_DEFAULT_STYLE_NAME,
                      true, true);

    
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_STYLE_NAME,
                      true, true);

    
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sAnchorCharStyleName,
                      XML_NAMESPACE_TEXT, XML_CITATION_BODY_STYLE_NAME,
                      true, true);

    
    lcl_exportString( GetExport(), rFootnoteConfig,
                      sPageStyleName,
                      XML_NAMESPACE_TEXT, XML_MASTER_PAGE_NAME,
                      true, true );

    
    lcl_exportString( GetExport(), rFootnoteConfig, sPrefix,
                      XML_NAMESPACE_STYLE, XML_NUM_PREFIX, false, true);

    
    lcl_exportString( GetExport(), rFootnoteConfig, sSuffix,
                      XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, false, true);



    Any aAny;

    
    OUStringBuffer sBuffer;
    aAny = rFootnoteConfig->getPropertyValue(sNumberingType);
    sal_Int16 nNumbering = 0;
    aAny >>= nNumbering;
    GetExport().GetMM100UnitConverter().convertNumFormat( sBuffer, nNumbering);
    GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                             sBuffer.makeStringAndClear() );
    GetExport().GetMM100UnitConverter().convertNumLetterSync( sBuffer, nNumbering);
    if (!sBuffer.isEmpty() )
    {
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                                 sBuffer.makeStringAndClear());
    }

    
    aAny = rFootnoteConfig->getPropertyValue(sStartAt);
    sal_Int16 nOffset = 0;
    aAny >>= nOffset;
    ::sax::Converter::convertNumber(sBuffer, (sal_Int32)nOffset);
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_START_VALUE,
                             sBuffer.makeStringAndClear());

    
    if (!bIsEndnote)
    {
        
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

    
    SvXMLElementExport aFootnoteConfigElement(
        GetExport(), XML_NAMESPACE_TEXT,
        XML_NOTES_CONFIGURATION,
        sal_True, sal_True);

    
    if (!bIsEndnote)
    {
        OUString sTmp;

        
        aAny = rFootnoteConfig->getPropertyValue(sEndNotice);
        aAny >>= sTmp;

        if (!sTmp.isEmpty())
        {
            SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT,
                                     XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD,
                                     sal_True, sal_False);
            GetExport().Characters(sTmp);
        }

        
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
