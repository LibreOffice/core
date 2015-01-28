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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FLDEDT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FLDEDT_HXX
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>

class SwView;
class SwWrtShell;
class SwFldMgr;

class SwFldEditDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;
    PushButton* m_pPrevBT;
    PushButton* m_pNextBT;
    PushButton* m_pAddressBT;

    DECL_LINK(AddressHdl, void *);
    DECL_LINK(NextPrevHdl, Button *pBt = 0);

    void            Init();
    SfxTabPage*     CreatePage(sal_uInt16 nGroup);

    void EnsureSelection(SwField *pCurFld, SwFldMgr &rMgr);
public:

    SwFldEditDlg(SwView& rVw);
    virtual ~SwFldEditDlg();

    DECL_LINK(OKHdl, void *);

    virtual short   Execute() SAL_OVERRIDE;

    void            EnableInsert(bool bEnable);
    void            InsertHdl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
