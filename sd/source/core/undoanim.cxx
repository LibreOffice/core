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

#include <sal/config.h>

#include <CustomAnimationCloner.hxx>

#include <undoanim.hxx>
#include <strings.hrc>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <CustomAnimationEffect.hxx>
#include <drawdoc.hxx>

namespace com { namespace sun { namespace star { namespace animations { class XAnimationNode; } } } }

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using namespace ::com::sun::star::animations;

namespace sd
{

struct UndoAnimationImpl
{
    SdPage*         mpPage;
    Reference< XAnimationNode > mxOldNode;
    Reference< XAnimationNode > mxNewNode;
    bool            mbNewNodeSet;
};

UndoAnimation::UndoAnimation( SdDrawDocument* pDoc, SdPage* pThePage )
: SdrUndoAction( *pDoc ), mpImpl( new UndoAnimationImpl )
{
    mpImpl->mpPage = pThePage;
    mpImpl->mbNewNodeSet = false;

    try
    {
        if( pThePage->mxAnimationNode.is() )
            mpImpl->mxOldNode = ::sd::Clone( pThePage->getAnimationNode() );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::UndoAnimation::UndoAnimation(), exception caught!");
    }
}

UndoAnimation::~UndoAnimation()
{
}

void UndoAnimation::Undo()
{
    try
    {
        if( !mpImpl->mbNewNodeSet )
        {
            if( mpImpl->mpPage->mxAnimationNode.is() )
                mpImpl->mxNewNode.set( ::sd::Clone( mpImpl->mpPage->mxAnimationNode ) );
            mpImpl->mbNewNodeSet = true;
        }

        Reference< XAnimationNode > xOldNode;
        if( mpImpl->mxOldNode.is() )
            xOldNode = ::sd::Clone( mpImpl->mxOldNode );

        mpImpl->mpPage->setAnimationNode( xOldNode );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::UndoAnimation::Undo(), exception caught!");
    }
}

void UndoAnimation::Redo()
{
    try
    {
        Reference< XAnimationNode > xNewNode;
        if( mpImpl->mxNewNode.is() )
            xNewNode = ::sd::Clone( mpImpl->mxNewNode );
        mpImpl->mpPage->setAnimationNode( xNewNode );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::UndoAnimation::Redo(), exception caught!");
    }
}

OUString UndoAnimation::GetComment() const
{
    return SdResId(STR_UNDO_ANIMATION);
}

struct UndoAnimationPathImpl
{
    SdPage*         mpPage;
    sal_Int32       mnEffectOffset;
    OUString msUndoPath;
    OUString msRedoPath;

    UndoAnimationPathImpl( SdPage* pThePage, const css::uno::Reference< css::animations::XAnimationNode >& xNode )
        : mpPage( pThePage )
        , mnEffectOffset( -1 )
    {
        if( !(mpPage && xNode.is()) )
            return;

        std::shared_ptr< sd::MainSequence > pMainSequence( mpPage->getMainSequence() );
        if( pMainSequence.get() )
        {
            CustomAnimationEffectPtr pEffect( pMainSequence->findEffect( xNode ) );
            if( pEffect.get() )
            {
                mnEffectOffset = pMainSequence->getOffsetFromEffect( pEffect );
                msUndoPath = pEffect->getPath();
            }
        }
    }
    UndoAnimationPathImpl(const UndoAnimationPathImpl&) = delete;
    UndoAnimationPathImpl& operator=(const UndoAnimationPathImpl&) = delete;

    CustomAnimationEffectPtr getEffect() const
    {
        CustomAnimationEffectPtr pEffect;
        if( mpPage && (mnEffectOffset >= 0) )
        {
            std::shared_ptr< sd::MainSequence > pMainSequence( mpPage->getMainSequence() );
            if( pMainSequence.get() )
                pEffect = pMainSequence->getEffectFromOffset( mnEffectOffset );
        }
        return pEffect;
    }
};

UndoAnimationPath::UndoAnimationPath( SdDrawDocument* pDoc, SdPage* pThePage, const css::uno::Reference< css::animations::XAnimationNode >& xNode )
: SdrUndoAction( *pDoc )
, mpImpl( new UndoAnimationPathImpl( pThePage, xNode ) )
{
}

UndoAnimationPath::~UndoAnimationPath()
{
}

void UndoAnimationPath::Undo()
{
    CustomAnimationEffectPtr pEffect = mpImpl->getEffect();
    if( pEffect.get() )
    {
        mpImpl->msRedoPath = pEffect->getPath();
        pEffect->setPath( mpImpl->msUndoPath );
    }
}

void UndoAnimationPath::Redo()
{
    CustomAnimationEffectPtr pEffect = mpImpl->getEffect();
    if( pEffect.get() )
    {
        pEffect->setPath( mpImpl->msRedoPath );
    }
}

OUString UndoAnimationPath::GetComment() const
{
    return SdResId(STR_UNDO_ANIMATION);
}

struct UndoTransitionImpl
{
    SdPage*         mpPage;

    sal_Int16 mnNewTransitionType;
    sal_Int16 mnNewTransitionSubtype;
    bool mbNewTransitionDirection;
    sal_Int32 mnNewTransitionFadeColor;
    double mfNewTransitionDuration;
    OUString maNewSoundFile;
    bool mbNewSoundOn;
    bool mbNewLoopSound;
    bool mbNewStopSound;

    sal_Int16 mnOldTransitionType;
    sal_Int16 mnOldTransitionSubtype;
    bool mbOldTransitionDirection;
    sal_Int32 mnOldTransitionFadeColor;
    double mfOldTransitionDuration;
    OUString maOldSoundFile;
    bool mbOldSoundOn;
    bool mbOldLoopSound;
    bool mbOldStopSound;
};

UndoTransition::UndoTransition( SdDrawDocument* _pDoc, SdPage* pThePage )
: SdUndoAction( _pDoc ), mpImpl( new UndoTransitionImpl )
{
    mpImpl->mpPage = pThePage;

    mpImpl->mnNewTransitionType = -1;
    mpImpl->mnOldTransitionType = pThePage->mnTransitionType;
    mpImpl->mnOldTransitionSubtype = pThePage->mnTransitionSubtype;
    mpImpl->mbOldTransitionDirection = pThePage->mbTransitionDirection;
    mpImpl->mnOldTransitionFadeColor = pThePage->mnTransitionFadeColor;
    mpImpl->mfOldTransitionDuration = pThePage->mfTransitionDuration;
    mpImpl->maOldSoundFile = pThePage->maSoundFile;
    mpImpl->mbOldSoundOn = pThePage->mbSoundOn;
    mpImpl->mbOldLoopSound = pThePage->mbLoopSound;
    mpImpl->mbOldStopSound = pThePage->mbStopSound;
}

UndoTransition::~UndoTransition()
{
}

void UndoTransition::Undo()
{
    if( mpImpl->mnNewTransitionType == -1 )
    {
        mpImpl->mnNewTransitionType = mpImpl->mpPage->mnTransitionType;
        mpImpl->mnNewTransitionSubtype = mpImpl->mpPage->mnTransitionSubtype;
        mpImpl->mbNewTransitionDirection = mpImpl->mpPage->mbTransitionDirection;
        mpImpl->mnNewTransitionFadeColor = mpImpl->mpPage->mnTransitionFadeColor;
        mpImpl->mfNewTransitionDuration = mpImpl->mpPage->mfTransitionDuration;
        mpImpl->maNewSoundFile = mpImpl->mpPage->maSoundFile;
        mpImpl->mbNewSoundOn = mpImpl->mpPage->mbSoundOn;
        mpImpl->mbNewLoopSound = mpImpl->mpPage->mbLoopSound;
        mpImpl->mbNewStopSound = mpImpl->mpPage->mbStopSound;
    }

    mpImpl->mpPage->mnTransitionType = mpImpl->mnOldTransitionType;
    mpImpl->mpPage->mnTransitionSubtype = mpImpl->mnOldTransitionSubtype;
    mpImpl->mpPage->mbTransitionDirection = mpImpl->mbOldTransitionDirection;
    mpImpl->mpPage->mnTransitionFadeColor = mpImpl->mnOldTransitionFadeColor;
    mpImpl->mpPage->mfTransitionDuration = mpImpl->mfOldTransitionDuration;
    mpImpl->mpPage->maSoundFile = mpImpl->maOldSoundFile;
    mpImpl->mpPage->mbSoundOn = mpImpl->mbOldSoundOn;
    mpImpl->mpPage->mbLoopSound = mpImpl->mbOldLoopSound;
    mpImpl->mpPage->mbStopSound = mpImpl->mbOldStopSound;
}

void UndoTransition::Redo()
{
    mpImpl->mpPage->mnTransitionType = mpImpl->mnNewTransitionType;
    mpImpl->mpPage->mnTransitionSubtype = mpImpl->mnNewTransitionSubtype;
    mpImpl->mpPage->mbTransitionDirection = mpImpl->mbNewTransitionDirection;
    mpImpl->mpPage->mnTransitionFadeColor = mpImpl->mnNewTransitionFadeColor;
    mpImpl->mpPage->mfTransitionDuration = mpImpl->mfNewTransitionDuration;
    mpImpl->mpPage->maSoundFile = mpImpl->maNewSoundFile;
    mpImpl->mpPage->mbSoundOn = mpImpl->mbNewSoundOn;
    mpImpl->mpPage->mbLoopSound = mpImpl->mbNewLoopSound;
    mpImpl->mpPage->mbStopSound = mpImpl->mbNewStopSound;
}

OUString UndoTransition::GetComment() const
{
    return SdResId(STR_UNDO_SLIDE_PARAMS);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
