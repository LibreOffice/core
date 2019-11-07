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

#include "ChartTypePanel.hxx"

#include <bitmaps.hlst>
#include <ChartController.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ChartResourceGroupDlgs.hxx>
#include <ChartTypeDialogController.hxx>
#include <ChartTypeHelper.hxx>
#include <DiagramHelper.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <unonames.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

#include <comphelper/processfactory.hxx>
#include <svtools/valueset.hxx>
#include <tools/diagnose_ex.h>

//Borrowed from vcl/inc/bitmaps.hlst
#define IMG_ERROR "dbaccess/res/exerror.png"
#define IMG_INFO "dbaccess/res/exinfo.png"
#define IMG_WARN "dbaccess/res/exwarning.png"

using namespace css;
using namespace css::uno;

namespace chart
{
enum
{
    POS_3DSCHEME_SIMPLE = 0,
    POS_3DSCHEME_REALISTIC = 1
};

class Dim3DLookResourceGroup_unwelded : public ChangingResource
{
public:
    explicit Dim3DLookResourceGroup_unwelded(VclBuilderContainer* pWindow);

    void showControls(bool bShow);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter);

private:
    DECL_LINK(Dim3DLookCheckHdl, CheckBox&, void);
    DECL_LINK(SelectSchemeHdl, ListBox&, void);

private:
    VclPtr<CheckBox> m_pCB_3DLook;
    VclPtr<ListBox> m_pLB_Scheme;
};

Dim3DLookResourceGroup_unwelded::Dim3DLookResourceGroup_unwelded(VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pCB_3DLook, "3dlook");
    pWindow->get(m_pLB_Scheme, "3dscheme");

    m_pCB_3DLook->SetToggleHdl(LINK(this, Dim3DLookResourceGroup_unwelded, Dim3DLookCheckHdl));
    m_pLB_Scheme->SetSelectHdl(LINK(this, Dim3DLookResourceGroup_unwelded, SelectSchemeHdl));
}

void Dim3DLookResourceGroup_unwelded::showControls(bool bShow)
{
    m_pCB_3DLook->Show(bShow);
    m_pLB_Scheme->Show(bShow);
}

void Dim3DLookResourceGroup_unwelded::fillControls(const ChartTypeParameter& rParameter)
{
    m_pCB_3DLook->Check(rParameter.b3DLook);
    m_pLB_Scheme->Enable(rParameter.b3DLook);

    if (rParameter.eThreeDLookScheme == ThreeDLookScheme_Simple)
        m_pLB_Scheme->SelectEntryPos(POS_3DSCHEME_SIMPLE);
    else if (rParameter.eThreeDLookScheme == ThreeDLookScheme_Realistic)
        m_pLB_Scheme->SelectEntryPos(POS_3DSCHEME_REALISTIC);
    else
        m_pLB_Scheme->SetNoSelection();
}

void Dim3DLookResourceGroup_unwelded::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.b3DLook = m_pCB_3DLook->IsChecked();
    const sal_Int32 nPos = m_pLB_Scheme->GetSelectedEntryPos();
    if (nPos == POS_3DSCHEME_SIMPLE)
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Simple;
    else if (nPos == POS_3DSCHEME_REALISTIC)
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;
    else
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Unknown;
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup_unwelded, Dim3DLookCheckHdl, CheckBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup_unwelded, SelectSchemeHdl, ListBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}

class StackingResourceGroup_unwelded : public ChangingResource
{
public:
    explicit StackingResourceGroup_unwelded(VclBuilderContainer* pWindow);

    void showControls(bool bShow, bool bShowDeepStacking);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter);

private:
    DECL_LINK(StackingChangeHdl, RadioButton&, void);
    DECL_LINK(StackingEnableHdl, CheckBox&, void);

private:
    VclPtr<CheckBox> m_pCB_Stacked;
    VclPtr<RadioButton> m_pRB_Stack_Y;
    VclPtr<RadioButton> m_pRB_Stack_Y_Percent;
    VclPtr<RadioButton> m_pRB_Stack_Z;
};

StackingResourceGroup_unwelded::StackingResourceGroup_unwelded(VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pCB_Stacked, "stack");
    pWindow->get(m_pRB_Stack_Y, "ontop");
    pWindow->get(m_pRB_Stack_Y_Percent, "percent");
    pWindow->get(m_pRB_Stack_Z, "deep");

    m_pCB_Stacked->SetToggleHdl(LINK(this, StackingResourceGroup_unwelded, StackingEnableHdl));
    m_pRB_Stack_Y->SetToggleHdl(LINK(this, StackingResourceGroup_unwelded, StackingChangeHdl));
    m_pRB_Stack_Y_Percent->SetToggleHdl(
        LINK(this, StackingResourceGroup_unwelded, StackingChangeHdl));
    m_pRB_Stack_Z->SetToggleHdl(LINK(this, StackingResourceGroup_unwelded, StackingChangeHdl));
}

void StackingResourceGroup_unwelded::showControls(bool bShow, bool bShowDeepStacking)
{
    m_pCB_Stacked->Show(bShow);
    m_pRB_Stack_Y->Show(bShow);
    m_pRB_Stack_Y_Percent->Show(bShow);
    m_pRB_Stack_Z->Show(bShow && bShowDeepStacking);
}

void StackingResourceGroup_unwelded::fillControls(const ChartTypeParameter& rParameter)
{
    m_pCB_Stacked->Check(
        rParameter.eStackMode != GlobalStackMode_NONE
        && rParameter.eStackMode
               != GlobalStackMode_STACK_Z); //todo remove this condition if z stacking radio button is really used
    switch (rParameter.eStackMode)
    {
        case GlobalStackMode_STACK_Y:
            m_pRB_Stack_Y->Check();
            break;
        case GlobalStackMode_STACK_Y_PERCENT:
            m_pRB_Stack_Y_Percent->Check();
            break;
        case GlobalStackMode_STACK_Z:
            //todo uncomment this condition if z stacking radio button is really used
            /*
            if( rParameter.b3DLook )
                m_pRB_Stack_Z->Check();
            else
            */
            m_pRB_Stack_Y->Check();
            break;
        default:
            m_pRB_Stack_Y->Check();
            break;
    }
    //dis/enabling
    m_pCB_Stacked->Enable(!rParameter.bXAxisWithValues);
    m_pRB_Stack_Y->Enable(m_pCB_Stacked->IsChecked() && !rParameter.bXAxisWithValues);
    m_pRB_Stack_Y_Percent->Enable(m_pCB_Stacked->IsChecked() && !rParameter.bXAxisWithValues);
    m_pRB_Stack_Z->Enable(m_pCB_Stacked->IsChecked() && rParameter.b3DLook);
}

void StackingResourceGroup_unwelded::fillParameter(ChartTypeParameter& rParameter)
{
    if (!m_pCB_Stacked->IsChecked())
        rParameter.eStackMode = GlobalStackMode_NONE;
    else if (m_pRB_Stack_Y->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y;
    else if (m_pRB_Stack_Y_Percent->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y_PERCENT;
    else if (m_pRB_Stack_Z->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Z;
}

IMPL_LINK(StackingResourceGroup_unwelded, StackingChangeHdl, RadioButton&, rRadio, void)
{
    //for each radio click there are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if (m_pChangeListener && rRadio.IsChecked())
        m_pChangeListener->stateChanged(this);
}

IMPL_LINK_NOARG(StackingResourceGroup_unwelded, StackingEnableHdl, CheckBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}

#define POS_LINETYPE_STRAIGHT 0
#define POS_LINETYPE_SMOOTH 1
#define POS_LINETYPE_STEPPED 2

class SplineResourceGroup_unwelded : public ChangingResource
{
public:
    explicit SplineResourceGroup_unwelded(VclBuilderContainer* pWindow);

    void showControls(bool bShow);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter);

private:
    DECL_LINK(LineTypeChangeHdl, ListBox&, void);
    DECL_LINK(SplineDetailsDialogHdl, Button*, void);
    DECL_LINK(SteppedDetailsDialogHdl, Button*, void);
    SplinePropertiesDialog& getSplinePropertiesDialog();
    SteppedPropertiesDialog& getSteppedPropertiesDialog();

private:
    VclPtr<FixedText> m_pFT_LineType;
    VclPtr<ListBox> m_pLB_LineType;
    VclPtr<PushButton> m_pPB_DetailsDialog;
    std::unique_ptr<SplinePropertiesDialog> m_xSplinePropertiesDialog;
    std::unique_ptr<SteppedPropertiesDialog> m_xSteppedPropertiesDialog;
};

SplineResourceGroup_unwelded::SplineResourceGroup_unwelded(VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pFT_LineType, "linetypeft");
    pWindow->get(m_pLB_LineType, "linetype");
    pWindow->get(m_pPB_DetailsDialog, "properties");

    m_pLB_LineType->SetSelectHdl(LINK(this, SplineResourceGroup_unwelded, LineTypeChangeHdl));
}

SplinePropertiesDialog& SplineResourceGroup_unwelded::getSplinePropertiesDialog()
{
    if (!m_xSplinePropertiesDialog.get())
    {
        Dialog* pDialog = m_pPB_DetailsDialog->GetParentDialog();
        m_xSplinePropertiesDialog.reset(
            new SplinePropertiesDialog(pDialog ? pDialog->GetFrameWeld() : nullptr));
    }
    return *m_xSplinePropertiesDialog;
}

SteppedPropertiesDialog& SplineResourceGroup_unwelded::getSteppedPropertiesDialog()
{
    if (!m_xSteppedPropertiesDialog)
    {
        m_xSteppedPropertiesDialog.reset(
            new SteppedPropertiesDialog(m_pPB_DetailsDialog->GetFrameWeld()));
    }
    return *m_xSteppedPropertiesDialog;
}

void SplineResourceGroup_unwelded::showControls(bool bShow)
{
    m_pFT_LineType->Show(bShow);
    m_pLB_LineType->Show(bShow);
    m_pPB_DetailsDialog->Show(bShow);
}

void SplineResourceGroup_unwelded::fillControls(const ChartTypeParameter& rParameter)
{
    switch (rParameter.eCurveStyle)
    {
        case chart2::CurveStyle_LINES:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STRAIGHT);
            m_pPB_DetailsDialog->Enable(false);
            break;
        case chart2::CurveStyle_CUBIC_SPLINES:
        case chart2::CurveStyle_B_SPLINES:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_SMOOTH);
            m_pPB_DetailsDialog->Enable();
            m_pPB_DetailsDialog->SetClickHdl(
                LINK(this, SplineResourceGroup_unwelded, SplineDetailsDialogHdl));
            m_pPB_DetailsDialog->SetQuickHelpText(SchResId(STR_DLG_SMOOTH_LINE_PROPERTIES));
            getSplinePropertiesDialog().fillControls(rParameter);
            break;
        case chart2::CurveStyle_STEP_START:
        case chart2::CurveStyle_STEP_END:
        case chart2::CurveStyle_STEP_CENTER_X:
        case chart2::CurveStyle_STEP_CENTER_Y:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STEPPED);
            m_pPB_DetailsDialog->Enable();
            m_pPB_DetailsDialog->SetClickHdl(
                LINK(this, SplineResourceGroup_unwelded, SteppedDetailsDialogHdl));
            m_pPB_DetailsDialog->SetQuickHelpText(SchResId(STR_DLG_STEPPED_LINE_PROPERTIES));
            getSteppedPropertiesDialog().fillControls(rParameter);
            break;
        default:
            m_pLB_LineType->SetNoSelection();
            m_pPB_DetailsDialog->Enable(false);
    }
}
void SplineResourceGroup_unwelded::fillParameter(ChartTypeParameter& rParameter)
{
    switch (m_pLB_LineType->GetSelectedEntryPos())
    {
        case POS_LINETYPE_SMOOTH:
            getSplinePropertiesDialog().fillParameter(rParameter, true);
            break;
        case POS_LINETYPE_STEPPED:
            getSteppedPropertiesDialog().fillParameter(rParameter, true);
            break;
        default: // includes POS_LINETYPE_STRAIGHT
            rParameter.eCurveStyle = chart2::CurveStyle_LINES;
            break;
    }
}
IMPL_LINK_NOARG(SplineResourceGroup_unwelded, LineTypeChangeHdl, ListBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}
IMPL_LINK_NOARG(SplineResourceGroup_unwelded, SplineDetailsDialogHdl, Button*, void)
{
    ChartTypeParameter aOldParameter;
    getSplinePropertiesDialog().fillParameter(aOldParameter, m_pLB_LineType->GetSelectedEntryPos()
                                                                 == POS_LINETYPE_SMOOTH);

    const sal_Int32 iOldLineTypePos = m_pLB_LineType->GetSelectedEntryPos();
    m_pLB_LineType->SelectEntryPos(POS_LINETYPE_SMOOTH);
    if (getSplinePropertiesDialog().run() == RET_OK)
    {
        if (m_pChangeListener)
            m_pChangeListener->stateChanged(this);
    }
    else
    {
        //restore old state:
        m_pLB_LineType->SelectEntryPos(iOldLineTypePos);
        getSplinePropertiesDialog().fillControls(aOldParameter);
    }
}
IMPL_LINK_NOARG(SplineResourceGroup_unwelded, SteppedDetailsDialogHdl, Button*, void)
{
    ChartTypeParameter aOldParameter;
    getSteppedPropertiesDialog().fillParameter(aOldParameter, m_pLB_LineType->GetSelectedEntryPos()
                                                                  == POS_LINETYPE_STEPPED);

    const sal_Int32 iOldLineTypePos = m_pLB_LineType->GetSelectedEntryPos();
    m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STEPPED);
    if (getSteppedPropertiesDialog().run() == RET_OK)
    {
        if (m_pChangeListener)
            m_pChangeListener->stateChanged(this);
    }
    else
    {
        //restore old state:
        m_pLB_LineType->SelectEntryPos(iOldLineTypePos);
        getSteppedPropertiesDialog().fillControls(aOldParameter);
    }
}

class BarGeometryResources_unwelded
{
public:
    explicit BarGeometryResources_unwelded(VclBuilderContainer* pParent);

    void Show(bool bShow);
    void Enable(bool bEnable);

    sal_Int32 GetSelectedEntryCount() const;
    sal_Int32 GetSelectedEntryPos() const;
    void SelectEntryPos(sal_Int32 nPos);

    void SetSelectHdl(const Link<ListBox&, void>& rLink);

private:
    VclPtr<FixedText> m_pFT_Geometry;
    VclPtr<ListBox> m_pLB_Geometry;
};

BarGeometryResources_unwelded::BarGeometryResources_unwelded(VclBuilderContainer* pWindow)
{
    pWindow->get(m_pFT_Geometry, "shapeft");
    pWindow->get(m_pLB_Geometry, "shape");
}

void BarGeometryResources_unwelded::SetSelectHdl(const Link<ListBox&, void>& rLink)
{
    m_pLB_Geometry->SetSelectHdl(rLink);
}

void BarGeometryResources_unwelded::Show(bool bShow)
{
    m_pFT_Geometry->Show(bShow);
    m_pLB_Geometry->Show(bShow);
}
void BarGeometryResources_unwelded::Enable(bool bEnable)
{
    m_pFT_Geometry->Enable(bEnable);
    m_pLB_Geometry->Enable(bEnable);
}

sal_Int32 BarGeometryResources_unwelded::GetSelectedEntryCount() const
{
    return m_pLB_Geometry->GetSelectedEntryCount();
}

sal_Int32 BarGeometryResources_unwelded::GetSelectedEntryPos() const
{
    return m_pLB_Geometry->GetSelectedEntryPos();
}

void BarGeometryResources_unwelded::SelectEntryPos(sal_Int32 nPos)
{
    if (nPos < m_pLB_Geometry->GetEntryCount())
        m_pLB_Geometry->SelectEntryPos(nPos);
}

class GeometryResourceGroup_unwelded : public ChangingResource
{
public:
    explicit GeometryResourceGroup_unwelded(VclBuilderContainer* pWindow);

    void showControls(bool bShow);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter);

private:
    DECL_LINK(GeometryChangeHdl, ListBox&, void);

private:
    BarGeometryResources_unwelded m_aGeometryResources;
};

GeometryResourceGroup_unwelded::GeometryResourceGroup_unwelded(VclBuilderContainer* pWindow)
    : ChangingResource()
    , m_aGeometryResources(pWindow)
{
    m_aGeometryResources.SetSelectHdl(
        LINK(this, GeometryResourceGroup_unwelded, GeometryChangeHdl));
}

void GeometryResourceGroup_unwelded::showControls(bool bShow) { m_aGeometryResources.Show(bShow); }

void GeometryResourceGroup_unwelded::fillControls(const ChartTypeParameter& rParameter)
{
    sal_uInt16 nGeometry3D = static_cast<sal_uInt16>(rParameter.nGeometry3D);
    m_aGeometryResources.SelectEntryPos(nGeometry3D);
    m_aGeometryResources.Enable(rParameter.b3DLook);
}

void GeometryResourceGroup_unwelded::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.nGeometry3D = 1;
    if (m_aGeometryResources.GetSelectedEntryCount())
        rParameter.nGeometry3D = m_aGeometryResources.GetSelectedEntryPos();
}

IMPL_LINK_NOARG(GeometryResourceGroup_unwelded, GeometryChangeHdl, ListBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}

class SortByXValuesResourceGroup_unwelded : public ChangingResource
{
public:
    explicit SortByXValuesResourceGroup_unwelded(VclBuilderContainer* pWindow);

    void showControls(bool bShow);

    void fillControls(const ChartTypeParameter& rParameter);
    void fillParameter(ChartTypeParameter& rParameter);

private:
    DECL_LINK(SortByXValuesCheckHdl, CheckBox&, void);

private:
    VclPtr<CheckBox> m_pCB_XValueSorting;
};

SortByXValuesResourceGroup_unwelded::SortByXValuesResourceGroup_unwelded(
    VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pCB_XValueSorting, "sort");
    m_pCB_XValueSorting->SetToggleHdl(
        LINK(this, SortByXValuesResourceGroup_unwelded, SortByXValuesCheckHdl));
}

void SortByXValuesResourceGroup_unwelded::showControls(bool bShow)
{
    m_pCB_XValueSorting->Show(bShow);
}

void SortByXValuesResourceGroup_unwelded::fillControls(const ChartTypeParameter& rParameter)
{
    m_pCB_XValueSorting->Check(rParameter.bSortByXValues);
}

void SortByXValuesResourceGroup_unwelded::fillParameter(ChartTypeParameter& rParameter)
{
    rParameter.bSortByXValues = m_pCB_XValueSorting->IsChecked();
}

IMPL_LINK_NOARG(SortByXValuesResourceGroup_unwelded, SortByXValuesCheckHdl, CheckBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged(this);
}

namespace sidebar
{
namespace
{
Image createImage(const OUString& rImage)
{
    if (rImage.lastIndexOf('.') != rImage.getLength() - 4)
    {
        assert((rImage == "dialog-warning" || rImage == "dialog-error"
                || rImage == "dialog-information")
               && "unknown stock image");
        if (rImage == "dialog-warning")
            return Image(BitmapEx(IMG_WARN));
        else if (rImage == "dialog-error")
            return Image(BitmapEx(IMG_ERROR));
        else if (rImage == "dialog-information")
            return Image(BitmapEx(IMG_INFO));
    }
    return Image(BitmapEx(rImage));
}
} //end of anonymous namespace

ChartTypePanel::ChartTypePanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               ::chart::ChartController* pController)
    : PanelLayout(pParent, "ChartTypePanel", "modules/schart/ui/sidebartype.ui", rxFrame)
    , maContext()
    , mxModel(pController->getModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mbModelValid(true)
    , m_pDim3DLookResourceGroup(new Dim3DLookResourceGroup_unwelded(this))
    , m_pStackingResourceGroup(new StackingResourceGroup_unwelded(this))
    , m_pSplineResourceGroup(new SplineResourceGroup_unwelded(this))
    , m_pGeometryResourceGroup(new GeometryResourceGroup_unwelded(this))
    , m_pSortByXValuesResourceGroup(new SortByXValuesResourceGroup_unwelded(this))
    , m_xChartModel(mxModel, css::uno::UNO_QUERY_THROW)
    , m_aChartTypeDialogControllerList(0)
    , m_pCurrentMainType(nullptr)
    , m_nChangingCalls(0)
    , m_aTimerTriggeredControllerLock(uno::Reference<frame::XModel>(m_xChartModel, uno::UNO_QUERY))
{
    get(mpChartTypeLabel, "lbl_chartType");
    get(m_pMainTypeList, "cmb_chartType");
    get(m_pSubTypeList, "subtype");

    Size aSize(m_pSubTypeList->LogicToPixel(Size(120, 40), MapMode(MapUnit::MapAppFont)));
    m_pSubTypeList->set_width_request(aSize.Width());
    m_pSubTypeList->set_height_request(aSize.Height());

    m_pMainTypeList->SetSelectHdl(LINK(this, ChartTypePanel, SelectMainTypeHdl));
    m_pSubTypeList->SetSelectHdl(LINK(this, ChartTypePanel, SelectSubTypeHdl));

    m_pSubTypeList->SetStyle(m_pSubTypeList->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER
                             | WB_NAMEFIELD | WB_FLATVALUESET | WB_3DLOOK);
    m_pSubTypeList->SetColCount(4);
    m_pSubTypeList->SetLineCount(1);

    bool bEnableComplexChartTypes = true;
    uno::Reference<beans::XPropertySet> xProps(m_xChartModel, uno::UNO_QUERY);
    if (xProps.is())
    {
        try
        {
            xProps->getPropertyValue("EnableComplexChartTypes") >>= bEnableComplexChartTypes;
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("chart2", "Exception caught. " << e);
        }
    }

    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<ColumnChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<BarChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<PieChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<AreaChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<LineChartDialogController>());

    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<XYChartDialogController>());
        m_aChartTypeDialogControllerList.push_back(
            o3tl::make_unique<BubbleChartDialogController>());
    }

    m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<NetChartDialogController>());

    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(o3tl::make_unique<StockChartDialogController>());
    }

    m_aChartTypeDialogControllerList.push_back(
        o3tl::make_unique<CombiColumnLineChartDialogController>());

    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        m_pMainTypeList->InsertEntry(elem->getName(), createImage(elem->getImage()));
        elem->setChangeListener(this);
    }

    m_pDim3DLookResourceGroup->setChangeListener(this);
    m_pStackingResourceGroup->setChangeListener(this);
    m_pSplineResourceGroup->setChangeListener(this);
    m_pGeometryResourceGroup->setChangeListener(this);
    m_pSortByXValuesResourceGroup->setChangeListener(this);

    Initialize();
}

ChartTypePanel::~ChartTypePanel() { disposeOnce(); }

void ChartTypePanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel,
                                                                    css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);

    mpChartTypeLabel.clear();
    m_pMainTypeList.clear();
    m_pSubTypeList.clear();

    //delete all resource helpers
    m_pDim3DLookResourceGroup.reset();
    m_pStackingResourceGroup.reset();
    m_pSplineResourceGroup.reset();
    m_pGeometryResourceGroup.reset();
    m_pSortByXValuesResourceGroup.reset();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(ChartTypePanel, SelectMainTypeHdl, ListBox&, void) { selectMainType(); }

IMPL_LINK_NOARG(ChartTypePanel, SelectSubTypeHdl, ValueSet*, void)
{
    /*if( m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        fillAllControls( aParameter, false );
        commitToModel( aParameter );
    }*/
}

void ChartTypePanel::Initialize()
{
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xChartModel, uno::UNO_QUERY);
    uno::Reference<css::chart2::XDiagram> xDiagram(ChartModelHelper::findDiagram(xModel));
    DiagramHelper::tTemplateWithServiceName aTemplate
        = DiagramHelper::getTemplateForDiagram(xDiagram, xTemplateManager);
    OUString aServiceName(aTemplate.second);

    bool bFound = false;

    sal_uInt16 nM = 0;
    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        if (elem->isSubType(aServiceName))
        {
            bFound = true;

            m_pMainTypeList->SelectEntryPos(nM);
            showAllControls(*elem);
            uno::Reference<beans::XPropertySet> xTemplateProps(aTemplate.first, uno::UNO_QUERY);
            ChartTypeParameter aParameter
                = elem->getChartTypeParameterForService(aServiceName, xTemplateProps);
            m_pCurrentMainType = getSelectedMainType();

            //set ThreeDLookScheme
            aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme(xDiagram);
            if (!aParameter.b3DLook && aParameter.eThreeDLookScheme != ThreeDLookScheme_Realistic)
                aParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;

            try
            {
                uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
                xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES)
                    >>= aParameter.bSortByXValues;
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }

            fillAllControls(aParameter);
            if (m_pCurrentMainType)
                m_pCurrentMainType->fillExtraControls(aParameter, m_xChartModel, xTemplateProps);
            break;
        }
        ++nM;
    }

    if (!bFound)
    {
        m_pSubTypeList->Hide();
        m_pDim3DLookResourceGroup->showControls(false);
        m_pStackingResourceGroup->showControls(false, false);
        m_pSplineResourceGroup->showControls(false);
        m_pGeometryResourceGroup->showControls(false);
        m_pSortByXValuesResourceGroup->showControls(false);
    }
}

void ChartTypePanel::updateData()
{
    // Chart Type related
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xChartModel, uno::UNO_QUERY);
    uno::Reference<css::chart2::XDiagram> xDiagram(ChartModelHelper::findDiagram(xModel));
    DiagramHelper::tTemplateWithServiceName aTemplate
        = DiagramHelper::getTemplateForDiagram(xDiagram, xTemplateManager);
    OUString aServiceName(aTemplate.second);

    sal_uInt16 nM = 0;
    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        if (elem->isSubType(aServiceName))
        {
            m_pMainTypeList->SelectEntryPos(nM);
            break;
        }
        ++nM;
    }
}

VclPtr<vcl::Window> ChartTypePanel::Create(vcl::Window* pParent,
                                           const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                           ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartTypePanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartTypePanel::Create", nullptr,
                                             1);
    return VclPtr<ChartTypePanel>::Create(pParent, rxFrame, pController);
}

void ChartTypePanel::DataChanged(const DataChangedEvent&) { updateData(); }

void ChartTypePanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
    updateData();
}

void ChartTypePanel::modelInvalid() { mbModelValid = false; }

void ChartTypePanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel,
                                                                        css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel,
                                                                       css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);
}

ChartTypeDialogController* ChartTypePanel::getSelectedMainType()
{
    ChartTypeDialogController* pTypeController = nullptr;
    auto nM = static_cast<std::vector<ChartTypeDialogController*>::size_type>(
        m_pMainTypeList->GetSelectedEntryPos());
    if (nM < m_aChartTypeDialogControllerList.size())
        pTypeController = m_aChartTypeDialogControllerList[nM].get();
    return pTypeController;
}

void ChartTypePanel::showAllControls(ChartTypeDialogController& rTypeController)
{
    m_pMainTypeList->Show();
    m_pSubTypeList->Show();

    bool bShow = rTypeController.shouldShow_3DLookControl();
    m_pDim3DLookResourceGroup->showControls(bShow);

    bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls(bShow, rTypeController.shouldShow_DeepStackingControl());

    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls(bShow);

    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls(bShow);

    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls(bShow);

    rTypeController.showExtraControls(this);
}

void ChartTypePanel::fillAllControls(const ChartTypeParameter& rParameter,
                                     bool bAlsoResetSubTypeList)
{
    m_nChangingCalls++;

    if (m_pCurrentMainType && bAlsoResetSubTypeList)
    {
        m_pCurrentMainType->fillSubTypeList(*m_pSubTypeList, rParameter);
    }

    m_pSubTypeList->SelectItem(static_cast<sal_uInt16>(rParameter.nSubTypeIndex));

    m_pDim3DLookResourceGroup->fillControls(rParameter);
    m_pStackingResourceGroup->fillControls(rParameter);
    m_pSplineResourceGroup->fillControls(rParameter);
    m_pGeometryResourceGroup->fillControls(rParameter);
    m_pSortByXValuesResourceGroup->fillControls(rParameter);

    m_nChangingCalls--;
}

ChartTypeParameter ChartTypePanel::getCurrentParamter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>(m_pSubTypeList->GetSelectedItemId());
    m_pDim3DLookResourceGroup->fillParameter(aParameter);
    m_pStackingResourceGroup->fillParameter(aParameter);
    m_pSplineResourceGroup->fillParameter(aParameter);
    m_pGeometryResourceGroup->fillParameter(aParameter);
    m_pSortByXValuesResourceGroup->fillParameter(aParameter);
    return aParameter;
}

void ChartTypePanel::stateChanged(ChangingResource* /*pResource*/)
{
    if (m_nChangingCalls)
        return;
    m_nChangingCalls++;

    ChartTypeParameter aParameter(getCurrentParamter());
    if (m_pCurrentMainType)
    {
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        m_pCurrentMainType->adjustSubTypeAndEnableControls(aParameter);
    }
    commitToModel(aParameter);

    //detect the new ThreeDLookScheme
    uno::Reference<css::chart2::XDiagram> xDiagram = ChartModelHelper::findDiagram(m_xChartModel);
    aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme(xDiagram);
    try
    {
        uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
        xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    //the controls have to be enabled/disabled accordingly
    fillAllControls(aParameter);

    m_nChangingCalls--;
}

void ChartTypePanel::commitToModel(const ChartTypeParameter& rParameter)
{
    if (!m_pCurrentMainType)
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    m_pCurrentMainType->commitToModel(rParameter, m_xChartModel);
}

void ChartTypePanel::selectMainType()
{
    ChartTypeParameter aParameter(getCurrentParamter());

    if (m_pCurrentMainType)
    {
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        m_pCurrentMainType->hideExtraControls();
    }

    m_pCurrentMainType = getSelectedMainType();
    if (m_pCurrentMainType)
    {
        showAllControls(*m_pCurrentMainType);

        m_pCurrentMainType->adjustParameterToMainType(aParameter);
        commitToModel(aParameter);
        //detect the new ThreeDLookScheme
        aParameter.eThreeDLookScheme
            = ThreeDHelper::detectScheme(ChartModelHelper::findDiagram(m_xChartModel));
        if (!aParameter.b3DLook && aParameter.eThreeDLookScheme != ThreeDLookScheme_Realistic)
            aParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;

        uno::Reference<css::chart2::XDiagram> xDiagram
            = ChartModelHelper::findDiagram(m_xChartModel);
        try
        {
            uno::Reference<beans::XPropertySet> xPropSet(xDiagram, uno::UNO_QUERY_THROW);
            xPropSet->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }

        fillAllControls(aParameter);
        uno::Reference<beans::XPropertySet> xTemplateProps(getCurrentTemplate(), uno::UNO_QUERY);
        m_pCurrentMainType->fillExtraControls(aParameter, m_xChartModel, xTemplateProps);
    }
}

uno::Reference<css::chart2::XChartTypeTemplate> ChartTypePanel::getCurrentTemplate() const
{
    if (m_pCurrentMainType && m_xChartModel.is())
    {
        ChartTypeParameter aParameter(getCurrentParamter());
        m_pCurrentMainType->adjustParameterToSubType(aParameter);
        uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
            m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
        return m_pCurrentMainType->getCurrentTemplate(aParameter, xTemplateManager);
    }
    return nullptr;
}
}
} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
