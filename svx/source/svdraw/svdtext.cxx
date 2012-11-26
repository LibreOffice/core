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

#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT

#include "svx/svdotext.hxx"
#include "svx/svdetc.hxx"
#include "editeng/outlobj.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svdmodel.hxx"
#include "editeng/fhgtitem.hxx"
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svx/svdtrans.hxx>

SdrText::SdrText( SdrTextObj& rObject, OutlinerParaObject* pOutlinerParaObject /* = 0 */ )
: mpOutlinerParaObject( pOutlinerParaObject )
, mrObject( rObject )
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
        // Optimierung: ggf. BigTextObject erzeugen
        mbPortionInfoChecked=true;

        if(mpOutlinerParaObject && rOutliner.ShouldCreateBigTextObject())
        {
            // #i102062# MemoryLeak closed
            delete mpOutlinerParaObject;
            mpOutlinerParaObject = rOutliner.CreateParaObject();
        }
    }
}

void SdrText::ReformatText()
{
    mbPortionInfoChecked = false;
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
        if( mpOutlinerParaObject )
        {
            delete mpOutlinerParaObject;
        }

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
    OutlinerParaObject* pOPO = mpOutlinerParaObject;

    mpOutlinerParaObject = 0;
    mbPortionInfoChecked = false;

    return pOPO;
}

void SdrText::ForceOutlinerParaObject( sal_uInt16 nOutlMode )
{
    if( !mpOutlinerParaObject )
    {
        Outliner* pOutliner = SdrMakeOutliner( nOutlMode, &mrObject.getSdrModelFromSdrObject() );
        if( pOutliner )
        {
            Outliner& aDrawOutliner = mrObject.getSdrModelFromSdrObject().GetDrawOutliner();
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

void SdrText::ImpModelChange(SdrModel& rSourceModel, SdrModel& rTargetModel)
{
    if(mpOutlinerParaObject && &rSourceModel != &rTargetModel)
    {
        const bool bHgtSet(SFX_ITEM_SET == GetObjectItemSet().GetItemState(EE_CHAR_FONTHEIGHT, true));
        const MapUnit aOldUnit(rSourceModel.GetExchangeObjectUnit());
        const MapUnit aNewUnit(rTargetModel.GetExchangeObjectUnit());
        const bool bScaleUnitChanged(aNewUnit != aOldUnit);

        // und nun dem OutlinerParaObject einen neuen Pool verpassen
        // !!! Hier muss noch DefTab und RefDevice der beiden Models
        // !!! verglichen werden und dann ggf. AutoGrow zuschlagen !!!
        // !!! fehlende Implementation !!!
        const sal_uInt32 nOldFontHgt(rSourceModel.GetDefaultFontHeight());
        const sal_uInt32 nNewFontHgt(rTargetModel.GetDefaultFontHeight());
        const bool bDefHgtChanged(nNewFontHgt != nOldFontHgt);
        const bool bSetHgtItem(bDefHgtChanged && !bHgtSet);

        if(bSetHgtItem)
        {
            // #32665#
            // zunaechst das HeightItem festklopfen, damit
            // 1. Es eben bestehen bleibt und
            // 2. DoStretchChars vom richtigen Wert ausgeht
            SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
        }

        // erst jetzt den Outliner holen, etc. damit obiges SetAttr auch wirkt
        SdrOutliner& rOutliner = mrObject.ImpGetDrawOutliner();

        rOutliner.SetText(*mpOutlinerParaObject);
        delete mpOutlinerParaObject;
        mpOutlinerParaObject = 0;

        if(bScaleUnitChanged)
        {
            Fraction aMetricFactor(GetMapFactor(aOldUnit, aNewUnit).X());

            // Funktioniert nicht richtig:
            // Geht am Outliner leider nur in %
            // double nPercFloat=double(aMetricFactor)*100+0.5;
            // sal_uInt16 nPerc=(sal_uInt16)nPercFloat;
            // rOutliner.DoStretchChars(100,nPerc);

            if(bSetHgtItem)
            {
                // Und nun noch das Rahmenattribut korregieren
                const sal_uInt32 nAdaptedFontHgt(BigMulDiv(nOldFontHgt,aMetricFactor.GetNumerator(),aMetricFactor.GetDenominator()));

                SetObjectItem(SvxFontHeightItem(nAdaptedFontHgt, 100, EE_CHAR_FONTHEIGHT));
            }
        }

        SetOutlinerParaObject(rOutliner.CreateParaObject()); // #34494#
        mpOutlinerParaObject->ClearPortionInfo();
        mbPortionInfoChecked = false;
        rOutliner.Clear();
    }
}

// eof