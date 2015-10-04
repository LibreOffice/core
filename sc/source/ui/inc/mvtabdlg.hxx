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

#ifndef INCLUDED_SC_SOURCE_UI_INC_MVTABDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_MVTABDLG_HXX

#include "address.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

class ScMoveTableDlg : public ModalDialog
{
public:
    ScMoveTableDlg(vcl::Window* pParent, const OUString& rDefault);
    virtual ~ScMoveTableDlg();
    virtual void dispose() SAL_OVERRIDE;

    sal_uInt16  GetSelectedDocument     () const { return nDocument; }
    SCTAB   GetSelectedTable        () const { return nTable; }
    bool    GetCopyTable            () const { return bCopyTable; }
    bool    GetRenameTable          () const { return bRenameTable; }
    void    GetTabNameString( OUString& rString ) const;
    void    SetForceCopyTable       ();
    void    EnableRenameTable       (bool bFlag=true);

private:
    void ResetRenameInput();
    void CheckNewTabName();
    ScDocument* GetSelectedDoc();
    bool IsCurrentDocSelected() const;

private:
    VclPtr<RadioButton>     pBtnMove;
    VclPtr<RadioButton>     pBtnCopy;
    VclPtr<ListBox>         pLbDoc;
    VclPtr<ListBox>         pLbTable;
    VclPtr<Edit>            pEdTabName;
    VclPtr<FixedText>       pFtWarn;
    VclPtr<OKButton>        pBtnOk;

    OUString   msCurrentDoc;
    OUString   msNewDoc;

    OUString   msStrTabNameUsed;
    OUString   msStrTabNameEmpty;
    OUString   msStrTabNameInvalid;

    const OUString maDefaultName;

    sal_uInt16      mnCurrentDocPos;
    sal_uInt16      nDocument;
    SCTAB           nTable;
    bool            bCopyTable:1;
    bool            bRenameTable:1;
    bool            mbEverEdited:1;

    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK_TYPED( OkHdl, Button*, void );
    DECL_LINK_TYPED( SelHdl, ListBox&, void );
    DECL_LINK_TYPED( CheckBtnHdl, RadioButton&, void );
    DECL_LINK( CheckNameHdl, Edit * );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_MVTABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
