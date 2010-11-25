/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_UNOAPRMS_HXX
#define _SD_UNOAPRMS_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "sdundo.hxx"
#include "anmdef.hxx"
#include <svx/svdopath.hxx>
#include <tools/color.hxx>


class SdDrawDocument;
class SdrObject;

class SdAnimationPrmsUndoAction : public SdUndoAction
{
    SdrObject*      pObject;
    BOOL            bOldActive;
    BOOL            bNewActive;
    BOOL            bOldDimPrevious;
    BOOL            bNewDimPrevious;
    BOOL            bOldDimHide;
    BOOL            bNewDimHide;
    BOOL            bOldSoundOn;
    BOOL            bNewSoundOn;
    BOOL            bOldSecondSoundOn;
    BOOL            bNewSecondSoundOn;
    BOOL            bOldPlayFull;
    BOOL            bNewPlayFull;
    BOOL            bOldSecondPlayFull;
    BOOL            bNewSecondPlayFull;
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
    String          aOldSoundFile;
    String          aNewSoundFile;
    SdrPathObj*     pOldPathObj;
    SdrPathObj*     pNewPathObj;
    ::com::sun::star::presentation::ClickAction     eOldClickAction;
    ::com::sun::star::presentation::ClickAction     eNewClickAction;
    String          aOldBookmark;
    String          aNewBookmark;
    BOOL            bOldInvisibleInPres;
    BOOL            bNewInvisibleInPres;
    USHORT          nOldVerb;
    USHORT          nNewVerb;
    ULONG           nOldPresOrder;
    ULONG           nNewPresOrder;

    BOOL            bInfoCreated;

public:
    TYPEINFO();
        SdAnimationPrmsUndoAction(SdDrawDocument* pTheDoc, SdrObject* pObj,
                                  BOOL bCreated):
        SdUndoAction    (pTheDoc),
        pObject         (pObj),
        bInfoCreated    (bCreated)
        {}

        void SetActive(BOOL bTheOldActive, BOOL bTheNewActive)
            { bOldActive = bTheOldActive; bNewActive = bTheNewActive; }
        void SetEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldEffect = eTheOldEffect; eNewEffect = eTheNewEffect; }
        void SetTextEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldTextEffect = eTheOldEffect; eNewTextEffect = eTheNewEffect; }
        void SetSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSpeed = eTheOldSpeed; eNewSpeed = eTheNewSpeed; }
        void SetDim(BOOL bTheOldDim, BOOL bTheNewDim)
            { bOldDimPrevious = bTheOldDim; bNewDimPrevious = bTheNewDim; }
        void SetDimColor(Color aTheOldDimColor, Color aTheNewDimColor)
            { aOldDimColor = aTheOldDimColor; aNewDimColor = aTheNewDimColor; }
        void SetDimHide(BOOL bTheOldDimHide, BOOL bTheNewDimHide)
            { bOldDimHide = bTheOldDimHide; bNewDimHide = bTheNewDimHide; }
        void SetSoundOn(BOOL bTheOldSoundOn, BOOL bTheNewSoundOn)
            { bOldSoundOn = bTheOldSoundOn; bNewSoundOn = bTheNewSoundOn; }
        void SetSound(String aTheOldSound, String aTheNewSound)
            { aOldSoundFile = aTheOldSound; aNewSoundFile = aTheNewSound; }
        void SetBlueScreen(Color aTheOldBlueScreen, Color aTheNewBlueScreen)
            { aOldBlueScreen = aTheOldBlueScreen; aNewBlueScreen = aTheNewBlueScreen; }
        void SetPlayFull(BOOL bTheOldPlayFull, BOOL bTheNewPlayFull)
            { bOldPlayFull = bTheOldPlayFull; bNewPlayFull = bTheNewPlayFull; }
        void SetPathObj(SdrPathObj* pTheOldPath, SdrPathObj* pTheNewPath)
            { pOldPathObj = pTheOldPath; pNewPathObj = pTheNewPath; }
        void SetClickAction(::com::sun::star::presentation::ClickAction eTheOldAction, ::com::sun::star::presentation::ClickAction eTheNewAction)
            { eOldClickAction = eTheOldAction; eNewClickAction = eTheNewAction; }
        void SetBookmark(String aTheOldBookmark, String aTheNewBookmark)
            { aOldBookmark = aTheOldBookmark; aNewBookmark = aTheNewBookmark; }
        void SetInvisibleInPres(BOOL bTheOldInvisibleInPres, BOOL bTheNewInvisibleInPres)
            { bOldInvisibleInPres = bTheOldInvisibleInPres; bNewInvisibleInPres = bTheNewInvisibleInPres; }
        void SetVerb(USHORT nTheOldVerb, USHORT nTheNewVerb)
            { nOldVerb = nTheOldVerb; nNewVerb = nTheNewVerb; }
        void SetSecondEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldSecondEffect = eTheOldEffect; eNewSecondEffect = eTheNewEffect; }
        void SetSecondSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSecondSpeed = eTheOldSpeed; eNewSecondSpeed = eTheNewSpeed; }
        void SetSecondSoundOn(BOOL bTheOldSoundOn, BOOL bTheNewSoundOn)
            { bOldSecondSoundOn = bTheOldSoundOn; bNewSecondSoundOn = bTheNewSoundOn; }
        void SetSecondPlayFull(BOOL bTheOldPlayFull, BOOL bTheNewPlayFull)
            { bOldSecondPlayFull = bTheOldPlayFull; bNewSecondPlayFull = bTheNewPlayFull; }
        void SetPresOrder(ULONG nTheOldPresOrder, ULONG nTheNewPresOrder)
            { nOldPresOrder = nTheOldPresOrder; nNewPresOrder = nTheNewPresOrder; }

    virtual ~SdAnimationPrmsUndoAction();
    virtual void Undo();
    virtual void Redo();
};

#endif      // _SD_UNOAPRMS_HXX

