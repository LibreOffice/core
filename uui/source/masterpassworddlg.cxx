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



#include <svtools/filedlg.hxx>
#include <vcl/msgbox.hxx>

#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif
#ifndef UUI_MASTERPASSWORDDLG_HRC
#include <masterpassworddlg.hrc>
#endif
#include <masterpassworddlg.hxx>

// MasterPasswordDialog---------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK( MasterPasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

MasterPasswordDialog::MasterPasswordDialog
(
    Window*                                     pParent,
    ::com::sun::star::task::PasswordRequestMode aDialogMode,
    ResMgr*                                     pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_MASTERPASSWORD, *pResMgr ) ),

    aFTMasterPassword       ( this, ResId( FT_MASTERPASSWORD, *pResMgr ) ),
    aEDMasterPassword       ( this, ResId( ED_MASTERPASSWORD, *pResMgr ) ),
    aFL ( this, ResId( FL_FIXED_LINE, *pResMgr ) ),
    aOKBtn                  ( this, ResId( BTN_MASTERPASSWORD_OK, *pResMgr ) ),
    aCancelBtn              ( this, ResId( BTN_MASTERPASSWORD_CANCEL, *pResMgr ) ),
    aHelpBtn                ( this, ResId( BTN_MASTERPASSWORD_HELP, *pResMgr ) ),
    nDialogMode             ( aDialogMode ),
    pResourceMgr            ( pResMgr )
{
    if( nDialogMode == ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        String aErrorMsg( ResId( STR_ERROR_MASTERPASSWORD_WRONG, *pResourceMgr ));
        ErrorBox aErrorBox( pParent, WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, MasterPasswordDialog, OKHdl_Impl ) );
};

