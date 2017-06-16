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

#include <sdr/properties/e3dsceneproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/svddef.hxx>
#include <svx/scene3d.hxx>
#include <svx/svditer.hxx>
#include <memory>


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

        const SfxItemSet& E3dSceneProperties::GetMergedItemSet() const
        {
            // prepare ItemSet
            if(mpItemSet)
            {
                // filter for SDRATTR_3DSCENE_ items, only keep those items
                SfxItemSet aNew(*mpItemSet->GetPool(), svl::Items<SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST>{});
                aNew.Put(*mpItemSet);
                mpItemSet->ClearItem();
                mpItemSet->Put(aNew);
            }
            else
            {
                // No ItemSet yet, force local ItemSet
                GetObjectItemSet();
            }

            // collect all ItemSets of contained 3d objects
            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                SdrObject* pObj = pSub->GetObj(a);

                if(pObj && dynamic_cast<const E3dCompoundObject* >(pObj) !=  nullptr)
                {
                    const SfxItemSet& rSet = pObj->GetMergedItemSet();
                    SfxWhichIter aIter(rSet);
                    sal_uInt16 nWhich(aIter.FirstWhich());

                    while(nWhich)
                    {
                        // Leave out the SDRATTR_3DSCENE_ range, this would only be double
                        // and always equal.
                        if(nWhich <= SDRATTR_3DSCENE_FIRST || nWhich >= SDRATTR_3DSCENE_LAST)
                        {
                            if(SfxItemState::DONTCARE == rSet.GetItemState(nWhich, false))
                            {
                                mpItemSet->InvalidateItem(nWhich);
                            }
                            else
                            {
                                mpItemSet->MergeValue(rSet.Get(nWhich), true);
                            }
                        }

                        nWhich = aIter.NextWhich();
                    }
                }
            }

            // call parent
            return E3dProperties::GetMergedItemSet();
        }

        void E3dSceneProperties::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems)
        {
            // Set SDRATTR_3DOBJ_ range at contained objects.
            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            if(nCount)
            {
                // Generate filtered ItemSet which contains all but the SDRATTR_3DSCENE items.
                // #i50808# Leak fix, Clone produces a new instance and we get ownership here
                std::unique_ptr<SfxItemSet> pNewSet(rSet.Clone());
                DBG_ASSERT(pNewSet, "E3dSceneProperties::SetMergedItemSet(): Could not clone ItemSet (!)");

                for(sal_uInt16 b(SDRATTR_3DSCENE_FIRST); b <= SDRATTR_3DSCENE_LAST; b++)
                {
                    pNewSet->ClearItem(b);
                }

                if(pNewSet->Count())
                {
                    for(size_t a = 0; a < nCount; ++a)
                    {
                        SdrObject* pObj = pSub->GetObj(a);

                        if(pObj && dynamic_cast<const E3dCompoundObject* >(pObj) !=  nullptr)
                        {
                            // set merged ItemSet at contained 3d object.
                            pObj->SetMergedItemSet(*pNewSet, bClearAllItems);
                        }
                    }
                }
            }

            // call parent. This will set items on local object, too.
            E3dProperties::SetMergedItemSet(rSet, bClearAllItems);
        }

        void E3dSceneProperties::SetMergedItem(const SfxPoolItem& rItem)
        {
            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->SetMergedItem(rItem);
            }

            // #i43809# call parent. This will set items on local object, too.
            E3dProperties::SetMergedItem(rItem);
        }

        void E3dSceneProperties::ClearMergedItem(const sal_uInt16 nWhich)
        {
            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
            const size_t nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->ClearMergedItem(nWhich);
            }

            // #i43809# call parent. This will clear items on local object, too.
            E3dProperties::ClearMergedItem(nWhich);
        }

        void E3dSceneProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            // call parent
            E3dProperties::PostItemChange(nWhich);

            // local changes
            E3dScene& rObj = static_cast<E3dScene&>(GetSdrObject());
            rObj.StructureChanged();

            switch(nWhich)
            {
                case SDRATTR_3DSCENE_PERSPECTIVE            :
                case SDRATTR_3DSCENE_DISTANCE               :
                case SDRATTR_3DSCENE_FOCAL_LENGTH           :
                {
                    // #83387#, #83391#
                    // one common function for the camera attributes
                    // since SetCamera() sets all three back to the ItemSet
                    Camera3D aSceneCam(rObj.GetCamera());
                    bool bChange(false);

                    // for SDRATTR_3DSCENE_PERSPECTIVE:
                    if(aSceneCam.GetProjection() != rObj.GetPerspective())
                    {
                        aSceneCam.SetProjection(rObj.GetPerspective());
                        bChange = true;
                    }

                    // for SDRATTR_3DSCENE_DISTANCE:
                    basegfx::B3DPoint aActualPosition(aSceneCam.GetPosition());
                    double fNew = rObj.GetDistance();

                    if(fNew != aActualPosition.getZ())
                    {
                        aSceneCam.SetPosition(basegfx::B3DPoint(aActualPosition.getX(), aActualPosition.getY(), fNew));
                        bChange = true;
                    }

                    // for SDRATTR_3DSCENE_FOCAL_LENGTH:
                    fNew = rObj.GetFocalLength() / 100.0;

                    if(aSceneCam.GetFocalLength() != fNew)
                    {
                        aSceneCam.SetFocalLength(fNew);
                        bChange = true;
                    }

                    // for all
                    if(bChange)
                    {
                        rObj.SetCamera(aSceneCam);
                    }

                    break;
                }
            }
        }

        void E3dSceneProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(size_t a = 0; a < nCount; ++a)
            {
                pSub->GetObj(a)->SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }

        SfxStyleSheet* E3dSceneProperties::GetStyleSheet() const
        {
            SfxStyleSheet* pRetval = nullptr;

            const SdrObjList* pSub = static_cast<const E3dScene&>(GetSdrObject()).GetSubList();
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

        void E3dSceneProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
            if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
            {
                // call parent
                E3dProperties::MoveToItemPool(pSrcPool, pDestPool, pNewModel);

                // own reaction, but only with outmost scene
                E3dScene& rObj = static_cast<E3dScene&>(GetSdrObject());
                const SdrObjList* pSubList = rObj.GetSubList();

                if(pSubList && rObj.GetScene() == &rObj)
                {
                    SdrObjListIter a3DIterator(*pSubList, SdrIterMode::DeepWithGroups);

                    while(a3DIterator.IsMore())
                    {
                        E3dObject* pObj = static_cast<E3dObject*>(a3DIterator.Next());
                        DBG_ASSERT(dynamic_cast<const E3dObject* >(pObj) !=  nullptr, "In scenes there are only 3D objects allowed (!)");
                        pObj->GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
                    }
                }
            }
        }

        void E3dSceneProperties::SetSceneItemsFromCamera()
        {
            // force ItemSet
            GetObjectItemSet();

            E3dScene& rObj = static_cast<E3dScene&>(GetSdrObject());
            Camera3D aSceneCam(rObj.GetCamera());

            // ProjectionType
            mpItemSet->Put(Svx3DPerspectiveItem(aSceneCam.GetProjection()));

            // CamPos
            mpItemSet->Put(makeSvx3DDistanceItem((sal_uInt32)(aSceneCam.GetPosition().getZ() + 0.5)));

            // FocalLength
            mpItemSet->Put(makeSvx3DFocalLengthItem((sal_uInt32)((aSceneCam.GetFocalLength() * 100.0) + 0.5)));
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
