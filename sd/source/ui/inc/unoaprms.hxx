/*************************************************************************
 *
 *  $RCSfile: unoaprms.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:42 $
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

#ifndef _SD_UNOAPRMS_HXX
#define _SD_UNOAPRMS_HXX

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif

#ifndef _SD_SDUNDO_HXX
#include "sdundo.hxx"
#endif
#ifndef _SD_ANMDEF_HXX
#include "anmdef.hxx"
#endif

#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif

#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif


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

        SdAnimationPrmsUndoAction( SdDrawDocument* pTheDoc, SdrObject* pObj );

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
    virtual void Repeat();
};

#endif      // _SD_UNOAPRMS_HXX

