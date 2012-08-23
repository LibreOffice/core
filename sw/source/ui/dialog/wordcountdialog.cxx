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
#include <wordcountdialog.hrc>
#include <cmdid.h>
#include "vcl/msgbox.hxx" // RET_CANCEL
#include <swmodule.hxx>
#include <wview.hxx>
#include <sfx2/viewfrm.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>

//TODO, add asian/non-asian word count to UI when CJK mode is enabled.

SwWordCountDialog::SwWordCountDialog(Window* pParent) :
#if defined _MSC_VER
#pragma warning (disable : 4355)
#endif
    aCurrentFL( pParent, SW_RES(              FL_CURRENT            )),
    aCurrentWordFT( pParent, SW_RES(          FT_CURRENTWORD        )),
    aCurrentWordFI( pParent, SW_RES(          FI_CURRENTWORD        )),
    aCurrentCharacterFT( pParent, SW_RES(     FT_CURRENTCHARACTER   )),
    aCurrentCharacterFI( pParent, SW_RES(     FI_CURRENTCHARACTER   )),
    aCurrentCharacterExcludingSpacesFT( pParent, SW_RES(     FT_CURRENTCHARACTEREXCLUDINGSPACES   )),
    aCurrentCharacterExcludingSpacesFI( pParent, SW_RES(     FI_CURRENTCHARACTEREXCLUDINGSPACES   )),

    aDocFL( pParent, SW_RES(                  FL_DOC                )),
    aDocWordFT( pParent, SW_RES(              FT_DOCWORD            )),
    aDocWordFI( pParent, SW_RES(              FI_DOCWORD            )),
    aDocCharacterFT( pParent, SW_RES(         FT_DOCCHARACTER       )),
    aDocCharacterFI( pParent, SW_RES(         FI_DOCCHARACTER       )),
    aDocCharacterExcludingSpacesFT( pParent, SW_RES(         FT_DOCCHARACTEREXCLUDINGSPACES       )),
    aDocCharacterExcludingSpacesFI( pParent, SW_RES(         FI_DOCCHARACTEREXCLUDINGSPACES       )),
    aBottomFL(pParent, SW_RES(                FL_BOTTOM             )),
    aOK( pParent, SW_RES(                     PB_OK                 )),
    aHelp( pParent, SW_RES(                   PB_HELP               ))
#if defined _MSC_VER
#pragma warning (default : 4355)
#endif
{
    aOK.SetClickHdl(LINK(this,SwWordCountDialog,        OkHdl));
}

IMPL_LINK_NOARG(SwWordCountDialog, OkHdl)
{   
    SfxViewFrame* pVFrame = ::GetActiveView()->GetViewFrame();
    if (pVFrame != NULL)
    {
        pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
    }
    return 0;
}

SwWordCountDialog::~SwWordCountDialog()
{
    ViewShell::SetCareWin( 0 );
}

void  SwWordCountDialog::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
    aCurrentWordFI.SetText(     String::CreateFromInt32(rCurrent.nWord ));
    aCurrentCharacterFI.SetText(String::CreateFromInt32(rCurrent.nChar ));
    aCurrentCharacterExcludingSpacesFI.SetText(String::CreateFromInt32(rCurrent.nCharExcludingSpaces ));
    aDocWordFI.SetText(         String::CreateFromInt32(rDoc.nWord ));
    aDocCharacterFI.SetText(    String::CreateFromInt32(rDoc.nChar ));
    aDocCharacterExcludingSpacesFI.SetText(    String::CreateFromInt32(rDoc.nCharExcludingSpaces ));
}


SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         Window *pParent,
                                         SfxChildWinInfo* pInfo)
    : SfxModelessDialog(_pBindings, pChild, pParent, SW_RES(DLG_WORDCOUNT)),
#if defined _MSC_VER
#pragma warning (disable : 4355)
#endif
      aDlg(this)
#if defined _MSC_VER
#pragma warning (default : 4355)
#endif
{
    FreeResource();
    Initialize(pInfo);
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
    aDlg.SetValues(aCurrCnt, aDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
