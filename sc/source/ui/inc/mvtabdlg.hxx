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
#include <vcl/imagebtn.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
                    ScMoveTableDlg( Window* pParent );
                    ~ScMoveTableDlg();

    USHORT  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    BOOL    GetCopyTable            () const;
    BOOL    GetRenameTable          () const;
    void    GetTabNameString( String& rString ) const;
    void    SetCopyTable            (BOOL bFlag=TRUE);
    void    EnableCopyTable         (BOOL bFlag=TRUE);
    void    SetRenameTable          (BOOL bFlag=TRUE);
    void    SetTabNameVisible       (BOOL bFlag=TRUE);

private:
    FixedText       aFtDoc;
    ListBox         aLbDoc;
    FixedText       aFtTable;
    ListBox         aLbTable;
    CheckBox        aBtnCopy;
    CheckBox        aBtnRename;
    FixedText       aFtTabName;
    Edit            aEdTabName;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    USHORT          nDocument;
    SCTAB           nTable;
    BOOL            bCopyTable;
    BOOL            bRenameTable;
    //--------------------------------------
    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK( OkHdl, void * );
    DECL_LINK( SelHdl, ListBox * );
    DECL_LINK( RenameHdl, void * );
};

#include <layout/layout-post.hxx>

#endif // SC_MVTABDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
