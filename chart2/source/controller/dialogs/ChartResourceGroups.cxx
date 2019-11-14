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

#include <ChartResourceGroups.hxx>
#include <ChartResourceGroupDlgs.hxx>

#include <strings.hrc>
#include <ResId.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

Dim3DLookResourceGroup::Dim3DLookResourceGroup(weld::Builder* pBuilder)
    : ChangingResource()
    , m_xCB_3DLook(pBuilder->weld_check_button("3dlook"))
    , m_xLB_Scheme(pBuilder->weld_combo_box("3dscheme"))
{
    m_xCB_3DLook->connect_toggled(LINK(this, Dim3DLookResourceGroup, Dim3DLookCheckHdl));
    m_xLB_Scheme->connect_changed(LINK(this, Dim3DLookResourceGroup, SelectSchemeHdl));
}

void Dim3DLookResourceGroup::showControls(bool bShow)
{
    m_xCB_3DLook->set_visible(bShow);
    m_xLB_Scheme->set_visible(bShow);
}

void Dim3DLookResourceGroup::fillControls(const ChartTypeParameter& rParameter)
{
    m_xCB_3DLook->set_active(rParameter.b3DLook);
    m_xLB_Scheme->set_sensitive(rParameter.b3DLook);

    if (rParameter.eThreeDLookScheme == ThreeDLookScheme_Simple)
        m_xLB_Scheme->set_active(POS_3DSCHEME_SIMPLE);
    else if (rParameter.eThreeDLookScheme == ThreeDLookScheme_Realistic)
        m_xLB_Scheme->set_active(POS_3DSCHEME_REALISTIC);
    else
        m_xLB_Scheme->set_active(-1);
}

void Dim3DLookResourceGroup::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.b3DLook = m_xCB_3DLook->get_active();
    const int nPos = m_xLB_Scheme->get_active();
    if (nPos == POS_3DSCHEME_SIMPLE)
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Simple;
    else if (nPos == POS_3DSCHEME_REALISTIC)
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;
    else
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Unknown;
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup, Dim3DLookCheckHdl, weld::ToggleButton&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup, SelectSchemeHdl, weld::ComboBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

SortByXValuesResourceGroup::SortByXValuesResourceGroup(weld::Builder* pBuilder)
    : ChangingResource()
    , m_xCB_XValueSorting(pBuilder->weld_check_button("sort"))
{
    m_xCB_XValueSorting->connect_toggled(
        LINK(this, SortByXValuesResourceGroup, SortByXValuesCheckHdl));
}

void SortByXValuesResourceGroup::showControls(bool bShow)
{
    m_xCB_XValueSorting->set_visible(bShow);
}

void SortByXValuesResourceGroup::fillControls(const ChartTypeParameter& rParameter)
{
    m_xCB_XValueSorting->set_active(rParameter.bSortByXValues);
}

void SortByXValuesResourceGroup::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.bSortByXValues = m_xCB_XValueSorting->get_active();
}

IMPL_LINK_NOARG(SortByXValuesResourceGroup, SortByXValuesCheckHdl, weld::ToggleButton&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

StackingResourceGroup::StackingResourceGroup(weld::Builder* pBuilder)
    : ChangingResource()
    , m_xCB_Stacked(pBuilder->weld_check_button("stack"))
    , m_xRB_Stack_Y(pBuilder->weld_radio_button("ontop"))
    , m_xRB_Stack_Y_Percent(pBuilder->weld_radio_button("percent"))
    , m_xRB_Stack_Z(pBuilder->weld_radio_button("deep"))
{
    m_xCB_Stacked->connect_toggled(LINK(this, StackingResourceGroup, StackingEnableHdl));
    m_xRB_Stack_Y->connect_toggled(LINK(this, StackingResourceGroup, StackingChangeHdl));
    m_xRB_Stack_Y_Percent->connect_toggled(LINK(this, StackingResourceGroup, StackingChangeHdl));
    m_xRB_Stack_Z->connect_toggled(LINK(this, StackingResourceGroup, StackingChangeHdl));
}

void StackingResourceGroup::showControls(bool bShow, bool bShowDeepStacking)
{
    m_xCB_Stacked->set_visible(bShow);
    m_xRB_Stack_Y->set_visible(bShow);
    m_xRB_Stack_Y_Percent->set_visible(bShow);
    m_xRB_Stack_Z->set_visible(bShow && bShowDeepStacking);
}

void StackingResourceGroup::fillControls(const ChartTypeParameter& rParameter)
{
    m_xCB_Stacked->set_active(
        rParameter.eStackMode != GlobalStackMode_NONE
        && rParameter.eStackMode
               != GlobalStackMode_STACK_Z); //todo remove this condition if z stacking radio button is really used
    switch (rParameter.eStackMode)
    {
        case GlobalStackMode_STACK_Y:
            m_xRB_Stack_Y->set_active(true);
            break;
        case GlobalStackMode_STACK_Y_PERCENT:
            m_xRB_Stack_Y_Percent->set_active(true);
            break;
        case GlobalStackMode_STACK_Z:
            //todo uncomment this condition if z stacking radio button is really used
            /*
            if( rParameter.b3DLook )
                m_xRB_Stack_Z->set_active(true);
            else
            */
            m_xRB_Stack_Y->set_active(true);
            break;
        default:
            m_xRB_Stack_Y->set_active(true);
            break;
    }
    //dis/enabling
    m_xCB_Stacked->set_sensitive(!rParameter.bXAxisWithValues);
    m_xRB_Stack_Y->set_sensitive(m_xCB_Stacked->get_active() && !rParameter.bXAxisWithValues);
    m_xRB_Stack_Y_Percent->set_sensitive(m_xCB_Stacked->get_active()
                                         && !rParameter.bXAxisWithValues);
    m_xRB_Stack_Z->set_sensitive(m_xCB_Stacked->get_active() && rParameter.b3DLook);
}

void StackingResourceGroup::fillParameter(ChartTypeParameter& rParameter)
{
    if (!m_xCB_Stacked->get_active())
        rParameter.eStackMode = GlobalStackMode_NONE;
    else if (m_xRB_Stack_Y->get_active())
        rParameter.eStackMode = GlobalStackMode_STACK_Y;
    else if (m_xRB_Stack_Y_Percent->get_active())
        rParameter.eStackMode = GlobalStackMode_STACK_Y_PERCENT;
    else if (m_xRB_Stack_Z->get_active())
        rParameter.eStackMode = GlobalStackMode_STACK_Z;
}

IMPL_LINK(StackingResourceGroup, StackingChangeHdl, weld::ToggleButton&, rRadio, void)
{
    //for each radio click there are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if (m_pChangeListener && rRadio.get_active())
        m_pChangeListener->stateChanged();
}

IMPL_LINK_NOARG(StackingResourceGroup, StackingEnableHdl, weld::ToggleButton&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

SplineResourceGroup::SplineResourceGroup(weld::Builder* pBuilder, weld::Window* pParent)
    : ChangingResource()
    , m_pParent(pParent)
    , m_xFT_LineType(pBuilder->weld_label("linetypeft"))
    , m_xLB_LineType(pBuilder->weld_combo_box("linetype"))
    , m_xPB_DetailsDialog(pBuilder->weld_button("properties"))
{
    m_xLB_LineType->connect_changed(LINK(this, SplineResourceGroup, LineTypeChangeHdl));
}

SplinePropertiesDialog& SplineResourceGroup::getSplinePropertiesDialog()
{
    if (!m_xSplinePropertiesDialog)
    {
        m_xSplinePropertiesDialog.reset(new SplinePropertiesDialog(m_pParent));
    }
    return *m_xSplinePropertiesDialog;
}

SteppedPropertiesDialog& SplineResourceGroup::getSteppedPropertiesDialog()
{
    if (!m_xSteppedPropertiesDialog)
    {
        m_xSteppedPropertiesDialog.reset(new SteppedPropertiesDialog(m_pParent));
    }
    return *m_xSteppedPropertiesDialog;
}

void SplineResourceGroup::showControls(bool bShow)
{
    m_xFT_LineType->set_visible(bShow);
    m_xLB_LineType->set_visible(bShow);
    m_xPB_DetailsDialog->set_visible(bShow);
}

void SplineResourceGroup::fillControls(const ChartTypeParameter& rParameter)
{
    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_LINES:
            m_xLB_LineType->set_active(POS_LINETYPE_STRAIGHT);
            m_xPB_DetailsDialog->set_sensitive(false);
            break;
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
            m_xLB_LineType->set_active(POS_LINETYPE_SMOOTH);
            m_xPB_DetailsDialog->set_sensitive(true);
            m_xPB_DetailsDialog->connect_clicked(
                LINK(this, SplineResourceGroup, SplineDetailsDialogHdl));
            m_xPB_DetailsDialog->set_tooltip_text(SchResId(STR_DLG_SMOOTH_LINE_PROPERTIES));
            getSplinePropertiesDialog().fillControls(rParameter);
            break;
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
            m_xLB_LineType->set_active(POS_LINETYPE_STEPPED);
            m_xPB_DetailsDialog->set_sensitive(true);
            m_xPB_DetailsDialog->connect_clicked(
                LINK(this, SplineResourceGroup, SteppedDetailsDialogHdl));
            m_xPB_DetailsDialog->set_tooltip_text(SchResId(STR_DLG_STEPPED_LINE_PROPERTIES));
            getSteppedPropertiesDialog().fillControls(rParameter);
            break;
        default:
            m_xLB_LineType->set_active(-1);
            m_xPB_DetailsDialog->set_sensitive(false);
    }
}
void SplineResourceGroup::fillParameter(ChartTypeParameter& rParameter)
{
    switch (m_xLB_LineType->get_active())
    {
        case POS_LINETYPE_SMOOTH:
            getSplinePropertiesDialog().fillParameter(rParameter, true);
            break;
        case POS_LINETYPE_STEPPED:
            getSteppedPropertiesDialog().fillParameter(rParameter, true);
            break;
        default: // includes POS_LINETYPE_STRAIGHT
            rParameter.eCurveStyle = CurveStyle_LINES;
            break;
    }
}

IMPL_LINK_NOARG(SplineResourceGroup, LineTypeChangeHdl, weld::ComboBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

IMPL_LINK_NOARG(SplineResourceGroup, SplineDetailsDialogHdl, weld::Button&, void)
{
    ChartTypeParameter aOldParameter;
    getSplinePropertiesDialog().fillParameter(aOldParameter,
                                              m_xLB_LineType->get_active() == POS_LINETYPE_SMOOTH);

    const sal_Int32 iOldLineTypePos = m_xLB_LineType->get_active();
    m_xLB_LineType->set_active(POS_LINETYPE_SMOOTH);
    if (getSplinePropertiesDialog().run() == RET_OK)
    {
        if (m_pChangeListener)
            m_pChangeListener->stateChanged();
    }
    else
    {
        //restore old state:
        m_xLB_LineType->set_active(iOldLineTypePos);
        getSplinePropertiesDialog().fillControls(aOldParameter);
    }
}

IMPL_LINK_NOARG(SplineResourceGroup, SteppedDetailsDialogHdl, weld::Button&, void)
{
    ChartTypeParameter aOldParameter;
    getSteppedPropertiesDialog().fillParameter(aOldParameter, m_xLB_LineType->get_active()
                                                                  == POS_LINETYPE_STEPPED);

    const sal_Int32 iOldLineTypePos = m_xLB_LineType->get_active();
    m_xLB_LineType->set_active(POS_LINETYPE_STEPPED);
    if (getSteppedPropertiesDialog().run() == RET_OK)
    {
        if (m_pChangeListener)
            m_pChangeListener->stateChanged();
    }
    else
    {
        //restore old state:
        m_xLB_LineType->set_active(iOldLineTypePos);
        getSteppedPropertiesDialog().fillControls(aOldParameter);
    }
}

GeometryResourceGroup::GeometryResourceGroup(weld::Builder* pBuilder)
    : ChangingResource()
    , m_aGeometryResources(pBuilder)
{
    m_aGeometryResources.connect_changed(LINK(this, GeometryResourceGroup, GeometryChangeHdl));
}

void GeometryResourceGroup::showControls(bool bShow) { m_aGeometryResources.set_visible(bShow); }

void GeometryResourceGroup::fillControls(const ChartTypeParameter& rParameter)
{
    sal_uInt16 nGeometry3D = static_cast<sal_uInt16>(rParameter.nGeometry3D);
    m_aGeometryResources.select(nGeometry3D);
    m_aGeometryResources.set_sensitive(rParameter.b3DLook);
}

void GeometryResourceGroup::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.nGeometry3D = 1;
    int nSelected = m_aGeometryResources.get_selected_index();
    if (nSelected != -1)
        rParameter.nGeometry3D = nSelected;
}

IMPL_LINK_NOARG(GeometryResourceGroup, GeometryChangeHdl, weld::TreeView&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
