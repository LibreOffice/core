/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cppuhelper/cppuhelperdllapi.h>
#include "interfacecontainer4.hxx"
#include "unoimplbase.hxx"
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <mutex>

/**
This is a straight copy of the include/comphelper/compbase.hxx file, copied here
because it is nigh impossible to move shared code down into the URE layer.
*/

namespace cppuhelper
{
/**
    Serves two purposes
    (1) extracts code that doesn't need to be templated
    (2) helps to handle the custom where we have conflicting interfaces
        e.g. multiple UNO interfaces that extend css::lang::XComponent
*/
class CPPUHELPER_DLLPUBLIC WeakComponentImplHelperBase2 : public virtual UnoImplBase,
                                                          public cppu::OWeakObject,
                                                          public css::lang::XComponent
{
public:
    virtual ~WeakComponentImplHelperBase2() override;

    // css::lang::XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL
    addEventListener(css::uno::Reference<css::lang::XEventListener> const& rxListener) override;
    virtual void SAL_CALL
    removeEventListener(css::uno::Reference<css::lang::XEventListener> const& rxListener) override;

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const& rType) override;

    /**
        Called by dispose for subclasses to do dispose() work.
        The mutex is held when called, and subclasses can unlock() the guard if necessary.
     */
    virtual void disposing(std::unique_lock<std::mutex>&);

protected:
    void throwIfDisposed(std::unique_lock<std::mutex>&)
    {
        if (m_bDisposed)
            throw css::lang::DisposedException(OUString(), static_cast<cppu::OWeakObject*>(this));
    }
    OInterfaceContainerHelper4<css::lang::XEventListener> maEventListeners;
};

/** WeakComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any
WeakComponentImplHelper_query(css::uno::Type const& rType, cppu::class_data* cd,
                              WeakComponentImplHelperBase2* pBase);

template <typename... Ifc>
class SAL_DLLPUBLIC_TEMPLATE WeakComponentImplHelper2 : public WeakComponentImplHelperBase2,
                                                        public css::lang::XTypeProvider,
                                                        public Ifc...
{
public:
    virtual void SAL_CALL acquire() noexcept override { OWeakObject::acquire(); }

    virtual void SAL_CALL release() noexcept override { OWeakObject::release(); }

    // css::lang::XComponent
    virtual void SAL_CALL dispose() noexcept final override
    {
        WeakComponentImplHelperBase2::dispose();
    }
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const& rxListener) final override
    {
        WeakComponentImplHelperBase2::addEventListener(rxListener);
    }
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const& rxListener) final override
    {
        WeakComponentImplHelperBase2::removeEventListener(rxListener);
    }

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const& rType) override
    {
        return WeakComponentImplHelper_query(rType, class_data_get(), this);
    }

    // css::lang::XTypeProvider
    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override
    {
        static const css::uno::Sequence<css::uno::Type> aTypeList{
            cppu::UnoType<css::uno::XWeak>::get(), cppu::UnoType<css::lang::XComponent>::get(),
            cppu::UnoType<css::lang::XTypeProvider>::get(), cppu::UnoType<Ifc>::get()...
        };
        return aTypeList;
    }
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override
    {
        return css::uno::Sequence<sal_Int8>();
    }

private:
    static cppu::class_data* class_data_get()
    {
        return cppu::detail::ImplClassData<WeakComponentImplHelper2, Ifc...>{}();
    }
};

} //  namespace cppuextra

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
