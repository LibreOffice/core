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

#ifndef _FMRWRK_SOLDLG_HXX
#define _FMRWRK_SOLDLG_HXX

#include <tools/wintypes.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/edit.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/prgsbar.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>

// forwards
class GenericInformationList;

//
// class SolNewProjectDlg
//

class SolNewProjectDlg : public ModalDialog
{
public:
    OKButton            maOkButton;
    CancelButton        maCancelButton;
    FixedText           maFTName;
    Edit                maEName;
    FixedText           maFTShort;
    Edit                maEShort;
    FixedText           maFTDeps;
    Edit                maEDeps;

    SolNewProjectDlg( Window* pParent, const ResId& rResId );
    DECL_LINK( OkHdl, Button* pOkBtn );
    DECL_LINK( CancelHdl, Button* pCancelBtn );
};

//
// SolNewDirectoryDlg
//

class SolNewDirectoryDlg : public ModalDialog
{
public:
    OKButton            maOkButton;
    CancelButton        maCancelButton;
    FixedText           maFTName;
    Edit                maEName;
    FixedText           maFTFlag;
    Edit                maEFlag;
    FixedText           maFTDeps;
    Edit                maEDeps;
    FixedText           maFTAction;
    Edit                maEAction;
    FixedText           maFTEnv;
    Edit                maEEnv;

    SolNewDirectoryDlg( Window* pParent, const ResId& rResId );
    DECL_LINK( OkHdl, Button* pOkBtn );
    DECL_LINK( CancelHdl, Button* pCancelBtn );
};

//
// SolHelpDlg
//

class SolHelpDlg : public ModalDialog
{
public:
    OKButton            maOkButton;
    MultiLineEdit       maMLEHelp;

    SolHelpDlg( Window* pParent, const ResId& rResId );
    DECL_LINK( OkHdl, Button* pOkBtn );
};

//
// class SolSelectVersionDlg
//

class SolSelectVersionDlg : public ModalDialog
{
private:
    ListBox maVersionListBox;
    GroupBox maVersionGroupBox;
    Edit     maMinorEditBox;
    GroupBox maMinorGroupBox;
    OKButton maOKButton;
    CancelButton maCancelButton;
    ByteString msVersionMajor;
    ByteString msVersionMinor;

public:
    SolSelectVersionDlg( Window *pParent, GenericInformationList *pStandLst );
    ByteString GetVersionMajor();
    ByteString GetVersionMinor();
    DECL_LINK( DoubleClickHdl, ListBox * );
};

class SolAutoarrangeDlg : public ModelessDialog
{
private:
    GroupBox maGroupBox;
    FixedText maModuleText;
    FixedText maOverallText;
    ProgressBar maModuleBar;
    ProgressBar maOverallBar;

public:
    SolAutoarrangeDlg( Window *pParent );

    ProgressBar *GetSubBar() { return &maModuleBar; }
    ProgressBar *GetMainBar() { return &maOverallBar; }

    FixedText *GetSubText() { return &maModuleText; }
    FixedText *GetMainText() { return &maOverallText; }
};

class SolFindProjectDlg : public ModalDialog
{
private:
    ComboBox maCombobox;
    OKButton maOKButton;
    CancelButton maCancelButton;

public:
    SolFindProjectDlg( Window *pParent, ObjWinList* pObjList );
    ByteString GetProject();
};

#endif  // _FMRWRK_SOLDLG_HXX
