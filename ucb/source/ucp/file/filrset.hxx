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

#include <comphelper/interfacecontainer2.hxx>
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
#include <cppuhelper/implbase.hxx>

namespace fileaccess {

class Notifier;

class XResultSet_impl : public Notifier,
        public cppu::WeakImplHelper<  css::lang::XEventListener,
                                      css::sdbc::XRow,
                                      css::sdbc::XResultSet,
                                      css::ucb::XDynamicResultSet,
                                      css::sdbc::XCloseable,
                                      css::sdbc::XResultSetMetaDataSupplier,
                                      css::beans::XPropertySet,
                                      css::ucb::XContentAccess >
    {
    public:

        XResultSet_impl( TaskManager* pMyShell,
                         const OUString& aUnqPath,
                         sal_Int32 OpenMode,
                         const css::uno::Sequence< css::beans::Property >& seq,
                         const css::uno::Sequence< css::ucb::NumberedSortingInfo >& seqSort );

        virtual ~XResultSet_impl() override;

        virtual ContentEventNotifier*        cDEL() override
        {
            return nullptr;
        }

        virtual ContentEventNotifier*        cEXC( const OUString& ) override
        {
            return nullptr;
        }

        virtual ContentEventNotifier*          cCEL() override
        {
            return nullptr;
        }

        virtual PropertySetInfoChangeNotifier* cPSL() override
        {
            return nullptr;
        }

        virtual PropertyChangeNotifier*        cPCL() override
        {
            return nullptr;
        }

        sal_Int32 SAL_CALL CtorSuccess() { return m_nErrorCode;}
        sal_Int32 SAL_CALL getMinorError() { return m_nMinorErrorCode;}

        // XEventListener
        virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source ) override;

        // XComponent
        virtual void SAL_CALL
        dispose() override;

        virtual void SAL_CALL
        addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

        virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;


        // XRow
        virtual sal_Bool SAL_CALL
        wasNull() override
        {
            if( 0<= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                m_nWasNull = m_aItems[m_nRow]->wasNull();
            else
                m_nWasNull = true;
            return m_nWasNull;
        }

        virtual OUString SAL_CALL
        getString( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getString( columnIndex );
            else
                return OUString();
        }

        virtual sal_Bool SAL_CALL
        getBoolean( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBoolean( columnIndex );
            else
                return false;
        }

        virtual sal_Int8 SAL_CALL
        getByte( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getByte( columnIndex );
            else
                return sal_Int8( 0 );
        }

        virtual sal_Int16 SAL_CALL
        getShort( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getShort( columnIndex );
            else
                return sal_Int16( 0 );
        }

        virtual sal_Int32 SAL_CALL
        getInt( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getInt( columnIndex );
            else
                return sal_Int32( 0 );
        }

        virtual sal_Int64 SAL_CALL
        getLong( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getLong( columnIndex );
            else
                return sal_Int64( 0 );
        }

        virtual float SAL_CALL
        getFloat( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getFloat( columnIndex );
            else
                return float( 0 );
        }

        virtual double SAL_CALL
        getDouble( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDouble( columnIndex );
            else
                return double( 0 );
        }

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getBytes( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBytes( columnIndex );
            else
                return css::uno::Sequence< sal_Int8 >();
        }

        virtual css::util::Date SAL_CALL
        getDate( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDate( columnIndex );
            else
                return css::util::Date();
        }

        virtual css::util::Time SAL_CALL
        getTime( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTime( columnIndex );
            else
                return css::util::Time();
        }

        virtual css::util::DateTime SAL_CALL
        getTimestamp( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTimestamp( columnIndex );
            else
                return css::util::DateTime();
        }

        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
        getBinaryStream( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBinaryStream( columnIndex );
            else
                return css::uno::Reference< css::io::XInputStream >();
        }

        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
        getCharacterStream( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getCharacterStream( columnIndex );
            else
                return css::uno::Reference< css::io::XInputStream >();
        }

        virtual css::uno::Any SAL_CALL
        getObject( sal_Int32 columnIndex,
            const css::uno::Reference< css::container::XNameAccess >& typeMap ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getObject( columnIndex,typeMap );
            else
                return css::uno::Any();
        }

        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL
        getRef( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getRef( columnIndex );
            else
                return css::uno::Reference< css::sdbc::XRef >();
        }

        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL
        getBlob( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBlob( columnIndex );
            else
                return css::uno::Reference< css::sdbc::XBlob >();
        }

        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL
        getClob( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getClob( columnIndex );
            else
                return css::uno::Reference< css::sdbc::XClob >();
        }

        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL
        getArray( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getArray( columnIndex );
            else
                return css::uno::Reference< css::sdbc::XArray >();
        }


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


        virtual  css::uno::Reference<  css::uno::XInterface > SAL_CALL
        getStatement() override;


        // XDynamicResultSet

        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
        getStaticResultSet() override;

        virtual void SAL_CALL
        setListener(
            const css::uno::Reference<
            css::ucb::XDynamicResultSetListener >& Listener ) override;

        virtual void SAL_CALL
        connectToCache( const css::uno::Reference< css::ucb::XDynamicResultSet > & xCache ) override;

        virtual sal_Int16 SAL_CALL
        getCapabilities() override;


        // XCloseable

        virtual void SAL_CALL
        close() override;

        // XContentAccess

        virtual OUString SAL_CALL
        queryContentIdentifierString() override;

        virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
        queryContentIdentifier() override;

        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        queryContent() override;

        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL
        getMetaData() override;


        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

        virtual void SAL_CALL setPropertyValue(
            const OUString& aPropertyName,
            const css::uno::Any& aValue ) override;

        virtual css::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName ) override;

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    private:

        TaskManager*                              m_pMyShell;
        bool                                m_nIsOpen;
        sal_Int32                           m_nRow;
        bool                                m_nWasNull;
        sal_Int32                           m_nOpenMode;
        bool                                m_bRowCountFinal;

        typedef std::vector< css::uno::Reference< css::ucb::XContentIdentifier > > IdentSet;
        typedef std::vector< css::uno::Reference< css::sdbc::XRow > >         ItemSet;
        typedef std::vector< OUString >                        UnqPathSet;

        IdentSet                            m_aIdents;
        ItemSet                             m_aItems;
        UnqPathSet                          m_aUnqPath;
        const OUString                 m_aBaseDirectory;

        osl::Directory                        m_aFolder;
        css::uno::Sequence< css::beans::Property >      m_sProperty;
        css::uno::Sequence< css::ucb::NumberedSortingInfo >  m_sSortingInfo;

        osl::Mutex                          m_aMutex;
        osl::Mutex                          m_aEventListenerMutex;
        comphelper::OInterfaceContainerHelper2*   m_pDisposeEventListeners;

        comphelper::OInterfaceContainerHelper2*   m_pRowCountListeners;
        comphelper::OInterfaceContainerHelper2*   m_pIsFinalListeners;

        css::uno::Reference< css::ucb::XDynamicResultSetListener >       m_xListener;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Methods
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        bool SAL_CALL OneMore();

        void rowCountChanged();
        void isFinalChanged();
    };


} // end namespace fileaccess


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
