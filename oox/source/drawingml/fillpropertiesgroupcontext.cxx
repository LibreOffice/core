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

#include "drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sfx2/docfile.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace drawingml {

SolidFillContext::SolidFillContext( ContextHandler2Helper& rParent,
        const AttributeList&, FillProperties& rFillProps ) :
    ColorContext( rParent, rFillProps.maFillColor )
{
}

GradientFillContext::GradientFillContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, GradientFillProperties& rGradientProps ) :
    ContextHandler2( rParent ),
    mrGradientProps( rGradientProps )
{
    mrGradientProps.moShadeFlip = rAttribs.getToken( XML_flip );
    mrGradientProps.moRotateWithShape = rAttribs.getBool( XML_rotWithShape );
}

ContextHandlerRef GradientFillContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( gsLst ):
            return this;    // for gs elements

        case A_TOKEN( gs ):
            if( rAttribs.hasAttribute( XML_pos ) )
            {
                double fPosition = getLimitedValue< double >( rAttribs.getDouble( XML_pos, 0.0 ) / 100000.0, 0.0, 1.0 );
                return new ColorContext( *this, mrGradientProps.maGradientStops[ fPosition ] );
            }
        break;

        case A_TOKEN( lin ):
            mrGradientProps.moShadeAngle = rAttribs.getInteger( XML_ang );
            mrGradientProps.moShadeScaled = rAttribs.getBool( XML_scaled );
        break;

        case A_TOKEN( path ):
            // always set a path type, this disables linear gradient in conversion
            mrGradientProps.moGradientPath = rAttribs.getToken( XML_path, XML_rect );
            return this;    // for fillToRect element

        case A_TOKEN( fillToRect ):
            mrGradientProps.moFillToRect = GetRelativeRect( rAttribs.getFastAttributeList() );
        break;

        case A_TOKEN( tileRect ):
            mrGradientProps.moTileRect = GetRelativeRect( rAttribs.getFastAttributeList() );
        break;
    }
    return nullptr;
}

PatternFillContext::PatternFillContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, PatternFillProperties& rPatternProps ) :
    ContextHandler2( rParent ),
    mrPatternProps( rPatternProps )
{
    mrPatternProps.moPattPreset = rAttribs.getToken( XML_prst );
}

ContextHandlerRef PatternFillContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( bgClr ):
            return new ColorContext( *this, mrPatternProps.maPattBgColor );
        case A_TOKEN( fgClr ):
            return new ColorContext( *this, mrPatternProps.maPattFgColor );
    }
    return nullptr;
}

ColorChangeContext::ColorChangeContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps )
{
    mrBlipProps.maColorChangeFrom.setUnused();
    mrBlipProps.maColorChangeTo.setUnused();
    mbUseAlpha = rAttribs.getBool( XML_useA, true );
}

ColorChangeContext::~ColorChangeContext()
{
    if( !mbUseAlpha )
        mrBlipProps.maColorChangeTo.clearTransparence();
}

ContextHandlerRef ColorChangeContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( clrFrom ):
            return new ColorContext( *this, mrBlipProps.maColorChangeFrom );
        case A_TOKEN( clrTo ):
            return new ColorContext( *this, mrBlipProps.maColorChangeTo );
    }
    return nullptr;
}

BlipContext::BlipContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps )
{
    if( rAttribs.hasAttribute( R_TOKEN( embed ) ) )
    {
        // internal picture URL
        OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( embed ), OUString() ) );
        if( !aFragmentPath.isEmpty() )
            mrBlipProps.mxGraphic = getFilter().getGraphicHelper().importEmbeddedGraphic( aFragmentPath );
    }
    else if( rAttribs.hasAttribute( R_TOKEN( link ) ) )
    {
        // external URL

        // we will embed this link, this is better than just doing nothing..
        // TODO: import this graphic as real link, but this requires some
        // code rework.
        OUString aRelId = rAttribs.getString( R_TOKEN( link ), OUString() );
        OUString aTargetLink = getFilter().getAbsoluteUrl( getRelations().getExternalTargetFromRelId( aRelId ) );
        SfxMedium aMed( aTargetLink, STREAM_STD_READ );
        aMed.Download();
        Reference< io::XInputStream > xInStrm = aMed.GetInputStream();
        if ( xInStrm.is() )
            mrBlipProps.mxGraphic = getFilter().getGraphicHelper().importGraphic( xInStrm );
    }
}

ContextHandlerRef BlipContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( biLevel ):
        case A_TOKEN( grayscl ):
            mrBlipProps.moColorEffect = getBaseToken( nElement );
        break;

        case A_TOKEN( clrChange ):
            return new ColorChangeContext( *this, rAttribs, mrBlipProps );

        case A_TOKEN( duotone ):
            return new DuotoneContext( *this, rAttribs, mrBlipProps );

        case A_TOKEN( extLst ):
            return new BlipExtensionContext( *this, mrBlipProps );

        case A_TOKEN( lum ):
            mrBlipProps.moBrightness = rAttribs.getInteger( XML_bright );
            mrBlipProps.moContrast = rAttribs.getInteger( XML_contrast );
        break;
    }
    return nullptr;
}

DuotoneContext::DuotoneContext( ContextHandler2Helper& rParent,
        const AttributeList& /*rAttribs*/, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps ),
    mnColorIndex( 0 )
{
    mrBlipProps.maDuotoneColors[0].setUnused();
    mrBlipProps.maDuotoneColors[1].setUnused();
}

DuotoneContext::~DuotoneContext()
{
}

::oox::core::ContextHandlerRef DuotoneContext::onCreateContext(
        sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
    if( mnColorIndex < 2 )
        return new ColorValueContext( *this, mrBlipProps.maDuotoneColors[mnColorIndex++] );
    return nullptr;
}

BlipFillContext::BlipFillContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps )
{
    mrBlipProps.moRotateWithShape = rAttribs.getBool( XML_rotWithShape );
}

ContextHandlerRef BlipFillContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( blip ):
            return new BlipContext( *this, rAttribs, mrBlipProps );

        case A_TOKEN( srcRect ):
            mrBlipProps.moClipRect = GetRelativeRect( rAttribs.getFastAttributeList() );
        break;

        case A_TOKEN( tile ):
            mrBlipProps.moBitmapMode = getBaseToken( nElement );
            mrBlipProps.moTileOffsetX = rAttribs.getInteger( XML_tx );
            mrBlipProps.moTileOffsetY = rAttribs.getInteger( XML_ty );
            mrBlipProps.moTileScaleX = rAttribs.getInteger( XML_sx );
            mrBlipProps.moTileScaleY = rAttribs.getInteger( XML_sy );
            mrBlipProps.moTileAlign = rAttribs.getToken( XML_algn );
            mrBlipProps.moTileFlip = rAttribs.getToken( XML_flip );
        break;

        case A_TOKEN( stretch ):
            mrBlipProps.moBitmapMode = getBaseToken( nElement );
            return this;    // for fillRect element

        case A_TOKEN( fillRect ):
            mrBlipProps.moFillRect = GetRelativeRect( rAttribs.getFastAttributeList() );
        break;
    }
    return nullptr;
}

FillPropertiesContext::FillPropertiesContext( ContextHandler2Helper& rParent, FillProperties& rFillProps ) :
    ContextHandler2( rParent ),
    mrFillProps( rFillProps )
{
}

ContextHandlerRef FillPropertiesContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    return createFillContext( *this, nElement, rAttribs, mrFillProps );
}

ContextHandlerRef FillPropertiesContext::createFillContext(
        ContextHandler2Helper& rParent, sal_Int32 nElement,
        const AttributeList& rAttribs, FillProperties& rFillProps )
{
    switch( nElement )
    {
        case A_TOKEN( noFill ):     { rFillProps.moFillType = getBaseToken( nElement ); return nullptr; };
        case A_TOKEN( solidFill ):  { rFillProps.moFillType = getBaseToken( nElement ); return new SolidFillContext( rParent, rAttribs, rFillProps ); };
        case A_TOKEN( gradFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new GradientFillContext( rParent, rAttribs, rFillProps.maGradientProps ); };
        case A_TOKEN( pattFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new PatternFillContext( rParent, rAttribs, rFillProps.maPatternProps ); };
        case A_TOKEN( blipFill ):   { rFillProps.moFillType = getBaseToken( nElement ); return new BlipFillContext( rParent, rAttribs, rFillProps.maBlipProps ); };
        case A_TOKEN( grpFill ):    { rFillProps.moFillType = getBaseToken( nElement ); return nullptr; };    // TODO
    }
    return nullptr;
}

SimpleFillPropertiesContext::SimpleFillPropertiesContext( ContextHandler2Helper& rParent, Color& rColor ) :
    FillPropertiesContext( rParent, *this ),
    mrColor( rColor )
{
}

SimpleFillPropertiesContext::~SimpleFillPropertiesContext()
{
    mrColor = getBestSolidColor();
}

BlipExtensionContext::BlipExtensionContext( ContextHandler2Helper& rParent, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps )
{
}

BlipExtensionContext::~BlipExtensionContext()
{
}

ContextHandlerRef BlipExtensionContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( ext ):
            return new BlipExtensionContext( *this, mrBlipProps );

        case OOX_TOKEN( a14, imgProps ):
            return new ArtisticEffectContext( *this, mrBlipProps.maEffect );
    }
    return nullptr;
}

ArtisticEffectContext::ArtisticEffectContext( ContextHandler2Helper& rParent, ArtisticEffectProperties& rEffect ) :
    ContextHandler2( rParent ),
    maEffect( rEffect )
{
}

ArtisticEffectContext::~ArtisticEffectContext()
{
}

ContextHandlerRef ArtisticEffectContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    // containers
    if( nElement == OOX_TOKEN( a14, imgLayer ) )
    {
        if( rAttribs.hasAttribute( R_TOKEN( embed ) ) )
        {
            OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( embed ), OUString() ) );
            if( !aFragmentPath.isEmpty() )
            {
                getFilter().importBinaryData( maEffect.mrOleObjectInfo.maEmbeddedData, aFragmentPath );
                maEffect.mrOleObjectInfo.maProgId = aFragmentPath;
            }
        }
        return new ArtisticEffectContext( *this, maEffect );
    }
    if( nElement == OOX_TOKEN( a14, imgEffect ) )
        return new ArtisticEffectContext( *this, maEffect );

    // effects
    maEffect.msName = ArtisticEffectProperties::getEffectString( nElement );
    if( maEffect.isEmpty() )
        return nullptr;

    // effect attributes
    sal_Int32 aAttribs[19] = {
            XML_visible, XML_trans, XML_crackSpacing, XML_pressure, XML_numberOfShades,
            XML_grainSize, XML_intensity, XML_smoothness, XML_gridSize, XML_pencilSize,
            XML_size, XML_brushSize, XML_scaling, XML_detail, XML_bright, XML_contrast,
            XML_colorTemp, XML_sat, XML_amount
    };
    for(sal_Int32 nAttrib : aAttribs)
    {
        if( rAttribs.hasAttribute( nAttrib ) )
        {
            OUString sName = ArtisticEffectProperties::getEffectString( nAttrib );
            if( !sName.isEmpty() )
                maEffect.maAttribs[sName] = uno::makeAny( rAttribs.getInteger( nAttrib, 0 ) );
        }
    }

    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
