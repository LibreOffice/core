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

#include <memory>
#include <sal/config.h>

#include <map>

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sdbcx/VTable.hxx>

namespace com::sun::star::sdbc { class XConnection; }
namespace com::sun::star::sdb::tools { class XIndexAlteration; }
namespace com::sun::star::sdb::tools { class XKeyAlteration; }
namespace com::sun::star::sdb::tools { class XTableAlteration; }
namespace com::sun::star::sdb::tools { class XTableRename; }
namespace connectivity::sdbcx { struct KeyProperties; }

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

    typedef std::map<OUString, std::shared_ptr<sdbcx::KeyProperties>> TKeyMap;

    struct OTableHelperImpl;

    class OOO_DLLPUBLIC_DBTOOLS OTableHelper : public OTable_TYPEDEF
    {
        ::std::unique_ptr<OTableHelperImpl> m_pImpl;

        void refreshPrimaryKeys(::std::vector< OUString>& _rKeys);
        void refreshForeignKeys(::std::vector< OUString>& _rKeys);

    protected:
        /** creates the column collection for the table
            @param  _rNames
                The column names.
        */
        virtual sdbcx::OCollection* createColumns(const ::std::vector< OUString>& _rNames) = 0;

        /** creates the key collection for the table
            @param  _rNames
                The key names.
        */
        virtual sdbcx::OCollection* createKeys(const ::std::vector< OUString>& _rNames) = 0;

        /** creates the index collection for the table
            @param  _rNames
                The index names.
        */
        virtual sdbcx::OCollection* createIndexes(const ::std::vector< OUString>& _rNames) = 0;

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;

        /** The default returns "RENAME TABLE " or "RENAME VIEW " depending on the type.
        *
        * \return The start of the rename statement.
        */
        virtual OUString getRenameStart() const;

        virtual ~OTableHelper() override;

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
                        const OUString& Name,
                        const OUString& Type,
                        const OUString& Description = OUString(),
                        const OUString& SchemaName = OUString(),
                        const OUString& CatalogName = OUString()
            );

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const override;
        css::uno::Reference< css::sdbc::XConnection> const & getConnection() const;

        // XRename
        virtual void SAL_CALL rename( const OUString& newName ) override;

        // XAlterTable
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
        // XNamed
        virtual OUString SAL_CALL getName() override;

        // helper method to get key properties
        std::shared_ptr<sdbcx::KeyProperties> getKeyProperties(const OUString& _sName) const;
        void addKey(const OUString& _sName,const std::shared_ptr<sdbcx::KeyProperties>& _aKeyProperties);

        virtual OUString getTypeCreatePattern() const;

        css::uno::Reference< css::sdb::tools::XTableRename> const &      getRenameService() const;
        css::uno::Reference< css::sdb::tools::XTableAlteration> const &  getAlterService() const;
        css::uno::Reference< css::sdb::tools::XKeyAlteration> const &    getKeyService() const;
        css::uno::Reference< css::sdb::tools::XIndexAlteration> const &  getIndexService() const;
    };
}
#endif // INCLUDED_CONNECTIVITY_TTABLEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
