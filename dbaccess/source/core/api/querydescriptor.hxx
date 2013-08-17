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

#ifndef _DBA_COREAPI_QUERYDESCRIPTOR_HXX_
#define _DBA_COREAPI_QUERYDESCRIPTOR_HXX_

#include <cppuhelper/implbase3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include "apitools.hxx"
#include "column.hxx"
#include "datasettings.hxx"
#include "commandbase.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>

namespace dbaccess
{

// OQueryDescriptor_Base - a query descriptor (as the name suggests :)
typedef ::cppu::ImplHelper3<
        ::com::sun::star::sdbcx::XColumnsSupplier,
        ::com::sun::star::lang::XUnoTunnel,
        ::com::sun::star::lang::XServiceInfo >  OQueryDescriptor_BASE;

class OQueryDescriptor_Base
        :public OQueryDescriptor_BASE
        ,public OCommandBase
        ,public IColumnFactory
        ,public ::connectivity::sdbcx::IRefreshableColumns
{
private:
    sal_Bool        m_bColumnsOutOfDate : 1;    // the columns have to be rebuild on the next getColumns ?
    ::osl::Mutex&   m_rMutex;

protected:
    OColumns*       m_pColumns;                 // our column descriptions
    OUString m_sElementName;
    virtual ~OQueryDescriptor_Base();

    void        setColumnsOutOfDate( sal_Bool _bOutOfDate = sal_True );
    sal_Bool    isColumnsOutOfDate() const { return m_bColumnsOutOfDate; }

    sal_Int32   getColumnCount() const { return m_pColumns ? m_pColumns->getCount() : 0; }
    void        clearColumns( );

    void        implAppendColumn( const OUString& _rName, OColumn* _pColumn );

public:
    OQueryDescriptor_Base(::osl::Mutex& _rMutex,::cppu::OWeakObject& _rMySelf);
    /** constructs the object with a UNO QueryDescriptor. If you use this ctor, the resulting object
        won't have any column information (the column container will be empty)
    */
    OQueryDescriptor_Base(const OQueryDescriptor_Base& _rSource,::cppu::OWeakObject& _rMySelf);

// ::com::sun::star::sdbcx::XColumnsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    DECLARE_IMPLEMENTATION_ID( );

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

protected:

// IColumnFactory
    virtual OColumn*    createColumn(const OUString& _rName) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor();
    virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor );
    virtual void columnDropped(const OUString& _sName);

    /** rebuild our columns set

        clearColumns has already been called before, do <em>NOT</em> call it, again
    */
    virtual void rebuildColumns( );

    virtual void disposeColumns();

    // IRefreshableColumns overridables
    virtual void refreshColumns();
};

class OQueryDescriptor : public comphelper::OMutexAndBroadcastHelper
                        ,public ::cppu::OWeakObject
                        ,public OQueryDescriptor_Base
                        ,public ::comphelper::OPropertyArrayUsageHelper< OQueryDescriptor_Base >
                        ,public ODataSettings
{
    OQueryDescriptor(const OQueryDescriptor&);
    void operator =(const OQueryDescriptor&);
    // helper
    void registerProperties();
protected:
    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual ~OQueryDescriptor();
public:
    OQueryDescriptor();
    OQueryDescriptor(const OQueryDescriptor_Base& _rSource);

    // com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

};
}   // namespace dbaccess

#endif // _DBA_COREAPI_QUERYDESCRIPTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
