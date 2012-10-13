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
#ifndef _RESULTSETBASE_HXX
#define _RESULTSETBASE_HXX

#include <vector>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>


namespace chelp {

    class ResultSetBase
        : public cppu::OWeakObject,
          public com::sun::star::lang::XComponent,
          public com::sun::star::sdbc::XRow,
          public com::sun::star::sdbc::XResultSet,
          public com::sun::star::sdbc::XCloseable,
          public com::sun::star::sdbc::XResultSetMetaDataSupplier,
          public com::sun::star::beans::XPropertySet,
          public com::sun::star::ucb::XContentAccess
    {
    public:

        ResultSetBase( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&  xMSF,
                       const com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >&  xProvider,
                       sal_Int32 nOpenMode,
                       const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq,
                       const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >& seqSort );

        virtual ~ResultSetBase();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XComponent
        virtual void SAL_CALL
        dispose(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException );


        // XRow
        virtual sal_Bool SAL_CALL
        wasNull(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException )
        {
            if( 0<= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                m_nWasNull = m_aItems[m_nRow]->wasNull();
            else
                m_nWasNull = true;
            return m_nWasNull;
        }

        virtual OUString SAL_CALL
        getString(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getString( columnIndex );
            else
                return OUString();
        }

        virtual sal_Bool SAL_CALL
        getBoolean(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getBoolean( columnIndex );
            else
                return false;
        }

        virtual sal_Int8 SAL_CALL
        getByte(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getByte( columnIndex );
            else
                return sal_Int8( 0 );
        }

        virtual sal_Int16 SAL_CALL
        getShort(
            sal_Int32 columnIndex )
            throw(
                com::sun::star::sdbc::SQLException,
                com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getShort( columnIndex );
            else
                return sal_Int16( 0 );
        }

        virtual sal_Int32 SAL_CALL
        getInt(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException )
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getInt( columnIndex );
            else
                return sal_Int32( 0 );
        }

        virtual sal_Int64 SAL_CALL
        getLong(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getLong( columnIndex );
            else
                return sal_Int64( 0 );
        }

        virtual float SAL_CALL
        getFloat(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException )
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getFloat( columnIndex );
            else
                return float( 0 );
        }

        virtual double SAL_CALL
        getDouble(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException )
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getDouble( columnIndex );
            else
                return double( 0 );
        }

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getBytes(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException )
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getBytes( columnIndex );
            else
                return com::sun::star::uno::Sequence< sal_Int8 >();
        }

        virtual com::sun::star::util::Date SAL_CALL
        getDate(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getDate( columnIndex );
            else
                return com::sun::star::util::Date();
        }

        virtual com::sun::star::util::Time SAL_CALL
        getTime(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getTime( columnIndex );
            else
                return com::sun::star::util::Time();
        }

        virtual com::sun::star::util::DateTime SAL_CALL
        getTimestamp(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getTimestamp( columnIndex );
            else
                return com::sun::star::util::DateTime();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getBinaryStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getBinaryStream( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::io::XInputStream >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getCharacterStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getCharacterStream( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::io::XInputStream >();
        }

        virtual com::sun::star::uno::Any SAL_CALL
        getObject(
            sal_Int32 columnIndex,
            const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& typeMap )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getObject( columnIndex,typeMap );
            else
                return com::sun::star::uno::Any();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRef > SAL_CALL
        getRef(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getRef( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XRef >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XBlob > SAL_CALL
        getBlob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getBlob( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XBlob >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XClob > SAL_CALL
        getClob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getClob( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XClob >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XArray > SAL_CALL
        getArray(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException)
        {
            if( 0 <= m_nRow && sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aItems.size() )
                return m_aItems[m_nRow]->getArray( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XArray >();
        }


        // XResultSet

        virtual sal_Bool SAL_CALL
        next(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        isBeforeFirst(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        isAfterLast(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        isFirst(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        isLast(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        beforeFirst(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        afterLast(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        first(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        last(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL
        getRow(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        absolute(
            sal_Int32 row )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        relative(
            sal_Int32 rows )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        previous(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        refreshRow(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        rowUpdated(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        rowInserted(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        rowDeleted(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);


        virtual  com::sun::star::uno::Reference<  com::sun::star::uno::XInterface > SAL_CALL
        getStatement(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        // XCloseable

        virtual void SAL_CALL
        close(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);

        // XContentAccess

        virtual OUString SAL_CALL
        queryContentIdentifierString(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        queryContentIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XResultSetMetaDataSupplier
        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSetMetaData > SAL_CALL
        getMetaData(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException);


        // XPropertySet
        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setPropertyValue(
            const OUString& aPropertyName,
            const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::PropertyVetoException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException);

    protected:

        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >  m_xMSF;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >  m_xProvider;
        sal_Int32                           m_nRow;
        bool                                m_nWasNull;
        sal_Int32                           m_nOpenMode;
        bool                                m_bRowCountFinal;

        typedef std::vector< com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > > IdentSet;
        typedef std::vector< com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > >              ItemSet;
        typedef std::vector< OUString >                                                             PathSet;

        IdentSet                            m_aIdents;
        ItemSet                             m_aItems;
        PathSet                             m_aPath;

        com::sun::star::uno::Sequence< com::sun::star::beans::Property >           m_sProperty;
        com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >  m_sSortingInfo;

        osl::Mutex                          m_aMutex;
        cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;

        cppu::OInterfaceContainerHelper*    m_pRowCountListeners;
        cppu::OInterfaceContainerHelper*    m_pIsFinalListeners;
    };


} // end namespace fileaccess


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
