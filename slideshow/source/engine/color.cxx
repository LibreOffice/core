/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <hslcolor.hxx>
#include <rgbcolor.hxx>

#include <basegfx/numeric/ftools.hxx>

#include <cmath> // for fmod
#include <algorithm>


namespace slideshow
{
    namespace internal
    {
        namespace
        {
            // helper functions
            // ================

            double getMagic( double nLuminance, double nSaturation )
            {
                if( nLuminance <= 0.5 )
                    return nLuminance*(1.0 + nSaturation);
                else
                    return nLuminance + nSaturation - nLuminance*nSaturation;
            }

            HSLColor::HSLTriple rgb2hsl( double nRed, double nGreen, double nBlue )
            {
                // r,g,b in [0,1], h in [0,360] and s,l in [0,1]
                HSLColor::HSLTriple aRes;

                const double nMax( ::std::max(nRed,::std::max(nGreen, nBlue)) );
                const double nMin( ::std::min(nRed,::std::min(nGreen, nBlue)) );

                const double nDelta( nMax - nMin );

                aRes.mnLuminance = (nMax + nMin) / 2.0;

                if( ::basegfx::fTools::equalZero( nDelta ) )
                {
                    aRes.mnSaturation = 0.0;

                    // hue undefined (achromatic case)
                    aRes.mnHue = 0.0;
                }
                else
                {
                    aRes.mnSaturation = aRes.mnLuminance > 0.5 ?
                        nDelta/(2.0-nMax-nMin) :
                        nDelta/(nMax + nMin);

                    if( nRed == nMax )
                        aRes.mnHue = (nGreen - nBlue)/nDelta;
                    else if( nGreen == nMax )
                        aRes.mnHue = 2.0 + (nBlue - nRed)/nDelta;
                    else if( nBlue == nMax )
                        aRes.mnHue = 4.0 + (nRed - nGreen)/nDelta;

                    aRes.mnHue *= 60.0;

                    if( aRes.mnHue < 0.0 )
                        aRes.mnHue += 360.0;
                }

                return aRes;
            }

            double hsl2rgbHelper( double nValue1, double nValue2, double nHue )
            {
                // clamp hue to [0,360]
                nHue = fmod( nHue, 360.0 );

                // cope with wrap-arounds
                if( nHue < 0.0 )
                    nHue += 360.0;

                if( nHue < 60.0 )
                    return nValue1 + (nValue2 - nValue1)*nHue/60.0;
                else if( nHue < 180.0 )
                    return nValue2;
                else if( nHue < 240.0 )
                    return nValue1 + (nValue2 - nValue1)*(240.0 - nHue)/60.0;
                else
                    return nValue1;
            }

            RGBColor::RGBTriple hsl2rgb( double nHue, double nSaturation, double nLuminance )
            {
                if( ::basegfx::fTools::equalZero( nSaturation ) )
                    return RGBColor::RGBTriple(0.0, 0.0, nLuminance );

                const double nVal1( getMagic(nLuminance, nSaturation) );
                const double nVal2( 2.0*nLuminance - nVal1 );

                RGBColor::RGBTriple aRes;

                aRes.mnRed = hsl2rgbHelper( nVal2,
                                            nVal1,
                                            nHue + 120.0 );
                aRes.mnGreen = hsl2rgbHelper( nVal2,
                                              nVal1,
                                              nHue );
                aRes.mnBlue = hsl2rgbHelper( nVal2,
                                             nVal1,
                                             nHue - 120.0 );

                return aRes;
            }

            /// Truncate range of value to [0,1]
            double truncateRangeStd( double nVal )
            {
                return ::std::max( 0.0,
                                   ::std::min( 1.0,
                                               nVal ) );
            }

            /// Truncate range of value to [0,360]
            double truncateRangeHue( double nVal )
            {
                return ::std::max( 0.0,
                                   ::std::min( 360.0,
                                               nVal ) );
            }

            /// convert RGB color to sal_uInt8, truncate range appropriately before
            sal_uInt8 colorToInt( double nCol )
            {
                return static_cast< sal_uInt8 >(
                    ::basegfx::fround( truncateRangeStd( nCol ) * 255.0 ) );
            }
        }



        // HSLColor
        // ===============================================

        HSLColor::HSLTriple::HSLTriple() :
            mnHue(),
            mnSaturation(),
            mnLuminance()
        {
        }

        HSLColor::HSLTriple::HSLTriple( double nHue, double nSaturation, double nLuminance ) :
            mnHue( nHue ),
            mnSaturation( nSaturation ),
            mnLuminance( nLuminance )
        {
        }

        HSLColor::HSLColor() :
            maHSLTriple( 0.0, 0.0, 0.0 ),
            mnMagicValue( getMagic( maHSLTriple.mnLuminance,
                                    maHSLTriple.mnSaturation ) )
        {
        }

        HSLColor::HSLColor( ::cppcanvas::Color::IntSRGBA nRGBColor ) :
            maHSLTriple( rgb2hsl( ::cppcanvas::getRed( nRGBColor ) / 255.0,
                                  ::cppcanvas::getGreen( nRGBColor ) / 255.0,
                                  ::cppcanvas::getBlue( nRGBColor ) / 255.0 ) ),
            mnMagicValue( getMagic( maHSLTriple.mnLuminance,
                                    maHSLTriple.mnSaturation ) )
        {
        }

        HSLColor::HSLColor( double nHue, double nSaturation, double nLuminance ) :
            maHSLTriple( nHue, nSaturation, nLuminance ),
            mnMagicValue( getMagic( maHSLTriple.mnLuminance,
                                    maHSLTriple.mnSaturation ) )
        {
        }

        HSLColor::HSLColor( const RGBColor& rColor ) :
            maHSLTriple( rgb2hsl( truncateRangeStd( rColor.getRed() ),
                                  truncateRangeStd( rColor.getGreen() ),
                                  truncateRangeStd( rColor.getBlue() ) ) ),
            mnMagicValue( getMagic( maHSLTriple.mnLuminance,
                                    maHSLTriple.mnSaturation ) )
        {
        }

        double HSLColor::getHue() const
        {
            return maHSLTriple.mnHue;
        }

        double HSLColor::getSaturation() const
        {
            return maHSLTriple.mnSaturation;
        }

        double HSLColor::getLuminance() const
        {
            return maHSLTriple.mnLuminance;
        }

        double HSLColor::getRed() const
        {
            if( ::basegfx::fTools::equalZero( getSaturation() ) )
                return getLuminance();

            return hsl2rgbHelper( 2.0*getLuminance() - mnMagicValue,
                                  mnMagicValue,
                                  getHue() + 120.0 );
        }

        double HSLColor::getGreen() const
        {
            if( ::basegfx::fTools::equalZero( getSaturation() ) )
                return getLuminance();

            return hsl2rgbHelper( 2.0*getLuminance() - mnMagicValue,
                                  mnMagicValue,
                                  getHue() );
        }

        double HSLColor::getBlue() const
        {
            if( ::basegfx::fTools::equalZero( getSaturation() ) )
                return getLuminance();

            return hsl2rgbHelper( 2.0*getLuminance() - mnMagicValue,
                                  mnMagicValue,
                                  getHue() - 120.0 );
        }

        RGBColor HSLColor::getRGBColor() const
        {
            RGBColor::RGBTriple aColor( hsl2rgb( getHue(),
                                                 getSaturation(),
                                                 getLuminance() ) );
            return RGBColor( aColor.mnRed, aColor.mnGreen, aColor.mnBlue );
        }

        RGBColor::RGBColor(const RGBColor& rLHS)
        {
            maRGBTriple.mnRed = rLHS.getRed();
            maRGBTriple.mnGreen = rLHS.getGreen();
            maRGBTriple.mnBlue = rLHS.getBlue();
        }

        RGBColor& RGBColor::operator=( const RGBColor& rLHS ){

            maRGBTriple.mnRed = rLHS.getRed();
            maRGBTriple.mnGreen = rLHS.getGreen();
            maRGBTriple.mnBlue = rLHS.getBlue();
            return *this;
        }

        HSLColor operator+( const HSLColor& rLHS, const HSLColor& rRHS )
        {
            return HSLColor( rLHS.getHue() + rRHS.getHue(),
                             rLHS.getSaturation() + rRHS.getSaturation(),
                             rLHS.getLuminance() + rRHS.getLuminance() );
        }

        HSLColor operator*( const HSLColor& rLHS, const HSLColor& rRHS )
        {
            return HSLColor( rLHS.getHue() * rRHS.getHue(),
                             rLHS.getSaturation() * rRHS.getSaturation(),
                             rLHS.getLuminance() * rRHS.getLuminance() );
        }

        HSLColor operator*( double nFactor, const HSLColor& rRHS )
        {
            return HSLColor( nFactor * rRHS.getHue(),
                             nFactor * rRHS.getSaturation(),
                             nFactor * rRHS.getLuminance() );
        }

        HSLColor interpolate( const HSLColor& rFrom, const HSLColor& rTo, double t, bool bCCW )
        {
            const double nFromHue( rFrom.getHue() );
            const double nToHue  ( rTo.getHue()   );

            double nHue=0.0;

            if( nFromHue <= nToHue && !bCCW )
            {
                // interpolate hue clockwise. That is, hue starts at
                // high values and ends at low ones. Therefore, we
                // must 'cross' the 360 degrees and start at low
                // values again (imagine the hues to lie on the
                // circle, where values above 360 degrees are mapped
                // back to [0,360)).
                nHue = (1.0-t)*(nFromHue + 360.0) + t*nToHue;
            }
            else if( nFromHue > nToHue && bCCW )
            {
                // interpolate hue counter-clockwise. That is, hue
                // starts at high values and ends at low
                // ones. Therefore, we must 'cross' the 360 degrees
                // and start at low values again (imagine the hues to
                // lie on the circle, where values above 360 degrees
                // are mapped back to [0,360)).
                nHue = (1.0-t)*nFromHue + t*(nToHue + 360.0);
            }
            else
            {
                // interpolate hue counter-clockwise. That is, hue
                // starts at low values and ends at high ones (imagine
                // the hue value as degrees on a circle, with
                // increasing values going counter-clockwise)
                nHue = (1.0-t)*nFromHue + t*nToHue;
            }

            return HSLColor( nHue,
                             (1.0-t)*rFrom.getSaturation() + t*rTo.getSaturation(),
                             (1.0-t)*rFrom.getLuminance() + t*rTo.getLuminance() );
        }



        // RGBColor
        // ===============================================


        RGBColor::RGBTriple::RGBTriple() :
            mnRed(),
            mnGreen(),
            mnBlue()
        {
        }

        RGBColor::RGBTriple::RGBTriple( double nRed, double nGreen, double nBlue ) :
            mnRed( nRed ),
            mnGreen( nGreen ),
            mnBlue( nBlue )
        {
        }

        RGBColor::RGBColor() :
            maRGBTriple( 0.0, 0.0, 0.0 )
        {
        }

        RGBColor::RGBColor( ::cppcanvas::Color::IntSRGBA nRGBColor ) :
            maRGBTriple( ::cppcanvas::getRed( nRGBColor ) / 255.0,
                         ::cppcanvas::getGreen( nRGBColor ) / 255.0,
                         ::cppcanvas::getBlue( nRGBColor ) / 255.0 )
        {
        }

        RGBColor::RGBColor( double nRed, double nGreen, double nBlue ) :
            maRGBTriple( nRed, nGreen, nBlue )
        {
        }

        RGBColor::RGBColor( const HSLColor& rColor ) :
            maRGBTriple( hsl2rgb( truncateRangeHue( rColor.getHue() ),
                                  truncateRangeStd( rColor.getSaturation() ),
                                  truncateRangeStd( rColor.getLuminance() ) ) )
        {
        }

        double RGBColor::getHue() const
        {
            return rgb2hsl( getRed(),
                            getGreen(),
                            getBlue() ).mnHue;
        }

        double RGBColor::getSaturation() const
        {
            return rgb2hsl( getRed(),
                            getGreen(),
                            getBlue() ).mnSaturation;
        }

        double RGBColor::getLuminance() const
        {
            return rgb2hsl( getRed(),
                            getGreen(),
                            getBlue() ).mnLuminance;
        }

        double RGBColor::getRed() const
        {
            return maRGBTriple.mnRed;
        }

        double RGBColor::getGreen() const
        {
            return maRGBTriple.mnGreen;
        }

        double RGBColor::getBlue() const
        {
            return maRGBTriple.mnBlue;
        }

        HSLColor RGBColor::getHSLColor() const
        {
            HSLColor::HSLTriple aColor( rgb2hsl( getRed(),
                                                 getGreen(),
                                                 getBlue() ) );
            return HSLColor( aColor.mnHue, aColor.mnSaturation, aColor.mnLuminance );
        }

        ::cppcanvas::Color::IntSRGBA RGBColor::getIntegerColor() const
        {
            return ::cppcanvas::makeColor( colorToInt( getRed() ),
                                           colorToInt( getGreen() ),
                                           colorToInt( getBlue() ),
                                           255 );
        }

        RGBColor operator+( const RGBColor& rLHS, const RGBColor& rRHS )
        {
            return RGBColor( rLHS.getRed() + rRHS.getRed(),
                             rLHS.getGreen() + rRHS.getGreen(),
                             rLHS.getBlue() + rRHS.getBlue() );
        }

        RGBColor operator*( const RGBColor& rLHS, const RGBColor& rRHS )
        {
            return RGBColor( rLHS.getRed() * rRHS.getRed(),
                             rLHS.getGreen() * rRHS.getGreen(),
                             rLHS.getBlue() * rRHS.getBlue() );
        }

        RGBColor operator*( double nFactor, const RGBColor& rRHS )
        {
            return RGBColor( nFactor * rRHS.getRed(),
                             nFactor * rRHS.getGreen(),
                             nFactor * rRHS.getBlue() );
        }

        RGBColor interpolate( const RGBColor& rFrom, const RGBColor& rTo, double t )
        {
            return RGBColor( (1.0-t)*rFrom.getRed() + t*rTo.getRed(),
                             (1.0-t)*rFrom.getGreen() + t*rTo.getGreen(),
                             (1.0-t)*rFrom.getBlue() + t*rTo.getBlue() );
        }
    }
}
