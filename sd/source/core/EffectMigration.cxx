/*************************************************************************
 *
 *  $RCSfile: EffectMigration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-18 16:45:03 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_SHAPEANIMATIONSUBTYPE_HPP_
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_TEXTANIMATIONTYPE_HPP_
#include <com/sun/star/presentation/TextAnimationType.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include <CustomAnimationPreset.hxx>
#endif

#ifndef _SD_TRANSITIONPRESET_HXX
#include <TransitionPreset.hxx>
#endif

#ifndef _SD_EFFECT_MIGRATION_HXX
#include <EffectMigration.hxx>
#endif

#ifndef _SD_ANMINFO_HXX
#include <anminfo.hxx>
#endif

using namespace ::vos;
using namespace ::rtl;
using namespace ::sd;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::drawing::XShape;


struct deprecated_FadeEffect_conversion_table_entry
{
    FadeEffect  meFadeEffect;
    const sal_Char* mpPresetId;
}
deprecated_FadeEffect_conversion_table[] =
{
    { FadeEffect_FADE_FROM_LEFT,            "wipe-right" },
    { FadeEffect_FADE_FROM_TOP,             "wipe-down" },
    { FadeEffect_FADE_FROM_RIGHT,           "wipe-left" },
    { FadeEffect_FADE_FROM_BOTTOM,          "wipe-up" },
    { FadeEffect_FADE_TO_CENTER,            "box-in" },
    { FadeEffect_FADE_FROM_CENTER,          "box-out" },
    { FadeEffect_MOVE_FROM_LEFT,            "cover-right" },
    { FadeEffect_MOVE_FROM_TOP,             "cover-down" },
    { FadeEffect_MOVE_FROM_RIGHT,           "cover-left" },
    { FadeEffect_MOVE_FROM_BOTTOM,          "cover-up" },
    { FadeEffect_ROLL_FROM_LEFT,            "push-right" },
    { FadeEffect_ROLL_FROM_TOP,             "push-down" },
    { FadeEffect_ROLL_FROM_RIGHT,           "push-left" },
    { FadeEffect_ROLL_FROM_BOTTOM,          "push-up" },
    { FadeEffect_VERTICAL_STRIPES,          "venetian-blinds-vertical" },
    { FadeEffect_HORIZONTAL_STRIPES,        "venetian-blinds-horizontal" },
    { FadeEffect_CLOCKWISE,                 "clock-wipe-twelve" },
    { FadeEffect_COUNTERCLOCKWISE,          "reverse-clock-wipe-twelve" },
    { FadeEffect_FADE_FROM_UPPERLEFT,       "diagonal-squares-right-down" },
    { FadeEffect_FADE_FROM_UPPERRIGHT,      "diagonal-squares-left-down" },
    { FadeEffect_FADE_FROM_LOWERLEFT,       "diagonal-squares-right-up" },
    { FadeEffect_FADE_FROM_LOWERRIGHT,      "diagonal-squares-left-up" },
    { FadeEffect_CLOSE_VERTICAL,            "split-horizontal-in" },
    { FadeEffect_CLOSE_HORIZONTAL,          "split-vertical-in" },
    { FadeEffect_OPEN_VERTICAL,             "split-horizontal-out" },
    { FadeEffect_OPEN_HORIZONTAL,           "split-vertical-out" },
    { FadeEffect_SPIRALIN_LEFT,             "spiral-wipe-top-left-clockwise" },
    { FadeEffect_SPIRALIN_RIGHT,            "spiral-wipe-top-right-counter-clockwise" },
    { FadeEffect_SPIRALOUT_LEFT,            "spiral-wipe-out-to-bottom-right-clockwise" },
    { FadeEffect_SPIRALOUT_RIGHT,           "spiral-wipe-out-to-bottom-left-counter-clockwise" },
    { FadeEffect_DISSOLVE,                  "dissolve" },
    { FadeEffect_WAVYLINE_FROM_LEFT,        "snake-wipe-top-left-vertical" },
    { FadeEffect_WAVYLINE_FROM_TOP,         "snake-wipe-top-left-horizontal" },
    { FadeEffect_WAVYLINE_FROM_RIGHT,       "snake-wipe-bottom-right-vertical" },
    { FadeEffect_WAVYLINE_FROM_BOTTOM,      "snake-wipe-bottom-right-horizontal" },
    { FadeEffect_RANDOM,                    "random-transition" },
    { FadeEffect_STRETCH_FROM_LEFT,         "wipe-right" }, // todo
    { FadeEffect_STRETCH_FROM_TOP,          "wipe-down" },  // todo
    { FadeEffect_STRETCH_FROM_RIGHT,        "wipe-left" },  // todo
    { FadeEffect_STRETCH_FROM_BOTTOM,       "wipe-up" },    // todo
    { FadeEffect_VERTICAL_LINES,            "random-bars-vertical" },
    { FadeEffect_HORIZONTAL_LINES,          "random-bars-horizontal" },
    { FadeEffect_MOVE_FROM_UPPERLEFT,       "cover-right-down" },
    { FadeEffect_MOVE_FROM_UPPERRIGHT,      "cover-left-down" },
    { FadeEffect_MOVE_FROM_LOWERRIGHT,      "cover-left-up" },
    { FadeEffect_MOVE_FROM_LOWERLEFT,       "cover-right-up" },
    { FadeEffect_UNCOVER_TO_LEFT,           "uncover-left" },
    { FadeEffect_UNCOVER_TO_UPPERLEFT,      "uncover-left-up" },
    { FadeEffect_UNCOVER_TO_TOP,            "uncover-up" },
    { FadeEffect_UNCOVER_TO_UPPERRIGHT,     "uncover-right-up" },
    { FadeEffect_UNCOVER_TO_RIGHT,          "uncover-right" },
    { FadeEffect_UNCOVER_TO_LOWERRIGHT,     "uncover-right-down" },
    { FadeEffect_UNCOVER_TO_BOTTOM,         "uncover-down" },
    { FadeEffect_UNCOVER_TO_LOWERLEFT,      "uncover-left-down" },
    { FadeEffect_VERTICAL_CHECKERBOARD,     "checkerboard-down" },
    { FadeEffect_HORIZONTAL_CHECKERBOARD,   "checkerboard-across" },

// the following effects have where not supported in OOo 1.0,
// so we match to a similiar effect

    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-1-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-2-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-3-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-4-spokes" },
    { FadeEffect_CLOCKWISE,                 "wheel-clockwise-8-spokes" },
    { FadeEffect_CLOCKWISE,                 "wedge" },
    { FadeEffect_CLOCKWISE,                 "zoom-rotate-in" },

    { FadeEffect_HORIZONTAL_LINES,          "comb-horizontal" },
    { FadeEffect_VERTICAL_LINES,            "comb-vertical" },

    { FadeEffect_DISSOLVE,                  "fade-smoothly" },
    { FadeEffect_DISSOLVE,                  "fade-through-black" },

    { FadeEffect_NONE, 0 }
};

/* todo
cut                             cut                                 (same as NONE?)
cut-through-black               cut         toBlack
shape-circle                    circle
shape-diamond                   diamond
shape-plus                      plus
wedge                           wedge
*/

void EffectMigration::SetFadeEffect( SdPage* pPage, ::com::sun::star::presentation::FadeEffect eNewEffect)
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
        pPage->setTransitionDirection( 0 );
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
    { AnimationEffect_FADE_FROM_LEFT, "ooo-entrance-wipe","from-left" },
    { AnimationEffect_FADE_FROM_TOP, "ooo-entrance-wipe","from-bottom" },
    { AnimationEffect_FADE_FROM_RIGHT, "ooo-entrance-wipe","from-right" },
    { AnimationEffect_FADE_FROM_BOTTOM, "ooo-entrance-wipe","from-top" },
    { AnimationEffect_FADE_TO_CENTER, "ooo-entrance-box","in" },
    { AnimationEffect_FADE_FROM_CENTER, "ooo-entrance-box","out" },
    { AnimationEffect_MOVE_FROM_LEFT, "ooo-entrance-fly-in","from-left" },
    { AnimationEffect_MOVE_FROM_TOP, "ooo-entrance-fly-in","from-top" },
    { AnimationEffect_MOVE_FROM_RIGHT, "ooo-entrance-fly-in","from-right" },
    { AnimationEffect_MOVE_FROM_BOTTOM, "ooo-entrance-fly-in","from-bottom" },
    { AnimationEffect_VERTICAL_STRIPES, "ooo-entrance-venetian-blinds","horizontal" },
    { AnimationEffect_HORIZONTAL_STRIPES, "ooo-entrance-venetian-blinds","vertical" },
    { AnimationEffect_CLOCKWISE, "ooo-entrance-clock-wipe","clockwise" },
    { AnimationEffect_COUNTERCLOCKWISE, "ooo-entrance-clock-wipe","counter-clockwise" },
    { AnimationEffect_FADE_FROM_UPPERLEFT, "ooo-entrance-diagonal-squares","right-to-bottom" },
    { AnimationEffect_FADE_FROM_UPPERRIGHT, "ooo-entrance-diagonal-squares","left-to-bottom" },
    { AnimationEffect_FADE_FROM_LOWERLEFT, "ooo-entrance-diagonal-squares","right-to-top" },
    { AnimationEffect_FADE_FROM_LOWERRIGHT, "ooo-entrance-diagonal-squares","left-to-top" },
    { AnimationEffect_CLOSE_VERTICAL, "ooo-entrance-split","horizontal-in" },
    { AnimationEffect_CLOSE_HORIZONTAL, "ooo-entrance-split","vertical-in" },
    { AnimationEffect_OPEN_VERTICAL, "ooo-entrance-split","horizontal-out" },
    { AnimationEffect_OPEN_HORIZONTAL, "ooo-entrance-split","vertical-out" },
    { AnimationEffect_PATH, "ooo-entrance-spiral-in",0 },
    { AnimationEffect_MOVE_TO_LEFT, "ooo-entrance-random",0 },
    { AnimationEffect_MOVE_TO_TOP, "ooo-entrance-random",0 },
    { AnimationEffect_MOVE_TO_RIGHT, "ooo-entrance-random",0 },
    { AnimationEffect_MOVE_TO_BOTTOM, "ooo-entrance-random",0 },
    { AnimationEffect_SPIRALIN_LEFT, "ooo-entrance-spiral-wipe", "from-top-left-clockwise" },
    { AnimationEffect_SPIRALIN_RIGHT, "ooo-entrance-spiral-wipe", "from-top-right-counter-clockwise" },
    { AnimationEffect_SPIRALOUT_LEFT, "ooo-entrance-spiral-wipe", "from-center-clockwise" },
    { AnimationEffect_SPIRALOUT_RIGHT, "ooo-entrance-spiral-wipe", "from-center-counter-clockwise" },
    { AnimationEffect_DISSOLVE, "ooo-entrance-dissolve-in",0 },
    { AnimationEffect_WAVYLINE_FROM_LEFT, "ooo-entrance-snake-wipe","from-top-left-vertical" },
    { AnimationEffect_WAVYLINE_FROM_TOP, "ooo-entrance-snake-wipe","from-top-left-horizontal" },
    { AnimationEffect_WAVYLINE_FROM_RIGHT, "ooo-entrance-snake-wipe","from-bottom-right-vertical" },
    { AnimationEffect_WAVYLINE_FROM_BOTTOM, "ooo-entrance-snake-wipe","from-bottom-right-horizontal" },
    { AnimationEffect_RANDOM, "ooo-entrance-random",0 },
    { AnimationEffect_VERTICAL_LINES, "ooo-entrance-random-bars","horizontal" },
    { AnimationEffect_HORIZONTAL_LINES, "ooo-entrance-random-bars","vertical" },
    { AnimationEffect_LASER_FROM_LEFT, "ooo-entrance-fly-in","from-left" },
    { AnimationEffect_LASER_FROM_TOP, "ooo-entrance-fly-in","from-top" },
    { AnimationEffect_LASER_FROM_RIGHT, "ooo-entrance-fly-in","from-right" },
    { AnimationEffect_LASER_FROM_BOTTOM, "ooo-entrance-fly-in","from-bottom" },
    { AnimationEffect_LASER_FROM_UPPERLEFT, "ooo-entrance-fly-in","from-top-left" },
    { AnimationEffect_LASER_FROM_UPPERRIGHT, "ooo-entrance-fly-in","from-top-right" },
    { AnimationEffect_LASER_FROM_LOWERLEFT, "ooo-entrance-fly-in","from-bottom-left" },
    { AnimationEffect_LASER_FROM_LOWERRIGHT, "ooo-entrance-fly-in","from-bottom-right" },
    { AnimationEffect_APPEAR, "ooo-entrance-appear",0 },
    { AnimationEffect_HIDE, "ooo-exit-disappear",0 },
    { AnimationEffect_MOVE_FROM_UPPERLEFT, "ooo-entrance-fly-in","from-top-left" },
    { AnimationEffect_MOVE_FROM_UPPERRIGHT, "ooo-entrance-fly-in","from-top-right" },
    { AnimationEffect_MOVE_FROM_LOWERRIGHT, "ooo-entrance-fly-in","from-bottom-right" },
    { AnimationEffect_MOVE_FROM_LOWERLEFT, "ooo-entrance-fly-in","from-bottom-left" },
    { AnimationEffect_MOVE_TO_UPPERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_TO_UPPERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_TO_LOWERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_TO_LOWERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_FROM_LEFT, "ooo-entrance-peek-in","from-left" },
    { AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT, "ooo-entrance-peek-in","from-left" },
    { AnimationEffect_MOVE_SHORT_FROM_TOP, "ooo-entrance-peek-in","from-top" },
    { AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT, "ooo-entrance-peek-in","from-top" },
    { AnimationEffect_MOVE_SHORT_FROM_RIGHT, "ooo-entrance-peek-in","from-right" },
    { AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT, "ooo-entrance-peek-in","from-right" },
    { AnimationEffect_MOVE_SHORT_FROM_BOTTOM, "ooo-entrance-peek-in","from-bottom" },
    { AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT, "ooo-entrance-peek-in","from-bottom" },
    { AnimationEffect_MOVE_SHORT_TO_LEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_UPPERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_TOP, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_RIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_BOTTOM, "ooo-entrance-appear",0 },
    { AnimationEffect_MOVE_SHORT_TO_LOWERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_VERTICAL_CHECKERBOARD, "ooo-entrance-checkerboard","downward" },
    { AnimationEffect_HORIZONTAL_CHECKERBOARD, "ooo-entrance-checkerboard","across" },
    { AnimationEffect_HORIZONTAL_ROTATE, "ooo-entrance-swivel","vertical" },
    { AnimationEffect_VERTICAL_ROTATE, "ooo-entrance-swivel","horizontal" },
    { AnimationEffect_HORIZONTAL_STRETCH, "ooo-entrance-stretchy","across" },
    { AnimationEffect_VERTICAL_STRETCH, "ooo-entrance-stretchy","downward" },
    { AnimationEffect_STRETCH_FROM_LEFT, "ooo-entrance-stretchy","from-left" },
    { AnimationEffect_STRETCH_FROM_UPPERLEFT, "ooo-entrance-stretchy","from-top-left" },
    { AnimationEffect_STRETCH_FROM_TOP, "ooo-entrance-stretchy","from-top" },
    { AnimationEffect_STRETCH_FROM_UPPERRIGHT, "ooo-entrance-stretchy","from-top-right" },
    { AnimationEffect_STRETCH_FROM_RIGHT, "ooo-entrance-stretchy","from-right" },
    { AnimationEffect_STRETCH_FROM_LOWERRIGHT, "ooo-entrance-stretchy","from-bottom-right" },
    { AnimationEffect_STRETCH_FROM_BOTTOM, "ooo-entrance-stretchy","from-bottom" },
    { AnimationEffect_STRETCH_FROM_LOWERLEFT, "ooo-entrance-stretchy","from-bottom-left" },
    { AnimationEffect_ZOOM_IN, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_SMALL, "ooo-entrance-zoom","in-slightly" },
    { AnimationEffect_ZOOM_IN_SPIRAL, "ooo-entrance-zoom","in-slightly" },
    { AnimationEffect_ZOOM_OUT, "ooo-entrance-zoom","out" },
    { AnimationEffect_ZOOM_OUT_SMALL, "ooo-entrance-zoom","out-slightly" },
    { AnimationEffect_ZOOM_OUT_SPIRAL, "ooo-entrance-zoom","out-slightly" },
    { AnimationEffect_ZOOM_IN_FROM_LEFT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_UPPERLEFT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_TOP, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_RIGHT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_BOTTOM, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_LOWERLEFT, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_IN_FROM_CENTER, "ooo-entrance-zoom","in" },
    { AnimationEffect_ZOOM_OUT_FROM_LEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_TOP, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_RIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_BOTTOM, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT, "ooo-entrance-appear",0 },
    { AnimationEffect_ZOOM_OUT_FROM_CENTER, "ooo-entrance-appear",0 },
    { AnimationEffect_NONE, 0, 0 }
};

EffectSequence::iterator ImplFindEffect( MainSequencePtr& pMainSequence, const Reference< XShape >& rShape, sal_Int16 nSubItem )
{
    EffectSequence::iterator aIter;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( (pEffect->getTargetShape() == rShape) && (pEffect->getTargetSubItem() == nSubItem) )
            break;
    }

    return aIter;
}

void EffectMigration::SetAnimationEffect( SvxShape* pShape, AnimationEffect eEffect )
{
    OUString aPresetId;
    OUString aPresetSubType;

    if( !ConvertAnimationEffect( eEffect, aPresetId, aPresetSubType ) )
    {
        DBG_ERROR( "sd::EffectMigration::SetAnimationEffect(), no mapping for given AnimationEffect value" );
        return;
    }

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();

    CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( aPresetId ) );
    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    if( pPreset.get() && pMainSequence.get() )
    {
        const Reference< XShape > xShape( pShape );

        EffectSequence::iterator aIterOnlyBackground( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::ONLY_BACKGROUND ) );
        EffectSequence::iterator aIterAsWhole( ImplFindEffect( pMainSequence, xShape, ShapeAnimationSubType::AS_WHOLE ) );
        const EffectSequence::iterator aEnd( pMainSequence->getEnd() );

        bool bEffectCreated = false;

        if( (aIterOnlyBackground == aEnd) && (aIterAsWhole == aEnd) )
        {
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
                CustomAnimationEffectPtr pEffect( new CustomAnimationEffect( pPreset->create( aPresetSubType ) ) );
                pEffect->setTarget( makeAny( xShape ) );
                SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
                if( pPage && pPage->GetPresChange() != PRESCHANGE_MANUAL )
                    pEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );

                pMainSequence->append( pEffect );
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

// --------------------------------------------------------------------

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

        for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
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
        ConvertPreset( aPresetId, 0, eEffect );

    return eEffect;
}


// --------------------------------------------------------------------

void EffectMigration::SetTextAnimationEffect( SvxShape* pShape, AnimationEffect eEffect )
{
    // first map the deprecated AnimationEffect to a preset and subtype
    OUString aPresetId;
    OUString aPresetSubType;

    if( !ConvertAnimationEffect( eEffect, aPresetId, aPresetSubType ) )
    {
        DBG_ERROR( "sd::EffectMigration::SetAnimationEffect(), no mapping for given AnimationEffect value" );
        return;
    }

    SdrObject* pObj = pShape->GetSdrObject();
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );

    // ignore old text effects on shape without text
    if( (pTextObj == 0) || (!pTextObj->HasText()) )
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
        if( pGroup.get() == 0 )
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
                    OUString aEmpty;
                    CustomAnimationPresetPtr pShapePreset( rPresets.getEffectDescriptor( OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo-entrance-appear" ) ) ) );
                    pShapeEffect.reset( new CustomAnimationEffect( pShapePreset->create( aEmpty ) ) );
                    pShapeEffect->setTarget( makeAny( xShape ) );
                    pShapeEffect->setDuration( 0.1 );
                    pMainSequence->append( pShapeEffect );

                    SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
                    if( pPage && pPage->GetPresChange() != PRESCHANGE_MANUAL )
                        pShapeEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );
                }
            }

            SdPage* pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
            const bool bManual = (pPage == 0) || (pPage->GetPresChange() == PRESCHANGE_MANUAL);

            // now create effects for each paragraph
            pGroup =
                pMainSequence->
                    createTextGroup( pShapeEffect, 10, pShapeEffect->getDuration(), sal_True, sal_False );
        }

        if( pGroup.get() != 0 )
        {
            const bool bLaserEffect = (eEffect >= AnimationEffect_LASER_FROM_LEFT) && (eEffect <= AnimationEffect_LASER_FROM_LOWERRIGHT);

            // now we have a group, so check if all effects are same as we like to have them
            const EffectSequence& rEffects = pGroup->getEffects();

            EffectSequence::const_iterator aIter;
            for( aIter = rEffects.begin(); aIter != rEffects.end(); aIter++ )
            {
                // only work on paragraph targets
                if( (*aIter)->getTarget().getValueType() == ::getCppuType((const ParagraphTarget*)0) )
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

// --------------------------------------------------------------------

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
        ConvertPreset( aPresetId, 0, eEffect );

    return eEffect;
}

// --------------------------------------------------------------------

bool EffectMigration::ConvertPreset( const OUString& rPresetId, const OUString* pPresetSubType, AnimationEffect& rEffect )
{
    rEffect = AnimationEffect_NONE;
    if( rPresetId.getLength() )
    {
        // first try a match for preset id and subtype
        deprecated_AnimationEffect_conversion_table_entry* p = deprecated_AnimationEffect_conversion_table;
        while( p->mpPresetId )
        {
            if( rPresetId.equalsAscii( p->mpPresetId ) &&
                (( p->mpPresetSubType == 0 ) ||
                 ( pPresetSubType == 0) ||
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

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

double EffectMigration::ConvertAnimationSpeed( AnimationSpeed eSpeed )
{
    double fDuration;
    switch( eSpeed )
    {
    case AnimationSpeed_SLOW: fDuration = 5.0; break;
    case AnimationSpeed_FAST: fDuration = 0.5; break;
    //case AnimationSpeed_MEDIUM:
    default:
        fDuration = 2.0; break;
    }
    return fDuration;
}
// --------------------------------------------------------------------

void EffectMigration::SetAnimationSpeed( SvxShape* pShape, AnimationSpeed eSpeed )
{
    double fDuration = ConvertAnimationSpeed( eSpeed );

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
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

// --------------------------------------------------------------------

AnimationSpeed EffectMigration::GetAnimationSpeed( SvxShape* pShape )
{
    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;

    double fDuration = 2.0;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
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

// --------------------------------------------------------------------

AnimationSpeed EffectMigration::ConvertDuration( double fDuration )
{
    AnimationSpeed eSpeed;

    if( fDuration < 1.5 )
        eSpeed = AnimationSpeed_FAST;
    else if( fDuration > 2.5 )
        eSpeed = AnimationSpeed_SLOW;
    else
        eSpeed = AnimationSpeed_MEDIUM;

    return eSpeed;
}

// --------------------------------------------------------------------

void EffectMigration::SetDimColor( SvxShape* pShape, sal_Int32 nColor )
{
    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( true );
            pEffect->setDimColor( makeAny( nColor ) );
            pEffect->setMasterRel( 2 );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

// --------------------------------------------------------------------

sal_Int32 EffectMigration::GetDimColor( SvxShape* pShape )
{
    // todo
    return 0;
}

// --------------------------------------------------------------------


void EffectMigration::SetDimHide( SvxShape* pShape, sal_Bool bDimHide )
{
    Any aEmpty;


    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( bDimHide ? true : false );
            if( bDimHide )
                pEffect->setDimColor( aEmpty );
            pEffect->setMasterRel( bDimHide ? 0 : 2 );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

// --------------------------------------------------------------------

sal_Bool EffectMigration::GetDimHide( SvxShape* pShape )
{
    // todo
    return sal_False;
}

// --------------------------------------------------------------------

void EffectMigration::SetDimPrevious( SvxShape* pShape, sal_Bool bDimPrevious )
{
    Any aColor;

    if( bDimPrevious )
        aColor <<= (sal_Int32)COL_LIGHTGRAY;

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    const Reference< XShape > xShape( pShape );

    EffectSequence::iterator aIter;
    bool bNeedRebuild = false;

    for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
    {
        CustomAnimationEffectPtr pEffect( (*aIter) );
        if( pEffect->getTargetShape() == xShape )
        {
            pEffect->setHasAfterEffect( bDimPrevious );
            if( !bDimPrevious || !pEffect->getDimColor().hasValue() )
                pEffect->setDimColor( aColor );
            pEffect->setMasterRel( 2 );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
        pMainSequence->rebuild();
}

// --------------------------------------------------------------------

sal_Bool EffectMigration::GetDimPrevious( SvxShape* pShape )
{
    return sal_False;
}

// --------------------------------------------------------------------

void EffectMigration::SetPresentationOrder( SvxShape* pShape, sal_Int32 nNewPos )
{
    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    EffectSequence& rSequence = pMainSequence->getSequence();
    sal_Int32 nPos;
    sal_Int32 nCurrentPos = -1;
    std::vector< std::vector< EffectSequence::iterator > > aEffectVector(1);

    Reference< XShape > xThis( pShape );
    Reference< XShape > xCurrent;

    EffectSequence::iterator aIter( rSequence.begin() );
    EffectSequence::iterator aEnd( rSequence.end() );
    for( nPos = 0; aIter != aEnd; aIter++ )
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

    // check if there is at least one effect for xThis
    if( nCurrentPos == -1 )
    {
        DBG_ERROR("sd::EffectMigration::SetPresentationOrder() failed cause this shape has no effect" );
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

        if( nNewPos == aEffectVector.size() )
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

// --------------------------------------------------------------------

/** Returns the position of the given SdrObject in the Presentation order.
 *  This function returns -1 if the SdrObject is not in the Presentation order
 *  or if its the path-object.
 */
sal_Int32 EffectMigration::GetPresentationOrder( SvxShape* pShape )
{
    sal_Int32 nPos = 0, nFound = -1;

    SdrObject* pObj = pShape->GetSdrObject();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();

    EffectSequence& rSequence = pMainSequence->getSequence();

    Reference< XShape > xThis( pShape );
    Reference< XShape > xCurrent;

    EffectSequence::iterator aIter( rSequence.begin() );
    EffectSequence::iterator aEnd( rSequence.end() );
    for( ; aIter != aEnd; aIter++ )
    {
        CustomAnimationEffectPtr pEffect = (*aIter);

        if( !xCurrent.is() )
        {
            xCurrent = pEffect->getTargetShape();
        }
        else if( pEffect->getTargetShape() != xCurrent )
        {
            nPos++;
        }

        // is this the first effect for xThis shape?
        if( xCurrent == xThis )
        {
            nFound = nPos;
            break;
        }
    }

    return nFound;
}

// --------------------------------------------------------------------

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
        if( pInfo->bSoundOn )
            aSoundFile = pInfo->aSoundFile;

        for( aIter = pMainSequence->getBegin(); aIter != pMainSequence->getEnd(); aIter++ )
        {
            CustomAnimationEffectPtr pEffect( (*aIter) );
            if( pEffect->getTargetShape() == xShape )
            {
                if( aSoundFile.getLength() )
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

// --------------------------------------------------------------------

OUString EffectMigration::GetSoundFile( SvxShape* pShape )
{
    // TODO
    OUString aEmpty;
    return aEmpty;
}

// --------------------------------------------------------------------

sal_Bool EffectMigration::GetSoundOn( SvxShape* pShape )
{
    return sal_False;
}

// --------------------------------------------------------------------

#if 0  // remove me

void EffectMigration::AddEffectFromAnimationInfo( SdrObject* pObject, SdAnimationInfo* pInfo )
{
    // todo check if pObject and pObject->GetPage() != 0
    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();
    sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObject->GetPage())->getMainSequence();
    Reference< XShape > xShape( pObject->getUnoObject(), UNO_QUERY );

    // first, create a shape effect, if needed

    if( nBuildType )
    {
        // now convert nFlyMethod and nFlyDirection to aPresetId and aPresetSubType
        // aPresetId and aPresetSubType match to the values in sd/xml/effects.xml
        OUString aPresetId;
        OUString aPresetSubType;

        // create an effect from the presets
        CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( aPresetId ) );
        // todo check if pPreset.get() != 0
        CustomAnimationEffectPtr pEffect( new CustomAnimationEffect( pPreset->create( aPresetSubType ) ) );
        // todo check if pEffect.get() != 0

        // set the shape targeted by this effect
        pEffect->setTarget( makeAny( xShape ) );

        // is the effect started on click or after the last effect (Another possible value is EffectNodeType::WITH_PREVIOUS )
        sal_Int16 nNodeType = ??? ? EffectNodeType::AFTER_PREVIOUS : EffectNodeType::ON_CLICK;
        pEffect->setNodeType( nNodeType );

        // calculate duration in seconds from nFlyMethod and nFlyDirection
        double fDuration = 1.0;
        pEffect->setDuration( fDuration );

        // set dim effects
        bool bDimHide = ???;
        bool bDimPrevious = ???;
        sal_Int32 nDimColor = ???;

        if( pDimPrevious || bDimHide )
        {
            pEffect->setHasAfterEffect( true );
            pEffect->setDimColor( makeAny( nDimColor ) );
            pEffect->setMasterRel( bDimHide ? 0 : 2 );
        }

        // set sound
        if( pAnim->nSoundRef && ( pAnim->nFlags & 0x0010 ) )
        {
            pEffect->createAudio( makeAny( aSoundFileUrl ) );
        }

        // text iteration
        if( pAnim->nSubEffect )
            pEffect->setIterateType( pAnim->nSubEffect == 2 ? TextAnimationType::BY_LETTER : TextAnimationType::BY_WORD );

        // append the effect to the main sequence
        pMainSequence->append( pEffect );

        if( nBuildType > 1 )
        {
            // create text effects
            sal_Int32 nTextGrouping = nBuildType - 1; // paragraph level that is animated
            double fTextGroupingAuto = ??; // set to -1 to wait on mouse clicks or >= 0 for a delay in seconds
            sal_Bool bAnimateForm = sal_True; // set to false if only the text will be animated
            sal_Bool bTextReverse = sal_False; // set to true if the text is animated reverse
            pMainSequence->
                createTextGroup( pEffect, nTextGrouping, fTextGroupingAuto, bAnimateForm, bTextReverse );
        }
    }
}

#endif
