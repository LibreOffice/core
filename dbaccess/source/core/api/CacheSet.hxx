/*************************************************************************
 *
 *  $RCSfile: CacheSet.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:38 $
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
#ifndef DBACCESS_CORE_API_CACHESET_HXX
#define DBACCESS_CORE_API_CACHESET_HXX

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif

namespace com{ namespace sun { namespace star{namespace sdbc{ class XParameters; } } } }
namespace connectivity{ class OSQLParseTreeIterator; }

namespace dbaccess
{
    class OCacheSet
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xDriverSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>                 m_xDriverRow;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>   m_xSetMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;

        ORowSetRow                                  m_aInsertRow;
        const connectivity::OSQLParseTreeIterator*  m_pIterator; // the parseiterator from RowSet
        ::rtl::OUString                             m_aComposedTableName;
        sal_Bool                                    m_bInserted;
        sal_Bool                                    m_bUpdated;
        sal_Bool                                    m_bDeleted;

        OCacheSet(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,
                    const connectivity::OSQLParseTreeIterator*  _pIterator)
            : m_xDriverSet(_xDriverSet)
            ,m_xDriverRow(m_xDriverSet,::com::sun::star::uno::UNO_QUERY)
            ,m_xSetMetaData(
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::sdbc::XResultSetMetaDataSupplier>(_xDriverSet,::com::sun::star::uno::UNO_QUERY)->getMetaData())
            ,m_xConnection( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>(m_xDriverSet->getStatement(),::com::sun::star::uno::UNO_QUERY)->getConnection())
            ,m_pIterator(_pIterator)
            ,m_bInserted(sal_False)
            ,m_bUpdated(sal_False)
            ,m_bDeleted(sal_False)
        {
        }

        void setParameter(sal_Int32 nPos,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters > _xParameter,const ORowSetValue& _rValue) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void fillTableName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    public:
        virtual void fillValueRow(ORowSetRow& _rRow);

        // ::com::sun::star::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->wasNull();
        }
        // -------------------------------------------------------------------------
        virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getString(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getBoolean(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getByte(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getShort(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getInt(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getLong(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getFloat(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getDouble(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getBytes(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getDate(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getTime(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getTimestamp(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getBinaryStream(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getCharacterStream(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getObject(columnIndex,typeMap);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getRef(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getBlob(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getClob(columnIndex);
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverRow->getArray(columnIndex);
        }
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->next();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->isBeforeFirst();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->isAfterLast();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->isFirst();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->isLast();
        }
        // -------------------------------------------------------------------------
        virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            m_xDriverSet->beforeFirst();
        }
        // -------------------------------------------------------------------------
        virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            m_xDriverSet->afterLast();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->first();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->last();
        }
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->getRow();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->absolute(row);
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->relative(rows);
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->previous();
        }
        // -------------------------------------------------------------------------
        virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            m_xDriverSet->refreshRow();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->rowUpdated();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->rowInserted();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->rowDeleted();
        }
        // -------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xDriverSet->getStatement();
        }
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark( const ORowSetRow& _rRow ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRow( const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL deleteRow( const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;
    };
}
#endif //DBACCESS_CORE_API_CACHESET_HXX
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/09/18 14:52:46  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:24:14  oj
    rowset addon

    Revision 1.0 26.07.2000 14:45:35  oj
------------------------------------------------------------------------*/

