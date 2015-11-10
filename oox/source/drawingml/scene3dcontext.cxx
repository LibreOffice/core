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

#include "drawingml/scene3dcontext.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include "drawingml/colorchoicecontext.hxx"
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
        if( rAttribs.hasAttribute( XML_fov ) )
            mr3DProperties.mfFieldOfVision = rAttribs.getInteger( XML_fov, 0 ) / 60000.0; // 60000ths of degree
        if( rAttribs.hasAttribute( XML_zoom ) )
            mr3DProperties.mfZoom = rAttribs.getInteger( XML_zoom, 100000 ) / 100000.0;
        if( rAttribs.hasAttribute( XML_prst ) )
            mr3DProperties.mnPreset = rAttribs.getToken( XML_prst, XML_none );

        return new Scene3DRotationPropertiesContext( *this, mr3DProperties.maCameraRotation );

    case A_TOKEN( lightRig ):
        mr3DProperties.mnLightRigDirection = rAttribs.getToken( XML_dir, XML_none );
        mr3DProperties.mnLightRigType = rAttribs.getToken( XML_rig, XML_none );

        return new Scene3DRotationPropertiesContext( *this, mr3DProperties.maLightRigRotation );

    case A_TOKEN( backdrop ):
    case A_TOKEN( extLst ):
        return nullptr; // TODO: later (backdrop is not supported by core anyway)
    }
    return nullptr;
}

Shape3DPropertiesContext::Shape3DPropertiesContext( ContextHandler2Helper& rParent, const AttributeList& rAttribs, Shape3DProperties& r3DProperties ) throw()
: ContextHandler2( rParent )
, mr3DProperties( r3DProperties )
{
    if( rAttribs.hasAttribute( XML_extrusionH ) )
        mr3DProperties.mnExtrusionH = rAttribs.getInteger( XML_extrusionH, 0 );
    if( rAttribs.hasAttribute( XML_contourW ) )
        mr3DProperties.mnContourW = rAttribs.getInteger( XML_contourW, 0 );
    if( rAttribs.hasAttribute( XML_z ) )
        mr3DProperties.mnShapeZ = rAttribs.getInteger( XML_z, 0 );
    if( rAttribs.hasAttribute( XML_prstMaterial ) )
        mr3DProperties.mnMaterial = rAttribs.getToken( XML_prstMaterial, XML_none );
}

ContextHandlerRef Shape3DPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( bevelT ):
    case A_TOKEN( bevelB ):
    {
        BevelProperties aProps;
        if( rAttribs.hasAttribute( XML_w ) )
            aProps.mnWidth = rAttribs.getInteger( XML_w, 0 );
        if( rAttribs.hasAttribute( XML_h ) )
            aProps.mnHeight = rAttribs.getInteger( XML_h, 0 );
        if( rAttribs.hasAttribute( XML_prst ) )
            aProps.mnPreset = rAttribs.getToken( XML_prst, XML_none );

        if( aElementToken == A_TOKEN( bevelT ) )
            mr3DProperties.maTopBevelProperties.set( aProps );
        else
            mr3DProperties.maBottomBevelProperties.set( aProps );

        break;
    }
    case A_TOKEN( extrusionClr ):
        return new ColorContext( *this, mr3DProperties.maExtrusionColor );

    case A_TOKEN( contourClr ):
        return new ColorContext( *this, mr3DProperties.maContourColor );
    }
    return nullptr;
}

Scene3DRotationPropertiesContext::Scene3DRotationPropertiesContext( ContextHandler2Helper& rParent, RotationProperties& rRotationProperties ) throw()
: ContextHandler2( rParent )
, mrRotationProperties( rRotationProperties )
{
}

ContextHandlerRef Scene3DRotationPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( rot ):
        mrRotationProperties.mnLatitude = rAttribs.getInteger( XML_lat, 0 );
        mrRotationProperties.mnLongitude = rAttribs.getInteger( XML_lon, 0 );
        mrRotationProperties.mnRevolution = rAttribs.getInteger( XML_rev, 0 );
        break;
    }
    return nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
