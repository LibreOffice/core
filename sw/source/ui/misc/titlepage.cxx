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
    bool lcl_GetPageDesc(SwWrtShell *pSh, sal_uInt16 &rPageNo, std::unique_ptr<const SwFormatPageDesc>* ppPageFormatDesc)
    {
        bool bRet = false;
        SfxItemSet aSet( pSh->GetAttrPool(), svl::Items<RES_PAGEDESC, RES_PAGEDESC>{} );
        if (pSh->GetCurAttr( aSet ))
        {
            const SfxPoolItem* pItem(nullptr);
            if (SfxItemState::SET == aSet.GetItemState( RES_PAGEDESC, true, &pItem ) && pItem)
            {
                ::std::optional<sal_uInt16> oNumOffset = static_cast<const SwFormatPageDesc *>(pItem)->GetNumOffset();
                if (oNumOffset)
                    rPageNo = *oNumOffset;
                if (ppPageFormatDesc)
                    ppPageFormatDesc->reset(static_cast<const SwFormatPageDesc *>(pItem->Clone()));
                bRet = true;
            }
        }
        return bRet;
    }

    void lcl_ChangePage(SwWrtShell *pSh, sal_uInt16 nNewNumber,
        const SwPageDesc *pNewDesc)
    {
        const size_t nCurIdx = pSh->GetCurPageDesc();
        const SwPageDesc &rCurrentDesc = pSh->GetPageDesc( nCurIdx );

        std::unique_ptr<const SwFormatPageDesc> pPageFormatDesc;
        sal_uInt16 nDontCare;
        lcl_GetPageDesc(pSh, nDontCare, &pPageFormatDesc);

        // If we want a new number then set it, otherwise reuse the existing one
        sal_uInt16 nPgNo;
        if (nNewNumber)
        {
            nPgNo = nNewNumber;
        }
        else
        {
            if (pPageFormatDesc)
            {
                ::std::optional<sal_uInt16> oNumOffset = pPageFormatDesc->GetNumOffset();
                if (oNumOffset)
                {
                    nPgNo = *oNumOffset;
                }
                else
                {
                    nPgNo = 0;
                }
            }
            else
            {
                nPgNo = 0;
            }
        }

        // If we want a new descriptor then set it, otherwise reuse the existing one
        if (pNewDesc || nPgNo)
        {
            SwFormatPageDesc aPageFormatDesc(pNewDesc ? pNewDesc : &rCurrentDesc);
            if (nPgNo) aPageFormatDesc.SetNumOffset(nPgNo);
            pSh->SetAttrItem(aPageFormatDesc);
        }
    }

    void lcl_PushCursor(SwWrtShell *pSh)
    {
        pSh->LockView( true );
        pSh->StartAllAction();
        pSh->SwCursorShell::Push();
    }

    void lcl_PopCursor(SwWrtShell *pSh)
    {
        pSh->SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
        pSh->EndAllAction();
        pSh->LockView( false );
    }

    sal_uInt16 lcl_GetCurrentPage(SwWrtShell const *pSh)
    {
        OUString sDummy;
        sal_uInt16 nPhyNum=1, nVirtNum=1;
        pSh->GetPageNumber(0, true, nPhyNum, nVirtNum, sDummy);
        return nPhyNum;
    }
}

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
    // FIXME: If GetInsertPosition is greater than the current number of pages,
    // it needs to be reduced to page-count + 1.
    if (m_xPageStartNF->get_sensitive())
        nPage = m_xPageStartNF->get_value();
    return nPage;
}

SwTitlePageDlg::SwTitlePageDlg(weld::Window *pParent)
    : SfxDialogController(pParent, "modules/swriter/ui/titlepage.ui", "DLG_TITLEPAGE")
    , m_xUseExistingPagesRB(m_xBuilder->weld_radio_button("RB_USE_EXISTING_PAGES"))
    , m_xPageCountNF(m_xBuilder->weld_spin_button("NF_PAGE_COUNT"))
    , m_xDocumentStartRB(m_xBuilder->weld_radio_button("RB_DOCUMENT_START"))
    , m_xPageStartRB(m_xBuilder->weld_radio_button("RB_PAGE_START"))
    , m_xPageStartNF(m_xBuilder->weld_spin_button("NF_PAGE_START"))
    , m_xRestartNumberingCB(m_xBuilder->weld_check_button("CB_RESTART_NUMBERING"))
    , m_xRestartNumberingNF(m_xBuilder->weld_spin_button("NF_RESTART_NUMBERING"))
    , m_xSetPageNumberCB(m_xBuilder->weld_check_button("CB_SET_PAGE_NUMBER"))
    , m_xSetPageNumberNF(m_xBuilder->weld_spin_button("NF_SET_PAGE_NUMBER"))
    , m_xPagePropertiesLB(m_xBuilder->weld_combo_box("LB_PAGE_PROPERTIES"))
    , m_xPagePropertiesPB(m_xBuilder->weld_button("PB_PAGE_PROPERTIES"))
    , m_xOkPB(m_xBuilder->weld_button("ok"))
{
    m_xOkPB->connect_clicked(LINK(this, SwTitlePageDlg, OKHdl));
    m_xRestartNumberingCB->connect_toggled(LINK(this, SwTitlePageDlg, RestartNumberingHdl));
    m_xSetPageNumberCB->connect_toggled(LINK(this, SwTitlePageDlg, SetPageNumberHdl));

    sal_uInt16 nSetPage = 1;
    sal_uInt16 nResetPage = 1;
    sal_uInt16 nTitlePages = 1;
    mpSh = ::GetActiveView()->GetWrtShellPtr();
    lcl_PushCursor(mpSh);

    SwView& rView = mpSh->GetView();
    rView.InvalidateRulerPos();

    bool bMaybeResetNumbering = false;

    mpTitleDesc = mpSh->GetPageDescFromPool(RES_POOLPAGE_FIRST);
    mpIndexDesc = mpSh->GetPageDescFromPool(RES_POOLPAGE_REGISTER);
    mpNormalDesc = mpSh->GetPageDescFromPool(RES_POOLPAGE_STANDARD);

    mpSh->StartOfSection();
    if (lcl_GetPageDesc( mpSh, nSetPage, &mpPageFormatDesc ))
    {
        if (mpPageFormatDesc->GetPageDesc() == mpTitleDesc)
        {
            while (mpSh->SttNxtPg())
            {
                const size_t nCurIdx = mpSh->GetCurPageDesc();
                const SwPageDesc &rPageDesc = mpSh->GetPageDesc( nCurIdx );

                if (mpIndexDesc != &rPageDesc)
                {
                    mpNormalDesc = &rPageDesc;
                    bMaybeResetNumbering = lcl_GetPageDesc(mpSh, nResetPage, nullptr);
                    break;
                }
                ++nTitlePages;
            }
        }
    }
    lcl_PopCursor(mpSh);

    m_xUseExistingPagesRB->set_active(true);
    m_xPageCountNF->set_value(nTitlePages);
    m_xPageCountNF->connect_value_changed(LINK(this, SwTitlePageDlg, ValueChangeHdl));

    m_xDocumentStartRB->set_active(true);
    m_xPageStartNF->set_sensitive(false);
    m_xPageStartNF->set_value(lcl_GetCurrentPage(mpSh));
    Link<weld::ToggleButton&,void> aStartPageHdl = LINK(this, SwTitlePageDlg, StartPageHdl);
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

IMPL_LINK_NOARG(SwTitlePageDlg, RestartNumberingHdl, weld::ToggleButton&, void)
{
    m_xRestartNumberingNF->set_sensitive(m_xRestartNumberingCB->get_active());
}

IMPL_LINK_NOARG(SwTitlePageDlg, SetPageNumberHdl, weld::ToggleButton&, void)
{
    m_xSetPageNumberNF->set_sensitive(m_xSetPageNumberCB->get_active());
}

IMPL_LINK_NOARG(SwTitlePageDlg, StartPageHdl, weld::ToggleButton&, void)
{
    m_xPageStartNF->set_sensitive(m_xPageStartRB->get_active());
}

SwTitlePageDlg::~SwTitlePageDlg()
{
}

IMPL_LINK_NOARG(SwTitlePageDlg, EditHdl, weld::Button&, void)
{
    SwView& rView = mpSh->GetView();
    rView.GetDocShell()->FormatPage(m_xPagePropertiesLB->get_active_text(), "page", *mpSh);
    rView.InvalidateRulerPos();
}

IMPL_LINK_NOARG(SwTitlePageDlg, OKHdl, weld::Button&, void)
{
    // FIXME: This wizard is almost completely non-functional for inserting new pages.

    lcl_PushCursor(mpSh);

    mpSh->StartUndo();

    SwFormatPageDesc aTitleDesc(mpTitleDesc);

    if (m_xSetPageNumberCB->get_active())
        aTitleDesc.SetNumOffset(m_xSetPageNumberNF->get_value());
    else if (mpPageFormatDesc)
        aTitleDesc.SetNumOffset(mpPageFormatDesc->GetNumOffset());

    sal_uInt16 nNoPages = m_xPageCountNF->get_value();
    if (!m_xUseExistingPagesRB->get_active())
    {
        // FIXME: If the starting page number is larger than the last page,
        // probably should add pages AFTER the last page, not before it.
        mpSh->GotoPage(GetInsertPosition(), false);
        // FIXME: These new pages cannot be accessed currently with GotoPage. It doesn't know they exist.
        for (sal_uInt16 nI=0; nI < nNoPages; ++nI)
            mpSh->InsertPageBreak();
    }

    mpSh->GotoPage(GetInsertPosition(), false);
    // FIXME: GotoPage is pointing to a page after the newly created index pages, so the wrong pages are getting Index style.
    for (sal_uInt16 nI=1; nI < nNoPages; ++nI)
    {
        if (mpSh->SttNxtPg())
            lcl_ChangePage(mpSh, 0, mpIndexDesc);
    }

    mpSh->GotoPage(GetInsertPosition(), false);
    mpSh->SetAttrItem(aTitleDesc);

    if (nNoPages > 1 && mpSh->GotoPage(GetInsertPosition() + nNoPages, false))
    {
        // FIXME: By definition, GotoPage(x,bRecord=false) returns false. This is dead code.
        SwFormatPageDesc aPageFormatDesc(mpNormalDesc);
        mpSh->SetAttrItem(aPageFormatDesc);
    }

    if (m_xRestartNumberingCB->get_active() || nNoPages > 1)
    {
        sal_uInt16 nPgNo = m_xRestartNumberingCB->get_active() ? m_xRestartNumberingNF->get_value() : 0;
        const SwPageDesc *pNewDesc = nNoPages > 1 ? mpNormalDesc : nullptr;
        mpSh->GotoPage(GetInsertPosition() + nNoPages, false);
        lcl_ChangePage(mpSh, nPgNo, pNewDesc);
    }

    mpSh->EndUndo();
    lcl_PopCursor(mpSh);
    if (!m_xUseExistingPagesRB->get_active())
        mpSh->GotoPage(GetInsertPosition(), false);
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
