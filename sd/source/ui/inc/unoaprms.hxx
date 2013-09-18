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

#ifndef _SD_UNOAPRMS_HXX
#define _SD_UNOAPRMS_HXX

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
    sal_Bool            bOldActive;
    sal_Bool            bNewActive;
    sal_Bool            bOldDimPrevious;
    sal_Bool            bNewDimPrevious;
    sal_Bool            bOldDimHide;
    sal_Bool            bNewDimHide;
    sal_Bool            bOldSoundOn;
    sal_Bool            bNewSoundOn;
    sal_Bool            bOldSecondSoundOn;
    sal_Bool            bNewSecondSoundOn;
    sal_Bool            bOldPlayFull;
    sal_Bool            bNewPlayFull;
    sal_Bool            bOldSecondPlayFull;
    sal_Bool            bNewSecondPlayFull;
    ::com::sun::star::presentation::AnimationEffect eOldEffect;
    ::com::sun::star::presentation::AnimationEffect eNewEffect;
    ::com::sun::star::presentation::AnimationEffect eOldTextEffect;
    ::com::sun::star::presentation::AnimationEffect eNewTextEffect;
    ::com::sun::star::presentation::AnimationSpeed  eOldSpeed;
    ::com::sun::star::presentation::AnimationSpeed  eNewSpeed;
    ::com::sun::star::presentation::AnimationEffect eOldSecondEffect;
    ::com::sun::star::presentation::AnimationEffect eNewSecondEffect;
    ::com::sun::star::presentation::AnimationSpeed  eOldSecondSpeed;
    ::com::sun::star::presentation::AnimationSpeed  eNewSecondSpeed;
    Color           aOldDimColor;
    Color           aNewDimColor;
    Color           aOldBlueScreen;
    Color           aNewBlueScreen;
    OUString        aOldSoundFile;
    OUString        aNewSoundFile;
    SdrPathObj*     pOldPathObj;
    SdrPathObj*     pNewPathObj;
    ::com::sun::star::presentation::ClickAction     eOldClickAction;
    ::com::sun::star::presentation::ClickAction     eNewClickAction;
    OUString        aOldBookmark;
    OUString        aNewBookmark;
    sal_Bool            bOldInvisibleInPres;
    sal_Bool            bNewInvisibleInPres;
    sal_uInt16          nOldVerb;
    sal_uInt16          nNewVerb;
    sal_uLong           nOldPresOrder;
    sal_uLong           nNewPresOrder;

    sal_Bool            bInfoCreated;

public:
    TYPEINFO();
        SdAnimationPrmsUndoAction(SdDrawDocument* pTheDoc, SdrObject* pObj,
                                  sal_Bool bCreated):
        SdUndoAction    (pTheDoc),
        pObject         (pObj),
        bInfoCreated    (bCreated)
        {}

        void SetActive(sal_Bool bTheOldActive, sal_Bool bTheNewActive)
            { bOldActive = bTheOldActive; bNewActive = bTheNewActive; }
        void SetEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldEffect = eTheOldEffect; eNewEffect = eTheNewEffect; }
        void SetTextEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldTextEffect = eTheOldEffect; eNewTextEffect = eTheNewEffect; }
        void SetSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSpeed = eTheOldSpeed; eNewSpeed = eTheNewSpeed; }
        void SetDim(sal_Bool bTheOldDim, sal_Bool bTheNewDim)
            { bOldDimPrevious = bTheOldDim; bNewDimPrevious = bTheNewDim; }
        void SetDimColor(Color aTheOldDimColor, Color aTheNewDimColor)
            { aOldDimColor = aTheOldDimColor; aNewDimColor = aTheNewDimColor; }
        void SetDimHide(sal_Bool bTheOldDimHide, sal_Bool bTheNewDimHide)
            { bOldDimHide = bTheOldDimHide; bNewDimHide = bTheNewDimHide; }
        void SetSoundOn(sal_Bool bTheOldSoundOn, sal_Bool bTheNewSoundOn)
            { bOldSoundOn = bTheOldSoundOn; bNewSoundOn = bTheNewSoundOn; }
        void SetSound(String aTheOldSound, OUString aTheNewSound)
            { aOldSoundFile = aTheOldSound; aNewSoundFile = aTheNewSound; }
        void SetBlueScreen(Color aTheOldBlueScreen, Color aTheNewBlueScreen)
            { aOldBlueScreen = aTheOldBlueScreen; aNewBlueScreen = aTheNewBlueScreen; }
        void SetPlayFull(sal_Bool bTheOldPlayFull, sal_Bool bTheNewPlayFull)
            { bOldPlayFull = bTheOldPlayFull; bNewPlayFull = bTheNewPlayFull; }
        void SetPathObj(SdrPathObj* pTheOldPath, SdrPathObj* pTheNewPath)
            { pOldPathObj = pTheOldPath; pNewPathObj = pTheNewPath; }
        void SetClickAction(::com::sun::star::presentation::ClickAction eTheOldAction, ::com::sun::star::presentation::ClickAction eTheNewAction)
            { eOldClickAction = eTheOldAction; eNewClickAction = eTheNewAction; }
        void SetBookmark(String aTheOldBookmark, OUString aTheNewBookmark)
            { aOldBookmark = aTheOldBookmark; aNewBookmark = aTheNewBookmark; }
        void SetInvisibleInPres(sal_Bool bTheOldInvisibleInPres, sal_Bool bTheNewInvisibleInPres)
            { bOldInvisibleInPres = bTheOldInvisibleInPres; bNewInvisibleInPres = bTheNewInvisibleInPres; }
        void SetVerb(sal_uInt16 nTheOldVerb, sal_uInt16 nTheNewVerb)
            { nOldVerb = nTheOldVerb; nNewVerb = nTheNewVerb; }
        void SetSecondEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldSecondEffect = eTheOldEffect; eNewSecondEffect = eTheNewEffect; }
        void SetSecondSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSecondSpeed = eTheOldSpeed; eNewSecondSpeed = eTheNewSpeed; }
        void SetSecondSoundOn(sal_Bool bTheOldSoundOn, sal_Bool bTheNewSoundOn)
            { bOldSecondSoundOn = bTheOldSoundOn; bNewSecondSoundOn = bTheNewSoundOn; }
        void SetSecondPlayFull(sal_Bool bTheOldPlayFull, sal_Bool bTheNewPlayFull)
            { bOldSecondPlayFull = bTheOldPlayFull; bNewSecondPlayFull = bTheNewPlayFull; }
        void SetPresOrder(sal_uLong nTheOldPresOrder, sal_uLong nTheNewPresOrder)
            { nOldPresOrder = nTheOldPresOrder; nNewPresOrder = nTheNewPresOrder; }

    virtual ~SdAnimationPrmsUndoAction();
    virtual void Undo();
    virtual void Redo();
};

#endif      // _SD_UNOAPRMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
