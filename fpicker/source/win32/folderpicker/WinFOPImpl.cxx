/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "WinFOPImpl.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/lang/EventObject.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include "FopEvtDisp.hxx"
#include <osl/file.hxx>
#include "FolderPicker.hxx"





using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::EventObject;

using namespace com::sun::star::ui::dialogs;
using osl::FileBase;


//


const OUString BACKSLASH( "\\" );





CWinFolderPickerImpl::CWinFolderPickerImpl( CFolderPicker* aFolderPicker ) :
   CMtaFolderPicker( BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_EDITBOX | BIF_VALIDATE ),
   m_pFolderPicker( aFolderPicker ),
   m_nLastDlgResult( ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL )
{
}










void SAL_CALL CWinFolderPickerImpl::setDisplayDirectory( const OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    OUString sysDir;

    if( aDirectory.getLength( ) )
    {
        
        
        ::osl::FileBase::RC rc =
            ::osl::FileBase::getSystemPathFromFileURL( aDirectory, sysDir );

        if ( ::osl::FileBase::E_None != rc )
            throw IllegalArgumentException(
                OUString( "directory is not a valid file url" ),
                static_cast< cppu::OWeakObject * >( m_pFolderPicker ),
                1 );

        
        
        
        
        
        
        if ( sysDir.lastIndexOf( BACKSLASH ) != (sysDir.getLength( ) - 1) )
            sysDir += BACKSLASH;
    }

    
    CMtaFolderPicker::setDisplayDirectory( sysDir );
}





OUString CWinFolderPickerImpl::getDisplayDirectory( )
    throw( RuntimeException )
{
    
    OUString displayDirectory = CMtaFolderPicker::getDisplayDirectory( );

    OUString displayDirectoryURL;
    if ( displayDirectory.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( displayDirectory, displayDirectoryURL );

    return displayDirectoryURL;
}


//


OUString SAL_CALL CWinFolderPickerImpl::getDirectory( ) throw( RuntimeException )
{
    OUString sysDir = CMtaFolderPicker::getDirectory( );
    OUString dirURL;

    if ( sysDir.getLength( ) )
        ::osl::FileBase::getFileURLFromSystemPath( sysDir, dirURL );

    return dirURL;
}


//


sal_Int16 SAL_CALL CWinFolderPickerImpl::execute( ) throw( RuntimeException )
{
    return m_nLastDlgResult = CMtaFolderPicker::browseForFolder( ) ?
        ::com::sun::star::ui::dialogs::ExecutableDialogResults::OK :
        ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
}


//


void CWinFolderPickerImpl::onSelChanged( const OUString& aNewPath )
{
    setStatusText( aNewPath );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
