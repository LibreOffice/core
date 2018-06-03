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
#include <vcl/weld.hxx>
#include <numberingtypelistbox.hxx>

enum SwFootnoteNum : unsigned;
class SwWrtShell;

class SwEndNoteOptionPage : public SfxTabPage
{
    OUString aNumDoc;
    OUString aNumPage;
    OUString aNumChapter;
    SwWrtShell *pSh;
    bool    bPosDoc;
    bool    bEndNote;

    std::unique_ptr<SwNumberingTypeListBox> m_xNumViewBox;
    std::unique_ptr<weld::Label> m_xOffsetLbl;
    std::unique_ptr<weld::SpinButton> m_xOffsetField;
    std::unique_ptr<weld::ComboBoxText> m_xNumCountBox;
    std::unique_ptr<weld::Entry> m_xPrefixED;
    std::unique_ptr<weld::Entry> m_xSuffixED;
    std::unique_ptr<weld::Label> m_xPosFT;
    std::unique_ptr<weld::RadioButton> m_xPosPageBox;
    std::unique_ptr<weld::RadioButton> m_xPosChapterBox;
    std::unique_ptr<weld::Widget> m_xStylesContainer;
    std::unique_ptr<weld::ComboBoxText> m_xParaTemplBox;
    std::unique_ptr<weld::Label> m_xPageTemplLbl;
    std::unique_ptr<weld::ComboBoxText> m_xPageTemplBox;
    std::unique_ptr<weld::ComboBoxText> m_xFootnoteCharAnchorTemplBox;
    std::unique_ptr<weld::ComboBoxText> m_xFootnoteCharTextTemplBox;
    std::unique_ptr<weld::Entry> m_xContEdit;
    std::unique_ptr<weld::Entry> m_xContFromEdit;

    inline void SelectNumbering(SwFootnoteNum eNum);
    SwFootnoteNum GetNumbering() const;

    DECL_LINK(PosPageHdl, weld::Button&, void);
    DECL_LINK(PosChapterHdl, weld::Button&, void);
    DECL_LINK(NumCountHdl, weld::ComboBoxText&, void);

public:
    SwEndNoteOptionPage(TabPageParent pParent, bool bEndNote, const SfxItemSet &rSet);
    virtual ~SwEndNoteOptionPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);
    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset( const SfxItemSet* ) override;

    void SetShell( SwWrtShell &rShell );
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
    friend class VclPtr<SwFootNoteOptionPage>;
    SwFootNoteOptionPage(TabPageParent pParent, const SfxItemSet &rSet );
    virtual ~SwFootNoteOptionPage() override;

public:
    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
