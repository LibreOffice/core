/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "TabPageNotifiable.hxx"

#include <rtl/ref.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace cpo::uno
{
class XComponentContext;
}

namespace chart
{
class ChartModel;
class ChartTypeTabPage;
class DialogModel;
class RangeChooserTabPage;
class TitlesAndObjectsTabPage;

/// The chart creation dialog for a phone, the wizard pages on the tabs of one full screen dialog.
class CreationWizardMobile final : public weld::GenericDialogController, public TabPageNotifiable
{
public:
    CreationWizardMobile(weld::Window* pParent,
                         const rtl::Reference<::chart::ChartModel>& xChartModel,
                         const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext);
    virtual ~CreationWizardMobile() override;

    // TabPageNotifiable
    virtual void setInvalidPage(BuilderPage* pTabPage) override;
    virtual void setValidPage(BuilderPage* pTabPage) override;

private:
    DECL_LINK(ActivatePageHdl, const OUString&, void);
    DECL_LINK(DeactivatePageHdl, const OUString&, bool);
    DECL_LINK(OkHdl, weld::Button&, void);

    rtl::Reference<::chart::ChartModel> m_xChartModel;
    std::unique_ptr<DialogModel> m_pDialogModel;

    std::unique_ptr<ChartTypeTabPage> m_xTypePage;
    std::unique_ptr<RangeChooserTabPage> m_xRangePage;
    std::unique_ptr<TitlesAndObjectsTabPage> m_xElementsPage;

    std::unique_ptr<weld::Notebook> m_xTabControl;
    std::unique_ptr<weld::Button> m_xButtonOK;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
