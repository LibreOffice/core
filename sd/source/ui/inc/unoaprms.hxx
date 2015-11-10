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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNOAPRMS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNOAPRMS_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "sdundo.hxx"
#include <svx/svdopath.hxx>
#include <tools/color.hxx>

class SdDrawDocument;
class SdrObject;

class SdAnimationPrmsUndoAction : public SdUndoAction
{
    SdrObject*      pObject;
    bool            bOldActive;
    bool            bNewActive;
    bool            bOldDimPrevious;
    bool            bNewDimPrevious;
    bool            bOldDimHide;
    bool            bNewDimHide;
    bool            bOldSoundOn;
    bool            bNewSoundOn;
    bool            bOldSecondSoundOn;
    bool            bNewSecondSoundOn;
    bool            bOldPlayFull;
    bool            bNewPlayFull;
    bool            bOldSecondPlayFull;
    bool            bNewSecondPlayFull;
    css::presentation::AnimationEffect eOldEffect;
    css::presentation::AnimationEffect eNewEffect;
    css::presentation::AnimationEffect eOldTextEffect;
    css::presentation::AnimationEffect eNewTextEffect;
    css::presentation::AnimationSpeed  eOldSpeed;
    css::presentation::AnimationSpeed  eNewSpeed;
    css::presentation::AnimationEffect eOldSecondEffect;
    css::presentation::AnimationEffect eNewSecondEffect;
    css::presentation::AnimationSpeed  eOldSecondSpeed;
    css::presentation::AnimationSpeed  eNewSecondSpeed;
    Color           aOldDimColor;
    Color           aNewDimColor;
    OUString        aOldSoundFile;
    OUString        aNewSoundFile;
    SdrPathObj*     pOldPathObj;
    SdrPathObj*     pNewPathObj;
    css::presentation::ClickAction     eOldClickAction;
    css::presentation::ClickAction     eNewClickAction;
    OUString        aOldBookmark;
    OUString        aNewBookmark;
    sal_uInt16          nOldVerb;
    sal_uInt16          nNewVerb;
    sal_uLong           nOldPresOrder;
    sal_uLong           nNewPresOrder;

    bool            bInfoCreated;

public:
    TYPEINFO_OVERRIDE();
    SdAnimationPrmsUndoAction(SdDrawDocument* pTheDoc, SdrObject* pObj,
                                  bool bCreated)
        : SdUndoAction(pTheDoc)
        , pObject(pObj)
        , bOldActive(false)
        , bNewActive(false)
        , bOldDimPrevious(false)
        , bNewDimPrevious(false)
        , bOldDimHide(false)
        , bNewDimHide(false)
        , bOldSoundOn(false)
        , bNewSoundOn(false)
        , bOldSecondSoundOn(false)
        , bNewSecondSoundOn(false)
        , bOldPlayFull(false)
        , bNewPlayFull(false)
        , bOldSecondPlayFull(false)
        , bNewSecondPlayFull(false)
        , eOldEffect(css::presentation::AnimationEffect_NONE)
        , eNewEffect(css::presentation::AnimationEffect_NONE)
        , eOldTextEffect(css::presentation::AnimationEffect_NONE)
        , eNewTextEffect(css::presentation::AnimationEffect_NONE)
        , eOldSpeed(css::presentation::AnimationSpeed_SLOW)
        , eNewSpeed(css::presentation::AnimationSpeed_SLOW)
        , eOldSecondEffect(css::presentation::AnimationEffect_NONE)
        , eNewSecondEffect(css::presentation::AnimationEffect_NONE)
        , eOldSecondSpeed(css::presentation::AnimationSpeed_SLOW)
        , eNewSecondSpeed(css::presentation::AnimationSpeed_SLOW)
        , pOldPathObj(nullptr)
        , pNewPathObj(nullptr)
        , eOldClickAction(css::presentation::ClickAction_NONE)
        , eNewClickAction(css::presentation::ClickAction_NONE)
        , nOldVerb(0)
        , nNewVerb(0)
        , nOldPresOrder(0)
        , nNewPresOrder(0)
        , bInfoCreated(bCreated)
    {
    }

        void SetActive(bool bTheOldActive, bool bTheNewActive)
            { bOldActive = bTheOldActive; bNewActive = bTheNewActive; }
        void SetEffect(css::presentation::AnimationEffect eTheOldEffect, css::presentation::AnimationEffect eTheNewEffect)
            { eOldEffect = eTheOldEffect; eNewEffect = eTheNewEffect; }
        void SetTextEffect(css::presentation::AnimationEffect eTheOldEffect, css::presentation::AnimationEffect eTheNewEffect)
            { eOldTextEffect = eTheOldEffect; eNewTextEffect = eTheNewEffect; }
        void SetSpeed(css::presentation::AnimationSpeed eTheOldSpeed, css::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSpeed = eTheOldSpeed; eNewSpeed = eTheNewSpeed; }
        void SetDim(bool bTheOldDim, bool bTheNewDim)
            { bOldDimPrevious = bTheOldDim; bNewDimPrevious = bTheNewDim; }
        void SetDimColor(Color aTheOldDimColor, Color aTheNewDimColor)
            { aOldDimColor = aTheOldDimColor; aNewDimColor = aTheNewDimColor; }
        void SetDimHide(bool bTheOldDimHide, bool bTheNewDimHide)
            { bOldDimHide = bTheOldDimHide; bNewDimHide = bTheNewDimHide; }
        void SetSoundOn(bool bTheOldSoundOn, bool bTheNewSoundOn)
            { bOldSoundOn = bTheOldSoundOn; bNewSoundOn = bTheNewSoundOn; }
        void SetSound(const OUString& aTheOldSound, const OUString& aTheNewSound)
            { aOldSoundFile = aTheOldSound; aNewSoundFile = aTheNewSound; }
        void SetPlayFull(bool bTheOldPlayFull, bool bTheNewPlayFull)
            { bOldPlayFull = bTheOldPlayFull; bNewPlayFull = bTheNewPlayFull; }
        void SetPathObj(SdrPathObj* pTheOldPath, SdrPathObj* pTheNewPath)
            { pOldPathObj = pTheOldPath; pNewPathObj = pTheNewPath; }
        void SetClickAction(css::presentation::ClickAction eTheOldAction, css::presentation::ClickAction eTheNewAction)
            { eOldClickAction = eTheOldAction; eNewClickAction = eTheNewAction; }
        void SetBookmark(const OUString& aTheOldBookmark, const OUString& aTheNewBookmark)
            { aOldBookmark = aTheOldBookmark; aNewBookmark = aTheNewBookmark; }
        void SetVerb(sal_uInt16 nTheOldVerb, sal_uInt16 nTheNewVerb)
            { nOldVerb = nTheOldVerb; nNewVerb = nTheNewVerb; }
        void SetSecondEffect(css::presentation::AnimationEffect eTheOldEffect, css::presentation::AnimationEffect eTheNewEffect)
            { eOldSecondEffect = eTheOldEffect; eNewSecondEffect = eTheNewEffect; }
        void SetSecondSpeed(css::presentation::AnimationSpeed eTheOldSpeed, css::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSecondSpeed = eTheOldSpeed; eNewSecondSpeed = eTheNewSpeed; }
        void SetSecondSoundOn(bool bTheOldSoundOn, bool bTheNewSoundOn)
            { bOldSecondSoundOn = bTheOldSoundOn; bNewSecondSoundOn = bTheNewSoundOn; }
        void SetSecondPlayFull(bool bTheOldPlayFull, bool bTheNewPlayFull)
            { bOldSecondPlayFull = bTheOldPlayFull; bNewSecondPlayFull = bTheNewPlayFull; }

    virtual ~SdAnimationPrmsUndoAction();
    virtual void Undo() override;
    virtual void Redo() override;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_UNOAPRMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
