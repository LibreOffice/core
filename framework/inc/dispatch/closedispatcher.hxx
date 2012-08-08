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

#ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>
#include <general.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <cppuhelper/weak.hxx>
#include <vcl/evntpost.hxx>


class SystemWindow;

namespace framework{

//-----------------------------------------------
/**
    @short          helper to dispatch the URLs ".uno:CloseDoc"/".uno:CloseWin"/".uno:CloseFrame"
                    to close a frame/document or the whole application implicitly in case it was the last frame

    @descr          These URLs implements a special functionality to close a document or the whole frame ...
                    and handle the state, it was the last frame or document. Then we create the
                    default backing document which can be used to open new ones using the file open dialog
                    or some other menu entries. Or we terminate the whole application in case this backing mode shouldnt
                    be used.
 */
class CloseDispatcher : public css::lang::XTypeProvider
                      , public css::frame::XNotifyingDispatch             // => XDispatch
                      , public css::frame::XDispatchInformationProvider
                        // baseclasses ... order is neccessary for right initialization!
                      , private ThreadHelpBase
                      , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // types

    private:

        //---------------------------------------
        /** @short  describe, which request must be done here.
        @descr      The incoming URLs {.uno:CloseDoc/CloseWin and CloseFrame
                    can be classified so and checked later performant.}*/
        enum EOperation
        {
            E_CLOSE_DOC,
            E_CLOSE_FRAME,
            E_CLOSE_WIN
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short reference to an uno service manager,
                   which can be used to create own needed
                   uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  reference to the target frame, which should be
                    closed by this dispatch. */
        css::uno::WeakReference< css::frame::XFrame > m_xCloseFrame;

        //---------------------------------------
        /** @short  used for asynchronous callbacks within the main thread.
            @descr  Internaly we work asynchronous. Because our callis
                    are not aware, that her request can kill its own environment ... */
        ::vcl::EventPoster m_aAsyncCallback;

        //---------------------------------------
        /** @short  used inside asyncronous callback to decide,
                    which operation must be executed. */
        EOperation m_eOperation;

        //---------------------------------------
        /** @short  for asynchronous operations we must hold us self alive! */
        css::uno::Reference< css::uno::XInterface > m_xSelfHold;

        //---------------------------------------
        /** @short  list of registered status listener */
        ListenerHash m_lStatusListener;

        //---------------------------------------
        /** @short  holded alive for internaly asynchronous operations! */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;

        SystemWindow* m_pSysWindow;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        /** @short  connect a new CloseDispatcher instance to its frame.
            @descr  One CloseDispatcher instance is bound to onw frame only.
                    That makes an implementation (e.g. of listener support)
                    much more easier .-)

            @param  xSMGR
                    an un oservice manager, which is needed to create uno resource
                    internaly.

            @param  xFrame
                    the frame where the corresponding dispatch was started.

            @param  sTarget
                    help us to find the right target for this close operation.
         */
        CloseDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                        const css::uno::Reference< css::frame::XFrame >&              xFrame ,
                        const ::rtl::OUString&                                        sTarget);

        //---------------------------------------
        /** @short  does nothing real. */
        virtual ~CloseDispatcher();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //---------------------------------------
        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatchInformationProvider
        virtual css::uno::Sequence< sal_Int16 >                       SAL_CALL getSupportedCommandGroups         (                         ) throw (css::uno::RuntimeException);
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) throw (css::uno::RuntimeException);

    //-------------------------------------------
    // internal helper

    private:

        //---------------------------------------
        /** @short  a callback for asynchronous started operations.

            @descr  As already mentione, we make internaly all operations
                    asynchronous. Otherwhise our callis kill its own environment
                    during they call us ...
        */
        DECL_LINK( impl_asyncCallback, void* );

        //---------------------------------------
        /** @short  prepare m_xCloseFrame so it should be closeable without problems.

            @descr  Thats needed to be shure, that the document cant disagree
                    later with e.g. an office termination.
                    The problem: Closing of documents can show UI. If the user
                    ignores it and open/close other documents, we cant know
                    which state the office has after closing of this frame.

            @param  bAllowSuspend
                    force calling of XController->suspend().

            @param  bCloseAllOtherViewsToo
                    if there are other top level frames, which
                    contains views to the same document then our m_xCloseFrame,
                    they are forced to be closed too.
                    We need it to implement the CLOSE_DOC semantic.

            @return [boolean]
                    sal_True if closing was successfully.
         */
        sal_Bool implts_prepareFrameForClosing(const css::uno::Reference< css::frame::XFrame >& xFrame                ,
                                                     sal_Bool                                   bAllowSuspend         ,
                                                     sal_Bool                                   bCloseAllOtherViewsToo,
                                                     sal_Bool&                                  bControllerSuspended  );

        //---------------------------------------
        /** @short  close the member m_xCloseFrame.

            @descr  This method does not look for any document
                    inside this frame. Such views must be cleared
                    before (e.g. by calling implts_closeView()!

                    Otherwhise e.g. the XController->suspend()
                    call isnt made and no UI warn the user about
                    loosing document changes. Because the
                    frame is closed ....

            @return [bool]
                    sal_True if closing was successfully.
         */
        sal_Bool implts_closeFrame();

        //---------------------------------------
        /** @short  set the special BackingComponent (now StartModule)
                    as new component of our m_xCloseFrame.

            @return [bool]
                    sal_True if operation was successfully.
         */
        sal_Bool implts_establishBackingMode();

        //---------------------------------------
        /** @short  calls XDesktop->terminate().

            @descr  No office code has to be called
                    afterwards! Because the process is dieing ...
                    The only exception is a might be registered
                    listener at this instance here.
                    Because he should know, that such things will happen :-)

            @return [bool]
                    sal_True if termination of the application was started ...
         */
        sal_Bool implts_terminateApplication();

        //---------------------------------------
        /** @short  notify a DispatchResultListener.

            @descr  We check the listener reference before we use it.
                    So this method can be called everytimes!

            @parama xListener
                    the listener, which should be notified.
                    Can be null!

            @param  nState
                    directly used as css::frame::DispatchResultState value.

            @param  aResult
                    not used yet realy ...
         */
        void implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                               sal_Int16                                                   nState   ,
                                         const css::uno::Any&                                              aResult  );

        //---------------------------------------
        /** @short  try to find the right target frame where this close request
                    must be realy done.

            @descr  The problem behind: closing some resources depends sometimes from the
                    context where its dispatched. Sometimes the start frame of the dispatch
                    has to be closed itself (target=_self) ... sometimes it's parent frame
                    has to be closed - BUT(!) it means a parent frame containing a top level
                    window. _top cant be used then for dispatch - because it adress TopFrames
                    not frames containg top level windows. So normaly _magic (which btw does not
                    exists at the moment .-) ) should be used. So we interpret target=<empty>
                    as _magic !

            @param  xFrame
                    start point for search of right dispatch frame.

            @param  sTarget
                    give us an idea how this target frame must be searched.
        */

        static css::uno::Reference< css::frame::XFrame > static_impl_searchRightTargetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame ,
                                                                                            const ::rtl::OUString&                           sTarget);

}; // class CloseDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
