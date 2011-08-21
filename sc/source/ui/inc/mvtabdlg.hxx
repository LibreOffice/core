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

#ifndef SC_MVTABDLG_HXX
#define SC_MVTABDLG_HXX


#include "address.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
                    ScMoveTableDlg( Window* pParent, const String& rDefault );
                    ~ScMoveTableDlg();

    sal_uInt16  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    bool    GetCopyTable            () const;
    bool    GetRenameTable          () const;
    void    GetTabNameString( String& rString ) const;
    void    SetForceCopyTable       ();
    void    EnableCopyTable         (sal_Bool bFlag=true);
    void    EnableRenameTable       (sal_Bool bFlag=true);

private:
    void ResetRenameInput();
    void CheckNewTabName();
    ScDocument* GetSelectedDoc();

private:
    FixedLine       aFlAction;
    RadioButton     aBtnMove;
    RadioButton     aBtnCopy;
    FixedLine       aFlLocation;
    FixedText       aFtDoc;
    ListBox         aLbDoc;
    FixedText       aFtTable;
    ListBox         aLbTable;
    FixedLine       aFlName;
    FixedText       aFtTabName;
    Edit            aEdTabName;
    FixedText       aFtWarn;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    String          maStrTabNameUsed;
    String          maStrTabNameEmpty;
    String          maStrTabNameInvalid;

    const String&   mrDefaultName;

    sal_uInt16          nDocument;
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

#include <layout/layout-post.hxx>

#endif // SC_MVTABDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
