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

#include <drawingml/misccontexts.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <vcl/GraphicExternalLink.hxx>
#include <vcl/graph.hxx>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandlerRef;

namespace oox::drawingml {

SolidFillContext::SolidFillContext(ContextHandler2Helper const & rParent, FillProperties& rFillProps, model::SolidFill* pSolidFill)
    : ColorContext(rParent, rFillProps.maFillColor, pSolidFill ? &pSolidFill->maColorDefinition : nullptr)
{
}

SolidFillContext::~SolidFillContext()
{}

GradientFillContext::GradientFillContext(ContextHandler2Helper const & rParent,
        const AttributeList& rAttribs, GradientFillProperties& rGradientProps, model::GradientFill* pGradientFill)
    : ContextHandler2(rParent)
    , mpGradientFill(pGradientFill)
    , mrGradientProps(rGradientProps)
{
    auto oRotateWithShape = rAttribs.getBool(XML_rotWithShape);
    mrGradientProps.moShadeFlip = rAttribs.getToken( XML_flip );
    mrGradientProps.moRotateWithShape = oRotateWithShape;
    if (mpGradientFill && oRotateWithShape)
        mpGradientFill->mbRotateWithShape = *oRotateWithShape;
}

ContextHandlerRef GradientFillContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( gsLst ):
            return this;    // for gs elements

        case A_TOKEN( gs ):
            if (rAttribs.hasAttribute(XML_pos))
            {
                double fPosition = getLimitedValue<double>(rAttribs.getDouble(XML_pos, 0.0) / 100000.0, 0.0, 1.0);
                auto aElement = mrGradientProps.maGradientStops.emplace(fPosition, Color());

                model::ColorDefinition* pColorDefinition = nullptr;
                if (mpGradientFill)
                {
                    model::GradientStop& rStop = mpGradientFill->maGradientStops.emplace_back();
                    rStop.mfPosition = fPosition;
                    pColorDefinition = &rStop.maColor;
                }

                return new ColorContext(*this, aElement->second, pColorDefinition);
            }
        break;

        case A_TOKEN( lin ):
        {
            mrGradientProps.moShadeAngle = rAttribs.getInteger(XML_ang);
            mrGradientProps.moShadeScaled = rAttribs.getBool(XML_scaled);

            if (mpGradientFill)
            {
                mpGradientFill->meGradientType = model::GradientType::Linear;
                mpGradientFill->maLinearGradient.mnAngle = rAttribs.getInteger(XML_ang, 0);
                mpGradientFill->maLinearGradient.mbScaled = rAttribs.getBool(XML_scaled, false);
            }
        }
        break;

        case A_TOKEN( path ):
        {
            // always set a path type, this disables linear gradient in conversion
            sal_Int32 nToken = rAttribs.getToken(XML_path, XML_rect);
            mrGradientProps.moGradientPath = nToken;
            if (mpGradientFill)
            {
                switch (nToken)
                {
                    case XML_rect:
                        mpGradientFill->meGradientType = model::GradientType::Rectangle;
                        break;
                    case XML_circle:
                        mpGradientFill->meGradientType = model::GradientType::Circle;
                        break;
                    case XML_shape:
                        mpGradientFill->meGradientType = model::GradientType::Shape;
                        break;
                    default:
                        break;
                }
            }
            return this;    // for fillToRect element
        }
        case A_TOKEN( fillToRect ):
        {
            mrGradientProps.moFillToRect = GetRelativeRect( rAttribs.getFastAttributeList() );
            if (mpGradientFill)
                fillRelativeRectangle(mpGradientFill->maFillToRectangle, rAttribs.getFastAttributeList());
        }
        break;

        case A_TOKEN( tileRect ):
            mrGradientProps.moTileRect = GetRelativeRect(rAttribs.getFastAttributeList());
            if (mpGradientFill)
                fillRelativeRectangle(mpGradientFill->maTileRectangle, rAttribs.getFastAttributeList());
        break;
    }
    return nullptr;
}


namespace
{

std::unordered_map<sal_Int32, model::PatternPreset> constPatternPresetMap =
{
    { XML_pct5, model::PatternPreset::Percent_5 },
    { XML_pct10, model::PatternPreset::Percent_10 },
    { XML_pct20, model::PatternPreset::Percent_20 },
    { XML_pct25, model::PatternPreset::Percent_25 },
    { XML_pct30, model::PatternPreset::Percent_30 },
    { XML_pct40, model::PatternPreset::Percent_40 },
    { XML_pct50, model::PatternPreset::Percent_50 },
    { XML_pct60, model::PatternPreset::Percent_60 },
    { XML_pct70, model::PatternPreset::Percent_70 },
    { XML_pct75, model::PatternPreset::Percent_75 },
    { XML_pct80, model::PatternPreset::Percent_80 },
    { XML_pct90, model::PatternPreset::Percent_90 },
    { XML_horz, model::PatternPreset::Horizontal },
    { XML_vert, model::PatternPreset::Vertical },
    { XML_ltHorz, model::PatternPreset::LightHorizontal },
    { XML_ltVert, model::PatternPreset::LightVertical },
    { XML_dkHorz, model::PatternPreset::DarkHorizontal },
    { XML_dkVert, model::PatternPreset::DarkVertical },
    { XML_narHorz, model::PatternPreset::NarrowHorizontal },
    { XML_narVert, model::PatternPreset::NarrowVertical },
    { XML_dashHorz, model::PatternPreset::DashedHorizontal },
    { XML_dashVert, model::PatternPreset::DashedVertical },
    { XML_cross, model::PatternPreset::Cross },
    { XML_dnDiag, model::PatternPreset::DownwardDiagonal },
    { XML_upDiag, model::PatternPreset::UpwardDiagonal },
    { XML_ltDnDiag, model::PatternPreset::LightDownwardDiagonal },
    { XML_ltUpDiag, model::PatternPreset::LightUpwardDiagonal },
    { XML_dkDnDiag, model::PatternPreset::DarkDownwardDiagonal },
    { XML_dkUpDiag, model::PatternPreset::DarkUpwardDiagonal },
    { XML_wdDnDiag, model::PatternPreset::WideDownwardDiagonal },
    { XML_wdUpDiag, model::PatternPreset::WideUpwardDiagonal },
    { XML_dashDnDiag, model::PatternPreset::DashedDownwardDiagonal },
    { XML_dashUpDiag, model::PatternPreset::DashedUpwardDiagonal },
    { XML_diagCross, model::PatternPreset::DiagonalCross },
    { XML_smCheck, model::PatternPreset::SmallCheckerBoard },
    { XML_lgCheck, model::PatternPreset::LargeCheckerBoard },
    { XML_smGrid, model::PatternPreset::SmallGrid },
    { XML_lgGrid, model::PatternPreset::LargeGrid },
    { XML_dotGrid, model::PatternPreset::DottedGrid },
    { XML_smConfetti, model::PatternPreset::SmallConfetti },
    { XML_lgConfetti, model::PatternPreset::LargeConfetti },
    { XML_horzBrick, model::PatternPreset::HorizontalBrick },
    { XML_diagBrick, model::PatternPreset::DiagonalBrick },
    { XML_solidDmnd, model::PatternPreset::SolidDiamond },
    { XML_openDmnd, model::PatternPreset::OpenDiamond },
    { XML_dotDmnd, model::PatternPreset::DottedDiamond },
    { XML_plaid, model::PatternPreset::Plaid },
    { XML_sphere, model::PatternPreset::Sphere },
    { XML_weave, model::PatternPreset::Weave },
    { XML_divot, model::PatternPreset::Divot },
    { XML_shingle, model::PatternPreset::Shingle },
    { XML_wave, model::PatternPreset::Wave },
    { XML_trellis, model::PatternPreset::Trellis },
    { XML_zigZag, model::PatternPreset::ZigZag }
};

} // end anonymous namespace
PatternFillContext::PatternFillContext(ContextHandler2Helper const & rParent,
        const AttributeList& rAttribs, PatternFillProperties& rPatternProps, model::PatternFill* pPatternFill)
    : ContextHandler2(rParent)
    , mpPatternFill(pPatternFill)
    , mrPatternProps(rPatternProps)
{
    mrPatternProps.moPattPreset = rAttribs.getToken(XML_prst);

    if (mpPatternFill)
    {
        sal_Int32 nToken = rAttribs.getToken(XML_prst, XML_TOKEN_INVALID);

        auto aIterator = constPatternPresetMap.find(nToken);
        if (aIterator != constPatternPresetMap.end())
        {
            auto const& aPair = *aIterator;
            model::PatternPreset ePatternPreset = aPair.second;
            mpPatternFill->mePatternPreset = ePatternPreset;
        }
    }
}

ContextHandlerRef PatternFillContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    model::ColorDefinition* pColorDefinition = nullptr;
    switch( nElement )
    {
        case A_TOKEN( bgClr ):
            if (mpPatternFill)
                pColorDefinition = &mpPatternFill->maBackgroundColor;
            return new ColorContext(*this, mrPatternProps.maPattBgColor, pColorDefinition);
        case A_TOKEN( fgClr ):
            if (mpPatternFill)
                pColorDefinition = &mpPatternFill->maForegroundColor;
            return new ColorContext(*this, mrPatternProps.maPattFgColor, pColorDefinition);
    }
    return nullptr;
}

ColorChangeContext::ColorChangeContext( ContextHandler2Helper const & rParent,
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
            return new ColorContext(*this, mrBlipProps.maColorChangeFrom, nullptr);
        case A_TOKEN( clrTo ):
            return new ColorContext(*this, mrBlipProps.maColorChangeTo, nullptr);
    }
    return nullptr;
}

BlipContext::BlipContext( ContextHandler2Helper const & rParent,
        const AttributeList& rAttribs, BlipFillProperties& rBlipProps ) :
    ContextHandler2( rParent ),
    mrBlipProps( rBlipProps )
{
    if( rAttribs.hasAttribute( R_TOKEN( embed ) ) )
    {
        // internal picture URL
        OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( embed )) );
        if (!aFragmentPath.isEmpty())
            mrBlipProps.mxFillGraphic = getFilter().getGraphicHelper().importEmbeddedGraphic( aFragmentPath );
    }
    else if( rAttribs.hasAttribute( R_TOKEN( link ) ) )
    {
        // external URL

        // we will embed this link, this is better than just doing nothing...
        // TODO: import this graphic as real link, but this requires some
        // code rework.
        OUString aRelId = rAttribs.getStringDefaulted( R_TOKEN( link ));
        OUString aTargetLink = getFilter().getAbsoluteUrl( getRelations().getExternalTargetFromRelId( aRelId ) );
        GraphicExternalLink aLink(aTargetLink);
        Graphic aGraphic(aLink);
        mrBlipProps.mxFillGraphic = aGraphic.GetXGraphic();
    }
}

ContextHandlerRef BlipContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( biLevel ):
            mrBlipProps.moBiLevelThreshold = rAttribs.getInteger( XML_thresh );
            mrBlipProps.moColorEffect = getBaseToken(nElement);
            break;

        case A_TOKEN( grayscl ):
            mrBlipProps.moColorEffect = getBaseToken( nElement );
        break;

        case A_TOKEN( clrChange ):
            return new ColorChangeContext( *this, rAttribs, mrBlipProps );

        case A_TOKEN( duotone ):
            return new DuotoneContext( *this, mrBlipProps );

        case A_TOKEN( extLst ):
            return new BlipExtensionContext( *this, mrBlipProps );

        case A_TOKEN( lum ):
            mrBlipProps.moBrightness = rAttribs.getInteger( XML_bright );
            mrBlipProps.moContrast = rAttribs.getInteger( XML_contrast );
        break;
        case A_TOKEN( alphaModFix ):
            mrBlipProps.moAlphaModFix = rAttribs.getInteger(XML_amt);
        break;
    }
    return nullptr;
}

DuotoneContext::DuotoneContext( ContextHandler2Helper const & rParent,
        BlipFillProperties& rBlipProps ) :
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
        return new ColorValueContext(*this, mrBlipProps.maDuotoneColors[mnColorIndex++], nullptr);
    return nullptr;
}

BlipFillContext::BlipFillContext( ContextHandler2Helper const & rParent,
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

FillPropertiesContext::FillPropertiesContext( ContextHandler2Helper const & rParent, FillProperties& rFillProps ) :
    ContextHandler2( rParent ),
    mrFillProps( rFillProps )
{
}

ContextHandlerRef FillPropertiesContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    return createFillContext(*this, nElement, rAttribs, mrFillProps, &maFillStyle);
}

ContextHandlerRef FillPropertiesContext::createFillContext(
        ContextHandler2Helper const & rParent, sal_Int32 nElement,
        const AttributeList& rAttribs, FillProperties& rFillProps,
        model::FillStyle* pFillStyle)
{
    switch( nElement )
    {
        case A_TOKEN( noFill ):
        {
            rFillProps.moFillType = getBaseToken(nElement);
            if (pFillStyle)
            {
                pFillStyle->mpFill = std::make_shared<model::NoFill>();
            }
            return nullptr;
        }
        case A_TOKEN( solidFill ):
        {
            rFillProps.moFillType = getBaseToken(nElement);
            model::SolidFill* pSolidFill = nullptr;
            if (pFillStyle)
            {
                pFillStyle->mpFill = std::make_shared<model::SolidFill>();
                pSolidFill = static_cast<model::SolidFill*>(pFillStyle->mpFill.get());
            }
            return new SolidFillContext(rParent, rFillProps, pSolidFill);
        }
        case A_TOKEN( gradFill ):
        {
            rFillProps.moFillType = getBaseToken(nElement);
            model::GradientFill* pGradientFill = nullptr;
            if (pFillStyle)
            {
                pFillStyle->mpFill = std::make_shared<model::GradientFill>();
                pGradientFill = static_cast<model::GradientFill*>(pFillStyle->mpFill.get());
            }
            return new GradientFillContext(rParent, rAttribs, rFillProps.maGradientProps, pGradientFill);
        }
        case A_TOKEN( pattFill ):
        {
            rFillProps.moFillType = getBaseToken( nElement );
            model::PatternFill* pPatternFill = nullptr;
            if (pFillStyle)
            {
                auto pFill = std::make_shared<model::PatternFill>();
                pPatternFill = pFill.get();
                pFillStyle->mpFill = pFill;
            }
            return new PatternFillContext(rParent, rAttribs, rFillProps.maPatternProps, pPatternFill);
        }
        case A_TOKEN( blipFill ):
        {
            rFillProps.moFillType = getBaseToken( nElement );
            return new BlipFillContext( rParent, rAttribs, rFillProps.maBlipProps );
        }
        case A_TOKEN( grpFill ):
        {
            // TODO
            rFillProps.moFillType = getBaseToken( nElement );
            return nullptr;
        };
    }
    return nullptr;
}

SimpleFillPropertiesContext::SimpleFillPropertiesContext( ContextHandler2Helper const & rParent, Color& rColor ) :
    FillPropertiesContext( rParent, *this ),
    mrColor( rColor )
{
}

SimpleFillPropertiesContext::~SimpleFillPropertiesContext()
{
    mrColor = getBestSolidColor();
}

BlipExtensionContext::BlipExtensionContext( ContextHandler2Helper const & rParent, BlipFillProperties& rBlipProps ) :
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

ArtisticEffectContext::ArtisticEffectContext( ContextHandler2Helper const & rParent, ArtisticEffectProperties& rEffect ) :
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
            OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( embed )) );
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
    sal_Int32 const aAttribs[19] = {
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
                maEffect.maAttribs[sName] <<= rAttribs.getInteger( nAttrib, 0 );
        }
    }

    return nullptr;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
