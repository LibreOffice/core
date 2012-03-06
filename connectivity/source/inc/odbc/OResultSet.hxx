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

#ifndef _CONNECTIVITY_ODBC_ORESULTSET_HXX_
#define _CONNECTIVITY_ODBC_ORESULTSET_HXX_

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
#include <cppuhelper/compbase12.hxx>
#include <comphelper/proparrhlp.hxx>
#include "odbc/OFunctions.hxx"
#include "odbc/OStatement.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include "connectivity/CommonTools.hxx"
#include "connectivity/FValue.hxx"
#include "TSkipDeletedSet.hxx"

namespace connectivity
{
    namespace odbc
    {

        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper12<      ::com::sun::star::sdbc::XResultSet,
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
                                return sal_True;
                            else if(*pLHBack && !(*pRHBack))
                                return sal_False;
                        }
                        for(i=0,++pLHBack,++pRHBack;i < nCount;++pLHBack,++pRHBack,++i)
                            if(*pLHBack < *pRHBack)
                                return sal_True;
                        return sal_False;
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
            ::com::sun::star::uno::Sequence<sal_Int8>   m_aBookmark;

            TDataRow                                    m_aRow; // only used when SQLGetData can't be called in any order
            ORowSetValue                                m_aEmptyValue;  // needed for the getValue method when no prefetch is used
            SQLHANDLE                                   m_aStatementHandle;
            SQLHANDLE                                   m_aConnectionHandle;
            OStatement_Base*                            m_pStatement;
            OSkipDeletedSet*                            m_pSkipDeletedSet;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            SQLUSMALLINT*                               m_pRowStatusArray;
            rtl_TextEncoding                            m_nTextEncoding;
            sal_Int32                                   m_nRowPos;
            sal_Int32                                   m_nLastColumnPos;       // used for m_aRow just to know where we are
            mutable sal_uInt32                          m_nUseBookmarks;
            SQLRETURN                                   m_nCurrentFetchState;
            sal_Bool                                    m_bWasNull;
            sal_Bool                                    m_bEOF;                 // after last record
            sal_Bool                                    m_bLastRecord;
            sal_Bool                                    m_bFreeHandle;
            sal_Bool                                    m_bInserting;
            sal_Bool                                    m_bFetchData;           // true when SQLGetaData can be called in any order or when fetching data for m_aRow
            sal_Bool                                    m_bRowInserted;
            sal_Bool                                    m_bRowDeleted;
            sal_Bool                                    m_bUseFetchScroll;

            sal_Bool  isBookmarkable()          const;
            sal_Int32 getResultSetConcurrency() const;
            sal_Int32 getResultSetType()        const;
            sal_Int32 getFetchDirection()       const;
            sal_Int32 getFetchSize()            const;
            ::rtl::OUString getCursorName()     const;
            template < typename T, SQLINTEGER BufferLength > T getStmtOption (SQLINTEGER fOption, T dflt = 0) const;

            void setFetchDirection(sal_Int32 _par0);
            void setFetchSize(sal_Int32 _par0);
            template < typename T, SQLINTEGER BufferLength > SQLRETURN setStmtOption (SQLINTEGER fOption, T value) const;


            void fillRow(sal_Int32 _nToColumn);
            void allocBuffer();
            void releaseBuffer();
            void updateValue(sal_Int32 columnIndex,SQLSMALLINT _nType,void* _pValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void fillNeededData(SQLRETURN _nRet);
            const ORowSetValue& getValue(sal_Int32 _nColumnIndex,SQLSMALLINT _nType,void* _pValue,SQLINTEGER _rSize);
            sal_Bool moveImpl(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData);
            TVoidPtr allocBindColumn(sal_Int32 _nType,sal_Int32 _nColumnIndex);
            SQLRETURN unbind(sal_Bool _bUnbindHandle = sal_True);
            SWORD impl_getColumnType_nothrow(sal_Int32 columnIndex);

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                     )
                                     throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            DECLARE_SERVICE_INFO();
            // A ctor that is needed for returning the object
            OResultSet( SQLHANDLE _pStatementHandle,OStatement_Base* pStmt);
            virtual ~OResultSet();

            void construct();

            inline oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const
            {
                return m_pStatement->getOdbcFunction(_nIndex);
            }

            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(OResultSet_BASE*)this);
            }

            inline void setMetaData(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>& _xMetaData) { m_xMetaData = _xMetaData;}

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XResultSet
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
            virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetUpdate
            virtual void SAL_CALL insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRowUpdate
            virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRowLocate
            virtual ::com::sun::star::uno::Any SAL_CALL getBookmark(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XDeleteRows
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // IResultSetHelper
            virtual sal_Bool move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData);
            virtual sal_Int32 getDriverPos() const;
            virtual sal_Bool deletedVisible() const;
            virtual sal_Bool isRowDeleted() const;

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };
    }
}
#endif // _CONNECTIVITY_ODBC_ORESULTSET_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
