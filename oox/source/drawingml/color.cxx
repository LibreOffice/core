/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:17:03 $
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

#include "oox/drawingml/color.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include <algorithm>

using namespace ::oox::core;

namespace oox { namespace drawingml {

Color::Color()
: mnColor( 0 )
, mnAlpha( 0 )
, mbUsed( sal_False )
, mbSchemeColor( sal_False )
, mbAlphaColor( sal_False )
{
}
Color::~Color()
{
}

const sal_uInt8 setAbs( const sal_Int32& nValue )
{
    sal_uInt32 n = static_cast< sal_uInt32 >( ( ( nValue / 100000.0 ) * 255.0 ) + 0.5 );
    if ( n > 255 )
        n = 255;
    return static_cast< sal_uInt8 >( n );
}

const sal_uInt8 setOff( const sal_uInt8& nSource, const sal_Int32& nValue )
{
    sal_uInt32 n = setAbs( nValue );
    n += nSource;
    if ( n > 255 )
        n = 255;
    return static_cast< sal_uInt8 >( n );
}

const sal_uInt8 setMod( const sal_uInt8& nSource, const sal_Int32& nValue )
{
    sal_Int32 nVal = static_cast< sal_Int32 >( ( nSource * nValue ) / 255.0 );
    return setAbs( nVal );
}

sal_Int32 Color::getColor( const oox::core::XmlFilterBase& rFilterBase ) const
{
    sal_Int32 nColor = mbSchemeColor ? rFilterBase.getSchemeClr( mnColor ) : mnColor;
    if ( maColorTransformation.size() )
    {
        sal_uInt8 nR = static_cast< sal_uInt8 >( nColor );
        sal_uInt8 nG = static_cast< sal_uInt8 >( nColor >> 8 );
        sal_uInt8 nB = static_cast< sal_uInt8 >( nColor >> 16 );

        std::vector< ColorTransformation >::const_iterator aIter( maColorTransformation.begin() );
        while( aIter != maColorTransformation.end() )
        {
            sal_Int32 nValue = aIter->mnValue;
            switch( aIter->mnToken )
            {
                case XML_red:       // CT_Percentage
                {
                    nR = setAbs( nValue );
                }
                break;
                case XML_redOff:    // CT_Percentage
                {
                    nR = setOff( nR, nValue );
                }
                break;
                case XML_redMod:    // CT_Percentage
                {
                    nR = setMod( nR, nValue );
                }
                break;
                case XML_green:     // CT_Percentage
                {
                    nG = setAbs( nValue );
                }
                break;
                case XML_greenOff:  // CT_Percentage
                {
                    nG = setOff( nG, nValue );
                }
                break;
                case XML_greenMod:  // CT_Percentage
                {
                    nG = setMod( nG, nValue );
                }
                break;
                case XML_blue:      // CT_Percentage
                {
                    nB = setAbs( nValue );
                }
                break;
                case XML_blueOff:   // CT_Percentage
                {
                    nB = setOff( nB, nValue );
                }
                break;
                case XML_blueMod:   // CT_Percentage
                {
                    nB = setMod( nB, nValue );
                }
                break;
            }
            aIter++;
        }

        double fH = 0;
        double fS = 0;
        double fL = 0;

        RGBtoHSL( nR, nG, nB, fH, fS, fL );

        aIter = maColorTransformation.begin();
        while( aIter != maColorTransformation.end() )
        {
            sal_Int32 nValue = aIter->mnValue;
            switch( aIter->mnToken )
            {
                case XML_sat:       // CT_Percentage
                {
                    fS = nValue / 100000.0;
                }
                break;
                case XML_satOff:    // CT_Percentage
                {
                    fS += nValue / 100000.0;
                }
                break;
                case XML_satMod:    // CT_Percentage
                {
                    fS *= nValue / 100000.0;
                }
                break;
                case XML_hue:       // CT_PositiveFixedAngle
                {
                    fH = nValue / ( 60000.0 * 360.0 );
                }
                break;
                case XML_hueOff:    // CT_Angle
                {
                    fH += nValue / ( 60000.0 * 360.0 );
                }
                break;
                case XML_hueMod:    // CT_PositivePercentage
                {
                    fH *= nValue / 100000.0;
                }
                break;
                case XML_lum:       // CT_Percentage
                {
                    fL = nValue / 100000.0;
                }
                break;
                case XML_lumOff:    // CT_Percentage
                {
                    fL += nValue / 100000.0;
                }
                break;
                case XML_lumMod:    // CT_Percentage
                {
                    fL *= nValue / 100000.0;
                }
                break;
            }
            aIter++;
        }

        HSLtoRGB( fH, fS, fL, nR, nG, nB );
        nColor = nB;
        nColor <<= 8;
        nColor |= nG;
        nColor <<= 8;
        nColor |= nR;
    }
    return nColor;
}
sal_Int32 Color::getAlpha() const
{
    return mnAlpha;
}

void Color::RGBtoHSL( sal_uInt8 R, sal_uInt8 G, sal_uInt8 B, double& H, double& S, double& L )
{
    double fR =  R / 255.0;                 //RGB 0 ÷ 255
    double fG =  G / 255.0;                 //HSL results = 0 ÷ 1
    double fB =  B / 255.0;

    double fMin = std::min( std::min( fR, fG ), fB );
    double fMax = std::max( std::max( fR, fG ), fB );
    double fdMax = fMax - fMin;

    L = ( fMax + fMin ) / 2.0;

    if ( fdMax == 0 )
    {
       H = 0;
       S = 0;
    }
    else
    {
       if ( L < 0.5 )
           S = fdMax / ( fMax + fMin );
       else
           S = fdMax / ( 2 - fMax - fMin );

       double fdR = ( ( ( fMax - fR ) / 6.0 ) + ( fdMax / 2.0 ) ) / fdMax;
       double fdG = ( ( ( fMax - fG ) / 6.0 ) + ( fdMax / 2.0 ) ) / fdMax;
       double fdB = ( ( ( fMax - fB ) / 6.0 ) + ( fdMax / 2.0 ) ) / fdMax;

       if ( fR == fMax )
           H = fdB - fdG;
       else if ( fG == fMax )
           H = ( 1.0 / 3.0 ) + fdR - fdB;
       else if ( fB == fMax )
           H = ( 2.0 / 3.0 ) + fdG - fdR;

       if ( H < 0 )
           H++;
       if ( H > 1 )
           H--;
    }
}

static double Hue_2_RGB( double v1, double v2, double vH )
{
   if ( vH < 0 )
       vH += 1;
   if ( vH > 1 )
       vH -= 1;
   if ( ( 6 * vH ) < 1 )
       return ( v1 + ( v2 - v1 ) * 6 * vH );
   if ( ( 2 * vH ) < 1 )
       return ( v2 );
   if ( ( 3 * vH ) < 2 )
       return ( v1 + ( v2 - v1 ) * ( ( 2.0 / 3.0 ) - vH ) * 6 );
   return v1;
}

void Color::HSLtoRGB( double H, double S, double L, sal_uInt8& R, sal_uInt8& G, sal_uInt8& B )
{
    if ( S == 0 )
        R = G = B = static_cast< sal_uInt8 >( L * 255.0 );
    else
    {
        double var_2 = ( L < 0.5 ) ? L * ( 1 + S ) : ( L + S ) - ( S * L );
        double var_1 = 2 * L - var_2;
        R = static_cast< sal_uInt8 >( 255.0 * Hue_2_RGB( var_1, var_2, H + ( 1.0 / 3.0 ) ) );
        G = static_cast< sal_uInt8 >( 255.0 * Hue_2_RGB( var_1, var_2, H ) );
        B = static_cast< sal_uInt8 >( 255.0 * Hue_2_RGB( var_1, var_2, H - ( 1.0 / 3.0 ) ) );
    }
}

} }
