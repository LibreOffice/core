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

#include "commontimenodecontext.hxx"

#include <algorithm>

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>

#include "oox/helper/attributelist.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/ppt/pptimport.hxx"
#include <oox/ppt/pptfilterhelpers.hxx>
#include "oox/drawingml/drawingmltypes.hxx"

#include "animationtypes.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::xml::sax;

using ::com::sun::star::beans::NamedValue;

namespace oox { namespace ppt {

const convert_subtype* convert_subtype::getList()
{
    static const convert_subtype aList[] =
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

    return aList;
}

const preset_maping* preset_maping::getList()
{

    static const preset_maping aList[] =
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

    return aList;
};

OUString getConvertedSubType( sal_Int16 nPresetClass, sal_Int32 nPresetId, sal_Int32 nPresetSubType )
{
    const sal_Char* pStr = 0;

    if( (nPresetClass == EffectPresetClass::ENTRANCE) || (nPresetClass == EffectPresetClass::EXIT) )
    {
        // skip wheel effect
        if( nPresetId != 21 )
        {
            if( nPresetId == 5 )
            {
                // checkerboard
                switch( nPresetSubType )
                {
                case  5: pStr = "downward"; break;
                case 10: pStr = "across"; break;
                }
            }
            else if( nPresetId == 17 )
            {
                // stretch
                if( nPresetSubType == 10 )
                    pStr = "across";
            }
            else if( nPresetId == 18 )
            {
                // strips
                switch( nPresetSubType )
                {
                case 3: pStr = "right-to-top"; break;
                case 6: pStr = "right-to-bottom"; break;
                case 9: pStr = "left-to-top"; break;
                case 12: pStr = "left-to-bottom"; break;
                }
            }

            if( pStr == 0 )
            {
                const convert_subtype* p = convert_subtype::getList();

                while( p->mpStrSubType )
                {
                    if( p->mnID == nPresetSubType )
                    {
                        pStr = p->mpStrSubType;
                        break;
                    }
                    p++;
                }
            }
        }
    }

    if( pStr )
        return OUString::createFromAscii( pStr );
    else
        return OUString::number( nPresetSubType );
}

    CommonTimeNodeContext::CommonTimeNodeContext(
            FragmentHandler2& rParent,
            sal_Int32  aElement,
            const Reference< XFastAttributeList >& xAttribs,
            const TimeNodePtr & pNode )
        : TimeNodeContext( rParent, aElement, xAttribs, pNode )
            , mbIterate( false )
    {
        AttributeList attribs( xAttribs );
        sal_Int32 nInt; // some temporary int value for float conversions

        NodePropertyMap & aProps = pNode->getNodeProperties();
        TimeNode::UserDataMap & aUserData = pNode->getUserData();

        if( attribs.hasAttribute( XML_accel ) )
        {
            double dPercent = ::oox::drawingml::GetPositiveFixedPercentage( xAttribs->getOptionalValue( XML_accel ) );
            aProps[ NP_ACCELERATION ] <<= dPercent;
        }

        if( attribs.hasAttribute( XML_afterEffect ) )
        {
            aUserData[ "after-effect" ]
                = makeAny( attribs.getBool( XML_afterEffect, false ) );
        }
        aProps[ NP_AUTOREVERSE ] = makeAny( attribs.getBool( XML_autoRev, false ) );

        // TODO
        if( attribs.hasAttribute( XML_bldLvl ) )
        {
            attribs.getInteger( XML_bldLvl, 0 );
        }
        if( attribs.hasAttribute( XML_decel ) )
        {
            double dPercent = ::oox::drawingml::GetPositiveFixedPercentage( xAttribs->getOptionalValue( XML_decel ) );
            aProps[ NP_DECELERATE ] <<= dPercent;
        }
        // TODO
        if( attribs.hasAttribute( XML_display ) )
        {
            aProps[ NP_DISPLAY ] <<= attribs.getBool( XML_display, true );
        }
        if( attribs.hasAttribute( XML_dur ) )
        {
            aProps[ NP_DURATION ] = GetTime( xAttribs->getOptionalValue( XML_dur) );
        }
        // TODO
        if( attribs.hasAttribute( XML_evtFilter ) )
        {
            xAttribs->getOptionalValue( XML_evtFilter );
        }
        // ST_TLTimeNodeFillType
        if( attribs.hasAttribute( XML_fill ) )
        {
            nInt = xAttribs->getOptionalValueToken( XML_fill, 0 );
            if( nInt != 0 )
            {
                sal_Int16 nEnum;
                switch( nInt )
                {
                case XML_remove:
                    nEnum = AnimationFill::REMOVE;
                    break;
                case XML_freeze:
                    nEnum = AnimationFill::FREEZE;
                    break;
                case XML_hold:
                    nEnum = AnimationFill::HOLD;
                    break;
                case XML_transition:
                    nEnum = AnimationFill::TRANSITION;
                    break;
                default:
                    nEnum = AnimationFill::DEFAULT;
                    break;
                }
                aProps[ NP_FILL ] <<=  (sal_Int16)nEnum;
            }
        }
        if( attribs.hasAttribute( XML_grpId ) )
        {
            attribs.getUnsigned( XML_grpId, 0 );
        }
        // ST_TLTimeNodeID
        if( attribs.hasAttribute( XML_id ) )
        {
            sal_uInt32 nId = attribs.getUnsigned( XML_id, 0 );
            pNode->setId( nId );
        }
        // ST_TLTimeNodeMasterRelation
        nInt = xAttribs->getOptionalValueToken( XML_masterRel, 0 );
        if( nInt )
        {
            // TODO
            switch(nInt)
            {
            case XML_sameClick:
            case XML_lastClick:
            case XML_nextClick:
                break;
            }
        }

        // TODO
        if( attribs.hasAttribute( XML_nodePh ) )
        {
            attribs.getBool( XML_nodePh, false );
        }
        // ST_TLTimeNodeType
        nInt = xAttribs->getOptionalValueToken( XML_nodeType, 0 );
        if( nInt != 0 )
        {
            sal_Int16 nEnum;
            switch( nInt )
            {
            case XML_clickEffect:
            case XML_clickPar:
                nEnum = EffectNodeType::ON_CLICK;
                break;
            case XML_withEffect:
            case XML_withGroup:
                nEnum = EffectNodeType::WITH_PREVIOUS;
                break;
            case XML_mainSeq:
                nEnum = EffectNodeType::MAIN_SEQUENCE;
                break;
            case XML_interactiveSeq:
                nEnum = EffectNodeType::INTERACTIVE_SEQUENCE;
                break;
            case XML_afterGroup:
            case XML_afterEffect:
                nEnum = EffectNodeType::AFTER_PREVIOUS;
                break;
            case XML_tmRoot:
                nEnum = EffectNodeType::TIMING_ROOT;
                break;
            default:
                nEnum = EffectNodeType::DEFAULT;
                break;
            }
            aUserData[ "node-type" ] <<= nEnum;
        }

        // ST_TLTimeNodePresetClassType
        nInt = xAttribs->getOptionalValueToken( XML_presetClass, 0 );
        if( nInt != 0 )
        {
            sal_Int16 nEffectPresetClass = 0;
            // TODO put that in a function
            switch( nInt )
            {
            case XML_entr:
                nEffectPresetClass = EffectPresetClass::ENTRANCE;
                break;
            case XML_exit:
                nEffectPresetClass = EffectPresetClass::EXIT;
                break;
            case XML_emph:
                nEffectPresetClass = EffectPresetClass::EMPHASIS;
                break;
            case XML_path:
                nEffectPresetClass = EffectPresetClass::MOTIONPATH;
                break;
            case XML_verb:
                // TODO check that the value below is correct
                nEffectPresetClass = EffectPresetClass::OLEACTION;
                break;
            case XML_mediacall:
                nEffectPresetClass = EffectPresetClass::MEDIACALL;
                break;
            default:
                nEffectPresetClass = 0;
                break;
            }
            aUserData[ "preset-class" ] = makeAny( nEffectPresetClass );
            if( attribs.hasAttribute( XML_presetID ) )
            {
                sal_Int32 nPresetId = attribs.getInteger( XML_presetID, 0 );
                const preset_maping* p = preset_maping::getList();
                while( p->mpStrPresetId && ((p->mnPresetClass != nEffectPresetClass) || (p->mnPresetId != nPresetId )) )
                    p++;

                aUserData[ "preset-id" ]
                    = makeAny( OUString::createFromAscii( p->mpStrPresetId ) );
                sal_Int32 nPresetSubType = attribs.getInteger( XML_presetSubtype, 0 );
                if( nPresetSubType )
                {
                    aUserData[ "preset-sub-type" ] = makeAny( getConvertedSubType( nEffectPresetClass, nPresetId, nPresetSubType ) );
                }
            }
        }
        if( attribs.hasAttribute( XML_repeatCount ) )
        {
            aProps[ NP_REPEATCOUNT ] = GetTime( xAttribs->getOptionalValue( XML_repeatCount ) );
        }
        /* see pptinanimation */
//          aProps[ NP_REPEATCOUNT ] <<= (fCount < ((float)3.40282346638528860e+38)) ? makeAny( (double)fCount ) : makeAny( Timing_INDEFINITE );
        if( attribs.hasAttribute( XML_repeatDur ) )
        {
            aProps[ NP_REPEATDURATION ] = GetTime( xAttribs->getOptionalValue( XML_repeatDur ) );
        }
        // TODO repeatDur is otherwise the same as dur. What shall we do? -- Hub

        // ST_TLTimeNodeRestartType
        nInt = xAttribs->getOptionalValueToken( XML_restart, 0 );
        if( nInt != 0 )
        {
            // TODO put that in a function
            sal_Int16 nEnum;
            switch( nInt )
            {
            case XML_always:
                nEnum = AnimationRestart::ALWAYS;
                break;
            case XML_whenNotActive:
                nEnum = AnimationRestart::WHEN_NOT_ACTIVE;
                break;
            case XML_never:
                nEnum = AnimationRestart::NEVER;
                break;
            default:
                nEnum = AnimationRestart::DEFAULT;
                break;
            }
            aProps[ NP_RESTART ] <<= (sal_Int16)nEnum;
        }
        // ST_Percentage TODO
        xAttribs->getOptionalValue( XML_spd /*"10000" */ );
        // ST_TLTimeNodeSyncType TODO
        xAttribs->getOptionalValue( XML_syncBehavior );
        // TODO (string)
        xAttribs->getOptionalValue( XML_tmFilter );
    }

    CommonTimeNodeContext::~CommonTimeNodeContext( ) throw ( )
    {
    }

    void CommonTimeNodeContext::onEndElement()
    {
        if( isCurrentElement( PPT_TOKEN( iterate ) ) )
        {
            mbIterate = false;
        }
    }

    ::oox::core::ContextHandlerRef CommonTimeNodeContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {

        switch ( aElementToken )
        {
        case PPT_TOKEN( childTnLst ):
        case PPT_TOKEN( subTnLst ):
            return new TimeNodeListContext( *this, mpNode->getChildren() );

        case PPT_TOKEN( stCondLst ):
            return new CondListContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode, mpNode->getStartCondition() );
        case PPT_TOKEN( endCondLst ):
            return new CondListContext( *this, aElementToken, rAttribs.getFastAttributeList(), mpNode, mpNode->getEndCondition() );

        case PPT_TOKEN( endSync ):
            return new CondContext( *this, rAttribs.getFastAttributeList(), mpNode, mpNode->getEndSyncValue() );
        case PPT_TOKEN( iterate ):
        {
            sal_Int32 nVal = rAttribs.getToken( XML_type, XML_el );
            if( nVal != 0 )
            {
                // TODO put that in a function
                sal_Int16 nEnum;
                switch( nVal )
                {
                case XML_el:
                    nEnum = TextAnimationType::BY_PARAGRAPH;
                    break;
                case XML_lt:
                    nEnum = TextAnimationType::BY_LETTER;
                    break;
                case XML_wd:
                    nEnum = TextAnimationType::BY_WORD;
                    break;
                default:
                    // default is BY_WORD. See Ppt97Animation::GetTextAnimationType()
                    // in sd/source/filter/ppt/ppt97animations.cxx:297
                    nEnum = TextAnimationType::BY_WORD;
                    break;
                }
                mpNode->getNodeProperties()[ NP_ITERATETYPE ] <<= nEnum;
            }
            // in case of exception we ignore the whole tag.
            // TODO what to do with this
            /*bool bBackwards =*/ rAttribs.getBool( XML_backwards, false );
            mbIterate = true;
            return this;
        }
        case PPT_TOKEN( tmAbs ):
            if( mbIterate )
            {
                double fTime = rAttribs.getUnsigned( XML_val, 0 );
                // time in ms. property is in % TODO
                mpNode->getNodeProperties()[ NP_ITERATEINTERVAL ] <<= fTime;
            }
            return this;
        case PPT_TOKEN( tmPct ):
            if( mbIterate )
            {
                double fPercent = (double)rAttribs.getUnsigned( XML_val, 0 ) / 100000.0;
                mpNode->getNodeProperties()[ NP_ITERATEINTERVAL ] <<= fPercent;
            }
            return this;
        default:
            break;
        }

        return this;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
