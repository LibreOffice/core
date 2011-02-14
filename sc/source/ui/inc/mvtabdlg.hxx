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

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
                    ScMoveTableDlg( Window* pParent );
                    ~ScMoveTableDlg();

    sal_uInt16  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    sal_Bool    GetCopyTable            () const;
    void    SetCopyTable            (sal_Bool bFlag=sal_True);
    void    EnableCopyTable         (sal_Bool bFlag=sal_True);

private:
    FixedText       aFtDoc;
    ListBox         aLbDoc;
    FixedText       aFtTable;
    ListBox         aLbTable;
    CheckBox        aBtnCopy;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_uInt16          nDocument;
    SCTAB           nTable;
    sal_Bool            bCopyTable;
    //--------------------------------------
    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK( OkHdl, void * );
    DECL_LINK( SelHdl, ListBox * );
};

#include <layout/layout-post.hxx>

#endif // SC_MVTABDLG_HXX


