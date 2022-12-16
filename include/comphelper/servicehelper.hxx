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

#ifndef INCLUDED_COMPHELPER_SERVICEHELPER_HXX
#define INCLUDED_COMPHELPER_SERVICEHELPER_HXX

#include <rtl/uuid.h>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Sequence.hxx>

namespace comphelper {

    // Class incapsulating UIDs used as e.g. tunnel IDs for css::lang::XUnoTunnel,
    // or implementation IDs for css::lang::XTypeProvider
    class UnoIdInit
    {
    private:
        css::uno::Sequence< sal_Int8 > m_aSeq;
    public:
        UnoIdInit() : m_aSeq(16)
        {
            rtl_createUuid(reinterpret_cast<sal_uInt8*>(m_aSeq.getArray()), nullptr, true);
        }
        const css::uno::Sequence< sal_Int8 >& getSeq() const { return m_aSeq; }
    };

    inline sal_Int64 getSomething_cast(void* p)
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(p));
    }

    template<class T> inline T* getSomething_cast(sal_Int64 n)
    {
        return reinterpret_cast<T*>(sal::static_int_cast<sal_IntPtr>(n));
    }

    template <class T> T* getFromUnoTunnel(const css::uno::Reference<css::lang::XUnoTunnel>& xUT)
    {
        if (!xUT.is())
            return nullptr;

        return getSomething_cast<T>(xUT->getSomething(T::getUnoTunnelId()));
    }

    // Takes an interface
    template <class T> T* getFromUnoTunnel(const css::uno::Reference<css::uno::XInterface>& xIface)
    {
        return getFromUnoTunnel<T>(
            css::uno::Reference<css::lang::XUnoTunnel>{ xIface, css::uno::UNO_QUERY });
    }

    // Takes an Any
    template <class T> T* getFromUnoTunnel(const css::uno::Any& rAny)
    {
        // For unclear reason, using a Reference ctor taking an Any
        // gives different results compared to use of operator >>=.
        css::uno::Reference<css::lang::XUnoTunnel> xUnoTunnel;
        rAny >>= xUnoTunnel;
        return getFromUnoTunnel<T>(xUnoTunnel);
    }

    template <typename T>
    inline bool isUnoTunnelId(const css::uno::Sequence< sal_Int8 >& rId)
    {
        return rId.getLength() == 16
            && memcmp(T::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0;
    }

    template<typename T> struct MixinToGetSomethingOf {
        static bool get(css::uno::Sequence<sal_Int8> const & id, T * p, sal_Int64 * result) {
            if (!isUnoTunnelId<T>(id)) {
                return false;
            }
            *result = getSomething_cast(p);
            return true;
        }
    };

    template <class Base> struct FallbackToGetSomethingOf
    {
        static sal_Int64 get(const css::uno::Sequence<sal_Int8>& rId, Base* p)
        {
            return p->Base::getSomething(rId);
        }
    };

    template <> struct FallbackToGetSomethingOf<void>
    {
        static sal_Int64 get(const css::uno::Sequence<sal_Int8>&, void*) { return 0; }
    };

    // There are five cases how to implement T::getSomething:
    // (1) Delegate to Base:
    //     Either, if Base has only getUnoTunnelId but no getSomething:
    //     return getSomethingImpl<Base>(aIdentifier, this);
    //     Or, if Base has getSomething:
    //     return Base::getSomething(aIdentifier)
    // (2) Check against T::getUnoTunnelId, else return 0:
    //     return getSomethingImpl(aIdentifier, this);
    // (3) Check against T::getUnoTunnelId, else delegate to Base:
    //     return getSomethingImpl(aIdentifier, this, FallbackToGetSomethingOf<Base>{});
    // (4) Check against T::getUnoTunnelId, else check against each Mixins::getUnoTunnelId, else
    //     delegate to Base:
    //     return getSomethingImpl(
    //         aIdentifier, this, MixinToGetSomethingOf<Mixin1>{}, ...,
    //         MixinToGetSomethingOf<MixinN>{}, FallbackToGetSomethingOf<Base>{});
    // (5) Check against each Mixins::getUnoTunnelId, else delegate to Base:
    //     return getSomethingImpl_skipDerived(
    //         aIdentifier, this, MixinToGetSomethingOf<Mixin1>{}, ...,
    //         MixinToGetSomethingOf<MixinN>{}, FallbackToGetSomethingOf<Base>{});

    template <class T, class Base = void>
    sal_Int64 getSomethingImpl(const css::uno::Sequence<sal_Int8>& rId, T* pThis,
        FallbackToGetSomethingOf<Base> = {})
    {
        sal_Int64 res;
        if (MixinToGetSomethingOf<T>::get(rId, pThis, &res)) {
            return res;
        }

        return FallbackToGetSomethingOf<Base>::get(rId, pThis);
    }

#if defined _MSC_VER && _MSC_VER < 1930 && !defined __clang__
    // Without this additional overload, at least VS 2019 16.11.21 has sometimes issues deducing the
    // Base template argument in calls to the "full" getSomethingImpl overload with zero arguments
    // substituted for the variadic Mixins parameter:
    template <class T, class Mixin, class Base>
    sal_Int64 getSomethingImpl(const css::uno::Sequence<sal_Int8>& rId, T* pThis,
        MixinToGetSomethingOf<Mixin>,
        FallbackToGetSomethingOf<Base>)
    {
        sal_Int64 res;
        if (MixinToGetSomethingOf<T>::get(rId, pThis, &res)
            || MixinToGetSomethingOf<Mixin>::get(rId, pThis, &res))
        {
            return res;
        }

        return FallbackToGetSomethingOf<Base>::get(rId, pThis);
    }
#endif

    template <class T, class Mixin, class... Mixins, class Base>
    sal_Int64 getSomethingImpl(const css::uno::Sequence<sal_Int8>& rId, T* pThis,
        MixinToGetSomethingOf<Mixin>, MixinToGetSomethingOf<Mixins>...,
        FallbackToGetSomethingOf<Base>)
    {
        sal_Int64 res;
        if (((MixinToGetSomethingOf<T>::get(rId, pThis, &res)
              || MixinToGetSomethingOf<Mixin>::get(rId, pThis, &res)) || ...
             || MixinToGetSomethingOf<Mixins>::get(rId, pThis, &res)))
        {
            return res;
        }

        return FallbackToGetSomethingOf<Base>::get(rId, pThis);
    }

    template <class T, class Mixin, class... Mixins, class Base>
    sal_Int64 getSomethingImpl_skipDerived(const css::uno::Sequence<sal_Int8>& rId, T* pThis,
        MixinToGetSomethingOf<Mixin>, MixinToGetSomethingOf<Mixins>...,
        FallbackToGetSomethingOf<Base>)
    {
        sal_Int64 res;
        if ((MixinToGetSomethingOf<Mixin>::get(rId, pThis, &res) || ...
             || MixinToGetSomethingOf<Mixins>::get(rId, pThis, &res)))
        {
            return res;
        }

        return FallbackToGetSomethingOf<Base>::get(rId, pThis);
    }

}

/** the UNO3_GETIMPLEMENTATION_* macros  implement a static helper function
    that gives access to your implementation for a given interface reference,
    if possible.

    Example:
        MyClass* pClass = comphelper::getFromUnoTunnel<MyClass>( xRef );

    Usage:
        Put a UNO3_GETIMPLEMENTATION_DECL( classname ) inside your class
        definition and UNO3_GETIMPLEMENTATION_IMPL( classname ) inside
        your cxx file. Your class must inherit css::lang::XUnoTunnel
        and export it with queryInterface. Implementation of XUnoTunnel is
        done by this macro.
*/
#define UNO3_GETIMPLEMENTATION_DECL( classname ) \
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() noexcept; \
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

#define UNO3_GETIMPLEMENTATION_BASE_IMPL( classname ) \
const css::uno::Sequence< sal_Int8 > & classname::getUnoTunnelId() noexcept \
{ \
    static const comphelper::UnoIdInit aId; \
    return aId.getSeq(); \
}

#define UNO3_GETIMPLEMENTATION_IMPL( classname )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) \
{ \
    return comphelper::getSomethingImpl(rId, this); \
}

#define UNO3_GETIMPLEMENTATION2_IMPL( classname, baseclass )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) \
{ \
    return comphelper::getSomethingImpl(rId, this, comphelper::FallbackToGetSomethingOf<baseclass>{}); \
}


#endif // INCLUDED_COMPHELPER_SERVICEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
