/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "QuickFindPanel.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <svl/srchitem.hxx>
#include <view.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <swmodule.hxx>
#include <pam.hxx>
#include <contentindex.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <edtwin.hxx>
#include <com/sun/star/uno/Any.h>
using namespace css;
using namespace std;

const int MinimumPanelWidth = 250;

namespace sw::sidebar
{
std::unique_ptr<PanelLayout> QuickFindPanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to QuickFindPanel::Create", nullptr, 0);
    return std::make_unique<QuickFindPanel>(pParent);
}

QuickFindPanel::QuickFindPanel(weld::Widget* pParent)
    : PanelLayout(pParent, "QuickFindPanel", "modules/swriter/ui/sidebarquickfind.ui")
    , m_xSearchFindEntry(m_xBuilder->weld_entry("Find"))
    , m_xSearchFindsList(m_xBuilder->weld_tree_view("searchfinds"))
    , m_nRowHeight(m_xSearchFindsList->get_height_rows(4))
    , m_pWrtShell(::GetActiveWrtShell())

{
    m_xContainer->set_size_request(MinimumPanelWidth, -1);
    m_xSearchFindsList->set_size_request(1, m_nRowHeight);
    m_xSearchFindEntry->connect_activate(
        LINK(this, QuickFindPanel, SearchFindEntryActivateHandler));
    m_xSearchFindEntry->connect_changed(LINK(this, QuickFindPanel, SearchFindEntryChangedHandler));
    m_xSearchFindsList->connect_custom_get_size(
        LINK(this, QuickFindPanel, SearchFindsListCustomGetSizeHandler));
    m_xSearchFindsList->connect_custom_render(LINK(this, QuickFindPanel, SearchFindsListRender));
    m_xSearchFindsList->set_column_custom_renderer(1, true);
    m_xSearchFindsList->connect_changed(
        LINK(this, QuickFindPanel, SearchFindsListSelectionChangedHandler));
    m_xSearchFindsList->connect_row_activated(
        LINK(this, QuickFindPanel, SearchFindsListRowActivatedHandler));
}

QuickFindPanel::~QuickFindPanel()
{
    m_xSearchFindEntry.reset();
    m_xSearchFindsList.reset();
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryActivateHandler, weld::Entry&, bool)
{
    FillSearchFindsList();
    return true;
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args,
                Size)
{
    return Size(1, m_nRowHeight);
}

IMPL_LINK(QuickFindPanel, SearchFindsListRender, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    const OUString& rId = std::get<3>(aPayload);
    int nIndex = m_xSearchFindsList->find_id(rId);
    OUString aEntry(m_xSearchFindsList->get_text(nIndex));
    DrawTextFlags const nTextStyle = DrawTextFlags::Left | DrawTextFlags::VCenter
                                     | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak;
    tools::Rectangle aRect(
        rRect.TopLeft(),
        Size(rRenderContext.GetOutputSize().Width() - rRect.Left(), rRect.GetHeight()));
    rRenderContext.DrawText(aRect, aEntry, nTextStyle);
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListSelectionChangedHandler, weld::TreeView&, void)
{
    std::unique_ptr<SwPaM>& rxPaM = m_vPaMs[m_xSearchFindsList->get_cursor_index()];

    m_pWrtShell->StartAction();
    bool bFound = false;
    for (SwPaM& rPaM : m_pWrtShell->GetCursor()->GetRingContainer())
    {
        if (*rxPaM->GetPoint() == *rPaM.GetPoint() && *rxPaM->GetMark() == *rPaM.GetMark())
        {
            bFound = true;
            break;
        }
        m_pWrtShell->GoNextCursor();
    }
    if (!bFound)
    {
        m_pWrtShell->AssureStdMode();
        m_pWrtShell->SetSelection(*rxPaM);
    }
    m_pWrtShell->EndAction();

    SwShellCursor* pShellCursor = m_pWrtShell->GetCursor_();
    std::vector<basegfx::B2DRange> vRanges;
    for (const SwRect& rRect : *pShellCursor)
    {
        tools::Rectangle aRect = rRect.SVRect();
        vRanges.emplace_back(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
    }
    m_pWrtShell->GetView().BringToAttention(std::move(vRanges));
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindsListRowActivatedHandler, weld::TreeView&, bool)
{
    m_pWrtShell->GetView().GetEditWin().GrabFocus();
    return true;
}

IMPL_LINK_NOARG(QuickFindPanel, SearchFindEntryChangedHandler, weld::Entry&, void)
{
    m_xSearchFindsList->clear();
}

void QuickFindPanel::FillSearchFindsList()
{
    m_vPaMs.clear();
    m_xSearchFindsList->clear();
    const OUString& sText = m_xSearchFindEntry->get_text();
    css::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", css::uno::Any(sText) },
        { "SearchItem.Backward", css::uno::Any(false) },
        { "SearchItem.Command", css::uno::Any(sal_uInt16(SvxSearchCmd::FIND_ALL)) },
    }));

    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);

    if (m_pWrtShell->HasMark())
    {
        for (SwPaM& rPaM : m_pWrtShell->GetCursor()->GetRingContainer())
        {
            SwPosition* pMarkPosition = rPaM.GetMark();
            const SwContentIndex aContentIndex = pMarkPosition->nContent;
            const SwContentNode* pContentNode = aContentIndex.GetContentNode();
            const SwTextNode* pTextNode = pContentNode->GetTextNode();
            const OUString& sNodeText = pTextNode->GetText();
            auto nMarkIndex = rPaM.GetMark()->nContent.GetIndex();
            auto nPointIndex = rPaM.GetPoint()->nContent.GetIndex();

            auto nStartIndex = nMarkIndex - 50;
            if (nStartIndex < 0)
                nStartIndex = 0;
            auto nEndIndex = nPointIndex + 50;
            if (nEndIndex > sNodeText.getLength())
            {
                nEndIndex = sNodeText.getLength();
            }
            auto nCount = nMarkIndex - nStartIndex;
            OUString sTextBeforeFind = OUString::Concat(sNodeText.subView(nStartIndex, nCount));
            auto nCount1 = nPointIndex - nMarkIndex;
            OUString sFind = OUString::Concat(sNodeText.subView(nMarkIndex, nCount1));
            auto nCount2 = nEndIndex - nPointIndex;
            OUString sTextAfterFind = OUString::Concat(sNodeText.subView(nPointIndex, nCount2));
            OUString sStr = sTextBeforeFind + "[" + sFind + "]" + sTextAfterFind;

            std::unique_ptr<SwPaM> xPaM(std::make_unique<SwPaM>(*rPaM.GetMark(), *rPaM.GetPoint()));
            m_vPaMs.push_back(std::move(xPaM));
            OUString sId = OUString::number(m_xSearchFindsList->n_children());
            m_xSearchFindsList->append(sId, sStr);
        }
    }
}
}

// end of namespace ::sw::sidebar
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
