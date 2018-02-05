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

#include <svx/svdotext.hxx>
#include <svx/svdetc.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <libxml/xmlwriter.h>
#include <memory>

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
}

void SdrText::CheckPortionInfo( SdrOutliner& rOutliner )
{
    if(!mbPortionInfoChecked)
    {
        // #i102062# no action when the Outliner is the HitTestOutliner,
        // this will remove WrongList info at the OPO
        if(mpModel && &rOutliner == &mpModel->GetHitTestOutliner())
            return;

        // TODO: optimization: we could create a BigTextObject
        mbPortionInfoChecked=true;
        if(mpOutlinerParaObject!=nullptr && rOutliner.ShouldCreateBigTextObject())
        {
            // #i102062# MemoryLeak closed
            mpOutlinerParaObject.reset( rOutliner.CreateParaObject() );
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
    if( mpOutlinerParaObject.get() != pTextObject )
    {
        if( mpModel )
        {
            // Update HitTestOutliner
            const SdrTextObj* pTestObj = mpModel->GetHitTestOutliner().GetTextObj();
            if( pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject.get() )
                mpModel->GetHitTestOutliner().SetTextObj( nullptr );
        }

        mpOutlinerParaObject.reset(pTextObject);

        mbPortionInfoChecked = false;
    }
}

OutlinerParaObject* SdrText::GetOutlinerParaObject() const
{
    return mpOutlinerParaObject.get();
}

/** returns the current OutlinerParaObject and removes it from this instance */
OutlinerParaObject* SdrText::RemoveOutlinerParaObject()
{
    if( mpModel )
    {
        // Update HitTestOutliner
        const SdrTextObj* pTestObj = mpModel->GetHitTestOutliner().GetTextObj();
        if( pTestObj && pTestObj->GetOutlinerParaObject() == mpOutlinerParaObject.get() )
            mpModel->GetHitTestOutliner().SetTextObj( nullptr );
    }

    OutlinerParaObject* pOPO = mpOutlinerParaObject.release();

    mbPortionInfoChecked = false;

    return pOPO;
}

void SdrText::SetModel( SdrModel* pNewModel )
{
    if( pNewModel == mpModel )
        return;

    SdrModel* pOldModel = mpModel;
    mpModel = pNewModel;

    if( !mpOutlinerParaObject || pOldModel==nullptr || pNewModel==nullptr)
        return;

    bool bHgtSet = GetObjectItemSet().GetItemState(EE_CHAR_FONTHEIGHT) == SfxItemState::SET;

    MapUnit aOldUnit(pOldModel->GetScaleUnit());
    MapUnit aNewUnit(pNewModel->GetScaleUnit());
    bool bScaleUnitChanged=aNewUnit!=aOldUnit;
    // Now move the OutlinerParaObject into a new Pool.
    // TODO: We should compare the DefTab and RefDevice of both Models to
    // see whether we need to use AutoGrow!
    sal_Int32 nOldFontHgt=pOldModel->GetDefaultFontHeight();
    sal_Int32 nNewFontHgt=pNewModel->GetDefaultFontHeight();
    bool bDefHgtChanged=nNewFontHgt!=nOldFontHgt;
    bool bSetHgtItem=bDefHgtChanged && !bHgtSet;
    if (bSetHgtItem)
    {
        // fix the value of HeightItem, so
        // 1. it remains and
        // 2. DoStretchChars gets the right value
        SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
    }
    // now use the Outliner, etc. so the above SetAttr can work at all
    SdrOutliner& rOutliner = mrObject.ImpGetDrawOutliner();
    rOutliner.SetText(*mpOutlinerParaObject);
    mpOutlinerParaObject.reset();
    if (bScaleUnitChanged)
    {
        Fraction aMetricFactor=GetMapFactor(aOldUnit,aNewUnit).X();

        if (bSetHgtItem)
        {
            // Now correct the frame attribute
            nOldFontHgt=BigMulDiv(nOldFontHgt,aMetricFactor.GetNumerator(),aMetricFactor.GetDenominator());
            SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
        }
    }
    SetOutlinerParaObject(rOutliner.CreateParaObject());
    mpOutlinerParaObject->ClearPortionInfo();
    mbPortionInfoChecked=false;
    rOutliner.Clear();
}

void SdrText::ForceOutlinerParaObject( OutlinerMode nOutlMode )
{
    if( mpModel && !mpOutlinerParaObject )
    {
        std::unique_ptr<Outliner> pOutliner(SdrMakeOutliner(nOutlMode, *mpModel));
        if( pOutliner )
        {
            Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
            pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );

            pOutliner->SetStyleSheet( 0, GetStyleSheet());
            OutlinerParaObject* pOutlinerParaObject = pOutliner->CreateParaObject();
            SetOutlinerParaObject( pOutlinerParaObject );
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

void SdrText::dumpAsXml(struct _xmlTextWriter * pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrText"));
    mpOutlinerParaObject->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
