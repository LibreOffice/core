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



#include "vcl/msgbox.hxx"
#include "osl/file.hxx"

#include "ids.hrc"
#include "nameclashdlg.hrc"
#include "nameclashdlg.hxx"

// NameClashDialog ---------------------------------------------------------

IMPL_LINK( NameClashDialog, ButtonHdl_Impl, PushButton *, pBtn )
{
    long nRet = (long) ABORT;
    if ( &maBtnRename == pBtn )
    {
        nRet = (long) RENAME;
        rtl::OUString aNewName = maEDNewName.GetText();
        if ( ( aNewName == maNewName ) || !aNewName.getLength() )
        {
            ErrorBox aError( NULL, WB_OK, maSameName );
            aError.Execute();
            return 1;
        }
        maNewName = aNewName;
    }
    else if ( &maBtnOverwrite == pBtn )
        nRet = (long) OVERWRITE;

    EndDialog( nRet );

    return 1;
}

// -----------------------------------------------------------------------
NameClashDialog::NameClashDialog( Window* pParent, ResMgr* pResMgr,
                                  rtl::OUString const & rTargetFolderURL,
                                  rtl::OUString const & rClashingName,
                                  rtl::OUString const & rProposedNewName,
                                  bool bAllowOverwrite )
    : ModalDialog( pParent, ResId( DLG_SIMPLE_NAME_CLASH, *pResMgr ) ),
    maFTMessage            ( this, ResId( FT_FILE_EXISTS_WARNING, *pResMgr ) ),
    maEDNewName            ( this, ResId( EDIT_NEW_NAME, *pResMgr ) ),
    maBtnOverwrite         ( this, ResId( BTN_OVERWRITE, *pResMgr ) ),
    maBtnRename            ( this, ResId( BTN_RENAME, *pResMgr ) ),
    maBtnCancel            ( this, ResId( BTN_CANCEL, *pResMgr ) ),
    maBtnHelp              ( this, ResId( BTN_HELP, *pResMgr ) ),
    maNewName              ( rClashingName )
{
    FreeResource();

    Link aLink( LINK( this, NameClashDialog, ButtonHdl_Impl ) );
    maBtnOverwrite.SetClickHdl( aLink );
    maBtnRename.SetClickHdl( aLink );
    maBtnCancel.SetClickHdl( aLink );

    String aInfo;
    if ( bAllowOverwrite )
    {
        aInfo = String( ResId( STR_RENAME_OR_REPLACE, *pResMgr ) );
    }
    else
    {
        aInfo = String( ResId( STR_NAME_CLASH_RENAME_ONLY, *pResMgr ) );
        maBtnOverwrite.Hide();
    }

    rtl::OUString aPath;
    if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( rTargetFolderURL, aPath ) )
        aPath = rTargetFolderURL;

    maSameName = String ( ResId( STR_SAME_NAME_USED, *pResMgr ) );

    aInfo.SearchAndReplaceAscii( "%NAME", rClashingName );
    aInfo.SearchAndReplaceAscii( "%FOLDER", aPath );
    maFTMessage.SetText( aInfo );
    if ( rProposedNewName.getLength() )
        maEDNewName.SetText( rProposedNewName );
    else
        maEDNewName.SetText( rClashingName );
}

