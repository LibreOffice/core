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

#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/EnumContext.hxx>
#include "ChartSidebarModifyListener.hxx"
#include <ChartTypeDialogController.hxx>
#include <ChartTypeTemplateProvider.hxx>
#include <TimerTriggeredControllerLock.hxx>

namespace com::sun::star::util
{
class XModifyListener;
}

namespace weld
{
class CustomWeld;
}

namespace chart
{
class ChartController;
class Dim3DLookResourceGroup;
class StackingResourceGroup;
class SplineResourceGroup;
class GeometryResourceGroup;
class ChartTypeParameter;
class SortByXValuesResourceGroup;

namespace sidebar
{
class ChartTypePanel : public ResourceChangeListener,
                       public PanelLayout,
                       public ::sfx2::sidebar::IContextChangeReceiver,
                       public sfx2::sidebar::SidebarModelUpdate,
                       public ChartSidebarModifyListenerParent,
                       public ChartTypeTemplateProvider
{
public:
    virtual void DataChanged(const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(const vcl::EnumContext& rContext) override;

    // constructor/destructor
    ChartTypePanel(weld::Widget* pParent, ::chart::ChartController* pController);

    virtual ~ChartTypePanel() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

    virtual css::uno::Reference<css::chart2::XChartTypeTemplate>
    getCurrentTemplate() const override;

private:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls(ChartTypeDialogController& rTypeController);
    void fillAllControls(const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList = true);
    ChartTypeParameter getCurrentParameter() const;

    virtual void stateChanged() override;

    void commitToModel(const ChartTypeParameter& rParameter);
    void selectMainType();

    DECL_LINK(SelectMainTypeHdl, weld::ComboBox&, void);
    DECL_LINK(SelectSubTypeHdl, ValueSet*, void);

    vcl::EnumContext maContext;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    bool mbModelValid;

    void Initialize();

    std::unique_ptr<Dim3DLookResourceGroup> m_pDim3DLookResourceGroup;
    std::unique_ptr<StackingResourceGroup> m_pStackingResourceGroup;
    std::unique_ptr<SplineResourceGroup> m_pSplineResourceGroup;
    std::unique_ptr<GeometryResourceGroup> m_pGeometryResourceGroup;
    std::unique_ptr<SortByXValuesResourceGroup> m_pSortByXValuesResourceGroup;

    css::uno::Reference<css::chart2::XChartDocument> m_xChartModel;

    std::vector<std::unique_ptr<ChartTypeDialogController>> m_aChartTypeDialogControllerList;
    ChartTypeDialogController* m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;

    TimerTriggeredControllerLock m_aTimerTriggeredControllerLock;

    std::unique_ptr<weld::ComboBox> m_xMainTypeList;
    std::unique_ptr<ValueSet> m_xSubTypeList;
    std::unique_ptr<weld::CustomWeld> m_xSubTypeListWin;
};
}
} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
