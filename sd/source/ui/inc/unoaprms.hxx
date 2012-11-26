/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool            bOldInvisibleInPres;
    bool            bNewInvisibleInPres;
    sal_uInt16          nOldVerb;
    sal_uInt16          nNewVerb;
    sal_uLong           nOldPresOrder;
    sal_uLong           nNewPresOrder;

    bool            bInfoCreated;

public:
        SdAnimationPrmsUndoAction(SdDrawDocument* pTheDoc, SdrObject* pObj,
                                  bool bCreated):
        SdUndoAction    (pTheDoc),
        pObject         (pObj),
        bInfoCreated    (bCreated)
        {}

        void SetActive(bool bTheOldActive, bool bTheNewActive)
            { bOldActive = bTheOldActive; bNewActive = bTheNewActive; }
        void SetEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldEffect = eTheOldEffect; eNewEffect = eTheNewEffect; }
        void SetTextEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldTextEffect = eTheOldEffect; eNewTextEffect = eTheNewEffect; }
        void SetSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSpeed = eTheOldSpeed; eNewSpeed = eTheNewSpeed; }
        void SetDim(bool bTheOldDim, bool bTheNewDim)
            { bOldDimPrevious = bTheOldDim; bNewDimPrevious = bTheNewDim; }
        void SetDimColor(Color aTheOldDimColor, Color aTheNewDimColor)
            { aOldDimColor = aTheOldDimColor; aNewDimColor = aTheNewDimColor; }
        void SetDimHide(bool bTheOldDimHide, bool bTheNewDimHide)
            { bOldDimHide = bTheOldDimHide; bNewDimHide = bTheNewDimHide; }
        void SetSoundOn(bool bTheOldSoundOn, bool bTheNewSoundOn)
            { bOldSoundOn = bTheOldSoundOn; bNewSoundOn = bTheNewSoundOn; }
        void SetSound(String aTheOldSound, String aTheNewSound)
            { aOldSoundFile = aTheOldSound; aNewSoundFile = aTheNewSound; }
        void SetBlueScreen(Color aTheOldBlueScreen, Color aTheNewBlueScreen)
            { aOldBlueScreen = aTheOldBlueScreen; aNewBlueScreen = aTheNewBlueScreen; }
        void SetPlayFull(bool bTheOldPlayFull, bool bTheNewPlayFull)
            { bOldPlayFull = bTheOldPlayFull; bNewPlayFull = bTheNewPlayFull; }
        void SetPathObj(SdrPathObj* pTheOldPath, SdrPathObj* pTheNewPath)
            { pOldPathObj = pTheOldPath; pNewPathObj = pTheNewPath; }
        void SetClickAction(::com::sun::star::presentation::ClickAction eTheOldAction, ::com::sun::star::presentation::ClickAction eTheNewAction)
            { eOldClickAction = eTheOldAction; eNewClickAction = eTheNewAction; }
        void SetBookmark(String aTheOldBookmark, String aTheNewBookmark)
            { aOldBookmark = aTheOldBookmark; aNewBookmark = aTheNewBookmark; }
        void SetInvisibleInPres(bool bTheOldInvisibleInPres, bool bTheNewInvisibleInPres)
            { bOldInvisibleInPres = bTheOldInvisibleInPres; bNewInvisibleInPres = bTheNewInvisibleInPres; }
        void SetVerb(sal_uInt16 nTheOldVerb, sal_uInt16 nTheNewVerb)
            { nOldVerb = nTheOldVerb; nNewVerb = nTheNewVerb; }
        void SetSecondEffect(::com::sun::star::presentation::AnimationEffect eTheOldEffect, ::com::sun::star::presentation::AnimationEffect eTheNewEffect)
            { eOldSecondEffect = eTheOldEffect; eNewSecondEffect = eTheNewEffect; }
        void SetSecondSpeed(::com::sun::star::presentation::AnimationSpeed eTheOldSpeed, ::com::sun::star::presentation::AnimationSpeed eTheNewSpeed)
            { eOldSecondSpeed = eTheOldSpeed; eNewSecondSpeed = eTheNewSpeed; }
        void SetSecondSoundOn(bool bTheOldSoundOn, bool bTheNewSoundOn)
            { bOldSecondSoundOn = bTheOldSoundOn; bNewSecondSoundOn = bTheNewSoundOn; }
        void SetSecondPlayFull(bool bTheOldPlayFull, bool bTheNewPlayFull)
            { bOldSecondPlayFull = bTheOldPlayFull; bNewSecondPlayFull = bTheNewPlayFull; }
        void SetPresOrder(sal_uLong nTheOldPresOrder, sal_uLong nTheNewPresOrder)
            { nOldPresOrder = nTheOldPresOrder; nNewPresOrder = nTheNewPresOrder; }

    virtual ~SdAnimationPrmsUndoAction();
    virtual void Undo();
    virtual void Redo();
};

#endif      // _SD_UNOAPRMS_HXX

