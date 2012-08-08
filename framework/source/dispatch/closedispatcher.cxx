/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#include <dispatch/closedispatcher.hxx>
#include <pattern/frame.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <framework/framelistanalyzer.hxx>
#include <services.h>
#include <general.h>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include "vcl/syswin.hxx"
#include <osl/mutex.hxx>
#include <unotools/moduleoptions.hxx>

//_______________________________________________
// namespace

using namespace com::sun::star;

namespace framework{

#ifdef fpf
    #error "Who uses \"fpf\" as define. It will overwrite my namespace alias ..."
#endif
namespace fpf = ::framework::pattern::frame;

const char URL_CLOSEDOC[] = ".uno:CloseDoc";
const char URL_CLOSEWIN[] = ".uno:CloseWin";
const char URL_CLOSEFRAME[] = ".uno:CloseFrame";

//_______________________________________________
// declarations

DEFINE_XINTERFACE_4(CloseDispatcher                                           ,
                    OWeakObject                                               ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider                ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch          ),
                    DIRECT_INTERFACE(css::frame::XDispatch                   ),
                    DIRECT_INTERFACE(css::frame::XDispatchInformationProvider))

// Note: XStatusListener is an implementation detail. Hide it for scripting!
DEFINE_XTYPEPROVIDER_4(CloseDispatcher                         ,
                       css::lang::XTypeProvider                ,
                       css::frame::XDispatchInformationProvider,
                       css::frame::XNotifyingDispatch          ,
                       css::frame::XDispatch                   )

//-----------------------------------------------
CloseDispatcher::CloseDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                                 const css::uno::Reference< css::frame::XFrame >&              xFrame ,
                                 const ::rtl::OUString&                                        sTarget)
    : ThreadHelpBase     (&Application::GetSolarMutex()                   )
    , ::cppu::OWeakObject(                                                )
    , m_xSMGR            (xSMGR                                           )
    , m_aAsyncCallback   (LINK( this, CloseDispatcher, impl_asyncCallback))
    , m_lStatusListener  (m_aLock.getShareableOslMutex()                  )
    , m_pSysWindow(NULL)
{
    uno::Reference<frame::XFrame> xTarget = static_impl_searchRightTargetFrame(xFrame, sTarget);
    m_xCloseFrame = xTarget;

    // Try to retrieve the system window instance of the closing frame.
    uno::Reference<awt::XWindow> xWindow = xTarget->getContainerWindow();
    if (xWindow.is())
    {
        Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
        if (pWindow->IsSystemWindow())
            m_pSysWindow = dynamic_cast<SystemWindow*>(pWindow);
    }
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
css::uno::Sequence< sal_Int16 > SAL_CALL CloseDispatcher::getSupportedCommandGroups()
    throw(css::uno::RuntimeException)
{
    css::uno::Sequence< sal_Int16 > lGroups(2);
    lGroups[0] = css::frame::CommandGroup::VIEW;
    lGroups[1] = css::frame::CommandGroup::DOCUMENT;
    return lGroups;
}

//-----------------------------------------------
css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL CloseDispatcher::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw(css::uno::RuntimeException)
{
    if (nCommandGroup == css::frame::CommandGroup::VIEW)
    {
        /* Attention: Dont add .uno:CloseFrame here. Because its not realy
                      a configurable feature ... and further it does not have
                      a valid UIName entry inside the GenericCommands.xcu ... */
        css::uno::Sequence< css::frame::DispatchInformation > lViewInfos(1);
        lViewInfos[0].Command = rtl::OUString(URL_CLOSEWIN);
        lViewInfos[0].GroupId = css::frame::CommandGroup::VIEW;
        return lViewInfos;
    }
    else if (nCommandGroup == css::frame::CommandGroup::DOCUMENT)
    {
        css::uno::Sequence< css::frame::DispatchInformation > lDocInfos(1);
        lDocInfos[0].Command = rtl::OUString(URL_CLOSEDOC);
        lDocInfos[0].GroupId = css::frame::CommandGroup::DOCUMENT;
        return lDocInfos;
    }

    return css::uno::Sequence< css::frame::DispatchInformation >();
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                 const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL CloseDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                    const css::util::URL&                                     /*aURL*/     )
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
    if ( aURL.Complete == URL_CLOSEDOC )
        m_eOperation = E_CLOSE_DOC;
    else if ( aURL.Complete == URL_CLOSEWIN )
        m_eOperation = E_CLOSE_WIN;
    else if ( aURL.Complete == URL_CLOSEFRAME )
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

    if (m_pSysWindow && m_pSysWindow->GetCloseHdl().IsSet())
    {
        // The closing frame has its own close handler.  Call it instead.
        m_pSysWindow->GetCloseHdl().Call(m_pSysWindow);
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

    sal_Bool bIsSynchron = sal_False;
    for (sal_Int32 nArgs=0; nArgs<lArguments.getLength(); nArgs++ )
    {
        if ( lArguments[nArgs].Name == "SynchronMode" )
        {
            lArguments[nArgs].Value >>= bIsSynchron;
            break;
        }
    }

    if ( bIsSynchron )
        impl_asyncCallback(0);
    else
        m_aAsyncCallback.Post(0);
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
IMPL_LINK_NOARG(CloseDispatcher, impl_asyncCallback)
{
    try
    {

    // Allow calling of XController->suspend() everytimes.
    // Dispatch is an UI functionality. We implement such dispatch object here.
    // And further XController->suspend() was designed to bring an UI ...
    sal_Bool bAllowSuspend        = sal_True;
    sal_Bool bControllerSuspended = sal_False;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // Closing of all views, related to the same document, is allowed
    // only if the dispatched URL was ".uno:CloseDoc"!
    sal_Bool bCloseAllViewsToo = (m_eOperation == E_CLOSE_DOC);

    // BTW: Make some copies, which are needed later ...
    EOperation                                                  eOperation  = m_eOperation;
    css::uno::Reference< css::lang::XMultiServiceFactory >      xSMGR       = m_xSMGR;
    css::uno::Reference< css::frame::XFrame >                   xCloseFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchResultListener >  xListener   = m_xResultListener;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // frame already dead ?!
    // Nothing to do !
    if (! xCloseFrame.is())
        return 0;

    sal_Bool bCloseFrame           = sal_False;
    sal_Bool bEstablishBackingMode = sal_False;
    sal_Bool bTerminateApp         = sal_False;

    // Analyze the environment a first time.
    // If we found some special cases, we can
    // make some decisions erliar!
    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
    FrameListAnalyzer aCheck1(xDesktop, xCloseFrame, FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    // a) If the curent frame (where the close dispatch was requested for) does not have
    //    any parent frame ... it will close this frame only. Such frame isnt part of the
    //    global desktop tree ... and such frames are used as "implementation details" only.
    //    E.g. the live previews of our wizards doing such things. And then the owner of the frame
    //    is responsible for closing the application or accepting closing of the application
    //    by others.
    if ( ! xCloseFrame->getCreator().is())
        bCloseFrame = sal_True;
    else

    // b) The help window cant disagree with any request.
    //    Because it doesnt implement a controller - it uses a window only.
    //    Further t cant be the last open frame - if we do all other things
    //    right inside this CloseDispatcher implementation.
    //    => close it!
    if (aCheck1.m_bReferenceIsHelp)
        bCloseFrame = sal_True;
    else

    // c) If we are already in "backing mode", we have to terminate
    //    the application, if this special frame is closed.
    //    It doesnt matter, how many other frames (can be the help or hidden frames only)
    //    are open then.
    //    => terminate the application!
    if (aCheck1.m_bReferenceIsBacking)
        bTerminateApp = sal_True;
    else

    // d) Otherwhise we have to: close all views to the same document, close the
    //    document inside our own frame and decide then again, what has to be done!
    {
        if (implts_prepareFrameForClosing(m_xCloseFrame, bAllowSuspend, bCloseAllViewsToo, bControllerSuspended))
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

            // c2) if we close the current view ... but not all other views
            //     to the same document, we must close the current frame only!
            //     Because implts_closeView() suspended this view only - does not
            //     close the frame.
            if (
                (!bCloseAllViewsToo                    ) &&
                (aCheck2.m_lModelFrames.getLength() > 0)
               )
                bCloseFrame = sal_True;

            else
            // c3) there is no other (visible) frame open ...
            //     The help module will be ignored everytimes!
            //     But we have to decide if we must terminate the
            //     application or establish the backing mode now.
            //     And that depends from the dispatched URL ...
            {
                if (eOperation == E_CLOSE_FRAME)
                    bTerminateApp = sal_True;
                else if( SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE) )
                    bEstablishBackingMode = sal_True;
                else
                    bTerminateApp = sal_True;
            }
        }
    }

    // Do it now ...
    sal_Bool bSuccess = sal_False;
    if (bCloseFrame)
        bSuccess = implts_closeFrame();
    else
    if (bEstablishBackingMode)
    #if defined QUARTZ
    {
        // on mac close down, quickstarter keeps the process alive
        // however if someone has shut down the quickstarter
        // behave as any other platform

        bool bQuickstarterRunning = false;
        // get quickstart service
        try
        {
            css::uno::Reference< css::beans::XFastPropertySet > xSet( xSMGR->createInstance(IMPLEMENTATIONNAME_QUICKLAUNCHER), css::uno::UNO_QUERY_THROW );
            if( xSet.is() )
            {
                css::uno::Any aVal( xSet->getFastPropertyValue( 0 ) );
                sal_Bool bState = sal_False;
                if( aVal >>= bState )
                    bQuickstarterRunning = bState;
            }
        }
        catch( const css::uno::Exception& )
        {
        }
        bSuccess = bQuickstarterRunning ? implts_terminateApplication() : implts_establishBackingMode();
    }
    #else
        bSuccess = implts_establishBackingMode();
    #endif
    else
    if (bTerminateApp)
        bSuccess = implts_terminateApplication();

    if (
        ( ! bSuccess             ) &&
        (   bControllerSuspended )
       )
    {
        css::uno::Reference< css::frame::XController > xController = xCloseFrame->getController();
        if (xController.is())
            xController->suspend(sal_False);
    }

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

    }
    catch(const css::lang::DisposedException&)
    {
    }

    return 0;
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_prepareFrameForClosing(const css::uno::Reference< css::frame::XFrame >& xFrame                ,
                                                              sal_Bool                                   bAllowSuspend         ,
                                                              sal_Bool                                   bCloseAllOtherViewsToo,
                                                              sal_Bool&                                  bControllerSuspended  )
{
    // Frame already dead ... so this view is closed ... is closed ... is ... .-)
    if (! xFrame.is())
        return sal_True;

    // Close all views to the same document ... if forced to do so.
    // But dont touch our own frame here!
    // We must do so ... because the may be following controller->suspend()
    // will show the "save/discard/cancel" dialog for the last view only!
    if (bCloseAllOtherViewsToo)
    {
        // SAFE -> ----------------------------------
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = m_xSMGR;
        aReadLock.unlock();
        // <- SAFE ----------------------------------

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

    // If allowed - inform user about modified documents or
    // still running jobs (e.g. printing).
    if (bAllowSuspend)
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if (xController.is()) // some views dont uses a controller .-( (e.g. the help window)
        {
            bControllerSuspended = xController->suspend(sal_True);
            if (! bControllerSuspended)
                return sal_False;
        }
    }

    // dont remove the component realy by e.g. calling setComponent(null, null).
    // It's enough to suspend the controller.
    // If we close the frame later this controller doesnt show the same dialog again.
    return sal_True;
}

//-----------------------------------------------
sal_Bool CloseDispatcher::implts_closeFrame()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame > xFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // frame already dead ? => so it's closed ... it's closed ...
    if ( ! xFrame.is() )
        return sal_True;

    // dont deliver owner ship; our "UI user" will try it again if it failed.
    // OK - he will get an empty frame then. But normaly an empty frame
    // should be closeable always :-)
    if (!fpf::closeIt(xFrame, sal_False))
        return sal_False;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xCloseFrame = css::uno::WeakReference< css::frame::XFrame >();
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
    css::uno::Reference< css::frame::XFrame >              xFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (!xFrame.is())
        return sal_False;

    css::uno::Reference < css::document::XActionLockable > xLock( xFrame, css::uno::UNO_QUERY );
    if ( xLock.is() && xLock->isActionLocked() )
        return sal_False;

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

//-----------------------------------------------
css::uno::Reference< css::frame::XFrame > CloseDispatcher::static_impl_searchRightTargetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame ,
                                                                                              const ::rtl::OUString&                           sTarget)
{
    if (sTarget.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("_self")))
        return xFrame;

    OSL_ENSURE(sTarget.isEmpty(), "CloseDispatch used for unexpected target. Magic things will happen now .-)");

    css::uno::Reference< css::frame::XFrame > xTarget = xFrame;
    while(sal_True)
    {
        // a) top frames wil be closed
        if (xTarget->isTop())
            return xTarget;

        // b) even child frame containing top level windows (e.g. query designer of database) will be closed
        css::uno::Reference< css::awt::XWindow >    xWindow        = xTarget->getContainerWindow();
        css::uno::Reference< css::awt::XTopWindow > xTopWindowCheck(xWindow, css::uno::UNO_QUERY);
        if (xTopWindowCheck.is())
        {
            // b1) Note: Toolkit interface XTopWindow sometimes is used by real VCL-child-windows also .-)
            //     Be sure that these window is realy a "top system window".
            //     Attention ! Checking Window->GetParent() isnt the right approach here.
            //     Because sometimes VCL create "implicit border windows" as parents even we created
            //     a simple XWindow using the toolkit only .-(
            SolarMutexGuard aSolarLock;
            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if (
                (pWindow                  ) &&
                (pWindow->IsSystemWindow())
               )
                return xTarget;
        }

        // c) try to find better results on parent frame
        //    If no parent frame exists (because this frame is used outside the desktop tree)
        //    the given frame must be used directly.
        css::uno::Reference< css::frame::XFrame > xParent(xTarget->getCreator(), css::uno::UNO_QUERY);
        if ( ! xParent.is())
            return xTarget;

        // c1) check parent frame inside next loop ...
        xTarget = xParent;
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
