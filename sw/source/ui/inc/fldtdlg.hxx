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
#ifndef _SWFLDTDLG_HXX
#define _SWFLDTDLG_HXX
#include <sfx2/tabdlg.hxx>

class SfxBindings;
class SfxTabPage;
class SwChildWinWrapper;
struct SfxChildWinInfo;

class SwFldDlg: public SfxTabDialog
{
    SwChildWinWrapper*  m_pChildWin;
    SfxBindings*        m_pBindings;
    sal_Bool            m_bHtmlMode;
    sal_Bool            m_bDataBaseMode;
    sal_uInt16          m_nDokId;
    sal_uInt16          m_nVarId;
    sal_uInt16          m_nDokInf;
    sal_uInt16          m_nRefId;
    sal_uInt16          m_nFuncId;
    sal_uInt16          m_nDbId;

    virtual sal_Bool    Close();
    virtual SfxItemSet* CreateInputItemSet( sal_uInt16 nId );
    virtual void        Activate();
    virtual void        PageCreated(sal_uInt16 nId, SfxTabPage& rPage);

    void                ReInitTabPage( sal_uInt16 nPageId,
                                        sal_Bool bOnlyActivate = sal_False );

public:
    SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent);
    virtual ~SwFldDlg();

    DECL_LINK(OKHdl, void *);
    DECL_LINK(CancelHdl, void *);

    void                Initialize(SfxChildWinInfo *pInfo);
    void                ReInitDlg();
    void                EnableInsert(sal_Bool bEnable);
    void                InsertHdl();
    void                ActivateDatabasePage();
    void                ShowReferencePage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
