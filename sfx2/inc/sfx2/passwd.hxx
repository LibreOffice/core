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


#ifndef _SFX_PASSWD_HXX
#define _SFX_PASSWD_HXX

#include "sal/config.h"
#include <sfx2/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/app.hxx>

// defines ---------------------------------------------------------------

#define SHOWEXTRAS_NONE      ((sal_uInt16)0x0000)
#define SHOWEXTRAS_USER      ((sal_uInt16)0x0001)
#define SHOWEXTRAS_CONFIRM   ((sal_uInt16)0x0002)
#define SHOWEXTRAS_PASSWORD2 ((sal_uInt16)0x0004)
#define SHOWEXTRAS_CONFIRM2  ((sal_uInt16)0x0008)
#define SHOWEXTRAS_ALL       ((sal_uInt16)(SHOWEXTRAS_USER | SHOWEXTRAS_CONFIRM))

// class SfxPasswordDialog -----------------------------------------------

class SFX2_DLLPUBLIC SfxPasswordDialog : public ModalDialog
{
private:
    FixedLine       maPasswordBox;
    FixedText       maUserFT;
    Edit            maUserED;
    FixedText       maPasswordFT;
    Edit            maPasswordED;
    FixedText       maConfirmFT;
    Edit            maConfirmED;
    FixedLine       maPassword2Box;
    FixedText       maPassword2FT;
    Edit            maPassword2ED;
    FixedText       maConfirm2FT;
    Edit            maConfirm2ED;

    OKButton        maOKBtn;
    CancelButton    maCancelBtn;
    HelpButton      maHelpBtn;

    sal_uInt16          mnMinLen;
    sal_uInt16          mnExtras;

    bool            mbAsciiOnly;
    DECL_DLLPRIVATE_LINK( EditModifyHdl, Edit* );
    DECL_DLLPRIVATE_LINK( OKHdl, OKButton* );

public:
    SfxPasswordDialog( Window* pParent, const String* pGroupText = NULL );

    String          GetUser() const { return maUserED.GetText(); }
    String          GetPassword() const { return maPasswordED.GetText(); }
    String          GetConfirm() const { return maConfirmED.GetText(); }

    String          GetPassword2() const { return maPassword2ED.GetText(); }
    String          GetConfirm2() const { return maConfirm2ED.GetText(); }
    void            SetGroup2Text( const String& i_rText ) { maPassword2Box.SetText( i_rText ); }

    void            SetMinLen( sal_uInt16 Len );
    void            SetMaxLen( sal_uInt16 Len );
    void            SetEditHelpId( const rtl::OString& rId ) { maPasswordED.SetHelpId( rId ); }
    void            ShowExtras( sal_uInt16 nExtras ) { mnExtras = nExtras; }
    void            AllowAsciiOnly( bool i_bAsciiOnly = true ) { mbAsciiOnly = i_bAsciiOnly; }

    virtual short   Execute();
};

#endif // #ifndef _SFX_PASSWD_HXX

