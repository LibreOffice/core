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

#ifndef _SORTRESULT_HXX
#define _SORTRESULT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/ListAction.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <deque>
#include <ucbhelper/macros.hxx>

namespace cppu {
    class OInterfaceContainerHelper;
}

//-----------------------------------------------------------------------------
struct  SortInfo;
struct  SortListData;
class   SRSPropertySetInfo;
class   PropertyChangeListeners_Impl;

//-----------------------------------------------------------------------------
class SortedEntryList
{
    std::deque < SortListData* > maData;

public:
                         SortedEntryList(){}
                        ~SortedEntryList(){ Clear(); }

    sal_uInt32          Count() const { return (sal_uInt32) maData.size(); }

    void                Clear();
    void                Insert( SortListData *pEntry, long nPos );
    SortListData*       Remove( long nPos );
    SortListData*       GetData( long nPos );

    long                operator [] ( long nPos ) const;
};

//-----------------------------------------------------------------------------

class EventList
{
    std::deque < css::ucb::ListAction* > maData;

public:
                     EventList(){}
                    ~EventList(){ Clear(); }

    sal_uInt32      Count() { return (sal_uInt32) maData.size(); }

    void            AddEvent( long nType, long nPos, long nCount );
    void            Insert( css::ucb::ListAction *pAction ) { maData.push_back( pAction ); }
    void            Clear();
    css::ucb::ListAction*     GetAction( long nIndex ) { return maData[ nIndex ]; }
};

//-----------------------------------------------------------------------------

class SimpleList
{
    std::deque < void* > maData;

public:
                     SimpleList(){}
                    ~SimpleList(){ Clear(); }

    sal_uInt32      Count() { return (sal_uInt32) maData.size(); }
    void            Clear() { maData.clear(); }

    void            Remove( sal_uInt32 nPos );
    void            Remove( void* pData );

    void            Append( void* pData )
                        { maData.push_back( pData ); }
    void            Insert( void* pData, sal_uInt32 nPos );
    void*           GetObject( sal_uInt32 nPos ) const;
    void            Replace( void* pData, sal_uInt32 nPos );
};

//-----------------------------------------------------------------------------

#define RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedResultSet"

//-----------------------------------------------------------------------------

class SortedResultSet:
                public cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::lang::XServiceInfo,
                public css::lang::XComponent,
                public css::ucb::XContentAccess,
                public css::sdbc::XResultSet,
                public css::sdbc::XRow,
                public css::sdbc::XCloseable,
                public css::sdbc::XResultSetMetaDataSupplier,
                public css::beans::XPropertySet
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;
    PropertyChangeListeners_Impl    *mpPropChangeListeners;
    PropertyChangeListeners_Impl    *mpVetoChangeListeners;

    css::uno::Reference < css::sdbc::XResultSet >            mxOriginal;
    css::uno::Reference < css::sdbc::XResultSet >            mxOther;

    SRSPropertySetInfo* mpPropSetInfo;
    SortInfo*           mpSortInfo;
    osl::Mutex          maMutex;
    SortedEntryList     maS2O;          // maps the sorted entries to the original ones
    SimpleList          maO2S;          // maps the original Entries to the sorted ones
    SimpleList          maModList;      // keeps track of modified entries
    long                mnLastSort;     // index of the last sorted entry;
    long                mnCurEntry;     // index of the current entry
    long                mnCount;        // total count of the elements
    sal_Bool            mbIsCopy;


private:

    long                FindPos( SortListData *pEntry, long nStart, long nEnd )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    long                Compare( SortListData *pOne,
                                 SortListData *pTwo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    void                BuildSortInfo( css::uno::Reference< css::sdbc::XResultSet > aResult,
                                       const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                       const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    long                CompareImpl( css::uno::Reference < css::sdbc::XResultSet > xResultOne,
                                     css::uno::Reference < css::sdbc::XResultSet > xResultTwo,
                                     long nIndexOne, long nIndexTwo,
                                     SortInfo* pSortInfo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    long                CompareImpl( css::uno::Reference < css::sdbc::XResultSet > xResultOne,
                                     css::uno::Reference < css::sdbc::XResultSet > xResultTwo,
                                     long nIndexOne, long nIndexTwo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    void                PropertyChanged( const css::beans::PropertyChangeEvent& rEvt );

public:
                        SortedResultSet( css::uno::Reference< css::sdbc::XResultSet > aResult );
                        ~SortedResultSet();

    const SortedEntryList*      GetS2OList() const { return &maS2O; }
    const SimpleList*           GetO2SList() const { return &maO2S; }
    css::uno::Reference < css::sdbc::XResultSet >    GetResultSet() const { return mxOriginal; }
    SortInfo*                   GetSortInfo() const { return mpSortInfo; }
    long                        GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                    const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    void                CheckProperties( long nOldCount, sal_Bool bWasFinal );

    void                InsertNew( long nPos, long nCount );
    void                SetChanged( long nPos, long nCount );
    void                Remove( long nPos, long nCount, EventList *pList );
    void                Move( long nPos, long nCount, long nOffset );

    void                ResortModified( EventList* pList );
    void                ResortNew( EventList* pList );

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose() throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException );

    // XContentAccess
    virtual OUString SAL_CALL
    queryContentIdentifierString()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::ucb::XContent > SAL_CALL
    queryContent()
        throw( css::uno::RuntimeException );

    // XResultSet
    virtual sal_Bool SAL_CALL
    next()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual void SAL_CALL
    beforeFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual void SAL_CALL
    afterLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    first()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    last()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    previous()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual void SAL_CALL
    refreshRow()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::uno::XInterface > SAL_CALL
    getStatement()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull() throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual OUString SAL_CALL
    getString( sal_Int32 columnIndex )
    throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference<
                   css::container::XNameAccess >& typeMap )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );
    virtual css::uno::Reference<
                css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    // XCloseable
    virtual void SAL_CALL
    close()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );

    // XResultSetMetaDataSupplier
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData()
        throw( css::sdbc::SQLException, css::uno::RuntimeException );


    // XPropertySet
    virtual css::uno::Reference<
                css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    setPropertyValue( const OUString& PropertyName,
                      const css::uno::Any& Value )
        throw( css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName )
        throw( css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException );

    virtual void SAL_CALL
    addPropertyChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XPropertyChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException );

    virtual void SAL_CALL
    removePropertyChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XPropertyChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException );

    virtual void SAL_CALL
    addVetoableChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XVetoableChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException );

    virtual void SAL_CALL
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XVetoableChangeListener >& aListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
