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

#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#define _CONNECTIVITY_SDBCX_TABLE_HXX_

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/implbase4.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/CommonTools.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

namespace connectivity
{
    namespace sdbcx
    {

        class OTable;
        class OCollection;

        typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::sdbcx::XColumnsSupplier,
                                                    ::com::sun::star::sdbcx::XKeysSupplier,
                                                    ::com::sun::star::container::XNamed,
                                                    ::com::sun::star::lang::XServiceInfo> OTableDescriptor_BASE;

        typedef ::cppu::ImplHelper4<                ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                                    ::com::sun::star::sdbcx::XIndexesSupplier,
                                                    ::com::sun::star::sdbcx::XRename,
                                                    ::com::sun::star::sdbcx::XAlterTable > OTable_BASE;

        typedef ::comphelper::OIdPropertyArrayUsageHelper<OTable> OTable_PROP;


        class OOO_DLLPUBLIC_DBTOOLS OTable :
                                 public comphelper::OBaseMutex,
                                 public OTable_BASE,
                                 public OTableDescriptor_BASE,
                                 public IRefreshableColumns,
                                 public OTable_PROP,
                                 public ODescriptor
        {
        protected:
            ::rtl::OUString m_CatalogName;
            ::rtl::OUString m_SchemaName;
            ::rtl::OUString m_Description;
            ::rtl::OUString m_Type;

            OCollection*    m_pKeys;
            OCollection*    m_pColumns;
            OCollection*    m_pIndexes;
            OCollection*    m_pTables;  // must hold his own container to notify him when renaming

            using OTableDescriptor_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 _nId ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OTable( OCollection*    _pTables,
                    sal_Bool _bCase);
            OTable( OCollection*    _pTables,
                    sal_Bool _bCase,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName  = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString());

            virtual ~OTable();

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual void    SAL_CALL acquire() throw();
            virtual void    SAL_CALL release() throw();
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // ODescriptor
            virtual void construct();
            virtual void refreshColumns();
            virtual void refreshKeys();
            virtual void refreshIndexes();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XColumnsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
            // XKeysSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getKeys(  ) throw(::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor( void ) throw(::com::sun::star::uno::RuntimeException);
            // XIndexesSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getIndexes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XRename
            virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

            // helper method
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const;
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_TABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
