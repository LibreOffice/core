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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_JAVAEDIT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_JAVAEDIT_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/button.hxx>

class SwWrtShell;
class SwFieldMgr;
class SwScriptField;

namespace sfx2 { class FileDialogHelper; }

class SwJavaEditDialog : public SvxStandardDialog
{
private:
    VclPtr<Edit>               m_pTypeED;
    VclPtr<RadioButton>        m_pUrlRB;
    VclPtr<RadioButton>        m_pEditRB;
    VclPtr<PushButton>         m_pUrlPB;
    VclPtr<Edit>               m_pUrlED;
    VclPtr<VclMultiLineEdit>   m_pEditED;

    VclPtr<OKButton>           m_pOKBtn;
    VclPtr<PushButton>         m_pPrevBtn;
    VclPtr<PushButton>         m_pNextBtn;

    OUString       aText;
    OUString       aType;

    bool                bNew;
    bool                bIsUrl;

    SwScriptField*          pField;
    SwFieldMgr*               pMgr;
    SwWrtShell*             pSh;
    sfx2::FileDialogHelper* pFileDlg;
    VclPtr<vcl::Window>     pOldDefDlgParent;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(PrevHdl, void *);
    DECL_LINK(NextHdl, void *);
    DECL_LINK(RadioButtonHdl, void *);
    DECL_LINK(InsertFileHdl, PushButton *);
    DECL_LINK(DlgClosedHdl, void *);

    virtual void    Apply() SAL_OVERRIDE;

    void            CheckTravel();
    void            SetField();

public:
    SwJavaEditDialog(vcl::Window* pParent, SwWrtShell* pWrtSh);
    virtual ~SwJavaEditDialog();
    virtual void dispose() SAL_OVERRIDE;

    OUString GetScriptText() const { return aText; }

    OUString GetScriptType() const { return aType; }

    bool IsUrl() const { return bIsUrl; }
    bool IsNew() const { return bNew; }
    bool IsUpdate() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
