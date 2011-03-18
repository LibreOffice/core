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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include <tools/debug.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "XMLTableMasterPageExport.hxx"
#include <comphelper/extract.hxx>

#include "unonames.hxx"
#include "xmlexprt.hxx"

using ::rtl::OUString;
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
            sal_Bool bAutoStyles, sal_Bool bProgress )
{
    DBG_ASSERT( rText.is(), "There is the text" );

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
                                                    const sal_Bool bDisplay)
{
    if( xHeaderFooter.is() )
    {
        Reference < XText > xCenter(xHeaderFooter->getCenterText());
        Reference < XText > xLeft(xHeaderFooter->getLeftText());
        Reference < XText > xRight(xHeaderFooter->getRightText());
        if (xCenter.is() && xLeft.is() && xRight.is())
        {
            rtl::OUString sCenter (xCenter->getString());
            rtl::OUString sLeft (xLeft->getString());
            rtl::OUString sRight (xRight->getString());

            if( !bDisplay )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                      aName, sal_True, sal_True );
            if (sCenter.getLength() && !sLeft.getLength() && !sRight.getLength())
                exportHeaderFooterContent( xCenter, false, false );
            else
            {
                if (sLeft.getLength())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_LEFT, sal_True, sal_True );
                    exportHeaderFooterContent( xLeft, false, false );
                }
                if (sCenter.getLength())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_CENTER, sal_True, sal_True );
                    exportHeaderFooterContent( xCenter, false, false );
                }
                if (sRight.getLength())
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
                sal_Bool bAutoStyles )
{
    Reference < sheet::XHeaderFooterContent > xHeader(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_RIGHTHDRCON ) ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xHeaderLeft(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_LEFTHDRCONT ) ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xFooter(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_RIGHTFTRCON ) ) ), uno::UNO_QUERY);

    Reference < sheet::XHeaderFooterContent > xFooterLeft(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_LEFTFTRCONT ) ) ), uno::UNO_QUERY);

    if( bAutoStyles )
    {
        if( xHeader.is() )
        {
            exportHeaderFooterContent( xHeader->getCenterText(), sal_True, false );
            exportHeaderFooterContent( xHeader->getLeftText(), sal_True, false );
            exportHeaderFooterContent( xHeader->getRightText(), sal_True, false );
        }
        if( xHeaderLeft.is())
        {
            exportHeaderFooterContent( xHeaderLeft->getCenterText(), sal_True, false );
            exportHeaderFooterContent( xHeaderLeft->getLeftText(), sal_True, false );
            exportHeaderFooterContent( xHeaderLeft->getRightText(), sal_True, false );
        }
        if( xFooter.is() )
        {
            exportHeaderFooterContent( xFooter->getCenterText(), sal_True, false );
            exportHeaderFooterContent( xFooter->getLeftText(), sal_True, false );
            exportHeaderFooterContent( xFooter->getRightText(), sal_True, false );
        }
        if( xFooterLeft.is())
        {
            exportHeaderFooterContent( xFooterLeft->getCenterText(), sal_True, false );
            exportHeaderFooterContent( xFooterLeft->getLeftText(), sal_True, false );
            exportHeaderFooterContent( xFooterLeft->getRightText(), sal_True, false );
        }
    }
    else
    {
        sal_Bool bHeader(::cppu::any2bool(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_HDRON ) ) )));

        exportHeaderFooter(xHeader, XML_HEADER, bHeader );

        sal_Bool bLeftHeader(!::cppu::any2bool(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_HDRSHARED ) ) )) && bHeader);

        exportHeaderFooter( xHeaderLeft, XML_HEADER_LEFT, bLeftHeader );

        sal_Bool bFooter(::cppu::any2bool(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_FTRON ) ) )));

        exportHeaderFooter( xFooter, XML_FOOTER, bFooter );

        sal_Bool bLeftFooter = (!::cppu::any2bool(rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_FTRSHARED ) ) )) && bFooter);

        exportHeaderFooter( xFooterLeft, XML_FOOTER_LEFT, bLeftFooter );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
