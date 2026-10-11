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

#include <dlg_CreationWizardMobile.hxx>
#include <ChartModel.hxx>

#include "tp_ChartType.hxx"
#include "tp_RangeChooser.hxx"
#include "tp_Wizard_TitlesAndObjects.hxx"
#include "DialogModel.hxx"

#include <KitDevice.hxx>

namespace chart
{

CreationWizardMobile::CreationWizardMobile(
    weld::Window* pParent, const rtl::Reference<::chart::ChartModel>& xChartModel,
    const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext)
    : GenericDialogController(pParent, u"modules/schart/ui/chartwizard-mobile.ui"_ustr,
                              u"ChartWizard"_ustr, true /* bMobile */)
    , m_xChartModel(xChartModel)
    , m_pDialogModel(new DialogModel(xChartModel))
    , m_xTabControl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
    , m_xButtonOK(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xTypePage = std::make_unique<ChartTypeTabPage>(m_xTabControl->get_page(u"type"_ustr), this,
                                                     m_xChartModel, false /* bShowDescription */);
    m_xTypePage->initializePage();
    m_pDialogModel->setTemplate(m_xTypePage->getCurrentTemplate());

    // A chart with its own data has no range to choose.
    if (m_pDialogModel->getModel().isDataFromSpreadsheet())
    {
        m_xRangePage = std::make_unique<RangeChooserTabPage>(
            m_xTabControl->get_page(u"range"_ustr), this, *m_pDialogModel, m_xTypePage.get(),
            true /* bHideDescription */);
    }
    else
        m_xTabControl->remove_page(u"range"_ustr);

    m_xElementsPage = std::make_unique<TitlesAndObjectsTabPage>(
        m_xTabControl->get_page(u"elements"_ustr), this, m_xChartModel, xContext);

    m_xTabControl->connect_enter_page(LINK(this, CreationWizardMobile, ActivatePageHdl));
    m_xTabControl->connect_leave_page(LINK(this, CreationWizardMobile, DeactivatePageHdl));
    m_xButtonOK->connect_clicked(LINK(this, CreationWizardMobile, OkHdl));
}

CreationWizardMobile::~CreationWizardMobile()
{
    m_xTypePage.reset();
    m_xRangePage.reset();
    m_xElementsPage.reset();
}

IMPL_LINK(CreationWizardMobile, ActivatePageHdl, const OUString&, rPage, void)
{
    if (rPage == "type")
        m_xTypePage->initializePage();
    else if (rPage == "range" && m_xRangePage)
        m_xRangePage->Activate();
    else if (rPage == "elements")
        m_xElementsPage->initializePage();
}

// The kit tab control switches the tab whatever is returned here, an invalid range only
// disables OK.
IMPL_LINK(CreationWizardMobile, DeactivatePageHdl, const OUString&, rPage, bool)
{
    if (rPage == "range" && m_xRangePage)
        return m_xRangePage->commitPage();
    if (rPage == "elements")
        m_xElementsPage->commitPage(vcl::WizardTypes::eTravelForward);
    return true;
}

IMPL_LINK_NOARG(CreationWizardMobile, OkHdl, weld::Button&, void)
{
    if (m_xRangePage && !m_xRangePage->commitPage())
        return;
    m_xElementsPage->commitPage(vcl::WizardTypes::eFinish);
    m_xDialog->response(RET_OK);
}

void CreationWizardMobile::setInvalidPage(BuilderPage* pTabPage)
{
    if (pTabPage != m_xRangePage.get())
        return;
    m_xButtonOK->set_sensitive(false);
}

void CreationWizardMobile::setValidPage(BuilderPage* pTabPage)
{
    if (pTabPage != m_xRangePage.get())
        return;
    m_xButtonOK->set_sensitive(true);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
