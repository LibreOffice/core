/*************************************************************************
 *
 *  $RCSfile: MResultSet.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:29 $
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
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#include <vector>
#include <algorithm>
#ifndef CONNECTIVITY_SRESULTSET_HXX
#include "MResultSet.hxx"
#endif
#ifndef CONNECTIVITY_SRESULSETMETADATA_HXX
#include "MResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif

#ifdef _DEBUG
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* _DEBUG */
# define OUtoCStr( x ) ("dummy")
#endif /* _DEBUG */

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mozab;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
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
     Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
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
    ,m_xMetaData(NULL)
    ,m_nIsAlwaysFalseQuery(sal_False)
    ,m_nParamIndex(0)
    ,m_nRowCountResult(-1)
    ,m_aQuery(pStmt->getOwnConnection()->getColumnAlias().getAliasMap())
    ,m_pKeySet(NULL)
    ,m_pStatement(pStmt)
    ,m_pParseTree(_aSQLIterator.getParseTree())
    ,m_aSQLIterator(_aSQLIterator)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nResultSetConcurrency(ResultSetConcurrency::READ_ONLY)
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
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
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
    return 0;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------

Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return Date();
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return 0.0;
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::getRow" );
    return 0;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
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
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return 0;
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
    // Check whether we've already fetched the row...
    if ( !(*m_aRow)[0].isNull() && (sal_Int32)(*m_aRow)[0] == (sal_Int32)rowIndex )
        return sal_True;

    if ( validRow( rowIndex ) == sal_False )
        return sal_False;

    (*m_aRow)[0] = (sal_Int32)rowIndex;
    sal_Int32 nCount = m_aColumnNames.getLength();
    for( sal_Int32 i = 1; i <= nCount; i++ )
    {
        if ( (*m_aRow)[i].isBound() )
        {
            //
            // Everything in the addressbook is a string!
            //
            if ( !m_aQuery.getRowValue( (*m_aRow)[i], rowIndex, m_aColumnNames[i-1], DataType::VARCHAR )) {
                ::dbtools::throwGenericSQLException( m_aQuery.getErrorString(), NULL );
            }
        }
#ifdef _DEBUG
        else {
            OSL_TRACE("Row[%d] is NOT Bound", i );
        }
#endif
    }
    return sal_True;
}
// -------------------------------------------------------------------------

const ORowSetValue& OResultSet::getValue(sal_uInt32 rowIndex, sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if ( fetchRow( rowIndex ) == sal_False )
    {
        OSL_ASSERT("fetchRow() returned False" );
        m_bWasNull = sal_True;
        return *ODatabaseMetaDataResultSet::getEmptyValue();
    }

    m_bWasNull = (*m_aRow)[columnIndex].isNull();
    return (*m_aRow)[columnIndex];

}
// -------------------------------------------------------------------------


::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");
    OSL_ENSURE(columnIndex <= (sal_Int32)m_xColumns->size(), "Trying to access invalid columns number");
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkIndex( columnIndex );

    // If this query was sorted then we should have a valid KeySet, so use it
    if ( m_pKeySet.isValid() )
    {
        OSL_ENSURE( m_nRowPos > 0 && m_nRowPos <= m_pKeySet->size(), "Invalid Row Position");
        return getValue( (*m_pKeySet)[m_nRowPos-1], mapColumn( columnIndex ) );
    }
    else
        return getValue( m_nRowPos, mapColumn( columnIndex ) );

}
// -------------------------------------------------------------------------

Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return Time();
}
// -------------------------------------------------------------------------


DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return DateTime();
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
    return m_nRowPos > currentRowCount() && m_aQuery.queryComplete();
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
    return m_nRowPos == currentRowCount() && m_aQuery.queryComplete();
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
    OSL_TRACE("In/Out: OResultSet::first" );
    return seekRow( FIRST_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::last" );
    return seekRow( LAST_POS );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::absolute" );
    return seekRow( ABSOLUTE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::relative" );
    return seekRow( RELATIVE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
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
    OSL_TRACE("In/Out: OResultSet::rowDeleted" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::rowInserted" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::rowUpdated" );
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    return seekRow( NEXT_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

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
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::refreshRow" );
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(4);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
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
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw()
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
    OSL_TRACE("Parameter name [%d]: %s\n", m_nParamIndex,OUtoCStr(aParameterName) );

    if ( m_aParameterRow.isValid() ) {
        OSL_ENSURE( m_nParamIndex < (sal_Int32)m_aParameterRow->size() + 1, "More parameters than values found" );
        rMatchString = (*m_aParameterRow)[(sal_uInt16)m_nParamIndex];
#ifdef _DEBUG
        OSL_TRACE("Prop Value       : %s\n", OUtoCStr( rMatchString ) );
#endif
    }
#ifdef _DEBUG
    else {
        OSL_TRACE("Prop Value       : Invalid ParameterRow!\n" );
    }
#endif
}

void OResultSet::analyseWhereClause( const OSQLParseNode*                 parseTree,
                                     MQueryExpression                     &queryExpression,
                                     connectivity::OSQLParseTreeIterator& aSQLIterator)
{
    ::rtl::OUString         columnName;
    MQueryOp::cond_type     op;
    ::rtl::OUString         matchString;

    if ( parseTree == NULL )
        return;

    if ( aSQLIterator.getParseTree() != NULL ) {
        ::vos::ORef<OSQLColumns> xColumns = aSQLIterator.getParameters();
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
#ifdef _DEBUG
                    OSL_TRACE("Prop Value       : %s\n", OUtoCStr( aParameterValue ) );
#endif
                }
#ifdef _DEBUG
                else {
                    OSL_TRACE("Prop Value       : Invalid ParameterRow!\n" );
                }
#endif
                i++;
            }
        }

    }

    if ( SQL_ISRULE(parseTree,where_clause) )
    {
        OSL_TRACE("analyseSQL : Got WHERE clause\n");
        // Reset Parameter Counter
        resetParameters();
        analyseWhereClause( parseTree->getChild( 1 ), queryExpression, aSQLIterator);
    }
    else if ( parseTree->count() == 3 &&                         // Handle ()'s
        SQL_ISPUNCTUATION(parseTree->getChild(0),"(") &&
        SQL_ISPUNCTUATION(parseTree->getChild(2),")"))
    {

        OSL_TRACE("analyseSQL : Got Punctuation ()\n");
        MQueryExpression *subExpression = new MQueryExpression();
        analyseWhereClause( parseTree->getChild( 1 ), *subExpression, aSQLIterator );
        queryExpression.getExpressions().push_back( subExpression );
    }
    else if ((SQL_ISRULE(parseTree,search_condition) || (SQL_ISRULE(parseTree,boolean_term)))
             && parseTree->count() == 3)                   // Handle AND/OR
    {

        OSL_TRACE("analyseSQL : Got AND/OR clause\n");

        // TODO - Need to take care or AND, for now match is always OR
        analyseWhereClause( parseTree->getChild( 0 ), queryExpression, aSQLIterator );
        analyseWhereClause( parseTree->getChild( 2 ), queryExpression, aSQLIterator );

        if (SQL_ISTOKEN(parseTree->getChild(1),OR)) {         // OR-Operator
            queryExpression.setExpressionCondition( MQueryExpression::OR );
        }
        else if (SQL_ISTOKEN(parseTree->getChild(1),AND)) {  // AND-Operator
            queryExpression.setExpressionCondition( MQueryExpression::AND );
        }
        else {
            OSL_ASSERT("analyseSQL: Error in Parse Tree");
        }
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
            op = MQueryOp::Is;
        else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            op = MQueryOp::IsNot;

        ::rtl::OUString sTableRange;
        if(SQL_ISRULE(parseTree->getChild(0),column_ref))
            aSQLIterator.getColumnRange(parseTree->getChild(0),columnName,sTableRange);
        else if(parseTree->getChild(0)->isToken())
            columnName = parseTree->getChild(0)->getTokenValue();

        if ( SQL_ISRULE(parseTree->getChild(2),parameter) ) {
            parseParameter( parseTree->getChild(2), matchString );
        }
        else {
            matchString = parseTree->getChild(2)->getTokenValue();
        }

        if ( columnName.compareToAscii("0") ==0 && op == MQueryOp::Is &&
             matchString.compareToAscii("1") == 0 ) {
            OSL_TRACE("Query always evaluates to FALSE");
            m_nIsAlwaysFalseQuery = sal_True;
        }
        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op, matchString ));
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
            OSL_TRACE("analyseSQL : pAtom->count() = %d\n", pAtom->count() );

            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Invalid Statement - Not a String"),NULL);
        }

        const sal_Unicode WILDCARD = '%';
        const sal_Unicode ALT_WILDCARD = '*';
        const sal_Unicode MATCHCHAR = '_';

        ::rtl::OUString sTableRange;
        if(SQL_ISRULE(pColumn,column_ref))
            aSQLIterator.getColumnRange(pColumn,columnName,sTableRange);

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
            op = MQueryOp::Exists;
            // Will be ignored for Exists case, but clear anyway.
            matchString = ::rtl::OUString::createFromAscii("");
        }
        else if ( matchString.indexOf ( WILDCARD ) == -1 &&
             matchString.indexOf ( MATCHCHAR ) == -1 )
        {
            // Simple string , eg. "to match"
            if ( parseTree->count() == 5 )
                op = MQueryOp::DoesNotContain;
            else
                op = MQueryOp::Contains;
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
                op = MQueryOp::DoesNotContain;
            else
                op = MQueryOp::Contains;
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
                    op = MQueryOp::EndsWith;
                    matchString = matchString.replaceAt( 0, 1, rtl::OUString());
                }
                else if ( matchString.indexOf ( WILDCARD ) == matchString.getLength() -1 )
                {
                    op = MQueryOp::BeginsWith;
                    matchString = matchString.replaceAt( matchString.getLength() -1 , 1, rtl::OUString() );
                }
                else
                {
                    sal_Int32 pos = matchString.indexOf ( WILDCARD );
                    matchString = matchString.replaceAt( pos, 1,::rtl::OUString::createFromAscii(".*") );
                    op = MQueryOp::RegExp;
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

                op = MQueryOp::RegExp;
            }
        }

        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op, matchString ));
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
                op = MQueryOp::Exists;

        ::rtl::OUString sTableRange;
        aSQLIterator.getColumnRange(parseTree->getChild(0),columnName,sTableRange);

        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op ));
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
    OSL_ENSURE( m_pStatement, "Require a statement" );

    MQueryExpression queryExpression;

    OConnection* xConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
    m_xColumns = m_aSQLIterator.getSelectColumns();

    OSL_ENSURE(m_xColumns.isValid(), "Need the Columns!!");

    OSQLColumns::const_iterator aIter = m_xColumns->begin();
    const ::rtl::OUString sProprtyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    ::rtl::OUString sName;
    m_aAttributeStrings.clear();
    m_aAttributeStrings.reserve(m_xColumns->size());
    for (sal_Int32 i = 1; aIter != m_xColumns->end();++aIter, i++)
    {
        (*aIter)->getPropertyValue(sProprtyName) >>= sName;
#ifdef _DEBUG
        OSL_TRACE("Query Columns : (%d) %s\n", i, OUtoCStr(sName) );
#endif
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

        analyseWhereClause( pParseTree, queryExpression, m_aSQLIterator);
    }
    else
    {
        OSL_TRACE("\tDon't have a Where Clause\n");

        MQueryExpression::ExprVector    eVector;

        // LDAP does not allow a query without restriction, so we add a dummy
        // for PrimaryEmail
        // For other types we stick to the old behaviour of using
        // card:nsIAbCard.
        OSL_ENSURE(m_pStatement, "Cannot determine Parent Statement");
        ::rtl::OUString aStr;
        if (xConnection->isLDAP())
            aStr = ::rtl::OUString::createFromAscii("PrimaryEmail");
        else
            aStr = ::rtl::OUString::createFromAscii("card:nsIAbCard");
        eVector.push_back( new MQueryExpressionString(aStr, MQueryOp::Exists) );

        queryExpression.setExpressions( eVector );
    }

    // If the query is a 0=1 then set Row count to 0 and return
    if ( m_nIsAlwaysFalseQuery ) {
        m_nRowCountResult = 0;
        return;
    }

    m_aQuery.setExpression( queryExpression );

    // We need a unique id for caching mechanism so should fetch card:URI
    m_aQuery.setAttributes( m_aAttributeStrings );

    rtl::OUString aStr(  m_pTable->getName() );
    m_aQuery.setAddressbook( aStr );

    sal_Int32 rv = m_aQuery.executeQuery(xConnection->isOutlookExpress(), xConnection);
    if ( rv == -1 ) {
        ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Error querying addressbook"),NULL);
    }
#ifdef _DEBUG
    OSL_TRACE( "executeQuery returned %d\n", rv );

    OSL_TRACE( "\tOUT OResultSet::fillRowData()\n" );
#endif
}

// -----------------------------------------------------------------------------
static sal_Bool matchRow( OValueRow& row1, OValueRow& row2 )
{
    OValueVector::iterator row1Iter = row1->begin();
    OValueVector::iterator row2Iter = row2->begin();
    for ( ++row1Iter,++row2Iter; // the first column is the bookmark column
          row1Iter != row1->end(); ++row1Iter,++row2Iter)
    {
        if ( row1Iter->isBound())
        {
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
                ::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("COUNT() - Driver does not support this function."), NULL);
            }
            else
            {
                sal_Bool bDistinct = sal_False;
                sal_Bool bWasSorted = sal_False;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT)
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
                        if ( !m_aQuery.waitForQueryComplete() ) {
                            ::dbtools::throwGenericSQLException( m_aQuery.getErrorString(), NULL );
                        }

                    OSL_ENSURE( m_aQuery.queryComplete(), "Query not complete!!");

                    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

                    OSL_TRACE("OrderbyColumnNumber->size() = %d",m_aOrderbyColumnNumber.size());
#ifdef _DEBUG
                    for ( sal_uInt32 i = 0; i < m_aColMapping.size(); i++ )
                        OSL_TRACE("Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
                    for ( sal_Int32 nRow = 1; nRow <= m_aQuery.getRowCount(); nRow++ ) {

                        OKeyValue* pKeyValue = OKeyValue::createKeyValue((nRow));

                        ::std::vector<sal_Int32>::iterator aIter = m_aOrderbyColumnNumber.begin();
                        for (;aIter != m_aOrderbyColumnNumber.end(); ++aIter)
                        {
                            const ORowSetValue& value = getValue(nRow, *aIter);

                            OSL_TRACE( "Adding Value: (%d,%d) : %s", nRow, *aIter,OUtoCStr( value ));

                            pKeyValue->pushKey(new ORowSetValueDecorator(value));
                        }

                        m_pSortIndex->AddKeyValue( pKeyValue );
                    }

                    m_pKeySet = m_pSortIndex->CreateKeySet();
#ifdef _DEBUG
                    for( i = 0; i < m_pKeySet->size(); i++ )
                        OSL_TRACE("Sorted: %d -> %d", i, (*m_pKeySet)[i] );
#endif

                    m_pSortIndex = NULL;
                    beforeFirst(); // Go back to start
                }

                // Handle the DISTINCT case
                if ( bDistinct && m_pKeySet.isValid() )
                {
                    OValueRow aSearchRow = new OValueVector( m_aRow->size() );

                    for( i = 0; i < m_pKeySet->size(); i++ )
                    {
                        fetchRow( (*m_pKeySet)[i] );        // Fills m_aRow
                        if ( matchRow( m_aRow, aSearchRow ) )
                        {
                            (*m_pKeySet)[i] = 0;   // Marker for later to be removed
                        }
                        else
                        {
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
            ::dbtools::throwFunctionNotSupportedException(::rtl::OUString::createFromAscii("COUNT() - Driver does not support this function."), NULL);
            break;
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

    ::std::vector< ::rtl::OUString> aColumnNames;
    aColumnNames.reserve(_rxColumns->size());
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
            sal_Int32 nColumnPos = 0;
            for (   OSQLColumns::iterator aIter = _rxColumns->begin();
                    aIter != _rxColumns->end();
                    ++aIter,++nColumnPos
                )
            {
                if(nColumnPos < aColumnNames.size())
                    sSelectColumnRealName = aColumnNames[nColumnPos];
                else
                {
                    if((*aIter)->getPropertySetInfo()->hasPropertyByName(sRealName))
                        (*aIter)->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                    else
                        (*aIter)->getPropertyValue(sName) >>= sSelectColumnRealName;
                    aColumnNames.push_back(sSelectColumnRealName);
                }

                if (aCase(sTableColumnName, sSelectColumnRealName))
                {
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = aIter - _rxColumns->begin() + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column
                            // ...
#ifdef _DEBUG
                        OSL_TRACE("Set Col Mapping: %d -> %d", nSelectColumnPos, nTableColumnPos );
#endif
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                    }

                    aRowIter->setBound(sal_True);
                    aRowIter->setTypeKind(DataType::VARCHAR);
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
            m_pParseTree->count() > 2                                                       &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist)                      &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)               &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct) &&
            m_pParseTree->getChild(2)->getChild(0)->getChild(0)->count() == 4
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
#ifdef _DEBUG
            OSL_TRACE("validRow: waiting...");
#endif
            m_aQuery.checkRowAvailable( nRow );
            if ( m_aQuery.errorOccurred() ) {
                ::dbtools::throwGenericSQLException( m_aQuery.getErrorString(), NULL );
            }
            nNumberOfRecords = m_aQuery.getRealRowCount();
    }

    if (( nRow == 0 ) ||
        ( nRow > (sal_uInt32)nNumberOfRecords && m_aQuery.queryComplete()) ){
        OSL_TRACE("validRow(%u): return False", nRow);
        return sal_False;
    }
#ifdef _DEBUG
    OSL_TRACE("validRow(%u): return True", nRow);
#endif
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::seekRow( eRowPosition pos, sal_Int32 nOffset )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

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
            if ( m_aQuery.errorOccurred() ) {
                ::dbtools::throwGenericSQLException( m_aQuery.getErrorString(), NULL );
            }
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
void OResultSet::setColumnMapping(const ::std::vector<sal_Int32>& _aColumnMapping)
{
    m_aColMapping = _aColumnMapping;
#ifdef _DEBUG
    for ( sal_uInt32 i = 0; i < m_aColMapping.size(); i++ )
        OSL_TRACE("Set Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
}
// -----------------------------------------------------------------------------

