/*************************************************************************
 *
 *  $RCSfile: FilePicker.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:52:13 $
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

#include <tchar.h>

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _FILEPICKER_HXX_
#include "filepicker.hxx"
#endif

#ifndef _WINFILEOPENIMPL_HXX_
#include "WinFileOpenImpl.hxx"
#endif

#include "FPServiceInfo.hxx"
#include "..\misc\WinImplHelper.hxx"

#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif

#ifndef _FILEPICKEREVENTNOTIFICATION_HXX_
#include "filepickereventnotification.hxx"
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star;

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FILE_PICKER_DLL_NAME  TEXT("fps.dll")

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<rtl::OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(2);
        aRet[0] = rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.FilePicker");
        aRet[1] = rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.SystemFilePicker");
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

CFilePicker::CFilePicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr) :
    cppu::WeakComponentImplHelper9<
        XFilterManager,
        XFilterGroupManager,
        XFilePickerControlAccess,
        XFilePickerNotifier,
        XFilePreview,
        lang::XInitialization,
        util::XCancellable,
        lang::XEventListener,
        lang::XServiceInfo>(m_rbHelperMtx),
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
            rtl::OUString::createFromAscii( "object is already disposed" ),
            static_cast< XFilePicker* >( this ) );

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
            rtl::OUString::createFromAscii( "object is already disposed" ),
            static_cast< XFilePicker* >( this ) );

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

rtl::OUString SAL_CALL CFilePicker::helpRequested(FilePickerEvent aEvent) const
{
    rtl::OUString aHelpText;

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

                rtl::OUString temp;

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
                OSL_ENSURE( false, "RuntimeException during event dispatching" );
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

void SAL_CALL CFilePicker::setTitle(const rtl::OUString& aTitle) throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setTitle(aTitle);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::appendFilter(const rtl::OUString& aTitle, const rtl::OUString& aFilter)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->appendFilter(aTitle, aFilter);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setCurrentFilter(const rtl::OUString& aTitle)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setCurrentFilter(aTitle);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL CFilePicker::getCurrentFilter() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getCurrentFilter();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::appendFilterGroup(const rtl::OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters)
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->appendFilterGroup(sGroupTitle, aFilters);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDefaultName(const rtl::OUString& aName)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setDefaultName(aName);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDisplayDirectory(const rtl::OUString& aDirectory)
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setDisplayDirectory(aDirectory);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL CFilePicker::getDisplayDirectory() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getDisplayDirectory();
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL CFilePicker::getFiles() throw(uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());
    osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->getFiles();
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
        OSL_ENSURE(sal_False, "Could not start event notifier thread!");

        throw uno::RuntimeException(
            rtl::OUString::createFromAscii("Error executing dialog"),
            static_cast<XFilePicker*>(this));
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

void SAL_CALL CFilePicker::setLabel(sal_Int16 aControlId, const ::rtl::OUString& aLabel)
    throw (uno::RuntimeException)
{
    OSL_ASSERT(0 != m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->setLabel(aControlId, aLabel);
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL CFilePicker::getLabel(sal_Int16 aControlId)
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
            rtl::OUString::createFromAscii( "no arguments" ),
            static_cast<XFilePicker*>(this), 1);

    aAny = aArguments[0];

    if ( (aAny.getValueType() != ::getCppuType((sal_Int16*)0)) &&
         (aAny.getValueType() != ::getCppuType((sal_Int8*)0)) )
         throw lang::IllegalArgumentException(
            rtl::OUString::createFromAscii("invalid argument type"),
            static_cast<XFilePicker*>(this), 1);

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    sal_Bool   bFileOpenDialog  = sal_True;
    sal_uInt32 winResTemplateId = 0;
    sal_Bool   bIsWin2000       = IsWindows2000Platform();

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
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILESAVE_AUTOEXT_PASSWORD_BOX_ID;
        else
            winResTemplateId = TMPL95_FILESAVE_AUTOEXT_PASSWORD_BOX_ID;
        break;

    case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        bFileOpenDialog = sal_False;
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_AUTOEXT_PASSWORD_FILTEROPTION_BOX;
        else
            winResTemplateId = TMPL95_AUTOEXT_PASSWORD_FILTEROPTION_BOX;
        break;

    case FILESAVE_AUTOEXTENSION_SELECTION:
        bFileOpenDialog = sal_False;
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_AUTOEXT_SELECTION_BOX;
        else
            winResTemplateId = TMPL95_AUTOEXT_SELECTION_BOX;
        break;

    case FILESAVE_AUTOEXTENSION_TEMPLATE:
        bFileOpenDialog = sal_False;
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID;
        else
            winResTemplateId = TMPL95_FILEOPEN_AUTOEXT_TEMPLATE_BOX_ID;
        break;

    case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_ID;
        else
            winResTemplateId = TMPL95_FILEOPEN_LINK_PREVIEW_BOX_ID;
        break;

    case FILEOPEN_PLAY:
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_PLAY_PUSHBUTTON;
        else
            winResTemplateId = TMPL95_PLAY_PUSHBUTTON;
        break;

    case FILEOPEN_READONLY_VERSION:
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILEOPEN_READONLY_VERSION_BOX_ID;
        else
            winResTemplateId = TMPL95_FILEOPEN_READONLY_VERSION_BOX_ID;
        break;

    case FILEOPEN_LINK_PREVIEW:
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID;
        else
            winResTemplateId = TMPL95_FILEOPEN_LINK_PREVIEW_BOX_SIMPLE_ID;
        break;

    case FILESAVE_AUTOEXTENSION:
        bFileOpenDialog = sal_False;
        if ( bIsWin2000 )
            winResTemplateId = TMPL2000_FILESAVE_AUTOEXT;
        else
            winResTemplateId = TMPL95_FILESAVE_AUTOEXT;
        break;

    default:
        throw lang::IllegalArgumentException(
            rtl::OUString::createFromAscii( "Unknown template" ),
            static_cast< XFilePicker* >( this ),
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
{
    OSL_ASSERT(m_pImpl.get());

    osl::MutexGuard aGuard(m_aMutex);
    m_pImpl->cancel();
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL CFilePicker::getImplementationName()
    throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(FILE_PICKER_IMPL_NAME);
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CFilePicker::supportsService(const rtl::OUString& ServiceName)
    throw(uno::RuntimeException )
{
    uno::Sequence <rtl::OUString> SupportedServicesNames = FilePicker_getSupportedServiceNames();

    for (sal_Int32 n = SupportedServicesNames.getLength(); n--;)
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL CFilePicker::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return FilePicker_getSupportedServiceNames();
}
