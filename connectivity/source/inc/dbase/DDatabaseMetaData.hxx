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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DDATABASEMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DDATABASEMETADATA_HXX

#include "file/FDatabaseMetaData.hxx"

namespace connectivity
{
    namespace dbase
    {

        //************ Class: java.sql.DatabaseMetaDataDate


        class ODbaseDatabaseMetaData :  public file::ODatabaseMetaData
        {
            virtual css::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getURL(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getIndexInfo( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            virtual sal_Int32 SAL_CALL getMaxBinaryLiteralLength(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxCharLiteralLength(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxColumnNameLength(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxColumnsInIndex(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxColumnsInTable(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            virtual bool        impl_storesMixedCaseQuotedIdentifiers_throw(  ) override;
            virtual bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  ) override;
        protected:
            virtual ~ODbaseDatabaseMetaData();
        public:
            ODbaseDatabaseMetaData(file::OConnection* _pCon);
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_DBASE_DDATABASEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
