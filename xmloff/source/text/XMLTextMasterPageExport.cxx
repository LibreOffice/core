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

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>
#include <sal/log.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

const OUStringLiteral gsHeaderText( "HeaderText" );
const OUStringLiteral gsHeaderOn( "HeaderIsOn" );
const OUStringLiteral gsHeaderShareContent( "HeaderIsShared" );
const OUStringLiteral gsHeaderTextFirst( "HeaderTextFirst" );
const OUStringLiteral gsHeaderTextLeft( "HeaderTextLeft" );
const OUStringLiteral gsFirstShareContent( "FirstIsShared" );
const OUStringLiteral gsFooterText( "FooterText" );
const OUStringLiteral gsFooterOn( "FooterIsOn" );
const OUStringLiteral gsFooterShareContent( "FooterIsShared" );
const OUStringLiteral gsFooterTextFirst( "FooterTextFirst" );
const OUStringLiteral gsFooterTextLeft( "FooterTextLeft" );

XMLTextMasterPageExport::XMLTextMasterPageExport( SvXMLExport& rExp ) :
    XMLPageExport( rExp )
{
}

XMLTextMasterPageExport::~XMLTextMasterPageExport()
{
}


void XMLTextMasterPageExport::exportHeaderFooterContent(
            const Reference< XText >& rText,
            bool bAutoStyles, bool bExportParagraph )
{
    SAL_WARN_IF( !rText.is(), "xmloff", "There is the text" );

    // tracked changes (autostyles + changes list)
    GetExport().GetTextParagraphExport()->recordTrackedChangesForXText(rText);
    GetExport().GetTextParagraphExport()->exportTrackedChanges(rText,
                                                               bAutoStyles);
    if( bAutoStyles )
        GetExport().GetTextParagraphExport()
                ->collectTextAutoStyles( rText, true, bExportParagraph );
    else
    {
        GetExport().GetTextParagraphExport()->exportTextDeclarations( rText );
        GetExport().GetTextParagraphExport()->exportText( rText, true, bExportParagraph );
    }

    // tracked changes (end of XText)
    GetExport().GetTextParagraphExport()->recordTrackedChangesNoXText();
}

void XMLTextMasterPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                bool bAutoStyles )
{
    Any aAny;

    Reference < XText > xHeaderText;
    aAny = rPropSet->getPropertyValue( gsHeaderText );
    aAny >>= xHeaderText;

    Reference < XText > xHeaderTextFirst;
    aAny = rPropSet->getPropertyValue( gsHeaderTextFirst );
    aAny >>= xHeaderTextFirst;

    Reference < XText > xHeaderTextLeft;
    aAny = rPropSet->getPropertyValue( gsHeaderTextLeft );
    aAny >>= xHeaderTextLeft;

    Reference < XText > xFooterText;
    aAny = rPropSet->getPropertyValue( gsFooterText );
    aAny >>= xFooterText;

    Reference < XText > xFooterTextFirst;
    aAny = rPropSet->getPropertyValue( gsFooterTextFirst );
    aAny >>= xFooterTextFirst;

    Reference < XText > xFooterTextLeft;
    aAny = rPropSet->getPropertyValue( gsFooterTextLeft );
    aAny >>= xFooterTextLeft;

    if( bAutoStyles )
    {
        if( xHeaderText.is() )
            exportHeaderFooterContent( xHeaderText, true );
        if( xHeaderTextFirst.is() && xHeaderTextFirst != xHeaderText )
            exportHeaderFooterContent( xHeaderTextFirst, true );
        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
            exportHeaderFooterContent( xHeaderTextLeft, true );
        if( xFooterText.is() )
            exportHeaderFooterContent( xFooterText, true );
        if( xFooterTextFirst.is() && xFooterTextFirst != xFooterText )
            exportHeaderFooterContent( xFooterTextFirst, true );
        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
            exportHeaderFooterContent( xFooterTextLeft, true );
    }
    else
    {
        auto const nVersion(GetExport().getSaneDefaultVersion());

        aAny = rPropSet->getPropertyValue( gsHeaderOn );
        bool bHeader = false;
        aAny >>= bHeader;

        bool bHeaderFirstShared = false;
        if( bHeader )
        {
            aAny = rPropSet->getPropertyValue( gsFirstShareContent );
            aAny >>= bHeaderFirstShared;
        }

        bool bHeaderLeftShared = false;
        if( bHeader )
        {
            aAny = rPropSet->getPropertyValue( gsHeaderShareContent );
            aAny >>= bHeaderLeftShared;
        }

        if( xHeaderText.is() )
        {
            if( !bHeader )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER, true, true );
            exportHeaderFooterContent( xHeaderText, false );
        }

        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
        {
            if (bHeaderLeftShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER_LEFT, true, true );
            exportHeaderFooterContent( xHeaderTextLeft, false );
        }

        if (xHeaderTextFirst.is() && xHeaderTextFirst != xHeaderText
            && SvtSaveOptions::ODFSVER_012 < nVersion)
        {
            if (bHeaderFirstShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            // ODF 1.3 OFFICE-3789
            SvXMLElementExport aElem( GetExport(),
                                        SvtSaveOptions::ODFSVER_013 <= nVersion
                                            ? XML_NAMESPACE_STYLE
                                            : XML_NAMESPACE_LO_EXT,
                                        XML_HEADER_FIRST, true, true );
            exportHeaderFooterContent( xHeaderTextFirst, false );
        }

        aAny = rPropSet->getPropertyValue( gsFooterOn );
        bool bFooter = false;
        aAny >>= bFooter;

        bool bFooterFirstShared = false;
        if( bFooter )
        {
            aAny = rPropSet->getPropertyValue( gsFirstShareContent );
            aAny >>= bFooterFirstShared;
        }

        bool bFooterLeftShared = false;
        if( bFooter )
        {
            aAny = rPropSet->getPropertyValue( gsFooterShareContent );
            aAny >>= bFooterLeftShared;
        }

        if( xFooterText.is() )
        {
            if( !bFooter )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER, true, true );
            exportHeaderFooterContent( xFooterText, false );
        }

        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
        {
            if (bFooterLeftShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER_LEFT, true, true );
            exportHeaderFooterContent( xFooterTextLeft, false );
        }

        if (xFooterTextFirst.is() && xFooterTextFirst != xFooterText
            && SvtSaveOptions::ODFSVER_012 < nVersion)
        {
            if (bFooterFirstShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            // ODF 1.3 OFFICE-3789
            SvXMLElementExport aElem( GetExport(),
                                        SvtSaveOptions::ODFSVER_013 <= nVersion
                                            ? XML_NAMESPACE_STYLE
                                            : XML_NAMESPACE_LO_EXT,
                                        XML_FOOTER_FIRST, true, true );
            exportHeaderFooterContent( xFooterTextFirst, false );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
