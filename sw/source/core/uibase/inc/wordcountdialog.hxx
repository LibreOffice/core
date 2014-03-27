/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_INC_WORDCOUNTDIALOG_HXX
#define INCLUDED_SW_SOURCE_UI_INC_WORDCOUNTDIALOG_HXX
#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/layout.hxx>
#include <vcl/button.hxx>
struct SwDocStat;
#include <sfx2/childwin.hxx>
#include "swabstdlg.hxx"

class SwWordCountFloatDlg : public SfxModelessDialog
{
    virtual void    Activate() SAL_OVERRIDE;
    void SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc);
    void showCJK(bool bShowCJK);

    FixedText* m_pCurrentWordFT;
    FixedText* m_pCurrentCharacterFT;
    FixedText* m_pCurrentCharacterExcludingSpacesFT;
    FixedText* m_pCurrentCjkcharsFT;

    FixedText* m_pDocWordFT;
    FixedText* m_pDocCharacterFT;
    FixedText* m_pDocCharacterExcludingSpacesFT;
    FixedText* m_pDocCjkcharsFT;

    FixedText* m_pCjkcharsLabelFT;

    CloseButton* m_pClosePB;

    DECL_LINK( CloseHdl,     void* );
public:
    SwWordCountFloatDlg(     SfxBindings* pBindings,
                             SfxChildWindow* pChild,
                             Window *pParent,
                             SfxChildWinInfo* pInfo);
    ~SwWordCountFloatDlg();
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
