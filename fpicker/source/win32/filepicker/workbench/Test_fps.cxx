/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Test_fps.cxx,v $
 * $Revision: 1.9 $
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


//_________________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________________
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <osl/file.hxx>

//_________________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________________
#include <cppuhelper/servicefactory.hxx>

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#include <sal/types.h>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

#ifndef _COM_SUN_STAR_UI_DIALOGS_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>

#include <osl/thread.h>

#include <stdio.h>
#include <windows.h>

#include "..\FPServiceInfo.hxx"

//
//  namesapces
//

using namespace ::rtl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::ui::dialogs   ;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::ListboxControlActions;

using namespace std                     ;

// forward

void TestFilterManager( Reference< XFilePicker > xFilePicker );


#define RDB_SYSPATH "D:\\Projects\\gsl\\sysui\\wntmsci7\\bin\\applicat.rdb"

//_________________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________________

Reference< XMultiServiceFactory >   g_xFactory;

const OUString BMP_EXTENSION = OUString::createFromAscii( "bmp" );

//-------------------------------------------------------------------------------------------------------------------------
// a test client
//-------------------------------------------------------------------------------------------------------------------------

class FilePickerListener : public WeakImplHelper1< XFilePickerListener >
{
public:

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw(::com::sun::star::uno::RuntimeException);

    // XFilePickerListener
    virtual void SAL_CALL fileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL directoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL helpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL controlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL dialogSizeChanged(  )
        throw (::com::sun::star::uno::RuntimeException);
};

void SAL_CALL FilePickerListener::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL FilePickerListener::fileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
    throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        Reference< XFilePicker > rXFilePicker( aEvent.Source, UNO_QUERY );
        Reference< XFilePreview > rXFilePreview( rXFilePicker, UNO_QUERY );

        if ( !rXFilePreview.is( ) )
            return;

        Sequence< OUString > aFileList = rXFilePicker->getFiles( );
        if ( 1 == aFileList.getLength( ) )
        {
            OUString FilePath = aFileList[0];

            // detect file extension
            sal_Int32 nIndex = FilePath.lastIndexOf( BMP_EXTENSION );
            if ( (FilePath.getLength( ) - 3) == nIndex )
            {
                 OUString FileSysPath;
                ::osl::FileBase::getSystemPathFromFileURL(
                    FilePath, FileSysPath );

                 HANDLE hFile = CreateFileW(
                     FileSysPath.getStr( ),
                     GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL) ;

                 if (hFile == INVALID_HANDLE_VALUE)
                      return;

                 DWORD dwHighSize;
                 DWORD dwFileSize = GetFileSize (hFile, &dwHighSize) ;

                 if (dwHighSize)
                 {
                      CloseHandle (hFile) ;
                      return;
                 }

                 Sequence< sal_Int8 > aDIB( dwFileSize );

                 DWORD dwBytesRead;
                 BOOL bSuccess = ReadFile (hFile, aDIB.getArray( ), dwFileSize, &dwBytesRead, NULL) ;
                 CloseHandle (hFile);

                 BITMAPFILEHEADER* pbmfh = (BITMAPFILEHEADER*)aDIB.getConstArray( );
                 if (!bSuccess || (dwBytesRead != dwFileSize)
                               || (pbmfh->bfType != * (WORD *) "BM")
                               || (pbmfh->bfSize != dwFileSize))
                 {
                    return;
                 }

                Any aAny;

                aAny <<= aDIB;
                rXFilePreview->setImage( 1, aAny );
            }
        }
    }
    catch( IllegalArgumentException& ex )
    {
        ex = ex;
    }
}

void SAL_CALL FilePickerListener::directoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XFilePickerControlAccess > rFilePickerCtrlAccess( aEvent.Source, UNO_QUERY );
}

OUString SAL_CALL FilePickerListener::helpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( );
}

void SAL_CALL FilePickerListener::controlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent )
    throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        Reference< XFilePickerControlAccess > rFPCtrlAccess( aEvent.Source, UNO_QUERY );

        Any aValue;

        OUString lbString( L"Ein Eintrag 1" );
        aValue <<= lbString;
        rFPCtrlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aValue );

        lbString = OUString( L"Ein Eintrag 2" );
        aValue <<= lbString;
        rFPCtrlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aValue );

        lbString = OUString( L"Ein Eintrag 3" );
        aValue <<= lbString;
        rFPCtrlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aValue );

        sal_Int16 nSel = 1;
        aValue <<= nSel;
        rFPCtrlAccess->setValue( LISTBOX_VERSION, SET_SELECT_ITEM, aValue );

        sal_Int32 nDel = 0;
        aValue <<= nDel;
        rFPCtrlAccess->setValue( LISTBOX_VERSION, DELETE_ITEM, aValue );
    }
    catch( ... )
    {
    }
}

void SAL_CALL FilePickerListener::dialogSizeChanged( )
    throw(::com::sun::star::uno::RuntimeException)
{
}

//--------------------------------------------------------
//  main
//--------------------------------------------------------


int SAL_CALL main(int nArgc, char* Argv[], char* Env[]  )
{
    printf("Starting test of FPS-Service\n");

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------

    // Get global factory for uno services.
    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occured.
    if ( g_xFactory.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    Sequence< Any > arguments(1);
        //arguments[0] = makeAny( FILEOPEN_SIMPLE );
        //arguments[0] = makeAny( FILESAVE_SIMPLE );
        //arguments[0] = makeAny( FILESAVE_AUTOEXTENSION_PASSWORD );
        //arguments[0] = makeAny( FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS );
        //arguments[0] = makeAny( FILESAVE_AUTOEXTENSION_SELECTION );
        //arguments[0] = makeAny( FILESAVE_AUTOEXTENSION_TEMPLATE );
        //arguments[0] = makeAny( FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE );
        //arguments[0] = makeAny( FILEOPEN_PLAY );
        arguments[0] = makeAny( FILEOPEN_READONLY_VERSION );

    Reference< XFilePicker > xFilePicker = Reference< XFilePicker >(
        g_xFactory->createInstanceWithArguments(
            OUString::createFromAscii( FILE_PICKER_SERVICE_NAME ), arguments ), UNO_QUERY );

        // install a FilePicker notifier
        Reference< XFilePickerListener > xFPListener(
            static_cast< XFilePickerListener* >( new FilePickerListener()), UNO_QUERY );

        Reference< XFilePickerNotifier > xFPNotifier( xFilePicker, UNO_QUERY );
        if ( xFPNotifier.is( ) )
            xFPNotifier->addFilePickerListener( xFPListener );

        xFilePicker->setTitle( OUString::createFromAscii("FileOpen Simple..."));
        xFilePicker->setMultiSelectionMode( sal_True );
        xFilePicker->setDefaultName( OUString::createFromAscii("d:\\test2.sxw"));

        OUString aDirURL;
        OUString aSysPath = OStringToOUString( "d:\\ueaeoe", osl_getThreadTextEncoding( ) );
        ::osl::FileBase::getFileURLFromSystemPath( aSysPath, aDirURL );
        xFilePicker->setDisplayDirectory( aDirURL );

        Reference< XFilterManager > xFilterMgr( xFilePicker, UNO_QUERY );
        if ( xFilterMgr.is( ) )
        {
            xFilterMgr->appendFilter( L"Alle", L"*.*" );
            xFilterMgr->appendFilter( L"BMP", L"*.bmp" );
            xFilterMgr->appendFilter( L"SDW", L"*.sdw;*.sdc;*.sdi" );
            xFilterMgr->appendFilter( L"SXW", L"*.sxw;*.sxi" );
        }

        Reference< XFilePickerControlAccess > xFPControlAccess( xFilePicker, UNO_QUERY );

        Any aAny;
        sal_Bool bChkState = sal_False;

        aAny.setValue( &bChkState, getCppuType( (sal_Bool*)0 ) );
        xFPControlAccess->setValue( CHECKBOX_AUTOEXTENSION, 0, aAny );

        OUString aVersion( L"Version 1" );
        aAny <<= aVersion;
        xFPControlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aAny );
        xFPControlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aAny );
        xFPControlAccess->setValue( LISTBOX_VERSION, ADD_ITEM, aAny );

        xFilePicker->execute( );

        sal_Bool bCheckState;
        aAny = xFPControlAccess->getValue( CHECKBOX_AUTOEXTENSION, 0 );
        if ( aAny.hasValue( ) )
            bCheckState = *reinterpret_cast< const sal_Bool* >( aAny.getValue( ) );

        aAny = xFPControlAccess->getValue( CHECKBOX_READONLY, 0 );
        if ( aAny.hasValue( ) )
            bCheckState = *reinterpret_cast< const sal_Bool* >( aAny.getValue( ) );

        aAny = xFPControlAccess->getValue( LISTBOX_VERSION, GET_SELECTED_ITEM );
        sal_Int32 nSel;
        if ( aAny.hasValue( ) )
            aAny >>= nSel;

        aDirURL = xFilePicker->getDisplayDirectory( );
        Sequence< OUString > aFileList = xFilePicker->getFiles( );
        for ( int i = 0; i < aFileList.getLength( ); i++ )
        {
            OUString nextPath = aFileList[i];
        }

        if ( xFPNotifier.is( ) )
            xFPNotifier->removeFilePickerListener( xFPListener );

    //--------------------------------------------------
    // shutdown
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occured.
    if ( xComponent.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Error shuting down");
    }

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    printf("Test successful\n");

    return 0;
}
