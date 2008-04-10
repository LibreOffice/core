/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colorchoicecontext.cxx,v $
 * $Revision: 1.4 $
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
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_scrgbClr:   // CT_ScRgbColor
    {
        sal_Int32 r = ((xAttribs->getOptionalValue( XML_r ).toInt32() * 256) / 1000) & 0xff;
        sal_Int32 g = ((xAttribs->getOptionalValue( XML_g ).toInt32() * 256) / 1000) & 0xff;
        sal_Int32 b = ((xAttribs->getOptionalValue( XML_b ).toInt32() * 256) / 1000) & 0xff;
        mrColor.mnColor = (r << 16) | (g << 8) | b;
        mrColor.mbUsed = sal_True;
        break;
    }
    case NMSP_DRAWINGML|XML_srgbClr:    // CT_SRgbColor
    {
        mrColor.mnColor = xAttribs->getOptionalValue( XML_val ).toInt32( 16 );
        mrColor.mbUsed = sal_True;
        break;
    }
    case NMSP_DRAWINGML|XML_hslClr: // CT_HslColor
        {
        sal_uInt8 r = 0;
        sal_uInt8 g = 0;
        sal_uInt8 b = 0;
        double fH = xAttribs->getOptionalValue( XML_hue ).toInt32() / ( 60000.0 * 360.0 );
        double fL = xAttribs->getOptionalValue( XML_lum ).toInt32() / 100000.0;
        double fS = xAttribs->getOptionalValue( XML_sat ).toInt32() / 100000.0;
        oox::drawingml::Color::HSLtoRGB( fH, fS, fL, r, g, b );
        mrColor.mnColor = (r << 16) | (g << 8) | b;
        mrColor.mbUsed = sal_True;
        break;
        }
    case NMSP_DRAWINGML|XML_sysClr: // CT_SystemColor
        sal_Int32 nColor;
        if( !ClrScheme::getSystemColor( xAttribs->getOptionalValueToken( XML_val, XML_TOKEN_INVALID ), nColor ) )
            nColor = xAttribs->getOptionalValue( XML_lastClr ).toInt32( 16 );
        mrColor.mnColor = nColor;
        mrColor.mbUsed = sal_True;
        break;
    case NMSP_DRAWINGML|XML_schemeClr:  // CT_SchemeColor
    {
        mrColor.mnColor = xAttribs->getOptionalValueToken( XML_val, XML_nothing );
        mrColor.mbUsed = sal_True;
        mrColor.mbSchemeColor = sal_True;
        break;
    }
    case NMSP_DRAWINGML|XML_prstClr:    // CT_PresetColor
        // todo
        break;

    }
}

Reference< XFastContextHandler > colorChoiceContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    // colorTransformGroup

    // color should be available as rgb in member mnColor already, now modify it depending on
    // the transformation elements

    switch( aElementToken )
    {
/* TODO
    case NMSP_DRAWINGML|XML_tint:       // CT_PositiveFixedPercentage
    case NMSP_DRAWINGML|XML_shade:      // CT_PositiveFixedPercentage
    case NMSP_DRAWINGML|XML_comp:       // CT_ComplementTransform
    case NMSP_DRAWINGML|XML_inv:        // CT_InverseTransform
    case NMSP_DRAWINGML|XML_gray:       // CT_GrayscaleTransform
    case NMSP_DRAWINGML|XML_alphaOff:   // CT_FixedPercentage
    case NMSP_DRAWINGML|XML_alphaMod:   // CT_PositivePercentage
    break;
*/
    case NMSP_DRAWINGML|XML_alpha:      // CT_PositiveFixedPercentage
        {
            mrColor.mbUsed = sal_True;
            mrColor.mbAlphaColor = sal_True;
            mrColor.mnAlpha = GetPercent( rxAttribs->getOptionalValue( XML_val ) );
        }
    break;
    case NMSP_DRAWINGML|XML_hue:        // CT_PositiveFixedAngle
    case NMSP_DRAWINGML|XML_hueOff: // CT_Angle
    case NMSP_DRAWINGML|XML_hueMod: // CT_PositivePercentage
    case NMSP_DRAWINGML|XML_sat:        // CT_Percentage
    case NMSP_DRAWINGML|XML_satOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_satMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_lum:        // CT_Percentage
    case NMSP_DRAWINGML|XML_lumOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_lumMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_red:        // CT_Percentage
    case NMSP_DRAWINGML|XML_redOff: // CT_Percentage
    case NMSP_DRAWINGML|XML_redMod: // CT_Percentage
    case NMSP_DRAWINGML|XML_green:      // CT_Percentage
    case NMSP_DRAWINGML|XML_greenOff:   // CT_Percentage
    case NMSP_DRAWINGML|XML_greenMod:   // CT_Percentage
    case NMSP_DRAWINGML|XML_blue:       // CT_Percentage
    case NMSP_DRAWINGML|XML_blueOff:    // CT_Percentage
    case NMSP_DRAWINGML|XML_blueMod:    // CT_Percentage
        {
            sal_Int32 nToken = aElementToken&(~NMSP_MASK);
            ColorTransformation aColorTransformation( nToken, rxAttribs->getOptionalValue( XML_val ).toInt32() );
            mrColor.maColorTransformation.push_back( aColorTransformation );
        }
        break;
    }
    return this;
}


} }
