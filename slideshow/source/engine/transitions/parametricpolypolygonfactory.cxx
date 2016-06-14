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
        std::shared_ptr< ParametricPolyPolygon >
        ParametricPolyPolygonFactory::createClipPolyPolygon(
            sal_Int16 nType, sal_Int16 nSubType )
        {
            using namespace ::com::sun::star::animations::TransitionType;
            using namespace ::com::sun::star::animations::TransitionSubType;

            switch (nType)
            {
            case BARWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BarWipePolyPolygon );
            case BLINDSWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BarWipePolyPolygon( 6 ) );
            case BOXWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BoxWipe( nSubType == LEFTCENTER ||
                                 nSubType == TOPCENTER ||
                                 nSubType == RIGHTCENTER||
                                 nSubType == BOTTOMCENTER ) );
            case FOURBOXWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new FourBoxWipe( nSubType == CORNERSOUT ) );
            case BARNDOORWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BarnDoorWipe );
            case DIAGONALWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BarWipePolyPolygon );
            case VEEWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new VeeWipe );
            case IRISWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new IrisWipe );
            case ELLIPSEWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new EllipseWipe(nSubType) );
            case CHECKERBOARDWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new CheckerBoardWipe );
            case RANDOMBARWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new RandomWipe( 128, true /* bars */ ) );
            case DISSOLVE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new RandomWipe( 16 * 16, // for now until dxcanvas is faster
//                                     64 * 64 /* elements */,
                                    false /* dissolve */ ) );
            case WATERFALLWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new WaterfallWipe(
                        128,
                        // flipOnYAxis:
                        nSubType == VERTICALRIGHT ||
                        nSubType == HORIZONTALLEFT ) );
            case CLOCKWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new ClockWipe );
            case FANWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
                    new PinWheelWipe( blades ) );
            }
            case SNAKEWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
                    new SpiralWipe(
                        // elements:
                        8 * 8,
                        // flipOnYAxis:
                        nSubType == TOPLEFTCOUNTERCLOCKWISE ||
                        nSubType == TOPRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMRIGHTCOUNTERCLOCKWISE ||
                        nSubType == BOTTOMLEFTCOUNTERCLOCKWISE ) );
            case BOXSNAKESWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BoxSnakesWipe(
                        // elements:
                        8 * 8,
                        // fourBox:
                        nSubType == FOURBOXVERTICAL ||
                        nSubType == FOURBOXHORIZONTAL ) );
            case SINGLESWEEPWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
                    new FanWipe(
                        //center:
                        true,
                        // single:
                        false,
                        // fanIn:
                        nSubType == FANINVERTICAL ||
                        nSubType == FANINHORIZONTAL ) );
            case TRIANGLEWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    FigureWipe::createTriangleWipe() );
            case ARROWHEADWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    FigureWipe::createArrowHeadWipe() );
            case PENTAGONWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
                    FigureWipe::createPentagonWipe() );
            case HEXAGONWIPE:
                return std::shared_ptr< ParametricPolyPolygon >(
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
                return std::shared_ptr< ParametricPolyPolygon >(
                    FigureWipe::createStarWipe(points) );
            }
            case MISCDIAGONALWIPE: {
                switch (nSubType) {
                case DOUBLEBARNDOOR:
                    return std::shared_ptr< ParametricPolyPolygon >(
                        new BarnDoorWipe( true /* doubled */ ) );
                case DOUBLEDIAMOND:
                    return std::shared_ptr< ParametricPolyPolygon >(
                        new DoubleDiamondWipe );
                }
                break;
            }
            case ZIGZAGWIPE:
                return std::shared_ptr< ParametricPolyPolygon >( new ZigZagWipe(5) );
            case BARNZIGZAGWIPE:
                return std::shared_ptr< ParametricPolyPolygon >( new BarnZigZagWipe(5) );

            case BOWTIEWIPE:
            case BARNVEEWIPE:
            case EYEWIPE:
            case ROUNDRECTWIPE:
            case MISCSHAPEWIPE:
            case SALOONDOORWIPE:
            case WINDSHIELDWIPE:
                // for now, map to barwipe transition
                return std::shared_ptr< ParametricPolyPolygon >(
                    new BarWipePolyPolygon );

            default:
            case PUSHWIPE:
            case SLIDEWIPE:
            case FADE:
                ENSURE_OR_THROW( false,
                                  "createShapeClipPolyPolygonAnimation(): Transition type mismatch" );
            }

            return std::shared_ptr< ParametricPolyPolygon >();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
