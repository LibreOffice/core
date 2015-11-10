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


#include <boost/current_function.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include "transitionfactory.hxx"
#include "transitionfactorytab.hxx"
#include "tools.hxx"

#include <algorithm>

using namespace ::com::sun::star;

namespace slideshow {
namespace internal {

namespace {

static const TransitionInfo lcl_transitionInfo[] =
{
    {
        0,
        0,
        TransitionInfo::TRANSITION_INVALID,
        0.0,
        0.0,
        0.0,
        TransitionInfo::REVERSEMETHOD_IGNORE,
        false,
        false
    },
    {
        // mapped to BarWipePolyPolygon:
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::LEFTTORIGHT, // (1)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        false, // 'out' by subtraction
        false // scale isotrophically to target size
    },
    {
        // mapped to BarWipePolyPolygon:
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::TOPTOBOTTOM, // (2)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        false, // 'out' by subtraction
        false // scale isotrophically to target size
    },

    {
        // mapped to BarWipePolyPolygon(nBars=5):
        animations::TransitionType::BLINDSWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarWipePolyPolygon(nBars=5):
        animations::TransitionType::BLINDSWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPLEFT, // (3)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // possible via bottomRight
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPRIGHT, // (4)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // possible via bottomLeft
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMRIGHT, // (5)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // possible via topLeft
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMLEFT, // (6)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // possible via topRight
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPCENTER, // (23)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::RIGHTCENTER, // (24)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMCENTER, // (25)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxWipe:
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::LEFTCENTER, // (26)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FourBoxWipe:
        animations::TransitionType::FOURBOXWIPE,
        animations::TransitionSubType::CORNERSIN, // (7)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FourBoxWipe:
        animations::TransitionType::FOURBOXWIPE,
        animations::TransitionSubType::CORNERSOUT, // (8)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to BarnDoorWipe:
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::VERTICAL, // (21)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarnDoorWipe:
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::HORIZONTAL, // (22)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarnDoorWipe:
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::DIAGONALBOTTOMLEFT, // (45)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarnDoorWipe:
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::DIAGONALTOPLEFT, // (46)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -45.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to BarWipePolyPolygon:
        animations::TransitionType::DIAGONALWIPE,
        animations::TransitionSubType::TOPLEFT, // (41)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarWipePolyPolygon:
        animations::TransitionType::DIAGONALWIPE,
        animations::TransitionSubType::TOPRIGHT, // (42)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        135.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },


    {
        animations::TransitionType::BOWTIEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::BOWTIEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to BarnDoorWipe (doubled=true):
        animations::TransitionType::MISCDIAGONALWIPE,
        animations::TransitionSubType::DOUBLEBARNDOOR, // (47)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to DoubleDiamondWipe:
        animations::TransitionType::MISCDIAGONALWIPE,
        animations::TransitionSubType::DOUBLEDIAMOND, // (48)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to VeeWipe:
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::DOWN, // (61)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to VeeWipe:
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::LEFT, // (62)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::UP, // (63)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },


    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::TOP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::LEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::UP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to ZigZagWipe:
        animations::TransitionType::ZIGZAGWIPE,
        animations::TransitionSubType::LEFTTORIGHT, // (71)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ZigZagWipe:
        animations::TransitionType::ZIGZAGWIPE,
        animations::TransitionSubType::TOPTOBOTTOM, // (72)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarnZigZagWipe:
        animations::TransitionType::BARNZIGZAGWIPE,
        animations::TransitionSubType::VERTICAL, // (73)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BarnZigZagWipe:
        animations::TransitionType::BARNZIGZAGWIPE,
        animations::TransitionSubType::HORIZONTAL, // (74)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to IrisWipe:
        animations::TransitionType::IRISWIPE,
        animations::TransitionSubType::RECTANGLE, // (101)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to IrisWipe:
        animations::TransitionType::IRISWIPE,
        animations::TransitionSubType::DIAMOND, // (102)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, // rotation
        M_SQRT2, // scaling
        M_SQRT2, // scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },


    {
        // mapped to FigureWipe(triangle):
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::UP, // (103)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(triangle):
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::RIGHT, // (104)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(triangle):
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::DOWN, // (105)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(triangle):
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::LEFT, // (106)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FigureWipe(arrowHead):
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::UP, // (107)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(arrowHead):
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::RIGHT, // (108)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(arrowHead):
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::DOWN, // (109)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(arrowHead):
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::LEFT, // (110)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FigureWipe(pentagon):
        animations::TransitionType::PENTAGONWIPE,
        animations::TransitionSubType::UP, // (111)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(pentagon):
        animations::TransitionType::PENTAGONWIPE,
        animations::TransitionSubType::DOWN, // (112)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FigureWipe(hexagon):
        animations::TransitionType::HEXAGONWIPE,
        animations::TransitionSubType::HORIZONTAL, // (113)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(hexagon):
        animations::TransitionType::HEXAGONWIPE,
        animations::TransitionSubType::VERTICAL, // (114)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to EllipseWipe:
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::CIRCLE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size
    },
    {
        // mapped to EllipseWipe:
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to EllipseWipe:
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },


    {
        animations::TransitionType::EYEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::EYEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::ROUNDRECTWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::ROUNDRECTWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to FigureWipe(star, points=4):
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::FOURPOINT, // (127)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(star, points=5):
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::FIVEPOINT, // (128)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FigureWipe(star, points=6):
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::SIXPOINT, // (129)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        animations::TransitionType::MISCSHAPEWIPE,
        animations::TransitionSubType::HEART,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::MISCSHAPEWIPE,
        animations::TransitionSubType::KEYHOLE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to ClockWipe:
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISETWELVE, // (201)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ClockWipe:
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISETHREE, // (202)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ClockWipe:
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISESIX, // (203)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ClockWipe:
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISENINE, // (204)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::ONEBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },
    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::TWOBLADEVERTICAL, // (205)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },
    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::TWOBLADEHORIZONTAL, // (206)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },
    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::THREEBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },
    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::FOURBLADE, // (207)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },
    {
        // mapped to PinWheelWipe:
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::EIGHTBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size, like ppt
    },

    {
        // mapped to SweepWipe (center=true, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISETOP, // (221)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISERIGHT, // (222)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISEBOTTOM, // (223)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISELEFT, // (224)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISETOPLEFT, // (241)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=true, flipOnYAxis=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::COUNTERCLOCKWISEBOTTOMLEFT, // (242)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISEBOTTOMRIGHT, // (243)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=true, flipOnYAxis=true):
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::COUNTERCLOCKWISETOPRIGHT, // (244)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FanWipe(center=true):
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::CENTERTOP, // (211)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe(center=true):
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::CENTERRIGHT, // (212)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe:
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::TOP, // (231)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe:
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::RIGHT, // (232)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe:
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::BOTTOM, // (233)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe:
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::LEFT, // (234)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to FanWipe(center=true, single=false, fanIn=false):
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANOUTVERTICAL, // (213)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe(center=true, single=false, fanIn=false):
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANOUTHORIZONTAL, // (214)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe(center=true, single=false, fanIn=true):
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANINVERTICAL, // (235)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to FanWipe(center=true, single=false, fanIn=true):
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANINHORIZONTAL, // (236)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to SweepWipe (center=true, single=false):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELVERTICAL, // (225)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=false):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONAL, // (226)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=false,
        //                      oppositeVertical=true):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::OPPOSITEVERTICAL, // (227)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=true, single=false,
        //                      oppositeVertical=true):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::OPPOSITEHORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=false):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONALTOPLEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SweepWipe (center=false, single=false):
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONALBOTTOMLEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::TOP, // (251)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::LEFT, // (252)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::BOTTOM, // (253)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::RIGHT, // (254)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::UP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to SnakeWipe:
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTHORIZONTAL, // (301)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SnakeWipe(flipOnYAxis=true):
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTVERTICAL, // (302)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SnakeWipe(diagonal=true):
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTDIAGONAL, // (303)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SnakeWipe(diagonal=true, flipOnYAxis=true):
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPRIGHTDIAGONAL, // (304)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SnakeWipe(diagonal=true):
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::BOTTOMRIGHTDIAGONAL, // (305)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SnakeWipe(diagonal=true, flipOnYAxis=true):
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::BOTTOMLEFTDIAGONAL, // (306)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to SpiralWipe:
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPLEFTCLOCKWISE, // (310)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe:
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPRIGHTCLOCKWISE, // (311)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe:
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMRIGHTCLOCKWISE, // (312)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe:
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMLEFTCLOCKWISE, // (313)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe(flipOnYAxis=true):
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPLEFTCOUNTERCLOCKWISE, // (314)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe(flipOnYAxis=true):
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPRIGHTCOUNTERCLOCKWISE, // (315)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe(flipOnYAxis=true):
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMRIGHTCOUNTERCLOCKWISE, // (316)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to SpiralWipe(flipOnYAxis=true):
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMLEFTCOUNTERCLOCKWISE, // (317)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to ParallelSnakesWipe:
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALTOPSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe:
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALBOTTOMSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (opposite=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (flipOnYAxis=true, opposite=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALBOTTOMLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe:
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALLEFTSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe:
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALRIGHTSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (flipOnYAxis=true, opposite=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (opposite=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALTOPRIGHTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (diagonal=true, opposite=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::DIAGONALBOTTOMLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to ParallelSnakesWipe (diagonal=true, opposite=true,
        //                               flipOnYAxis=true):
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::DIAGONALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to BoxSnakesWipe:
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXTOP, // (340)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxSnakesWipe:
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXBOTTOM, // (341)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxSnakesWipe:
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXLEFT, // (342)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxSnakesWipe:
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXRIGHT, // (343)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxSnakesWipe(fourBox=true):
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::FOURBOXVERTICAL, // (344)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to BoxSnakesWipe(fourBox=true):
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::FOURBOXHORIZONTAL, // (345)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to WaterfallWipe:
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::VERTICALLEFT, // (350)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to WaterfallWipe (flipOnYAxis=true):
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::VERTICALRIGHT, // (351)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to WaterfallWipe (flipOnYAxis=true):
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::HORIZONTALLEFT, // (352)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to WaterfallWipe, flipOnYAxis=false:
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::HORIZONTALRIGHT, // (353)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOP,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOM,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOPRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOPLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::COMBHORIZONTAL,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::COMBVERTICAL,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOP,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOM,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOPRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOPLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE, // special code for this transition
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::CROSSFADE,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADETOCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADEFROMCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADEOVERCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },
    // this is the cut through black fade (does not fade, but does a
    // hard cut)
    {
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::FADEOVERCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        // TODO(F2): Setup parameters
        0.0,                    // no rotation
        1.0,                    // no scaling
        1.0,                    // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   // 'out' by parameter sweep inversion
        false                   // scale isotrophically to target size
    },

    {
        // mapped to RandomWipe:
        animations::TransitionType::RANDOMBARWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to RandomWipe:
        animations::TransitionType::RANDOMBARWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to CheckerBoard:
        animations::TransitionType::CHECKERBOARDWIPE,
        animations::TransitionSubType::DOWN,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, // rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },
    {
        // mapped to CheckerBoard:
        animations::TransitionType::CHECKERBOARDWIPE,
        animations::TransitionSubType::ACROSS, // (default)
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, // 'out' by parameter sweep inversion
        false // scale isotrophically to target size
    },

    {
        // mapped to RandomWipe:
        animations::TransitionType::DISSOLVE,
        animations::TransitionSubType::DEFAULT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size
    },


    // NOTE: This entry MUST be the last, to keep
    // createSlideTransition() from infinite recursion. Because
    // getRandomTransitionInfo() below will exclude the last entry of
    // the table from the random number generation.

    {
        // specially handled
        animations::TransitionType::RANDOM,
        animations::TransitionSubType::DEFAULT,
        TransitionInfo::TRANSITION_SPECIAL,
        0.0, // no rotation
        1.0, // no scaling
        1.0, // no scaling
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, // 'out' by parameter sweep inversion
        true // scale isotrophically to target size
    }

    // NOTE: DON'T add after this entry! See comment above!

};

} // anon namespace

const TransitionInfo* getTransitionInfo(
    sal_Int16 nTransitionType, sal_Int16 nTransitionSubType )
{
    static const TransitionInfo* pTableEnd = lcl_transitionInfo+
        SAL_N_ELEMENTS(lcl_transitionInfo);

    const TransitionInfo* pRes = ::std::find_if(
        lcl_transitionInfo, pTableEnd,
        TransitionInfo::Comparator( nTransitionType,
                                    nTransitionSubType ) );
    if (pRes != pTableEnd)
        return pRes;
    else
        return nullptr;
}

const TransitionInfo* getRandomTransitionInfo()
{
    return lcl_transitionInfo + getRandomOrdinal(
        SAL_N_ELEMENTS(lcl_transitionInfo)
        - 1 /* exclude random transition at end of table */ );
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
