/*************************************************************************
 *
 *  $RCSfile: FilePicker.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: tra $ $Date: 2001-08-24 08:58:17 $
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

#include "FPServiceInfo.hxx"
#include "..\misc\WinImplHelper.hxx"

#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::XEventListener;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::lang::XInitialization;
using ::com::sun::star::lang::EventObject;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

using ::cppu::WeakComponentImplHelper7;
using ::rtl::OUString;
using ::osl::MutexGuard;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FILE_PICKER_DLL_NAME  "fps.dll"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    Sequence< OUString > SAL_CALL FilePicker_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(2);
        aRet[0] = OUString::createFromAscii("com.sun.star.ui.dialogs.FilePicker");
        aRet[1] = OUString::createFromAscii("com.sun.star.ui.dialogs.SystemFilePicker");
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

CFilePicker::CFilePicker( const Reference< XMultiServiceFactory >& xServiceMgr ) :
    WeakComponentImplHelper7<
        XFilterManager,
        XFilePickerControlAccess,
        XFilePickerNotifier,
        XFilePreview,
        XInitialization,
        XEventListener,
        XServiceInfo >( m_rbHelperMtx )
{
    HINSTANCE hInstance = GetModuleHandleA( FILE_PICKER_DLL_NAME );
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

void SAL_CALL CFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException(
            OUString::createFromAscii( "object is already disposed" ),
            static_cast< XFilePicker* >( this ) );

    if ( !rBHelper.bInDispose && !rBHelper.bDisposed )
        rBHelper.aLC.addInterface( getCppuType( &xListener ), xListener );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& xListener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException(
            OUString::createFromAscii( "object is already disposed" ),
            static_cast< XFilePicker* >( this ) );

    rBHelper.aLC.removeInterface( getCppuType( &xListener ), xListener );
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL CFilePicker::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
    Reference< XFilePickerListener > xFilePickerListener( aEvent.Source, ::com::sun::star::uno::UNO_QUERY );

    if ( xFilePickerListener.is( ) )
        removeFilePickerListener( xFilePickerListener );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::fileSelectionChanged( FilePickerEvent aEvent ) const
{
    PFNCXFPLISTENER pfncFPListener = &XFilePickerListener::fileSelectionChanged;
    aEvent.Source = Reference< XInterface > (
        static_cast< XFilePickerNotifier* >(
            const_cast< CFilePicker* >( this ) ) );
    notifyAllListener( pfncFPListener, aEvent );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::directoryChanged( FilePickerEvent aEvent ) const
{
    PFNCXFPLISTENER pfncFPListener = &XFilePickerListener::directoryChanged;
    aEvent.Source = Reference< XInterface > (
        static_cast< XFilePickerNotifier* >(
            const_cast< CFilePicker* >( this ) ) );
    notifyAllListener( pfncFPListener, aEvent );
}

//-----------------------------------------------------------------------------------------
// If there are more then one listener the return value of the last one wins
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::helpRequested( FilePickerEvent aEvent ) const
{
    OUString aHelpText;

    if ( !rBHelper.bDisposed )
    {
        ::osl::ClearableMutexGuard aGuard( rBHelper.rMutex );

        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            ::cppu::OInterfaceContainerHelper* pICHelper =
                rBHelper.aLC.getContainer( getCppuType( ( Reference< XFilePickerListener > * ) 0 ) );

            if ( pICHelper )
            {
                ::cppu::OInterfaceIteratorHelper iter( *pICHelper );

                while( iter.hasMoreElements( ) )
                {
                    try
                    {
                        /*
                            if there are multiple listeners responding
                            to this notification the next response
                            overwrittes  the one before if it is not empty
                        */

                        OUString temp;

                        Reference< XFilePickerListener > xFPListener( iter.next( ), ::com::sun::star::uno::UNO_QUERY );
                        if ( xFPListener.is( ) )
                        {
                            temp = xFPListener->helpRequested( aEvent );
                            if ( temp.getLength( ) )
                                aHelpText = temp;
                        }

                    }
                    catch( RuntimeException& )
                    {
                        OSL_ENSURE( false, "RuntimeException during event dispatching" );
                    }
                }
            }
        }
    }

    return aHelpText;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::controlStateChanged( FilePickerEvent aEvent ) const
{
    PFNCXFPLISTENER pfncFPListener = &XFilePickerListener::controlStateChanged;
    aEvent.Source = Reference< XInterface > (
        static_cast< XFilePickerNotifier* >(
            const_cast< CFilePicker* >( this ) ) );
    notifyAllListener( pfncFPListener, aEvent );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::dialogSizeChanged( ) const
{
    // not yet implemented
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::notifyAllListener( PFNCXFPLISTENER pfncFPListener, FilePickerEvent aEvent ) const
{
    OSL_ASSERT( 0 != pfncFPListener );

    if ( !rBHelper.bDisposed )
    {
        ::osl::ClearableMutexGuard aGuard( rBHelper.rMutex );

        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            ::cppu::OInterfaceContainerHelper* pICHelper =
                rBHelper.aLC.getContainer( getCppuType( ( Reference< XFilePickerListener > * ) 0 ) );

            if ( pICHelper )
            {
                ::cppu::OInterfaceIteratorHelper iter( *pICHelper );

                while( iter.hasMoreElements( ) )
                {
                    try
                    {
                        Reference< XFilePickerListener > xFPListener( iter.next( ), ::com::sun::star::uno::UNO_QUERY );
                        if ( xFPListener.is( ) )
                            (xFPListener.get( )->*pfncFPListener)(aEvent);
                    }
                    catch( RuntimeException& )
                    {
                        OSL_ENSURE( false, "RuntimeException during event dispatching" );
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setMultiSelectionMode( bMode );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setTitle( const OUString& aTitle ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setTitle( aTitle );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::appendFilter( const OUString& aTitle, const OUString& aFilter )
    throw(IllegalArgumentException, RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->appendFilter( aTitle, aFilter );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setCurrentFilter( const OUString& aTitle )
    throw(IllegalArgumentException, RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setCurrentFilter( aTitle );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getCurrentFilter( ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pImpl->getCurrentFilter( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDefaultName( const OUString& aName )
    throw(IllegalArgumentException, RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setDefaultName( aName );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setDisplayDirectory( const OUString& aDirectory )
    throw(IllegalArgumentException, RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setDisplayDirectory( aDirectory );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getDisplayDirectory(  ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pImpl->getDisplayDirectory( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

Sequence< OUString > SAL_CALL CFilePicker::getFiles( ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pImpl->getFiles( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CFilePicker::execute( ) throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );

    // we should not block in this call else
    // in the case of an event the client can'tgetPImplFromHandle( hWnd )
    // call another function an we run into a
    // deadlock !!!!!
    return m_pImpl->execute( );
}

//------------------------------------------------------------------------------------
// XFilePicker functions
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const Any& aValue )
    throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    m_pImpl->setValue( aControlId, aControlAction, aValue );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

Any  SAL_CALL CFilePicker::getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
    throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    return m_pImpl->getValue( aControlId, aControlAction );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::enableControl( sal_Int16 aControlId, sal_Bool bEnable )
    throw(RuntimeException)
{
    OSL_ASSERT( 0 != m_pImpl.get( ) );
    m_pImpl->enableControl( aControlId, bEnable );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_pImpl->setLabel( aControlId, aLabel );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFilePicker::getLabel( sal_Int16 aControlId )
    throw (RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    return m_pImpl->getLabel( aControlId );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

Sequence< sal_Int16 > SAL_CALL CFilePicker::getSupportedImageFormats(  ) throw (RuntimeException)
{
    return m_pImpl->getSupportedImageFormats( );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getTargetColorDepth(  ) throw (RuntimeException)
{
    return m_pImpl->getTargetColorDepth( );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getAvailableWidth(  ) throw (RuntimeException)
{
    return m_pImpl->getAvailableWidth( );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilePicker::getAvailableHeight(  ) throw (RuntimeException)
{
    return m_pImpl->getAvailableHeight( );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::setImage( sal_Int16 aImageFormat, const Any& aImage )
    throw (IllegalArgumentException, RuntimeException)
{
    m_pImpl->setImage( aImageFormat, aImage );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilePicker::setShowState( sal_Bool bShowState ) throw (RuntimeException)
{
    return m_pImpl->setShowState( bShowState );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilePicker::getShowState(  ) throw (RuntimeException)
{
    return m_pImpl->getShowState( );
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void SAL_CALL CFilePicker::initialize( const Sequence< Any >& aArguments )
    throw( Exception, RuntimeException)
{
    // parameter checking
    Any aAny;
    if ( 0 == aArguments.getLength( ) )
        throw IllegalArgumentException(
            OUString::createFromAscii( "no arguments" ),
            static_cast< XFilePicker* >( this ),
            1 );

    aAny = aArguments[0];

    if ( (aAny.getValueType() != ::getCppuType((sal_Int16*)0)) &&
         (aAny.getValueType() != ::getCppuType((sal_Int8*)0)) )
        throw IllegalArgumentException(
            OUString::createFromAscii( "invalid argument type" ),
            static_cast< XFilePicker* >( this ),
            1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    sal_Bool   bFileOpenDialog  = sal_True;
    sal_uInt32 winResTemplateId = 0;
    sal_Bool   bIsWin2000       = IsWin2000( );

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
        throw IllegalArgumentException(
            OUString::createFromAscii( "Unknown template" ),
            static_cast< XFilePicker* >( this ),
            1 );
    }

    HINSTANCE hInstance = GetModuleHandleA( FILE_PICKER_DLL_NAME );
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

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CFilePicker::getImplementationName( )
    throw( RuntimeException )
{
    return OUString::createFromAscii( FILE_PICKER_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CFilePicker::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = FilePicker_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CFilePicker::getSupportedServiceNames( )
    throw( RuntimeException )
{
    return FilePicker_getSupportedServiceNames();
}