/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbarwrapper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-11 14:14:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________


#ifndef __FRAMEWORK_UIELEMENT_TOOLBARWRAPPER_HXX_
#include <uielement/toolbarwrapper.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_ACTIONTRIGGERHELPER_HXX_
#include <helper/actiontriggerhelper.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_CONSTITEMCONTAINER_HXX_
#include <uielement/constitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_ROOTITEMCONTAINER_HXX_
#include <uielement/rootitemcontainer.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#include <uielement/toolbarmanager.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARW_HXX_
#include <uielement/toolbar.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTMENUPEER_HPP_
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XMENUBAR_HPP_
#include <com/sun/star/awt/XMenuBar.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_UIELEMENTTYPE_HPP_
#include <com/sun/star/ui/UIElementType.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/logfile.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

ToolBarWrapper::ToolBarWrapper( const Reference< XMultiServiceFactory >& xServiceManager ) :
    UIConfigElementWrapperBase( UIElementType::TOOLBAR ),
    m_xServiceManager( xServiceManager )
{
}

ToolBarWrapper::~ToolBarWrapper()
{
}

// XInterface
void SAL_CALL ToolBarWrapper::acquire() throw()
{
    UIConfigElementWrapperBase::acquire();
}

void SAL_CALL ToolBarWrapper::release() throw()
{
    UIConfigElementWrapperBase::release();
}

uno::Any SAL_CALL ToolBarWrapper::queryInterface( const uno::Type & rType )
throw( ::com::sun::star::uno::RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( ::com::sun::star::ui::XUIFunctionListener*, this ) );

    if( a.hasValue() )
        return a;

    return UIConfigElementWrapperBase::queryInterface( rType );
}

// XComponent
void SAL_CALL ToolBarWrapper::dispose() throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        ResetableGuard aLock( m_aLock );
        if ( m_bDisposed )
            return;
    }

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
        m_xToolBarManager->dispose();
    m_xToolBarManager.clear();
    m_xConfigSource.clear();
    m_xConfigData.clear();
    m_xToolBarWindow.clear();

    m_bDisposed = sal_True;
}

// XInitialization
void SAL_CALL ToolBarWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        UIConfigElementWrapperBase::initialize( aArguments );

        sal_Bool bPopupMode( sal_False );
        for ( sal_Int32 i = 0; i < aArguments.getLength(); i++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAsciiL( "PopupMode", 9 ))
                {
                    aPropValue.Value >>= bPopupMode;
                    break;
                }
            }
        }

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() && m_xConfigSource.is() )
        {
            // Create VCL based toolbar which will be filled with settings data
            ToolBar* pToolBar = 0;
            ToolBarManager* pToolBarManager = 0;
            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    ULONG nStyles = WB_LINESPACING | WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

                    pToolBar = new ToolBar( pWindow, nStyles );
                    m_xToolBarWindow = VCLUnoHelper::GetInterface( pToolBar );
                    pToolBarManager = new ToolBarManager( m_xServiceManager, xFrame, m_aResourceURL, pToolBar );
                    pToolBar->SetToolBarManager( pToolBarManager );
                    m_xToolBarManager = Reference< XComponent >( static_cast< OWeakObject *>( pToolBarManager ), UNO_QUERY );
                    pToolBar->WillUsePopupMode( bPopupMode );
                }
            }

            try
            {
                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() && pToolBar && pToolBarManager )
                {
                    // Fill toolbar with container contents
                    pToolBarManager->FillToolbar( m_xConfigData );
                    pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
                    pToolBar->EnableCustomize( TRUE );
                    ::Size aActSize( pToolBar->GetSizePixel() );
                    ::Size aSize( pToolBar->CalcWindowSizePixel() );
                    aSize.Width() = aActSize.Width();
                    pToolBar->SetOutputSizePixel( aSize );
                }
            }
            catch ( NoSuchElementException& )
            {
                // No settings in our configuration manager. This means we are
                // a transient toolbar which has no persistent settings.
                m_bPersistent = sal_False;
                if ( pToolBar && pToolBarManager )
                {
                    pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
                    pToolBar->EnableCustomize( TRUE );
                    ::Size aActSize( pToolBar->GetSizePixel() );
                    ::Size aSize( pToolBar->CalcWindowSizePixel() );
                    aSize.Width() = aActSize.Width();
                    pToolBar->SetOutputSizePixel( aSize );
                }
            }
        }
    }
}

// XEventListener
void SAL_CALL ToolBarWrapper::disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    // nothing todo
}

// XUpdatable
void SAL_CALL ToolBarWrapper::update() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
    if ( pToolBarManager )
        pToolBarManager->CheckAndUpdateImages();
}

// XUIElementSettings
void SAL_CALL ToolBarWrapper::updateSettings() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xToolBarManager.is() )
    {
        if ( m_xConfigSource.is() && m_bPersistent )
        {
            try
            {
                ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );

                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() )
                    pToolBarManager->FillToolbar( m_xConfigData );
            }
            catch ( NoSuchElementException& )
            {
            }
        }
        else if ( !m_bPersistent )
        {
            // Transient toolbar: do nothing
        }
    }
}

Reference< XIndexAccess > SAL_CALL ToolBarWrapper::getSettings( sal_Bool bWriteable ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( bWriteable )
        return Reference< XIndexAccess >( static_cast< OWeakObject * >( new RootItemContainer( m_xConfigData ) ), UNO_QUERY );
    else
        return m_xConfigData;
}

void SAL_CALL ToolBarWrapper::setSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xSettings ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( xSettings.is() )
    {
        // Create a copy of the data if the container is not const
        Reference< XIndexReplace > xReplace( xSettings, UNO_QUERY );
        if ( xReplace.is() )
            m_xConfigData = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( xSettings ) ), UNO_QUERY );
        else
            m_xConfigData = xSettings;

        if ( m_xConfigSource.is() && m_bPersistent )
        {
            OUString aResourceURL( m_aResourceURL );
            Reference< XUIConfigurationManager > xUICfgMgr( m_xConfigSource );

            aLock.unlock();

            try
            {
                xUICfgMgr->replaceSettings( aResourceURL, m_xConfigData );
            }
            catch( NoSuchElementException& )
            {
            }
        }
        else if ( !m_bPersistent )
        {
            // Transient toolbar => Fill toolbar with new data
            ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
            if ( pToolBarManager )
                pToolBarManager->FillToolbar( m_xConfigData );
        }
    }
}

//  XUIConfigurationListener
void SAL_CALL ToolBarWrapper::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ToolBarWrapper::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ToolBarWrapper::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
}

// XUIElement interface
Reference< XInterface > SAL_CALL ToolBarWrapper::getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            Window* pWindow = (Window *)pToolBarManager->GetToolBar();
            return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
        }
    }

    return Reference< XInterface >();
}

//XUIFunctionExecute
void SAL_CALL ToolBarWrapper::functionExecute(
    const ::rtl::OUString& aUIElementName,
    const ::rtl::OUString& aCommand )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
            pToolBarManager->notifyRegisteredControllers( aUIElementName, aCommand );
    }
}

void SAL_CALL ToolBarWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any&  aValue ) throw( com::sun::star::uno::Exception )
{
    ResetableGuard aLock( m_aLock );
    sal_Bool bNoClose( m_bNoClose );
    aLock.unlock();

    UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast( nHandle, aValue );

    aLock.lock();

    sal_Bool bNewNoClose( m_bNoClose );
    if ( m_xToolBarManager.is() && !m_bDisposed && ( bNewNoClose != bNoClose ))
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            ToolBox* pToolBox = pToolBarManager->GetToolBar();
            if ( pToolBox )
            {
                if ( bNewNoClose )
                {
                    pToolBox->SetStyle( pToolBox->GetStyle() & ~WB_CLOSEABLE );
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() & ~WB_CLOSEABLE );
                }
                else
                {
                    pToolBox->SetStyle( pToolBox->GetStyle() | WB_CLOSEABLE );
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() | WB_CLOSEABLE );
                }
            }
        }
    }
}

} // namespace framework
