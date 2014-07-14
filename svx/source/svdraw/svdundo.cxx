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
#include "precompiled_svx.hxx"

#include <svx/svdundo.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/scene3d.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <svx/svdocapt.hxx>
#include <svl/whiter.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>

// #124389#
#include <svx/svdotable.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrUndoAction::CanRepeat(SfxRepeatTarget& rView) const
{
    SdrView* pV = dynamic_cast< SdrView* >(&rView);

    if(pV)
    {
        return CanSdrRepeat(*pV);
    }

    return false;
}

void SdrUndoAction::Repeat(SfxRepeatTarget& rView)
{
    SdrView* pV = dynamic_cast< SdrView* >(&rView);

    if(pV)
    {
        SdrRepeat(*pV);
    }

    DBG_ASSERT(pV, "Repeat: Uebergebenes SfxRepeatTarget ist keine SdrView");
}

XubString SdrUndoAction::GetRepeatComment(SfxRepeatTarget& rView) const
{
    SdrView* pV = dynamic_cast< SdrView* >(&rView);

    if(pV)
    {
        return GetSdrRepeatComment(*pV);
    }

    return String();
}

bool SdrUndoAction::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

void SdrUndoAction::SdrRepeat(SdrView& /*rView*/)
{
}

XubString SdrUndoAction::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    return String();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoGroup::SdrUndoGroup(SdrModel& rNewMod)
:   SdrUndoAction(rNewMod),
    maList(),
    meFunction(SDRREPFUNC_OBJ_NONE)
{
}

SdrUndoGroup::SdrUndoGroup(SdrModel& rNewMod,const String& rStr)
:   SdrUndoAction(rNewMod),
    maList(),
    maComment(rStr),
    meFunction(SDRREPFUNC_OBJ_NONE)
{
}

SdrUndoGroup::~SdrUndoGroup()
{
    Clear();
}

SdrUndoAction* SdrUndoGroup::GetAction(sal_uInt32 nNum) const
{
    if(nNum < maList.size())
    {
        return *(maList.begin() + nNum);
    }
    else
    {
        OSL_ENSURE(false, "SdrUndoGroup::GetAction access out of range (!)");
        return 0;
    }
}

void SdrUndoGroup::Clear()
{
    for(SdrUndoActionContainerType::iterator aCandidate(maList.begin());
        aCandidate != maList.end(); aCandidate++)
    {
        delete *aCandidate;
    }

    maList.clear();
}

void SdrUndoGroup::AddAction(SdrUndoAction* pAct)
{
    maList.push_back(pAct);
}

void SdrUndoGroup::push_front( SdrUndoAction* pAct )
{
    maList.insert(maList.begin(), pAct);
}

void SdrUndoGroup::Undo()
{
    for(SdrUndoActionContainerType::reverse_iterator aCandidate(maList.rbegin());
        aCandidate != maList.rend(); aCandidate++)
    {
        (*aCandidate)->Undo();
    }
}

void SdrUndoGroup::Redo()
{
    for(SdrUndoActionContainerType::iterator aCandidate(maList.begin());
        aCandidate != maList.end(); aCandidate++)
    {
        (*aCandidate)->Redo();
    }
}

XubString SdrUndoGroup::GetComment() const
{
    XubString aRet(maComment);
    const sal_Char aSearchText[] = "%1";
    const String aSearchString(aSearchText, sizeof(aSearchText - 1));

    aRet.SearchAndReplace(aSearchString, maObjDescription);

    return aRet;
}

bool SdrUndoGroup::CanSdrRepeat(SdrView& rView) const
{
    switch(meFunction)
    {
        case SDRREPFUNC_OBJ_NONE            :  return false;
        case SDRREPFUNC_OBJ_DELETE          :  return rView.areSdrObjectsSelected();
        case SDRREPFUNC_OBJ_COMBINE_POLYPOLY:  return rView.IsCombinePossible(false);
        case SDRREPFUNC_OBJ_COMBINE_ONEPOLY :  return rView.IsCombinePossible(true);
        case SDRREPFUNC_OBJ_DISMANTLE_POLYS :  return rView.IsDismantlePossible(false);
        case SDRREPFUNC_OBJ_DISMANTLE_LINES :  return rView.IsDismantlePossible(true);
        case SDRREPFUNC_OBJ_CONVERTTOPOLY   :  return rView.IsConvertToPolyObjPossible(false);
        case SDRREPFUNC_OBJ_CONVERTTOPATH   :  return rView.IsConvertToPathObjPossible(false);
        case SDRREPFUNC_OBJ_GROUP           :  return rView.IsGroupPossible();
        case SDRREPFUNC_OBJ_UNGROUP         :  return rView.IsUnGroupPossible();
        case SDRREPFUNC_OBJ_PUTTOTOP        :  return rView.IsToTopPossible();
        case SDRREPFUNC_OBJ_PUTTOBTM        :  return rView.IsToBtmPossible();
        case SDRREPFUNC_OBJ_MOVTOTOP        :  return rView.IsToTopPossible();
        case SDRREPFUNC_OBJ_MOVTOBTM        :  return rView.IsToBtmPossible();
        case SDRREPFUNC_OBJ_REVORDER        :  return rView.IsReverseOrderPossible();
        case SDRREPFUNC_OBJ_IMPORTMTF       :  return rView.IsImportMtfPossible();
        default: break;
    }

    return false;
}

void SdrUndoGroup::SdrRepeat(SdrView& rView)
{
    switch(meFunction)
    {
        case SDRREPFUNC_OBJ_NONE            :  break;
        case SDRREPFUNC_OBJ_DELETE          :  rView.DeleteMarked();                break;
        case SDRREPFUNC_OBJ_COMBINE_POLYPOLY:  rView.CombineMarkedObjects(false);   break;
        case SDRREPFUNC_OBJ_COMBINE_ONEPOLY :  rView.CombineMarkedObjects(true);    break;
        case SDRREPFUNC_OBJ_DISMANTLE_POLYS :  rView.DismantleMarkedObjects(false); break;
        case SDRREPFUNC_OBJ_DISMANTLE_LINES :  rView.DismantleMarkedObjects(true);  break;
        case SDRREPFUNC_OBJ_CONVERTTOPOLY   :  rView.ConvertMarkedToPolyObj(false); break;
        case SDRREPFUNC_OBJ_CONVERTTOPATH   :  rView.ConvertMarkedToPathObj(false); break;
        case SDRREPFUNC_OBJ_GROUP           :  rView.GroupMarked();                 break;
        case SDRREPFUNC_OBJ_UNGROUP         :  rView.UnGroupMarked();               break;
        case SDRREPFUNC_OBJ_PUTTOTOP        :  rView.PutMarkedToTop();              break;
        case SDRREPFUNC_OBJ_PUTTOBTM        :  rView.PutMarkedToBtm();              break;
        case SDRREPFUNC_OBJ_MOVTOTOP        :  rView.MovMarkedToTop();              break;
        case SDRREPFUNC_OBJ_MOVTOBTM        :  rView.MovMarkedToBtm();              break;
        case SDRREPFUNC_OBJ_REVORDER        :  rView.ReverseOrderOfMarked();        break;
        case SDRREPFUNC_OBJ_IMPORTMTF       :  rView.DoImportMarkedMtf();           break;
        default: break;
    }
}

XubString SdrUndoGroup::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aRet(maComment);
    const sal_Char aSearchText[] = "%1";
    const String aSearchString(aSearchText, sizeof(aSearchText - 1));

    aRet.SearchAndReplace(aSearchString, ImpGetResStr(STR_ObjNameSingulPlural));

    return aRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObj::SdrUndoObj(SdrObject& rNewObj)
:   SdrUndoAction(rNewObj.getSdrModelFromSdrObject()),
    mrSdrObject(rNewObj)
{
}

void SdrUndoObj::GetDescriptionStringForObject(const SdrObject& _rForObject, sal_uInt16 nStrCacheID, String& rStr, bool bRepeat)
{
    rStr = ImpGetResStr(nStrCacheID);
    const sal_Char aSearchText[] = "%1";
    const String aSearchString(aSearchText, sizeof(aSearchText - 1));
    const xub_StrLen nPos(rStr.Search(aSearchString));

    if(STRING_NOTFOUND != nPos)
    {
        rStr.Erase(nPos, 2);

        if(bRepeat)
        {
            rStr.Insert(ImpGetResStr(STR_ObjNameSingulPlural), nPos);
        }
        else
        {
            XubString aStr;

            _rForObject.TakeObjNameSingul(aStr);
            rStr.Insert(aStr, nPos);
        }
    }
}

void SdrUndoObj::TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, XubString& rStr, bool bRepeat) const
{
    GetDescriptionStringForObject( mrSdrObject, nStrCacheID, rStr, bRepeat );
}

// #94278# common call method for evtl. page change when UNDO/REDO is triggered
void SdrUndoObj::ImpShowPageOfThisObject()
{
    if(mrSdrObject.IsObjectInserted()
        && mrSdrObject.getSdrPageFromSdrObject())
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(mrSdrObject, HINT_SWITCHTOPAGE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoAttrObj::ensureStyleSheetInStyleSheetPool(SfxStyleSheetBasePool& rStyleSheetPool, SfxStyleSheet& rSheet)
{
    SfxStyleSheetBase* pThere = rStyleSheetPool.Find(rSheet.GetName(), rSheet.GetFamily());

    if(!pThere)
    {
        // re-insert remembered style which was removed in the meantime. To do this
        // without assertion, do it without parent and set parent after insertion
        const UniString aParent(rSheet.GetParent());

        rSheet.SetParent(UniString());
        rStyleSheetPool.Insert(&rSheet);
        rSheet.SetParent(aParent);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoAttrObj::SdrUndoAttrObj(SdrObject& rNewObj, bool bStyleSheet1, bool bSaveText)
:   SdrUndoObj(rNewObj),
    mpUndoItemSet(0),
    mpRedoItemSet(0),
    mpRepeatItemSet(0),
    mxUndoStyleSheet(),
    mxRedoStyleSheet(),
    mpTextUndoOPO(0),
    mpTextRedoOPO(0),
    mpUndoGroup(0),
    mbStyleSheet(bStyleSheet1),
    mbHaveToTakeRedoSet(true)
{
    SdrObjList* pOL = rNewObj.getChildrenOfSdrObject();
    const bool bIsGroup(pOL && pOL->GetObjCount());
    const bool bIs3DScene(bIsGroup && dynamic_cast< E3dScene* >(&mrSdrObject));

    if(bIsGroup)
    {
        mpUndoGroup = new SdrUndoGroup(mrModel);
        sal_uInt32 nObjAnz(pOL->GetObjCount());

        for(sal_uInt32 nObjNum(0); nObjNum < nObjAnz; nObjNum++)
        {
            mpUndoGroup->AddAction(new SdrUndoAttrObj(*pOL->GetObj(nObjNum), bStyleSheet1));
        }
    }

    if(!bIsGroup || bIs3DScene)
    {
        if(mpUndoItemSet)
        {
            delete mpUndoItemSet;
        }

        mpUndoItemSet = new SfxItemSet(mrSdrObject.GetMergedItemSet());

        if(mbStyleSheet)
        {
            mxUndoStyleSheet = mrSdrObject.GetStyleSheet();
        }

        if(bSaveText)
        {
            mpTextUndoOPO = mrSdrObject.GetOutlinerParaObject();

            if(mpTextUndoOPO)
            {
                mpTextUndoOPO = new OutlinerParaObject(*mpTextUndoOPO);
            }
        }
    }
}

SdrUndoAttrObj::~SdrUndoAttrObj()
{
    if(mpUndoItemSet)
    {
        delete mpUndoItemSet;
    }

    if(mpRedoItemSet)
    {
        delete mpRedoItemSet;
    }

    if(mpRepeatItemSet)
    {
        delete mpRepeatItemSet;
    }

    if(mpUndoGroup)
    {
        delete mpUndoGroup;
    }

    if(mpTextUndoOPO)
    {
        delete mpTextUndoOPO;
    }

    if(mpTextRedoOPO)
    {
        delete mpTextRedoOPO;
    }
}

void SdrUndoAttrObj::SetRepeatAttr(const SfxItemSet& rSet)
{
    if(mpRepeatItemSet)
    {
        delete mpRepeatItemSet;
    }

    mpRepeatItemSet = new SfxItemSet(rSet);
}

void SdrUndoAttrObj::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrSdrObject);
    const bool bIs3DScene(dynamic_cast< E3dScene* >(&mrSdrObject));

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(!mpUndoGroup || bIs3DScene)
    {
        if(mbHaveToTakeRedoSet)
        {
            mbHaveToTakeRedoSet = false;

            if(mpRedoItemSet)
            {
                delete mpRedoItemSet;
            }

            mpRedoItemSet = new SfxItemSet(mrSdrObject.GetMergedItemSet());

            if(mbStyleSheet)
            {
                mxRedoStyleSheet = mrSdrObject.GetStyleSheet();
            }

            if(mpTextUndoOPO)
            {
                // #i8508#
                mpTextRedoOPO = mrSdrObject.GetOutlinerParaObject();

                if(mpTextRedoOPO)
                {
                    mpTextRedoOPO = new OutlinerParaObject(*mpTextRedoOPO);
                }
            }
        }

        if(mbStyleSheet)
        {
            mxRedoStyleSheet = mrSdrObject.GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxUndoStyleSheet.get());

            if(pSheet && mrSdrObject.getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*mrSdrObject.getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                mrSdrObject.SetStyleSheet(pSheet, true);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(mrSdrObject);

        // #105122# Since ClearItem sets back everything to normal
        // it also sets fit-to-size text to non-fit-to-size text and
        // switches on autogrowheight (the default). That may lead to
        // losing the geometry size info for the object when it is
        // re-layouted from AdjustTextFrameWidthAndHeight(). This makes
        // rescuing the size of the object necessary.
        const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(mrSdrObject));

        if(mpUndoItemSet)
        {
            // #109587#
            if(dynamic_cast< SdrCaptionObj* >(&mrSdrObject))
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*mpUndoItemSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_SET != mpUndoItemSet->GetItemState(nWhich, false))
                    {
                        mrSdrObject.ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                mrSdrObject.ClearMergedItem();
            }

            mrSdrObject.SetMergedItemSet(*mpUndoItemSet);
        }

        // #105122# Restore prev size here when it was changed.
        if(aSnapRange != sdr::legacy::GetSnapRange(mrSdrObject))
        {
            sdr::legacy::SetSnapRange(mrSdrObject, aSnapRange);
        }

        if(mpTextUndoOPO)
        {
            mrSdrObject.SetOutlinerParaObject(new OutlinerParaObject(*mpTextUndoOPO));
        }
    }

    if(mpUndoGroup)
    {
        mpUndoGroup->Undo();
    }
}

void SdrUndoAttrObj::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrSdrObject);
    const bool bIs3DScene(dynamic_cast< E3dScene* >(&mrSdrObject));

    if(!mpUndoGroup || bIs3DScene)
    {
        if(mbStyleSheet)
        {
            mxUndoStyleSheet = mrSdrObject.GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxRedoStyleSheet.get());

            if(pSheet && mrSdrObject.getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*mrSdrObject.getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                mrSdrObject.SetStyleSheet(pSheet, true);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(mrSdrObject);

        // #105122#
        const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(mrSdrObject));

        if(mpRedoItemSet)
        {
            // #109587#
            if(dynamic_cast< SdrCaptionObj* >(&mrSdrObject))
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*mpRedoItemSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_SET != mpRedoItemSet->GetItemState(nWhich, false))
                    {
                        mrSdrObject.ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                mrSdrObject.ClearMergedItem();
            }

            mrSdrObject.SetMergedItemSet(*mpRedoItemSet);
        }

        // #105122# Restore prev size here when it was changed.
        if(aSnapRange != sdr::legacy::GetSnapRange(mrSdrObject))
        {
            sdr::legacy::SetSnapRange(mrSdrObject, aSnapRange);
        }

        // #i8508#
        if(mpTextRedoOPO)
        {
            mrSdrObject.SetOutlinerParaObject(new OutlinerParaObject(*mpTextRedoOPO));
        }
    }

    if(mpUndoGroup)
    {
        mpUndoGroup->Redo();
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

XubString SdrUndoAttrObj::GetComment() const
{
    XubString aStr;

    if(mbStyleSheet)
    {
        TakeMarkedDescriptionString(STR_EditSetStylesheet, aStr);
    }
    else
    {
        TakeMarkedDescriptionString(STR_EditSetAttributes, aStr);
    }

    return aStr;
}

void SdrUndoAttrObj::SdrRepeat(SdrView& rView)
{
    if(mpRepeatItemSet)
    {
        rView.SetAttrToMarked(*mpRepeatItemSet, false);
    }
}

bool SdrUndoAttrObj::CanSdrRepeat(SdrView& rView) const
{
    return (mpRepeatItemSet && rView.areSdrObjectsSelected());
}

XubString SdrUndoAttrObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aStr;

    if(mbStyleSheet)
    {
        TakeMarkedDescriptionString(STR_EditSetStylesheet, aStr, true);
    }
    else
    {
        TakeMarkedDescriptionString(STR_EditSetAttributes, aStr, true);
    }

    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoGeoObj::SdrUndoGeoObj(SdrObject& rNewObj):
    SdrUndoObj(rNewObj),
    mpUndoSdrObjGeoData(0),
    mpRedoSdrObjGeoData(0),
    mpUndoGroup(0)
{
    SdrObjList* pOL = rNewObj.getChildrenOfSdrObject();

    if(pOL && pOL->GetObjCount() && !dynamic_cast< E3dScene* >(&rNewObj))
    {
        // Aha, Gruppenobjekt, aber keine 3D-Szene, dann nur fuer die Szene selbst den Undo anlegen
        mpUndoGroup = new SdrUndoGroup(mrModel);
        const sal_uInt32 nObjAnz(pOL->GetObjCount());

        for(sal_uInt32 nObjNum(0); nObjNum < nObjAnz; nObjNum++)
        {
            mpUndoGroup->AddAction(new SdrUndoGeoObj(*pOL->GetObj(nObjNum)));
        }
    }
    else
    {
        mpUndoSdrObjGeoData = mrSdrObject.GetGeoData();
    }
}

SdrUndoGeoObj::~SdrUndoGeoObj()
{
    if(mpUndoSdrObjGeoData)
    {
        delete mpUndoSdrObjGeoData;
    }

    if(mpRedoSdrObjGeoData)
    {
        delete mpRedoSdrObjGeoData;
    }

    if(mpUndoGroup)
    {
        delete mpUndoGroup;
    }
}

void SdrUndoGeoObj::Undo()
{
    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(mpUndoGroup)
    {
        mpUndoGroup->Undo();

        // #97172#
        // only repaint, no objectchange
        mrSdrObject.ActionChanged();
    }
    else
    {
        if(mpRedoSdrObjGeoData)
        {
            delete mpRedoSdrObjGeoData;
        }

        mpRedoSdrObjGeoData = mrSdrObject.GetGeoData();
        mrSdrObject.SetGeoData(*mpUndoSdrObjGeoData);
    }
}

void SdrUndoGeoObj::Redo()
{
    if(mpUndoGroup)
    {
        mpUndoGroup->Redo();

        // #97172#
        // only repaint, no objectchange
        mrSdrObject.ActionChanged();
    }
    else
    {
        if(mpUndoSdrObjGeoData)
        {
            delete mpUndoSdrObjGeoData;
        }

        mpUndoSdrObjGeoData = mrSdrObject.GetGeoData();
        mrSdrObject.SetGeoData(*mpRedoSdrObjGeoData);
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

XubString SdrUndoGeoObj::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_DragMethObjOwn,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObjList::SdrUndoObjList(SdrObject& rNewObj)
:   SdrUndoObj(rNewObj),
    mpObjList(0),
    mnOrdNum(0),
    mbOwner(false)
{
    mpObjList = mrSdrObject.getParentOfSdrObject();
    mnOrdNum = mrSdrObject.GetNavigationPosition();
}

SdrUndoObjList::~SdrUndoObjList()
{
    if(IsOwner())
    {
        // Attribute muessen wieder in den regulaeren Pool
        SetOwner(false);

        // nun loeschen
        SdrObject* pPointer(&mrSdrObject);
        deleteSdrObjectSafeAndClearPointer( pPointer );
    }
}

void SdrUndoObjList::SetOwner(bool bNew)
{
    mbOwner = bNew;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoRemoveObj::Undo()
{
    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(!mrSdrObject.IsObjectInserted(),"UndoRemoveObj: mrSdrObject ist bereits Inserted");

    if(!mrSdrObject.IsObjectInserted())
    {
        // #i11426#
        // For UNDOs in Calc/Writer it is necessary to adapt the anchor
        // pos of the target object.
        basegfx::B2DPoint aOwnerAnchorPos(0.0, 0.0);

        if(mpObjList)
        {
            SdrObject* pOwner = mpObjList->getSdrObjectFromSdrObjList();

            if(pOwner)
            {
                aOwnerAnchorPos = pOwner->GetAnchorPos();
        }

            E3DModifySceneSnapRectUpdater aUpdater(pOwner);
            mpObjList->InsertObjectToSdrObjList(mrSdrObject, mnOrdNum);
        }

        // #i11426#
        if(!aOwnerAnchorPos.equalZero())
        {
            mrSdrObject.SetAnchorPos(aOwnerAnchorPos);
        }
    }
}

void SdrUndoRemoveObj::Redo()
{
    DBG_ASSERT(mrSdrObject.IsObjectInserted(),"RedoRemoveObj: mrSdrObject ist nicht Inserted");

    if (mrSdrObject.IsObjectInserted())
    {
        E3DModifySceneSnapRectUpdater aUpdater(&mrSdrObject);
        mpObjList->RemoveObjectFromSdrObjList(mnOrdNum);
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoInsertObj::Undo()
{
    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(mrSdrObject.IsObjectInserted(),"UndoInsertObj: mrSdrObject ist nicht Inserted");

    if (mrSdrObject.IsObjectInserted())
    {
#ifdef DBG_UTIL
        SdrObject* pChkObj=
#endif
        mpObjList->RemoveObjectFromSdrObjList(mnOrdNum);

        DBG_ASSERT(pChkObj == &mrSdrObject,"UndoInsertObj: RemoveObjNum!=mrSdrObject");
    }
}

void SdrUndoInsertObj::Redo()
{
    DBG_ASSERT(!mrSdrObject.IsObjectInserted(),"RedoInsertObj: mrSdrObject ist bereits Inserted");

    if (!mrSdrObject.IsObjectInserted())
    {
        // OD 2005-05-10 #i45952# - restore anchor position of an object,
        // which becomes a member of a group, because its cleared in method
        // <InsertObjectToSdrObjList(..)>. Needed for correct ReDo in Writer.
        basegfx::B2DPoint aAnchorPos( 0.0, 0.0 );

        if( mpObjList )
        {
            if(mpObjList->getSdrObjectFromSdrObjList())
            {
                aAnchorPos = mrSdrObject.GetAnchorPos();
            }

            mpObjList->InsertObjectToSdrObjList(mrSdrObject, mnOrdNum);
        }

        if ( !aAnchorPos.equalZero() )
        {
            mrSdrObject.SetAnchorPos( aAnchorPos );
        }
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoDelObj::TryToFlushGraphicContent()
{
    SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(&mrSdrObject);

    if(pSdrGrafObj)
    {
        sdr::contact::ViewContactOfGraphic* pVC = dynamic_cast< sdr::contact::ViewContactOfGraphic* >(&pSdrGrafObj->GetViewContact());

        if(pVC)
        {
            pVC->flushViewObjectContacts();
            pVC->flushGraphicObjects();
        }

        pSdrGrafObj->ForceSwapOut();
    }
}

SdrUndoDelObj::SdrUndoDelObj(SdrObject& rNewObj)
:   SdrUndoRemoveObj(rNewObj)
{
    SetOwner(true);

    // #122985# if graphic object is deleted (but goes to undo) flush it's graphic content
    // since it is potentially no longer needed
    TryToFlushGraphicContent();
}

SdrUndoDelObj::~SdrUndoDelObj()
{
}

void SdrUndoDelObj::Undo()
{
    SdrUndoRemoveObj::Undo();
    DBG_ASSERT(IsOwner(),"UndoDeleteObj: mrSdrObject gehoert nicht der UndoAction");
    SetOwner(false);
}

void SdrUndoDelObj::Redo()
{
    SdrUndoRemoveObj::Redo();
    DBG_ASSERT(!IsOwner(),"RedoDeleteObj: mrSdrObject gehoert bereits der UndoAction");
    SetOwner(true);

    // #122985# if graphic object is deleted (but goes to undo) flush it's graphic content
    // since it is potentially no longer needed
    TryToFlushGraphicContent();
}

XubString SdrUndoDelObj::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_EditDelete,aStr);
    return aStr;
}

void SdrUndoDelObj::SdrRepeat(SdrView& rView)
{
    rView.DeleteMarked();
}

bool SdrUndoDelObj::CanSdrRepeat(SdrView& rView) const
{
    return rView.areSdrObjectsSelected();
}

XubString SdrUndoDelObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_EditDelete,aStr,true);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewObj::Undo()
{
    SdrUndoInsertObj::Undo();
    DBG_ASSERT(!IsOwner(),"RedoNewObj: mrSdrObject gehoert bereits der UndoAction");
    SetOwner(true);
}

void SdrUndoNewObj::Redo()
{
    SdrUndoInsertObj::Redo();
    DBG_ASSERT(IsOwner(),"RedoNewObj: mrSdrObject gehoert nicht der UndoAction");
    SetOwner(false);
}

String SdrUndoNewObj::GetComment( const SdrObject& _rForObject )
{
    String sComment;
    GetDescriptionStringForObject( _rForObject, STR_UndoInsertObj, sComment );
    return sComment;
}

XubString SdrUndoNewObj::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoInsertObj,aStr);
    return aStr;
}

SdrUndoReplaceObj::SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1)
:   SdrUndoObj(rOldObj1),
    mpObjList(0),
    mnOrdNum(0),
    mrNewSdrObject(rNewObj1),
    mbOldOwner(false),
    mbNewOwner(false)
{
    SetOldOwner(true);

    mpObjList = mrSdrObject.getParentOfSdrObject();
    mnOrdNum = mrSdrObject.GetNavigationPosition();
}

SdrUndoReplaceObj::~SdrUndoReplaceObj()
{
    if (IsOldOwner())
    {
        // Attribute muessen wieder in den regulaeren Pool
        SetOldOwner(false);

        // nun loeschen
        SdrObject* pPointer(&mrSdrObject);
        deleteSdrObjectSafeAndClearPointer( pPointer );
    }

    if (IsNewOwner())
    {
        // Attribute muessen wieder in den regulaeren Pool
        SetNewOwner(false);

        // nun loeschen
        SdrObject* pPointer(&mrNewSdrObject);
        deleteSdrObjectSafeAndClearPointer( pPointer );
    }
}

void SdrUndoReplaceObj::Undo()
{
    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if (IsOldOwner() && !IsNewOwner())
    {
        DBG_ASSERT(!mrSdrObject.IsObjectInserted(),"SdrUndoReplaceObj::Undo(): Altes Objekt ist bereits inserted!");
        DBG_ASSERT(mrNewSdrObject.IsObjectInserted(),"SdrUndoReplaceObj::Undo(): Neues Objekt ist nicht inserted!");

        SetOldOwner(false);
        SetNewOwner(true);

        mpObjList->ReplaceObjectInSdrObjList(mrSdrObject, mnOrdNum);
    }
    else
    {
        DBG_ERROR("SdrUndoReplaceObj::Undo(): IsMine-Flags stehen verkehrt. Doppelter Undo-Aufruf?");
    }
}

void SdrUndoReplaceObj::Redo()
{
    if (!IsOldOwner() && IsNewOwner())
    {
        DBG_ASSERT(!mrNewSdrObject.IsObjectInserted(),"SdrUndoReplaceObj::Redo(): Neues Objekt ist bereits inserted!");
        DBG_ASSERT(mrSdrObject.IsObjectInserted(),"SdrUndoReplaceObj::Redo(): Altes Objekt ist nicht inserted!");

        SetOldOwner(true);
        SetNewOwner(false);

        mpObjList->ReplaceObjectInSdrObjList(mrNewSdrObject,mnOrdNum);
    }
    else
    {
        DBG_ERROR("SdrUndoReplaceObj::Redo(): IsMine-Flags stehen verkehrt. Doppelter Redo-Aufruf?");
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

void SdrUndoReplaceObj::SetNewOwner(bool bNew)
{
    mbNewOwner = bNew;
}

void SdrUndoReplaceObj::SetOldOwner(bool bNew)
{
    mbOldOwner = bNew;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XubString SdrUndoCopyObj::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoCopyObj,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i11702#

SdrUndoObjectLayerChange::SdrUndoObjectLayerChange(SdrObject& rObj, SdrLayerID aOldLayer, SdrLayerID aNewLayer)
:   SdrUndoObj(rObj),
    maOldLayer(aOldLayer),
    maNewLayer(aNewLayer)
{
}

void SdrUndoObjectLayerChange::Undo()
{
    ImpShowPageOfThisObject();
    mrSdrObject.SetLayer(maOldLayer);
}

void SdrUndoObjectLayerChange::Redo()
{
    mrSdrObject.SetLayer(maNewLayer);
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObjOrdNum::SdrUndoObjOrdNum(SdrObject& rNewObj, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
:   SdrUndoObj(rNewObj),
    mnOldOrdNum(nOldOrdNum1),
    mnNewOrdNum(nNewOrdNum1)
{
}

void SdrUndoObjOrdNum::Undo()
{
    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    SdrObjList* pOL = mrSdrObject.getParentOfSdrObject();

    if(!pOL)
    {
        DBG_ERROR("UndoObjOrdNum: mrSdrObject hat keine ObjList");
        return;
    }

    pOL->SetNavigationPosition(mnNewOrdNum, mnOldOrdNum);
}

void SdrUndoObjOrdNum::Redo()
{
    SdrObjList* pOL = mrSdrObject.getParentOfSdrObject();

    if(!pOL)
    {
        DBG_ERROR("RedoObjOrdNum: mrSdrObject hat keine ObjList");
        return;
    }

    pOL->SetNavigationPosition(mnOldOrdNum, mnNewOrdNum);

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

XubString SdrUndoObjOrdNum::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoObjOrdNum,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObjSetText::SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText)
:   SdrUndoObj(rNewObj),
    mpOldText(0),
    mpNewText(0),
    mnText(nText),
    mbNewTextAvailable(false),
    mbEmptyPresObj(false)
{
    SdrText* pText = static_cast< SdrTextObj*>( &rNewObj )->getText(mnText);

    if( pText && pText->GetOutlinerParaObject() )
    {
        mpOldText = new OutlinerParaObject(*pText->GetOutlinerParaObject());
    }

    mbEmptyPresObj = rNewObj.IsEmptyPresObj();
}

SdrUndoObjSetText::~SdrUndoObjSetText()
{
    if ( mpOldText )
    {
        delete mpOldText;
    }

    if ( mpNewText )
    {
        delete mpNewText;
    }
}

void SdrUndoObjSetText::AfterSetText()
{
    if (!mbNewTextAvailable)
    {
        SdrText* pText = static_cast< SdrTextObj*>( &mrSdrObject )->getText(mnText);

        if( pText && pText->GetOutlinerParaObject() )
        {
            mpNewText = new OutlinerParaObject(*pText->GetOutlinerParaObject());
        }

        mbNewTextAvailable = true;
    }
}

void SdrUndoObjSetText::Undo()
{
    // only works with SdrTextObj
    SdrTextObj* pTarget = dynamic_cast< SdrTextObj* >(&mrSdrObject);

    if(!pTarget)
    {
        OSL_ENSURE(false, "SdrUndoObjSetText::Undo with SdrObject not based on SdrTextObj (!)");
        return;
    }

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();

    // alten Text sichern fuer Redo
    if(!mbNewTextAvailable)
    {
        AfterSetText();
    }

    // Text fuer Undo kopieren, denn SetOutlinerParaObject() ist Eigentumsuebereignung
    OutlinerParaObject* pText1 = mpOldText;

    if(pText1)
    {
        pText1 = new OutlinerParaObject(*pText1);
    }

    SdrText* pText = pTarget->getText(mnText);

    if(pText)
    {
        pTarget->SetOutlinerParaObjectForText(pText1, pText);
    }

    pTarget->SetEmptyPresObj(mbEmptyPresObj);
    pTarget->ActionChanged();

    // #124389# if it's a table, als oneed to relayout TextFrame
    if(0 != dynamic_cast< sdr::table::SdrTableObj* >(pTarget))
    {
        pTarget->AdjustTextFrameWidthAndHeight();
    }

    // #122410# SetOutlinerParaObject at SdrText does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(mrSdrObject);
}

void SdrUndoObjSetText::Redo()
{
    // only works with SdrTextObj
    SdrTextObj* pTarget = dynamic_cast< SdrTextObj* >(&mrSdrObject);

    if(!pTarget)
    {
        OSL_ENSURE(false, "SdrUndoObjSetText::Redo with SdrObject not based on SdrTextObj (!)");
        return;
    }

    // Text fuer Undo kopieren, denn SetOutlinerParaObject() ist Eigentumsuebereignung
    OutlinerParaObject* pText1 = mpNewText;

    if(pText1)
    {
        pText1 = new OutlinerParaObject(*pText1);
    }

    SdrText* pText = pTarget->getText(mnText);

    if(pText)
    {
        pTarget->SetOutlinerParaObjectForText(pText1, pText);
    }

    pTarget->ActionChanged();

    // #124389# if it's a table, als oneed to relayout TextFrame
    if(0 != dynamic_cast< sdr::table::SdrTableObj* >(pTarget))
    {
        pTarget->AdjustTextFrameWidthAndHeight();
    }

    // #122410# SetOutlinerParaObjectForText at SdrTextObj does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(mrSdrObject);

    // #94278# Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

XubString SdrUndoObjSetText::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoObjSetText,aStr);
    return aStr;
}

XubString SdrUndoObjSetText::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoObjSetText,aStr);
    return aStr;
}

void SdrUndoObjSetText::SdrRepeat(SdrView& rView)
{
    if (mbNewTextAvailable && rView.areSdrObjectsSelected())
    {
        const bool bUndo(rView.IsUndoEnabled());

        if( bUndo )
        {
            XubString aStr;
            TakeMarkedDescriptionString(STR_UndoObjSetText,aStr);
            rView.BegUndo(aStr);
        }

        if(rView.areSdrObjectsSelected())
        {
            const SdrObjectVector aSelection(rView.getSelectedSdrObjectVectorFromSdrMarkView());

            for (sal_uInt32 nm(0); nm < aSelection.size(); nm++)
            {
                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(aSelection[nm]);

                if (pTextObj)
                {
                    if( bUndo )
                    {
                        rView.AddUndo(new SdrUndoObjSetText(*pTextObj,0));
                    }

                    OutlinerParaObject* pText1 = mpNewText;

                    if (pText1)
                    {
                        pText1 = new OutlinerParaObject(*pText1);
                    }

                    pTextObj->SetOutlinerParaObject(pText1);
                }
            }
        }

        if( bUndo )
        {
            rView.EndUndo();
        }
    }
}

bool SdrUndoObjSetText::CanSdrRepeat(SdrView& rView) const
{
    bool bOk(false);

    if(mbNewTextAvailable && rView.areSdrObjectsSelected())
    {
        bOk = true;
    }

    return bOk;
}

SdrUndoObjStrAttr::SdrUndoObjStrAttr(SdrObject& rNewObj, const ObjStrAttrType eObjStrAttr, const String& sOldStr, const String& sNewStr)
    : SdrUndoObj( rNewObj ),
      meObjStrAttr( eObjStrAttr ),
      msOldStr( sOldStr ),
      msNewStr( sNewStr )
{
}

void SdrUndoObjStrAttr::Undo()
{
    ImpShowPageOfThisObject();

    switch ( meObjStrAttr )
    {
        case OBJ_NAME:
        {
            mrSdrObject.SetName( msOldStr );
            break;
        }
        case OBJ_TITLE:
        {
            mrSdrObject.SetTitle( msOldStr );
            break;
        }
        case OBJ_DESCRIPTION:
        {
            mrSdrObject.SetDescription( msOldStr );
            break;
        }
    }
}

void SdrUndoObjStrAttr::Redo()
{
    switch ( meObjStrAttr )
    {
        case OBJ_NAME:
        {
            mrSdrObject.SetName( msNewStr );
            break;
        }
        case OBJ_TITLE:
        {
            mrSdrObject.SetTitle( msNewStr );
            break;
        }
        case OBJ_DESCRIPTION:
        {
            mrSdrObject.SetDescription( msNewStr );
            break;
        }
    }

    ImpShowPageOfThisObject();
}

String SdrUndoObjStrAttr::GetComment() const
{
    String aStr;

    switch ( meObjStrAttr )
    {
        case OBJ_NAME:
        {
            TakeMarkedDescriptionString( STR_UndoObjName, aStr );
            aStr += sal_Unicode(' ');
            aStr += sal_Unicode('\'');
            aStr += msNewStr;
            aStr += sal_Unicode('\'');
            break;
        }
        case OBJ_TITLE:
        {
            TakeMarkedDescriptionString( STR_UndoObjTitle, aStr );
            break;
        }
        case OBJ_DESCRIPTION:
        {
            TakeMarkedDescriptionString( STR_UndoObjDescription, aStr );
            break;
        }
    }

    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoLayer::SdrUndoLayer(sal_uInt32 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
:   SdrUndoAction(rNewModel),
    mpLayer(rNewLayerAdmin.GetLayer(nLayerNum)),
    mrLayerAdmin(rNewLayerAdmin),
    mnLayerNum(nLayerNum),
    mbItsMine(false)
{
}

SdrUndoLayer::~SdrUndoLayer()
{
    if (mbItsMine)
    {
        delete mpLayer;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewLayer::Undo()
{
    DBG_ASSERT(!mbItsMine,"SdrUndoNewLayer::Undo(): Layer gehoert bereits der UndoAction");

    mbItsMine = true;

#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    mrLayerAdmin.RemoveLayer(mnLayerNum);

    DBG_ASSERT(pCmpLayer == mpLayer,"SdrUndoNewLayer::Undo(): Removter Layer ist != mpLayer");
}

void SdrUndoNewLayer::Redo()
{
    DBG_ASSERT(mbItsMine,"SdrUndoNewLayer::Undo(): Layer gehoert nicht der UndoAction");
    mbItsMine = false;
    mrLayerAdmin.InsertLayerFromUndoRedo(mpLayer, mnLayerNum);
}

XubString SdrUndoNewLayer::GetComment() const
{
    return ImpGetResStr(STR_UndoNewLayer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoDelLayer::Undo()
{
    DBG_ASSERT(mbItsMine,"SdrUndoDelLayer::Undo(): Layer gehoert nicht der UndoAction");
    mbItsMine = false;
    mrLayerAdmin.InsertLayerFromUndoRedo(mpLayer, mnLayerNum);
}

void SdrUndoDelLayer::Redo()
{
    DBG_ASSERT(!mbItsMine,"SdrUndoDelLayer::Undo(): Layer gehoert bereits der UndoAction");

    mbItsMine = true;

#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    mrLayerAdmin.RemoveLayer(mnLayerNum);

    DBG_ASSERT(pCmpLayer==mpLayer,"SdrUndoDelLayer::Redo(): Removter Layer ist != mpLayer");
}

XubString SdrUndoDelLayer::GetComment() const
{
    return ImpGetResStr(STR_UndoDelLayer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPage::SdrUndoPage(SdrPage& rNewPg)
:   SdrUndoAction(rNewPg.getSdrModelFromSdrPage()),
    mrPage(rNewPg)
{
}

void SdrUndoPage::ImpInsertPage(sal_uInt32 nNum)
{
    DBG_ASSERT(!mrPage.IsInserted(),"SdrUndoPage::ImpInsertPage(): mrPage ist bereits Inserted");

    if(!mrPage.IsInserted())
    {
        if(mrPage.IsMasterPage())
        {
            mrModel.InsertMasterPage(&mrPage, nNum);
        }
        else
        {
            mrModel.InsertPage(&mrPage, nNum);
        }
    }
}

void SdrUndoPage::ImpRemovePage(sal_uInt32 nNum)
{
    DBG_ASSERT(mrPage.IsInserted(),"SdrUndoPage::ImpRemovePage(): mrPage ist nicht Inserted");

    if (mrPage.IsInserted())
    {
        SdrPage* pChkPg = 0;

        if(mrPage.IsMasterPage())
        {
            pChkPg = mrModel.RemoveMasterPage(nNum);
        }
        else
        {
            pChkPg = mrModel.RemovePage(nNum);
        }

        DBG_ASSERT(pChkPg==&mrPage,"SdrUndoPage::ImpRemovePage(): RemovePage!=&mrPage");
    }
}

void SdrUndoPage::ImpMovePage(sal_uInt32 nOldNum, sal_uInt32 nNewNum)
{
    DBG_ASSERT(mrPage.IsInserted(),"SdrUndoPage::ImpMovePage(): mrPage ist nicht Inserted");

    if (mrPage.IsInserted())
    {
        if (mrPage.IsMasterPage())
        {
            mrModel.MoveMasterPage(nOldNum,nNewNum);
        }
        else
        {
            mrModel.MovePage(nOldNum,nNewNum);
        }
    }
}

void SdrUndoPage::TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 /*n*/, bool /*bRepeat*/) const
{
    rStr=ImpGetResStr(nStrCacheID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageList::SdrUndoPageList(SdrPage& rNewPg)
:   SdrUndoPage(rNewPg),
    mnPageNum(0),
    mbItsMine(false)
{
    mnPageNum = rNewPg.GetPageNumber();
}

SdrUndoPageList::~SdrUndoPageList()
{
    if(mbItsMine)
    {
        delete (&mrPage);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoDelPage::SdrUndoDelPage(SdrPage& rNewPg):
    SdrUndoPageList(rNewPg),
    mpUndoGroup(0)
{
    mbItsMine = true;

    // Und nun ggf. die MasterPage-Beziehungen merken
    if(mrPage.IsMasterPage())
    {
        sal_uInt32 nPageAnz(mrModel.GetPageCount());

        for(sal_uInt32 nPageNum2(0); nPageNum2 < nPageAnz; nPageNum2++)
        {
            SdrPage* pDrawPage = mrModel.GetPage(nPageNum2);

            if(pDrawPage->TRG_HasMasterPage())
            {
                SdrPage& rMasterPage = pDrawPage->TRG_GetMasterPage();

                if(&mrPage == &rMasterPage)
                {
                    if(!mpUndoGroup)
                    {
                        mpUndoGroup = new SdrUndoGroup(mrModel);
                    }

                    mpUndoGroup->AddAction(mrModel.GetSdrUndoFactory().CreateUndoPageRemoveMasterPage(*pDrawPage));
                }
            }
        }
    }
}

SdrUndoDelPage::~SdrUndoDelPage()
{
    if (mpUndoGroup)
    {
        delete mpUndoGroup;
    }
}

void SdrUndoDelPage::Undo()
{
    ImpInsertPage(mnPageNum);

    if (mpUndoGroup)
    {
        // MasterPage-Beziehungen wiederherstellen
        mpUndoGroup->Undo();
    }

    DBG_ASSERT(mbItsMine,"UndoDeletePage: mrPage gehoert nicht der UndoAction");
    mbItsMine = false;
}

void SdrUndoDelPage::Redo()
{
    ImpRemovePage(mnPageNum);

    // Die MasterPage-Beziehungen werden ggf. von selbst geloesst
    DBG_ASSERT(!mbItsMine,"RedoDeletePage: mrPage gehoert bereits der UndoAction");

    mbItsMine = true;
}

XubString SdrUndoDelPage::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoDelPage,aStr,0,false);
    return aStr;
}

XubString SdrUndoDelPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoDelPage,aStr,0,false);
    return aStr;
}

void SdrUndoDelPage::SdrRepeat(SdrView& /*rView*/)
{
}

bool SdrUndoDelPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewPage::Undo()
{
    ImpRemovePage(mnPageNum);
    DBG_ASSERT(!mbItsMine,"UndoNewPage: mrPage gehoert bereits der UndoAction");
    mbItsMine=true;
}

void SdrUndoNewPage::Redo()
{
    ImpInsertPage(mnPageNum);
    DBG_ASSERT(mbItsMine,"RedoNewPage: mrPage gehoert nicht der UndoAction");
    mbItsMine=false;
}

XubString SdrUndoNewPage::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoNewPage,aStr,0,false);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XubString SdrUndoCopyPage::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoCopPage,aStr,0,false);
    return aStr;
}

XubString SdrUndoCopyPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoCopPage,aStr,0,false);
    return aStr;
}

void SdrUndoCopyPage::SdrRepeat(SdrView& /*rView*/)
{

}

bool SdrUndoCopyPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoSetPageNum::Undo()
{
    ImpMovePage(mnNewPageNum, mnOldPageNum);
}

void SdrUndoSetPageNum::Redo()
{
    ImpMovePage(mnOldPageNum, mnNewPageNum);
}

XubString SdrUndoSetPageNum::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoMovPage,aStr,0,false);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageMasterPage::SdrUndoPageMasterPage(SdrPage& rChangedPage)
:   SdrUndoPage(rChangedPage),
    maOldSet(),
    maOldMasterPageNumber(0),
    mbOldHadMasterPage(mrPage.TRG_HasMasterPage())
{
    if(mbOldHadMasterPage)
    {
        maOldSet = mrPage.TRG_GetMasterPageVisibleLayers();
        maOldMasterPageNumber = mrPage.TRG_GetMasterPage().GetPageNumber();
    }
}

SdrUndoPageMasterPage::~SdrUndoPageMasterPage()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageRemoveMasterPage::SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage)
{
}

void SdrUndoPageRemoveMasterPage::Undo()
{
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_SetMasterPage(*mrModel.GetMasterPage(maOldMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageRemoveMasterPage::Redo()
{
    mrPage.TRG_ClearMasterPage();
}

XubString SdrUndoPageRemoveMasterPage::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoDelPageMasterDscr,aStr,0,false);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageChangeMasterPage::SdrUndoPageChangeMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage),
    maNewSet(),
    maNewMasterPageNumber(0),
    mbNewHadMasterPage(false)
{
}

void SdrUndoPageChangeMasterPage::Undo()
{
    // remember values from new page
    if(mrPage.TRG_HasMasterPage())
    {
        mbNewHadMasterPage = true;
        maNewSet = mrPage.TRG_GetMasterPageVisibleLayers();
        maNewMasterPageNumber = mrPage.TRG_GetMasterPage().GetPageNumber();
    }

    // restore old values
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrModel.GetMasterPage(maOldMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageChangeMasterPage::Redo()
{
    // restore new values
    if(mbNewHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrModel.GetMasterPage(maNewMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maNewSet);
    }
}

XubString SdrUndoPageChangeMasterPage::GetComment() const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_UndoChgPageMasterDscr,aStr,0,false);
    return aStr;
}

///////////////////////////////////////////////////////////////////////

SdrUndoFactory::~SdrUndoFactory()
{
}

SdrUndoAction* SdrUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return new SdrUndoGeoObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return new SdrUndoAttrObj( rObject, bStyleSheet1 ? true : false, bSaveText ? true : false );
}

SdrUndoAction* SdrUndoFactory::CreateUndoRemoveObject( SdrObject& rObject )
{
    return new SdrUndoRemoveObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoInsertObject( SdrObject& rObject )
{
    return new SdrUndoInsertObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoDeleteObject( SdrObject& rObject )
{
    return new SdrUndoDelObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoNewObject( SdrObject& rObject )
{
    return new SdrUndoNewObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoCopyObject( SdrObject& rObject )
{
    return new SdrUndoCopyObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return new SdrUndoObjOrdNum( rObject, nOldOrdNum1, nNewOrdNum1 );
}

SdrUndoAction* SdrUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject )
{
    return new SdrUndoReplaceObj( rOldObject, rNewObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return new SdrUndoObjectLayerChange( rObject, aOldLayer, aNewLayer );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return new SdrUndoObjSetText( rNewObj, nText );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectStrAttr( SdrObject& rObject, SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType, String sOldStr, String sNewStr )
{
    return new SdrUndoObjStrAttr( rObject, eObjStrAttrType, sOldStr, sNewStr );
}

// layer
SdrUndoAction* SdrUndoFactory::CreateUndoNewLayer(sal_uInt32 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return new SdrUndoNewLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* SdrUndoFactory::CreateUndoDeleteLayer(sal_uInt32 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return new SdrUndoDelLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

// page
SdrUndoAction*  SdrUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return new SdrUndoDelPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return new SdrUndoNewPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return new SdrUndoCopyPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt32 nOldPageNum1, sal_uInt32 nNewPageNum1)
{
    return new SdrUndoSetPageNum( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
SdrUndoAction* SdrUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return new SdrUndoPageRemoveMasterPage( rChangedPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return new SdrUndoPageChangeMasterPage(rChangedPage);
}

//////////////////////////////////////////////////////////////////////////////
// eof
