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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTTYPEPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTTYPEPANEL_HXX

#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <svx/sidebar/PanelLayout.hxx>
//#include <vcl/layout.hxx>
#include "ChartSidebarModifyListener.hxx"
#include <ChartTypeDialogController.hxx>
#include <TimerTriggeredControllerLock.hxx>
#include <TitleHelper.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace util
{
class XModifyListener;
}
}
}
}

class ListBox;
class ValueSet;

namespace chart
{
class ChartController;
class Dim3DLookResourceGroup_unwelded;
class StackingResourceGroup_unwelded;
class SplineResourceGroup_unwelded;
class GeometryResourceGroup_unwelded;
class ChartTypeParameter_unwelded;
class SortByXValuesResourceGroup_unwelded;

namespace sidebar
{
class ChartTypePanel : public ResourceChangeListener,
                       public PanelLayout,
                       public ::sfx2::sidebar::IContextChangeReceiver,
                       public sfx2::sidebar::SidebarModelUpdate,
                       public ChartSidebarModifyListenerParent
{
public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                      ChartController* pController);

    virtual void DataChanged(const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(const vcl::EnumContext& rContext) override;

    // constructor/destructor
    ChartTypePanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
                   ::chart::ChartController* pController);

    virtual ~ChartTypePanel() override;

    virtual void dispose() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls(ChartTypeDialogController& rTypeController);
    void fillAllControls(const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList = true);
    ChartTypeParameter getCurrentParamter() const;

    virtual void stateChanged() override;

    void commitToModel(const ChartTypeParameter& rParameter);

    /*DECL_LINK(SelectMainTypeHdl, ListBox&, void);
    DECL_LINK(SelectSubTypeHdl, ValueSet*, void);*/

    //ui controls
    VclPtr<FixedText> mpChartTypeLabel;
    VclPtr<ListBox> m_pMainTypeList;
    VclPtr<ValueSet> m_pSubTypeList;

    vcl::EnumContext maContext;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    bool mbModelValid;

    OUString maTextTitle;
    OUString maTextSubTitle;

    void Initialize();

    std::unique_ptr<Dim3DLookResourceGroup_unwelded> m_pDim3DLookResourceGroup;
    /*std::unique_ptr<StackingResourceGroup>      m_pStackingResourceGroup;
    std::unique_ptr<SplineResourceGroup>        m_pSplineResourceGroup;
    std::unique_ptr<GeometryResourceGroup>      m_pGeometryResourceGroup;
    std::unique_ptr<SortByXValuesResourceGroup> m_pSortByXValuesResourceGroup;*/

    css::uno::Reference<css::chart2::XChartDocument> m_xChartModel;

    std::vector<std::unique_ptr<ChartTypeDialogController>> m_aChartTypeDialogControllerList;
    ChartTypeDialogController* m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;

    TimerTriggeredControllerLock m_aTimerTriggeredControllerLock;
};
}
} // end of namespace ::chart::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
