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
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

#include <basegfx/numeric/ftools.hxx>

namespace oox {
namespace drawingml {

void EffectShadowProperties::assignUsed(const EffectShadowProperties& rSourceProps)
{
    moShadowDist.assignIfUsed( rSourceProps.moShadowDist );
    moShadowDir.assignIfUsed( rSourceProps.moShadowDir );
    moShadowColor.assignIfUsed( rSourceProps.moShadowColor );
}

void EffectProperties::assignUsed( const EffectProperties& rSourceProps )
{
    maShadow.assignUsed(rSourceProps.maShadow);
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
            std::map< OUString, css::uno::Any >::const_iterator attribIt = it->maAttribs.find( "dir" );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrDir;

            attribIt = it->maAttribs.find( "dist" );
            if( attribIt != it->maAttribs.end() )
                attribIt->second >>= nAttrDist;

            // Negative X or Y dist indicates left or up, respectively
            double nAngle = basegfx::deg2rad(static_cast<double>(nAttrDir) / PER_DEGREE);
            sal_Int32 nDist = convertEmuToHmm( nAttrDist );
            sal_Int32 nXDist = cos(nAngle) * nDist;
            sal_Int32 nYDist = sin(nAngle) * nDist;

            rPropMap.setProperty( PROP_Shadow, true );
            rPropMap.setProperty( PROP_ShadowXDistance, nXDist);
            rPropMap.setProperty( PROP_ShadowYDistance, nYDist);
            rPropMap.setProperty( PROP_ShadowColor, it->moColor.getColor(rGraphicHelper ) );
            rPropMap.setProperty( PROP_ShadowTransparence, it->moColor.getTransparency());
        }
    }
}

css::beans::PropertyValue Effect::getEffect()
{
    css::beans::PropertyValue aRet;
    if( msName.isEmpty() )
        return aRet;

    css::uno::Sequence< css::beans::PropertyValue > aSeq( maAttribs.size() );
    sal_uInt32 i = 0;
    for (auto const& attrib : maAttribs)
    {
        aSeq[i].Name = attrib.first;
        aSeq[i].Value = attrib.second;
        i++;
    }

    aRet.Name = msName;
    aRet.Value <<= aSeq;

    return aRet;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
