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

#include <TimerTriggeredControllerLock.hxx>

#include <vcl/wizardmachine.hxx>

#include <memory>

namespace chart { class LegendPositionResources; }
namespace chart { class TitleResources; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{

class TitlesAndObjectsTabPage final : public vcl::OWizardPage
{
public:
    TitlesAndObjectsTabPage(weld::Container* pPage, weld::DialogController* pController,
                            const css::uno::Reference< css::chart2::XChartDocument >& xChartModel,
                            const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~TitlesAndObjectsTabPage() override;

    virtual void        initializePage() override;
    virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason eReason ) override;
    virtual bool        canAdvance() const override;

private:
    void commitToModel();
    DECL_LINK( ChangeHdl, LinkParamNone*, void );
    DECL_LINK( ChangeEditHdl, weld::Entry&, void );
    DECL_LINK( ChangeCheckBoxHdl, weld::ToggleButton&, void );

    std::unique_ptr< TitleResources >            m_xTitleResources;
    std::unique_ptr< LegendPositionResources >   m_xLegendPositionResources;

    css::uno::Reference< css::chart2::XChartDocument >   m_xChartModel;
    css::uno::Reference< css::uno::XComponentContext>    m_xCC;

    bool    m_bCommitToModel;
    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

    std::unique_ptr<weld::CheckButton> m_xCB_Grid_X;
    std::unique_ptr<weld::CheckButton> m_xCB_Grid_Y;
    std::unique_ptr<weld::CheckButton> m_xCB_Grid_Z;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
