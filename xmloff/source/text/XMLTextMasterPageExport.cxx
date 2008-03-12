/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextMasterPageExport.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:04:29 $
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
#include "precompiled_xmloff.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
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

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX
#include <xmloff/XMLTextMasterPageExport.hxx>
#endif


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

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
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER, sal_True, sal_True );
            exportHeaderFooterContent( xHeaderText, sal_False );
        }

        if( xHeaderTextLeft.is() && xHeaderTextLeft != xHeaderText )
        {
            if( !bHeaderLeft )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_HEADER_LEFT, sal_True, sal_True );
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
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER, sal_True, sal_True );
            exportHeaderFooterContent( xFooterText, sal_False );
        }

        if( xFooterTextLeft.is() && xFooterTextLeft != xFooterText )
        {
            if( !bFooterLeft )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DISPLAY, XML_FALSE );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                        XML_FOOTER_LEFT, sal_True, sal_True );
            exportHeaderFooterContent( xFooterTextLeft, sal_False );
        }
    }
}

