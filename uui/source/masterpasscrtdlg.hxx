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



#ifndef UUI_MASTERPASSCRTDLG_HXX
#define UUI_MASTERPASSCRTDLG_HXX

#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP
#include <com/sun/star/task/PasswordRequestMode.hpp>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>

//============================================================================
class MasterPasswordCreateDialog : public ModalDialog
{
private:
    FixedText       aFTInfoText;
    FixedLine       aFLInfoText;

    FixedText       aFTMasterPasswordCrt;
    Edit            aEDMasterPasswordCrt;
    FixedText       aFTMasterPasswordRepeat;
    Edit            aEDMasterPasswordRepeat;

    FixedText       aFTCautionText;
    FixedLine       aFLCautionText;

    FixedText       aFTMasterPasswordWarning;
    FixedLine       aFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;


    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( EditHdl_Impl, Edit * );

public:
    MasterPasswordCreateDialog( Window* pParent, ResMgr * pResMgr );

    String          GetMasterPassword() const { return aEDMasterPasswordCrt.GetText(); }

private:
    ResMgr*                                         pResourceMgr;
    sal_uInt16                                      nMinLen;

    void            CalculateTextHeight();
};

#endif // UUI_MASTERPASSCRTDLG_HXX
