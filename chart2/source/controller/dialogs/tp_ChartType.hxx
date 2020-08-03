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

#pragma once

#include <vector>

#include <ChartTypeDialogController.hxx>
#include <ChartTypeTemplateProvider.hxx>
#include <TimerTriggeredControllerLock.hxx>

#include <vcl/wizardmachine.hxx>

namespace com::sun::star::chart2 { class XChartDocument; }
namespace weld { class CustomWeld; }

class ValueSet;

namespace chart
{

class Dim3DLookResourceGroup;
class StackingResourceGroup;
class SplineResourceGroup;
class GeometryResourceGroup;
class SortByXValuesResourceGroup;

class ChartTypeTabPage final : public ResourceChangeListener, public vcl::OWizardPage, public ChartTypeTemplateProvider
{
public:
    ChartTypeTabPage( weld::Container* pPage, weld::DialogController* pController
                , const css::uno::Reference< css::chart2::XChartDocument >& xChartModel
                , bool bShowDescription = true );
    virtual ~ChartTypeTabPage() override;

    virtual void        initializePage() override;
    virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason eReason ) override;

    virtual css::uno::Reference< css::chart2::XChartTypeTemplate > getCurrentTemplate() const override;

private:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls( ChartTypeDialogController& rTypeController );
    void fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList=true );
    ChartTypeParameter getCurrentParamter() const;

    virtual void stateChanged() override;

    void commitToModel( const ChartTypeParameter& rParameter );
    void selectMainType();

    DECL_LINK(SelectMainTypeHdl, weld::TreeView&, void);
    DECL_LINK(SelectSubTypeHdl, ValueSet*, void );

    std::unique_ptr<Dim3DLookResourceGroup>     m_pDim3DLookResourceGroup;
    std::unique_ptr<StackingResourceGroup>      m_pStackingResourceGroup;
    std::unique_ptr<SplineResourceGroup>        m_pSplineResourceGroup;
    std::unique_ptr<GeometryResourceGroup>      m_pGeometryResourceGroup;
    std::unique_ptr<SortByXValuesResourceGroup> m_pSortByXValuesResourceGroup;

    css::uno::Reference< css::chart2::XChartDocument >   m_xChartModel;

    std::vector< std::unique_ptr<ChartTypeDialogController> > m_aChartTypeDialogControllerList;
    ChartTypeDialogController*                  m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

    std::unique_ptr<weld::Label>  m_xFT_ChooseType;
    std::unique_ptr<weld::TreeView> m_xMainTypeList;
    std::unique_ptr<ValueSet> m_xSubTypeList;
    std::unique_ptr<weld::CustomWeld> m_xSubTypeListWin;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
