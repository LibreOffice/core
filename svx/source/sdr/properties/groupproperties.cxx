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

#include <sdr/properties/groupproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdpage.hxx>



namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet* GroupProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Groups have in principle no ItemSet. To support methods like
            // GetMergedItemSet() the local one is used. Thus, all items in the pool
            // may be used and a pool itemset is created.
            return new SfxItemSet(rPool);
        }

        GroupProperties::GroupProperties(SdrObject& rObj)
        :   DefaultProperties(rObj)
        {
        }

        GroupProperties::GroupProperties(const GroupProperties& rProps, SdrObject& rObj)
        :   DefaultProperties(rProps, rObj)
        {
        }

        GroupProperties::~GroupProperties()
        {
        }

        BaseProperties& GroupProperties::Clone(SdrObject& rObj) const
        {
            return *(new GroupProperties(*this, rObj));
        }

        const SfxItemSet& GroupProperties::GetObjectItemSet() const
        {
            DBG_ASSERT(false, "GroupProperties::GetObjectItemSet() should never be called (!)");
            return DefaultProperties::GetObjectItemSet();
        }

        const SfxItemSet& GroupProperties::GetMergedItemSet() const
        {
            // prepare ItemSet
            if(mpItemSet)
            {
                // clear local itemset for merge
                mpItemSet->ClearItem();
            }
            else
            {
                // force local itemset
                DefaultProperties::GetObjectItemSet();
            }

            // collect all ItemSets in mpItemSet
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                const SfxItemSet& rSet = pSub->GetObj(a)->GetMergedItemSet();
                SfxWhichIter aIter(rSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SfxItemState::DONTCARE == rSet.GetItemState(nWhich, false))
                    {
                        mpItemSet->InvalidateItem(nWhich);
                    }
                    else
                    {
                        mpItemSet->MergeValue(rSet.Get(nWhich), true);
                    }

                    nWhich = aIter.NextWhich();
                }
            }

            // For group proerties, do not call parent since groups do
            // not have local ItemSets.
            return *mpItemSet;
        }

        void GroupProperties::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems)
        {
            // iterate over contained SdrObjects
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                SdrObject* pObj = pSub->GetObj(a);

                if(pObj)
                {
                    // Set merged ItemSet at contained object
                    pObj->SetMergedItemSet(rSet, bClearAllItems);
                }
            }

            // Do not call parent here. Group objects do not have local ItemSets
            // where items need to be set.
            // DefaultProperties::SetMergedItemSet(rSet, bClearAllItems);
        }

        void GroupProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(false, "GroupProperties::SetObjectItem() should never be called (!)");
        }

        void GroupProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(false, "GroupProperties::SetObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(false, "GroupProperties::ClearObjectItem() should never be called (!)");
        }

        void GroupProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(false, "GroupProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().SetMergedItem(rItem);
            }
        }

        void GroupProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().ClearMergedItem(nWhich);
            }
        }

        void GroupProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(false, "GroupProperties::SetObjectItemSet() should never be called (!)");
        }

        void GroupProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(false, "GroupProperties::ItemSetChanged() should never be called (!)");
        }

        bool GroupProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            DBG_ASSERT(false, "GroupProperties::AllowItemChange() should never be called (!)");
            return false;
        }

        void GroupProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            DBG_ASSERT(false, "GroupProperties::ItemChange() should never be called (!)");
        }

        void GroupProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(false, "GroupProperties::PostItemChange() should never be called (!)");
        }

        SfxStyleSheet* GroupProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = nullptr;

            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                SfxStyleSheet* pCandidate = pSub->GetObj(a)->GetStyleSheet();

                if(pRetval)
                {
                    if(pCandidate != pRetval)
                    {
                        // different StyleSheelts, return none
                        return nullptr;
                    }
                }
                else
                {
                    pRetval = pCandidate;
                }
            }

            return pRetval;
        }

        void GroupProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }

        void GroupProperties::ForceDefaultAttributes()
        {
            // nothing to do here, groups have no items and thus no default items, too.
        }

        void GroupProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
            if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
            {
                const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
                const size_t nCount(pSub->GetObjCount());

                for(size_t a = 0; a < nCount; ++a)
                {
                    pSub->GetObj(a)->GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
                }

                // also clear local ItemSet, it's only temporary for group objects anyways.
                if(mpItemSet)
                {
                    // copy/paste is still using clone operators and MoveToItemPool functionality.
                    // Since SfxItemSet contains a pool pointer, ClearItem is not enough here.
                    // The ItemSet for merge is constructed on demand, so it's enough here to
                    // just delete it and set to 0L.
                    // mpItemSet->ClearItem();
                    delete mpItemSet;
                    mpItemSet = nullptr;
                }
            }
        }

        void GroupProperties::ForceStyleToHardAttributes()
        {
            const SdrObjList* pSub = static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().ForceStyleToHardAttributes();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
