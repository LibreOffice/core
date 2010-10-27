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
#ifndef _DBA_COREAPI_STATEMENT_HXX_
#define _DBA_COREAPI_STATEMENT_HXX_

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

#include <cppuhelper/implbase3.hxx>

//************************************************************
//  OStatementBase
//************************************************************
class OStatementBase :  public comphelper::OBaseMutex,
                        public OSubComponent,
                        public ::cppu::OPropertySetHelper,
                        public ::comphelper::OPropertyArrayUsageHelper < OStatementBase >,
                        public ::com::sun::star::util::XCancellable,
                        public ::com::sun::star::sdbc::XWarningsSupplier,
                        public ::com::sun::star::sdbc::XPreparedBatchExecution,
                        public ::com::sun::star::sdbc::XMultipleResults,
                        public ::com::sun::star::sdbc::XCloseable,
                        public ::com::sun::star::sdbc::XGeneratedResultSet
{
protected:
    ::osl::Mutex            m_aCancelMutex;

    ::com::sun::star::uno::WeakReferenceHelper  m_aResultSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xAggregateAsSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCancellable > m_xAggregateAsCancellable;
    sal_Bool                m_bUseBookmarks;
    sal_Bool                m_bEscapeProcessing;

    virtual ~OStatementBase();

public:
    OStatementBase(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & _xConn,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _xStatement);


// ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

// ::com::sun::star::sdbc::XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XCancellable
    virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XMultipleResults
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XPreparedBatchExecution
    virtual void SAL_CALL addBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
// ::com::sun::star::sdbc::XGeneratedResultSet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getGeneratedValues(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// Helper
    void disposeResultSet();

protected:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};

//************************************************************
//  OStatement
//************************************************************
typedef ::cppu::ImplHelper3 <   ::com::sun::star::sdbc::XStatement
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::sdbc::XBatchExecution
                            >   OStatement_IFACE;
class OStatement    :public OStatementBase
                    ,public OStatement_IFACE
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement >                  m_xAggregateStatement;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >   m_xComposer;
    bool                                                                                    m_bAttemptedComposerCreation;

public:
    OStatement(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & _xConn,
               const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _xStatement);

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XStatement
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL executeUpdate( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL execute( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XBatchExecution
    virtual void SAL_CALL addBatch( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    using OStatementBase::addBatch;

private:
    /** does escape processing for the given SQL command, if the our EscapeProcessing
        property allows so.
    */
    ::rtl::OUString impl_doEscapeProcessing_nothrow( const ::rtl::OUString& _rSQL ) const;
    bool            impl_ensureComposer_nothrow() const;
};

#endif // _DBA_COREAPI_STATEMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
