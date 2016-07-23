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

#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <svx/unoshape.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <CustomAnimationPreset.hxx>
#include <TransitionPreset.hxx>
#include <EffectMigration.hxx>
#include <anminfo.hxx>

using namespace ::sd;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::beans::NamedValue;

struct deprecated_FadeEffect_conversion_table_entry
{
    FadeEffect  meFadeEffect;
    const sal_Char* mpPresetId;
}

deprecated_FadeEffect_conversion_table[] =
{
// OOo 1.x transitions
    { FadeEffect_FADE_FROM_LEFT,            "wipe-right" },
    { FadeEffect_FADE_FROM_TOP,             "wipe-down" },
    { FadeEffect_FADE_FROM_RIGHT,           "wipe-left" },
    { FadeEffect_FADE_FROM_BOTTOM,          "wipe-up" },

    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-1-spoke" },

    { FadeEffect_UNCOVER_TO_LEFT,           "uncover-left" },
    { FadeEffect_UNCOVER_TO_UPPERLEFT,      "uncover-left-up" },
    { FadeEffect_UNCOVER_TO_TOP,            "uncover-up" },
    { FadeEffect_UNCOVER_TO_UPPERRIGHT,     "uncover-right-up" },
    { FadeEffect_UNCOVER_TO_RIGHT,          "uncover-right" },
    { FadeEffect_UNCOVER_TO_LOWERRIGHT,     "uncover-right-down" },
    { FadeEffect_UNCOVER_TO_BOTTOM,         "uncover-down" },
    { FadeEffect_UNCOVER_TO_LOWERLEFT,      "uncover-left-down" },

    { FadeEffect_VERTICAL_LINES,            "random-bars-vertical" },
    { FadeEffect_HORIZONTAL_LINES,          "random-bars-horizontal" },

    { FadeEffect_VERTICAL_CHECKERBOARD,     "checkerboard-down" },
    { FadeEffect_HORIZONTAL_CHECKERBOARD,   "checkerboard-across" },

    { FadeEffect_FADE_TO_CENTER,            "box-in" },
    { FadeEffect_FADE_FROM_CENTER,          "box-out" },

    { FadeEffect_VERTICAL_STRIPES,          "venetian-blinds-vertical" },
    { FadeEffect_HORIZONTAL_STRIPES,        "venetian-blinds-horizontal" },

    { FadeEffect_MOVE_FROM_LEFT,            "cover-right" },
    { FadeEffect_MOVE_FROM_TOP,             "cover-down" },
    { FadeEffect_MOVE_FROM_RIGHT,           "cover-left" },
    { FadeEffect_MOVE_FROM_BOTTOM,          "cover-up" },
    { FadeEffect_MOVE_FROM_UPPERLEFT,       "cover-right-down" },
    { FadeEffect_MOVE_FROM_UPPERRIGHT,      "cover-left-down" },
    { FadeEffect_MOVE_FROM_LOWERRIGHT,      "cover-left-up" },
    { FadeEffect_MOVE_FROM_LOWERLEFT,       "cover-right-up" },

    { FadeEffect_DISSOLVE,                  "dissolve" },

    { FadeEffect_RANDOM,                    "random-transition" },

    { FadeEffect_ROLL_FROM_LEFT,            "push-right" },
    { FadeEffect_ROLL_FROM_TOP,             "push-down" },
    { FadeEffect_ROLL_FROM_RIGHT,           "push-left" },
    { FadeEffect_ROLL_FROM_BOTTOM,          "push-up" },

    { FadeEffect_CLOSE_VERTICAL,            "split-horizontal-in" },
    { FadeEffect_CLOSE_HORIZONTAL,          "split-vertical-in" },
    { FadeEffect_OPEN_VERTICAL,             "split-horizontal-out" },
    { FadeEffect_OPEN_HORIZONTAL,           "split-vertical-out" },

    { FadeEffect_FADE_FROM_UPPERLEFT,       "diagonal-squares-right-down" },
    { FadeEffect_FADE_FROM_UPPERRIGHT,      "diagonal-squares-left-down" },
    { FadeEffect_FADE_FROM_LOWERLEFT,       "diagonal-squares-right-up" },
    { FadeEffect_FADE_FROM_LOWERRIGHT,      "diagonal-squares-left-up" },

// OOo 1.x transitions not in OOo 2.x
    { FadeEffect_CLOCKWISE,                 "clock-wipe-twelve" },
    { FadeEffect_COUNTERCLOCKWISE,          "reverse-clock-wipe-twelve" },
    { FadeEffect_SPIRALIN_LEFT,             "spiral-wipe-top-left-clockwise" },
    { FadeEffect_SPIRALIN_RIGHT,            "spiral-wipe-top-right-counter-clockwise" },
    { FadeEffect_SPIRALOUT_LEFT,            "spiral-wipe-out-to-bottom-right-clockwise" },
    { FadeEffect_SPIRALOUT_RIGHT,           "spiral-wipe-out-to-bottom-left-counter-clockwise" },
    { FadeEffect_WAVYLINE_FROM_LEFT,        "snake-wipe-top-left-vertical" },
    { FadeEffect_WAVYLINE_FROM_TOP,         "snake-wipe-top-left-horizontal" },
    { FadeEffect_WAVYLINE_FROM_RIGHT,       "snake-wipe-bottom-right-vertical" },
    { FadeEffect_WAVYLINE_FROM_BOTTOM,      "snake-wipe-bottom-right-horizontal" },
    { FadeEffect_STRETCH_FROM_LEFT,         "wipe-right" }, // todo
    { FadeEffect_STRETCH_FROM_TOP,          "wipe-down" },  // todo
    { FadeEffect_STRETCH_FROM_RIGHT,        "wipe-left" },  // todo
    { FadeEffect_STRETCH_FROM_BOTTOM,       "wipe-up" },    // todo

// OOo 1.x not available transitions

    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-2-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-3-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-4-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-8-spokes" },

    { FadeEffect_FADE_FROM_CENTER,          "shape-circle" },
    { FadeEffect_FADE_FROM_CENTER,          "shape-diamond" },
    { FadeEffect_FADE_FROM_CENTER,          "shape-plus" },

    { FadeEffect_CLOCKWISE,                 "wedge" },

    { FadeEffect_DISSOLVE,                  "fade-through-black" },

    { FadeEffect_CLOCKWISE,                 "zoom-rotate-in" },

    { FadeEffect_HORIZONTAL_LINES,          "comb-horizontal" },
    { FadeEffect_VERTICAL_LINES,            "comb-vertical" },

    { FadeEffect_DISSOLVE,                  "fade-smoothly" },

    { FadeEffect_NONE, nullptr }
};

/* todo
cut                             cut                                 (same as NONE?)
cut-through-black               cut         toBlack
wedge                           wedge
*/

void EffectMigration::SetFadeEffect( SdPage* pPage, css::presentation::FadeEffect eNewEffect)
{
    deprecated_FadeEffect_conversion_table_entry* pEntry = deprecated_FadeEffect_conversion_table;
    while( (pEntry->meFadeEffect != FadeEffect_NONE) && (pEntry->meFadeEffect != eNewEffect) )
        pEntry++;

    if( pEntry->mpPresetId )
    {
        const OUString aPresetId( OUString::createFromAscii( pEntry->mpPresetId ) );

        const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();

        TransitionPresetList::const_iterator aIt( rPresetList.begin());
        const TransitionPresetList::const_iterator aEndIt( rPresetList.end());
        for( ; aIt != aEndIt; ++aIt )
        {
            if( (*aIt)->getPresetId() == aPresetId)
            {
                pPage->setTransitionType( (*aIt)->getTransition() );
                pPage->setTransitionSubtype( (*aIt)->getSubtype() );
                pPage->setTransitionDirection( (*aIt)->getDirection() );
                pPage->setTransitionFadeColor( (*aIt)->getFadeColor() );
                break;
            }
        }
    }
    else
    {
        pPage->setTransitionType( 0 );
        pPage->setTransitionSubtype( 0 );
        pPage->setTransitionDirection( false );
        pPage->setTransitionFadeColor( 0 );
    }
}

FadeEffect EffectMigration::GetFadeEffect( const SdPage* pPage )
{
    const TransitionPresetList & rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIt( rPresetList.begin());
    const TransitionPresetList::const_iterator aEndIt( rPresetList.end());
    for( ; aIt != aEndIt; ++aIt )
    {
        if( ( (*aIt)->getTransition() == pPage->getTransitionType() ) &&
            ( (*aIt)->getSubtype() == pPage->getTransitionSubtype() ) &&
            ( (*aIt)->getDirection() == pPage->getTransitionDirection() ) &&
            ( (*aIt)->getFadeColor() == pPage->getTransitionFadeColor() ) )
        {
            const OUString& aPresetId = (*aIt)->getPresetId();

            deprecated_FadeEffect_conversion_table_entry* pEntry = deprecated_FadeEffect_conversion_table;
            while( (pEntry->meFadeEffect != FadeEffect_NONE) && (!aPresetId.equalsAscii( pEntry->mpPresetId ) ) )
                pEntry++;

            return pEntry->meFadeEffect;
        }
    }
    return FadeEffect_NONE;
}

struct deprecated_AnimationEffect_conversion_table_entry
{
    AnimationEffect meEffect;
    const sal_Char* mpPresetId;
    const sal_Char* mpPresetSubType;
}
deprecated_AnimationEffect_conversion_table[] =
{
// OOo 1.x entrance effects
    { AnimationEffect_APPEAR, "ooo-entrance-appear",nullptr },

    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-box","in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-box","out" },

    { AnimationEffect_VERTICAL_CHECKERBOARD, "ooo-entrance-checkerboard","downward" },
    { AnimationEffect_HORIZONTAL_CHECKERBOARD, "ooo-entrance-checkerboard","across" },

    { AnimationEffect_FADE_FROM_UPPERLEFT, "ooo-entrance-diagonal-squares","right-to-bottom" },
    { AnimationEffect_FADE_FROM_UPPERRIGHT, "ooo-entrance-diagonal-squares","left-to-bottom" },
    { AnimationEffect_FADE_FROM_LOWERLEFT, "ooo-entrance-diagonal-squares","right-to-top" },
    { AnimationEffect_FADE_FROM_LOWERRIGHT, "ooo-entrance-diagonal-squares","left-to-top" },

    { AnimationEffect_DISSOLVE, "ooo-entrance-dissolve-in",nullptr },

    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-fly-in","from-left" },
    { AnimationEffect_MOVE_FROM_TOP, "ooo-entrance-fly-in","from-top" },
    { AnimationEffect_MOVE_FROM_RIGHT, "ooo-entrance-fly-in","from-right" },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-fly-in","from-bottom" },
    { AnimationEffect_MOVE_FROM_UPPERLEFT, "ooo-entrance-fly-in","from-top-left" },
    { AnimationEffect_MOVE_FROM_UPPERRIGHT, "ooo-entrance-fly-in","from-top-right" },
    { AnimationEffect_MOVE_FROM_LOWERRIGHT, "ooo-entrance-fly-in","from-bottom-right" },
    { AnimationEffect_MOVE_FROM_LOWERLEFT, "ooo-entrance-fly-in","from-bottom-left" },

    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-fly-in-slow", "from-bottom" },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-fly-in-slow", "from-left" },
    { AnimationEffect_MOVE_FROM_RIGHT, "ooo-entrance-fly-in-slow", "from-right" },
    { AnimationEffect_MOVE_FROM_TOP, "ooo-entrance-fly-in-slow", "from-top" },

    { AnimationEffect_MOVE_SHORT_FROM_LEFT, "ooo-entrance-peek-in","from-left" },
    { AnimationEffect_MOVE_SHORT_FROM_TOP, "ooo-entrance-peek-in","from-top" },
    { AnimationEffect_MOVE_SHORT_FROM_RIGHT, "ooo-entrance-peek-in","from-right" },
    { AnimationEffect_MOVE_SHORT_FROM_BOTTOM, "ooo-entrance-peek-in","from-bottom" },

    { AnimationEffect_VERTICAL_LINES, "ooo-entrance-random-bars","horizontal" },
    { AnimationEffect_HORIZONTAL_LINES, "ooo-entrance-random-bars","vertical" },

    { AnimationEffect_RANDOM, "ooo-entrance-random",nullptr },

    { AnimationEffect_CLOSE_VERTICAL, "ooo-entrance-split","horizontal-in" },
    { AnimationEffect_CLOSE_HORIZONTAL, "ooo-entrance-split","vertical-in" },
    { AnimationEffect_OPEN_VERTICAL, "ooo-entrance-split","horizontal-out" },
    { AnimationEffect_OPEN_HORIZONTAL, "ooo-entrance-split","vertical-out" },

    { AnimationEffect_VERTICAL_STRIPES, "ooo-entrance-venetian-blinds","horizontal" },
    { AnimationEffect_HORIZONTAL_STRIPES, "ooo-entrance-venetian-blinds","vertical" },

    { AnimationEffect_FADE_FROM_LEFT, "ooo-entrance-wipe","from-left" },
    { AnimationEffect_FADE_FROM_TOP, "ooo-entrance-wipe","from-bottom" },
    { AnimationEffect_FADE_FROM_RIGHT, "ooo-entrance-wipe","from-right" },
    { AnimationEffect_FADE_FROM_BOTTOM, "ooo-entrance-wipe","from-top" },

    { AnimationEffect_HORIZONTAL_ROTATE, "ooo-entrance-swivel","vertical" },
    { AnimationEffect_VERTICAL_ROTATE, "ooo-entrance-swivel","horizontal" },

    { AnimationEffect_STRETCH_FROM_LEFT, "ooo-entrance-stretchy","from-left" },
    { AnimationEffect_STRETCH_FROM_UPPERLEFT, "ooo-entrance-stretchy","from-top-left" },
    { AnimationEffect_STRETCH_FROM_TOP, "ooo-entrance-stretchy","from-top" },
    { AnimationEffect_STRETCH_FROM_UPPERRIGHT, "ooo-entrance-stretchy","from-top-right" },
    { AnimationEffect_STRETCH_FROM_RIGHT, "ooo-entrance-stretchy","from-right" },
    { AnimationEffect_STRETCH_FROM_LOWERRIGHT, "ooo-entrance-stretchy","from-bottom-right" },
    { AnimationEffect_STRETCH_FROM_BOTTOM, "ooo-entrance-stretchy","from-bottom" },
    { AnimationEffect_STRETCH_FROM_LOWERLEFT, "ooo-entrance-stretchy","from-bottom-left" },

    { AnimationEffect_HORIZONTAL_STRETCH, "ooo-entrance-expand", nullptr },

    { AnimationEffect_CLOCKWISE, "ooo-entrance-wheel","1" },
    { AnimationEffect_COUNTERCLOCKWISE, "ooo-entrance-clock-wipe","counter-clockwise" },

    { AnimationEffect_SPIRALIN_LEFT, "ooo-entrance-spiral-wipe", "from-top-left-clockwise" },
    { AnimationEffect_SPIRALIN_RIGHT, "ooo-entrance-spiral-wipe", "from-top-right-counter-clockwise" },
    { AnimationEffect_SPIRALOUT_LEFT, "ooo-entrance-spiral-wipe", "from-center-clockwise" },
    { AnimationEffect_SPIRALOUT_RIGHT, "ooo-entrance-spiral-wipe", "from-center-counter-clockwise" },

    { AnimationEffect_WAVYLINE_FROM_LEFT, "ooo-entrance-snake-wipe","from-top-left-vertical" },
    { AnimationEffect_WAVYLINE_FROM_TOP, "ooo-entrance-snake-wipe","from-top-left-horizontal" },
    { AnimationEffect_WAVYLINE_FROM_RIGHT, "ooo-entrance-snake-wipe","from-bottom-right-vertical" },
    { AnimationEffect_WAVYLINE_FROM_BOTTOM, "ooo-entrance-snake-wipe","from-bottom-right-horizontal" },

// ooo 1.x exit effects
    { AnimationEffect_HIDE, "ooo-exit-disappear",nullptr },
    { AnimationEffect_MOVE_TO_LEFT, "ooo-exit-fly-out", "from-right" },
    { AnimationEffect_MOVE_TO_TOP, "ooo-exit-fly-out", "from-bottom" },
    { AnimationEffect_MOVE_TO_RIGHT, "ooo-exit-fly-out", "from-left" },
    { AnimationEffect_MOVE_TO_BOTTOM, "ooo-exit-fly-out", "from-top" },
    { AnimationEffect_MOVE_TO_UPPERLEFT, "ooo-exit-fly-out", "from-top-right" },
    { AnimationEffect_MOVE_TO_UPPERRIGHT, "ooo-exit-fly-out", "from-top-left" },
    { AnimationEffect_MOVE_TO_LOWERRIGHT, "ooo-exit-fly-out", "from-bottom-left" },
    { AnimationEffect_MOVE_TO_LOWERLEFT, "ooo-exit-fly-out", "from-bottom-right" },
    { AnimationEffect_MOVE_SHORT_TO_LEFT, "ooo-exit-peek-out", "from-right" },
    { AnimationEffect_MOVE_SHORT_TO_UPPERLEFT, "ooo-exit-peek-out", "from-right" },
    { AnimationEffect_MOVE_SHORT_TO_TOP, "ooo-exit-peek-out", "from-bottom" },
    { AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT, "ooo-exit-peek-out", "from-bottom" },
    { AnimationEffect_MOVE_SHORT_TO_RIGHT, "ooo-exit-peek-out", "from-left" },
    { AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT, "ooo-exit-peek-out","from-left" },
    { AnimationEffect_MOVE_SHORT_TO_BOTTOM, "ooo-exit-peek-out", "from-top" },
    { AnimationEffect_MOVE_SHORT_TO_LOWERLEFT, "ooo-exit-peek-out", "from-top" },

// no matching in OOo 2.x
    { AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT, "ooo-entrance-peek-in","from-left" },
    { AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT, "ooo-entrance-peek-in","from-top" },
    { AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT, "ooo-entrance-peek-in","from-right" },
    { AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT, "ooo-entrance-peek-in","from-bottom" },
    { AnimationEffect_LASER_FROM_LEFT, "ooo-entrance-fly-in","from-left" },
    { AnimationEffect_LASER_FROM_TOP, "ooo-entrance-fly-in","from-top" },
    { AnimationEffect_LASER_FROM_RIGHT, "ooo-entrance-fly-in","from-right" },
    { AnimationEffect_LASER_FROM_BOTTOM, "ooo-entrance-fly-in","from-bottom" },
    { AnimationEffect_LASER_FROM_UPPERLEFT, "ooo-entrance-fly-in","from-top-left" },
    { AnimationEffect_LASER_FROM_UPPERRIGHT, "ooo-entrance-fly-in","from-top-right" },
    { AnimationEffect_LASER_FROM_LOWERLEFT, "ooo-entrance-fly-in","from-bottom-left" },
    { AnimationEffect_LASER_FROM_LOWERRIGHT, "ooo-entrance-fly-in","from-bottom-right" },

// no matching in OOo 1.x

    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-circle", "in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-circle", "out" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-diamond", "in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-diamond", "out" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-plus", "in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-plus", "out" },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-wedge", nullptr },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-wheel", "2" },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-wheel", "3" },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-wheel", "4" },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-wheel", "8" },

    { AnimationEffect_MOVE_FROM_RIGHT, "ooo-entrance-boomerang", nullptr },
    { AnimationEffect_MOVE_FROM_UPPERRIGHT, "ooo-entrance-bounce", nullptr },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-curve-up", nullptr },
    { AnimationEffect_MOVE_FROM_TOP, "ooo-entrance-float", nullptr },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-glide", nullptr },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-magnify", nullptr },
    { AnimationEffect_HORIZONTAL_ROTATE, "ooo-entrance-pinwheel", nullptr },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-breaks", nullptr },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-sling", nullptr },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-spiral-in", nullptr },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-thread", nullptr },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-ascend", nullptr },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-center-revolve", nullptr },
    { AnimationEffect_APPEAR, "ooo-entrance-compress", nullptr },
    { AnimationEffect_MOVE_SHORT_FROM_TOP, "ooo-entrance-descend", nullptr },
    { AnimationEffect_MOVE_SHORT_FROM_LEFT, "ooo-entrance-ease-in", nullptr },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-rise-up", nullptr },
    { AnimationEffect_HORIZONTAL_ROTATE, "ooo-entrance-spin-in", nullptr },
    { AnimationEffect_STRETCH_FROM_LEFT, "ooo-entrance-stretchy", "across" },
    { AnimationEffect_STRETCH_FROM_TOP, "ooo-entrance-stretchy", "downward" },

    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-zoom","in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-zoom","in-slightly" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-zoom","in-from-screen-center" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-zoom","out" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-zoom","out-slightly" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-zoom","out-from-screen-center" },

    { AnimationEffect_DISSOLVE, "ooo-entrance-fade-in", nullptr },
    { AnimationEffect_DISSOLVE, "ooo-entrance-fade-in-and-zoom", nullptr },
    { AnimationEffect_DISSOLVE, "ooo-entrance-fade-in-and-swivel", nullptr },

    // still open (no matching effect: AnimationEffect_ZOOM_IN_FROM_*,
    // AnimationEffect_ZOOM_OUT_FROM_*, AnimationEffect_PATH

    { AnimationEffect_NONE, nullptr, nullptr }
};

EffectSequence::iterator ImplFindEffect( MainSequencePtr& pMainSequence, const Reference< XShape >& rShape, sal_Int16 nSubItem )
{
    EffectSequence::iterator aIter;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( (pEffect->getTargetShape() == rShape) && (pEffect->getTargetSubItem() == nSubItem) )
            break;
    }

    return aIter;
}

static bool implIsInsideGroup( SdrObject* pObj )
{
    return pObj && pObj->GetObjList() && pObj->GetObjList()->GetUpList();
}

void EffectMigration::SetAnimationEffect( SvxShape* pShape, AnimationEffect eEffect )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    OUString aPresetId;
    OUString aPresetSubType;

    if( !ConvertAnimationEffect( eEffect, aPresetId, aPresetSubType ) )
    {
        OSL_FAIL( "sd::EffectMigration::SetAnimationEffect(), no mapping for given AnimationEffect value" );
        return;
    }

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();

    CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( aPresetId ) );
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    if( pPreset.get() && pMainSequence.get() )
    {
        const Reference< XShape > xShape( pShape );

        EffectSequence::iterator aIterOnlyBackground( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_BACKGROUND ) );
        EffectSequence::iterator aIterAsWhole( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::AS_WHOLE ) );
        const EffectSequence::iterator aEnd( pMainSequence->getEnd() );

        if( (aIterOnlyBackground == aEnd) && (aIterAsWhole == aEnd) )
        {
            bool bEffectCreated = false;

            // check if there is already an text effect for this shape
            EffectSequence::iterator aIterOnlyText( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_TEXT ) );
            if( aIterOnlyText != aEnd )
            {
                // check if this is an animation text group
                sal_Int32 nGroupId = (*aIterOnlyText)->getGroupId();
                if( nGroupId >= 0 )
                {
                    CustomAnimationTextGroupPtr pGroup = pMainSequence->findGroup( nGroupId );
                    if( pGroup.get() )
                    {
                        // add an effect to animate the shape
                        pMainSequence->setAnimateForm( pGroup, true );

                        // find this effect
                        EffectSequence::iterator aIter( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_BACKGROUND ) );

                        if( aIter != aEnd )
                        {
                            if( ((*aIter)->getPresetId() != aPresetId) ||
                                ((*aIter)->getPresetSubType() != aPresetSubType) )
                            {
                                (*aIter)->replaceNode( pPreset->create( aPresetSubType ) );
                                pMainSequence->rebuild();
                                bEffectCreated = true;
                            }
                        }
                    }
                }
            }

            if( !bEffectCreated )
            {
                // if there is not yet an effect that target this shape, we generate one
                // we insert the shape effect before it
                Reference< XAnimationNode > xNode( pPreset->create( aPresetSubType ) );
                DBG_ASSERT( xNode.is(), "EffectMigration::SetAnimationEffect(), could not create preset!" );
                if( xNode.is() )
                {
                    CustomAnimationEffectPtr pEffect( new CustomAnimationEffect( xNode ) );
                    pEffect->setTarget( makeAny( xShape ) );
                    SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
                    const bool bManual = (pPage == nullptr) || (pPage->GetPresChange() == PRESCHANGE_MANUAL);
                    if( !bManual )
                        pEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );

                    pMainSequence->append( pEffect );

                    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_OUTLINETEXT ) )
                    {
                        // special case for outline text, effects are always mapped to text group effect
                        pMainSequence->
                            createTextGroup( pEffect, 10, bManual ? -1 : 0.0, false, false );
                    }
                }
            }
        }
        else
        {
            // if there is already an effect targeting this shape
            // just replace it
            CustomAnimationEffectPtr pEffect;
            if( aIterAsWhole != aEnd )
            {
                pEffect = (*aIterAsWhole);
            }
            else
            {
                pEffect = (*aIterOnlyBackground);
            }

            if( pEffect.get() )
            {
                if( (pEffect->getPresetId() != aPresetId) ||
                    (pEffect->getPresetSubType() != aPresetSubType) )
                {
                    pMainSequence->replace( pEffect, pPreset, aPresetSubType );
                }
            }
        }
    }
}

AnimationEffect EffectMigration::GetAnimationEffect( SvxShape* pShape )
{
    OUString aPresetId;
    OUString aPresetSubType;

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    if( pMainSequence.get() )
    {
        const Reference< XShape > xShape( pShape );

        EffectSequence::iterator aIter;

        for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
        {
            CustomAnimationEffectPtr pEffect( (*aIter) );
            if( pEffect->getTargetShape() == xShape )
            {
                if( (pEffect->getTargetSubItem() == ShapeAnimationSubType::ONLY_BACKGROUND) ||
                    (pEffect->getTargetSubItem() == ShapeAnimationSubType::AS_WHOLE))
                {
                    if( pEffect->getDuration() != 0.1 ) // ignore appear effects created from old text effect import
                    {
                        aPresetId = (*aIter)->getPresetId();
                        aPresetSubType = (*aIter)->getPresetSubType();
                        break;
                    }
                }
            }
        }
    }

    // now find old effect
    AnimationEffect eEffect = AnimationEffect_NONE;

    if( !ConvertPreset( aPresetId, &aPresetSubType, eEffect ) )
        ConvertPreset( aPresetId, nullptr, eEffect );

    return eEffect;
}

void EffectMigration::SetTextAnimationEffect( SvxShape* pShape, AnimationEffect eEffect )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    // first map the deprecated AnimationEffect to a preset and subtype
    OUString aPresetId;
    OUString aPresetSubType;

    if( !ConvertAnimationEffect( eEffect, aPresetId, aPresetSubType ) )
    {
        OSL_FAIL( "sd::EffectMigration::SetAnimationEffect(), no mapping for given AnimationEffect value" );
        return;
    }

    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );

    // ignore old text effects on shape without text
    if( (pTextObj == nullptr) || (!pTextObj->HasText()) )
        return;

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();

    // create an effect from this preset
    CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( aPresetId ) );

    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    if( pPreset.get() && pMainSequence.get() )
    {
        const Reference< XShape > xShape( pShape );

        EffectSequence::iterator aIterOnlyText( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_TEXT ) );
        const EffectSequence::iterator aEnd( pMainSequence->getEnd() );

        CustomAnimationTextGroupPtr pGroup;

        // is there already an animation text group for this shape?
        if( aIterOnlyText != aEnd )
        {
            const sal_Int32 nGroupId = (*aIterOnlyText)->getGroupId();
            if( nGroupId >= 0 )
                pGroup = pMainSequence->findGroup( nGroupId );
        }

        // if there is not yet a group, create it
        if( pGroup.get() == nullptr )
        {
            CustomAnimationEffectPtr pShapeEffect;

            EffectSequence::iterator aIterOnlyBackground( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_BACKGROUND ) );
            if( aIterOnlyBackground != aEnd )
            {
                pShapeEffect = (*aIterOnlyBackground);
            }
            else
            {
                EffectSequence::iterator aIterAsWhole( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::AS_WHOLE ) );
                if( aIterAsWhole != aEnd )
                {
                    pShapeEffect = (*aIterAsWhole);
                }
                else
                {
                    CustomAnimationPresetPtr pShapePreset( rPresets.getEffectDescriptor( "ooo-entrance-appear" ) );

                    Reference< XAnimationNode > xNode( pPreset->create( "" ) );
                    DBG_ASSERT( xNode.is(), "EffectMigration::SetTextAnimationEffect(), could not create preset!" );
                    if( xNode.is() )
                    {
                        pShapeEffect.reset( new CustomAnimationEffect( xNode ) );
                        pShapeEffect->setTarget( makeAny( xShape ) );
                        pShapeEffect->setDuration( 0.1 );
                        pMainSequence->append( pShapeEffect );

                        SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
                        if( pPage && pPage->GetPresChange() != PRESCHANGE_MANUAL )
                            pShapeEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );
                    }
                }
            }

            if( pShapeEffect.get() )
            {
                SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
                const bool bManual = (pPage == nullptr) || (pPage->GetPresChange() == PRESCHANGE_MANUAL);

                // now create effects for each paragraph
                pGroup =
                    pMainSequence->
                        createTextGroup( pShapeEffect, 10, bManual ? -1 : 0.0, true, false );
            }
        }

        if( pGroup.get() != nullptr )
        {
            const bool bLaserEffect = (eEffect >= AnimationEffect_LASER_FROM_LEFT) && (eEffect <= AnimationEffect_LASER_FROM_LOWERRIGHT);

            // now we have a group, so check if all effects are same as we like to have them
            const EffectSequence& rEffects = pGroup->getEffects();

            EffectSequence::const_iterator aIter;
            for( aIter = rEffects.begin(); aIter != rEffects.end(); ++aIter )
            {
                // only work on paragraph targets
                if( (*aIter)->getTarget().getValueType() == ::cppu::UnoType<ParagraphTarget>::get() )
                {
                    if( ((*aIter)->getPresetId() != aPresetId) ||
                        ((*aIter)->getPresetSubType() != aPresetSubType) )
                    {
                        (*aIter)->replaceNode( pPreset->create( aPresetSubType ) );
                    }

                    if( bLaserEffect )
                    {
                        (*aIter)->setIterateType( TextAnimationType::BY_LETTER );
                        (*aIter)->setIterateInterval( 0.5 );// TODO:
                                                             // Determine
                                                             // interval
                                                             // according
                                                             // to
                                                             // total
                                                             // effect
                                                             // duration
                    }
                }
            }
        }
        pMainSequence->rebuild();
    }
}

AnimationEffect EffectMigration::GetTextAnimationEffect( SvxShape* pShape )
{
    OUString aPresetId;
    OUString aPresetSubType;

    SdrObject* pObj = pShape->GetSdrObject();
    if( pObj )
    {
        sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

        if( pMainSequence.get() )
        {
            const Reference< XShape > xShape( pShape );
            EffectSequence::iterator aIter( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_TEXT ) );
            if( aIter != pMainSequence->getEnd() )
            {
                aPresetId = (*aIter)->getPresetId();
                aPresetSubType = (*aIter)->getPresetSubType();
            }
        }
    }

    // now find old effect
    AnimationEffect eEffect = AnimationEffect_NONE;

    if( !ConvertPreset( aPresetId, &aPresetSubType, eEffect ) )
        ConvertPreset( aPresetId, nullptr, eEffect );

    return eEffect;
}

bool EffectMigration::ConvertPreset( const OUString& rPresetId, const OUString* pPresetSubType, AnimationEffect& rEffect )
{
    rEffect = AnimationEffect_NONE;
    if( !rPresetId.isEmpty() )
    {
        // first try a match for preset id and subtype
        deprecated_AnimationEffect_conversion_table_entry* p = deprecated_AnimationEffect_conversion_table;
        while( p->mpPresetId )
        {
            if( rPresetId.equalsAscii( p->mpPresetId ) &&
                (( p->mpPresetSubType == nullptr ) ||
                 ( pPresetSubType == nullptr) ||
                 ( pPresetSubType->equalsAscii( p->mpPresetSubType )) ) )
            {
                rEffect = p->meEffect;
                return true;
            }
            p++;
        }
        return false;
    }
    else
    {
        // empty preset id means AnimationEffect_NONE
        return true;
    }
}

bool EffectMigration::ConvertAnimationEffect( const AnimationEffect& rEffect, OUString& rPresetId, OUString& rPresetSubType )
{
    deprecated_AnimationEffect_conversion_table_entry* p = deprecated_AnimationEffect_conversion_table;
    while( p->mpPresetId )
    {
        if( p->meEffect == rEffect )
        {
            rPresetId = OUString::createFromAscii( p->mpPresetId );
            rPresetSubType = OUString::createFromAscii( p->mpPresetSubType );
            return true;
        }
        p++;
    }

    return false;
}

double EffectMigration::ConvertAnimationSpeed( AnimationSpeed eSpeed )
{
    double fDuration;
    switch( eSpeed )
    {
    case AnimationSpeed_SLOW: fDuration = 2.0; break;
    case AnimationSpeed_FAST: fDuration = 0.5; break;
    default:
        fDuration = 1.0; break;
    }
    return fDuration;
}

void EffectMigration::SetAnimationSpeed( SvxShape* pShape, AnimationSpeed eSpeed )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    double fDuration = ConvertAnimationSpeed( eSpeed );

    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            if( pEffect->getDuration() != 0.1 )
                pEffect->setDuration( fDuration );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

AnimationSpeed EffectMigration::GetAnimationSpeed( SvxShape* pShape )
{
    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;

    double fDuration = 1.0;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            if( pEffect->getDuration() != 0.1 )
            {
                fDuration = pEffect->getDuration();
                break;
            }
        }
    }

    return ConvertDuration( fDuration );
}

AnimationSpeed EffectMigration::ConvertDuration( double fDuration )
{
    AnimationSpeed eSpeed;

    if( fDuration < 1.0 )
        eSpeed = AnimationSpeed_FAST;
    else if( fDuration > 1.5 )
        eSpeed = AnimationSpeed_SLOW;
    else
        eSpeed = AnimationSpeed_MEDIUM;

    return eSpeed;
}

void EffectMigration::SetDimColor( SvxShape* pShape, sal_Int32 nColor )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( true );
            pEffect->setDimColor( makeAny( nColor ) );
            pEffect->setAfterEffectOnNext( true );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

sal_Int32 EffectMigration::GetDimColor( SvxShape* pShape )
{
    sal_Int32 nColor = 0;
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj && pObj->GetPage() )
        {
            sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

            const Reference< XShape > xShape( pShape );
            EffectSequence::iterator aIter;

            for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
            {
                CustomAnimationEffectPtr pEffect( (*aIter) );
                if( (pEffect->getTargetShape() == xShape) &&
                    pEffect->getDimColor().hasValue() &&
                    pEffect->hasAfterEffect())
                {
                    pEffect->getDimColor() >>= nColor;
                    break;
                }
            }
        }
    }

    return nColor;
}

void EffectMigration::SetDimHide( SvxShape* pShape, bool bDimHide )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( bDimHide );
            if( bDimHide ) {
                Any aEmpty;
                pEffect->setDimColor( aEmpty );
            }
            pEffect->setAfterEffectOnNext( false );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

bool EffectMigration::GetDimHide( SvxShape* pShape )
{
    bool bRet = false;
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj && pObj->GetPage() )
        {
            sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

            const Reference< XShape > xShape( pShape );

            EffectSequence::iterator aIter;
            for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
            {
                CustomAnimationEffectPtr pEffect( (*aIter) );
                if( pEffect->getTargetShape() == xShape )
                {
                    bRet = pEffect->hasAfterEffect() &&
                            !pEffect->getDimColor().hasValue() &&
                            (!pEffect->IsAfterEffectOnNext());
                    break;
                }
            }
        }
    }

    return bRet;
}

void EffectMigration::SetDimPrevious( SvxShape* pShape, bool bDimPrevious )
{
    DBG_ASSERT( pShape && pShape->GetSdrObject() && pShape->GetSdrObject()->GetPage(),
                "sd::EffectMigration::SetAnimationEffect(), invalid argument!" );
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    if( implIsInsideGroup( pObj ) )
        return;

    Any aColor;

    if( bDimPrevious )
        aColor <<= (sal_Int32)COL_LIGHTGRAY;

    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( bDimPrevious );
            if( !bDimPrevious || !pEffect->getDimColor().hasValue() )
                pEffect->setDimColor( aColor );
            pEffect->setAfterEffectOnNext( true );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

bool EffectMigration::GetDimPrevious( SvxShape* pShape )
{
    bool bRet = false;
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj && pObj->GetPage() )
        {
            sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

            const Reference< XShape > xShape( pShape );

            EffectSequence::iterator aIter;
            for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
            {
                CustomAnimationEffectPtr pEffect( (*aIter) );
                if( pEffect->getTargetShape() == xShape )
                {
                    bRet = pEffect->hasAfterEffect() &&
                            pEffect->getDimColor().hasValue() &&
                            pEffect->IsAfterEffectOnNext();
                    break;
                }
            }
        }
    }

    return bRet;
}

void EffectMigration::SetPresentationOrder( SvxShape* pShape, sal_Int32 nNewPos )
{
    if( !pShape || !pShape->GetSdrObject() || !pShape->GetSdrObject()->GetPage() )
        return;

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    EffectSequence& rSequence = pMainSequence->getSequence();
    sal_Int32 nPos;
    sal_Int32 nCurrentPos = -1;
    std::vector< std::vector< EffectSequence::iterator > > aEffectVector(1);

    if( !rSequence.empty() )
    {
        Reference< XShape > xThis( pShape );
        Reference< XShape > xCurrent;

        EffectSequence::iterator aIter( rSequence.begin() );
        EffectSequence::iterator aEnd( rSequence.end() );
        for( nPos = 0; aIter != aEnd; ++aIter )
        {
            CustomAnimationEffectPtr pEffect = (*aIter);

            if( !xCurrent.is() )
            {
                xCurrent = pEffect->getTargetShape();
            }
            else if( pEffect->getTargetShape() != xCurrent )
            {
                nPos++;
                xCurrent = pEffect->getTargetShape();
                aEffectVector.resize( nPos+1 );
            }

            // is this the first effect for xThis shape?
            if(( nCurrentPos == -1 ) && ( xCurrent == xThis ) )
            {
                nCurrentPos = nPos;
            }

            aEffectVector[nPos].push_back( aIter );
        }
    }

    // check if there is at least one effect for xThis
    if( nCurrentPos == -1 )
    {
        OSL_FAIL("sd::EffectMigration::SetPresentationOrder() failed cause this shape has no effect" );
        return;
    }

    // check trivial case
    if( nCurrentPos != nNewPos )
    {
        std::vector< CustomAnimationEffectPtr > aEffects;

        std::vector< EffectSequence::iterator >::iterator aIter( aEffectVector[nCurrentPos].begin() );
        std::vector< EffectSequence::iterator >::iterator aEnd( aEffectVector[nCurrentPos].end() );
        while( aIter != aEnd )
        {
            aEffects.push_back( (*(*aIter)) );
            rSequence.erase( (*aIter++) );
        }

        if( nNewPos > nCurrentPos )
            nNewPos++;

        std::vector< CustomAnimationEffectPtr >::iterator aTempIter( aEffects.begin() );
        std::vector< CustomAnimationEffectPtr >::iterator aTempEnd( aEffects.end() );

        if( nNewPos == (sal_Int32)aEffectVector.size() )
        {
            while( aTempIter != aTempEnd )
            {
                rSequence.push_back( (*aTempIter++) );
            }
        }
        else
        {
            EffectSequence::iterator aPos( aEffectVector[nNewPos][0] );
            while( aTempIter != aTempEnd )
            {
                rSequence.insert( aPos, (*aTempIter++) );
            }
        }
    }
}

/** Returns the position of the given SdrObject in the Presentation order.
 *  This function returns -1 if the SdrObject is not in the Presentation order
 *  or if its the path-object.
 */
sal_Int32 EffectMigration::GetPresentationOrder( SvxShape* pShape )
{
    sal_Int32 nPos = -1, nFound = -1;

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    EffectSequence& rSequence = pMainSequence->getSequence();

    Reference< XShape > xThis( pShape );
    Reference< XShape > xCurrent;

    EffectSequence::iterator aIter( rSequence.begin() );
    EffectSequence::iterator aEnd( rSequence.end() );
    for( ; aIter != aEnd; ++aIter )
    {
        CustomAnimationEffectPtr pEffect = (*aIter);

        if( !xCurrent.is() || pEffect->getTargetShape() != xCurrent )
        {
            nPos++;
            xCurrent = pEffect->getTargetShape();

            // is this the first effect for xThis shape?
            if( xCurrent == xThis )
            {
                nFound = nPos;
                break;
            }
        }
    }

    return nFound;
}

void EffectMigration::UpdateSoundEffect( SvxShape* pShape, SdAnimationInfo* pInfo )
{
    if( pInfo )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

        const Reference< XShape > xShape( pShape );

        EffectSequence::iterator aIter;
        bool bNeedRebuild = false;

        OUString aSoundFile;
        if( pInfo->mbSoundOn )
            aSoundFile = pInfo->maSoundFile;

        for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); ++aIter )
        {
            CustomAnimationEffectPtr pEffect( (*aIter) );
            if( pEffect->getTargetShape() == xShape )
            {
                if( !aSoundFile.isEmpty() )
                {
                    pEffect->createAudio( makeAny( aSoundFile ) );
                }
                else
                {
                    pEffect->removeAudio();
                }
                bNeedRebuild = true;
            }
        }

        if( bNeedRebuild )
            pMainSequence->rebuild();
    }
}

OUString EffectMigration::GetSoundFile( SvxShape* pShape )
{
    OUString aSoundFile;

    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj && pObj->GetPage() )
        {
            sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

            const Reference< XShape > xShape( pShape );

            EffectSequence::iterator aIter;

            for(    aIter = pMainSequence->getBegin();
                    (aSoundFile.isEmpty()) && (aIter != pMainSequence->getEnd());
                    ++aIter )
            {
                CustomAnimationEffectPtr pEffect( (*aIter) );
                if( pEffect->getTargetShape() == xShape )
                {
                    if( pEffect->getAudio().is() )
                        pEffect->getAudio()->getSource() >>= aSoundFile;
                }
            }
        }
    }
    return aSoundFile;
}

bool EffectMigration::GetSoundOn( SvxShape* pShape )
{
    return !GetSoundFile( pShape ).isEmpty();
}

void EffectMigration::SetAnimationPath( SvxShape* pShape, SdrPathObj* pPathObj )
{
    if( pShape && pPathObj )
    {
        SdrObject* pObj = pShape->GetSdrObject();

        if( pObj )
        {
            const Reference< XShape > xShape( pShape );
            SdPage* pPage = dynamic_cast< SdPage* >(pPathObj->GetPage());
            if( pPage )
            {
                std::shared_ptr< sd::MainSequence > pMainSequence( pPage->getMainSequence() );
                if( pMainSequence.get() )
                    CustomAnimationEffectPtr pCreated( pMainSequence->append( *pPathObj, makeAny( xShape ), -1.0 ) );
            }
        }
    }
}

// #i42894# helper which creates the needed XAnimate for changing visibility and all the (currently) needed embeddings
void createVisibilityOnOffNode(Reference< XTimeContainer >& rxParentContainer, SdrObject& rCandidate, bool bVisible, bool bOnClick, double fDuration)
{
    Reference< XMultiServiceFactory > xMsf(::comphelper::getProcessServiceFactory());

    // create par container node
    Reference< XAnimationNode > xOuterSeqTimeContainer(xMsf->createInstance("com.sun.star.animations.ParallelTimeContainer"), UNO_QUERY_THROW);

    // set begin
    xOuterSeqTimeContainer->setBegin(Any((double)0.0));

    // set fill
    xOuterSeqTimeContainer->setFill(AnimationFill::HOLD);

    // set named values
    Sequence< NamedValue > aUserDataSequence;
    aUserDataSequence.realloc(1);

    aUserDataSequence[0].Name = "node-type";
    aUserDataSequence[0].Value <<= bOnClick ? EffectNodeType::ON_CLICK : EffectNodeType::AFTER_PREVIOUS;

    xOuterSeqTimeContainer->setUserData(aUserDataSequence);

    // create animate set to change visibility for rCandidate
    Reference< XAnimationNode > xAnimateSetForLast(xMsf->createInstance("com.sun.star.animations.AnimateSet"), UNO_QUERY_THROW);

    // set begin
    xAnimateSetForLast->setBegin(Any((double)0.0));

    // set duration
    xAnimateSetForLast->setDuration(Any(fDuration));

    // set fill
    xAnimateSetForLast->setFill(AnimationFill::HOLD);

    // set target
    Reference< XAnimate > xAnimate(xAnimateSetForLast, UNO_QUERY);
    Reference< XShape > xTargetShape(rCandidate.getUnoShape(), UNO_QUERY);
    xAnimate->setTarget(Any(xTargetShape));

    // set AttributeName
    xAnimate->setAttributeName("Visibility");

    // set attribute value
    xAnimate->setTo(Any(bVisible));

    // ad set node to par node
    Reference< XTimeContainer > xParentContainer(xOuterSeqTimeContainer, UNO_QUERY_THROW);
    xParentContainer->appendChild(xAnimateSetForLast);

    // add node
    rxParentContainer->appendChild(xOuterSeqTimeContainer);
}

// #i42894# older native formats supported animated group objects, that means all members of the group
// were shown animated by showing one after the other. This is no longer supported, but the following
// fallback will create the needed SMIL animation stuff. Unfortunately the members of the group
// have to be moved directly to the page, else the (explained to be generic, thus I expected this to
// work) animations will not work in slideshow
void EffectMigration::CreateAnimatedGroup(SdrObjGroup& rGroupObj, SdPage& rPage)
{
    // aw080 will give a vector immeditately
    SdrObjListIter aIter(rGroupObj);

    if(aIter.Count())
    {
        std::shared_ptr< sd::MainSequence > pMainSequence(rPage.getMainSequence());

        if(pMainSequence.get())
        {
            std::vector< SdrObject* > aObjects;
            aObjects.reserve(aIter.Count());

            while(aIter.IsMore())
            {
                // do move to page rough with old/current stuff, will be different in aw080 anyways
                SdrObject* pCandidate = aIter.Next();
                rGroupObj.GetSubList()->NbcRemoveObject(pCandidate->GetOrdNum());
                rPage.NbcInsertObject(pCandidate);
                aObjects.push_back(pCandidate);
            }

            // create main node
            Reference< XMultiServiceFactory > xMsf(::comphelper::getProcessServiceFactory());
            Reference< XAnimationNode > xOuterSeqTimeContainer(xMsf->createInstance("com.sun.star.animations.ParallelTimeContainer"), UNO_QUERY_THROW);

            // set begin
            xOuterSeqTimeContainer->setBegin(Any((double)(0.0)));

            // prepare parent container
            Reference< XTimeContainer > xParentContainer(xOuterSeqTimeContainer, UNO_QUERY_THROW);

            // prepare loop over objects
            SdrObject* pNext = nullptr;
            const double fDurationShow(0.2);
            const double fDurationHide(0.001);

            for(size_t a(0); a < aObjects.size(); a++)
            {
                SdrObject* pLast = pNext;
                pNext = aObjects[a];

                // create node
                if(pLast)
                {
                    createVisibilityOnOffNode(xParentContainer, *pLast, false, false, fDurationHide);
                }

                if(pNext)
                {
                    createVisibilityOnOffNode(xParentContainer, *pNext, true, !a, fDurationShow);
                }
            }

            // create end node
            if(pNext)
            {
                createVisibilityOnOffNode(xParentContainer, *pNext, false, false, fDurationHide);
            }

            // add to main sequence and rebuild
            pMainSequence->createEffects(xOuterSeqTimeContainer);
            pMainSequence->rebuild();
        }
    }
}

void EffectMigration::DocumentLoaded(SdDrawDocument & rDoc)
{
    if (DOCUMENT_TYPE_DRAW == rDoc.GetDocumentType())
        return; // no animations in Draw
    for (sal_uInt16 n = 0; n < rDoc.GetSdPageCount(PK_STANDARD); ++n)
    {
        SdPage *const pPage = rDoc.GetSdPage(n, PK_STANDARD);
        if (pPage->hasAnimationNode())
        {
            // this will force the equivalent of the MainSequence::onTimerHdl
            // so that the animations are present in export-able representation
            // *before* the import is finished
            pPage->getMainSequence()->getRootNode();
        }
    }
    for (sal_uInt16 n = 0; n < rDoc.GetMasterSdPageCount(PK_STANDARD); ++n)
    {
        SdPage *const pPage = rDoc.GetMasterSdPage(n, PK_STANDARD);
        if (pPage->hasAnimationNode())
        {
            pPage->getMainSequence()->getRootNode();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
