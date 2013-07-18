/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xmloff/XMLFontStylesContext.hxx>
#include "XMLFontStylesContext_impl.hxx"

#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <osl/file.hxx>
#include <rtl/logfile.hxx>
#include <vcl/embeddedfontshelper.hxx>

#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "fonthdl.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/maptype.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;


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

static const SvXMLTokenMapEntry* lcl_getFontStyleAttrTokenMap()
{
    static const SvXMLTokenMapEntry aFontStyleAttrTokenMap[] =
    {
        { XML_NAMESPACE_SVG, XML_FONT_FAMILY,
                XML_TOK_FONT_STYLE_ATTR_FAMILY },
        { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC,
                XML_TOK_FONT_STYLE_ATTR_FAMILY_GENERIC },
        { XML_NAMESPACE_STYLE, XML_FONT_ADORNMENTS,
                XML_TOK_FONT_STYLE_ATTR_STYLENAME },
        { XML_NAMESPACE_STYLE, XML_FONT_PITCH,
                XML_TOK_FONT_STYLE_ATTR_PITCH },
        { XML_NAMESPACE_STYLE, XML_FONT_CHARSET,
                XML_TOK_FONT_STYLE_ATTR_CHARSET },

        XML_TOKEN_MAP_END
    };
    return aFontStyleAttrTokenMap;
}

TYPEINIT1( XMLFontStyleContextFontFace, SvXMLStyleContext );

XMLFontStyleContextFontFace::XMLFontStyleContextFontFace( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        XMLFontStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_FONT ),
    xStyles( &rStyles )
{
    OUString sEmpty;
    aFamilyName <<= sEmpty;
    aStyleName <<= sEmpty;
    aFamily <<= (sal_Int16)awt::FontFamily::DONTKNOW;
    aPitch <<= (sal_Int16)awt::FontPitch::DONTKNOW;
    aEnc <<= (sal_Int16)rStyles.GetDfltCharset();
}

void XMLFontStyleContextFontFace::SetAttribute( sal_uInt16 nPrefixKey,
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

XMLFontStyleContextFontFace::~XMLFontStyleContextFontFace()
{
}

void XMLFontStyleContextFontFace::FillProperties(
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

SvXMLImportContext * XMLFontStyleContextFontFace::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList )
{
    if( nPrefix == XML_NAMESPACE_SVG && IsXMLToken( rLocalName, XML_FONT_FACE_SRC ))
        return new XMLFontStyleContextFontFaceSrc( GetImport(), nPrefix, rLocalName, *this );
    return SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

OUString XMLFontStyleContextFontFace::familyName() const
{
    OUString ret;
    aFamilyName >>= ret;
    return ret;
}


TYPEINIT1( XMLFontStyleContextFontFaceSrc, SvXMLImportContext );

XMLFontStyleContextFontFaceSrc::XMLFontStyleContextFontFaceSrc( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const XMLFontStyleContextFontFace& _font )
    : SvXMLImportContext( rImport, nPrfx, rLName )
    , font( _font )
{
}

SvXMLImportContext * XMLFontStyleContextFontFaceSrc::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList )
{
    if( nPrefix == XML_NAMESPACE_SVG && IsXMLToken( rLocalName, XML_FONT_FACE_URI ))
        return new XMLFontStyleContextFontFaceUri( GetImport(), nPrefix, rLocalName, xAttrList, font );
    return SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}


TYPEINIT1( XMLFontStyleContextFontFaceUri, SvXMLImportContext );

XMLFontStyleContextFontFaceUri::XMLFontStyleContextFontFaceUri( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        const XMLFontStyleContextFontFace& _font )
    : SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList )
    , font( _font )
{
}

void XMLFontStyleContextFontFaceUri::SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName,
    const OUString& rValue )
{
    if( nPrefixKey == XML_NAMESPACE_XLINK && IsXMLToken( rLocalName, XML_HREF ))
        handleEmbeddedFont( rValue );
    else
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}

void XMLFontStyleContextFontFaceUri::handleEmbeddedFont( const OUString& url )
{
    if( GetImport().embeddedFontAlreadyProcessed( url ))
    {
        GetImport().NotifyEmbeddedFontRead();
        return;
    }
    OUString fontName = font.familyName();
    // If there's any giveMeStreamForThisURL(), then it's well-hidden for me to find it.
    if( GetImport().IsPackageURL( url ))
    {
        uno::Reference< embed::XStorage > storage;
        storage.set( GetImport().GetSourceStorage(), UNO_QUERY_THROW );
        if( url.indexOf( '/' ) > -1 ) // TODO what if more levels?
            storage.set( storage->openStorageElement( url.copy( 0, url.indexOf( '/' )),
                ::embed::ElementModes::READ ), uno::UNO_QUERY_THROW );
        uno::Reference< io::XInputStream > inputStream;
        inputStream.set( storage->openStreamElement( url.copy( url.indexOf( '/' ) + 1 ), ::embed::ElementModes::READ ),
            UNO_QUERY_THROW );
        if( EmbeddedFontsHelper::addEmbeddedFont( inputStream, fontName, "?" ))
            GetImport().NotifyEmbeddedFontRead();
        inputStream->closeInput();
    }
    else
        SAL_WARN( "xmloff", "External URL for font file not handled." );
}

SvXMLStyleContext *XMLFontStylesContext::CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle;
    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_FONT_FACE ) )
    {
        pStyle = new XMLFontStyleContextFontFace( GetImport(), nPrefix,
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
    pFamilyNameHdl( new XMLFontFamilyNamePropHdl ),
    pFamilyHdl( new XMLFontFamilyPropHdl ),
    pPitchHdl( new XMLFontPitchPropHdl ),
    pEncHdl( new XMLFontEncodingPropHdl ),
    pFontStyleAttrTokenMap( new SvXMLTokenMap(lcl_getFontStyleAttrTokenMap()) ),
    eDfltEncoding( eDfltEnc )
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
    const SvXMLStyleContext* pStyle = FindStyleChildContext( XML_STYLE_FAMILY_FONT, rName, sal_True );
    const XMLFontStyleContextFontFace *pFontStyle = PTR_CAST( XMLFontStyleContextFontFace,pStyle);// use temp var, PTR_CAST is a bad macro, FindStyleChildContext will be called twice
    if( pFontStyle )
        pFontStyle->FillProperties( rProps, nFamilyNameIdx, nStyleNameIdx,
                                    nFamilyIdx, nPitchIdx, nCharsetIdx );
    return 0 != pFontStyle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
