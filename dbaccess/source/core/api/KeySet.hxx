/*************************************************************************
 *
 *  $RCSfile: KeySet.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:51:57 $
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

#ifndef DBACCESS_CORE_API_KEYSET_HXX
#define DBACCESS_CORE_API_KEYSET_HXX

#ifndef DBACCESS_CORE_API_CACHESET_HXX
#include "CacheSet.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _MAP_
#include <map>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace dbaccess
{
    typedef ::std::pair<sal_Int32,sal_Int32>    TPositionTypePair;
    DECLARE_STL_MAP(::rtl::OUString, TPositionTypePair,::comphelper::UStringMixLess,OColumnNamePos);

    // the elements of _rxQueryColumns must have the properties PROPERTY_REALNAME and PROPERTY_TABLENAME
    void getColumnPositions(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxQueryColumns,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns,
                            const ::rtl::OUString& _rsUpdateTableName,
                            OColumnNamePos& _rColumnNames /* out */);

    typedef ::std::pair<ORowSetRow,sal_Int32> OKeySetValue;
    typedef ::std::map<sal_Int32,OKeySetValue > OKeySetMatrix;
    // is used when the source supports keys
    class OKeySet : public OCacheSet
    {
        OKeySetMatrix                                           m_aKeyMap;
        OKeySetMatrix::iterator                                 m_aKeyIter;

        connectivity::ORowVector< connectivity::ORowSetValue >  m_aParameterRow; // contains the parameters from rowset
        ::std::vector< ::rtl::OUString >                        m_aAutoColumns;  // contains all columns which are autoincrement ones
        ::std::vector< sal_Int32 >                              m_aKeyColumnTypes;

        OColumnNamePos*                                         m_pKeyColumnNames;  // contains all key column names
        OColumnNamePos*                                         m_pColumnNames;     // contains all column names
        connectivity::OSQLTable                                 m_xTable; // reference to our table
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement>   m_xStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>                 m_xRow;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xComposer;
        ::rtl::OUString                                                                 m_sUpdateTableName;
        ::rtl::OUString                                                                 m_aSelectComposedTableName;

        sal_Bool m_bRowCountFinal;

        /**
            getComposedTableName return the composed table name for the query
            @param _sCatalog    the catalogname may be empty
            @param _sSchema     the schemaname may be empty
            @param _sTable      the tablename

            @return the composed name
        */
        ::rtl::OUString getComposedTableName( const ::rtl::OUString& _sCatalog,
                                              const ::rtl::OUString& _sSchema,
                                              const ::rtl::OUString& _sTable);
        /**
            fetchValue fetches a single value out of the current row
            @param _nPos    the current column position
            @param _nType   the type of the current column
            @param _xRow    the row where to fetch the data from

            @param _rValue  out - value which was fetched
        */
        void fetchValue(sal_Int32 _nPos,
                        sal_Int32 _nType,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>& _xRow,
                        ::connectivity::ORowSetValue& _rValue);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getKeyColumns() const;
        void fillAllRows();
        sal_Bool fetchRow();
    public:
        OKeySet(const connectivity::OSQLTable& _xTable,
                const ::rtl::OUString& _rUpdateTableName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >& _xComposer);
        ~OKeySet();
        // late ctor which can throw exceptions
        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet);

        void setExternParameters(const connectivity::ORowVector< connectivity::ORowSetValue >& _rParameterRow);

        // ::com::sun::star::sdbc::XRow
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
        virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark( const ORowSetRow& _rRow ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_KEYSET_HXX

