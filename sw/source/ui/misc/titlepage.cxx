/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <charfmt.hxx>
#include <docstyle.hxx>

#include "fldbas.hxx"
#include "lineinfo.hxx"
#include "globals.hrc"
#include "titlepage.hxx"
#include "uitool.hxx"
#include "fmtpdsc.hxx"
#include "pagedesc.hxx"

#include <IDocumentStylePoolAccess.hxx>

namespace
{
    bool lcl_GetPageDesc(SwWrtShell *pSh, sal_uInt16 &rPageNo, const SwFmtPageDesc **ppPageFmtDesc)
    {
        bool bRet = false;
        SfxItemSet aSet( pSh->GetAttrPool(), RES_PAGEDESC, RES_PAGEDESC );
        if (pSh->GetCurAttr( aSet ))
        {
            const SfxPoolItem* pItem(0);
            if (SfxItemState::SET == aSet.GetItemState( RES_PAGEDESC, true, &pItem ) && pItem)
            {
                ::boost::optional<sal_uInt16> oNumOffset = ((const SwFmtPageDesc *)pItem)->GetNumOffset();
                if (oNumOffset)
                    rPageNo = oNumOffset.get();
                if (ppPageFmtDesc)
                    (*ppPageFmtDesc) = (const SwFmtPageDesc *)(pItem->Clone());
                bRet = true;
            }
        }
        return bRet;
    }

    void lcl_ChangePage(SwWrtShell *pSh, sal_uInt16 nNewNumber,
        const SwPageDesc *pNewDesc)
    {
        const sal_uInt16 nCurIdx = pSh->GetCurPageDesc();
        const SwPageDesc &rCurrentDesc = pSh->GetPageDesc( nCurIdx );

        const SwFmtPageDesc *pPageFmtDesc(0);
        sal_uInt16 nDontCare;
        lcl_GetPageDesc(pSh, nDontCare, &pPageFmtDesc);

        // If we want a new number then set it, otherwise reuse the existing one
        sal_uInt16 nPgNo;
        if (nNewNumber)
        {
            nPgNo = nNewNumber;
        }
        else
        {
            if (pPageFmtDesc)
            {
                ::boost::optional<sal_uInt16> oNumOffset = pPageFmtDesc->GetNumOffset();
                if (oNumOffset)
                {
                    nPgNo = oNumOffset.get();
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

        // If we want a new descriptior then set it, otherwise reuse the existing one
        if (!pNewDesc)
        {
            SwFmtPageDesc aPageFmtDesc(pPageFmtDesc ? *pPageFmtDesc : &rCurrentDesc);
            if (nPgNo) aPageFmtDesc.SetNumOffset(nPgNo);
            pSh->SetAttrItem(aPageFmtDesc);
        }
        else
        {
            SwFmtPageDesc aPageFmtDesc(pNewDesc);
            if (nPgNo) aPageFmtDesc.SetNumOffset(nPgNo);
            pSh->SetAttrItem(aPageFmtDesc);
        }

        delete pPageFmtDesc;
    }

    void lcl_PushCursor(SwWrtShell *pSh)
    {
        pSh->LockView( true );
        pSh->StartAllAction();
        pSh->SwCrsrShell::Push();
    }

    void lcl_PopCursor(SwWrtShell *pSh)
    {
        pSh->SwCrsrShell::Pop( false );
        pSh->EndAllAction();
        pSh->LockView( false );
    }

    sal_uInt16 lcl_GetCurrentPage(SwWrtShell *pSh)
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
    sal_uInt16 nTitlePages = m_pPageCountNF->GetValue();
    m_pPagePropertiesLB->Clear();
    if (mpTitleDesc)
        m_pPagePropertiesLB->InsertEntry(mpTitleDesc->GetName());
    if (nTitlePages > 1 && mpIndexDesc)
        m_pPagePropertiesLB->InsertEntry(mpIndexDesc->GetName());
    if (mpNormalDesc)
        m_pPagePropertiesLB->InsertEntry(mpNormalDesc->GetName());
    m_pPagePropertiesLB->SelectEntryPos(0);
}

sal_uInt16 SwTitlePageDlg::GetInsertPosition() const
{
    sal_uInt16 nPage = 1;
    if (m_pPageStartNF->IsEnabled())
        nPage = m_pPageStartNF->GetValue();
    return nPage;
}

SwTitlePageDlg::SwTitlePageDlg( vcl::Window *pParent ) :
    SfxModalDialog( pParent, "DLG_TITLEPAGE", "modules/swriter/ui/titlepage.ui"),
    mpPageFmtDesc(0)
{
    get(m_pUseExistingPagesRB, "RB_USE_EXISTING_PAGES");
    get(m_pPageCountNF, "NF_PAGE_COUNT");
    get(m_pDocumentStartRB, "RB_DOCUMENT_START");
    get(m_pPageStartRB, "RB_PAGE_START");
    get(m_pPageStartNF, "NF_PAGE_START");
    get(m_pRestartNumberingCB, "CB_RESTART_NUMBERING");
    get(m_pRestartNumberingNF, "NF_RESTART_NUMBERING");
    get(m_pSetPageNumberCB, "CB_SET_PAGE_NUMBER");
    get(m_pSetPageNumberNF, "NF_SET_PAGE_NUMBER");
    get(m_pPagePropertiesLB, "LB_PAGE_PROPERTIES");
    get(m_pPagePropertiesPB, "PB_PAGE_PROPERTIES");
    get(m_pOkPB, "PB_OK");

    m_pOkPB->SetClickHdl(LINK(this, SwTitlePageDlg, OKHdl));
    m_pRestartNumberingCB->SetClickHdl(LINK(this, SwTitlePageDlg, RestartNumberingHdl));
    m_pSetPageNumberCB->SetClickHdl(LINK(this, SwTitlePageDlg, SetPageNumberHdl));

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

    mpSh->SttDoc();
    if (lcl_GetPageDesc( mpSh, nSetPage, &mpPageFmtDesc ))
    {
        if (mpPageFmtDesc->GetPageDesc() == mpTitleDesc)
        {
            while (mpSh->SttNxtPg())
            {
                const sal_uInt16 nCurIdx = mpSh->GetCurPageDesc();
                const SwPageDesc &rPageDesc = mpSh->GetPageDesc( nCurIdx );

                if (mpIndexDesc != &rPageDesc)
                {
                    mpNormalDesc = &rPageDesc;
                    bMaybeResetNumbering = lcl_GetPageDesc(mpSh, nResetPage, NULL);
                    break;
                }
                ++nTitlePages;
            }
        }
    }
    lcl_PopCursor(mpSh);

    m_pUseExistingPagesRB->Check();
    m_pPageCountNF->SetValue(nTitlePages);
    m_pPageCountNF->SetUpHdl(LINK(this, SwTitlePageDlg, UpHdl));
    m_pPageCountNF->SetDownHdl(LINK(this, SwTitlePageDlg, DownHdl));

    m_pDocumentStartRB->Check();
    m_pPageStartNF->Enable(false);
    m_pPageStartNF->SetValue(lcl_GetCurrentPage(mpSh));
    Link aStartPageHdl = LINK(this, SwTitlePageDlg, StartPageHdl);
    m_pDocumentStartRB->SetClickHdl(aStartPageHdl);
    m_pPageStartRB->SetClickHdl(aStartPageHdl);

    if (bMaybeResetNumbering && nResetPage > 0)
    {
        m_pRestartNumberingCB->Check();
        m_pRestartNumberingNF->SetValue(nResetPage);
    }
    m_pRestartNumberingNF->Enable(m_pRestartNumberingCB->IsChecked());

    m_pSetPageNumberNF->SetValue(nSetPage);
    if (nSetPage > 1)
        m_pSetPageNumberCB->Check();
    m_pSetPageNumberNF->Enable(m_pSetPageNumberCB->IsChecked());

    FillList();
    m_pPagePropertiesPB->SetClickHdl(LINK(this, SwTitlePageDlg, EditHdl));
}

IMPL_LINK_NOARG(SwTitlePageDlg, UpHdl)
{
    if (m_pPageCountNF->GetValue() == 2)
        FillList();
    return 0;
}

IMPL_LINK_NOARG(SwTitlePageDlg, DownHdl)
{
    if (m_pPageCountNF->GetValue() == 1)
        FillList();
    return 0;
}

IMPL_LINK_NOARG(SwTitlePageDlg, RestartNumberingHdl)
{
    m_pRestartNumberingNF->Enable(m_pRestartNumberingCB->IsChecked());
    return 0;
}

IMPL_LINK_NOARG(SwTitlePageDlg, SetPageNumberHdl)
{
    m_pSetPageNumberNF->Enable(m_pSetPageNumberCB->IsChecked());
    return 0;
}

IMPL_LINK_NOARG(SwTitlePageDlg, StartPageHdl)
{
    m_pPageStartNF->Enable(m_pPageStartRB->IsChecked());
    return 0;
}

SwTitlePageDlg::~SwTitlePageDlg()
{
    delete mpPageFmtDesc;
}

IMPL_LINK_NOARG(SwTitlePageDlg, EditHdl)
{
    SwView& rView = mpSh->GetView();
    rView.GetDocShell()->FormatPage(m_pPagePropertiesLB->GetSelectEntry(), "page", *mpSh);
    rView.InvalidateRulerPos();

    return 0;
}

IMPL_LINK_NOARG(SwTitlePageDlg, OKHdl)
{
    lcl_PushCursor(mpSh);

    mpSh->StartUndo();

    SwFmtPageDesc aTitleDesc(mpTitleDesc);

    if (m_pSetPageNumberCB->IsChecked())
        aTitleDesc.SetNumOffset(m_pSetPageNumberNF->GetValue());
    else if (mpPageFmtDesc)
        aTitleDesc.SetNumOffset(mpPageFmtDesc->GetNumOffset());

    sal_uInt16 nNoPages = m_pPageCountNF->GetValue();
    if (!m_pUseExistingPagesRB->IsChecked())
    {
        mpSh->GotoPage(GetInsertPosition(), false);
        for (sal_uInt16 nI=0; nI < nNoPages; ++nI)
            mpSh->InsertPageBreak();
    }

    mpSh->GotoPage(GetInsertPosition(), false);
    for (sal_uInt16 nI=1; nI < nNoPages; ++nI)
    {
        if (mpSh->SttNxtPg())
            lcl_ChangePage(mpSh, 0, mpIndexDesc);
    }

    mpSh->GotoPage(GetInsertPosition(), false);
    mpSh->SetAttrItem(aTitleDesc);

    if (nNoPages > 1 && mpSh->GotoPage(GetInsertPosition() + nNoPages, false))
    {
        SwFmtPageDesc aPageFmtDesc(mpNormalDesc);
        mpSh->SetAttrItem(aPageFmtDesc);
    }

    if (m_pRestartNumberingCB->IsChecked() || nNoPages > 1)
    {
        sal_uInt16 nPgNo = m_pRestartNumberingCB->IsChecked() ? m_pRestartNumberingNF->GetValue() : 0;
        const SwPageDesc *pNewDesc = nNoPages > 1 ? mpNormalDesc : 0;
        mpSh->GotoPage(GetInsertPosition() + nNoPages, false);
        lcl_ChangePage(mpSh, nPgNo, pNewDesc);
    }

    mpSh->EndUndo();
    lcl_PopCursor(mpSh);
    if (!m_pUseExistingPagesRB->IsChecked())
        mpSh->GotoPage(GetInsertPosition(), false);
    EndDialog( RET_OK );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
