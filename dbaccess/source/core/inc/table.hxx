/*************************************************************************
 *
 *  $RCSfile: table.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-14 13:30:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include <connectivity/sdbcx/VTable.hxx>
#endif

namespace dbaccess
{

    typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
    //==========================================================================
    //= OTables
    //==========================================================================
    class ODBTable;
    typedef ::comphelper::OIdPropertyArrayUsageHelper< ODBTable >   ODBTable_PROP;
    typedef connectivity::sdbcx::OTable                             OTable_Base;

    class ODBTable  :public ODataSettings_Base
                    ,public ODBTable_PROP
                    ,public OTable_Base
    {
    protected:
        //  OWeakConnection                                                                 m_aConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >   m_xTable;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;
    // <properties>
        sal_Int32                                                                       m_nPrivileges;
    // </properties>

        void refreshPrimaryKeys(std::vector< ::rtl::OUString>& _rKeys);
        void refreshForgeinKeys(std::vector< ::rtl::OUString>& _rKeys);


        DECLARE_CTY_PROPERTY(ODBTable_PROP,OTable_Base)
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
        ODBTable(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConn,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >& _rxTable,
                const ::rtl::OUString& _rCatalog, const ::rtl::OUString& _rSchema, const ::rtl::OUString& _rName,
                const ::rtl::OUString& _rType, const ::rtl::OUString& _rDesc)
            throw(::com::sun::star::sdbc::SQLException);
        virtual ~ODBTable();

        // ODescriptor
        virtual void construct();

        //XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    // com::sun::star::lang::XTypeProvider
        //  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        //  virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

    // com::sun::star::beans::XPropertySet
        //  virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;

    // ::com::sun::star::sdbcx::XRename,
        virtual void SAL_CALL rename( const ::rtl::OUString& _rNewName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XAlterTable,
        virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDescriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 _nIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDescriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XNamed
        virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const { return m_xMetaData; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() const { return m_xMetaData->getConnection(); }

        virtual void refreshColumns();
        virtual void refreshKeys();
        virtual void refreshIndexes();
    };
}
#endif // _DBA_CORE_TABLE_HXX_


