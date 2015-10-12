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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_INTERCEPT_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_INTERCEPT_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include "documentdefinition.hxx"
#include <vcl/svapp.hxx>

namespace dbaccess
{


class OInterceptor : public ::cppu::WeakImplHelper< css::frame::XDispatchProviderInterceptor,
                                                    css::frame::XInterceptorInfo,
                                                    css::frame::XDispatch,
                                                    css::document::XDocumentEventListener>
{
    DECL_LINK_TYPED( OnDispatch, void*, void );
protected:
    virtual ~OInterceptor();
public:

    explicit OInterceptor( ODocumentDefinition* _pContentHolder );

    void SAL_CALL dispose() throw(css::uno::RuntimeException);

    //XDispatch
    virtual void SAL_CALL
    dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
    addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    virtual void SAL_CALL
    removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    //XInterceptorInfo
    virtual css::uno::Sequence< OUString >
    SAL_CALL getInterceptedURLs(  )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    //XDispatchProvider ( inherited by XDispatchProviderInterceptor )
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL
    queryDispatch(
        const css::util::URL& URL,
        const OUString& TargetFrameName,
        sal_Int32 SearchFlags )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const css::uno::Sequence< css::frame::DispatchDescriptor >& Requests )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    //XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getSlaveDispatchProvider(  )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    virtual void SAL_CALL
    setSlaveDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewDispatchProvider )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getMasterDispatchProvider(  )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    virtual void SAL_CALL
    setMasterDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewSupplier )
        throw (
            css::uno::RuntimeException, std::exception
        ) override;

    // XDocumentEventListener
    virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

private:

    osl::Mutex   m_aMutex;

    ODocumentDefinition*   m_pContentHolder;

    css::uno::Reference< css::frame::XDispatchProvider > m_xSlaveDispatchProvider;
    css::uno::Reference< css::frame::XDispatchProvider > m_xMasterDispatchProvider;

    css::uno::Sequence< OUString >      m_aInterceptedURL;

    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    PropertyChangeListenerContainer*    m_pStatCL;
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_INTERCEPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
