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

#include <tchar.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <osl/diagnose.h>

#include "FilePicker.hxx"
#include "WinFileOpenImpl.hxx"

#include "FPServiceInfo.hxx"
#include "../misc/WinImplHelper.hxx"
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include "filepickereventnotification.hxx"

#include <comphelper/sequenceasvector.hxx>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star;

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

#define FILE_PICKER_DLL_NAME  TEXT("fps.dll")

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controlling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<OUString> aRet(2);
        aRet[0] = OUString("com.sun.star.ui.dialogs.FilePicker");
        aRet[1] = OUString("com.sun.star.ui.dialogs.SystemFilePicker");
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

CFilePicker::CFilePicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr) :
    CFilePicker_Base(m_rbHelperMtx),
    m_xServiceMgr(xServiceMgr),
    m_aAsyncEventNotifier(rBHelper)
{
    HINSTANCE hInstance = GetModuleHandle(FILE_PICKER_DLL_NAME);
    OSL_POSTCOND( hInstance, "The name of the service dll must have changed" );

    // create a default FileOpen dialog without any additional ui elements
    m_pImpl = std::auto_ptr< CWinFileOpenImpl >(
        new CWinFileOpenImpl(
            this,
            true,
            0,
            0,
            hInstance ) );
}

//------------------------------------------------------------------------------------
// XFPEventListenerManager
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::addFilePickerListener(const uno::Reference<XFilePickerListener>& xListener)
    throw(uno::RuntimeException)
{
    if ( rBHelper.bDisposed )
        throw lang::DisposedException(
            OUString( "object is already disposed" ),
            static_cast< XFilePicker2* >( this ) );

    if ( !rBHelper.bInDispose && !rBHelper.bDisposed )
        rBHelper.aLC.addInterface( getCppuType( &xListener ), xListener );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::removeFilePickerListener(const uno::Reference<XFilePickerListener>& xListener )
    throw(uno::RuntimeException)
{
    if ( rBHelper.bDisposed )
        throw lang::DisposedException(
            OUString( "object is already disposed" ),
            static_cast< XFilePicker2* >( this ) );

    rBHelper.aLC.removeInterface( getCppuType( &xListener ), xListener );
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL CFilePicker::disposing(const lang::EventObject& aEvent) throw(uno::RuntimeException)
{
    uno::Reference<XFilePickerListener> xFilePickerListener(aEvent.Source, ::com::sun::star::uno::UNO_QUERY);

    if (xFilePickerListener.is())
        removeFilePickerListener(xFilePickerListener);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::fileSelectionChanged(FilePickerEvent aEvent)
{
    aEvent.Source = uno::Reference<uno::XInterface>(static_cast<XFilePickerNotifier*>(this));
    m_aAsyncEventNotifier.notifyEvent(
        new CFilePickerParamEventNotification(&XFilePickerListener::fileSelectionChanged,aEvent));
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::directoryChanged(FilePickerEvent aEvent)
{
    aEvent.Source = uno::Reference<uno::XInterface>(static_cast<XFilePickerNotifier*>(this));
    m_aAsyncEventNotifier.notifyEvent(
        new CFilePickerParamEventNotification(&XFilePickerListener::directoryChanged,aEvent));
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::controlStateChanged(FilePickerEvent aEvent)
{
    aEvent.Source = uno::Reference<uno::XInterface>(static_cast<XFilePickerNotifier*>(this));
    m_aAsyncEventNotifier.notifyEvent(
        new CFilePickerParamEventNotification(&XFilePickerListener::controlStateChanged,aEvent));
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::dialogSizeChanged()
{
    m_aAsyncEventNotifier.notifyEvent(
        new CFilePickerEventNotification(&XFilePickerListener::dialogSizeChanged));
}

//-----------------------------------------------------------------------------------------
// If there are more then one listener the return value of the last one wins
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::helpRequested(FilePickerEvent aEvent) const
{
    OUString aHelpText;

    ::cppu::OInterfaceContainerHelper* pICHelper =
        rBHelper.getContainer( getCppuType((uno::Reference<XFilePickerListener>*)0));

    if (pICHelper)
    {
        ::cppu::OInterfaceIteratorHelper iter(*pICHelper);

        while(iter.hasMoreElements())
        {
            try
            {
                /*
                  if there are multiple listeners responding
                  to this notification the next response
                  overwrittes  the one before if it is not empty
                */

                OUString temp;

                uno::Reference<XFilePickerListener> xFPListener(iter.next(), uno::UNO_QUERY);
                if (xFPListener.is())
                {
                    temp = xFPListener->helpRequested(aEvent);
                    if (temp.getLength())
                        aHelpText = temp;
                }

            }
            catch(uno::RuntimeException&)
            {
                OSL_FAIL( "RuntimeException during event dispatching" );
            }
        }
    }

    return aHelpText;
}

//-------------------------------------
//
//-------------------------------------

bool CFilePicker::startupEventNotification(bool bStartupSuspended)
{
    return m_aAsyncEventNotifier.startup(bStartupSuspended);
}

//-------------------------------------
//
//-------------------------------------

void CFilePicker::shutdownEventNotification()
{
    m_aAsyncEventNotifier.shutdown();
}

//-------------------------------------
//
//-------------------------------------

void CFilePicker::suspendEventNotification()
{
    m_aAsyncEventNotifier.suspend();
}

//-------------------------------------
//
//-------------------------------------

void CFilePicker::resumeEventNotification()
{
    m_aAsyncEventNotifier.resume();
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setMultiSelectionMode(sal_Bool bMode) throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setMultiSelectionMode(bMode);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setTitle(const OUString& aTitle) throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setTitle(aTitle);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::appendFilter(const OUString& aTitle, const OUString& aFilter)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->appendFilter(aTitle, aFilter);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setCurrentFilter(const OUString& aTitle)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setCurrentFilter(aTitle);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getCurrentFilter() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getCurrentFilter();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::appendFilterGroup(const OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters)
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->appendFilterGroup(sGroupTitle, aFilters);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDefaultName(const OUString& aName)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setDefaultName(aName);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDisplayDirectory(const OUString& aDirectory)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setDisplayDirectory(aDirectory);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getDisplayDirectory() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getDisplayDirectory();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

uno::Sequence<OUString> SAL_CALL CFilePicker::getFiles() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getFiles();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL CFilePicker::getSelectedFiles() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);

    const uno::Sequence< OUString > lSource = m_pImpl->getFiles();
    const ::sal_Int32                      c       = lSource.getLength();
    if (c < 2)
        return lSource;

    const OUString                                   sPath  = lSource[0];
          ::comphelper::SequenceAsVector< OUString > lTarget;
          ::sal_Int32                                       i      = 1;
    for (i=1; i<c; ++i)
    {
        const OUString sFile = lSource[i];
        if (sFile.indexOf ('/') > 0)
        {
            // a) file contains own path !
            lTarget.push_back(sFile);
        }
        else
        {
            // b) file is relative to given path
            OUStringBuffer sFull(256);

            sFull.append     (sPath);
            sFull.appendAscii("/"  );
            sFull.append     (sFile);

            lTarget.push_back(sFull.makeStringAndClear());
        }
    }

    return lTarget.getAsConstList();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CFilePicker::execute() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    sal_Int16 ret;

    if (startupEventNotification(STARTUP_SUSPENDED))
    {
        // we should not block in this call else
        // in the case of an event the client can't
        // call another function an we run into a
        // deadlock !!!!!
        ret = m_pImpl->execute( );

        shutdownEventNotification();
    }
    else
    {
        OSL_FAIL("Could not start event notifier thread!");

        throw uno::RuntimeException(
            OUString("Error executing dialog"),
            static_cast<XFilePicker2*>(this));
    }

    return ret;
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setValue(sal_Int16 aControlId, sal_Int16 aControlAction, const uno::Any& aValue)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setValue(aControlId, aControlAction, aValue);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

uno::Any SAL_CALL CFilePicker::getValue(sal_Int16 aControlId, sal_Int16 aControlAction)
    throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getValue(aControlId, aControlAction);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::enableControl(sal_Int16 aControlId, sal_Bool bEnable)
throw(uno::RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );

    osl::MutexGuard aGuard( m_aMutex );
    m_pImpl->enableControl( aControlId, bEnable );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setLabel(sal_Int16 aControlId, const OUString& aLabel)
    throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setLabel(aControlId, aLabel);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getLabel(sal_Int16 aControlId)
    throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getLabel(aControlId);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

uno::Sequence<sal_Int16> SAL_CALL CFilePicker::getSupportedImageFormats() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getSupportedImageFormats();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getTargetColorDepth() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getTargetColorDepth();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getAvailableWidth() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getAvailableWidth();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getAvailableHeight() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getAvailableHeight();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setImage(sal_Int16 aImageFormat, const uno::Any& aImage)
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setImage(aImageFormat, aImage);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilePicker::setShowState(sal_Bool bShowState) throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->setShowState(bShowState);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilePicker::getShowState() throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getShowState();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::initialize(const uno::Sequence<uno::Any>& aArguments)
    throw( uno::Exception, uno::RuntimeException)
{
    // parameter checking
    uno::Any aAny;
    if ( 0 == aArguments.getLength( ) )
        throw lang::IllegalArgumentException(
            OUString( "no arguments" ),
            static_cast<XFilePicker2*>(this), 1);

    aAny = aArguments[0];

    if ( (aAny.getValueType() != ::getCppuType((sal_Int16*)0)) &&
         (aAny.getValueType() != ::getCppuType((sal_Int8*)0)) )
         throw lang::IllegalArgumentException(
            OUString("invalid argument type"),
            static_cast<XFilePicker2*>(this), 1);

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    sal_Bool   bFileOpenDialog  = sal_True;
    sal_uInt32 winResTemplateId = 0;

    switch ( templateId )
    {
    case FILEOPEN_SIMPLE:
        bFileOpenDialog = sal_True;
        break;

    case FILESAVE_SIMPLE:
        bFileOpenDialog = sal_False;
        break;

    case FILESAVE_AUTOEXTENSION_PASSWORD:
        bFileOpenDialog = sal_False;
        winResTemplateId = TMPL2000_FILESAVE_AUTOEXT_PASSWORD_BOX_ID;
        break;

    case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        bFileOpenDialog = sal_False;
        winResTemplateId = TMPL2000_AUTOEXT_PASSWORD_FILTEROPTION_BOX;
        break;

    case FILESAVE_AUTOEXTENSION_SELECTION:
        bFileOpenDialog = sal_False;
        winResTemplateId = TMPL2000_AUTOEXT_SELECTION_BOX;
        break;

    case FILESAVE_AUTOEXTENSION_TEMPLATE:
        bFileOpenDialog = sal_False;
        winResTemplateId = TMPL2000_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID;
        break;

    case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        winResTemplateId = TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_ID;
        break;

    case FILEOPEN_PLAY:
        winResTemplateId = TMPL2000_PLAY_PUSHBUTTON;
        break;

    case FILEOPEN_READONLY_VERSION:
        winResTemplateId = TMPL2000_FILEOPEN_READONLY_VERSION_BOX_ID;
        break;

    case FILEOPEN_LINK_PREVIEW:
        winResTemplateId = TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID;
        break;

    case FILESAVE_AUTOEXTENSION:
        bFileOpenDialog = sal_False;
        winResTemplateId = TMPL2000_FILESAVE_AUTOEXT;
        break;

    default:
        throw lang::IllegalArgumentException(
            OUString( "Unknown template" ),
            static_cast< XFilePicker2* >( this ),
            1 );
    }

    HINSTANCE hInstance = GetModuleHandle( FILE_PICKER_DLL_NAME );
    OSL_POSTCOND( hInstance, "The name of the service dll must have changed" );

    // create a new impl-class here based on the
    // given string, if the given string is empty
    // we do nothing
    m_pImpl = std::auto_ptr< CWinFileOpenImpl >(
        new CWinFileOpenImpl(
            this,
            bFileOpenDialog,
            0,
            winResTemplateId,
            hInstance ) );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::cancel()
    throw(uno::RuntimeException)
{
    OSL_ASSERT(m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->cancel();
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CFilePicker::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(FILE_PICKER_IMPL_NAME));
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CFilePicker::supportsService(const OUString& ServiceName)
    throw(uno::RuntimeException )
{
    uno::Sequence <OUString> SupportedServicesNames = FilePicker_getSupportedServiceNames();

    for (sal_Int32 n = SupportedServicesNames.getLength(); n--;)
        if (SupportedServicesNames[n] == ServiceName)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

uno::Sequence<OUString> SAL_CALL CFilePicker::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return FilePicker_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
