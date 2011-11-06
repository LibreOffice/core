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



#ifndef SC_UI_PROTECTION_DLG_HXX
#define SC_UI_PROTECTION_DLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svx/checklbx.hxx>

class Window;
class ScTableProtection;

class ScTableProtectionDlg : public ModalDialog
{
public:
    explicit ScTableProtectionDlg(Window* pParent);
    virtual ~ScTableProtectionDlg();

    virtual short Execute();

    void SetDialogData(const ScTableProtection& rData);

    void WriteData(ScTableProtection& rData) const;

private:
    ScTableProtectionDlg(); // disabled

    void Init();

    void EnableOptionalWidgets(bool bEnable = true);

    CheckBox        maBtnProtect;

    FixedText       maPassword1Text;
    Edit            maPassword1Edit;
    FixedText       maPassword2Text;
    Edit            maPassword2Edit;

    FixedLine       maOptionsLine;
    FixedText       maOptionsText;
    SvxCheckListBox maOptionsListBox;

    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    String          maSelectLockedCells;
    String          maSelectUnlockedCells;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( CheckBoxHdl, CheckBox* );
    DECL_LINK( PasswordModifyHdl, Edit* );
};

#endif
