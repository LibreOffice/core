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
#include <sfx2/viewfrm.hxx>
#include <vcl/commandinfoprovider.hxx>

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
{
    m_pToolbarList->enable_toggle_buttons(weld::ColumnToggleType::Check);
    m_pToolbarList->connect_toggled(LINK(this, ToolbarTabPage, ToggleHdl));
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
    xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= m_xLayoutManager;

    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aSeqDocToolBars;
    aSeqDocToolBars = xUIConfigMgr->getUIElementsInfo(css::ui::UIElementType::TOOLBAR);

    for (css::uno::Sequence<css::beans::PropertyValue> const& props : aSeqDocToolBars)
    {
        for (css::beans::PropertyValue const& prop : props)
        {
            if (prop.Name == u"ResourceURL"_ustr)
            {
                css::uno::Sequence<css::beans::PropertyValue> aCmdProps;
                OUString sResourceURL;
                OUString sUIName;
                bool bHide = true;

                prop.Value >>= sResourceURL;

                if (xModuleConf->hasByName(sResourceURL))
                {
                    xModuleConf->getByName(sResourceURL) >>= aCmdProps;
                    for (const auto& aCmdProp : aCmdProps)
                    {
                        if (aCmdProp.Name == u"UIName"_ustr)
                            aCmdProp.Value >>= sUIName;
                        else if (aCmdProp.Name == u"HideFromToolbarMenu"_ustr)
                            aCmdProp.Value >>= bHide;
                    }
                    if (!bHide)
                    {
                        m_pToolbarList->append();
                        const int nRow = m_pToolbarList->n_children() - 1;
                        m_pToolbarList->set_id(nRow, sResourceURL);
                        m_pToolbarList->set_text(nRow, sUIName, 0);
                        const bool bShow = m_xLayoutManager->isElementVisible(sResourceURL);
                        m_pToolbarList->set_toggle(nRow, bShow ? TRISTATE_TRUE : TRISTATE_FALSE);
                    }
                }
            }
        }
    }
    m_pToolbarList->make_sorted();
}

IMPL_LINK(ToolbarTabPage, ToggleHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    const int nRow(m_pToolbarList->get_iter_index_in_parent(rRowCol.first));
    m_pToolbarList->select(nRow);
    OUString sToolbarUrl(m_pToolbarList->get_id(nRow));
    if (m_pToolbarList->get_toggle(nRow) == TRISTATE_TRUE)
    {
        m_xLayoutManager->createElement(sToolbarUrl);
        m_xLayoutManager->showElement(sToolbarUrl);
    }
    else
    {
        m_xLayoutManager->hideElement(sToolbarUrl);
        m_xLayoutManager->destroyElement(sToolbarUrl);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
