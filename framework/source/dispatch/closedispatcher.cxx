/*************************************************************************
 *
 *  $RCSfile: closedispatcher.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:20:58 $
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

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
#include <dispatch/closedispatcher.hxx>
#endif

#ifndef __FRAMEWORK_PATTERN_FRAME_HXX_
#include <pattern/frame.hxx>
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

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

//_______________________________________________
// includes of other projects

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

#ifdef fpf
    #error "Who uses \"fpf\" as define. It will overwrite my namespace alias ..."
#endif
namespace fpf = ::framework::pattern::frame;

//_______________________________________________
// non exported const

static ::rtl::OUString URL_CLOSEDOC    = DECLARE_ASCII(".uno:CloseDoc"  );
static ::rtl::OUString URL_CLOSEWIN    = DECLARE_ASCII(".uno:CloseWin"  );
static ::rtl::OUString URL_CLOSEFRAME  = DECLARE_ASCII(".uno:CloseFrame");

//_______________________________________________
// declarations

DEFINE_XINTERFACE_4(CloseDispatcher                                 ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ),
                    DIRECT_INTERFACE(css::frame::XStatusListener   ))

// Note: XStatusListener is an implementation detail. Hide it for scripting!
DEFINE_XTYPEPROVIDER_3(CloseDispatcher               ,
                       css::lang::XTypeProvider      ,
                       css::frame::XNotifyingDispatch,
                       css::frame::XDispatch         )

//-----------------------------------------------
CloseDispatcher::CloseDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR      ,
                                 const css::uno::Reference< css::frame::XFrame >&              xCloseFrame)
    : ThreadHelpBase     (&Application::GetSolarMutex()                   )
    , ::cppu::OWeakObject(                                                )
    , m_xSMGR            (xSMGR                                           )
    , m_xCloseFrame      (xCloseFrame                                     )
    , m_aAsyncCallback   (LINK( this, CloseDispatcher, impl_asyncCallback))
    , m_lStatusListener  (m_aLock.getShareableOslMutex()                  )
{
}

//-----------------------------------------------
CloseDispatcher::~CloseDispatcher()
{
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                 const css::util::URL&                                     aURL     )
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                    const css::util::URL&                                     aURL     )
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // This reference indicates, that we was already called before and
    // our asynchronous process was not finished yet.
    // We have to reject double calls. Otherwhise we risk,
    // that we try to close an already closed resource ...
    // And its no problem to do nothing then. The UI user will try it again, if
    // non of these jobs was successfully.
    if (m_xSelfHold.is())
    {
        aWriteLock.unlock();
        // <- SAFE ------------------------------

        implts_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::DONTKNOW,
            css::uno::Any());
        return;
    }

    // First we have to check, if this dispatcher is used right. Means if valid URLs are used.
    // If not - we have to break this operation. But an optional listener must be informed.
    // BTW: We save the information about the requested operation. Because
    // we need it later.
    if (aURL.Complete.equals(URL_CLOSEDOC))
        m_eOperation = E_CLOSE_DOC;
    else
    if (aURL.Complete.equals(URL_CLOSEWIN))
        m_eOperation = E_CLOSE_WIN;
    else
    if (aURL.Complete.equals(URL_CLOSEFRAME))
        m_eOperation = E_CLOSE_FRAME;
    else
    {
        aWriteLock.unlock();
        // <- SAFE ------------------------------

        implts_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
        return;
    }

    // OK - URLs are the right ones.
    // But we cant execute synchronously :-)
    // May we are called from a generic key-input handler,
    // which isnt aware that this call kill its own environment ...
    // Do it asynchronous everytimes!

    // But dont forget to hold usself alive.
    // We are called back from an environment, which doesnt know an uno reference.
    // They call us back by using our c++ interface.

    m_xResultListener = xListener;
    m_xSelfHold       = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    m_aAsyncCallback.Post(0);
}

//-----------------------------------------------
/**
    @short  special way to get notifications from the special menu closer.

    @descr  Its not part of the specification of this object doing so.
            But our new layout manager doesnt provide any other mechanis.
            So this special dispatcher is registered for the menu closer as callback ...

    @param  aState
            normaly not needed - because the call itself is enough.
 */
void SAL_CALL CloseDispatcher::statusChanged(const css::frame::FeatureStateEvent& aState)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // Check for still running asynchronous operations, which was started before.
    // Ignore new requests. Our UI user can try it again, if nothing will happen.
    if (m_xSelfHold.is())
        return;

    m_xResultListener.clear();
    m_eOperation = E_CLOSE_WIN;
    m_xSelfHold  = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    m_aAsyncCallback.Post(0);
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::disposing(const css::lang::EventObject& aSource)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("CloseDispatcher::disposing()", "Not allowed to be called. Listener interface is an implementation helper only .-)")
}

//-----------------------------------------------
/**
    @short      asynchronous callback
    @descr      We start all actions inside this object asnychronoue.
                (see comments there).
                Now we do the following:
                - close all views to the same document, if needed and possible
                - make the current frame empty
                  ! This step is neccessary to handle errors during closing the
                    document inside the frame. May the document shows a dialog and
                    the user ignore it. Then the state of the office can be changed
                    during we try to close frame and document.
                - check the environment (menas count open frames - exlcuding our
                  current one)
                - decide then, if we must close this frame only, establish the backing mode
                  or shutdown the whole application.
*/
IMPL_LINK( CloseDispatcher, impl_asyncCallback, void*, pVoid )
{
    // Allow calling of XController->suspend() everytimes.
    // Dispatch is an UI functionality. We implement such dispatch object here.
    // And further XController->suspend() was designed to bring an UI ...
    sal_Bool bAllowSuspend = sal_True;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // Closing of all views, related to the same document, is allowed
    // only if the dispatched URL was ".uno:CloseDoc"!
    sal_Bool bCloseAllViewsToo = (m_eOperation == E_CLOSE_DOC);

    // BTW: Make some copies, which are needed later ...
    EOperation                                                  eOperation  = m_eOperation;
    css::uno::Reference< css::lang::XMultiServiceFactory >      xSMGR       = m_xSMGR;
    css::uno::Reference< css::frame::XFrame >                   xCloseFrame = m_xCloseFrame;
    css::uno::Reference< css::frame::XDispatchResultListener >  xListener   = m_xResultListener;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    sal_Bool bCloseFrame           = sal_False;
    sal_Bool bEstablishBackingMode = sal_False;
    sal_Bool bTerminateApp         = sal_False;

    // Analyze the environment a first time.
    // If we found some special cases, we can
    // make some decisions erliar!
    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
    FrameListAnalyzer aCheck1(xDesktop, xCloseFrame, FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    // a) The help window cant disagree with any request.
    //    Because it doesnt implement a controller - it uses a window only.
    //    Further t cant be the last open frame - if we do all other things
    //    right inside this CloseDispatcher implementation.
    //    => close it!
    if (aCheck1.m_bReferenceIsHelp)
        bCloseFrame = sal_True;
    else

    // b) If we are already in "backing mode", we have to terminate
    //    the application, if this special frame is closed.
    //    It doesnt matter, how many other frames (can be the help or hidden frames only)
    //    are open then.
    //    => terminate the application!
    if (aCheck1.m_bReferenceIsBacking)
        bTerminateApp = sal_True;
    else

    // c) Otherwhise we have to: close all views to the same document, close the
    //    document inside our own frame and decide then again, what has to be done!
    {
        if (implts_closeView(bAllowSuspend, bCloseAllViewsToo))
        {
            // OK; this frame is empty now.
            // Check the environment again to decide, what is the next step.
            FrameListAnalyzer aCheck2(xDesktop, xCloseFrame, FrameListAnalyzer::E_ALL);

            // c1) there is as minimum 1 frame open, which is visible and contains a document
            //     different from our one. And its not the help!
            //     => close our frame only - nothing else.
            if (aCheck2.m_lOtherVisibleFrames.getLength()>0)
                bCloseFrame = sal_True;
            else

            // c2) there is no other (visible) frame open ...
            //     The help module will be ignored everytimes!
            //     But we have to decide if we must terminate the
            //     application or establish the backing mode now.
            //     And that depends from the dispatched URL ...
            {
                if (eOperation == E_CLOSE_FRAME)
                    bTerminateApp = sal_True;
                else
                    bEstablishBackingMode = sal_True;
            }
        }
    }

    // Do it now ...
    sal_Bool bSuccess = sal_False;
    if (bCloseFrame)
        bSuccess = implts_closeFrame();
    else
    if (bEstablishBackingMode)
        bSuccess = implts_establishBackingMode();
    else
    if (bTerminateApp)
        bSuccess = implts_terminateApplication();

    // inform listener
    sal_Int16 nState = css::frame::DispatchResultState::FAILURE;
    if (bSuccess)
        nState = css::frame::DispatchResultState::SUCCESS;
    implts_notifyResultListener(xListener, nState, css::uno::Any());

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // This method was called asynchronous from our main thread by using a pointer.
    // We reached this method only, by using a reference to ourself :-)
    // Further this member is used to detect still running and not yet finished
    // ansynchronous operations. So its time now to release this reference.
    // But hold it temp alive. Otherwhise we die before we can finish this method realy :-))
    css::uno::Reference< css::uno::XInterface > xTempHold = m_xSelfHold;
    m_xSelfHold.clear();
    m_xResultListener.clear();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    return 0;
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_closeView(sal_Bool bAllowSuspend         ,
                                           sal_Bool bCloseAllOtherViewsToo)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = m_xSMGR;
    css::uno::Reference< css::frame::XFrame >              xFrame = m_xCloseFrame;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // If allowed - inform user about modified documents or
    // still running jobs (e.g. printing).
    if (bAllowSuspend)
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if (
            (xController.is()               ) &&  // some views dont uses a controller .-( (e.g. the help window)
            (!xController->suspend(sal_True))
           )
        {
            return sal_False;
        }
    }

    // Close all views to the same document ... if forced to do so.
    // But dont touch our own frame here!
    if (bCloseAllOtherViewsToo)
    {
        css::uno::Reference< css::frame::XFramesSupplier > xDesktop(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
        FrameListAnalyzer aCheck(xDesktop, xFrame, FrameListAnalyzer::E_ALL);

        sal_Int32 c = aCheck.m_lModelFrames.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            if (!fpf::closeIt(aCheck.m_lModelFrames[i], sal_False))
                return sal_False;
        }
    }

    // Make this frame "empty". Means close its view and controller.
    return xFrame->setComponent(
                css::uno::Reference< css::awt::XWindow >(),
                css::uno::Reference< css::frame::XController >());
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_closeFrame()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame > xFrame = m_xCloseFrame;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // dont deliver owner ship; our "UI user" will try it again if it failed.
    // OK - he will get an empty frame then. But normaly an empty frame
    // should be closeable always :-)
    if (!fpf::closeIt(xFrame, sal_False))
        return sal_False;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xCloseFrame.clear();
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    return sal_True;
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_establishBackingMode()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = m_xSMGR;
    css::uno::Reference< css::frame::XFrame >              xFrame = m_xCloseFrame;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::awt::XWindow > xContainerWindow = xFrame->getContainerWindow();
    css::uno::Sequence< css::uno::Any > lArgs(1);
    lArgs[0] <<= xContainerWindow;

    css::uno::Reference< css::frame::XController > xBackingComp(
        xSMGR->createInstanceWithArguments(SERVICENAME_STARTMODULE, lArgs),
        css::uno::UNO_QUERY_THROW);

    // Attention: You MUST(!) call setComponent() before you call attachFrame().
    css::uno::Reference< css::awt::XWindow > xBackingWin(xBackingComp, css::uno::UNO_QUERY);
    xFrame->setComponent(xBackingWin, xBackingComp);
    xBackingComp->attachFrame(xFrame);
    xContainerWindow->setVisible(sal_True);

    return sal_True;
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_terminateApplication()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XDesktop > xDesktop(
        xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);

    return xDesktop->terminate();
}

//-----------------------------------------------
void CloseDispatcher::implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                                        sal_Int16                                                   nState   ,
                                                  const css::uno::Any&                                              aResult  )
{
    if (!xListener.is())
        return;

    css::frame::DispatchResultEvent aEvent(
        css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY),
        nState,
        aResult);

    xListener->dispatchFinished(aEvent);
}

} // namespace framework
