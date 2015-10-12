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
    VclPtr<SwNumberingTypeListBox> m_pNumViewBox;
    VclPtr<FixedText> m_pOffsetLbl;
    VclPtr<NumericField> m_pOffsetField;
    VclPtr<ListBox> m_pNumCountBox;
    VclPtr<Edit> m_pPrefixED;
    VclPtr<Edit> m_pSuffixED;
    VclPtr<FixedText> m_pPosFT;
    VclPtr<RadioButton> m_pPosPageBox;
    VclPtr<RadioButton> m_pPosChapterBox;

    VclPtr<VclContainer> m_pStylesContainer;

    VclPtr<ListBox> m_pParaTemplBox;
    VclPtr<FixedText> m_pPageTemplLbl;
    VclPtr<ListBox> m_pPageTemplBox;

    VclPtr<ListBox> m_pFootnoteCharAnchorTemplBox;
    VclPtr<ListBox> m_pFootnoteCharTextTemplBox;

    VclPtr<Edit> m_pContEdit;
    VclPtr<Edit> m_pContFromEdit;

    OUString aNumDoc;
    OUString aNumPage;
    OUString aNumChapter;
    SwWrtShell *pSh;
    bool    bPosDoc;
    bool    bEndNote;

    inline void SelectNumbering(int eNum);
    int GetNumbering() const;

    DECL_LINK_TYPED(PosPageHdl, Button*, void);
    DECL_LINK_TYPED(PosChapterHdl, Button*, void);
    DECL_LINK_TYPED(NumCountHdl, ListBox&, void);

public:
    SwEndNoteOptionPage( vcl::Window *pParent, bool bEndNote,
                         const SfxItemSet &rSet );
    virtual ~SwEndNoteOptionPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset( const SfxItemSet* ) override;

    void SetShell( SwWrtShell &rShell );
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
    friend class VclPtr<SwFootNoteOptionPage>;
    SwFootNoteOptionPage( vcl::Window *pParent, const SfxItemSet &rSet );
    virtual ~SwFootNoteOptionPage();

public:
    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
