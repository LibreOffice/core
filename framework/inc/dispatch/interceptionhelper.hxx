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

#ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_
#define __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_

#include <services/frame.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>

#include <tools/wldcrd.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/weakref.hxx>

#include <deque>

namespace framework{

/** @short      implements a helper to support interception with additional functionality.

    @descr      This helper implements the complete XDispatchProviderInterception interface with
                master/slave functionality AND using of optional features like registration of URL pattern!

    @attention  Don't use this class as direct member - use it dynamicly. Do not derive from this class.
                We hold a weakreference to ouer owner not to ouer superclass.
 */
class InterceptionHelper : // order of base classes is important for right initialization of mutex member!
                           private ThreadHelpBase,
                           public  ::cppu::WeakImplHelper3<
                                     css::frame::XDispatchProvider,
                                     css::frame::XDispatchProviderInterception,
                                     css::lang::XEventListener >
{
    //_____________________________________________________
    // structs, helper

    /** @short bind an interceptor component to it's URL pattern registration. */
    struct InterceptorInfo
    {
        /** @short reference to the interceptor component. */
        css::uno::Reference< css::frame::XDispatchProvider > xInterceptor;

        /** @short it's registration for URL patterns.

            @descr If the interceptor component does not support the optional interface
                   XInterceptorInfo, it will be registered for one pattern "*" by default.
                   That would make it possible to handle it in the same manner then real
                   registered interceptor objects and we must not implement any special code. */
        css::uno::Sequence< OUString > lURLPattern;
    };

    //_____________________________________________________

    /** @short implements a list of items of type InterceptorInfo, and provides some special
               functions on it.

        @descr Because interceptor objects can be registered for URL patterns,
               it supports a wildcard search on all list items.
     */
    class InterceptorList : public ::std::deque< InterceptorInfo >
    {
        public:

            //_____________________________________________

            /** @short search for an interceptor inside this list using it's reference.

                @param xInterceptor
                        points to the interceptor object, which should be located inside this list.

                @return An iterator object, which points directly to the located item inside this list.
                        In case no interceptor could be found, it points to the end of this list!
              */
            iterator findByReference(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
            {
                css::uno::Reference< css::frame::XDispatchProviderInterceptor > xProviderInterface(xInterceptor, css::uno::UNO_QUERY);
                iterator pIt;
                for (pIt=begin(); pIt!=end(); ++pIt)
                {
                    if (pIt->xInterceptor == xProviderInterface)
                        return pIt;
                }
                return end();
            }

            //_____________________________________________

            /** @short search for an interceptor inside this list using it's reference.

                @param xInterceptor
                        points to the interceptor object, which should be located inside this list.

                @return An iterator object, which points directly to the located item inside this list.
                        In case no interceptor could be found, it points to the end of this list!
              */
            iterator findByPattern(const OUString& sURL)
            {
                iterator pIt;
                for (pIt=begin(); pIt!=end(); ++pIt)
                {
                    sal_Int32              c        = pIt->lURLPattern.getLength();
                    const OUString* pPattern = pIt->lURLPattern.getConstArray();

                    for (sal_Int32 i=0; i<c; ++i)
                    {
                        WildCard aPattern(pPattern[i]);
                        if (aPattern.Matches(sURL))
                            return pIt;
                    }
                }
                return end();
            }
    };

    //_____________________________________________________
    // member

    private:

        /** @short reference to the frame, which uses this instance to implement it's own interception.

            @descr We hold a weak reference only, to make disposing operations easy. */
        css::uno::WeakReference< css::frame::XFrame > m_xOwnerWeak;

        /** @short this interception helper implements the top level master of an interceptor list ...
                   but this member is the lowest possible slave! */
        css::uno::Reference< css::frame::XDispatchProvider > m_xSlave;

        /** @short contains all registered interceptor objects. */
        InterceptorList m_lInterceptionRegs;

        /** @short it regulates, which interceptor is used first.
                   The last or the first registered one. */
        static sal_Bool m_bPreferrFirstInterceptor;

    //_____________________________________________________
    // native interface

    public:

        //_________________________________________________

        /** @short creates a new interception helper instance.

            @param xOwner
                    points to the frame, which use this instances to support it's own interception interfaces.

            @param xSlave
                    an outside creates dispatch provider, which has to be used here as lowest slave "interceptor".
         */
        InterceptionHelper(const css::uno::Reference< css::frame::XFrame >&            xOwner,
                           const css::uno::Reference< css::frame::XDispatchProvider >& xSlave);

    protected:

        //_________________________________________________

        /** @short standard destructor.

            @descr This method destruct an instance of this class and clear some member.
                   This method is protected, because its not allowed to use this class as a direct member!
                   You MUST use a dynamical instance (pointer). That's the reason for a protected dtor.
         */
        virtual ~InterceptionHelper();

    //_____________________________________________________
    // uno interface

    public:

        //_________________________________________________
        // XDispatchProvider

        /** @short  query for a dispatch, which implements the requested feature.

            @descr  We search inside our list of interception registrations, to locate
                    any interested interceptor. In case no interceptor exists or nobody is
                    interested on this URL our lowest slave will be used.

            @param  aURL
                        describes the requested dispatch functionality.

            @param  sTargetFrameName
                        the name of the target frame or a special name like "_blank", "_top" ...
                        Won't be used here ... but may by one of our registered interceptor objects
                        or our slave.

            @param  nSearchFlags
                        optional search parameter for targeting, if sTargetFrameName isn't a special one.

            @return A valid dispatch object, if any interceptor or at least our slave is interested on the given URL;
                    or NULL otherwise.
         */
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(const css::util::URL&  aURL            ,
                                                                                    const OUString& sTargetFrameName,
                                                                                          sal_Int32        nSearchFlags    )
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProvider

        /** @short implements an optimized queryDispatch() for remote.

            @descr It capsulate more then one queryDispatch() requests and return a lits of dispatch objects
                   as result. Because both lists (in and out) coreespond together, it's not allowed to
                   pack it - means supress NULL references!

            @param lDescriptor
                    a list of queryDispatch() arguments.

            @return A list of dispatch objects.
         */
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor)
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProviderInterception

        /** @short      register an interceptor.

            @descr      Somebody can register himself to intercept all or some special dispatches.
                        It's depend from his supported interfaces. If he implement XInterceptorInfo
                        he his called for some special URLs only - otherwise we call it for every request!

            @attention  We don't check for double registrations here!

            @param      xInterceptor
                        reference to interceptor, which wish to be registered here.

            @throw      A RuntimeException if the given reference is NULL!
         */
        virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProviderInterception

        /** @short      release an interceptor.

            @descr      Remove the registered interceptor from our internal list
                        and delete all special information about it.

            @param      xInterceptor
                        reference to the interceptor, which wish to be deregistered.

            @throw      A RuntimeException if the given reference is NULL!
         */
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

        //_________________________________________________
        // XEventListener

        /** @short      Is called from our owner frame, in case he will be disposed.

            @descr      We have to relaease all references to him then.
                        Normaly we will die by ref count too ...
         */
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

}; // class InterceptionHelper

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
