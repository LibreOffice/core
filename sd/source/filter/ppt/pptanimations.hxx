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

#ifndef _SD_PPT_ANIMATIONS_HXX
#define _SD_PPT_ANIMATIONS_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>

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
#define PPT_TRANSITION_TYPE_PULL            7
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11
#define PPT_TRANSITION_TYPE_SPLIT          13

// effects, new in xp
#define PPT_TRANSITION_TYPE_DIAMOND         17
#define PPT_TRANSITION_TYPE_PLUS            18
#define PPT_TRANSITION_TYPE_WEDGE           19
#define PPT_TRANSITION_TYPE_PUSH            20
#define PPT_TRANSITION_TYPE_COMB            21
#define PPT_TRANSITION_TYPE_NEWSFLASH       22
#define PPT_TRANSITION_TYPE_SMOOTHFADE      23
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

typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any > PropertySetMap_t;

class PropertySet
{
public:
    PropertySetMap_t maProperties;

    bool hasProperty( sal_Int32 nProperty ) const;
    ::com::sun::star::uno::Any getProperty( sal_Int32 nProperty ) const;
};


enum MS_AttributeNames
{
    MS_PPT_X, MS_PPT_Y, MS_PPT_W, MS_PPT_H, MS_PPT_C, MS_R, MS_XSHEAR, MS_FILLCOLOR, MS_FILLTYPE,
    MS_STROKECOLOR, MS_STROKEON, MS_STYLECOLOR, MS_STYLEROTATION, MS_FONTWEIGHT,
    MS_STYLEUNDERLINE, MS_STYLEFONTFAMILY, MS_STYLEFONTSIZE, MS_STYLEFONTSTYLE,
    MS_STYLEVISIBILITY, MS_STYLEOPACITY, MS_UNKNOWN
};

struct ImplAttributeNameConversion
{
    MS_AttributeNames   meAttribute;
    const char* mpMSName;
    const char* mpAPIName;
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

    friend SvStream& operator>>(SvStream& rIn, AnimationNode& rAtom);
    friend SvStream& operator<<(SvStream& rOut, AnimationNode& rAtom);
};

static const ImplAttributeNameConversion gImplConversionList[] =
{
    { MS_PPT_X,             "ppt_x",                        "X" },
    { MS_PPT_Y,             "ppt_y",                        "Y" },
    { MS_PPT_W,             "ppt_w",                        "Width" },
    { MS_PPT_H,             "ppt_h",                        "Height" },
    { MS_PPT_C,             "ppt_c",                        "DimColor" },
    { MS_R,                 "r",                            "Rotate" },
    { MS_XSHEAR,            "xshear",                       "SkewX" },
    { MS_FILLCOLOR,         "fillColor",                    "FillColor" },
    { MS_FILLCOLOR,         "fillcolor",                    "FillColor" },
    { MS_FILLTYPE,          "fill.type",                    "FillStyle" },
    { MS_STROKECOLOR,       "stroke.color",                 "LineColor" },
    { MS_STROKEON,          "stroke.on",                    "LineStyle" },
    { MS_STYLECOLOR,        "style.color",                  "CharColor" },
    { MS_STYLEROTATION,     "style.rotation",               "Rotate" },
    { MS_FONTWEIGHT,        "style.fontWeight",             "CharWeight" },
    { MS_STYLEUNDERLINE,    "style.textDecorationUnderline","CharUnderline" },
    { MS_STYLEFONTFAMILY,   "style.fontFamily",             "CharFontName" },
    { MS_STYLEFONTSIZE,     "style.fontSize",               "CharHeight" },
    { MS_STYLEFONTSTYLE,    "style.fontStyle",              "CharPosture" },
    { MS_STYLEVISIBILITY,   "style.visibility",             "Visibility" },
    { MS_STYLEOPACITY,      "style.opacity",                "Opacity" },
    { MS_UNKNOWN, NULL, NULL }
};

struct transition
{
    const sal_Char* mpName;
    sal_Int16 mnType;
    sal_Int16 mnSubType;
    sal_Bool mbDirection; // true: default geometric direction

    static const transition* find( const OUString& rName );
    static const sal_Char* find( const sal_Int16 mnType, const sal_Int16 mnSubType, const sal_Bool bDirection );
};
static const transition gTransitions[] =
{
{ "wipe(up)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, sal_True },
{ "wipe(right)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, sal_False },
{ "wipe(left)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::LEFTTORIGHT, sal_True },
{ "wipe(down)", ::com::sun::star::animations::TransitionType::BARWIPE, ::com::sun::star::animations::TransitionSubType::TOPTOBOTTOM, sal_False },
{ "wheel(1)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::ONEBLADE, sal_True },
{ "wheel(2)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::TWOBLADEVERTICAL, sal_True },
{ "wheel(3)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::THREEBLADE, sal_True },
{ "wheel(4)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::FOURBLADE, sal_True },
{ "wheel(8)", ::com::sun::star::animations::TransitionType::PINWHEELWIPE, ::com::sun::star::animations::TransitionSubType::EIGHTBLADE, sal_True },
{ "strips(downLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, sal_True },
{ "strips(upLeft)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, sal_False },
{ "strips(downRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALLEFT, sal_True },
{ "strips(upRight)", ::com::sun::star::animations::TransitionType::WATERFALLWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTALRIGHT, sal_False },
{ "barn(inVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_False },
{ "barn(outVertical)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True },
{ "barn(inHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_False },
{ "barn(outHorizontal)", ::com::sun::star::animations::TransitionType::BARNDOORWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
{ "randombar(vertical)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True},
{ "randombar(horizontal)", ::com::sun::star::animations::TransitionType::RANDOMBARWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
{ "checkerboard(down)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::DOWN, sal_True},
{ "checkerboard(across)", ::com::sun::star::animations::TransitionType::CHECKERBOARDWIPE, ::com::sun::star::animations::TransitionSubType::ACROSS, sal_True },
{ "plus(out)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, sal_False },
{ "plus(in)", ::com::sun::star::animations::TransitionType::FOURBOXWIPE, ::com::sun::star::animations::TransitionSubType::CORNERSIN, sal_True },
{ "diamond(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, sal_True },
{ "diamond(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::DIAMOND, sal_False },
{ "circle(out)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
{ "circle(in)", ::com::sun::star::animations::TransitionType::ELLIPSEWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_False },
{ "box(out)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, sal_True },
{ "box(in)", ::com::sun::star::animations::TransitionType::IRISWIPE, ::com::sun::star::animations::TransitionSubType::RECTANGLE, sal_False },
{ "wedge", ::com::sun::star::animations::TransitionType::FANWIPE, ::com::sun::star::animations::TransitionSubType::CENTERTOP, sal_True },
{ "blinds(vertical)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::VERTICAL, sal_True },
{ "blinds(horizontal)", ::com::sun::star::animations::TransitionType::BLINDSWIPE, ::com::sun::star::animations::TransitionSubType::HORIZONTAL, sal_True },
{ "fade", ::com::sun::star::animations::TransitionType::FADE, ::com::sun::star::animations::TransitionSubType::CROSSFADE, sal_True },
{ "slide(fromTop)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMTOP, sal_True },
{ "slide(fromRight)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMRIGHT, sal_True },
{ "slide(fromLeft)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMLEFT, sal_True },
{ "slide(fromBottom)", ::com::sun::star::animations::TransitionType::SLIDEWIPE, ::com::sun::star::animations::TransitionSubType::FROMBOTTOM, sal_True },
{ "dissolve", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, sal_True },
{ "image", ::com::sun::star::animations::TransitionType::DISSOLVE, ::com::sun::star::animations::TransitionSubType::DEFAULT, sal_True }, // TODO
{ NULL, 0, 0, sal_False }
};

struct convert_subtype
{
    sal_Int32 mnID;
    const sal_Char* mpStrSubType;
};
static const convert_subtype gConvertArray[] =
{
    // fly in
    {   1, "from-top" },
    {   2, "from-right" },
    {   3, "from-top-right" },
    {   4, "from-bottom" },
    {   5, "horizontal" },
    {   6, "from-bottom-right" },
    {   8, "from-left" },
    {   9, "from-top-left" },
    {  10, "vertical" },
    {  12, "from-bottom-left" },
    {  16, "in" },
    {  21, "vertical-in" },
    {  26, "horizontal-in" },
    {  32, "out" },
    {  36, "out-from-screen-center" },
    {  37, "vertical-out" },
    {  42, "horizontal-out" },
    {  272, "in-slightly" },
    {  288, "out-slightly" },
    {  528, "in-from-screen-center" },
    {  0, 0 }
};

struct preset_maping
{
    sal_Int32   mnPresetClass;
    sal_Int32   mnPresetId;
    const sal_Char* mpStrPresetId;
};

static const preset_maping gPresetMaping[] =
{
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 1    ,"ooo-entrance-appear" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 2    ,"ooo-entrance-fly-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 3    ,"ooo-entrance-venetian-blinds" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 4    ,"ooo-entrance-box" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 5    ,"ooo-entrance-checkerboard" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 6    ,"ooo-entrance-circle" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 7    ,"ooo-entrance-fly-in-slow" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 8    ,"ooo-entrance-diamond" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 9    ,"ooo-entrance-dissolve-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 10   ,"ooo-entrance-fade-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 11   ,"ooo-entrance-flash-once" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 12   ,"ooo-entrance-peek-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 13   ,"ooo-entrance-plus" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 14   ,"ooo-entrance-random-bars" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 15   ,"ooo-entrance-spiral-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 16   ,"ooo-entrance-split" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 17   ,"ooo-entrance-stretchy" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 18   ,"ooo-entrance-diagonal-squares" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 19   ,"ooo-entrance-swivel" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 20   ,"ooo-entrance-wedge" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 21   ,"ooo-entrance-wheel" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 22   ,"ooo-entrance-wipe" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 23   ,"ooo-entrance-zoom" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 24   ,"ooo-entrance-random" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 25   ,"ooo-entrance-boomerang" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 26   ,"ooo-entrance-bounce" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 27   ,"ooo-entrance-colored-lettering" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 28   ,"ooo-entrance-movie-credits" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 29   ,"ooo-entrance-ease-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 30   ,"ooo-entrance-float" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 31   ,"ooo-entrance-turn-and-grow" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 34   ,"ooo-entrance-breaks" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 35   ,"ooo-entrance-pinwheel" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 37   ,"ooo-entrance-rise-up" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 38   ,"ooo-entrance-falling-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 39   ,"ooo-entrance-thread" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 40   ,"ooo-entrance-unfold" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 41   ,"ooo-entrance-whip" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 42   ,"ooo-entrance-ascend" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 43   ,"ooo-entrance-center-revolve" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 45   ,"ooo-entrance-fade-in-and-swivel" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 47   ,"ooo-entrance-descend" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 48   ,"ooo-entrance-sling" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 49   ,"ooo-entrance-spin-in" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 50   ,"ooo-entrance-compress" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 51   ,"ooo-entrance-magnify" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 52   ,"ooo-entrance-curve-up" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 53   ,"ooo-entrance-fade-in-and-zoom" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 54   ,"ooo-entrance-glide" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 55   ,"ooo-entrance-expand" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 56   ,"ooo-entrance-flip" },
    { ::com::sun::star::presentation::EffectPresetClass::ENTRANCE, 58   ,"ooo-entrance-fold" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 1    ,"ooo-emphasis-fill-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 2    ,"ooo-emphasis-font" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 3    ,"ooo-emphasis-font-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 4    ,"ooo-emphasis-font-size" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 5    ,"ooo-emphasis-font-style" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 6    ,"ooo-emphasis-grow-and-shrink" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 7    ,"ooo-emphasis-line-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 8    ,"ooo-emphasis-spin" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 9    ,"ooo-emphasis-transparency" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 10   ,"ooo-emphasis-bold-flash" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 14   ,"ooo-emphasis-blast" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 15   ,"ooo-emphasis-bold-reveal" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 16   ,"ooo-emphasis-color-over-by-word" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 18   ,"ooo-emphasis-reveal-underline" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 19   ,"ooo-emphasis-color-blend" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 20   ,"ooo-emphasis-color-over-by-letter" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 21   ,"ooo-emphasis-complementary-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 22   ,"ooo-emphasis-complementary-color-2" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 23   ,"ooo-emphasis-contrasting-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 24   ,"ooo-emphasis-darken" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 25   ,"ooo-emphasis-desaturate" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 26   ,"ooo-emphasis-flash-bulb" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 27   ,"ooo-emphasis-flicker" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 28   ,"ooo-emphasis-grow-with-color" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 30   ,"ooo-emphasis-lighten" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 31   ,"ooo-emphasis-style-emphasis" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 32   ,"ooo-emphasis-teeter" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 33   ,"ooo-emphasis-vertical-highlight" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 34   ,"ooo-emphasis-wave" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 35   ,"ooo-emphasis-blink" },
    { ::com::sun::star::presentation::EffectPresetClass::EMPHASIS, 36   ,"ooo-emphasis-shimmer" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 1        ,"ooo-exit-disappear" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 2        ,"ooo-exit-fly-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 3        ,"ooo-exit-venetian-blinds" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 4        ,"ooo-exit-box" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 5        ,"ooo-exit-checkerboard" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 6        ,"ooo-exit-circle" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 7        ,"ooo-exit-crawl-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 8        ,"ooo-exit-diamond" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 9        ,"ooo-exit-dissolve" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 10       ,"ooo-exit-fade-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 11       ,"ooo-exit-flash-once" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 12       ,"ooo-exit-peek-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 13       ,"ooo-exit-plus" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 14       ,"ooo-exit-random-bars" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 15       ,"ooo-exit-spiral-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 16       ,"ooo-exit-split" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 17       ,"ooo-exit-collapse" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 18       ,"ooo-exit-diagonal-squares" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 19       ,"ooo-exit-swivel" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 20       ,"ooo-exit-wedge" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 21       ,"ooo-exit-wheel" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 22       ,"ooo-exit-wipe" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 23       ,"ooo-exit-zoom" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 24       ,"ooo-exit-random" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 25       ,"ooo-exit-boomerang" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 26       ,"ooo-exit-bounce" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 27       ,"ooo-exit-colored-lettering" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 28       ,"ooo-exit-movie-credits" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 29       ,"ooo-exit-ease-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 30       ,"ooo-exit-float" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 31       ,"ooo-exit-turn-and-grow" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 34       ,"ooo-exit-breaks" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 35       ,"ooo-exit-pinwheel" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 37       ,"ooo-exit-sink-down" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 38       ,"ooo-exit-swish" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 39       ,"ooo-exit-thread" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 40       ,"ooo-exit-unfold" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 41       ,"ooo-exit-whip" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 42       ,"ooo-exit-descend" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 43       ,"ooo-exit-center-revolve" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 45       ,"ooo-exit-fade-out-and-swivel" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 47       ,"ooo-exit-ascend" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 48       ,"ooo-exit-sling" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 53       ,"ooo-exit-fade-out-and-zoom" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 55       ,"ooo-exit-contract" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 49       ,"ooo-exit-spin-out" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 50       ,"ooo-exit-stretchy" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 51       ,"ooo-exit-magnify" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 52       ,"ooo-exit-curve-down" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 54       ,"ooo-exit-glide" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 56       ,"ooo-exit-flip" },
    { ::com::sun::star::presentation::EffectPresetClass::EXIT, 58       ,"ooo-exit-fold" },




    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 16     ,"ooo-motionpath-4-point-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 5      ,"ooo-motionpath-5-point-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 11     ,"ooo-motionpath-6-point-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 17     ,"ooo-motionpath-8-point-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 1      ,"ooo-motionpath-circle" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 6      ,"ooo-motionpath-crescent-moon" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 3      ,"ooo-motionpath-diamond" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 13     ,"ooo-motionpath-equal-triangle" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 12     ,"ooo-motionpath-oval" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 9      ,"ooo-motionpath-heart" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 4      ,"ooo-motionpath-hexagon" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 10     ,"ooo-motionpath-octagon" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 14     ,"ooo-motionpath-parallelogram" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 15     ,"ooo-motionpath-pentagon" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 2      ,"ooo-motionpath-right-triangle" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 7      ,"ooo-motionpath-square" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 18     ,"ooo-motionpath-teardrop" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 8      ,"ooo-motionpath-trapezoid" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 37     ,"ooo-motionpath-arc-down" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 51     ,"ooo-motionpath-arc-left" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 58     ,"ooo-motionpath-arc-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 44     ,"ooo-motionpath-arc-up" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 41     ,"ooo-motionpath-bounce-left" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 54     ,"ooo-motionpath-bounce-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 48     ,"ooo-motionpath-curvy-left" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 61     ,"ooo-motionpath-curvy-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 60     ,"ooo-motionpath-decaying-wave" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 49     ,"ooo-motionpath-diagonal-down-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 56     ,"ooo-motionpath-diagonal-up-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 42     ,"ooo-motionpath-down" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 52     ,"ooo-motionpath-funnel" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 53     ,"ooo-motionpath-spring" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 62     ,"ooo-motionpath-stairs-down" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 50     ,"ooo-motionpath-turn-down" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 36     ,"ooo-motionpath-turn-down-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 43     ,"ooo-motionpath-turn-up" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 57     ,"ooo-motionpath-turn-up-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 64     ,"ooo-motionpath-up" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 47     ,"ooo-motionpath-wave" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 38     ,"ooo-motionpath-zigzag" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 31     ,"ooo-motionpath-bean" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 25     ,"ooo-motionpath-buzz-saw" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 20     ,"ooo-motionpath-curved-square" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 21     ,"ooo-motionpath-curved-x" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 23     ,"ooo-motionpath-curvy-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 28     ,"ooo-motionpath-figure-8-four" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 26     ,"ooo-motionpath-horizontal-figure-8" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 34     ,"ooo-motionpath-inverted-square" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 33     ,"ooo-motionpath-inverted-triangle" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 24     ,"ooo-motionpath-loop-de-loop" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 29     ,"ooo-motionpath-neutron" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 27     ,"ooo-motionpath-peanut" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 32     ,"ooo-motionpath-clover" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 19     ,"ooo-motionpath-pointy-star" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 30     ,"ooo-motionpath-swoosh" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 22     ,"ooo-motionpath-vertical-figure-8" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 35     ,"ooo-motionpath-left" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 63     ,"ooo-motionpath-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 55     ,"ooo-motionpath-spiral-left" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 46     ,"ooo-motionpath-spiral-right" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 40     ,"ooo-motionpath-sine-wave" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 59     ,"ooo-motionpath-s-curve-1" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 39     ,"ooo-motionpath-s-curve-2" },
    { ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH, 45     ,"ooo-motionpath-heartbeat" },


    { 0,0,0 }
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
