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

#pragma once

#include "contentresultsetwrapper.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ucb/XFetchProvider.hpp>
#include <com/sun/star/ucb/XFetchProviderForContentAccess.hpp>
#include <com/sun/star/ucb/FetchResult.hpp>
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#include <com/sun/star/ucb/XCachedContentResultSetFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <memory>

namespace com::sun::star::script {
    class XTypeConverter;
}

class CCRS_PropertySetInfo;
class CachedContentResultSet
                : public ContentResultSetWrapper
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
{


    class CCRS_Cache
    {
    private:
        std::unique_ptr<css::ucb::FetchResult>
                                         m_pResult;
        css::uno::Reference< css::ucb::XContentIdentifierMapping >
                                         m_xContentIdentifierMapping;
        std::unique_ptr<css::uno::Sequence< sal_Bool >>  m_pMappedReminder;

    private:
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        css::uno::Any&
        getRowAny( sal_Int32 nRow );

        void clear();


        void remindMapped( sal_Int32 nRow );
        bool isRowMapped( sal_Int32 nRow );
        css::uno::Sequence< sal_Bool >* getMappedReminder();

    public:
        CCRS_Cache( const css::uno::Reference<
                css::ucb::XContentIdentifierMapping > & xMapping );
        ~CCRS_Cache();

        void loadData(
            const css::ucb::FetchResult& rResult );

        bool
        hasRow( sal_Int32 nRow );

        bool
        hasCausedException( sal_Int32 nRow );

        sal_Int32
        getMaxRow() const;

        bool
        hasKnownLast() const;

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        const css::uno::Any&
        getAny( sal_Int32 nRow, sal_Int32 nColumnIndex );

        /// @throws css::uno::RuntimeException
        OUString const &
        getContentIdentifierString( sal_Int32 nRow );

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::ucb::XContentIdentifier >
        getContentIdentifier( sal_Int32 nRow );

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::ucb::XContent >
        getContent( sal_Int32 nRow );
    };

    //members

    css::uno::Reference< css::uno::XComponentContext >
                            m_xContext;

    //different Interfaces from Origin:
    css::uno::Reference< css::ucb::XFetchProvider >
                            m_xFetchProvider; //XFetchProvider-interface from m_xOrigin

    css::uno::Reference< css::ucb::XFetchProviderForContentAccess >
                            m_xFetchProviderForContentAccess; //XFetchProviderForContentAccess-interface from m_xOrigin

    rtl::Reference< CCRS_PropertySetInfo >
                            m_xMyPropertySetInfo;


    css::uno::Reference< css::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;// can be used for remote optimized ContentAccess

    //some Properties and helping variables
    sal_Int32               m_nRow;
    bool                    m_bAfterLast; // TRUE, if m_nRow is after final count; can be TRUE without knowing the exact final count

    sal_Int32               m_nLastAppliedPos;
    bool                    m_bAfterLastApplied;

    sal_Int32               m_nKnownCount; // count we know from the Origin
    bool                    m_bFinalCount; // TRUE if the Origin has reached final count and we got that count in m_nKnownCount

    sal_Int32               m_nFetchSize;
    sal_Int32               m_nFetchDirection;

    bool                    m_bLastReadWasFromCache;
    bool                    m_bLastCachedReadWasNull;

    //cache:
    CCRS_Cache              m_aCache;
    CCRS_Cache              m_aCacheContentIdentifierString;
    CCRS_Cache              m_aCacheContentIdentifier;
    CCRS_Cache              m_aCacheContent;


private:

    //helping XPropertySet methods.
    virtual void impl_initPropertySetInfo() override;

    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    bool
    applyPositionToOrigin( sal_Int32 nRow );

    /// @throws css::uno::RuntimeException
    void
    impl_fetchData( sal_Int32 nRow, sal_Int32 nCount
                    , sal_Int32 nFetchDirection );

    bool
    impl_isKnownValidPosition( sal_Int32 nRow );

    bool
    impl_isKnownInvalidPosition( sal_Int32 nRow );

    void
    impl_changeRowCount( sal_Int32 nOld, sal_Int32 nNew );

    void
    impl_changeIsRowCountFinal( bool bOld, bool bNew );

public:
    CachedContentResultSet(
                        const css::uno::Reference< css::uno::XComponentContext > &         rxContext,
                        const css::uno::Reference< css::sdbc::XResultSet > &               xOrigin,
                        const css::uno::Reference< css::ucb::XContentIdentifierMapping > & xContentIdentifierMapping );

    virtual ~CachedContentResultSet() override;


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertySet inherited


    virtual void SAL_CALL
    setPropertyValue( const OUString& aPropertyName,
                      const css::uno::Any& aValue ) override;

    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName ) override;


    // own inherited

    virtual void
    impl_disposing( const css::lang::EventObject& Source ) override;

    virtual void
    impl_propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    virtual void
    impl_vetoableChange( const css::beans::PropertyChangeEvent& aEvent ) override;


    // XContentAccess inherited

    virtual OUString SAL_CALL
    queryContentIdentifierString() override;

    virtual css::uno::Reference<
                css::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier() override;

    virtual css::uno::Reference<
                css::ucb::XContent > SAL_CALL
    queryContent() override;


    // XResultSet inherited

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


    // XRow inherited

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

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex ) override;

    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

    virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex ) override;

    virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex ) override;


    // Type Converter support


private:
    bool m_bTriedToGetTypeConverter;
    css::uno::Reference< css::script::XTypeConverter > m_xTypeConverter;

    const css::uno::Reference<
        css::script::XTypeConverter >& getTypeConverter();

    template<typename T> T rowOriginGet(
        T (SAL_CALL css::sdbc::XRow::* f)(sal_Int32), sal_Int32 columnIndex);
};


class CachedContentResultSetFactory final :
                public cppu::WeakImplHelper<
                    css::lang::XServiceInfo,
                    css::ucb::XCachedContentResultSetFactory>
{
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;

public:

    CachedContentResultSetFactory( const css::uno::Reference< css::uno::XComponentContext > & rxContext);

    virtual ~CachedContentResultSetFactory() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XCachedContentResultSetFactory

    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSet(
            const css::uno::Reference< css::sdbc::XResultSet > & xSource,
            const css::uno::Reference< css::ucb::XContentIdentifierMapping > & xMapping ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
