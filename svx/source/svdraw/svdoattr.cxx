/*************************************************************************
 *
 *  $RCSfile: svdoattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-13 11:50:51 $
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

#include "svdoattr.hxx"
#include "xpool.hxx"
#include "svditext.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx"
#include "svdxout.hxx"
#include "svdpage.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svdotext.hxx"
#include "svdocapt.hxx"
#include "svdograf.hxx"
#include "svdoole2.hxx"
#include "svdorect.hxx"
#include "svdocirc.hxx"
#include "svdomeas.hxx"

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _XENUM_HXX //autogen
#include "xenum.hxx"
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include "xlineit0.hxx"
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include "xlnstwit.hxx"
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include "xlnedwit.hxx"
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include "xfillit0.hxx"
#endif

#ifndef _SVX_XFLBMTIT_HXX //autogen
#include "xflbmtit.hxx"
#endif

#ifndef _SVX_TEXTIT0_HXX //autogen
#include "xtextit0.hxx"
#endif

#ifndef _SVX_XFLBSTIT_HXX //autogen
#include "xflbstit.hxx"
#endif

#ifndef _SVX_XFLBTOXY_HXX //autogen
#include "xflbtoxy.hxx"
#endif

#ifndef _SVX_XFTSHIT_HXX //autogen
#include "xftshit.hxx"
#endif

#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SVX_FWDTITEM_HXX //autogen
#include "fwdtitem.hxx"
#endif

#ifndef _SVX_XLNSTCIT_HXX //autogen
#include <xlnstcit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XLNTRIT_HXX //autogen
#include <xlntrit.hxx>
#endif

#ifndef _SVX_XFLTRIT_HXX //autogen
#include <xfltrit.hxx>
#endif

#ifndef _SVX_XFLHTIT_HXX //autogen
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include <xlnedit.hxx>
#endif

#ifndef _SVX_XLNEDCIT_HXX //autogen
#include <xlnedcit.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX //autogen
#include <xlnstit.hxx>
#endif

#ifndef _SVX_ADJITEM_HXX
#include <adjitem.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrAttrObj,SdrObject);

SdrAttrObj::SdrAttrObj()
//-/    pLineAttr(NULL),
//-/    pFillAttr(NULL),
//-/    pTextAttr(NULL),
//-/    pShadAttr(NULL),
//-/    pOutlAttr(NULL),
//-/    pMiscAttr(NULL),
:   mpStyleSheet(NULL),
    mpObjectItemSet(NULL)
{
}

SdrAttrObj::~SdrAttrObj()
{
//-/    pLineAttr=(XLineAttrSetItem*)  ImpSetNewAttr(pLineAttr,NULL,FALSE);
//-/    pFillAttr=(XFillAttrSetItem*)  ImpSetNewAttr(pFillAttr,NULL,FALSE);
//-/    pTextAttr=(XTextAttrSetItem*)  ImpSetNewAttr(pTextAttr,NULL,FALSE);
//-/    pShadAttr=(SdrShadowSetItem*)  ImpSetNewAttr(pShadAttr,NULL,FALSE);
//-/    pOutlAttr=(SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,NULL,FALSE);
//-/    pMiscAttr=(SdrMiscSetItem*)    ImpSetNewAttr(pMiscAttr,NULL,FALSE);
    ImpDeleteItemSet();
}

void SdrAttrObj::ImpDeleteItemSet()
{
    if(mpObjectItemSet)
    {
        if(GetStyleSheet())
            RemoveStyleSheet();

        delete mpObjectItemSet;
        mpObjectItemSet = 0L;
    }
}

void SdrAttrObj::ImpForceItemSet()
{
    if(!mpObjectItemSet)
    {
        SfxItemPool* pPool = GetItemPool();
        mpObjectItemSet = CreateNewItemSet(*pPool);
        ForceDefaultAttr();
    }
}

const Rectangle& SdrAttrObj::GetSnapRect() const
{
    if(bSnapRectDirty)
    {
        ((SdrAttrObj*)this)->RecalcSnapRect();
        ((SdrAttrObj*)this)->bSnapRectDirty = FALSE;
    }
    return maSnapRect;
}

void SdrAttrObj::operator=(const SdrObject& rObj)
{
    // call parent
    SdrObject::operator=(rObj);

    SdrAttrObj* pAO = PTR_CAST(SdrAttrObj, (SdrObject*)&rObj);
    if(pAO)
    {
//-/        pLineAttr=(const XLineAttrSetItem*)  ImpSetNewAttr(pLineAttr,pAO->pLineAttr);
//-/        pFillAttr=(const XFillAttrSetItem*)  ImpSetNewAttr(pFillAttr,pAO->pFillAttr);
//-/        pTextAttr=(const XTextAttrSetItem*)  ImpSetNewAttr(pTextAttr,pAO->pTextAttr);
//-/        pShadAttr=(const SdrShadowSetItem*)  ImpSetNewAttr(pShadAttr,pAO->pShadAttr);
//-/        pOutlAttr=(const SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,pAO->pOutlAttr);
//-/        pMiscAttr=(const SdrMiscSetItem*)    ImpSetNewAttr(pMiscAttr,pAO->pMiscAttr);
        ImpDeleteItemSet();

        if(pAO->mpObjectItemSet)
            mpObjectItemSet = pAO->mpObjectItemSet->Clone(TRUE);
//-/        mpObjectItemSet = new SfxItemSet(*pAO->mpObjectItemSet);

        if(pAO->GetStyleSheet())
            AddStyleSheet(pAO->GetStyleSheet(), TRUE);
    }
}

void SdrAttrObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if(rIn.GetError())
        return;

    // call parent
    SdrObject::ReadData(rHead, rIn);

    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        sal_uInt16 nSetID;
        SfxItemSet aNewSet(GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_LINE;
        const XLineAttrSetItem* pLineAttr = (const XLineAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pLineAttr)
            aNewSet.Put(pLineAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_FILL;
        const XFillAttrSetItem* pFillAttr = (const XFillAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pFillAttr)
            aNewSet.Put(pFillAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = XATTRSET_TEXT;
        const XTextAttrSetItem* pTextAttr = (const XTextAttrSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pTextAttr)
            aNewSet.Put(pTextAttr->GetItemSet());

        if(rHead.GetVersion() < 11)
            { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
        nSetID = SDRATTRSET_SHADOW;
        const SdrShadowSetItem* pShadAttr = (const SdrShadowSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pShadAttr)
            aNewSet.Put(pShadAttr->GetItemSet());

        if(rHead.GetVersion() >= 5)
        {
            if(rHead.GetVersion() < 11)
                { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
            nSetID = SDRATTRSET_OUTLINER;
            const SdrOutlinerSetItem* pOutlAttr = (const SdrOutlinerSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
            if(pOutlAttr)
                aNewSet.Put(pOutlAttr->GetItemSet());
        }

        if(rHead.GetVersion() >= 6)
        {
            if(rHead.GetVersion() < 11)
                { sal_uInt16 nWhichDum; rIn >> nWhichDum; }
            nSetID = SDRATTRSET_MISC;
            const SdrMiscSetItem* pMiscAttr = (const SdrMiscSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
            if(pMiscAttr)
                aNewSet.Put(pMiscAttr->GetItemSet());
        }

        SetItemSet(aNewSet);
    }
    else
    {
        // an den Surrogaten und ggf. auch Whiches vorbeiseeken
        // ganz zu anfang waren es 4 SetItems
        sal_uInt16 nAnz(4);

        if(rHead.GetVersion() >= 5)
            nAnz++;

        if(rHead.GetVersion() >= 6)
            nAnz++;

        nAnz *= sizeof(sal_uInt16);

        if(rHead.GetVersion() < 11)
            nAnz *= 2;

        rIn.SeekRel(nAnz);
    }

    // TextToContour: altes Format(Flag) in neues Format(Item) wandeln
    if(rHead.GetVersion() <= 4 && pPool)
    {
        SetItem(XFormTextStyleItem(XFT_NONE));
    }

    // Fuer die StyleSheetgeschichte gehoert eigentlich auch noch eine
    // Versionsabfrage hierher.
    // Name und Familie des StyleSheet einlesen, in Pointer auf StyleSheet
    // umwandeln lassen (SB)
    XubString aStyleSheetName;
    SfxStyleFamily eFamily;
    sal_uInt16 nRead;

    // UNICODE: rIn>>aStyleSheetName;
    rIn.ReadByteString(aStyleSheetName);

    if(aStyleSheetName.Len())
    {
        rIn >> nRead;
        eFamily = (SfxStyleFamily)(int)nRead;

        // ab Version 1 wird der CharacterSet gelesen, ab V11 nicht mehr
        if(rHead.GetVersion() > 0 && rHead.GetVersion() < 11)
        {
            sal_Int16 nCharSet;
            rIn >> nCharSet;
            //aStyleSheetName.Convert((CharSet)nCharSet);
            // nicht mehr noetig, da ab Vers 11 der CharSet bereits am
            // Stream gesetzt wird.
        }

        DBG_ASSERT(pModel, "SdrAttrObj::ReadData(): pModel=NULL, StyleSheet kann nicht gesetzt werden!");
        if(pModel)
        {
            SfxStyleSheetBasePool *pPool = pModel->GetStyleSheetPool();
            DBG_ASSERT(pPool, "SdrAttrObj::ReadData(): kein StyleSheetPool");

            if(pPool)
            {
                SfxStyleSheet *pTmpStyleSheet = (SfxStyleSheet*)pPool->Find(aStyleSheetName, eFamily);
                DBG_ASSERT(pTmpStyleSheet, "SdrAttrObj::ReadData(): StyleSheet nicht gefunden");

                if(pTmpStyleSheet)
                    AddStyleSheet(pTmpStyleSheet, TRUE);
//-/                {
//-/                    StartListening(*pPool);
//-/                    StartListening(*pTmpStyleSheet);
//-/
//-///-/                    SfxItemSet* pParent = &pTmpStyleSheet->GetItemSet();
//-///-/                    if (pLineAttr!=NULL) ((SfxItemSet*)&pLineAttr->GetItemSet())->SetParent(pParent);
//-///-/                    if (pFillAttr!=NULL) ((SfxItemSet*)&pFillAttr->GetItemSet())->SetParent(pParent);
//-///-/                    if (pTextAttr!=NULL) ((SfxItemSet*)&pTextAttr->GetItemSet())->SetParent(pParent);
//-///-/                    if (pShadAttr!=NULL) ((SfxItemSet*)&pShadAttr->GetItemSet())->SetParent(pParent);
//-///-/                    if (pOutlAttr!=NULL) ((SfxItemSet*)&pOutlAttr->GetItemSet())->SetParent(pParent);
//-///-/                    if (pMiscAttr!=NULL) ((SfxItemSet*)&pMiscAttr->GetItemSet())->SetParent(pParent);
//-/
//-/                    if(!mpObjectItemSet)
//-/                        mpObjectItemSet = new SfxItemSet(*pPool);
//-/
//-/                    mpObjectItemSet->SetParent(&pTmpStyleSheet->GetItemSet());
//-/                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrAttrObj::PreSave()
{
    // call parent
    SdrObject::PreSave();

    // prepare SetItems for storage
    const SfxItemSet& rSet = GetItemSet();
    const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;

    XLineAttrSetItem aLineAttr(rSet.GetPool());
    aLineAttr.GetItemSet().Put(rSet);
    aLineAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aLineAttr);

    XFillAttrSetItem aFillAttr(rSet.GetPool());
    aFillAttr.GetItemSet().Put(rSet);
    aFillAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aFillAttr);

    XTextAttrSetItem aTextAttr(rSet.GetPool());
    aTextAttr.GetItemSet().Put(rSet);
    aTextAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aTextAttr);

    SdrShadowSetItem aShadAttr(rSet.GetPool());
    aShadAttr.GetItemSet().Put(rSet);
    aShadAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aShadAttr);

    SdrOutlinerSetItem aOutlAttr(rSet.GetPool());
    aOutlAttr.GetItemSet().Put(rSet);
    aOutlAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aOutlAttr);

    SdrMiscSetItem aMiscAttr(rSet.GetPool());
    aMiscAttr.GetItemSet().Put(rSet);
    aMiscAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aMiscAttr);
}

void SdrAttrObj::PostSave()
{
    // call parent
    SdrObject::PostSave();

    // remove SetItems from local itemset
    ((SdrAttrObj*)this)->ImpForceItemSet();
    mpObjectItemSet->ClearItem(XATTRSET_LINE);
    mpObjectItemSet->ClearItem(XATTRSET_FILL);
    mpObjectItemSet->ClearItem(XATTRSET_TEXT);
    mpObjectItemSet->ClearItem(SDRATTRSET_SHADOW);
    mpObjectItemSet->ClearItem(SDRATTRSET_OUTLINER);
    mpObjectItemSet->ClearItem(SDRATTRSET_MISC);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrAttrObj::WriteData(SvStream& rOut) const
{
    // call parent
    SdrObject::WriteData(rOut);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_LINE));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_FILL));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_TEXT));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_SHADOW));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_OUTLINER));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MISC));



//-/        const SfxItemSet& rSet = GetItemSet();
//-/
//-/        XLineAttrSetItem aLineAttr(pPool);
//-/        aLineAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rLineAttr = pPool->Put(aLineAttr);
//-/        pPool->StoreSurrogate(rOut, &rLineAttr);
//-/        pPool->Remove(rLineAttr);
//-/
//-/        XFillAttrSetItem aFillAttr(pPool);
//-/        aFillAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rFillAttr = pPool->Put(aFillAttr);
//-/        pPool->StoreSurrogate(rOut, &rFillAttr);
//-/        pPool->Remove(rFillAttr);
//-/
//-/        XTextAttrSetItem aTextAttr(pPool);
//-/        aTextAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rTextAttr = pPool->Put(aTextAttr);
//-/        pPool->StoreSurrogate(rOut, &rTextAttr);
//-/        pPool->Remove(rTextAttr);
//-/
//-/        SdrShadowSetItem aShadAttr(pPool);
//-/        aShadAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rShadAttr = pPool->Put(aShadAttr);
//-/        pPool->StoreSurrogate(rOut, &rShadAttr);
//-/        pPool->Remove(rShadAttr);
//-/
//-/        SdrOutlinerSetItem aOutlAttr(pPool);
//-/        aOutlAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rOutlAttr = pPool->Put(aOutlAttr);
//-/        pPool->StoreSurrogate(rOut, &rOutlAttr);
//-/        pPool->Remove(rOutlAttr);
//-/
//-/        SdrMiscSetItem aMiscAttr(pPool);
//-/        aMiscAttr.GetItemSet().Put(rSet);
//-/        const SfxPoolItem& rMiscAttr = pPool->Put(aMiscAttr);
//-/        pPool->StoreSurrogate(rOut, &rMiscAttr);
//-/        pPool->Remove(rMiscAttr);
    }
    else
    {
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
        rOut << sal_uInt16(SFX_ITEMS_NULL);
    }

    // StyleSheet-Pointer als Name, Familie abspeichern
    // wenn kein StyleSheet vorhanden: leeren String speichern
    if(GetStyleSheet())
    {
        // UNICODE: rOut << pStyleSheet->GetName();
        rOut.WriteByteString(GetStyleSheet()->GetName());
        rOut << (sal_uInt16)(int)(GetStyleSheet()->GetFamily());
    }
    else
    {
        // UNICODE: rOut << String();
        rOut.WriteByteString(String());
    }
}

//-/SfxItemPool* SdrAttrObj::ImpGetItemPool(const SfxPoolItem* pAlternateItem1, const SfxPoolItem* pAlternateItem2) const
//-/{
//-/    SfxItemPool* pPool= (pModel==NULL) ? NULL : &pModel->GetItemPool();
//-/    if (pPool==NULL) {
//-/        SfxSetItem* pSI;
//-/        if (pAlternateItem1!=NULL) {
//-/            pSI=PTR_CAST(SfxSetItem,pAlternateItem1);
//-/            if (pSI!=NULL) pPool=(SfxItemPool*)pSI->GetItemSet().GetPool();
//-/        }
//-/        if (pPool==NULL && pAlternateItem2!=NULL) {
//-/            pSI=PTR_CAST(SfxSetItem,pAlternateItem2);
//-/            if (pSI!=NULL) pPool=(SfxItemPool*)pSI->GetItemSet().GetPool();
//-/        }
//-/    }
//-/    return pPool;
//-/}

//-/SfxItemPool* SdrAttrObj::ImpGetItemPool() const
//-/{
//-/    SfxItemPool* pPool = (!pModel) ? NULL : &pModel->GetItemPool();
//-/
//-/    if(!pPool && mpObjectItemSet)
//-/        pPool = mpObjectItemSet->GetPool();
//-/    {
//-/        USHORT nAnz=GetSetItemCount();
//-/        for (USHORT i=0; i<nAnz && pPool==NULL; i++) {
//-/            const SfxSetItem* pSI=GetSetItem(i);
//-/            if (pSI!=NULL) pPool=pSI->GetItemSet().GetPool();
//-/        }
//-/    }
//-/
//-/    return pPool;
//-/}

//-/const SfxPoolItem* SdrAttrObj::ImpSetNewAttr(const SfxPoolItem* pAkt, const SfxPoolItem* pNew, FASTBOOL bChg)
//-/{
//-/    SfxItemPool* pPool=ImpGetItemPool(pAkt,pNew);
//-/    if (pPool!=NULL) {
//-/        if (pAkt!=NULL) pPool->Remove(*pAkt);
//-/        if (pNew!=NULL) pNew=&pPool->Put(*pNew);
//-/    }
//-/    return pNew;
//-/}

static void ImpScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
{
    sal_Int32 nMul(rScale.GetNumerator());
    sal_Int32 nDiv(rScale.GetDenominator());

    if(!rScale.IsValid() || !nDiv)
        return;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich(aIter.FirstWhich());
    const SfxPoolItem *pItem = NULL;

    while(nWhich)
    {
        if(SFX_ITEM_SET == rSet.GetItemState(nWhich, FALSE, &pItem))
        {
            if(pItem->HasMetrics())
            {
                SfxPoolItem* pNewItem = pItem->Clone();
                pNewItem->ScaleMetrics(nMul, nDiv);
                rSet.Put(*pNewItem);
            }
        }
        nWhich = aIter.NextWhich();
    }
}

void SdrAttrObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel = pModel;

    // test for correct pool in ItemSet; move to new pool if necessary
    if(pNewModel && mpObjectItemSet && mpObjectItemSet->GetPool() != &pNewModel->GetItemPool())
        MigrateItemPool(mpObjectItemSet->GetPool(), &pNewModel->GetItemPool());

    // call parent
    SdrObject::SetModel(pNewModel);

    if(pOldModel != pNewModel && pNewModel && !pNewModel->IsLoading())
    {
//-/        // fuer ein frisch eingefuegtes Obj die Defaultattribute setzen
//-/        if(!pOldModel)
//-/            ForceDefaultAttr(&pModel->GetItemPool());

        // fuer ein bereits "lebendes" Model die Attribute von einem Pool in den anderen schieben
        if(pOldModel)
        {
            // Checken, ob sich die ScaleUnit geaendert hat.
            // Denn dann muessen naemlich alle MetrikItems umgerechnet werden.
            MapUnit aOldUnit(pOldModel->GetScaleUnit());
            MapUnit aNewUnit(pNewModel->GetScaleUnit());
            BOOL bScaleUnitChanged(aNewUnit != aOldUnit);
            Fraction aMetricFactor;

            if(bScaleUnitChanged)
            {
                aMetricFactor = GetMapFactor(aOldUnit, aNewUnit).X();

                if(mpObjectItemSet)
                    ImpScaleItemSet(*mpObjectItemSet, aMetricFactor);
            }

            // Und nun alle Items auf die das Obj verweisst aus
            // dem alten Pools raus und in den neuen rein.
//-/            SdrItemPool& rOldPool = (SdrItemPool&)pOldModel->GetItemPool();
//-/            SdrItemPool& rNewPool = (SdrItemPool&)pModel->GetItemPool();
            SfxStyleSheet* pOldStyleSheet = GetStyleSheet();

//-/            DBG_ASSERT(&rOldPool != NULL, "SdrAttrObj::SetModel(): Kein ItemPool am alten Model");
//-/            DBG_ASSERT(&rNewPool != NULL, "SdrAttrObj::SetModel(): Kein ItemPool am neuen Model");

//-/            if(mpObjectItemSet)
//-/            {
//-/                SfxItemSet* pNewSet = mpObjectItemSet->Clone(TRUE, &rNewPool);
//-/
//-/                if(bScaleUnitChanged)
//-/                    ImpScaleItemSet(*pNewSet, aMetricFactor);
//-/
//-/                // set new itemset
//-/                ImpDeleteItemSet();
//-/                mpObjectItemSet = pNewSet;
//-/            }
//-/            sal_uInt16 nItemSetAnz(GetSetItemCount());
//-/            for(sal_uInt16 i(0); i < nItemSetAnz; i++)
//-/            {
//-/                const SfxSetItem* pSI=GetSetItem(i);
//-/                if (pSI!=NULL) {
//-/                    const SfxSetItem* pNewAttr=NULL;
//-/                    if (!bScaleUnitChanged) {
//-/                        pNewAttr=&(const SfxSetItem&)rNewPool.Put(*pSI);
//-/                    } else  { // ggf. alle MetrikItems Skalieren
//-/                        SfxSetItem* pNewSetItem=(SfxSetItem*)pSI->Clone(&rNewPool);
//-/                        ImpScaleItemSet(pNewSetItem->GetItemSet(),aMetricFactor);
//-/                        pNewAttr=&(const SfxSetItem&)rNewPool.Put(*pNewSetItem);
//-/                    }
//-/                    rOldPool.Remove(*pSI);
//-/                    SetSetItem(i,pNewAttr);
//-/                }
//-/            }

            // ***** StyleSheets Anfang *****
            // ggfs. StyleSheet und dessen Parents kopieren
            // Follows werden nicht beruecksichtigt (ganz wie im Writer)
            if(pOldStyleSheet)
            {
                SfxStyleSheetBase* pSheet = pOldStyleSheet;
                SfxStyleSheetBasePool* pOldPool = pOldModel->GetStyleSheetPool();
                SfxStyleSheetBasePool* pNewPool = pModel->GetStyleSheetPool();
                DBG_ASSERT(pOldPool, "SdrAttrObj::SetModel(): Objekt hat StyleSheet aber keinen StyleSheetPool am SdrModel");

                if(pOldPool && pNewPool)
                {
                    // Liste der zu kopierenden Vorlagen
                    List aList;
                    SfxStyleSheetBase* pAnchor = NULL;

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
                            // die gesuchte Vorlage gibt's schon
                            pSheet = NULL;
                        }
                    }

                    // kopieren und Parents der Kopien setzen
                    pSheet = (SfxStyleSheetBase*)aList.First();
                    SfxStyleSheetBase* pNewSheet = NULL;
                    SfxStyleSheetBase* pLastSheet = NULL;
                    SfxStyleSheetBase* pForThisObject = NULL;

                    while(pSheet)
                    {
                        pNewSheet = &pNewPool->Make(pSheet->GetName(), pSheet->GetFamily(), pSheet->GetMask());
                        pNewSheet->GetItemSet().Put(pSheet->GetItemSet(), FALSE);

                        if(bScaleUnitChanged)
                            ImpScaleItemSet(pNewSheet->GetItemSet(), aMetricFactor);

                        if(pLastSheet)
                            pLastSheet->SetParent(pNewSheet->GetName());

                        if(!pForThisObject)
                            pForThisObject = pNewSheet;

                        pLastSheet = pNewSheet;
                        pSheet = (SfxStyleSheetBase*)aList.Next();
                    }

                    // Veknuepfung mit der im Zielpool gefundenen Vorlage
                    if(pAnchor && pLastSheet)
                        pLastSheet->SetParent(pAnchor->GetName());

                    // falls die Liste leer war (alle Vorlagen schon im Zielpool
                    // vorhanden) ist pForThisObject noch nicht gesetzt
                    if(!pForThisObject && pAnchor)
                        pForThisObject = pAnchor;

                    // am alten StyleSheet ab- und am neuen anmelden
                    if(GetStyleSheet() != pForThisObject)
                    {
                        RemoveStyleSheet();
                        AddStyleSheet((SfxStyleSheet*)pForThisObject, TRUE);
                    }
                }
                else
                {
                    // Aha, im neuen Model gibt's also kein StyleSheetPool
                    // also setzte ich "einfach" alle Attribute des alten StyleSheets hart
                    List aList;
                    const SfxItemSet* pItemSet = &pOldStyleSheet->GetItemSet();

                    while(pItemSet)
                    {
                        aList.Insert((void*)pItemSet, CONTAINER_APPEND);
                        pItemSet = pItemSet->GetParent();
                    }

                    SfxItemSet* pNewSet = CreateNewItemSet(pNewModel->GetItemPool());
                    pItemSet = (SfxItemSet*)aList.Last();

                    while(pItemSet)
                    {
                        pNewSet->Put(*pItemSet);
                        pItemSet = (SfxItemSet*)aList.Prev();
                    }

                    // Attribute, die schon vorher hart gesetzt
                    // waren muessen aber erhalten bleiben:
                    if(mpObjectItemSet)
                    {
                        SfxWhichIter aIter(*mpObjectItemSet);
                        sal_uInt16 nWhich = aIter.FirstWhich();

                        while(nWhich)
                        {
                            if(mpObjectItemSet->GetItemState(nWhich, FALSE) == SFX_ITEM_SET)
                                pNewSet->Put(mpObjectItemSet->Get(nWhich));

                            nWhich = aIter.NextWhich();
                        }
                    }
//-/                        for(sal_uInt16 i(0); i < nItemSetAnz; i++)
//-/                        {
//-/                            const SfxSetItem* pSI = GetSetItem(i);
//-/                            if(pSI)
//-/                            {
//-/                                const SfxItemSet& rIS = pSI->GetItemSet();
//-/                                SfxWhichIter aIter(rIS);
//-/                                sal_uInt16 nWhich = aIter.FirstWhich();
//-/
//-/                                while(nWhich)
//-/                                {
//-/                                    if(rIS.GetItemState(nWhich, FALSE) == SFX_ITEM_SET)
//-/                                        aSet.ClearItem(nWhich);
//-/
//-/                                    nWhich = aIter.NextWhich();
//-/                                }
//-/                            }
//-/                        }


                    if(bScaleUnitChanged)
                        ImpScaleItemSet(*pNewSet, aMetricFactor);

                    ImpDeleteItemSet();
                    mpObjectItemSet = pNewSet;

//-/                    SetAttributes(*pNewSet, FALSE);
//-/                    RemoveStyleSheet();
                }
            }
            // ***** StyleSheets Ende *****
        }

        // Jedes Objekt bekommt initial den DefaultStyleSheet
        // des Model, falls noch kein StyleSheet gesetzt.
        if(mpObjectItemSet && !GetStyleSheet() && pModel && !pModel->IsLoading())
        {
            SfxStyleSheet* pDefSS = pModel->GetDefaultStyleSheet();

            if(pDefSS)
            {
                // Dabei jedoch nicht die harte Attributierung loeschen!
                AddStyleSheet(pDefSS, TRUE);
            }
        }
    }
}

void SdrAttrObj::ForceDefaultAttr()
{
//-/    if(pPool)
//-/    {
    SdrTextObj* pText = PTR_CAST(SdrTextObj, this);
    BOOL bTextFrame(pText && pText->IsTextFrame());

//-/        mpObjectItemSet = CreateNewItemSet(*pPool);

    ImpForceItemSet();
    if(bTextFrame)
    {
        SdrCaptionObj* pCapt = PTR_CAST(SdrCaptionObj, this);
        BOOL bCaption(pCapt != 0L);

        if(!bCaption)
             mpObjectItemSet->Put(XLineStyleItem(XLINE_NONE));

        mpObjectItemSet->Put(XFillColorItem(String(), Color(COL_WHITE)));
        mpObjectItemSet->Put(XFillStyleItem(XFILL_NONE));
    }
    else
    {
        mpObjectItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        mpObjectItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
        mpObjectItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
    }
//-/    }
//-/    if (pPool!=NULL && pLineAttr==NULL) {
//-/        SdrTextObj* pText=PTR_CAST(SdrTextObj,this);
//-/        SdrCaptionObj* pCapt=PTR_CAST(SdrCaptionObj,this);
//-/        FASTBOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
//-/        FASTBOOL bCaption=pCapt!=NULL;
//-/
//-///-/#ifdef SDRDEFITEMCACHE
//-///-/        // SdrItemPool in der Verkettung suchen (wg. Writer)
//-///-/        SdrItemPool* pSdrPool=NULL;
//-///-/        SfxItemPool* pTmpPool=pPool;
//-///-/        while (pTmpPool!=NULL && pSdrPool==NULL) {
//-///-/            // Etwas komplizierte Abfrage und auch Hack, weil am
//-///-/            // Pool kein RTTI vorhanden ist! Joe, 02-09-1997
//-///-/            if (pTmpPool->GetFirstWhich()<=SDRATTR_START && pTmpPool->GetLastWhich()>=SDRATTR_END) {
//-///-/                if (pTmpPool->GetName().EqualsAscii("XOutdevItemPool"))
//-///-/                    pSdrPool=(SdrItemPool*)pTmpPool;
//-///-/            }
//-///-/            pTmpPool=pTmpPool->GetSecondaryPool();
//-///-/        }
//-///-/        DBG_ASSERT(pSdrPool!=NULL,"SdrAttrObj::ForceDefaultAttr(): Kein SdrItemPool gefunden!");
//-///-/        if (pSdrPool->pDefLineAttr==NULL) pSdrPool->ImpMakeDefItems();
//-///-/        if (bTextFrame) {
//-///-/            pLineAttr=bCaption ? pSdrPool->pDefLineAttr : pSdrPool->pDefTextFrameLineAttr;
//-///-/            pFillAttr=pSdrPool->pDefTextFrameFillAttr;
//-///-/            pTextAttr=pSdrPool->pDefTextFrameTextAttr;
//-///-/            pShadAttr=pSdrPool->pDefTextFrameShadAttr;
//-///-/            pOutlAttr=pSdrPool->pDefTextFrameOutlAttr;
//-///-/            pMiscAttr=pSdrPool->pDefTextFrameMiscAttr;
//-///-/        } else {
//-///-/            pLineAttr=pSdrPool->pDefLineAttr;
//-///-/            pFillAttr=pSdrPool->pDefFillAttr;
//-///-/            pTextAttr=pSdrPool->pDefTextAttr;
//-///-/            pShadAttr=pSdrPool->pDefShadAttr;
//-///-/            pOutlAttr=pSdrPool->pDefOutlAttr;
//-///-/            pMiscAttr=pSdrPool->pDefMiscAttr;
//-///-/        }
//-///-/        pLineAttr->AddRef();
//-///-/        pFillAttr->AddRef();
//-///-/        pTextAttr->AddRef();
//-///-/        pShadAttr->AddRef();
//-///-/        pOutlAttr->AddRef();
//-///-/        pMiscAttr->AddRef();
//-///-/#else // SDRDEFITEMCACHE
//-/        if (pLineAttr==NULL) {
//-/            XLineAttrSetItem aSetItem(pPool);
//-/            if (bTextFrame && !bCaption) {
//-/                 aSetItem.GetItemSet().Put(XLineStyleItem(XLINE_NONE));
//-/            }
//-/            pLineAttr=(XLineAttrSetItem*)ImpSetNewAttr(pLineAttr,&aSetItem,FALSE);
//-/        }
//-/        if (pFillAttr==NULL) {
//-/            XFillAttrSetItem aSetItem(pPool);
//-/            if (bTextFrame) {
//-/                aSetItem.GetItemSet().Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
//-/                aSetItem.GetItemSet().Put(XFillStyleItem(XFILL_NONE));
//-/            }
//-/            pFillAttr=(XFillAttrSetItem*)ImpSetNewAttr(pFillAttr,&aSetItem,FALSE);
//-/        }
//-/        if (pTextAttr==NULL) {
//-/            XTextAttrSetItem aSetItem(pPool);
//-/            pTextAttr=(XTextAttrSetItem*)ImpSetNewAttr(pTextAttr,&aSetItem,FALSE);
//-/        }
//-/        if (pShadAttr==NULL) {
//-/            SdrShadowSetItem aSetItem(pPool);
//-/            pShadAttr=(SdrShadowSetItem*)ImpSetNewAttr(pShadAttr,&aSetItem,FALSE);
//-/        }
//-/        if (pOutlAttr==NULL) {
//-/            SdrOutlinerSetItem aSetItem(pPool);
//-/            if (!bTextFrame) {
//-/                aSetItem.GetItemSet().Put(SvxAdjustItem(SVX_ADJUST_CENTER));
//-/            }
//-/            pOutlAttr=(SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,&aSetItem,FALSE);
//-/        }
//-/        if (pMiscAttr==NULL) {
//-/            SdrMiscSetItem aSetItem(pPool);
//-/            if (!bTextFrame) {
//-/                aSetItem.GetItemSet().Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
//-/                aSetItem.GetItemSet().Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
//-/            }
//-/            pMiscAttr=(SdrMiscSetItem*)ImpSetNewAttr(pMiscAttr,&aSetItem,FALSE);
//-/        }
//-///-/#endif // SDRDEFITEMCACHE
//-/    }
}

//-/void SdrAttrObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
//-/{
//-/    USHORT nAnz=GetSetItemCount();
//-/    FASTBOOL bHasText=GetOutlinerParaObject()!=NULL;
//-/    SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,this);
//-/    FASTBOOL bIsContourFrame=pTextObj!=NULL && pTextObj->IsContourTextFrame();
//-/    SdrMeasureObj* pMeasObj=PTR_CAST(SdrMeasureObj,this);
//-/    if (!bHasText && pTextObj!=NULL && pTextObj->IsTextEditActive()) bHasText=TRUE;
//-/    FASTBOOL bTextFrame=pTextObj!=NULL && pTextObj->IsTextFrame();
//-/    FASTBOOL bCanFontwork=bHasText && pTextObj!=NULL && !bTextFrame && pMeasObj==NULL;
//-/    FASTBOOL bCanContourFrame=bHasText && pTextObj!=NULL && !bTextFrame;
//-/    FASTBOOL bGrafObj=HAS_BASE(SdrGrafObj,this) || HAS_BASE(SdrOle2Obj,this);
//-/    FASTBOOL bHasEckRad=!bGrafObj && !bIsUnoObj && HAS_BASE(SdrRectObj,this) && !HAS_BASE(SdrCircObj,this);
//-/    FASTBOOL bHasFill=bClosedObj;
//-/    FASTBOOL bHasLEnd=!bClosedObj || HAS_BASE(SdrCaptionObj,this);
//-/    for (USHORT i=0; i<nAnz; i++) {
//-/        const SfxSetItem* pSI=GetSetItem(i);
//-/        FASTBOOL bOk=pSI!=NULL;
//-/        if (pSI==pFillAttr && !bHasFill) bOk=FALSE;
//-/        if (pSI==pTextAttr && !bCanFontwork) bOk=FALSE;
//-/        if (pSI==pOutlAttr && !bHasText) bOk=FALSE;
//-/        if (bGrafObj && (pSI==pFillAttr || pSI==pLineAttr)) bOk=FALSE; // fuer Grafik und OLE
//-/        if (pSI!=NULL && !bMerge && bOnlyHardAttr) bOk=TRUE; // z.B. im Falle Undo
//-/        if (bOk) {
//-/            const SfxItemSet& rIS=pSI->GetItemSet();
//-/            SfxWhichIter aIter(rIS);
//-/            USHORT nWhich=aIter.FirstWhich();
//-/            while (nWhich!=0) {
//-/                if ((bHasLEnd || nWhich<XATTR_LINESTART || nWhich>XATTR_LINEENDCENTER) &&
//-/                    (bHasEckRad || nWhich!=SDRATTR_ECKENRADIUS) &&
//-/                    (bHasText || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_CONTOURFRAME) &&
//-/                    (bTextFrame || (nWhich!=SDRATTR_TEXT_AUTOGROWHEIGHT && nWhich!=SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich!=SDRATTR_TEXT_MAXFRAMEHEIGHT &&
//-/                                    nWhich!=SDRATTR_TEXT_AUTOGROWWIDTH && nWhich!=SDRATTR_TEXT_MINFRAMEWIDTH && nWhich!=SDRATTR_TEXT_MAXFRAMEWIDTH)) &&
//-/                    (bCanContourFrame || nWhich!=SDRATTR_TEXT_CONTOURFRAME) &&
//-/                    (!bIsContourFrame || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_HORZADJUST) &&
//-/                    (nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) )
//-/                {
//-/                    if (!bOnlyHardAttr || rIS.GetItemState(nWhich,FALSE)==SFX_ITEM_SET) {
//-/                        const SfxPoolItem& rItem=rIS.Get(nWhich);
//-/                        if (bMerge) rAttr.MergeValue(rItem,TRUE);
//-/                        else rAttr.Put(rItem);
//-/                    }
//-/                }
//-/                nWhich=aIter.NextWhich();
//-/            }
//-/        }
//-/    }
//-/    if (!bMerge && pLineAttr!=NULL) {
//-/        rAttr.SetParent(pLineAttr->GetItemSet().GetParent());
//-/    }
//-/}

//-/void SdrAttrObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    SfxItemPool* pPool=GetItemPool();
//-/    if (pPool==NULL) pPool=(SfxItemPool*)rAttr.GetPool();
//-/    ForceDefaultAttr(pPool);
//-/    USHORT nAnz=GetSetItemCount();
//-/
//-/    if (bReplaceAll) {
//-/        SfxWhichIter aWhIter(rAttr);
//-/        for (USHORT i=0; i<nAnz; i++) {
//-/            const SfxSetItem* pOldSI=GetSetItem(i);
//-/            if (pOldSI!=NULL) {
//-/                const SfxItemSet& rOldIS=pOldSI->GetItemSet();
//-/                const USHORT* pRanges=rOldIS.GetRanges();
//-/                FASTBOOL bOneRange=pRanges[2]==0;
//-/                USHORT nAnf=pRanges[0];
//-/                USHORT nEnd=pRanges[1];
//-/                SfxSetItem* pNewSI=NULL; //MakeNewSetItem(i,TRUE);
//-/                SfxItemSet* pDstSet=NULL; //pNewSI->GetItemSet();
//-/                // funkt nicht:
//-/                // pNewSI->GetItemSet().Set(rAttr);
//-/                // deshalb manuell #36313#
//-/                pNewSI=MakeNewSetItem(i,TRUE);
//-/                pDstSet=&pNewSI->GetItemSet();
//-/                USHORT nWhich=aWhIter.FirstWhich();
//-/                while (nWhich!=0) {
//-/                    const SfxPoolItem* pItem=NULL;
//-/                    SfxItemState eState=rAttr.GetItemState(nWhich,TRUE,&pItem);
//-/                    if (eState==SFX_ITEM_DEFAULT) {
//-/                        pDstSet->ClearItem(nWhich);
//-/                    } else if (eState==SFX_ITEM_SET) {
//-/                        if (nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) {
//-/                            pDstSet->Put(*pItem);
//-/                        }
//-/                    }
//-/                    nWhich=aWhIter.NextWhich();
//-/                }
//-/                if (pNewSI!=NULL) {
//-/                    SetSetItem(i,(SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
//-/                    delete pNewSI;
//-/                }
//-/            }
//-/        }
//-/    } else {
//-/        SfxItemIter aItIter(rAttr);
//-/        for (USHORT i=0; i<nAnz; i++) {
//-/            const SfxSetItem* pOldSI=GetSetItem(i);
//-/            if (pOldSI!=NULL) {
//-/                const SfxItemSet& rOldIS=pOldSI->GetItemSet();
//-/                const USHORT* pRanges=rOldIS.GetRanges();
//-/                FASTBOOL bOneRange=pRanges[2]==0;
//-/                USHORT nAnf=pRanges[0];
//-/                USHORT nEnd=pRanges[1];
//-/                SfxSetItem* pNewSI=NULL; //MakeNewSetItem(i,TRUE);
//-/                SfxItemSet* pDstSet=NULL; //pNewSI->GetItemSet();
//-/                // funkt nicht:
//-/                //pNewSI->GetItemSet().Put(rAttr,FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
//-/                // deshalb manuell. (Bug 21223)
//-/                const SfxPoolItem* pItem=aItIter.FirstItem();
//-/                BOOL bFillAttrChangeHint(FALSE);
//-/
//-/                while (pItem!=NULL) {
//-/                    if (!IsInvalidItem(pItem)) {
//-/                        USHORT nWhich=pItem->Which();
//-/                        if ((nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) &&
//-/                            (!bOneRange || (nWhich>=nAnf && nWhich<=nEnd))) {
//-/                            if (pNewSI==NULL) {
//-/                                pNewSI=MakeNewSetItem(i,TRUE);
//-/                                pDstSet=&pNewSI->GetItemSet();
//-/                            }
//-/
//-/                            // Hinweis auf Veraenderung FillStyle
//-/                            if(i==1 && nWhich == XATTR_FILLSTYLE)
//-/                                bFillAttrChangeHint = TRUE;
//-/
//-/                            pDstSet->Put(*pItem);
//-/                        }
//-/                    }
//-/                    pItem=aItIter.NextItem();
//-/                }
//-/
//-/                if (pNewSI!=NULL)
//-/                {
//-/                    if(bFillAttrChangeHint)
//-/                    {
//-/                        // FillStyle hat sich eventuell geaendert
//-/                        XFillStyle eFillStyle = ((XFillStyleItem&)(rAttr.Get(XATTR_FILLSTYLE))).GetValue();
//-/
//-/                        // Alle vom momentanen Stil nicht benutzten Items
//-/                        // rauswerfen. Dies gilt NICHT fuer die Fuellfarbe
//-/                        // (XFILL_SOLID). Diese wird als immer vorhandene
//-/                        // Grundfarbe des Objektes angesehen (wird von 3D
//-/                        // bereits so verwendet)
//-/
//-/                        if(eFillStyle != XFILL_GRADIENT)
//-/                            pDstSet->ClearItem(XATTR_FILLGRADIENT);
//-/
//-/                        if(eFillStyle != XFILL_HATCH)
//-/                            pDstSet->ClearItem(XATTR_FILLHATCH);
//-/
//-/                        if(eFillStyle != XFILL_BITMAP)
//-/                            pDstSet->ClearItem(XATTR_FILLBITMAP);
//-/                    }
//-/
//-/                    SetSetItem(i,(SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
//-/                    delete pNewSI;
//-/                }
//-/            }
//-/        }
//-/    }
//-/    bBoundRectDirty=TRUE;
//-/    SetRectsDirty(TRUE);
//-/}

//-/void SdrAttrObj::SetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    Rectangle aBoundRect0;
//-/
//-/    if(pUserCall)
//-/        aBoundRect0 = GetBoundRect();
//-/
//-/    SendRepaintBroadcast();
//-/    NbcSetAttributes(rAttr, bReplaceAll);
//-/    SetChanged();
//-/    SendRepaintBroadcast();
//-/    SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

// ItemSet access
const SfxItemSet& SdrAttrObj::GetUnmergedItemSet() const
{
    return SdrAttrObj::GetItemSet();
}

const SfxItemSet& SdrAttrObj::GetItemSet() const
{
    ((SdrAttrObj*)this)->ImpForceItemSet();
    return *mpObjectItemSet;
}

void SdrAttrObj::SetItem(const SfxPoolItem& rItem)
{
    ((SdrAttrObj*)this)->ImpForceItemSet();
    mpObjectItemSet->Put(rItem);
    bBoundRectDirty = TRUE;
    SetRectsDirty(TRUE);
    SetChanged();
}

void SdrAttrObj::ClearItem( USHORT nWhich )
{
    if(mpObjectItemSet)
    {
        mpObjectItemSet->ClearItem(nWhich);
        bBoundRectDirty = TRUE;
        SetRectsDirty(TRUE);
        SetChanged();
    }
}

void SdrAttrObj::SetItemSet( const SfxItemSet& rSet )
{
    ((SdrAttrObj*)this)->ImpForceItemSet();
    mpObjectItemSet->Put(rSet);
    bBoundRectDirty = TRUE;
    SetRectsDirty(TRUE);
    SetChanged();
}

SfxItemSet* SdrAttrObj::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items, 2D and 3D
    return new SfxItemSet(rPool,
        // ranges from SdrAttrObj
        SDRATTR_START, SDRATTRSET_SHADOW,
        SDRATTRSET_OUTLINER, SDRATTRSET_MISC,

        // outliner and end
        EE_ITEMS_START, EE_ITEMS_END,
        0, 0);
}

//-/void SdrAttrObj::BroadcastItemChange(const SdrBroadcastItemChange& rChange)
//-/{
//-/    Rectangle aBoundRect0;
//-/
//-/    if(pUserCall)
//-/        aBoundRect0 = GetBoundRect();
//-/
//-/    SendRepaintBroadcast();
//-/    NbcSetAttributes(rAttr, bReplaceAll);
//-/    SetChanged();
//-/    SendRepaintBroadcast();
//-/    SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
    const SfxHint& rHint, const TypeId& rHintType)
{
    SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    BOOL bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());
    SfxStyleSheetHint *pStyleHint = PTR_CAST(SfxStyleSheetHint, &rHint);
    BOOL bDying(pStyleHint && SFX_STYLESHEET_INDESTRUCTION == pStyleHint->GetHint());

    if(bDataChg || bDying)
    {
        Rectangle aBoundRect0;

        if(pUserCall)
            aBoundRect0 = GetBoundRect();

        SfxStyleSheet* pNewStSh = NULL;
        BOOL bBoundRectDirty0 = bBoundRectDirty;

        if(bDying)
        {
            // wenn es einen Parent gibt, wird jetzt der genommen
            if(pModel && HAS_BASE(SfxStyleSheet, GetStyleSheet()))
            {
                // Sonst ist pStyleSheet schon zu weit weggestorben
                pNewStSh = (SfxStyleSheet*)pModel->GetStyleSheetPool()->Find(
                    GetStyleSheet()->GetParent(), GetStyleSheet()->GetFamily());
            }

            // wenn es keinen Parent gibt, nehmen wir die Standardvorlage
            if(!pNewStSh && pModel)
                pNewStSh = pModel->GetDefaultStyleSheet();

            // alten StyleSheet vor SendRepaintBroadcast entfernen #42276#
            RemoveStyleSheet(); // setzt bBoundRectDirty=TRUE
        }

        if(!bBoundRectDirty0)
        {
            bBoundRectDirty = FALSE;  // fuer Broadcast mit dem alten Rect

            if(pPage && pPage->IsInserted())
                SendRepaintBroadcast(); // Erstmal mit dem alten Rect

            bBoundRectDirty = TRUE;
            SetRectsDirty(TRUE);    // Durch Vorlagenloeschung evtl. andere Linienbreite
        }

        if(pNewStSh && !bDying)
            AddStyleSheet(pNewStSh, TRUE);

        if(pPage && pPage->IsInserted())
            SendRepaintBroadcast();

        SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
    }
}

SfxStyleSheet* SdrAttrObj::GetStyleSheet() const
{
    return mpStyleSheet;
}

void SdrAttrObj::RemoveStyleSheet()
{
    // Typ checken, weil bei dying sich der Typ abbaut (bis zum Broadcaster runter)
    if(GetStyleSheet() && HAS_BASE(SfxStyleSheet, mpStyleSheet))
    {
        EndListening(*mpStyleSheet);
        EndListening(mpStyleSheet->GetPool());

        // das ItemSet der Vorlage ist jetzt nicht mehr Parent der
        // eigenen ItemSets
        if(mpObjectItemSet)
            mpObjectItemSet->SetParent(NULL);

//-/        USHORT nAnz=GetSetItemCount();
//-/        for (USHORT i=0; i<nAnz; i++) {
//-/            SfxSetItem* pNewSI=MakeNewSetItem(i,TRUE);
//-/            if (pNewSI!=NULL) {
//-/                pNewSI->GetItemSet().SetParent(NULL);
//-/                const SfxSetItem* pOldSI=GetSetItem(i);
//-/                // Alle Items aus der Vorlage von pOldSI nach pNewSI putten,
//-/                // also hart Attributieren.
//-/                // ... fehlende Implementation
//-/
//-/                const SfxSetItem* pNewSI2=(const SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI);
//-/                SetSetItem(i,pNewSI2);
//-/                delete pNewSI;
//-/            }
//-/        }
        bBoundRectDirty = TRUE;
        SetRectsDirty(TRUE);
    }
//-/
    mpStyleSheet = NULL;
}

//-/USHORT SdrAttrObj::GetSetItemCount() const
//-/{
//-/    return 6;
//-/}

//-/const SfxSetItem* SdrAttrObj::GetSetItem(USHORT nNum) const
//-/{
//-/    switch (nNum)
//-/    {
//-/        case 0 : return pLineAttr;
//-/        case 1 : return pFillAttr;
//-/        case 2 : return pTextAttr;
//-/        case 3 : return pShadAttr;
//-/        case 4 : return pOutlAttr;
//-/        case 5 : return pMiscAttr;
//-/    }
//-/    return NULL; // damit der Compiler nicht meckert
//-/}

//-/void SdrAttrObj::SetSetItem(USHORT nNum, const SfxSetItem* pAttr)
//-/{
//-/    switch (nNum)
//-/    {
//-/        case 0 : pLineAttr = (const XLineAttrSetItem*)  pAttr; break;
//-/        case 1 : pFillAttr = (const XFillAttrSetItem*)  pAttr; break;
//-/        case 2 : pTextAttr = (const XTextAttrSetItem*)  pAttr; break;
//-/        case 3 : pShadAttr = (const SdrShadowSetItem*)  pAttr; break;
//-/        case 4 : pOutlAttr = (const SdrOutlinerSetItem*)pAttr; break;
//-/        case 5 : pMiscAttr = (const SdrMiscSetItem*)    pAttr; break;
//-/    }
//-/}

//-/SfxSetItem* SdrAttrObj::MakeNewSetItem(USHORT nNum, FASTBOOL bClone) const
//-/{
//-/    if (bClone)
//-/    {
//-/        switch (nNum)
//-/        {
//-/            case 0 : return new XLineAttrSetItem(*pLineAttr);
//-/            case 1 : return new XFillAttrSetItem(*pFillAttr);
//-/            case 2 : return new XTextAttrSetItem(*pTextAttr);
//-/            case 3 : return new SdrShadowSetItem(*pShadAttr);
//-/            case 4 : return new SdrOutlinerSetItem(*pOutlAttr);
//-/            case 5 : return new SdrMiscSetItem(*pMiscAttr);
//-/        }
//-/    }
//-/    else
//-/    {
//-/        switch (nNum)
//-/        {
//-/            case 0 : return new XLineAttrSetItem(GetItemPool());
//-/            case 1 : return new XFillAttrSetItem(GetItemPool());
//-/            case 2 : return new XTextAttrSetItem(GetItemPool());
//-/            case 3 : return new SdrShadowSetItem(GetItemPool());
//-/            case 4 : return new SdrOutlinerSetItem(GetItemPool());
//-/            case 5 : return new SdrMiscSetItem(GetItemPool());
//-/        }
//-/    }
//-/    return NULL; // damit der Compiler nicht meckert
//-/}

void SdrAttrObj::AddStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    // old StyleSheet is deleted
    DBG_ASSERT(!mpStyleSheet, "Old style sheet not deleted before setting new one (?)");

    if(pNewStyleSheet)
    {
        mpStyleSheet = pNewStyleSheet;

        // ItemSet is needed here, force it
        ImpForceItemSet();

        // als Listener anmelden
        StartListening(pNewStyleSheet->GetPool());
        StartListening(*pNewStyleSheet);

        // harte Attributierung dort loeschen, wo was in der Vorlage steht
        if(!bDontRemoveHardAttr)
        {
            const SfxItemSet& rStyle = pNewStyleSheet->GetItemSet();
            SfxWhichIter aIter(rStyle);
            sal_uInt16 nWhich = aIter.FirstWhich();

            while(nWhich)
            {
                if(SFX_ITEM_SET == rStyle.GetItemState(nWhich))
                    mpObjectItemSet->ClearItem(nWhich);
                nWhich = aIter.NextWhich();
            }
        }

        // set new stylesheet as parent
        mpObjectItemSet->SetParent(&pNewStyleSheet->GetItemSet());
    }
//-/    pStyleSheet=pNewStyleSheet;
//-/    if(pNewStyleSheet && GetStyleSheet() != pNewStyleSheet)
//-/    {
//-/        // als Listener anmelden
//-/        StartListening(pNewStyleSheet->GetPool());
//-/        StartListening(*pNewStyleSheet);
//-/
//-/        // harte Attributierung dort loeschen, wo was in der Vorlage steht?
//-/        const SfxItemSet& rStyle = pNewStyleSheet->GetItemSet();
//-/
//-/        // pWhichRangesInStyle enthaelt Zahlenpaare der Which-Ranges.
//-/        // am Ende steht eine 0.
//-/        const sal_uInt16* pWhichRangesInStyle = rStyle.GetRanges();
//-/        sal_uInt16 nAnz(GetSetItemCount());
//-/
//-/        for(sal_uInt16 i(0); i < nAnz; i++)
//-/        {
//-/            const SfxSetItem* pOldSI=GetSetItem(i);
//-/            SfxSetItem* pNewSI=MakeNewSetItem(i,pOldSI!=NULL);
//-/            pNewSI->GetItemSet().SetParent(NULL);
//-/            SfxItemSet& rIS=pNewSI->GetItemSet();
//-/
//-/            USHORT  nIndex  =0;
//-/            USHORT  nCurrent=pWhichRangesInStyle[nIndex];
//-/            USHORT  nEnd    =pWhichRangesInStyle[nIndex+1];
//-/
//-/            while (nCurrent!=0) {
//-/                if (!bDontRemoveHardAttr && rStyle.GetItemState(nCurrent)==SFX_ITEM_SET) {
//-/                    rIS.ClearItem(nCurrent);
//-/                }
//-/
//-/                nCurrent++;
//-/                if (nCurrent>nEnd) {
//-/                    nIndex+=2;
//-/                    nCurrent=pWhichRangesInStyle[nIndex];
//-/                    if (nCurrent!=0) {         // Ende der Ranges? nein
//-/                        nEnd=pWhichRangesInStyle[nIndex+1];
//-/                    }
//-/                }
//-/            }
//-/            // ItemSet des StyleSheets als Parent der ItemSets der SetItems setzen
//-/            rIS.SetParent(&(pNewStyleSheet->GetItemSet()));
//-/            // die Zeiger auf die SetItems neu setzen
//-/            SetSetItem(i,(const SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
//-/            delete pNewSI;
//-/        }
//-/    }
}

void SdrAttrObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    RemoveStyleSheet();
    AddStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
    bBoundRectDirty = TRUE;
    SetRectsDirty(TRUE);
}

void SdrAttrObj::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    Rectangle aBoundRect0;

    if(pUserCall)
        aBoundRect0 = GetBoundRect();

    SendRepaintBroadcast();
    NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
}

INT32 SdrAttrObj::ImpGetLineWdt() const
{
    const SfxItemSet& rSet = GetItemSet();
    XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();

    if(XLINE_NONE == eLine)
        return 0; // Garkeine Linie da.

    sal_Int32 nWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();

    return nWdt;
}

INT32 SdrAttrObj::ImpGetLineEndAdd() const
{
    const SfxItemSet& rSet = GetItemSet();
    BOOL bStartSet(TRUE);
    BOOL bEndSet(TRUE);

    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINESTART))
    {
        String aStr(((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetName());
        if(!aStr.Len())
            bStartSet = FALSE;
    }

    if(rSet.GetItemState(XATTR_LINEEND) != SFX_ITEM_DONTCARE)
    {
        String aStr(((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetName());
        if(!aStr.Len())
            bEndSet = FALSE;
    }

    BOOL bLineEndSet = bStartSet || bEndSet;
    XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();

    if(XLINE_NONE == eLine)
        return 0; // Garkeine Linie da.

    // Strichstaerke
    sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
    sal_Int32 nSttWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();

    if(nSttWdt < 0)
        nSttWdt = -nLineWdt * nSttWdt / 100;

    if(!bLineEndSet)
        nSttWdt = 0;

    BOOL bSttCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    sal_Int32 nSttHgt = 0;

    if(bSttCenter)
    {
        // Linienende steht um die Haelfe ueber
        XPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
        nSttHgt = XOutputDevice::InitLineStartEnd(aSttPoly, nSttWdt, bSttCenter);
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    }

    nSttWdt++;
    nSttWdt /= 2;

    // Lieber etwas mehr, dafuer keine Wurzel ziehen
    long nSttAdd = Max(nSttWdt, nSttHgt);
    nSttAdd *= 3;
    nSttAdd /= 2;

    long nEndWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();

    if(nEndWdt < 0)
        nEndWdt = -nLineWdt * nEndWdt / 100; // <0 = relativ

    if(!bLineEndSet)
        nEndWdt = 0;

    BOOL bEndCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
    sal_Int32 nEndHgt = 0;

    if(bEndCenter)
    {
        // Linienende steht um die Haelfe ueber
        XPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
        nEndHgt = XOutputDevice::InitLineStartEnd(aEndPoly, nEndWdt, bEndCenter);
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    }

    nEndWdt++;
    nEndWdt /= 2;

    // Lieber etwas mehr, dafuer keine Wurzel ziehen
    sal_Int32 nEndAdd = Max(nEndWdt, nEndHgt);
    nEndAdd *= 3;
    nEndAdd /= 2;

    return Max(nSttAdd, nEndAdd);
}

//////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrAttrObj::ImpLineEndHitTest(const Point& rEndPt, double nSin, double nCos, FASTBOOL bStart, const Point& rHit, USHORT nTol) const
{
    const SfxItemSet& rSet = GetItemSet();
    sal_Int32 nWdt = 0;
    BOOL bCenter = FALSE;
    XPolygon aXPoly;

    if(bStart)
    {
        nWdt = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
        bCenter = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
        aXPoly = ((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue();
    }
    else
    {
        nWdt = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
        bCenter = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
        aXPoly = ((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue();
    }

    if(nWdt < 0)
    {
        sal_Int32 nLineWdt = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
        nWdt = -nLineWdt * nWdt / 100; // <0 = relativ
    }

    // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
    XOutputDevice::InitLineStartEnd(aXPoly, nWdt, bCenter);
    RotateXPoly(aXPoly, Point(), nSin, nCos);
    Point aHit(rHit);
    aHit -= rEndPt;
    Rectangle aHitRect(aHit.X() - nTol, aHit.Y() - nTol, aHit.X() + nTol, aHit.Y() + nTol);
    FASTBOOL bHit = IsRectTouchesPoly(XOutCreatePolygon(aXPoly, NULL), aHitRect);

    return bHit;
}

FASTBOOL SdrAttrObj::ImpGetShadowDist(sal_Int32& nXDist, sal_Int32& nYDist) const
{
    const SfxItemSet& rSet = GetItemSet();

    nXDist = 0L;
    nYDist = 0L;

    BOOL bShadOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();
    if(bShadOn)
    {
        nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
        return TRUE;
    }

    return FALSE;
}

void SdrAttrObj::ImpAddShadowToBoundRect()
{
    sal_Int32 nXDist;
    sal_Int32 nYDist;

    if(ImpGetShadowDist(nXDist, nYDist))
    {
        if(nXDist > 0)
            aOutRect.Right() += nXDist;
        else
            aOutRect.Left() += nXDist;

        if(nYDist > 0)
            aOutRect.Bottom() += nYDist;
        else
            aOutRect.Top() += nYDist;
    }
}

FASTBOOL SdrAttrObj::ImpSetShadowAttributes(ExtOutputDevice& rXOut, FASTBOOL bNoFill) const
{
    const SfxItemSet& rSet = GetItemSet();
    BOOL bShadOn=((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();

    if(bShadOn)
    {
// LineAttr for shadow no longer necessary, lines and line shadows are drawn in Paint()
// routines individually (grep for CreateLinePoly())
//
//          if (pLineAttr!=NULL) {
//              XLineAttrSetItem aL(*pLineAttr);
//              aL.GetItemSet().Put(XLineColorItem(String(),aShadCol));
//              aL.GetItemSet().Put(XLineTransparenceItem(nTransp));
//              rXOut.SetLineAttr(aL);
//          }
        if(!bNoFill)
        {
            const SdrShadowColorItem& rShadColItem = ((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
            Color aShadCol(rShadColItem.GetValue());
            sal_uInt16 nTransp = ((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
            XFillStyle eStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

//-/            XFillAttrSetItem aF(rSet.GetPool());
            SfxItemSet aSet(rSet);
//-/            aF.GetItemSet().Put(rSet);

            if(eStyle==XFILL_HATCH)
            {
                // #41666#
                XHatch aHatch = ((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetValue();
                aHatch.SetColor(aShadCol);
                aSet.Put(XFillHatchItem(String(), aHatch));
            }
            else
            {
                if(eStyle != XFILL_NONE && eStyle != XFILL_SOLID)
                {
                    // also fuer Gradient und Bitmap
                    aSet.Put(XFillStyleItem(XFILL_SOLID));
                }

                aSet.Put(XFillColorItem(String(),aShadCol));
                aSet.Put(XFillTransparenceItem(nTransp));
            }

            rXOut.SetFillAttr(aSet);
        }
        return TRUE;
    }

    return FALSE;
}

void SdrAttrObj::BurnInStyleSheetAttributes()
{
//-/    SfxItemPool* pPool = GetItemPool();
//-/
//-/    if(pPool)
//-/    {
//-/        SfxItemSet aSet(*pPool,
//-/            SDRATTR_START,              SDRATTR_NOTPERSIST_FIRST-1,
//-/            SDRATTR_NOTPERSIST_LAST+1,  SDRATTR_END,
//-/            EE_ITEMS_START,             EE_ITEMS_END,
//-/            0, 0);
//-/
//-/        TakeAttributes(aSet,FALSE,FALSE);
//-/        NbcSetAttributes(aSet,FALSE);
//-/    }
}

//-/void SdrAttrObj::CopyAttributes(SdrObject* pDestObj) const
//-/{
//-/    // ...
//-/}

// ItemPool fuer dieses Objekt wechseln
void SdrAttrObj::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool)
{
    if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
    {
        // call parent
        SdrObject::MigrateItemPool(pSrcPool, pDestPool);

        // eigene Reaktion
        if(pDestPool && pSrcPool && (pDestPool != pSrcPool))
        {
            if(mpObjectItemSet)
            {
                // migrate ItemSet to new pool. Scaling is NOT necessary
                // because this functionality is used by UNDO only. Thus
                // objects and ItemSets would be moved back to their original
                // pool before usage.
                SfxItemSet* pNewSet = mpObjectItemSet->Clone(TRUE, pDestPool);
                SfxStyleSheet* pStySheet = GetStyleSheet();

                if(GetStyleSheet())
                    pNewSet->SetParent(&GetStyleSheet()->GetItemSet());

                // set new itemset
                ImpDeleteItemSet();
                mpObjectItemSet = pNewSet;

                // set stylesheet (if used)
                if(pStySheet)
                    AddStyleSheet(pStySheet, TRUE);
            }
        }
    }
//-/        UINT16 nItemSetAnz = GetSetItemCount();
//-/        for(UINT16 i=0;i<nItemSetAnz;i++)
//-/        {
//-/            const SfxSetItem* pSI = GetSetItem(i);
//-/            if(pSI != NULL)
//-/            {
//-/                const SfxSetItem* pNewAttr = &(const SfxSetItem&)pDestPool->Put(*pSI);
//-/                pSrcPool->Remove(*pSI);
//-/                SetSetItem(i, pNewAttr);
//-/            }
//-/        }
}

BOOL SdrAttrObj::HasFill() const
{
    return (!bClosedObj) ? FALSE
        : ((XFillStyleItem&)(GetItem(XATTR_FILLSTYLE))).GetValue()!=XFILL_NONE;
}

BOOL SdrAttrObj::HasLine() const
{
    return ((XLineStyleItem&)(GetItem(XATTR_LINESTYLE))).GetValue()!=XLINE_NONE;
}
