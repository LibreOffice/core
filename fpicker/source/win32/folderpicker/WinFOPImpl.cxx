/*************************************************************************
 *
 *  $RCSfile: WinFOPImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hro $ $Date: 2002-08-14 15:45:02 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _WINDIRBROWSEIMPL_HXX_
#include "WinFOPImpl.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#ifndef _FOPEVENTDISPATCHER_HXX_
#include "FopEvtDisp.hxx"
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _FOLDERPICKER_HXX_
#include "FolderPicker.hxx"
#endif

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
