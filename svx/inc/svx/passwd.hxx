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


#ifndef _SVX_PASSWD_HXX
#define _SVX_PASSWD_HXX

// include ---------------------------------------------------------------

#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include "svx/svxdllapi.h"

// class SvxPasswordDialog -----------------------------------------------

class SVX_DLLPUBLIC SvxPasswordDialog : public SfxModalDialog
{
private:
    FixedLine       aOldFL;
    FixedText       aOldPasswdFT;
    Edit            aOldPasswdED;
    FixedLine       aNewFL;
    FixedText       aNewPasswdFT;
    Edit            aNewPasswdED;
    FixedText       aRepeatPasswdFT;
    Edit            aRepeatPasswdED;
    OKButton        aOKBtn;
    CancelButton    aEscBtn;
    HelpButton      aHelpBtn;

    String          aOldPasswdErrStr;
    String          aRepeatPasswdErrStr;

    Link            aCheckPasswordHdl;

    sal_Bool            bEmpty;

    DECL_LINK( ButtonHdl, OKButton * );
    DECL_LINK( EditModifyHdl, Edit * );

public:
                    SvxPasswordDialog( Window* pParent, sal_Bool bAllowEmptyPasswords = sal_False, sal_Bool bDisableOldPassword = sal_False );
                    ~SvxPasswordDialog();

    String          GetOldPassword() const { return aOldPasswdED.GetText(); }
    String          GetNewPassword() const { return aNewPasswdED.GetText(); }

    void            SetCheckPasswordHdl( const Link& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

