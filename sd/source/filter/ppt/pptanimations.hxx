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

#ifndef INCLUDED_SD_SOURCE_FILTER_PPT_PPTANIMATIONS_HXX
#define INCLUDED_SD_SOURCE_FILTER_PPT_PPTANIMATIONS_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>

#include <oox/ppt/pptfilterhelpers.hxx>

#include <map>
#include <sal/types.h>

class SvStream;

namespace ppt
{

// old transition types
#define PPT_TRANSITION_TYPE_NONE            0
#define PPT_TRANSITION_TYPE_RANDOM          1
#define PPT_TRANSITION_TYPE_BLINDS          2
#define PPT_TRANSITION_TYPE_CHECKER         3
#define PPT_TRANSITION_TYPE_COVER           4
#define PPT_TRANSITION_TYPE_DISSOLVE        5
#define PPT_TRANSITION_TYPE_FADE            6
#define PPT_TRANSITION_TYPE_PULL            7 // Uncover in MS-PPT Specs
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11 // Box In/Out in MS-PPT Specs
#define PPT_TRANSITION_TYPE_SPLIT          13

// effects, new in xp
#define PPT_TRANSITION_TYPE_DIAMOND         17
#define PPT_TRANSITION_TYPE_PLUS            18
#define PPT_TRANSITION_TYPE_WEDGE           19
#define PPT_TRANSITION_TYPE_PUSH            20
#define PPT_TRANSITION_TYPE_COMB            21
#define PPT_TRANSITION_TYPE_NEWSFLASH       22
#define PPT_TRANSITION_TYPE_SMOOTHFADE      23 // Alpha Fade in MS-PPT Specs
#define PPT_TRANSITION_TYPE_WHEEL           26
#define PPT_TRANSITION_TYPE_CIRCLE          27

// atoms
#define DFF_msofbtAnimEvent                         0xf125
#define DFF_msofbtAnimNode                          0xf127
#define DFF_msofbtAnimTrigger                       0xf128
#define DFF_msofbtAnimValue                         0xf129
#define DFF_msofbtAnimateTarget                     0xf12a
#define DFF_msofbtAnimate                           0xf12b
#define DFF_msofbtAnimateColor                      0xf12c
#define DFF_msofbtAnimateFilter                     0xf12d
#define DFF_msofbtAnimateMotion                     0xf12e
#define DFF_msofbtAnimateRotation                   0xf12f
#define DFF_msofbtAnimateScale                      0xf130
#define DFF_msofbtAnimateSet                        0xf131
#define DFF_msofbtAnimCommand                       0xf132
#define DFF_msofbtAnimateTargetSettings             0xf133
#define DFF_msofbtAnimateData                       0xf134
#define DFF_msofbtAnimateColorData                  0xf135
#define DFF_msofbtAnimateFilterData                 0xf136
#define DFF_msofbtAnimateMotionData                 0xf137
#define DFF_msofbtAnimateScaleData                  0xf139
#define DFF_msofbtAnimateSetData                    0xf13a
#define DFF_msofbtCommandData                       0xf13b
#define DFF_msofbtAnimateTargetElement              0xf13c
#define DFF_msofbtAnimPropertySet                   0xf13d
#define DFF_msofbtAnimateAttributeNames             0xf13e
#define DFF_msofbtAnimKeyPoints                     0xf13f
#define DFF_msofbtAnimIteration                     0xf140
#define DFF_msofbtAnimAction                        0xf141          // correct name??
#define DFF_msofbtAnimAttributeValue                0xf142
#define DFF_msofbtAnimKeyTime                       0xf143
#define DFF_msofbtAnimGroup                         0xf144
#define DFF_msofbtAnimSubGoup                       0xf145
#define DFF_msofbtAnimateRotationData               0xf138
#define DFF_msofbtAnimReference                     0x2afb

// property ids
#define DFF_ANIM_ID                         1
#define DFF_ANIM_RUNTIMECONTEXT             2
#define DFF_ANIM_PATH_EDIT_MODE             3
#define DFF_ANIM_COLORSPACE                 4
#define DFF_ANIM_DIRECTION                  5       // TODO: Conflict?
#define DFF_ANIM_MASTERREL                  5       // TODO: Conflict?
#define DFF_ANIM_OVERRIDE                   6
#define DFF_ANIM_PRESET_ID                  9
#define DFF_ANIM_PRESET_SUB_TYPE            10
#define DFF_ANIM_PRESET_CLASS               11
#define DFF_ANIM_AFTEREFFECT                13
#define DFF_ANIM_ENDAFTERSLIDE              15
#define DFF_ANIM_TIMEFILTER                 16
#define DFF_ANIM_EVENT_FILTER               17
#define DFF_ANIM_GROUP_ID                   19
#define DFF_ANIM_NODE_TYPE                  20
#define DFF_ANIM_VOLUME                     22
#define DFF_ANIM_PROPERTY_ID_COUNT			(DFF_ANIM_VOLUME + 1)

// property types
#define DFF_ANIM_PROP_TYPE_BYTE             0
#define DFF_ANIM_PROP_TYPE_INT32            1
#define DFF_ANIM_PROP_TYPE_FLOAT            2
#define DFF_ANIM_PROP_TYPE_UNISTRING        3

#define DFF_ANIM_PRESS_CLASS_USER_DEFINED   0
#define DFF_ANIM_PRESS_CLASS_ENTRANCE       1
#define DFF_ANIM_PRESS_CLASS_EXIT           2
#define DFF_ANIM_PRESS_CLASS_EMPHASIS       3
#define DFF_ANIM_PRESS_CLASS_MOTIONPATH     4
#define DFF_ANIM_PRESS_CLASS_OLE_ACTION     5
#define DFF_ANIM_PRESS_CLASS_MEDIACALL      6

#define DFF_ANIM_NODE_TYPE_ON_CLICK         1
#define DFF_ANIM_NODE_TYPE_WITH_PREVIOUS    2
#define DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS   3
#define DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE    4
#define DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ  5
#define DFF_ANIM_NODE_TYPE_TIMING_ROOT      9

/* constants for fill entry in AnimationNode */
const sal_Int32 mso_Anim_GroupType_PAR      = 0;
const sal_Int32 mso_Anim_GroupType_SEQ      = 1;
const sal_Int32 mso_Anim_GroupType_NODE     = 3;
const sal_Int32 mso_Anim_GroupType_MEDIA    = 4;

/* constants for fill entry in AnimationNode */
const sal_Int32 mso_Anim_Fill_ALWAYS    = 1;
const sal_Int32 mso_Anim_Fill_WHENOFF   = 2;
const sal_Int32 mso_Anim_Fill_NEVER     = 3;

/* constants for fill entry in AnimationNode */
const sal_Int32 mso_Anim_Fill_REMOVE    = 1;
const sal_Int32 mso_Anim_Fill_FREEZE    = 2;
const sal_Int32 mso_Anim_Fill_HOLD      = 3;

/* constants for behaviour entry in PPtAnimationNode */
const sal_Int32 mso_Anim_Behaviour_FILTER   = 24;
const sal_Int32 mso_Anim_Behaviour_ANIMATION= 25;

typedef ::std::map< sal_Int32, css::uno::Any > PropertySetMap_t;

class PropertySet
{
public:
    PropertySetMap_t maProperties;

    bool hasProperty( sal_Int32 nProperty ) const;
    css::uno::Any getProperty( sal_Int32 nProperty ) const;
};

/** this atom is the first entry in each animation group */
struct AnimationNode
{
public:
    /** see mso_Anim_GroupType_? */
    sal_Int32 mnGroupType;

    /** see mso_Anim_Restart_? */
    sal_Int32 mnRestart;

    /** see mso_Anim_Fill_? */
    sal_Int32 mnFill;

    /** see mso_Anim_Behaviour_? */
    sal_Int32 mnNodeType;

    /** duration of this group in 1000th seconds */
    sal_Int32 mnDuration;

    sal_Int32 mnU1, mnU3, mnU4;

    AnimationNode()
        : mnGroupType(0)
        , mnRestart(0)
        , mnFill(0)
        , mnNodeType(0)
        , mnDuration(0)
        , mnU1(0), mnU3(0), mnU4(0)
    {
    }
public:

    friend SvStream& WriteAnimationNode(SvStream& rOut, AnimationNode& rAtom);
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
