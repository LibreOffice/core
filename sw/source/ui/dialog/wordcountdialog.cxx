/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>
#include <dialog.hrc>
#include <cmdid.h>
#include "vcl/msgbox.hxx" // RET_CANCEL
#include <swmodule.hxx>
#include <wview.hxx>
#include <sfx2/viewfrm.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>

IMPL_LINK_NOARG(SwWordCountFloatDlg, CloseHdl)
{   
    SfxViewFrame* pVFrame = ::GetActiveView()->GetViewFrame();
    if (pVFrame != NULL)
    {
        pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
    }
    return 0;
}

SwWordCountFloatDlg::~SwWordCountFloatDlg()
{
    ViewShell::SetCareWin( 0 );
}

void SwWordCountFloatDlg::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
    m_pCurrentWordFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rCurrent.nWord)));
    m_pCurrentCharacterFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rCurrent.nChar)));
    m_pCurrentCharacterExcludingSpacesFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rCurrent.nCharExcludingSpaces)));
    m_pDocWordFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rDoc.nWord)));
    m_pDocCharacterFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rDoc.nChar)));
    m_pDocCharacterExcludingSpacesFT->SetText(rtl::OUString::valueOf(static_cast<sal_Int64>(rDoc.nCharExcludingSpaces)));
}

//TODO, add asian/non-asian word count to UI when CJK mode is enabled.
SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         Window *pParent,
                                         SfxChildWinInfo* pInfo)
    : SfxModelessDialog(_pBindings, pChild, pParent, "WordCountDialog", "modules/swriter/ui/wordcount.ui")
{
    m_pUIBuilder->get(m_pCurrentWordFT, "selectwords");
    m_pUIBuilder->get(m_pCurrentCharacterFT, "selectchars");
    m_pUIBuilder->get(m_pCurrentCharacterExcludingSpacesFT, "selectcharsnospaces");
    m_pUIBuilder->get(m_pDocWordFT, "docwords");
    m_pUIBuilder->get(m_pDocCharacterFT, "docchars");
    m_pUIBuilder->get(m_pDocCharacterExcludingSpacesFT, "doccharsnospaces");
    m_pUIBuilder->get(m_pClosePB, "close");

    Initialize(pInfo);

    m_pClosePB->SetClickHdl(LINK(this, SwWordCountFloatDlg, CloseHdl));
    m_pClosePB->GrabFocus();
}

void SwWordCountFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
}

void SwWordCountFloatDlg::UpdateCounts()
{
    SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
    SwDocStat aCurrCnt;
    SwDocStat aDocStat;
    {
        SwWait aWait( *::GetActiveView()->GetDocShell(), sal_True );
        rSh.StartAction();
        rSh.CountWords( aCurrCnt );
        aDocStat = rSh.GetUpdatedDocStat();
        rSh.EndAction();
    }
    SetValues(aCurrCnt, aDocStat);
}

void SwWordCountFloatDlg::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    aDlg.SetValues(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
