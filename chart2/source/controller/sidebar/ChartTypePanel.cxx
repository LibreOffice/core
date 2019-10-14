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
#include <ChartTypeDialogController.hxx>
#include <ChartTypeHelper.hxx>
#include <DiagramHelper.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <unonames.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>

#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/button.hxx>

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
        m_pChangeListener->stateChanged();
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup_unwelded, SelectSchemeHdl, ListBox&, void)
{
    if (m_pChangeListener)
        m_pChangeListener->stateChanged();
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
    , m_xChartModel(mxModel, css::uno::UNO_QUERY_THROW)
    , m_aChartTypeDialogControllerList(0)
    , m_pCurrentMainType(nullptr)
    , m_nChangingCalls(0)
    , m_aTimerTriggeredControllerLock(m_xChartModel)
{
    get(mpChartTypeLabel, "lbl_chartType");
    get(m_pMainTypeList, "cmb_chartType");
    get(m_pSubTypeList, "subtype");

    //Chart Type related
    /*m_pMainTypeList->SetSelectHdl(LINK(this, ChartTypePanel, SelectMainTypeHdl));
    m_pSubTypeList->SetSelectHdl(LINK(this, ChartTypePanel, SelectSubTypeHdl));*/

    Size aSize(m_pSubTypeList->LogicToPixel(Size(120, 40), MapMode(MapUnit::MapAppFont)));
    m_pSubTypeList->set_width_request(aSize.Width());
    m_pSubTypeList->set_height_request(aSize.Height());

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
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "");
        }
    }

    m_aChartTypeDialogControllerList.push_back(std::make_unique<ColumnChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<BarChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<PieChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<AreaChartDialogController>());
    m_aChartTypeDialogControllerList.push_back(std::make_unique<LineChartDialogController>());

    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(std::make_unique<XYChartDialogController>());
        m_aChartTypeDialogControllerList.push_back(std::make_unique<BubbleChartDialogController>());
    }
    m_aChartTypeDialogControllerList.push_back(std::make_unique<NetChartDialogController>());
    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(std::make_unique<StockChartDialogController>());
    }
    m_aChartTypeDialogControllerList.push_back(
        std::make_unique<CombiColumnLineChartDialogController>());

    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        m_pMainTypeList->InsertEntry(elem->getName(), createImage(elem->getImage()));
        elem->setChangeListener(this);
    }

    m_pDim3DLookResourceGroup->setChangeListener(this);

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

    PanelLayout::dispose();
}

/*IMPL_LINK_NOARG(ChartTypePanel, SelectMainTypeHdl, ListBox&, void)
{
    //selectMainType();
}

IMPL_LINK_NOARG(ChartTypePanel, SelectSubTypeHdl, ValueSet*, void)
{
    if( m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        fillAllControls( aParameter, false );
        commitToModel( aParameter );
    }
}*/

void ChartTypePanel::Initialize()
{
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xChartModel);
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
                m_pCurrentMainType->fillExtraControls(m_xChartModel, xTemplateProps);
            break;
        }
        ++nM;
    }

    if (!bFound)
    {
        m_pSubTypeList->Hide();
        m_pDim3DLookResourceGroup->showControls(false);
        /*m_pStackingResourceGroup->showControls( false, false );
            m_pSplineResourceGroup->showControls( false );
            m_pGeometryResourceGroup->showControls( false );
            m_pSortByXValuesResourceGroup->showControls( false );*/
    }
}

void ChartTypePanel::updateData()
{
    // Chart Type related
    if (!m_xChartModel.is())
        return;
    uno::Reference<lang::XMultiServiceFactory> xTemplateManager(
        m_xChartModel->getChartTypeManager(), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xChartModel);
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
    /*bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls( bShow, rTypeController.shouldShow_DeepStackingControl() );
    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls( bShow );
    rTypeController.showExtraControls(this);*/
}

void ChartTypePanel::fillAllControls(const ChartTypeParameter& rParameter,
                                     bool bAlsoResetSubTypeList)
{
    m_nChangingCalls++;
    if (m_pCurrentMainType && bAlsoResetSubTypeList)
    {
        // FIXME: This is just to test. This if-block should just call m_pCurrentMainType->fillSubTypeList(*m_pSubTypeList, rParameter);
        // after adding a new method to ColumnChartDialogController and its children
        //m_pCurrentMainType->fillSubTypeList(*m_pSubTypeList, rParameter);
        m_pSubTypeList->Clear();

        if (rParameter.b3DLook)
        {
            switch (rParameter.nGeometry3D)
            {
                case css::chart2::DataPointGeometry3D::CYLINDER:
                    m_pSubTypeList->InsertItem(1, Image(StockImage::Yes, BMP_SAEULE_3D_1));
                    m_pSubTypeList->InsertItem(2, Image(StockImage::Yes, BMP_SAEULE_3D_2));
                    m_pSubTypeList->InsertItem(3, Image(StockImage::Yes, BMP_SAEULE_3D_3));
                    m_pSubTypeList->InsertItem(4, Image(StockImage::Yes, BMP_SAEULE_3D_4));
                    break;
                case css::chart2::DataPointGeometry3D::CONE:
                    m_pSubTypeList->InsertItem(1, Image(StockImage::Yes, BMP_KEGEL_3D_1));
                    m_pSubTypeList->InsertItem(2, Image(StockImage::Yes, BMP_KEGEL_3D_2));
                    m_pSubTypeList->InsertItem(3, Image(StockImage::Yes, BMP_KEGEL_3D_3));
                    m_pSubTypeList->InsertItem(4, Image(StockImage::Yes, BMP_KEGEL_3D_4));
                    break;
                case css::chart2::DataPointGeometry3D::PYRAMID:
                    m_pSubTypeList->InsertItem(1, Image(StockImage::Yes, BMP_PYRAMID_3D_1));
                    m_pSubTypeList->InsertItem(2, Image(StockImage::Yes, BMP_PYRAMID_3D_2));
                    m_pSubTypeList->InsertItem(3, Image(StockImage::Yes, BMP_PYRAMID_3D_3));
                    m_pSubTypeList->InsertItem(4, Image(StockImage::Yes, BMP_PYRAMID_3D_4));
                    break;
                default: //DataPointGeometry3D::CUBOID:
                    m_pSubTypeList->InsertItem(1, Image(StockImage::Yes, BMP_COLUMNS_3D_1));
                    m_pSubTypeList->InsertItem(2, Image(StockImage::Yes, BMP_COLUMNS_3D_2));
                    m_pSubTypeList->InsertItem(3, Image(StockImage::Yes, BMP_COLUMNS_3D_3));
                    m_pSubTypeList->InsertItem(4, Image(StockImage::Yes, BMP_COLUMNS_3D));
                    break;
            }
        }
        else
        {
            m_pSubTypeList->InsertItem(1, Image(StockImage::Yes, BMP_COLUMNS_2D_1));
            m_pSubTypeList->InsertItem(2, Image(StockImage::Yes, BMP_COLUMNS_2D_2));
            m_pSubTypeList->InsertItem(3, Image(StockImage::Yes, BMP_COLUMNS_2D_3));
        }

        m_pSubTypeList->SetItemText(1, SchResId(STR_NORMAL));
        m_pSubTypeList->SetItemText(2, SchResId(STR_STACKED));
        m_pSubTypeList->SetItemText(3, SchResId(STR_PERCENT));
        m_pSubTypeList->SetItemText(4, SchResId(STR_DEEP));
    }
    m_pSubTypeList->SelectItem(static_cast<sal_uInt16>(rParameter.nSubTypeIndex));
    m_pDim3DLookResourceGroup->fillControls(rParameter);
    /*m_pStackingResourceGroup->fillControls( rParameter );
    m_pSplineResourceGroup->fillControls( rParameter );
    m_pGeometryResourceGroup->fillControls( rParameter );
    m_pSortByXValuesResourceGroup->fillControls( rParameter );*/
    m_nChangingCalls--;
}

ChartTypeParameter ChartTypePanel::getCurrentParamter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>(m_pSubTypeList->GetSelectedItemId());
    m_pDim3DLookResourceGroup->fillParameter(aParameter);
    //m_pStackingResourceGroup->fillParameter( aParameter );
    //m_pSplineResourceGroup->fillParameter( aParameter );
    //m_pGeometryResourceGroup->fillParameter( aParameter );
    //m_pSortByXValuesResourceGroup->fillParameter( aParameter );
    return aParameter;
}

void ChartTypePanel::stateChanged()
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
}
} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
