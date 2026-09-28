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

#include <mutex>
#include <vector>
#include <osl/file.hxx>

#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/beans/Property.hpp>
#include "filrow.hxx"
#include <cppuhelper/implbase.hxx>

namespace fileaccess {

class XResultSet_impl :
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
                         const cpo::uno::Sequence< css::beans::Property >& seq,
                         const cpo::uno::Sequence< css::ucb::NumberedSortingInfo >& seqSort );

        virtual ~XResultSet_impl() override;

        TaskHandlerErr CtorSuccess() const { return m_nErrorCode;}
        sal_Int32 getMinorError() const { return m_nMinorErrorCode;}

        // XEventListener
        virtual void
        disposing( const css::lang::EventObject& Source ) override;

        // XComponent
        virtual void
        dispose() override;

        virtual void
        addEventListener(
            const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;

        virtual void
        removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;


        // XRow
        virtual bool
        wasNull() override
        {
            if( 0<= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                m_nWasNull = m_aItems[m_nRow]->wasNull();
            else
                m_nWasNull = true;
            return m_nWasNull;
        }

        virtual OUString
        getString( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getString( columnIndex );
            else
                return OUString();
        }

        virtual bool
        getBoolean( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBoolean( columnIndex );
            else
                return false;
        }

        virtual sal_Int8
        getByte( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getByte( columnIndex );
            else
                return sal_Int8( 0 );
        }

        virtual sal_Int16
        getShort( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getShort( columnIndex );
            else
                return sal_Int16( 0 );
        }

        virtual sal_Int32
        getInt( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getInt( columnIndex );
            else
                return 0;
        }

        virtual sal_Int64
        getLong( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getLong( columnIndex );
            else
                return sal_Int64( 0 );
        }

        virtual float
        getFloat( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getFloat( columnIndex );
            else
                return float( 0 );
        }

        virtual double
        getDouble( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDouble( columnIndex );
            else
                return double( 0 );
        }

        virtual cpo::uno::Sequence< sal_Int8 >
        getBytes( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBytes( columnIndex );
            else
                return cpo::uno::Sequence< sal_Int8 >();
        }

        virtual css::util::Date
        getDate( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getDate( columnIndex );
            else
                return css::util::Date();
        }

        virtual css::util::Time
        getTime( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTime( columnIndex );
            else
                return css::util::Time();
        }

        virtual css::util::DateTime
        getTimestamp( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getTimestamp( columnIndex );
            else
                return css::util::DateTime();
        }

        virtual cpo::uno::Reference< css::io::XInputStream >
        getBinaryStream( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBinaryStream( columnIndex );
            else
                return cpo::uno::Reference< css::io::XInputStream >();
        }

        virtual cpo::uno::Reference< css::io::XInputStream >
        getCharacterStream( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getCharacterStream( columnIndex );
            else
                return cpo::uno::Reference< css::io::XInputStream >();
        }

        virtual cpo::uno::Any
        getObject( sal_Int32 columnIndex,
            const cpo::uno::Reference< css::container::XNameAccess >& typeMap ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getObject( columnIndex,typeMap );
            else
                return cpo::uno::Any();
        }

        virtual cpo::uno::Reference< css::sdbc::XRef >
        getRef( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getRef( columnIndex );
            else
                return cpo::uno::Reference< css::sdbc::XRef >();
        }

        virtual cpo::uno::Reference< css::sdbc::XBlob >
        getBlob( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getBlob( columnIndex );
            else
                return cpo::uno::Reference< css::sdbc::XBlob >();
        }

        virtual cpo::uno::Reference< css::sdbc::XClob >
        getClob( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getClob( columnIndex );
            else
                return cpo::uno::Reference< css::sdbc::XClob >();
        }

        virtual cpo::uno::Reference< css::sdbc::XArray >
        getArray( sal_Int32 columnIndex ) override
        {
            if( 0 <= m_nRow && m_nRow < sal::static_int_cast<sal_Int32>(m_aItems.size()) )
                return m_aItems[m_nRow]->getArray( columnIndex );
            else
                return cpo::uno::Reference< css::sdbc::XArray >();
        }


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


        virtual  cpo::uno::Reference<  cpo::uno::XInterface >
        getStatement() override;


        // XDynamicResultSet

        virtual cpo::uno::Reference< css::sdbc::XResultSet >
        getStaticResultSet() override;

        virtual void
        setListener(
            const cpo::uno::Reference<
            css::ucb::XDynamicResultSetListener >& Listener ) override;

        virtual void
        connectToCache( const cpo::uno::Reference< css::ucb::XDynamicResultSet > & xCache ) override;

        virtual sal_Int16
        getCapabilities() override;


        // XCloseable

        virtual void
        close() override;

        // XContentAccess

        virtual OUString
        queryContentIdentifierString() override;

        virtual cpo::uno::Reference< css::ucb::XContentIdentifier >
        queryContentIdentifier() override;

        virtual cpo::uno::Reference< css::ucb::XContent >
        queryContent() override;

        // XResultSetMetaDataSupplier
        virtual cpo::uno::Reference< css::sdbc::XResultSetMetaData >
        getMetaData() override;


        // XPropertySet
        virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
        getPropertySetInfo() override;

        virtual void setPropertyValue(
            const OUString& aPropertyName,
            const cpo::uno::Any& aValue ) override;

        virtual cpo::uno::Any
        getPropertyValue(
            const OUString& PropertyName ) override;

        virtual void
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

        virtual void
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

        virtual void
        addVetoableChangeListener(
            const OUString& PropertyName,
            const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        virtual void removeVetoableChangeListener(
            const OUString& PropertyName,
            const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    private:

        TaskManager*                              m_pMyShell;
        bool                                m_nIsOpen;
        sal_Int32                           m_nRow;
        bool                                m_nWasNull;
        sal_Int32                           m_nOpenMode;
        bool                                m_bRowCountFinal;

        typedef std::vector< cpo::uno::Reference< css::ucb::XContentIdentifier > > IdentSet;
        typedef std::vector< cpo::uno::Reference< css::sdbc::XRow > >         ItemSet;

        IdentSet                            m_aIdents;
        ItemSet                             m_aItems;
        std::vector< OUString >             m_aUnqPath;
        const OUString                 m_aBaseDirectory;

        osl::Directory                        m_aFolder;
        cpo::uno::Sequence< css::beans::Property >      m_sProperty;
        cpo::uno::Sequence< css::ucb::NumberedSortingInfo >  m_sSortingInfo;

        std::mutex                          m_aMutex;
        comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aDisposeEventListeners;
        comphelper::OInterfaceContainerHelper4<css::beans::XPropertyChangeListener> m_aRowCountListeners;
        comphelper::OInterfaceContainerHelper4<css::beans::XPropertyChangeListener> m_aIsFinalListeners;

        cpo::uno::Reference< css::ucb::XDynamicResultSetListener >       m_xListener;

        TaskHandlerErr                                     m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Methods
        /// @throws css::sdbc::SQLException
        /// @throws cpo::uno::RuntimeException
        bool OneMore(std::unique_lock<std::mutex>&);

        void rowCountChanged(std::unique_lock<std::mutex>&);
        void isFinalChanged(std::unique_lock<std::mutex>&);
    };


} // end namespace fileaccess


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
