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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CHRDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CHRDLG_HXX

#include <sfx2/tabdlg.hxx>
#include "chrdlgmodes.hxx"
#include <svl/macitem.hxx>
#include <optional>

class SwView;

class SwCharDlg final : public SfxTabDialogController
{
    SwView&   m_rView;
    SwCharDlgMode m_nDialogMode;

public:
    SwCharDlg(weld::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
              SwCharDlgMode nDialogMode, const OUString* pFormatStr);

    virtual ~SwCharDlg() override;

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;
};

class SwCharURLPage final : public SfxTabPage
{
    std::optional<SvxMacroTableDtor> m_oINetMacroTable;
    bool                m_bModified;

    std::unique_ptr<weld::Entry> m_xURLED;
    std::unique_ptr<weld::Label> m_xTextFT;
    std::unique_ptr<weld::Entry> m_xTextED;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::ComboBox> m_xTargetFrameLB;
    std::unique_ptr<weld::Button> m_xURLPB;
    std::unique_ptr<weld::Button> m_xEventPB;
    std::unique_ptr<weld::ComboBox> m_xVisitedLB;
    std::unique_ptr<weld::ComboBox> m_xNotVisitedLB;
    std::unique_ptr<weld::Widget> m_xCharStyleContainer;

    DECL_LINK(InsertFileHdl, weld::Button&, void);
    DECL_LINK(EventHdl, weld::Button&, void);

public:
    SwCharURLPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);

    virtual ~SwCharURLPage() override;
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                     const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
