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
#ifndef CONNECTIVITY_MYSQL_TABLES_HXX
#define CONNECTIVITY_MYSQL_TABLES_HXX

#include "connectivity/sdbcx/VCollection.hxx"
#include "connectivity/SQLStatementHelper.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
namespace connectivity
{
    namespace mysql
    {
        class OTables : public sdbcx::OCollection,
            public ::dbtools::ISQLStatementHelper
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;

        protected:
            virtual sdbcx::ObjectType createObject(const OUString& _rName) SAL_OVERRIDE;
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor() SAL_OVERRIDE;
            virtual sdbcx::ObjectType appendObject( const OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) SAL_OVERRIDE;
            virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) SAL_OVERRIDE;

            void createTable( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual OUString getNameForObject(const sdbcx::ObjectType& _xObject) SAL_OVERRIDE;
        public:
            OTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rMetaData,::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
                const TStringVector &_rVector) : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
                ,m_xMetaData(_rMetaData)
            {}

            // only the name is identical to ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;

            // XDrop
            void appendNew(const OUString& _rsNewTable);
            // some helper functions
            /**
                returns a sql string which contains the column definition part for create or alter statements
            */
            static OUString getColumnSqlType(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColProp);
            /**
                returns the "not null" part or the default part of the table statement
            */
            static OUString getColumnSqlNotNullDefault(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColProp);
            /**
                returns the corresponding typename
                can contain () which have to filled with values
            */
            static OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColProp);

            /** convert the sql statement to fit MySQL notation
                @param  _sSql in/out
            */
            static OUString adjustSQL(const OUString& _sSql);

            // ISQLStatementHelper
            virtual void addComment(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,OUStringBuffer& _rOut) SAL_OVERRIDE;
        };
    }
}
#endif // CONNECTIVITY_MYSQL_TABLES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
