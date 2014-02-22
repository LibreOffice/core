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

#include <svx/sdr/properties/e3dsceneproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svddef.hxx>
#include <svx/scene3d.hxx>
#include <svx/svditer.hxx>



namespace sdr
{
    namespace properties
    {
        E3dSceneProperties::E3dSceneProperties(SdrObject& rObj)
        :   E3dProperties(rObj)
        {
        }

        E3dSceneProperties::E3dSceneProperties(const E3dSceneProperties& rProps, SdrObject& rObj)
        :   E3dProperties(rProps, rObj)
        {
        }

        E3dSceneProperties::~E3dSceneProperties()
        {
        }

        BaseProperties& E3dSceneProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dSceneProperties(*this, rObj));
        }

        const SfxItemSet& E3dSceneProperties::GetObjectItemSet() const
        {
            
            return E3dProperties::GetObjectItemSet();
        }

        const SfxItemSet& E3dSceneProperties::GetMergedItemSet() const
        {
            
            if(mpItemSet)
            {
                
                SfxItemSet aNew(*mpItemSet->GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
                aNew.Put(*mpItemSet);
                mpItemSet->ClearItem();
                mpItemSet->Put(aNew);
            }
            else
            {
                
                GetObjectItemSet();
            }

            
            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                SdrObject* pObj = pSub->GetObj(a);

                if(pObj && pObj->ISA(E3dCompoundObject))
                {
                    const SfxItemSet& rSet = pObj->GetMergedItemSet();
                    SfxWhichIter aIter(rSet);
                    sal_uInt16 nWhich(aIter.FirstWhich());

                    while(nWhich)
                    {
                        
                        
                        if(nWhich <= SDRATTR_3DSCENE_FIRST || nWhich >= SDRATTR_3DSCENE_LAST)
                        {
                            if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, false))
                            {
                                mpItemSet->InvalidateItem(nWhich);
                            }
                            else
                            {
                                mpItemSet->MergeValue(rSet.Get(nWhich), sal_True);
                            }
                        }

                        nWhich = aIter.NextWhich();
                    }
                }
            }

            
            return E3dProperties::GetMergedItemSet();
        }

        void E3dSceneProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            
            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            if(nCount)
            {
                
                
                SfxItemSet* pNewSet = rSet.Clone(sal_True);
                DBG_ASSERT(pNewSet, "E3dSceneProperties::SetMergedItemSet(): Could not clone ItemSet (!)");

                for(sal_uInt16 b(SDRATTR_3DSCENE_FIRST); b <= SDRATTR_3DSCENE_LAST; b++)
                {
                    pNewSet->ClearItem(b);
                }

                if(pNewSet->Count())
                {
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        SdrObject* pObj = pSub->GetObj(a);

                        if(pObj && pObj->ISA(E3dCompoundObject))
                        {
                            
                            pObj->SetMergedItemSet(*pNewSet, bClearAllItems);
                        }
                    }
                }

                delete pNewSet;
            }

            
            E3dProperties::SetMergedItemSet(rSet, bClearAllItems);
        }

        void E3dSceneProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->SetMergedItem(rItem);
            }

            
            E3dProperties::SetMergedItem(rItem);
        }

        void E3dSceneProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->ClearMergedItem(nWhich);
            }

            
            E3dProperties::ClearMergedItem(nWhich);
        }

        void E3dSceneProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            
            E3dProperties::PostItemChange(nWhich);

            
            E3dScene& rObj = (E3dScene&)GetSdrObject();
            rObj.StructureChanged();

            switch(nWhich)
            {
                case SDRATTR_3DSCENE_PERSPECTIVE            :
                case SDRATTR_3DSCENE_DISTANCE               :
                case SDRATTR_3DSCENE_FOCAL_LENGTH           :
                {
                    
                    
                    
                    Camera3D aSceneCam(rObj.GetCamera());
                    bool bChange(false);

                    
                    if(aSceneCam.GetProjection() != rObj.GetPerspective())
                    {
                        aSceneCam.SetProjection(rObj.GetPerspective());
                        bChange = true;
                    }

                    
                    basegfx::B3DPoint aActualPosition(aSceneCam.GetPosition());
                    double fNew = rObj.GetDistance();

                    if(fNew != aActualPosition.getZ())
                    {
                        aSceneCam.SetPosition(basegfx::B3DPoint(aActualPosition.getX(), aActualPosition.getY(), fNew));
                        bChange = true;
                    }

                    
                    fNew = rObj.GetFocalLength() / 100.0;

                    if(aSceneCam.GetFocalLength() != fNew)
                    {
                        aSceneCam.SetFocalLength(fNew);
                        bChange = true;
                    }

                    
                    if(bChange)
                    {
                        rObj.SetCamera(aSceneCam);
                    }

                    break;
                }
            }
        }

        void E3dSceneProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }

        SfxStyleSheet* E3dSceneProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = 0L;

            const SdrObjList* pSub = ((const E3dScene&)GetSdrObject()).GetSubList();
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

        void E3dSceneProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
            if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
            {
                
                E3dProperties::MoveToItemPool(pSrcPool, pDestPool, pNewModel);

                
                E3dScene& rObj = (E3dScene&)GetSdrObject();
                const SdrObjList* pSubList = rObj.GetSubList();

                if(pSubList && rObj.GetScene() == &rObj)
                {
                    SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);

                    while(a3DIterator.IsMore())
                    {
                        E3dObject* pObj = (E3dObject*)a3DIterator.Next();
                        DBG_ASSERT(pObj->ISA(E3dObject), "In scenes there are only 3D objects allowed (!)");
                        pObj->GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
                    }
                }
            }
        }

        void E3dSceneProperties::SetSceneItemsFromCamera()
        {
            
            GetObjectItemSet();

            E3dScene& rObj = (E3dScene&)GetSdrObject();
            Camera3D aSceneCam(rObj.GetCamera());

            
            mpItemSet->Put(Svx3DPerspectiveItem((sal_uInt16)aSceneCam.GetProjection()));

            
            mpItemSet->Put(Svx3DDistanceItem((sal_uInt32)(aSceneCam.GetPosition().getZ() + 0.5)));

            
            mpItemSet->Put(Svx3DFocalLengthItem((sal_uInt32)((aSceneCam.GetFocalLength() * 100.0) + 0.5)));
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
