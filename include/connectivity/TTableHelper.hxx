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

#ifndef INCLUDED_CONNECTIVITY_TTABLEHELPER_HXX
#define INCLUDED_CONNECTIVITY_TTABLEHELPER_HXX

#include <sal/config.h>

#include <map>

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sdbcx/VTable.hxx>
#include <connectivity/sdbcx/VKey.hxx>
#include <connectivity/StdTypeDefs.hxx>
#include <com/sun/star/sdb/tools/XTableRename.hpp>
#include <com/sun/star/sdb/tools/XTableAlteration.hpp>
#include <com/sun/star/sdb/tools/XKeyAlteration.hpp>
#include <com/sun/star/sdb/tools/XIndexAlteration.hpp>

namespace connectivity
{
    typedef sal_Int32   OrdinalPosition;
    struct ColumnDesc
    {
        OUString sName;
        OUString aField6;
        OUString sField12; // REMARKS
        OUString sField13;
        sal_Int32       nField5
                    ,   nField7
                    ,   nField9
                    ,   nField11;

        OrdinalPosition nOrdinalPosition;

        ColumnDesc() {}
        ColumnDesc( const OUString& _rName
            , sal_Int32     _nField5
            , const OUString& _aField6
            , sal_Int32     _nField7
            , sal_Int32     _nField9
            , sal_Int32     _nField11
            , const OUString& _sField12
            , const OUString& _sField13
            ,OrdinalPosition _nPosition )
            :sName( _rName )
            ,aField6(_aField6)
            ,sField12(_sField12)
            ,sField13(_sField13)
            ,nField5(_nField5)
            ,nField7(_nField7)
            ,nField9(_nField9)
            ,nField11(_nField11)
            ,nOrdinalPosition( _nPosition )
        {
        }
    };
    typedef connectivity::sdbcx::OTable OTable_TYPEDEF;

    typedef std::map<OUString, sdbcx::TKeyProperties> TKeyMap;

    struct OTableHelperImpl;

    class OOO_DLLPUBLIC_DBTOOLS OTableHelper : public OTable_TYPEDEF
    {
        ::std::unique_ptr<OTableHelperImpl> m_pImpl;

        void refreshPrimaryKeys(TStringVector& _rKeys);
        void refreshForeignKeys(TStringVector& _rKeys);

    protected:
        /** creates the column collection for the table
            @param  _rNames
                The column names.
        */
        virtual sdbcx::OCollection* createColumns(const TStringVector& _rNames) = 0;

        /** creates the key collection for the table
            @param  _rNames
                The key names.
        */
        virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames) = 0;

        /** creates the index collection for the table
            @param  _rNames
                The index names.
        */
        virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames) = 0;

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;

        /** The default returns "RENAME TABLE " or "RENAME VIEW " depending on the type.
        *
        * \return The start of the rename statement.
        */
        virtual OUString getRenameStart() const;

        virtual ~OTableHelper();

    public:
        virtual void refreshColumns() override;
        virtual void refreshKeys() override;
        virtual void refreshIndexes() override;

        const ColumnDesc* getColumnDescription(const OUString& _sName) const;

    public:
        OTableHelper(   sdbcx::OCollection* _pTables,
                        const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                        bool _bCase);
        OTableHelper(   sdbcx::OCollection* _pTables,
                        const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                        bool _bCase,
                        const OUString& _Name,
                        const OUString& _Type,
                        const OUString& _Description = OUString(),
                        const OUString& _SchemaName = OUString(),
                        const OUString& _CatalogName = OUString()
            );

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const override;
        css::uno::Reference< css::sdbc::XConnection> getConnection() const;

        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

        // XRename
        virtual void SAL_CALL rename( const OUString& newName ) throw(css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

        // XAlterTable
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        // XNamed
        virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;

        // helper method to get key properties
        sdbcx::TKeyProperties getKeyProperties(const OUString& _sName) const;
        void addKey(const OUString& _sName,const sdbcx::TKeyProperties& _aKeyProperties);

        virtual OUString getTypeCreatePattern() const;

        css::uno::Reference< css::sdb::tools::XTableRename>      getRenameService() const;
        css::uno::Reference< css::sdb::tools::XTableAlteration>  getAlterService() const;
        css::uno::Reference< css::sdb::tools::XKeyAlteration>    getKeyService() const;
        css::uno::Reference< css::sdb::tools::XIndexAlteration>  getIndexService() const;
    };
}
#endif // INCLUDED_CONNECTIVITY_TTABLEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
