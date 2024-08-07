/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#ifndef INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SRESULTSET_HXX
#define INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SRESULTSET_HXX

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
#include "SStatement.hxx"
#include "OSubComponent.hxx"

namespace connectivity
{
    namespace skeleton
    {

        /*
        **  OResultSet
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

        class OResultSet :  public  OBase_Mutex,
                            public  OResultSet_BASE,
                            public  ::cppu::OPropertySetHelper,
                            public  OPropertyArrayUsageHelper<OResultSet>
        {
        protected:
            OStatement_Base*                            m_pStatement;
            ::com::sun::star::uno::WeakReferenceHelper  m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            rtl_TextEncoding                            m_nTextEncoding;
            sal_Bool                                    m_bWasNull;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue );
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                     );
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;

            // you can't delete objects of this type
            virtual ~OResultSet();
        public:
            DECLARE_SERVICE_INFO();

            OResultSet( OStatement_Base* pStmt);


            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(OResultSet_BASE*)this);
            }

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing();
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType );
            virtual void SAL_CALL acquire() SAL_NOEXCEPT;
            virtual void SAL_CALL release() SAL_NOEXCEPT;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  );
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  );
            // XResultSet
            virtual sal_Bool SAL_CALL next(  );
            virtual sal_Bool SAL_CALL isBeforeFirst(  );
            virtual sal_Bool SAL_CALL isAfterLast(  );
            virtual sal_Bool SAL_CALL isFirst(  );
            virtual sal_Bool SAL_CALL isLast(  );
            virtual void SAL_CALL beforeFirst(  );
            virtual void SAL_CALL afterLast(  );
            virtual sal_Bool SAL_CALL first(  );
            virtual sal_Bool SAL_CALL last(  );
            virtual sal_Int32 SAL_CALL getRow(  );
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row );
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows );
            virtual sal_Bool SAL_CALL previous(  );
            virtual void SAL_CALL refreshRow(  );
            virtual sal_Bool SAL_CALL rowUpdated(  );
            virtual sal_Bool SAL_CALL rowInserted(  );
            virtual sal_Bool SAL_CALL rowDeleted(  );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  );
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  );
            virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex );
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex );
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex );
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex );
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex );
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex );
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex );
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex );
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex );
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex );
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex );
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex );
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  );
            // XCancellable
            virtual void SAL_CALL cancel(  );
            // XCloseable
            virtual void SAL_CALL close(  );
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  );
            virtual void SAL_CALL clearWarnings(  );
            // XResultSetUpdate
            virtual void SAL_CALL insertRow(  );
            virtual void SAL_CALL updateRow(  );
            virtual void SAL_CALL deleteRow(  );
            virtual void SAL_CALL cancelRowUpdates(  );
            virtual void SAL_CALL moveToInsertRow(  );
            virtual void SAL_CALL moveToCurrentRow(  );
            // XRowUpdate
            virtual void SAL_CALL updateNull( sal_Int32 columnIndex );
            virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x );
            virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x );
            virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x );
            virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x );
            virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x );
            virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x );
            virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x );
            virtual void SAL_CALL updateString( sal_Int32 columnIndex, const ::rtl::OUString& x );
            virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x );
            virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x );
            virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x );
            virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x );
            virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length );
            virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length );
            virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x );
            virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale );
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName );
            // XRowLocate
            virtual ::com::sun::star::uno::Any SAL_CALL getBookmark(  );
            virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark );
            virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows );
            virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second );
            virtual sal_Bool SAL_CALL hasOrderedBookmarks(  );
            virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark );
            // XDeleteRows
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows );
        };
    }
}

#endif // INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SRESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
