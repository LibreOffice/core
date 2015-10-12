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
class SwFieldMgr;

class SwFieldEditDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;
    VclPtr<PushButton> m_pPrevBT;
    VclPtr<PushButton> m_pNextBT;
    VclPtr<PushButton> m_pAddressBT;

    DECL_LINK_TYPED(AddressHdl, Button *, void);
    DECL_LINK_TYPED(NextPrevHdl, Button *, void);

    void            Init();
    VclPtr<SfxTabPage> CreatePage(sal_uInt16 nGroup);

    void EnsureSelection(SwField *pCurField, SwFieldMgr &rMgr);
public:

    SwFieldEditDlg(SwView& rVw);
    virtual ~SwFieldEditDlg();
    virtual void dispose() override;

    DECL_LINK_TYPED(OKHdl, Button*, void);

    virtual short   Execute() override;

    void            EnableInsert(bool bEnable);
    void            InsertHdl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
