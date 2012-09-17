/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Novell, Inc.
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


#include "ODMAFilePicker.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>

#ifndef ODMA_LIB_HXX
#include <windows.h>
#include <odma_lib.hxx>
#endif

// using ----------------------------------------------------------------

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::utl;

//------------------------------------------------------------------------------------
// class ODMAFilePicker
//------------------------------------------------------------------------------------
ODMAFilePicker::ODMAFilePicker( const Reference < XComponentContext >& rxContext ) :
    cppu::WeakComponentImplHelper9<
      XFilterManager,
      XFilterGroupManager,
      XFilePickerControlAccess,
      XFilePickerNotifier,
      XFilePreview,
      XInitialization,
      XCancellable,
      XEventListener,
      XServiceInfo>( m_rbHelperMtx ),
    m_xContext( rxContext ),
    m_bMultiSelectionMode( sal_False ),
    m_aDefaultName( ),
    m_aFiles( ),
    m_nDialogKind( OPEN )
{
    m_bUseDMS = ::odma::DMSsAvailable();
    m_xSystemFilePicker = rxContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.ui.dialogs.Win32FilePicker", rxContext);
}

// XExecutableDialog functions

void SAL_CALL ODMAFilePicker::setTitle( const ::rtl::OUString& aTitle )
    throw (RuntimeException)
{
    Reference< XExecutableDialog > xExecutableDialog( m_xSystemFilePicker, UNO_QUERY );
    xExecutableDialog->setTitle( aTitle);
}

inline bool is_current_process_window(HWND hwnd)
{
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return (pid == GetCurrentProcessId());
}

HWND choose_parent_window()
{
    HWND hwnd_parent = GetForegroundWindow();
    if (!is_current_process_window(hwnd_parent))
       hwnd_parent = GetDesktopWindow();
    return hwnd_parent;
}

sal_Int16 SAL_CALL ODMAFilePicker::execute( )
    throw (RuntimeException)
{
    ODMSTATUS status;
    ODMHANDLE handle;
    WORD count = 0;
    DWORD flags;

    status = NODMRegisterApp( &handle, ODM_API_VERSION, const_cast<char*>("sodma"), (DWORD) choose_parent_window( ), NULL );
    if (status == ODM_SUCCESS)
    {
        if (m_nDialogKind == OPEN)
        {
            const int MAXDOCS = 10;
            char docids[ODM_DOCID_MAX*MAXDOCS+1];
            WORD docidslen = sizeof( docids );

            flags = 0;
            if (m_bMultiSelectionMode)
                count = MAXDOCS;
            else
                count = 1;
            status = NODMSelectDocEx( handle, docids, &docidslen, &count, &flags, NULL );
            if (status == ODM_SUCCESS)
            {
                // GroupWise doesn't set docidslen or count, so
                // calculate number of document IDs manually
                char *p = docids;
                count = 0;
                while (*p) {
                    count++;
                    p += strlen( p ) + 1;
                }
            }
            else if (status == ODM_E_NOSUPPORT)
            {
                status = NODMSelectDoc( handle, docids, &flags );
                if (status == ODM_SUCCESS)
                    count = 1;
            }
            NODMUnRegisterApp( handle );
            if (status == ODM_SUCCESS)
            {
                rtl::OUString *strings = new rtl::OUString[count];
                int i;
                char *p = docids;

                for (i = 0; i < count; i++) {
                    // Insane API... the first element is a full URI, the rest
                    // are just the "basenames" in the same "directory".
                    if (i == 0)
                        strings[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.odma:/" )) + rtl::OUString::createFromAscii( p );
                    else
                        strings[i] = rtl::OUString::createFromAscii( p );
                    p += strlen( p );
                }

                m_aFiles = Sequence< rtl::OUString >( strings, count );
                delete[] strings;
                return ExecutableDialogResults::OK;
            }
        }
        else /* m_nDialogKind == SAVE */
        {
            char newdocid[ODM_DOCID_MAX+1];

            if (m_aDefaultName.getLength() == 0 ||
                !m_aDefaultName.matchIgnoreAsciiCaseAsciiL( "::ODMA\\", 7, 0 ))
            {
                char tempdocid[ODM_DOCID_MAX+1];

                status = NODMNewDoc( handle, tempdocid, ODM_SILENT, NULL, NULL );
                if (status == ODM_SUCCESS)
                    status = NODMSaveAs( handle, tempdocid, newdocid, NULL, NULL, NULL );
            }
            else
            {
                rtl::OString sDefaultName = rtl::OUStringToOString( m_aDefaultName, RTL_TEXTENCODING_ASCII_US );
                status = NODMSaveAs( handle,
                                     const_cast<sal_Char*>( sDefaultName.getStr() ),
                                     newdocid, NULL, NULL, NULL );
            }

            NODMUnRegisterApp( handle );
            if (status == ODM_SUCCESS)
            {
                rtl::OUString s( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.odma:/" )) +
                                 rtl::OUString::createFromAscii( newdocid ) );
                // Create a Content for the odma URL so that
                // odma::ContentProvider will learn about the DOCID we
                // just created.
                ucbhelper::Content content( s, Reference< XCommandEnvironment >(), m_xContext );
                m_aFiles = Sequence< rtl::OUString >( &s, 1 );
                return ExecutableDialogResults::OK;
            }
        }
    }
    if (status == ODM_E_APPSELECT)
    {
        m_bUseDMS = sal_False;

        Reference< XExecutableDialog > xExecutableDialog( m_xSystemFilePicker, UNO_QUERY );
        return xExecutableDialog->execute();
    }

    // Fallback
    return ExecutableDialogResults::CANCEL;
}

// XFilePicker functions

void SAL_CALL ODMAFilePicker::setMultiSelectionMode( sal_Bool bMode )
    throw( RuntimeException )
{
    m_bMultiSelectionMode = bMode;

    Reference< XFilePicker > xFilePicker( m_xSystemFilePicker, UNO_QUERY );
    xFilePicker->setMultiSelectionMode( bMode );
}

void SAL_CALL ODMAFilePicker::setDefaultName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    // When editing a document from DMS, and doing Save As, this is
    // called twice, first with the complete DOCID, the second time
    // with the "extension" removed. Of course, this is bogus, as
    // DOCIDs should be treated as opaque strings, they don't have
    // "extensions".

    // In the GroupWise case a DOCID is like
    // ::ODMA\GRPWISE\FOO1.BAR.FOO-Bla_bla:12345.12 where the final
    // dot separates the version number, not an "extension".
    //
    // So ignore the second call.
    //
    // The second call without "extension" is done if the
    // XFilePickerControlAccess is set to have auto-extension turned
    // on. (See sfx2/source/dialog/filedlghelper.cxx:
    // FileDialogHelper_Impl::implInitializeFileName().) Thus we could
    // alternatively make sure that a getValue call to get
    // ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION returns
    // false. But that might be hard as we don't know in advance
    // whether the user will click the "use application dialog"
    // button. If so, we indeed do use auto-extensions.
    //
    // Yes, this is ugly, relying on knowing details on how this
    // function will be called.

    if ( m_aDefaultName.getLength() == 0 )
        m_aDefaultName = aName;

    Reference< XFilePicker > xFilePicker( m_xSystemFilePicker, UNO_QUERY );
    xFilePicker->setDefaultName( aName );
}

void SAL_CALL ODMAFilePicker::setDisplayDirectory( const rtl::OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    m_aDisplayDirectory = aDirectory;

    Reference< XFilePicker > xFilePicker( m_xSystemFilePicker, UNO_QUERY );
    xFilePicker->setDisplayDirectory( aDirectory );
}

rtl::OUString SAL_CALL ODMAFilePicker::getDisplayDirectory( )
    throw( RuntimeException )
{
    if (m_bUseDMS)
        return m_aDisplayDirectory;

    Reference< XFilePicker > xFilePicker( m_xSystemFilePicker, UNO_QUERY );
    return xFilePicker->getDisplayDirectory();
}

Sequence< rtl::OUString > SAL_CALL ODMAFilePicker::getFiles( )
    throw( RuntimeException )
{
    if (m_bUseDMS)
        return m_aFiles;

    Reference< XFilePicker > xFilePicker( m_xSystemFilePicker, UNO_QUERY );
    return xFilePicker->getFiles();
}

// XFilePickerControlAccess functions

void SAL_CALL ODMAFilePicker::setValue( sal_Int16 nElementID,
                                        sal_Int16 nControlAction,
                                        const Any& rValue )
    throw( RuntimeException )
{
    Reference< XFilePickerControlAccess > xFilePickerControlAccess( m_xSystemFilePicker, UNO_QUERY );
    xFilePickerControlAccess->setValue( nElementID, nControlAction, rValue );
}

Any SAL_CALL ODMAFilePicker::getValue( sal_Int16 nElementID, sal_Int16 nControlAction )
    throw( RuntimeException )
{
    Reference< XFilePickerControlAccess > xFilePickerControlAccess( m_xSystemFilePicker, UNO_QUERY );
    return xFilePickerControlAccess->getValue( nElementID, nControlAction );
}

void SAL_CALL ODMAFilePicker::setLabel( sal_Int16 nLabelID, const rtl::OUString& rValue )
    throw ( RuntimeException )
{
    Reference< XFilePickerControlAccess > xFilePickerControlAccess( m_xSystemFilePicker, UNO_QUERY );
    xFilePickerControlAccess->setLabel( nLabelID, rValue );
}

rtl::OUString SAL_CALL ODMAFilePicker::getLabel( sal_Int16 nLabelID )
    throw ( RuntimeException )
{
    Reference< XFilePickerControlAccess > xFilePickerControlAccess( m_xSystemFilePicker, UNO_QUERY );
    return xFilePickerControlAccess->getLabel( nLabelID );
}

void SAL_CALL ODMAFilePicker::enableControl( sal_Int16 nElementID, sal_Bool bEnable )
    throw( RuntimeException )
{
    Reference< XFilePickerControlAccess > xFilePickerControlAccess( m_xSystemFilePicker, UNO_QUERY );
    xFilePickerControlAccess->enableControl( nElementID, bEnable );
}

// XFilePickerNotifier functions

void SAL_CALL ODMAFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener )
    throw ( RuntimeException )
{
    Reference< XFilePickerNotifier > xFilePickerNotifier( m_xSystemFilePicker, UNO_QUERY );
    xFilePickerNotifier->addFilePickerListener( xListener );
}

void SAL_CALL ODMAFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& xListener )
    throw ( RuntimeException )
{
    Reference< XFilePickerNotifier > xFilePickerNotifier( m_xSystemFilePicker, UNO_QUERY );
    xFilePickerNotifier->removeFilePickerListener( xListener );
}

// XFilePreview functions

Sequence< sal_Int16 > SAL_CALL ODMAFilePicker::getSupportedImageFormats( )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->getSupportedImageFormats();
}

sal_Int32 SAL_CALL ODMAFilePicker::getTargetColorDepth( )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->getTargetColorDepth();
}

sal_Int32 SAL_CALL ODMAFilePicker::getAvailableWidth( )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->getAvailableWidth();
}

sal_Int32 SAL_CALL ODMAFilePicker::getAvailableHeight( )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->getAvailableHeight();
}

void SAL_CALL ODMAFilePicker::setImage( sal_Int16 aImageFormat, const Any& rImage )
    throw ( IllegalArgumentException, RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    xFilePreview->setImage( aImageFormat, rImage);
}

sal_Bool SAL_CALL ODMAFilePicker::setShowState( sal_Bool bShowState )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->setShowState( bShowState);
}

sal_Bool SAL_CALL ODMAFilePicker::getShowState( )
    throw ( RuntimeException )
{
    Reference< XFilePreview > xFilePreview( m_xSystemFilePicker, UNO_QUERY );
    return xFilePreview->getShowState();
}

// XFilterGroupManager functions

void SAL_CALL ODMAFilePicker::appendFilterGroup( const ::rtl::OUString& sGroupTitle,
                                                 const Sequence< StringPair >& aFilters )
    throw ( IllegalArgumentException, RuntimeException )
{
    Reference< XFilterGroupManager > xFilterGroupManager( m_xSystemFilePicker, UNO_QUERY );
    xFilterGroupManager->appendFilterGroup( sGroupTitle, aFilters );
}

// XFilterManager functions

void SAL_CALL ODMAFilePicker::appendFilter( const rtl::OUString& aTitle,
                                            const rtl::OUString& aFilter )
    throw( IllegalArgumentException, RuntimeException )
{
    Reference< XFilterManager > xFilterManager( m_xSystemFilePicker, UNO_QUERY );
    xFilterManager->appendFilter( aTitle, aFilter );
}

void SAL_CALL ODMAFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
    throw( IllegalArgumentException, RuntimeException )
{
    Reference< XFilterManager > xFilterManager( m_xSystemFilePicker, UNO_QUERY );
    xFilterManager->setCurrentFilter( aTitle );
}

rtl::OUString SAL_CALL ODMAFilePicker::getCurrentFilter( )
    throw( RuntimeException )
{
    Reference< XFilterManager > xFilterManager( m_xSystemFilePicker, UNO_QUERY );
    return xFilterManager->getCurrentFilter();
}

// XInitialization functions

void SAL_CALL ODMAFilePicker::initialize( const Sequence< Any >& aArguments )
    throw ( Exception, RuntimeException )
{
    if (aArguments.getLength( ) > 0)
    {
        Any aAny = aArguments[0];

        if ( (aAny.getValueType() == ::getCppuType((sal_Int16*)0)) ||
             (aAny.getValueType() == ::getCppuType((sal_Int8*)0)) )
        {
            sal_Int16 nTemplateId = -1;
            aAny >>= nTemplateId;

            switch (nTemplateId) {
            case TemplateDescription::FILEOPEN_SIMPLE:
            case TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            case TemplateDescription::FILEOPEN_PLAY:
            case TemplateDescription::FILEOPEN_READONLY_VERSION:
            case TemplateDescription::FILEOPEN_LINK_PREVIEW:
                m_nDialogKind = OPEN;
                break;
            default:
                m_nDialogKind = SAVE;
            }
        }
    }

    Reference< XInitialization > xInitialization( m_xSystemFilePicker, UNO_QUERY );
    xInitialization->initialize( aArguments );
}

// XCancellable functions

void SAL_CALL ODMAFilePicker::cancel( )
    throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XCancellable > xCancellable( m_xSystemFilePicker, UNO_QUERY );
    xCancellable->cancel();
}

// XEventListener functions

void SAL_CALL ODMAFilePicker::disposing( const ::com::sun::star::lang::EventObject& aEvent )
    throw( RuntimeException )
{
    Reference< XEventListener > xEventListener( m_xSystemFilePicker, UNO_QUERY );
    xEventListener->disposing( aEvent );
}

// XServiceInfo functions

rtl::OUString SAL_CALL ODMAFilePicker::getImplementationName( )
    throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

sal_Bool SAL_CALL ODMAFilePicker::supportsService( const rtl::OUString& sServiceName )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 i = 0; i < seqServiceNames.getLength(); i++ )
    {
        if ( sServiceName == pArray[i] )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

Sequence< rtl::OUString > SAL_CALL ODMAFilePicker::getSupportedServiceNames( )
    throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

Sequence< rtl::OUString > ODMAFilePicker::impl_getStaticSupportedServiceNames( )
{
    Sequence< rtl::OUString > seqServiceNames( 2 );
    rtl::OUString* pArray = seqServiceNames.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.ODMAFilePicker" ));

    // Super-ugly, but check the function sal_Bool
    // lcl_isSystemFilePicker( const Reference< XFilePicker >& ) in
    // sfx2/source/dialog/filedlghelper.cxx

    // Lovely undocumented coupling right across abstraction layers,
    // isn't it?  If we don't claim to implement this "service" (which
    // is not defined in any IDL file, btw), we get hangs.

    pArray[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.SystemFilePicker" ));
    return seqServiceNames ;
}

rtl::OUString ODMAFilePicker::impl_getStaticImplementationName( )
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svtools.ODMAFilePicker" ));
}

Reference< XInterface > SAL_CALL ODMAFilePicker::impl_createInstance(
    const Reference< XComponentContext >& rxContext)
    throw( Exception )
{
    return Reference< XInterface >( *new ODMAFilePicker( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
