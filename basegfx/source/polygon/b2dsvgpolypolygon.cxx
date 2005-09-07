/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dsvgpolypolygon.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:47:54 $
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

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

namespace basegfx
{
    namespace tools
    {
        namespace
        {
            void lcl_skipSpaces(sal_Int32&              io_rPos,
                                const ::rtl::OUString&  rStr,
                                const sal_Int32         nLen)
            {
                while( io_rPos < nLen &&
                       sal_Unicode(' ') == rStr[io_rPos] )
                {
                    ++io_rPos;
                }
            }

            void lcl_skipSpacesAndCommas(sal_Int32&             io_rPos,
                                         const ::rtl::OUString& rStr,
                                         const sal_Int32        nLen)
            {
                while(io_rPos < nLen
                      && (sal_Unicode(' ') == rStr[io_rPos] || sal_Unicode(',') == rStr[io_rPos]))
                {
                    ++io_rPos;
                }
            }

            bool lcl_isOnNumberChar(const ::rtl::OUString& rStr, const sal_Int32 nPos, bool bSignAllowed = true)
            {
                const sal_Unicode aChar(rStr[nPos]);

                const bool bPredicate( (sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                                       || (bSignAllowed && sal_Unicode('+') == aChar)
                                       || (bSignAllowed && sal_Unicode('-') == aChar) );

                return bPredicate;
            }

            bool lcl_getDoubleChar(double&                  o_fRetval,
                                   sal_Int32&               io_rPos,
                                   const ::rtl::OUString&   rStr,
                                   const sal_Int32          nLen)
            {
                sal_Unicode aChar( rStr[io_rPos] );
                ::rtl::OUStringBuffer sNumberString;

                if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    aChar = rStr[++io_rPos];
                }

                while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                      || sal_Unicode('.') == aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    aChar = rStr[++io_rPos];
                }

                if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    aChar = rStr[++io_rPos];

                    if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                    {
                        sNumberString.append(rStr[io_rPos]);
                        aChar = rStr[++io_rPos];
                    }

                    while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                    {
                        sNumberString.append(rStr[io_rPos]);
                        aChar = rStr[++io_rPos];
                    }
                }

                if(sNumberString.getLength())
                {
                    rtl_math_ConversionStatus eStatus;
                    o_fRetval = ::rtl::math::stringToDouble( sNumberString.makeStringAndClear(),
                                                             (sal_Unicode)('.'),
                                                             (sal_Unicode)(','),
                                                             &eStatus,
                                                             NULL );
                    return ( eStatus == rtl_math_ConversionStatus_Ok );
                }

                return false;
            }

            bool lcl_importDoubleAndSpaces( double&                 o_fRetval,
                                            sal_Int32&              io_rPos,
                                            const ::rtl::OUString&  rStr,
                                            const sal_Int32         nLen )
            {
                if( !lcl_getDoubleChar(o_fRetval, io_rPos, rStr, nLen) )
                    return false;

                lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);

                return true;
            }

            void lcl_skipNumber(sal_Int32&              io_rPos,
                                const ::rtl::OUString&  rStr,
                                const sal_Int32         nLen)
            {
                bool bSignAllowed(true);

                while(io_rPos < nLen && lcl_isOnNumberChar(rStr, io_rPos, bSignAllowed))
                {
                    bSignAllowed = false;
                    ++io_rPos;
                }
            }

            void lcl_skipDouble(sal_Int32&              io_rPos,
                                const ::rtl::OUString&  rStr,
                                const sal_Int32         nLen)
            {
                sal_Unicode aChar( rStr[io_rPos] );

                if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                    aChar = rStr[++io_rPos];

                while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                      || sal_Unicode('.') == aChar)
                {
                    aChar = rStr[++io_rPos];
                }

                if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
                {
                    aChar = rStr[++io_rPos];

                    if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                        aChar = rStr[++io_rPos];

                    while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                    {
                        aChar = rStr[++io_rPos];
                    }
                }
            }
            void lcl_skipNumberAndSpacesAndCommas(sal_Int32&                io_rPos,
                                                  const ::rtl::OUString&    rStr,
                                                  const sal_Int32           nLen)
            {
                lcl_skipNumber(io_rPos, rStr, nLen);
                lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);
            }

            // #100617# Allow to skip doubles, too.
            void lcl_skipDoubleAndSpacesAndCommas(sal_Int32&                io_rPos,
                                                  const ::rtl::OUString&    rStr,
                                                  const sal_Int32           nLen)
            {
                lcl_skipDouble(io_rPos, rStr, nLen);
                lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);
            }
        }

        bool importFromSvgD( ::basegfx::B2DPolyPolygon& o_rPolyPoly,
                             const ::rtl::OUString&     rSvgDStatement )
        {
            const sal_Int32 nLen( rSvgDStatement.getLength() );
            sal_Int32       nPos(0);
            B2DPolygon      aCurrPoly;

            lcl_skipSpaces(nPos, rSvgDStatement, nLen);

            // #104076# reset closed flag for next to be started polygon
            bool bIsClosed = false;

            double nLastX( 0.0 );
            double nLastY( 0.0 );

            while( nPos < nLen )
            {
                bool bRelative(false);
                bool bMoveTo(false);

                const sal_Unicode aCurrChar( rSvgDStatement[nPos] );

                switch( aCurrChar )
                {
                    case 'z' :
                    case 'Z' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        // #104076# remember closed state of current polygon
                        bIsClosed = true;
                        break;
                    }

                    case 'm' :
                    case 'M' :
                    {
                        bMoveTo = true;
                        // FALLTHROUGH intended
                    }
                    case 'l' :
                    case 'L' :
                    {
                        if( aCurrChar == 'm' || aCurrChar == 'l' )
                            bRelative = true;

                        if( bMoveTo )
                        {
                            if( aCurrPoly.count() )
                            {
                                // #104076# end-process current poly
                                aCurrPoly.setClosed( bIsClosed );

                                // reset closed flag for next to be started polygon
                                bIsClosed = false;

                                // next poly
                                o_rPolyPoly.append( aCurrPoly );

                                aCurrPoly.clear();
                            }
                        }

                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    case 'h' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'H' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY( nLastY );

                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                                nX += nLastX;

                            // set last position
                            nLastX = nX;

                            // add point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    case 'v' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'V' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX( nLastX ), nY;

                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                                nY += nLastY;

                            // set last position
                            nLastY = nY;

                            // add point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    case 's' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'S' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX2, nY2;

                            if( !lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                            {
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // set control points/vectors
                            sal_uInt32 nNumEntries( aCurrPoly.count() );

                            if( nNumEntries == 0 )
                            {
                                // ensure valid prev point - if none
                                // was explicitely added, take last
                                // position
                                aCurrPoly.append( B2DPoint( nLastX, nLastY ) );
                                ++nNumEntries;
                            }

                            if( nNumEntries > 1 )
                            {
                                // set reflected control
                                // vector.
                                aCurrPoly.setControlVectorA( nNumEntries-1,
                                                             -aCurrPoly.getControlVectorB( nNumEntries-2 ) );
                            }
                            // as required in the SVG spec, if there's
                            // no previous control point, the implicit
                            // control point shall be coincident with
                            // the current point. That means, there's
                            // effectively a zero control vector B at
                            // index nNumEntries-2

                            aCurrPoly.setControlPointB( nNumEntries-1,
                                                        B2DPoint( nX2, nY2 ) );

                            // set last position
                            if( !nNumEntries )
                            {
                                aCurrPoly.append( B2DPoint( nLastX, nLastY ) );
                                ++nNumEntries;
                            }
                            nLastX = nX;
                            nLastY = nY;

                            // add point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    case 'c' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'C' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;
                            double nX2, nY2;

                            if( !lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure at least one previous point (to
                            // set the control vector at)
                            sal_uInt32 nNumEntries( aCurrPoly.count() );

                            if( nNumEntries == 0 )
                            {
                                // ensure valid prev point - if none
                                // was explicitely added, take last
                                // position
                                aCurrPoly.append( B2DPoint( nLastX, nLastY ) );
                                ++nNumEntries;
                            }

                            aCurrPoly.setControlPointA( nNumEntries-1,
                                                        B2DPoint( nX1, nY1 ) );
                            aCurrPoly.setControlPointB( nNumEntries-1,
                                                        B2DPoint( nX2, nY2 ) );

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add new point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    // #100617# quadratic beziers are imported as cubic ones
                    case 'q' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'Q' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;

                            if( !lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // calculate the cubic bezier coefficients from the quadratic ones
                            const double nX1Prime( (nX1*2.0 + nLastX) / 3.0 );
                            const double nY1Prime( (nY1*2.0 + nLastY) / 3.0 );
                            const double nX2Prime( (nX1*2.0 + nX) / 3.0 );
                            const double nY2Prime( (nY1*2.0 + nY) / 3.0 );

                            sal_uInt32 nNumEntries( aCurrPoly.count() );

                            // ensure at least one previous point (to
                            // set the control vector at)
                            if( nNumEntries == 0 )
                            {
                                // ensure valid prev point - if none
                                // was explicitely added, take last
                                // position
                                aCurrPoly.append( B2DPoint( nLastX, nLastY ) );
                                ++nNumEntries;
                            }

                            aCurrPoly.setControlPointA( nNumEntries-1,
                                                        B2DPoint( nX1Prime, nY1Prime ) );
                            aCurrPoly.setControlPointB( nNumEntries-1,
                                                        B2DPoint( nX2Prime, nY2Prime ) );

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add new point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    // #100617# relative quadratic beziers are imported as cubic
                    case 't' :
                    {
                        bRelative = true;
                        // FALLTHROUGH intended
                    }
                    case 'T' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if( !lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen) )
                                return false;
                            if( !lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen) )
                                return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // calc control points/vectors. Since the
                            // previous curve segment (if any) was
                            // already converted to cubic, we can
                            // simply take controlVectorB and apply it
                            // without further adaptions.
                            sal_uInt32 nNumEntries( aCurrPoly.count() );

                            if( nNumEntries == 0 )
                            {
                                // ensure valid prev point - if none
                                // was explicitely added, take last
                                // position
                                aCurrPoly.append( B2DPoint( nLastX, nLastY ) );
                                ++nNumEntries;
                            }

                            if( nNumEntries > 1 )
                            {
                                // TODO(F2): Strictly speaking, we
                                // must check whether the previous
                                // segment is indeed quadratic
                                // (otherwise, if it is cubic, the
                                // spec requires the control point to
                                // be coincident with the current
                                // point)

                                const B2DVector aVecA( -aCurrPoly.getControlVectorB( nNumEntries-2 ) );

                                // set reflected control vector
                                aCurrPoly.setControlVectorA( nNumEntries-1, aVecA );

                                // calc real quadratic control point from prev point and prev
                                // cubic control vector
                                const B2DPoint aQuadControlPoint(
                                    (3.0*aCurrPoly.getControlPointB( nNumEntries-2 ) -
                                     aCurrPoly.getB2DPoint( nNumEntries-1 )) / 2.0 );

                                // calc new cubic control point
                                const double nX2Prime( (aQuadControlPoint.getX()*2.0 + nX) / 3.0 );
                                const double nY2Prime( (aQuadControlPoint.getY()*2.0 + nY) / 3.0 );

                                aCurrPoly.setControlPointB( nNumEntries-1,
                                                            B2DPoint( nX2Prime, nY2Prime ) );
                            }
                            // after the SVG spec, if there's no previous control point, the implicit
                            // control point shall be coincident with the current point. Which means,
                            // the quadratic bezier is a straight line in this case

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add point
                            aCurrPoly.append( B2DPoint( nX, nY ) );
                        }
                        break;
                    }

                    // #100617# not yet supported: elliptical arc
                    case 'A' :
                        // FALLTHROUGH intended
                    case 'a' :
                    {
                        OSL_ENSURE( false,
                                    "importFromSvgD(): non-interpreted tags in svg:d element (elliptical arc)!");
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            lcl_skipDoubleAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipDoubleAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipDoubleAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipNumberAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipNumberAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipDoubleAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                            lcl_skipDoubleAndSpacesAndCommas(nPos, rSvgDStatement, nLen);
                        }
                        break;
                    }

                    default:
                    {
                        OSL_ENSURE( false,
                                    "importFromSvgD(): skipping tags in svg:d element (unknown)!" );
                        OSL_TRACE( "importFromSvgD(): skipping tags in svg:d element (unknown: \"%c\")!", aCurrChar );
                        ++nPos;
                        break;
                    }
                }
            }

            if( aCurrPoly.count() )
            {
                // #104076# end-process closed state of last poly
                aCurrPoly.setClosed( bIsClosed );

                // add last polygon
                o_rPolyPoly.append( aCurrPoly );
            }

            return true;
        }
    }
}
