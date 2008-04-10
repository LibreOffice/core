/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dsvgpolypolygon.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_basegfx.hxx"

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <rtl/ustring.hxx>
#include <rtl/math.hxx>

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

        bool importFromSvgD(B2DPolyPolygon& o_rPolyPolygon, const ::rtl::OUString&  rSvgDStatement)
        {
            o_rPolyPolygon.clear();
            const sal_Int32 nLen(rSvgDStatement.getLength());
            sal_Int32 nPos(0);
            bool bIsClosed(false);
            double nLastX( 0.0 );
            double nLastY( 0.0 );
            B2DPolygon aCurrPoly;

            // skip initial whitespace
            lcl_skipSpaces(nPos, rSvgDStatement, nLen);

            while(nPos < nLen)
            {
                bool bRelative(false);
                bool bMoveTo(false);
                const sal_Unicode aCurrChar(rSvgDStatement[nPos]);

                switch(aCurrChar)
                {
                    case 'z' :
                    case 'Z' :
                    {
                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        // remember closed state of current polygon
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
                        if('m' == aCurrChar || 'l' == aCurrChar)
                        {
                            bRelative = true;
                        }

                        if(bMoveTo)
                        {
                            // new polygon start, finish old one
                            if(aCurrPoly.count())
                            {
                                aCurrPoly.setClosed(bIsClosed);
                                bIsClosed = false;
                                o_rPolyPolygon.append(aCurrPoly);
                                aCurrPoly.clear();
                            }
                        }

                        nPos++;
                        lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
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
                            double nX, nY(nLastY);

                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                            }

                            // set last position
                            nLastX = nX;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
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
                            double nX(nLastX), nY;

                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nY += nLastY;
                            }

                            // set last position
                            nLastY = nY;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
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

                            if(!lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existance of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // get first control point. It's the reflection of the PrevControlPoint
                            // of the last point. If not existent, use current point (see SVG)
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                // use mirrored previous control point
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
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

                            if(!lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existance of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1, nY1), B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
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

                            if(!lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // calculate the cubic bezier coefficients from the quadratic ones
                            const double nX1Prime((nX1 * 2.0 + nLastX) / 3.0);
                            const double nY1Prime((nY1 * 2.0 + nLastY) / 3.0);
                            const double nX2Prime((nX1 * 2.0 + nX) / 3.0);
                            const double nY2Prime((nY1 * 2.0 + nY) / 3.0);

                            // ensure existance of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1Prime, nY1Prime), B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
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

                            if(!lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existance of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // get first control point. It's the reflection of the PrevControlPoint
                            // of the last point. If not existent, use current point (see SVG)
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);
                            const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                // use mirrored previous control point
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            if(!aPrevControl.equal(aPrevPoint))
                            {
                                // there is a prev control point, and we have the already mirrored one
                                // in aPrevControl. We also need the quadratic control point for this
                                // new quadratic segment to calculate the 2nd cubic control point
                                const B2DPoint aQuadControlPoint(
                                    ((3.0 * aPrevControl.getX()) - aPrevPoint.getX()) / 2.0,
                                    ((3.0 * aPrevControl.getY()) - aPrevPoint.getY()) / 2.0);

                                // calculate the cubic bezier coefficients from the quadratic ones.
                                const double nX2Prime((aQuadControlPoint.getX() * 2.0 + nX) / 3.0);
                                const double nY2Prime((aQuadControlPoint.getY() * 2.0 + nY) / 3.0);

                                // append curved edge, use mirrored cubic control point directly
                                aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));
                            }
                            else
                            {
                                // when no previous control, SVG says to use current point -> straight line.
                                // Just add end point
                                aCurrPoly.append(B2DPoint(nX, nY));
                            }

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    // #100617# not yet supported: elliptical arc
                    case 'A' :
                        // FALLTHROUGH intended
                    case 'a' :
                    {
                        OSL_ENSURE(false, "importFromSvgD(): non-interpreted tags in svg:d element (elliptical arc)!");
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
                        OSL_ENSURE(false, "importFromSvgD(): skipping tags in svg:d element (unknown)!");
                        OSL_TRACE("importFromSvgD(): skipping tags in svg:d element (unknown: \"%c\")!", aCurrChar);
                        ++nPos;
                        break;
                    }
                }
            }

            if(aCurrPoly.count())
            {
                // end-process last poly
                aCurrPoly.setClosed(bIsClosed);
                o_rPolyPolygon.append(aCurrPoly);
            }

            return true;
        }

        ::rtl::OUString exportToSvgD(
            const B2DPolyPolygon& rPolyPolygon,
            bool bUseRelativeCoordinates,
            bool bDetectQuadraticBeziers)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());
            ::rtl::OUString aResult;
            B2DPoint aLastPoint(0.0, 0.0); // SVG assumes (0,0) as the initial current point

            for(sal_uInt32 i(0); i < nCount; i++)
            {
                const B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(i));
                const sal_uInt32 nPointCount(aPolygon.count());
                const bool bPolyUsesControlPoints(aPolygon.areControlPointsUsed());
                sal_Unicode aLastSVGCommand(' '); // last SVG command char
                B2DPoint aLeft, aRight; // for quadratic bezier test

                for(sal_uInt32 j(0); j < nPointCount; j++)
                {
                    const B2DPoint aCurrent(aPolygon.getB2DPoint(j));

                    if(0 == j)
                    {
                        // handle first polygon point
                        aResult += ::rtl::OUString::valueOf(lcl_getCommand('M', 'm', bUseRelativeCoordinates));
                        lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                        lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                        aLastSVGCommand =  lcl_getCommand('L', 'l', bUseRelativeCoordinates);
                    }
                    else
                    {
                        // handle edge from j-1 to j
                        const bool bEdgeIsBezier(bPolyUsesControlPoints
                            && (aPolygon.isNextControlPointUsed(j - 1) || aPolygon.isPrevControlPointUsed(j)));

                        if(bEdgeIsBezier)
                        {
                            // handle bezier edge
                            const B2DPoint aControl0(aPolygon.getNextControlPoint(j - 1));
                            const B2DPoint aControl1(aPolygon.getPrevControlPoint(j));
                            const B2VectorContinuity aPrevCont(aPolygon.getContinuityInPoint(j - 1));
                            const bool bSymmetricControlVector(CONTINUITY_C2 == aPrevCont);
                            bool bIsQuadraticBezier(false);

                            if(bDetectQuadraticBeziers)
                            {
                                // check for quadratic beziers - that's
                                // the case if both control points are in
                                // the same place when they are prolonged
                                // to the common quadratic control point
                                //
                                // Left: P = (3P1 - P0) / 2
                                // Right: P = (3P2 - P3) / 2
                                aLeft = B2DPoint((3.0 * aControl0 - aLastPoint) / 2.0);
                                aRight= B2DPoint((3.0 * aControl1 - aCurrent) / 2.0);
                                bIsQuadraticBezier = aLeft.equal(aRight);
                            }

                            if(bIsQuadraticBezier)
                            {
                                // approximately equal, export as quadratic bezier
                                if(bSymmetricControlVector)
                                {
                                    const sal_Unicode aCommand(lcl_getCommand('T', 't', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult += ::rtl::OUString::valueOf(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(lcl_getCommand('Q', 'q', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult += ::rtl::OUString::valueOf(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace(aResult, aLeft.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aLeft.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                }
                            }
                            else
                            {
                                // export as cubic bezier
                                if(bSymmetricControlVector)
                                {
                                    const sal_Unicode aCommand(lcl_getCommand('S', 's', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult += ::rtl::OUString::valueOf(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace(aResult, aControl1.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aControl1.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(lcl_getCommand('C', 'c', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult += ::rtl::OUString::valueOf(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    lcl_putNumberCharWithSpace(aResult, aControl0.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aControl0.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aControl1.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aControl1.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                    lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                }
                            }
                        }
                        else
                        {
                            // normal straight line points
                            if(aLastPoint.getX() == aCurrent.getX())
                            {
                                // export as vertical line
                                const sal_Unicode aCommand(lcl_getCommand('V', 'v', bUseRelativeCoordinates));

                                if(aLastSVGCommand != aCommand)
                                {
                                    aResult += ::rtl::OUString::valueOf(aCommand);
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                            }
                            else if(aLastPoint.getY() == aCurrent.getY())
                            {
                                // export as horizontal line
                                const sal_Unicode aCommand(lcl_getCommand('H', 'h', bUseRelativeCoordinates));

                                if(aLastSVGCommand != aCommand)
                                {
                                    aResult += ::rtl::OUString::valueOf(aCommand);
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                            }
                            else
                            {
                                // export as line
                                const sal_Unicode aCommand(lcl_getCommand('L', 'l', bUseRelativeCoordinates));

                                if(aLastSVGCommand != aCommand)
                                {
                                    aResult += ::rtl::OUString::valueOf(aCommand);
                                    aLastSVGCommand = aCommand;
                                }

                                lcl_putNumberCharWithSpace(aResult, aCurrent.getX(), aLastPoint.getX(), bUseRelativeCoordinates);
                                lcl_putNumberCharWithSpace(aResult, aCurrent.getY(), aLastPoint.getY(), bUseRelativeCoordinates);
                            }
                        }
                    }

                    aLastPoint = aCurrent;
                }

                // close path if closed poly (Z and z are equivalent here, but looks nicer
                // when case is matched)
                if(aPolygon.isClosed())
                {
                    aResult += ::rtl::OUString::valueOf(lcl_getCommand('Z', 'z', bUseRelativeCoordinates));
                }
            }

            return aResult;
        }
    }
}

// eof
