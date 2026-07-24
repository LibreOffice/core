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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
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
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/compbase.hxx>
#include <unotools/weakref.hxx>

namespace dbaccess { class OConnection; class OResultSet; }

//  OStatementBase

class OStatementBase :  public cppu::BaseMutex,
                        public ::cppu::WeakComponentImplHelper<>,
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
    unotools::WeakReference<::dbaccess::OConnection> m_xParent;
    ::osl::Mutex            m_aCancelMutex;

    unotools::WeakReference<::dbaccess::OResultSet> m_xWeakResultSet;
    css::uno::Reference< css::beans::XPropertySet > m_xAggregateAsSet;
    css::uno::Reference< css::util::XCancellable >  m_xAggregateAsCancellable;
    bool                m_bUseBookmarks;
    bool                m_bEscapeProcessing;

    virtual ~OStatementBase() override;

public:
    OStatementBase(const rtl::Reference< ::dbaccess::OConnection > & _xConn,
                   const css::uno::Reference< css::uno::XInterface > & _xStatement);


// css::lang::XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;

// css::uno::XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

// OComponentHelper
    virtual void disposing() override;

// css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

// comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    virtual bool convertFastPropertyValue(
                            cpo::uno::Any & rConvertedValue,
                            cpo::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const cpo::uno::Any& rValue ) override;
    virtual void setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const cpo::uno::Any& rValue
                                                 ) override;
    virtual void getFastPropertyValue( cpo::uno::Any& rValue, sal_Int32 nHandle ) const override;

// css::sdbc::XWarningsSupplier
    virtual cpo::uno::Any getWarnings(  ) override;
    virtual void clearWarnings(  ) override;

// css::util::XCancellable
    virtual void cancel(  ) override;

// css::sdbc::XCloseable
    virtual void close(  ) override;

// css::sdbc::XMultipleResults
    virtual css::uno::Reference< css::sdbc::XResultSet > getResultSet(  ) override;
    virtual sal_Int32 getUpdateCount(  ) override;
    virtual bool getMoreResults(  ) override;

// css::sdbc::XPreparedBatchExecution
    virtual void addBatch(  ) override;
    virtual void clearBatch(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > executeBatch(  ) override;
// css::sdbc::XGeneratedResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > getGeneratedValues(  ) override;

// Helper
    void disposeResultSet();

protected:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};


//  OStatement

typedef ::cppu::ImplHelper3 <   css::sdbc::XStatement
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
    OStatement(const rtl::Reference< ::dbaccess::OConnection > & _xConn,
               const css::uno::Reference< css::uno::XInterface > & _xStatement);

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

// css::lang::XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

// css::sdbc::XStatement
    virtual css::uno::Reference< css::sdbc::XResultSet > executeQuery( const OUString& sql ) override;
    virtual sal_Int32 executeUpdate( const OUString& sql ) override;
    virtual bool execute( const OUString& sql ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;

    // OComponentHelper
    virtual void disposing() override;

    // XBatchExecution
    virtual void addBatch( const OUString& sql ) override;
    virtual void clearBatch(  ) override;
    virtual cpo::uno::Sequence< sal_Int32 > executeBatch(  ) override;

    using OStatementBase::addBatch;

private:
    /** does escape processing for the given SQL command, if the our EscapeProcessing
        property allows so.
    */
    OUString impl_doEscapeProcessing_nothrow( const OUString& _rSQL ) const;
    bool            impl_ensureComposer_nothrow() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
