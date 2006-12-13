/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dsvgpolypolygon.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:08:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

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
                                   const sal_Int32          /*nLen*/)
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
                                const sal_Int32         /*nLen*/)
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

            void lcl_putNumberChar( ::rtl::OUString& rStr,
                                    double           fValue )
            {
                rStr += ::rtl::OUString::valueOf( fValue );
            }

            void lcl_putNumberCharWithSpace( ::rtl::OUString& rStr,
                                             double           fValue,
                                             double           fOldValue,
                                             bool             bUseRelativeCoordinates )
            {
                if( bUseRelativeCoordinates )
                    fValue -= fOldValue;

                const sal_Int32 aLen( rStr.getLength() );
                if(aLen)
                {
                    if( lcl_isOnNumberChar(rStr, aLen - 1, false) &&
                        fValue >= 0.0 )
                    {
                        rStr += ::rtl::OUString::valueOf(
                            sal_Unicode(' ') );
                    }
                }

                lcl_putNumberChar(rStr, fValue);
            }

            inline sal_Unicode lcl_getCommand( sal_Char cUpperCaseCommand,
                                               sal_Char cLowerCaseCommand,
                                               bool     bUseRelativeCoordinates )
            {
                return bUseRelativeCoordinates ? cLowerCaseCommand : cUpperCaseCommand;
            }
        }

        bool importFromSvgD( B2DPolyPolygon& o_rPolyPoly,
                             const ::rtl::OUString&     rSvgDStatement )
        {
            o_rPolyPoly.clear();

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

        // TODO(P1): Implement writing of relative coordinates (might
        // save some space)
        ::rtl::OUString exportToSvgD( const B2DPolyPolygon& rPolyPoly,
                                      bool                  bUseRelativeCoordinates,
                                      bool                  bDetectQuadraticBeziers )
        {
            ::rtl::OUString aResult;
            B2DPoint        aLastPoint(0.0, 0.0); // SVG assumes (0,0) as the initial current point

            const sal_Int32 nCount( rPolyPoly.count() );
            for( sal_Int32 i=0; i<nCount; ++i )
            {
                const B2DPolygon&   rPoly( rPolyPoly.getB2DPolygon( i ) );
                const sal_Int32     nPoints( rPoly.count() );
                const bool          bPolyUsesControlPoints( rPoly.areControlPointsUsed() );

                bool        bFirstPoint( true ); // true, if this is the first point
                sal_Unicode aLastSVGCommand( ' ' ); // last SVG command char

                for( sal_Int32 j=0; j<nPoints; ++j )
                {
                    const B2DPoint& rPoint( rPoly.getB2DPoint( j ) );

                    if( bFirstPoint )
                    {
                        bFirstPoint = false;

                        aResult += ::rtl::OUString::valueOf(
                            lcl_getCommand( 'M', 'm', bUseRelativeCoordinates ) );

                        lcl_putNumberCharWithSpace( aResult,
                                                    rPoint.getX(),
                                                    aLastPoint.getX(),
                                                    bUseRelativeCoordinates );
                        lcl_putNumberCharWithSpace( aResult,
                                                    rPoint.getY(),
                                                    aLastPoint.getY(),
                                                    bUseRelativeCoordinates );

                        aLastSVGCommand =
                            lcl_getCommand( 'L', 'l', bUseRelativeCoordinates );
                    }
                    else
                    {
                        // subtlety: as B2DPolygon stores the control
                        // points at p0, but the SVG statement expects
                        // them together with p3 (i.e. p0 is always
                        // taken from the current point), have to
                        // check for control vectors on _previous_
                        // point.
                        const bool bControlPointUsed(
                            bPolyUsesControlPoints &&
                            (!rPoly.getControlVectorA(j-1).equalZero() ||
                             !rPoly.getControlVectorB(j-1).equalZero()) );

                        if( bControlPointUsed )
                        {
                            // bezier points
                            // -------------

                            const B2DPoint& rControl0( rPoly.getControlPointA( j-1 ) );
                            const B2DPoint& rControl1( rPoly.getControlPointB( j-1 ) );

                            // check whether the previous segment was
                            // also a curve, and, if yes, whether it
                            // had a symmetric control vector
                            bool bSymmetricControlVector( false );
                            if( j > 1 )
                            {
                                const B2DPoint& rControlVec0( rPoly.getControlVectorA( j-1 ) );
                                const B2DPoint  aPrevControlVec1( -1.0*rPoly.getControlVectorB( j-2 ) );

                                // check whether mirrored prev vector
                                // 2 is approximately equal to current
                                // vector 1
                                bSymmetricControlVector = rControlVec0.equal( aPrevControlVec1 );
                            }

                            // check whether one of the optimized
                            // output primitives can be used
                            // (quadratic and/or symmetric control
                            // points)

                            // check for quadratic beziers - that's
                            // the case if both control points are in
                            // the same place when they are prolonged
                            // to the common quadratic control point
                            //
                            // Left: P = (3P1 - P0) / 2
                            // Right: P = (3P2 - P3) / 2
                            const B2DPoint aLeft( (3.0 * rControl0 - aLastPoint) / 2.0 );
                            const B2DPoint aRight( (3.0 * rControl1 - rPoint) / 2.0 );

                            if( bDetectQuadraticBeziers &&
                                aLeft.equal( aRight ) )
                            {
                                // approximately equal, export as
                                // quadratic bezier
                                if( bSymmetricControlVector )
                                {
                                    const sal_Unicode aCommand(
                                        lcl_getCommand( 'T', 't', bUseRelativeCoordinates ) );

                                    if( aLastSVGCommand != aCommand )
                                    {
                                        aResult += ::rtl::OUString::valueOf( aCommand );
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );

                                    aLastSVGCommand = aCommand;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(
                                        lcl_getCommand( 'Q', 'q', bUseRelativeCoordinates ) );

                                    if( aLastSVGCommand != aCommand )
                                    {
                                        aResult += ::rtl::OUString::valueOf( aCommand );
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace( aResult,
                                                                aLeft.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                aLeft.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );

                                    aLastSVGCommand = aCommand;
                                }
                            }
                            else
                            {
                                // export as cubic bezier
                                if( bSymmetricControlVector )
                                {
                                    const sal_Unicode aCommand(
                                        lcl_getCommand( 'S', 's', bUseRelativeCoordinates ) );

                                    if( aLastSVGCommand != aCommand )
                                    {
                                        aResult += ::rtl::OUString::valueOf( aCommand );
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl1.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl1.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );

                                    aLastSVGCommand = aCommand;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(
                                        lcl_getCommand( 'C', 'c', bUseRelativeCoordinates ) );

                                    if( aLastSVGCommand != aCommand )
                                    {
                                        aResult += ::rtl::OUString::valueOf( aCommand );
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl0.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl0.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl1.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rControl1.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getX(),
                                                                aLastPoint.getX(),
                                                                bUseRelativeCoordinates );
                                    lcl_putNumberCharWithSpace( aResult,
                                                                rPoint.getY(),
                                                                aLastPoint.getY(),
                                                                bUseRelativeCoordinates );

                                    aLastSVGCommand = aCommand;
                                }
                            }
                        }
                        else
                        {
                            // normal straight line points
                            // ---------------------------

                            // check whether one of the optimized
                            // output primitives can be used
                            // (horizontal or vertical line)
                            if( aLastPoint.getX() == rPoint.getX() )
                            {
                                const sal_Unicode aCommand(
                                    lcl_getCommand( 'V', 'v', bUseRelativeCoordinates ) );

                                if( aLastSVGCommand != aCommand )
                                {
                                    aResult += ::rtl::OUString::valueOf( aCommand );
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace( aResult,
                                                            rPoint.getY(),
                                                            aLastPoint.getY(),
                                                            bUseRelativeCoordinates );
                            }
                            else if( aLastPoint.getY() == rPoint.getY() )
                            {
                                const sal_Unicode aCommand(
                                    lcl_getCommand( 'H', 'h', bUseRelativeCoordinates ) );

                                if( aLastSVGCommand != aCommand )
                                {
                                    aResult += ::rtl::OUString::valueOf( aCommand );
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace( aResult,
                                                            rPoint.getX(),
                                                            aLastPoint.getX(),
                                                            bUseRelativeCoordinates );
                            }
                            else
                            {
                                const sal_Unicode aCommand(
                                    lcl_getCommand( 'L', 'l', bUseRelativeCoordinates ) );

                                if( aLastSVGCommand != aCommand )
                                {
                                    aResult += ::rtl::OUString::valueOf(aCommand);
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace( aResult,
                                                            rPoint.getX(),
                                                            aLastPoint.getX(),
                                                            bUseRelativeCoordinates );
                                lcl_putNumberCharWithSpace( aResult,
                                                            rPoint.getY(),
                                                            aLastPoint.getY(),
                                                            bUseRelativeCoordinates );
                            }
                        }
                    }

                    aLastPoint = rPoint;
                }

                // close path if closed poly (Z and z are
                // equivalent here, but looks nicer when case is
                // matched)
                if( rPoly.isClosed() )
                    aResult += ::rtl::OUString::valueOf(
                        lcl_getCommand( 'Z', 'z', bUseRelativeCoordinates ) );
            }

            return aResult;
        }
    }
}
