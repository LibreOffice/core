/*************************************************************************
 *
 *  $RCSfile: RowSetCache.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: oj $ $Date: 2002-12-05 14:10:10 $
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
#ifndef DBACCESS_CORE_API_ROWSETCACHE_HXX
#define DBACCESS_CORE_API_ROWSETCACHE_HXX

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVEBROADCASTER_HPP_
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE11_HXX_
#include <cppuhelper/compbase11.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#include "RowSetCacheIterator.hxx"
#endif

namespace dbaccess
{
    class OCacheSet;
    class ORowSetDataColumns;
    class ORowSetCacheClone;

    class ORowSetCache
    {
        friend class ORowSetBase;
        friend class ORowSet;
        friend class ORowSetClone;
        friend class ORowSetCacheIterator;

        ::osl::Mutex            m_aRowCountMutex, // mutex for rowcount changes
                                // we need a extra mutex for columns to prevend deadlock when setting new values
                                // for a row
                                m_aColumnsMutex;

        //the set can be static, bookmarkable or keyset
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>       m_xSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xMetaData; // must be before m_aInsertRow
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xServiceFactory;

        OCacheSet*                      m_pCacheSet;            // is a bookmarkable, keyset or static resultset
        ORowSetMatrix*                  m_pMatrix;              // represent the table struct
        ORowSetMatrix::iterator         m_aMatrixIter;          // represent a row of the table
        ORowSetMatrix::iterator         m_aMatrixEnd;           // present the row behind the last row of the table
        ORowSetCacheMap                 m_aCacheIterators;

        ORowSetMatrix*                  m_pInsertMatrix;        // represent the rows which should be inserted normally this is only one
        ORowSetMatrix::iterator         m_aInsertRow;           // represent a insert row

        //  ORowSetRow                      m_aInsertRow;           // present the row that should be inserted
        sal_Int32                       m_nLastColumnIndex;     // the last column ask for, used for wasNull()

        connectivity::OSQLTable         m_aUpdateTable;         // used for updates/deletes and inserts

        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nRowCount;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nPosition;                // 0 means beforefirst

        sal_Int32                   m_nStartPos;                // start pos of the window zero based
        sal_Int32                   m_nEndPos;                  // end   pos of the window zero based

        sal_Bool                    m_bRowCountFinal ;
        sal_Bool                    m_bBeforeFirst ;
        sal_Bool                    m_bAfterLast ;
        sal_Bool                    m_bInserted;
        sal_Bool                    m_bDeleted ;
        sal_Bool                    m_bUpdated ;
        sal_Bool&                   m_bModified ;           // points to the rowset member m_bModified
        sal_Bool&                   m_bNew ;                // points to the rowset member m_bNew

        sal_Bool fillMatrix(sal_Int32 &_nNewStartPos,sal_Int32 _nNewEndPos);
        sal_Bool moveWindow();

        void firePropertyChange(sal_Int32 _nColumnIndex,const ::connectivity::ORowSetValue& _rOldValue);

        void rotateCacheIterator(sal_Int16 _nDist);
        void updateValue(sal_Int32 columnIndex,const connectivity::ORowSetValue& x);
        connectivity::ORowSetValue getValue(sal_Int32 columnIndex);
        // checks and set the flags isAfterLast isLast and position when afterlast is true
        void checkPositionFlags();
        void checkUpdateConditions(sal_Int32 columnIndex);
        sal_Bool checkJoin( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >& _xComposer,
                            const ::rtl::OUString& _sUpdateTableName);

        // clears the insert row
        void                    clearInsertRow();
        ORowSetMatrix::iterator calcPosition() const;
    protected:
        ORowSetMatrix::iterator& getIterator() { return m_aMatrixIter;}
        ORowSetMatrix::iterator& getEnd() { return m_aMatrixEnd;}
        // is called when after a moveToInsertRow a movement (next, etc) was called
        void cancelRowModification();
    public:
        ORowSetCache(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >&,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >& _xComposer,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory,
                     const ORowSetValueVector& _rParameterRow,
                     const ::rtl::OUString& _rUpdateTableName,
                     sal_Bool&  _bModified,
                     sal_Bool&  _bNew);
        ~ORowSetCache();


        // called from the rowset when a updateXXX was called for the first time
        void setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow);
        ORowSetCacheIterator createIterator();
        // sets the size of the matrix
        void setMaxRowSize(sal_Int32 _nSize);

    // OComponentHelper
        void disposing(void);

    // ::com::sun::star::lang::XEventListener
        void disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XCloseable
        void close(  );

    // ::com::sun::star::sdbc::XResultSetMetaDataSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > getMetaData(  );

    // ::com::sun::star::sdbcx::XColumnsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRow
        sal_Bool wasNull(  );
        ::rtl::OUString getString( sal_Int32 columnIndex );
        sal_Bool getBoolean( sal_Int32 columnIndex );
        sal_Int8 getByte( sal_Int32 columnIndex );
        sal_Int16 getShort( sal_Int32 columnIndex );
        sal_Int32 getInt( sal_Int32 columnIndex );
        sal_Int64 getLong( sal_Int32 columnIndex );
        float getFloat( sal_Int32 columnIndex );
        double getDouble( sal_Int32 columnIndex );
        ::com::sun::star::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex );
        ::com::sun::star::util::Date getDate( sal_Int32 columnIndex );
        ::com::sun::star::util::Time getTime( sal_Int32 columnIndex );
        ::com::sun::star::util::DateTime getTimestamp( sal_Int32 columnIndex );
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getBinaryStream( sal_Int32 columnIndex );
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getCharacterStream( sal_Int32 columnIndex );
        ::com::sun::star::uno::Any getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > getRef( sal_Int32 columnIndex );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > getBlob( sal_Int32 columnIndex );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > getClob( sal_Int32 columnIndex );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > getArray( sal_Int32 columnIndex );

    // ::com::sun::star::sdbcx::XRowLocate
        ::com::sun::star::uno::Any getBookmark(  );
        sal_Bool moveToBookmark( const ::com::sun::star::uno::Any& bookmark );
        sal_Bool moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows );
        sal_Int32 compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second );
        sal_Bool hasOrderedBookmarks(  );
        sal_Int32 hashBookmark( const ::com::sun::star::uno::Any& bookmark );

    // ::com::sun::star::sdbc::XRowUpdate
        void updateNull( sal_Int32 columnIndex );
        void updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length );
        void updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length );
        void updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x );
        void updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale );

    // ::com::sun::star::sdbc::XResultSet
        sal_Bool next(  );
        sal_Bool isBeforeFirst(  );
        sal_Bool isAfterLast(  );
        sal_Bool isFirst(  );
        sal_Bool isLast(  );
        void beforeFirst(  );
        void afterLast(  );
        sal_Bool first(  );
        sal_Bool last(  );
        sal_Int32 getRow(  );
        sal_Bool absolute( sal_Int32 row );
        sal_Bool relative( sal_Int32 rows );
        sal_Bool previous(  );
        void refreshRow(  );
        sal_Bool rowUpdated(  );
        sal_Bool rowInserted(  );
        sal_Bool rowDeleted(  );
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getStatement(  );

    // ::com::sun::star::sdbc::XResultSetUpdate
        void insertRow(  );
        void updateRow(  );
        void updateRow( ORowSetMatrix::iterator& _rUpdateRow );
        void deleteRow(  );
        void cancelRowUpdates(  );
        void moveToInsertRow(  );
        void moveToCurrentRow(  );

    // ::com::sun::star::sdbc::XRowSet
        void execute(  );
    // ::com::sun::star::util::XCancellable
        void cancel(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDeleteRows
        ::com::sun::star::uno::Sequence< sal_Int32 > deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows );

    // ::com::sun::star::sdbc::XWarningsSupplier
        ::com::sun::star::uno::Any getWarnings(  );
        void clearWarnings(  );
    };
}
#endif
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.17  2002/12/05 09:54:54  fs
    #105390# #i8481# cancelInsert renamed to cancelRowModification

    Revision 1.16  2001/11/29 16:35:26  oj
    #95225# changes for bookmarkable resultset

    Revision 1.15  2001/08/24 06:25:57  oj
    #90015# code corrcetions for some speedup's

    Revision 1.14  2001/08/09 13:12:51  oj
    #90801# clear insertrow after insert

    Revision 1.13  2001/07/24 13:25:25  oj
    #89430# move ORowSetValue into dbtools

    Revision 1.12  2001/07/19 09:29:22  oj
    #86186# check parsetree for joins

    Revision 1.11  2001/07/12 07:56:32  oj
    #89437# positioning cache when standing on a row outside the cache

    Revision 1.9  2001/06/26 10:30:55  oj
    #87808# setObject corrected and some more

    Revision 1.8  2001/06/26 09:32:05  fs
    #88392# added columnModified for diagnostics

    Revision 1.7  2001/02/01 14:23:57  oj
    change for insert , delete and update rows

    Revision 1.6  2001/01/22 07:38:24  oj
    #82632# change member

    Revision 1.5  2000/10/17 10:18:12  oj
    some changes for the rowset

    Revision 1.4  2000/10/11 11:18:11  fs
    replace unotools with comphelper

    Revision 1.3  2000/10/05 09:33:39  fs
    using comphelper::OPropertyContainer instead of connectivity::OSimplePropertyContainer

    Revision 1.2  2000/09/29 15:20:51  oj
    rowset impl

    Revision 1.1.1.1  2000/09/19 00:15:38  hr
    initial import

    Revision 1.2  2000/09/18 14:52:47  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:20:59  oj
    rowset addons

    Revision 1.0 25.07.2000 16:37:44  oj
------------------------------------------------------------------------*/

