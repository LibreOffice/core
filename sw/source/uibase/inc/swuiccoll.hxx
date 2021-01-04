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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWUICCOLL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWUICCOLL_HXX

#include <sfx2/tabdlg.hxx>

#include <ccoll.hxx>

class SwWrtShell;
class SwFormat;

class SwCondCollPage : public SfxTabPage
{
    std::vector<OUString> m_aStrArr;

    SwWrtShell          &m_rSh;
    const CommandStruct*m_pCmds;
    SwFormat*              m_pFormat;

    std::unique_ptr<weld::TreeView> m_xTbLinks;
    std::unique_ptr<weld::TreeView> m_xStyleLB;
    std::unique_ptr<weld::ComboBox> m_xFilterLB;
    std::unique_ptr<weld::Button> m_xRemovePB;
    std::unique_ptr<weld::Button> m_xAssignPB;

    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    DECL_LINK(OnOffHdl, weld::ToggleButton&, void);
    DECL_LINK(AssignRemoveTreeListBoxHdl, weld::TreeView&, bool);
    DECL_LINK(AssignRemoveClickHdl, weld::Button&, void);
    DECL_LINK(SelectTreeListBoxHdl, weld::TreeView&, void);
    DECL_LINK(SelectListBoxHdl, weld::ComboBox&, void);
    void AssignRemove(const weld::Widget*);
    void SelectHdl(const weld::Widget*);

    static const sal_uInt16 m_aPageRg[];

public:
    SwCondCollPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwCondCollPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return m_aPageRg; }

    virtual bool FillItemSet(      SfxItemSet *rSet) override;
    virtual void Reset      (const SfxItemSet *rSet) override;

    void SetCollection( SwFormat* pFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
