/*************************************************************************
 *
 *  $RCSfile: MResultSet.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dkenny $ $Date: 2001-12-12 15:32:45 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef CONNECTIVITY_SRESULTSET_HXX
#define CONNECTIVITY_SRESULTSET_HXX

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
#ifndef _CPPUHELPER_COMPBASE8_HXX_
#include <cppuhelper/compbase8.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef CONNECTIVITY_SSTATEMENT_HXX
#include "MStatement.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "connectivity/FValue.hxx"
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif
#ifndef CONNECTIVITY_TSORTINDEX_HXX
#include "TSortIndex.hxx"
#endif
#include "mozillasrc/MQuery.hxx"

namespace connectivity
{
    namespace mozab
    {

        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper8<      ::com::sun::star::sdbc::XResultSet,
                                                        ::com::sun::star::sdbc::XRow,
                                                        ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                        ::com::sun::star::util::XCancellable,
                                                        ::com::sun::star::sdbc::XWarningsSupplier,
                                                        ::com::sun::star::sdbc::XCloseable,
                                                        ::com::sun::star::sdbc::XColumnLocate,
                                                        ::com::sun::star::lang::XServiceInfo> OResultSet_BASE;


        typedef sal_Int64 TVoidPtr;
        typedef ::std::allocator< TVoidPtr >    TVoidAlloc;
        typedef ::std::vector<TVoidPtr>         TVoidVector;

        class OResultSet :  public  comphelper::OBaseMutex,
                            public  OResultSet_BASE,
                            public  ::cppu::OPropertySetHelper,
                            public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {
        protected:
            OStatement_Base*                            m_pStatement;
            ::com::sun::star::uno::WeakReferenceHelper  m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            sal_uInt32                                  m_nRowPos;
            sal_Bool                                    m_bWasNull;
            sal_Int32                                   m_nFetchSize;
            sal_Int32                                   m_nResultSetType;
            sal_Int32                                   m_nFetchDirection;
            sal_Int32                                   m_nResultSetConcurrency;



            connectivity::OSQLParseTreeIterator&        m_aSQLIterator;
            const connectivity::OSQLParseNode*          m_pParseTree;

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

            // you can't delete objects of this type
            virtual ~OResultSet();
        public:
            DECLARE_SERVICE_INFO();

            OResultSet(OStatement_Base* pStmt, connectivity::OSQLParseTreeIterator&   _aSQLIterator );


            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(OResultSet_BASE*)this);
            }

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
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
protected:
            MQuery                   m_aQuery;
            OTable*                  m_pTable;

            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                                     m_xTableColumns;

            ::std::vector<sal_Int32> m_aColMapping; // pos 0 is unused so we don't have to decrement 1 everytime
            ::std::vector<sal_Int32> m_aOrderbyColumnNumber;
            ::std::vector<sal_Int16> m_aOrderbyAscending;
            ::com::sun::star::uno::Sequence< ::rtl::OUString> m_aColumnNames;
            OValueRow                m_aRow;
            OValueRow                m_aParameterRow;
            ::std::vector< ::rtl::OUString> m_aAttributeStrings;
            sal_Int32                m_nParamIndex;
            sal_Int32                m_nRowCountResult;
            sal_Bool                 m_nIsAlwaysFalseQuery;
            ::vos::ORef<OKeySet>     m_pKeySet;
            OSortIndex*              m_pSortIndex;


            inline void resetParameters() { m_nParamIndex = 0; }

            ::vos::ORef<connectivity::OSQLColumns>  m_xColumns; // this are the select columns
            ::vos::ORef<connectivity::OSQLColumns>  m_xParamColumns;

            void parseParameter( const OSQLParseNode* pNode, rtl::OUString& rMatchString );
            void fillRowData() throw( ::com::sun::star::sdbc::SQLException );
            void initializeRow(OValueRow& _rRow,sal_Int32 _nColumnCount);
            void analyseWhereClause( const OSQLParseNode*                 parseTree,
                                     MQueryExpression                    &queryExpression,
                                     connectivity::OSQLParseTreeIterator& aSQLIterator);

            sal_Bool isCount() const;

            // XXX sal_Bool IsSorted() const { return !m_aOrderbyColumnNumber.empty() && m_aOrderbyColumnNumber[0] != STRING_NOTFOUND ;}
            sal_Bool IsSorted() const { return !m_aOrderbyColumnNumber.empty(); }

            enum eRowPosition {
                NEXT_POS, PRIOR_POS, FIRST_POS, LAST_POS, ABSOLUTE_POS, RELATIVE_POS
            };

            sal_uInt32  currentRowCount();

            sal_Bool fetchRow(sal_uInt32 rowIndex) throw( ::com::sun::star::sdbc::SQLException,
                                                          ::com::sun::star::uno::RuntimeException);
            sal_Bool validRow( sal_uInt32 nRow );
            sal_Bool seekRow( eRowPosition pos, sal_Int32 nOffset = 0 );

            const ORowSetValue& getValue(sal_uInt32 rowIndex, sal_Int32 columnIndex)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
public:
            // MozAddressbook Specific methods
            void SAL_CALL executeQuery() throw( ::com::sun::star::sdbc::SQLException,
                                                ::com::sun::star::uno::RuntimeException);

            void setTable(OTable* _rTable)
            {
                m_pTable = _rTable;
                m_pTable->acquire();
                m_xTableColumns = m_pTable->getColumns();
                if(m_xTableColumns.is())
                    m_aColumnNames = m_xTableColumns->getElementNames();
            }

            void setParameterRow(const OValueRow& _rParaRow)
                      { m_aParameterRow = _rParaRow; }

            void setParameterColumns(const ::vos::ORef<connectivity::OSQLColumns>& _xParamColumns)
                      { m_xParamColumns = _xParamColumns; }

            void setBindingRow(const OValueRow& _aRow)
                      { m_aRow = _aRow; }

            void setColumnMapping(const ::std::vector<sal_Int32>& _aColumnMapping);

            void setOrderByColumns(const ::std::vector<sal_Int32>& _aColumnOrderBy)
                      { m_aOrderbyColumnNumber = _aColumnOrderBy; }

            void setOrderByAscending(const ::std::vector<sal_Int16>& _aOrderbyAsc)
                      { m_aOrderbyAscending = _aOrderbyAsc; }

            inline sal_Int32 mapColumn(sal_Int32 column);

            void checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException);

            static void setBoundedColumns(
                const OValueRow& _rRow,
                const ::vos::ORef<connectivity::OSQLColumns>& _rxColumns,
                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xNames,
                sal_Bool _bSetColumnMapping,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData,
                ::std::vector<sal_Int32>& _rColMapping);
        };

        inline sal_Int32 OResultSet::mapColumn(sal_Int32 column)
        {
            sal_Int32   map = column;

            OSL_ENSURE(column > 0, "OResultSet::mapColumn: invalid column index!");
                // the first column (index 0) is for convenience only. The
                // first real select column is no 1.
            if ((column > 0) && (column < (sal_Int32)m_aColMapping.size()))
                map = m_aColMapping[column];

            return map;
        }

    }
}
#endif // CONNECTIVITY_SRESULTSET_HXX
