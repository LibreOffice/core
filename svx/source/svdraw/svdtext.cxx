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
        
        
        if(mpModel && &rOutliner == &mpModel->GetHitTestOutliner())
            return;

        
        mbPortionInfoChecked=true;
        if(mpOutlinerParaObject!=NULL && rOutliner.ShouldCreateBigTextObject())
        {
            
            delete mpOutlinerParaObject;
            mpOutlinerParaObject = rOutliner.CreateParaObject();
        }
    }
}

void SdrText::ReformatText()
{
    mbPortionInfoChecked=false;
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
            
            const SdrTextObj* pTestObj = mpModel->GetHitTestOutliner().GetTextObj();
            if( pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject )
                mpModel->GetHitTestOutliner().SetTextObj( 0 );
        }

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
        bool bHgtSet = GetObjectItemSet().GetItemState(EE_CHAR_FONTHEIGHT, true) == SFX_ITEM_SET;

        MapUnit aOldUnit(pOldModel->GetScaleUnit());
        MapUnit aNewUnit(pNewModel->GetScaleUnit());
        bool bScaleUnitChanged=aNewUnit!=aOldUnit;
        
        
        
        sal_uIntPtr nOldFontHgt=pOldModel->GetDefaultFontHeight();
        sal_uIntPtr nNewFontHgt=pNewModel->GetDefaultFontHeight();
        bool bDefHgtChanged=nNewFontHgt!=nOldFontHgt;
        bool bSetHgtItem=bDefHgtChanged && !bHgtSet;
        if (bSetHgtItem)
        {
            
            
            
            SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
        }
        
        SdrOutliner& rOutliner = mrObject.ImpGetDrawOutliner();
        rOutliner.SetText(*mpOutlinerParaObject);
        delete mpOutlinerParaObject;
        mpOutlinerParaObject=0;
        if (bScaleUnitChanged)
        {
            Fraction aMetricFactor=GetMapFactor(aOldUnit,aNewUnit).X();

            if (bSetHgtItem)
            {
                
                nOldFontHgt=BigMulDiv(nOldFontHgt,aMetricFactor.GetNumerator(),aMetricFactor.GetDenominator());
                SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
            }
        }
        SetOutlinerParaObject(rOutliner.CreateParaObject());
        mpOutlinerParaObject->ClearPortionInfo();
        mbPortionInfoChecked=false;
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
