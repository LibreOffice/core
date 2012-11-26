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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/aeitem.hxx>
#include "svx/xtable.hxx"
#include <svx/svdlegacy.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <basegfx/polygon/b2dpolygon.hxx>

#include "strings.hrc"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "anminfo.hxx"
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#include "View.hxx"
#include "sdabstdlg.hxx"
#include "sdresid.hxx"

using namespace ::com::sun::star;

namespace sd {

#define ATTR_MISSING    0       // Attribut nicht verfuegbar
#define ATTR_MIXED      1       // Attribut uneindeutig (bei Mehrfachselektion)
#define ATTR_SET        2       // Attribut eindeutig

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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
    sal_uInt32 nObject  = 0;

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

                                    // defaulten (fuer Undo-Aktion)
    presentation::AnimationEffect eEffect         = presentation::AnimationEffect_NONE;
    presentation::AnimationEffect eTextEffect     = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed  eSpeed          = presentation::AnimationSpeed_MEDIUM;
    bool            bActive         = false;
    bool            bFadeOut        = false;
    Color           aFadeColor      = COL_LIGHTGRAY;
    bool            bInvisible      = false;
    bool            bSoundOn        = false;
    String          aSound;
    bool            bPlayFull       = false;
    presentation::ClickAction     eClickAction    = presentation::ClickAction_NONE;
    String          aBookmark;

    presentation::AnimationEffect eSecondEffect   = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed  eSecondSpeed    = presentation::AnimationSpeed_MEDIUM;
    bool            bSecondSoundOn  = false;
    bool            bSecondPlayFull = false;

    SdAnimationInfo* pInfo(0);

    // das erste Objekt untersuchen
    const SdrObjectVector aSelection(mpView->getSelectedSdrObjectVectorFromSdrMarkView());

    if(aSelection.size())
    {
        pInfo = mpDoc->GetAnimationInfo(aSelection[0]);
    }

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

    // ggfs. weitere Objekte untersuchen
    for( nObject = 1; nObject < aSelection.size(); nObject++ )
    {
        SdrObject* pObject = aSelection[nObject];
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
            if (nAnimationSet == ATTR_SET && bActive == true)
                nAnimationSet = ATTR_MIXED;

            if (nEffectSet == ATTR_SET && eEffect != presentation::AnimationEffect_NONE)
                nEffectSet = ATTR_MIXED;

            if (nTextEffectSet == ATTR_SET && eTextEffect != presentation::AnimationEffect_NONE)
                nTextEffectSet = ATTR_MIXED;

            if (nSpeedSet == ATTR_SET)
                nSpeedSet = ATTR_MIXED;

            if (nFadeOutSet == ATTR_SET && bFadeOut == true)
                nFadeOutSet = ATTR_MIXED;

            if (nFadeColorSet == ATTR_SET)
                nFadeColorSet = ATTR_MIXED;

            if (nInvisibleSet == ATTR_SET && bInvisible == true)
                nInvisibleSet = ATTR_MIXED;

            if (nSoundOnSet == ATTR_SET && bSoundOn == true)
                nSoundOnSet = ATTR_MIXED;

            if (nSoundFileSet == ATTR_SET)
                nSoundFileSet = ATTR_MIXED;

            if (nPlayFullSet == ATTR_SET && bPlayFull == true)
                nPlayFullSet = ATTR_MIXED;

            if (nClickActionSet == ATTR_SET && eClickAction != presentation::ClickAction_NONE)
                nClickActionSet = ATTR_MIXED;

            if (nBookmarkSet == ATTR_SET)
                nBookmarkSet = ATTR_MIXED;

            if (nSecondEffectSet == ATTR_SET && eSecondEffect != presentation::AnimationEffect_NONE)
                nSecondEffectSet = ATTR_MIXED;

            if (nSecondSpeedSet == ATTR_SET)
                nSecondSpeedSet = ATTR_MIXED;

            if (nSecondSoundOnSet == ATTR_SET && bSecondSoundOn == true)
                nSecondSoundOnSet = ATTR_MIXED;

            if (nSecondPlayFullSet == ATTR_SET && bSecondPlayFull == true)
                nSecondPlayFullSet = ATTR_MIXED;
        }
    }

    // Genau zwei Objekte mit Pfadeffekt? Dann gilt nur die Animationsinfo
    // am bewegten Objekt.
    if (2 == aSelection.size())
    {
        SdrObject* pObject1 = aSelection[0];
        SdrObject* pObject2 = aSelection[1];
        SdAnimationInfo* pInfo1 = mpDoc->GetAnimationInfo(pObject1);
        SdAnimationInfo* pInfo2 = mpDoc->GetAnimationInfo(pObject2);
        pInfo  = NULL;

        if(pObject1 && pInfo2 && pInfo2->meEffect == presentation::AnimationEffect_PATH)
        {
            SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObject1);

            if(pSdrPathObj && !pSdrPathObj->isClosed())
            {
                pInfo = pInfo2;
            }
        }

        if(pObject2 && pInfo1 && pInfo1->meEffect == presentation::AnimationEffect_PATH)
        {
            SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObject2);

            if(pSdrPathObj && !pSdrPathObj->isClosed())
            {
                pInfo = pInfo1;
            }
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
        // ItemSet fuer Dialog fuellen
        SfxItemSet aSet(mpDoc->GetItemPool(), ATTR_ANIMATION_START, ATTR_ACTION_END);

        // das Set besetzen
        if (nAnimationSet == ATTR_SET)
            aSet.Put( SfxBoolItem( ATTR_ANIMATION_ACTIVE, bActive));
        else if (nAnimationSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_ACTIVE);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_ACTIVE, false));

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
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_FADEOUT, false));

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
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_INVISIBLE, false));

        if (nSoundOnSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, bSoundOn));
        else if (nSoundOnSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDON);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_SOUNDON, false));

        if (nSoundFileSet == ATTR_SET)
            aSet.Put(SfxStringItem(ATTR_ANIMATION_SOUNDFILE, aSound));
        else
            aSet.InvalidateItem(ATTR_ANIMATION_SOUNDFILE);

        if (nPlayFullSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, bPlayFull));
        else if (nPlayFullSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ANIMATION_PLAYFULL);
        else
            aSet.Put(SfxBoolItem(ATTR_ANIMATION_PLAYFULL, false));

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
            aSet.Put(SfxBoolItem(ATTR_ACTION_SOUNDON, false));

        if (nSecondPlayFullSet == ATTR_SET)
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, bSecondPlayFull));
        else if (nPlayFullSet == ATTR_MIXED)
            aSet.InvalidateItem(ATTR_ACTION_PLAYFULL);
        else
            aSet.Put(SfxBoolItem(ATTR_ACTION_PLAYFULL, false));

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        SfxAbstractDialog* pDlg = pFact ? pFact->CreatSdActionDialog( NULL, &aSet, mpView ) : 0;

        sal_uInt16 nResult = pDlg ? pDlg->Execute() : RET_CANCEL;

        if( nResult == RET_OK )
        {
            rReq.Done( *( pDlg->GetOutputItemSet() ) );
            pArgs = rReq.GetArgs();
        }

        delete pDlg;

        if( nResult != RET_OK )
            return;
    }

    // Auswertung des ItemSets
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

    // Wenn irgendwelche Attribute ausgewaehlt wurden
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
        // String fuer Undo-Group und List-Action
        String aComment(SdResId(STR_UNDO_ANIMATION));

        // bei 'an Kurve entlang' gibt's noch eine extra UndoAction, darum
        // hier klammern
        pUndoMgr->EnterListAction(aComment, aComment);

        // Undo Gruppe erzeugen
        SdUndoGroup* pUndoGroup = new SdUndoGroup(mpDoc);
        pUndoGroup->SetComment(aComment);


        // fuer den Pfad-Effekt einige Dinge merken
        SdrObject*  pRunningObj = NULL;
        SdrPathObj* pPath       = NULL;
        if (eEffect == presentation::AnimationEffect_PATH && nEffectSet == ATTR_SET)
        {
            DBG_ASSERT(2 == aSelection.size(), "dieser Effekt braucht genau 2 selektierte Objekte");
            SdrObject* pObject1 = aSelection[0];
            SdrObject* pObject2 = aSelection[1];
            SdrObjKind eKind1   = (SdrObjKind)pObject1->GetObjIdentifier();
            SdrObjKind eKind2   = (SdrObjKind)pObject2->GetObjIdentifier();

            if(pObject1)
            {
                SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObject1);

                if(pSdrPathObj && !pSdrPathObj->isClosed())
                {
                    pPath = pSdrPathObj;
                    pRunningObj = pObject2;
                }
            }

            if(pObject2)
            {
                SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObject2);

                if(pSdrPathObj && !pSdrPathObj->isClosed())
                {
                    pPath = pSdrPathObj;
                    pRunningObj = pObject1;
                }
            }

            DBG_ASSERT(pPath, "keine Kurve gefunden");

            // das laufende Objekt auf das Kurvenende schieben
            const basegfx::B2DRange aCurRange(sdr::legacy::GetLogicRange(*pRunningObj));
            const basegfx::B2DPolyPolygon aPolyPolygon(pPath->getB2DPolyPolygonInObjectCoordinates());
            const basegfx::B2DPolygon aPolygon(aPolyPolygon.count() ? aPolyPolygon.getB2DPolygon(aPolyPolygon.count() - 1) : basegfx::B2DPolygon());
            const basegfx::B2DPoint aNewB2DCenter(aPolygon.count() ? aPolygon.getB2DPoint(aPolygon.count() - 1) : basegfx::B2DPoint());

            sdr::legacy::transformSdrObject(*pRunningObj, basegfx::tools::createTranslateB2DHomMatrix(aNewB2DCenter - aCurRange.getCenter()));

            pUndoMgr->AddUndoAction(mpDoc->GetSdrUndoFactory().CreateUndoGeoObject( *pRunningObj ));
        }

        for (nObject = 0; nObject < aSelection.size(); nObject++)
        {
            SdrObject* pObject = aSelection[nObject];
            pInfo = mpDoc->GetAnimationInfo(pObject);

            bool bCreated = false;
            if( !pInfo )
            {
                pInfo = SdDrawDocument::GetShapeUserData(*pObject,true);
                bCreated = true;
            }

            // das Pfadobjekt fuer 'an Kurve entlang'?
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
//              pAction->SetPathObj(pInfo->mpPathObj, pInfo->mpPathObj);
                pAction->SetClickAction(pInfo->meClickAction, pInfo->meClickAction);
                pAction->SetBookmark(pInfo->GetBookmark(), pInfo->GetBookmark());
//              pAction->SetInvisibleInPres(pInfo->mbInvisibleInPresentation, true);
                pAction->SetVerb(pInfo->mnVerb, pInfo->mnVerb);
                pAction->SetSecondEffect(pInfo->meSecondEffect, pInfo->meSecondEffect);
                pAction->SetSecondSpeed(pInfo->meSecondSpeed, pInfo->meSecondSpeed);
                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, pInfo->mbSecondSoundOn);
                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull, pInfo->mbSecondPlayFull);
                pUndoGroup->AddAction(pAction);

//              pInfo->mbInvisibleInPresentation = true;
            }
            else
            {

                // Undo-Action mit alten und neuen Groessen erzeugen
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
//              pAction->SetInvisibleInPres(pInfo->mbInvisibleInPresentation,
//                                          pInfo->mbInvisibleInPresentation);
                pAction->SetVerb(pInfo->mnVerb, (sal_uInt16)pInfo->GetBookmark().ToInt32() );
                pAction->SetSecondEffect(pInfo->meSecondEffect, eSecondEffect);
                pAction->SetSecondSpeed(pInfo->meSecondSpeed, eSecondSpeed);
                pAction->SetSecondSoundOn(pInfo->mbSecondSoundOn, bSecondSoundOn);
                pAction->SetSecondPlayFull(pInfo->mbSecondPlayFull,bSecondPlayFull);
                pUndoGroup->AddAction(pAction);

                // neue Werte am Infoblock des Objekts eintragen
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

                // noch ein paar Spezialitaeten
//              if (eEffect == presentation::AnimationEffect_PATH && nEffectSet == ATTR_SET)
//                  pInfo->mSetPath(pPath);

                if (eClickAction == presentation::ClickAction_VERB)
                    pInfo->mnVerb = (sal_uInt16)aBookmark.ToInt32();
            }
        }
        // Undo Gruppe dem Undo Manager uebergeben
        pUndoMgr->AddUndoAction(pUndoGroup);
        pUndoMgr->LeaveListAction();

        // Model geaendert
        mpDoc->SetChanged();
    }
    // sieht man nicht, also muss an den Bindings nicht invalidiert werden
}

} // end of namespace sd
