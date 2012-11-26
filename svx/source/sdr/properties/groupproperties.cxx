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
#include <svx/sdr/properties/groupproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& GroupProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Groups have in principle no ItemSet. To support methods like
            // GetMergedItemSet() the local one is used. Thus, all items in the pool
            // may be used and a pool itemset is created.
            return *(new SfxItemSet(rPool));
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
            DBG_ASSERT(sal_False, "GroupProperties::GetObjectItemSet() should never be called (!)");
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
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                const SfxItemSet& rSet = pSub->GetObj(a)->GetMergedItemSet();
                SfxWhichIter aIter(rSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, sal_False))
                    {
                        mpItemSet->InvalidateItem(nWhich);
                    }
                    else
                    {
                        mpItemSet->MergeValue(rSet.Get(nWhich), sal_True);
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
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
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
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItem() should never be called (!)");
        }

        void GroupProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ClearObjectItem() should never be called (!)");
        }

        void GroupProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().SetMergedItem(rItem);
            }
        }

        void GroupProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().ClearMergedItem(nWhich);
            }
        }

        void GroupProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItemSet() should never be called (!)");
        }

        void GroupProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ItemSetChanged() should never be called (!)");
        }

        bool GroupProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            DBG_ASSERT(sal_False, "GroupProperties::AllowItemChange() should never be called (!)");
            return sal_False;
        }

        void GroupProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ItemChange() should never be called (!)");
        }

        void GroupProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "GroupProperties::PostItemChange() should never be called (!)");
        }

        SfxStyleSheet* GroupProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = 0L;

            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                SfxStyleSheet* pCandidate = pSub->GetObj(a)->GetStyleSheet();

                if(pRetval)
                {
                    if(pCandidate != pRetval)
                    {
                        // different StyleSheelts, return none
                        return 0L;
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
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
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
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();
            const sal_uInt32 nCount(pSub ? pSub->GetObjCount() : 0);

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().ForceStyleToHardAttributes();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
