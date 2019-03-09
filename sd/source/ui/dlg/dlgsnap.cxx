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
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>
#include <svtools/unitconv.hxx>

#include <sdattr.hrc>
#include <View.hxx>
#include <drawdoc.hxx>
#include <dlgsnap.hxx>
#include <sdenumdef.hxx>

/**
 * dialog to adjust grid (scarcely ESO!)
 */
SdSnapLineDlg::SdSnapLineDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View const * pView)
    : GenericDialogController(pWindow, "modules/sdraw/ui/dlgsnap.ui", "SnapObjectDialog")
    , aUIScale(pView->GetDoc().GetUIScale())
    , m_xFtX(m_xBuilder->weld_label("xlabel"))
    , m_xMtrFldX(m_xBuilder->weld_metric_spin_button("x", FieldUnit::CM))
    , m_xFtY(m_xBuilder->weld_label("ylabel"))
    , m_xMtrFldY(m_xBuilder->weld_metric_spin_button("y", FieldUnit::CM))
    , m_xRadioGroup(m_xBuilder->weld_widget("radiogroup"))
    , m_xRbPoint(m_xBuilder->weld_radio_button("point"))
    , m_xRbVert(m_xBuilder->weld_radio_button("vert"))
    , m_xRbHorz(m_xBuilder->weld_radio_button("horz"))
    , m_xBtnDelete(m_xBuilder->weld_button("delete"))
{
    m_xRbHorz->connect_clicked(LINK(this, SdSnapLineDlg, ClickHdl));
    m_xRbVert->connect_clicked(LINK(this, SdSnapLineDlg, ClickHdl));
    m_xRbPoint->connect_clicked(LINK(this, SdSnapLineDlg, ClickHdl));

    m_xBtnDelete->connect_clicked(LINK(this, SdSnapLineDlg, ClickHdl));

    FieldUnit eUIUnit = pView->GetDoc().GetUIUnit();
    SetFieldUnit(*m_xMtrFldX, eUIUnit, true);
    SetFieldUnit(*m_xMtrFldY, eUIUnit, true);

    // get WorkArea
    ::tools::Rectangle aWorkArea = pView->GetWorkArea();

    // determine PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Where's the Pool?" );
    MapUnit ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // #i48497# Consider page origin
    SdrPageView* pPV = pView->GetSdrPageView();
    Point aLeftTop(aWorkArea.Left()+1, aWorkArea.Top()+1);
    pPV->LogicToPagePos(aLeftTop);
    Point aRightBottom(aWorkArea.Right()-2, aWorkArea.Bottom()-2);
    pPV->LogicToPagePos(aRightBottom);

    // determine max and min values depending on
    // WorkArea, PoolUnit and FieldUnit:
    auto const map = [ePoolUnit](std::unique_ptr<weld::MetricSpinButton> const & msb, long value) {
            auto const n1 = OutputDevice::LogicToLogic(value, ePoolUnit, MapUnit::Map100thMM);
            auto const n2 = msb->normalize(n1);
            auto const n3 = msb->convert_value_from(n2, FieldUnit::MM_100TH);
            auto const n4 = msb->convert_value_to(n3, FieldUnit::NONE);
            return n4;
        };
    m_xMtrFldX->set_range(map(m_xMtrFldX, aLeftTop.X()), map(m_xMtrFldX, aRightBottom.X()),
                          FieldUnit::NONE);
    m_xMtrFldY->set_range(map(m_xMtrFldY, aLeftTop.Y()), map(m_xMtrFldY, aRightBottom.Y()),
                          FieldUnit::NONE);

    // set values
    nXValue = static_cast<const SfxInt32Item&>( rInAttrs.Get(ATTR_SNAPLINE_X)).GetValue();
    nYValue = static_cast<const SfxInt32Item&>( rInAttrs.Get(ATTR_SNAPLINE_Y)).GetValue();
    nXValue = sal_Int32(nXValue / aUIScale);
    nYValue = sal_Int32(nYValue / aUIScale);
    SetMetricValue(*m_xMtrFldX, nXValue, MapUnit::Map100thMM);
    SetMetricValue(*m_xMtrFldY, nYValue, MapUnit::Map100thMM);

    m_xRbPoint->set_active(true);
}

SdSnapLineDlg::~SdSnapLineDlg()
{
}

/**
 * fills provided item sets with dialog box attributes
 */
IMPL_LINK( SdSnapLineDlg, ClickHdl, weld::Button&, rBtn, void )
{
    if (&rBtn == m_xRbPoint.get())   SetInputFields(true, true);
    else if (&rBtn == m_xRbHorz.get())    SetInputFields(false, true);
    else if (&rBtn == m_xRbVert.get())    SetInputFields(true, false);
    else if (&rBtn == m_xBtnDelete.get()) m_xDialog->response(RET_SNAP_DELETE);
}

/**
 * fills provided item sets with dialog box attributes
 */
void SdSnapLineDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    SnapKind eKind;

    if (m_xRbHorz->get_active())      eKind = SK_HORIZONTAL;
    else if (m_xRbVert->get_active()) eKind = SK_VERTICAL;
    else                              eKind = SK_POINT;

    nXValue = sal_Int32(GetCoreValue(*m_xMtrFldX, MapUnit::Map100thMM) * aUIScale);
    nYValue = sal_Int32(GetCoreValue(*m_xMtrFldY, MapUnit::Map100thMM) * aUIScale);

    rOutAttrs.Put(SfxAllEnumItem(ATTR_SNAPLINE_KIND, static_cast<sal_uInt16>(eKind)));
    rOutAttrs.Put(SfxInt32Item(ATTR_SNAPLINE_X, nXValue));
    rOutAttrs.Put(SfxInt32Item(ATTR_SNAPLINE_Y, nYValue));
}

void SdSnapLineDlg::HideRadioGroup()
{
    m_xRadioGroup->hide();
}

/**
 * disable X or Y input fields
 */
void SdSnapLineDlg::SetInputFields(bool bEnableX, bool bEnableY)
{
    if ( bEnableX )
    {
        if (!m_xMtrFldX->get_sensitive())
            m_xMtrFldX->set_value(nXValue, FieldUnit::NONE);
        m_xMtrFldX->set_sensitive(true);
        m_xFtX->set_sensitive(true);
    }
    else if (m_xMtrFldX->get_sensitive())
    {
        nXValue = m_xMtrFldX->get_value(FieldUnit::NONE);
        m_xMtrFldX->set_text(OUString());
        m_xMtrFldX->set_sensitive(false);
        m_xFtX->set_sensitive(false);
    }
    if ( bEnableY )
    {
        if (!m_xMtrFldY->get_sensitive())
            m_xMtrFldY->set_value(nYValue, FieldUnit::NONE);
        m_xMtrFldY->set_sensitive(true);
        m_xFtY->set_sensitive(true);
    }
    else if (m_xMtrFldY->get_sensitive())
    {
        nYValue = m_xMtrFldY->get_value(FieldUnit::NONE);
        m_xMtrFldY->set_text(OUString());
        m_xMtrFldY->set_sensitive(false);
        m_xFtY->set_sensitive(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
