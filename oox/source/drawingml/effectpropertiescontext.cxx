/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/effectpropertiescontext.hxx>
#include "effectproperties.hxx"
#include <drawingml/colorchoicecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;

// CT_EffectProperties

namespace oox::drawingml {

EffectPropertiesContext::EffectPropertiesContext( ContextHandler2Helper const& rParent,
        EffectProperties& rEffectProperties, model::EffectStyle* pEffectStyle) noexcept
    : ContextHandler2(rParent)
    , mpEffectStyle(pEffectStyle)
    , mrEffectProperties(rEffectProperties)
{
}

EffectPropertiesContext::~EffectPropertiesContext()
{
}

void EffectPropertiesContext::saveUnsupportedAttribs( Effect& rEffect, const AttributeList& rAttribs )
{
    if( rAttribs.hasAttribute( XML_algn ) )
        rEffect.maAttribs["algn"] <<= rAttribs.getStringDefaulted( XML_algn);
    if( rAttribs.hasAttribute( XML_blurRad ) )
        rEffect.maAttribs["blurRad"] <<= rAttribs.getInteger( XML_blurRad, 0 );
    if( rAttribs.hasAttribute( XML_dir ) )
        rEffect.maAttribs["dir"] <<= rAttribs.getInteger( XML_dir, 0 );
    if( rAttribs.hasAttribute( XML_dist ) )
        rEffect.maAttribs["dist"] <<= rAttribs.getInteger( XML_dist, 0 );
    if( rAttribs.hasAttribute( XML_kx ) )
        rEffect.maAttribs["kx"] <<= rAttribs.getInteger( XML_kx, 0 );
    if( rAttribs.hasAttribute( XML_ky ) )
        rEffect.maAttribs["ky"] <<= rAttribs.getInteger( XML_ky, 0 );
    if( rAttribs.hasAttribute( XML_rotWithShape ) )
        rEffect.maAttribs["rotWithShape"] <<= rAttribs.getInteger( XML_rotWithShape, 0 );
    if( rAttribs.hasAttribute( XML_sx ) )
        rEffect.maAttribs["sx"] <<= rAttribs.getInteger( XML_sx, 0 );
    if( rAttribs.hasAttribute( XML_sy ) )
        rEffect.maAttribs["sy"] <<= rAttribs.getInteger( XML_sy, 0 );
    if( rAttribs.hasAttribute( XML_rad ) )
        rEffect.maAttribs["rad"] <<= rAttribs.getInteger( XML_rad, 0 );
    if( rAttribs.hasAttribute( XML_endA ) )
        rEffect.maAttribs["endA"] <<= rAttribs.getInteger( XML_endA, 0 );
    if( rAttribs.hasAttribute( XML_endPos ) )
        rEffect.maAttribs["endPos"] <<= rAttribs.getInteger( XML_endPos, 0 );
    if( rAttribs.hasAttribute( XML_fadeDir ) )
        rEffect.maAttribs["fadeDir"] <<= rAttribs.getInteger( XML_fadeDir, 0 );
    if( rAttribs.hasAttribute( XML_stA ) )
        rEffect.maAttribs["stA"] <<= rAttribs.getInteger( XML_stA, 0 );
    if( rAttribs.hasAttribute( XML_stPos ) )
        rEffect.maAttribs["stPos"] <<= rAttribs.getInteger( XML_stPos, 0 );
    if( rAttribs.hasAttribute( XML_grow ) )
        rEffect.maAttribs["grow"] <<= rAttribs.getInteger( XML_grow, 0 );
}

ContextHandlerRef EffectPropertiesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    sal_Int32 nPos = mrEffectProperties.m_Effects.size();
    mrEffectProperties.m_Effects.push_back(std::make_unique<Effect>());
    switch( nElement )
    {
        case A_TOKEN( outerShdw ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = "outerShdw";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );
            mrEffectProperties.maShadow.moShadowSx = rAttribs.getInteger( XML_sx, 0 );
            mrEffectProperties.maShadow.moShadowSy = rAttribs.getInteger( XML_sy, 0 );
            mrEffectProperties.maShadow.moShadowBlur = rAttribs.getInteger( XML_blurRad, 0 );
            mrEffectProperties.maShadow.moShadowAlignment = convertToRectangleAlignment( rAttribs.getToken(XML_algn, XML_b) );

            model::ComplexColor* pColor = nullptr;
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::OuterShadow;
                rEffect.mnBlurRadius = rAttribs.getInteger(XML_blurRad, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDistance = rAttribs.getInteger(XML_dist, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDirection = rAttribs.getInteger(XML_dir, 0); // ST_PositiveFixedAngle, default 0
                rEffect.mnScaleX = GetPercent( rAttribs.getStringDefaulted(XML_sx)); // ST_Percentage, default 100%
                rEffect.mnScaley = GetPercent( rAttribs.getStringDefaulted(XML_sy)); // ST_Percentage, default 100%
                rEffect.mnScewX = rAttribs.getInteger(XML_kx, 0); // ST_FixedAngle, default 0
                rEffect.mnScewY = rAttribs.getInteger(XML_ky, 0); // ST_FixedAngle, default 0
                // ST_RectAlignment, default "b" - Bottom
                rEffect.meAlignment = convertToRectangleAlignment(rAttribs.getToken(XML_algn, XML_b));
                rEffect.mbRotateWithShape = rAttribs.getBool(XML_rotWithShape, true); // boolean, default "true"
                pColor = &rEffect.maColor;
            }
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor, pColor);
        }
        break;
        case A_TOKEN( innerShdw ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = "innerShdw";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );

            model::ComplexColor* pColor = nullptr;
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::InnerShadow;
                rEffect.mnBlurRadius = rAttribs.getInteger(XML_blurRad, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDistance = rAttribs.getInteger(XML_dist, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDirection = rAttribs.getInteger(XML_dir, 0); // ST_PositiveFixedAngle, default 0
                pColor = &rEffect.maColor;
            }
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor, pColor);
        }
        break;
        case A_TOKEN( glow ):
        {
            mrEffectProperties.maGlow.moGlowRad = rAttribs.getInteger( XML_rad, 0 );
            // undo push_back to effects
            mrEffectProperties.m_Effects.pop_back();

            model::ComplexColor* pColor = nullptr;
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::Glow;
                rEffect.mnRadius = rAttribs.getInteger(XML_rad, 0); //ST_PositiveCoordinate, default 0
                pColor = &rEffect.maColor;
            }
            return new ColorContext(*this, mrEffectProperties.maGlow.moGlowColor, pColor);

        }
        case A_TOKEN( softEdge ):
        {
            mrEffectProperties.maSoftEdge.moRad = rAttribs.getInteger(XML_rad, 0);
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::SoftEdge;
                rEffect.mnRadius = rAttribs.getInteger(XML_rad, 0); // ST_PositiveCoordinate, default 0
            }
            return this; // no inner elements
        }
        case A_TOKEN( reflection ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = "reflection";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            model::ComplexColor* pColor = nullptr;
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::Reflection;
                rEffect.mnBlurRadius = rAttribs.getInteger(XML_blurRad, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDistance = rAttribs.getInteger(XML_dist, 0); // ST_PositiveCoordinate, default 0
                rEffect.mnDirection = rAttribs.getInteger(XML_dir, 0); // ST_PositiveFixedAngle, default 0
                rEffect.mnScaleX = GetPercent(rAttribs.getStringDefaulted(XML_sx)); // ST_Percentage, default 100%
                rEffect.mnScaley = GetPercent(rAttribs.getStringDefaulted(XML_sy)); // ST_Percentage, default 100%
                rEffect.mnScewX = rAttribs.getInteger(XML_kx, 0); // ST_FixedAngle, default 0
                rEffect.mnScewY = rAttribs.getInteger(XML_ky, 0); // ST_FixedAngle, default 0
                // ST_RectAlignment, default "b" - Bottom
                rEffect.meAlignment = convertToRectangleAlignment(rAttribs.getToken(XML_algn, XML_b));
                rEffect.mbRotateWithShape = rAttribs.getBool(XML_rotWithShape, true); // boolean, default "true"

                rEffect.mnEndAlpha = GetPositiveFixedPercentage(rAttribs.getStringDefaulted(XML_endA)); // ST_PositiveFixedPercentage, default 100%
                rEffect.mnEndPosition = GetPositiveFixedPercentage(rAttribs.getStringDefaulted(XML_endPos)); // ST_PositiveFixedPercentage, default 0%
                rEffect.mnStartAlpha = GetPositiveFixedPercentage(rAttribs.getStringDefaulted(XML_stA)); // ST_PositiveFixedPercentage, default 0%
                rEffect.mnStartPosition = GetPositiveFixedPercentage(rAttribs.getStringDefaulted(XML_stPos)); // ST_PositiveFixedPercentage, default 100%
                rEffect.mnFadeDirection = rAttribs.getInteger(XML_fadeDir, 5400000); // ST_PositiveFixedAngle, default 5400000

                pColor = &rEffect.maColor;
            }
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor, pColor);
        }
        case A_TOKEN( blur ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = "blur";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            model::ComplexColor* pColor = nullptr;
            if (mpEffectStyle)
            {
                auto& rEffect = mpEffectStyle->maEffectList.emplace_back();
                rEffect.meType = model::EffectType::Blur;
                rEffect.mnRadius = rAttribs.getInteger(XML_rad, 0); // ST_PositiveCoordinate, default 0
                rEffect.mbGrow = rAttribs.getBool(XML_grow, true); // boolean, default true
                pColor = &rEffect.maColor;
            }
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor, pColor);
        }
        break;
    }

    mrEffectProperties.m_Effects.pop_back();
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
