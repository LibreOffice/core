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

#ifndef _DBA_CORE_TABLEDECORATOR_HXX_
#define _DBA_CORE_TABLEDECORATOR_HXX_

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/compbase9.hxx>
#include <cppuhelper/implbase5.hxx>
#include "apitools.hxx"
#include "datasettings.hxx"
#include "column.hxx"

#include <connectivity/CommonTools.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <comphelper/IdPropArrayHelper.hxx>

namespace dbaccess
{
    typedef ::cppu::WeakComponentImplHelper9<   ::com::sun::star::sdbcx::XColumnsSupplier,
                                                ::com::sun::star::sdbcx::XKeysSupplier,
                                                ::com::sun::star::container::XNamed,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                                ::com::sun::star::sdbcx::XIndexesSupplier,
                                                ::com::sun::star::sdbcx::XRename,
                                                ::com::sun::star::lang::XUnoTunnel,
                                                ::com::sun::star::sdbcx::XAlterTable> OTableDescriptor_BASE;
    //==========================================================================
    //= OTables
    //==========================================================================
    class ODBTableDecorator;
    typedef ::comphelper::OIdPropertyArrayUsageHelper< ODBTableDecorator >  ODBTableDecorator_PROP;

    class ODBTableDecorator :public comphelper::OBaseMutex
                            ,public OTableDescriptor_BASE
                            ,public ODataSettings //ODataSettings_Base
                            ,public IColumnFactory
                            ,public ::connectivity::sdbcx::IRefreshableColumns
                            ,public ODBTableDecorator_PROP
    {
        void fillPrivileges() const;
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xColumnMediator;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >       m_xTable;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xColumnDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xNumberFormats;

    // <properties>
        mutable sal_Int32                                                                   m_nPrivileges;
    // </properties>
        ::connectivity::sdbcx::OCollection*                                                 m_pColumns;
        ::connectivity::sdbcx::OCollection*                                                 m_pTables;

        // IColumnFactory
        virtual OColumn*    createColumn(const ::rtl::OUString& _rName) const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor();
        virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor );
        virtual void columnDropped(const ::rtl::OUString& _sName);

        virtual void refreshColumns();

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 _nId) const;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )

                                         throw (::com::sun::star::uno::Exception);

        virtual ~ODBTableDecorator();
    public:
        /** constructs a wrapper supporting the com.sun.star.sdb.Table service.

            @param _rxConn
                the connection the table belongs to. Must not be <NULL/>
            @param _rxTable
                the table from the driver can be <NULL/>
        */
        ODBTableDecorator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >& _rxTable,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _rxNumberFormats,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumnDefinitions
        )   throw(::com::sun::star::sdbc::SQLException);



        // ODescriptor
        virtual void construct();

        //XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();
        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::sdbcx::XRename,
        virtual void SAL_CALL rename( const ::rtl::OUString& _rNewName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAlterTable,
        virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDescriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDescriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XNamed
        virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const { return m_xMetaData; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() const { return m_xMetaData.is() ? m_xMetaData->getConnection() : ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>(); }

        // XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw (::com::sun::star::uno::RuntimeException);
        // XKeysSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getKeys(  ) throw (::com::sun::star::uno::RuntimeException);
        // XIndexesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getIndexes(  ) throw (::com::sun::star::uno::RuntimeException);
        // XDataDescriptorFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        using ODataSettings::getFastPropertyValue;
    };
}
#endif // _DBA_CORE_TABLEDECORATOR_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
