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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABRESULTSETMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABRESULTSETMETADATA_HXX

#include "MacabConnection.hxx"
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase.hxx>
#include <connectivity/dbexception.hxx>
#include <rtl/ref.hxx>

namespace connectivity
{
    namespace macab
    {
        /*
        **  MacabResultSetMetaData
        */
        typedef ::cppu::WeakImplHelper< ::com::sun::star::sdbc::XResultSetMetaData>   MacabResultSetMetaData_BASE;

        class MacabResultSetMetaData : public MacabResultSetMetaData_BASE
        {
            MacabConnection*                m_pConnection;
            OUString                 m_sTableName;
            ::std::vector<sal_Int32>    m_aMacabFields; // for each selected column, contains the number
                                                        //  of the corresponding AddressBook field

        protected:
            virtual ~MacabResultSetMetaData();

        public:
            MacabResultSetMetaData(MacabConnection* _pConnection, OUString const & _sTableName);

            // avoid ambigous cast error from the compiler
            inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
                { return this; }

            void setMacabFields(
                const ::rtl::Reference<connectivity::OSQLColumns> &xColumns) throw(::com::sun::star::sdbc::SQLException);
            inline sal_uInt32 fieldAtColumn(sal_Int32 columnIndex) const
                { return m_aMacabFields[columnIndex - 1]; }

            virtual sal_Int32 SAL_CALL getColumnCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getColumnName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getSchemaName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getTableName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getCatalogName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
            virtual OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABRESULTSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
