/*************************************************************************
 *
 *  $RCSfile: MResultSet.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmaher $ $Date: 2001-10-11 10:07:54 $
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

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_TSORTINDEX_HXX_
#include <TSortIndex.hxx>
#endif

#include <rtl/string.hxx>
#include <vector>

#include "MResultSet.hxx"
#include "MResultSetMetaData.hxx"

#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif

#ifdef DEBUG
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* DEBUG */
# define OUtoCStr( x ) ("dummy")
#endif /* DEBUG */

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mozab;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
::rtl::OUString SAL_CALL OResultSet::getImplementationName(  ) throw ( RuntimeException)    \
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.mozab.ResultSet");
}
// -------------------------------------------------------------------------
 Sequence< ::rtl::OUString > SAL_CALL OResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
    aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// -------------------------------------------------------------------------
OResultSet::OResultSet(OStatement_Base* pStmt, connectivity::OSQLParseTreeIterator&   _aSQLIterator )
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_aStatement((OWeakObject*)pStmt)
    ,m_nRowPos(0)
    ,m_bLastRecord(sal_False)
    ,m_bEOF(sal_False)
    ,m_xMetaData(NULL)
    ,m_bInserting(sal_False)
    ,m_nLastColumnPos(0)
    ,m_nIsAlwaysFalseQuery(sal_False)
    ,m_nParamIndex(0)
    ,m_nRowCountResult(-1)
    ,m_nTextEncoding(pStmt->getOwnConnection()->getTextEncoding())
    ,m_aQuery(pStmt->getOwnConnection()->getColumnAlias().getAliasMap())
    ,m_pKeySet(NULL)
    ,m_pStatement(pStmt)
    ,m_pParseTree(_aSQLIterator.getParseTree())
    ,m_aSQLIterator(_aSQLIterator)
    ,m_nIsBookmarkable(sal_False)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    ,m_nFetchDirection(FetchDirection::FORWARD)
{
    m_aQuery.setMaxNrOfReturns(pStmt->getOwnConnection()->getMaxResultRecords());
}
// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
}

// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_aStatement    = NULL;
    m_xMetaData     = NULL;
    m_pParseTree    = NULL;
    m_xColumns      = NULL;
    m_xParamColumns = NULL;
    m_pKeySet       = NULL;
    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL OResultSet::getTypes(  ) throw( RuntimeException)
{
    OTypeCollection aTypes( ::getCppuType( (const  Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName

    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    m_bWasNull = sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int8 nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------

Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    Date nRet;
    return nRet;
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    double nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    float nVal(0);
    return nVal;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRet=0;
    return nRet;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nValue = 0;
    OSL_TRACE("In/Out: OResultSet::getRow" );
    return nValue;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_Int64();
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData( m_aSQLIterator.getSelectColumns(), m_aSQLIterator.getTables().begin()->first ,m_pTable );
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int16 nRet=0;
    return nRet;
}
// -------------------------------------------------------------------------

void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if(columnIndex <= 0 || columnIndex > (sal_Int32)m_xColumns->size())
        ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
sal_uInt32  OResultSet::currentRowCount()
{
    if ( m_nRowCountResult != -1 )
        return m_nRowCountResult;
    if ( m_pKeySet.isValid() )
        return m_pKeySet->size();
    else
        return m_aQuery.getRealRowCount();
}
// -------------------------------------------------------------------------

sal_Bool OResultSet::fetchRow(sal_uInt32 rowIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ORowSetValue value;
    ::rtl::OUString sTableColumnName;
    Reference<XPropertySet> xTableColumn;
    Reference<XIndexAccess> xNames(m_xTableColumns,UNO_QUERY);
    const ::rtl::OUString sPropertyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);

    // Check whether we've already fetched the row...
    if ( !(*m_aRow)[0].isNull() && (sal_Int32)(*m_aRow)[0] == (sal_Int32)rowIndex )
        return sal_True;

    if ( validRow( rowIndex ) == sal_False )
        return sal_False;

#ifndef DARREN_WORK
    (*m_aRow)[0] = (sal_Int32)rowIndex;
    for( sal_Int32 i = 1; i < xNames->getCount(); i++ ) {
#else  /* DARREN_WORK */
    for( sal_Int32 i = 1; i < xNames->getCount(); i++ ) {
#endif /* DARREN_WORK */
        if ( (*m_aRow)[i].isBound() ) {
            OSL_TRACE("Row[%d] is Bound", i );
            xNames->getByIndex(i-1) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::fetchRow: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sPropertyName) >>= sTableColumnName;
            else
                sTableColumnName = ::rtl::OUString();

            OSL_TRACE("getValue : columnName is : %s ", OUtoCStr( sTableColumnName ) );
            //
            // Everything in the addressbook is a string!
            //
            m_aQuery.getRowValue( value, rowIndex, sTableColumnName, DataType::VARCHAR );
            OSL_TRACE("getValue : %s returned", OUtoCStr( value ) );
            (*m_aRow)[i] =  value;
        }
        else {
            OSL_TRACE("Row[%d] is NOT Bound", i );
        }
    }
    return sal_True;
}
// -------------------------------------------------------------------------

const ORowSetValue& OResultSet::getValue(sal_uInt32 rowIndex, sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if ( fetchRow( rowIndex ) == sal_False ) {
        OSL_ASSERT("fetchRow() returned False" );
        m_bWasNull = sal_True;
        return *ODatabaseMetaDataResultSet::getEmptyValue();
    }

    m_bWasNull = (*m_aRow)[columnIndex].isNull();
    return (*m_aRow)[columnIndex];

#ifdef DARREN_WORK
    ORowSetValue value;
    ::rtl::OUString sTableColumnName;
    Reference<XPropertySet> xTableColumn;
    Reference<XIndexAccess> xNames(m_xTableColumns,UNO_QUERY);
    OSL_ENSURE( columnIndex < xNames->getCount(), "Invalid Column Index");

    const ::rtl::OUString sPropertyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);

    OSL_TRACE("In/Out: OResultSet::getValue" );
    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");

    xNames->getByIndex(columnIndex) >>= xTableColumn;
    OSL_ENSURE(xTableColumn.is(), "OResultSet::getValue: invalid table column!");
    if (xTableColumn.is())
        xTableColumn->getPropertyValue(sPropertyName) >>= sTableColumnName;
    else
        sTableColumnName = ::rtl::OUString();

    //
    // Everything in the addressbook is a string!
    //
    m_aQuery.getRowValue( value, rowIndex, sTableColumnName, DataType::VARCHAR );
    OSL_TRACE("getValue : %s returned", OUtoCStr( value ) );
    m_bWasNull = value.isNull();
    return value;
#endif /* DARREN_WORK */
}
// -------------------------------------------------------------------------


::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::getString" );
    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");
    OSL_ENSURE(columnIndex <= (sal_Int32)m_xColumns->size(), "Trying to access invalid columns number");
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex( columnIndex );

    // If this query was sorted then we should have a valid KeySet, so use it
    if ( m_pKeySet.isValid() ) {
        OSL_TRACE( "%u <= %u", m_nRowPos, m_pKeySet->size());
        OSL_ENSURE( m_nRowPos > 0 && m_nRowPos <= m_pKeySet->size(), "Invalid Row Position");
        return getValue( (*m_pKeySet)[m_nRowPos-1], mapColumn( columnIndex ) );
    }
    else
        return getValue( m_nRowPos, mapColumn( columnIndex ) );

#ifdef DARREN_WORK
    const ::rtl::OUString sPropertyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);

    ORowSetValue value;
    ::rtl::OUString columnName;
    ((m_xColumns.getBody())[columnIndex-1])->getPropertyValue( sPropertyName ) >>= columnName;
    // If this query was sorted then we should have a valid KeySet, so use it
    if ( m_pKeySet.isValid() ) {
        OSL_TRACE( "%u <= %u", m_nRowPos, m_pKeySet->size());
        OSL_ENSURE( m_nRowPos > 0 && m_nRowPos <= m_pKeySet->size(), "Invalid Row Position");
        m_aQuery.getRowValue( value, (*m_pKeySet)[m_nRowPos-1], columnName, DataType::VARCHAR );
    }
    else
        m_aQuery.getRowValue( value, m_nRowPos, columnName, DataType::VARCHAR );
    OSL_TRACE("getString : %s returned", OUtoCStr( value ) );
    m_bWasNull = sal_False;
    return value;
#endif /* DARREN_WORK */
}
// -------------------------------------------------------------------------

Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );

    Time nRet;
    return nRet;
}
// -------------------------------------------------------------------------


DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    ::osl::MutexGuard aGuard( m_aMutex );

    DateTime nRet;
    return nRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    // here you have to implement your movements
    // return true means there is no data
    OSL_TRACE("In/Out: OResultSet::isBeforeFirst" );
    return( m_nRowPos < 1 );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::isAfterLast" );
    if ( m_nRowPos > currentRowCount() && m_aQuery.queryComplete()) {
        return sal_True;
    }

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::isFirst" );
    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::isLast" );
    if ( m_nRowPos == currentRowCount() && m_aQuery.queryComplete()) {
        return sal_True;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // move before the first row so that isBeforeFirst returns false
    OSL_TRACE("In/Out: OResultSet::beforeFirst" );
    if ( first() )
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_TRACE("In/Out: OResultSet::afterLast" );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(last())
        next();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    OSL_TRACE("In/Out: OResultSet::close" );
    // dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::first" );
    return seekRow( FIRST_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::last" );
    return seekRow( LAST_POS );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::absolute" );
    return seekRow( ABSOLUTE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::relative" );
    return seekRow( RELATIVE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::previous" );
    return seekRow( PRIOR_POS );
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::getStatement" );
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::rowDeleted" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::rowInserted" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::rowUpdated" );
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::next" );
    return seekRow( NEXT_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::wasNull" );
    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OSL_TRACE("In/Out: OResultSet::cancel" );

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::clearWarnings" );
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::getWarnings" );
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // you only have to implement this if you want to insert new rows
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    // only when you allow updates
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    // only when you allow insert's
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_TRACE("In/Out: OResultSet::moveToCurrentRow" );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const Date& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const Time& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const DateTime& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    OSL_TRACE("In/Out: OResultSet::refreshRow" );

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // if you don't want to support bookmark you must remove the XRowLocate interface

    OSL_TRACE("In/Out: OResultSet::getBookmark" );
     return Any();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::moveToBookmark" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    OSL_TRACE("In/Out: OResultSet::moveRelativeToBookmark" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::compareBookmarks( const  Any& first, const  Any& second ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OSL_TRACE("In/Out: OResultSet::compareBookmarks" );
    return CompareBookmark::NOT_EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::hasOrderedBookmarks" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    throw SQLException();
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const  Sequence<  Any >& rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
#ifdef DARREN_WORK
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            ;
#else  /* DARREN_WORK */
        case PROPERTY_ID_ISBOOKMARKABLE:
            // rValue = bool2any(isBookmarkable());
            rValue = bool2any(m_nIsBookmarkable);
            break;
        case PROPERTY_ID_CURSORNAME:
            // rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= (sal_Int32)m_nResultSetConcurrency;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= m_nResultSetType;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= m_nFetchDirection;
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= m_nFetchSize;
            break;
#endif /* DARREN_WORK */
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw(::com::sun::star::uno::RuntimeException)
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void OResultSet::initializeRow(OValueRow& _rRow,sal_Int32 _nColumnCount)
{
    if(!_rRow.isValid())
    {
        _rRow   = new OValueVector(_nColumnCount);
        (*_rRow)[0].setBound(sal_True);
        ::std::for_each(_rRow->begin()+1,_rRow->end(),TSetBound(sal_False));
    }
}

// -----------------------------------------------------------------------------

// -------------------------------------------------------------------------
void OResultSet::fillColumns()
{
#ifdef DARREN_WORK

    OSL_TRACE( "IN OResultSet::fillColumns()\n" );


    ::rtl::OUString     aTypeName;

    m_aAttributeStrings.clear();
    m_aAttributeStrings.push_back( ::rtl::OUString::createFromAscii("card:URI") );

    Reference< XDatabaseMetaData > xMtd = m_pConnection->getMetaData();
    ::comphelper::UStringMixEqual aCase(xMtd->storesMixedCaseQuotedIdentifiers());

    Reference< XResultSet > xRes = xMtd->getColumns( Any(),
                                                       ::rtl::OUString(),
                                                       m_Name,
                                                       ::rtl::OUString::createFromAscii("%"));
    Reference<XRow> xRow(xRes,UNO_QUERY);

    // clear the whole vector stuff
    if(!m_aColumns.isValid())
        m_aColumns = new OSQLColumns();
    else
        m_aColumns->clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();
    // reserve some space to speed up this think
    m_aAttributeStrings.reserve(m_nNR_OF_FIELDS+1);
    m_aColumns->reserve(m_nNR_OF_FIELDS);
    m_aTypes.reserve(m_nNR_OF_FIELDS);
    m_aPrecisions.reserve(m_nNR_OF_FIELDS);
    m_aScales.reserve(m_nNR_OF_FIELDS);

    for (sal_Int32 i = 1; i <= m_nNR_OF_FIELDS; i++)
    {
        ::rtl::OUString aColumnName;
        sal_Int32 eType         = DataType::OTHER;
        sal_Bool bCurrency      = sal_False;
        sal_Int32 nPrecision    = 0;   //! ...
        sal_Int32 nDecimals     = 0;    //! ...
        sal_Int32 nNullable     = ColumnValue::NULLABLE;

        if(xRes.is() && xRes->next())
        {
            aColumnName = xRow->getString(4);
            eType       = xRow->getShort(5);
            aTypeName   = xRow->getString(6);
            nPrecision  = xRow->getInt(7);
            nDecimals   = xRow->getInt(9);
            nNullable   = xRow->getInt(11);
        }
        else
        {
            OSL_TRACE(0,"OResultSet::fillColumns: getColumns doesn't return a resultset!");
        }
        //  getColumnInfo( i, aColumnName, eType );

        OSL_TRACE( "inserting string %s\n", OUtoCStr( aColumnName ) );
        m_aAttributeStrings.push_back( aColumnName );

        // check if the column name already exists
        ::rtl::OUString aAlias = aColumnName;
        OSQLColumns::const_iterator aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->end())
        {
            (aAlias = aColumnName) += ::rtl::OUString::valueOf((sal_Int32)++nExprCnt);
            aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn( aAlias, aTypeName, ::rtl::OUString(),
                                                nNullable, nPrecision, nDecimals,
                                                eType, sal_False, sal_False, bCurrency,
                                                aCase.isCaseSensitive() );
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nDecimals);
    }


    OSL_TRACE( "\tOUT OResultSet::fillColumns()\n" );

#endif /* DARREN_WORK */
}

// -------------------------------------------------------------------------
#ifdef DARREN_WORK
#if defined DEBUG || defined DBG_UTIL
extern void printParseTree( const OSQLParseNode*  parseTree, rtl::OString tab );
#endif
#endif /* DARREN_WORK */
// -------------------------------------------------------------------------
void OResultSet::parseParameter( const OSQLParseNode* pNode, rtl::OUString& rMatchString )
{
    OSL_ENSURE(pNode->count() > 0,"Error parsing parameter in Parse Tree");
    OSQLParseNode *pMark = pNode->getChild(0);

    // Initialize to empty string
    rMatchString = ::rtl::OUString::createFromAscii("");

    rtl::OUString aParameterName;
    if (SQL_ISPUNCTUATION(pMark,"?")) {
        aParameterName = ::rtl::OUString::createFromAscii("?");
    }
    else if (SQL_ISPUNCTUATION(pMark,":")) {
        aParameterName = pNode->getChild(1)->getTokenValue();
    }
    // XXX - Now we know name, what's value????
    m_nParamIndex ++;
    OSL_TRACE("Parameter name [%d]: %s\n", m_nParamIndex,
              OUtoCStr(aParameterName) );

    if ( m_aParameterRow.isValid() ) {
        OSL_ENSURE( m_nParamIndex < (sal_Int32)m_aParameterRow->size() + 1, "More parameters than values found" );
        rMatchString = (*m_aParameterRow)[(sal_uInt16)m_nParamIndex];
        OSL_TRACE("Prop Value       : %s\n", OUtoCStr( rMatchString ) );
    }
    else {
        OSL_TRACE("Prop Value       : Invalid ParameterRow!\n" );
    }
}

void OResultSet::analyseWhereClause( const OSQLParseNode*    parseTree,
                ::std::vector< ::rtl::OUString >       &matchItems,
                ::std::vector< MQuery::eSqlOppr > &matchOper,
                ::std::vector< ::rtl::OUString >       &matchValues,
                connectivity::OSQLParseTreeIterator& m_aSQLIterator)
{
    ::rtl::OUString         columnName;
    MQuery::eSqlOppr   op;
    ::rtl::OUString         matchString;

    if ( parseTree == NULL )
        return;

    if ( m_aSQLIterator.getParseTree() != NULL ) {
        OSL_TRACE("FULL QUERY IS : \n" );
#ifdef DARREN_WORK
#if defined DEBUG || defined DBG_UTIL
        printParseTree( m_aSQLIterator.getParseTree(), "XX " );
#endif
#endif /* DARREN_WORK */
        OSL_TRACE("FULL QUERY IS : \n" );

        ::vos::ORef<OSQLColumns> xColumns = m_aSQLIterator.getParameters();
        if(xColumns.isValid())
        {
            ::rtl::OUString aTabName,aColName,aParameterName,aParameterValue;
            OSQLColumns::iterator aIter = xColumns->begin();
            sal_Int32 i = 1;
            for(;aIter != xColumns->end();++aIter)
            {
                (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
                OSL_TRACE("Prop Column Name : %s\n", OUtoCStr( aColName ) );
                if ( m_aParameterRow.isValid() ) {
                    aParameterValue = (*m_aParameterRow)[(sal_uInt16)i];
                    OSL_TRACE("Prop Value       : %s\n", OUtoCStr( aParameterValue ) );
                }
                else {
                    OSL_TRACE("Prop Value       : Invalid ParameterRow!\n" );
                }
                i++;
            }
        }

    }
#ifdef DARREN_WORK
#if defined DEBUG || defined DBG_UTIL
    printParseTree( parseTree, "XX " );
#endif
#endif /* DARREN_WORK */

    if ( SQL_ISRULE(parseTree,where_clause) )
    {
        OSL_TRACE("analyseSQL : Got WHERE clause\n");
        // Reset Parameter Counter
        resetParameters();
        analyseWhereClause( parseTree->getChild( 1 ), matchItems, matchOper, matchValues, m_aSQLIterator);
    }
    else if ( SQL_ISRULE(parseTree,where_clause) )
    {
        OSL_TRACE("analyseSQL : Got WHERE clause\n");
        analyseWhereClause( parseTree->getChild( 1 ), matchItems, matchOper, matchValues, m_aSQLIterator);
    }
    else if ( parseTree->count() == 3 &&                         // Handle ()'s
        SQL_ISPUNCTUATION(parseTree->getChild(0),"(") &&
        SQL_ISPUNCTUATION(parseTree->getChild(2),")"))
    {

        OSL_TRACE("analyseSQL : Got Punctuation ()\n");
        analyseWhereClause( parseTree->getChild( 1 ), matchItems, matchOper, matchValues,m_aSQLIterator );
    }
    else if ((SQL_ISRULE(parseTree,search_condition) || (SQL_ISRULE(parseTree,boolean_term)))
             && parseTree->count() == 3)                   // Handle AND/OR
    {

        OSL_TRACE("analyseSQL : Got AND/OR clause\n");

        // TODO - Need to take care or AND, for now match is always OR
        analyseWhereClause( parseTree->getChild( 0 ), matchItems, matchOper, matchValues, m_aSQLIterator );
        analyseWhereClause( parseTree->getChild( 2 ), matchItems, matchOper, matchValues, m_aSQLIterator );
    }
    else if (SQL_ISRULE(parseTree,comparison_predicate))
    {
        OSL_ENSURE(parseTree->count() == 3, "Error parsing COMPARE predicate");
        if (!(SQL_ISRULE(parseTree->getChild(0),column_ref) ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_STRING ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_INTNUM ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_APPROXNUM ||
          SQL_ISTOKEN(parseTree->getChild(2),TRUE) ||
          SQL_ISTOKEN(parseTree->getChild(2),FALSE) ||
          SQL_ISRULE(parseTree->getChild(2),parameter) ||
          // odbc date
          (SQL_ISRULE(parseTree->getChild(2),set_fct_spec) && SQL_ISPUNCTUATION(parseTree->getChild(2)->getChild(0),"{"))))
        {
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("Statement too complex"),NULL);
        }

        OSQLParseNode *pPrec = parseTree->getChild(1);
        if (pPrec->getNodeType() == SQL_NODE_EQUAL)
            op = MQuery::matchIs;
        else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            op = MQuery::matchIsNot;

        ::rtl::OUString sTableRange;
        if(SQL_ISRULE(parseTree->getChild(0),column_ref))
            m_aSQLIterator.getColumnRange(parseTree->getChild(0),columnName,sTableRange);
        else if(parseTree->getChild(0)->isToken())
            columnName = parseTree->getChild(0)->getTokenValue();

        if ( SQL_ISRULE(parseTree->getChild(2),parameter) ) {
            parseParameter( parseTree->getChild(2), matchString );
        }
        else {
            matchString = parseTree->getChild(2)->getTokenValue();
        }

        if ( columnName.compareToAscii("0") ==0 && op == MQuery::matchIs &&
             matchString.compareToAscii("1") == 0 ) {
            OSL_TRACE("Query always evaluates to FALSE");
            m_nIsAlwaysFalseQuery = sal_True;
        }
        matchItems.push_back( columnName );
        matchOper.push_back( op );
        matchValues.push_back( matchString );
    }
    else if (SQL_ISRULE(parseTree,like_predicate))
    {
        OSL_ENSURE(parseTree->count() >= 4, "Error parsing LIKE predicate");

        OSL_TRACE("analyseSQL : Got LIKE rule\n");

        if ( !(SQL_ISRULE(parseTree->getChild(0), column_ref)) )
        {
            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Invalid Statement - Not a Column"),NULL);
        }


        OSQLParseNode *pColumn;
        OSQLParseNode *pAtom;
        OSQLParseNode *pOptEscape;
        pColumn     = parseTree->getChild(0);                        // Match Item
        pAtom       = parseTree->getChild(parseTree->count()-2);     // Match String
        pOptEscape  = parseTree->getChild(parseTree->count()-1);     // Opt Escape Rule

        if (!(pAtom->getNodeType() == SQL_NODE_STRING ||
              pAtom->getNodeType() == SQL_NODE_NAME ||
              SQL_ISRULE(pAtom,parameter) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQL_NODE_NAME ) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQL_NODE_STRING )
              ) )
        {
#ifdef DARREN_WORK
#if defined DEBUG || defined DBG_UTIL
            printParseTree( pAtom, "AnalyseSQL " );
#endif
#endif /* DARREN_WORK */
            OSL_TRACE("analyseSQL : pAtom->count() = %d\n", pAtom->count() );
#ifdef DBG_UTIL
                for ( sal_uInt32 i = 0; i < pAtom->count(); ++i )
                    OSL_TRACE("analyseSQL : pAtom (%d) : %d, %d  = %s\n", i,
                            (sal_Int32)pAtom->getRuleID(),
                            pAtom->getChild(i)->getNodeType(),
                            OUtoCStr(pAtom->getChild(i)->getTokenValue()));
#endif


            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Invalid Statement - Not a String"),NULL);
        }

        const sal_Unicode WILDCARD = '%';
        const sal_Unicode ALT_WILDCARD = '*';
        const sal_Unicode MATCHCHAR = '_';

        ::rtl::OUString sTableRange;
        if(SQL_ISRULE(pColumn,column_ref))
            m_aSQLIterator.getColumnRange(pColumn,columnName,sTableRange);

        OSL_TRACE("ColumnName = %s\n", OUtoCStr( columnName ) );

        if ( SQL_ISRULE(pAtom,parameter) ) {
            parseParameter( pAtom, matchString );
            // Replace all '*' with '%' : UI Usually does this but not with
            // Parameters for some reason.
            matchString = matchString.replace( ALT_WILDCARD, WILDCARD );
        }
        else
        {
            matchString = pAtom->getTokenValue();
        }

        // Determine where '%' character is...

        if ( matchString.equals( ::rtl::OUString::valueOf( WILDCARD ) ) )
        {
            // String containing only a '%' and nothing else
            op = MQuery::matchExists;
            // Will be ignored for Exists case, but clear anyway.
            matchString = ::rtl::OUString::createFromAscii("");
        }
        else if ( matchString.indexOf ( WILDCARD ) == -1 &&
             matchString.indexOf ( MATCHCHAR ) == -1 )
        {
            // Simple string , eg. "to match"
            if ( parseTree->count() == 5 )
                op = MQuery::matchDoesNotContain;
            else
                op = MQuery::matchContains;
        }
        else if (  matchString.indexOf ( WILDCARD ) == 0
                   && matchString.lastIndexOf ( WILDCARD ) == matchString.getLength() -1
                   && matchString.indexOf ( WILDCARD, 1 ) == matchString.lastIndexOf ( WILDCARD )
                   && matchString.indexOf( MATCHCHAR ) == -1
                 )
        {
            // Relatively simple "%string%" - ie, contains...
            // Cut '%'  from front and rear
            matchString = matchString.replaceAt( 0, 1, rtl::OUString() );
            matchString = matchString.replaceAt( matchString.getLength() -1 , 1, rtl::OUString() );

            if ( parseTree->count() == 5 )
                op = MQuery::matchDoesNotContain;
            else
                op = MQuery::matchContains;
        }
        else if ( parseTree->count() == 5 )
        {
            // We currently can't handle a 'NOT LIKE' when there are '%' or
            // '_' dispersed throughout
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("Statement too complex"),NULL);
        }
        else
        {
            if ( (matchString.indexOf ( WILDCARD ) == matchString.lastIndexOf ( WILDCARD ))
                  && matchString.indexOf( MATCHCHAR ) == -1
                )
            {
                // One occurance of '%' - no '_' matches...
                if ( matchString.indexOf ( WILDCARD ) == 0 )
                {
                    op = MQuery::matchEndsWith;
                    matchString = matchString.replaceAt( 0, 1, rtl::OUString());
                }
                else if ( matchString.indexOf ( WILDCARD ) == matchString.getLength() -1 )
                {
                    op = MQuery::matchBeginsWith;
                    matchString = matchString.replaceAt( matchString.getLength() -1 , 1, rtl::OUString() );
                }
                else
                {
                    sal_Int32 pos = matchString.indexOf ( WILDCARD );
                    matchString = matchString.replaceAt( pos, 1,::rtl::OUString::createFromAscii(".*") );
                    op = MQuery::matchRegExp;
                }

            }
            else
            {
                // Most Complex, need to use an RE
                sal_Int32 pos = matchString.indexOf ( WILDCARD );
                while ( (pos = matchString.indexOf ( WILDCARD )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, ::rtl::OUString::createFromAscii(".*") );
                }

                pos = matchString.indexOf ( MATCHCHAR );
                while ( (pos = matchString.indexOf( MATCHCHAR )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, ::rtl::OUString::createFromAscii(".") );
                }

                op = MQuery::matchRegExp;
            }
        }

        matchItems.push_back( columnName );
        matchOper.push_back( op );
        matchValues.push_back( matchString );
    }
    else if (SQL_ISRULE(parseTree,test_for_null))
    {
        OSL_ENSURE(parseTree->count() >= 3,"Error in ParseTree");
        OSL_ENSURE(SQL_ISTOKEN(parseTree->getChild(1),IS),"Error in ParseTree");

        if (!SQL_ISRULE(parseTree->getChild(0),column_ref))
        {
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement - Not a Column"),NULL);
        }

        if (SQL_ISTOKEN(parseTree->getChild(2),NOT))
        {
            ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("Statement too complex"),NULL);
        }
        else
                op = MQuery::matchExists;

        ::rtl::OUString sTableRange;
        m_aSQLIterator.getColumnRange(parseTree->getChild(0),columnName,sTableRange);

        matchItems.push_back( columnName );
        matchOper.push_back( op );
        matchValues.push_back( rtl::OUString() );
    }
    else
    {
        OSL_TRACE( "Unexpected statement!!!" );

        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement - Unexpected"),NULL);
    }
}


// -----------------------------------------------------------------------------

void OResultSet::fillRowData()
    throw( ::com::sun::star::sdbc::SQLException )
{
    OSL_TRACE( "IN OResultSet::fillRowData()\n" );

    OSL_ENSURE( m_pStatement, "Require a statement" );

    ::std::vector< ::rtl::OUString >        matchItems;
    ::std::vector< MQuery::eSqlOppr >  matchOper;
    ::std::vector< ::rtl::OUString >        matchValues;

    OConnection* xConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
    m_xColumns = m_aSQLIterator.getSelectColumns();

    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");

    OSQLColumns::const_iterator aIter = m_xColumns->begin();
    const ::rtl::OUString sProprtyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    ::rtl::OUString sName;
    m_aAttributeStrings.clear();
    for (sal_Int32 i = 1; aIter != m_xColumns->end();++aIter, i++)
    {
        (*aIter)->getPropertyValue(sProprtyName) >>= sName;
        OSL_TRACE("Query Columns : (%d) %s\n", i, OUtoCStr(sName) );
        m_aAttributeStrings.push_back( sName );
    }


    // Generate Match Conditions for Query
    const OSQLParseNode*  pParseTree = m_aSQLIterator.getWhereTree();

    // const OSQLParseNode*  pParseTree = NULL;

    m_nIsAlwaysFalseQuery = sal_False;
    if ( pParseTree != NULL )
    {
        // Extract required info

        OSL_TRACE("\tHave a Where Clause\n");

        analyseWhereClause( pParseTree, matchItems, matchOper, matchValues ,m_aSQLIterator);
    }
    else
    {
        OSL_TRACE("\tDon't have a Where Clause\n");

        // LDAP does not allow a query without restriction, so we add a dummy
        // for FirstName
        // For other types we stick to the old behaviour of using
        // card:nsIAbCard.
        OSL_ENSURE(m_pStatement, "Cannot determine Parent Statement");
        if (xConnection->isLDAP())
            matchItems.push_back( ::rtl::OUString::createFromAscii("FirstName") );
        else
            matchItems.push_back( ::rtl::OUString::createFromAscii("card:nsIAbCard") );

        matchOper.push_back( MQuery::matchExists );
        matchValues.push_back( ::rtl::OUString()  );

//      matchItems.push_back( ::rtl::OUString::createFromAscii("DisplayName") );
//      matchOper.push_back( MQuery::matchBeginsWith );
//      matchValues.push_back( ::rtl::OUString::createFromAscii("Darren")  );
//      matchItems.push_back( ::rtl::OUString::createFromAscii("PrimaryEmail") );
//      matchOper.push_back( MQuery::matchBeginsWith );
//      matchValues.push_back( ::rtl::OUString::createFromAscii("Darren")  );
    }

    // If the query is a 0=1 then set Row count to 0 and return
    if ( m_nIsAlwaysFalseQuery ) {
        m_nRowCountResult = 0;
        return;
    }

    m_aQuery.setMatchItems( matchItems );
    m_aQuery.setSqlOppr( matchOper );
    m_aQuery.setMatchValues( matchValues );

    // We need a unique id for caching mechanism so should fetch card:URI
    m_aQuery.setAttributes( m_aAttributeStrings );

    m_aQuery.setAddressbook( m_pTable->getName() );

    sal_Int32 rv = m_aQuery.executeQuery(xConnection->isOutlookExpress());
    if ( rv == -1 ) {
        ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Error querying addressbook"),NULL);
    }

    OSL_TRACE( "executeQuery returned %d\n", rv );

    OSL_TRACE( "\tOUT OResultSet::fillRowData()\n" );
}

// -----------------------------------------------------------------------------
static sal_Bool matchRow( OValueRow& row1, OValueRow& row2 ) {
    OValueVector::iterator row1Iter = row1->begin();
    OValueVector::iterator row2Iter = row2->begin();
    for ( ++row1Iter,++row2Iter; // the first column is the bookmark column
          row1Iter != row1->end(); ++row1Iter,++row2Iter) {
        if ( row1Iter->isBound()) {
            // Compare values, if at anytime there's a mismatch return false
            if ( !( (*row1Iter) == (*row2Iter) ) )
                return sal_False;
        }
    }

    // If we get to here the rows match
    return sal_True;
}
// -----------------------------------------------------------------------------

void SAL_CALL OResultSet::executeQuery() throw( ::com::sun::star::sdbc::SQLException,
                                                ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OSL_ENSURE( m_pTable, "Need a Table object");
    if(!m_pTable)
    {
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if ((xTabs.begin() == xTabs.end()) || !xTabs.begin()->second.is())
            ::dbtools::throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid."),
                                        static_cast<XWeak*>(this),
                                        makeAny(m_aSQLIterator.getWarning())
                                    );

        m_pTable = static_cast< OTable* > ((xTabs.begin()->second).get());

    }

    m_nRowPos = 0;
    m_nRowCountResult = -1;

    fillRowData();

    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");

    sal_Int32 nColumnCount = m_xColumns->size();

    // initializeRow(m_aRow,nColumnCount);
    // initializeRow(m_aEvaluateRow,nColumnCount);

    sal_uInt32 nLoopCount = 0;
    switch(m_aSQLIterator.getStatementType())
    {
        case SQL_STATEMENT_SELECT:
        {
            if(m_nIsAlwaysFalseQuery) {
                break;
            }
            else if(isCount())
            {
                m_nRowCountResult = 0;
                m_aQuery.waitForQueryComplete();
                m_nRowCountResult = m_aQuery.getRowCount();
            }
            else
            {
                sal_Bool bDistinct = sal_False;
                sal_Bool bWasSorted = sal_False;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT
)
                {
                    if(!IsSorted())
                    {
                        m_aOrderbyColumnNumber.push_back(m_aColMapping[1]);
                        m_aOrderbyAscending.push_back(SQL_DESC);
                    }
                    else
                        bWasSorted = sal_True;
                    bDistinct = sal_True;
                }

                OSortIndex::TKeyTypeVector eKeyType(m_aOrderbyColumnNumber.size());
                OValueVector::iterator aRowIter = m_aRow->begin()+1;
                ::std::vector<sal_Int32>::iterator aOrderByIter = m_aOrderbyColumnNumber.begin();
                for (::std::vector<sal_Int16>::size_type i=0;aOrderByIter != m_aOrderbyColumnNumber.end(); ++aOrderByIter,++i)
                {
                    OSL_ENSURE((sal_Int32)m_aRow->size() > *aOrderByIter,"Invalid Index");
                    switch ((m_aRow->begin()+*aOrderByIter)->getTypeKind())
                    {
                    case DataType::CHAR:
                        case DataType::VARCHAR:
                            eKeyType[i] = SQL_ORDERBYKEY_STRING;
                            break;

                        case DataType::OTHER:
                        case DataType::TINYINT:
                        case DataType::SMALLINT:
                        case DataType::INTEGER:
                        case DataType::DECIMAL:
                        case DataType::NUMERIC:
                        case DataType::REAL:
                        case DataType::DOUBLE:
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                        case DataType::BIT:
                            eKeyType[i] = SQL_ORDERBYKEY_DOUBLE;
                            break;

                    // Andere Typen sind nicht implementiert (und damit immer
                    // FALSE)
                        default:
                            eKeyType[i] = SQL_ORDERBYKEY_NONE;
                            OSL_ASSERT("MResultSet::executeQuery: Order By Data Type not implemented");
                            break;
                    }
                }

                if (IsSorted())
                {
                    // Implement Sorting

                    // So that we can sort we need to wait until the executed
                    // query to the mozilla addressbooks has returned all
                    // values.

                    OSL_TRACE("Query is to be sorted");
                    if( ! m_aQuery.queryComplete() )
                        m_aQuery.waitForQueryComplete();

                    OSL_ENSURE( m_aQuery.queryComplete(), "Query not complete!!");

                    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

                    OSL_TRACE("OrderbyColumnNumber->size() = %d",m_aOrderbyColumnNumber.size());
                    for ( sal_uInt32 i = 0; i < m_aColMapping.size(); i++ )
                        OSL_TRACE("Mapped: %d -> %d", i, m_aColMapping[i] );
                    for ( sal_Int32 nRow = 1; nRow <= m_aQuery.getRowCount(); nRow++ ) {

                        OKeyValue* pKeyValue = new OKeyValue(nRow);

                        ::std::vector<sal_Int32>::iterator aIter = m_aOrderbyColumnNumber.begin();
                        for (;aIter != m_aOrderbyColumnNumber.end(); ++aIter)
                        {
                            ORowSetValue value = getValue(nRow, *aIter);

                            OSL_TRACE( "Adding Value: (%d,%d) : %s", nRow, *aIter,
                                       OUtoCStr( value ));

                            pKeyValue->pushKey(new ORowSetValueDecorator(value));
                        }

                        m_pSortIndex->AddKeyValue( pKeyValue );
                    }

                    m_pKeySet = m_pSortIndex->CreateKeySet();
                    for( i = 0; i < m_pKeySet->size(); i++ )
                        OSL_TRACE("Sorted: %d -> %d", i, (*m_pKeySet)[i] );

                    m_pSortIndex = NULL;
                    beforeFirst(); // Go back to start
                }

                // Handle the DISTINCT case
                if ( bDistinct && m_pKeySet.isValid() ) {
                    OValueRow aSearchRow = new OValueVector( m_aRow->size() );

                    for( i = 0; i < m_pKeySet->size(); i++ ) {
                        fetchRow( (*m_pKeySet)[i] );        // Fills m_aRow
                        if ( matchRow( m_aRow, aSearchRow ) ) {
                            (*m_pKeySet)[i] = 0;   // Marker for later to be removed
                        }
                        else {
                            // They don't match, so it's not a duplicate.
                            // Use the current Row as the next one to match against
                            *aSearchRow = *m_aRow;
                        }
                    }
                    // Now remove any keys marked with a 0
                    m_pKeySet->erase(::std::remove_if(m_pKeySet->begin(),m_pKeySet->end()
                                    ,::std::bind2nd(::std::equal_to<sal_Int32>(),0))
                                    ,m_pKeySet->end());

                }
            }
        }   break;

        case SQL_STATEMENT_SELECT_COUNT:
        case SQL_STATEMENT_UPDATE:
        case SQL_STATEMENT_DELETE:
        case SQL_STATEMENT_INSERT:
            break;
    }
}

// -----------------------------------------------------------------------------

void OResultSet::setBoundedColumns(const OValueRow& _rRow,
                                   const ::vos::ORef<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   sal_Bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   ::std::vector<sal_Int32>& _rColMapping)
{
    ::comphelper::UStringMixEqual aCase(_xMetaData->storesMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    ::rtl::OUString sTableColumnName, sSelectColumnRealName;

    const ::rtl::OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const ::rtl::OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);
    const ::rtl::OUString sType     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE);

    OValueVector::iterator aRowIter = _rRow->begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
            aRowIter != _rRow->end();
            ++i, ++aRowIter
        )
    {
        try
        {
            // get the table column and it's name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::setBoundedColumns: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sName) >>= sTableColumnName;
            else
                sTableColumnName = ::rtl::OUString();

            // look if we have such a select column
            // TODO: would like to have a O(log n) search here ...
            for (   OSQLColumns::iterator aIter = _rxColumns->begin();
                    aIter != _rxColumns->end();
                    ++aIter
                )
            {
                if((*aIter)->getPropertySetInfo()->hasPropertyByName(sRealName))
                    (*aIter)->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                else
                    (*aIter)->getPropertyValue(sName) >>= sSelectColumnRealName;

                if (aCase(sTableColumnName, sSelectColumnRealName))
                {
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = aIter - _rxColumns->begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column
                            // ...
                        OSL_TRACE("Set Col Mapping: %d -> %d", nSelectColumnPos, nTableColumnPos );
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                    }

                    aRowIter->setBound(sal_True);
                    sal_Int32 nType = DataType::OTHER;
                    if (xTableColumn.is())
                        xTableColumn->getPropertyValue(sType) >>= nType;
                    aRowIter->setTypeKind(nType);
                }
            }
        }
        catch (Exception&)
        {
            OSL_ENSURE(sal_False, "OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
}


// -----------------------------------------------------------------------------
sal_Bool OResultSet::isCount() const
{
    return (m_pParseTree &&
            m_pParseTree->count() > 2 &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist) &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)
&&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct)
            );
}

// -----------------------------------------------------------------------------
//
// Check for valid row in m_aQuery
//
sal_Bool OResultSet::validRow( sal_uInt32 nRow )
{
    sal_Int32  nNumberOfRecords = m_aQuery.getRealRowCount();

    while ( nRow > (sal_uInt32)nNumberOfRecords && !m_aQuery.queryComplete() ) {
            OSL_TRACE("validRow: waiting...");
            m_aQuery.checkRowAvailable( nRow );
            nNumberOfRecords = m_aQuery.getRealRowCount();
    }

    if (( nRow == 0 ) ||
        ( nRow > (sal_uInt32)nNumberOfRecords && m_aQuery.queryComplete()) ){
        OSL_TRACE("validRow(%u): return False", nRow);
        return sal_False;
    }

    OSL_TRACE("validRow(%u): return True", nRow);
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::seekRow( eRowPosition pos, sal_Int32 nOffset )
{
    sal_Int32  nNumberOfRecords = currentRowCount();
    sal_Int32  nCurPos = m_nRowPos;

    OSL_TRACE("seekRow: nCurPos = %d", nCurPos );
    switch( pos ) {
        case NEXT_POS:
            OSL_TRACE("seekRow: NEXT");
            nCurPos++;
            break;
        case PRIOR_POS:
            OSL_TRACE("seekRow: PRIOR");
            if ( nCurPos > 0 )
                nCurPos--;
            break;

        case FIRST_POS:
            OSL_TRACE("seekRow: FIRST");
            nCurPos = 1;
            break;

        case LAST_POS:
            OSL_TRACE("seekRow: LAST");
            nCurPos = nNumberOfRecords;
            break;
        case ABSOLUTE_POS:
            OSL_TRACE("seekRow: ABSOLUTE : %d", nOffset);
            nCurPos = nOffset;
            break;
        case RELATIVE_POS:
            OSL_TRACE("seekRow: RELATIVE : %d", nOffset);
            nCurPos += sal_uInt32( nOffset );
            break;
    }
    while ( nCurPos > nNumberOfRecords && !m_aQuery.queryComplete() ) {
            m_aQuery.checkRowAvailable( nCurPos );
            nNumberOfRecords = currentRowCount();
    }

    if ( nCurPos <= 0 ) {
        m_nRowPos = 0;
        OSL_TRACE("seekRow: return False, m_nRowPos = %u", m_nRowPos );
        return sal_False;
    }
    if ( nCurPos > nNumberOfRecords && m_aQuery.queryComplete()) {
        m_nRowPos = nNumberOfRecords + 1;
        OSL_TRACE("seekRow: return False, m_nRowPos = %u", m_nRowPos );
        return sal_False;
    }

    m_nRowPos = (sal_uInt32)nCurPos;
    OSL_TRACE("seekRow: return True, m_nRowPos = %u", m_nRowPos );
    return sal_True;
}
// -----------------------------------------------------------------------------
#ifndef DARREN_WORK
void OResultSet::setColumnMapping(const ::std::vector<sal_Int32>& _aColumnMapping)
{
    m_aColMapping = _aColumnMapping;
    for ( sal_uInt32 i = 0; i < m_aColMapping.size(); i++ )
        OSL_TRACE("Set Mapped: %d -> %d", i, m_aColMapping[i] );
}
#endif /* DARREN_WORK */

