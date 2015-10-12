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

#ifndef INCLUDED_FRAMEWORK_SOURCE_INC_DISPATCH_LOADDISPATCHER_HXX
#define INCLUDED_FRAMEWORK_SOURCE_INC_DISPATCH_LOADDISPATCHER_HXX

#include <loadenv/loadenv.hxx>

#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>

#include <cppuhelper/implbase.hxx>

namespace framework{

/** @short      implements a dispatch object which can be used to load
                non-visible components (by using the mechanism of ContentHandler)
                or visible-components (by using the mechanism of FrameLoader).

    @author     as96863
 */
class LoadDispatcher : public  ::cppu::WeakImplHelper< css::frame::XNotifyingDispatch,          // => XDispatch => XInterface
                                                        css::frame::XSynchronousDispatch >
{

    // member

    private:
        osl::Mutex m_mutex;

        /** @short  can be used to create own needed services on demand. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** @short  TODO document me */
        css::uno::WeakReference< css::frame::XFrame > m_xOwnerFrame;

        /** @short  TODO document me */
        OUString m_sTarget;

        /** @short  TODO document me */
        sal_Int32 m_nSearchFlags;

        /** @short  TODO document me */
        LoadEnv m_aLoader;

    // native interface

    public:

        /** @short  creates a new instance and initialize it with all necessary parameters.

            @descr  Every instance of such LoadDispatcher can be used for the specified context only.
                    That means: it can be used to load any further requested content into the here(!)
                    specified target frame.

            @param  xContext
                    will be used to create own needed services on demand.

            @param  xOwnerFrame
                    used as startpoit to locate the right target frame.

            @param  sTargetName
                    the name or the target frame for loading or a special qualifier
                    which define such target.

            @param  nSearchFlags
                    used in case sTargetFrame isn't a special one.
         */
        LoadDispatcher(const css::uno::Reference< css::uno::XComponentContext >& xContext,
                       const css::uno::Reference< css::frame::XFrame >&          xOwnerFrame ,
                       const OUString&                                           sTargetName ,
                             sal_Int32                                           nSearchFlags);

        /** @short  used to free internal resources.
         */
        virtual ~LoadDispatcher();

    // uno interface

    public:

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;

        // XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException, std::exception) override;

        // XSynchronousDispatch
        virtual css::uno::Any SAL_CALL dispatchWithReturnValue( const css::util::URL&                                  aURL      ,
                                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
            throw( css::uno::RuntimeException, std::exception ) override;

    private:
        css::uno::Any impl_dispatch( const css::util::URL& rURL,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
}; // class LoadDispatcher

} // namespace framework

#endif // INCLUDED_FRAMEWORK_SOURCE_INC_DISPATCH_LOADDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
