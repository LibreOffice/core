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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_CHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_CHARTTYPE_HXX

#include <vector>

#include "ChartTypeDialogController.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/wizardmachine.hxx>
#include <svtools/valueset.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace chart
{

class Dim3DLookResourceGroup;
class StackingResourceGroup;
class SplineResourceGroup;
class GeometryResourceGroup;
class ChartTypeParameter;
class SortByXValuesResourceGroup;
class GL3DResourceGroup;

class ChartTypeTabPage : public ResourceChangeListener, public svt::OWizardPage, public ChartTypeTemplateProvider
{
public:
    ChartTypeTabPage( vcl::Window* pParent
                , const css::uno::Reference< css::chart2::XChartDocument >& xChartModel
                , bool bDoLiveUpdate, bool bShowDescription = true );
    virtual ~ChartTypeTabPage();
    virtual void        dispose() override;

    virtual void        initializePage() override;
    virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason eReason ) override;

    virtual css::uno::Reference< css::chart2::XChartTypeTemplate > getCurrentTemplate() const override;

protected:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls( ChartTypeDialogController& rTypeController );
    void fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList=true );
    ChartTypeParameter getCurrentParamter() const;

    virtual void stateChanged( ChangingResource* pResource ) override;

    void commitToModel( const ChartTypeParameter& rParameter );
    void selectMainType();

    DECL_LINK_TYPED( SelectMainTypeHdl, ListBox&, void );
    DECL_LINK_TYPED( SelectSubTypeHdl, ValueSet*, void );

protected:
    VclPtr<FixedText>  m_pFT_ChooseType;
    VclPtr<ListBox>    m_pMainTypeList;
    VclPtr<ValueSet>   m_pSubTypeList;

    Dim3DLookResourceGroup*     m_pDim3DLookResourceGroup;
    StackingResourceGroup*      m_pStackingResourceGroup;
    SplineResourceGroup*        m_pSplineResourceGroup;
    GeometryResourceGroup*      m_pGeometryResourceGroup;
    SortByXValuesResourceGroup* m_pSortByXValuesResourceGroup;
    GL3DResourceGroup* m_pGL3DResourceGroup;

    css::uno::Reference< css::chart2::XChartDocument >   m_xChartModel;

    ::std::vector< ChartTypeDialogController* > m_aChartTypeDialogControllerList;
    ChartTypeDialogController*                  m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;
    bool      m_bDoLiveUpdate;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
