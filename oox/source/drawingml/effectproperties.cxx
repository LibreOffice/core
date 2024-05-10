/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "effectproperties.hxx"
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/properties.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <comphelper/propertyvalue.hxx>
#include <docmodel/theme/FormatScheme.hxx>

#include <algorithm>

namespace oox::drawingml {

void EffectGlowProperties ::assignUsed(const EffectGlowProperties& rSourceProps)
{
    assignIfUsed( moGlowRad, rSourceProps.moGlowRad );
    moGlowColor.assignIfUsed( rSourceProps.moGlowColor );
}

void EffectSoftEdgeProperties::assignUsed(const EffectSoftEdgeProperties& rSourceProps)
{
    assignIfUsed(moRad, rSourceProps.moRad);
}

void EffectShadowProperties::assignUsed(const EffectShadowProperties& rSourceProps)
{
    assignIfUsed( moShadowDist, rSourceProps.moShadowDist );
    assignIfUsed( moShadowDir, rSourceProps.moShadowDir );
    assignIfUsed( moShadowSx, rSourceProps.moShadowSx );
    assignIfUsed( moShadowSy, rSourceProps.moShadowSy );
    moShadowColor.assignIfUsed( rSourceProps.moShadowColor );
    assignIfUsed( moShadowBlur, rSourceProps.moShadowBlur );
    assignIfUsed( moShadowAlignment, rSourceProps.moShadowAlignment );

}

void EffectProperties::assignUsed( const EffectProperties& rSourceProps )
{
    maShadow.assignUsed(rSourceProps.maShadow);
    maGlow.assignUsed(rSourceProps.maGlow);
    maSoftEdge.assignUsed(rSourceProps.maSoftEdge);
    if (!rSourceProps.m_Effects.empty())
    {
        m_Effects.clear();
        m_Effects.reserve(rSourceProps.m_Effects.size());
        for (auto const& it : rSourceProps.m_Effects)
        {
            m_Effects.push_back(std::make_unique<Effect>(*it));
        }
    }
}

void EffectProperties::pushToPropMap( PropertyMap& rPropMap,
        const GraphicHelper& rGraphicHelper ) const
{
    for (auto const& it : m_Effects)
    {
        if( it->msName == "outerShdw" )
        {
            sal_Int32 nAttrDir = 0, nAttrDist = 0;
            sal_Int32 nAttrSizeX = 100000, nAttrSizeY = 100000; // If shadow size is %100=100000 (means equal to object's size), sx sy is not exists,
                                                                // Default values of sx, sy should be 100000 in this case.
            sal_Int32 nAttrBlur = 0;

            std::map< OUString, css::uno::Any >::const_iterator attribIt = it->maAttribs.find( u"dir"_ustr );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrDir;

            attribIt = it->maAttribs.find( u"dist"_ustr );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrDist;

            attribIt = it->maAttribs.find( u"sx"_ustr );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrSizeX;

            attribIt = it->maAttribs.find( u"sy"_ustr );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrSizeY;

            attribIt = it->maAttribs.find( u"blurRad"_ustr );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrBlur;

            // Negative X or Y dist indicates left or up, respectively
            // Negative X or Y dist indicates left or up, respectively
            double nAngle = basegfx::deg2rad(static_cast<double>(nAttrDir) / PER_DEGREE);
            sal_Int32 nDist = convertEmuToHmm( nAttrDist );
            sal_Int32 nXDist = cos(nAngle) * nDist;
            sal_Int32 nYDist = sin(nAngle) * nDist;


            rPropMap.setProperty( PROP_Shadow, true );
            rPropMap.setProperty( PROP_ShadowXDistance, nXDist);
            rPropMap.setProperty( PROP_ShadowYDistance, nYDist);
            rPropMap.setProperty( PROP_ShadowSizeX, nAttrSizeX);
            rPropMap.setProperty( PROP_ShadowSizeY, nAttrSizeY);
            rPropMap.setProperty( PROP_ShadowColor, it->moColor.getColor(rGraphicHelper ) );
            rPropMap.setProperty( PROP_ShadowTransparence, it->moColor.getTransparency());
            rPropMap.setProperty( PROP_ShadowBlur, convertEmuToHmm(nAttrBlur));
            rPropMap.setProperty(
                PROP_ShadowAlignment,
                static_cast<sal_Int32>(maShadow.moShadowAlignment.value_or(model::RectangleAlignment::Bottom)));

        }
    }
}

css::beans::PropertyValue Effect::getEffect()
{
    css::beans::PropertyValue aRet;
    if( msName.isEmpty() )
        return aRet;

    css::uno::Sequence< css::beans::PropertyValue > aSeq( maAttribs.size() );
    std::transform(maAttribs.begin(), maAttribs.end(), aSeq.getArray(),
                   [](const auto& attrib)
                   { return comphelper::makePropertyValue(attrib.first, attrib.second); });

    aRet.Name = msName;
    aRet.Value <<= aSeq;

    return aRet;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
