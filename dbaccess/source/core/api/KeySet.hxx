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

#include <cppuhelper/implbase1.hxx>
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
        sal_Bool        bNullable;

        SelectColumnDescription()
            :nPosition( 0 )
            ,nType( 0 )
            ,nScale( 0 )
            ,bNullable(sal_False)
        {
        }

        SelectColumnDescription( sal_Int32 _nPosition, sal_Int32 _nType, sal_Int32 _nScale,sal_Bool _bNullable, const OUString& _rDefaultValue )
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
    void getColumnPositions(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxQueryColumns,
                            const ::com::sun::star::uno::Sequence< OUString >& _rColumnNames,
                            const OUString& _rsUpdateTableName,
                            SelectColumnsMetaData& o_rColumnNames /* out */,
                            bool i_bAppendTableName = false);

    typedef ::std::pair<ORowSetRow,::std::pair<sal_Int32,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow> > > OKeySetValue;
    typedef ::std::map<sal_Int32,OKeySetValue > OKeySetMatrix;
    typedef ::std::map<sal_Int32,ORowSetValueVector > OUpdatedParameter;
    // is used when the source supports keys
    class OKeySet : public OCacheSet
    {
    protected:
        OKeySetMatrix                                           m_aKeyMap;
        OKeySetMatrix::iterator                                 m_aKeyIter;

        ::std::vector< OUString >                        m_aAutoColumns;  // contains all columns which are autoincrement ones

        OUpdatedParameter                                       m_aUpdatedParameter;    // contains all parameter which have been updated and are needed for refetching
        ORowSetValueVector                                      m_aParameterValueForCache;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pKeyColumnNames;      // contains all key column names
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pColumnNames;         // contains all column names
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pParameterNames;      // contains all parameter names
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pForeignColumnNames;  // contains all column names of the rest
        SAL_WNODEPRECATED_DECLARATIONS_POP
        connectivity::OSQLTable                                 m_xTable; // reference to our table
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>    m_xTableKeys;
        // we need a different SQL (statement) for each different combination
        // of NULLness of key & foreign columns;
        // each subclause is either "colName = ?" or "colName IS NULL"
        // (we avoid the standard "colName IS NOT DISTINCT FROM ?" because it is not widely supported)
        typedef ::std::vector< bool > FilterColumnsNULL_t;
        typedef ::std::map< FilterColumnsNULL_t,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > >
                vStatements_t;
        vStatements_t                                           m_vStatements;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement>   m_xStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>                 m_xRow;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >   m_xComposer;
        const OUString                                   m_sUpdateTableName;
        ::std::vector< OUString >                        m_aFilterColumns;
        sal_Int32&                                              m_rRowCount;

        sal_Bool m_bRowCountFinal;

        /** copies the values from the insert row into the key row
        *
        * \param _rInsertRow the row which was inserted
        * \param _rKeyRow The current key row of the row set.
        + \param i_nBookmark The bookmark is used to update the parameter
        */
        void copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow,sal_Int32 i_nBookmark);

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getKeyColumns() const;
        // returns true if it did any work
        bool fillAllRows();
        sal_Bool fetchRow();
        void invalidateRow();

        void impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData);
        void initColumns();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        void findTableColumnsMatching_throw( const ::com::sun::star::uno::Any& i_aTable,
                                             const OUString& i_rUpdateTableName,
                                             const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& i_xMeta,
                                             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& i_xQueryColumns,
                                             ::std::auto_ptr<SelectColumnsMetaData>& o_pKeyColumnNames);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        void ensureStatement( );
        virtual void makeNewStatement( );
        void setOneKeyColumnParameter( sal_Int32 &nPos,
                                       const ::com::sun::star::uno::Reference<  ::com::sun::star::sdbc::XParameters > &_xParameter,
                                       const connectivity::ORowSetValue &_rValue,
                                       sal_Int32 _nType,
                                       sal_Int32 _nScale ) const;
        OUStringBuffer createKeyFilter( );
        bool doTryRefetch_throw() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);;
        void tryRefetch(const ORowSetRow& _rInsertRow,bool bRefetch);
        void executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOriginalRow,const OUString& i_sSQL,const OUString& i_sTableName,const ::std::vector<sal_Int32>& _aIndexColumnPositions = ::std::vector<sal_Int32>());
        void executeInsert( const ORowSetRow& _rInsertRow,const OUString& i_sSQL,const OUString& i_sTableName = OUString(),bool bRefetch = false);
        void executeStatement(OUStringBuffer& io_aFilter, ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer>& io_xAnalyzer);

        virtual ~OKeySet();
    public:
        OKeySet(const connectivity::OSQLTable& _xTable,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xTableKeys,
                const OUString& _rUpdateTableName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                const ORowSetValueVector& _aParameterValueForCache,
                sal_Int32 i_nMaxRows,
                sal_Int32& o_nRowCount);

        // late ctor which can throw exceptions
        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter);
        virtual void reset(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet);

        // ::com::sun::star::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);


        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL ensureRowForData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


        virtual sal_Bool previous_checked( sal_Bool i_bFetchRow );
        virtual sal_Bool absolute_checked( sal_Int32 row,sal_Bool i_bFetchRow );
        virtual sal_Bool last_checked( sal_Bool i_bFetchRow);
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_KEYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
