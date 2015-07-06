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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILRSET_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILRSET_HXX

#include <vector>
#include <osl/file.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/beans/Property.hpp>
#include "filrow.hxx"
#include "filnot.hxx"
#include <cppuhelper/implbase8.hxx>



namespace fileaccess {

    class Notifier;

class XResultSet_impl : public Notifier,
        public cppu::WeakImplHelper8< css::lang::XEventListener,
                                      css::sdbc::XRow,
                                      css::sdbc::XResultSet,
                                      css::ucb::XDynamicResultSet,
                                      css::sdbc::XCloseable,
                                      css::sdbc::XResultSetMetaDataSupplier,
                                      css::beans::XPropertySet,
                                      css::ucb::XContentAccess >
    {
    public:

        XResultSet_impl( shell* pMyShell,
                         const OUString& aUnqPath,
                         sal_Int32 OpenMode,
                         const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq,
                         const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >& seqSort );

        virtual ~XResultSet_impl();

        virtual ContentEventNotifier*        cDEL() SAL_OVERRIDE
        {
            return 0;
        }

        virtual ContentEventNotifier*        cEXC( const OUString& ) SAL_OVERRIDE
        {
            return 0;
        }

        virtual ContentEventNotifier*          cCEL() SAL_OVERRIDE
        {
            return 0;
        }

        virtual PropertySetInfoChangeNotifier* cPSL() SAL_OVERRIDE
        {
            return 0;
        }

        virtual PropertyChangeNotifier*        cPCL() SAL_OVERRIDE
        {
            return 0;
        }

        sal_Int32 SAL_CALL CtorSuccess() { return m_nErrorCode;}
        sal_Int32 SAL_CALL getMinorError() { return m_nMinorErrorCode;}

        // XEventListener
        virtual void SAL_CALL
        disposing(
            const com::sun::star::lang::EventObject& Source )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XComponent
        virtual void SAL_CALL
        dispose(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual void SAL_CALL
        addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual void SAL_CALL
        removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


        // XRow
        virtual sal_Bool SAL_CALL
        wasNull(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            if( 0<= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                m_nWasNull = m_aItems[m_nRow]->wasNull();
            else
                m_nWasNull = true;
            return m_nWasNull;
        }

        virtual OUString SAL_CALL
        getString(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getString( columnIndex );
            else
                return OUString();
        }

        virtual sal_Bool SAL_CALL
        getBoolean(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBoolean( columnIndex );
            else
                return false;
        }

        virtual sal_Int8 SAL_CALL
        getByte(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getByte( columnIndex );
            else
                return sal_Int8( 0 );
        }

        virtual sal_Int16 SAL_CALL
        getShort(
            sal_Int32 columnIndex )
            throw(
                com::sun::star::sdbc::SQLException,
                com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getShort( columnIndex );
            else
                return sal_Int16( 0 );
        }

        virtual sal_Int32 SAL_CALL
        getInt(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getInt( columnIndex );
            else
                return sal_Int32( 0 );
        }

        virtual sal_Int64 SAL_CALL
        getLong(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getLong( columnIndex );
            else
                return sal_Int64( 0 );
        }

        virtual float SAL_CALL
        getFloat(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getFloat( columnIndex );
            else
                return float( 0 );
        }

        virtual double SAL_CALL
        getDouble(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDouble( columnIndex );
            else
                return double( 0 );
        }

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getBytes(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBytes( columnIndex );
            else
                return com::sun::star::uno::Sequence< sal_Int8 >();
        }

        virtual com::sun::star::util::Date SAL_CALL
        getDate(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDate( columnIndex );
            else
                return com::sun::star::util::Date();
        }

        virtual com::sun::star::util::Time SAL_CALL
        getTime(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTime( columnIndex );
            else
                return com::sun::star::util::Time();
        }

        virtual com::sun::star::util::DateTime SAL_CALL
        getTimestamp(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTimestamp( columnIndex );
            else
                return com::sun::star::util::DateTime();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getBinaryStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBinaryStream( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::io::XInputStream >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getCharacterStream(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getCharacterStream( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::io::XInputStream >();
        }

        virtual com::sun::star::uno::Any SAL_CALL
        getObject(
            sal_Int32 columnIndex,
            const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& typeMap )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getObject( columnIndex,typeMap );
            else
                return com::sun::star::uno::Any();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRef > SAL_CALL
        getRef(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getRef( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XRef >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XBlob > SAL_CALL
        getBlob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBlob( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XBlob >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XClob > SAL_CALL
        getClob(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getClob( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XClob >();
        }

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XArray > SAL_CALL
        getArray(
            sal_Int32 columnIndex )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getArray( columnIndex );
            else
                return com::sun::star::uno::Reference< com::sun::star::sdbc::XArray >();
        }


        // XResultSet

        virtual sal_Bool SAL_CALL
        next(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        isBeforeFirst(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        isAfterLast(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        isFirst(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        isLast(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        beforeFirst(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        afterLast(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        first(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        last(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Int32 SAL_CALL
        getRow(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        absolute(
            sal_Int32 row )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        relative(
            sal_Int32 rows )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        previous(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        refreshRow(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        rowUpdated(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        rowInserted(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL
        rowDeleted(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        virtual  com::sun::star::uno::Reference<  com::sun::star::uno::XInterface > SAL_CALL
        getStatement(
            void  )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XDynamicResultSet

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet > SAL_CALL
        getStaticResultSet(
            void )
            throw( com::sun::star::ucb::ListenerAlreadySetException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual void SAL_CALL
        setListener(
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XDynamicResultSetListener >& Listener )
            throw( com::sun::star::ucb::ListenerAlreadySetException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual void SAL_CALL
        connectToCache( const com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet > & xCache )
            throw( com::sun::star::ucb::ListenerAlreadySetException,
                   com::sun::star::ucb::AlreadyInitializedException,
                   com::sun::star::ucb::ServiceNotFoundException,
                   com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Int16 SAL_CALL
        getCapabilities()
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


        // XCloseable

        virtual void SAL_CALL
        close(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XContentAccess

        virtual OUString SAL_CALL
        queryContentIdentifierString(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        queryContentIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XResultSetMetaDataSupplier
        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSetMetaData > SAL_CALL
        getMetaData(
            void )
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XPropertySet
        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
            throw( com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL setPropertyValue(
            const OUString& aPropertyName,
            const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::PropertyVetoException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual com::sun::star::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:

        // Members
        //  const uno::Reference< lang::XMultiServiceFactory > m_xMSF;
        //  const uno::Reference< ucb::XContentProvider > m_xProvider;

        shell*                              m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >  m_xProvider;
        bool                            m_nIsOpen;
        sal_Int32                           m_nRow;
        bool                            m_nWasNull;
        sal_Int32                           m_nOpenMode;
        bool                            m_bRowCountFinal;

        typedef std::vector< com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > > IdentSet;
        typedef std::vector< com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > >         ItemSet;
        typedef std::vector< OUString >                        UnqPathSet;

        IdentSet                            m_aIdents;
        ItemSet                             m_aItems;
        UnqPathSet                          m_aUnqPath;
        const OUString                 m_aBaseDirectory;

        osl::Directory                        m_aFolder;
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >      m_sProperty;
        com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >  m_sSortingInfo;

        osl::Mutex                          m_aMutex;
        osl::Mutex                          m_aEventListenerMutex;
        cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;

        cppu::OInterfaceContainerHelper*    m_pRowCountListeners;
        cppu::OInterfaceContainerHelper*    m_pIsFinalListeners;

        com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >       m_xListener;
        bool                            m_bStatic;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Methods
        bool SAL_CALL OneMore()
            throw( com::sun::star::sdbc::SQLException,
                   com::sun::star::uno::RuntimeException );

        void rowCountChanged();
        void isFinalChanged();
    };


} // end namespace fileaccess


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
