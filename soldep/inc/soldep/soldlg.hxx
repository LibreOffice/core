/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
