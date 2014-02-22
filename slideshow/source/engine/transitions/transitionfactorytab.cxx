/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <boost/current_function.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include "transitionfactory.hxx"
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
        
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::LEFTTORIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        false, 
        false 
    },
    {
        
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::TOPTOBOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        false, 
        false 
    },

    {
        
        animations::TransitionType::BLINDSWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BLINDSWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },

    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::TOPCENTER, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::RIGHTCENTER, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::BOTTOMCENTER, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXWIPE,
        animations::TransitionSubType::LEFTCENTER, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },

    {
        
        animations::TransitionType::FOURBOXWIPE,
        animations::TransitionSubType::CORNERSIN, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FOURBOXWIPE,
        animations::TransitionSubType::CORNERSOUT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::VERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::HORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::DIAGONALBOTTOMLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BARNDOORWIPE,
        animations::TransitionSubType::DIAGONALTOPLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -45.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::DIAGONALWIPE,
        animations::TransitionSubType::TOPLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DIAGONALWIPE,
        animations::TransitionSubType::TOPRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        135.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },


    {
        animations::TransitionType::BOWTIEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::BOWTIEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::MISCDIAGONALWIPE,
        animations::TransitionSubType::DOUBLEBARNDOOR, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::MISCDIAGONALWIPE,
        animations::TransitionSubType::DOUBLEDIAMOND, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },

    {
        
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::DOWN, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::LEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::UP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        animations::TransitionType::VEEWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },


    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::TOP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::LEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::UP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::BARNVEEWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::ZIGZAGWIPE,
        animations::TransitionSubType::LEFTTORIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        
        animations::TransitionType::ZIGZAGWIPE,
        animations::TransitionSubType::TOPTOBOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BARNZIGZAGWIPE,
        animations::TransitionSubType::VERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BARNZIGZAGWIPE,
        animations::TransitionSubType::HORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },

    {
        
        animations::TransitionType::IRISWIPE,
        animations::TransitionSubType::RECTANGLE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::IRISWIPE,
        animations::TransitionSubType::DIAMOND, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        45.0, 
        M_SQRT2, 
        M_SQRT2, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },


    {
        
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::UP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::RIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::DOWN, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::TRIANGLEWIPE,
        animations::TransitionSubType::LEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::UP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::RIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::DOWN, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::ARROWHEADWIPE,
        animations::TransitionSubType::LEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::PENTAGONWIPE,
        animations::TransitionSubType::UP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PENTAGONWIPE,
        animations::TransitionSubType::DOWN, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::HEXAGONWIPE,
        animations::TransitionSubType::HORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::HEXAGONWIPE,
        animations::TransitionSubType::VERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::CIRCLE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        true 
    },
    {
        
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::ELLIPSEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },


    {
        animations::TransitionType::EYEWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::EYEWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::ROUNDRECTWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::ROUNDRECTWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::FOURPOINT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::FIVEPOINT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::STARWIPE,
        animations::TransitionSubType::SIXPOINT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        animations::TransitionType::MISCSHAPEWIPE,
        animations::TransitionSubType::HEART,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::MISCSHAPEWIPE,
        animations::TransitionSubType::KEYHOLE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISETWELVE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISETHREE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISESIX, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::CLOCKWIPE,
        animations::TransitionSubType::CLOCKWISENINE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },

    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::ONEBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        true 
    },
    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::TWOBLADEVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        true 
    },
    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::TWOBLADEHORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        true 
    },
    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::THREEBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        true 
    },
    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::FOURBLADE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        true 
    },
    {
        
        animations::TransitionType::PINWHEELWIPE,
        animations::TransitionSubType::EIGHTBLADE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        true 
    },

    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISETOP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISERIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISEBOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISELEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISETOPLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::COUNTERCLOCKWISEBOTTOMLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::CLOCKWISEBOTTOMRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SINGLESWEEPWIPE,
        animations::TransitionSubType::COUNTERCLOCKWISETOPRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::CENTERTOP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::CENTERRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::TOP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::RIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::BOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::FANWIPE,
        animations::TransitionSubType::LEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },

    {
        
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANOUTVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANOUTHORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANINVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLEFANWIPE,
        animations::TransitionSubType::FANINHORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::OPPOSITEVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::OPPOSITEHORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONALTOPLEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::DOUBLESWEEPWIPE,
        animations::TransitionSubType::PARALLELDIAGONALBOTTOMLEFT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::TOP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   
        false                   
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::LEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   
        false                   
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::BOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   
        false                   
    },
    {
        animations::TransitionType::SALOONDOORWIPE,
        animations::TransitionSubType::RIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   
        false                   
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::RIGHT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   
        false                   
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::UP,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true,                   
        false                   
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::WINDSHIELDWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTHORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPLEFTDIAGONAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::TOPRIGHTDIAGONAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::BOTTOMRIGHTDIAGONAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SNAKEWIPE,
        animations::TransitionSubType::BOTTOMLEFTDIAGONAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },

    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPLEFTCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPRIGHTCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMRIGHTCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMLEFTCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPLEFTCOUNTERCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::TOPRIGHTCOUNTERCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMRIGHTCOUNTERCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        270.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },
    {
        
        animations::TransitionType::SPIRALWIPE,
        animations::TransitionSubType::BOTTOMLEFTCOUNTERCLOCKWISE, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT,
        true, 
        false 
    },

    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALTOPSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALBOTTOMSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::VERTICALBOTTOMLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALLEFTSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALRIGHTSAME,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::HORIZONTALTOPRIGHTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::DIAGONALBOTTOMLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        
        animations::TransitionType::PARALLELSNAKESWIPE,
        animations::TransitionSubType::DIAGONALTOPLEFTOPPOSITE,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },

    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXTOP, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXBOTTOM, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::TWOBOXRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        180.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::FOURBOXVERTICAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::BOXSNAKESWIPE,
        animations::TransitionSubType::FOURBOXHORIZONTAL, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },

    {
        
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::VERTICALLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::VERTICALRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::HORIZONTALLEFT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        -90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },
    {
        
        animations::TransitionType::WATERFALLWIPE,
        animations::TransitionSubType::HORIZONTALRIGHT, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_ROTATE_180,
        true, 
        false 
    },

    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOP,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOM,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMBOTTOMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOPRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::FROMTOPLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::COMBHORIZONTAL,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   
        false                   
    },
    {
        animations::TransitionType::PUSHWIPE,
        animations::TransitionSubType::COMBVERTICAL,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOP,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOM,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOMRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOPRIGHT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMTOPLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::SLIDEWIPE,
        animations::TransitionSubType::FROMBOTTOMLEFT,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE, 
        true,                   
        false                   
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::CROSSFADE,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADETOCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADEFROMCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    {
        animations::TransitionType::FADE,
        animations::TransitionSubType::FADEOVERCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },
    
    
    {
        animations::TransitionType::BARWIPE,
        animations::TransitionSubType::FADEOVERCOLOR,
        TransitionInfo::TRANSITION_SPECIAL,
        
        0.0,                    
        1.0,                    
        1.0,                    
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true,                   
        false                   
    },

    {
        
        animations::TransitionType::RANDOMBARWIPE,
        animations::TransitionSubType::VERTICAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },
    {
        
        animations::TransitionType::RANDOMBARWIPE,
        animations::TransitionSubType::HORIZONTAL,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        false 
    },

    {
        
        animations::TransitionType::CHECKERBOARDWIPE,
        animations::TransitionSubType::DOWN,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        90.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_Y,
        true, 
        false 
    },
    {
        
        animations::TransitionType::CHECKERBOARDWIPE,
        animations::TransitionSubType::ACROSS, 
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_FLIP_X,
        true, 
        false 
    },

    {
        
        animations::TransitionType::DISSOLVE,
        animations::TransitionSubType::DEFAULT,
        TransitionInfo::TRANSITION_CLIP_POLYPOLYGON,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        true 
    },

    
    
    
    
    
    
    {
        
        animations::TransitionType::RANDOM,
        animations::TransitionSubType::DEFAULT,
        TransitionInfo::TRANSITION_SPECIAL,
        0.0, 
        1.0, 
        1.0, 
        TransitionInfo::REVERSEMETHOD_IGNORE,
        true, 
        true 
    }
    
    
    
};

} 

const TransitionInfo* TransitionFactory::getTransitionInfo(
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
        return NULL;
}

const TransitionInfo* TransitionFactory::getRandomTransitionInfo()
{
    return lcl_transitionInfo + getRandomOrdinal(
        SAL_N_ELEMENTS(lcl_transitionInfo)
        - 1 /* exclude random transition at end of table */ );
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
