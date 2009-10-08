/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colorchoicecontext.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/color.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::oox::core::ContextHandler;

namespace oox {
namespace drawingml {

// ============================================================================

ColorValueContext::ColorValueContext( ContextHandler& rParent, Color& rColor ) :
    ContextHandler( rParent ),
    mrColor( rColor )
{
}

void ColorValueContext::startFastElement( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
    throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case A_TOKEN( scrgbClr ):
            mrColor.setScrgbClr(
                aAttribs.getInteger( XML_r, 0 ),
                aAttribs.getInteger( XML_g, 0 ),
                aAttribs.getInteger( XML_b, 0 ) );
        break;

        case A_TOKEN( srgbClr ):
            mrColor.setSrgbClr( aAttribs.getIntegerHex( XML_val, 0 ) );
        break;

        case A_TOKEN( hslClr ):
            mrColor.setHslClr(
                aAttribs.getInteger( XML_hue, 0 ),
                aAttribs.getInteger( XML_sat, 0 ),
                aAttribs.getInteger( XML_lum, 0 ) );
        break;

        case A_TOKEN( sysClr ):
            mrColor.setSysClr(
                aAttribs.getToken( XML_val, XML_TOKEN_INVALID ),
                aAttribs.getIntegerHex( XML_lastClr, -1 ) );
        break;

        case A_TOKEN( schemeClr ):
            mrColor.setSchemeClr( aAttribs.getToken( XML_val, XML_TOKEN_INVALID ) );
        break;

        case A_TOKEN( prstClr ):
            mrColor.setPrstClr( aAttribs.getToken( XML_val, XML_TOKEN_INVALID ) );
        break;
    }
}

Reference< XFastContextHandler > ColorValueContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
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
            mrColor.addTransformation( nElement, aAttribs.getInteger( XML_val, 0 ) );
        break;
        case A_TOKEN( comp ):
        case A_TOKEN( gamma ):
        case A_TOKEN( gray ):
        case A_TOKEN( inv ):
        case A_TOKEN( invGamma ):
            mrColor.addTransformation( nElement );
        break;
    }
    return 0;
}

// ============================================================================

ColorContext::ColorContext( ContextHandler& rParent, Color& rColor ) :
    ContextHandler( rParent ),
    mrColor( rColor )
{
}

Reference< XFastContextHandler > ColorContext::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& ) throw (SAXException, RuntimeException)
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
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

