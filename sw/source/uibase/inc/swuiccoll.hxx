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
class SwFmt;

class SwCondCollPage : public SfxTabPage
{
    CheckBox*           m_pConditionCB;

    FixedText*          m_pContextFT;
    FixedText*          m_pUsedFT;
    SvTabListBox*       m_pTbLinks;

    FixedText*          m_pStyleFT;
    ListBox*            m_pStyleLB;
    ListBox*            m_pFilterLB;

    PushButton*         m_pRemovePB;
    PushButton*         m_pAssignPB;

    std::vector<OUString> m_aStrArr;

    SwWrtShell          &rSh;
    const CommandStruct*pCmds;
    SwFmt*              pFmt;

    bool                bNewTemplate;

    SwCondCollPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwCondCollPage();

    virtual int     DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;

    DECL_LINK( OnOffHdl, CheckBox* );
    DECL_LINK( AssignRemoveHdl, PushButton*);
    DECL_LINK( SelectHdl, ListBox* );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges();

    virtual bool FillItemSet(      SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset      (const SfxItemSet *rSet) SAL_OVERRIDE;

    void SetCollection( SwFmt* pFormat, bool bNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
