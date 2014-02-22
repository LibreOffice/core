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

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "XMLTableMasterPageExport.hxx"
#include <comphelper/extract.hxx>

#include "unonames.hxx"
#include "xmlexprt.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace xmloff::token;

XMLTableMasterPageExport::XMLTableMasterPageExport( ScXMLExport& rExp ) :
        XMLTextMasterPageExport ( rExp )
{
}

XMLTableMasterPageExport::~XMLTableMasterPageExport()
{
}

void XMLTableMasterPageExport::exportHeaderFooterContent(
            const Reference< XText >& rText,
            bool bAutoStyles, bool bProgress )
{
    OSL_ENSURE( rText.is(), "There is the text" );

    if( bAutoStyles )
        GetExport().GetTextParagraphExport()
                ->collectTextAutoStyles( rText, bProgress, false );
    else
    {
        GetExport().GetTextParagraphExport()->exportTextDeclarations( rText );
        GetExport().GetTextParagraphExport()->exportText( rText, bProgress, false );
    }
}

void XMLTableMasterPageExport::exportHeaderFooter(const com::sun::star::uno::Reference < com::sun::star::sheet::XHeaderFooterContent >& xHeaderFooter,
                                                    const XMLTokenEnum aName,
                                                    const bool bDisplay)
{
    if( xHeaderFooter.is() )
    {
        Reference < XText > xCenter(xHeaderFooter->getCenterText());
        Reference < XText > xLeft(xHeaderFooter->getLeftText());
        Reference < XText > xRight(xHeaderFooter->getRightText());
        if (xCenter.is() && xLeft.is() && xRight.is())
        {
            OUString sCenter (xCenter->getString());
            OUString sLeft (xLeft->getString());
            OUString sRight (xRight->getString());

            if( !bDisplay )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                      aName, sal_True, sal_True );
            if (!sCenter.isEmpty() && sLeft.isEmpty() && sRight.isEmpty())
                exportHeaderFooterContent( xCenter, false, false );
            else
            {
                if (!sLeft.isEmpty())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_LEFT, sal_True, sal_True );
                    exportHeaderFooterContent( xLeft, false, false );
                }
                if (!sCenter.isEmpty())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_CENTER, sal_True, sal_True );
                    exportHeaderFooterContent( xCenter, false, false );
                }
                if (!sRight.isEmpty())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_RIGHT, sal_True, sal_True );
                    exportHeaderFooterContent( xRight, false, false );
                }
            }
        }
    }
}

void XMLTableMasterPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                bool bAutoStyles )
{
    Reference < sheet::XHeaderFooterContent > xHeader(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_RIGHTHDRCON ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xHeaderLeft(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_LEFTHDRCONT ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xFooter(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_RIGHTFTRCON ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xFooterLeft(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_LEFTFTRCONT ) ), uno::UNO_QUERY);

    if( bAutoStyles )
    {
        if( xHeader.is() )
        {
            exportHeaderFooterContent( xHeader->getCenterText(), true, false );
            exportHeaderFooterContent( xHeader->getLeftText(), true, false );
            exportHeaderFooterContent( xHeader->getRightText(), true, false );
        }
        if( xHeaderLeft.is())
        {
            exportHeaderFooterContent( xHeaderLeft->getCenterText(), true, false );
            exportHeaderFooterContent( xHeaderLeft->getLeftText(), true, false );
            exportHeaderFooterContent( xHeaderLeft->getRightText(), true, false );
        }
        if( xFooter.is() )
        {
            exportHeaderFooterContent( xFooter->getCenterText(), true, false );
            exportHeaderFooterContent( xFooter->getLeftText(), true, false );
            exportHeaderFooterContent( xFooter->getRightText(), true, false );
        }
        if( xFooterLeft.is())
        {
            exportHeaderFooterContent( xFooterLeft->getCenterText(), true, false );
            exportHeaderFooterContent( xFooterLeft->getLeftText(), true, false );
            exportHeaderFooterContent( xFooterLeft->getRightText(), true, false );
        }
    }
    else
    {
        sal_Bool bHeader(::cppu::any2bool(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_HDRON ) )));

        exportHeaderFooter(xHeader, XML_HEADER, bHeader );

        sal_Bool bLeftHeader(!::cppu::any2bool(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_HDRSHARED ) )) && bHeader);

        exportHeaderFooter( xHeaderLeft, XML_HEADER_LEFT, bLeftHeader );

        sal_Bool bFooter(::cppu::any2bool(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_FTRON ) )));

        exportHeaderFooter( xFooter, XML_FOOTER, bFooter );

        sal_Bool bLeftFooter = (!::cppu::any2bool(rPropSet->getPropertyValue( OUString( SC_UNO_PAGE_FTRSHARED ) )) && bFooter);

        exportHeaderFooter( xFooterLeft, XML_FOOTER_LEFT, bLeftFooter );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
