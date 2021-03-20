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
    SwFootNoteOptionDlg& m_rParent;
    OUString aNumDoc;
    OUString aNumPage;
    OUString aNumChapter;
    OUString m_sCharAnchor;
    OUString m_sCharText;
    SwWrtShell *pSh;
    bool    bPosDoc;
    bool    bEndNote;

    std::unique_ptr<SwNumberingTypeListBox> m_xNumViewBox;
    std::unique_ptr<weld::Label> m_xOffsetLbl;
    std::unique_ptr<weld::SpinButton> m_xOffsetField;
    std::unique_ptr<weld::ComboBox> m_xNumCountBox;
    std::unique_ptr<weld::Entry> m_xPrefixED;
    std::unique_ptr<weld::Entry> m_xSuffixED;
    std::unique_ptr<weld::Label> m_xPosFT;
    std::unique_ptr<weld::RadioButton> m_xPosPageBox;
    std::unique_ptr<weld::RadioButton> m_xPosChapterBox;
    std::unique_ptr<weld::Widget> m_xStylesContainer;
    std::unique_ptr<weld::ComboBox> m_xParaTemplBox;
    std::unique_ptr<weld::Label> m_xPageTemplLbl;
    std::unique_ptr<weld::ComboBox> m_xPageTemplBox;
    std::unique_ptr<weld::ComboBox> m_xFootnoteCharAnchorTemplBox;
    std::unique_ptr<weld::ComboBox> m_xFootnoteCharTextTemplBox;
    std::unique_ptr<weld::Entry> m_xContEdit;
    std::unique_ptr<weld::Entry> m_xContFromEdit;

    inline void SelectNumbering(SwFootnoteNum eNum);
    SwFootnoteNum GetNumbering() const;

    DECL_LINK(PosPageHdl, weld::Button&, void);
    DECL_LINK(PosChapterHdl, weld::Button&, void);
    DECL_LINK(NumCountHdl, weld::ComboBox&, void);
    DECL_LINK(CharAnchorHdl, weld::ComboBox&, void);
    DECL_LINK(CharTextHdl, weld::ComboBox&, void);

public:
    SwEndNoteOptionPage(weld::Container* pPage, weld::DialogController* pController, bool bEndNote, const SfxItemSet &rSet);
    virtual ~SwEndNoteOptionPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset( const SfxItemSet* ) override;

    void SetShell( SwWrtShell &rShell );
    void UpdateCharStyleList(bool bEndNoteId, const OUString sAdd, const OUString sDel, std::optional<bool> bAnchor = std::nullopt);
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
public:
    SwFootNoteOptionPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet );
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    virtual ~SwFootNoteOptionPage() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
