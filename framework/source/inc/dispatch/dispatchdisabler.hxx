/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_FRAMEWORK_INC_SERVICES_DISPATCH_DISABLER_HXX
#define INCLUDED_FRAMEWORK_INC_SERVICES_DISPATCH_DISABLER_HXX

#include <set>

#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/weakref.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>

#include <macros/xserviceinfo.hxx>

namespace framework {

/**
 * Implementation of a service to make it easy to disable a whole
 * suite of UNO commands in a batch - and have that act in-process.
 *
 * Often external re-use of LibreOffice wants a very cut-down set
 * of functionality included, and disabling elements remotely one
 * by one performs poorly.
 */
class DispatchDisabler : public ::cppu::WeakImplHelper5<
                                        css::lang::XInitialization,
                                        css::container::XNameContainer,
                                        css::frame::XDispatchProviderInterceptor,
                                        css::frame::XInterceptorInfo,
                                        css::lang::XServiceInfo >
{
    std::set<OUString> maDisabledURLs;
    css::uno::Reference< css::frame::XDispatchProvider > mxSlave;
    css::uno::Reference< css::frame::XDispatchProvider > mxMaster;
    css::uno::Reference< css::uno::XComponentContext >   mxContext;
public:
             DispatchDisabler(const css::uno::Reference< css::uno::XComponentContext >& rxContext);
    virtual ~DispatchDisabler() {}

    // XInitialization
    virtual void SAL_CALL initialize( const ::css::uno::Sequence< ::css::uno::Any >& aArguments )
        throw (::css::uno::Exception, ::css::uno::RuntimeException, ::std::exception) override;

    // XDispatchProvider
    virtual ::css::uno::Reference< ::css::frame::XDispatch > SAL_CALL
        queryDispatch( const ::css::util::URL& URL,
               const OUString& TargetFrameName,
               ::sal_Int32 SearchFlags )
           throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual ::css::uno::Sequence< ::css::uno::Reference< ::css::frame::XDispatch > > SAL_CALL
        queryDispatches( const ::css::uno::Sequence< ::css::frame::DispatchDescriptor >& Requests )
           throw (::css::uno::RuntimeException, ::std::exception) override;

    // XDispatchProviderInterceptor
    virtual ::css::uno::Reference< ::css::frame::XDispatchProvider > SAL_CALL
        getSlaveDispatchProvider() throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual void SAL_CALL
        setSlaveDispatchProvider( const ::css::uno::Reference< ::css::frame::XDispatchProvider >& NewDispatchProvider )
            throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual ::css::uno::Reference< ::css::frame::XDispatchProvider > SAL_CALL
        getMasterDispatchProvider() throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual void SAL_CALL
        setMasterDispatchProvider( const ::css::uno::Reference< ::css::frame::XDispatchProvider >& NewSupplier )
            throw (::css::uno::RuntimeException, ::std::exception) override;

    // XInterceptorInfo
    virtual ::css::uno::Sequence< OUString > SAL_CALL
        getInterceptedURLs() throw (::css::uno::RuntimeException, ::std::exception) override;

    // XElementAccess
    virtual ::css::uno::Type SAL_CALL getElementType()
        throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual ::sal_Bool SAL_CALL hasElements()
        throw (::css::uno::RuntimeException, ::std::exception) override;

    // XNameAccess
    virtual ::css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (::css::container::NoSuchElementException, ::css::lang::WrappedTargetException,
               ::css::uno::RuntimeException, ::std::exception) override;
    virtual ::css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw (::css::uno::RuntimeException, ::std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (::css::uno::RuntimeException, ::std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::css::uno::Any& aElement )
        throw (::css::lang::IllegalArgumentException, ::css::container::NoSuchElementException,
               ::css::lang::WrappedTargetException, ::css::uno::RuntimeException, ::std::exception) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::css::uno::Any& aElement )
        throw (::css::lang::IllegalArgumentException, ::css::container::ElementExistException,
               ::css::lang::WrappedTargetException, ::css::uno::RuntimeException, ::std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw (::css::container::NoSuchElementException, ::css::lang::WrappedTargetException,
               ::css::uno::RuntimeException, ::std::exception) override;

    DECLARE_XSERVICEINFO
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_SERVICES_DISPATCH_DISABLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
