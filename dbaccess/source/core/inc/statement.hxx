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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_STATEMENT_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_STATEMENT_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include "apitools.hxx"
#include <comphelper/broadcasthelper.hxx>

#include <cppuhelper/implbase.hxx>


//  OStatementBase

class OStatementBase :  public comphelper::OBaseMutex,
                        public OSubComponent,
                        public ::cppu::OPropertySetHelper,
                        public ::comphelper::OPropertyArrayUsageHelper < OStatementBase >,
                        public css::util::XCancellable,
                        public css::sdbc::XWarningsSupplier,
                        public css::sdbc::XPreparedBatchExecution,
                        public css::sdbc::XMultipleResults,
                        public css::sdbc::XCloseable,
                        public css::sdbc::XGeneratedResultSet
{
protected:
    ::osl::Mutex            m_aCancelMutex;

    css::uno::WeakReferenceHelper                   m_aResultSet;
    css::uno::Reference< css::beans::XPropertySet > m_xAggregateAsSet;
    css::uno::Reference< css::util::XCancellable >  m_xAggregateAsCancellable;
    bool                m_bUseBookmarks;
    bool                m_bEscapeProcessing;

    virtual ~OStatementBase();

public:
    OStatementBase(const css::uno::Reference< css::sdbc::XConnection > & _xConn,
                   const css::uno::Reference< css::uno::XInterface > & _xStatement);


// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

// comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            css::uno::Any & rConvertedValue,
                            css::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const css::uno::Any& rValue )
                                throw (css::lang::IllegalArgumentException) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue
                                                 )
                                                 throw (css::uno::Exception, std::exception) override;
    virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

// css::sdbc::XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::util::XCancellable
    virtual void SAL_CALL cancel(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdbc::XCloseable
    virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::sdbc::XMultipleResults
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getUpdateCount(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMoreResults(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::sdbc::XPreparedBatchExecution
    virtual void SAL_CALL addBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
// css::sdbc::XGeneratedResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getGeneratedValues(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// Helper
    void disposeResultSet();

protected:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};


//  OStatement

typedef ::cppu::ImplHelper <   css::sdbc::XStatement
                           ,   css::lang::XServiceInfo
                           ,   css::sdbc::XBatchExecution
                           >   OStatement_IFACE;
class OStatement    :public OStatementBase
                    ,public OStatement_IFACE
{
private:
    css::uno::Reference< css::sdbc::XStatement >                  m_xAggregateStatement;
    css::uno::Reference< css::sdb::XSingleSelectQueryComposer >   m_xComposer;
    bool                                                          m_bAttemptedComposerCreation;

public:
    OStatement(const css::uno::Reference< css::sdbc::XConnection > & _xConn,
               const css::uno::Reference< css::uno::XInterface > & _xStatement);

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdbc::XStatement
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XBatchExecution
    virtual void SAL_CALL addBatch( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    using OStatementBase::addBatch;

private:
    /** does escape processing for the given SQL command, if the our EscapeProcessing
        property allows so.
    */
    OUString impl_doEscapeProcessing_nothrow( const OUString& _rSQL ) const;
    bool            impl_ensureComposer_nothrow() const;
};

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_STATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
