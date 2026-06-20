/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include "SStatement.hxx"
#include <connectivity/CommonTools.hxx>
#include <connectivity/FValue.hxx>

namespace connectivity::skeleton
{

    /*
    **  java_sql_ResultSet
    */
    typedef ::cppu::WeakComponentImplHelper<      css::sdbc::XResultSet,
                                                  css::sdbc::XRow,
                                                  css::sdbc::XResultSetMetaDataSupplier,
                                                  css::util::XCancellable,
                                                  css::sdbc::XWarningsSupplier,
                                                  css::sdbc::XResultSetUpdate,
                                                  css::sdbc::XRowUpdate,
                                                  css::sdbcx::XRowLocate,
                                                  css::sdbcx::XDeleteRows,
                                                  css::sdbc::XCloseable,
                                                  css::sdbc::XColumnLocate,
                                                  css::lang::XServiceInfo> OResultSet_BASE;


    typedef sal_Int64 TVoidPtr;
    typedef std::allocator< TVoidPtr >    TVoidAlloc;
    typedef std::vector<TVoidPtr>         TVoidVector;

    class OResultSet :  public  cppu::BaseMutex,
                        public  OResultSet_BASE,
                        public  ::cppu::OPropertySetHelper,
                        public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
    {
    protected:
        TVoidVector                                 m_aBindVector;
        std::vector<sal_Int32>                    m_aLengthVector;
        std::vector<sal_Int32>                    m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time
        std::vector< ORowSetValue>                m_aRow; // only used when SQLGetData can't be called in any order
        OStatement_Base*                            m_pStatement;
        css::uno::WeakReferenceHelper               m_aStatement;
        css::uno::Reference< css::sdbc::XResultSetMetaData>        m_xMetaData;
        rtl_TextEncoding                            m_nTextEncoding;
        sal_Int32                                   m_nRowPos;
        sal_Int32                                   m_nLastColumnPos;       // used for m_aRow just to know where we are
        bool                                    m_bWasNull;
        bool                                    m_bBOF;                 // before first record
        bool                                    m_bEOF;                 // after last record
        bool                                    m_bLastRecord;
        bool                                    m_bFreeHandle;
        bool                                    m_bInserting;
        bool                                    m_bFetchData;           // true when SQLGetaData can be called in any order or when fetching data for m_aRow

        bool  isBookmarkable()          const throw(css::sdbc::SQLException, css::uno::RuntimeException);
        sal_Int32 getResultSetConcurrency() const throw(css::sdbc::SQLException, css::uno::RuntimeException);
        sal_Int32 getResultSetType()        const throw(css::sdbc::SQLException, css::uno::RuntimeException);
        sal_Int32 getFetchDirection()       const throw(css::sdbc::SQLException, css::uno::RuntimeException);
        sal_Int32 getFetchSize()            const throw(css::sdbc::SQLException, css::uno::RuntimeException);
        OUString getCursorName()            const throw(css::sdbc::SQLException, css::uno::RuntimeException);

        void setFetchDirection(sal_Int32 _par0) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        void setFetchSize(sal_Int32 _par0) throw(css::sdbc::SQLException, css::uno::RuntimeException);


        void fillRow(sal_Int32 _nToColumn);
        void allocBuffer(bool _bAllocRow);
        void releaseBuffer();


        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        virtual bool SAL_CALL convertFastPropertyValue(
                            cpo::uno::Any & rConvertedValue,
                            cpo::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const cpo::uno::Any& rValue )
                                throw (css::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const cpo::uno::Any& rValue
                                 )
                                 throw (css::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue(
                                cpo::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const;

        // you can't delete objects of this type
        virtual ~OResultSet();
    public:
        DECLARE_SERVICE_INFO();

        OResultSet( OStatement_Base* pStmt);


        css::uno::Reference< css::uno::XInterface > operator *()
        {
            return css::uno::Reference< css::uno::XInterface >(*(OResultSet_BASE*)this);
        }

        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing();
        // XInterface
        virtual cpo::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw(css::uno::RuntimeException);
        virtual void SAL_CALL release() throw(css::uno::RuntimeException);
        //XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException);
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException);
        // XResultSet
        virtual bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL isLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL relative( sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRow
        virtual bool SAL_CALL wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual cpo::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XCancellable
        virtual void SAL_CALL cancel(  ) throw(css::uno::RuntimeException);
        // XCloseable
        virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XWarningsSupplier
        virtual cpo::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XResultSetUpdate
        virtual void SAL_CALL insertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRowUpdate
        virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, bool x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const css::util::Date& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const css::util::Time& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const cpo::uno::Any& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const cpo::uno::Any& x, sal_Int32 scale ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XColumnLocate
        virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRowLocate
        virtual cpo::uno::Any SAL_CALL getBookmark(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL moveToBookmark( const cpo::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL moveRelativeToBookmark( const cpo::uno::Any& bookmark, sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL compareBookmarks( const cpo::uno::Any& first, const cpo::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool SAL_CALL hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL hashBookmark( const cpo::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XDeleteRows
        virtual css::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const css::uno::Sequence< cpo::uno::Any >& rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
    };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
