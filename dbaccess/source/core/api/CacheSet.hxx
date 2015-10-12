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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_CACHESET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_CACHESET_HXX

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include "RowSetRow.hxx"
#include <cppuhelper/implbase.hxx>

#include <list>
namespace com{ namespace sun { namespace star{namespace sdbc{ class XParameters; } } } }

namespace dbaccess
{
    class OCacheSet : public ::cppu::WeakImplHelper< css::sdbc::XRow>
    {
    protected:
        css::uno::Reference< css::sdbc::XResultSet>           m_xDriverSet;
        css::uno::Reference< css::sdbc::XRow>                 m_xDriverRow;
        css::uno::Reference< css::sdbc::XResultSetMetaData>   m_xSetMetaData;
        css::uno::Reference< css::sdbc::XConnection>          m_xConnection;

        css::uno::Sequence<sal_Bool>            m_aNullable;
        css::uno::Sequence<sal_Bool>            m_aSignedFlags;
        css::uno::Sequence<sal_Int32>           m_aColumnTypes;
        ORowSetRow                              m_aInsertRow;
        OUString                                m_aComposedTableName;
        sal_Int32                               m_nMaxRows;
        bool                                    m_bInserted;
        bool                                    m_bUpdated;
        bool                                    m_bDeleted;
        OUString                                m_sRowSetFilter;

        explicit OCacheSet(sal_Int32 i_nMaxRows);
        virtual ~OCacheSet();

        static void setParameter(sal_Int32 nPos
                            ,const css::uno::Reference< css::sdbc::XParameters >& _xParameter
                            ,const connectivity::ORowSetValue& _rValue
                            ,sal_Int32 _nType
                            ,sal_Int32 _nScale
                            );
        void fillParameters( const ORowSetRow& _rRow
                            ,const connectivity::OSQLTable& _xTable
                            ,OUStringBuffer& _sCondition
                            ,OUStringBuffer& _sParameter
                            ,::std::list< sal_Int32>& _rOrgValues);
        void fillTableName(const css::uno::Reference< css::beans::XPropertySet>& _xTable)  throw(css::sdbc::SQLException, css::uno::RuntimeException);

        OUString getIdentifierQuoteString() const;
    public:

        // late constructor
        virtual void construct(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter);
        virtual void fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition);

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
        // css::sdbc::XResultSet
        virtual bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // css::sdbcx::XRowLocate
        virtual css::uno::Any SAL_CALL getBookmark() throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;
        virtual bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;
        virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;
        virtual bool SAL_CALL hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;
        virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;
        // css::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception);
        virtual void SAL_CALL updateRow( const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) = 0;
        virtual void SAL_CALL deleteRow( const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException) = 0;

        virtual bool isResultSetChanged() const;
        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) = 0;
        virtual void mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_aChangedColumns);
        virtual bool columnValuesUpdated(ORowSetValueVector::Vector& o_aCachedRow,const ORowSetValueVector::Vector& i_aRow);
        virtual bool updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const ::std::vector<sal_Int32>& i_aChangedColumns);
        virtual void fillMissingValues(ORowSetValueVector::Vector& io_aRow) const;
        virtual bool previous_checked( bool i_bFetchRow );
        virtual bool absolute_checked( sal_Int32 row,bool i_bFetchRow );
        virtual bool last_checked( bool i_bFetchRow);
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_CACHESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
