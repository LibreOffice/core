/*************************************************************************
 *
 *  $RCSfile: closedispatcher.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:15:51 $
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

#ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
#include <dispatch/closedispatcher.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_
#include <classes/framelistanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_TARGETS_H_
#include <targets.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define URL_CLOSEDOC        ".uno:CloseDoc"
#define URL_CLOSEWIN        ".uno:CloseWin"
#define URL_CLOSEFRAME      ".uno:CloseFrame"

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

DEFINE_XINTERFACE_3( CloseDispatcher                                 ,
                     OWeakObject                                     ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                     DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                     DIRECT_INTERFACE(css::frame::XDispatch         )
                   )

DEFINE_XTYPEPROVIDER_3( CloseDispatcher               ,
                        css::lang::XTypeProvider      ,
                        css::frame::XNotifyingDispatch,
                        css::frame::XDispatch
                      )

//_______________________________________________

/**
    @short      standard ctor
    @descr      It initialize a new instance of this class with needed informations for work.

    @param      xSMGR
                    reference to the uno servicemanager for creation of new services

    @param      xTarget
                    reference to our target frame, at which we will work
*/
CloseDispatcher::CloseDispatcher( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR   ,
                                  /*IN*/ const css::uno::Reference< css::frame::XFrame >&              xTarget )
    // init baseclasses first
    : ThreadHelpBase     ( &Application::GetSolarMutex()                     )
    , ::cppu::OWeakObject(                                                   )
    // initialize members
    , m_xSMGR            ( xSMGR                                             )
    , m_xTarget          ( xTarget                                           )
    , m_aAsyncCallback   ( LINK( this, CloseDispatcher, impl_asyncCallback ) )
    , m_lStatusListener  ( m_aLock.getShareableOslMutex()                    )
{
}

//_______________________________________________

/**
    @short      standard dtor
    @descr      Here we have to release all our internal used ressources.
*/
CloseDispatcher::~CloseDispatcher()
{
    m_xSMGR           = css::uno::Reference< css::lang::XMultiServiceFactory >();
    m_xTarget         = css::uno::Reference< css::frame::XFrame >();
    m_xResultListener = css::uno::Reference< css::frame::XDispatchResultListener >();
}

//_______________________________________________

/**
    @short      implementation of XDispatch::dispatch()
    @descr      We don't do the right things here directly. Because the derived interface XNotifyingDispatch
                does the same ... but with an optional listener, we forward this request to this interface
                using an empty listener.

    @see        dispatchWithNotifcation()

    @param      aURL
                    must be one of the following one:
                    <ul>
                        <li>.uno:CloseDoc</li>
                        <li>.uno:CloseWin</li>
                    </ul>

    @param      lArguments
                    optional arguments for this request
*/
void SAL_CALL CloseDispatcher::dispatch( /*IN*/ const css::util::URL&                                  aURL       ,
                                         /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >() );
}

//_______________________________________________

/**
    @short      implementation of XDispatch::addStatusListener()
    @descr      It adds the interested listener to our internal container and register it for the given URL.
                Of course we inform it immediatly about the current state (corresponding to the URL).

    @see        removeStatusListener()

    @param      aURL
                    must be one of the following one:
                    <ul>
                        <li>.uno:CloseDoc</li>
                        <li>.uno:CloseWin</li>
                    </ul>

    @param      xListener
                    the interested listener

    @throw      [com::sun::star::uno::RuntimeException]
                    in case the listener is given as a <NULL/> reference
*/
void SAL_CALL CloseDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                  const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    if (!xListener.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("registration of NULL listener references not very usefull"), xThis);

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_lStatusListener.addInterface(aURL.Complete, xListener);
    css::uno::Reference< css::frame::XFrame > xTarget = m_xTarget;
    aWriteLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XController >  xController;
    css::uno::Reference< css::frame::XModel >       xModel     ;
    css::uno::Reference< css::beans::XPropertySet > xPropSet   (xTarget, css::uno::UNO_QUERY);

    if (xTarget.is())
        xController = xTarget->getController();

    if (xController.is())
        xModel = xController->getModel();

    css::uno::Reference< css::beans::XPropertySet > xSet(xTarget, css::uno::UNO_QUERY);
    sal_Bool bIsTargetAlreadyBackingWindow = sal_False;
    if (xSet.is())
        xSet->getPropertyValue(FRAME_PROPNAME_ISBACKINGMODE)>>=bIsTargetAlreadyBackingWindow;

    sal_Bool bIsEnabled = (
                            ( aURL.Complete.equalsAscii(URL_CLOSEDOC) && ( xModel.is() || xController.is() ) )
                            ||
                            (aURL.Complete.equalsAscii(URL_CLOSEWIN) && !bIsTargetAlreadyBackingWindow)
                          );

    css::frame::FeatureStateEvent aEvent(
        xThis            ,
        aURL             ,
        ::rtl::OUString(),
        bIsEnabled       ,
        sal_False        ,
        css::uno::Any()) ;

    xListener->statusChanged(aEvent);
}

//_______________________________________________

void SAL_CALL CloseDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                     const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    if (!xListener.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("deregistration of NULL listener references not very usefull"), xThis);

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_lStatusListener.removeInterface(aURL.Complete, xListener);
    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/**
    @short      implementation of XNotifyingDispatch::dispatchWithNotification()
    @descr

    @param      aURL
                    must be one of the following one:
                    <ul>
                        <li>.uno:CloseDoc</li>
                        <li>.uno:CloseWin</li>
                    </ul>

    @param      lArguments
                    optional arguments for this request

    @param      xListener
                    an optional listener, which is interested on the results of this dispatch
*/
void SAL_CALL CloseDispatcher::dispatchWithNotification( /*IN*/ const css::util::URL&                                             aURL      ,
                                                         /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                         /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException)
{
    // First we have to check, if this dispatcher is used right. Means if valid URLs are used.
    // If not - we have to break this operation. But an optional listener must be informed.
    sal_Bool bCloseDoc   = aURL.Complete.equalsAscii(URL_CLOSEDOC  );
    sal_Bool bCloseWin   = aURL.Complete.equalsAscii(URL_CLOSEWIN  );
    sal_Bool bCloseFrame = aURL.Complete.equalsAscii(URL_CLOSEFRAME);

    if (
        (!bCloseDoc  ) &&
        (!bCloseWin  ) &&
        (!bCloseFrame)
       )
    {
        impl_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
        return;
    }

    // OK - URLs are the right ones.
    // Handle it at different places.
    if (bCloseDoc)
        impl_dispatchCloseDoc(lArguments, xListener);
    else
    if (bCloseWin)
        impl_dispatchCloseWin(lArguments, xListener);
    else
    if (bCloseFrame)
        impl_dispatchCloseFrame(lArguments, xListener);
}

//_______________________________________________

/**
    @short      handle dispatch for ".uno:CloseDoc"
    @descr      We have to close all views for the same document and of course the document itself.
                If that will close all currently opened frames too, we have to establish the backing window
                mode.

    @param      lArguments
                    the optional arguments of the original dispatch request

    @param      xListener
                    an (optional) interested listener for dispatch result events
*/
void CloseDispatcher::impl_dispatchCloseDoc( /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                             /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // analyze our target frame
    // We must know someting about its internal state (controller/model ...)
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier    (m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFrame >          xTarget           = m_xTarget;
    css::uno::Reference< css::frame::XController >     xTargetController = m_xTarget->getController();
    css::uno::Reference< css::frame::XModel >          xTargetModel      ;
    if (xTargetController.is())
        xTargetModel = xTargetController->getModel();

    aReadLock.unlock();
    /* } SAFE */

    // no model -> no close doc!
    if (!xTargetModel.is())
    {
        if ( xTargetController.is() )
        {
            impl_dispatchCloseWin( lArguments, xListener );
        }
        else
        {
            impl_notifyResultListener(
                xListener,
                css::frame::DispatchResultState::FAILURE,
                css::uno::Any());
        }

        return;
    }

    // Search for all top frames and split by using different criterion.
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        xTarget,
        FrameListAnalyzer::E_ALL);

    // Close the document by closing all views/frames.
    // But don't close the original target frame for this dispatch!
    // Otherwhise our office will may be shutdown automaticly ...
    sal_Int32 nMax = aAnalyzer.m_lModelFrames.getLength();
    for (sal_Int32 f=(nMax-1); f>=0; --f)
    {
        // Call it with FALSE to supress the dialog for "saving modified documents here".
        // Otherwhise it will pop up for every view of this document.
        // Use TRUE for closing of last view only ... and that can be our xTarget frame only!
        css::uno::Reference< css::frame::XFrame > xModelFrame = aAnalyzer.m_lModelFrames[f];
        aAnalyzer.m_lModelFrames.realloc(f);
        if (!impl_closeFrame(xModelFrame, sal_False))
        {
            impl_notifyResultListener(
                xListener,
                css::frame::DispatchResultState::FAILURE,
                css::uno::Any());
            return;
        }
    }

    // Attention: Do any action here asynchronous. Otherwhise e.g. some key handlers of our applications
    // are not aware of some closed ressource ... and can produce a crash :-(
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // exit application?
    // Only possible if we are already in backing state!
    // And then it doesn't matter, if any hidden or help window is open.
    if (aAnalyzer.m_bReferenceIsBacking)
    {
        m_eAsyncOperation = E_EXIT_APP;
    }
    else
    // establish backing mode ?
    // Only allowed, if:
    // - the backing state does not already exist
    // - or if no other visible frame seams to be open.
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==0) &&
        (aAnalyzer.m_lModelFrames.getLength()==0       ) &&
        (!aAnalyzer.m_xBackingComponent.is()           ) &&
        (!aAnalyzer.m_bReferenceIsBacking              )
       )
    {
        m_eAsyncOperation = E_ESTABLISH_BACKINGMODE;
    }
    else
    // close our target frame only.
    // There is no relation to any other frame (which uses the same document)
    // nor is it the backing component or any other special frame.
    {
        m_eAsyncOperation = E_CLOSE_TARGET;
    }
    m_xResultListener = xListener;
    m_xSelfHold       = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    m_aAsyncCallback.Post(0);
    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/**
    @short      handle dispatch for ".uno:CloseWin"
    @descr      We have to close our target frame/view (means its window) only.
                But of course it can be the last open frame and we have to
                establish our backing window.

    @param      lArguments
                    the optional arguments of the original dispatch request

    @param      xListener
                    an (optional) interested listener for dispatch result events
*/
void CloseDispatcher::impl_dispatchCloseWin( /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                             /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // analyze our target frame
    // We must know someting about its internal state (window ...)
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier (m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFrame >          xTarget       = m_xTarget;
    css::uno::Reference< css::awt::XWindow >           xTargetWindow ;
    if (xTarget.is())
        xTargetWindow = xTarget->getContainerWindow();

    aReadLock.unlock();
    /* } SAFE */

    // no window -> no close win!
    if (!xTargetWindow.is())
    {
        impl_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
        return;
    }

    // analyze the list of current open tasks
    // Note: We use an empty model reference - because we are not interested on splitting
    // of model and other frames. We need an information about an open help task here only.
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        xTarget,
        FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_HIDDEN | FrameListAnalyzer::E_BACKINGCOMPONENT);

    // Attention: Do any action here asynchronous. Otherwhise e.g. some key handlers of our applications
    // are not aware of some closed ressource ... and can produce a crash :-(
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // exit application?
    // Only possible if we are already in backing state!
    // And then it doesn't matter, if any hidden or help window is open.
    if (aAnalyzer.m_bReferenceIsBacking)
    {
        m_eAsyncOperation = E_EXIT_APP;
    }
    else
    // establish backing mode ?
    // Only allowed, if:
    // - the backing state does not already exist
    // - or if no other visible frame seams to be open.
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==0) &&
        (!aAnalyzer.m_xBackingComponent.is()           ) &&
        (!aAnalyzer.m_bReferenceIsBacking              )
       )
    {
        m_eAsyncOperation = E_ESTABLISH_BACKINGMODE;
    }
    else
    // close our target frame only.
    // There is no relation to any other frame (which uses the same document)
    // nor is it the backing component or any other special frame.
    {
        m_eAsyncOperation = E_CLOSE_TARGET;
    }
    m_xResultListener = xListener;
    m_xSelfHold       = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    m_aAsyncCallback.Post(0);
    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/**
    @short      handle dispatch for ".uno:CloseFrame"
    @descr      We have to close our target frame/view only.
                But of course it can be the last open frame and we have to
                exit the application then.

    @param      lArguments
                    the optional arguments of the original dispatch request

    @param      xListener
                    an (optional) interested listener for dispatch result events
*/
void CloseDispatcher::impl_dispatchCloseFrame( /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                               /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // analyze our target frame
    // We must know someting about its internal state (window ...)
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier (m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFrame >          xTarget       = m_xTarget;
    css::uno::Reference< css::awt::XWindow >           xTargetWindow ;
    if (xTarget.is())
        xTargetWindow = xTarget->getContainerWindow();

    aReadLock.unlock();
    /* } SAFE */

    // no window -> no close win!
    if (!xTargetWindow.is())
    {
        impl_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
        return;
    }

    // analyze the list of current open tasks
    // Note: We use an empty model reference - because we are not interested on splitting
    // of model and other frames. We need an information about an open help task here only.
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        xTarget,
        FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_HIDDEN | FrameListAnalyzer::E_ZOMBIE);

    // Attention: Do any action here asynchronous. Otherwhise e.g. some key handlers of our applications
    // are not aware of some closed ressource ... and can produce a crash :-(
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // exit application?
    if (aAnalyzer.m_lOtherVisibleFrames.getLength()==0)
    {
        m_eAsyncOperation = E_EXIT_APP;
    }
    else
    // close our target frame only.
    {
        m_eAsyncOperation = E_CLOSE_TARGET;
    }
    m_xResultListener = xListener;
    m_xSelfHold       = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    m_aAsyncCallback.Post(0);
    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/**
    @short      it replaces the current component inside our target frame with a default version
    @descr      This default component shows an empty window and a simple menu. So an user
                can start opening/creation of new documents.

    @attention  But we have to do it asynchronous! Otherwhise it crashes inside the writer module
                e.g. inside some key input handler.

    @see        impl_dispatchCloseDoc()
    @see        impl_dispatchCloseWin()
    @see        impl_dispatchCloseFrame()
*/
IMPL_LINK( CloseDispatcher, impl_asyncCallback, void*, pVoid )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // This method was called asynchronous from our main thread by using a pointer.
    // But we live only yet, because we hold us self alive by an internal memeber reference!
    // This reference must be released now. But we must use another temp. reference to ourself
    // too, to be shure that this method can be finished successfully.
    css::uno::Reference< css::uno::XInterface > xHoldAliveForThisCall = m_xSelfHold;
                                                m_xSelfHold           = css::uno::Reference< css::uno::XInterface >();

    // decide, which operation must be done here
    sal_Bool bSuccess = sal_False;
    if (m_eAsyncOperation == E_ESTABLISH_BACKINGMODE)
        bSuccess = impl_establishBackingMode();
    else
    if (m_eAsyncOperation == E_CLOSE_TARGET)
        bSuccess = impl_closeFrame(m_xTarget, sal_True);
    else
    if (m_eAsyncOperation == E_EXIT_APP)
        bSuccess = impl_exitApp(m_xTarget);

    // notify a possible interested dispatch result listener
    if (bSuccess)
    {
        impl_notifyResultListener(
            m_xResultListener,
            css::frame::DispatchResultState::SUCCESS,
            css::uno::Any());
    }
    else
    {
        impl_notifyResultListener(
            m_xResultListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
    }
    m_xResultListener = css::uno::Reference< css::frame::XDispatchResultListener >();

    aWriteLock.unlock();
    /* } SAFE */

    return 0;
}

sal_Bool CloseDispatcher::impl_establishBackingMode()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    sal_Bool bEstablished = sal_False;

    if (m_xTarget.is())
    {
        css::uno::Reference< css::frame::XController > xController      = m_xTarget->getController();
        css::uno::Reference< css::awt::XWindow >       xContainerWindow = m_xTarget->getContainerWindow();

        // Ask controller for its agreement.
        // In case not - break this operation.
        // We must do that, because we wish to replace it
        // with our default component.
        if (
            (xController.is()              ) &&
            (xController->suspend(sal_True)) &&
            (xContainerWindow.is()         )
           )
        {
            css::uno::Sequence< css::uno::Any > lArgs(1);
            lArgs[0] <<= xContainerWindow;

            css::uno::Reference< css::frame::XController > xBackingComp(
                m_xSMGR->createInstanceWithArguments(DECLARE_ASCII("com.sun.star.comp.sfx2.view.BackingComp"), lArgs),
                css::uno::UNO_QUERY);

            if (xBackingComp.is())
            {
                css::uno::Reference< css::awt::XWindow > xBackingWin(xBackingComp, css::uno::UNO_QUERY);
                m_xTarget->setComponent(xBackingWin, xBackingComp);

                xBackingComp->attachFrame(m_xTarget);
                xContainerWindow->setVisible(sal_True);

                // Don't forget to set right state for this frame. It's in backing mode now.
                // We have to do that AFTER xFrame->setComponent() was called. Because this method
                // reset this state hardly!
                css::uno::Reference< css::beans::XPropertySet > xSet(m_xTarget, css::uno::UNO_QUERY);
                if (xSet.is())
                {
                    xSet->setPropertyValue(DECLARE_ASCII("IsBackingMode"), css::uno::makeAny(sal_True));
                    bEstablished = sal_True;
                }
            }
        }
    }

    aReadLock.unlock();
    /* } SAFE */

    return bEstablished;
}

sal_Bool CloseDispatcher::impl_exitApp( css::uno::Reference< css::frame::XFrame >& xTarget )
{
    // SAFE {
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XDesktop > xDesktop(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // } SAFE

    sal_Bool bTerminated = sal_False;
    if (xDesktop.is())
        bTerminated = xDesktop->terminate();
    if (bTerminated)
        xTarget = css::uno::Reference< css::frame::XFrame >();
    return bTerminated;
}

//_______________________________________________

/**
    @short      try to close the given frame
    @descr      Because there exist many ways to close such frame and this functionality is
                needed at different places, this helper can be called. It returns a boolean value,
                which shows the result.

    @attention  In case operation was successfully, the frame reference will be set to <NULL/>!
                So it can't be used any longer.

    @param      xFrame
                    the frame, which should be closed

    @param      bCallSuspend
                    If we close multiple views to the same document, we wouldn't see any messages which asks:
                    "Whish you to save your modified document?". Such messages comes up by our Controller->suspend() call.
                    The the outside code can call us n times with "bCallSuspend=no" and one time with "...=yes".

    @return     <TRUE/> if frame could be closed successfully, or<br>
                <FALSE/> if not.
*/
sal_Bool CloseDispatcher::impl_closeFrame( /*INOUT*/ css::uno::Reference< css::frame::XFrame >& xFrame       ,
                                           /*IN*/    sal_Bool                                   bCallSuspend )
{
    if (!xFrame.is())
        return sal_True;

    if (bCallSuspend)
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if (
            (xController.is()               ) &&
            (!xController->suspend(sal_True))
           )
        {
            return sal_False;
        }
    }

    sal_Bool bClosed = sal_False;
    css::uno::Reference< css::util::XCloseable >   xCloseable  ( xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::lang::XComponent >   xDisposeable( xFrame, css::uno::UNO_QUERY );
    try
    {
        if (xCloseable.is())
        {
            xCloseable->close(sal_False); // don't deliver the ownership! We will control next closing try!
            bClosed = sal_True;
        }
        else
        if (xDisposeable.is())
        {
            xDisposeable->dispose();
            bClosed = sal_True;
        }
    }
    catch( css::uno::Exception& )
    {
        bClosed = sal_False;
    }

    if (bClosed)
        xFrame = css::uno::Reference< css::frame::XFrame >();

    return bClosed;
}

//_______________________________________________

/**
    @short      notify an optional dispatch result listener
    @descr      This method tries(!) to notify the given listener using the other given parameters.
                We check for valid values here and pack the right event structure.
                So an outside code must do it more then once.

    @param      xListener
                    an (optional) interested listener for dispatch result events
                    Can be null!

    @param      nState
                    the result state for the dispatch request

    @param      aResult
                    the result for the dispatch request
*/
void CloseDispatcher::impl_notifyResultListener( /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                                 /*IN*/       sal_Int16                                                   nState   ,
                                                 /*IN*/ const css::uno::Any&                                              aResult  )
{
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent(
            css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY),
            nState,
            aResult);

        xListener->dispatchFinished(aEvent);
    }
}

} // namespace framework
