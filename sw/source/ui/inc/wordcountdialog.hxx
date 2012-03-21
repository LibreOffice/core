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
#ifndef SW_WORDCOUNTDIALOG_HXX
#define SW_WORDCOUNTDIALOG_HXX
#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/button.hxx>
struct SwDocStat;
#include <sfx2/childwin.hxx>
#include "swabstdlg.hxx"

class SwWordCountDialog
{
    VclVBox vbox;
    VclVBox content_area;

    VclHBox aCurrentSelection;
    FixedText aCurrentSelectionText;
    FixedLine aCurrentSelectionLine;

    VclVBox aSelectionBox;
    VclHBox aSelectionRow1;
    FixedText aCurrentWordFT;
    FixedInfo aCurrentWordFI;
    VclHBox aSelectionRow2;
    FixedText aCurrentCharacterFT;
    FixedInfo aCurrentCharacterFI;
    VclHBox aSelectionRow3;
    FixedText aCurrentCharacterExcludingSpacesFT;
    FixedInfo aCurrentCharacterExcludingSpacesFI;

    VclHBox aDoc;
    FixedText aDocText;
    FixedLine aDocLine;

    VclVBox aDocBox;
    VclHBox aDocRow1;
    FixedText aDocWordFT;
    FixedInfo aDocWordFI;
    VclHBox aDocRow2;
    FixedText aDocCharacterFT;
    FixedInfo aDocCharacterFI;
    VclHBox aDocRow3;
    FixedText aDocCharacterExcludingSpacesFT;
    FixedInfo aDocCharacterExcludingSpacesFI;

    FixedLine aBottomFL;

    VclHButtonBox action_area;
    OKButton aOK;
    HelpButton aHelp;

    void InitControls();

public:
    SwWordCountDialog(Dialog* pParent);
    ~SwWordCountDialog();

    void    SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc);

    SW_DLLPRIVATE DECL_LINK( OkHdl,     void* );
};

class SwWordCountFloatDlg : public SfxModelessDialog
{
    SwWordCountDialog   aDlg;
    virtual void    Activate();
    public:
    SwWordCountFloatDlg(     SfxBindings* pBindings,
                             SfxChildWindow* pChild,
                             Window *pParent,
                             SfxChildWinInfo* pInfo);
    void    UpdateCounts();

    void    SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat);
};

class SwWordCountWrapper : public SfxChildWindow
{
    AbstractSwWordCountFloatDlg*   pAbstDlg;
protected:
    SwWordCountWrapper(    Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW_WITHID(SwWordCountWrapper);

public:
    void    UpdateCounts();
    void    SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
