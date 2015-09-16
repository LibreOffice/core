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

#include <tools/resary.hxx>

class SwWrtShell;
class SwFormat;

class SwCondCollPage : public SfxTabPage
{
    VclPtr<CheckBox>           m_pConditionCB;

    VclPtr<FixedText>          m_pContextFT;
    VclPtr<FixedText>          m_pUsedFT;
    VclPtr<SvTabListBox>       m_pTbLinks;

    VclPtr<FixedText>          m_pStyleFT;
    VclPtr<ListBox>            m_pStyleLB;
    VclPtr<ListBox>            m_pFilterLB;

    VclPtr<PushButton>         m_pRemovePB;
    VclPtr<PushButton>         m_pAssignPB;

    std::vector<OUString> m_aStrArr;

    SwWrtShell          &m_rSh;
    const CommandStruct*m_pCmds;
    SwFormat*              m_pFormat;

    bool                m_bNewTemplate;

    virtual ~SwCondCollPage();
    virtual void dispose() SAL_OVERRIDE;

    virtual sfxpg   DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;

    DECL_LINK_TYPED( OnOffHdl, Button*, void );
    DECL_LINK_TYPED( AssignRemoveHdl, ListBox&, void);
    DECL_LINK_TYPED( AssignRemoveTreeListBoxHdl, SvTreeListBox*, bool);
    DECL_LINK_TYPED( AssignRemoveClickHdl, Button*, void);
    DECL_LINK_TYPED( SelectTreeListBoxHdl, SvTreeListBox*, void );
    DECL_LINK( SelectHdl, void* );
    void AssignRemove(void*);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    static const sal_uInt16 m_aPageRg[];

public:
    SwCondCollPage(vcl::Window *pParent, const SfxItemSet &rSet);

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return m_aPageRg; }

    virtual bool FillItemSet(      SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset      (const SfxItemSet *rSet) SAL_OVERRIDE;

    void SetCollection( SwFormat* pFormat, bool bNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
