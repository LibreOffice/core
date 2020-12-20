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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FLDTDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FLDTDLG_HXX

#include <sal/config.h>

#include <string_view>

#include <sfx2/tabdlg.hxx>

class SfxBindings;
class SfxTabPage;
class SwChildWinWrapper;
struct SfxChildWinInfo;

class SwFieldDlg : public SfxTabDialogController
{
    SwChildWinWrapper*  m_pChildWin;
    SfxBindings*        m_pBindings;
    bool                m_bHtmlMode;
    bool                m_bDataBaseMode;
    bool                m_bClosing;

    virtual SfxItemSet* CreateInputItemSet(const OString& rId) override;
    virtual void        PageCreated(const OString& rId, SfxTabPage& rPage) override;

    void                ReInitTabPage(std::string_view rPageId,
                                      bool bOnlyActivate = false);

public:
    SwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent);
    virtual ~SwFieldDlg() override;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

    void                Initialize(SfxChildWinInfo const *pInfo);
    void                ReInitDlg();
    void                EnableInsert(bool bEnable);
    void                InsertHdl();
    void                ActivateDatabasePage();
    void                ShowReferencePage();
    virtual void        Close() override;
    virtual void        EndDialog() override;
    virtual void        Activate() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
