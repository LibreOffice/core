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
        std::unique_ptr<SfxItemSet> GroupProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Groups have in principle no ItemSet. To support methods like
            // GetMergedItemSet() the local one is used. Thus, all items in the pool
            // may be used and a pool itemset is created.
            return std::make_unique<SfxItemSet>(rPool);
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

        std::unique_ptr<BaseProperties> GroupProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new GroupProperties(*this, rObj));
        }

        const SfxItemSet& GroupProperties::GetObjectItemSet() const
        {
            assert(!"GroupProperties::GetObjectItemSet() should never be called");
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
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

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

            // For group properties, do not call parent since groups do
            // not have local ItemSets.
            return *mpItemSet;
        }

        void GroupProperties::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems)
        {
            // iterate over contained SdrObjects
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

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
            assert(!"GroupProperties::SetObjectItem() should never be called");
        }

        void GroupProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            assert(!"GroupProperties::SetObjectItemDirect() should never be called");
        }

        void GroupProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            assert(!"GroupProperties::ClearObjectItem() should never be called");
        }

        void GroupProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            assert(!"GroupProperties::ClearObjectItemDirect() should never be called");
        }

        void GroupProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().SetMergedItem(rItem);
            }
        }

        void GroupProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().ClearMergedItem(nWhich);
            }
        }

        void GroupProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            assert(!"GroupProperties::SetObjectItemSet() should never be called");
        }

        void GroupProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            assert(!"GroupProperties::ItemSetChanged() should never be called");
        }

        bool GroupProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            assert(!"GroupProperties::AllowItemChange() should never be called");
            return false;
        }

        void GroupProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            assert(!"GroupProperties::ItemChange() should never be called");
        }

        void GroupProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            assert(!"GroupProperties::PostItemChange() should never be called");
        }

        SfxStyleSheet* GroupProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = nullptr;

            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

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
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }

        void GroupProperties::ForceDefaultAttributes()
        {
            // nothing to do here, groups have no items and thus no default items, too.
        }

        void GroupProperties::ForceStyleToHardAttributes()
        {
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->GetProperties().ForceStyleToHardAttributes();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
