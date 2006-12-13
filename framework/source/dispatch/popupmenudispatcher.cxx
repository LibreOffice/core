/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: popupmenudispatcher.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:05:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
//#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_POPUPMENUDISPATCHER_HXX_
#include <dispatch/popupmenudispatcher.hxx>
#endif
#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif
#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif
#include <services.h>
#include <properties.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <ucbhelper/content.hxx>
#include <vos/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::com::sun::star                ;
using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::vos                           ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________
const char*     PROTOCOL_VALUE      = "vnd.sun.star.popup:";
const sal_Int32 PROTOCOL_LENGTH     = 19;

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
PopupMenuDispatcher::PopupMenuDispatcher(
    const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        ,   OWeakObject             (                                )
        // Init member
        ,   m_xFactory              ( xFactory                       )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex() )
        ,   m_bAlreadyDisposed      ( sal_False                      )
        ,   m_bActivateListener     ( sal_False                      )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
PopupMenuDispatcher::~PopupMenuDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_7     ( PopupMenuDispatcher                                     ,
                          ::cppu::OWeakObject                                     ,
                          DIRECT_INTERFACE( XTypeProvider                         ),
                          DIRECT_INTERFACE( XServiceInfo                          ),
                          DIRECT_INTERFACE( XDispatchProvider                     ),
                          DIRECT_INTERFACE( XDispatch                             ),
                          DIRECT_INTERFACE( XEventListener                        ),
                          DIRECT_INTERFACE( XInitialization                       ),
                          DERIVED_INTERFACE( XFrameActionListener, XEventListener )
                        )

DEFINE_XTYPEPROVIDER_7  (   PopupMenuDispatcher ,
                            XTypeProvider       ,
                            XServiceInfo        ,
                            XDispatchProvider   ,
                            XDispatch           ,
                            XEventListener      ,
                            XInitialization     ,
                            XFrameActionListener
                        )

DEFINE_XSERVICEINFO_MULTISERVICE( PopupMenuDispatcher                    ,
                                  ::cppu::OWeakObject                    ,
                                  SERVICENAME_PROTOCOLHANDLER            ,
                                  IMPLEMENTATIONNAME_POPUPMENUDISPATCHER )

DEFINE_INIT_SERVICE(PopupMenuDispatcher,
{
    /*Attention
    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
    to create a new instance of this class by our own supported service factory.
    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
    */
}
)

//*****************************************************************************************************************
//  XInitialization
//*****************************************************************************************************************
void SAL_CALL PopupMenuDispatcher::initialize(
    const css::uno::Sequence< css::uno::Any >& lArguments )
throw( css::uno::Exception, css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    for (int a=0; a<lArguments.getLength(); ++a)
    {
        if (a==0)
        {
            lArguments[a] >>= xFrame;
            m_xWeakFrame = xFrame;

            m_bActivateListener = sal_True;
            Reference< css::frame::XFrameActionListener > xFrameActionListener(
                (OWeakObject *)this, css::uno::UNO_QUERY );
            xFrame->addFrameActionListener( xFrameActionListener );
        }
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//*****************************************************************************************************************
//  XDispatchProvider
//*****************************************************************************************************************
css::uno::Reference< css::frame::XDispatch >
SAL_CALL PopupMenuDispatcher::queryDispatch(
    const css::util::URL&  rURL    ,
    const ::rtl::OUString& sTarget ,
    sal_Int32              nFlags  )
throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( rURL.Complete.compareToAscii( PROTOCOL_VALUE, PROTOCOL_LENGTH ) == 0 )
    {
        // --- SAFE ---
        ResetableGuard aGuard( m_aLock );
        impl_RetrievePopupControllerQuery();
        impl_CreateUriRefFactory();

        css::uno::Reference< css::container::XNameAccess > xPopupCtrlQuery( m_xPopupCtrlQuery );
        css::uno::Reference< css::uri::XUriReferenceFactory > xUriRefFactory( m_xUriRefFactory );
        aGuard.unlock();
        // --- SAFE ---

        if ( xPopupCtrlQuery.is() )
        {
            try
            {
                // Just use the main part of the URL for popup menu controllers
                sal_Int32     nQueryPart( 0 );
                sal_Int32     nSchemePart( 0 );
                rtl::OUString aBaseURL( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.popup:" ));
                rtl::OUString aURL( rURL.Complete );

                nSchemePart = aURL.indexOf( ':' );
                if (( nSchemePart > 0 ) &&
                    ( aURL.getLength() > ( nSchemePart+1 )))
                {
                    nQueryPart  = aURL.indexOf( '?', nSchemePart );
                    if ( nQueryPart > 0 )
                        aBaseURL += aURL.copy( nSchemePart+1, nQueryPart-(nSchemePart+1) );
                    else if ( nQueryPart == -1 )
                        aBaseURL += aURL.copy( nSchemePart+1 );
                }

                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider;

                // Find popup menu controller using the base URL
                Any a = xPopupCtrlQuery->getByName( aBaseURL );
                a >>= xDispatchProvider;
                aGuard.unlock();

                // Ask popup menu dispatch provider for dispatch object
                if ( xDispatchProvider.is() )
                    xDispatch = xDispatchProvider->queryDispatch( rURL, sTarget, nFlags );
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
            }
        }
    }
    return xDispatch;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
PopupMenuDispatcher::queryDispatches(
    const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = this->queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::dispatch(
    const URL&                        /*aURL*/            ,
    const Sequence< PropertyValue >&  /*seqProperties*/ )
throw( RuntimeException )
{
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL&                          aURL    )
throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Add listener to container.
    m_aListenerContainer.addInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::removeStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL&                          aURL    )
throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Add listener to container.
    m_aListenerContainer.removeInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//   XFrameActionListener
//*****************************************************************************************************************

void
SAL_CALL PopupMenuDispatcher::frameAction(
    const FrameActionEvent& aEvent )
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING ) ||
        ( aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ))
    {
        // Reset query reference to requery it again next time
        m_xPopupCtrlQuery.clear();
    }
}

//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::disposing( const EventObject& ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    LOG_ASSERT( !(m_bAlreadyDisposed==sal_True), "MenuDispatcher::disposing()\nObject already disposed .. don't call it again!\n" )

    if( m_bAlreadyDisposed == sal_False )
    {
        m_bAlreadyDisposed = sal_True;

        if ( m_bActivateListener )
        {
            Reference< XFrame > xFrame( m_xWeakFrame.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                xFrame->removeFrameActionListener( Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
                m_bActivateListener = sal_False;
            }
        }

        // Forget our factory.
        m_xFactory = Reference< XMultiServiceFactory >();
    }
}

void PopupMenuDispatcher::impl_RetrievePopupControllerQuery()
{
    if ( !m_xPopupCtrlQuery.is() )
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Reference< css::frame::XFrame > xFrame( m_xWeakFrame );

        if ( xFrame.is() )
        {
            css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
            if ( xPropSet.is() )
            {
                try
                {
                    Any a = xPropSet->getPropertyValue( FRAME_PROPNAME_LAYOUTMANAGER );
                    a >>= xLayoutManager;

                    if ( xLayoutManager.is() )
                    {
                        css::uno::Reference< css::ui::XUIElement > xMenuBar;
                        rtl::OUString aMenuBar( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" ));
                        xMenuBar = xLayoutManager->getElement( aMenuBar );

                        m_xPopupCtrlQuery = css::uno::Reference< css::container::XNameAccess >(
                                                xMenuBar, css::uno::UNO_QUERY );
                    }
                }
                catch ( css::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( css::uno::Exception& )
                {
                }
            }
        }
    }
}

void PopupMenuDispatcher::impl_CreateUriRefFactory()
{
    if ( !m_xUriRefFactory.is() )
    {
        rtl::OUString aUriRefFactoryService(
            RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.uri.UriReferenceFactory" ));

        m_xUriRefFactory = css::uno::Reference< css::uri::XUriReferenceFactory >(
                                m_xFactory->createInstance( aUriRefFactoryService ),
                                css::uno::UNO_QUERY);

    }
}

}       //  namespace framework
