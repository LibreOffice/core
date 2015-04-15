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
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue)throw (::com::sun::star::uno::Exception);

        public:
            virtual void refreshColumns();
            virtual void refreshKeys();
            virtual void refreshIndexes();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing();

        public:
            OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog,_ADOTable* _pTable);
            OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog);


            virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
            OUString getSchema() const { return m_SchemaName; }
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const;
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            // XRename
            virtual void SAL_CALL rename( const OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

            sal_Bool create() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            WpADOTable getImpl() const { return m_aTable;}
            OCatalog* getCatalog() const { return m_pCatalog; }
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ATABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
