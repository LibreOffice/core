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
const int nWCSpacing = 50;

SwWordCountDialog::SwWordCountDialog(Dialog* pParent)
    : dialog_vbox1(pParent)
    , box1(&dialog_vbox1)
    , aCurrentSelection(&box1, false, 3)
    , aCurrentSelectionText(&aCurrentSelection, SW_RES(FT_CURRENT))
    , aCurrentSelectionLine(&aCurrentSelection, SW_RES(FL_CURRENT))
    , aSelectionBox(&box1, false, 7)
    , aSelectionRow1(&aSelectionBox, false, nWCSpacing)
    , aCurrentWordFT(&aSelectionRow1, SW_RES(FT_CURRENTWORD))
    , aCurrentWordFI(&aSelectionRow1, SW_RES(FI_CURRENTWORD))
    , aSelectionRow2(&aSelectionBox, false, nWCSpacing)
    , aCurrentCharacterFT(&aSelectionRow2, SW_RES(FT_CURRENTCHARACTER))
    , aCurrentCharacterFI(&aSelectionRow2, SW_RES(FI_CURRENTCHARACTER))
    , aSelectionRow3(&aSelectionBox, false, nWCSpacing)
    , aCurrentCharacterExcludingSpacesFT(&aSelectionRow3, SW_RES(FT_CURRENTCHARACTEREXCLUDINGSPACES))
    , aCurrentCharacterExcludingSpacesFI(&aSelectionRow3, SW_RES(FI_CURRENTCHARACTEREXCLUDINGSPACES))
    , aDoc(&box1, false, 3)
    , aDocText(&aDoc, SW_RES(FT_DOC))
    , aDocLine(&aDoc, SW_RES(FL_DOC))
    , aDocBox(&box1, false, 7)
    , aDocRow1(&aDocBox, false, nWCSpacing)
    , aDocWordFT(&aDocRow1, SW_RES(FT_DOCWORD))
    , aDocWordFI(&aDocRow1, SW_RES(FI_DOCWORD))
    , aDocRow2(&aDocBox, false, nWCSpacing)
    , aDocCharacterFT(&aDocRow2, SW_RES(FT_DOCCHARACTER))
    , aDocCharacterFI(&aDocRow2, SW_RES(FI_DOCCHARACTER))
    , aDocRow3(&aDocBox, false, nWCSpacing)
    , aDocCharacterExcludingSpacesFT(&aDocRow3, SW_RES(FT_DOCCHARACTEREXCLUDINGSPACES))
    , aDocCharacterExcludingSpacesFI(&aDocRow3, SW_RES(FI_DOCCHARACTEREXCLUDINGSPACES))
    , aBottomFL(&dialog_vbox1, SW_RES(FL_BOTTOM))
    , dialog_action_area1(&dialog_vbox1)
    , aOK(&dialog_action_area1, SW_RES(PB_OK))
    , aHelp(&dialog_action_area1, SW_RES(PB_HELP))
{
    Size aSize;

    rtl::OString sFill(RTL_CONSTASCII_STRINGPARAM("fill"));
    rtl::OString sExpand(RTL_CONSTASCII_STRINGPARAM("expand"));
    rtl::OString sPackType(RTL_CONSTASCII_STRINGPARAM("pack-type"));
    rtl::OString sBorderWidth(RTL_CONSTASCII_STRINGPARAM("border-width"));

    dialog_vbox1.setChildProperty(sFill, true);
    //TO-DO, when vbox1 belongs to dialog via builder, this becomes
    //content-area-border on the dialog
    dialog_vbox1.setChildProperty(sBorderWidth, sal_Int32(7));

    dialog_action_area1.setChildProperty(sFill, true);

    box1.setChildProperty(sFill, true);
    box1.setChildProperty(sExpand, true);

    aCurrentSelection.setChildProperty(sFill, true);
    aCurrentSelectionText.setChildProperty(sFill, true);
    aCurrentSelectionLine.setChildProperty(sFill, true);
    aCurrentSelectionLine.setChildProperty(sExpand, true);

    aSelectionBox.setChildProperty(sFill, true);
    aSelectionBox.setChildProperty(sBorderWidth, sal_Int32(7));

    aSelectionRow1.setChildProperty(sFill, true);
    aSelectionRow1.setChildProperty(sExpand, true);
    aCurrentWordFT.setChildProperty(sFill, true);
    aCurrentWordFI.setChildProperty(sFill, true);
    aCurrentWordFI.setChildProperty(sExpand, true);

    aSelectionRow2.setChildProperty(sFill, true);
    aSelectionRow2.setChildProperty(sExpand, true);
    aCurrentCharacterFT.setChildProperty(sFill, true);
    aCurrentCharacterFI.setChildProperty(sFill, true);
    aCurrentCharacterFI.setChildProperty(sExpand, true);
    aSelectionRow3.setChildProperty(sFill, true);
    aSelectionRow3.setChildProperty(sExpand, true);
    aCurrentCharacterExcludingSpacesFT.setChildProperty(sFill, true);
    aCurrentCharacterExcludingSpacesFI.setChildProperty(sFill, true);
    aCurrentCharacterExcludingSpacesFI.setChildProperty(sExpand, true);

    aDoc.setChildProperty(sFill, true);
    aDocText.setChildProperty(sFill, true);
    aDocLine.setChildProperty(sFill, true);
    aDocLine.setChildProperty(sExpand, true);

    aDocBox.setChildProperty(sFill, true);
    aDocBox.setChildProperty(sBorderWidth, sal_Int32(7));

    aDocRow1.setChildProperty(sFill, true);
    aDocRow1.setChildProperty(sExpand, true);
    aDocWordFT.setChildProperty(sFill, true);
    aDocWordFI.setChildProperty(sFill, true);
    aDocWordFI.setChildProperty(sExpand, true);

    aDocRow2.setChildProperty(sFill, true);
    aDocRow2.setChildProperty(sExpand, true);
    aDocCharacterFT.setChildProperty(sFill, true);
    aDocCharacterFI.setChildProperty(sFill, true);
    aDocCharacterFI.setChildProperty(sExpand, true);
    aDocRow3.setChildProperty(sFill, true);
    aDocRow3.setChildProperty(sExpand, true);
    aDocCharacterExcludingSpacesFT.setChildProperty(sFill, true);
    aDocCharacterExcludingSpacesFI.setChildProperty(sFill, true);
    aDocCharacterExcludingSpacesFI.setChildProperty(sExpand, true);

    aBottomFL.setChildProperty(sFill, true);
    aBottomFL.setChildProperty(sFill, true);

    aSize = dialog_vbox1.GetOptimalSize(WINDOWSIZE_PREFERRED);
    dialog_vbox1.SetSizePixel(aSize);

    aSize = pParent->GetOptimalSize(WINDOWSIZE_PREFERRED);
    pParent->SetSizePixel(aSize);
    fprintf(stderr, "size is %ld %ld on %p\n", aSize.Width(), aSize.Height(), pParent);

    aSize = dialog_vbox1.GetOptimalSize(WINDOWSIZE_PREFERRED);
    dialog_vbox1.SetSizePixel(aSize);

    aOK.setChildProperty<sal_Int32>(sPackType, VCL_PACK_END);
    aHelp.setChildProperty<sal_Int32>(sPackType, VCL_PACK_END);

    aOK.SetClickHdl(LINK(this, SwWordCountDialog, OkHdl));

    fprintf(stderr, "aOk is %p\n", &aOK);
    fprintf(stderr, "aHelp is %p\n", &aHelp);
    fprintf(stderr, "dialog_action_area1 is is %p\n", &dialog_action_area1);
    fprintf(stderr, "aCurrentSelectionLine is is %p\n", &aCurrentSelectionLine);
    fprintf(stderr, "aCurrentSelectionText is is %p\n", &aCurrentSelectionText);
    fprintf(stderr, "aCurrentSelection is is %p\n", &aCurrentSelection);
    fprintf(stderr, "dialog_vbox1 is is %p\n", &dialog_vbox1);

    pParent->SetMinOutputSizePixel(dialog_vbox1.GetOptimalSize(WINDOWSIZE_PREFERRED));
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
