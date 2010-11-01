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

#ifndef CONNECTIVITY_TABLEHELPER_HXX
#define CONNECTIVITY_TABLEHELPER_HXX

#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/sdbcx/VTable.hxx"
#include "connectivity/sdbcx/VKey.hxx"
#include "connectivity/StdTypeDefs.hxx"
#include <comphelper/stl_types.hxx>
#include <com/sun/star/sdb/tools/XTableRename.hpp>
#include <com/sun/star/sdb/tools/XTableAlteration.hpp>
#include <com/sun/star/sdb/tools/XKeyAlteration.hpp>
#include <com/sun/star/sdb/tools/XIndexAlteration.hpp>

namespace connectivity
{
    typedef sal_Int32   OrdinalPosition;
    struct ColumnDesc
    {
        ::rtl::OUString sName;
        ::rtl::OUString aField6;
        ::rtl::OUString sField12; // REMARKS
        ::rtl::OUString sField13;
        sal_Int32       nField5
                    ,   nField7
                    ,   nField9
                    ,   nField11;

        OrdinalPosition nOrdinalPosition;

        ColumnDesc() {}
        ColumnDesc( const ::rtl::OUString& _rName
            , sal_Int32     _nField5
            , const ::rtl::OUString& _aField6
            , sal_Int32     _nField7
            , sal_Int32     _nField9
            , sal_Int32     _nField11
            , const ::rtl::OUString& _sField12
            , const ::rtl::OUString& _sField13
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

    OOO_DLLPUBLIC_DBTOOLS ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

    DECLARE_STL_USTRINGACCESS_MAP( sdbcx::TKeyProperties , TKeyMap);

    struct OTableHelperImpl;

    class OOO_DLLPUBLIC_DBTOOLS OTableHelper : public OTable_TYPEDEF
    {
        ::std::auto_ptr<OTableHelperImpl> m_pImpl;

        void refreshPrimaryKeys(TStringVector& _rKeys);
        void refreshForgeinKeys(TStringVector& _rKeys);

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
        virtual void SAL_CALL disposing();

        /** The default returns "RENAME TABLE " or "RENAME VIEW " depending on the type.
        *
        * \return The start of the rename statement.
        */
        virtual ::rtl::OUString getRenameStart() const;

        virtual ~OTableHelper();

    public:
        virtual void refreshColumns();
        virtual void refreshKeys();
        virtual void refreshIndexes();

        const ColumnDesc* getColumnDescription(const ::rtl::OUString& _sName) const;

    public:
        OTableHelper(   sdbcx::OCollection* _pTables,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                        sal_Bool _bCase);
        OTableHelper(   sdbcx::OCollection* _pTables,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                        sal_Bool _bCase,
                        const ::rtl::OUString& _Name,
                        const ::rtl::OUString& _Type,
                        const ::rtl::OUString& _Description = ::rtl::OUString(),
                        const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                        const ::rtl::OUString& _CatalogName = ::rtl::OUString()
            );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() const;

        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XRename
        virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

        // XAlterTable
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        // XNamed
        virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

        // helper method to get key properties
        sdbcx::TKeyProperties getKeyProperties(const ::rtl::OUString& _sName) const;
        void addKey(const ::rtl::OUString& _sName,const sdbcx::TKeyProperties& _aKeyProperties);

        virtual ::rtl::OUString getTypeCreatePattern() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableRename>      getRenameService() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableAlteration>  getAlterService() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XKeyAlteration>    getKeyService() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XIndexAlteration>  getIndexService() const;
    };
}
#endif // CONNECTIVITY_TABLEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
