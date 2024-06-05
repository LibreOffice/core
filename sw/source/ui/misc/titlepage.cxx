/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>

#include <titlepage.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>

namespace
{
    bool lcl_GetPageDesc(SwWrtShell& rSh, sal_uInt16 &rPageNo, std::unique_ptr<const SwFormatPageDesc>* ppPageFormatDesc)
    {
        bool bRet = false;
        SfxItemSetFixed<RES_PAGEDESC, RES_PAGEDESC> aSet(rSh.GetAttrPool());
        if (rSh.GetCurAttr(aSet))
        {
            if (const SwFormatPageDesc* pDescItem = aSet.GetItemIfSet( RES_PAGEDESC ))
            {
                ::std::optional<sal_uInt16> oNumOffset = pDescItem->GetNumOffset();
                if (oNumOffset)
                    rPageNo = *oNumOffset;
                if (ppPageFormatDesc)
                    ppPageFormatDesc->reset(static_cast<const SwFormatPageDesc *>(pDescItem->Clone()));
                bRet = true;
            }
        }
        return bRet;
    }

    void lcl_ChangePage(SwWrtShell& rSh, sal_uInt16 nNewNumber, const SwPageDesc *pNewDesc)
    {
        const size_t nCurIdx = rSh.GetCurPageDesc();
        const SwPageDesc &rCurrentDesc = rSh.GetPageDesc(nCurIdx);

        std::unique_ptr<const SwFormatPageDesc> pPageFormatDesc;
        sal_uInt16 nDontCare;
        lcl_GetPageDesc(rSh, nDontCare, &pPageFormatDesc);

        // If we want a new number then set it, otherwise reuse the existing one
        sal_uInt16 nPgNo = 0;
        if (nNewNumber)
        {
            // -1: Allow special case to prevent inheriting re-numbering from the existing page.
            nPgNo = nNewNumber == SAL_MAX_UINT16 ? 0 : nNewNumber;
        }
        else if (pPageFormatDesc)
        {
            ::std::optional<sal_uInt16> oNumOffset = pPageFormatDesc->GetNumOffset();
            if (oNumOffset)
                nPgNo = *oNumOffset;
        }

        // If we want a new descriptor then set it, otherwise reuse the existing one
        if (pNewDesc || nPgNo)
        {
            SwFormatPageDesc aPageFormatDesc(pNewDesc ? pNewDesc : &rCurrentDesc);
            if (nPgNo) aPageFormatDesc.SetNumOffset(nPgNo);
            rSh.SetAttrItem(aPageFormatDesc);
        }
    }

    void lcl_PushCursor(SwWrtShell& rSh)
    {
        rSh.LockView(true);
        rSh.StartAllAction();
        rSh.SwCursorShell::Push();
    }

    void lcl_PopCursor(SwWrtShell& rSh)
    {
        rSh.SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
        rSh.EndAllAction();
        rSh.LockView(false);
    }

    sal_uInt16 lcl_GetCurrentPage(const SwWrtShell& rSh)
    {
        OUString sDummy;
        sal_uInt16 nPhyNum=1, nVirtNum=1;
        rSh.GetPageNumber(0, true, nPhyNum, nVirtNum, sDummy);
        return nPhyNum;
    }

bool lcl_GotoPage(SwWrtShell& rSh, const sal_uInt16 nStartingPage, sal_uInt16 nOffset = 0)
{
    rSh.GotoPage(nStartingPage, /*bRecord=*/false);

    sal_uInt16 nCurrentPage = lcl_GetCurrentPage(rSh);
    // return false if at document end (unless that was the requested destination)
    if (nCurrentPage == rSh.GetPageCnt())
        return nCurrentPage == nStartingPage + nOffset;

    if (nCurrentPage != nStartingPage)
    {
        assert(nStartingPage != 1 && "Physical page 1 couldn't be found/moved to?");
        // Probably there is an auto-inserted blank page to handle odd/even, which Goto doesn't understand.
        rSh.GotoPage(nStartingPage + 1, /*bRecord=*/false);

        nCurrentPage = lcl_GetCurrentPage(rSh);
        assert(nCurrentPage == nStartingPage + 1 && "Impossible, since unknown goes to last page");
        if (nCurrentPage != nStartingPage + 1)
            return false;
    }
    // Now that we have the correct starting point, move to the correct offset.
    while (nOffset--)
        rSh.SttNxtPg();
    return true;
}
} // namespace

/*
 * Only include the Index page in the list if the page count implies one
 * to reduce confusing things
 */
void SwTitlePageDlg::FillList()
{
    sal_uInt16 nTitlePages = m_xPageCountNF->get_value();
    m_xPagePropertiesLB->clear();
    if (mpTitleDesc)
        m_xPagePropertiesLB->append_text(mpTitleDesc->GetName());
    if (nTitlePages > 1 && mpIndexDesc)
        m_xPagePropertiesLB->append_text(mpIndexDesc->GetName());
    if (mpNormalDesc)
        m_xPagePropertiesLB->append_text(mpNormalDesc->GetName());
    m_xPagePropertiesLB->set_active(0);
}

sal_uInt16 SwTitlePageDlg::GetInsertPosition() const
{
    sal_uInt16 nPage = 1;
    if (m_xPageStartNF->get_sensitive())
        nPage = m_xPageStartNF->get_value();
    return nPage;
}

SwTitlePageDlg::SwTitlePageDlg(weld::Window *pParent)
    : SfxDialogController(pParent, u"modules/swriter/ui/titlepage.ui"_ustr, u"DLG_TITLEPAGE"_ustr)
    , mpTitleDesc(nullptr)
    , mpIndexDesc(nullptr)
    , mpNormalDesc(nullptr)
    , m_xUseExistingPagesRB(m_xBuilder->weld_radio_button(u"RB_USE_EXISTING_PAGES"_ustr))
    , m_xInsertNewPagesRB(m_xBuilder->weld_radio_button(u"RB_INSERT_NEW_PAGES"_ustr))
    , m_xPageCountNF(m_xBuilder->weld_spin_button(u"NF_PAGE_COUNT"_ustr))
    , m_xDocumentStartRB(m_xBuilder->weld_radio_button(u"RB_DOCUMENT_START"_ustr))
    , m_xPageStartRB(m_xBuilder->weld_radio_button(u"RB_PAGE_START"_ustr))
    , m_xPageStartNF(m_xBuilder->weld_spin_button(u"NF_PAGE_START"_ustr))
    , m_xRestartNumberingCB(m_xBuilder->weld_check_button(u"CB_RESTART_NUMBERING"_ustr))
    , m_xRestartNumberingNF(m_xBuilder->weld_spin_button(u"NF_RESTART_NUMBERING"_ustr))
    , m_xSetPageNumberCB(m_xBuilder->weld_check_button(u"CB_SET_PAGE_NUMBER"_ustr))
    , m_xSetPageNumberNF(m_xBuilder->weld_spin_button(u"NF_SET_PAGE_NUMBER"_ustr))
    , m_xPagePropertiesLB(m_xBuilder->weld_combo_box(u"LB_PAGE_PROPERTIES"_ustr))
    , m_xPagePropertiesPB(m_xBuilder->weld_button(u"PB_PAGE_PROPERTIES"_ustr))
    , m_xOkPB(m_xBuilder->weld_button(u"ok"_ustr))
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rWrtShell = pView->GetWrtShell();

    m_xOkPB->connect_clicked(LINK(this, SwTitlePageDlg, OKHdl));
    m_xRestartNumberingCB->connect_toggled(LINK(this, SwTitlePageDlg, RestartNumberingHdl));
    m_xSetPageNumberCB->connect_toggled(LINK(this, SwTitlePageDlg, SetPageNumberHdl));
    m_xPageStartNF->set_max(rWrtShell.GetPageCnt() + 1);

    sal_uInt16 nSetPage = 1;
    sal_uInt16 nResetPage = 1;
    sal_uInt16 nTitlePages = 1;
    lcl_PushCursor(rWrtShell);

    SwView& rView = rWrtShell.GetView();
    rView.InvalidateRulerPos();

    bool bMaybeResetNumbering = false;

    mpTitleDesc = rWrtShell.GetPageDescFromPool(RES_POOLPAGE_FIRST);
    mpIndexDesc = rWrtShell.GetPageDescFromPool(RES_POOLPAGE_REGISTER);
    mpNormalDesc = rWrtShell.GetPageDescFromPool(RES_POOLPAGE_STANDARD);

    rWrtShell.StartOfSection();
    if (lcl_GetPageDesc(rWrtShell, nSetPage, &mpPageFormatDesc))
    {
        if (mpPageFormatDesc->GetPageDesc() == mpTitleDesc)
        {
            while (rWrtShell.SttNxtPg())
            {
                const size_t nCurIdx = rWrtShell.GetCurPageDesc();
                const SwPageDesc& rPageDesc = rWrtShell.GetPageDesc(nCurIdx);

                if (mpIndexDesc != &rPageDesc)
                {
                    mpNormalDesc = &rPageDesc;
                    bMaybeResetNumbering = lcl_GetPageDesc(rWrtShell, nResetPage, nullptr);
                    break;
                }
                ++nTitlePages;
            }
        }
    }
    lcl_PopCursor(rWrtShell);

    m_xUseExistingPagesRB->set_active(true);
    m_xPageCountNF->set_value(nTitlePages);
    m_xPageCountNF->connect_value_changed(LINK(this, SwTitlePageDlg, ValueChangeHdl));

    m_xDocumentStartRB->set_active(true);
    m_xPageStartNF->set_sensitive(false);
    m_xPageStartNF->set_value(lcl_GetCurrentPage(rWrtShell));
    Link<weld::Toggleable&,void> aStartPageHdl = LINK(this, SwTitlePageDlg, StartPageHdl);
    m_xDocumentStartRB->connect_toggled(aStartPageHdl);
    m_xPageStartRB->connect_toggled(aStartPageHdl);

    m_xRestartNumberingNF->set_value(nResetPage);
    if (bMaybeResetNumbering && nResetPage > 0)
    {
        m_xRestartNumberingCB->set_active(true);
    }
    m_xRestartNumberingNF->set_sensitive(m_xRestartNumberingCB->get_active());

    m_xSetPageNumberNF->set_value(nSetPage);
    if (nSetPage > 1)
        m_xSetPageNumberCB->set_active(true);
    m_xSetPageNumberNF->set_sensitive(m_xSetPageNumberCB->get_active());

    FillList();
    m_xPagePropertiesPB->connect_clicked(LINK(this, SwTitlePageDlg, EditHdl));
}

IMPL_LINK_NOARG(SwTitlePageDlg, ValueChangeHdl, weld::SpinButton&, void)
{
    if (m_xPageCountNF->get_value() == 1 || m_xPageCountNF->get_value() == 2)
        FillList();
}

IMPL_LINK_NOARG(SwTitlePageDlg, RestartNumberingHdl, weld::Toggleable&, void)
{
    m_xRestartNumberingNF->set_sensitive(m_xRestartNumberingCB->get_active());
}

IMPL_LINK_NOARG(SwTitlePageDlg, SetPageNumberHdl, weld::Toggleable&, void)
{
    m_xSetPageNumberNF->set_sensitive(m_xSetPageNumberCB->get_active());
}

IMPL_LINK_NOARG(SwTitlePageDlg, StartPageHdl, weld::Toggleable&, void)
{
    m_xPageStartNF->set_sensitive(m_xPageStartRB->get_active());
}

SwTitlePageDlg::~SwTitlePageDlg()
{
}

IMPL_LINK_NOARG(SwTitlePageDlg, EditHdl, weld::Button&, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rWrtShell = pView->GetWrtShell();
    SwView& rView = rWrtShell.GetView();
    rView.GetDocShell()->FormatPage(getDialog(), m_xPagePropertiesLB->get_active_text(), u"page"_ustr, rWrtShell);
    rView.InvalidateRulerPos();
}

IMPL_LINK_NOARG(SwTitlePageDlg, OKHdl, weld::Button&, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rWrtShell = pView->GetWrtShell();
    lcl_PushCursor(rWrtShell);

    rWrtShell.StartUndo();

    SwFormatPageDesc aTitleDesc(mpTitleDesc);

    if (m_xSetPageNumberCB->get_active())
        aTitleDesc.SetNumOffset(m_xSetPageNumberNF->get_value());
    else if (mpPageFormatDesc)
        aTitleDesc.SetNumOffset(mpPageFormatDesc->GetNumOffset());

    sal_uInt16 nNumTitlePages = m_xPageCountNF->get_value();
    if (!m_xUseExistingPagesRB->get_active())
    {
        // Assuming that a failure to GotoPage means the end of the document,
        // insert new pages after the last page.
        if (!lcl_GotoPage(rWrtShell, GetInsertPosition()))
        {
            rWrtShell.EndPg();
            // Add one more page as a content page to follow the new title pages.
            rWrtShell.InsertPageBreak();
        }
        for (sal_uInt16 nI = 0; nI < nNumTitlePages; ++nI)
            rWrtShell.InsertPageBreak();
        // In order to be able to access these new pages, the layout needs to be recalculated first.
        rWrtShell.CalcLayout();
    }

    if (lcl_GotoPage(rWrtShell, GetInsertPosition()))
    {
        rWrtShell.SetAttrItem(aTitleDesc);
        for (sal_uInt16 nI = 1; nI < nNumTitlePages; ++nI)
        {
            if (rWrtShell.SttNxtPg())
                lcl_ChangePage(rWrtShell, SAL_MAX_UINT16, mpIndexDesc);
        }
    }

    if ((m_xRestartNumberingCB->get_active() || nNumTitlePages > 1)
        && lcl_GotoPage(rWrtShell, GetInsertPosition(), nNumTitlePages))
    {
        sal_uInt16 nPgNo
            = m_xRestartNumberingCB->get_active() ? m_xRestartNumberingNF->get_value() : 0;
        const SwPageDesc* pNewDesc = nNumTitlePages > 1 ? mpNormalDesc : nullptr;
        lcl_ChangePage(rWrtShell, nPgNo, pNewDesc);
    }

    rWrtShell.EndUndo();
    lcl_PopCursor(rWrtShell);
    if (!m_xUseExistingPagesRB->get_active())
        lcl_GotoPage(rWrtShell, GetInsertPosition());
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
