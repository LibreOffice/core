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

#ifndef INCLUDED_UCB_SOURCE_SORTER_SORTRESULT_HXX
#define INCLUDED_UCB_SOURCE_SORTER_SORTRESULT_HXX

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
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <deque>
#include <memory>

namespace comphelper {
    class OInterfaceContainerHelper2;
}


struct  SortInfo;
struct  SortListData;
class   SRSPropertySetInfo;
class   PropertyChangeListeners_Impl;


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
    std::deque < std::unique_ptr<css::ucb::ListAction> > maData;

public:
                     EventList(){}

    sal_uInt32      Count() { return static_cast<sal_uInt32>(maData.size()); }

    void            AddEvent( sal_IntPtr nType, sal_Int32 nPos );
    void            Insert( std::unique_ptr<css::ucb::ListAction> pAction ) { maData.push_back( std::move(pAction) ); }
    void            Clear();
    css::ucb::ListAction*     GetAction( sal_Int32 nIndex ) { return maData[ nIndex ].get(); }
};


#define RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedResultSet"


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
    comphelper::OInterfaceContainerHelper2 *mpDisposeEventListeners;
    std::unique_ptr<PropertyChangeListeners_Impl>    mpPropChangeListeners;
    std::unique_ptr<PropertyChangeListeners_Impl>    mpVetoChangeListeners;

    css::uno::Reference < css::sdbc::XResultSet >            mxOriginal;
    css::uno::Reference < css::sdbc::XResultSet >            mxOther;

    rtl::Reference<SRSPropertySetInfo> mpPropSetInfo;
    SortInfo*           mpSortInfo;
    osl::Mutex          maMutex;
    SortedEntryList     maS2O;          // maps the sorted entries to the original ones
    std::deque<sal_IntPtr> m_O2S;       /// maps the original Entries to the sorted ones
    std::deque<SortListData*> m_ModList; /// keeps track of modified entries
    sal_Int32          mnLastSort;     // index of the last sorted entry;
    sal_Int32          mnCurEntry;     // index of the current entry
    sal_Int32          mnCount;        // total count of the elements
    bool                mbIsCopy;


private:
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    sal_Int32          FindPos( SortListData const *pEntry, sal_IntPtr nStart, sal_IntPtr nEnd );
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    sal_Int32          Compare( SortListData const *pOne,
                                 SortListData const *pTwo );
    void                BuildSortInfo( const css::uno::Reference< css::sdbc::XResultSet >& aResult,
                                       const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                       const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    static sal_Int32   CompareImpl( const css::uno::Reference < css::sdbc::XResultSet >& xResultOne,
                                     const css::uno::Reference < css::sdbc::XResultSet >& xResultTwo,
                                     sal_Int32 nIndexOne, sal_Int32 nIndexTwo,
                                     SortInfo const * pSortInfo );
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    sal_Int32          CompareImpl( const css::uno::Reference < css::sdbc::XResultSet >& xResultOne,
                                     const css::uno::Reference < css::sdbc::XResultSet >& xResultTwo,
                                     sal_Int32 nIndexOne, sal_Int32 nIndexTwo );
    void                PropertyChanged( const css::beans::PropertyChangeEvent& rEvt );

public:
                        SortedResultSet( css::uno::Reference< css::sdbc::XResultSet > const & aResult );
                        virtual ~SortedResultSet() override;

    sal_Int32          GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                    const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    void                CheckProperties( sal_Int32 nOldCount, bool bWasFinal );

    void                InsertNew( sal_Int32 nPos, sal_Int32 nCount );
    void                SetChanged( sal_Int32 nPos, sal_Int32 nCount );
    void                Remove( sal_Int32 nPos, sal_Int32 nCount, EventList *pList );
    void                Move( sal_Int32 nPos, sal_Int32 nCount, sal_Int32 nOffset );

    void                ResortModified( EventList* pList );
    void                ResortNew( EventList* pList );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL
    dispose() override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;

    // XContentAccess
    virtual OUString SAL_CALL
    queryContentIdentifierString() override;
    virtual css::uno::Reference<
                css::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier() override;
    virtual css::uno::Reference<
                css::ucb::XContent > SAL_CALL
    queryContent() override;

    // XResultSet
    virtual sal_Bool SAL_CALL
    next() override;
    virtual sal_Bool SAL_CALL
    isBeforeFirst() override;
    virtual sal_Bool SAL_CALL
    isAfterLast() override;
    virtual sal_Bool SAL_CALL
    isFirst() override;
    virtual sal_Bool SAL_CALL
    isLast() override;
    virtual void SAL_CALL
    beforeFirst() override;
    virtual void SAL_CALL
    afterLast() override;
    virtual sal_Bool SAL_CALL
    first() override;
    virtual sal_Bool SAL_CALL
    last() override;
    virtual sal_Int32 SAL_CALL
    getRow() override;
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row ) override;
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows ) override;
    virtual sal_Bool SAL_CALL
    previous() override;
    virtual void SAL_CALL
    refreshRow() override;
    virtual sal_Bool SAL_CALL
    rowUpdated() override;
    virtual sal_Bool SAL_CALL
    rowInserted() override;
    virtual sal_Bool SAL_CALL
    rowDeleted() override;
    virtual css::uno::Reference<
                css::uno::XInterface > SAL_CALL
    getStatement() override;

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull() override;

    virtual OUString SAL_CALL
    getString( sal_Int32 columnIndex ) override;

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex ) override;

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex ) override;

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex ) override;

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex ) override;

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex ) override;

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex ) override;

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex ) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex ) override;

    virtual css::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex ) override;

    virtual css::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex ) override;

    virtual css::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex ) override;

    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference<
                   css::container::XNameAccess >& typeMap ) override;
    virtual css::uno::Reference<
                css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex ) override;
    virtual css::uno::Reference<
                css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex ) override;
    virtual css::uno::Reference<
                css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex ) override;
    virtual css::uno::Reference<
                css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex ) override;

    // XCloseable
    virtual void SAL_CALL
    close() override;

    // XResultSetMetaDataSupplier
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData() override;


    // XPropertySet
    virtual css::uno::Reference<
                css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() override;

    virtual void SAL_CALL
    setPropertyValue( const OUString& PropertyName,
                      const css::uno::Any& Value ) override;

    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName ) override;

    virtual void SAL_CALL
    addPropertyChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XPropertyChangeListener >& Listener ) override;

    virtual void SAL_CALL
    removePropertyChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XPropertyChangeListener >& Listener ) override;

    virtual void SAL_CALL
    addVetoableChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XVetoableChangeListener >& Listener ) override;

    virtual void SAL_CALL
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XVetoableChangeListener >& aListener ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
