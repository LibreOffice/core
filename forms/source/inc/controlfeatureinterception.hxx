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

#ifndef INCLUDED_FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX
#define INCLUDED_FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>


namespace frm
{


    class UrlTransformer;

    //= ControlFeatureInterception

    /** helper class for controls which allow some of their features to be intercepted
        by external instances

        For using this class, instantiate it as member, derive yourself from
        <type scope="com::sun::star::frame">XDispatchProviderInterception</type>, and forward all
        respective methods to this member.

        Additionally, don't forget to call <member>dispose</member> when your class is disposed itself.
    */
    class ControlFeatureInterception
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >
                                            m_xFirstDispatchInterceptor;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< UrlTransformer >   m_pUrlTransformer;
        SAL_WNODEPRECATED_DECLARATIONS_POP

    public:
        /** retrieves our URL transformer, so our clients may use it, too
        */
        const UrlTransformer& getTransformer() const { return *m_pUrlTransformer; }

    public:
        ControlFeatureInterception( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB );

        // XDispatchProviderInterception
        void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException );
        void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException );

        // XComponent
        void SAL_CALL dispose();

        /** queries the interceptor chain for the given dispatch
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const ::com::sun::star::util::URL& _rURL, const OUString& _rTargetFrameName, ::sal_Int32 _nSearchFlags ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** queries the interceptor chain for the given dispatch, with a blank target frame and no frame search flags
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const ::com::sun::star::util::URL& _rURL ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** queries the interceptor chain for the URL given as ASCII string,
            with a blank target frame and no frame search flags
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const sal_Char* _pAsciiURL ) SAL_THROW((::com::sun::star::uno::RuntimeException));
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
