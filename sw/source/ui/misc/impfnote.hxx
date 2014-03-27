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
#ifndef INCLUDED_SW_SOURCE_UI_MISC_IMPFNOTE_HXX
#define INCLUDED_SW_SOURCE_UI_MISC_IMPFNOTE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <numberingtypelistbox.hxx>

class SwWrtShell;

class SwEndNoteOptionPage : public SfxTabPage
{
    SwNumberingTypeListBox* m_pNumViewBox;
    FixedText* m_pOffsetLbl;
    NumericField* m_pOffsetFld;
    ListBox* m_pNumCountBox;
    Edit* m_pPrefixED;
    Edit* m_pSuffixED;
    FixedText* m_pPosFT;
    RadioButton* m_pPosPageBox;
    RadioButton* m_pPosChapterBox;

    VclContainer *m_pStylesContainer;

    ListBox* m_pParaTemplBox;
    FixedText* m_pPageTemplLbl;
    ListBox* m_pPageTemplBox;

    ListBox* m_pFtnCharAnchorTemplBox;
    ListBox* m_pFtnCharTextTemplBox;

    Edit* m_pContEdit;
    Edit* m_pContFromEdit;

    OUString aNumDoc;
    OUString aNumPage;
    OUString aNumChapter;
    SwWrtShell *pSh;
    bool    bPosDoc;
    bool    bEndNote;

    inline void SelectNumbering(int eNum);
    int GetNumbering() const;

    DECL_LINK(PosPageHdl, void *);
    DECL_LINK(PosChapterHdl, void *);
    DECL_LINK(NumCountHdl, void *);

public:
    SwEndNoteOptionPage( Window *pParent, bool bEndNote,
                         const SfxItemSet &rSet );
    ~SwEndNoteOptionPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    virtual bool FillItemSet(SfxItemSet &rSet) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet& ) SAL_OVERRIDE;

    void SetShell( SwWrtShell &rShell );
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
    SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet );
    ~SwFootNoteOptionPage();

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
