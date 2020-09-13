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
#pragma once

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "RowSetRow.hxx"
#include <cppuhelper/implbase.hxx>

#include <vector>

namespace com::sun::star::sdbc{ class XParameters; }

namespace dbaccess
{
    class OCacheSet : public ::cppu::WeakImplHelper< css::sdbc::XRow>
    {
    protected:
        css::uno::Reference< css::sdbc::XResultSet>           m_xDriverSet;
        css::uno::Reference< css::sdbc::XRow>                 m_xDriverRow;
        css::uno::Reference< css::sdbc::XResultSetMetaData>   m_xSetMetaData;
        css::uno::Reference< css::sdbc::XConnection>          m_xConnection;

        std::vector<bool>                       m_aNullable;
        std::vector<bool>                       m_aSignedFlags;
        std::vector<sal_Int32>                  m_aColumnTypes;
        OUString                                m_aComposedTableName;
        sal_Int32                               m_nMaxRows;
        bool                                    m_bInserted;
        bool                                    m_bUpdated;
        bool                                    m_bDeleted;
        OUString                                m_sRowSetFilter;

        explicit OCacheSet(sal_Int32 i_nMaxRows);
        virtual ~OCacheSet() override;

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
                            ,std::vector< sal_Int32>& _rOrgValues);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void fillTableName(const css::uno::Reference< css::beans::XPropertySet>& _xTable);

        OUString getIdentifierQuoteString() const;
    public:

        // late constructor
        virtual void construct(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter);
        virtual void fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition);

        // css::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) override;
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) override;
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) override;
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) override;
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) override;
        // css::sdbc::XResultSet
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool next();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void beforeFirst(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void afterLast(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool first();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool last();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual sal_Int32 getRow(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool absolute( sal_Int32 row );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool previous(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void refreshRow(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool rowUpdated(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool rowInserted(  );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool rowDeleted(  );
        // css::sdbcx::XRowLocate
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual css::uno::Any getBookmark() = 0;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool moveToBookmark( const css::uno::Any& bookmark ) = 0;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual sal_Int32 compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) = 0;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual bool hasOrderedBookmarks(  ) = 0;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual sal_Int32 hashBookmark( const css::uno::Any& bookmark ) = 0;
        // css::sdbc::XResultSetUpdate
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void updateRow( const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        virtual void deleteRow( const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable ) = 0;

        virtual bool isResultSetChanged() const;
        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) = 0;
        virtual void mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,std::vector<sal_Int32>& o_aChangedColumns);
        virtual bool columnValuesUpdated(ORowSetValueVector::Vector& o_aCachedRow,const ORowSetValueVector::Vector& i_aRow);
        virtual bool updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const std::vector<sal_Int32>& i_aChangedColumns);
        virtual void fillMissingValues(ORowSetValueVector::Vector& io_aRow) const;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
