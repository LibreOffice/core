/*************************************************************************
 *
 *  $RCSfile: parametricpolypolygonfactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:07:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include <parametricpolypolygonfactory.hxx>
#include "barwipepolypolygon.hxx"
#include "boxwipe.hxx"
#include "fourboxwipe.hxx"
#include "barndoorwipe.hxx"
#include "doublediamondwipe.hxx"
#include "veewipe.hxx"
#include "iriswipe.hxx"
#include "ellipsewipe.hxx"
#include "checkerboardwipe.hxx"
#include "randomwipe.hxx"
#include "waterfallwipe.hxx"
#include "clockwipe.hxx"
#include "fanwipe.hxx"
#include "pinwheelwipe.hxx"
#include "snakewipe.hxx"
#include "spiralwipe.hxx"
#include "sweepwipe.hxx"
#include "figurewipe.hxx"
#include "zigzagwipe.hxx"
#include "com/sun/star/animations/TransitionType.hpp"
#include "com/sun/star/animations/TransitionSubType.hpp"


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        ParametricPolyPolygonSharedPtr
        ParametricPolyPolygonFactory::createClipPolyPolygon(
            sal_Int16 nType, sal_Int16 nSubType )
        {
            using namespace ::com::sun::star::animations::TransitionType;
            using namespace ::com::sun::star::animations::TransitionSubType;

            switch (nType)
            {
            case BARWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BarWipePolyPolygon );
            case BLINDSWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BarWipePolyPolygon( 5 ) );
            case BOXWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BoxWipe( nSubType == LEFTCENTER ||
                                 nSubType == TOPCENTER ||
                                 nSubType == RIGHTCENTER||
                                 nSubType == BOTTOMCENTER ) );
            case FOURBOXWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new FourBoxWipe( nSubType == CORNERSOUT ) );
            case BARNDOORWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BarnDoorWipe );
            case DIAGONALWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BarWipePolyPolygon );
            case VEEWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new VeeWipe );
            case IRISWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new IrisWipe );
            case ELLIPSEWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new EllipseWipe(nSubType) );
            case CHECKERBOARDWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new CheckerBoardWipe );
            case RANDOMBARWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new RandomWipe( 100, true /* bars */ ) );
            case DISSOLVE:
                return ParametricPolyPolygonSharedPtr(
                    new RandomWipe( 100, false /* dissolve */ ) );
            case WATERFALLWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new WaterfallWipe(
                        100,
                        // flipOnYAxis:
                        nSubType == VERTICALRIGHT ||
                        nSubType == HORIZONTALLEFT ) );
            case CLOCKWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new ClockWipe );
            case FANWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new FanWipe( // center:
                                 nSubType == CENTERTOP ||
                                 nSubType == CENTERRIGHT ) );
            case PINWHEELWIPE: {
                sal_Int32 blades;
                switch (nSubType) {
                case ONEBLADE:
                    blades = 1;
                    break;
                case THREEBLADE:
                    blades = 3;
                    break;
                case FOURBLADE:
                    blades = 4;
                    break;
                case EIGHTBLADE:
                    blades = 8;
                    break;
                default:
                    blades = 2;
                    break;
                }
                return ParametricPolyPolygonSharedPtr(
                    new PinWheelWipe( blades ) );
            }
            case SNAKEWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new SnakeWipe(
                        100,
                        // diagonal:
                        nSubType == TOPLEFTDIAGONAL ||
                        nSubType == TOPRIGHTDIAGONAL ||
                        nSubType == BOTTOMRIGHTDIAGONAL ||
                        nSubType == BOTTOMLEFTDIAGONAL,
                        // flipOnYAxis:
                        nSubType == TOPLEFTVERTICAL ||
                        nSubType == TOPRIGHTDIAGONAL ||
                        nSubType == BOTTOMLEFTDIAGONAL
                        ) );
            case PARALLELSNAKESWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new ParallelSnakesWipe(
                        100,
                        // diagonal:
                        nSubType == DIAGONALBOTTOMLEFTOPPOSITE ||
                        nSubType == DIAGONALTOPLEFTOPPOSITE,
                        // flipOnYAxis:
                        nSubType == VERTICALBOTTOMLEFTOPPOSITE ||
                        nSubType == HORIZONTALTOPLEFTOPPOSITE ||
                        nSubType == DIAGONALTOPLEFTOPPOSITE,
                        // opposite:
                        nSubType == VERTICALTOPLEFTOPPOSITE ||
                        nSubType == VERTICALBOTTOMLEFTOPPOSITE ||
                        nSubType == HORIZONTALTOPLEFTOPPOSITE ||
                        nSubType == HORIZONTALTOPRIGHTOPPOSITE ||
                        nSubType == DIAGONALBOTTOMLEFTOPPOSITE ||
                        nSubType == DIAGONALTOPLEFTOPPOSITE
                        ) );
            case SPIRALWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new SpiralWipe(
                        // elements:
                        100,
                        // flipOnYAxis:
                        nSubType == TOPLEFTCOUNTERCLOCKWISE ||
                        nSubType == TOPRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMLEFTCOUNTERCLOCKWISE ) );
            case BOXSNAKESWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BoxSnakesWipe(
                        // elements:
                        100,
                        // fourBox:
                        nSubType == FOURBOXVERTICAL ||
                        nSubType == FOURBOXHORIZONTAL ) );
            case SINGLESWEEPWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new SweepWipe(
                        // center:
                        nSubType == CLOCKWISETOP ||
                        nSubType == CLOCKWISERIGHT ||
                        nSubType == CLOCKWISEBOTTOM ||
                        nSubType == CLOCKWISELEFT,
                        // single:
                        true,
                        // oppositeVertical:
                        false,
                        // flipOnYAxis:
                        nSubType == COUNTERCLOCKWISEBOTTOMLEFT ||
                        nSubType == COUNTERCLOCKWISETOPRIGHT
                        ) );
            case DOUBLESWEEPWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new SweepWipe(
                        // center:
                        nSubType == PARALLELVERTICAL ||
                        nSubType == PARALLELDIAGONAL ||
                        nSubType == OPPOSITEVERTICAL ||
                        nSubType == OPPOSITEHORIZONTAL,
                        // single:
                        false,
                        // oppositeVertical:
                        nSubType == OPPOSITEVERTICAL ||
                        nSubType == OPPOSITEHORIZONTAL,
                        // flipOnYAxis:
                        false ) );
            case DOUBLEFANWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new FanWipe(
                        //center:
                        true,
                        // single:
                        false,
                        // fanIn:
                        nSubType == FANINVERTICAL ||
                        nSubType == FANINHORIZONTAL ) );
            case TRIANGLEWIPE:
                return ParametricPolyPolygonSharedPtr(
                    FigureWipe::createTriangleWipe() );
            case ARROWHEADWIPE:
                return ParametricPolyPolygonSharedPtr(
                    FigureWipe::createArrowHeadWipe() );
            case PENTAGONWIPE:
                return ParametricPolyPolygonSharedPtr(
                    FigureWipe::createPentagonWipe() );
            case HEXAGONWIPE:
                return ParametricPolyPolygonSharedPtr(
                    FigureWipe::createHexagonWipe() );
            case STARWIPE: {
                sal_Int32 points;
                switch (nSubType) {
                case FIVEPOINT:
                    points = 5;
                    break;
                case SIXPOINT:
                    points = 6;
                    break;
                default:
                    points = 4;
                    break;
                }
                return ParametricPolyPolygonSharedPtr(
                    FigureWipe::createStarWipe(points) );
            }
            case MISCDIAGONALWIPE: {
                switch (nSubType) {
                case DOUBLEBARNDOOR:
                    return ParametricPolyPolygonSharedPtr(
                        new BarnDoorWipe( true /* doubled */ ) );
                case DOUBLEDIAMOND:
                    return ParametricPolyPolygonSharedPtr(
                        new DoubleDiamondWipe );
                }
                break;
            }
            case ZIGZAGWIPE:
                return ParametricPolyPolygonSharedPtr( new ZigZagWipe(5) );
            case BARNZIGZAGWIPE:
                return ParametricPolyPolygonSharedPtr( new BarnZigZagWipe(5) );

            case BOWTIEWIPE:
            case BARNVEEWIPE:
            case EYEWIPE:
            case ROUNDRECTWIPE:
            case MISCSHAPEWIPE:
            case SALOONDOORWIPE:
            case WINDSHIELDWIPE:
                // for now, map to barwipe transition
                return ParametricPolyPolygonSharedPtr(
                    new BarWipePolyPolygon );

            default:
            case PUSHWIPE:
            case SLIDEWIPE:
            case FADE:
                ENSURE_AND_THROW( false,
                                  "createShapeClipPolyPolygonAnimation(): Transition type mismatch" );
            }

            return ParametricPolyPolygonSharedPtr();
        }
    }
}
