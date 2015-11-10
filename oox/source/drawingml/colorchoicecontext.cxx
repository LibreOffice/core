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

#include "oox/drawingml/drawingmltypes.hxx"
#include "drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/color.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::oox::core::ContextHandler;

namespace oox {
namespace drawingml {

ColorValueContext::ColorValueContext( ContextHandler2Helper& rParent, Color& rColor ) :
    ContextHandler2( rParent ),
    mrColor( rColor )
{
}

void ColorValueContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case A_TOKEN( scrgbClr ):
            mrColor.setScrgbClr(
                rAttribs.getInteger( XML_r, 0 ),
                rAttribs.getInteger( XML_g, 0 ),
                rAttribs.getInteger( XML_b, 0 ) );
        break;

        case A_TOKEN( srgbClr ):
            mrColor.setSrgbClr( rAttribs.getIntegerHex( XML_val, 0 ) );
        break;

        case A_TOKEN( hslClr ):
            mrColor.setHslClr(
                rAttribs.getInteger( XML_hue, 0 ),
                rAttribs.getInteger( XML_sat, 0 ),
                rAttribs.getInteger( XML_lum, 0 ) );
        break;

        case A_TOKEN( sysClr ):
            mrColor.setSysClr(
                rAttribs.getToken( XML_val, XML_TOKEN_INVALID ),
                rAttribs.getIntegerHex( XML_lastClr, -1 ) );
        break;

        case A_TOKEN( schemeClr ):
        {
            mrColor.setSchemeClr( rAttribs.getToken( XML_val, XML_TOKEN_INVALID ) );
            oox::OptValue<rtl::OUString> sSchemeName = rAttribs.getString( XML_val );
            if( sSchemeName.has() )
                mrColor.setSchemeName( sSchemeName.use() );
        }
        break;

        case A_TOKEN( prstClr ):
            mrColor.setPrstClr( rAttribs.getToken( XML_val, XML_TOKEN_INVALID ) );
        break;
    }
}

::oox::core::ContextHandlerRef ColorValueContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( alpha ):
        case A_TOKEN( alphaMod ):
        case A_TOKEN( alphaOff ):
        case A_TOKEN( blue ):
        case A_TOKEN( blueMod ):
        case A_TOKEN( blueOff ):
        case A_TOKEN( hue ):
        case A_TOKEN( hueMod ):
        case A_TOKEN( hueOff ):
        case A_TOKEN( lum ):
        case A_TOKEN( lumMod ):
        case A_TOKEN( lumOff ):
        case A_TOKEN( green ):
        case A_TOKEN( greenMod ):
        case A_TOKEN( greenOff ):
        case A_TOKEN( red ):
        case A_TOKEN( redMod ):
        case A_TOKEN( redOff ):
        case A_TOKEN( sat ):
        case A_TOKEN( satMod ):
        case A_TOKEN( satOff ):
        case A_TOKEN( shade ):
        case A_TOKEN( tint ):
        {
            OUString aValue = rAttribs.getString( XML_val, OUString() );
            sal_Int32 nVal = 0;
            if (aValue.endsWith("%"))
            {
                nVal = aValue.toDouble() * PER_PERCENT;
            }
            else
                nVal = rAttribs.getInteger(XML_val, 0);
            mrColor.addTransformation( nElement, nVal );
        }
        break;
        case A_TOKEN( comp ):
        case A_TOKEN( gamma ):
        case A_TOKEN( gray ):
        case A_TOKEN( inv ):
        case A_TOKEN( invGamma ):
            mrColor.addTransformation( nElement );
        break;
    }
    return nullptr;
}

ColorContext::ColorContext( ContextHandler2Helper& rParent, Color& rColor ) :
    ContextHandler2( rParent ),
    mrColor( rColor )
{
}

::oox::core::ContextHandlerRef ColorContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( scrgbClr ):
        case A_TOKEN( srgbClr ):
        case A_TOKEN( hslClr ):
        case A_TOKEN( sysClr ):
        case A_TOKEN( schemeClr ):
        case A_TOKEN( prstClr ):
            return new ColorValueContext( *this, mrColor );
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
