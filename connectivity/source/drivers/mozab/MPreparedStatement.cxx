/*************************************************************************
 *
 *  $RCSfile: MPreparedStatement.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:38:05 $
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
#include <stdio.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include "MColumn.hxx"
#include "MPreparedStatement.hxx"
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#include "MResultSetMetaData.hxx"
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mozab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.mozab.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const ::rtl::OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,m_bPrepared(sal_False)
    ,m_sSqlStatement(sql)
    ,m_nNumParams(0)
    ,m_pResultSet( NULL )
{
}
// -----------------------------------------------------------------------------
OPreparedStatement::~OPreparedStatement()
{
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pResultSet)
        m_pResultSet->release();

    clearMyResultSet();

    OStatement_BASE2::disposing();

    m_xMetaData = NULL;
    if(m_aParameterRow.isValid())
    {
        m_aParameterRow->clear();
        m_aParameterRow = NULL;
    }
    m_xParamColumns = NULL;
}
// -----------------------------------------------------------------------------

void OPreparedStatement::parseSql( const ::rtl::OUString& sql ) throw (
     ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
{
    OSL_TRACE("In/Out :: OPreparedStatement::parseSql()");
    OStatement_Base::parseSql( sql );

    m_xParamColumns = new OSQLColumns();

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);

    // describe all parameters need for the resultset
    describeParameter();

    OResultSet::setBoundedColumns(m_aRow,m_xParamColumns,xNames,sal_False,m_xDBMetaData,m_aColMapping);

    m_pResultSet = createResultSet();
    m_pResultSet->acquire();
    m_xResultSet = Reference<XResultSet>(m_pResultSet);
    initializeResultSet(m_pResultSet);
}

// -----------------------------------------------------------------------------

OResultSet* OPreparedStatement::createResultSet( )
{
    OSL_TRACE("In/Out : OPreparedStatement::createResultSet( )");
    return new OResultSet( this, m_aSQLIterator );
}

// -----------------------------------------------------------------------------
void OPreparedStatement::initializeResultSet( OResultSet* _pResult )
{
    OStatement_Base::initializeResultSet( _pResult );

    OSL_TRACE("In/Out : OPreparedStatement::initializeResultSet( )");
    _pResult->setParameterColumns(m_xParamColumns);
    _pResult->setParameterRow(m_aParameterRow);
}

// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData( m_aSQLIterator.getSelectColumns(), m_aSQLIterator.getTables().begin()->first ,m_pTable );
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Reset last warning message

    try {
        clearWarnings ();
        clearMyResultSet();
        // OStatement_BASE2::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    Reference< XResultSet> xRS = OStatement_Base::executeQuery( m_sSqlStatement );
    // same as in statement with the difference that this statement also can contain parameter

    OSL_TRACE("In/Out: OPreparedStatement::execute" );
    return xRS.is();
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    OSL_TRACE("prepStmt::setString( %s )", OUtoCStr( x ) );
    setParameter( parameterIndex, x );
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = OStatement_Base::executeQuery( m_sSqlStatement );

    OSL_TRACE("In/Out: OPreparedStatement::executeQuery" );
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (*m_aParameterRow)[parameterIndex].setNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::implSetObject(this,parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    if( !_parameterIndex || _parameterIndex > m_nNumParams)
        ::dbtools::throwInvalidIndexException(*this);
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkAndResizeParameters(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if ( !m_aParameterRow.isValid() ) {
        m_aParameterRow = new OValueVector();
        m_aParameterRow->push_back(sal_Int32(0));
    }

    if ((sal_Int32)(*m_aParameterRow).size() <= parameterIndex)
        (*m_aParameterRow).resize(parameterIndex+1);
}
// -----------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const
ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    OSL_TRACE("setParameter( %d, '%s')", parameterIndex, OUtoCStr(x) );
    (*m_aParameterRow)[parameterIndex] = x;
}

//------------------------------------------------------------------
sal_uInt32 OPreparedStatement::AddParameter(OSQLParseNode * pParameter, const Reference<XPropertySet>& _xCol)
{

    // Nr. des neu hinzuzufuegenden Parameters:
    sal_uInt32 nParameter = m_xParamColumns->size()+1;

    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument ist kein Parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: Fehler im Parse Tree");
    OSQLParseNode * pMark = pParameter->getChild(0);

    ::rtl::OUString sParameterName;

    // Parameter-Column aufsetzen:
    sal_Int32 eType = DataType::VARCHAR;
    sal_uInt32 nPrecision = 255;
    sal_Int32 nScale = 0;
    sal_Int32 nNullable = ColumnValue::NULLABLE;

    if (_xCol.is())
    {
        // Typ, Precision, Scale ... der angegebenen Column verwenden,
        // denn dieser Column wird der Wert zugewiesen bzw. mit dieser
        // Column wird der Wert verglichen.
        eType = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
        nPrecision = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
        nScale = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
        nNullable = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sParameterName;
    }

    Reference<XPropertySet> xParaColumn = new connectivity::sdbcx::OColumn(sParameterName
                                                    ,::rtl::OUString()
                                                    ,::rtl::OUString()
                                                    ,nNullable
                                                    ,nPrecision
                                                    ,nScale
                                                    ,eType
                                                    ,sal_False
                                                    ,sal_False
                                                    ,sal_False
                                                    ,m_aSQLIterator.isCaseSensitive());
    m_xParamColumns->push_back(xParaColumn);
    return nParameter;
}
// -----------------------------------------------------------------------------
void OPreparedStatement::describeColumn(OSQLParseNode*
_pParameter,OSQLParseNode* _pNode,const OSQLTable& _xTable)
{
    Reference<XPropertySet> xProp;
    if(SQL_ISRULE(_pNode,column_ref))
    {
        ::rtl::OUString sColumnName,sTableRange;
        m_aSQLIterator.getColumnRange(_pNode,sColumnName,sTableRange);
        if(sColumnName.getLength())
        {
            Reference<XNameAccess> xNameAccess = _xTable->getColumns();
            if(xNameAccess->hasByName(sColumnName))
                xNameAccess->getByName(sColumnName) >>= xProp;
            AddParameter(_pParameter,xProp);
        }
    }
    //  else
        //  AddParameter(_pParameter,xProp);
}
// -------------------------------------------------------------------------
void OPreparedStatement::describeParameter()
{
    ::std::vector< OSQLParseNode*> aParseNodes;
    scanParameter(m_pParseTree,aParseNodes);
    if(aParseNodes.size())
    {
        m_xParamColumns = new OSQLColumns();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.size())
        {
            OSQLTable xTable = xTabs.begin()->second;
            ::std::vector< OSQLParseNode*>::const_iterator aIter =
aParseNodes.begin();
            for (;aIter != aParseNodes.end();++aIter )
            {
                describeColumn(*aIter,(*aIter)->getParent()->getChild(0),xTable);
            }
        }
    }
}

// -----------------------------------------------------------------------------
void OPreparedStatement::scanParameter(OSQLParseNode* pParseNode,::std::vector< OSQLParseNode*>& _rParaNodes)
{
    OSL_ENSURE(pParseNode != NULL,"OResultSet: interner Fehler: ungueltiger ParseNode");

    // Parameter Name-Regel gefunden?
    if (SQL_ISRULE(pParseNode,parameter))
    {
        OSL_ENSURE(pParseNode->count() >= 1,"OResultSet: Parse Tree fehlerhaft");
        OSL_ENSURE(pParseNode->getChild(0)->getNodeType() == SQL_NODE_PUNCTUATION,"OResultSet: Parse Tree fehlerhaft");

        _rParaNodes.push_back(pParseNode);
        // Weiterer Abstieg nicht erforderlich
        return;
    }

    // Weiter absteigen im Parse Tree
    for (sal_uInt32 i = 0; i < pParseNode->count(); i++)
        scanParameter(pParseNode->getChild(i),_rParaNodes);
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL OPreparedStatement::getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OPreparedStatement::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OPreparedStatement::getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}
// -----------------------------------------------------------------------------


