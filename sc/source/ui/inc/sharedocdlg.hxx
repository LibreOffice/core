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



#ifndef SC_SHAREDOCDLG_HXX
#define SC_SHAREDOCDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svx/simptabl.hxx>

class ScViewData;
class ScDocShell;


//=============================================================================
// class ScShareDocumentDlg
//=============================================================================

class ScShareDocumentDlg : public ModalDialog
{
private:
    CheckBox            maCbShare;
    FixedText           maFtWarning;
    FixedLine           maFlUsers;
    FixedText           maFtUsers;
    SvxSimpleTable      maLbUsers;
    FixedLine           maFlEnd;
    HelpButton          maBtnHelp;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;

    String              maStrTitleName;
    String              maStrTitleAccessed;
    String              maStrNoUserData;
    String              maStrUnkownUser;
    String              maStrExclusiveAccess;

    ScViewData*         mpViewData;
    ScDocShell*         mpDocShell;

    DECL_LINK( ToggleHandle, void* );

public:
                        ScShareDocumentDlg( Window* pParent, ScViewData* pViewData );
                        ~ScShareDocumentDlg();

    bool                IsShareDocumentChecked() const;
    void                UpdateView();
};

#endif
