/*************************************************************************
 *
 *  $RCSfile: sdpage_animations.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:47:52 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif

#include "sdpage.hxx"

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include <CustomAnimationPreset.hxx>
#endif

#ifndef _SD_TRANSITIONPRESET_HXX
#include <TransitionPreset.hxx>
#endif


using namespace ::vos;
using namespace ::rtl;
using namespace ::sd;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::drawing::XShape;

/** returns a helper class to manipulate effects inside the main sequence */
sd::MainSequencePtr SdPage::getMainSequence()
{
    if( 0 == mpMainSequence.get() )
        mpMainSequence.reset( new sd::MainSequence( getAnimationNode() ) );

    return mpMainSequence;
}

/** returns the main animation node */
Reference< XAnimationNode > SdPage::getAnimationNode() throw (RuntimeException)
{
    if( !mxAnimationNode.is() )
    {
        mxAnimationNode = Reference< XAnimationNode >::query(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.animations.ParallelTimeContainer"))));
        if( mxAnimationNode.is() )
        {
            Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
            aUserData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "node-type" ) );
            aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT;
            mxAnimationNode->setUserData( aUserData );
        }
    }

    return mxAnimationNode;
}

/** removes all custom animations for the given shape */
void SdPage::removeAnimations( const SdrObject* pObj )
{
    if( mpMainSequence.get() )
    {
        Reference< XShape > xShape( const_cast<SdrObject*>(pObj)->getUnoShape(), UNO_QUERY );
        mpMainSequence->disposeShape( xShape );
    }
}

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
    { FadeEffect_MOVE_FROM_LEFT,            "cover-left" },
    { FadeEffect_MOVE_FROM_TOP,             "cover-down" },
    { FadeEffect_MOVE_FROM_RIGHT,           "cover-right" },
    { FadeEffect_MOVE_FROM_BOTTOM,          "cover-up" },
    { FadeEffect_ROLL_FROM_LEFT,            "push-left" },
    { FadeEffect_ROLL_FROM_TOP,             "push-down" },
    { FadeEffect_ROLL_FROM_RIGHT,           "push-right" },
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
    { FadeEffect_SPIRALOUT_LEFT,            "reverse-spiral-wipe-bottom-right-counter-clockwise" },
    { FadeEffect_SPIRALOUT_RIGHT,           "reverse-spiral-wipe-bottom-left-counter-clockwise" },
    { FadeEffect_DISSOLVE,                  "dissolve" },
    { FadeEffect_WAVYLINE_FROM_LEFT,        "snake-wipe-top-left-vertical" },
    { FadeEffect_WAVYLINE_FROM_TOP,         "snake-wipe-top-left-horizontal" },
    { FadeEffect_WAVYLINE_FROM_RIGHT,       "snake-wipe-bottom-right-vertical" },
    { FadeEffect_WAVYLINE_FROM_BOTTOM,      "snake-wipe-bottom-right-horizontal" },
    { FadeEffect_RANDOM,                    "random" },
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
    { FadeEffect_VERTICAL_CHECKERBOARD,     "checkerboard-across" },
    { FadeEffect_HORIZONTAL_CHECKERBOARD,   "checkerboard-down" },

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

void SdPage::SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect)
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
                setTransitionType( (*aIt)->getTransition() );
                setTransitionSubtype( (*aIt)->getSubtype() );
                setTransitionDirection( (*aIt)->getDirection() );
                setTransitionFadeColor( (*aIt)->getFadeColor() );
                break;
            }
        }
    }
    else
    {
        setTransitionType( 0 );
        setTransitionSubtype( 0 );
        setTransitionDirection( 0 );
        setTransitionFadeColor( 0 );
    }
}

FadeEffect SdPage::GetFadeEffect() const
{
    const TransitionPresetList & rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIt( rPresetList.begin());
    const TransitionPresetList::const_iterator aEndIt( rPresetList.end());
    for( ; aIt != aEndIt; ++aIt )
    {
        if( ( (*aIt)->getTransition() == getTransitionType() ) &&
            ( (*aIt)->getSubtype() == getTransitionSubtype() ) &&
            ( (*aIt)->getDirection() == getTransitionDirection() ) &&
            ( (*aIt)->getFadeColor() == getTransitionFadeColor() ) )
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

/** callback from the sd::View when a new paragraph for one object on this page is created */
void SdPage::onParagraphInserted( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj )
{
    if( mpMainSequence.get() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape = Reference< XShape >( pObj->getUnoShape(), UNO_QUERY );
        aTarget.Paragraph = (sal_Int16)pOutliner->GetAbsPos( pPara );

        mpMainSequence->insertTextRange( makeAny( aTarget ) );
    }
}

/** callback from the sd::View when a paragraph from one object on this page is removed */
void SdPage::onParagraphRemoving( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj )
{
    if( mpMainSequence.get() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape = Reference< XShape >( pObj->getUnoShape(), UNO_QUERY );
        aTarget.Paragraph = (sal_Int16)pOutliner->GetAbsPos( pPara );

        mpMainSequence->disposeTextRange( makeAny( aTarget ) );
    }
}
