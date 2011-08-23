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

#include "xpool.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdpool.hxx"
#include "svdocapt.hxx"

#include <bf_svtools/smplhint.hxx>

#include "xlnstwit.hxx"

#include "xlnedwit.hxx"

#include <eeitemid.hxx>

#include "eeitem.hxx"

#include <xlnstcit.hxx>

#include <xlnwtit.hxx>


#include <bf_svtools/style.hxx>

#include <bf_svtools/whiter.hxx>


#include <xflclit.hxx>


#include <xfltrit.hxx>

#include <xlnedcit.hxx>

#include <adjitem.hxx>

#include "xflbckit.hxx"


#include "xbtmpit.hxx"
#include "xlndsit.hxx"
#include "xlnedit.hxx"
#include "xflftrit.hxx"
#include "xflhtit.hxx"
#include "xlnstit.hxx"
#include "xoutx.hxx"
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrAttrObj,SdrObject);

/*N*/ SdrAttrObj::SdrAttrObj()
/*N*/ :	mpStyleSheet(NULL),
/*N*/ 	mpObjectItemSet(NULL)
/*N*/ {
/*N*/ }

/*N*/ SdrAttrObj::~SdrAttrObj()
/*N*/ {
/*N*/ 	ImpDeleteItemSet();
/*N*/ }

/*N*/ void SdrAttrObj::ImpDeleteItemSet()
/*N*/ {
/*N*/ 	if(mpObjectItemSet)
/*N*/ 	{
/*N*/ 		if(GetStyleSheet())
/*N*/ 			RemoveStyleSheet();
/*N*/ 
/*N*/ 		delete mpObjectItemSet;
/*N*/ 		mpObjectItemSet = 0L;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrAttrObj::ImpForceItemSet()
/*N*/ {
/*N*/ 	if(!mpObjectItemSet)
/*N*/ 	{
/*N*/ 		SfxItemPool* pPool = GetItemPool();
/*N*/ 		mpObjectItemSet = CreateNewItemSet(*pPool);
/*N*/ 		ForceDefaultAttr();
/*N*/ 	}
/*N*/ }

/*N*/ const Rectangle& SdrAttrObj::GetSnapRect() const
/*N*/ {
/*N*/ 	if(bSnapRectDirty)
/*N*/ 	{
/*N*/ 		((SdrAttrObj*)this)->RecalcSnapRect();
/*N*/ 		((SdrAttrObj*)this)->bSnapRectDirty = FALSE;
/*N*/ 	}
/*N*/ 	return maSnapRect;
/*N*/ }

/*N*/ void SdrAttrObj::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrObject::operator=(rObj);
/*N*/ 
/*N*/ 	SdrAttrObj* pAO = PTR_CAST(SdrAttrObj, (SdrObject*)&rObj);
/*N*/ 	if(pAO)
/*N*/ 	{
/*N*/ 		ImpDeleteItemSet();
/*N*/ 
/*N*/ 		if(pAO->mpObjectItemSet)
/*N*/ 			mpObjectItemSet = pAO->mpObjectItemSet->Clone(TRUE);
/*N*/ 
/*N*/ 		if(pAO->GetStyleSheet())
/*?*/ 			AddStyleSheet(pAO->GetStyleSheet(), TRUE);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrAttrObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if(rIn.GetError())
/*N*/ 		return;
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrObject::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrAttrObj");
/*N*/ #endif
/*N*/ 	SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		sal_uInt16 nSetID;
/*N*/ 
/*N*/ 		// #89025# if mpObjectItemSet is set and contains items, it is because of ForceDefaultAttr()
/*N*/ 		// and the items need to be deleted.
/*N*/ 		if(mpObjectItemSet && mpObjectItemSet->Count())
/*N*/ 			mpObjectItemSet->ClearItem(0);
/*N*/ 
/*N*/ 		// Do this initialization AFTER the above fix
/*N*/ 		SfxItemSet aNewSet(GetItemSet());
/*N*/ 
/*N*/ 		if(rHead.GetVersion() < 11)
/*N*/ 			{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 		nSetID = XATTRSET_LINE;
/*N*/ 		const XLineAttrSetItem* pLineAttr = (const XLineAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pLineAttr)
/*N*/ 			aNewSet.Put(pLineAttr->GetItemSet());
/*N*/ 
/*N*/ 		if(rHead.GetVersion() < 11)
/*N*/ 			{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 		nSetID = XATTRSET_FILL;
/*N*/ 		const XFillAttrSetItem* pFillAttr = (const XFillAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pFillAttr)
/*N*/ 			aNewSet.Put(pFillAttr->GetItemSet());
/*N*/ 
/*N*/ 		if(rHead.GetVersion() < 11)
/*N*/ 			{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 		nSetID = XATTRSET_TEXT;
/*N*/ 		const XTextAttrSetItem* pTextAttr = (const XTextAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pTextAttr)
/*N*/ 			aNewSet.Put(pTextAttr->GetItemSet());
/*N*/ 
/*N*/ 		if(rHead.GetVersion() < 11)
/*N*/ 			{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 		nSetID = SDRATTRSET_SHADOW;
/*N*/ 		const SdrShadowSetItem* pShadAttr = (const SdrShadowSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pShadAttr)
/*N*/ 			aNewSet.Put(pShadAttr->GetItemSet());
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 5)
/*N*/ 		{
/*N*/ 			if(rHead.GetVersion() < 11)
/*N*/ 				{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 			nSetID = SDRATTRSET_OUTLINER;
/*N*/ 			const SdrOutlinerSetItem* pOutlAttr = (const SdrOutlinerSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 			if(pOutlAttr)
/*N*/ 				aNewSet.Put(pOutlAttr->GetItemSet());
/*N*/ 		}
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 6)
/*N*/ 		{
/*N*/ 			if(rHead.GetVersion() < 11)
/*N*/ 				{ sal_uInt16 nWhichDum; rIn >> nWhichDum; }
/*N*/ 			nSetID = SDRATTRSET_MISC;
/*N*/ 			const SdrMiscSetItem* pMiscAttr = (const SdrMiscSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 			if(pMiscAttr)
/*N*/ 				aNewSet.Put(pMiscAttr->GetItemSet());
/*N*/ 		}
/*N*/ 
/*N*/ 		SetItemSet(aNewSet);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// an den Surrogaten und ggf. auch Whiches vorbeiseeken
/*N*/ 		// ganz zu anfang waren es 4 SetItems
/*N*/ 		sal_uInt16 nAnz(4);
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 5)
/*N*/ 			nAnz++;
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 6)
/*N*/ 			nAnz++;
/*N*/ 
/*N*/ 		nAnz *= sizeof(sal_uInt16);
/*N*/ 
/*N*/ 		if(rHead.GetVersion() < 11)
/*N*/ 			nAnz *= 2;
/*N*/ 
/*N*/ 		rIn.SeekRel(nAnz);
/*N*/ 	}
/*N*/ 
/*N*/ 	// TextToContour: altes Format(Flag) in neues Format(Item) wandeln
/*N*/ 	if(rHead.GetVersion() <= 4 && pPool)
/*N*/ 	{
/*?*/ 		SetItem(XFormTextStyleItem(XFT_NONE));
/*N*/ 	}
/*N*/ 
    // Fuer die StyleSheetgeschichte gehoert eigentlich auch noch eine
    // Versionsabfrage hierher.
    // Name und Familie des StyleSheet einlesen, in Pointer auf StyleSheet
    // umwandeln lassen (SB)
/*N*/ 	XubString aStyleSheetName;
/*N*/ 	SfxStyleFamily eFamily;
/*N*/ 	sal_uInt16 nRead;
/*N*/ 
/*N*/ 	// UNICODE: rIn>>aStyleSheetName;
/*N*/ 	rIn.ReadByteString(aStyleSheetName);
/*N*/ 
/*N*/ 	if(aStyleSheetName.Len())
/*N*/ 	{
/*N*/ 		rIn >> nRead;
/*N*/ 		eFamily = (SfxStyleFamily)(int)nRead;
/*N*/ 
/*N*/ 		// ab Version 1 wird der CharacterSet gelesen, ab V11 nicht mehr
/*N*/ 		if(rHead.GetVersion() > 0 && rHead.GetVersion() < 11)
/*N*/ 		{
/*N*/ 			sal_Int16 nCharSet;
/*N*/ 			rIn >> nCharSet;
/*N*/ 			//aStyleSheetName.Convert((CharSet)nCharSet);
/*N*/ 			// nicht mehr noetig, da ab Vers 11 der CharSet bereits am
/*N*/ 			// Stream gesetzt wird.
/*N*/ 		}
/*N*/ 
/*N*/ 		DBG_ASSERT(pModel, "SdrAttrObj::ReadData(): pModel=NULL, StyleSheet kann nicht gesetzt werden!");
/*N*/ 		if(pModel)
/*N*/ 		{
/*N*/ 			SfxStyleSheetBasePool *pPool = pModel->GetStyleSheetPool();
/*N*/ 			if(pPool)
/*N*/ 			{
/*N*/ 				SfxStyleSheet *pTmpStyleSheet = (SfxStyleSheet*)pPool->Find(aStyleSheetName, eFamily);
/*N*/ 				DBG_ASSERT(pTmpStyleSheet, "SdrAttrObj::ReadData(): StyleSheet nicht gefunden");
/*N*/ 
/*N*/ 				if(pTmpStyleSheet)
/*N*/ 					AddStyleSheet(pTmpStyleSheet, TRUE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrAttrObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrObject::PreSave();
/*N*/ 
/*N*/ 	// prepare SetItems for storage
/*N*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*N*/ 
/*N*/ 	XLineAttrSetItem aLineAttr(rSet.GetPool());
/*N*/ 	aLineAttr.GetItemSet().Put(rSet);
/*N*/ 	aLineAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aLineAttr);
/*N*/ 
/*N*/ 	XFillAttrSetItem aFillAttr(rSet.GetPool());
/*N*/ 	aFillAttr.GetItemSet().Put(rSet);
/*N*/ 	aFillAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aFillAttr);
/*N*/ 
/*N*/ 	XTextAttrSetItem aTextAttr(rSet.GetPool());
/*N*/ 	aTextAttr.GetItemSet().Put(rSet);
/*N*/ 	aTextAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aTextAttr);
/*N*/ 
/*N*/ 	SdrShadowSetItem aShadAttr(rSet.GetPool());
/*N*/ 	aShadAttr.GetItemSet().Put(rSet);
/*N*/ 	aShadAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aShadAttr);
/*N*/ 
/*N*/ 	SdrOutlinerSetItem aOutlAttr(rSet.GetPool());
/*N*/ 	aOutlAttr.GetItemSet().Put(rSet);
/*N*/ 	aOutlAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aOutlAttr);
/*N*/ 
/*N*/ 	SdrMiscSetItem aMiscAttr(rSet.GetPool());
/*N*/ 	aMiscAttr.GetItemSet().Put(rSet);
/*N*/ 	aMiscAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aMiscAttr);
/*N*/ }

/*N*/ void SdrAttrObj::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrObject::PostSave();
/*N*/ 
/*N*/ 	// remove SetItems from local itemset
/*N*/ 	((SdrAttrObj*)this)->ImpForceItemSet();
/*N*/ 	mpObjectItemSet->ClearItem(XATTRSET_LINE);
/*N*/ 	mpObjectItemSet->ClearItem(XATTRSET_FILL);
/*N*/ 	mpObjectItemSet->ClearItem(XATTRSET_TEXT);
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_SHADOW);
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_OUTLINER);
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_MISC);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrAttrObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrObject::WriteData(rOut);
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrAttrObj");
/*N*/ #endif
/*N*/ 	SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_LINE));
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_FILL));
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_TEXT));
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_SHADOW));
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_OUTLINER));
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MISC));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*?*/ 		rOut << sal_uInt16(SFX_ITEMS_NULL);
/*N*/ 	}

    // StyleSheet-Pointer als Name, Familie abspeichern
    // wenn kein StyleSheet vorhanden: leeren String speichern
/*N*/ 	if(GetStyleSheet())
/*N*/ 	{
/*N*/ 		// UNICODE: rOut << pStyleSheet->GetName();
/*N*/ 		rOut.WriteByteString(GetStyleSheet()->GetName());
/*N*/ 		rOut << (sal_uInt16)(int)(GetStyleSheet()->GetFamily());
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// UNICODE: rOut << String();
/*N*/ 		rOut.WriteByteString(String());
/*N*/ 	}
/*N*/ }


/*N*/ void SdrAttrObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrModel* pOldModel = pModel;
/*N*/ 
/*N*/ 	// test for correct pool in ItemSet; move to new pool if necessary
/*N*/ 	if(pNewModel && mpObjectItemSet && mpObjectItemSet->GetPool() != &pNewModel->GetItemPool())
/*N*/ 		MigrateItemPool(mpObjectItemSet->GetPool(), &pNewModel->GetItemPool(), pNewModel);
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrObject::SetModel(pNewModel);
/*N*/ 
/*N*/ 	if(pOldModel != pNewModel && pNewModel && !pNewModel->IsLoading())
/*N*/ 	{
/*N*/ 
/*N*/ 		// fuer ein bereits "lebendes" Model die Attribute von einem Pool in den anderen schieben
/*N*/ 		if(pOldModel)
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 
/*N*/ 		// Jedes Objekt bekommt initial den DefaultStyleSheet
/*N*/ 		// des Model, falls noch kein StyleSheet gesetzt.
/*N*/ 		if(mpObjectItemSet && !GetStyleSheet() && pModel && !pModel->IsLoading())
/*N*/ 			NbcSetStyleSheet(pModel->GetDefaultStyleSheet(), TRUE);

/* this code was removed because NbcSetStyleSheet called with TRUE does not
   alter the hard attributes. So they don't need to be restored, a useless
   process that cost us up to 20% for xml import. Also there is a memory
   leek with aSet.Put( *pItem->Clone() );
        {
            SfxStyleSheet* pDefSS = pModel->GetDefaultStyleSheet();

            if(pDefSS)
            {
                SfxItemPool* pPool = GetItemPool();
                if ( pPool )
                {
                    // Take hard attributes
                    SfxItemSet aSet(*pPool,
                        SDRATTR_START,SDRATTR_NOTPERSIST_FIRST-1,
                        SDRATTR_NOTPERSIST_LAST+1, SDRATTR_END,
                        EE_ITEMS_START,EE_ITEMS_END,
                        0,0);

                    const SfxItemSet& rItemSet = GetItemSet();

                    SfxWhichIter aIter( rItemSet );
                    sal_uInt16 nWhich( aIter.FirstWhich() );
                    const SfxPoolItem* pItem = NULL;

                    while( nWhich )
                    {
                        if( SFX_ITEM_SET == rItemSet.GetItemState( nWhich, FALSE, &pItem ) )
                            aSet.Put( *pItem->Clone() );
                        nWhich = aIter.NextWhich();
                    }
                    // Set the StyleSheet
                    NbcSetStyleSheet(pDefSS, TRUE);

                    // Set the hard attributes
                    SetItemSet( aSet );
                  }
                else
                    NbcSetStyleSheet(pDefSS, TRUE);
            }
        }
*/
/*N*/ 	}
/*N*/ }

/*N*/ void SdrAttrObj::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrTextObj* pText = PTR_CAST(SdrTextObj, this);
/*N*/ 	BOOL bTextFrame(pText && pText->IsTextFrame());
/*N*/ 
/*N*/ 	ImpForceItemSet();
/*N*/ 	if(bTextFrame)
/*N*/ 	{
/*N*/ 		SdrCaptionObj* pCapt = PTR_CAST(SdrCaptionObj, this);
/*N*/ 		BOOL bCaption(pCapt != 0L);
/*N*/ 
/*N*/ 		if(!bCaption)
/*N*/ 			 mpObjectItemSet->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 
/*N*/ 		mpObjectItemSet->Put(XFillColorItem(String(), Color(COL_WHITE)));
/*N*/ 		mpObjectItemSet->Put(XFillStyleItem(XFILL_NONE));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		mpObjectItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));
/*N*/ 		mpObjectItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
/*N*/ 		mpObjectItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& SdrAttrObj::GetItemSet() const
/*N*/ {
/*N*/ 	((SdrAttrObj*)this)->ImpForceItemSet();
/*N*/ 	return *mpObjectItemSet;
/*N*/ }

/*N*/ SfxItemSet* SdrAttrObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses

/*N*/ const SfxItemSet& SdrAttrObj::GetUnmergedItemSet() const
/*N*/ {
/*N*/ 	return SdrAttrObj::GetItemSet();
/*N*/ }

/*N*/ void SdrAttrObj::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/ 	if(pNewItem)
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem = pNewItem;
/*N*/ 
/*N*/ 		switch( nWhich )
/*N*/ 		{
/*N*/ 		case XATTR_FILLBITMAP:
/*N*/ 			pItem = ((XFillBitmapItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_LINEDASH:
/*N*/ 			pItem = ((XLineDashItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_LINESTART:
/*N*/ 			pItem = ((XLineStartItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_LINEEND:
/*N*/ 			pItem = ((XLineEndItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_FILLGRADIENT:
/*N*/ 			pItem = ((XFillGradientItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_FILLFLOATTRANSPARENCE:
/*N*/ 			// #85953# allow all kinds of XFillFloatTransparenceItem to be set
/*N*/ 			pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		case XATTR_FILLHATCH:
/*N*/ 			pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pModel );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		// set item
/*N*/ 		if( pItem )
/*N*/ 		{
/*N*/ 			((SdrAttrObj*)this)->ImpForceItemSet();
/*N*/ 			mpObjectItemSet->Put(*pItem);
/*N*/ 
/*N*/ 			// delete item if it was a generated one
/*N*/ 			if( pItem != pNewItem)
/*N*/ 				delete (SfxPoolItem*)pItem;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// clear item
/*N*/ 		if(mpObjectItemSet)
/*N*/ 		{
/*N*/ 			mpObjectItemSet->ClearItem(nWhich);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrAttrObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrObject::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// own modifications
/*N*/ 	bBoundRectDirty = TRUE;
/*N*/ 	SetRectsDirty(TRUE);
/*N*/ 	SetChanged();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
/*N*/ 	const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint, &rHint);
/*N*/ 	BOOL bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());
/*N*/ 	SfxStyleSheetHint *pStyleHint = PTR_CAST(SfxStyleSheetHint, &rHint);
/*N*/ 	BOOL bDying(pStyleHint
/*N*/ 				&& pStyleHint->GetStyleSheet() == GetStyleSheet()
/*N*/ 				&& ( SFX_STYLESHEET_INDESTRUCTION == pStyleHint->GetHint() ||
/*N*/ 					 SFX_STYLESHEET_ERASED == pStyleHint->GetHint() ));
/*N*/ 
/*N*/ 	if(bDataChg || bDying)
/*N*/ 	{
/*N*/ 		Rectangle aBoundRect0;
/*N*/ 
/*N*/ 		if(pUserCall)
/*N*/ 			aBoundRect0 = GetBoundRect();
/*N*/ 
/*N*/ 		SfxStyleSheet* pNewStSh = NULL;
/*N*/ 		BOOL bBoundRectDirty0 = bBoundRectDirty;
/*N*/ 
/*N*/ 		if(bDying)
/*N*/ 		{
/*N*/ 			// wenn es einen Parent gibt, wird jetzt der genommen
/*N*/ 			if(pModel && HAS_BASE(SfxStyleSheet, GetStyleSheet()))
/*N*/ 			{
/*N*/ 				// Sonst ist pStyleSheet schon zu weit weggestorben
/*N*/ 				pNewStSh = (SfxStyleSheet*)pModel->GetStyleSheetPool()->Find(
/*N*/ 					GetStyleSheet()->GetParent(), GetStyleSheet()->GetFamily());
/*N*/ 			}
/*N*/ 
/*N*/ 			// wenn es keinen Parent gibt, nehmen wir die Standardvorlage
/*N*/ 			if(!pNewStSh && pModel)
/*N*/ 				pNewStSh = pModel->GetDefaultStyleSheet();
/*N*/ 
/*N*/ 			// alten StyleSheet vor SendRepaintBroadcast entfernen #42276#
/*N*/ 			RemoveStyleSheet(); // setzt bBoundRectDirty=TRUE
/*N*/ 		}
/*N*/ 
/*N*/ 		if(!bBoundRectDirty0)
/*N*/ 		{
/*N*/ 			bBoundRectDirty = FALSE;  // fuer Broadcast mit dem alten Rect
/*N*/ 
/*N*/ 			if(pPage && pPage->IsInserted())
/*N*/ 				SendRepaintBroadcast(); // Erstmal mit dem alten Rect
/*N*/ 
/*N*/ 			bBoundRectDirty = TRUE;
/*N*/ 			SetRectsDirty(TRUE);    // Durch Vorlagenloeschung evtl. andere Linienbreite
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pNewStSh && !bDying)
/*N*/ 			AddStyleSheet(pNewStSh, TRUE);
/*N*/ 
/*N*/ 		if(pPage && pPage->IsInserted())
/*N*/ 			SendRepaintBroadcast();
/*N*/ 
/*N*/ 		SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ SfxStyleSheet* SdrAttrObj::GetStyleSheet() const
/*N*/ {
/*N*/ 	return mpStyleSheet;
/*N*/ }

/*N*/ void SdrAttrObj::RemoveStyleSheet()
/*N*/ {
/*N*/ 	// Typ checken, weil bei dying sich der Typ abbaut (bis zum Broadcaster runter)
/*N*/ 	if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
/*N*/ 	{
/*N*/ 		EndListening(*mpStyleSheet);
/*N*/ 		EndListening(mpStyleSheet->GetPool());
/*N*/ 
/*N*/ 		// das ItemSet der Vorlage ist jetzt nicht mehr Parent der
/*N*/ 		// eigenen ItemSets
/*N*/ 		if(mpObjectItemSet)
/*N*/ 			mpObjectItemSet->SetParent(NULL);
/*N*/ 
/*N*/ 		bBoundRectDirty = TRUE;
/*N*/ 		SetRectsDirty(TRUE);
/*N*/ 	}
/*N*/ 	mpStyleSheet = NULL;
/*N*/ }

/*N*/ void SdrAttrObj::AddStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	// old StyleSheet is deleted
/*N*/ 	DBG_ASSERT(!mpStyleSheet, "Old style sheet not deleted before setting new one (?)");
/*N*/ 
/*N*/ 	if(pNewStyleSheet)
/*N*/ 	{
/*N*/ 		mpStyleSheet = pNewStyleSheet;
/*N*/ 
/*N*/ 		// ItemSet is needed here, force it
/*N*/ 		ImpForceItemSet();
/*N*/ 
/*N*/ 		// als Listener anmelden
/*N*/ 		StartListening(pNewStyleSheet->GetPool());
/*N*/ 		StartListening(*pNewStyleSheet);
/*N*/ 
/*N*/ 		// harte Attributierung dort loeschen, wo was in der Vorlage steht
/*N*/ 		if(!bDontRemoveHardAttr)
/*N*/ 		{
/*N*/ 			const SfxItemSet& rStyle = pNewStyleSheet->GetItemSet();
/*N*/ 			SfxWhichIter aIter(rStyle);
/*N*/ 			sal_uInt16 nWhich = aIter.FirstWhich();
/*N*/ 
/*N*/ 			while(nWhich)
/*N*/ 			{
/*N*/ 				if(SFX_ITEM_SET == rStyle.GetItemState(nWhich))
/*N*/ 					mpObjectItemSet->ClearItem(nWhich);
/*N*/ 				nWhich = aIter.NextWhich();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// set new stylesheet as parent
/*N*/ 		mpObjectItemSet->SetParent(&pNewStyleSheet->GetItemSet());
/*N*/ 	}
/*N*/ }

/*N*/ void SdrAttrObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	RemoveStyleSheet();
/*N*/ 	AddStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
/*N*/ 	bBoundRectDirty = TRUE;
/*N*/ 	SetRectsDirty(TRUE);
/*N*/ }

/*N*/ void SdrAttrObj::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0;
/*N*/ 
/*N*/ 	if(pUserCall)
/*N*/ 		aBoundRect0 = GetBoundRect();
/*N*/ 
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
/*N*/ }

/*N*/ INT32 SdrAttrObj::ImpGetLineWdt() const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
/*N*/ 
/*N*/ 	if(XLINE_NONE == eLine)
/*N*/ 		return 0; // Garkeine Linie da.
/*N*/ 
/*N*/ 	sal_Int32 nWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
/*N*/ 
/*N*/ 	return nWdt;
/*N*/ }

/*N*/ INT32 SdrAttrObj::ImpGetLineEndAdd() const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	BOOL bStartSet(TRUE);
/*N*/ 	BOOL bEndSet(TRUE);
/*N*/ 
/*N*/ 	if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINESTART))
/*N*/ 	{
/*N*/ 		String aStr(((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetName());
/*N*/ 		if(!aStr.Len())
/*N*/ 			bStartSet = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(rSet.GetItemState(XATTR_LINEEND) != SFX_ITEM_DONTCARE)
/*N*/ 	{
/*N*/ 		String aStr(((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetName());
/*N*/ 		if(!aStr.Len())
/*N*/ 			bEndSet = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bLineEndSet = bStartSet || bEndSet;
/*N*/ 	XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
/*N*/ 
/*N*/ 	if(XLINE_NONE == eLine)
/*N*/ 		return 0; // Garkeine Linie da.
/*N*/ 
/*N*/ 	// Strichstaerke
/*N*/ 	sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
/*N*/ 	sal_Int32 nSttWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
/*N*/ 
/*N*/ 	if(nSttWdt < 0)
/*N*/ 		nSttWdt = -nLineWdt * nSttWdt / 100;
/*N*/ 
/*N*/ 	if(!bLineEndSet)
/*N*/ 		nSttWdt = 0;

/*N*/ 	BOOL bSttCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
/*N*/ 	sal_Int32 nSttHgt = 0;
/*N*/ 
/*N*/ 	if(bSttCenter)
/*N*/ 	{
/*N*/ 		// Linienende steht um die Haelfe ueber
/*N*/ 		XPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
/*N*/ 		nSttHgt = XOutputDevice::InitLineStartEnd(aSttPoly, nSttWdt, bSttCenter);
/*N*/ 		// InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
/*N*/ 	}
/*N*/ 
/*N*/ 	nSttWdt++;
/*N*/ 	nSttWdt /= 2;
/*N*/ 
/*N*/ 	// Lieber etwas mehr, dafuer keine Wurzel ziehen
/*N*/ 	sal_Int32 nSttAdd = Max(nSttWdt, nSttHgt);
/*N*/ 	nSttAdd *= 3;
/*N*/ 	nSttAdd /= 2;
/*N*/ 
/*N*/ 	sal_Int32 nEndWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
/*N*/ 
/*N*/ 	if(nEndWdt < 0)
/*N*/ 		nEndWdt = -nLineWdt * nEndWdt / 100; // <0 = relativ
/*N*/ 
/*N*/ 	if(!bLineEndSet)
/*N*/ 		nEndWdt = 0;
/*N*/ 
/*N*/ 	BOOL bEndCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
/*N*/ 	sal_Int32 nEndHgt = 0;
/*N*/ 
/*N*/ 	if(bEndCenter)
/*N*/ 	{
/*N*/ 		// Linienende steht um die Haelfe ueber
/*N*/ 		XPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
/*N*/ 		nEndHgt = XOutputDevice::InitLineStartEnd(aEndPoly, nEndWdt, bEndCenter);
/*N*/ 		// InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
/*N*/ 	}
/*N*/ 
/*N*/ 	nEndWdt++;
/*N*/ 	nEndWdt /= 2;
/*N*/ 
/*N*/ 	// Lieber etwas mehr, dafuer keine Wurzel ziehen
/*N*/ 	sal_Int32 nEndAdd = Max(nEndWdt, nEndHgt);
/*N*/ 	nEndAdd *= 3;
/*N*/ 	nEndAdd /= 2;
/*N*/ 
/*N*/ 	return Max(nSttAdd, nEndAdd);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////


/*N*/ FASTBOOL SdrAttrObj::ImpGetShadowDist(sal_Int32& nXDist, sal_Int32& nYDist) const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 
/*N*/ 	nXDist = 0L;
/*N*/ 	nYDist = 0L;
/*N*/ 
/*N*/ 	BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
/*N*/ 	if(bShadOn)
/*N*/ 	{
/*N*/ 		nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
/*N*/ 		nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SdrAttrObj::ImpAddShadowToBoundRect()
/*N*/ {
/*N*/ 	sal_Int32 nXDist;
/*N*/ 	sal_Int32 nYDist;
/*N*/ 
/*N*/ 	if(ImpGetShadowDist(nXDist, nYDist))
/*N*/ 	{
/*N*/ 		if(nXDist > 0)
/*N*/ 			aOutRect.Right() += nXDist;
/*N*/ 		else
/*?*/ 			aOutRect.Left() += nXDist;
/*N*/ 
/*N*/ 		if(nYDist > 0)
/*N*/ 			aOutRect.Bottom() += nYDist;
/*N*/ 		else
/*?*/ 			aOutRect.Top() += nYDist;
/*N*/ 	}
/*N*/ }

/*N*/ FASTBOOL SdrAttrObj::ImpSetShadowAttributes( const SfxItemSet& rSet, SfxItemSet& rShadowSet ) const
/*N*/ {
/*N*/ 	BOOL bShadOn=((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
/*N*/ 
/*N*/ 	if(bShadOn)
/*?*/ 	{
// LineAttr for shadow no longer necessary, lines and line shadows are drawn in Paint()
// routines individually (grep for CreateLinePoly())
//
//			if (pLineAttr!=NULL) {
//				XLineAttrSetItem aL(*pLineAttr);
//				aL.GetItemSet().Put(XLineColorItem(String(),aShadCol));
//				aL.GetItemSet().Put(XLineTransparenceItem(nTransp));
//				rXOut.SetLineAttr(aL);
//			}

// #103692# Caller must now handle noFill case       
// 		if(!bNoFill)
// 		{

/*?*/         const SdrShadowColorItem& rShadColItem = ((const SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
/*?*/         Color aShadCol(rShadColItem.GetValue());
/*?*/         sal_uInt16 nTransp = ((const SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
/*?*/         XFillStyle eStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
/*?*/         BOOL bFillBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();
/*?*/ 
/*?*/         if(eStyle==XFILL_HATCH && !bFillBackground)
/*?*/         {
/*?*/             // #41666#
/*?*/             XHatch aHatch = ((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetValue();
/*?*/             aHatch.SetColor(aShadCol);
/*?*/             rShadowSet.Put(XFillHatchItem(String(), aHatch));
/*?*/         }
/*?*/         else
/*?*/         {
/*?*/             if(eStyle != XFILL_NONE && eStyle != XFILL_SOLID)
/*?*/             {
/*?*/                 // also fuer Gradient und Bitmap
/*?*/                 rShadowSet.Put(XFillStyleItem(XFILL_SOLID));
/*?*/             }
/*?*/ 
/*?*/             rShadowSet.Put(XFillColorItem(String(),aShadCol));
/*?*/ 				
/*?*/             // #92183# set XFillTransparenceItem only when no FloatTransparence is used,
/*?*/             // else the OutDev will use the wrong method
/*?*/             if(nTransp)
/*?*/             {
/*?*/                 const XFillFloatTransparenceItem& rFillFloatTransparence = 
/*?*/                     (const XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE);
/*?*/                 if(!rFillFloatTransparence.IsEnabled())
/*?*/                     rShadowSet.Put(XFillTransparenceItem(nTransp));
/*?*/             }
/*?*/         }
/*?*/ 
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

// ItemPool fuer dieses Objekt wechseln
/*N*/ void SdrAttrObj::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
/*N*/ {
/*N*/ 	if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
/*N*/ 	{
/*N*/ 		// call parent
/*N*/ 		SdrObject::MigrateItemPool(pSrcPool, pDestPool, pNewModel);
/*N*/ 
/*N*/ 		if(mpObjectItemSet)
/*N*/ 		{
/*N*/ 			// migrate ItemSet to new pool. Scaling is NOT necessary
/*N*/ 			// because this functionality is used by UNDO only. Thus
/*N*/ 			// objects and ItemSets would be moved back to their original
/*N*/ 			// pool before usage.
/*N*/ 
/*N*/ 			SfxItemSet* pOldSet = mpObjectItemSet;
/*N*/ 			SfxStyleSheet* pStySheet = GetStyleSheet();
/*N*/ 
/*N*/ 			if(GetStyleSheet())
/*?*/ 				RemoveStyleSheet();
/*N*/ 
/*N*/ 			mpObjectItemSet = CreateNewItemSet(*pDestPool);
/*N*/ 
/*N*/ 			GetModel()->MigrateItemSet( pOldSet, mpObjectItemSet, pNewModel );
/*N*/ 
/*N*/ 			// set stylesheet (if used)
/*N*/ 			if(pStySheet)
/*?*/ 				AddStyleSheet(pStySheet, TRUE);
/*N*/ 
/*N*/ 			delete pOldSet;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrAttrObj::HasFill() const
/*N*/ {
/*N*/ 	return (!bClosedObj) ? FALSE
/*N*/ 		: ((XFillStyleItem&)(GetItem(XATTR_FILLSTYLE))).GetValue()!=XFILL_NONE;
/*N*/ }

/*N*/ BOOL SdrAttrObj::HasLine() const
/*N*/ {
/*N*/ 	return ((XLineStyleItem&)(GetItem(XATTR_LINESTYLE))).GetValue()!=XLINE_NONE;
/*N*/ }

// #94547# Have to re-activate more performant, but corrected version.
// This is necessary since SetItemSet() of the old implementation calls
// ItemSetChanged() which replaces in textobjects all text items which
// is wrong behaviour for BurnInStyleSheet.

// #91695# back to corrected old version. Have to check new version again for later builds.
//void SdrAttrObj::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
//{
//	SfxItemPool* pPool = GetItemPool();
//	if ( pPool && mpStyleSheet )
//	{
//		// Get StyleSheet attributes
//		SfxItemSet aSet(*pPool,
//			SDRATTR_START, SDRATTR_NOTPERSIST_FIRST-1,
//			SDRATTR_NOTPERSIST_LAST+1, SDRATTR_END,
//			EE_ITEMS_START,EE_ITEMS_END,
//			0,0);
//
//		SfxWhichIter aIter( mpStyleSheet->GetItemSet() );
//		sal_uInt16 nWhich( aIter.FirstWhich() );
//		const SfxPoolItem* pItem = NULL;
//
//		while( nWhich )
//		{
//			if( SFX_ITEM_SET == mpStyleSheet->GetItemSet().GetItemState(nWhich, TRUE, &pItem) )
//				aSet.Put( *pItem );
//
//			nWhich = aIter.NextWhich();
//		}
//
//		SfxWhichIter aHardAttrIter( GetItemSet() );
//		nWhich = aHardAttrIter.FirstWhich();
//
//		while( nWhich )
//		{
//			if( SFX_ITEM_SET == GetItemSet().GetItemState(nWhich, FALSE, &pItem) )
//				aSet.Put( *pItem );
//
//			nWhich = aHardAttrIter.NextWhich();
//		}
//
//		// Set StyleSheet attributes as hard attributes
//		SetItemSet( aSet );
//  	}
//}

/*
void SdrAttrObj::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
{
    // #89025# Added more performant implementation
    if(mpStyleSheet)
    {
        const SfxItemSet& rSet = mpStyleSheet->GetItemSet();
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());
        const SfxPoolItem *pItem = NULL;

        ImpForceItemSet();
        const SfxItemSet* pParentSet = mpObjectItemSet->GetParent();
        if(pParentSet != 0L)
            mpObjectItemSet->SetParent(0L);

        while(nWhich)
        {
            if(SFX_ITEM_SET == rSet.GetItemState(nWhich, TRUE, &pItem))
                mpObjectItemSet->Put(*pItem);
            nWhich = aIter.NextWhich();
        }

        if(pParentSet != 0L)
            mpObjectItemSet->SetParent(pParentSet);
    }
}
*/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
