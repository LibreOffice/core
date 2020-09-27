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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OHIERARCHYHOLDER_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OHIERARCHYHOLDER_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <comphelper/sequenceashashmap.hxx>

#include <rtl/ref.hxx>

#include <unordered_map>
#include <vector>

struct OHierarchyElement_Impl;

typedef std::unordered_map< OUString,
                         ::rtl::Reference< OHierarchyElement_Impl > > OHierarchyElementList_Impl;

typedef ::std::vector< css::uno::WeakReference< css::embed::XExtendedStorageStream > >
                        OWeakStorRefVector_Impl;

struct OHierarchyElement_Impl : public cppu::WeakImplHelper< css::embed::XTransactionListener >
{
    ::osl::Mutex m_aMutex;

    ::rtl::Reference< OHierarchyElement_Impl > m_rParent;
    css::uno::Reference< css::embed::XStorage > m_xOwnStorage;
    css::uno::WeakReference< css::embed::XStorage > m_xWeakOwnStorage;

    OHierarchyElementList_Impl m_aChildren;

    OWeakStorRefVector_Impl m_aOpenStreams;

public:
    explicit OHierarchyElement_Impl( const css::uno::Reference< css::embed::XStorage >& xStorage )
    : m_xOwnStorage( xStorage )
    {}

    explicit OHierarchyElement_Impl( const css::uno::WeakReference< css::embed::XStorage >& xWeakStorage )
    : m_xWeakOwnStorage( xWeakStorage )
    {}

    void Commit();

    void SetParent( const ::rtl::Reference< OHierarchyElement_Impl >& rParent ) { m_rParent = rParent; }

    void TestForClosing();

    void RemoveElement( const ::rtl::Reference< OHierarchyElement_Impl >& aRef );

    css::uno::Reference< css::embed::XExtendedStorageStream >
        GetStreamHierarchically( sal_Int32 nStorageMode,
                                std::vector<OUString>& aPath,
                                sal_Int32 nStreamMode,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void RemoveStreamHierarchically( std::vector<OUString>& aListPath );

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XTransactionListener
    virtual void SAL_CALL preCommit( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL commited( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL preRevert( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL reverted( const css::lang::EventObject& aEvent ) override;

};

class OHierarchyHolder_Impl : public ::cppu::OWeakObject
{
    css::uno::WeakReference< css::embed::XStorage > m_xWeakOwnStorage;
    ::rtl::Reference< OHierarchyElement_Impl > m_xChild;
public:
    explicit OHierarchyHolder_Impl( const css::uno::Reference< css::embed::XStorage >& xOwnStorage )
    : m_xWeakOwnStorage( xOwnStorage )
    , m_xChild( new OHierarchyElement_Impl( css::uno::WeakReference< css::embed::XStorage >( xOwnStorage ) ) )
    {}

    static std::vector<OUString> GetListPathFromString( const OUString& aPath );

    css::uno::Reference< css::embed::XExtendedStorageStream >
        GetStreamHierarchically( sal_Int32 nStorageMode,
                                std::vector<OUString>& aListPath,
                                sal_Int32 nStreamMode,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData = ::comphelper::SequenceAsHashMap() );

    void RemoveStreamHierarchically( std::vector<OUString>& aListPath );
};

#endif // _OHIERARCHYHOLDER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
