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

#pragma once

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <vcl/evntpost.hxx>
#include <vcl/vclptr.hxx>

class SystemWindow;

namespace framework{

/**
    @short          helper to dispatch the URLs ".uno:CloseDoc"/".uno:CloseWin"/".uno:CloseFrame"
                    to close a frame/document or the whole application implicitly in case it was the last frame

    @descr          These URLs implements a special functionality to close a document or the whole frame ...
                    and handle the state, it was the last frame or document. Then we create the
                    default backing document which can be used to open new ones using the file open dialog
                    or some other menu entries. Or we terminate the whole application in case this backing mode should not
                    be used.
 */
class CloseDispatcher final : public  ::cppu::WeakImplHelper<
                                    css::frame::XNotifyingDispatch,             // => XDispatch
                                    css::frame::XDispatchInformationProvider >
{

    // types

    private:

        /** @short  describe, which request must be done here.
        @descr      The incoming URLs {.uno:CloseDoc/CloseWin and CloseFrame
                    can be classified so and checked later performant.}*/
        enum EOperation
        {
            E_CLOSE_DOC,
            E_CLOSE_FRAME,
            E_CLOSE_WIN
        };

    // member

    private:

        /** @short reference to a uno service manager,
                   which can be used to create own needed
                   uno resources. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** @short  reference to the target frame, which should be
                    closed by this dispatch. */
        css::uno::WeakReference< css::frame::XFrame > m_xCloseFrame;

        /** @short  used for asynchronous callbacks within the main thread.
            @descr  Internally we work asynchronously. Because our callees
                    are not aware that their request can kill its own environment... */
        std::unique_ptr<vcl::EventPoster> m_aAsyncCallback;

        /** @short  used inside asynchronous callback to decide,
                    which operation must be executed. */
        EOperation m_eOperation;

        /** @short  for asynchronous operations we must hold us self alive! */
        css::uno::Reference< css::uno::XInterface > m_xSelfHold;

        /** @short  holded alive for internally asynchronous operations! */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;

        VclPtr<SystemWindow> m_pSysWindow;

    // native interface

    public:

        /** @short  connect a new CloseDispatcher instance to its frame.
            @descr  One CloseDispatcher instance is bound to own frame only.
                    That makes an implementation (e.g. of listener support)
                    much more easier .-)

            @param  rxContext
                    an un oservice manager, which is needed to create uno resource
                    internally.

            @param  xFrame
                    the frame where the corresponding dispatch was started.

            @param  sTarget
                    help us to find the right target for this close operation.
         */
        CloseDispatcher(const css::uno::Reference< css::uno::XComponentContext >& rxContext  ,
                        const css::uno::Reference< css::frame::XFrame >&          xFrame ,
                        const OUString&                                           sTarget);

        /** @short  does nothing real. */
        virtual ~CloseDispatcher() override;

    // uno interface

    public:

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) override;

        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments) override;
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) override;
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) override;

        // XDispatchInformationProvider
        virtual css::uno::Sequence< sal_Int16 >                       SAL_CALL getSupportedCommandGroups         (                         ) override;
        virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) override;

    // internal helper

    private:

        /** @short  a callback for asynchronous started operations.

            @descr  As already mentioned, we make internally all operations
                    asynchronous. Otherwise our callis kill its own environment
                    during they call us...
        */
        DECL_LINK( impl_asyncCallback, LinkParamNone*, void );

        /** @short  prepare m_xCloseFrame so it should be closeable without problems.

            @descr  That's needed to be sure, that the document can't disagree
                    later with e.g. an office termination.
                    The problem: Closing of documents can show UI. If the user
                    ignores it and open/close other documents, we can't know
                    which state the office has after closing of this frame.

            @param  bCloseAllOtherViewsToo
                    if there are other top level frames, which
                    contains views to the same document then our m_xCloseFrame,
                    they are forced to be closed too.
                    We need it to implement the CLOSE_DOC semantic.

            @return [boolean]
                    sal_True if closing was successful.
         */
        bool implts_prepareFrameForClosing(const css::uno::Reference< css::frame::XFrame >& xFrame,
                                           bool                                   bCloseAllOtherViewsToo,
                                           bool&                                  bControllerSuspended  );

        /** @short  close the member m_xCloseFrame.

            @descr  This method does not look for any document
                    inside this frame. Such views must be cleared
                    before (e.g. by calling implts_closeView()!

                    Otherwise e.g. the XController->suspend()
                    call is not made and no UI warn the user about
                    losing document changes. Because the
                    frame is closed...

            @return [bool]
                    sal_True if closing was successful.
         */
        bool implts_closeFrame();

        /** @short  set the special BackingComponent (now StartModule)
                    as new component of our m_xCloseFrame.

            @return [bool]
                    sal_True if operation was successful.
         */
        bool implts_establishBackingMode();

        /** @short  calls XDesktop->terminate().

            @descr  No office code has to be called
                    afterwards! Because the process is dying...
                    The only exception is a might be registered
                    listener at this instance here.
                    Because he should know, that such things will happen :-)

            @return [bool]
                    sal_True if termination of the application was started ...
         */
        bool implts_terminateApplication();

        /** @short  notify a DispatchResultListener.

            @descr  We check the listener reference before we use it.
                    So this method can be called every time!

            @parama xListener
                    the listener, which should be notified.
                    Can be null!

            @param  nState
                    directly used as css::frame::DispatchResultState value.

            @param  aResult
                    not used yet really ...
         */
        void implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                               sal_Int16                                                   nState   ,
                                         const css::uno::Any&                                              aResult  );

        /** @short  try to find the right target frame where this close request
                    must be really done.

            @descr  The problem behind: closing some resources depends sometimes from the
                    context where its dispatched. Sometimes the start frame of the dispatch
                    has to be closed itself (target=_self) ... sometimes its parent frame
                    has to be closed - BUT(!) it means a parent frame containing a top level
                    window. _top can't be used then for dispatch - because it address TopFrames
                    not frames containing top level windows. So normally _magic (which btw does not
                    exists at the moment .-) ) should be used. So we interpret target=<empty>
                    as _magic !

            @param  xFrame
                    start point for search of right dispatch frame.

            @param  sTarget
                    give us an idea how this target frame must be searched.
        */

        static css::uno::Reference< css::frame::XFrame > static_impl_searchRightTargetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame ,
                                                                                            const OUString&                           sTarget);

}; // class CloseDispatcher

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
