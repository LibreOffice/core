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

typedef ::std::map< sal_Int32, css::uno::Any > PropertySetMap_t;

class PropertySet
{
public:
    PropertySetMap_t maProperties;

    bool hasProperty( sal_Int32 nProperty ) const;
    css::uno::Any getProperty( sal_Int32 nProperty ) const;
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

    friend SvStream& WriteAnimationNode(SvStream& rOut, AnimationNode& rAtom);
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
    bool mbDirection; // true: default geometric direction

    static const transition* find( const OUString& rName );
};
static const transition gTransitions[] =
{
{ "wipe(up)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, true },
{ "wipe(right)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, false },
{ "wipe(left)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, true },
{ "wipe(down)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, false },
{ "wheel(1)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::ONEBLADE, true },
{ "wheel(2)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::TWOBLADEVERTICAL, true },
{ "wheel(3)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::THREEBLADE, true },
{ "wheel(4)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::FOURBLADE, true },
{ "wheel(8)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::EIGHTBLADE, true },
{ "strips(downLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, true },
{ "strips(upLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, false },
{ "strips(downRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, true },
{ "strips(upRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, false },
{ "barn(inVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, false },
{ "barn(outVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, true },
{ "barn(inHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
{ "barn(outHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
{ "randombar(vertical)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::VERTICAL, true},
{ "randombar(horizontal)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
{ "checkerboard(down)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::DOWN, true},
{ "checkerboard(across)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::ACROSS, true },
{ "plus(out)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, false },
{ "plus(in)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, true },
{ "diamond(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, true },
{ "diamond(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, false },
{ "circle(out)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
{ "circle(in)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
{ "box(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, true },
{ "box(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, false },
{ "wedge", css::animations::TransitionType::FANWIPE, css::animations::TransitionSubType::CENTERTOP, true },
{ "blinds(vertical)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::VERTICAL, true },
{ "blinds(horizontal)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
{ "fade", css::animations::TransitionType::FADE, css::animations::TransitionSubType::CROSSFADE, true },
{ "slide(fromTop)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMTOP, true },
{ "slide(fromRight)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMRIGHT, true },
{ "slide(fromLeft)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMLEFT, true },
{ "slide(fromBottom)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMBOTTOM, true },
{ "dissolve", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true },
{ "image", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true }, // TODO
{ NULL, 0, 0, false }
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
    { css::presentation::EffectPresetClass::ENTRANCE, 1    ,"ooo-entrance-appear" },
    { css::presentation::EffectPresetClass::ENTRANCE, 2    ,"ooo-entrance-fly-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 3    ,"ooo-entrance-venetian-blinds" },
    { css::presentation::EffectPresetClass::ENTRANCE, 4    ,"ooo-entrance-box" },
    { css::presentation::EffectPresetClass::ENTRANCE, 5    ,"ooo-entrance-checkerboard" },
    { css::presentation::EffectPresetClass::ENTRANCE, 6    ,"ooo-entrance-circle" },
    { css::presentation::EffectPresetClass::ENTRANCE, 7    ,"ooo-entrance-fly-in-slow" },
    { css::presentation::EffectPresetClass::ENTRANCE, 8    ,"ooo-entrance-diamond" },
    { css::presentation::EffectPresetClass::ENTRANCE, 9    ,"ooo-entrance-dissolve-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 10   ,"ooo-entrance-fade-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 11   ,"ooo-entrance-flash-once" },
    { css::presentation::EffectPresetClass::ENTRANCE, 12   ,"ooo-entrance-peek-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 13   ,"ooo-entrance-plus" },
    { css::presentation::EffectPresetClass::ENTRANCE, 14   ,"ooo-entrance-random-bars" },
    { css::presentation::EffectPresetClass::ENTRANCE, 15   ,"ooo-entrance-spiral-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 16   ,"ooo-entrance-split" },
    { css::presentation::EffectPresetClass::ENTRANCE, 17   ,"ooo-entrance-stretchy" },
    { css::presentation::EffectPresetClass::ENTRANCE, 18   ,"ooo-entrance-diagonal-squares" },
    { css::presentation::EffectPresetClass::ENTRANCE, 19   ,"ooo-entrance-swivel" },
    { css::presentation::EffectPresetClass::ENTRANCE, 20   ,"ooo-entrance-wedge" },
    { css::presentation::EffectPresetClass::ENTRANCE, 21   ,"ooo-entrance-wheel" },
    { css::presentation::EffectPresetClass::ENTRANCE, 22   ,"ooo-entrance-wipe" },
    { css::presentation::EffectPresetClass::ENTRANCE, 23   ,"ooo-entrance-zoom" },
    { css::presentation::EffectPresetClass::ENTRANCE, 24   ,"ooo-entrance-random" },
    { css::presentation::EffectPresetClass::ENTRANCE, 25   ,"ooo-entrance-boomerang" },
    { css::presentation::EffectPresetClass::ENTRANCE, 26   ,"ooo-entrance-bounce" },
    { css::presentation::EffectPresetClass::ENTRANCE, 27   ,"ooo-entrance-colored-lettering" },
    { css::presentation::EffectPresetClass::ENTRANCE, 28   ,"ooo-entrance-movie-credits" },
    { css::presentation::EffectPresetClass::ENTRANCE, 29   ,"ooo-entrance-ease-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 30   ,"ooo-entrance-float" },
    { css::presentation::EffectPresetClass::ENTRANCE, 31   ,"ooo-entrance-turn-and-grow" },
    { css::presentation::EffectPresetClass::ENTRANCE, 34   ,"ooo-entrance-breaks" },
    { css::presentation::EffectPresetClass::ENTRANCE, 35   ,"ooo-entrance-pinwheel" },
    { css::presentation::EffectPresetClass::ENTRANCE, 37   ,"ooo-entrance-rise-up" },
    { css::presentation::EffectPresetClass::ENTRANCE, 38   ,"ooo-entrance-falling-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 39   ,"ooo-entrance-thread" },
    { css::presentation::EffectPresetClass::ENTRANCE, 40   ,"ooo-entrance-unfold" },
    { css::presentation::EffectPresetClass::ENTRANCE, 41   ,"ooo-entrance-whip" },
    { css::presentation::EffectPresetClass::ENTRANCE, 42   ,"ooo-entrance-ascend" },
    { css::presentation::EffectPresetClass::ENTRANCE, 43   ,"ooo-entrance-center-revolve" },
    { css::presentation::EffectPresetClass::ENTRANCE, 45   ,"ooo-entrance-fade-in-and-swivel" },
    { css::presentation::EffectPresetClass::ENTRANCE, 47   ,"ooo-entrance-descend" },
    { css::presentation::EffectPresetClass::ENTRANCE, 48   ,"ooo-entrance-sling" },
    { css::presentation::EffectPresetClass::ENTRANCE, 49   ,"ooo-entrance-spin-in" },
    { css::presentation::EffectPresetClass::ENTRANCE, 50   ,"ooo-entrance-compress" },
    { css::presentation::EffectPresetClass::ENTRANCE, 51   ,"ooo-entrance-magnify" },
    { css::presentation::EffectPresetClass::ENTRANCE, 52   ,"ooo-entrance-curve-up" },
    { css::presentation::EffectPresetClass::ENTRANCE, 53   ,"ooo-entrance-fade-in-and-zoom" },
    { css::presentation::EffectPresetClass::ENTRANCE, 54   ,"ooo-entrance-glide" },
    { css::presentation::EffectPresetClass::ENTRANCE, 55   ,"ooo-entrance-expand" },
    { css::presentation::EffectPresetClass::ENTRANCE, 56   ,"ooo-entrance-flip" },
    { css::presentation::EffectPresetClass::ENTRANCE, 58   ,"ooo-entrance-fold" },
    { css::presentation::EffectPresetClass::EMPHASIS, 1    ,"ooo-emphasis-fill-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 2    ,"ooo-emphasis-font" },
    { css::presentation::EffectPresetClass::EMPHASIS, 3    ,"ooo-emphasis-font-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 4    ,"ooo-emphasis-font-size" },
    { css::presentation::EffectPresetClass::EMPHASIS, 5    ,"ooo-emphasis-font-style" },
    { css::presentation::EffectPresetClass::EMPHASIS, 6    ,"ooo-emphasis-grow-and-shrink" },
    { css::presentation::EffectPresetClass::EMPHASIS, 7    ,"ooo-emphasis-line-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 8    ,"ooo-emphasis-spin" },
    { css::presentation::EffectPresetClass::EMPHASIS, 9    ,"ooo-emphasis-transparency" },
    { css::presentation::EffectPresetClass::EMPHASIS, 10   ,"ooo-emphasis-bold-flash" },
    { css::presentation::EffectPresetClass::EMPHASIS, 14   ,"ooo-emphasis-blast" },
    { css::presentation::EffectPresetClass::EMPHASIS, 15   ,"ooo-emphasis-bold-reveal" },
    { css::presentation::EffectPresetClass::EMPHASIS, 16   ,"ooo-emphasis-color-over-by-word" },
    { css::presentation::EffectPresetClass::EMPHASIS, 18   ,"ooo-emphasis-reveal-underline" },
    { css::presentation::EffectPresetClass::EMPHASIS, 19   ,"ooo-emphasis-color-blend" },
    { css::presentation::EffectPresetClass::EMPHASIS, 20   ,"ooo-emphasis-color-over-by-letter" },
    { css::presentation::EffectPresetClass::EMPHASIS, 21   ,"ooo-emphasis-complementary-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 22   ,"ooo-emphasis-complementary-color-2" },
    { css::presentation::EffectPresetClass::EMPHASIS, 23   ,"ooo-emphasis-contrasting-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 24   ,"ooo-emphasis-darken" },
    { css::presentation::EffectPresetClass::EMPHASIS, 25   ,"ooo-emphasis-desaturate" },
    { css::presentation::EffectPresetClass::EMPHASIS, 26   ,"ooo-emphasis-flash-bulb" },
    { css::presentation::EffectPresetClass::EMPHASIS, 27   ,"ooo-emphasis-flicker" },
    { css::presentation::EffectPresetClass::EMPHASIS, 28   ,"ooo-emphasis-grow-with-color" },
    { css::presentation::EffectPresetClass::EMPHASIS, 30   ,"ooo-emphasis-lighten" },
    { css::presentation::EffectPresetClass::EMPHASIS, 31   ,"ooo-emphasis-style-emphasis" },
    { css::presentation::EffectPresetClass::EMPHASIS, 32   ,"ooo-emphasis-teeter" },
    { css::presentation::EffectPresetClass::EMPHASIS, 33   ,"ooo-emphasis-vertical-highlight" },
    { css::presentation::EffectPresetClass::EMPHASIS, 34   ,"ooo-emphasis-wave" },
    { css::presentation::EffectPresetClass::EMPHASIS, 35   ,"ooo-emphasis-blink" },
    { css::presentation::EffectPresetClass::EMPHASIS, 36   ,"ooo-emphasis-shimmer" },
    { css::presentation::EffectPresetClass::EXIT, 1        ,"ooo-exit-disappear" },
    { css::presentation::EffectPresetClass::EXIT, 2        ,"ooo-exit-fly-out" },
    { css::presentation::EffectPresetClass::EXIT, 3        ,"ooo-exit-venetian-blinds" },
    { css::presentation::EffectPresetClass::EXIT, 4        ,"ooo-exit-box" },
    { css::presentation::EffectPresetClass::EXIT, 5        ,"ooo-exit-checkerboard" },
    { css::presentation::EffectPresetClass::EXIT, 6        ,"ooo-exit-circle" },
    { css::presentation::EffectPresetClass::EXIT, 7        ,"ooo-exit-crawl-out" },
    { css::presentation::EffectPresetClass::EXIT, 8        ,"ooo-exit-diamond" },
    { css::presentation::EffectPresetClass::EXIT, 9        ,"ooo-exit-dissolve" },
    { css::presentation::EffectPresetClass::EXIT, 10       ,"ooo-exit-fade-out" },
    { css::presentation::EffectPresetClass::EXIT, 11       ,"ooo-exit-flash-once" },
    { css::presentation::EffectPresetClass::EXIT, 12       ,"ooo-exit-peek-out" },
    { css::presentation::EffectPresetClass::EXIT, 13       ,"ooo-exit-plus" },
    { css::presentation::EffectPresetClass::EXIT, 14       ,"ooo-exit-random-bars" },
    { css::presentation::EffectPresetClass::EXIT, 15       ,"ooo-exit-spiral-out" },
    { css::presentation::EffectPresetClass::EXIT, 16       ,"ooo-exit-split" },
    { css::presentation::EffectPresetClass::EXIT, 17       ,"ooo-exit-collapse" },
    { css::presentation::EffectPresetClass::EXIT, 18       ,"ooo-exit-diagonal-squares" },
    { css::presentation::EffectPresetClass::EXIT, 19       ,"ooo-exit-swivel" },
    { css::presentation::EffectPresetClass::EXIT, 20       ,"ooo-exit-wedge" },
    { css::presentation::EffectPresetClass::EXIT, 21       ,"ooo-exit-wheel" },
    { css::presentation::EffectPresetClass::EXIT, 22       ,"ooo-exit-wipe" },
    { css::presentation::EffectPresetClass::EXIT, 23       ,"ooo-exit-zoom" },
    { css::presentation::EffectPresetClass::EXIT, 24       ,"ooo-exit-random" },
    { css::presentation::EffectPresetClass::EXIT, 25       ,"ooo-exit-boomerang" },
    { css::presentation::EffectPresetClass::EXIT, 26       ,"ooo-exit-bounce" },
    { css::presentation::EffectPresetClass::EXIT, 27       ,"ooo-exit-colored-lettering" },
    { css::presentation::EffectPresetClass::EXIT, 28       ,"ooo-exit-movie-credits" },
    { css::presentation::EffectPresetClass::EXIT, 29       ,"ooo-exit-ease-out" },
    { css::presentation::EffectPresetClass::EXIT, 30       ,"ooo-exit-float" },
    { css::presentation::EffectPresetClass::EXIT, 31       ,"ooo-exit-turn-and-grow" },
    { css::presentation::EffectPresetClass::EXIT, 34       ,"ooo-exit-breaks" },
    { css::presentation::EffectPresetClass::EXIT, 35       ,"ooo-exit-pinwheel" },
    { css::presentation::EffectPresetClass::EXIT, 37       ,"ooo-exit-sink-down" },
    { css::presentation::EffectPresetClass::EXIT, 38       ,"ooo-exit-swish" },
    { css::presentation::EffectPresetClass::EXIT, 39       ,"ooo-exit-thread" },
    { css::presentation::EffectPresetClass::EXIT, 40       ,"ooo-exit-unfold" },
    { css::presentation::EffectPresetClass::EXIT, 41       ,"ooo-exit-whip" },
    { css::presentation::EffectPresetClass::EXIT, 42       ,"ooo-exit-descend" },
    { css::presentation::EffectPresetClass::EXIT, 43       ,"ooo-exit-center-revolve" },
    { css::presentation::EffectPresetClass::EXIT, 45       ,"ooo-exit-fade-out-and-swivel" },
    { css::presentation::EffectPresetClass::EXIT, 47       ,"ooo-exit-ascend" },
    { css::presentation::EffectPresetClass::EXIT, 48       ,"ooo-exit-sling" },
    { css::presentation::EffectPresetClass::EXIT, 53       ,"ooo-exit-fade-out-and-zoom" },
    { css::presentation::EffectPresetClass::EXIT, 55       ,"ooo-exit-contract" },
    { css::presentation::EffectPresetClass::EXIT, 49       ,"ooo-exit-spin-out" },
    { css::presentation::EffectPresetClass::EXIT, 50       ,"ooo-exit-stretchy" },
    { css::presentation::EffectPresetClass::EXIT, 51       ,"ooo-exit-magnify" },
    { css::presentation::EffectPresetClass::EXIT, 52       ,"ooo-exit-curve-down" },
    { css::presentation::EffectPresetClass::EXIT, 54       ,"ooo-exit-glide" },
    { css::presentation::EffectPresetClass::EXIT, 56       ,"ooo-exit-flip" },
    { css::presentation::EffectPresetClass::EXIT, 58       ,"ooo-exit-fold" },

    { css::presentation::EffectPresetClass::MOTIONPATH, 16     ,"ooo-motionpath-4-point-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 5      ,"ooo-motionpath-5-point-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 11     ,"ooo-motionpath-6-point-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 17     ,"ooo-motionpath-8-point-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 1      ,"ooo-motionpath-circle" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 6      ,"ooo-motionpath-crescent-moon" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 3      ,"ooo-motionpath-diamond" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 13     ,"ooo-motionpath-equal-triangle" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 12     ,"ooo-motionpath-oval" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 9      ,"ooo-motionpath-heart" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 4      ,"ooo-motionpath-hexagon" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 10     ,"ooo-motionpath-octagon" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 14     ,"ooo-motionpath-parallelogram" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 15     ,"ooo-motionpath-pentagon" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 2      ,"ooo-motionpath-right-triangle" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 7      ,"ooo-motionpath-square" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 18     ,"ooo-motionpath-teardrop" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 8      ,"ooo-motionpath-trapezoid" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 37     ,"ooo-motionpath-arc-down" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 51     ,"ooo-motionpath-arc-left" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 58     ,"ooo-motionpath-arc-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 44     ,"ooo-motionpath-arc-up" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 41     ,"ooo-motionpath-bounce-left" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 54     ,"ooo-motionpath-bounce-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 48     ,"ooo-motionpath-curvy-left" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 61     ,"ooo-motionpath-curvy-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 60     ,"ooo-motionpath-decaying-wave" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 49     ,"ooo-motionpath-diagonal-down-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 56     ,"ooo-motionpath-diagonal-up-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 42     ,"ooo-motionpath-down" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 52     ,"ooo-motionpath-funnel" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 53     ,"ooo-motionpath-spring" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 62     ,"ooo-motionpath-stairs-down" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 50     ,"ooo-motionpath-turn-down" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 36     ,"ooo-motionpath-turn-down-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 43     ,"ooo-motionpath-turn-up" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 57     ,"ooo-motionpath-turn-up-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 64     ,"ooo-motionpath-up" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 47     ,"ooo-motionpath-wave" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 38     ,"ooo-motionpath-zigzag" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 31     ,"ooo-motionpath-bean" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 25     ,"ooo-motionpath-buzz-saw" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 20     ,"ooo-motionpath-curved-square" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 21     ,"ooo-motionpath-curved-x" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 23     ,"ooo-motionpath-curvy-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 28     ,"ooo-motionpath-figure-8-four" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 26     ,"ooo-motionpath-horizontal-figure-8" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 34     ,"ooo-motionpath-inverted-square" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 33     ,"ooo-motionpath-inverted-triangle" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 24     ,"ooo-motionpath-loop-de-loop" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 29     ,"ooo-motionpath-neutron" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 27     ,"ooo-motionpath-peanut" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 32     ,"ooo-motionpath-clover" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 19     ,"ooo-motionpath-pointy-star" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 30     ,"ooo-motionpath-swoosh" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 22     ,"ooo-motionpath-vertical-figure-8" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 35     ,"ooo-motionpath-left" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 63     ,"ooo-motionpath-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 55     ,"ooo-motionpath-spiral-left" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 46     ,"ooo-motionpath-spiral-right" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 40     ,"ooo-motionpath-sine-wave" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 59     ,"ooo-motionpath-s-curve-1" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 39     ,"ooo-motionpath-s-curve-2" },
    { css::presentation::EffectPresetClass::MOTIONPATH, 45     ,"ooo-motionpath-heartbeat" },

    { 0,0,0 }
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
