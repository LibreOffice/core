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



#ifndef UUI_NAMECLASHDLG_HXX
#define UUI_NAMECLASHDLG_HXX

#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/edit.hxx"

//============================================================================

enum NameClashResolveDialogResult { ABORT, RENAME, OVERWRITE };

class NameClashDialog : public ModalDialog
{
    FixedText     maFTMessage;
    Edit          maEDNewName;
    PushButton    maBtnOverwrite;
    PushButton    maBtnRename;
    CancelButton  maBtnCancel;
    HelpButton    maBtnHelp;
    rtl::OUString maSameName;
    rtl::OUString maNewName;

    DECL_LINK( ButtonHdl_Impl, PushButton * );

public:
    NameClashDialog( Window* pParent, ResMgr* pResMgr,
                     rtl::OUString const & rTargetFolderURL,
                     rtl::OUString const & rClashingName,
                     rtl::OUString const & rProposedNewName,
                     bool bAllowOverwrite );
    rtl::OUString getNewName() const { return maNewName; }
};

#endif // UUI_COOKIEDG_HXX

