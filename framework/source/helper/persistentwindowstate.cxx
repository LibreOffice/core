/*************************************************************************
 *
 *  $RCSfile: persistentwindowstate.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-07-29 08:18:00 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_COMPONENTLOADER_HXX_
#include <helper/componentloader.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
#include <helper/persistentwindowstate.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ARGUMENTANALYZER_HXX_
#include <classes/argumentanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICXEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define DOCSERVICE_WRITER            DECLARE_ASCII("com.sun.star.text.TextDocument"                )
#define DOCSERVICE_WRITERWEB         DECLARE_ASCII("com.sun.star.text.WebDocument"                 )
#define DOCSERVICE_WRITERGLOBAL      DECLARE_ASCII("com.sun.star.text.GlobalDocument"              )
#define DOCSERVICE_CALC              DECLARE_ASCII("com.sun.star.sheet.SpreadsheetDocument"        )
#define DOCSERVICE_DRAW              DECLARE_ASCII("com.sun.star.drawing.DrawingDocument"          )
#define DOCSERVICE_IMPRESS           DECLARE_ASCII("com.sun.star.presentation.PresentationDocument")
#define DOCSERVICE_MATH              DECLARE_ASCII("com.sun.star.formula.FormulaProperties"        )
#define DOCSERVICE_CHART             DECLARE_ASCII("com.sun.star.chart.ChartDocument"              )

//_________________________________________________________________________________________________________________
//  non exported declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_5( PersistentWindowState                             ,
                     OWeakObject                                       ,
                     DIRECT_INTERFACE (css::lang::XTypeProvider        ),
                     DIRECT_INTERFACE (css::lang::XInitialization      ),
                     DIRECT_INTERFACE (css::frame::XFrameActionListener),
                     DIRECT_INTERFACE (css::util::XCloseListener       ),
                     DERIVED_INTERFACE(css::lang::XEventListener,css::frame::XFrameActionListener))

DEFINE_XTYPEPROVIDER_5( PersistentWindowState            ,
                        css::lang::XTypeProvider         ,
                        css::lang::XInitialization       ,
                        css::frame::XFrameActionListener ,
                        css::util::XCloseListener        ,
                        css::lang::XEventListener        )

/*-****************************************************************************************************//**
    @short      standard ctor
    @descr      It initialize a new instance of this class with some - not all(!) - informations we need later.
                Here it's a reference to an uno service manager, which we can use to create own service if neccessary.
                Real initiialization with the frame, on which we should listen for events, will be done using
                XInitialization.initialize().

    @seealso    initialize()

    @param      xFactory
                    reference to an uno service manager

    @threadsafe not neccessary, its not a singleton
    @modified   26.07.2002 15:00, as96863
*//*-*****************************************************************************************************/
PersistentWindowState::PersistentWindowState( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , m_xFactory    ( xFactory                      )
        , m_sWindowState( ::rtl::OUString()             )
{
}

/*-****************************************************************************************************//**
    @short      standard dtor
    @descr      Nothing to do here! Because all reactions has to be finished during calls disposing()
                or queryClosing()/notifyClosing(). If we reach this point without called methods (named before)
                it's an error!

    @threadsafe not neccessary, its not a singleton
    @modified   02.07.2002 10:07, as96863
*//*-*****************************************************************************************************/
PersistentWindowState::~PersistentWindowState()
{
    LOG_ASSERT2( m_xFrame.is(), "PersistentWindowState::~PersistentWindowState()", "Unsupported way to close this object! let it crash :-)" )
}

/*-****************************************************************************************************//**
    @interface  XInitialization
    @short      initialize this instance with the frame, on which we should listen for events
    @descr      Here we get the frame, on which we should restore it or make the window data persistent.
                To do so, we register us as listener for FrameActionEvents and Closing events.

    @param      lArguments
                    first any inside list must include a valid frame reference, which we need for registration

    @throws     [css::uno::Exception]
                    if no or no valid frame can be found

    @threadsafe yes
    @modified   26.07.2002 15:04, as96863
*//*-*****************************************************************************************************/
void SAL_CALL PersistentWindowState::initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw (css::uno::Exception       ,
                                                                                                                css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    // reject wrong initialization calls by throwing right exception
    if (
        ( lArguments.getLength() < 1      ) ||
        ( ! (lArguments[0] >>= m_xFrame ) ) ||
        ( ! m_xFrame.is()                 )
       )
    {
        throw css::uno::Exception( DECLARE_ASCII("No or no valid frame was given!"), static_cast< ::cppu::OWeakObject* >(this) );
    }

    // otherwise start listening immediately
    css::uno::Reference< css::frame::XFrame >           xActionBroadcaster = m_xFrame;
    css::uno::Reference< css::util::XCloseBroadcaster > xCloseBroadcaster  ( m_xFrame, css::uno::UNO_QUERY );
    aWriteLock.unlock();
    /* } SAFE */

    if ( ! xCloseBroadcaster.is())
        throw css::uno::Exception( DECLARE_ASCII("Frame doesn't provide the right interface XCloseable!"), static_cast< ::cppu::OWeakObject* >(this) );

    xActionBroadcaster->addFrameActionListener(this);
    xCloseBroadcaster->addCloseListener       (this);
}

/*-****************************************************************************************************//**
    @interface  XFrameActionListener
    @short      callback to know, if a document was loaded or changed inside the frame
    @descr      The notification COMPONENT_ATTACHED can be used to know, that the first
                document was loaded into this frame. Because we got the frame directly after his
                creation and he doesn't included a valid document there - we wasn't able to restore
                may existing peristent window data on it. Because we must know the application module to do so.
                But without a document we can't specify it. But this event let us know - this is the first
                document.

                But note: further calls of this callback must be ignored. Because we have to restore
                this data one times only for the same frame. Best way do reach that is to deregister us as
                listener for frame action events!

    @param      aEvent
                    must be the frame, on which we listen
                    And it should include the required action COMPONENT_ATTACHED too.
                    In case there comes some other ones (e.g. FRAME_DEACTIVATING) it indicates
                    that the frame wasn't realy used for loading or the load request failed.
                    Then it dies and of course - because we are registered - we get two callbacks.
                    One for closing the frame and one for deactivation.
                    Ignore it here! Or wait for dispose.

    @threadsafe yes
    @modified   26.07.2002 15:38, as96863
*//*-*****************************************************************************************************/
void SAL_CALL PersistentWindowState::frameAction( const css::frame::FrameActionEvent& aEvent ) throw (css::uno::RuntimeException)
{
    if (aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED)
    {
        /* SAFE { */
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
        css::uno::Reference< css::frame::XFrame >              xFrame   = m_xFrame  ;
        aReadLock.unlock();
        /* } SAFE */

        // We must be informed from our internal set frame. Otherwhise we was registered
        // from any outside code. But that's not allowed and I think, we must't react then right.
        // We work on our internal frame only!
        if (aEvent.Frame!=xFrame)
        {
            LOG_WARNING("PersistentWindowState::frameAction()", "Notified fom unknown broadcaster! Will be ignored ...")
            return;
        }

        css::uno::Reference< css::awt::XWindow > xWindow;
        SvtModuleOptions::EFactory               eModule;
        if (PersistentWindowState::implst_getFrameProps(xFactory,xFrame,&eModule,&xWindow))
        {
            // Note: It's not neccessary to save the current window state internaly!
            // Because we restore it here only. The member m_sWindowState is neccessary
            // for saving the value only. see interface XCloseListener for further informations!
            PersistentWindowState::implst_setWindowState(xWindow,SvtModuleOptions().GetFactoryWindowAttributes(eModule));

            // We should deregister us only for successfull restored window states.
            // F.e: In case we was coupled with a frame and a component inside, which
            // doesn't match any supported application module (e.g. basic) then we have normaly nothing to do.
            // But if some times later the same frame is used for a well known factory (e.g. writer), then
            // we can do our job here. So we should be registered till this moment comes .-)
            xFrame->removeFrameActionListener(this);
        }
    }
}

/*-****************************************************************************************************//**
    @interface  XCloseListener
    @short      callback to prepare closing of the listened frame
    @descr      If somewhere tries to close the frame we listen on, we are called to agree or throw the
                right veto exception. We have not veto for that. But we use the chance to get all
                neccessary informations we nee to make the current window state persistent.
                The reason: If we get the callback notifyClosing() it can be to late. Because then may the
                component of the frame was already closed. But we need the type of to know which entry
                inside the configuration package mustg be used. We save window states corresponding to
                application modules.

    @param      aSource
                    describe the source of this event
                    Must be our frame on which we listen. Otherwhise it's an error.

    @param      bGetsOwnerShip
                    can be ignored. Because we never throw the veto exception, there will no reason
                    to be the new owner of this frame and close it later.

    @threadsafe yes
    @modified   26.07.2002 15:54, as96863
*//*-*****************************************************************************************************/
void SAL_CALL PersistentWindowState::queryClosing ( const css::lang::EventObject& aSource        ,
                                                          sal_Bool                bGetsOwnership ) throw (css::util::CloseVetoException,
                                                                                                          css::uno::RuntimeException   )
{
    // reset our internal state values.
    // An empty state string means: no information available and saving inside notifyClosing() not neccessary!
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory       = m_xFactory;
    css::uno::Reference< css::frame::XFrame >              xFrame         = m_xFrame;
                                                           m_sWindowState = ::rtl::OUString();
    aWriteLock.unlock();
    /* } SAFE */

    // We must be informed from our internal set frame. Otherwhise we was registered
    // from any outside code. But that's not allowed and I think, we must't react then right.
    // We work on our internal frame only!
    if (aSource.Source!=xFrame)
    {
        LOG_WARNING("PersistentWindowState::queryClosing()", "Notified fom unknown broadcaster! Will be ignored ...")
        return;
    }

    css::uno::Reference< css::awt::XWindow > xWindow;
    SvtModuleOptions::EFactory               eModule;
    if (PersistentWindowState::implst_getFrameProps(xFactory,xFrame,&eModule,&xWindow))
    {
        ::rtl::OUString sWindowState = PersistentWindowState::implst_getWindowState(xWindow);
        // save it temp. till notifyClosing() is called!
        // Then we can make it persistent inside the configuration package.
        if (sWindowState.getLength() > 0)
        {
            /* SAFE { */
            aWriteLock.lock();
            m_sWindowState = sWindowState;
            m_eFactory     = eModule;
            aWriteLock.unlock();
            /* } SAFE */
        }
    }
}

/*-****************************************************************************************************//**
    @interface  XCloseListener
    @short      callback to react for closing of a frame
    @descr      Now it's time to make the collected information persistent. (getted inside queryClosing())
                We use the config item on top of the configuration package org.openoffice.Setup for saving.

                By the way: We accept callbacks from our internal save frame only! All other notifications
                are an error and will be ignored.

    @param      aSource
                    describe the source of this event
                    Must be our frame on which we listen. Otherwhise it's an error.

    @threadsafe yes
    @modified   02.07.2002 11:41, as96863
*//*-*****************************************************************************************************/
void SAL_CALL PersistentWindowState::notifyClosing( const css::lang::EventObject& aSource ) throw (css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    css::uno::Reference< css::frame::XFrame > xFrame         = m_xFrame      ;
    SvtModuleOptions::EFactory                eFactory       = m_eFactory    ;
    ::rtl::OUString                           sWindowState   = m_sWindowState;
    // Note: We reset the window state member here. So a second call of this method don't do anything!
                                              m_sWindowState = ::rtl::OUString();
    aWriteLock.unlock();
    /* } SAFE */

    // We must be informed from our internal set frame. Otherwhise we was registered
    // from any outside code. But that's not allowed and I think, we must't react then right.
    // We work on our internal frame only!
    if (aSource.Source!=xFrame)
    {
        LOG_WARNING("PersistentWindowState::notifyClosing()", "Notified fom unknown broadcaster! Will be ignored ...")
        return;
    }

    if (sWindowState.getLength() > 0)
        SvtModuleOptions().SetFactoryWindowAttributes(eFactory, sWindowState);
}

/*-****************************************************************************************************//**
    @interface  XEventListener
    @short      callback for disposing of the frame
    @descr      If the frame will be disposed() we must forget his reference. Otherwhise he can't die realy.
                This method is called in every case. It doesn't matter if XCloseListener mechanism was used
                before or not. But it's not a good idea to try to get the window state here. Because at this
                time the frame has guaranteed no component inside. It's to late!

                And further its not neccessary to deregister us as listener. Because the frame whish to die
                and will forget us automaticly. But we must forget him too. Relese his reference only here.

    @param      aSource
                    describe the source of this event
                    Must be our frame on which we listen. Otherwhise it's an error.

    @threadsafe yes
    @modified   26.07.2002 15:56, as96863
*//*-*****************************************************************************************************/
void SAL_CALL PersistentWindowState::disposing( const css::lang::EventObject& aSource ) throw (css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    if (aSource.Source==m_xFrame)
        m_xFrame = css::uno::Reference< css::frame::XFrame >();
    else
    if (aSource.Source==m_xFactory)
        m_xFactory = css::uno::Reference< css::lang::XMultiServiceFactory >();
    m_sWindowState = ::rtl::OUString();
    aWriteLock.unlock();
    /* } SAFE */
}

/*-****************************************************************************************************//**
    @short      find out the document service of the current framec omponent
    @descr      We return the docuemnt service name amd the window. So the outside code can decide,
                which application module currently is used and using the window it's possible to set/get
                the corresponding window state.

    @param      xFactory [in]
                    the uno service manager - neccessary ressource for internal work

    @param      xFrame [in]
                    the frame, on which the required informations exist

    @param      pModule [out]
                    returns the type of used application module
                    Value is undefined if method returns <FALSE/>

    @param      pContainerWindow [out]
                    returns the container window of the frame
                    Value is undefined if method returns <FALSE/>

    @returns    [boolean]
                    <TRUE/> if all requred informations could be found and are valid.
                    <FALSE/> otherwhise (Note: The out-parameter wasn't used then!)

    @threadsafe yes
    @modified   29.07.2002 08:21, as96863
*//*-*****************************************************************************************************/
sal_Bool PersistentWindowState::implst_getFrameProps( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory         ,
                                                      const css::uno::Reference< css::frame::XFrame >&              xFrame           ,
                                                            SvtModuleOptions::EFactory*                             pModule          ,
                                                            css::uno::Reference< css::awt::XWindow >*               pContainerWindow )
{
    if ( ! xFrame.is())
        return sal_False;

    css::uno::Reference< css::awt::XWindow >       xContainerWindow = xFrame->getContainerWindow();
    css::uno::Reference< css::frame::XController > xController      = xFrame->getController();

    // may its an empty frame - can occure only in case first load request failed
    // and framework try to close this frame immediatly
    // But then we hav nothing to do here!
    if ( ! xContainerWindow.is() || ! xController.is() )
        return sal_False;

    ::rtl::OUString sDocumentService = ComponentLoader::specifyDocServiceByDocument(xFactory,xController);
    if (sDocumentService.getLength() < 1)
        return sal_False;

    SvtModuleOptions::EFactory eModule;
    if ( ! SvtModuleOptions::ClassifyFactoryByName(sDocumentService,eModule))
        return sal_False;

    *pModule          = eModule;
    *pContainerWindow = xContainerWindow;

    return sal_True;
}

/*-****************************************************************************************************//**
    @short      get/set the current window state from the given window
    @descr      Because the toolkit interface XWindow doesn't provide this information - we try
                to gte the implementation window behind that and use it.

    @param      xWindow [in]
                    the window, on which we try to ask

    @param      sWindowState [in]
                    the new window state to set it

    @returns    [string]
                    the current window state of the given window

    @threadsafe yes
    @modified   29.07.2002 08:23, as96863
*//*-*****************************************************************************************************/
::rtl::OUString PersistentWindowState::implst_getWindowState( const css::uno::Reference< css::awt::XWindow >& xWindow )
{
    if ( ! xWindow.is())
        return ::rtl::OUString();

    ByteString sWindowState;

    /* SOLAR SAFE { */
    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    // check for system window is neccessary to guarantee correct pointer cast!
    if (pWindow!=NULL && pWindow->IsSystemWindow())
        sWindowState = ((SystemWindow*)pWindow)->GetWindowState();
    aSolarGuard.clear();
    /* } SOLAR SAFE */

    return B2U_ENC(sWindowState,RTL_TEXTENCODING_UTF8);
}

//*********************************************************************************************************
void PersistentWindowState::implst_setWindowState( const css::uno::Reference< css::awt::XWindow >& xWindow      ,
                                                   const ::rtl::OUString&                          sWindowState )
{
    if (! xWindow.is() || sWindowState.getLength() < 1)
        return;

    /* SOLAR SAFE { */
    ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    // check for system window is neccessary to guarantee correct pointer cast!
    if (pWindow!=NULL && pWindow->IsSystemWindow())
        ((SystemWindow*)pWindow)->SetWindowState(U2B_ENC(sWindowState,RTL_TEXTENCODING_UTF8));
    aSolarGuard.clear();
    /* } SOLAR SAFE */
}

} // namespace framework
