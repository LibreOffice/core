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
#ifndef INCLUDED_SW_INC_UNOBASECLASS_HXX
#define INCLUDED_SW_INC_UNOBASECLASS_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

class SfxPoolItem;
class SwClient;
class SwDoc;
class SwUnoTableCursor;

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::container::XEnumeration
>
SwSimpleEnumeration_Base;

enum class CursorType
{
    Body,
    Frame,
    TableText,
    Footnote,
    Header,
    Footer,
    Redline,
    All,         // for Search&Replace
    Selection,   // create a paragraph enumeration from
                        // a text range or cursor
    SelectionInTable,
    Meta,         // meta/meta-field
};

/*
    Start/EndAction or Start/EndAllAction
*/
class UnoActionContext
{
private:
        SwDoc * m_pDoc;

public:
        UnoActionContext(SwDoc *const pDoc);
        ~UnoActionContext();
};

/*
    interrupt Actions for a little while
    FIXME: this is a vile abomination that may cause recursive layout actions!
    C'thulhu fhtagn.
*/
class UnoActionRemoveContext
{
private:
        SwDoc *const m_pDoc;

public:
        UnoActionRemoveContext(SwDoc *const pDoc);
        UnoActionRemoveContext(SwUnoTableCursor const& rCursor);
        ~UnoActionRemoveContext();
};

/// helper function for implementing SwClient::Modify
void ClientModify(SwClient* pClient, const SfxPoolItem *pOld, const SfxPoolItem *pNew);

namespace sw {
    template<typename T>
    struct UnoImplPtrDeleter
    {
        void operator()(T* pUnoImpl)
        {
            SolarMutexGuard g; // #i105557#: call dtor with locked solar mutex
            delete pUnoImpl;
        }
    };
    /// Smart pointer class ensuring that the pointed object is deleted with a locked SolarMutex.
    template<typename T>
    using UnoImplPtr = std::unique_ptr<T, UnoImplPtrDeleter<T> >;

    template< class C > C *
    UnoTunnelGetImplementation( css::uno::Reference< css::lang::XUnoTunnel > const & xUnoTunnel)
    {
        if (!xUnoTunnel.is()) { return 0; }
        C *const pC( reinterpret_cast< C* >(
                        ::sal::static_int_cast< sal_IntPtr >(
                            xUnoTunnel->getSomething(C::getUnoTunnelId()))));
        return pC;
    }

    template< class C > sal_Int64
    UnoTunnelImpl(const css::uno::Sequence< sal_Int8 > & rId,
                  C *const pThis)
    {
        if ((rId.getLength() == 16) &&
            (0 == memcmp(C::getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16)))
        {
            return ::sal::static_int_cast< sal_Int64 >(
                    reinterpret_cast< sal_IntPtr >(pThis) );
        }
        return 0;
    }

    css::uno::Sequence< OUString >
    GetSupportedServiceNamesImpl(
            size_t const nServices, char const*const pServices[]);

} // namespace sw

#endif // INCLUDED_SW_INC_UNOBASECLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
