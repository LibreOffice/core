/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:38:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_COLOR_HXX
#define OOX_DRAWINGML_COLOR_HXX

#include <boost/shared_ptr.hpp>
#include "oox/drawingml/clrscheme.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include <map>

namespace oox { namespace drawingml {

class Color;
class colorChoiceContext;
class clrChangeContext;

typedef boost::shared_ptr< Color > ColorPtr;

struct ColorTransformation
{
    sal_Int32 mnToken;
    sal_Int32 mnValue;
    ColorTransformation( sal_Int32 nToken, sal_Int32 nValue ) : mnToken( nToken ), mnValue( nValue ) {}
};

class Color
{

    friend class oox::drawingml::colorChoiceContext;
    friend class oox::drawingml::clrChangeContext;

public:

    Color();
    ~Color();

    sal_Int32   getColor( const oox::core::XmlFilterBase& rFilterBase ) const;

    sal_Bool    hasAlpha() const { return mbAlphaColor; };
    sal_Int32   getAlpha() const;

    sal_Bool    isUsed() const { return mbUsed; };

    //HSL values = 0 ÷ 1
    //RGB values = 0 ÷ 255
    static void RGBtoHSL( sal_uInt8 R, sal_uInt8 G, sal_uInt8 B, double& H, double& S, double& L );
    static void HSLtoRGB( double H, double S, double L, sal_uInt8& R, sal_uInt8& G, sal_uInt8& B );

private:

    sal_Int32       mnColor;
    sal_Int32       mnAlpha;
    sal_Bool        mbUsed;
    sal_Bool        mbSchemeColor;
    sal_Bool        mbAlphaColor;

    std::vector< ColorTransformation > maColorTransformation;
};

} }

#endif  //  OOX_DRAWINGML_COLOR_HXX
