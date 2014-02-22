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
