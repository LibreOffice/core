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
SwWordCountDialog::SwWordCountDialog(Dialog* pParent)
    : vbox(pParent)
    , content_area(&vbox)
    , aCurrentSelection(&content_area, false, 3)
    , aCurrentSelectionText(&aCurrentSelection, SW_RES(FT_CURRENT))
    , aCurrentSelectionLine(&aCurrentSelection, SW_RES(FL_CURRENT))
    , aSelectionBox(&content_area, false, 7)
    , aSelectionRow1(&aSelectionBox)
    , aCurrentWordFT(&aSelectionRow1, SW_RES(FT_CURRENTWORD))
    , aCurrentWordFI(&aSelectionRow1, SW_RES(FI_CURRENTWORD))
    , aSelectionRow2(&aSelectionBox)
    , aCurrentCharacterFT(&aSelectionRow2, SW_RES(FT_CURRENTCHARACTER))
    , aCurrentCharacterFI(&aSelectionRow2, SW_RES(FI_CURRENTCHARACTER))
    , aSelectionRow3(&aSelectionBox)
    , aCurrentCharacterExcludingSpacesFT(&aSelectionRow3, SW_RES(FT_CURRENTCHARACTEREXCLUDINGSPACES))
    , aCurrentCharacterExcludingSpacesFI(&aSelectionRow3, SW_RES(FI_CURRENTCHARACTEREXCLUDINGSPACES))
    , aDoc(&content_area, false, 3)
    , aDocText(&aDoc, SW_RES(FT_DOC))
    , aDocLine(&aDoc, SW_RES(FL_DOC))
    , aDocBox(&content_area, false, 7)
    , aDocRow1(&aDocBox)
    , aDocWordFT(&aDocRow1, SW_RES(FT_DOCWORD))
    , aDocWordFI(&aDocRow1, SW_RES(FI_DOCWORD))
    , aDocRow2(&aDocBox)
    , aDocCharacterFT(&aDocRow2, SW_RES(FT_DOCCHARACTER))
    , aDocCharacterFI(&aDocRow2, SW_RES(FI_DOCCHARACTER))
    , aDocRow3(&aDocBox)
    , aDocCharacterExcludingSpacesFT(&aDocRow3, SW_RES(FT_DOCCHARACTEREXCLUDINGSPACES))
    , aDocCharacterExcludingSpacesFI(&aDocRow3, SW_RES(FI_DOCCHARACTEREXCLUDINGSPACES))
    , aBottomFL(&vbox, SW_RES(FL_BOTTOM))
    , action_area(&vbox)
    , aOK(&action_area, SW_RES(PB_OK))
    , aHelp(&action_area, SW_RES(PB_HELP))
{
    rtl::OUString sForceInitialSize(RTL_CONSTASCII_USTRINGPARAM("        "));
    aCurrentWordFI.SetText(sForceInitialSize);
    aCurrentCharacterFI.SetText(sForceInitialSize);
    aCurrentCharacterExcludingSpacesFI.SetText(sForceInitialSize);
    aDocWordFI.SetText(sForceInitialSize);
    aDocCharacterFI.SetText(sForceInitialSize);
    aDocCharacterExcludingSpacesFI.SetText(sForceInitialSize);

    content_area.set_expand(true);

    aCurrentSelectionLine.set_expand(true);

    aSelectionBox.set_border_width(7);

    aSelectionRow1.set_expand(true);
    aCurrentWordFI.set_expand(true);

    aSelectionRow2.set_expand(true);
    aCurrentCharacterFI.set_expand(true);
    aSelectionRow3.set_expand(true);
    aCurrentCharacterExcludingSpacesFI.set_expand(true);

    aDocLine.set_expand(true);

    aDocBox.set_border_width(7);

    aDocRow1.set_expand(true);
    aDocWordFT.set_expand(true);

    aDocRow2.set_expand(true);
    aDocCharacterFI.set_expand(true);
    aDocRow3.set_expand(true);
    aDocCharacterExcludingSpacesFI.set_expand(true);

    aOK.set_pack_type(VCL_PACK_END);
    aHelp.set_pack_type(VCL_PACK_END);

    aOK.SetClickHdl(LINK(this, SwWordCountDialog, OkHdl));

    pParent->SetMinOutputSizePixel(vbox.GetOptimalSize(WINDOWSIZE_PREFERRED));

#if OSL_DEBUG_LEVEL > 2
    aDocCharacterExcludingSpacesFT.SetControlBackground(Color(180,0,0));
    aDocCharacterExcludingSpacesFI.SetControlBackground(Color(0,180,0));
#endif
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

void SwWordCountFloatDlg::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    aDlg.SetValues(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
