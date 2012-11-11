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

#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
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
        if( !aFragmentPath.isEmpty() )
            mrBlipProps.mxGraphic = getFilter().getGraphicHelper().importEmbeddedGraphic( aFragmentPath );
    }
    else if( aAttribs.hasAttribute( R_TOKEN( link ) ) )
    {
        // external URL
        // OUString aRelId = aAttribs.getString( R_TOKEN( link ), OUString() );
        // OUString aTargetLink = getFilter().getAbsoluteUrl( getRelations().getExternalTargetFromRelId( aRelId ) );
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
            mrBlipProps.moColorEffect = getBaseToken( nElement );
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
            {
                rtl::OUString aDefault( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "0" ) ) );
                ::com::sun::star::geometry::IntegerRectangle2D aClipRect;
                aClipRect.X1 = GetPercent( aAttribs.getString( XML_l, aDefault ) );
                aClipRect.Y1 = GetPercent( aAttribs.getString( XML_t, aDefault ) );
                aClipRect.X2 = GetPercent( aAttribs.getString( XML_r, aDefault ) );
                aClipRect.Y2 = GetPercent( aAttribs.getString( XML_b, aDefault ) );
                mrBlipProps.moClipRect = aClipRect;
            }
        break;

        case A_TOKEN( tile ):
            mrBlipProps.moBitmapMode = getBaseToken( nElement );
            mrBlipProps.moTileOffsetX = aAttribs.getInteger( XML_tx );
            mrBlipProps.moTileOffsetY = aAttribs.getInteger( XML_ty );
            mrBlipProps.moTileScaleX = aAttribs.getInteger( XML_sx );
            mrBlipProps.moTileScaleY = aAttribs.getInteger( XML_sy );
            mrBlipProps.moTileAlign = aAttribs.getToken( XML_algn );
            mrBlipProps.moTileFlip = aAttribs.getToken( XML_flip );
        break;

        case A_TOKEN( stretch ):
            mrBlipProps.moBitmapMode = getBaseToken( nElement );
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
        case A_TOKEN( noFill ):     { rFillProps.moFillType = getBaseToken( nElement ); return 0; };
        case A_TOKEN( solidFill ):  { rFillProps.moFillType = getBaseToken( nElement ); return new SolidFillContext( rParent, rxAttribs, rFillProps ); };
        case A_TOKEN( gradFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new GradientFillContext( rParent, rxAttribs, rFillProps.maGradientProps ); };
        case A_TOKEN( pattFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new PatternFillContext( rParent, rxAttribs, rFillProps.maPatternProps ); };
        case A_TOKEN( blipFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new BlipFillContext( rParent, rxAttribs, rFillProps.maBlipProps ); };
        case A_TOKEN( grpFill ):    { rFillProps.moFillType = getBaseToken( nElement ); return 0; };    // TODO
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
