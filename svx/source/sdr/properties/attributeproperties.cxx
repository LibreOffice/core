/*************************************************************************
 *
 *  $RCSfile: attributeproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 14:31:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDR_PROPERTIES_ATTRIBUTEPROPERTIES_HXX
#include <svx/sdr/properties/attributeproperties.hxx>
#endif

#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#include <svx/sdr/properties/itemsettools.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _SVX_XIT_HXX
#include <xit.hxx>
#endif

#ifndef _SVX_XBTMPIT_HXX
#include <xbtmpit.hxx>
#endif

#ifndef _SVX_XLNDSIT_HXX
#include <xlndsit.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX
#include <xlnstit.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX
#include <xlnedit.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX
#include <xflgrit.hxx>
#endif

#ifndef _SVX_XFLFTRIT_HXX
#include <xflftrit.hxx>
#endif

#ifndef _SVX_XFLHTIT_HXX
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XLNASIT_HXX
#include <xlnasit.hxx>
#endif

#ifndef _SVX_XFLASIT_HXX
#include <xflasit.hxx>
#endif

#ifndef _SVX_XTXASIT_HXX
#include <xtxasit.hxx>
#endif

#ifndef SDSHSITM_HXX
#include <sdshsitm.hxx>
#endif

#ifndef _SDOLSITM_HXX
#include <sdolsitm.hxx>
#endif

#ifndef _SDMSITM_HXX
#include <sdmsitm.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

#ifndef _SVDTRANS_HXX
#include <svdtrans.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

// #114265#
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

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
                SDRATTR_START, SDRATTRSET_SHADOW,
                SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
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

        void AttributeProperties::ItemSetChanged(const SfxItemSet& rSet)
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

        void AttributeProperties::PreProcessSave()
        {
            // call parent
            DefaultProperties::PreProcessSave();

            // force ItemSet
            GetObjectItemSet();

            // prepare SetItems for storage
            const SfxItemSet& rSet = *mpItemSet;
            const SfxItemSet* pParent = mpStyleSheet ? &(mpStyleSheet->GetItemSet()) : 0L;

            XLineAttrSetItem aLineAttr(rSet.GetPool());
            aLineAttr.GetItemSet().Put(rSet);
            aLineAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aLineAttr);

            XFillAttrSetItem aFillAttr(rSet.GetPool());
            aFillAttr.GetItemSet().Put(rSet);
            aFillAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aFillAttr);

            XTextAttrSetItem aTextAttr(rSet.GetPool());
            aTextAttr.GetItemSet().Put(rSet);
            aTextAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aTextAttr);

            SdrShadowSetItem aShadAttr(rSet.GetPool());
            aShadAttr.GetItemSet().Put(rSet);
            aShadAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aShadAttr);

            SdrOutlinerSetItem aOutlAttr(rSet.GetPool());
            aOutlAttr.GetItemSet().Put(rSet);
            aOutlAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aOutlAttr);

            SdrMiscSetItem aMiscAttr(rSet.GetPool());
            aMiscAttr.GetItemSet().Put(rSet);
            aMiscAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aMiscAttr);
        }

        void AttributeProperties::PostProcessSave()
        {
            // call parent
            DefaultProperties::PostProcessSave();

            // remove SetItems from local itemset
            if(mpItemSet)
            {
                mpItemSet->ClearItem(XATTRSET_LINE);
                mpItemSet->ClearItem(XATTRSET_FILL);
                mpItemSet->ClearItem(XATTRSET_TEXT);
                mpItemSet->ClearItem(SDRATTRSET_SHADOW);
                mpItemSet->ClearItem(SDRATTRSET_OUTLINER);
                mpItemSet->ClearItem(SDRATTRSET_MISC);
            }
        }

        void AttributeProperties::MoveToItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
        {
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

                    mpItemSet = mpItemSet->Clone(FALSE, pDestPool);
                    GetSdrObject().GetModel()->MigrateItemSet(pOldSet, mpItemSet, pNewModel);

                    // set stylesheet (if used)
                    if(pStySheet)
                    {
                        ImpAddStyleSheet(pStySheet, sal_True);
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
                            List aList;
                            SfxStyleSheetBase* pAnchor = 0L;

                            while(pSheet)
                            {
                                pAnchor = pNewPool->Find(pSheet->GetName(), pSheet->GetFamily());

                                if(!pAnchor)
                                {
                                    aList.Insert(pSheet, LIST_APPEND);
                                    pSheet = pOldPool->Find(pSheet->GetParent(), pSheet->GetFamily());
                                }
                                else
                                {
                                    // the style does exist
                                    pSheet = 0L;
                                }
                            }

                            // copy and set the parents
                            pSheet = (SfxStyleSheetBase*)aList.First();
                            SfxStyleSheetBase* pNewSheet = 0L;
                            SfxStyleSheetBase* pLastSheet = 0L;
                            SfxStyleSheetBase* pForThisObject = 0L;

                            while(pSheet)
                            {
                                pNewSheet = &pNewPool->Make(pSheet->GetName(), pSheet->GetFamily(), pSheet->GetMask());
                                pNewSheet->GetItemSet().Put(pSheet->GetItemSet(), FALSE);

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
                                pSheet = (SfxStyleSheetBase*)aList.Next();
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
                            List aList;
                            const SfxItemSet* pItemSet = &pOldStyleSheet->GetItemSet();

                            while(pItemSet)
                            {
                                aList.Insert((void*)pItemSet, CONTAINER_APPEND);
                                pItemSet = pItemSet->GetParent();
                            }

                            SfxItemSet* pNewSet = &CreateObjectSpecificItemSet(pNewModel->GetItemPool());
                            pItemSet = (SfxItemSet*)aList.Last();

                            while(pItemSet)
                            {
                                pNewSet->Put(*pItemSet);
                                pItemSet = (SfxItemSet*)aList.Prev();
                            }

                            // Items which were hard attributes before need to stay
                            if(mpItemSet)
                            {
                                SfxWhichIter aIter(*mpItemSet);
                                sal_uInt16 nWhich = aIter.FirstWhich();

                                while(nWhich)
                                {
                                    if(mpItemSet->GetItemState(nWhich, FALSE) == SFX_ITEM_SET)
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
                if(mpItemSet && !GetStyleSheet() && pNewModel && !pNewModel->IsLoading())
                {
                    SetStyleSheet(pNewModel->GetDefaultStyleSheet(), sal_True);
                }
            }
        }

        void AttributeProperties::ForceStyleToHardAttributes(sal_Bool bPseudoSheetsOnly)
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

                // get itemset of the stylesheet
                const SfxItemSet& rSet = mpStyleSheet->GetItemSet();

                // prepare the iter; use the mpObjectItemSet which may have less
                // WhichIDs than the style.
                SfxWhichIter aIter(*pDestItemSet);
                sal_uInt16 nWhich(aIter.FirstWhich());
                const SfxPoolItem *pItem = NULL;

                // set all attributes of the stylesheet at the new itemset
                while(nWhich)
                {
                    if(SFX_ITEM_SET == rSet.GetItemState(nWhich, TRUE, &pItem))
                    {
                        pDestItemSet->Put(*pItem);
                    }

                    nWhich = aIter.NextWhich();
                }

                // prepare 2nd loop
                nWhich = aIter.FirstWhich();

                // now set all hard attributes of the current at the new itemset
                while(nWhich)
                {
                    if(SFX_ITEM_SET == mpItemSet->GetItemState(nWhich, FALSE, &pItem))
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
                SdrPage* pPage = rObj.GetPage();

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

            // #111111#
            // When it's the BackgroundObject, set the MasterPage to changed to
            // get a refresh for the evtl. changed BackgroundStyle

            // #114265#
            // To only invalidate the page when the StyleSheet change happens,
            // some more rigid testing is necessary.
            const SfxSimpleHint *pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);

            if(pSimpleHint
                && pSimpleHint->GetId() == SFX_HINT_DATACHANGED
                && GetSdrObject().IsMasterPageBackgroundObject())
            {
                GetSdrObject().GetPage()->ActionChanged();
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
