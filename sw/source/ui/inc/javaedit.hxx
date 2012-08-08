/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SW_JAVAEDIT_HXX
#define _SW_JAVAEDIT_HXX

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>

#include <svtools/svmedit.hxx>

#include <vcl/button.hxx>

#include <vcl/button.hxx>

class SwWrtShell;
class SwFldMgr;
class SwScriptField;

namespace sfx2 { class FileDialogHelper; }

// class SwJavaEditDialog -------------------------------------------------

class SwJavaEditDialog : public SvxStandardDialog
{
private:
    FixedText           aTypeFT;
    Edit                aTypeED;
    RadioButton         aUrlRB;
    RadioButton         aEditRB;
    PushButton          aUrlPB;
    Edit                aUrlED;
    MultiLineEdit       aEditED;
    FixedLine           aPostItFL;

    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    ImageButton         aPrevBtn;
    ImageButton         aNextBtn;
    HelpButton          aHelpBtn;

    rtl::OUString       aText;
    rtl::OUString       aType;

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
    DECL_LINK( InsertFileHdl, PushButton * );
    DECL_LINK(DlgClosedHdl, void *);

    virtual void    Apply();

    void            CheckTravel();
    void            SetFld();

    using Window::GetText;
    using Window::GetType;

public:
    SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh);
    ~SwJavaEditDialog();

    rtl::OUString       GetText() { return aText; }

    rtl::OUString       GetType() { return aType; }

    sal_Bool                IsUrl() { return bIsUrl; }
    sal_Bool                IsNew() { return bNew; }
    sal_Bool                IsUpdate();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
