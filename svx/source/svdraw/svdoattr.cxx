/*************************************************************************
 *
 *  $RCSfile: svdoattr.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-05 12:26:22 $
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

#include <charscaleitem.hxx>

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

#ifndef _SVX_XFLBCKIT_HXX
#include "xflbckit.hxx"
#endif

#ifndef _SVX_XBTMPIT_HXX
#include "xbtmpit.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_XLNDSIT_HXX
#include "xlndsit.hxx"
#endif

#ifndef _SVX_XFLGRIT_HXX
#include "xflgrit.hxx"
#endif

#ifndef _SVX_XFLFTRIT_HXX
#include "xflftrit.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrAttrObj,SdrObject);

SdrAttrObj::SdrAttrObj()
:   mpStyleSheet(NULL),
    mpObjectItemSet(NULL)
{
}

SdrAttrObj::~SdrAttrObj()
{
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
        ImpDeleteItemSet();

        if(pAO->mpObjectItemSet)
            mpObjectItemSet = pAO->mpObjectItemSet->Clone(TRUE);

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
            if(pPool)
            {
                SfxStyleSheet *pTmpStyleSheet = (SfxStyleSheet*)pPool->Find(aStyleSheetName, eFamily);
                DBG_ASSERT(pTmpStyleSheet, "SdrAttrObj::ReadData(): StyleSheet nicht gefunden");

                if(pTmpStyleSheet)
                    AddStyleSheet(pTmpStyleSheet, TRUE);
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
    const SfxItemSet& rSet = GetUnmergedItemSet();
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
        const SfxItemSet& rSet = GetUnmergedItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_LINE));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_FILL));
        pPool->StoreSurrogate(rOut, &rSet.Get(XATTRSET_TEXT));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_SHADOW));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_OUTLINER));
        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_MISC));
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
            SfxStyleSheet* pOldStyleSheet = GetStyleSheet();

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

                    if(bScaleUnitChanged)
                        ImpScaleItemSet(*pNewSet, aMetricFactor);

                    ImpDeleteItemSet();
                    mpObjectItemSet = pNewSet;
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
    SdrTextObj* pText = PTR_CAST(SdrTextObj, this);
    BOOL bTextFrame(pText && pText->IsTextFrame());

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
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

const SfxItemSet& SdrAttrObj::GetItemSet() const
{
    ((SdrAttrObj*)this)->ImpForceItemSet();
    return *mpObjectItemSet;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses

const SfxItemSet& SdrAttrObj::GetUnmergedItemSet() const
{
    return SdrAttrObj::GetItemSet();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

const SfxPoolItem* ImplCheckFillBitmapItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XFillBitmapItem* pBitmapItem = (XFillBitmapItem*)pNewItem;

    const GraphicObject& xGraphic = pBitmapItem->GetValue().GetGraphicObject();
    ByteString aUniqueID = xGraphic.GetUniqueID();

    String aUniqueName( pBitmapItem->GetName() );

    // 1. if we have no name check if we have the same bitmap
    // inside the bitmap table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XBitmapList* pBitmapList = pModel->GetBitmapList();
        if( pBitmapList )
        {
            const long nCount = pBitmapList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XBitmapEntry* pEntry = pBitmapList->Get(nIndex);
                if( pEntry->GetXBitmap().GetGraphicObject().GetUniqueID() == aUniqueID )
                    return new XFillBitmapItem( pEntry->GetName(), pEntry->GetXBitmap() );
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different bitmap
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        const USHORT nCount = rPool.GetItemCount( XATTR_FILLBITMAP );
        const XFillBitmapItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillBitmapItem*)rPool.GetItem( XATTR_FILLBITMAP, nSurrogate );

            if( pItem && ( pItem->GetName() == pBitmapItem->GetName() ) )
            {
                // if there is already an item with the same name and the same bitmap
                // its ok to set it
                if( pItem->GetValue().GetGraphicObject().GetUniqueID() == aUniqueID )
                    break;

                // same name but different bitmap, we need a new name for this item
                aUniqueName = String();
                break;
            }
        }
    }
    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        const USHORT nCount = rPool.GetItemCount( XATTR_FILLBITMAP );
        const XFillBitmapItem *pItem;

        sal_Int32 nUserIndex = 1;

        const String aUser( RTL_CONSTASCII_STRINGPARAM( "bitmap" ) );
        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillBitmapItem*)rPool.GetItem( XATTR_FILLBITMAP, nSurrogate );

            if( pItem && pItem->GetName().Len() )
            {
                if( pItem->GetValue().GetGraphicObject().GetUniqueID() == aUniqueID )
                    return new XFillBitmapItem( *pItem );

                if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                {
                    sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                    if( nThisIndex >= nUserIndex )
                        nUserIndex = nThisIndex + 1;
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        return new XFillBitmapItem( aUniqueName, pBitmapItem->GetValue() );
    }

    return pNewItem;
}

const SfxPoolItem* ImplCheckLineDashItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XLineDashItem* pLineDashItem = (XLineDashItem*)pNewItem;

    String aUniqueName( pLineDashItem->GetName() );

    // 1. if we have no name check if we have the same dash
    // inside the dash table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XDashList* pDashList = pModel->GetDashList();
        if( pDashList )
        {
            const long nCount = pDashList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XDashEntry* pEntry = pDashList->Get(nIndex);
                if( pEntry->GetDash() == pLineDashItem->GetValue() )
                    return new XLineDashItem( pEntry->GetName(), pEntry->GetDash() );
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different dash
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        const USHORT nCount = rPool.GetItemCount( XATTR_LINEDASH );
        const XLineDashItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XLineDashItem*)rPool.GetItem( XATTR_LINEDASH, nSurrogate );

            if( pItem && ( pItem->GetName() == pLineDashItem->GetName() ) )
            {
                // if there is already an item with the same name and the same bitmap
                // its ok to set it
                if( pItem->GetValue() == pLineDashItem->GetValue() )
                    break;

                // same name but different bitmap, we need a new name for this item
                aUniqueName = String();
                break;
            }
        }
    }
    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        const USHORT nCount = rPool.GetItemCount( XATTR_LINEDASH );
        const XLineDashItem *pItem;

        sal_Int32 nUserIndex = 1;

        const String aUser( RTL_CONSTASCII_STRINGPARAM( "dash" ) );
        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XLineDashItem*)rPool.GetItem( XATTR_LINEDASH, nSurrogate );

            if( pItem && pItem->GetName().Len() )
            {
                if( pItem->GetValue() == pLineDashItem->GetValue() )
                    return new XLineDashItem( *pItem );

                if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                {
                    sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                    if( nThisIndex >= nUserIndex )
                        nUserIndex = nThisIndex + 1;
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        return new XLineDashItem( aUniqueName, pLineDashItem->GetValue() );
    }

    return pNewItem;
}

const SfxPoolItem* ImplCheckLineStartItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XLineStartItem* pClosedItem = NULL;
    XLineStartItem* pLineStartItem = (XLineStartItem*)pNewItem;

    String aUniqueName( pLineStartItem->GetName() );

    const XPolygon& rPoly = pLineStartItem->GetValue();
    if( rPoly.GetPointCount() == 0 )
    {
        // if the polygon is empty, check if the name is empty
        if( aUniqueName.Len() == 0 )
            return pNewItem;

        // force empty name for empty polygons
        return new XLineEndItem( String(), rPoly );
    }

    if( rPoly.GetPointCount() > 1 )
    {
        // check if the polygon is closed
        if( rPoly[0] != rPoly[rPoly.GetPointCount() - 1] )
        {
            // force a closed polygon
            XPolygon aNewPolygon( rPoly );
            aNewPolygon[ rPoly.GetPointCount() ] = rPoly[0];
            pClosedItem = new XLineStartItem( aUniqueName, aNewPolygon );
            pLineStartItem = pClosedItem;
        }
    }

    // 1. if we have no name check if we have the same dash
    // inside the dash table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XLineEndList* pLineEndList = pModel->GetLineEndList();
        if( pLineEndList )
        {
            const long nCount = pLineEndList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XLineEndEntry* pEntry = pLineEndList->Get(nIndex);
                if( pEntry->GetLineEnd() == pLineStartItem->GetValue() )
                {
                    if( pClosedItem )
                        delete pClosedItem;

                    return new XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() );
                }
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINESTART );
            const XLineStartItem *pItem;

            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineStartItem*)rPool.GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same value
                    // its ok to set it
                    if( pItem->GetValue() == pLineStartItem->GetValue() )
                        break;

                    // same name but different end, we need a new name for this item
                    aUniqueName = String();
                    break;
                }
            }
        }

        if( aUniqueName.Len() != 0 )
        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINEEND );
            const XLineEndItem *pItem;

            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineEndItem*)rPool.GetItem( XATTR_LINEEND, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() == pLineStartItem->GetValue() )
                        break;

                    // same name but different bitmap, we need a new name for this item
                    aUniqueName = String();
                    break;
                }
            }
        }
    }

    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        sal_Int32 nUserIndex = 1;
        const String aUser( RTL_CONSTASCII_STRINGPARAM( "marker" ) );

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINESTART );
            const XLineStartItem *pItem;
            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineStartItem*)rPool.GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( pItem->GetValue() == pLineStartItem->GetValue() )
                    {
                        if( pClosedItem )
                            delete pClosedItem;
                        return new XLineStartItem( *pItem );
                    }
                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINEEND );
            const XLineEndItem *pItem;
            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineEndItem*)rPool.GetItem( XATTR_LINEEND, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( pItem->GetValue() == pLineStartItem->GetValue() )
                    {
                        if( pClosedItem )
                            delete pClosedItem;
                        return new XLineStartItem( pItem->GetName(), pItem->GetValue() );
                    }
                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }
        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        pLineStartItem = new XLineStartItem( aUniqueName, pLineStartItem->GetValue() );
        if( pClosedItem ) delete pClosedItem;
    }

    return pLineStartItem;
}

const SfxPoolItem* ImplCheckLineEndItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XLineEndItem* pClosedItem = NULL;
    XLineEndItem* pLineEndItem = (XLineEndItem*)pNewItem;

    String aUniqueName( pLineEndItem->GetName() );

    // check if the polygon is closed, if not, close it!
    const XPolygon& rPoly = pLineEndItem->GetValue();
    if( rPoly.GetPointCount() == 0 )
    {
        // if the polygon is empty, check if the name is empty
        if( aUniqueName.Len() == 0 )
            return pNewItem;

        // force empty name for empty polygons
        return new XLineEndItem( String(), rPoly );

    }

    if( rPoly.GetPointCount() > 1 )
    {
        if( rPoly[0] != rPoly[rPoly.GetPointCount() - 1] )
        {
            // force a closed polygon
            XPolygon aNewPolygon( rPoly );
            aNewPolygon[ rPoly.GetPointCount() ] = rPoly[0];
            pClosedItem = new XLineEndItem( aUniqueName, aNewPolygon );
            pLineEndItem = pClosedItem;
        }
    }

    // 1. if we have no name check if we have the same value
    // inside the line end table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XLineEndList* pLineEndList = pModel->GetLineEndList();
        if( pLineEndList )
        {
            const long nCount = pLineEndList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XLineEndEntry* pEntry = pLineEndList->Get(nIndex);
                if( pEntry->GetLineEnd() == pLineEndItem->GetValue() )
                {
                    if( pClosedItem )
                        delete pClosedItem;
                    return new XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() );
                }
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINEEND );
            const XLineEndItem *pItem;

            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineEndItem*)rPool.GetItem( XATTR_LINEEND, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() == pLineEndItem->GetValue() )
                        break;

                    // same name but different end, we need a new name for this item
                    aUniqueName = String();
                    break;
                }
            }
        }

        if( aUniqueName.Len() != 0 )
        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINESTART );
            const XLineStartItem *pItem;

            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineStartItem*)rPool.GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
                {
                    // if there is already an item with the same name and the same
                    // value its ok to set it
                    if( pItem->GetValue() == pLineEndItem->GetValue() )
                        break;

                    // same name but different bitmap, we need a new name for this item
                    aUniqueName = String();
                    break;
                }
            }
        }
    }
    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        sal_Int32 nUserIndex = 1;
        const String aUser( RTL_CONSTASCII_STRINGPARAM( "marker" ) );

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINEEND );
            const XLineEndItem *pItem;
            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineEndItem*)rPool.GetItem( XATTR_LINEEND, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( pItem->GetValue() == pLineEndItem->GetValue() )
                    {
                        if( pClosedItem )
                            delete pClosedItem;
                        return new XLineEndItem( *pItem );
                    }
                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        {
            const USHORT nCount = rPool.GetItemCount( XATTR_LINESTART );
            const XLineStartItem *pItem;
            for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
            {
                pItem = (XLineStartItem*)rPool.GetItem( XATTR_LINESTART, nSurrogate );

                if( pItem && pItem->GetName().Len() )
                {
                    if( pItem->GetValue() == pLineEndItem->GetValue() )
                    {
                        if( pClosedItem )
                            delete pClosedItem;
                        return new XLineEndItem( pItem->GetName(), pItem->GetValue() );
                    }
                    if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                    {
                        sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                        if( nThisIndex >= nUserIndex )
                            nUserIndex = nThisIndex + 1;
                    }
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        pLineEndItem = new XLineEndItem( aUniqueName, pLineEndItem->GetValue() );
        if( pClosedItem )
            delete pClosedItem;
    }

    return pLineEndItem;
}

const SfxPoolItem* ImplCheckFillGradientItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XFillGradientItem* pFillGradientItem = (XFillGradientItem*)pNewItem;

    String aUniqueName( pFillGradientItem->GetName() );

    // 1. if we have no name check if we have the same dash
    // inside the dash table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XGradientList* pGradientList = pModel->GetGradientList();
        if( pGradientList )
        {
            const long nCount = pGradientList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XGradientEntry* pEntry = pGradientList->Get(nIndex);
                if( pEntry->GetGradient() == pFillGradientItem->GetValue() )
                    return new XFillGradientItem( pEntry->GetName(), pEntry->GetGradient() );
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();

        const USHORT nCount = rPool.GetItemCount( XATTR_FILLGRADIENT );
        const XFillGradientItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillGradientItem*)rPool.GetItem( XATTR_FILLGRADIENT, nSurrogate );

            if( pItem && ( pItem->GetName() == pFillGradientItem->GetName() ) )
            {
                // if there is already an item with the same name and the same bitmap
                // its ok to set it
                if( pItem->GetValue() == pFillGradientItem->GetValue() )
                    break;

                // same name but different end, we need a new name for this item
                aUniqueName = String();
                break;
            }
        }
    }
    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        sal_Int32 nUserIndex = 1;
        const String aUser( RTL_CONSTASCII_STRINGPARAM( "gradient" ) );

        const USHORT nCount = rPool.GetItemCount( XATTR_FILLGRADIENT );
        const XFillGradientItem *pItem;
        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillGradientItem*)rPool.GetItem( XATTR_FILLGRADIENT, nSurrogate );

            if( pItem && pItem->GetName().Len() )
            {
                if( pItem->GetValue() == pFillGradientItem->GetValue() )
                    return new XFillGradientItem( *pItem );

                if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                {
                    sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                    if( nThisIndex >= nUserIndex )
                        nUserIndex = nThisIndex + 1;
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        return new XFillGradientItem( aUniqueName, pFillGradientItem->GetValue() );
    }

    return pNewItem;
}

const SfxPoolItem* ImplCheckFillFloatTransparenceItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XFillFloatTransparenceItem* pFillFloatTransparenceItem = (XFillFloatTransparenceItem*)pNewItem;
    String aUniqueName(pFillFloatTransparenceItem->GetName());

    // 1. if we have a name check if there is already an item with the
    // same name in the documents pool with a different FillFloatTransparence
    if(aUniqueName.Len())
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        const sal_uInt16 nCount(rPool.GetItemCount(XATTR_FILLFLOATTRANSPARENCE));
        const XFillFloatTransparenceItem *pItem;

        for(sal_uInt16 nSurrogate(0); nSurrogate < nCount; nSurrogate++)
        {
            pItem = (XFillFloatTransparenceItem*)rPool.GetItem(XATTR_FILLFLOATTRANSPARENCE, nSurrogate);

            if(pItem && (pItem->GetName() == pFillFloatTransparenceItem->GetName()))
            {
                // if there is already an item with the same name and the same gradient
                // and the same activation its ok to set it
                if(pItem->IsEnabled() == pFillFloatTransparenceItem->IsEnabled() &&
                    pItem->GetValue() == pFillFloatTransparenceItem->GetValue())
                    break;

                // same name but different end, we need a new name for this item
                aUniqueName = String();
                break;
            }
        }
    }

    // 2. if we have no name yet, find existing item with same conent or
    // create a unique name
    if(!aUniqueName.Len())
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        sal_Int32 nUserIndex(1L);
        const String aUser(RTL_CONSTASCII_STRINGPARAM("trans"));
        const sal_uInt16 nCount(rPool.GetItemCount(XATTR_FILLFLOATTRANSPARENCE));
        const XFillFloatTransparenceItem *pItem;

        for(sal_uInt16 nSurrogate(0); nSurrogate < nCount; nSurrogate++)
        {
            pItem = (XFillFloatTransparenceItem*)rPool.GetItem(XATTR_FILLFLOATTRANSPARENCE, nSurrogate);

            if(pItem && pItem->GetName().Len())
            {
                if(pItem->IsEnabled() == pFillFloatTransparenceItem->IsEnabled() &&
                    pItem->GetValue() == pFillFloatTransparenceItem->GetValue())
                {
                    return pItem->Clone();
                }

                if(!pItem->GetName().CompareTo(aUser, aUser.Len()))
                {
                    sal_Int32 nThisIndex(pItem->GetName().Copy(aUser.Len()).ToInt32());
                    if(nThisIndex >= nUserIndex)
                        nUserIndex = nThisIndex + 1;
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32(nUserIndex);

        return new XFillFloatTransparenceItem(aUniqueName,
            pFillFloatTransparenceItem->GetValue(), pFillFloatTransparenceItem->IsEnabled());
    }

    return pNewItem;
}

const SfxPoolItem* ImplCheckFillHatchItem( const SfxPoolItem* pNewItem, SdrModel* pModel )
{
    if( NULL == pModel )
        return pNewItem;

    XFillHatchItem* pFillHatchItem = (XFillHatchItem*)pNewItem;

    String aUniqueName( pFillHatchItem->GetName() );

    // 1. if we have no name check if we have the same dash
    // inside the dash table if so, use that name
    if( aUniqueName.Len() == 0 )
    {
        XHatchList* pHatchList = pModel->GetHatchList();
        if( pHatchList )
        {
            const long nCount = pHatchList->Count();
            for( long nIndex = 0; nIndex < nCount; nIndex++ )
            {
                XHatchEntry* pEntry = pHatchList->Get(nIndex);
                if( pEntry->GetHatch() == pFillHatchItem->GetValue() )
                    return new XFillHatchItem( pEntry->GetName(), pEntry->GetHatch() );
            }
        }
    }
    // 2. if we have a name check if there is already an item with the
    // same name in the documents pool with a different line end or start
    else
    {
        const SfxItemPool& rPool = pModel->GetItemPool();

        const USHORT nCount = rPool.GetItemCount( XATTR_FILLHATCH );
        const XFillHatchItem *pItem;

        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillHatchItem*)rPool.GetItem( XATTR_FILLHATCH, nSurrogate );

            if( pItem && ( pItem->GetName() == pFillHatchItem->GetName() ) )
            {
                // if there is already an item with the same name and the same bitmap
                // its ok to set it
                if( pItem->GetValue() == pFillHatchItem->GetValue() )
                    break;

                // same name but different end, we need a new name for this item
                aUniqueName = String();
                break;
            }
        }
    }
    // 3. if we have no name yet, find existing item with same conent or
    // create a unique name
    if( aUniqueName.Len() == 0 )
    {
        const SfxItemPool& rPool = pModel->GetItemPool();
        sal_Int32 nUserIndex = 1;
        const String aUser( RTL_CONSTASCII_STRINGPARAM( "hatch" ) );

        const USHORT nCount = rPool.GetItemCount( XATTR_FILLHATCH );
        const XFillHatchItem *pItem;
        for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
        {
            pItem = (XFillHatchItem*)rPool.GetItem( XATTR_FILLHATCH, nSurrogate );

            if( pItem && pItem->GetName().Len() )
            {
                if( pItem->GetValue() == pFillHatchItem->GetValue() )
                    return pItem->Clone();

                if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
                {
                    sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
                    if( nThisIndex >= nUserIndex )
                        nUserIndex = nThisIndex + 1;
                }
            }
        }

        aUniqueName = aUser;
        aUniqueName += String::CreateFromInt32( nUserIndex );

        return new XFillHatchItem( aUniqueName, pFillHatchItem->GetValue() );
    }

    return pNewItem;
}

void SdrAttrObj::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
{
    if(pNewItem)
    {
        const SfxPoolItem* pItem = pNewItem;

        switch( nWhich )
        {
        case XATTR_FILLBITMAP:
            pItem = ImplCheckFillBitmapItem( pItem, pModel );
            break;
        case XATTR_LINEDASH:
            pItem = ImplCheckLineDashItem( pItem, pModel );
            break;
        case XATTR_LINESTART:
            pItem = ImplCheckLineStartItem( pItem, pModel );
            break;
        case XATTR_LINEEND:
            pItem = ImplCheckLineEndItem( pItem, pModel );
            break;
        case XATTR_FILLGRADIENT:
            pItem = ImplCheckFillGradientItem( pItem, pModel );
            break;
        case XATTR_FILLFLOATTRANSPARENCE:
            pItem = ImplCheckFillFloatTransparenceItem( pItem, pModel );
            break;
        case XATTR_FILLHATCH:
            pItem = ImplCheckFillHatchItem( pItem, pModel );
            break;
        }

        // set item
        ((SdrAttrObj*)this)->ImpForceItemSet();
        mpObjectItemSet->Put(*pItem);

        // delete item if it was a generated one
        if(pItem != pNewItem)
            delete (SfxPoolItem*)pItem;
    }
    else
    {
        // clear item
        if(mpObjectItemSet)
        {
            mpObjectItemSet->ClearItem(nWhich);
        }
    }
}

void SdrAttrObj::ItemSetChanged()
{
    // call parent
    SdrObject::ItemSetChanged();

    // own modifications
    bBoundRectDirty = TRUE;
    SetRectsDirty(TRUE);
    SetChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
    const SfxHint& rHint, const TypeId& rHintType)
{
    SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    BOOL bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());
    SfxStyleSheetHint *pStyleHint = PTR_CAST(SfxStyleSheetHint, &rHint);
    BOOL bDying(pStyleHint
                && pStyleHint->GetStyleSheet() == GetStyleSheet()
                && ( SFX_STYLESHEET_INDESTRUCTION == pStyleHint->GetHint() ||
                     SFX_STYLESHEET_ERASED == pStyleHint->GetHint() ));

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

        bBoundRectDirty = TRUE;
        SetRectsDirty(TRUE);
    }
    mpStyleSheet = NULL;
}

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
            const SdrShadowColorItem& rShadColItem = ((const SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
            Color aShadCol(rShadColItem.GetValue());
            sal_uInt16 nTransp = ((const SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
            XFillStyle eStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
            BOOL bFillBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();
            SfxItemSet aSet(rSet);

            if(eStyle==XFILL_HATCH && !bFillBackground)
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
}

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
