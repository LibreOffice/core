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
#include <com/sun/star/embed/XStorage.hpp>

#include <comphelper/seqstream.hxx>

#include <sal/log.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "fonthdl.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;


#define XML_STYLE_FAMILY_FONT XmlStyleFamily::PAGE_MASTER

XMLFontStyleContextFontFace::XMLFontStyleContextFontFace( SvXMLImport& rImport,
        XMLFontStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, XML_STYLE_FAMILY_FONT ),
    xStyles( &rStyles )
{
    aFamilyName <<= OUString();
    aStyleName <<= OUString();
    aFamily <<= sal_Int16(awt::FontFamily::DONTKNOW);
    aPitch <<= sal_Int16(awt::FontPitch::DONTKNOW);
    aEnc <<= static_cast<sal_Int16>(rStyles.GetDfltCharset());
}

void XMLFontStyleContextFontFace::SetAttribute( sal_Int32 nElement,
                                        const OUString& rValue )
{
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();
    Any aAny;

    switch(nElement)
    {
    case XML_ELEMENT(SVG, XML_FONT_FAMILY):
    case XML_ELEMENT(SVG_COMPAT, XML_FONT_FAMILY):
        if( GetStyles()->GetFamilyNameHdl().importXML( rValue, aAny,
                                                          rUnitConv ) )
            aFamilyName = aAny;
        break;
    case XML_ELEMENT(STYLE, XML_FONT_ADORNMENTS):
        aStyleName <<= rValue;
        break;
    case XML_ELEMENT(STYLE, XML_FONT_FAMILY_GENERIC):
        if( GetStyles()->GetFamilyHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aFamily = aAny;
        break;
    case XML_ELEMENT(STYLE, XML_FONT_PITCH):
        if( GetStyles()->GetPitchHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aPitch = aAny;
        break;
    case XML_ELEMENT(STYLE, XML_FONT_CHARSET):
        if( GetStyles()->GetEncodingHdl().importXML( rValue, aAny,
                                                      rUnitConv ) )
            aEnc = aAny;
        break;
    default:
        SvXMLStyleContext::SetAttribute( nElement, rValue );
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

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFontStyleContextFontFace::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > &  )
{
    if( nElement == XML_ELEMENT(SVG, XML_FONT_FACE_SRC) ||
        nElement == XML_ELEMENT(SVG_COMPAT, XML_FONT_FACE_SRC) )
        return new XMLFontStyleContextFontFaceSrc( GetImport(), *this );
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

OUString XMLFontStyleContextFontFace::familyName() const
{
    OUString ret;
    aFamilyName >>= ret;
    return ret;
}


XMLFontStyleContextFontFaceFormat::XMLFontStyleContextFontFaceFormat( SvXMLImport& rImport,
        XMLFontStyleContextFontFaceUri& _uri )
    : SvXMLStyleContext( rImport )
    , uri(_uri)
{
}

void XMLFontStyleContextFontFaceFormat::SetAttribute( sal_Int32 nElement,
    const OUString& rValue )
{
    if( nElement == XML_ELEMENT(SVG, XML_STRING) || nElement == XML_ELEMENT(SVG_COMPAT, XML_STRING))
        uri.SetFormat(rValue);
    else
        SvXMLStyleContext::SetAttribute( nElement, rValue );
}


XMLFontStyleContextFontFaceSrc::XMLFontStyleContextFontFaceSrc( SvXMLImport& rImport,
        const XMLFontStyleContextFontFace& _font )
    : SvXMLImportContext( rImport )
    , font( _font )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFontStyleContextFontFaceSrc::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if( nElement == XML_ELEMENT(SVG, XML_FONT_FACE_URI) ||
        nElement == XML_ELEMENT(SVG_COMPAT, XML_FONT_FACE_URI) )
        return new XMLFontStyleContextFontFaceUri( GetImport(), font );
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}


XMLFontStyleContextFontFaceUri::XMLFontStyleContextFontFaceUri( SvXMLImport& rImport,
        const XMLFontStyleContextFontFace& _font )
    : SvXMLStyleContext( rImport )
    , font( _font )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFontStyleContextFontFaceUri::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if( nElement == XML_ELEMENT(SVG, XML_FONT_FACE_FORMAT) )
        return new XMLFontStyleContextFontFaceFormat( GetImport(), *this );
    else if( nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA) )
    {
        assert(linkPath.isEmpty());
        if( linkPath.isEmpty() )
        {
            mxBase64Stream.set( new comphelper::OSequenceOutputStream( maFontData ) );
            if( mxBase64Stream.is() )
                return new XMLBase64ImportContext( GetImport(), mxBase64Stream );
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void XMLFontStyleContextFontFaceUri::SetAttribute( sal_Int32 nElement,
    const OUString& rValue )
{
    if( nElement == XML_ELEMENT(XLINK, XML_HREF) )
        linkPath = rValue;
    else
        SvXMLStyleContext::SetAttribute( nElement, rValue );
}

void XMLFontStyleContextFontFaceUri::SetFormat( const OUString& rFormat )
{
    format = rFormat;
}

// the CSS2 standard ( http://www.w3.org/TR/2008/REC-CSS2-20080411/fonts.html#referencing )
// defines these format strings.
const char OPENTYPE_FORMAT[] = "opentype";
const char TRUETYPE_FORMAT[] = "truetype";
const char EOT_FORMAT[]      = "embedded-opentype";

void XMLFontStyleContextFontFaceUri::endFastElement(sal_Int32 )
{
    if( ( linkPath.getLength() == 0 ) && ( !maFontData.hasElements() ) )
    {
        SAL_WARN( "xmloff", "svg:font-face-uri tag with no link or base64 data; ignoring." );
        return;
    }
    bool eot;
    // Assume by default that the font is not compressed.
    if( format.getLength() == 0
        || format == OPENTYPE_FORMAT
        || format == TRUETYPE_FORMAT )
    {
        eot = false;
    }
    else if( format == EOT_FORMAT )
    {
        eot = true;
    }
    else
    {
        SAL_WARN( "xmloff", "Unknown format of embedded font; assuming TTF." );
        eot = false;
    }
    if ( !maFontData.hasElements() )
        handleEmbeddedFont( linkPath, eot );
    else
        handleEmbeddedFont( maFontData, eot );
}

void XMLFontStyleContextFontFaceUri::handleEmbeddedFont( const OUString& url, bool eot )
{
    if( GetImport().embeddedFontAlreadyProcessed( url ))
    {
        GetImport().NotifyContainsEmbeddedFont();
        return;
    }
    OUString fontName = font.familyName();
    // If there's any giveMeStreamForThisURL(), then it's well-hidden for me to find it.
    if( GetImport().IsPackageURL( url ))
    {
        uno::Reference< embed::XStorage > storage;
        storage.set( GetImport().GetSourceStorage(), UNO_SET_THROW );
        if( url.indexOf( '/' ) > -1 ) // TODO what if more levels?
            storage.set( storage->openStorageElement( url.copy( 0, url.indexOf( '/' )),
                ::embed::ElementModes::READ ), uno::UNO_SET_THROW );
        uno::Reference< io::XInputStream > inputStream;
        inputStream.set( storage->openStreamElement( url.copy( url.indexOf( '/' ) + 1 ), ::embed::ElementModes::READ ),
            UNO_QUERY_THROW );
        if (GetImport().addEmbeddedFont(inputStream, fontName, u"?", std::vector< unsigned char >(), eot))
            GetImport().NotifyContainsEmbeddedFont();
        inputStream->closeInput();
    }
    else
        SAL_WARN( "xmloff", "External URL for font file not handled." );
}

void XMLFontStyleContextFontFaceUri::handleEmbeddedFont( const ::css::uno::Sequence< sal_Int8 >& rData, const bool eot )
{
    const uno::Reference< io::XInputStream > xInput( new comphelper::SequenceInputStream( rData ) );
    const OUString fontName = font.familyName();
    if (GetImport().addEmbeddedFont(xInput, fontName, u"?", std::vector< unsigned char >(), eot))
        GetImport().NotifyContainsEmbeddedFont();
    xInput->closeInput();
}

SvXMLStyleContext *XMLFontStylesContext::CreateStyleChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_FONT_FACE) )
    {
        return new XMLFontStyleContextFontFace( GetImport(), *this );
    }
    return SvXMLStylesContext::CreateStyleChildContext( nElement, xAttrList );
}


XMLFontStylesContext::XMLFontStylesContext( SvXMLImport& rImport,
        rtl_TextEncoding eDfltEnc ) :
    SvXMLStylesContext( rImport ),
    m_pFamilyNameHdl( new XMLFontFamilyNamePropHdl ),
    m_pFamilyHdl( new XMLFontFamilyPropHdl ),
    m_pPitchHdl( new XMLFontPitchPropHdl ),
    m_pEncHdl( new XMLFontEncodingPropHdl ),
    m_eDefaultEncoding( eDfltEnc )
{
}

XMLFontStylesContext::~XMLFontStylesContext() {}

bool XMLFontStylesContext::FillProperties( const OUString& rName,
                         ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const
{
    const SvXMLStyleContext* pStyle = FindStyleChildContext( XML_STYLE_FAMILY_FONT, rName, true );
    const XMLFontStyleContextFontFace *pFontStyle = dynamic_cast<const XMLFontStyleContextFontFace*>(pStyle);// use temp var, PTR_CAST is a bad macro, FindStyleChildContext will be called twice
    if( pFontStyle )
        pFontStyle->FillProperties( rProps, nFamilyNameIdx, nStyleNameIdx,
                                    nFamilyIdx, nPitchIdx, nCharsetIdx );
    return nullptr != pFontStyle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
