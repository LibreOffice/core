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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_KEYSET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_KEYSET_HXX

#include "CacheSet.hxx"

#include <memory>
#include <map>
#include <vector>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <comphelper/stl_types.hxx>

namespace dbaccess
{
    struct SelectColumnDescription
    {
        OUString sRealName;      // may be empty
        OUString sTableName;      // may be empty
        OUString sDefaultValue;
        sal_Int32       nPosition;
        sal_Int32       nType;
        sal_Int32       nScale;
        bool        bNullable;

        SelectColumnDescription()
            :nPosition( 0 )
            ,nType( 0 )
            ,nScale( 0 )
            ,bNullable(false)
        {
        }

        SelectColumnDescription( sal_Int32 _nPosition, sal_Int32 _nType, sal_Int32 _nScale,bool _bNullable, const OUString& _rDefaultValue )
            :sDefaultValue( _rDefaultValue )
            ,nPosition( _nPosition )
            ,nType( _nType )
            ,nScale( _nScale )
            ,bNullable(_bNullable)
        {
        }
    };
    typedef ::std::map< OUString, SelectColumnDescription, ::comphelper::UStringMixLess >    SelectColumnsMetaData;

    // the elements of _rxQueryColumns must have the properties PROPERTY_REALNAME and PROPERTY_TABLENAME
    void getColumnPositions(const css::uno::Reference< css::container::XNameAccess >& _rxQueryColumns,
                            const css::uno::Sequence< OUString >& _rColumnNames,
                            const OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& o_rColumnNames /* out */,
                            bool i_bAppendTableName = false);

    typedef ::std::pair<ORowSetRow,::std::pair<sal_Int32,css::uno::Reference< css::sdbc::XRow> > > OKeySetValue;
    typedef ::std::map<sal_Int32,OKeySetValue > OKeySetMatrix;
    typedef ::std::map<sal_Int32, rtl::Reference<ORowSetValueVector> > OUpdatedParameter;
    // is used when the source supports keys
    class OKeySet : public OCacheSet
    {
    protected:
        OKeySetMatrix                                           m_aKeyMap;
        OKeySetMatrix::iterator                                 m_aKeyIter;

        ::std::vector< OUString >                               m_aAutoColumns;  // contains all columns which are autoincrement ones

        OUpdatedParameter                                       m_aUpdatedParameter;    // contains all parameter which have been updated and are needed for refetching
        rtl::Reference<ORowSetValueVector>                      m_aParameterValueForCache;
        ::std::unique_ptr<SelectColumnsMetaData>                m_pKeyColumnNames;      // contains all key column names
        ::std::unique_ptr<SelectColumnsMetaData>                m_pColumnNames;         // contains all column names
        ::std::unique_ptr<SelectColumnsMetaData>                m_pParameterNames;      // contains all parameter names
        ::std::unique_ptr<SelectColumnsMetaData>                m_pForeignColumnNames;  // contains all column names of the rest
        connectivity::OSQLTable                                 m_xTable; // reference to our table
        css::uno::Reference< css::container::XIndexAccess>      m_xTableKeys;
        // we need a different SQL (statement) for each different combination
        // of NULLness of key & foreign columns;
        // each subclause is either "colName = ?" or "colName IS NULL"
        // (we avoid the standard "colName IS NOT DISTINCT FROM ?" because it is not widely supported)
        typedef ::std::vector< bool > FilterColumnsNULL_t;
        typedef ::std::map< FilterColumnsNULL_t,
                            css::uno::Reference< css::sdbc::XPreparedStatement > >
                vStatements_t;
        vStatements_t                                           m_vStatements;
        css::uno::Reference< css::sdbc::XPreparedStatement>     m_xStatement;
        css::uno::Reference< css::sdbc::XResultSet>             m_xSet;
        css::uno::Reference< css::sdbc::XRow>                   m_xRow;
        css::uno::Reference< css::sdb::XSingleSelectQueryAnalyzer >   m_xComposer;
        const OUString                                          m_sUpdateTableName;
        ::std::vector< OUString >                               m_aFilterColumns;
        sal_Int32&                                              m_rRowCount;

        bool m_bRowCountFinal;

        /** copies the values from the insert row into the key row
        *
        * \param _rInsertRow the row which was inserted
        * \param _rKeyRow The current key row of the row set.
        + \param i_nBookmark The bookmark is used to update the parameter
        */
        void copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow,sal_Int32 i_nBookmark);

        css::uno::Reference< css::container::XNameAccess > getKeyColumns() const;
        // returns true if it did any work
        bool fillAllRows();
        bool fetchRow();
        void invalidateRow();

        static void impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData);
        void initColumns();
        void findTableColumnsMatching_throw( const css::uno::Any& i_aTable,
                                             const OUString& i_rUpdateTableName,
                                             const css::uno::Reference< css::sdbc::XDatabaseMetaData>& i_xMeta,
                                             const css::uno::Reference< css::container::XNameAccess>& i_xQueryColumns,
                                             ::std::unique_ptr<SelectColumnsMetaData>& o_pKeyColumnNames);
        void ensureStatement( );
        virtual void makeNewStatement( );
        static void setOneKeyColumnParameter( sal_Int32 &nPos,
                                       const css::uno::Reference<  css::sdbc::XParameters > &_xParameter,
                                       const connectivity::ORowSetValue &_rValue,
                                       sal_Int32 _nType,
                                       sal_Int32 _nScale );
        OUStringBuffer createKeyFilter( );
        bool doTryRefetch_throw() throw(css::sdbc::SQLException, css::uno::RuntimeException);;
        void tryRefetch(const ORowSetRow& _rInsertRow,bool bRefetch);
        void executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOriginalRow,const OUString& i_sSQL,const OUString& i_sTableName,const ::std::vector<sal_Int32>& _aIndexColumnPositions = ::std::vector<sal_Int32>());
        void executeInsert( const ORowSetRow& _rInsertRow,const OUString& i_sSQL,const OUString& i_sTableName = OUString(),bool bRefetch = false);
        void executeStatement(OUStringBuffer& io_aFilter, css::uno::Reference< css::sdb::XSingleSelectQueryComposer>& io_xAnalyzer);

        virtual ~OKeySet();
    public:
        OKeySet(const connectivity::OSQLTable& _xTable,
                const css::uno::Reference< css::container::XIndexAccess>& _xTableKeys,
                const OUString& _rUpdateTableName,
                const css::uno::Reference< css::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                const ORowSetValueVector& _aParameterValueForCache,
                sal_Int32 i_nMaxRows,
                sal_Int32& o_nRowCount);

        // late ctor which can throw exceptions
        virtual void construct(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter) override;
        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) override;

        // css::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;


        virtual bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        // css::sdbc::XResultSet
        virtual bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        void SAL_CALL ensureRowForData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        // css::sdbcx::XRowLocate
        virtual css::uno::Any SAL_CALL getBookmark() throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        virtual bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        virtual bool SAL_CALL hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        // css::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable   ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;


        virtual bool previous_checked( bool i_bFetchRow ) override;
        virtual bool absolute_checked( sal_Int32 row,bool i_bFetchRow ) override;
        virtual bool last_checked( bool i_bFetchRow) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_KEYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
