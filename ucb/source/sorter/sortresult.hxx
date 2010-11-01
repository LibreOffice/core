/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#define LISTACTION  com::sun::star::ucb::ListAction

class EventList
{
    std::deque < LISTACTION* > maData;

public:
                     EventList(){}
                    ~EventList(){ Clear(); }

    sal_uInt32      Count() { return (sal_uInt32) maData.size(); }

    void            AddEvent( long nType, long nPos, long nCount );
    void            Insert( LISTACTION *pAction ) { maData.push_back( pAction ); }
    void            Clear();
    LISTACTION*     GetAction( long nIndex ) { return maData[ nIndex ]; }
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

#define PROPERTYCHANGEEVENT     com::sun::star::beans::PropertyChangeEvent
#define RUNTIME_EXCEPTION       com::sun::star::uno::RuntimeException
#define REFERENCE               com::sun::star::uno::Reference
#define SEQUENCE                com::sun::star::uno::Sequence
#define XEVENTLISTENER          com::sun::star::lang::XEventListener
#define XRESULTSET              com::sun::star::sdbc::XResultSet
#define SQLEXCEPTION            com::sun::star::sdbc::SQLException
#define XRESULTSETMETADATA      com::sun::star::sdbc::XResultSetMetaData
#define NUMBERED_SORTINGINFO    com::sun::star::ucb::NumberedSortingInfo
#define XANYCOMPAREFACTORY      com::sun::star::ucb::XAnyCompareFactory

#define RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedResultSet"

//-----------------------------------------------------------------------------

class SortedResultSet:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XComponent,
                public com::sun::star::ucb::XContentAccess,
                public XRESULTSET,
                public com::sun::star::sdbc::XRow,
                public com::sun::star::sdbc::XCloseable,
                public com::sun::star::sdbc::XResultSetMetaDataSupplier,
                public com::sun::star::beans::XPropertySet
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;
    PropertyChangeListeners_Impl    *mpPropChangeListeners;
    PropertyChangeListeners_Impl    *mpVetoChangeListeners;

    REFERENCE < XRESULTSET >            mxOriginal;
    REFERENCE < XRESULTSET >            mxOther;

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
                            throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    long                Compare( SortListData *pOne,
                                 SortListData *pTwo )
                            throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    void                BuildSortInfo( REFERENCE< XRESULTSET > aResult,
                                       const SEQUENCE < NUMBERED_SORTINGINFO > &xSortInfo,
                                       const REFERENCE< XANYCOMPAREFACTORY > &xCompFac );
    long                CompareImpl( REFERENCE < XRESULTSET > xResultOne,
                                     REFERENCE < XRESULTSET > xResultTwo,
                                     long nIndexOne, long nIndexTwo,
                                     SortInfo* pSortInfo )
                            throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    long                CompareImpl( REFERENCE < XRESULTSET > xResultOne,
                                     REFERENCE < XRESULTSET > xResultTwo,
                                     long nIndexOne, long nIndexTwo )
                            throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    void                PropertyChanged( const PROPERTYCHANGEEVENT& rEvt );

public:
                        SortedResultSet( REFERENCE< XRESULTSET > aResult );
                        ~SortedResultSet();

    const SortedEntryList*      GetS2OList() const { return &maS2O; }
    const SimpleList*           GetO2SList() const { return &maO2S; }
    REFERENCE < XRESULTSET >    GetResultSet() const { return mxOriginal; }
    SortInfo*                   GetSortInfo() const { return mpSortInfo; }
    long                        GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const SEQUENCE < NUMBERED_SORTINGINFO > &xSortInfo,
                                    const REFERENCE< XANYCOMPAREFACTORY > &xCompFac );
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
    dispose() throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    // XContentAccess
    virtual rtl::OUString SAL_CALL
    queryContentIdentifierString()
        throw( RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent()
        throw( RUNTIME_EXCEPTION );

    // XResultSet
    virtual sal_Bool SAL_CALL
    next()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    isLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    beforeFirst()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    afterLast()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    first()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    last()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    previous()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual void SAL_CALL
    refreshRow()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::uno::XInterface > SAL_CALL
    getStatement()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull() throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
    throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual REFERENCE<
                com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual REFERENCE<
                com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const REFERENCE<
                   com::sun::star::container::XNameAccess >& typeMap )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );
    virtual REFERENCE<
                com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XCloseable
    virtual void SAL_CALL
    close()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );

    // XResultSetMetaDataSupplier
    virtual REFERENCE< XRESULTSETMETADATA > SAL_CALL
    getMetaData()
        throw( SQLEXCEPTION, RUNTIME_EXCEPTION );


    // XPropertySet
    virtual REFERENCE<
                com::sun::star::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    setPropertyValue( const rtl::OUString& PropertyName,
                      const com::sun::star::uno::Any& Value )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::beans::PropertyVetoException,
               com::sun::star::lang::IllegalArgumentException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual com::sun::star::uno::Any SAL_CALL
    getPropertyValue( const rtl::OUString& PropertyName )
        throw( com::sun::star::beans::UnknownPropertyException,
        com::sun::star::lang::WrappedTargetException,
        RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addPropertyChangeListener( const rtl::OUString& PropertyName,
                               const REFERENCE<
                                       com::sun::star::beans::XPropertyChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removePropertyChangeListener( const rtl::OUString& PropertyName,
                                  const REFERENCE<
                                      com::sun::star::beans::XPropertyChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addVetoableChangeListener( const rtl::OUString& PropertyName,
                               const REFERENCE<
                                       com::sun::star::beans::XVetoableChangeListener >& Listener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                  const REFERENCE<
                                      com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::lang::WrappedTargetException,
               RUNTIME_EXCEPTION );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
