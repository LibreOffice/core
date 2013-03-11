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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>

#include "dlgsnap.hrc"
#include "sdattr.hxx"
#include "View.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "dlgsnap.hxx"


/**
 * dialog to adjust grid (scarcely ESO!)
 */
SdSnapLineDlg::SdSnapLineDlg(
    ::Window* pWindow,
    const SfxItemSet& rInAttrs,
    ::sd::View* pView)
    : ModalDialog (pWindow, SdResId(DLG_SNAPLINE)),
      aFlPos      (this, SdResId(FL_POSITION)),
      aFtX        (this, SdResId(FT_X)),
      aMtrFldX    (this, SdResId(MTR_FLD_X)),
      aFtY        (this, SdResId(FT_Y)),
      aMtrFldY    (this, SdResId(MTR_FLD_Y)),
      aFlDir      (this, SdResId(FL_DIRECTION)),
      aRbPoint    (this, SdResId(RB_POINT)),
      aRbVert     (this, SdResId(RB_VERTICAL)),
      aRbHorz     (this, SdResId(RB_HORIZONTAL)),
      aBtnOK      (this, SdResId(BTN_OK)),
      aBtnCancel  (this, SdResId(BTN_CANCEL)),
      aBtnHelp    (this, SdResId(BTN_HELP)),
      aBtnDelete  (this, SdResId(BTN_DELETE)),
      eUIUnit(pView->GetDoc().GetUIUnit()),
      aUIScale(pView->GetDoc().GetUIScale())
{
    FreeResource();

    aRbHorz.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbVert.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbPoint.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    aBtnDelete.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    SetFieldUnit( aMtrFldX, eUIUnit, sal_True );
    SetFieldUnit( aMtrFldY, eUIUnit, sal_True );

    // get WorkArea
    Rectangle aWorkArea = pView->GetWorkArea();

    // determine PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Where's the Pool?" );
    SfxMapUnit ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // #i48497# Consider page origin
    SdrPageView* pPV = pView->GetSdrPageView();
    Point aLeftTop(aWorkArea.Left()+1, aWorkArea.Top()+1);
    pPV->LogicToPagePos(aLeftTop);
    Point aRightBottom(aWorkArea.Right()-2, aWorkArea.Bottom()-2);
    pPV->LogicToPagePos(aRightBottom);

    // determine max and min values depending on
    // WorkArea, PoolUnit and FieldUnit:
    SetMetricValue( aMtrFldX, aLeftTop.X(), ePoolUnit );

    long nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMin( nValue );
    aMtrFldX.SetFirst( nValue );

    SetMetricValue( aMtrFldX, aRightBottom.X(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMax( nValue );
    aMtrFldX.SetLast( nValue );

    SetMetricValue( aMtrFldY, aLeftTop.Y(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldY.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMin( nValue );
    aMtrFldY.SetFirst( nValue );

    SetMetricValue( aMtrFldY, aRightBottom.Y(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldY.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMax( nValue );
    aMtrFldY.SetLast( nValue );

    // set values
    nXValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_X)).GetValue();
    nYValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_Y)).GetValue();
    nXValue = Fraction(nXValue) / aUIScale;
    nYValue = Fraction(nYValue) / aUIScale;
    SetMetricValue( aMtrFldX, nXValue, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( aMtrFldY, nYValue, SFX_MAPUNIT_100TH_MM);

    aRbPoint.Check();
}

/**
 * fills provided item sets with dialog box attributes
 */
IMPL_LINK( SdSnapLineDlg, ClickHdl, Button *, pBtn )
{
    if ( pBtn == &aRbPoint )        SetInputFields(sal_True, sal_True);
    else if ( pBtn == &aRbHorz )    SetInputFields(sal_False, sal_True);
    else if ( pBtn == &aRbVert )    SetInputFields(sal_True, sal_False);
    else if ( pBtn == &aBtnDelete ) EndDialog(RET_SNAP_DELETE);

    return 0;
}

/**
 * fills provided item sets with dialog box attributes
 */
void SdSnapLineDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    SnapKind eKind;

    if ( aRbHorz.IsChecked() )      eKind = SK_HORIZONTAL;
    else if ( aRbVert.IsChecked() ) eKind = SK_VERTICAL;
    else                            eKind = SK_POINT;

    nXValue = Fraction( GetCoreValue( aMtrFldX, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    nYValue = Fraction( GetCoreValue( aMtrFldY, SFX_MAPUNIT_100TH_MM) ) * aUIScale;

    rOutAttrs.Put(SfxAllEnumItem(ATTR_SNAPLINE_KIND, (sal_uInt16)eKind));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_X, nXValue));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, nYValue));
}

void SdSnapLineDlg::HideRadioGroup()
{
    aFlDir.Hide();
    aRbHorz.Hide();
    aRbVert.Hide();
    aRbPoint.Hide();
}

/**
 * disable X or Y input fields
 */
void SdSnapLineDlg::SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY)
{
    if ( bEnableX )
    {
        if ( !aMtrFldX.IsEnabled() )
            aMtrFldX.SetValue(nXValue);
        aMtrFldX.Enable();
        aFtX.Enable();
    }
    else if ( aMtrFldX.IsEnabled() )
    {
        nXValue = static_cast<long>(aMtrFldX.GetValue());
        aMtrFldX.SetText(String());
        aMtrFldX.Disable();
        aFtX.Disable();
    }
    if ( bEnableY )
    {
        if ( !aMtrFldY.IsEnabled() )
            aMtrFldY.SetValue(nYValue);
        aMtrFldY.Enable();
        aFtY.Enable();
    }
    else if ( aMtrFldY.IsEnabled() )
    {
        nYValue = static_cast<long>(aMtrFldY.GetValue());
        aMtrFldY.SetText(String());
        aMtrFldY.Disable();
        aFtY.Disable();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
