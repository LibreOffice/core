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

#ifndef _OHIERARCHYHOLDER_HXX_
#define _OHIERARCHYHOLDER_HXX_

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <cppuhelper/implbase1.hxx>

#include <comphelper/sequenceashashmap.hxx>

#include <rtl/ref.hxx>

#include <boost/unordered_map.hpp>
#include <list>
#include <vector>

struct OHierarchyElement_Impl;

struct eqFunc
{
    sal_Bool operator()( const OUString &r1,
                         const OUString &r2) const
    {
        return r1 == r2;
    }
};
typedef ::boost::unordered_map< OUString,
                         ::rtl::Reference< OHierarchyElement_Impl >,
                         OUStringHash,
                         eqFunc > OHierarchyElementList_Impl;

typedef ::std::vector< OUString > OStringList_Impl;
typedef ::std::list< ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XExtendedStorageStream > >
                        OWeakStorRefList_Impl;

struct OHierarchyElement_Impl : public cppu::WeakImplHelper1< ::com::sun::star::embed::XTransactionListener >
{
    ::osl::Mutex m_aMutex;

    ::rtl::Reference< OHierarchyElement_Impl > m_rParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xOwnStorage;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage > m_xWeakOwnStorage;

    OHierarchyElementList_Impl m_aChildren;

    OWeakStorRefList_Impl m_aOpenStreams;

public:
    OHierarchyElement_Impl( OHierarchyElement_Impl* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
    : m_rParent( pParent )
    , m_xOwnStorage( xStorage )
    {}

    OHierarchyElement_Impl( const ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage >& xWeakStorage )
    : m_rParent( NULL )
    , m_xWeakOwnStorage( xWeakStorage )
    {}

    void Commit();

    void SetParent( const ::rtl::Reference< OHierarchyElement_Impl >& rParent ) { m_rParent = rParent; }

    void TestForClosing();

    void RemoveElement( const ::rtl::Reference< OHierarchyElement_Impl >& aRef );

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream >
        GetStreamHierarchically( sal_Int32 nStorageMode,
                                OStringList_Impl& aPath,
                                sal_Int32 nStreamMode,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData = ::comphelper::SequenceAsHashMap() );

    void RemoveStreamHierarchically( OStringList_Impl& aListPath );

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // XTransactionListener
    virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

};

class OHierarchyHolder_Impl : public ::cppu::OWeakObject
{
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage > m_xWeakOwnStorage;
    ::rtl::Reference< OHierarchyElement_Impl > m_xChild;
public:
    OHierarchyHolder_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xOwnStorage )
    : m_xWeakOwnStorage( xOwnStorage )
    , m_xChild( new OHierarchyElement_Impl( ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage >( xOwnStorage ) ) )
    {}

    static OStringList_Impl GetListPathFromString( const OUString& aPath );

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream >
        GetStreamHierarchically( sal_Int32 nStorageMode,
                                OStringList_Impl& aListPath,
                                sal_Int32 nStreamMode,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData = ::comphelper::SequenceAsHashMap() );

    void RemoveStreamHierarchically( OStringList_Impl& aListPath );
};

#endif // _OHIERARCHYHOLDER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
