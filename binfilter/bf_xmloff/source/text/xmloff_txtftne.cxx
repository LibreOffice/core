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

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif



#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif




#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_ 
#include <com/sun/star/text/XFootnote.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XENDNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_FOOTNOTENUMBERING_HPP_
#include <com/sun/star/text/FootnoteNumbering.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTCHARSTYLENAMESELEMENTEXPORT_HXX
#include "XMLTextCharStyleNamesElementExport.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::binfilter::xmloff::token;


void XMLTextParagraphExport::exportTextFootnote(
    const Reference<XPropertySet> & rPropSet,
    const OUString& sText,
    sal_Bool bAutoStyles, sal_Bool bProgress )
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
        exportTextFootnoteHelper(xFootnote, xText, sText, 
                                 bAutoStyles, bIsEndnote, bProgress );
    }
    else
    {	
        // create span (for citation mark) if necessary; footnote content 
        // wil be handled via exportTextFootnoteHelper, exportText
        sal_Bool bHasHyperlink;
        sal_Bool bIsUICharStyle = sal_False;
        OUString sStyle = FindTextStyleAndHyperlink( rPropSet, bHasHyperlink, 
                                                     bIsUICharStyle );
        sal_Bool bHasStyle = (sStyle.getLength() > 0);
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
                Any aAny = rPropSet->getPropertyValue(sHyperLinkEvents);
                Reference<XNameReplace> xName;
                aAny >>= xName;
                GetExport().GetEventExport().Export(xName, sal_False);
            }
        }

        {
            XMLTextCharStyleNamesElementExport aCharStylesExport(
                GetExport(), bIsUICharStyle && 
                             aCharStyleNamesPropInfoCache.hasProperty(
                                                    rPropSet ),
                rPropSet, sCharStyleNames );
            if( sStyle.getLength() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                          sStyle );
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                          XML_SPAN, sal_False, sal_False );
                exportTextFootnoteHelper(xFootnote, xText, sText, 
                                         bAutoStyles, bIsEndnote, bProgress );
            }
            else
            {
                exportTextFootnoteHelper(xFootnote, xText, sText, 
                                         bAutoStyles, bIsEndnote, bProgress );
            }
        }
    }
}


void XMLTextParagraphExport::exportTextFootnoteHelper(
    const Reference<XFootnote> & rFootnote,
    const Reference<XText> & rText,
    const OUString& sText,
    sal_Bool bAutoStyles,
    sal_Bool bIsEndnote,
    sal_Bool bProgress )
{
    if (bAutoStyles)
    {
        exportText(rText, bAutoStyles, bProgress, sal_True );
    }
    else
    {
        // export reference Id (for reference fields)
        Reference<XPropertySet> xPropSet(rFootnote, UNO_QUERY);
        Any aAny = xPropSet->getPropertyValue(sReferenceId);
        sal_Int32 nNumber;
        aAny >>= nNumber;
        OUStringBuffer aBuf;
        aBuf.appendAscii("ftn");
        aBuf.append(nNumber);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 aBuf.makeStringAndClear());

        SvXMLElementExport aNote(GetExport(), XML_NAMESPACE_TEXT,
                                 (bIsEndnote ? XML_ENDNOTE : XML_FOOTNOTE),
                                 sal_False, sal_False);
        {
            // handle label vs. automatic numbering
            OUString sLabel = rFootnote->getLabel();
            if (sLabel.getLength()>0)
            {
                GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_LABEL, 
                                         sLabel);
            }
            // else: automatic numbering -> no attribute

            SvXMLElementExport aCite(GetExport(), XML_NAMESPACE_TEXT,
                                     (bIsEndnote ? XML_ENDNOTE_CITATION :
                                                   XML_FOOTNOTE_CITATION),
                                     sal_False, sal_False);
            GetExport().Characters(sText);
        }

        { 
            SvXMLElementExport aBody(GetExport(), XML_NAMESPACE_TEXT,
                                     (bIsEndnote ? XML_ENDNOTE_BODY :
                                                   XML_FOOTNOTE_BODY),
                                     sal_False, sal_False);
            exportText(rText, bAutoStyles, bProgress, sal_True );
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


void lcl_exportString(
    SvXMLExport& rExport,
    const Reference<XPropertySet> & rPropSet,
    const OUString& sProperty,
    sal_uInt16 nPrefix,
    enum XMLTokenEnum eElement,
    sal_Bool bOmitIfEmpty = sal_True)
{
    DBG_ASSERT( eElement != XML_TOKEN_INVALID, "need element token");

    Any aAny = rPropSet->getPropertyValue(sProperty);
    OUString sTmp;
    aAny >>= sTmp;
    if (!bOmitIfEmpty || (sTmp.getLength() > 0))
    {
        rExport.AddAttribute(nPrefix, eElement, sTmp);
    }
}

void XMLTextParagraphExport::exportTextFootnoteConfigurationHelper(
    const Reference<XPropertySet> & rFootnoteConfig,
    sal_Bool bIsEndnote)
{
    // default/paragraph style
    lcl_exportString( GetExport(), rFootnoteConfig, sParaStyleName, 
                      XML_NAMESPACE_TEXT, XML_DEFAULT_STYLE_NAME, sal_True);

    // citation style
    lcl_exportString( GetExport(), rFootnoteConfig, sCharStyleName, 
                      XML_NAMESPACE_TEXT, XML_CITATION_STYLE_NAME, sal_True);

    // citation body style
    lcl_exportString( GetExport(), rFootnoteConfig, sAnchorCharStyleName, 
                      XML_NAMESPACE_TEXT, XML_CITATION_BODY_STYLE_NAME, 
                      sal_True);

    // page style
    lcl_exportString( GetExport(), rFootnoteConfig, sPageStyleName, 
                      XML_NAMESPACE_TEXT, XML_MASTER_PAGE_NAME, sal_True);

    // prefix
    lcl_exportString( GetExport(), rFootnoteConfig, sPrefix, 
                      XML_NAMESPACE_STYLE, XML_NUM_PREFIX, sal_True);

    // suffix
    lcl_exportString( GetExport(), rFootnoteConfig, sSuffix, 
                      XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, sal_True);



    Any aAny;

    // numbering style
    OUStringBuffer sBuffer;
    aAny = rFootnoteConfig->getPropertyValue(sNumberingType);
    sal_Int16 nNumbering;
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
    sal_Int16 nOffset;
    aAny >>= nOffset;
    SvXMLUnitConverter::convertNumber(sBuffer, (sal_Int32)nOffset);
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
        sal_Int16 nTmp;
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
        ( bIsEndnote ? XML_ENDNOTES_CONFIGURATION:XML_FOOTNOTES_CONFIGURATION),
        sal_True, sal_True);

    // two element for footnote content
    if (!bIsEndnote)
    {
        OUString sTmp;

        // end notice / quo vadis
        aAny = rFootnoteConfig->getPropertyValue(sEndNotice);
        aAny >>= sTmp;

        if (sTmp.getLength() > 0)
        {
            SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT, 
                                     XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD,
                                     sal_True, sal_False);
            GetExport().Characters(sTmp);
        }

        // begin notice / ergo sum
        aAny = rFootnoteConfig->getPropertyValue(sBeginNotice);
        aAny >>= sTmp;

        if (sTmp.getLength() > 0)
        {
            SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_TEXT, 
                                     XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD,
                                     sal_True, sal_False);
            GetExport().Characters(sTmp);
        }
    }
}
}//end of namespace binfilter
