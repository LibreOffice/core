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
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <osl/diagnose.h>


namespace sdr::properties
{
        GroupProperties::GroupProperties(SdrObject& rObj)
        :   BaseProperties(rObj)
        {
        }

        GroupProperties::~GroupProperties()
        {
        }

        std::unique_ptr<BaseProperties> GroupProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new GroupProperties(rObj));
        }

        SfxItemSet GroupProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return SfxItemSet(rPool);
        }

        const SfxItemSet& GroupProperties::GetObjectItemSet() const
        {
            assert(!"GroupProperties::GetObjectItemSet() should never be called");
            abort();
        }

        const SfxItemSet& GroupProperties::GetMergedItemSet() const
        {
            // prepare ItemSet
            if(mxItemSet)
                // clear local itemset for merge
                mxItemSet->ClearItem();
            else if(!mxItemSet)
                // force local itemset
                mxItemSet.emplace(GetSdrObject().GetObjectItemPool());

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
                    if(SfxItemState::DONTCARE == aIter.GetItemState(false))
                    {
                        mxItemSet->InvalidateItem(nWhich);
                    }
                    else
                    {
                        mxItemSet->MergeValue(rSet.Get(nWhich), true);
                    }

                    nWhich = aIter.NextWhich();
                }
            }

            // For group properties, do not call parent since groups do
            // not have local ItemSets.
            return *mxItemSet;
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
        }

        void GroupProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            assert(!"GroupProperties::SetObjectItem() should never be called");
        }

        void GroupProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            assert(!"GroupProperties::SetObjectItemDirect() should never be called");
        }

        void GroupProperties::ClearObjectItem(const sal_uInt16 nWhich)
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
                    pObj->GetProperties().ClearObjectItem(nWhich);
                }
            }
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

        void GroupProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr,
                bool bBroadcast)
        {
            const SdrObjList* pSub(static_cast<const SdrObjGroup&>(GetSdrObject()).GetSubList());
            OSL_ENSURE(nullptr != pSub, "Children of SdrObject expected (!)");
            const size_t nCount(nullptr == pSub ? 0 : pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                if(bBroadcast)
                    pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
                else
                    pSub->GetObj(a)->NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
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
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
