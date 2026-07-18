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
        cpo::uno::WeakReferenceHelper               m_aStatement;
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
        virtual ::cppu::IPropertyArrayHelper & getInfoHelper();

        virtual bool convertFastPropertyValue(
                            cpo::uno::Any & rConvertedValue,
                            cpo::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const cpo::uno::Any& rValue )
                                throw (css::lang::IllegalArgumentException);
        virtual void setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const cpo::uno::Any& rValue
                                 )
                                 throw (css::uno::Exception);
        virtual void getFastPropertyValue(
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
        virtual void disposing();
        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) throw(css::uno::RuntimeException);
        virtual void acquire() throw(css::uno::RuntimeException);
        virtual void release() throw(css::uno::RuntimeException);
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) throw(css::uno::RuntimeException);
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) throw(css::uno::RuntimeException);
        // XResultSet
        virtual bool next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool isFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool isLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool relative( sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::uno::XInterface > getStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRow
        virtual bool wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual OUString getString( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool getBoolean( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual float getFloat( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual double getDouble( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::Date getDate( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::Time getTime( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XCancellable
        virtual void cancel(  ) throw(css::uno::RuntimeException);
        // XCloseable
        virtual void close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XResultSetUpdate
        virtual void insertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void deleteRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void cancelRowUpdates(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void moveToInsertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void moveToCurrentRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRowUpdate
        virtual void updateNull( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateBoolean( sal_Int32 columnIndex, bool x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateFloat( sal_Int32 columnIndex, float x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateDouble( sal_Int32 columnIndex, double x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateString( sal_Int32 columnIndex, const OUString& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateBytes( sal_Int32 columnIndex, const cpo::uno::Sequence< sal_Int8 >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateDate( sal_Int32 columnIndex, const css::util::Date& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateTime( sal_Int32 columnIndex, const css::util::Time& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateObject( sal_Int32 columnIndex, const cpo::uno::Any& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual void updateNumericObject( sal_Int32 columnIndex, const cpo::uno::Any& x, sal_Int32 scale ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XColumnLocate
        virtual sal_Int32 findColumn( const OUString& columnName ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XRowLocate
        virtual cpo::uno::Any getBookmark(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool moveToBookmark( const cpo::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool moveRelativeToBookmark( const cpo::uno::Any& bookmark, sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 compareBookmarks( const cpo::uno::Any& first, const cpo::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual bool hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        virtual sal_Int32 hashBookmark( const cpo::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        // XDeleteRows
        virtual cpo::uno::Sequence< sal_Int32 > deleteRows( const cpo::uno::Sequence< cpo::uno::Any >& rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
    };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
