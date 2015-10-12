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
#include <sfx2/tabdlg.hxx>

class SfxBindings;
class SfxTabPage;
class SwChildWinWrapper;
struct SfxChildWinInfo;

class SwFieldDlg: public SfxTabDialog
{
    SwChildWinWrapper*  m_pChildWin;
    SfxBindings*        m_pBindings;
    bool            m_bHtmlMode;
    bool            m_bDataBaseMode;
    sal_uInt16          m_nDokId;
    sal_uInt16          m_nVarId;
    sal_uInt16          m_nDokInf;
    sal_uInt16          m_nRefId;
    sal_uInt16          m_nFuncId;
    sal_uInt16          m_nDbId;

    virtual bool    Close() override;
    virtual SfxItemSet* CreateInputItemSet( sal_uInt16 nId ) override;
    virtual void        Activate() override;
    virtual void        PageCreated(sal_uInt16 nId, SfxTabPage& rPage) override;

    void                ReInitTabPage( sal_uInt16 nPageId,
                                       bool bOnlyActivate = false );

public:
    SwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent);
    virtual ~SwFieldDlg();

    DECL_LINK_TYPED(OKHdl, Button*, void);
    DECL_LINK_TYPED(CancelHdl, Button*, void);

    void                Initialize(SfxChildWinInfo *pInfo);
    void                ReInitDlg();
    void                EnableInsert(bool bEnable);
    void                InsertHdl();
    void                ActivateDatabasePage();
    void                ShowReferencePage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
