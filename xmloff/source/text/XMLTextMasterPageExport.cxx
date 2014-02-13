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

#include <tools/debug.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

XMLTextMasterPageExport::XMLTextMasterPageExport( SvXMLExport& rExp ) :
    XMLPageExport( rExp ),
    sHeaderText( "HeaderText" ),
    sHeaderOn( "HeaderIsOn" ),
    sHeaderShareContent( "HeaderIsShared" ),
    sHeaderTextFirst( "HeaderTextFirst" ),
    sHeaderTextLeft( "HeaderTextLeft" ),
    sFirstShareContent( "FirstIsShared" ),
    sFooterText( "FooterText" ),
    sFooterOn( "FooterIsOn" ),
    sFooterShareContent( "FooterIsShared" ),
    sFooterTextFirst( "FooterTextFirst" ),
    sFooterTextLeft( "FooterTextLeft" )
{
}

XMLTextMasterPageExport::~XMLTextMasterPageExport()
{
}


void XMLTextMasterPageExport::exportHeaderFooterContent(
            const Reference< XText >& rText,
            bool bAutoStyles, bool bExportParagraph )
{
    DBG_ASSERT( rText.is(), "There is the text" );

    // tracked changes (autostyles + changes list)
    GetExport().GetTextParagraphExport()->recordTrackedChangesForXText(rText);
    GetExport().GetTextParagraphExport()->exportTrackedChanges(rText,
                                                               bAutoStyles);
    if( bAutoStyles )
        GetExport().GetTextParagraphExport()
                ->collectTextAutoStyles( rText, sal_True, bExportParagraph );
    else
    {
        GetExport().GetTextParagraphExport()->exportTextDeclarations( rText );
        GetExport().GetTextParagraphExport()->exportText( rText, sal_True, bExportParagraph );
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
    aAny = rPropSet->getPropertyValue( sHeaderText );
    aAny >>= xHeaderText;

    Reference < XText > xHeaderTextFirst;
    aAny = rPropSet->getPropertyValue( sHeaderTextFirst );
    aAny >>= xHeaderTextFirst;

    Reference < XText > xHeaderTextLeft;
    aAny = rPropSet->getPropertyValue( sHeaderTextLeft );
    aAny >>= xHeaderTextLeft;

    Reference < XText > xFooterText;
    aAny = rPropSet->getPropertyValue( sFooterText );
    aAny >>= xFooterText;

    Reference < XText > xFooterTextFirst;
    aAny = rPropSet->getPropertyValue( sFooterTextFirst );
    aAny >>= xFooterTextFirst;

    Reference < XText > xFooterTextLeft;
    aAny = rPropSet->getPropertyValue( sFooterTextLeft );
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
        aAny = rPropSet->getPropertyValue( sHeaderOn );
        sal_Bool bHeader = sal_False;
        aAny >>= bHeader;

        sal_Bool bHeaderFirstShared = sal_False;
        if( bHeader )
        {
            aAny = rPropSet->getPropertyValue( sFirstShareContent );
            aAny >>= bHeaderFirstShared;
        }

        sal_Bool bHeaderLeftShared = sal_False;
        if( bHeader )
        {
            aAny = rPropSet->getPropertyValue( sHeaderShareContent );
            aAny >>= bHeaderLeftShared;
        }

        if( xHeaderText.is() )
        {
            if( !bHeader )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderText, false );
        }

        if( xHeaderTextFirst.is() && xHeaderTextFirst != xHeaderText )
        {
            if (bHeaderFirstShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER_FIRST, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderTextFirst, false );
        }

        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
        {
            if (bHeaderLeftShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER_LEFT, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderTextLeft, false );
        }

        aAny = rPropSet->getPropertyValue( sFooterOn );
        sal_Bool bFooter = sal_False;
        aAny >>= bFooter;

        sal_Bool bFooterFirstShared = sal_False;
        if( bFooter )
        {
            aAny = rPropSet->getPropertyValue( sFirstShareContent );
            aAny >>= bFooterFirstShared;
        }

        sal_Bool bFooterLeftShared = sal_False;
        if( bFooter )
        {
            aAny = rPropSet->getPropertyValue( sFooterShareContent );
            aAny >>= bFooterLeftShared;
        }

        if( xFooterText.is() )
        {
            if( !bFooter )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER, sal_True, sal_True );
            exportHeaderFooterContent( xFooterText, false );
        }

        if( xFooterTextFirst.is() && xFooterTextFirst != xFooterText )
        {
            if (bFooterFirstShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER_FIRST, sal_True, sal_True );
            exportHeaderFooterContent( xFooterTextFirst, false );
        }

        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
        {
            if (bFooterLeftShared)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER_LEFT, sal_True, sal_True );
            exportHeaderFooterContent( xFooterTextLeft, false );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
