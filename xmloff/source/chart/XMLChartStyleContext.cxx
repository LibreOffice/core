/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChartStyleContext.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:33:51 $
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
#include "XMLChartStyleContext.hxx"

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif

#include "XMLChartPropertyContext.hxx"

using namespace com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_DATA_STYLE_NAME;
using ::xmloff::token::XML_PERCENTAGE_DATA_STYLE_NAME;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_CHART_PROPERTIES;


TYPEINIT1( XMLChartStyleContext, XMLPropStyleContext );

// protected

void XMLChartStyleContext::SetAttribute(
    sal_uInt16 nPrefixKey,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rValue )
{
    if( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
    {
        msDataStyleName =rValue;
    }
    if( IsXMLToken( rLocalName, XML_PERCENTAGE_DATA_STYLE_NAME ) )
    {
        msPercentageDataStyleName =rValue;
    }
    else
    {
        XMLShapeStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

//public

// CTOR
XMLChartStyleContext::XMLChartStyleContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    SvXMLStylesContext& rStyles, sal_uInt16 nFamily ) :

        XMLShapeStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
        mrStyles( rStyles )
{}

// DTOR
XMLChartStyleContext::~XMLChartStyleContext()
{}

namespace
{

    void lcl_NumberFormatStyleToProperty( const ::rtl::OUString& rStyleName, const ::rtl::OUString& rPropertyName,
                                      const SvXMLStylesContext& rStylesContext,
                                      const uno::Reference< beans::XPropertySet >& rPropSet )
{
    if( rStyleName.getLength())
    {
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)rStylesContext.FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, rStyleName, sal_True );
        if( pStyle )
        {
            uno::Any aNumberFormat;
            sal_Int32 nNumberFormat = pStyle->GetKey();
            aNumberFormat <<= nNumberFormat;
            rPropSet->setPropertyValue( rPropertyName, aNumberFormat );
        }
    }
}

}// anonymous namespace

void XMLChartStyleContext::FillPropertySet(
    const uno::Reference< beans::XPropertySet > & rPropSet )
{
    try
    {
        XMLShapeStyleContext::FillPropertySet( rPropSet );
    }
    catch( beans::UnknownPropertyException&  )
    {
        DBG_ASSERT( false, "unknown property exception -> shape style not completly imported for chart style" );
    }

    lcl_NumberFormatStyleToProperty( msDataStyleName, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberFormat" )), mrStyles, rPropSet );
    lcl_NumberFormatStyleToProperty( msPercentageDataStyleName, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PercentageNumberFormat" )), mrStyles, rPropSet );
}

SvXMLImportContext *XMLChartStyleContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_CHART;
        if( nFamily )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                pContext = new XMLChartPropertyContext(
                    GetImport(), nPrefix, rLocalName, xAttrList, nFamily,
                    GetProperties(), xImpPrMap );
        }
    }

    if( !pContext )
        pContext = XMLShapeStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                             xAttrList );

    return pContext;
}
