/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>


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

