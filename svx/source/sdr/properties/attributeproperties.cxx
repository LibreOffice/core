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

#include <utility>

#include <o3tl/make_unique.hxx>
#include <sdr/properties/attributeproperties.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>
#include <svl/poolitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svddef.hxx>
#include <svx/xit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnasit.hxx>
#include <svx/xflasit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdpage.hxx>

namespace sdr
{
    namespace properties
    {
        void AttributeProperties::ImpSetParentAtSfxItemSet(bool bDontRemoveHardAttr)
        {
            if(HasSfxItemSet() && mpStyleSheet)
            {
                // Delete hard attributes where items are set in the style sheet
                if(!bDontRemoveHardAttr)
                {
                    const SfxItemSet& rStyle = mpStyleSheet->GetItemSet();
                    SfxWhichIter aIter(rStyle);
                    sal_uInt16 nWhich = aIter.FirstWhich();

                    while(nWhich)
                    {
                        if(SfxItemState::SET == rStyle.GetItemState(nWhich))
                        {
                            mpItemSet->ClearItem(nWhich);
                        }

                        nWhich = aIter.NextWhich();
                    }
                }

                // set new stylesheet as parent
                mpItemSet->SetParent(&mpStyleSheet->GetItemSet());
            }
            else
            {
                OSL_ENSURE(false, "ImpSetParentAtSfxItemSet called without SfxItemSet/SfxStyleSheet (!)");
            }
        }

        void AttributeProperties::ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // test if old StyleSheet is cleared, else it would be lost
            // after this method -> memory leak (!)
            DBG_ASSERT(!mpStyleSheet, "Old style sheet not deleted before setting new one (!)");

            if(pNewStyleSheet)
            {
                // local remember
                mpStyleSheet = pNewStyleSheet;

                if(HasSfxItemSet())
                {
                    // register as listener
                    StartListening(*pNewStyleSheet->GetPool());
                    StartListening(*pNewStyleSheet);

                    // only apply the following when we have an SfxItemSet already, else
                    if(GetStyleSheet())
                    {
                        ImpSetParentAtSfxItemSet(bDontRemoveHardAttr);
                    }
                }
            }
        }

        void AttributeProperties::ImpRemoveStyleSheet()
        {
            // Check type since it is destroyed when the type is deleted
            if(GetStyleSheet() && dynamic_cast<const SfxStyleSheet *>(mpStyleSheet) != nullptr)
            {
                EndListening(*mpStyleSheet);
                if (auto const pool = mpStyleSheet->GetPool()) { // TTTT
                    EndListening(*pool);
                }

                // reset parent of ItemSet
                if(HasSfxItemSet())
                {
                    mpItemSet->SetParent(nullptr);
                }

                SdrObject& rObj = GetSdrObject();
                rObj.SetBoundRectDirty();
                rObj.SetRectsDirty(true);
            }

            mpStyleSheet = nullptr;
        }

        // create a new itemset
        std::unique_ptr<SfxItemSet> AttributeProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return o3tl::make_unique<SfxItemSet>(rPool,

                // ranges from SdrAttrObj
                svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION>{});
        }

        AttributeProperties::AttributeProperties(SdrObject& rObj)
        :   DefaultProperties(rObj),
            mpStyleSheet(nullptr)
        {
            // Do nothing else, esp. do *not* try to get and set
            // a default SfxStyle sheet. Nothing is allowed to be done
            // that may lead to calls to virtual functions like
            // CreateObjectSpecificItemSet - these would go *wrong*.
            // Thus the rest is lazy-init from here.
        }

        AttributeProperties::AttributeProperties(const AttributeProperties& rProps, SdrObject& rObj)
        :   DefaultProperties(rProps, rObj),
            mpStyleSheet(nullptr)
        {
            SfxStyleSheet* pTargetStyleSheet(rProps.GetStyleSheet());

            if(pTargetStyleSheet)
            {
                const bool bModelChange(&rObj.getSdrModelFromSdrObject() != &rProps.GetSdrObject().getSdrModelFromSdrObject());

                if(bModelChange)
                {
                    // tdf#117506
                    // The error shows that it is definitely necessary to solve this problem.
                    // Interestingly I already had a note here for 'work needed'.
                    // Checked in libreoffice-6-0 what happened there. In principle, the whole
                    // ::Clone of SdrPage and SdrObject happened in the same SdrModel, only
                    // afterwards a ::SetModel was used at the cloned SdrPage which went through
                    // all layers. The StyleSheet-problem was solved in
                    // AttributeProperties::MoveToItemPool at the end. There, a StyleSheet with the
                    // same name was searched for in the target-SdrModel.
                    // Start by resetting the current TargetStyleSheet so that nothing goes wrong
                    // when we do not find a fitting TargetStyleSheet.
                    // Note: The test for SdrModelChange above was wrong (compared the already set
                    // new SdrObject), so this never triggered and pTargetStyleSheet was never set to
                    // nullptr before. This means that a StyleSheet from another SdrModel was used
                    // what of course is very dangerous. Interestingly did not crash since when that
                    // other SdrModel was destroyed the ::Notify mechanism still worked reliably
                    // and de-connected this Properties successfully from the alien-StyleSheet.
                    pTargetStyleSheet = nullptr;

                    // Check if we have a TargetStyleSheetPool at the target-SdrModel. This *should*
                    // be the case already (SdrModel::Merge and SdDrawDocument::InsertBookmarkAsPage
                    // have already cloned the StyleSheets to the target-SdrModel when used in Draw/impress).
                    // If none is found, ImpGetDefaultStyleSheet will be used to set a 'default'
                    // StyleSheet as StyleSheet implicitly later (that's what happened in the task,
                    // thus the FillStyle changed to the 'default' Blue).
                    // Note: It *may* be necessary to do more for StyleSheets, e.g. clone/copy the
                    // StyleSheet Hierarchy from the source SdrModel and/or add the Items from there
                    // as hard attributes. If needed, have a look at the older AttributeProperties::SetModel
                    // implementation from e.g. libreoffice-6-0.
                    SfxStyleSheetBasePool* pTargetStyleSheetPool(rObj.getSdrModelFromSdrObject().GetStyleSheetPool());

                    if(nullptr != pTargetStyleSheetPool)
                    {
                        // If we have a TargetStyleSheetPool, search for the used StyleSheet
                        // in the target SdrModel using the Name from the original StyleSheet
                        // in the source-SdrModel.
                        pTargetStyleSheet = dynamic_cast< SfxStyleSheet* >(
                            pTargetStyleSheetPool->Find(
                                rProps.GetStyleSheet()->GetName(),
                                SfxStyleFamily::All));
                    }
                }
            }

            if(pTargetStyleSheet)
            {
                if(HasSfxItemSet())
                {
                    // The SfxItemSet has been cloned and exists,
                    // we can directly set the SfxStyleSheet at it
                    ImpAddStyleSheet(pTargetStyleSheet, true);
                }
                else
                {
                    // No SfxItemSet exists yet (there is none in
                    // the source, so none was cloned). Remember the
                    // SfxStyleSheet to set it when the SfxItemSet
                    // got constructed on-demand
                    mpStyleSheet = pTargetStyleSheet;
                }
            }
        }

        AttributeProperties::~AttributeProperties()
        {
            ImpRemoveStyleSheet();
        }

        std::unique_ptr<BaseProperties> AttributeProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new AttributeProperties(*this, rObj));
        }

        const SfxItemSet& AttributeProperties::GetObjectItemSet() const
        {
            // remember if we had a SfxItemSet already
            const bool bHadSfxItemSet(HasSfxItemSet());

            // call parent - this will guarantee SfxItemSet existence
            DefaultProperties::GetObjectItemSet();

            if(!bHadSfxItemSet)
            {
                // need to take care for SfxStyleSheet for newly
                // created SfxItemSet
                if(nullptr == mpStyleSheet)
                {
                    // Set missing defaults without removal of hard attributes.
                    // This is more complicated historically than I first thought:
                    // Originally for GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj
                    // SetStyleSheet(..., false) was used, while for GetDefaultStyleSheet
                    // SetStyleSheet(..., true) was used. Thus, for SdrGrafObj and SdrOle2Obj
                    // bDontRemoveHardAttr == false -> *do* delete hard attributes was used.
                    // This was probably not done by purpose, adding the method
                    // GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj additionally to
                    // GetDefaultStyleSheet was an enhancement to allow for SdrGrafObj/SdrOle2Obj
                    // with full AttributeSet (adding e.g. FillAttributes). To stay as compatible
                    // as possible these SdrObjects got a new default-StyleSheet.
                    // There is no reason to delete the HardAttributes and it anyways has only
                    // AFAIK effects on a single Item - the SdrTextHorzAdjustItem. To get things
                    // unified I will stay with not deleting the HardAttributes and adapt the
                    // UnitTests in CppunitTest_sd_import_tests accordingly.
                    const_cast< AttributeProperties* >(this)->applyDefaultStyleSheetFromSdrModel();
                }
                else
                {
                    // Late-Init of setting parent to SfxStyleSheet after
                    // it's creation. Can only happen from copy-constructor
                    // (where creation of SfxItemSet is avoided due to the
                    // problem with constructors and virtual functions in C++),
                    // thus DontRemoveHardAttr is not needed.
                    const_cast< AttributeProperties* >(this)->SetStyleSheet(
                        mpStyleSheet,
                        true);
                }
            }

            return *mpItemSet;
        }

        void AttributeProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            // own modifications
            SdrObject& rObj = GetSdrObject();

            rObj.SetBoundRectDirty();
            rObj.SetRectsDirty(true);
            rObj.SetChanged();
        }

        void AttributeProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            if(pNewItem)
            {
                const SfxPoolItem* pResultItem = nullptr;
                SdrModel& rModel(GetSdrObject().getSdrModelFromSdrObject());

                switch( nWhich )
                {
                    case XATTR_FILLBITMAP:
                    {
                        // TTTT checkForUniqueItem should use SdrModel&
                        pResultItem = static_cast<const XFillBitmapItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_LINEDASH:
                    {
                        pResultItem = static_cast<const XLineDashItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_LINESTART:
                    {
                        pResultItem = static_cast<const XLineStartItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_LINEEND:
                    {
                        pResultItem = static_cast<const XLineEndItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_FILLGRADIENT:
                    {
                        pResultItem = static_cast<const XFillGradientItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_FILLFLOATTRANSPARENCE:
                    {
                        // #85953# allow all kinds of XFillFloatTransparenceItem to be set
                        pResultItem = static_cast<const XFillFloatTransparenceItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                    case XATTR_FILLHATCH:
                    {
                        pResultItem = static_cast<const XFillHatchItem*>(pNewItem)->checkForUniqueItem( &rModel );
                        break;
                    }
                }

                // guarantee SfxItemSet existence
                GetObjectItemSet();

                if(pResultItem)
                {
                    // force ItemSet
                    mpItemSet->Put(*pResultItem);

                    // delete item if it was a generated one
                    delete pResultItem;
                }
                else
                {
                    mpItemSet->Put(*pNewItem);
                }
            }
            else
            {
                // clear item if ItemSet exists
                if(HasSfxItemSet())
                {
                    mpItemSet->ClearItem(nWhich);
                }
            }
        }

        void AttributeProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // guarantee SfxItemSet existence
            GetObjectItemSet();

            ImpRemoveStyleSheet();
            ImpAddStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            SdrObject& rObj = GetSdrObject();
            rObj.SetBoundRectDirty();
            rObj.SetRectsDirty(true);
        }

        SfxStyleSheet* AttributeProperties::GetStyleSheet() const
        {
            return mpStyleSheet;
        }

        void AttributeProperties::ForceStyleToHardAttributes()
        {
            if(!GetStyleSheet() || dynamic_cast<const SfxStyleSheet *>(mpStyleSheet) == nullptr)
                return;

            // guarantee SfxItemSet existence
            GetObjectItemSet();

            // prepare copied, new itemset, but WITHOUT parent
            SfxItemSet* pDestItemSet = new SfxItemSet(*mpItemSet);
            pDestItemSet->SetParent(nullptr);

            // prepare forgetting the current stylesheet like in RemoveStyleSheet()
            EndListening(*mpStyleSheet);
            EndListening(*mpStyleSheet->GetPool());

            // prepare the iter; use the mpObjectItemSet which may have less
            // WhichIDs than the style.
            SfxWhichIter aIter(*pDestItemSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem = nullptr;

            // now set all hard attributes of the current at the new itemset
            while(nWhich)
            {
                // #i61284# use mpItemSet with parents, makes things easier and reduces to
                // one loop
                if(SfxItemState::SET == mpItemSet->GetItemState(nWhich, true, &pItem))
                {
                    pDestItemSet->Put(*pItem);
                }

                nWhich = aIter.NextWhich();
            }

            // replace itemsets
            mpItemSet.reset(pDestItemSet);

            // set necessary changes like in RemoveStyleSheet()
            GetSdrObject().SetBoundRectDirty();
            GetSdrObject().SetRectsDirty(true);

            mpStyleSheet = nullptr;
        }

        void AttributeProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            bool bHintUsed(false);

            const SfxStyleSheetHint* pStyleHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);

            if(pStyleHint && pStyleHint->GetStyleSheet() == GetStyleSheet())
            {
                SdrObject& rObj = GetSdrObject();
                //SdrPage* pPage = rObj.GetPage();

                switch(pStyleHint->GetId())
                {
                    case SfxHintId::StyleSheetCreated         :
                    {
                        // cannot happen, nothing to do
                        break;
                    }
                    case SfxHintId::StyleSheetModified        :
                    case SfxHintId::StyleSheetChanged         :
                    {
                        // notify change
                        break;
                    }
                    case SfxHintId::StyleSheetErased          :
                    case SfxHintId::StyleSheetInDestruction   :
                    {
                        // Style needs to be exchanged
                        SfxStyleSheet* pNewStSh = nullptr;
                        SdrModel& rModel(rObj.getSdrModelFromSdrObject());

                        // Do nothing if object is in destruction, else a StyleSheet may be found from
                        // a StyleSheetPool which is just being deleted itself. and thus it would be fatal
                        // to register as listener to that new StyleSheet.
                        if(!rObj.IsInDestruction())
                        {
                            if(dynamic_cast<const SfxStyleSheet *>(GetStyleSheet()) != nullptr)
                            {
                                pNewStSh = static_cast<SfxStyleSheet*>(rModel.GetStyleSheetPool()->Find(
                                    GetStyleSheet()->GetParent(), GetStyleSheet()->GetFamily()));
                            }

                            if(!pNewStSh)
                            {
                                pNewStSh = rModel.GetDefaultStyleSheet();
                            }
                        }

                        // remove used style, it's erased or in destruction
                        ImpRemoveStyleSheet();

                        if(pNewStSh)
                        {
                            ImpAddStyleSheet(pNewStSh, true);
                        }

                        break;
                    }
                    default: break;
                }

                // Get old BoundRect. Do this after the style change is handled
                // in the ItemSet parts because GetBoundRect() may calculate a new
                tools::Rectangle aBoundRect = rObj.GetLastBoundRect();

                rObj.SetRectsDirty(true);

                // tell the object about the change
                rObj.SetChanged();
                rObj.BroadcastObjectChange();

                //if(pPage && pPage->IsInserted())
                //{
                //  rObj.BroadcastObjectChange();
                //}

                rObj.SendUserCall(SdrUserCallType::ChangeAttr, aBoundRect);

                bHintUsed = true;
            }

            if(!bHintUsed)
            {
                // forward to SdrObject ATM. Not sure if this will be necessary
                // in the future.
                GetSdrObject().Notify(rBC, rHint);
            }
        }

        bool AttributeProperties::isUsedByModel() const
        {
            const SdrObject& rObj(GetSdrObject());
            if (rObj.IsInserted())
            {
                const SdrPage* const pPage(rObj.getSdrPageFromSdrObject());
                if (pPage && pPage->IsInserted())
                    return true;
            }
            return false;
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
