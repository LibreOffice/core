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

#ifndef SC_MVTABDLG_HXX
#define SC_MVTABDLG_HXX


#include "address.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
    ScMoveTableDlg(Window* pParent, const OUString& rDefault);
    ~ScMoveTableDlg();

    sal_uInt16  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    bool    GetCopyTable            () const;
    bool    GetRenameTable          () const;
    void    GetTabNameString( OUString& rString ) const;
    void    SetForceCopyTable       ();
    void    EnableCopyTable         (sal_Bool bFlag=true);
    void    EnableRenameTable       (sal_Bool bFlag=true);

private:
    void ResetRenameInput();
    void CheckNewTabName();
    ScDocument* GetSelectedDoc();
    bool IsCurrentDocSelected() const;

private:
    RadioButton*     pBtnMove;
    RadioButton*     pBtnCopy;
    ListBox*         pLbDoc;
    ListBox*         pLbTable;
    Edit*            pEdTabName;
    FixedText*       pFtWarn;
    OKButton*        pBtnOk;

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

    //--------------------------------------
    void    Init            ();
    void    InitBtnRename   ();
    void    InitDocListBox  ();
    DECL_LINK( OkHdl, void * );
    DECL_LINK( SelHdl, ListBox * );
    DECL_LINK( CheckBtnHdl, void * );
    DECL_LINK( CheckNameHdl, Edit * );
};

#endif // SC_MVTABDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
