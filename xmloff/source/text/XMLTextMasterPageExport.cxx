/*************************************************************************
 *
 *  $RCSfile: XMLTextMasterPageExport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-22 19:51:07 $
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
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX
#include "XMLTextMasterPageExport.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;

XMLTextMasterPageExport::XMLTextMasterPageExport( SvXMLExport& rExp ) :
    XMLPageExport( rExp ),
    sHeaderText( RTL_CONSTASCII_USTRINGPARAM( "HeaderText" ) ),
    sHeaderOn( RTL_CONSTASCII_USTRINGPARAM( "HeaderIsOn" ) ),
    sHeaderShareContent( RTL_CONSTASCII_USTRINGPARAM( "HeaderIsShared" ) ),
    sHeaderTextLeft( RTL_CONSTASCII_USTRINGPARAM( "HeaderTextLeft" ) ),
    sFooterText( RTL_CONSTASCII_USTRINGPARAM( "FooterText" ) ),
    sFooterOn( RTL_CONSTASCII_USTRINGPARAM( "FooterIsOn" ) ),
    sFooterShareContent( RTL_CONSTASCII_USTRINGPARAM( "FooterIsShared" ) ),
    sFooterTextLeft( RTL_CONSTASCII_USTRINGPARAM( "FooterTextLeft" ) )
{
}

XMLTextMasterPageExport::~XMLTextMasterPageExport()
{
}


void XMLTextMasterPageExport::exportHeaderFooterContent(
            const Reference< XText >& rText,
            sal_Bool bAutoStyles, sal_Bool bExportParagraph )
{
    DBG_ASSERT( rText.is(), "There is the text" );
    if( bAutoStyles )
        GetExport().GetTextParagraphExport()
                ->collectTextAutoStyles( rText, sal_False, bExportParagraph );
    else
        GetExport().GetTextParagraphExport()->exportText( rText, sal_False, bExportParagraph );
}

void XMLTextMasterPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                sal_Bool bAutoStyles )
{
    Any aAny;

    Reference < XText > xHeaderText;
    aAny = rPropSet->getPropertyValue( sHeaderText );
    aAny >>= xHeaderText;

    Reference < XText > xHeaderTextLeft;
    aAny = rPropSet->getPropertyValue( sHeaderTextLeft );
    aAny >>= xHeaderTextLeft;

    Reference < XText > xFooterText;
    aAny = rPropSet->getPropertyValue( sFooterText );
    aAny >>= xFooterText;

    Reference < XText > xFooterTextLeft;
    aAny = rPropSet->getPropertyValue( sFooterTextLeft );
    aAny >>= xFooterTextLeft;

    if( bAutoStyles )
    {
        if( xHeaderText.is() )
            exportHeaderFooterContent( xHeaderText, sal_True );
        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
            exportHeaderFooterContent( xHeaderTextLeft, sal_True );
        if( xFooterText.is() )
            exportHeaderFooterContent( xFooterText, sal_True );
        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
            exportHeaderFooterContent( xFooterTextLeft, sal_True );
    }
    else
    {
        aAny = rPropSet->getPropertyValue( sHeaderOn );
        sal_Bool bHeader = *(sal_Bool *)aAny.getValue();

        sal_Bool bHeaderLeft = sal_False;
        if( bHeader )
        {
            aAny = rPropSet->getPropertyValue( sHeaderShareContent );
            bHeaderLeft = !*(sal_Bool *)aAny.getValue();
        }

        if( xHeaderText.is() )
        {
            if( !bHeader )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                             sXML_display, sXML_false );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        sXML_header, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderText, sal_False );
        }

        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
        {
            if( !bHeaderLeft )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                             sXML_display, sXML_false );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        sXML_header_left, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderTextLeft, sal_False );
        }

        aAny = rPropSet->getPropertyValue( sFooterOn );
        sal_Bool bFooter = *(sal_Bool *)aAny.getValue();

        sal_Bool bFooterLeft = sal_False;
        if( bFooter )
        {
            aAny = rPropSet->getPropertyValue( sFooterShareContent );
            bFooterLeft = !*(sal_Bool *)aAny.getValue();
        }

        if( xFooterText.is() )
        {
            if( !bFooter )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                             sXML_display, sXML_false );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        sXML_footer, sal_True, sal_True );
            exportHeaderFooterContent( xFooterText, sal_False );
        }

        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
        {
            if( !bFooterLeft )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                             sXML_display, sXML_false );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        sXML_footer_left, sal_True, sal_True );
            exportHeaderFooterContent( xFooterTextLeft, sal_False );
        }
    }
}

