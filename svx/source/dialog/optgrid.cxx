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

#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>

#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include "svx/optgrid.hxx"
#include <svx/strings.hrc>
#include "svx/dlgutil.hxx"

// local functions
static void    lcl_GetMinMax(MetricField& rField, long& nFirst, long& nLast, long& nMin, long& nMax)
{
    nFirst  = static_cast<long>(rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) ));
    nLast = static_cast<long>(rField.Denormalize( rField.GetLast( FUNIT_TWIP ) ));
    nMin = static_cast<long>(rField.Denormalize( rField.GetMin( FUNIT_TWIP ) ));
    nMax = static_cast<long>(rField.Denormalize( rField.GetMax( FUNIT_TWIP ) ));
}

static void    lcl_SetMinMax(MetricField& rField, long nFirst, long nLast, long nMin, long nMax)
{
    rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
    rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
    rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
}

SvxOptionsGrid::SvxOptionsGrid() :
    nFldDrawX       ( 100 ),
    nFldDivisionX   ( 0 ),
    nFldDrawY       ( 100 ),
    nFldDivisionY   ( 0 ),
    nFldSnapX       ( 100 ),
    nFldSnapY       ( 100 ),
    bUseGridsnap    ( false ),
    bSynchronize    ( true ),
    bGridVisible    ( false ),
    bEqualGrid      ( true )
{
}

SvxGridItem::SvxGridItem( const SvxGridItem& rItem )
:   SvxOptionsGrid()
,   SfxPoolItem(rItem)
{
    bUseGridsnap = rItem.bUseGridsnap ;
    bSynchronize = rItem.bSynchronize ;
    bGridVisible = rItem.bGridVisible ;
    bEqualGrid   = rItem.bEqualGrid   ;
    nFldDrawX    = rItem.nFldDrawX    ;
    nFldDivisionX= rItem.nFldDivisionX;
    nFldDrawY    = rItem.nFldDrawY    ;
    nFldDivisionY= rItem.nFldDivisionY;
    nFldSnapX    = rItem.nFldSnapX    ;
    nFldSnapY    = rItem.nFldSnapY    ;

};

SfxPoolItem*  SvxGridItem::Clone( SfxItemPool* ) const
{
    return new SvxGridItem( *this );
}

bool SvxGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxGridItem& rItem = static_cast<const SvxGridItem&>(rAttr);

    return (    bUseGridsnap == rItem.bUseGridsnap &&
                bSynchronize == rItem.bSynchronize &&
                bGridVisible == rItem.bGridVisible &&
                bEqualGrid   == rItem.bEqualGrid   &&
                nFldDrawX    == rItem.nFldDrawX    &&
                nFldDivisionX== rItem.nFldDivisionX&&
                nFldDrawY    == rItem.nFldDrawY    &&
                nFldDivisionY== rItem.nFldDivisionY&&
                nFldSnapX    == rItem.nFldSnapX    &&
                nFldSnapY    == rItem.nFldSnapY     );
}

bool  SvxGridItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = "SvxGridItem";
    return true;
}

// TabPage Screen Settings
SvxGridTabPage::SvxGridTabPage( vcl::Window* pParent, const SfxItemSet& rCoreSet) :

    SfxTabPage( pParent, "OptGridPage" , "svx/ui/optgridpage.ui", &rCoreSet ),
    bAttrModified( false )
{
    get(pCbxUseGridsnap,"usegridsnap");
    get(pCbxGridVisible,"gridvisible");
    get(pMtrFldDrawX,"mtrflddrawx");
    get(pMtrFldDrawY,"mtrflddrawy");
    get(pNumFldDivisionX,"numflddivisionx");
    get(pNumFldDivisionY,"numflddivisiony");
    get(pCbxSynchronize,"synchronize");

    get(pSnapFrames,"snapframes");
    get(pCbxSnapHelplines,"snaphelplines");
    get(pCbxSnapBorder,"snapborder");
    get(pCbxSnapFrame,"snapframe");
    get(pCbxSnapPoints,"snappoints");
    get(pMtrFldSnapArea,"mtrfldsnaparea");
    get(pCbxOrtho,"ortho");
    get(pCbxBigOrtho,"bigortho");
    get(pCbxRotate,"rotate");
    get(pMtrFldAngle,"mtrfldangle");
    get(pMtrFldBezAngle,"mtrfldbezangle");

    // This page requires exchange Support
    SetExchangeSupport();

    // Set Metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreSet );
    long nFirst, nLast, nMin, nMax;

    lcl_GetMinMax(*pMtrFldDrawX , nFirst, nLast, nMin, nMax);
    SetFieldUnit( *pMtrFldDrawX , eFUnit, true );
    lcl_SetMinMax(*pMtrFldDrawX , nFirst, nLast, nMin, nMax);

    lcl_GetMinMax(*pMtrFldDrawY, nFirst, nLast, nMin, nMax);
    SetFieldUnit( *pMtrFldDrawY, eFUnit, true );
    lcl_SetMinMax(*pMtrFldDrawY, nFirst, nLast, nMin, nMax);


    pCbxRotate->SetClickHdl( LINK( this, SvxGridTabPage, ClickRotateHdl_Impl ) );
    Link<Button*,void> aLink = LINK( this, SvxGridTabPage, ChangeGridsnapHdl_Impl );
    pCbxUseGridsnap->SetClickHdl( aLink );
    pCbxSynchronize->SetClickHdl( aLink );
    pCbxGridVisible->SetClickHdl( aLink );
    pMtrFldDrawX->SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    pMtrFldDrawY->SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    pNumFldDivisionX->SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );
    pNumFldDivisionY->SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );
}

SvxGridTabPage::~SvxGridTabPage()
{
    disposeOnce();
}

void SvxGridTabPage::dispose()
{
    pCbxUseGridsnap.clear();
    pCbxGridVisible.clear();
    pMtrFldDrawX.clear();
    pMtrFldDrawY.clear();
    pNumFldDivisionX.clear();
    pNumFldDivisionY.clear();
    pCbxSynchronize.clear();
    pSnapFrames.clear();
    pCbxSnapHelplines.clear();
    pCbxSnapBorder.clear();
    pCbxSnapFrame.clear();
    pCbxSnapPoints.clear();
    pMtrFldSnapArea.clear();
    pCbxOrtho.clear();
    pCbxBigOrtho.clear();
    pCbxRotate.clear();
    pMtrFldAngle.clear();
    pMtrFldBezAngle.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SvxGridTabPage::Create( vcl::Window* pParent, const SfxItemSet& rAttrSet )
{
    return VclPtr<SvxGridTabPage>::Create( pParent, rAttrSet );
}


bool SvxGridTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( bAttrModified )
    {
        SvxGridItem aGridItem( SID_ATTR_GRID_OPTIONS );

        aGridItem.bUseGridsnap  = pCbxUseGridsnap->IsChecked();
        aGridItem.bSynchronize  = pCbxSynchronize->IsChecked();
        aGridItem.bGridVisible  = pCbxGridVisible->IsChecked();

        MapUnit eUnit =
            rCoreSet->GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        long nX =GetCoreValue(  *pMtrFldDrawX, eUnit );
        long nY = GetCoreValue( *pMtrFldDrawY, eUnit );

        aGridItem.nFldDrawX    = (sal_uInt32) nX;
        aGridItem.nFldDrawY    = (sal_uInt32) nY;
        aGridItem.nFldDivisionX = static_cast<long>(pNumFldDivisionX->GetValue()-1);
        aGridItem.nFldDivisionY = static_cast<long>(pNumFldDivisionY->GetValue()-1);

        rCoreSet->Put( aGridItem );
    }
    return bAttrModified;
}


void SvxGridTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pAttr = nullptr;

    if( SfxItemState::SET == rSet->GetItemState( SID_ATTR_GRID_OPTIONS , false,
                                    &pAttr ))
    {
        const SvxGridItem* pGridAttr = static_cast<const SvxGridItem*>(pAttr);
        pCbxUseGridsnap->Check( pGridAttr->bUseGridsnap );
        pCbxSynchronize->Check( pGridAttr->bSynchronize );
        pCbxGridVisible->Check( pGridAttr->bGridVisible );

        MapUnit eUnit =
            rSet->GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        SetMetricValue( *pMtrFldDrawX , pGridAttr->nFldDrawX, eUnit );
        SetMetricValue( *pMtrFldDrawY , pGridAttr->nFldDrawY, eUnit );

        pNumFldDivisionX->SetValue( pGridAttr->nFldDivisionX+1 );
        pNumFldDivisionY->SetValue( pGridAttr->nFldDivisionY+1 );
    }

    ChangeGridsnapHdl_Impl( pCbxUseGridsnap );
    bAttrModified = false;
}


void SvxGridTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , false,
                                    &pAttr ))
    {
        const SvxGridItem* pGridAttr = static_cast<const SvxGridItem*>(pAttr);
        pCbxUseGridsnap->Check( pGridAttr->bUseGridsnap );

        ChangeGridsnapHdl_Impl( pCbxUseGridsnap );
    }

    // Metric Change if necessary (as TabPage is in the dialog, where the
    // metric can be set
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_METRIC , false,
                                    &pAttr ))
    {
        const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pAttr);

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != pMtrFldDrawX->GetUnit() )
        {
            // Set Metrics
            long nFirst, nLast, nMin, nMax;
            long nVal = static_cast<long>(pMtrFldDrawX->Denormalize( pMtrFldDrawX->GetValue( FUNIT_TWIP ) ));

            lcl_GetMinMax(*pMtrFldDrawX, nFirst, nLast, nMin, nMax);
            SetFieldUnit( *pMtrFldDrawX, eFUnit, true );
            lcl_SetMinMax(*pMtrFldDrawX, nFirst, nLast, nMin, nMax);

            pMtrFldDrawX->SetValue( pMtrFldDrawX->Normalize( nVal ), FUNIT_TWIP );

            nVal = static_cast<long>(pMtrFldDrawY->Denormalize( pMtrFldDrawY->GetValue( FUNIT_TWIP ) ));
            lcl_GetMinMax(*pMtrFldDrawY, nFirst, nLast, nMin, nMax);
            SetFieldUnit(*pMtrFldDrawY, eFUnit, true );
            lcl_SetMinMax(*pMtrFldDrawY, nFirst, nLast, nMin, nMax);
            pMtrFldDrawY->SetValue( pMtrFldDrawY->Normalize( nVal ), FUNIT_TWIP );

        }
    }
}


DeactivateRC SvxGridTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

IMPL_LINK( SvxGridTabPage, ChangeDrawHdl_Impl, Edit&, rField, void )
{
    bAttrModified = true;
    if( pCbxSynchronize->IsChecked() )
    {
        if(&rField == pMtrFldDrawX)
            pMtrFldDrawY->SetValue( pMtrFldDrawX->GetValue() );
        else
            pMtrFldDrawX->SetValue( pMtrFldDrawY->GetValue() );
    }
}


IMPL_LINK_NOARG(SvxGridTabPage, ClickRotateHdl_Impl, Button*, void)
{
    if( pCbxRotate->IsChecked() )
        pMtrFldAngle->Enable();
    else
        pMtrFldAngle->Disable();
}


IMPL_LINK( SvxGridTabPage, ChangeDivisionHdl_Impl, Edit&, rField, void )
{
    bAttrModified = true;
    if( pCbxSynchronize->IsChecked() )
    {
        if(pNumFldDivisionX == &rField)
            pNumFldDivisionY->SetValue( pNumFldDivisionX->GetValue() );
        else
            pNumFldDivisionX->SetValue( pNumFldDivisionY->GetValue() );
    }
}


IMPL_LINK_NOARG(SvxGridTabPage, ChangeGridsnapHdl_Impl, Button*, void)
{
    bAttrModified = true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
