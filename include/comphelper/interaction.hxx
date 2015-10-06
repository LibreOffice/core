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

#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    //= OInteraction

    /** template for instantiating concret interaction handlers<p/>
        the template argument must eb an interface derived from XInteractionContinuation
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
        virtual void SAL_CALL select() throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    private:
        bool    m_bSelected : 1;    /// indicates if the select event occurred
    };


    template <class INTERACTION>
    void SAL_CALL OInteraction< INTERACTION >::select(  ) throw(::com::sun::star::uno::RuntimeException)
    {
        m_bSelected = true;
    }


    //= OInteractionApprove

    typedef OInteraction< ::com::sun::star::task::XInteractionApprove > OInteractionApprove;


    //= OInteractionDispprove

    typedef OInteraction< ::com::sun::star::task::XInteractionDisapprove >  OInteractionDisapprove;


    //= OInteractionAbort

    typedef OInteraction< ::com::sun::star::task::XInteractionAbort >   OInteractionAbort;


    //= OInteractionRetry

    typedef OInteraction< ::com::sun::star::task::XInteractionRetry >   OInteractionRetry;


    //= OInteractionPassword

    class COMPHELPER_DLLPUBLIC OInteractionPassword : public OInteraction< ::com::sun::star::task::XInteractionPassword >
    {
    public:
        OInteractionPassword()
        {
        }

        OInteractionPassword( const OUString& _rInitialPassword )
            :m_sPassword( _rInitialPassword )
        {
        }

        // XInteractionPassword
        virtual void SAL_CALL setPassword( const OUString& _Password ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getPassword(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        OUString m_sPassword;
    };


    //= OInteractionRequest

    typedef ::cppu::WeakImplHelper <   ::com::sun::star::task::XInteractionRequest
                                   >   OInteractionRequest_Base;
    /** implements an interaction request (com.sun.star.task::XInteractionRequest)<p/>
        at run time, you can freely add any interaction continuation objects
    */
    class COMPHELPER_DLLPUBLIC OInteractionRequest : public OInteractionRequest_Base
    {
        ::com::sun::star::uno::Any
                    m_aRequest;         /// the request we represent
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >
                    m_aContinuations;   /// all registered continuations

    public:
        OInteractionRequest(const ::com::sun::star::uno::Any& _rRequestDescription);
        OInteractionRequest(const ::com::sun::star::uno::Any& rRequestDescription,
            css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> const& rContinuations);

        /// add a new continuation
        void addContinuation(const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >& _rxContinuation);

    // XInteractionRequest
        virtual ::com::sun::star::uno::Any SAL_CALL getRequest(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_INTERACTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
