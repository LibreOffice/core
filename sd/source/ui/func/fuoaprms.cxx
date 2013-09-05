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

#include "fuoaprms.hxx"

#include "sdattr.hxx"
#include <svx/svdpagv.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdundo.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <svl/aeitem.hxx>
#include "svx/xtable.hxx"

#include "strings.hrc"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "anminfo.hxx"
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Group
#include "View.hxx"
#include "sdabstdlg.hxx"
#include "sdresid.hxx"
#include <tools/helpers.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuObjectAnimationParameters, FuPoor );

#define ATTR_MISSING    0       ///< Attribute missing
#define ATTR_MIXED      1       ///< Attribute ambiguous (on multi-selection)
#define ATTR_SET        2       ///< Attribute unique


FuObjectAnimationParameters::FuObjectAnimationParameters (
    ViewShell*   pViewSh,
    ::sd::Window*        pWin,
    ::sd::View*      pView,
    SdDrawDocument* pDoc,
    SfxRequest&  rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuObjectAnimationParameters::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuObjectAnimationParameters( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuObjectAnimationParameters::DoExecute( SfxRequest& rReq )
{
    ::svl::IUndoManager* pUndoMgr = mpViewShell->GetViewFrame()->GetObjectShell()->GetUndoManager();

    const SdrMarkList& rMarkList  = mpView->GetMarkedObjectList();
    sal_uLong        nCount     = rMarkList.GetMarkCount();
    sal_uLong        nObject    = 0;

    short nAnimationSet     = ATTR_MISSING;
    short nEffectSet        = ATTR_MISSING;
    short nTextEffectSet    = ATTR_MISSING;
    short nSpeedSet         = ATTR_MISSING;
    short nFadeColorSet     = ATTR_MISSING;
    short nFadeOutSet       = ATTR_MISSING;
    short nInvisibleSet     = ATTR_MISSING;
    short nSoundOnSet       = ATTR_MISSING;
    short nSoundFileSet     = ATTR_MISSING;
    short nPlayFullSet      = ATTR_MISSING;
    short nClickActionSet   = ATTR_MISSING;
    short nBookmarkSet      = ATTR_MISSING;

    short nSecondEffectSet      = ATTR_MISSING;
    short nSecondSpeedSet       = ATTR_MISSING;
    short nSecondSoundOnSet     = ATTR_MISSING;
    short nSecondPlayFullSet    = ATTR_MISSING;

    // defaults (for Undo-Action)
    presentation::AnimationEffect eEffect         = presentation::AnimationEffect_NONE;
    presentation::AnimationEffect eTextEffect     = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed  eSpeed          = presentation::AnimationSpeed_MEDIUM;
    sal_Bool            bActive         = sal_False;
    sal_Bool            bFadeOut        = sal_False;
    Color           aFadeColor      = COL_LIGHTGRAY;
    sal_Bool            bInvisible      = sal_False;
    sal_Bool            bSoundOn        = sal_False;
    OUString        aSound;
    sal_Bool            bPlayFull       = sal_False;
    presentation::ClickAction     eClickAction    = presentation::ClickAction_NONE;
    OUString        aBookmark;

    presentation::AnimationEffect eSecondEffect   = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed  eSecondSpeed    = presentation::AnimationSpeed_MEDIUM;
    sal_Bool            bSecondSoundOn  = sal_False;
    sal_Bool            bSecondPlayFull = sal_False;


    SdAnimationInfo* pInfo;
    SdrMark* pMark;

    // inspect first object
    pMark = rMarkList.GetMark(0);
    pInfo = mpDoc->GetAnimationInfo(pMark->GetMarkedSdrObj());
    if( pInfo )
    {
        bActive             = pInfo->mbActive;
        nAnimationSet       = ATTR_SET;

        eEffect             = pInfo->meEffect;
        nEffectSet          = ATTR_SET;

        eTextEffect         = pInfo->meTextEffect;
        nTextEffectSet      = ATTR_SET;

        eSpeed              = pInfo->meSpeed;
        nSpeedSet           = ATTR_SET;

        bFadeOut            = pInfo->mbDimPrevious;
        nFadeOutSet         = ATTR_SET;

        aFadeColor          = pInfo->maDimColor;
        nFadeColorSet       = ATTR_SET;

        bInvisible          = pInfo->mbDimHide;
        nInvisibleSet       = ATTR_SET;

        bSoundOn            = pInfo->mbSoundOn;
        nSoundOnSet         = ATTR_SET;

        aSound              = pInfo->maSoundFile;
        nSoundFileSet       = ATTR_SET;

        bPlayFull           = pInfo->mbPlayFull;
        nPlayFullSet        = ATTR_SET;

        eClickAction        = pInfo->meClickAction;
        nClickActionSet     = ATTR_SET;

        aBookmark           = pInfo->GetBookmark();
        nBookmarkSet        = ATTR_SET;

        eSecondEffect       = pInfo->meSecondEffect;
        nSecondEffectSet    = ATTR_SET;

        eSecondSpeed        = pInfo->meSecondSpeed;
        nSecondSpeedSet     = ATTR_SET;

        bSecondSoundOn      = pInfo->mbSecondSoundOn;
        nSecondSoundOnSet   = ATTR_SET;

        bSecondPlayFull     = pInfo->mbSecondPlayFull;
        nSecondPlayFullSet  = ATTR_SET;
    }

    // if necessary, inspect more objects
    for( nObject = 1; nObject < nCount; nObject++ )
    {
        pMark = rMarkList.GetMark( nObject );
        SdrObject* pObject = pMark->GetMarkedSdrObj();
        pInfo = mpDoc->GetAnimationInfo(pObject);
        if( pInfo )
        {
            if( bActive != pInfo->mbActive )
                nAnimationSet = ATTR_MIXED;

            if( eEffect != pInfo->meEffect )
                nEffectSet = ATTR_MIXED;

            if( eTextEffect != pInfo->meTextEffect )
                nTextEffectSet = ATTR_MIXED;

            if( eSpeed != pInfo->meSpeed )
                nSpeedSet = ATTR_MIXED;

            if( bFadeOut != pInfo->mbDimPrevious )
                nFadeOutSet = ATTR_MIXED;

            if( aFadeColor != pInfo->maDimColor )
                nFadeColorSet = ATTR_MIXED;

            if( bInvisible != pInfo->mbDimHide )
                nInvisibleSet = ATTR_MIXED;

            if( bSoundOn != pInfo->mbSoundOn )
                nSoundOnSet = ATTR_MIXED;

            if( aSound != pInfo->maSoundFile )
                nSoundFileSet = ATTR_MIXED;

            if( bPlayFull != pInfo->mbPlayFull )
                nPlayFullSet = ATTR_MIXED;

            if( eClickAction != pInfo->meClickAction )
                nClickActionSet = ATTR_MIXED;

            if( aBookmark != pInfo->GetBookmark() )
                nBookmarkSet = ATTR_MIXED;

            if( eSecondEffect != pInfo->meSecondEffect )
                nSecondEffectSet = ATTR_MIXED;

            if( eSecondSpeed != pInfo->meSecondSpeed )
                nSecondSpeedSet = ATTR_MIXED;

            if( bSecondSoundOn != pInfo->mbSecondSoundOn )
                nSecondSoundOnSet = ATTR_MIXED;

            if( bSecondPlayFull != pInfo->mbSecondPlayFull )
                nSecondPlayFullSet = ATTR_MIXED;
        }
        else
        {
            if (nAnimationSet == ATTR_SET && bActive == sal_True)
                nAnimationSet = ATTR_MIXED;

            if (nEffectSet == ATTR_SET && eEffect != presentation::AnimationEffect_NONE)
                nEffectSet = ATTR_MIXED;

            if (nTextEffectSet == ATTR_SET && eTextEffect != presentation::AnimationEffect_NONE)
                nTextEffectSet = ATTR_MIXED;

            if (nSpeedSet == ATTR_SET)
                nSpeedSet = ATTR_MIXED;

            if (nFadeOutSet == ATTR_SET && bFadeOut == sal_True)
                nFadeOutSet = ATTR_MIXED;

            if (nFadeColorSet == ATTR_SET)
                nFadeColorSet = ATTR_MIXED;

            if (nInvisibleSet == ATTR_SET && bInvisible == sal_True)
                nInvisibleSet = ATTR_MIXED;

            if (nSoundOnSet == ATTR_SET && bSoundOn == sal_True)
                nSoundOnSet = ATTR_MIXED;

            if (nSoundFileSet == ATTR_SET)
                nSoundFileSet = ATTR_MIXED;

            if (nPlayFullSet == ATTR_SET && bPlayFull == sal_True)
                nPlayFullSet = ATTR_MIXED;

            if (nClickActionSet == ATTR_SET && eClickAction != presentation::ClickAction_NONE)
                nClickActionSet = ATTR_MIXED;

            if (nBookmarkSet == ATTR_SET)
                nBookmarkSet = ATTR_MIXED;

            if (nSecondEffectSet == ATTR_SET && eSecondEffect != presentation::AnimationEffect_NONE)
                nSecondEffectSet = ATTR_MIXED;

            if (nSecondSpeedSet == ATTR_SET)
                nSecondSpeedSet = ATTR_MIXED;

            if (nSecondSoundOnSet == ATTR_SET && bSecondSoundOn == sal_True)
                nSecondSoundOnSet = ATTR_MIXED;

            if (nSecondPlayFullSet == ATTR_SET && bSecondPlayFull == sal_True)
                nSecondPlayFullSet = ATTR_MIXED;
        }
    }

    /* Exactly two objects with path effect?
       Then, only the animation info at the moved object is valid. */
    if (nCount == 2)
    {
        SdrObject* pObject1 = rMarkList.GetMark(0)->GetMarkedSdrObj();
        SdrObject* pObject2 = rMarkList.GetMark(1)->GetMarkedSdrObj();
        SdrObjKind eKind1   = (SdrObjKind)pObject1->GetObjIdentifier();
        SdrObjKind eKind2   = (SdrObjKind)pObject2->GetObjIdentifier();
        SdAnimationInfo* pInfo1 = mpDoc->GetAnimationInfo(pObject1);
        SdAnimationInfo* pInfo2 = mpDoc->GetAnimationInfo(pObject2);
        pInfo  = NULL;

        if (pObject1->GetObjInventor() == SdrInventor &&
            ((eKind1 == OBJ_LINE) ||                        // 2 point line
             (eKind1 == OBJ_PLIN) ||                        // Polygon
             (eKind1 == OBJ_PATHLINE))                &&    // Bezier curve
             (pInfo2 && pInfo2->meEffect == presentation::AnimationEffect_PATH))
        {
            pInfo = pInfo2;
        }

        if (pObject2->GetObjInventor() == SdrInventor &&
            ((eKind2 == OBJ_LINE) ||                        // 2 point line
             (eKind2 == OBJ_PLIN) ||                        // Polygon
             (eKind2 == OBJ_PATHLINE))                &&    // Bezier curve
            (pInfo1 && pInfo1->meEffect == presentation::AnimationEffect_PATH))
        {
            pInfo = pInfo1;
        }

        if (pInfo)
        {
            bActive         = pInfo->mbActive;          nAnimationSet       = ATTR_SET;
            eEffect         = pInfo->meEffect;          nEffectSet          = ATTR_SET;
            eTextEffect     = pInfo->meTextEffect;      nTextEffectSet      = ATTR_SET;
            eSpeed          = pInfo->meSpeed;           nSpeedSet           = ATTR_SET;
            bFadeOut        = pInfo->mbDimPrevious;     nFadeOutSet         = ATTR_SET;
            aFadeColor      = pInfo->maDimColor;        nFadeColorSet       = ATTR_SET;
            bInvisible      = pInfo->mbDimHide;         nInvisibleSet       = ATTR_SET;
            bSoundOn        = pInfo->mbSoundOn;         nSoundOnSet         = ATTR_SET;
            aSound          = pInfo->maSoundFile;       nSoundFileSet       = ATTR_SET;
            bPlayFull       = pInfo->mbPlayFull;        nPlayFullSet        = ATTR_SET;
            eClickAction    = pInfo->meClickAction;     nClickActionSet     = ATTR_SET;
            aBookmark       = pInfo->GetBookmark();     nBookmarkSet        = ATTR_SET;
            eSecondEffect   = pInfo->meSecondEffect;    nSecondEffectSet    = ATTR_SET;
            eSecondSpeed    = pInfo->meSecondSpeed;     nSecondSpeedSet     = ATTR_SET;
            bSecondSoundOn  = pInfo->mbSecondSoundOn;   nSecondSoundOnSet   = ATTR_SET;
            bSecondPlayFull = pInfo->mbSecondPlayFull;  nSecondPlayFullSet  = ATTR_SET;
        }
    }

    const SfxItemSet* pArgs = rReq.GetArgs();

    if(!pArgs)
    {
        // fill ItemSet for dialog
        SfxItemSet aSet(mpDoc->GetPool(), ATTR_ANIMATION_START, ATTR_ACTION_END);

        // fill the set
        if (nAnimationSet == ATTR_SET)
            aSet.Put( SfxBoolItem( ATTR_ANIMATION_ACTIVE, bActive));
        else if (nAnimationSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_ACTIVE);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_ACTIVE, sal_False));

        if (nEffectSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_EFFECT, (sal_uInt16)eEffect));
        else if (nEffectSet == ATTR_MIXED)
            aSet.InvalidateItem( ATTR_ANIMATION_EFFECT );
        else
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_EFFECT, presentation::AnimationEffect_NONE));

        if (nTextEffectSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_TEXTEFFECT, (sal_uInt16)eTextEffect));
        else if (nTextEffectSet == ATTR_MIXED)
            aSet.InvalidateItem( ATTR_ANIMATION_TEXTEFFECT );
        else
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_TEXTEFFECT, presentation::AnimationEffect_NONE));

        if (nSpeedSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ANIMATION_SPEED, (sal_uInt16)eSpeed));
        else
            aSet.InvalidateItem(ATTR_ANIMATION_SPEED);

        if (nFadeOutSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_FADEOUT, bFadeOut));
        else if (nFadeOutSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_FADEOUT);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_FADEOUT, sal_False));

        if (nFadeColorSet == ATTR_SET)
            aSet.Put(SvxColorItem(aFadeColor, ATTR_ANIMATION_COLOR));
        else if (nFadeColorSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_COLOR);
        else
            aSet.Put(SvxColorItem(RGB_Color(COL_LIGHTGRAY), ATTR_ANIMATION_COLOR));

        if (nInvisibleSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_INVISIBLE, bInvisible));
        else if (nInvisibleSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_INVISIBLE);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_INVISIBLE, sal_False));

        if (nSoundOnSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, bSoundOn));
        else if (nSoundOnSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDON);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, sal_False));

        if (nSoundFileSet == ATTR_SET)
            aSet.Put(SfxStringItem(ATTR_ANIMATION_SOUNDFILE, aSound));
        else
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDFILE);

        if (nPlayFullSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, bPlayFull));
        else if (nPlayFullSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_PLAYFULL);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, sal_False));

        if (nClickActionSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ACTION, (sal_uInt16)eClickAction));
        else if (nClickActionSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ACTION);
        else
            aSet.Put(SfxAllEnumItem(ATTR_ACTION, presentation::ClickAction_NONE));

        if (nBookmarkSet == ATTR_SET)
            aSet.Put(SfxStringItem(ATTR_ACTION_FILENAME, aBookmark));
        else
            aSet.InvalidateItem(ATTR_ACTION_FILENAME);

        if (nSecondEffectSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ACTION_EFFECT, (sal_uInt16)eSecondEffect));
        else if (nSecondEffectSet == ATTR_MIXED)
            aSet.InvalidateItem( ATTR_ACTION_EFFECT );
        else
            aSet.Put(SfxAllEnumItem(ATTR_ACTION_EFFECT, presentation::AnimationEffect_NONE));

        if (nSecondSpeedSet == ATTR_SET)
            aSet.Put(SfxAllEnumItem(ATTR_ACTION_EFFECTSPEED, (sal_uInt16)eSecondSpeed));
        else
            aSet.InvalidateItem(ATTR_ACTION_EFFECTSPEED);

        if (nSecondSoundOnSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ACTION_SOUNDON, bSecondSoundOn));
        else if (nSecondSoundOnSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ACTION_SOUNDON);
        else
            aSet.Put(SfxBoolItem(ATTR_ACTION_SOUNDON, sal_False));

        if (nSecondPlayFullSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, bSecondPlayFull));
        else if (nPlayFullSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ACTION_PLAYFULL);
        else
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, sal_False));

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        SfxAbstractDialog* pDlg = pFact ? pFact->CreatSdActionDialog( NULL, &aSet, mpView ) : 0;

        short nResult = pDlg ? pDlg->Execute() : static_cast<short>(RET_CANCEL);

        if( nResult == RET_OK )
        {
            rReq.Done( *( pDlg->GetOutputItemSet() ) );
            pArgs = rReq.GetArgs();
        }

        delete pDlg;

        if( nResult != RET_OK )
            return;
    }

    // evaluation of the ItemSets
    if (pArgs->GetItemState(ATTR_ANIMATION_ACTIVE) == SFX_ITEM_SET)
    {
        bActive = ((SfxBoolItem&)pArgs->Get(ATTR_ANIMATION_ACTIVE)).GetValue();
        nAnimationSet = ATTR_SET;
    }
    else
        nAnimationSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_EFFECT) == SFX_ITEM_SET)
    {
        eEffect = (presentation::AnimationEffect)((SfxAllEnumItem&) pArgs->
                    Get(ATTR_ANIMATION_EFFECT)).GetValue();
        nEffectSet = ATTR_SET;
    }
    else
        nEffectSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_TEXTEFFECT) == SFX_ITEM_SET)
    {
        eTextEffect = (presentation::AnimationEffect)((SfxAllEnumItem&) pArgs->
                        Get(ATTR_ANIMATION_TEXTEFFECT)).GetValue();
        nTextEffectSet = ATTR_SET;
    }
    else
        nTextEffectSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_SPEED) == SFX_ITEM_SET)
    {
        eSpeed  = (presentation::AnimationSpeed)((SfxAllEnumItem&) pArgs->
                    Get(ATTR_ANIMATION_SPEED)).GetValue();
        nSpeedSet = ATTR_SET;
    }
    else
        nSpeedSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_FADEOUT) == SFX_ITEM_SET)
    {
        bFadeOut = ((SfxBoolItem&)pArgs->Get(ATTR_ANIMATION_FADEOUT)).GetValue();
        nFadeOutSet = ATTR_SET;
    }
    else
        nFadeOutSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_INVISIBLE) == SFX_ITEM_SET)
    {
        bInvisible = ((SfxBoolItem&)pArgs->Get(ATTR_ANIMATION_INVISIBLE)).GetValue();
        nInvisibleSet = ATTR_SET;
    }
    else
        nInvisibleSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_SOUNDON) == SFX_ITEM_SET)
    {
        bSoundOn = ((SfxBoolItem&)pArgs->Get(ATTR_ANIMATION_SOUNDON)).GetValue();
        nSoundOnSet = ATTR_SET;
    }
    else
        nSoundOnSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_SOUNDFILE) == SFX_ITEM_SET)
    {
        aSound =  ((SfxStringItem&)pArgs->Get(ATTR_ANIMATION_SOUNDFILE)).GetValue();
        nSoundFileSet = ATTR_SET;
    }
    else
        nSoundFileSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_COLOR) == SFX_ITEM_SET)
    {
        aFadeColor = ((SvxColorItem&)pArgs->Get(ATTR_ANIMATION_COLOR)).GetValue();
        nFadeColorSet = ATTR_SET;
    }
    else
        nFadeColorSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ANIMATION_PLAYFULL) == SFX_ITEM_SET)
    {
        bPlayFull = ((SfxBoolItem&)pArgs->Get(ATTR_ANIMATION_PLAYFULL)).GetValue();
        nPlayFullSet = ATTR_SET;
    }
    else
        nPlayFullSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION) == SFX_ITEM_SET)
    {
        eClickAction = (presentation::ClickAction)((SfxAllEnumItem&)pArgs->
                    Get(ATTR_ACTION)).GetValue();
        nClickActionSet = ATTR_SET;
    }
    else
        nClickActionSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION_FILENAME) == SFX_ITEM_SET)
    {
        aBookmark = ((SfxStringItem&)pArgs->
                        Get(ATTR_ACTION_FILENAME)).GetValue();
        nBookmarkSet = ATTR_SET;
    }
    else
        nBookmarkSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION_EFFECT) == SFX_ITEM_SET)
    {
        eSecondEffect = (presentation::AnimationEffect)((SfxAllEnumItem&) pArgs->
                    Get(ATTR_ACTION_EFFECT)).GetValue();
        nSecondEffectSet = ATTR_SET;
    }
    else
        nSecondEffectSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION_EFFECTSPEED) == SFX_ITEM_SET)
    {
        eSecondSpeed  = (presentation::AnimationSpeed)((SfxAllEnumItem&) pArgs->
                    Get(ATTR_ACTION_EFFECTSPEED)).GetValue();
        nSecondSpeedSet = ATTR_SET;
    }
    else
        nSecondSpeedSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION_SOUNDON) == SFX_ITEM_SET)
    {
        bSecondSoundOn = ((SfxBoolItem&)pArgs->Get(ATTR_ACTION_SOUNDON)).GetValue();
        nSecondSoundOnSet = ATTR_SET;
    }
    else
        nSecondSoundOnSet = ATTR_MISSING;

    if (pArgs->GetItemState(ATTR_ACTION_PLAYFULL) == SFX_ITEM_SET)
    {
        bSecondPlayFull = ((SfxBoolItem&)pArgs->Get(ATTR_ACTION_PLAYFULL)).GetValue();
        nSecondPlayFullSet = ATTR_SET;
    }
    else
        nSecondPlayFullSet = ATTR_MISSING;

    // if any attribute is chosen
    if (nEffectSet         == ATTR_SET  ||
        nTextEffectSet     == ATTR_SET  ||
        nSpeedSet          == ATTR_SET  ||
        nAnimationSet      == ATTR_SET  ||
        nFadeOutSet        == ATTR_SET  ||
        nFadeColorSet      == ATTR_SET  ||
        nInvisibleSet      == ATTR_SET  ||
        nSoundOnSet        == ATTR_SET  ||
        nSoundFileSet      == ATTR_SET  ||
        nPlayFullSet       == ATTR_SET  ||
        nClickActionSet    == ATTR_SET  ||
        nBookmarkSet       == ATTR_SET  ||
        nSecondEffectSet   == ATTR_SET  ||
        nSecondSpeedSet    == ATTR_SET  ||
        nSecondSoundOnSet  == ATTR_SET  ||
        nSecondPlayFullSet == ATTR_SET)
    {
        // String for undo-group and list-action
        OUString aComment(SdResId(STR_UNDO_ANIMATION));

        // with 'following curves', we have an additional UndoAction
        // therefore cling? here
        pUndoMgr->EnterListAction(aComment, aComment);

        // create undo group
        SdUndoGroup* pUndoGroup = new SdUndoGroup(mpDoc);
        pUndoGroup->SetComment(aComment);


        // for the path effect, remember some stuff
        SdrObject*  pRunningObj = NULL;
        SdrPathObj* pPath       = NULL;
        if (eEffect == presentation::AnimationEffect_PATH && nEffectSet == ATTR_SET)
        {
            DBG_ASSERT(nCount == 2, "This effect expects two selected objects");
            SdrObject* pObject1 = rMarkList.GetMark(0)->GetMarkedSdrObj();
            SdrObject* pObject2 = rMarkList.GetMark(1)->GetMarkedSdrObj();
            SdrObjKind eKind1   = (SdrObjKind)pObject1->GetObjIdentifier();
            SdrObjKind eKind2   = (SdrObjKind)pObject2->GetObjIdentifier();

            if (pObject1->GetObjInventor() == SdrInventor &&
                ((eKind1 == OBJ_LINE) ||        // 2 point line
                 (eKind1 == OBJ_PLIN) ||        // Polygon
                 (eKind1 == OBJ_PATHLINE)))     // Bezier curve
            {
                pPath = (SdrPathObj*)pObject1;
                pRunningObj = pObject2;
            }

            if (pObject2->GetObjInventor() == SdrInventor &&
                ((eKind2 == OBJ_LINE) ||        // 2 point line
                 (eKind2 == OBJ_PLIN) ||        // Polygon
                 (eKind2 == OBJ_PATHLINE)))     // Bezier curve
            {
                pPath = (SdrPathObj*)pObject2;
                pRunningObj = pObject1;
            }

            DBG_ASSERT(pPath, "no curve found");


            // push the running object to the end of the curve
            Rectangle aCurRect(pRunningObj->GetLogicRect());
            Point     aCurCenter(aCurRect.Center());
            const ::basegfx::B2DPolyPolygon& rPolyPolygon = pPath->GetPathPoly();
            sal_uInt32 nNoOfPolygons(rPolyPolygon.count());
            const ::basegfx::B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(nNoOfPolygons - 1L));
            sal_uInt32 nPoints(aPolygon.count());
            const ::basegfx::B2DPoint aNewB2DCenter(aPolygon.getB2DPoint(nPoints - 1L));
            const Point aNewCenter(FRound(aNewB2DCenter.getX()), FRound(aNewB2DCenter.getY()));
            Size aDistance(aNewCenter.X() - aCurCenter.X(), aNewCenter.Y() - aCurCenter.Y());
            pRunningObj->Move(aDistance);

            pUndoMgr->AddUndoAction(mpDoc->GetSdrUndoFactory().CreateUndoMoveObject( *pRunningObj, aDistance));
        }

        for (nObject = 0; nObject < nCount; nObject++)
        {
            SdrObject* pObject = rMarkList.GetMark(nObject)->GetMarkedSdrObj();

            pInfo = mpDoc->GetAnimationInfo(pObject);

            sal_Bool bCreated = sal_False;
            if( !pInfo )
            {
                pInfo = SdDrawDocument::GetShapeUserData(*pObject,true);
                bCreated = sal_True;
            }

            // path object for 'following curves'?
            if (eEffect == presentation::AnimationEffect_PATH && pObject == pPath)
            {
                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction
                                                (mpDoc, pObject, bCreated);
                pAction->SetActive(pInfo->mbActive, pInfo->mbActive);
                pAction->SetEffect(pInfo->meEffect, pInfo->meEffect);
                pAction->SetTextEffect(pInfo->meTextEffect, pInfo->meTextEffect);
                pAction->SetSpeed(pInfo->meSpeed, pInfo->meSpeed);
                pAction->SetDim(pInfo->mbDimPrevious, pInfo->mbDimPrevious);
                pAction->SetDimColor(pInfo->maDimColor, pInfo->maDimColor);
                pAction->SetDimHide(pInfo->mbDimHide, pInfo->mbDimHide);
                pAction->SetSoundOn(pInfo->mbSoundOn, pInfo->mbSoundOn);
                pAction->SetSound(pInfo->maSoundFile, pInfo->maSoundFile);
                pAction->SetPlayFull(pInfo->mbPlayFull, pInfo->mbPlayFull);
                pAction->SetClickAction(pInfo->meClickAction, pInfo->meClickAction);
                pAction->SetBookmark(pInfo->GetBookmark(), pInfo->GetBookmark());
                pAction->SetVerb(pInfo->mnVerb, pInfo->mnVerb);
                pAction->SetSecondEffect(pInfo->meSecondEffect, pInfo->meSecondEffect);
                pAction->SetSecondSpeed(pInfo->meSecondSpeed, pInfo->meSecondSpeed);
                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, pInfo->mbSecondSoundOn);
                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull, pInfo->mbSecondPlayFull);
                pUndoGroup->AddAction(pAction);

            }
            else
            {

                // create undo action with old and new sizes
                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction
                                                (mpDoc, pObject, bCreated);
                pAction->SetActive(pInfo->mbActive, bActive);
                pAction->SetEffect(pInfo->meEffect, eEffect);
                pAction->SetTextEffect(pInfo->meTextEffect, eTextEffect);
                pAction->SetSpeed(pInfo->meSpeed, eSpeed);
                pAction->SetDim(pInfo->mbDimPrevious, bFadeOut);
                pAction->SetDimColor(pInfo->maDimColor, aFadeColor);
                pAction->SetDimHide(pInfo->mbDimHide, bInvisible);
                pAction->SetSoundOn(pInfo->mbSoundOn, bSoundOn);
                pAction->SetSound(pInfo->maSoundFile, aSound);
                pAction->SetPlayFull(pInfo->mbPlayFull, bPlayFull);
                pAction->SetPathObj(pInfo->mpPathObj, pPath);
                pAction->SetClickAction(pInfo->meClickAction, eClickAction);
                pAction->SetBookmark(pInfo->GetBookmark(), aBookmark);
                pAction->SetVerb(pInfo->mnVerb, (sal_uInt16)pInfo->GetBookmark().toInt32() );
                pAction->SetSecondEffect(pInfo->meSecondEffect, eSecondEffect);
                pAction->SetSecondSpeed(pInfo->meSecondSpeed, eSecondSpeed);
                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, bSecondSoundOn);
                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull,bSecondPlayFull);
                pUndoGroup->AddAction(pAction);

                // insert new values at info block of the object
                if (nAnimationSet == ATTR_SET)
                    pInfo->mbActive = bActive;

                if (nEffectSet == ATTR_SET)
                    pInfo->meEffect = eEffect;

                if (nTextEffectSet == ATTR_SET)
                    pInfo->meTextEffect = eTextEffect;

                if (nSpeedSet == ATTR_SET)
                    pInfo->meSpeed = eSpeed;

                if (nFadeOutSet == ATTR_SET)
                    pInfo->mbDimPrevious = bFadeOut;

                if (nFadeColorSet == ATTR_SET)
                    pInfo->maDimColor = aFadeColor;

                if (nInvisibleSet == ATTR_SET)
                    pInfo->mbDimHide = bInvisible;

                if (nSoundOnSet == ATTR_SET)
                    pInfo->mbSoundOn = bSoundOn;

                if (nSoundFileSet == ATTR_SET)
                    pInfo->maSoundFile = aSound;

                if (nPlayFullSet == ATTR_SET)
                    pInfo->mbPlayFull = bPlayFull;

                if (nClickActionSet == ATTR_SET)
                    pInfo->meClickAction = eClickAction;

                if (nBookmarkSet == ATTR_SET)
                    pInfo->SetBookmark( aBookmark );

                if (nSecondEffectSet == ATTR_SET)
                    pInfo->meSecondEffect = eSecondEffect;

                if (nSecondSpeedSet == ATTR_SET)
                    pInfo->meSecondSpeed = eSecondSpeed;

                if (nSecondSoundOnSet == ATTR_SET)
                    pInfo->mbSecondSoundOn = bSecondSoundOn;

                if (nSecondPlayFullSet == ATTR_SET)
                    pInfo->mbSecondPlayFull = bSecondPlayFull;

                if (eClickAction == presentation::ClickAction_VERB)
                    pInfo->mnVerb = (sal_uInt16)aBookmark.toInt32();
            }
        }
        // Set the Undo Group in of the Undo Manager
        pUndoMgr->AddUndoAction(pUndoGroup);
        pUndoMgr->LeaveListAction();

        // Model changed
        mpDoc->SetChanged();
    }
    // not seen, therefore we do not need to invalidate at the bindings
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
