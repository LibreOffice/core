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
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>

namespace sdr
{
    namespace properties
    {
        SfxStyleSheet* AttributeProperties::ImpGetDefaultStyleSheet() const
        {
            // use correct default stylesheet #119287#
            const SdrGrafObj* pIsSdrGrafObj(dynamic_cast< const SdrGrafObj* >(&GetSdrObject()));
            const SdrOle2Obj* pIsSdrOle2Obj(dynamic_cast< const SdrOle2Obj* >(&GetSdrObject()));
            SfxStyleSheet* pRetval(nullptr);

            if(pIsSdrGrafObj || pIsSdrOle2Obj)
            {
                pRetval = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();
            }
            else
            {
                pRetval = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheet();
            }

            return pRetval;
        }

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
                    StartListening(pNewStyleSheet->GetPool());
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
                EndListening(mpStyleSheet->GetPool());

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

            if(pTargetStyleSheet && &rObj.getSdrModelFromSdrObject() != &GetSdrObject().getSdrModelFromSdrObject())
            {
                // TTTT It is a clone to another model, thus the TargetStyleSheet
                // is probably also from another SdrModel, so do *not* simply use it.
                //
                // The DefaultProperties::Clone already has cloned the ::SET items
                // to a new SfxItemSet in the new SfxItemPool. There are quite some
                // possibilities to continue:
                // - Do not use StyleSheet (will do this for now)
                // - Search for same StyleSheet in Target-SdrModel and use if found
                //   (use e.g. Name)
                // - Clone used StyleSheet(s) to Target-SdrModel and use
                // - Set all Attributes from the StyleSheet as hard attributes at the
                //   SfxItemSet
                // The original AW080 uses 'ImpModelChange' (see there) which Clones
                // and uses the used StyleSheets if there is a Target-SfxItemPool
                // and sets to hard attributes if not. This may be used later if needed,
                // but for now only a single UnitTest uses this Clone-scenario and works
                // well with not using the TargetStyleSheet. The logic Cloning
                // StyleSheets *should* - if needed - be on a higher level where it is
                // potentially better known what would be the correct thing to do.
                pTargetStyleSheet = nullptr;
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

        BaseProperties& AttributeProperties::Clone(SdrObject& rObj) const
        {
            return *(new AttributeProperties(*this, rObj));
        }

        const SfxItemSet& AttributeProperties::GetObjectItemSet() const
        {
            // remember if we had a SfxItemSet already
            const bool bHadSfxItemSet(HasSfxItemSet());

            // call parent - this will then guarantee
            // SfxItemSet existance
            DefaultProperties::GetObjectItemSet();

            if(!bHadSfxItemSet)
            {
                if(GetStyleSheet())
                {
                    // Late-Init of setting parent to SfxStyleSheet after
                    // it's creation. See copy-constructor and how it remembers
                    // the SfxStyleSheet there.
                    // It is necessary to reset mpStyleSheet to nullptr to
                    // not trigger alarm insde ImpAddStyleSheet (!)
                    SfxStyleSheet* pNew(mpStyleSheet);
                    const_cast< AttributeProperties* >(this)->mpStyleSheet = nullptr;
                    const_cast< AttributeProperties* >(this)->ImpAddStyleSheet(
                        pNew,
                        true);
                }
                else
                {
                    // Set missing defaults and do not RemoveHardAttributes
                    const_cast< AttributeProperties* >(this)->ImpAddStyleSheet(
                        ImpGetDefaultStyleSheet(),
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

                // set item
                if(!HasSfxItemSet())
                {
                    GetObjectItemSet();
                }

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
            // guarantee SfxItemSet existance here
            if(!HasSfxItemSet())
            {
                GetObjectItemSet();
            }

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

            // force SfxItemSet existence
            if(!HasSfxItemSet())
            {
                GetObjectItemSet();
            }

            // prepare copied, new itemset, but WITHOUT parent
            SfxItemSet* pDestItemSet = new SfxItemSet(*mpItemSet);
            pDestItemSet->SetParent(nullptr);

            // prepare forgetting the current stylesheet like in RemoveStyleSheet()
            EndListening(*mpStyleSheet);
            EndListening(mpStyleSheet->GetPool());

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
                const SdrPage* const pPage(rObj.GetPage());
                if (pPage && pPage->IsInserted())
                    return true;
            }
            return false;
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
