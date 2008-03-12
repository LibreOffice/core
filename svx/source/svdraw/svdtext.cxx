/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdtext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:56:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT

#include "svx/svdotext.hxx"
#include "svx/svdetc.hxx"
#include "svx/outlobj.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svdmodel.hxx"
#include "svx/fhgtitem.hxx"

SdrText::SdrText( SdrTextObj* pObject, OutlinerParaObject* pOutlinerParaObject /* = 0 */ )
: mpOutlinerParaObject( pOutlinerParaObject )
, mpObject( pObject )
, mpModel( pObject->GetModel() )
, mbPortionInfoChecked( false )
{
}

SdrText::~SdrText()
{
    delete mpOutlinerParaObject;
}

void SdrText::CheckPortionInfo( SdrOutliner& rOutliner )
{
    if(!mbPortionInfoChecked)
    {
        // Optimierung: ggf. BigTextObject erzeugen
        mbPortionInfoChecked=true;
        if(mpOutlinerParaObject!=NULL && rOutliner.ShouldCreateBigTextObject())
            mpOutlinerParaObject= rOutliner.CreateParaObject();
    }
}

void SdrText::ReformatText()
{
    mbPortionInfoChecked=FALSE;
    mpOutlinerParaObject->ClearPortionInfo();
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
        bool bHgtSet = GetObjectItemSet().GetItemState(EE_CHAR_FONTHEIGHT, TRUE) == SFX_ITEM_SET;

        MapUnit aOldUnit(pOldModel->GetScaleUnit());
        MapUnit aNewUnit(pNewModel->GetScaleUnit());
        FASTBOOL bScaleUnitChanged=aNewUnit!=aOldUnit;
        // und nun dem OutlinerParaObject einen neuen Pool verpassen
        // !!! Hier muss noch DefTab und RefDevice der beiden Models
        // !!! verglichen werden und dann ggf. AutoGrow zuschlagen !!!
        // !!! fehlende Implementation !!!
        ULONG nOldFontHgt=pOldModel->GetDefaultFontHeight();
        ULONG nNewFontHgt=pNewModel->GetDefaultFontHeight();
        BOOL bDefHgtChanged=nNewFontHgt!=nOldFontHgt;
        BOOL bSetHgtItem=bDefHgtChanged && !bHgtSet;
        if (bSetHgtItem)
        { // #32665#
            // zunaechst das HeightItem festklopfen, damit
            // 1. Es eben bestehen bleibt und
            // 2. DoStretchChars vom richtigen Wert ausgeht
            SetObjectItem(SvxFontHeightItem(nOldFontHgt, 100, EE_CHAR_FONTHEIGHT));
        }
        // erst jetzt den Outliner holen, etc. damit obiges SetAttr auch wirkt
        SdrOutliner& rOutliner=mpObject->ImpGetDrawOutliner();
        rOutliner.SetText(*mpOutlinerParaObject);
        delete mpOutlinerParaObject;
        mpOutlinerParaObject=0;
        if (bScaleUnitChanged)
        {
            Fraction aMetricFactor=GetMapFactor(aOldUnit,aNewUnit).X();

            // Funktioniert nicht richtig:
            // Geht am Outliner leider nur in %
            // double nPercFloat=double(aMetricFactor)*100+0.5;
            // USHORT nPerc=(USHORT)nPercFloat;
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
        mbPortionInfoChecked=FALSE;
        rOutliner.Clear();
    }
}

void SdrText::ForceOutlinerParaObject( USHORT nOutlMode )
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
    return mpObject->GetObjectItemSet();
}

void SdrText::SetObjectItem(const SfxPoolItem& rItem)
{
    mpObject->SetObjectItem( rItem );
}

SfxStyleSheet* SdrText::GetStyleSheet() const
{
    return mpObject->GetStyleSheet();
}
