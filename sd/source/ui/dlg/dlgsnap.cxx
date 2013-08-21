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

#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>

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
    : ModalDialog(pWindow, "SnapObjectDialog", "modules/sdraw/ui/dlgsnap.ui")
    , eUIUnit(pView->GetDoc().GetUIUnit())
    , aUIScale(pView->GetDoc().GetUIScale())
{
    get(m_pFtX, "xlabel");
    get(m_pMtrFldX, "x");
    get(m_pFtY, "ylabel");
    get(m_pMtrFldY, "y");
    get(m_pRadioGroup, "radiogroup");
    get(m_pRbPoint, "point");
    get(m_pRbVert, "vert");
    get(m_pRbHorz, "horz");
    get(m_pBtnDelete, "delete");

    m_pRbHorz->SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    m_pRbVert->SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    m_pRbPoint->SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    m_pBtnDelete->SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    SetFieldUnit( *m_pMtrFldX, eUIUnit, sal_True );
    SetFieldUnit( *m_pMtrFldY, eUIUnit, sal_True );

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
    SetMetricValue( *m_pMtrFldX, aLeftTop.X(), ePoolUnit );

    long nValue = static_cast<long>(m_pMtrFldX->GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    m_pMtrFldX->SetMin( nValue );
    m_pMtrFldX->SetFirst( nValue );

    SetMetricValue( *m_pMtrFldX, aRightBottom.X(), ePoolUnit );
    nValue = static_cast<long>(m_pMtrFldX->GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    m_pMtrFldX->SetMax( nValue );
    m_pMtrFldX->SetLast( nValue );

    SetMetricValue( *m_pMtrFldY, aLeftTop.Y(), ePoolUnit );
    nValue = static_cast<long>(m_pMtrFldY->GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    m_pMtrFldY->SetMin( nValue );
    m_pMtrFldY->SetFirst( nValue );

    SetMetricValue( *m_pMtrFldY, aRightBottom.Y(), ePoolUnit );
    nValue = static_cast<long>(m_pMtrFldY->GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    m_pMtrFldY->SetMax( nValue );
    m_pMtrFldY->SetLast( nValue );

    // set values
    nXValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_X)).GetValue();
    nYValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_Y)).GetValue();
    nXValue = Fraction(nXValue) / aUIScale;
    nYValue = Fraction(nYValue) / aUIScale;
    SetMetricValue( *m_pMtrFldX, nXValue, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( *m_pMtrFldY, nYValue, SFX_MAPUNIT_100TH_MM);

    m_pRbPoint->Check();
}

/**
 * fills provided item sets with dialog box attributes
 */
IMPL_LINK( SdSnapLineDlg, ClickHdl, Button *, pBtn )
{
    if ( pBtn == m_pRbPoint )        SetInputFields(sal_True, sal_True);
    else if ( pBtn == m_pRbHorz )    SetInputFields(sal_False, sal_True);
    else if ( pBtn == m_pRbVert )    SetInputFields(sal_True, sal_False);
    else if ( pBtn == m_pBtnDelete ) EndDialog(RET_SNAP_DELETE);

    return 0;
}

/**
 * fills provided item sets with dialog box attributes
 */
void SdSnapLineDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    SnapKind eKind;

    if ( m_pRbHorz->IsChecked() )      eKind = SK_HORIZONTAL;
    else if ( m_pRbVert->IsChecked() ) eKind = SK_VERTICAL;
    else                            eKind = SK_POINT;

    nXValue = Fraction( GetCoreValue( *m_pMtrFldX, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    nYValue = Fraction( GetCoreValue( *m_pMtrFldY, SFX_MAPUNIT_100TH_MM) ) * aUIScale;

    rOutAttrs.Put(SfxAllEnumItem(ATTR_SNAPLINE_KIND, (sal_uInt16)eKind));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_X, nXValue));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, nYValue));
}

void SdSnapLineDlg::HideRadioGroup()
{
    m_pRadioGroup->Hide();
}

/**
 * disable X or Y input fields
 */
void SdSnapLineDlg::SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY)
{
    if ( bEnableX )
    {
        if ( !m_pMtrFldX->IsEnabled() )
            m_pMtrFldX->SetValue(nXValue);
        m_pMtrFldX->Enable();
        m_pFtX->Enable();
    }
    else if ( m_pMtrFldX->IsEnabled() )
    {
        nXValue = static_cast<long>(m_pMtrFldX->GetValue());
        m_pMtrFldX->SetText(OUString());
        m_pMtrFldX->Disable();
        m_pFtX->Disable();
    }
    if ( bEnableY )
    {
        if ( !m_pMtrFldY->IsEnabled() )
            m_pMtrFldY->SetValue(nYValue);
        m_pMtrFldY->Enable();
        m_pFtY->Enable();
    }
    else if ( m_pMtrFldY->IsEnabled() )
    {
        nYValue = static_cast<long>(m_pMtrFldY->GetValue());
        m_pMtrFldY->SetText(OUString());
        m_pMtrFldY->Disable();
        m_pFtY->Disable();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
