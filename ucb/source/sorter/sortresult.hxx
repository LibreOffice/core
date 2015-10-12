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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
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
#include <deque>

namespace cppu {
    class OInterfaceContainerHelper;
}


struct  SortInfo;
struct  SortListData;
class   SRSPropertySetInfo;
class   PropertyChangeListeners_Impl;


class SortedEntryList
{
    std::deque < SortListData* > maData;

public:
                         SortedEntryList(){}
                        ~SortedEntryList(){ Clear(); }

    sal_uInt32          Count() const { return (sal_uInt32) maData.size(); }

    void                Clear();
    void                Insert( SortListData *pEntry, sal_IntPtr nPos );
    SortListData*       Remove( sal_IntPtr nPos );
    SortListData*       GetData( sal_IntPtr nPos );

    sal_IntPtr                operator [] ( sal_IntPtr nPos ) const;
};



class EventList
{
    std::deque < css::ucb::ListAction* > maData;

public:
                     EventList(){}
                    ~EventList(){ Clear(); }

    sal_uInt32      Count() { return (sal_uInt32) maData.size(); }

    void            AddEvent( sal_IntPtr nType, sal_IntPtr nPos, sal_IntPtr nCount );
    void            Insert( css::ucb::ListAction *pAction ) { maData.push_back( pAction ); }
    void            Clear();
    css::ucb::ListAction*     GetAction( sal_IntPtr nIndex ) { return maData[ nIndex ]; }
};



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
    sal_IntPtr          mnLastSort;     // index of the last sorted entry;
    sal_IntPtr          mnCurEntry;     // index of the current entry
    sal_IntPtr          mnCount;        // total count of the elements
    bool                mbIsCopy;


private:

    sal_IntPtr          FindPos( SortListData *pEntry, sal_IntPtr nStart, sal_IntPtr nEnd )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    sal_IntPtr          Compare( SortListData *pOne,
                                 SortListData *pTwo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    void                BuildSortInfo( css::uno::Reference< css::sdbc::XResultSet > aResult,
                                       const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                       const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    static sal_IntPtr   CompareImpl( css::uno::Reference < css::sdbc::XResultSet > xResultOne,
                                     css::uno::Reference < css::sdbc::XResultSet > xResultTwo,
                                     sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo,
                                     SortInfo* pSortInfo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    sal_IntPtr          CompareImpl( css::uno::Reference < css::sdbc::XResultSet > xResultOne,
                                     css::uno::Reference < css::sdbc::XResultSet > xResultTwo,
                                     sal_IntPtr nIndexOne, sal_IntPtr nIndexTwo )
                            throw( css::sdbc::SQLException, css::uno::RuntimeException );
    void                PropertyChanged( const css::beans::PropertyChangeEvent& rEvt );

public:
                        SortedResultSet( css::uno::Reference< css::sdbc::XResultSet > aResult );
                        virtual ~SortedResultSet();

    const SortedEntryList&      GetS2OList() const { return maS2O; }
    const SimpleList&           GetO2SList() const { return maO2S; }
    css::uno::Reference < css::sdbc::XResultSet >    GetResultSet() const { return mxOriginal; }
    SortInfo*                   GetSortInfo() const { return mpSortInfo; }
    sal_IntPtr                  GetCount() const { return mnCount; }

    void                CopyData( SortedResultSet* pSource );
    void                Initialize( const css::uno::Sequence < css::ucb::NumberedSortingInfo > &xSortInfo,
                                    const css::uno::Reference< css::ucb::XAnyCompareFactory > &xCompFac );
    void                CheckProperties( sal_IntPtr nOldCount, bool bWasFinal );

    void                InsertNew( sal_IntPtr nPos, sal_IntPtr nCount );
    void                SetChanged( sal_IntPtr nPos, sal_IntPtr nCount );
    void                Remove( sal_IntPtr nPos, sal_IntPtr nCount, EventList *pList );
    void                Move( sal_IntPtr nPos, sal_IntPtr nCount, sal_IntPtr nOffset );

    void                ResortModified( EventList* pList );
    void                ResortNew( EventList* pList );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException,
               std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException,
               std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException,
               std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XComponent
    virtual void SAL_CALL
    dispose() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XContentAccess
    virtual OUString SAL_CALL
    queryContentIdentifierString()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::ucb::XContent > SAL_CALL
    queryContent()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XResultSet
    virtual sal_Bool SAL_CALL
    next()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    beforeFirst()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    afterLast()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    first()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    last()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    previous()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    refreshRow()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::uno::XInterface > SAL_CALL
    getStatement()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull() throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
    getString( sal_Int32 columnIndex )
    throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference<
                   css::container::XNameAccess >& typeMap )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    // XCloseable
    virtual void SAL_CALL
    close()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;

    // XResultSetMetaDataSupplier
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData()
        throw( css::sdbc::SQLException, css::uno::RuntimeException, std::exception ) override;


    // XPropertySet
    virtual css::uno::Reference<
                css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    setPropertyValue( const OUString& PropertyName,
                      const css::uno::Any& Value )
        throw( css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName )
        throw( css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addPropertyChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XPropertyChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removePropertyChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XPropertyChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addVetoableChangeListener( const OUString& PropertyName,
                               const css::uno::Reference<
                                       css::beans::XVetoableChangeListener >& Listener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference<
                                      css::beans::XVetoableChangeListener >& aListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
