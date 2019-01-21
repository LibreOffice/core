/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "effectpropertiescontext.hxx"
#include "effectproperties.hxx"
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/misccontexts.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

// CT_EffectProperties

namespace oox { namespace drawingml {

EffectPropertiesContext::EffectPropertiesContext( ContextHandler2Helper const& rParent,
    EffectProperties& rEffectProperties ) throw()
: ContextHandler2( rParent )
, mrEffectProperties( rEffectProperties )
{
}

EffectPropertiesContext::~EffectPropertiesContext()
{
}

void EffectPropertiesContext::saveUnsupportedAttribs( Effect& rEffect, const AttributeList& rAttribs )
{
    if( rAttribs.hasAttribute( XML_algn ) )
        rEffect.maAttribs["algn"] <<= rAttribs.getString( XML_algn, "" );
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
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor);
        }
        break;
        case A_TOKEN( innerShdw ):
        {
            mrEffectProperties.m_Effects[nPos]->msName = "innerShdw";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor);
        }
        break;
        case A_TOKEN( glow ):
        case A_TOKEN( softEdge ):
        case A_TOKEN( reflection ):
        case A_TOKEN( blur ):
        {
            if( nElement == A_TOKEN( glow ) )
                mrEffectProperties.m_Effects[nPos]->msName = "glow";
            else if( nElement == A_TOKEN( softEdge ) )
                mrEffectProperties.m_Effects[nPos]->msName = "softEdge";
            else if( nElement == A_TOKEN( reflection ) )
                mrEffectProperties.m_Effects[nPos]->msName = "reflection";
            else if( nElement == A_TOKEN( blur ) )
                mrEffectProperties.m_Effects[nPos]->msName = "blur";
            saveUnsupportedAttribs(*mrEffectProperties.m_Effects[nPos], rAttribs);
            return new ColorContext(*this, mrEffectProperties.m_Effects[nPos]->moColor);
        }
        break;
    }

    mrEffectProperties.m_Effects.pop_back();
    return nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
