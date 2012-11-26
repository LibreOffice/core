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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>

#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif
#include "XMLShapePropertySetContext.hxx"
#include <xmloff/XMLGraphicsDefaultStyle.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;

// ---------------------------------------------------------------------

XMLGraphicsDefaultStyle::XMLGraphicsDefaultStyle( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvXMLStylesContext& rStyles )
: XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, sal_True )
{
}

XMLGraphicsDefaultStyle::~XMLGraphicsDefaultStyle()
{
}

SvXMLImportContext *XMLGraphicsDefaultStyle::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        if( nFamily )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap = GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                pContext = new XMLShapePropertySetContext( GetImport(), nPrefix, rLocalName, xAttrList, nFamily, GetProperties(), xImpPrMap );
        }
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// This method is called for every default style
void XMLGraphicsDefaultStyle::SetDefaults()
{
    Reference< XMultiServiceFactory > xFact( GetImport().GetModel(), UNO_QUERY );
    if( !xFact.is() )
        return;

    Reference< XPropertySet > xDefaults( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults") ) ), UNO_QUERY );
    if( !xDefaults.is() )
        return;
                                            // SJ: #i114750#
    sal_Bool bWordWrapDefault = sal_True;   // initializing with correct odf fo:wrap-option default
    sal_Int32 nUPD( 0 );
    sal_Int32 nBuild( 0 );
    const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
    if ( bBuildIdFound && (
        ((nUPD >= 600) &&  (nUPD < 700))
        ||
        ((nUPD == 300) && (nBuild <= 9535))
        ||
        ((nUPD > 300) && (nUPD <= 330))
    ) )
        bWordWrapDefault = sal_False;

    const OUString sTextWordWrap( RTL_CONSTASCII_USTRINGPARAM( "TextWordWrap" ) );
    Reference< XPropertySetInfo > xInfo( xDefaults->getPropertySetInfo() );
    if ( xInfo->hasPropertyByName( sTextWordWrap ) )
        xDefaults->setPropertyValue( sTextWordWrap, Any( bWordWrapDefault ) );

    FillPropertySet( xDefaults );
}


