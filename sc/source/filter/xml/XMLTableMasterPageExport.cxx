/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableMasterPageExport.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:16:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#include "XMLTableMasterPageExport.hxx"
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

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
                ->collectTextAutoStyles( rText, bProgress, sal_False );
    else
    {
        GetExport().GetTextParagraphExport()->exportTextDeclarations( rText );
        GetExport().GetTextParagraphExport()->exportText( rText, bProgress, sal_False );
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
                exportHeaderFooterContent( xCenter, sal_False, sal_False );
            else
            {
                if (sLeft.getLength())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_LEFT, sal_True, sal_True );
                    exportHeaderFooterContent( xLeft, sal_False, sal_False );
                }
                if (sCenter.getLength())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_CENTER, sal_True, sal_True );
                    exportHeaderFooterContent( xCenter, sal_False, sal_False );
                }
                if (sRight.getLength())
                {
                    SvXMLElementExport aSubElem( GetExport(), XML_NAMESPACE_STYLE,
                                                XML_REGION_RIGHT, sal_True, sal_True );
                    exportHeaderFooterContent( xRight, sal_False, sal_False );
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
            exportHeaderFooterContent( xHeader->getCenterText(), sal_True, sal_False );
            exportHeaderFooterContent( xHeader->getLeftText(), sal_True, sal_False );
            exportHeaderFooterContent( xHeader->getRightText(), sal_True, sal_False );
        }
        if( xHeaderLeft.is())
        {
            exportHeaderFooterContent( xHeaderLeft->getCenterText(), sal_True, sal_False );
            exportHeaderFooterContent( xHeaderLeft->getLeftText(), sal_True, sal_False );
            exportHeaderFooterContent( xHeaderLeft->getRightText(), sal_True, sal_False );
        }
        if( xFooter.is() )
        {
            exportHeaderFooterContent( xFooter->getCenterText(), sal_True, sal_False );
            exportHeaderFooterContent( xFooter->getLeftText(), sal_True, sal_False );
            exportHeaderFooterContent( xFooter->getRightText(), sal_True, sal_False );
        }
        if( xFooterLeft.is())
        {
            exportHeaderFooterContent( xFooterLeft->getCenterText(), sal_True, sal_False );
            exportHeaderFooterContent( xFooterLeft->getLeftText(), sal_True, sal_False );
            exportHeaderFooterContent( xFooterLeft->getRightText(), sal_True, sal_False );
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

