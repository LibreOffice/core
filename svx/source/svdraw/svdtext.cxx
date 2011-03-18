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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT

#include "svx/svdotext.hxx"
#include "svx/svdetc.hxx"
#include "editeng/outlobj.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svdmodel.hxx"
#include "editeng/fhgtitem.hxx"
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>

SdrText::SdrText( SdrTextObj& rObject, OutlinerParaObject* pOutlinerParaObject /* = 0 */ )
: mpOutlinerParaObject( pOutlinerParaObject )
, mrObject( rObject )
, mpModel( rObject.GetModel() )
, mbPortionInfoChecked( false )
{
    OSL_ENSURE(&mrObject, "SdrText created without SdrTextObj (!)");
}

SdrText::~SdrText()
{
    clearWeak();
    delete mpOutlinerParaObject;
}

void SdrText::CheckPortionInfo( SdrOutliner& rOutliner )
{
    if(!mbPortionInfoChecked)
    {
        // #i102062# no action when the Outliner is the HitTestOutliner,
        // this will remove WrongList info at the OPO
        if(mpModel && &rOutliner == &mpModel->GetHitTestOutliner())
            return;

        // Optimierung: ggf. BigTextObject erzeugen
        mbPortionInfoChecked=true;
        if(mpOutlinerParaObject!=NULL && rOutliner.ShouldCreateBigTextObject())
        {
            // #i102062# MemoryLeak closed
            delete mpOutlinerParaObject;
            mpOutlinerParaObject = rOutliner.CreateParaObject();
        }
    }
}

void SdrText::ReformatText()
{
    mbPortionInfoChecked=sal_False;
    mpOutlinerParaObject->ClearPortionInfo();
}

const SfxItemSet& SdrText::GetItemSet() const
{
    return const_cast< SdrText* >(this)->GetObjectItemSet();
}

void SdrText::SetOutlinerParaObject( OutlinerParaObject* pTextObject )
{
    if( mpOutlinerParaObject != pTextObject )
    {
        if( mpModel )
        {
            // Update HitTestOutliner
            const SdrTextObj* pTestObj = mpModel->GetHitTestOutliner().GetTextObj();
            if( pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject )
                mpModel->GetHitTestOutliner().SetTextObj( 0 );
        }

        if( mpOutlinerParaObject )
            delete mpOutlinerParaObject;

        mpOutlinerParaObject = pTextObject;

        mbPortionInfoChecked = false;
    }
}

OutlinerParaObject* SdrText::GetOutlinerParaObject() const
{
    return mpOutlinerParaObject;
}

/** returns the current OutlinerParaObject and removes it from this instance */
OutlinerParaObject* SdrText::RemoveOutlinerParaObject()
{
    if( mpModel )
    {
        // Update HitTestOutliner
        const SdrTextObj* pTestObj = mpModel->GetHitTestOutliner().GetTextObj();
        if( pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject )
            mpModel->GetHitTestOutliner().SetTextObj( 0 );
    }

    OutlinerParaObject* pOPO = mpOutlinerParaObject;

    mpOutlinerParaObject = 0;
    mbPortionInfoChecked = false;

    return pOPO;
}

void SdrText::SetModel( SdrModel* pNewModel )
{
    if( pNewModel == mpModel )
        return;

    SdrModel* pOldModel = mpModel;
    mpModel = pNewModel;

    if( mpOutlinerParaObject && pOldModel!=NULL && pNewModel!=NULL)
    {
        bool bHgtSet = GetObjectItemSet().GetItemState(EE_CHAR_FONTHEIGHT, sal_True) == SFX_ITEM_SET;

        MapUnit aOldUnit(pOldModel->GetScaleUnit());
        MapUnit aNewUnit(pNewModel->GetScaleUnit());
        bool bScaleUnitChanged=aNewUnit!=aOldUnit;
        // und nun dem OutlinerParaObject einen neuen Pool verpassen
        // !!! Hier muss noch DefTab und RefDevice der beiden Models
        // !!! verglichen werden und dann ggf. AutoGrow zuschlagen !!!
        // !!! fehlende Implementation !!!
        sal_uIntPtr nOldFontHgt=pOldModel->GetDefaultFontHeight();
        sal_uIntPtr nNewFontHgt=pNewModel->GetDefaultFontHeight();
        sal_Bool bDefHgtChanged=nNewFontHgt!=nOldFontHgt;
        sal_Bool bSetHgtItem=bDefHgtChanged && !bHgtSet;
        if (bSetHgtItem)
        { // #32665#
            // zunaechst das HeightItem festklopfen, damit
            // 1. Es eben bestehen bleibt und
            // 2. DoStretchChars vom richtigen Wert ausgeht
            SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
        }
        // erst jetzt den Outliner holen, etc. damit obiges SetAttr auch wirkt
        SdrOutliner& rOutliner = mrObject.ImpGetDrawOutliner();
        rOutliner.SetText(*mpOutlinerParaObject);
        delete mpOutlinerParaObject;
        mpOutlinerParaObject=0;
        if (bScaleUnitChanged)
        {
            Fraction aMetricFactor=GetMapFactor(aOldUnit,aNewUnit).X();

            // Funktioniert nicht richtig:
            // Geht am Outliner leider nur in %
            // double nPercFloat=double(aMetricFactor)*100+0.5;
            // sal_uInt16 nPerc=(sal_uInt16)nPercFloat;
            // rOutliner.DoStretchChars(100,nPerc);

            if (bSetHgtItem)
            {
                // Und nun noch das Rahmenattribut korregieren
                nOldFontHgt=BigMulDiv(nOldFontHgt,aMetricFactor.GetNumerator(),aMetricFactor.GetDenominator());
                SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
            }
        }
        SetOutlinerParaObject(rOutliner.CreateParaObject()); // #34494#
        mpOutlinerParaObject->ClearPortionInfo();
        mbPortionInfoChecked=sal_False;
        rOutliner.Clear();
    }
}

void SdrText::ForceOutlinerParaObject( sal_uInt16 nOutlMode )
{
    if( mpModel && !mpOutlinerParaObject )
    {
        Outliner* pOutliner = SdrMakeOutliner( nOutlMode, mpModel );
        if( pOutliner )
        {
            Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
            pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );

            pOutliner->SetStyleSheet( 0, GetStyleSheet());
            OutlinerParaObject* pOutlinerParaObject = pOutliner->CreateParaObject();
            SetOutlinerParaObject( pOutlinerParaObject );

            delete pOutliner;
        }
    }
}

const SfxItemSet& SdrText::GetObjectItemSet()
{
    return mrObject.GetObjectItemSet();
}

void SdrText::SetObjectItem(const SfxPoolItem& rItem)
{
    mrObject.SetObjectItem( rItem );
}

SfxStyleSheet* SdrText::GetStyleSheet() const
{
    return mrObject.GetStyleSheet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
