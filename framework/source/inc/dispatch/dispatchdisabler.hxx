/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>

namespace framework {

/**
 * Implementation of a service to make it easy to disable a whole
 * suite of UNO commands in a batch - and have that act in-process.
 *
 * Often external re-use of LibreOffice wants a very cut-down set
 * of functionality included, and disabling elements remotely one
 * by one performs poorly.
 */
class DispatchDisabler : public ::cppu::WeakImplHelper<
                                        css::lang::XInitialization,
                                        css::container::XNameContainer,
                                        css::frame::XDispatchProviderInterceptor,
                                        css::frame::XInterceptorInfo,
                                        css::lang::XServiceInfo >
{
    std::set<OUString> maDisabledURLs;
    css::uno::Reference< css::frame::XDispatchProvider > mxSlave;
    css::uno::Reference< css::frame::XDispatchProvider > mxMaster;
public:
             DispatchDisabler(const css::uno::Reference< css::uno::XComponentContext >& rxContext);

    // XInitialization
    virtual void SAL_CALL initialize( const ::css::uno::Sequence< ::css::uno::Any >& aArguments ) override;

    // XDispatchProvider
    virtual ::css::uno::Reference< ::css::frame::XDispatch > SAL_CALL
        queryDispatch( const ::css::util::URL& URL,
               const OUString& TargetFrameName,
               ::sal_Int32 SearchFlags ) override;
    virtual ::css::uno::Sequence< ::css::uno::Reference< ::css::frame::XDispatch > > SAL_CALL
        queryDispatches( const ::css::uno::Sequence< ::css::frame::DispatchDescriptor >& Requests ) override;

    // XDispatchProviderInterceptor
    virtual ::css::uno::Reference< ::css::frame::XDispatchProvider > SAL_CALL
        getSlaveDispatchProvider() override;
    virtual void SAL_CALL
        setSlaveDispatchProvider( const ::css::uno::Reference< ::css::frame::XDispatchProvider >& NewDispatchProvider ) override;
    virtual ::css::uno::Reference< ::css::frame::XDispatchProvider > SAL_CALL
        getMasterDispatchProvider() override;
    virtual void SAL_CALL
        setMasterDispatchProvider( const ::css::uno::Reference< ::css::frame::XDispatchProvider >& NewSupplier ) override;

    // XInterceptorInfo
    virtual ::css::uno::Sequence< OUString > SAL_CALL
        getInterceptedURLs() override;

    // XElementAccess
    virtual ::css::uno::Type SAL_CALL getElementType() override;
    virtual ::sal_Bool SAL_CALL hasElements() override;

    // XNameAccess
    virtual ::css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual ::css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::css::uno::Any& aElement ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    /* interface XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
