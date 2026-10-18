/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/effectpropertiescontext.hxx>
#include <oox/drawingml/effectproperties.hxx>
#include <drawingml/colorchoicecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

using namespace ::oox::core;
using namespace ::cpo::uno;

// CT_EffectProperties

namespace oox::drawingml {

namespace
{
/// Reads a length in EMU. A missing attribute gives the default.
gfx::Length readLength(const AttributeList& rAttribs, sal_Int32 nToken, gfx::Length aDefault)
{
    return gfx::Length::emu(rAttribs.getHyper(nToken, aDefault.as_emu<sal_Int64>()));
}

/// Reads an angle in 1/60000 degree. A missing attribute gives the default.
Degree60000 readAngle(const AttributeList& rAttribs, sal_Int32 nToken, Degree60000 aDefault)
{
    return Degree60000(rAttribs.getInteger(nToken, aDefault.get()));
}

/// Reads the blur radius, the distance and the direction of a shadow or a reflection. An attribute
/// that is missing keeps the default of the effect, which is the default of the schema.
template <class Effect> void readOffset(Effect& rEffect, const AttributeList& rAttribs)
{
    rEffect.maBlurRadius = readLength(rAttribs, XML_blurRad, rEffect.maBlurRadius);
    rEffect.maDistance = readLength(rAttribs, XML_dist, rEffect.maDistance);
    rEffect.mnDirection = readAngle(rAttribs, XML_dir, rEffect.mnDirection);
}

/// Reads the scale, the skew, the alignment and the rotation of an outer shadow or a reflection.
template <class Effect> void readTransform(Effect& rEffect, const AttributeList& rAttribs)
{
    rEffect.mnScaleX = rAttribs.getInteger(XML_sx, rEffect.mnScaleX);
    rEffect.mnScaleY = rAttribs.getInteger(XML_sy, rEffect.mnScaleY);
    rEffect.mnSkewX = readAngle(rAttribs, XML_kx, rEffect.mnSkewX);
    rEffect.mnSkewY = readAngle(rAttribs, XML_ky, rEffect.mnSkewY);
    rEffect.meAlignment = convertToRectangleAlignment(rAttribs.getToken(XML_algn, XML_b));
    rEffect.mbRotateWithShape = rAttribs.getBool(XML_rotWithShape, rEffect.mbRotateWithShape);
}
}

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
        rEffect.maAttribs[u"algn"_ustr] <<= rAttribs.getStringDefaulted( XML_algn);
    if( rAttribs.hasAttribute( XML_blurRad ) )
        rEffect.maAttribs[u"blurRad"_ustr] <<= rAttribs.getInteger( XML_blurRad, 0 );
    if( rAttribs.hasAttribute( XML_dir ) )
        rEffect.maAttribs[u"dir"_ustr] <<= rAttribs.getInteger( XML_dir, 0 );
    if( rAttribs.hasAttribute( XML_dist ) )
        rEffect.maAttribs[u"dist"_ustr] <<= rAttribs.getInteger( XML_dist, 0 );
    if( rAttribs.hasAttribute( XML_kx ) )
        rEffect.maAttribs[u"kx"_ustr] <<= rAttribs.getInteger( XML_kx, 0 );
    if( rAttribs.hasAttribute( XML_ky ) )
        rEffect.maAttribs[u"ky"_ustr] <<= rAttribs.getInteger( XML_ky, 0 );
    if( rAttribs.hasAttribute( XML_rotWithShape ) )
        rEffect.maAttribs[u"rotWithShape"_ustr] <<= rAttribs.getInteger( XML_rotWithShape, 0 );
    if( rAttribs.hasAttribute( XML_sx ) )
        rEffect.maAttribs[u"sx"_ustr] <<= rAttribs.getInteger( XML_sx, 0 );
    if( rAttribs.hasAttribute( XML_sy ) )
        rEffect.maAttribs[u"sy"_ustr] <<= rAttribs.getInteger( XML_sy, 0 );
    if( rAttribs.hasAttribute( XML_rad ) )
        rEffect.maAttribs[u"rad"_ustr] <<= rAttribs.getInteger( XML_rad, 0 );
    if( rAttribs.hasAttribute( XML_endA ) )
        rEffect.maAttribs[u"endA"_ustr] <<= rAttribs.getInteger( XML_endA, 0 );
    if( rAttribs.hasAttribute( XML_endPos ) )
        rEffect.maAttribs[u"endPos"_ustr] <<= rAttribs.getInteger( XML_endPos, 0 );
    if( rAttribs.hasAttribute( XML_fadeDir ) )
        rEffect.maAttribs[u"fadeDir"_ustr] <<= rAttribs.getInteger( XML_fadeDir, 0 );
    if( rAttribs.hasAttribute( XML_stA ) )
        rEffect.maAttribs[u"stA"_ustr] <<= rAttribs.getInteger( XML_stA, 0 );
    if( rAttribs.hasAttribute( XML_stPos ) )
        rEffect.maAttribs[u"stPos"_ustr] <<= rAttribs.getInteger( XML_stPos, 0 );
    if( rAttribs.hasAttribute( XML_grow ) )
        rEffect.maAttribs[u"grow"_ustr] <<= rAttribs.getInteger( XML_grow, 0 );
}

ContextHandlerRef EffectPropertiesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    sal_Int32 nPos = mrEffectProperties.m_Effects.size();
    mrEffectProperties.m_Effects.push_back(std::make_unique<Effect>());
    switch( nElement )
    {
        case A_TOKEN( outerShdw ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = u"outerShdw"_ustr;
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );
            mrEffectProperties.maShadow.moShadowSx = rAttribs.getInteger( XML_sx, 0 );
            mrEffectProperties.maShadow.moShadowSy = rAttribs.getInteger( XML_sy, 0 );
            mrEffectProperties.maShadow.moShadowBlur = rAttribs.getInteger( XML_blurRad, 0 );
            mrEffectProperties.maShadow.moShadowAlignment = convertToRectangleAlignment( rAttribs.getToken(XML_algn, XML_b) );

            auto& rEffect = mrEffectProperties.maEffects.moOuterShadow.emplace();
            readOffset(rEffect, rAttribs);
            readTransform(rEffect, rAttribs);
            return new ColorContext(*this, &mrEffectProperties.m_Effects[nPos]->moColor,
                                    &rEffect.maColor);
        }
        break;
        case A_TOKEN( innerShdw ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = u"innerShdw"_ustr;
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );

            auto& rEffect = mrEffectProperties.maEffects.moInnerShadow.emplace();
            readOffset(rEffect, rAttribs);
            return new ColorContext(*this, &mrEffectProperties.m_Effects[nPos]->moColor,
                                    &rEffect.maColor);
        }
        break;
        case A_TOKEN( glow ):
        {
            mrEffectProperties.maGlow.moGlowRad = rAttribs.getInteger( XML_rad, 0 );
            // undo push_back to effects
            mrEffectProperties.m_Effects.pop_back();

            auto& rEffect = mrEffectProperties.maEffects.moGlow.emplace();
            rEffect.maRadius = readLength(rAttribs, XML_rad, rEffect.maRadius);
            return new ColorContext(*this, &mrEffectProperties.maGlow.moGlowColor,
                                    &rEffect.maColor);
        }
        case A_TOKEN( softEdge ):
        {
            auto& rEffect = mrEffectProperties.maEffects.moSoftEdge.emplace();
            rEffect.maRadius = readLength(rAttribs, XML_rad, rEffect.maRadius);
            return this; // no inner elements
        }
        case A_TOKEN( reflection ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = u"reflection"_ustr;
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            auto& rEffect = mrEffectProperties.maEffects.moReflection.emplace();
            readOffset(rEffect, rAttribs);
            readTransform(rEffect, rAttribs);
            rEffect.mnStartAlpha = rAttribs.getInteger(XML_stA, rEffect.mnStartAlpha);
            rEffect.mnStartPosition = rAttribs.getInteger(XML_stPos, rEffect.mnStartPosition);
            rEffect.mnEndAlpha = rAttribs.getInteger(XML_endA, rEffect.mnEndAlpha);
            rEffect.mnEndPosition = rAttribs.getInteger(XML_endPos, rEffect.mnEndPosition);
            rEffect.mnFadeDirection = readAngle(rAttribs, XML_fadeDir, rEffect.mnFadeDirection);
            return new ColorContext(*this, &mrEffectProperties.m_Effects[nPos]->moColor, nullptr);
        }
        case A_TOKEN( blur ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = u"blur"_ustr;
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            auto& rEffect = mrEffectProperties.maEffects.moBlur.emplace();
            rEffect.maRadius = readLength(rAttribs, XML_rad, rEffect.maRadius);
            rEffect.mbGrow = rAttribs.getBool(XML_grow, rEffect.mbGrow);
            return new ColorContext(*this, &mrEffectProperties.m_Effects[nPos]->moColor, nullptr);
        }
        break;
    }

    mrEffectProperties.m_Effects.pop_back();
    return nullptr;
}

void EffectPropertiesContext::onEndElement()
{
    const sal_Int32 nElement = getCurrentElement();
    if (nElement == A_TOKEN(effectLst) || nElement == A_TOKEN(effectDag))
        mrEffectProperties.mbEffectList = true;
    // The effects of a theme effect style go to the theme as well.
    if (mpEffectStyle && nElement == A_TOKEN(effectLst))
        *mpEffectStyle = mrEffectProperties.maEffects;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
