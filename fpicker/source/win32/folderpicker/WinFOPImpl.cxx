/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "WinFOPImpl.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/lang/EventObject.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include "FopEvtDisp.hxx"
#include <osl/file.hxx>
#include "FolderPicker.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::EventObject;
using rtl::OUString;

using namespace com::sun::star::ui::dialogs;
using osl::FileBase;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

const OUString BACKSLASH(RTL_CONSTASCII_USTRINGPARAM( "\\" ));

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CWinFolderPickerImpl::CWinFolderPickerImpl( CFolderPicker* aFolderPicker ) :
   CMtaFolderPicker( BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_EDITBOX | BIF_VALIDATE ),
   m_pFolderPicker( aFolderPicker ),
   m_nLastDlgResult( ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL )
{
}

//------------------------------------------------------------------------
// get directory in URL format, convert it to system format and set the
// member variable
// If the given URL for the directory is invalid the function throws an
// IllegalArgumentException
// If the specified path is well formed but invalid for the underlying
// OS the FolderPicker starts in the root of the file system hierarchie
//------------------------------------------------------------------------

void SAL_CALL CWinFolderPickerImpl::setDisplayDirectory( const OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    OUString sysDir;

    if( aDirectory.getLength( ) )
    {
        // assuming that this function succeeds after successful execution
        // of getAbsolutePath
        ::osl::FileBase::RC rc =
            ::osl::FileBase::getSystemPathFromFileURL( aDirectory, sysDir );

        if ( ::osl::FileBase::E_None != rc )
            throw IllegalArgumentException(
                OUString(RTL_CONSTASCII_USTRINGPARAM( "directory is not a valid file url" )),
                static_cast< XFolderPicker* >( m_pFolderPicker ),
                1 );

        // we ensure that there is a trailing '/' at the end of
        // he given file url, because the windows functions only
        // works correctly when providing "c:\" or an environment
        // variable like "=c:=c:\.." etc. is set, else the
        // FolderPicker would stand in the root of the shell
        // hierarchie which is the desktop folder
        if ( sysDir.lastIndexOf( BACKSLASH ) != (sysDir.getLength( ) - 1) )
            sysDir += BACKSLASH;
    }

    // call base class method
    CMtaFolderPicker::setDisplayDirectory( sysDir );
}

//------------------------------------------------------------------------
// we return the directory in URL format
//------------------------------------------------------------------------

OUString CWinFolderPickerImpl::getDisplayDirectory( )
    throw( RuntimeException )
{
    // call base class method to get the directory in system format
    OUString displayDirectory = CMtaFolderPicker::getDisplayDirectory( );

    OUString displayDirectoryURL;
    if ( displayDirectory.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( displayDirectory, displayDirectoryURL );

    return displayDirectoryURL;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CWinFolderPickerImpl::getDirectory( ) throw( RuntimeException )
{
    OUString sysDir = CMtaFolderPicker::getDirectory( );
    OUString dirURL;

    if ( sysDir.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( sysDir, dirURL );

    return dirURL;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int16 SAL_CALL CWinFolderPickerImpl::execute( ) throw( RuntimeException )
{
    return m_nLastDlgResult = CMtaFolderPicker::browseForFolder( ) ?
        ::com::sun::star::ui::dialogs::ExecutableDialogResults::OK :
        ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
}

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

void CWinFolderPickerImpl::onSelChanged( const OUString& aNewPath )
{
    setStatusText( aNewPath );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
