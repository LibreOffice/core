/*************************************************************************
 *
 *  $RCSfile: OResultSet.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ODBC_ORESULTSET_HXX_
#define _CONNECTIVITY_ODBC_ORESULTSET_HXX_

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE12_HXX_
#include <cppuhelper/compbase12.hxx>
#endif
#ifndef _UNOTOOLS_PROPERTY_ARRAY_HELPER_HXX_
#include <unotools/proparrhlp.hxx>
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OSTATEMENT_HXX_
#include "odbc/OStatement.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif

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



        class OResultSet :  public  OBaseMutex,
                            public  OResultSet_BASE,
                            public  ::cppu::OPropertySetHelper,
                            public  ::utl::OPropertyArrayUsageHelper<OResultSet>
        {

            ::std::vector<void*>                        m_aBindVector;
            ::std::vector<sal_Int32>                    m_aLengthVector;
            ::std::vector<sal_Int32>                    m_aColMapping; // pos 0 is unused so we don't have to decrement 1 everytime
            ::std::vector< ::com::sun::star::uno::Any>  m_aRow; // only used when SQLGetData can't be called in any order
            SQLHANDLE                                   m_aStatementHandle;
            SQLHANDLE                                   m_aConnectionHandle;
            ::com::sun::star::uno::WeakReferenceHelper  m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            SQLUSMALLINT*                               m_pRowStatusArray;
            sal_Int32                                   m_nRowPos;
            sal_Int32                                   m_nLastColumnPos;       // used for m_aRow just to know where we are
            SQLRETURN                                   m_nCurrentFetchState;
            sal_Bool                                    m_bWasNull;
            sal_Bool                                    m_bBOF;                 // before first record
            sal_Bool                                    m_bEOF;                 // after last record
            sal_Bool                                    m_bLastRecord;
            sal_Bool                                    m_bFreeHandle;
            sal_Bool                                    m_bInserting;
            sal_Bool                                    m_bFetchData;           // true when SQLGetaData can be called in any order or when fetching data for m_aRow

            sal_Bool  isBookmarkable()          const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetConcurrency() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetType()        const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchDirection()       const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchSize()            const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::rtl::OUString getCursorName()     const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            void setFetchDirection(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setFetchSize(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);


            void fillRow(sal_Int32 _nToColumn);
            void allocBuffer(sal_Bool _bAllocRow);
            void releaseBuffer();

        protected:

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
            DECLARE_CTY_DEFAULTS(OResultSet_BASE);
            DECLARE_SERVICE_INFO();
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OResultSet( SQLHANDLE _pStatementHandle,OStatement_Base* pStmt);
            OResultSet( SQLHANDLE _pStatementHandle);
            ~OResultSet();


            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(OResultSet_BASE*)this);
            }

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
            }
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

            // special methods
            inline sal_Int32 mapColumn(sal_Int32    column);
            void openTablesTypes( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTypeInfo() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openCatalogs() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSchemas() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTables(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern,
                                            const ::rtl::OUString& tableNamePattern, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types )
                                            throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumnPrivileges(      const ::com::sun::star::uno::Any& catalog,    const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,   const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumns(       const ::com::sun::star::uno::Any& catalog,                            const ::rtl::OUString& schemaPattern,
                                                    const ::rtl::OUString& tableNamePattern,        const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedureColumns(      const ::com::sun::star::uno::Any& catalog,            const ::rtl::OUString& schemaPattern,
                                                            const ::rtl::OUString& procedureNamePattern,const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedures(    const ::com::sun::star::uno::Any& catalog,            const ::rtl::OUString& schemaPattern,
                                                            const ::rtl::OUString& procedureNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openVersionColumns(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                            const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,sal_Int32 scope,sal_Bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openForeignKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString* schema,const ::rtl::OUString* table,
                                                              const ::com::sun::star::uno::Any& catalog2, const ::rtl::OUString* schema2,const ::rtl::OUString* table2)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openExportedKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openImportedKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openPrimaryKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTablePrivileges(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern,
                                                              const ::rtl::OUString& tableNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSpecialColumns(sal_Bool _bRowVer,const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,sal_Int32 scope,   sal_Bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openIndexInfo( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                    const ::rtl::OUString& table,sal_Bool unique,sal_Bool approximate )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        };
        // -------------------------------------------------------------------------
        inline sal_Int32 OResultSet::mapColumn (sal_Int32   column)
        {
            sal_Int32   map = column;

            if (m_aColMapping.size())
            {
                // Validate column number
                OSL_ENSHURE(column>0,"OResultSet::mapColumn column <= 0");
                map = m_aColMapping[column];
            }

            return map;
        }
    }
}
#endif // _CONNECTIVITY_ODBC_ORESULTSET_HXX_


