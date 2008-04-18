/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colorchoicecontext.cxx,v $
 * $Revision: 1.5 $
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
#include "oox/drawingml/clrscheme.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

colorChoiceContext::colorChoiceContext( ContextHandler& rParent, Color& rColor )
: ContextHandler( rParent )
, mrColor( rColor )
{
}

void colorChoiceContext::startFastElement( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( xAttribs );

    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_scrgbClr:   // CT_ScRgbColor
            mrColor.setScrgbClr(
                aAttribs.getInteger( XML_r, 0 ),
                aAttribs.getInteger( XML_g, 0 ),
                aAttribs.getInteger( XML_b, 0 ) );
        break;

        case NMSP_DRAWINGML|XML_srgbClr:    // CT_SRgbColor
            mrColor.setSrgbClr( aAttribs.getHex( XML_val, 0 ) );
        break;

        case NMSP_DRAWINGML|XML_hslClr: // CT_HslColor
            mrColor.setHslClr(
                aAttribs.getInteger( XML_hue, 0 ),
                aAttribs.getInteger( XML_sat, 0 ),
                aAttribs.getInteger( XML_lum, 0 ) );
        break;

        case NMSP_DRAWINGML|XML_sysClr: // CT_SystemColor
            mrColor.setSysClr( aAttribs.getToken( XML_val ), aAttribs.getHex( XML_lastClr, -1 ) );
        break;

        case NMSP_DRAWINGML|XML_schemeClr:  // CT_SchemeColor
            mrColor.setSchemeClr( aAttribs.getToken( XML_val ) );
        break;

        case NMSP_DRAWINGML|XML_prstClr:    // CT_PresetColor
            mrColor.setPrstClr( aAttribs.getToken( XML_val ) );
        break;
    }
}

Reference< XFastContextHandler > colorChoiceContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    // colorTransformGroup
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_alpha:
        case NMSP_DRAWINGML|XML_alphaMod:
        case NMSP_DRAWINGML|XML_alphaOff:
        case NMSP_DRAWINGML|XML_blue:
        case NMSP_DRAWINGML|XML_blueMod:
        case NMSP_DRAWINGML|XML_blueOff:
        case NMSP_DRAWINGML|XML_hue:
        case NMSP_DRAWINGML|XML_hueMod:
        case NMSP_DRAWINGML|XML_hueOff:
        case NMSP_DRAWINGML|XML_lum:
        case NMSP_DRAWINGML|XML_lumMod:
        case NMSP_DRAWINGML|XML_lumOff:
        case NMSP_DRAWINGML|XML_green:
        case NMSP_DRAWINGML|XML_greenMod:
        case NMSP_DRAWINGML|XML_greenOff:
        case NMSP_DRAWINGML|XML_red:
        case NMSP_DRAWINGML|XML_redMod:
        case NMSP_DRAWINGML|XML_redOff:
        case NMSP_DRAWINGML|XML_sat:
        case NMSP_DRAWINGML|XML_satMod:
        case NMSP_DRAWINGML|XML_satOff:
        case NMSP_DRAWINGML|XML_shade:
        case NMSP_DRAWINGML|XML_tint:
            mrColor.addTransformation( aElementToken, rxAttribs->getOptionalValue( XML_val ).toInt32() );
        break;
        case NMSP_DRAWINGML|XML_comp:
        case NMSP_DRAWINGML|XML_gamma:
        case NMSP_DRAWINGML|XML_gray:
        case NMSP_DRAWINGML|XML_inv:
        case NMSP_DRAWINGML|XML_invGamma:
            mrColor.addTransformation( aElementToken );
        break;
    }
    return this;
}


} }
