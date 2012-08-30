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


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include "parametricpolypolygonfactory.hxx"
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


using namespace ::com::sun::star;

namespace slideshow
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
                    new BarWipePolyPolygon( 6 ) );
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
                    new RandomWipe( 128, true /* bars */ ) );
            case DISSOLVE:
                return ParametricPolyPolygonSharedPtr(
                    new RandomWipe( 16 * 16, // for now until dxcanvas is faster
//                                     64 * 64 /* elements */,
                                    false /* dissolve */ ) );
            case WATERFALLWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new WaterfallWipe(
                        128,
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
                        // elements:
                        8 * 8,
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
                        // elements:
                        8 * 8,
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
                        8 * 8,
                        // flipOnYAxis:
                        nSubType == TOPLEFTCOUNTERCLOCKWISE ||
                        nSubType == TOPRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMLEFTCOUNTERCLOCKWISE ) );
            case BOXSNAKESWIPE:
                return ParametricPolyPolygonSharedPtr(
                    new BoxSnakesWipe(
                        // elements:
                        8 * 8,
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
                ENSURE_OR_THROW( false,
                                  "createShapeClipPolyPolygonAnimation(): Transition type mismatch" );
            }

            return ParametricPolyPolygonSharedPtr();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
