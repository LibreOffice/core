/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/config.h>

#include <toolbartabpage.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/settings.hxx>

std::unique_ptr<SfxTabPage> ToolbarTabPage::Create(weld::Container* pPage,
                                                   weld::DialogController* pController,
                                                   const SfxItemSet* rAttr)
{
    return std::make_unique<ToolbarTabPage>(pPage, pController, *rAttr);
}

ToolbarTabPage::ToolbarTabPage(weld::Container* pPage, weld::DialogController* pController,
                               const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, u"cui/ui/toolbartabpage.ui"_ustr, u"ToolbarTabPage"_ustr,
                 &rAttr)
    , m_pToolbarList(m_xBuilder->weld_tree_view(u"tvToolbarList"_ustr))
    , m_pLockAllLabel(m_xBuilder->weld_label(u"lbLockAll"_ustr))
    , m_pLockAll(m_xBuilder->weld_check_button(u"cbLockAll"_ustr))
{
    m_pToolbarList->connect_toggled(LINK(this, ToolbarTabPage, ToggleHdl));
    m_pLockAll->connect_toggled(LINK(this, ToolbarTabPage, CheckBoxHdl));
}

void ToolbarTabPage::UpdateAllLocked()
{
    std::unique_ptr<weld::TreeIter> pIter = m_pToolbarList->make_iterator();
    bool bLoop = m_pToolbarList->get_iter_first(*pIter);
    TriState bAllLocked = m_pToolbarList->get_toggle(*pIter, 2);
    bLoop = m_pToolbarList->iter_next(*pIter);
    while (bLoop)
    {
        if (bAllLocked == m_pToolbarList->get_toggle(*pIter, 2))
            bLoop = m_pToolbarList->iter_next(*pIter);
        else
        {
            bAllLocked = TRISTATE_INDET;
            bLoop = false;
        }
    }
    m_pLockAll->set_state(bAllLocked);
}

void ToolbarTabPage::Reset(const SfxItemSet* /* rSet*/)
{
    css::uno::Reference<css::uno::XComponentContext> xContext(
        comphelper::getProcessComponentContext());
    if (!xContext)
        return;
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;
    css::uno::Reference<css::frame::XFrame> xFrame = pViewFrm->GetFrame().GetFrameInterface();
    if (!xFrame)
        return;
    css::uno::Reference<css::ui::XModuleUIConfigurationManagerSupplier> xSupplier
        = css::ui::theModuleUIConfigurationManagerSupplier::get(xContext);
    if (!xSupplier)
        return;
    const OUString sModuleId = vcl::CommandInfoProvider::GetModuleIdentifier(xFrame);
    css::uno::Reference<css::ui::XUIConfigurationManager> xUIConfigMgr(
        xSupplier->getUIConfigurationManager(sModuleId));
    if (!xUIConfigMgr)
        return;
    css::uno::Reference<css::container::XNameAccess> xUICmdDescription
        = css::ui::theWindowStateConfiguration::get(xContext);
    if (!xUICmdDescription)
        return;
    css::uno::Reference<css::container::XNameAccess> xModuleConf;
    xUICmdDescription->getByName(sModuleId) >>= xModuleConf;
    if (!xModuleConf)
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet(xFrame, css::uno::UNO_QUERY);

    css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;
    xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    m_xLayoutManager = dynamic_cast<framework::LayoutManager*>(xLayoutManager.get());

    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aSeqDocToolBars;
    aSeqDocToolBars = xUIConfigMgr->getUIElementsInfo(css::ui::UIElementType::TOOLBAR);

    tools::Long nLongestLabel = 0;
    const bool bIsExperimentalMode = officecfg::Office::Common::Misc::ExperimentalMode::get();

    m_pToolbarList->freeze();
    m_pToolbarList->clear();
    for (css::uno::Sequence<css::beans::PropertyValue> const& props : aSeqDocToolBars)
    {
        for (css::beans::PropertyValue const& prop : props)
        {
            if (prop.Name == u"ResourceURL"_ustr)
            {
                css::uno::Sequence<css::beans::PropertyValue> aCmdProps;
                OUString sResourceURL;
                OUString sUIName;
                bool bHideFromMenu = true;
                bool bLocked = false;
                bool bSensitive = false;
                // bool bVisible = false;

                prop.Value >>= sResourceURL;

                if (xModuleConf->hasByName(sResourceURL))
                {
                    xModuleConf->getByName(sResourceURL) >>= aCmdProps;
                    for (const auto& aCmdProp : aCmdProps)
                    {
                        if (aCmdProp.Name == u"UIName"_ustr)
                            aCmdProp.Value >>= sUIName;
                        else if (aCmdProp.Name == u"HideFromToolbarMenu"_ustr)
                            aCmdProp.Value >>= bHideFromMenu;
                        // else if (aCmdProp.Name == u"Visible"_ustr)
                        //     aCmdProp.Value >>= bVisible;
                        else if (aCmdProp.Name == u"ContextSensitive"_ustr)
                            aCmdProp.Value >>= bSensitive;
                        else if (aCmdProp.Name == u"Locked"_ustr)
                            aCmdProp.Value >>= bLocked;
                    }

                    if (!bHideFromMenu)
                    {
                        // prop:"Visible" is also true if the toolbar is contextually hidden
                        const bool bVisible = m_xLayoutManager->isElementVisible(sResourceURL);
                        const int nRow = m_pToolbarList->n_children();
                        TriState aState;
                        m_pToolbarList->append();
                        m_pToolbarList->set_id(nRow, sResourceURL);
                        m_pToolbarList->set_text(nRow, sUIName, 0);
                        aState = bVisible ? TRISTATE_TRUE : TRISTATE_FALSE;
                        m_pToolbarList->set_toggle(nRow, aState, 1);
                        aState = bLocked ? TRISTATE_TRUE : TRISTATE_FALSE;
                        m_pToolbarList->set_toggle(nRow, aState, 2);
                        if (bIsExperimentalMode)
                        {
                            aState = bSensitive ? TRISTATE_TRUE : TRISTATE_FALSE;
                            m_pToolbarList->set_toggle(nRow, aState, 3);
                        }

                        tools::Long nWidth = m_pToolbarList->get_pixel_size(sUIName).Width();
                        nLongestLabel = std::max(nLongestLabel, nWidth);
                    }
                }
            }
        }
    }
    m_pToolbarList->thaw();
    m_pToolbarList->make_sorted();

    // column widths
    std::vector<int> aWidths;
    aWidths.push_back(nLongestLabel + 50); // first col has some indention on kf6
    for (sal_uInt16 i = 1; i < 3; i++)
    {
        OUString sTitle = m_pToolbarList->get_column_title(i);
        aWidths.push_back(m_pToolbarList->get_pixel_size(sTitle).Width() + 12); // some padding
    }
    if (!bIsExperimentalMode)
        aWidths.push_back(0); // zero width for context_sensitive column, otherwise available space
    m_pToolbarList->set_column_fixed_widths(aWidths);

    // treeview height
    //    auto nWidth = std::accumulate(aWidths.begin(), aWidths.end(),
    //                                  Application::GetSettings().GetStyleSettings().GetScrollBarSize());
    m_pToolbarList->set_size_request(-1, m_pToolbarList->get_height_rows(20));

    // place Lock All checkbox underneath the Locked column
    OUString aLabel = m_pLockAllLabel->get_label();
    const int nLockLabelSize = m_pLockAllLabel->get_pixel_size(aLabel).Width();
    m_pLockAllLabel->set_margin_start(aWidths[0] + aWidths[1] - nLockLabelSize);

    UpdateAllLocked();
}

void ToolbarTabPage::ShowToolbar(const OUString& sName, const bool bShow)
{
    if (bShow)
    {
        m_xLayoutManager->createElement(sName);
        m_xLayoutManager->showElement(sName);
    }
    else
    {
        m_xLayoutManager->hideElement(sName);
        m_xLayoutManager->destroyElement(sName);
    }
}

void ToolbarTabPage::LockToolbar(const OUString& sName, const bool bLock)
{
    const bool bIsVisible = m_xLayoutManager->isElementVisible(sName);
    // changing the locked state is possible only for visible toolbars
    if (!bIsVisible)
        m_xLayoutManager->createElement(sName);
    if (bLock)
    {
        m_xLayoutManager->dockWindow(sName, css::ui::DockingArea_DOCKINGAREA_DEFAULT,
                                     css::awt::Point(SAL_MAX_INT32, SAL_MAX_INT32));
        m_xLayoutManager->lockWindow(sName);
    }
    else
        m_xLayoutManager->unlockWindow(sName);
    if (!bIsVisible)
        m_xLayoutManager->destroyElement(sName);
}

void ToolbarTabPage::SensitiveToolbar(const OUString& sName, const bool bSensitive)
{
    const bool bIsVisible = m_xLayoutManager->isElementVisible(sName);
    if (!bIsVisible)
        m_xLayoutManager->createElement(sName);

    m_xLayoutManager->makeContextSensitive(sName, bSensitive);

    if (!bIsVisible)
        m_xLayoutManager->destroyElement(sName);
}

IMPL_LINK(ToolbarTabPage, CheckBoxHdl, weld::Toggleable&, rCheckBox, void)
{
    const bool bLock = rCheckBox.get_state() == TRISTATE_TRUE;

    std::unique_ptr<weld::TreeIter> pIter = m_pToolbarList->make_iterator();
    bool bLoop = m_pToolbarList->get_iter_first(*pIter);
    while (bLoop)
    {
        m_pToolbarList->set_toggle(*pIter, bLock ? TRISTATE_TRUE : TRISTATE_FALSE, 2);
        LockToolbar(m_pToolbarList->get_id(*pIter), bLock);
        bLoop = m_pToolbarList->iter_next(*pIter);
    }
    // gtk3 does not auto-set the checkbox state
    m_pLockAll->set_state(bLock ? TRISTATE_TRUE : TRISTATE_FALSE);
}

IMPL_LINK(ToolbarTabPage, ToggleHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    const int nRow(m_pToolbarList->get_iter_index_in_parent(rRowCol.first));
    m_pToolbarList->select(nRow);
    const OUString sToolbarUrl(m_pToolbarList->get_id(nRow));
    if (rRowCol.second == 1) // visible
    {
        const bool bShow = m_pToolbarList->get_toggle(nRow, 1) == TRISTATE_TRUE;
        ShowToolbar(sToolbarUrl, bShow);
    }
    else if (rRowCol.second == 2) // locked
    {
        const bool bLocked = m_pToolbarList->get_toggle(nRow, 2) == TRISTATE_TRUE;
        LockToolbar(sToolbarUrl, bLocked);
        UpdateAllLocked();
    }
    else if (rRowCol.second == 3) // sensitive
    {
        const bool bSensitive = m_pToolbarList->get_toggle(nRow, 3) == TRISTATE_TRUE;
        SensitiveToolbar(sToolbarUrl, bSensitive);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
