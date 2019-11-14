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

#include <ChartResourceGroupDlgs.hxx>
#include <ChartTypeDialogController.hxx>

#include <strings.hrc>
#include <ResId.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

const sal_uInt16 CUBIC_SPLINE_POS = 0;
const sal_uInt16 B_SPLINE_POS = 1;

SplinePropertiesDialog::SplinePropertiesDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/schart/ui/smoothlinesdlg.ui", "SmoothLinesDialog")
    , m_xLB_Spline_Type(m_xBuilder->weld_combo_box("SplineTypeComboBox"))
    , m_xMF_SplineResolution(m_xBuilder->weld_spin_button("ResolutionSpinbutton"))
    , m_xFT_SplineOrder(m_xBuilder->weld_label("PolynomialsLabel"))
    , m_xMF_SplineOrder(m_xBuilder->weld_spin_button("PolynomialsSpinButton"))
{
    m_xDialog->set_title(SchResId(STR_DLG_SMOOTH_LINE_PROPERTIES));

    m_xLB_Spline_Type->connect_changed(LINK(this, SplinePropertiesDialog, SplineTypeListBoxHdl));
}

void SplinePropertiesDialog::fillControls(const ChartTypeParameter& rParameter)
{
    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_CUBIC_SPLINES:
            m_xLB_Spline_Type->set_active(CUBIC_SPLINE_POS);
            break;
        case CurveStyle_B_SPLINES:
            m_xLB_Spline_Type->set_active(B_SPLINE_POS);
            break;
        default:
            m_xLB_Spline_Type->set_active(CUBIC_SPLINE_POS);
            break;
    }
    m_xMF_SplineOrder->set_value(rParameter.nSplineOrder);
    m_xMF_SplineResolution->set_value(rParameter.nCurveResolution);

    //dis/enabling
    m_xFT_SplineOrder->set_sensitive(m_xLB_Spline_Type->get_active() == B_SPLINE_POS);
    m_xMF_SplineOrder->set_sensitive(m_xLB_Spline_Type->get_active() == B_SPLINE_POS);
}

void SplinePropertiesDialog::fillParameter(ChartTypeParameter& rParameter, bool bSmoothLines)
{
    if (!bSmoothLines)
        rParameter.eCurveStyle = CurveStyle_LINES;
    else if (m_xLB_Spline_Type->get_active() == CUBIC_SPLINE_POS)
        rParameter.eCurveStyle = CurveStyle_CUBIC_SPLINES;
    else if (m_xLB_Spline_Type->get_active() == B_SPLINE_POS)
        rParameter.eCurveStyle = CurveStyle_B_SPLINES;

    rParameter.nCurveResolution = m_xMF_SplineResolution->get_value();
    rParameter.nSplineOrder = m_xMF_SplineOrder->get_value();
}

IMPL_LINK_NOARG(SplinePropertiesDialog, SplineTypeListBoxHdl, weld::ComboBox&, void)
{
    m_xFT_SplineOrder->set_sensitive(m_xLB_Spline_Type->get_active() == B_SPLINE_POS);
    m_xMF_SplineOrder->set_sensitive(m_xLB_Spline_Type->get_active() == B_SPLINE_POS);
}

SteppedPropertiesDialog::SteppedPropertiesDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/schart/ui/steppedlinesdlg.ui", "SteppedLinesDialog")
    , m_xRB_Start(m_xBuilder->weld_radio_button("step_start_rb"))
    , m_xRB_End(m_xBuilder->weld_radio_button("step_end_rb"))
    , m_xRB_CenterX(m_xBuilder->weld_radio_button("step_center_x_rb"))
    , m_xRB_CenterY(m_xBuilder->weld_radio_button("step_center_y_rb"))
{
    m_xDialog->set_title(SchResId(STR_DLG_STEPPED_LINE_PROPERTIES));
}

void SteppedPropertiesDialog::fillControls(const ChartTypeParameter& rParameter)
{
    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_STEP_END:
            m_xRB_End->set_active(true);
            break;
        case CurveStyle_STEP_CENTER_X:
            m_xRB_CenterX->set_active(true);
            break;
        case CurveStyle_STEP_CENTER_Y:
            m_xRB_CenterY->set_active(true);
            break;
        default: // includes CurveStyle_STEP_START
            m_xRB_Start->set_active(true);
            break;
    }
}
void SteppedPropertiesDialog::fillParameter(ChartTypeParameter& rParameter, bool bSteppedLines)
{
    if (!bSteppedLines)
        rParameter.eCurveStyle = CurveStyle_LINES;
    else if (m_xRB_CenterY->get_active())
        rParameter.eCurveStyle = CurveStyle_STEP_CENTER_Y;
    else if (m_xRB_Start->get_active())
        rParameter.eCurveStyle = CurveStyle_STEP_START;
    else if (m_xRB_End->get_active())
        rParameter.eCurveStyle = CurveStyle_STEP_END;
    else if (m_xRB_CenterX->get_active())
        rParameter.eCurveStyle = CurveStyle_STEP_CENTER_X;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
