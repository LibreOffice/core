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

#include "oox/drawingml/scene3dcontext.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

Scene3DPropertiesContext::Scene3DPropertiesContext( ContextHandler2Helper& rParent, Shape3DProperties& r3DProperties ) throw()
: ContextHandler2( rParent )
, mr3DProperties( r3DProperties )
{
}

ContextHandlerRef Scene3DPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( camera ):
        mr3DProperties.mfFieldOfVision = rAttribs.getInteger( XML_fov, 0 ) / 60000.0; // 60000ths of degree
        mr3DProperties.mfZoom = rAttribs.getInteger( XML_zoom, 100000 ) / 100000.0;
        mr3DProperties.mnPreset = rAttribs.getToken( XML_prst, XML_none );
        // TODO: nested element XML_rot
        break;
    case A_TOKEN( lightRig ):
        mr3DProperties.mnLightRigDirection = rAttribs.getToken( XML_dir, XML_none );
        mr3DProperties.mnLightRigType = rAttribs.getToken( XML_rig, XML_none );
        // TODO: nested element XML_rot
        break;
    case A_TOKEN( backdrop ):
    case A_TOKEN( extLst ):
        return 0; // TODO: later (backdrop is not supported by core anyway)
    }
    return 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
