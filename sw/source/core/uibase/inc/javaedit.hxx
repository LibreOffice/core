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
#ifndef INCLUDED_SW_SOURCE_UI_INC_JAVAEDIT_HXX
#define INCLUDED_SW_SOURCE_UI_INC_JAVAEDIT_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/button.hxx>

class SwWrtShell;
class SwFldMgr;
class SwScriptField;

namespace sfx2 { class FileDialogHelper; }

// class SwJavaEditDialog -------------------------------------------------

class SwJavaEditDialog : public SvxStandardDialog
{
private:
    Edit*               m_pTypeED;
    RadioButton*        m_pUrlRB;
    RadioButton*        m_pEditRB;
    PushButton*         m_pUrlPB;
    Edit*               m_pUrlED;
    VclMultiLineEdit*   m_pEditED;

    OKButton*           m_pOKBtn;
    PushButton*         m_pPrevBtn;
    PushButton*         m_pNextBtn;

    OUString       aText;
    OUString       aType;

    sal_Bool                bNew;
    sal_Bool                bIsUrl;

    SwScriptField*          pFld;
    SwFldMgr*               pMgr;
    SwWrtShell*             pSh;
    sfx2::FileDialogHelper* pFileDlg;
    Window*                 pOldDefDlgParent;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(PrevHdl, void *);
    DECL_LINK(NextHdl, void *);
    DECL_LINK(RadioButtonHdl, void *);
    DECL_LINK(InsertFileHdl, PushButton *);
    DECL_LINK(DlgClosedHdl, void *);

    virtual void    Apply();

    void            CheckTravel();
    void            SetFld();

public:
    SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh);
    ~SwJavaEditDialog();

    OUString GetScriptText() const { return aText; }

    OUString GetScriptType() const { return aType; }

    sal_Bool IsUrl() const { return bIsUrl; }
    sal_Bool IsNew() const { return bNew; }
    bool IsUpdate() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
