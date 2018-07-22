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

#ifndef INCLUDED_UCBHELPER_INTERCEPTEDINTERACTION_HXX
#define INCLUDED_UCBHELPER_INTERCEPTEDINTERACTION_HXX

#include <vector>

#include <com/sun/star/task/XInteractionHandler.hpp>

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>
#include <ucbhelper/ucbhelperdllapi.h>


namespace ucbhelper{


/** @short  it wraps any other interaction handler and intercept
            its handle() requests.

    @descr  This class can be used as:
            - instance if special interactions must be suppressed
              only
            - or as base class if interactions must be modified.
 */
class UCBHELPER_DLLPUBLIC InterceptedInteraction : public ::cppu::WeakImplHelper< css::task::XInteractionHandler >
{

    // types
    public:

        struct InterceptedRequest
        {

            /** @short  marks an Handle as invalid.
             */
            static const sal_Int32 INVALID_HANDLE = -1;


            /** @short  contains the interaction request, which should be intercepted. */
            css::uno::Any Request;


            /** @short  specify the fix continuation, which must be selected, if the
                        interaction could be intercepted successfully.
              */
            css::uno::Type Continuation;


            /** @short  it's an unique identifier, which must be managed by the outside code.

                @descr  If there is a derived class, which overwrites the InterceptedInteraction::intercepted()
                        method, it will be called with a reference to an InterceptedRequest struct.
                        Then it can use the handle to react without checking the request type again.
             */
            sal_Int32 Handle;


            /** @short  default ctor.

                @descr  Such constructed object can't be used really.
                        Might it will crash if its used!
                        Don't forget to initialize all(!) members ...
             */
            InterceptedRequest()
            {
                Handle     = INVALID_HANDLE;
            }

        };


        /** @short  represent the different states, which can occur
                    as result of an interception.

            @see    impl_interceptRequest()
         */
        enum EInterceptionState
        {
            /** none of the specified interceptions match the incoming request */
            E_NOT_INTERCEPTED,
            /** the request could be intercepted - but the specified continuation could not be located.
                That's normally an error of the programmer. May be the interaction request does not use
                the right set of continuations ... or the interception list contains the wrong continuation. */
            E_NO_CONTINUATION_FOUND,
            /** the request could be intercepted and the specified continuation could be selected successfully. */
            E_INTERCEPTED
        };


    // member
    protected:


        /** @short  reference to the intercepted interaction handler.

            @descr  NULL is allowed for this member!
                    All interaction will be aborted then ...
                    expecting th handle() was overwritten by
                    a derived class.
         */
        css::uno::Reference< css::task::XInteractionHandler > m_xInterceptedHandler;


        /** @short  these list contains the requests, which should be intercepted.
         */
        ::std::vector< InterceptedRequest > m_lInterceptions;


    // native interface
    public:


        /** @short  initialize a new instance with default values.
         */
        InterceptedInteraction();


        /** @short  initialize a new instance with the interaction handler,
                    which should be intercepted.

            @attention  If such interaction handler isn't set here,
                        all incoming requests will be aborted ...
                        if the right continuation is available!

            @param  xInterceptedHandler
                    the outside interaction handler, which should
                    be intercepted here.
         */
        void setInterceptedHandler(const css::uno::Reference< css::task::XInteractionHandler >& xInterceptedHandler);


        /** @short  set a new list of intercepted interactions.

            @attention  If the interface method handle() will be overwritten by
                        a derived class, the functionality behind these static list
                        can't be used.

            @param  lInterceptions
                    the list of intercepted requests.
         */
        void setInterceptions(const ::std::vector< InterceptedRequest >& lInterceptions);


        /** @short  extract a requested continuation from the list of available ones.

            @param  lContinuations
                    the list of available continuations.

            @param  aType
                    is used to locate the right continuation,
                    by checking its interface type.

            @return A valid reference to the continuation, if it could be located...
                    or an empty reference otherwise.
         */
        static css::uno::Reference< css::task::XInteractionContinuation > extractContinuation(
                    const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >& lContinuations,
                    const css::uno::Type&                                                                                             aType         );


    // usable for derived classes
    protected:


        /** @short  can be overwritten by a derived class to handle interceptions
                    outside.

            @descr  This base implementation checks, if the request could be intercepted
                    successfully. Then this method intercepted() is called.
                    The default implementation returns "NOT_INTERCEPTED" every time.
                    So the method impl_interceptRequest() uses the right continuation automatically.

                    If this method was overwritten and something different "NO_INTERCEPTED"
                    is returned, the method impl_interceptRequest() will return immediately with
                    the result, which is returned by this intercepted() method.
                    Then the continuations must be selected inside the intercepted() call!

            @param  rRequest
                    it points to the intercepted request (means the item of the
                    set interception list). e.g. its "Handle" member can be used
                    to identify it and react very easy, without the need to check the
                    type of the exception ...

            @param  xOrgRequest
                    points to the original interaction, which was intercepted.
                    It provides access to the exception and the list of possible
                    continuations.

            @return The result of this operation.
                    Note: If E_NOT_INTERCEPTED is returned the default handling of the base class
                    will be used automatically for this request!
         */
        virtual EInterceptionState intercepted(const InterceptedRequest&                                                             rRequest   ,
                                               const css::uno::Reference< css::task::XInteractionRequest >& xOrgRequest);


    // uno interface
    public:


        /** @short  implements the default handling of this class...
                    or can be overwritten by any derived class.

            @descr  If no further class is derived from this one
                    -> the default implementation is used. Then the
                    internal list of requests is used to handle different
                    interactions automatically.
                    (see impl_interceptRequest())

                    If this method was overwritten by a derived implementation
                    -> the new implementation has to do everything by itself.
                    Of course it can access all members/helpers and work with it.
                    But the default implementation is not used automatically then.

            @param  xRequest
                    the interaction request, which should be intercepted.
         */
        virtual void SAL_CALL handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest) override;


    // helper
    private:


        /** @short  implements the default handling:
                    - intercept or forward to internal handler.
         */
        UCBHELPER_DLLPRIVATE void impl_handleDefault(const css::uno::Reference< css::task::XInteractionRequest >& xRequest);


        /** @short  implements the interception of requests.

            @descr  The incoming request will be analyzed, if it match
                    any request of the m_lIntercepions list.
                    If an interception could be found, its continuation will be
                    searched and selected.

                    The method return the state of that operation.
                    But it doesn't call the intercepted and here set
                    interaction handler. That has to be done in the outside method.

            @param  xRequest
                    the interaction request, which should be intercepted.

            @return A identifier, which indicates if the request was intercepted,
                    the continuation was found and selected... or not.
         */
        UCBHELPER_DLLPRIVATE EInterceptionState impl_interceptRequest(const css::uno::Reference< css::task::XInteractionRequest >& xRequest);
};

} // namespace ucbhelper

#endif // INCLUDED_UCBHELPER_INTERCEPTEDINTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
