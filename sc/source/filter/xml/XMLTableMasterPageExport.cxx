/*************************************************************************
 *
 *  $RCSfile: XMLTableMasterPageExport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-24 14:09:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexprt.hxx"
#endif

#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#include "XMLTableMasterPageExport.hxx"
#endif

#include "unonames.hxx"


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;

XMLTableMasterPageExport::XMLTableMasterPageExport( ScXMLExport& rExp ) :
        XMLTextMasterPageExport ( rExp )
{
}

XMLTableMasterPageExport::~XMLTableMasterPageExport()
{
}

void XMLTableMasterPageExport::exportHeaderFooter(const com::sun::star::uno::Reference < com::sun::star::sheet::XHeaderFooterContent >& xHeaderFooter,
                                                    const sal_Char *pName,
                                                    const sal_Bool bDisplay)
{
    if( xHeaderFooter.is() )
    {
        if( !bDisplay )
            GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                            sXML_display, sXML_false );
        Reference < XText > xCenter = xHeaderFooter->getCenterText();
        Reference < XText > xLeft = xHeaderFooter->getLeftText();
        Reference < XText > xRight = xHeaderFooter->getRightText();
        if (xCenter.is() && xLeft.is() && xRight.is())
        {
            rtl::OUString sCenter (xCenter->getString());
            rtl::OUString sLeft (xLeft->getString());
            rtl::OUString sRight (xRight->getString());

            if (sCenter.getLength() || sLeft.getLength() || sRight.getLength())
            {
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                            pName, sal_True, sal_True );
                if (sCenter.getLength() && !sLeft.getLength() && !sRight.getLength())
                    exportHeaderFooterContent( xCenter, sal_False );
                else
                {
                    if (sLeft.getLength())
                    {
                        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                                    sXML_region_left, sal_True, sal_True );
                         exportHeaderFooterContent( xLeft, sal_False );
                    }
                    if (sCenter.getLength())
                    {
                        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                                    sXML_region_center, sal_True, sal_True );
                         exportHeaderFooterContent( xCenter, sal_False );
                    }
                    if (sRight.getLength())
                    {
                        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                                    sXML_region_right, sal_True, sal_True );
                         exportHeaderFooterContent( xRight, sal_False );
                    }
                }
            }
        }
    }
}

void XMLTableMasterPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                sal_Bool bAutoStyles )
{
    Any aAny;

    Reference < sheet::XHeaderFooterContent > xHeader;
    aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_RIGHTHDRCON ) ) );
    aAny >>= xHeader;

    Reference < sheet::XHeaderFooterContent > xHeaderLeft;
    aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_LEFTHDRCONT ) ) );
    aAny >>= xHeaderLeft;

    Reference < sheet::XHeaderFooterContent > xFooter;
    aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_RIGHTFTRCON ) ) );
    aAny >>= xFooter;

    Reference < sheet::XHeaderFooterContent > xFooterLeft;
    aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_RIGHTFTRCON ) ) );
    aAny >>= xFooterLeft;

    if( bAutoStyles )
    {
        if( xHeader.is() )
        {
            exportHeaderFooterContent( xHeader->getCenterText(), sal_True );
            exportHeaderFooterContent( xHeader->getLeftText(), sal_True );
            exportHeaderFooterContent( xHeader->getRightText(), sal_True );
        }
        if( xHeaderLeft.is())
        {
            exportHeaderFooterContent( xHeaderLeft->getCenterText(), sal_True );
            exportHeaderFooterContent( xHeaderLeft->getLeftText(), sal_True );
            exportHeaderFooterContent( xHeaderLeft->getRightText(), sal_True );
        }
        if( xFooter.is() )
        {
            exportHeaderFooterContent( xFooter->getCenterText(), sal_True );
            exportHeaderFooterContent( xFooter->getLeftText(), sal_True );
            exportHeaderFooterContent( xFooter->getRightText(), sal_True );
        }
        if( xFooterLeft.is())
        {
            exportHeaderFooterContent( xFooterLeft->getCenterText(), sal_True );
            exportHeaderFooterContent( xFooterLeft->getLeftText(), sal_True );
            exportHeaderFooterContent( xFooterLeft->getRightText(), sal_True );
        }
    }
    else
    {
        aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_HDRON ) ) );
        sal_Bool bHeader = *(sal_Bool *)aAny.getValue();

        exportHeaderFooter(xHeader, sXML_header, bHeader );

        aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_HDRSHARED ) ) );
        sal_Bool bLeftHeader = !(*(sal_Bool *)aAny.getValue()) && bHeader;

        exportHeaderFooter( xHeaderLeft, sXML_header_left, bLeftHeader );

        aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_FTRON ) ) );
        sal_Bool bFooter = *(sal_Bool *)aAny.getValue();

        exportHeaderFooter( xFooter, sXML_footer, bFooter );

        aAny = rPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_PAGE_FTRSHARED ) ) );
        sal_Bool bLeftFooter = !(*(sal_Bool *)aAny.getValue()) && bFooter;

        exportHeaderFooter( xFooterLeft, sXML_footer_left, bLeftFooter );
    }
}

