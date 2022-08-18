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

#include "tp_ChartType.hxx"
#include <ChartResourceGroups.hxx>
#include <ChartTypeManager.hxx>
#include <strings.hrc>
#include <ResId.hxx>
#include <ChartModelHelper.hxx>
#include <ChartModel.hxx>
#include <ChartTypeTemplate.hxx>
#include <DiagramHelper.hxx>
#include <Diagram.hxx>
#include <unonames.hxx>

#include <svtools/valueset.hxx>

#include <utility>
#include <vcl/weld.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartTypeTabPage::ChartTypeTabPage(weld::Container* pPage, weld::DialogController* pController, rtl::Reference<::chart::ChartModel> xChartModel,
                                   bool bShowDescription)
    : OWizardPage(pPage, pController, "modules/schart/ui/tp_ChartType.ui", "tp_ChartType")
    , m_pDim3DLookResourceGroup( new Dim3DLookResourceGroup(m_xBuilder.get()) )
    , m_pStackingResourceGroup( new StackingResourceGroup(m_xBuilder.get()) )
    , m_pSplineResourceGroup( new SplineResourceGroup(m_xBuilder.get(), pController->getDialog()) )
    , m_pGeometryResourceGroup( new GeometryResourceGroup(m_xBuilder.get()) )
    , m_pSortByXValuesResourceGroup( new SortByXValuesResourceGroup(m_xBuilder.get()) )
    , m_xChartModel(std::move( xChartModel ))
    , m_aChartTypeDialogControllerList(0)
    , m_pCurrentMainType(nullptr)
    , m_nChangingCalls(0)
    , m_aTimerTriggeredControllerLock( m_xChartModel )
    , m_xFT_ChooseType(m_xBuilder->weld_label("FT_CAPTION_FOR_WIZARD"))
    , m_xMainTypeList(m_xBuilder->weld_tree_view("charttype"))
    , m_xSubTypeList(new ValueSet(m_xBuilder->weld_scrolled_window("subtypewin", true)))
    , m_xSubTypeListWin(new weld::CustomWeld(*m_xBuilder, "subtype", *m_xSubTypeList))
{
    Size aSize(m_xSubTypeList->GetDrawingArea()->get_ref_device().LogicToPixel(Size(150, 50), MapMode(MapUnit::MapAppFont)));
    m_xSubTypeListWin->set_size_request(aSize.Width(), aSize.Height());

    if (bShowDescription)
    {
        m_xFT_ChooseType->show();
    }
    else
    {
        m_xFT_ChooseType->hide();
    }

    SetPageTitle(SchResId(STR_PAGE_CHARTTYPE));

    m_xMainTypeList->connect_changed(LINK(this, ChartTypeTabPage, SelectMainTypeHdl));
    m_xSubTypeList->SetSelectHdl( LINK( this, ChartTypeTabPage, SelectSubTypeHdl ) );

    m_xSubTypeList->SetStyle(m_xSubTypeList->GetStyle() |
        WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD | WB_FLATVALUESET | WB_3DLOOK );
    m_xSubTypeList->SetColCount(4);
    m_xSubTypeList->SetLineCount(1);

    bool bEnableComplexChartTypes = true;
    uno::Reference< beans::XPropertySet > xProps( static_cast<cppu::OWeakObject*>(m_xChartModel.get()), uno::UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            xProps->getPropertyValue("EnableComplexChartTypes") >>= bEnableComplexChartTypes;
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
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
        m_aChartTypeDialogControllerList.push_back(
            std::make_unique<BubbleChartDialogController>());
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
        m_xMainTypeList->append("", elem->getName(), elem->getImage());
        elem->setChangeListener( this );
    }

    m_xMainTypeList->set_size_request(m_xMainTypeList->get_preferred_size().Width(), -1);

    m_pDim3DLookResourceGroup->setChangeListener( this );
    m_pStackingResourceGroup->setChangeListener( this );
    m_pSplineResourceGroup->setChangeListener( this );
    m_pGeometryResourceGroup->setChangeListener( this );
    m_pSortByXValuesResourceGroup->setChangeListener( this );
}

ChartTypeTabPage::~ChartTypeTabPage()
{
    //delete all dialog controller
    m_aChartTypeDialogControllerList.clear();

    //delete all resource helper
    m_pDim3DLookResourceGroup.reset();
    m_pStackingResourceGroup.reset();
    m_pSplineResourceGroup.reset();
    m_pGeometryResourceGroup.reset();
    m_pSortByXValuesResourceGroup.reset();
    m_xSubTypeListWin.reset();
    m_xSubTypeList.reset();
}

ChartTypeParameter ChartTypeTabPage::getCurrentParamter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>(m_xSubTypeList->GetSelectedItemId());
    m_pDim3DLookResourceGroup->fillParameter( aParameter );
    m_pStackingResourceGroup->fillParameter( aParameter );
    m_pSplineResourceGroup->fillParameter( aParameter );
    m_pGeometryResourceGroup->fillParameter( aParameter );
    m_pSortByXValuesResourceGroup->fillParameter( aParameter );
    return aParameter;
}

void ChartTypeTabPage::commitToModel( const ChartTypeParameter& rParameter )
{
    if( !m_pCurrentMainType )
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    m_pCurrentMainType->commitToModel( rParameter, m_xChartModel );
}

void ChartTypeTabPage::stateChanged()
{
    if(m_nChangingCalls)
        return;
    m_nChangingCalls++;

    ChartTypeParameter aParameter( getCurrentParamter() );
    if( m_pCurrentMainType )
    {
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        m_pCurrentMainType->adjustSubTypeAndEnableControls( aParameter );
    }
    commitToModel( aParameter );

    //detect the new ThreeDLookScheme
    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram(m_xChartModel);
    // tdf#124295 - select always a 3D scheme
    if (ThreeDLookScheme aThreeDLookScheme = ThreeDHelper::detectScheme(xDiagram);
        aThreeDLookScheme != ThreeDLookScheme::ThreeDLookScheme_Unknown)
        aParameter.eThreeDLookScheme = aThreeDLookScheme;

    try
    {
        xDiagram->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    //the controls have to be enabled/disabled accordingly
    fillAllControls( aParameter );

    m_nChangingCalls--;
}

ChartTypeDialogController* ChartTypeTabPage::getSelectedMainType()
{
    ChartTypeDialogController* pTypeController = nullptr;
    auto nM = static_cast< std::vector< ChartTypeDialogController* >::size_type >(
        m_xMainTypeList->get_selected_index() );
    if( nM<m_aChartTypeDialogControllerList.size() )
        pTypeController = m_aChartTypeDialogControllerList[nM].get();
    return pTypeController;
}

IMPL_LINK_NOARG(ChartTypeTabPage, SelectSubTypeHdl, ValueSet*, void)
{
    if( m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        fillAllControls( aParameter, false );
        commitToModel( aParameter );
    }
}

IMPL_LINK_NOARG(ChartTypeTabPage, SelectMainTypeHdl, weld::TreeView&, void)
{
    selectMainType();
}

void ChartTypeTabPage::selectMainType()
{
    ChartTypeParameter aParameter( getCurrentParamter() );

    if( m_pCurrentMainType )
    {
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        m_pCurrentMainType->hideExtraControls();
    }

    m_pCurrentMainType = getSelectedMainType();
    if( !m_pCurrentMainType )
        return;

    showAllControls(*m_pCurrentMainType);

    m_pCurrentMainType->adjustParameterToMainType( aParameter );
    commitToModel( aParameter );
    //detect the new ThreeDLookScheme
    aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme( ChartModelHelper::findDiagram( m_xChartModel ) );
    if (!aParameter.b3DLook
        && aParameter.eThreeDLookScheme != ThreeDLookScheme::ThreeDLookScheme_Realistic)
        aParameter.eThreeDLookScheme = ThreeDLookScheme::ThreeDLookScheme_Realistic;

    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram(m_xChartModel);
    try
    {
        xDiagram->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    fillAllControls( aParameter );
    uno::Reference< beans::XPropertySet > xTemplateProps( static_cast<cppu::OWeakObject*>(getCurrentTemplate().get()), uno::UNO_QUERY );
    m_pCurrentMainType->fillExtraControls(m_xChartModel,xTemplateProps);
}

void ChartTypeTabPage::showAllControls( ChartTypeDialogController& rTypeController )
{
    m_xMainTypeList->show();
    m_xSubTypeList->Show();

    bool bShow = rTypeController.shouldShow_3DLookControl();
    m_pDim3DLookResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls( bShow );
    rTypeController.showExtraControls(m_xBuilder.get());
}

void ChartTypeTabPage::fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList )
{
    m_nChangingCalls++;
    if( m_pCurrentMainType && bAlsoResetSubTypeList )
    {
        m_pCurrentMainType->fillSubTypeList(*m_xSubTypeList, rParameter);
    }
    m_xSubTypeList->SelectItem( static_cast<sal_uInt16>( rParameter.nSubTypeIndex) );
    m_pDim3DLookResourceGroup->fillControls( rParameter );
    m_pStackingResourceGroup->fillControls( rParameter );
    m_pSplineResourceGroup->fillControls( rParameter );
    m_pGeometryResourceGroup->fillControls( rParameter );
    m_pSortByXValuesResourceGroup->fillControls( rParameter );
    m_nChangingCalls--;
}

void ChartTypeTabPage::initializePage()
{
    if( !m_xChartModel.is() )
        return;
    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = m_xChartModel->getTypeManager();
    rtl::Reference< Diagram > xDiagram = ChartModelHelper::findDiagram( m_xChartModel );
    DiagramHelper::tTemplateWithServiceName aTemplate =
        DiagramHelper::getTemplateForDiagram( xDiagram, xChartTypeManager );
    OUString aServiceName( aTemplate.sServiceName );

    bool bFound = false;

    sal_uInt16 nM=0;
    for (auto const& elem : m_aChartTypeDialogControllerList)
    {
        if( elem->isSubType(aServiceName) )
        {
            bFound = true;

            m_xMainTypeList->select(nM);
            showAllControls(*elem);
            uno::Reference< beans::XPropertySet > xTemplateProps( static_cast<cppu::OWeakObject*>(aTemplate.xChartTypeTemplate.get()), uno::UNO_QUERY );
            ChartTypeParameter aParameter = elem->getChartTypeParameterForService( aServiceName, xTemplateProps );
            m_pCurrentMainType = getSelectedMainType();

            //set ThreeDLookScheme
            aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme( xDiagram );
            if (!aParameter.b3DLook
                && aParameter.eThreeDLookScheme != ThreeDLookScheme::ThreeDLookScheme_Realistic)
                aParameter.eThreeDLookScheme = ThreeDLookScheme::ThreeDLookScheme_Realistic;

            try
            {
                xDiagram->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= aParameter.bSortByXValues;
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }

            fillAllControls( aParameter );
            if( m_pCurrentMainType )
                m_pCurrentMainType->fillExtraControls(m_xChartModel,xTemplateProps);
            break;
        }
        ++nM;
    }

    if( !bFound )
    {
        m_xMainTypeList->show();
        m_xSubTypeList->Show();
        m_pDim3DLookResourceGroup->showControls( false );
        m_pStackingResourceGroup->showControls( false );
        m_pSplineResourceGroup->showControls( false );
        m_pGeometryResourceGroup->showControls( false );
        m_pSortByXValuesResourceGroup->showControls( false );
    }
}

bool ChartTypeTabPage::commitPage( ::vcl::WizardTypes::CommitPageReason /*eReason*/ )
{
    return true; // return false if this page should not be left
}

rtl::Reference< ChartTypeTemplate > ChartTypeTabPage::getCurrentTemplate() const
{
    if( m_pCurrentMainType && m_xChartModel.is() )
    {
        ChartTypeParameter aParameter( getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = m_xChartModel->getTypeManager();
        return m_pCurrentMainType->getCurrentTemplate( aParameter, xChartTypeManager );
    }
    return nullptr;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
