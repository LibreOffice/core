/*************************************************************************
 *
 *  $RCSfile: logindlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:04:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef UUI_LOGINDLG_HXX
#define UUI_LOGINDLG_HXX

#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif

//============================================================================
#define LF_NO_PATH              0x0001  // "Pfad" verstecken
#define LF_NO_USERNAME          0x0002  // "Name" verstecken
#define LF_NO_PASSWORD          0x0004  // "Passwort" verstecken
#define LF_NO_SAVEPASSWORD      0x0008  // "Passwort speichern" verstecken
#define LF_NO_ERRORTEXT         0x0010  // Meldung verstecken
#define LF_PATH_READONLY        0x0020  // "Pfad" readonly
#define LF_USERNAME_READONLY    0x0040  // "Name" readonly
#define LF_NO_ACCOUNT           0x0080  // "Account" verstecken

//============================================================================
class LoginDialog : public ModalDialog
{
    FixedInfo       aErrorInfo;
    GroupBox        aErrorGB;
    FixedInfo       aRequestInfo;
    FixedText       aPathFT;
    Edit            aPathED;
    FixedInfo       aPathInfo;
    PushButton      aPathBtn;
    FixedText       aNameFT;
    Edit            aNameED;
    FixedInfo       aNameInfo;
    FixedText       aPasswordFT;
    Edit            aPasswordED;
    FixedText       aAccountFT;
    Edit            aAccountED;
    CheckBox        aSavePasswdBtn;
    GroupBox        aLoginGB;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    void            HideControls_Impl( USHORT nFlags );

    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( PathHdl_Impl, PushButton * );

public:
    LoginDialog( Window* pParent, USHORT nFlags,
                 const String& rServer, const String& rRealm,
                 ResMgr * pResMgr );

    String          GetPath() const { return aPathED.GetText(); }
    void            SetPath( const String& rNewPath )
                        { aPathED.SetText( rNewPath );
                          aPathInfo.SetText( rNewPath );}
    String          GetName() const { return aNameED.GetText(); }
    void            SetName( const String& rNewName );
    String          GetPassword() const { return aPasswordED.GetText(); }
    void            SetPassword( const String& rNew )
                        { aPasswordED.SetText( rNew ); }
    String          GetAccount() const { return aAccountED.GetText(); }
    void            SetAccount( const String& rNew )
                        { aAccountED.SetText( rNew ); }
    BOOL            IsSavePassword() const
                        { return aSavePasswdBtn.IsChecked(); }
    void            SetSavePassword( BOOL bSave )
                        { aSavePasswdBtn.Check( bSave ); }
    void            SetSavePasswordText( const String& rTxt )
                        { aSavePasswdBtn.SetText( rTxt ); }
    void            SetErrorText( const String& rTxt )
                        { aErrorInfo.SetText( rTxt ); }
    void            SetLoginRequestText( const String& rTxt )
                        { aRequestInfo.SetText( rTxt ); }
    void            ClearPassword();
    void            ClearAccount();
};

#endif // UUI_LOGINDLG_HXX

