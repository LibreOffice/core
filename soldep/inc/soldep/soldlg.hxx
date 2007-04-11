/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soldlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:39:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FMRWRK_SOLDLG_HXX
#define _FMRWRK_SOLDLG_HXX

#include <vcl/wintypes.hxx>
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
