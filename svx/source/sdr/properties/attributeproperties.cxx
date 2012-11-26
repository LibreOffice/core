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
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void AttributeProperties::ImpSetParentAtSfxItemSet(bool bDontRemoveHardAttr)
        {
            if(mpItemSet && mpStyleSheet)
            {
                // Delete hard attributes where items are set in the style sheet
                if(!bDontRemoveHardAttr)
                {
                    const SfxItemSet& rStyle = mpStyleSheet->GetItemSet();
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
                mpItemSet->SetParent(&mpStyleSheet->GetItemSet());
            }
            else
            {
                OSL_ENSURE(false, "ImpSetParentAtSfxItemSet called without SfxItemSet/SfxStyleSheet (!)");
            }
        }

        void AttributeProperties::ImpAddStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // test if old StyleSheet is cleared
            if(GetStyleSheet())
            {
                OSL_ENSURE(false, "Old style sheet not deleted before setting new one (!)");
                ImpRemoveStyleSheet();
            }

            if(pNewStyleSheet)
            {
                // remember StyleSheet
                mpStyleSheet = pNewStyleSheet;

                // register as listener
                StartListening(pNewStyleSheet->GetPool());
                StartListening(*pNewStyleSheet);

                // only apply the following when we have an SfxItemSet already, else
                if(mpItemSet && GetStyleSheet())
                {
                    ImpSetParentAtSfxItemSet(bDontRemoveHardAttr);
                }
            }
        }

        void AttributeProperties::ImpRemoveStyleSheet()
        {
            if(GetStyleSheet())
            {
                // deregister as listener
                EndListening(*mpStyleSheet);
                EndListening(mpStyleSheet->GetPool());

                // reset parent of ItemSet
                if(mpItemSet)
                {
                    mpItemSet->SetParent(0);
                }

                // trigger change at owning SdrObject
                GetSdrObject().ActionChanged();
            }

            mpStyleSheet = 0;
        }

        void AttributeProperties::ImpModelChange(SdrModel& rSourceModel, SdrModel& rTargetModel)
        {
            // If metric has changed, scale items.
            const MapUnit aOldUnit(rSourceModel.GetExchangeObjectUnit());
            const MapUnit aNewUnit(rTargetModel.GetExchangeObjectUnit());
            const bool bScaleUnitChanged(aNewUnit != aOldUnit);
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
                SfxStyleSheetBasePool* pOldPool = rSourceModel.GetStyleSheetPool();
                SfxStyleSheetBasePool* pNewPool = rTargetModel.GetStyleSheetPool();

                if(pOldPool && pNewPool)
                {
                    // build a list of to-be-copied Styles
                    std::vector< SfxStyleSheetBase* > aStyleSheetStack;
                    SfxStyleSheetBase* pAnchor = 0;

                    while(pSheet)
                    {
                        pAnchor = pNewPool->Find(pSheet->GetName(), pSheet->GetFamily());

                        if(!pAnchor)
                        {
                            aStyleSheetStack.push_back(pSheet);
                            pSheet = pOldPool->Find(pSheet->GetParent(), pSheet->GetFamily());
                        }
                        else
                        {
                            // the style does exist
                            pSheet = 0L;
                        }
                    }

                    // copy and set the parents
                    SfxStyleSheetBase* pNewSheet = 0;
                    SfxStyleSheetBase* pLastSheet = 0;
                    SfxStyleSheetBase* pForThisObject = 0;

                    for(sal_uInt32 a(0); a < aStyleSheetStack.size(); a++)
                    {
                        pSheet = aStyleSheetStack[a];
                        pNewSheet = &pNewPool->Make(pSheet->GetName(), pSheet->GetFamily(), pSheet->GetMask());
                        pNewSheet->GetItemSet().Put(pSheet->GetItemSet(), sal_False);

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
                        ImpAddStyleSheet((SfxStyleSheet*)pForThisObject, true);
                    }
                }
                else
                {
                    // there is no StyleSheetPool in the new model, thus set
                    // all items as hard items in the object
                    std::vector< const SfxItemSet* > aItemSetStack;
                    const SfxItemSet* pItemSet = &pOldStyleSheet->GetItemSet();

                    while(pItemSet)
                    {
                        aItemSetStack.push_back(pItemSet);
                        pItemSet = pItemSet->GetParent();
                    }

                    SfxItemSet* pNewSet = &CreateObjectSpecificItemSet(rTargetModel.GetItemPool());

                    for(sal_uInt32 a(0); a < aItemSetStack.size(); a++)
                    {
                        pItemSet = aItemSetStack[aItemSetStack.size() - 1 - a];
                        pNewSet->Put(*pItemSet);
                    }

                    // Items which were hard attributes before need to stay
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
                        mpItemSet = 0;
                    }

                    mpItemSet = pNewSet;
                }
            }
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
            mpStyleSheet(0)
        {
            // use correct default stylesheet #119287#
            SfxStyleSheet* pTargetStyleSheet = 0;
            const SdrGrafObj* pIsSdrGrafObj = dynamic_cast< const SdrGrafObj* >(&GetSdrObject());
            const SdrOle2Obj* pIsSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(&GetSdrObject());

            if(pIsSdrGrafObj || pIsSdrOle2Obj)
            {
                pTargetStyleSheet = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();
            }
            else
            {
                pTargetStyleSheet = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheet();
            }

            if(pTargetStyleSheet)
            {
                ImpAddStyleSheet(pTargetStyleSheet, true);
            }
        }

        AttributeProperties::AttributeProperties(const AttributeProperties& rProps, SdrObject& rObj)
        :   DefaultProperties(rProps, rObj),
            mpStyleSheet(0)
        {
            SfxStyleSheet* pTargetStyleSheet = rProps.GetStyleSheet();

            if(pTargetStyleSheet)
            {
                if(&rObj.getSdrModelFromSdrObject() != &GetSdrObject().getSdrModelFromSdrObject())
                {
                    // It is a clone to another model
                    ImpModelChange(rObj.getSdrModelFromSdrObject(), GetSdrObject().getSdrModelFromSdrObject());
                }
            }

            if(!pTargetStyleSheet)
            {
                // use correct default stylesheet #119287#
                // this should not happen, all SDrObjects should have a default StyleSheet, so cloning
                // one without is an error. Still add one, but assert user
                OSL_ENSURE(false, "Cloning SdrObject without SfxStyleSheet, all should have a default StyleSheet (!)");
                const SdrGrafObj* pIsSdrGrafObj = dynamic_cast< const SdrGrafObj* >(&GetSdrObject());
                const SdrOle2Obj* pIsSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(&GetSdrObject());

                if(pIsSdrGrafObj || pIsSdrOle2Obj)
                {
                    pTargetStyleSheet = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();
                }
                else
                {
                    pTargetStyleSheet = GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheet();
                }
            }

            if(pTargetStyleSheet)
            {
                ImpAddStyleSheet(pTargetStyleSheet, true);
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

        const SfxItemSet& AttributeProperties::GetObjectItemSet() const
        {
            // call parent
            DefaultProperties::GetObjectItemSet();

            // Late-Init of setting parent to SfxStyleSheet after it's creation, same as in constructor
            if(GetStyleSheet() && !mpItemSet->GetParent())
            {
                const_cast< AttributeProperties* >(this)->ImpSetParentAtSfxItemSet(true);
            }

            return *mpItemSet;
        }

        void AttributeProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            // own modifications
            SdrObject& rObj = GetSdrObject();

            rObj.SetChanged();
        }

        void AttributeProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            if(pNewItem)
            {
                const SfxPoolItem* pItem = pNewItem;

                switch( nWhich )
                {
                    case XATTR_FILLBITMAP:
                    {
                        pItem = ((XFillBitmapItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_LINEDASH:
                    {
                        pItem = ((XLineDashItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_LINESTART:
                    {
                        pItem = ((XLineStartItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_LINEEND:
                    {
                        pItem = ((XLineEndItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_FILLGRADIENT:
                    {
                        pItem = ((XFillGradientItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_FILLFLOATTRANSPARENCE:
                    {
                        // #85953# allow all kinds of XFillFloatTransparenceItem to be set
                        pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
                        break;
                    }
                    case XATTR_FILLHATCH:
                    {
                        pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( &GetSdrObject().getSdrModelFromSdrObject() );
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

        void AttributeProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            ImpRemoveStyleSheet();
            ImpAddStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            SdrObject& rObj = GetSdrObject();
            rObj.ActionChanged();
        }

        SfxStyleSheet* AttributeProperties::GetStyleSheet() const
        {
            return mpStyleSheet;
        }

        void AttributeProperties::ForceStyleToHardAttributes()
        {
            if(GetStyleSheet() && dynamic_cast< SfxStyleSheet* >(mpStyleSheet))
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
                GetSdrObject().ActionChanged();

                mpStyleSheet = NULL;
            }
        }

        void AttributeProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            bool bHintUsed(sal_False);

            const SfxStyleSheetHint *pStyleHint = dynamic_cast< const SfxStyleSheetHint* >( &rHint);

            if(pStyleHint && pStyleHint->GetStyleSheet() == GetStyleSheet())
            {
                SdrObject& rObj = GetSdrObject();
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(rObj, HINT_OBJCHG_ATTR);

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

                        // #111111#
                        // Do nothing if object is in destruction, else a StyleSheet may be found from
                        // a StyleSheetPool which is just being deleted itself. and thus it would be fatal
                        // to register as listener to that new StyleSheet.
                        if(!rObj.getSdrModelFromSdrObject().IsInDestruction())
                        {
                            if(dynamic_cast< SfxStyleSheet* >(GetStyleSheet()))
                            {
                                pNewStSh = (SfxStyleSheet*)rObj.getSdrModelFromSdrObject().GetStyleSheetPool()->Find(
                                    GetStyleSheet()->GetParent(), GetStyleSheet()->GetFamily());
                            }

                            if(!pNewStSh)
                            {
                                pNewStSh = rObj.getSdrModelFromSdrObject().GetDefaultStyleSheet();
                            }
                        }

                        // remove used style, it's erased or in destruction
                        ImpRemoveStyleSheet();

                        if(pNewStSh)
                        {
                            ImpAddStyleSheet(pNewStSh, true);
                        }

                        break;
                    }
                }

                // Get old BoundRect. Do this after the style change is handled
                // in the ItemSet parts because GetBoundRect() may calculate a new
                rObj.SetChanged();

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

//////////////////////////////////////////////////////////////////////////////
// eof
