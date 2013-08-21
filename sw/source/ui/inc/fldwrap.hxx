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
#ifndef _FLDWRAP_HXX
#define _FLDWRAP_HXX

#include "chldwrap.hxx"
class AbstractSwFldDlg;

class SwFldDlgWrapper : public SwChildWinWrapper
{
public:
    AbstractSwFldDlg * pDlgInterface;
    SwFldDlgWrapper( Window* pParent, sal_uInt16 nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW_WITHID(SwFldDlgWrapper);

    virtual sal_Bool    ReInitDlg(SwDocShell *pDocSh);
    void ShowReferencePage();
};

/* --------------------------------------------------
 * field dialog only showing database page to support
 * mail merge
 * --------------------------------------------------*/
class SwFldDataOnlyDlgWrapper : public SwChildWinWrapper
{
public:
    AbstractSwFldDlg * pDlgInterface;
    SwFldDataOnlyDlgWrapper( Window* pParent, sal_uInt16 nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwFldDataOnlyDlgWrapper);

    virtual sal_Bool    ReInitDlg(SwDocShell *pDocSh);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
