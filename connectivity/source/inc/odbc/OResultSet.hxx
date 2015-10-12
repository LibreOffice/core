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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ORESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ORESULTSET_HXX

#include <com/sun/star/sdbc/FetchDirection.hpp>
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
#include "odbc/OFunctions.hxx"
#include "odbc/OStatement.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include <connectivity/CommonTools.hxx>
#include <connectivity/FValue.hxx>
#include "TSkipDeletedSet.hxx"

namespace connectivity
{
    namespace odbc
    {

        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper<      ::com::sun::star::sdbc::XResultSet,
                                                      ::com::sun::star::sdbc::XRow,
                                                      ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                      ::com::sun::star::util::XCancellable,
                                                      ::com::sun::star::sdbc::XWarningsSupplier,
                                                      ::com::sun::star::sdbc::XResultSetUpdate,
                                                      ::com::sun::star::sdbc::XRowUpdate,
                                                      ::com::sun::star::sdbcx::XRowLocate,
                                                      ::com::sun::star::sdbcx::XDeleteRows,
                                                      ::com::sun::star::sdbc::XCloseable,
                                                      ::com::sun::star::sdbc::XColumnLocate,
                                                      ::com::sun::star::lang::XServiceInfo> OResultSet_BASE;


        typedef ::std::pair<sal_Int64,sal_Int32> TVoidPtr;
        typedef ::std::allocator< TVoidPtr > TVoidAlloc;
        typedef ::std::vector<TVoidPtr> TVoidVector;
        //  typedef ::com::sun::star::uno::Sequence<TVoidPtr> TVoidVector;
        /// unary_function Functor object for class ZZ returntype is void
        struct OOO_DLLPUBLIC_ODBCBASE TBookmarkPosMapCompare : ::std::binary_function< ::com::sun::star::uno::Sequence<sal_Int8>, ::com::sun::star::uno::Sequence<sal_Int8>, bool >
        {
            inline bool operator()( const ::com::sun::star::uno::Sequence<sal_Int8>& _rLH,
                                    const ::com::sun::star::uno::Sequence<sal_Int8>& _rRH) const
            {
                if(_rLH.getLength() == _rRH.getLength())
                {
                    sal_Int32 nCount = _rLH.getLength();
                    if(nCount != 4)
                    {
                        const sal_Int8* pLHBack = _rLH.getConstArray() + nCount - 1;
                        const sal_Int8* pRHBack = _rRH.getConstArray() + nCount - 1;

                        sal_Int32 i;
                        for(i=0;i < nCount;++i,--pLHBack,--pRHBack)
                        {
                            if(!(*pLHBack) && *pRHBack)
                                return true;
                            else if(*pLHBack && !(*pRHBack))
                                return false;
                        }
                        for(i=0,++pLHBack,++pRHBack;i < nCount;++pLHBack,++pRHBack,++i)
                            if(*pLHBack < *pRHBack)
                                return true;
                        return false;
                    }
                    else
                        return *reinterpret_cast<const sal_Int32*>(_rLH.getConstArray()) < *reinterpret_cast<const sal_Int32*>(_rRH.getConstArray());

                }
                else
                    return _rLH.getLength() < _rRH.getLength();
            }
        };

        typedef ::std::map< ::com::sun::star::uno::Sequence<sal_Int8>, sal_Int32,TBookmarkPosMapCompare > TBookmarkPosMap;

        class OOO_DLLPUBLIC_ODBCBASE OResultSet :
                            public  comphelper::OBaseMutex,
                            public  ::connectivity::IResultSetHelper,
                            public  OResultSet_BASE,
                            public  ::cppu::OPropertySetHelper,
                            public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {
        protected:
            TBookmarkPosMap                             m_aPosToBookmarks;
            // used top hold the information about the value and the datatype to save calls to metadata
            typedef ::std::vector<ORowSetValue>         TDataRow;

            TVoidVector                                 m_aBindVector;
            ::std::vector<SQLLEN>                       m_aLengthVector;
            ::std::map<sal_Int32,SWORD>                 m_aODBCColumnTypes;

            // In baseline ODBC, SQLGetData can only be called on monotonically increasing column numbers.
            // additionally, any variable-length data can be fetched only once (possibly in parts);
            // after that, SQLGetData returns SQL_NO_DATA.
            // In order to insulate our callers from these restrictions,
            // we cache the current row in m_aRow.
            // If the driver claims to support the GD_ANY_ORDER extension,
            // we read and cache only the columns requested by a caller.
            // Else, we read and cache all columns whose number is <= a requested column.
            // m_aRow[colNumber].getBound() says if it contains an up-to-date value or not.
            TDataRow                                    m_aRow;
            bool                                    m_bFetchDataInOrder;
            SQLHANDLE                                   m_aStatementHandle;
            SQLHANDLE                                   m_aConnectionHandle;
            OStatement_Base*                            m_pStatement;
            OSkipDeletedSet*                            m_pSkipDeletedSet;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            SQLUSMALLINT*                               m_pRowStatusArray;
            rtl_TextEncoding                            m_nTextEncoding;
            sal_Int32                                   m_nRowPos;
            mutable sal_uInt32                          m_nUseBookmarks;
            SQLRETURN                                   m_nCurrentFetchState;
            bool                                    m_bWasNull;
            bool                                    m_bEOF;                 // after last record
            bool                                    m_bLastRecord;
            bool                                    m_bFreeHandle;
            bool                                    m_bInserting;
            bool                                    m_bRowInserted;
            bool                                    m_bRowDeleted;
            bool                                    m_bUseFetchScroll;

            bool      isBookmarkable()          const;
            sal_Int32 getResultSetConcurrency() const;
            sal_Int32 getResultSetType()        const;
            static sal_Int32 getFetchDirection() { return css::sdbc::FetchDirection::FORWARD; }
            sal_Int32 getFetchSize()            const;
            OUString getCursorName()     const;
            template < typename T, SQLINTEGER BufferLength > T getStmtOption (SQLINTEGER fOption, T dflt = 0) const;

            void setFetchDirection(sal_Int32 _par0);
            void setFetchSize(sal_Int32 _par0);
            template < typename T, SQLINTEGER BufferLength > SQLRETURN setStmtOption (SQLINTEGER fOption, T value) const;


            void ensureCacheForColumn(sal_Int32 columnIndex);
            void invalidateCache();
            void fillColumn(sal_Int32 _nToColumn);
            void allocBuffer();
            void releaseBuffer();
            void updateValue(sal_Int32 columnIndex,SQLSMALLINT _nType,void* _pValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void fillNeededData(SQLRETURN _nRet);
            bool moveImpl(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData);
            TVoidPtr allocBindColumn(sal_Int32 _nType,sal_Int32 _nColumnIndex);
            SQLRETURN unbind(bool _bUnbindHandle = true);
            SWORD impl_getColumnType_nothrow(sal_Int32 columnIndex);

            // helper to implement XRow::getXXX in simple cases
            template < typename T > T getValue( sal_Int32 columnIndex );
            // impl_getXXX are the functions that do the actual fetching from ODBC, ignoring the cache
            // for simple cases
            template < typename T > T impl_getValue( const sal_Int32 _nColumnIndex, SQLSMALLINT nType );
            // these cases need some special treatment
            bool impl_getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::uno::Sequence< sal_Int8 > impl_getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::util::Date impl_getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::util::Time impl_getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::util::DateTime impl_getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int64 impl_getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OUString impl_getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::com::sun::star::uno::Sequence<sal_Int8> impl_getBookmark(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                     )
                                     throw (::com::sun::star::uno::Exception, std::exception) override;
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const override;
        public:
            DECLARE_SERVICE_INFO();
            // A ctor that is needed for returning the object
            OResultSet( SQLHANDLE _pStatementHandle,OStatement_Base* pStmt);
            virtual ~OResultSet();

            void construct();

            inline oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
            {
                return m_pStatement->getOdbcFunction(_nIndex);
            }

            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*static_cast<OResultSet_BASE*>(this));
            }

            inline void setMetaData(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>& _xMetaData) { m_xMetaData = _xMetaData;}

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XResultSetUpdate
            virtual void SAL_CALL insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XRowUpdate
            virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XRowLocate
            virtual ::com::sun::star::uno::Any SAL_CALL getBookmark(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XDeleteRows
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // IResultSetHelper
            virtual bool move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData) override;
            virtual sal_Int32 getDriverPos() const override;
            virtual bool isRowDeleted() const override;

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ORESULTSET_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
