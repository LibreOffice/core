/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/properties/groupproperties.hxx>
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
        
        SfxItemSet& GroupProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            
            
            
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
            DBG_ASSERT(false, "GroupProperties::GetObjectItemSet() should never be called (!)");
            return DefaultProperties::GetObjectItemSet();
        }

        const SfxItemSet& GroupProperties::GetMergedItemSet() const
        {
            
            if(mpItemSet)
            {
                
                mpItemSet->ClearItem();
            }
            else
            {
                
                DefaultProperties::GetObjectItemSet();
            }

            
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                const SfxItemSet& rSet = pSub->GetObj(a)->GetMergedItemSet();
                SfxWhichIter aIter(rSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, false))
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

            
            
            return *mpItemSet;
        }

        void GroupProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                SdrObject* pObj = pSub->GetObj(a);

                if(pObj)
                {
                    
                    pObj->SetMergedItemSet(rSet, bClearAllItems);
                }
            }

            
            
            
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
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().SetMergedItem(rItem);
            }
        }

        void GroupProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
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

        sal_Bool GroupProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            DBG_ASSERT(false, "GroupProperties::AllowItemChange() should never be called (!)");
            return sal_False;
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
            SfxStyleSheet* pRetval = 0L;

            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                SfxStyleSheet* pCandidate = pSub->GetObj(a)->GetStyleSheet();

                if(pRetval)
                {
                    if(pCandidate != pRetval)
                    {
                        
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

        void GroupProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }

        void GroupProperties::ForceDefaultAttributes()
        {
            
        }

        void GroupProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
            if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
            {
                const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
                const sal_uInt32 nCount(pSub->GetObjCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    pSub->GetObj(a)->GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
                }

                
                if(mpItemSet)
                {
                    
                    
                    
                    
                    
                    
                    delete mpItemSet;
                    mpItemSet = 0L;
                }
            }
        }

        void GroupProperties::ForceStyleToHardAttributes()
        {
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().ForceStyleToHardAttributes();
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
