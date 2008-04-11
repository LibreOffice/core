/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WinFOPImpl.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _WINDIRBROWSEIMPL_HXX_
#include "WinFOPImpl.hxx"
#endif
#include <osl/diagnose.h>
#include <com/sun/star/lang/EventObject.hpp>

#ifndef _COM_SUN_STAR_UI_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
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

const OUString BACKSLASH = OUString::createFromAscii( "\\" );

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
                OUString::createFromAscii( "directory is not a valid file url" ),
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
