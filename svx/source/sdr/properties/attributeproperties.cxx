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

#include <svx/sdr/properties/attributeproperties.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
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


#include <svl/smplhint.hxx>



namespace sdr
{
    namespace properties
    {
        void AttributeProperties::ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            
            
            DBG_ASSERT(!mpStyleSheet, "Old style sheet not deleted before setting new one (!)");

            if(pNewStyleSheet)
            {
                mpStyleSheet = pNewStyleSheet;

                
                GetObjectItemSet();

                
                StartListening(pNewStyleSheet->GetPool());
                StartListening(*pNewStyleSheet);

                
                if(!bDontRemoveHardAttr)
                {
                    const SfxItemSet& rStyle = pNewStyleSheet->GetItemSet();
                    SfxWhichIter aIter(rStyle);
                    sal_uInt16 nWhich = aIter.FirstWhich();

                    while(nWhich)
                    {
                        if(SFX_ITEM_SET == rStyle.GetItemState(nWhich))
                        {
                            mpItemSet->ClearItem(nWhich);
                        }

                        nWhich = aIter.NextWhich();
                    }
                }

                
                mpItemSet->SetParent(&pNewStyleSheet->GetItemSet());
            }
        }

        void AttributeProperties::ImpRemoveStyleSheet()
        {
            
            if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
            {
                EndListening(*mpStyleSheet);
                EndListening(mpStyleSheet->GetPool());

                
                if(mpItemSet)
                {
                    mpItemSet->SetParent(0L);
                }

                SdrObject& rObj = GetSdrObject();
                rObj.SetBoundRectDirty();
                rObj.SetRectsDirty(sal_True);
            }

            mpStyleSheet = 0L;
        }

        
        SfxItemSet& AttributeProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                
                0, 0));
        }

        AttributeProperties::AttributeProperties(SdrObject& rObj)
        :   DefaultProperties(rObj),
            mpStyleSheet(0L)
        {
        }

        AttributeProperties::AttributeProperties(const AttributeProperties& rProps, SdrObject& rObj)
        :   DefaultProperties(rProps, rObj),
            mpStyleSheet(0L)
        {
            if(rProps.GetStyleSheet())
            {
                ImpAddStyleSheet(rProps.GetStyleSheet(), sal_True);
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

        void AttributeProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            
            SdrObject& rObj = GetSdrObject();

            rObj.SetBoundRectDirty();
            rObj.SetRectsDirty(sal_True);
            rObj.SetChanged();
        }

        void AttributeProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            if(pNewItem)
            {
                const SfxPoolItem* pItem = pNewItem;
                SdrModel* pModel = GetSdrObject().GetModel();

                switch( nWhich )
                {
                    case XATTR_FILLBITMAP:
                    {
                        pItem = ((XFillBitmapItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_LINEDASH:
                    {
                        pItem = ((XLineDashItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_LINESTART:
                    {
                        pItem = ((XLineStartItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_LINEEND:
                    {
                        pItem = ((XLineEndItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_FILLGRADIENT:
                    {
                        pItem = ((XFillGradientItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_FILLFLOATTRANSPARENCE:
                    {
                        
                        pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_FILLHATCH:
                    {
                        pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                }

                
                if(pItem)
                {
                    
                    GetObjectItemSet();
                    mpItemSet->Put(*pItem);

                    
                    if(pItem != pNewItem)
                    {
                        delete (SfxPoolItem*)pItem;
                    }
                }
            }
            else
            {
                
                if(mpItemSet)
                {
                    mpItemSet->ClearItem(nWhich);
                }
            }
        }

        void AttributeProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            ImpRemoveStyleSheet();
            ImpAddStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            SdrObject& rObj = GetSdrObject();
            rObj.SetBoundRectDirty();
            rObj.SetRectsDirty(sal_True);
        }

        SfxStyleSheet* AttributeProperties::GetStyleSheet() const
        {
            return mpStyleSheet;
        }

        void AttributeProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
            OSL_ASSERT(pNewModel!=NULL);

            if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
            {
                if(mpItemSet)
                {
                    
                    
                    
                    
                    SfxItemSet* pOldSet = mpItemSet;
                    SfxStyleSheet* pStySheet = GetStyleSheet();

                    if(pStySheet)
                    {
                        ImpRemoveStyleSheet();
                    }

                    mpItemSet = mpItemSet->Clone(sal_False, pDestPool);
                    GetSdrObject().GetModel()->MigrateItemSet(pOldSet, mpItemSet, pNewModel);

                    
                    if(pStySheet)
                    {
                        
                        SfxItemPool* pStyleSheetPool = &pStySheet->GetPool().GetPool();

                        if(pStyleSheetPool == pDestPool)
                        {
                            
                            ImpAddStyleSheet(pStySheet, sal_True);
                        }
                        else
                        {
                            
                            
                            

                            
                            OSL_ASSERT(pNewModel->GetStyleSheetPool() != NULL);
                            SfxStyleSheet* pNewStyleSheet = dynamic_cast<SfxStyleSheet*>(
                                pNewModel->GetStyleSheetPool()->Find(
                                    pStySheet->GetName(),
                                    SFX_STYLE_FAMILY_ALL));
                            if (pNewStyleSheet == NULL
                                || &pNewStyleSheet->GetPool().GetPool() != pDestPool)
                            {
                                
                                
                                pNewStyleSheet = pNewModel->GetDefaultStyleSheet();
                            }
                            ImpAddStyleSheet(pNewStyleSheet, sal_True);
                        }
                    }

                    delete pOldSet;
                }
            }
        }

        void AttributeProperties::SetModel(SdrModel* pOldModel, SdrModel* pNewModel)
        {
            if(pOldModel != pNewModel && pNewModel && !pNewModel->IsLoading())
            {
                
                if(pOldModel)
                {
                    
                    MapUnit aOldUnit(pOldModel->GetScaleUnit());
                    MapUnit aNewUnit(pNewModel->GetScaleUnit());
                    sal_Bool bScaleUnitChanged(aNewUnit != aOldUnit);
                    Fraction aMetricFactor;

                    if(bScaleUnitChanged)
                    {
                        aMetricFactor = GetMapFactor(aOldUnit, aNewUnit).X();
                        Scale(aMetricFactor);
                    }

                    
                    
                    SfxStyleSheet* pOldStyleSheet = GetStyleSheet();

                    if(pOldStyleSheet)
                    {
                        SfxStyleSheetBase* pSheet = pOldStyleSheet;
                        SfxStyleSheetBasePool* pOldPool = pOldModel->GetStyleSheetPool();
                        SfxStyleSheetBasePool* pNewPool = pNewModel->GetStyleSheetPool();
                        DBG_ASSERT(pOldPool, "Properties::SetModel(): Object has StyleSheet but no StyleSheetPool (!)");

                        if(pOldPool && pNewPool)
                        {
                            
                            std::vector<SfxStyleSheetBase*> aStyleList;
                            SfxStyleSheetBase* pAnchor = 0L;

                            while(pSheet)
                            {
                                pAnchor = pNewPool->Find(pSheet->GetName(), pSheet->GetFamily());

                                if(!pAnchor)
                                {
                                    aStyleList.push_back(pSheet);
                                    pSheet = pOldPool->Find(pSheet->GetParent(), pSheet->GetFamily());
                                }
                                else
                                {
                                    
                                    pSheet = 0L;
                                }
                            }

                            
                            SfxStyleSheetBase* pNewSheet = 0L;
                            SfxStyleSheetBase* pLastSheet = 0L;
                            SfxStyleSheetBase* pForThisObject = 0L;

                            std::vector<SfxStyleSheetBase*>::iterator iter;
                            for (iter = aStyleList.begin(); iter != aStyleList.end(); ++iter)
                            {
                                pNewSheet = &pNewPool->Make((*iter)->GetName(), (*iter)->GetFamily(), (*iter)->GetMask());
                                pNewSheet->GetItemSet().Put((*iter)->GetItemSet(), false);

                                if(bScaleUnitChanged)
                                {
                                    sdr::properties::ScaleItemSet(pNewSheet->GetItemSet(), aMetricFactor);
                                }

                                if(pLastSheet)
                                {
                                    pLastSheet->SetParent(pNewSheet->GetName());
                                }

                                if(!pForThisObject)
                                {
                                    pForThisObject = pNewSheet;
                                }

                                pLastSheet = pNewSheet;
                            }

                            
                            if(pAnchor && pLastSheet)
                            {
                                pLastSheet->SetParent(pAnchor->GetName());
                            }

                            
                            
                            if(!pForThisObject && pAnchor)
                            {
                                pForThisObject = pAnchor;
                            }

                            
                            if(GetStyleSheet() != pForThisObject)
                            {
                                ImpRemoveStyleSheet();
                                ImpAddStyleSheet((SfxStyleSheet*)pForThisObject, sal_True);
                            }
                        }
                        else
                        {
                            
                            
                            std::vector<const SfxItemSet*> aSetList;
                            const SfxItemSet* pItemSet = &pOldStyleSheet->GetItemSet();

                            while(pItemSet)
                            {
                                aSetList.push_back(pItemSet);
                                pItemSet = pItemSet->GetParent();
                            }

                            SfxItemSet* pNewSet = &CreateObjectSpecificItemSet(pNewModel->GetItemPool());

                            std::vector<const SfxItemSet*>::reverse_iterator riter;
                            for (riter = aSetList.rbegin(); riter != aSetList.rend(); ++riter)
                                pNewSet->Put(*(*riter));

                            
                            if(mpItemSet)
                            {
                                SfxWhichIter aIter(*mpItemSet);
                                sal_uInt16 nWhich = aIter.FirstWhich();

                                while(nWhich)
                                {
                                    if(mpItemSet->GetItemState(nWhich, false) == SFX_ITEM_SET)
                                    {
                                        pNewSet->Put(mpItemSet->Get(nWhich));
                                    }

                                    nWhich = aIter.NextWhich();
                                }
                            }

                            if(bScaleUnitChanged)
                            {
                                ScaleItemSet(*pNewSet, aMetricFactor);
                            }

                            if(mpItemSet)
                            {
                                if(GetStyleSheet())
                                {
                                    ImpRemoveStyleSheet();
                                }

                                delete mpItemSet;
                                mpItemSet = 0L;
                            }

                            mpItemSet = pNewSet;
                        }
                    }
                }

                
                if(!GetStyleSheet() && pNewModel && !pNewModel->IsLoading())
                {
                    GetObjectItemSet(); 
                    SetStyleSheet(pNewModel->GetDefaultStyleSheet(), sal_True);
                }
            }
        }

        void AttributeProperties::ForceStyleToHardAttributes()
        {
            if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
            {
                
                GetObjectItemSet();
                SfxItemSet* pDestItemSet = new SfxItemSet(*mpItemSet);
                pDestItemSet->SetParent(0L);

                
                EndListening(*mpStyleSheet);
                EndListening(mpStyleSheet->GetPool());

                
                
                SfxWhichIter aIter(*pDestItemSet);
                sal_uInt16 nWhich(aIter.FirstWhich());
                const SfxPoolItem *pItem = NULL;

                
                while(nWhich)
                {
                    
                    
                    if(SFX_ITEM_SET == mpItemSet->GetItemState(nWhich, true, &pItem))
                    {
                        pDestItemSet->Put(*pItem);
                    }

                    nWhich = aIter.NextWhich();
                }

                
                delete mpItemSet;
                mpItemSet = pDestItemSet;

                
                GetSdrObject().SetBoundRectDirty();
                GetSdrObject().SetRectsDirty(sal_True);

                mpStyleSheet = NULL;
            }
        }

        void AttributeProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            sal_Bool bHintUsed(sal_False);

            SfxStyleSheetHint *pStyleHint = PTR_CAST(SfxStyleSheetHint, &rHint);

            if(pStyleHint && pStyleHint->GetStyleSheet() == GetStyleSheet())
            {
                SdrObject& rObj = GetSdrObject();
                

                switch(pStyleHint->GetHint())
                {
                    case SFX_STYLESHEET_CREATED         :
                    {
                        
                        break;
                    }
                    case SFX_STYLESHEET_MODIFIED        :
                    case SFX_STYLESHEET_CHANGED         :
                    {
                        
                        break;
                    }
                    case SFX_STYLESHEET_ERASED          :
                    case SFX_STYLESHEET_INDESTRUCTION   :
                    {
                        
                        SfxStyleSheet* pNewStSh = 0L;
                        SdrModel* pModel = rObj.GetModel();

                        
                        
                        
                        
                        if(pModel && !rObj.IsInDestruction())
                        {
                            if(HAS_BASE(SfxStyleSheet, GetStyleSheet()))
                            {
                                pNewStSh = (SfxStyleSheet*)pModel->GetStyleSheetPool()->Find(
                                    GetStyleSheet()->GetParent(), GetStyleSheet()->GetFamily());
                            }

                            if(!pNewStSh)
                            {
                                pNewStSh = pModel->GetDefaultStyleSheet();
                            }
                        }

                        
                        ImpRemoveStyleSheet();

                        if(pNewStSh)
                        {
                            ImpAddStyleSheet(pNewStSh, sal_True);
                        }

                        break;
                    }
                }

                
                
                Rectangle aBoundRect = rObj.GetLastBoundRect();

                rObj.SetRectsDirty(sal_True);

                
                rObj.SetChanged();
                rObj.BroadcastObjectChange();

                
                
                
                

                rObj.SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect);

                bHintUsed = sal_True;
            }

            if(!bHintUsed)
            {
                
                
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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
