/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oox/drawingml/effectpropertiescontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/effectproperties.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

// CT_EffectProperties

namespace oox { namespace drawingml {


EffectPropertiesContext::EffectPropertiesContext( ContextHandler2Helper& rParent,
    EffectProperties& rEffectProperties ) throw()
: ContextHandler2( rParent )
, mrEffectProperties( rEffectProperties )
{
}

EffectPropertiesContext::~EffectPropertiesContext()
{
}

ContextHandlerRef EffectPropertiesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( outerShdw ):
        {
            mrEffectProperties.msUnsupportedEffectName = "outerShdw";
            if( rAttribs.hasAttribute( XML_algn ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "algn",
                                                                  makeAny( rAttribs.getString( XML_algn, "" ) ) );
            if( rAttribs.hasAttribute( XML_blurRad ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "blurRad",
                                                                  makeAny( rAttribs.getInteger( XML_blurRad, 0 ) ) );
            if( rAttribs.hasAttribute( XML_dir ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "dir",
                                                                  makeAny( rAttribs.getInteger( XML_dir, 0 ) ) );
            if( rAttribs.hasAttribute( XML_dist ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "dist",
                                                                  makeAny( rAttribs.getInteger( XML_dist, 0 ) ) );
            if( rAttribs.hasAttribute( XML_kx ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "kx",
                                                                  makeAny( rAttribs.getInteger( XML_kx, 0 ) ) );
            if( rAttribs.hasAttribute( XML_ky ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "ky",
                                                                  makeAny( rAttribs.getInteger( XML_ky, 0 ) ) );
            if( rAttribs.hasAttribute( XML_rotWithShape ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "rotWithShape",
                                                                  makeAny( rAttribs.getInteger( XML_rotWithShape, 0 ) ) );
            if( rAttribs.hasAttribute( XML_sx ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "sx",
                                                                  makeAny( rAttribs.getInteger( XML_sx, 0 ) ) );
            if( rAttribs.hasAttribute( XML_sy ) )
                mrEffectProperties.appendUnsupportedEffectAttrib( "sy",
                                                                  makeAny( rAttribs.getInteger( XML_sy, 0 ) ) );

            mrEffectProperties.maShadow.moShadowDist = rAttribs.getInteger( XML_dist, 0 );
            mrEffectProperties.maShadow.moShadowDir = rAttribs.getInteger( XML_dir, 0 );
            return new ColorContext( *this, mrEffectProperties.maShadow.moShadowColor );
        }
        break;
    }

    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
