/*************************************************************************
 *
 *  $RCSfile: XMLFontStylesContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-14 10:19:45 $
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

#ifndef _COM_SUN_STAR_AWT_FONTFAMILY_HPP
#include <com/sun/star/awt/FontFamily.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTPITCH_HPP
#include <com/sun/star/awt/FontPitch.hpp>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include "fonthdl.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif

#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX
#include "XMLFontStylesContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;

#define XML_STYLE_FAMILY_FONT 1

enum XMLFontStyleAttrTokens
{
    XML_TOK_FONT_STYLE_ATTR_FAMILY,
    XML_TOK_FONT_STYLE_ATTR_FAMILY_GENERIC,
    XML_TOK_FONT_STYLE_ATTR_STYLENAME,
    XML_TOK_FONT_STYLE_ATTR_PITCH,
    XML_TOK_FONT_STYLE_ATTR_CHARSET,

    XML_TOK_FONT_STYLE_ATTR_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aFontStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_FO, sXML_font_family,
            XML_TOK_FONT_STYLE_ATTR_FAMILY },
    { XML_NAMESPACE_STYLE, sXML_font_family_generic,
            XML_TOK_FONT_STYLE_ATTR_FAMILY_GENERIC },
    { XML_NAMESPACE_STYLE, sXML_font_style_name,
            XML_TOK_FONT_STYLE_ATTR_STYLENAME },
    { XML_NAMESPACE_STYLE, sXML_font_pitch,
            XML_TOK_FONT_STYLE_ATTR_PITCH },
    { XML_NAMESPACE_STYLE, sXML_font_charset,
            XML_TOK_FONT_STYLE_ATTR_CHARSET },

    XML_TOKEN_MAP_END
};

class XMLFontStyleContext_Impl : public SvXMLStyleContext
{
    Any aFamilyName;
    Any aStyleName;
    Any aFamily;
    Any aPitch;
    Any aEnc;

    SvXMLImportContextRef xStyles;

    XMLFontStylesContext *GetStyles()
    {
        return ((XMLFontStylesContext *)&xStyles);
    }

public:

    TYPEINFO();

    XMLFontStyleContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            XMLFontStylesContext& rStyles );
    virtual ~XMLFontStyleContext_Impl();

    void SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
                       const OUString& rValue );

    void FillProperties( ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const;

};

TYPEINIT1( XMLFontStyleContext_Impl, SvXMLStyleContext );

XMLFontStyleContext_Impl::XMLFontStyleContext_Impl( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        XMLFontStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    xStyles( &rStyles )
{
    SetFamily( XML_STYLE_FAMILY_FONT );
    OUString sEmpty;
    aFamilyName <<= sEmpty;
    aStyleName <<= sEmpty;
    aFamily <<= (sal_Int16)FontFamily::DONTKNOW;
    aPitch <<= (sal_Int16)FontPitch::DONTKNOW;
    aEnc <<= (sal_Int16)rStyles.GetDfltCharset();
}

void XMLFontStyleContext_Impl::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();
    const SvXMLTokenMap& rTokenMap = GetStyles()->GetFontStyleAttrTokenMap();
    Any aAny;

    switch( rTokenMap.Get( nPrefixKey, rLocalName ) )
    {
    case XML_TOK_FONT_STYLE_ATTR_FAMILY:
        if( GetStyles()->GetFamilyNameHdl().importXML( rValue, aAny,
                                                          rUnitConv ) )
            aFamilyName = aAny;
        break;
    case XML_TOK_FONT_STYLE_ATTR_STYLENAME:
        aStyleName <<= rValue;
        break;
    case XML_TOK_FONT_STYLE_ATTR_FAMILY_GENERIC:
        if( GetStyles()->GetFamilyHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aFamily = aAny;
        break;
    case XML_TOK_FONT_STYLE_ATTR_PITCH:
        if( GetStyles()->GetPitchHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aPitch = aAny;
        break;
    case XML_TOK_FONT_STYLE_ATTR_CHARSET:
        if( GetStyles()->GetEncodingHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aEnc = aAny;
        break;
    default:
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
        break;
    }
}

XMLFontStyleContext_Impl::~XMLFontStyleContext_Impl()
{
}

void XMLFontStyleContext_Impl::FillProperties(
        ::std::vector< XMLPropertyState > &rProps,
        sal_Int32 nFamilyNameIdx,
        sal_Int32 nStyleNameIdx,
        sal_Int32 nFamilyIdx,
        sal_Int32 nPitchIdx,
        sal_Int32 nCharsetIdx ) const
{
    if( nFamilyNameIdx != -1 )
    {
        XMLPropertyState aPropState( nFamilyNameIdx, aFamilyName );
        rProps.push_back( aPropState );
    }
    if( nStyleNameIdx != -1 )
    {
        XMLPropertyState aPropState( nStyleNameIdx, aStyleName );
        rProps.push_back( aPropState );
    }
    if( nFamilyIdx != -1 )
    {
        XMLPropertyState aPropState( nFamilyIdx, aFamily );
        rProps.push_back( aPropState );
    }
    if( nPitchIdx != -1 )
    {
        XMLPropertyState aPropState( nPitchIdx, aPitch );
        rProps.push_back( aPropState );
    }
    if( nCharsetIdx != -1 )
    {
        XMLPropertyState aPropState( nCharsetIdx, aEnc );
        rProps.push_back( aPropState );
    }
}

SvXMLStyleContext *XMLFontStylesContext::CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle;
    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.equalsAsciiL( sXML_font_decl, sizeof(sXML_font_decl)-1 ) )
    {
        pStyle = new XMLFontStyleContext_Impl( GetImport(), nPrefix,
                                               rLocalName, xAttrList, *this );
    }
    else
    {
        pStyle = SvXMLStylesContext::CreateStyleChildContext( nPrefix,
                                               rLocalName, xAttrList );
    }

    return pStyle;
}

TYPEINIT1( XMLFontStylesContext, SvXMLStylesContext );

XMLFontStylesContext::XMLFontStylesContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        rtl_TextEncoding eDfltEnc ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    eDfltEncoding( eDfltEnc ),
    pFamilyNameHdl( new XMLFontFamilyNamePropHdl ),
    pFamilyHdl( new XMLFontFamilyPropHdl ),
    pPitchHdl( new XMLFontPitchPropHdl ),
    pEncHdl( new XMLFontEncodingPropHdl ),
    pFontStyleAttrTokenMap( new SvXMLTokenMap(aFontStyleAttrTokenMap) )
{
}

XMLFontStylesContext::~XMLFontStylesContext()
{
    delete pFamilyNameHdl;
    delete pFamilyHdl;
    delete pPitchHdl;
    delete pEncHdl;
    delete pFontStyleAttrTokenMap;
}

sal_Bool XMLFontStylesContext::FillProperties( const OUString& rName,
                         ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const
{
    const XMLFontStyleContext_Impl *pFontStyle =
        PTR_CAST( XMLFontStyleContext_Impl,
            FindStyleChildContext( XML_STYLE_FAMILY_FONT, rName, sal_True ) );
    if( pFontStyle )
        pFontStyle->FillProperties( rProps, nFamilyNameIdx, nStyleNameIdx,
                                    nFamilyIdx, nPitchIdx, nCharsetIdx );
    return 0 != pFontStyle;
}
