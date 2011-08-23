/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::oox::core::ContextHandler;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

// ============================================================================

SolidFillContext::SolidFillContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >&, FillProperties& rFillProps ) :
    ColorContext( rParent, rFillProps.maFillColor )
{
}

// ============================================================================

GradientFillContext::GradientFillContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& rxAttribs, GradientFillProperties& rGradientProps ) :
    ContextHandler( rParent ),
    mrGradientProps( rGradientProps )
{
    AttributeList aAttribs( rxAttribs );
    mrGradientProps.moShadeFlip = aAttribs.getToken( XML_flip );
    mrGradientProps.moRotateWithShape = aAttribs.getBool( XML_rotWithShape );
}

Reference< XFastContextHandler > GradientFillContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case A_TOKEN( gsLst ):
            return this;    // for gs elements

        case A_TOKEN( gs ):
            if( aAttribs.hasAttribute( XML_pos ) )
            {
                double fPosition = getLimitedValue< double >( aAttribs.getDouble( XML_pos, 0.0 ) / 100000.0, 0.0, 1.0 );
                return new ColorContext( *this, mrGradientProps.maGradientStops[ fPosition ] );
            }
        break;

        case A_TOKEN( lin ):
            mrGradientProps.moShadeAngle = aAttribs.getInteger( XML_ang );
            mrGradientProps.moShadeScaled = aAttribs.getBool( XML_scaled );
        break;

        case A_TOKEN( path ):
            // always set a path type, this disables linear gradient in conversion
            mrGradientProps.moGradientPath = aAttribs.getToken( XML_path, XML_rect );
            return this;    // for fillToRect element

        case A_TOKEN( fillToRect ):
            mrGradientProps.moFillToRect = GetRelativeRect( rxAttribs );
        break;

        case A_TOKEN( tileRect ):
            mrGradientProps.moTileRect = GetRelativeRect( rxAttribs );
        break;
    }
    return 0;
}

// ============================================================================

PatternFillContext::PatternFillContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& rxAttribs, PatternFillProperties& rPatternProps ) :
    ContextHandler( rParent ),
    mrPatternProps( rPatternProps )
{
    AttributeList aAttribs( rxAttribs );
    mrPatternProps.moPattPreset = aAttribs.getToken( XML_prst );
}

Reference< XFastContextHandler > PatternFillContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case A_TOKEN( bgClr ):
            return new ColorContext( *this, mrPatternProps.maPattBgColor );
        case A_TOKEN( fgClr ):
            return new ColorContext( *this, mrPatternProps.maPattFgColor );
    }
    return 0;
}

// ============================================================================
// ============================================================================

ColorChangeContext::ColorChangeContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& rxAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler( rParent ),
    mrBlipProps( rBlipProps )
{
    mrBlipProps.maColorChangeFrom.setUnused();
    mrBlipProps.maColorChangeTo.setUnused();
    AttributeList aAttribs( rxAttribs );
    mbUseAlpha = aAttribs.getBool( XML_useA, true );
}

ColorChangeContext::~ColorChangeContext()
{
    if( !mbUseAlpha )
        mrBlipProps.maColorChangeTo.clearTransparence();
}

Reference< XFastContextHandler > ColorChangeContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case A_TOKEN( clrFrom ):
            return new ColorContext( *this, mrBlipProps.maColorChangeFrom );
        case A_TOKEN( clrTo ):
            return new ColorContext( *this, mrBlipProps.maColorChangeTo );
    }
    return 0;
}

// ============================================================================

BlipContext::BlipContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& rxAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler( rParent ),
    mrBlipProps( rBlipProps )
{
    AttributeList aAttribs( rxAttribs );
    if( aAttribs.hasAttribute( R_TOKEN( embed ) ) )
    {
        // internal picture URL
        OUString aFragmentPath = getFragmentPathFromRelId( aAttribs.getString( R_TOKEN( embed ), OUString() ) );
        if( aFragmentPath.getLength() > 0 )
            mrBlipProps.mxGraphic = getFilter().getGraphicHelper().importEmbeddedGraphic( aFragmentPath );
    }
    else if( aAttribs.hasAttribute( R_TOKEN( link ) ) )
    {
        // external URL
        OUString aRelId = aAttribs.getString( R_TOKEN( link ), OUString() );
        OUString aTargetLink = getFilter().getAbsoluteUrl( getRelations().getExternalTargetFromRelId( aRelId ) );
        // TODO: load external picture
    }
}

Reference< XFastContextHandler > BlipContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case A_TOKEN( biLevel ):
        case A_TOKEN( grayscl ):
            mrBlipProps.moColorEffect = getToken( nElement );
        break;

        case A_TOKEN( clrChange ):
            return new ColorChangeContext( *this, rxAttribs, mrBlipProps );

        case A_TOKEN( lum ):
            mrBlipProps.moBrightness = aAttribs.getInteger( XML_bright );
            mrBlipProps.moContrast = aAttribs.getInteger( XML_contrast );
        break;
    }
    return 0;
}

// ============================================================================

BlipFillContext::BlipFillContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& rxAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler( rParent ),
    mrBlipProps( rBlipProps )
{
    AttributeList aAttribs( rxAttribs );
    mrBlipProps.moRotateWithShape = aAttribs.getBool( XML_rotWithShape );
}

Reference< XFastContextHandler > BlipFillContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case A_TOKEN( blip ):
            return new BlipContext( *this, rxAttribs, mrBlipProps );

        case A_TOKEN( srcRect ):
            // TODO
        break;

        case A_TOKEN( tile ):
            mrBlipProps.moBitmapMode = getToken( nElement );
            mrBlipProps.moTileOffsetX = aAttribs.getInteger( XML_tx );
            mrBlipProps.moTileOffsetY = aAttribs.getInteger( XML_ty );
            mrBlipProps.moTileScaleX = aAttribs.getInteger( XML_sx );
            mrBlipProps.moTileScaleY = aAttribs.getInteger( XML_sy );
            mrBlipProps.moTileAlign = aAttribs.getToken( XML_algn );
            mrBlipProps.moTileFlip = aAttribs.getToken( XML_flip );
        break;

        case A_TOKEN( stretch ):
            mrBlipProps.moBitmapMode = getToken( nElement );
            return this;    // for fillRect element

        case A_TOKEN( fillRect ):
            mrBlipProps.moFillRect = GetRelativeRect( rxAttribs );
        break;
    }
    return 0;
}

// ============================================================================
// ============================================================================

FillPropertiesContext::FillPropertiesContext( ContextHandler& rParent, FillProperties& rFillProps ) :
    ContextHandler( rParent ),
    mrFillProps( rFillProps )
{
}

Reference< XFastContextHandler > FillPropertiesContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
    throw ( SAXException, RuntimeException )
{
    return createFillContext( *this, nElement, rxAttribs, mrFillProps );
}

/*static*/ Reference< XFastContextHandler > FillPropertiesContext::createFillContext(
        ContextHandler& rParent, sal_Int32 nElement,
        const Reference< XFastAttributeList >& rxAttribs, FillProperties& rFillProps )
{
    switch( nElement )
    {
        case A_TOKEN( noFill ):     { rFillProps.moFillType = getToken( nElement ); return 0; };
        case A_TOKEN( solidFill ):  { rFillProps.moFillType = getToken( nElement ); return new SolidFillContext( rParent, rxAttribs, rFillProps ); };
        case A_TOKEN( gradFill ):   { rFillProps.moFillType = getToken( nElement ); return new GradientFillContext( rParent, rxAttribs, rFillProps.maGradientProps ); };
        case A_TOKEN( pattFill ):   { rFillProps.moFillType = getToken( nElement ); return new PatternFillContext( rParent, rxAttribs, rFillProps.maPatternProps ); };
        case A_TOKEN( blipFill ):   { rFillProps.moFillType = getToken( nElement ); return new BlipFillContext( rParent, rxAttribs, rFillProps.maBlipProps ); };
        case A_TOKEN( grpFill ):    { rFillProps.moFillType = getToken( nElement ); return 0; };    // TODO
    }
    return 0;
}

// ============================================================================

SimpleFillPropertiesContext::SimpleFillPropertiesContext( ContextHandler& rParent, Color& rColor ) :
    FillPropertiesContext( rParent, *this ),
    mrColor( rColor )
{
}

SimpleFillPropertiesContext::~SimpleFillPropertiesContext()
{
    mrColor = getBestSolidColor();
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
