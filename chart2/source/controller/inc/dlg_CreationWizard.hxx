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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CREATIONWIZARD_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CREATIONWIZARD_HXX

#include "TimerTriggeredControllerLock.hxx"
#include "TabPageNotifiable.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <svtools/roadmapwizard.hxx>

#include <memory>

namespace chart
{

class DialogModel;
class ChartTypeTemplateProvider;

class CreationWizard : public svt::RoadmapWizard, public TabPageNotifiable
{
public:
    CreationWizard(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XModel>& xChartModel,
        const css::uno::Reference<css::uno::XComponentContext>& xContext);

    CreationWizard() = delete;
    virtual ~CreationWizard() override;

    // TabPageNotifiable
    virtual void setInvalidPage(TabPage * pTabPage) override;
    virtual void setValidPage(TabPage * pTabPage) override;

protected:
    virtual bool leaveState( WizardState _nState ) override;
    virtual WizardState determineNextState(WizardState nCurrentState) const override;
    virtual void enterState(WizardState nState) override;

    virtual OUString getStateDisplayName(WizardState nState) const override;

private:
    virtual VclPtr<TabPage> createPage(WizardState nState) override;

    css::uno::Reference<css::chart2::XChartDocument> m_xChartModel;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    ChartTypeTemplateProvider* m_pTemplateProvider;
    std::unique_ptr<DialogModel> m_pDialogModel;

    WizardState m_nLastState;

    TimerTriggeredControllerLock m_aTimerTriggeredControllerLock;

    bool m_bCanTravel;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
