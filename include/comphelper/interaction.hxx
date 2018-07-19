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

#ifndef INCLUDED_COMPHELPER_INTERACTION_HXX
#define INCLUDED_COMPHELPER_INTERACTION_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <comphelper/comphelperdllapi.h>
#include <vector>


namespace comphelper
{


    //= OInteraction

    /** template for instantiating concrete interaction handlers<p/>
        the template argument must be an interface derived from XInteractionContinuation
    */
    template <class INTERACTION>
    class OInteraction
            : public ::cppu::WeakImplHelper< INTERACTION >
    {
    public:
        OInteraction() : m_bSelected(false) {}

        /// determines whether or not this handler was selected
        bool    wasSelected() const { return m_bSelected; }

        // XInteractionContinuation
        virtual void SAL_CALL select() override;
    private:
        bool    m_bSelected : 1;    /// indicates if the select event occurred
    };


    template <class INTERACTION>
    void SAL_CALL OInteraction< INTERACTION >::select(  )
    {
        m_bSelected = true;
    }


    //= OInteractionApprove

    typedef OInteraction< css::task::XInteractionApprove > OInteractionApprove;


    //= OInteractionDisapprove

    typedef OInteraction< css::task::XInteractionDisapprove >  OInteractionDisapprove;


    //= OInteractionAbort

    typedef OInteraction< css::task::XInteractionAbort >   OInteractionAbort;


    //= OInteractionRetry

    typedef OInteraction< css::task::XInteractionRetry >   OInteractionRetry;


    //= OInteractionPassword

    class COMPHELPER_DLLPUBLIC OInteractionPassword : public OInteraction< css::task::XInteractionPassword >
    {
    public:
        OInteractionPassword( const OUString& _rInitialPassword )
            :m_sPassword( _rInitialPassword )
        {
        }

        // XInteractionPassword
        virtual void SAL_CALL setPassword( const OUString& Password ) override;
        virtual OUString SAL_CALL getPassword(  ) override;

    private:
        OUString m_sPassword;
    };


    //= OInteractionRequest

    typedef ::cppu::WeakImplHelper <   css::task::XInteractionRequest
                                   >   OInteractionRequest_Base;
    /** implements an interaction request (com.sun.star.task::XInteractionRequest)<p/>
        at run time, you can freely add any interaction continuation objects
    */
    class COMPHELPER_DLLPUBLIC OInteractionRequest : public OInteractionRequest_Base
    {
        css::uno::Any const
                    m_aRequest;         /// the request we represent
        std::vector< css::uno::Reference< css::task::XInteractionContinuation > >
                    m_aContinuations;   /// all registered continuations

    public:
        OInteractionRequest(const css::uno::Any& _rRequestDescription);
        OInteractionRequest(const css::uno::Any& rRequestDescription,
            std::vector<css::uno::Reference<css::task::XInteractionContinuation>> const& rContinuations);

        /// add a new continuation
        void addContinuation(const css::uno::Reference< css::task::XInteractionContinuation >& _rxContinuation);

    // XInteractionRequest
        virtual css::uno::Any SAL_CALL getRequest(  ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations(  ) override;
    };

}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_INTERACTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
