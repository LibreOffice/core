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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ATABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ATABLE_HXX

#include <connectivity/sdbcx/VTable.hxx>
#include "ado/Awrapadox.hxx"
#include "ado/ACatalog.hxx"

namespace connectivity
{
    namespace ado
    {
        typedef connectivity::sdbcx::OTable OTable_TYPEDEF;
        typedef connectivity::sdbcx::OTableDescriptor_BASE OTableDescriptor_BASE_TYPEDEF;

        class OAdoTable :   public OTable_TYPEDEF
        {
            WpADOTable      m_aTable;
            OCatalog*       m_pCatalog;

        protected:
            void fillPropertyValues();
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue)throw (css::uno::Exception);

        public:
            virtual void refreshColumns();
            virtual void refreshKeys();
            virtual void refreshIndexes();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing();

        public:
            OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog,_ADOTable* _pTable);
            OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog);


            virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException);
            OUString getSchema() const { return m_SchemaName; }
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const;
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException);
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            // XRename
            virtual void SAL_CALL rename( const OUString& newName ) throw(css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException);

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::container::NoSuchElementException, css::uno::RuntimeException);
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException);

            sal_Bool create() throw(css::sdbc::SQLException, css::uno::RuntimeException);

            WpADOTable getImpl() const { return m_aTable;}
            OCatalog* getCatalog() const { return m_pCatalog; }
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ATABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
