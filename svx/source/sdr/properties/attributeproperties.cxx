/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// #114265#
#include <svl/smplhint.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void AttributeProperties::ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            // test if old StyleSheet is cleared, else it would be lost
            // after this method -> memory leak (!)
            DBG_ASSERT(!mpStyleSheet, "Old style sheet not deleted before setting new one (!)");

            if(pNewStyleSheet)
            {
                mpStyleSheet = pNewStyleSheet;

                // local ItemSet is needed here, force it
                GetObjectItemSet();

                // register as listener
                StartListening(pNewStyleSheet->GetPool());
                StartListening(*pNewStyleSheet);

                // Delete hard attributes where items are set in the style sheet
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

                // set new stylesheet as parent
                mpItemSet->SetParent(&pNewStyleSheet->GetItemSet());
            }
        }

        void AttributeProperties::ImpRemoveStyleSheet()
        {
            // Check type since it is destroyed when the type is deleted
            if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
            {
                EndListening(*mpStyleSheet);
                EndListening(mpStyleSheet->GetPool());

                // reset parent of ItemSet
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

        // create a new itemset
        SfxItemSet& AttributeProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // ranges from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // end
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
            // own modifications
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
                        // #85953# allow all kinds of XFillFloatTransparenceItem to be set
                        pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                    case XATTR_FILLHATCH:
                    {
                        pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pModel );
                        break;
                    }
                }

                // set item
                if(pItem)
                {
                    // force ItemSet
                    GetObjectItemSet();
                    mpItemSet->Put(*pItem);

                    // delete item if it was a generated one
                    if(pItem != pNewItem)
                    {
                        delete (SfxPoolItem*)pItem;
                    }
                }
            }
            else
            {
                // clear item if ItemSet exists
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
                    // migrate ItemSet to new pool. Scaling is NOT necessary
                    // because this functionality is used by UNDO only. Thus
                    // objects and ItemSets would be moved back to their original
                    // pool before usage.
                    SfxItemSet* pOldSet = mpItemSet;
                    SfxStyleSheet* pStySheet = GetStyleSheet();

                    if(pStySheet)
                    {
                        ImpRemoveStyleSheet();
                    }

                    mpItemSet = mpItemSet->Clone(sal_False, pDestPool);
                    GetSdrObject().GetModel()->MigrateItemSet(pOldSet, mpItemSet, pNewModel);

                    // set stylesheet (if used)
                    if(pStySheet)
                    {
                        // #i109515#
                        SfxItemPool* pStyleSheetPool = &pStySheet->GetPool().GetPool();

                        if(pStyleSheetPool == pDestPool)
                        {
                            // just re-set stylesheet
                            ImpAddStyleSheet(pStySheet, sal_True);
                        }
                        else
                        {
                            // StyleSheet is NOT from the correct pool.
                            // Look one up in the right pool with the same
                            // name or use the default.

                            // Look up the style in the new document.
                            OSL_ASSERT(pNewModel->GetStyleSheetPool() != NULL);
                            SfxStyleSheet* pNewStyleSheet = dynamic_cast<SfxStyleSheet*>(
                                pNewModel->GetStyleSheetPool()->Find(
                                    pStySheet->GetName(),
                                    SFX_STYLE_FAMILY_ALL));
                            if (pNewStyleSheet == NULL
                                || &pNewStyleSheet->GetPool().GetPool() != pDestPool)
                            {
                                // There is no copy of the style in the new
                                // document.  Use the default as a fallback.
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
                // For a living model move the items from one pool to the other
                if(pOldModel)
                {
                    // If metric has changed, scale items.
                    MapUnit aOldUnit(pOldModel->GetScaleUnit());
                    MapUnit aNewUnit(pNewModel->GetScaleUnit());
                    sal_Bool bScaleUnitChanged(aNewUnit != aOldUnit);
                    Fraction aMetricFactor;

                    if(bScaleUnitChanged)
                    {
                        aMetricFactor = GetMapFactor(aOldUnit, aNewUnit).X();
                        Scale(aMetricFactor);
                    }

                    // Move all styles which are used by the object to the new
                    // StyleSheet pool
                    SfxStyleSheet* pOldStyleSheet = GetStyleSheet();

                    if(pOldStyleSheet)
                    {
                        SfxStyleSheetBase* pSheet = pOldStyleSheet;
                        SfxStyleSheetBasePool* pOldPool = pOldModel->GetStyleSheetPool();
                        SfxStyleSheetBasePool* pNewPool = pNewModel->GetStyleSheetPool();
                        DBG_ASSERT(pOldPool, "Properties::SetModel(): Object has StyleSheet but no StyleSheetPool (!)");

                        if(pOldPool && pNewPool)
                        {
                            // build a list of to-be-copied Styles
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
                                    // the style does exist
                                    pSheet = 0L;
                                }
                            }

                            // copy and set the parents
                            SfxStyleSheetBase* pNewSheet = 0L;
                            SfxStyleSheetBase* pLastSheet = 0L;
                            SfxStyleSheetBase* pForThisObject = 0L;

                            std::vector<SfxStyleSheetBase*>::iterator iter;
                            for (iter = aStyleList.begin(); iter != aStyleList.end(); ++iter)
                            {
                                pNewSheet = &pNewPool->Make((*iter)->GetName(), (*iter)->GetFamily(), (*iter)->GetMask());
                                pNewSheet->GetItemSet().Put((*iter)->GetItemSet(), sal_False);

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

                            // Set link to the Style found in the Pool
                            if(pAnchor && pLastSheet)
                            {
                                pLastSheet->SetParent(pAnchor->GetName());
                            }

                            // if list was empty (all Styles exist in destination pool)
                            // pForThisObject is not yet set
                            if(!pForThisObject && pAnchor)
                            {
                                pForThisObject = pAnchor;
                            }

                            // De-register at old and register at new Style
                            if(GetStyleSheet() != pForThisObject)
                            {
                                ImpRemoveStyleSheet();
                                ImpAddStyleSheet((SfxStyleSheet*)pForThisObject, sal_True);
                            }
                        }
                        else
                        {
                            // there is no StyleSheetPool in the new model, thus set
                            // all items as hard items in the object
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

                            // Items which were hard attributes before need to stay
                            if(mpItemSet)
                            {
                                SfxWhichIter aIter(*mpItemSet);
                                sal_uInt16 nWhich = aIter.FirstWhich();

                                while(nWhich)
                                {
                                    if(mpItemSet->GetItemState(nWhich, sal_False) == SFX_ITEM_SET)
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

                // each object gets the default Style if there is none set yet.
                if(!GetStyleSheet() && pNewModel && !pNewModel->IsLoading())
                {
                    GetObjectItemSet(); // i#118414 force ItemSet to allow style to be set
                    SetStyleSheet(pNewModel->GetDefaultStyleSheet(), sal_True);
                }
            }
        }

        void AttributeProperties::ForceStyleToHardAttributes()
        {
            if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
            {
                // prepare copied, new itemset, but WITHOUT parent
                GetObjectItemSet();
                SfxItemSet* pDestItemSet = new SfxItemSet(*mpItemSet);
                pDestItemSet->SetParent(0L);

                // pepare forgetting the current stylesheet like in RemoveStyleSheet()
                EndListening(*mpStyleSheet);
                EndListening(mpStyleSheet->GetPool());

                // prepare the iter; use the mpObjectItemSet which may have less
                // WhichIDs than the style.
                SfxWhichIter aIter(*pDestItemSet);
                sal_uInt16 nWhich(aIter.FirstWhich());
                const SfxPoolItem *pItem = NULL;

                // now set all hard attributes of the current at the new itemset
                while(nWhich)
                {
                    // #i61284# use mpItemSet with parents, makes things easier and reduces to
                    // one loop
                    if(SFX_ITEM_SET == mpItemSet->GetItemState(nWhich, true, &pItem))
                    {
                        pDestItemSet->Put(*pItem);
                    }

                    nWhich = aIter.NextWhich();
                }

                // replace itemsets
                delete mpItemSet;
                mpItemSet = pDestItemSet;

                // set necessary changes like in RemoveStyleSheet()
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
                //SdrPage* pPage = rObj.GetPage();

                switch(pStyleHint->GetHint())
                {
                    case SFX_STYLESHEET_CREATED         :
                    {
                        // cannot happen, nothing to do
                        break;
                    }
                    case SFX_STYLESHEET_MODIFIED        :
                    case SFX_STYLESHEET_CHANGED         :
                    {
                        // notify change
                        break;
                    }
                    case SFX_STYLESHEET_ERASED          :
                    case SFX_STYLESHEET_INDESTRUCTION   :
                    {
                        // Style needs to be exchanged
                        SfxStyleSheet* pNewStSh = 0L;
                        SdrModel* pModel = rObj.GetModel();

                        // #111111#
                        // Do nothing if object is in destruction, else a StyleSheet may be found from
                        // a StyleSheetPool which is just being deleted itself. and thus it would be fatal
                        // to register as listener to that new StyleSheet.
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

                        // remove used style, it's erased or in destruction
                        ImpRemoveStyleSheet();

                        if(pNewStSh)
                        {
                            ImpAddStyleSheet(pNewStSh, sal_True);
                        }

                        break;
                    }
                }

                // Get old BoundRect. Do this after the style change is handled
                // in the ItemSet parts because GetBoundRect() may calculate a new
                Rectangle aBoundRect = rObj.GetLastBoundRect();

                rObj.SetRectsDirty(sal_True);

                // tell the object about the change
                rObj.SetChanged();
                rObj.BroadcastObjectChange();

                //if(pPage && pPage->IsInserted())
                //{
                //  rObj.BroadcastObjectChange();
                //}

                rObj.SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect);

                bHintUsed = sal_True;
            }

            if(!bHintUsed)
            {
                // forward to SdrObject ATM. Not sure if this will be necessary
                // in the future.
                GetSdrObject().Notify(rBC, rHint);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
