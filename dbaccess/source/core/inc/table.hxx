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

#ifndef _DBA_CORE_TABLE_HXX_
#define _DBA_CORE_TABLE_HXX_

#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XALTERTABLE_HPP_
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase7.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_DATASETTINGS_HXX_
#include "datasettings.hxx"
#endif
#ifndef _DBA_COREAPI_COLUMN_HXX_
#include <column.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include <connectivity/TTableHelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif

namespace dbaccess
{

    //==========================================================================
    //= OTables
    //==========================================================================
    class ODBTable;
    class OContainerMediator;
    typedef ::comphelper::OIdPropertyArrayUsageHelper< ODBTable >   ODBTable_PROP;
    typedef ::connectivity::OTableHelper                            OTable_Base;

    class ODBTable  :public ODataSettings_Base
                    ,public ODBTable_PROP
                    ,public OTable_Base
                    ,public IColumnFactory
    {
    private:
        ::rtl::Reference< OContainerMediator >                                          m_pColumnMediator;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumnDefinitions;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xDriverColumns;

    // <properties>
        sal_Int32                                                                       m_nPrivileges;
    // </properties>

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        // IColumnFactory
        virtual OColumn*    createColumn(const ::rtl::OUString& _rName) const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createColumnDescriptor();
        virtual void columnAppended( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSourceDescriptor );
        virtual void columnDropped(const ::rtl::OUString& _sName);

        /** creates the column collection for the table
            @param  _rNames
                The column names.
        */
        virtual ::connectivity::sdbcx::OCollection* createColumns(const ::connectivity::TStringVector& _rNames);

        /** creates the key collection for the table
            @param  _rNames
                The key names.
        */
        virtual ::connectivity::sdbcx::OCollection* createKeys(const ::connectivity::TStringVector& _rNames);

        /** creates the index collection for the table
            @param  _rNames
                The index names.
        */
        virtual ::connectivity::sdbcx::OCollection* createIndexes(const ::connectivity::TStringVector& _rNames);

        // OComponentHelper
        virtual void SAL_CALL disposing(void);
    public:
        /** constructs a wrapper supporting the com.sun.star.sdb.Table service.<BR>
            @param          _rxConn         the connection the table belongs to
            @param          _rxTable        the table from the driver can be null
            @param          _rCatalog       the name of the catalog the table belongs to. May be empty.
            @param          _rSchema        the name of the schema the table belongs to. May be empty.
            @param          _rName          the name of the table
            @param          _rType          the type of the table, as supplied by the driver
            @param          _rDesc          the description of the table, as supplied by the driver
        */
        ODBTable(connectivity::sdbcx::OCollection* _pTables
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn
                ,const ::rtl::OUString& _rCatalog
                , const ::rtl::OUString& _rSchema
                , const ::rtl::OUString& _rName
                ,const ::rtl::OUString& _rType
                , const ::rtl::OUString& _rDesc
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumnDefinitions)
            throw(::com::sun::star::sdbc::SQLException);

        ODBTable(connectivity::sdbcx::OCollection* _pTables
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn)
                throw(::com::sun::star::sdbc::SQLException);
        virtual ~ODBTable();

        // ODescriptor
        virtual void construct();

        //XInterface
        DECLARE_XINTERFACE()
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

    // com::sun::star::beans::XPropertySet
        //  virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;

    // ::com::sun::star::sdbcx::XRename,
        virtual void SAL_CALL rename( const ::rtl::OUString& _rNewName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAlterTable,
        virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDescriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    private:
        using OTable_Base::createArrayHelper;
        using OTable_Base::getFastPropertyValue;
    };
}
#endif // _DBA_CORE_TABLE_HXX_


