/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupproperties.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 09:57:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDR_PROPERTIES_GROUPPROPERTIES_HXX
#include <svx/sdr/properties/groupproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _EEITEM_HXX
#include <eeitem.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif

#ifndef _SVDPOOL_HXX
#include <svdpool.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

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
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                const SfxItemSet& rSet = pSub->GetObj(a)->GetMergedItemSet();
                SfxWhichIter aIter(rSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, FALSE))
                    {
                        mpItemSet->InvalidateItem(nWhich);
                    }
                    else
                    {
                        mpItemSet->MergeValue(rSet.Get(nWhich), TRUE);
                    }

                    nWhich = aIter.NextWhich();
                }
            }

            // For group proerties, do not call parent since groups do
            // not have local ItemSets.
            return *mpItemSet;
        }

        void GroupProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            // iterate over contained SdrObjects
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

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

        void GroupProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItem() should never be called (!)");
        }

        void GroupProperties::SetObjectItemDirect(const SfxPoolItem& rItem)
        {
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::ClearObjectItem(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ClearObjectItem() should never be called (!)");
        }

        void GroupProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void GroupProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
            //BFS01{
                const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
                const sal_uInt32 nCount(pSub->GetObjCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    pSub->GetObj(a)->GetProperties().SetMergedItem(rItem);
                }
            //BFS01}
        }

        void GroupProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
            //BFS01{
                const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
                const sal_uInt32 nCount(pSub->GetObjCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    pSub->GetObj(a)->GetProperties().ClearMergedItem(nWhich);
                }
            //BFS01}
        }

        void GroupProperties::SetObjectItemSet(const SfxItemSet& rSet)
        {
            DBG_ASSERT(sal_False, "GroupProperties::SetObjectItemSet() should never be called (!)");
        }

        void GroupProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ItemSetChanged() should never be called (!)");
        }

        sal_Bool GroupProperties::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem) const
        {
            DBG_ASSERT(sal_False, "GroupProperties::AllowItemChange() should never be called (!)");
            return sal_False;
        }

        void GroupProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            DBG_ASSERT(sal_False, "GroupProperties::ItemChange() should never be called (!)");
        }

        void GroupProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "GroupProperties::PostItemChange() should never be called (!)");
        }

        SfxStyleSheet* GroupProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = 0L;

            //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
            //BFS01{
                const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
                const sal_uInt32 nCount(pSub->GetObjCount());

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
            //BFS01}

            return pRetval;
        }

        void GroupProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
            //BFS01{
                const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
                const sal_uInt32 nCount(pSub->GetObjCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
                }
            //BFS01}
        }

//BFS01     void GroupProperties::PreProcessSave()
//BFS01     {
//BFS01         //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
//BFS01         //BFS01{
//BFS01             const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
//BFS01             const sal_uInt32 nCount(pSub->GetObjCount());
//BFS01
//BFS01             for(sal_uInt32 a(0L); a < nCount; a++)
//BFS01             {
//BFS01                 pSub->GetObj(a)->GetProperties().PreProcessSave();
//BFS01             }
//BFS01         //BFS01}
//BFS01     }

//BFS01     void GroupProperties::PostProcessSave()
//BFS01     {
//BFS01         //BFS01if(!((const SdrObjGroup&)GetSdrObject()).IsLinkedGroup())
//BFS01         //BFS01{
//BFS01             const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
//BFS01             const sal_uInt32 nCount(pSub->GetObjCount());
//BFS01
//BFS01             for(sal_uInt32 a(0L); a < nCount; a++)
//BFS01             {
//BFS01                 pSub->GetObj(a)->GetProperties().PostProcessSave();
//BFS01             }
//BFS01         //BFS01}
//BFS01     }

        void GroupProperties::ForceDefaultAttributes()
        {
            // nothing to do here, groups have no items and thus no default items, too.
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

                // also clear local ItemSet, it's only temporary for group objects anyways.
                if(mpItemSet)
                {
                    // #121905#
                    // copy/paste is still using clone operators and MoveToItemPool functionality.
                    // Since SfxItemSet contains a pool pointer, ClearItem is not enough here.
                    // The ItemSet for merge is constructed on demand, so it's enough here to
                    // just delete it and set to 0L.
                    // mpItemSet->ClearItem();
                    delete mpItemSet;
                    mpItemSet = 0L;
                }
            }
        }

        void GroupProperties::ForceStyleToHardAttributes(sal_Bool bPseudoSheetsOnly)
        {
            const SdrObjList* pSub = ((const SdrObjGroup&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().ForceStyleToHardAttributes(bPseudoSheetsOnly);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
