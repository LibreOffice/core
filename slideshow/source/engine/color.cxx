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


#include <hslcolor.hxx>
#include <rgbcolor.hxx>

#include <basegfx/numeric/ftools.hxx>

#include <cmath>
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





        bool operator==( const HSLColor& rLHS, const HSLColor& rRHS )
        {
            return ( rLHS.getHue() == rRHS.getHue() &&
                     rLHS.getSaturation() == rRHS.getSaturation() &&
                     rLHS.getLuminance() == rRHS.getLuminance() );
        }

        bool operator!=( const HSLColor& rLHS, const HSLColor& rRHS )
        {
            return !( rLHS == rRHS );
        }

        HSLColor operator+( const HSLColor& rLHS, const HSLColor& rRHS )
        {
            return HSLColor( rLHS.getHue() + rRHS.getHue(),
                             rLHS.getSaturation() + rRHS.getSaturation(),
                             rLHS.getLuminance() + rRHS.getLuminance() );
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




        ::cppcanvas::Color::IntSRGBA RGBColor::getIntegerColor() const
        {
            return ::cppcanvas::makeColor( colorToInt( getRed() ),
                                           colorToInt( getGreen() ),
                                           colorToInt( getBlue() ),
                                           255 );
        }

        bool operator==( const RGBColor& rLHS, const RGBColor& rRHS )
        {
            return ( rLHS.getRed() == rRHS.getRed() &&
                     rLHS.getGreen() == rRHS.getGreen() &&
                     rLHS.getBlue() == rRHS.getBlue() );
        }

        bool operator!=( const RGBColor& rLHS, const RGBColor& rRHS )
        {
            return !( rLHS == rRHS );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
