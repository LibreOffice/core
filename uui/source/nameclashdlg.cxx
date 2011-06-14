/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

