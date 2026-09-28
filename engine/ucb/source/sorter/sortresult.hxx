/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/ListAction.hpp>
#include <comphelper/multiinterfacecontainer3.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <deque>
#include <memory>


struct  SortInfo;
struct  SortListData;
class   SRSPropertySetInfo;


class SortedEntryList
{
    std::deque < std::unique_ptr<SortListData> > maData;

public:
                        SortedEntryList();
                        ~SortedEntryList();

    sal_uInt32          Count() const { return static_cast<sal_uInt32>(maData.size()); }

    void                Clear();
    void                Insert( std::unique_ptr<SortListData> pEntry, sal_Int32 nPos );
    std::unique_ptr<SortListData> Remove( sal_Int32 nPos );
    SortListData*       GetData( sal_Int32 nPos );
    void                Move( sal_Int32 nOldPos, sal_Int32 nNewPos );

    sal_Int32          operator [] ( sal_Int32 nPos ) const;
};


class EventList
{
    std::deque <css::ucb::ListAction > maData;

public:
                     EventList(){}

    sal_uInt32      Count() const { return static_cast<sal_uInt32>(maData.size()); }

    void            AddEvent( sal_IntPtr nType, sal_Int32 nPos );
    void            Insert( const css::ucb::ListAction& rAction ) { maData.push_back( rAction ); }
    void            Clear();
    css::ucb::ListAction&  GetAction( sal_Int32 nIndex ) { return maData[ nIndex ]; }
};


inline constexpr OUString RESULTSET_SERVICE_NAME = u"com.sun.star.ucb.SortedResultSet"_ustr;


class SortedResultSet: public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::lang::XComponent,
    css::ucb::XContentAccess,
    css::sdbc::XResultSet,
    css::sdbc::XRow,
    css::sdbc::XCloseable,
    css::sdbc::XResultSetMetaDataSupplier,
    css::beans::XPropertySet >
{
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> maDisposeEventListeners;
    comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XPropertyChangeListener>    maPropChangeListeners;
    comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XVetoableChangeListener>    maVetoChangeListeners;

    cpo::uno::Reference < css::sdbc::XResultSet >            mxOriginal;
    cpo::uno::Reference < css::sdbc::XResultSet >            mxOther;

    rtl::Reference<SRSPropertySetInfo> mpPropSetInfo;
    SortInfo*           mpSortInfo;
    std::mutex          maMutex;
    SortedEntryList     maS2O;          // maps the sorted entries to the original ones
    std::deque<sal_IntPtr> m_O2S;       /// maps the original Entries to the sorted ones
    std::deque<SortListData*> m_ModList; /// keeps track of modified entries
    sal_Int32          mnLastSort;     // index of the last sorted entry;
    sal_Int32          mnCurEntry;     // index of the current entry
    sal_Int32          mnCount;        // total count of the elements
    bool                mbIsCopy;


private:
    /// @throws css::sdbc::SQLException
    /// @throws cpo::uno::RuntimeException
    sal_Int32          FindPos( SortListData const *pEntry, sal_IntPtr nStart, sal_IntPtr nEnd );
    /// @throws css::sdbc::SQLException
    /// @throws cpo::uno::RuntimeException
    sal_Int32          Compare( SortListData const *pOne,
                                 SortListData const *pTwo );
    void                BuildSortInfo( const cpo::uno::Reference< css::sdbc::XResultSet >& aResult,
                                       const cpo::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                       const cpo::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    /// @throws css::sdbc::SQLException
    /// @throws cpo::uno::RuntimeException
    static sal_Int32   CompareImpl( const cpo::uno::Reference < css::sdbc::XResultSet >& xResultOne,
                                     const cpo::uno::Reference < css::sdbc::XResultSet >& xResultTwo,
                                     sal_Int32 nIndexOne, sal_Int32 nIndexTwo,
                                     SortInfo const * pSortInfo );
    /// @throws css::sdbc::SQLException
    /// @throws cpo::uno::RuntimeException
    sal_Int32          CompareImpl( const cpo::uno::Reference < css::sdbc::XResultSet >& xResultOne,
                                     const cpo::uno::Reference < css::sdbc::XResultSet >& xResultTwo,
                                     sal_Int32 nIndexOne, sal_Int32 nIndexTwo );
    void               PropertyChangedImpl(std::unique_lock<std::mutex>& rGuard, const css::beans::PropertyChangeEvent& rEvt);

public:
                        SortedResultSet( cpo::uno::Reference< css::sdbc::XResultSet > const & aResult );
                        virtual ~SortedResultSet() override;

    sal_Int32          GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const cpo::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                    const cpo::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    void                CheckProperties( sal_Int32 nOldCount, bool bWasFinal );

    void                InsertNew( sal_Int32 nPos, sal_Int32 nCount );
    void                SetChanged( sal_Int32 nPos, sal_Int32 nCount );
    void                Remove( sal_Int32 nPos, sal_Int32 nCount, EventList *pList );
    void                Move( sal_Int32 nPos, sal_Int32 nCount, sal_Int32 nOffset );

    void                ResortModified( EventList* pList );
    void                ResortNew( EventList* pList );

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XComponent
    virtual void
    dispose() override;

    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;

    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;

    // XContentAccess
    virtual OUString
    queryContentIdentifierString() override;
    virtual cpo::uno::Reference<
                css::ucb::XContentIdentifier >
    queryContentIdentifier() override;
    virtual cpo::uno::Reference<
                css::ucb::XContent >
    queryContent() override;

    // XResultSet
    virtual bool
    next() override;
    virtual bool
    isBeforeFirst() override;
    virtual bool
    isAfterLast() override;
    virtual bool
    isFirst() override;
    virtual bool
    isLast() override;
    virtual void
    beforeFirst() override;
    virtual void
    afterLast() override;
    virtual bool
    first() override;
    virtual bool
    last() override;
    virtual sal_Int32
    getRow() override;
    virtual bool
    absolute( sal_Int32 row ) override;
    virtual bool
    relative( sal_Int32 rows ) override;
    virtual bool
    previous() override;
    virtual void
    refreshRow() override;
    virtual bool
    rowUpdated() override;
    virtual bool
    rowInserted() override;
    virtual bool
    rowDeleted() override;
    virtual cpo::uno::Reference<
                cpo::uno::XInterface >
    getStatement() override;

    // XRow
    virtual bool
    wasNull() override;

    virtual OUString
    getString( sal_Int32 columnIndex ) override;

    virtual bool
    getBoolean( sal_Int32 columnIndex ) override;

    virtual sal_Int8
    getByte( sal_Int32 columnIndex ) override;

    virtual sal_Int16
    getShort( sal_Int32 columnIndex ) override;

    virtual sal_Int32
    getInt( sal_Int32 columnIndex ) override;

    virtual sal_Int64
    getLong( sal_Int32 columnIndex ) override;

    virtual float
    getFloat( sal_Int32 columnIndex ) override;

    virtual double
    getDouble( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Sequence< sal_Int8 >
    getBytes( sal_Int32 columnIndex ) override;

    virtual css::util::Date
    getDate( sal_Int32 columnIndex ) override;

    virtual css::util::Time
    getTime( sal_Int32 columnIndex ) override;

    virtual css::util::DateTime
    getTimestamp( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference<
                css::io::XInputStream >
    getBinaryStream( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference<
                css::io::XInputStream >
    getCharacterStream( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Any
    getObject( sal_Int32 columnIndex,
               const cpo::uno::Reference<
                   css::container::XNameAccess >& typeMap ) override;
    virtual cpo::uno::Reference<
                css::sdbc::XRef >
    getRef( sal_Int32 columnIndex ) override;
    virtual cpo::uno::Reference<
                css::sdbc::XBlob >
    getBlob( sal_Int32 columnIndex ) override;
    virtual cpo::uno::Reference<
                css::sdbc::XClob >
    getClob( sal_Int32 columnIndex ) override;
    virtual cpo::uno::Reference<
                css::sdbc::XArray >
    getArray( sal_Int32 columnIndex ) override;

    // XCloseable
    virtual void
    close() override;

    // XResultSetMetaDataSupplier
    virtual cpo::uno::Reference< css::sdbc::XResultSetMetaData >
    getMetaData() override;


    // XPropertySet
    virtual cpo::uno::Reference<
                css::beans::XPropertySetInfo >
    getPropertySetInfo() override;

    virtual void
    setPropertyValue( const OUString& PropertyName,
                      const cpo::uno::Any& Value ) override;

    virtual cpo::uno::Any
    getPropertyValue( const OUString& PropertyName ) override;

    virtual void
    addPropertyChangeListener( const OUString& PropertyName,
                               const cpo::uno::Reference<
                                       css::beans::XPropertyChangeListener >& Listener ) override;

    virtual void
    removePropertyChangeListener( const OUString& PropertyName,
                                  const cpo::uno::Reference<
                                      css::beans::XPropertyChangeListener >& Listener ) override;

    virtual void
    addVetoableChangeListener( const OUString& PropertyName,
                               const cpo::uno::Reference<
                                       css::beans::XVetoableChangeListener >& Listener ) override;

    virtual void
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const cpo::uno::Reference<
                                      css::beans::XVetoableChangeListener >& aListener ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
