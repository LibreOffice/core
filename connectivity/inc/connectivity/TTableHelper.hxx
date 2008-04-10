/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TTableHelper.hxx,v $
 * $Revision: 1.6 $
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

#include "connectivity/sdbcx/VTable.hxx"
#include "connectivity/StdTypeDefs.hxx"

namespace connectivity
{
    typedef connectivity::sdbcx::OTable OTable_TYPEDEF;

    ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

    class OTableHelper : public OTable_TYPEDEF
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;

        void refreshPrimaryKeys(std::vector< ::rtl::OUString>& _rKeys);
        void refreshForgeinKeys(std::vector< ::rtl::OUString>& _rKeys);

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

    public:
        virtual void refreshColumns();
        virtual void refreshKeys();
        virtual void refreshIndexes();

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
        inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() const { return m_xConnection; }

        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XRename
        virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

        // XAlterTable
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        // XNamed
        virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif // CONNECTIVITY_TABLEHELPER_HXX

