/*************************************************************************
 *
 *  $RCSfile: FStatement.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:08:38 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_FILE_OSTATEMENT_HXX_
#include "file/FStatement.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ODRIVER_HXX_
#include "file/FDriver.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSET_HXX_
#include "file/FResultSet.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#define THROW_SQL(x) \
    OTools::ThrowException(x,m_aStatementHandle,SQL_HANDLE_STMT,*this)

namespace connectivity
{
    namespace file
    {

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(OStatement_BASE::rBHelper)
    ,rBHelper(OStatement_BASE::rBHelper)
    ,m_pConnection(_pConnection)
    ,m_pParseTree(NULL)
    ,m_aSQLIterator(_pConnection->createCatalog()->getTables(),_pConnection->getMetaData(),NULL)
    ,m_aParser(_pConnection->getDriver()->getFactory())
    ,m_nMaxFieldSize(0)
    ,m_nMaxRows(0)
    ,m_nQueryTimeOut(0)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::FORWARD_ONLY)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    ,m_pSQLAnalyzer(NULL)
    ,m_aOrderbyColumnNumber(SQL_ORDERBYKEYS,SQL_COLUMN_NOTFOUND)
    ,m_aOrderbyAscending(SQL_ORDERBYKEYS,1)
    ,m_xDBMetaData(_pConnection->getMetaData())
    ,m_pTable(NULL)
{
    m_pConnection->acquire();

    sal_Int32 nAttrib = 0;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),      PROPERTY_ID_CURSORNAME,         nAttrib,&m_aCursorName,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),        PROPERTY_ID_MAXFIELDSIZE,       nAttrib,&m_nMaxFieldSize,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),         PROPERTY_ID_MAXROWS,            nAttrib,&m_nMaxRows,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),        PROPERTY_ID_QUERYTIMEOUT,       nAttrib,&m_nQueryTimeOut,   ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),       PROPERTY_ID_FETCHSIZE,          nAttrib,&m_nFetchSize,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),   PROPERTY_ID_RESULTSETTYPE,      nAttrib,&m_nResultSetType,  ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),  PROPERTY_ID_FETCHDIRECTION,     nAttrib,&m_nFetchDirection, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),    PROPERTY_ID_ESCAPEPROCESSING,   nAttrib,&m_bEscapeProcessing,::getCppuBooleanType());

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),        PROPERTY_ID_RESULTSETCONCURRENCY,   nAttrib,&m_nResultSetConcurrency,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
// -----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
    osl_incrementInterlockedCount( &m_refCount );
    disposing();
    delete m_pSQLAnalyzer;
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if(m_pSQLAnalyzer)
        m_pSQLAnalyzer->dispose();

    if(m_aRow.isValid())
    {
        m_aRow->clear();
        m_aRow = NULL;
    }

    m_aSQLIterator.dispose();

    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }

    if (m_pConnection)
        m_pConnection->release();

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_BASE::acquire();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw(RuntimeException)
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
        ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                                        ::getCppuType( (const Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                                        ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    }
    dispose();
}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearWarnings ();

    if (m_xResultSet.get().is())
        clearMyResultSet();
}
//--------------------------------------------------------------------
// clearMyResultSet
// If a ResultSet was created for this Statement, close it
//--------------------------------------------------------------------

void OStatement_Base::clearMyResultSet () throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    Reference<XCloseable> xCloseable;
    if(::comphelper::query_interface(m_xResultSet.get(),xCloseable))
        xCloseable->close();
    m_xResultSet = Reference< XResultSet>();
}
//--------------------------------------------------------------------
// setWarning
// Sets the warning
//--------------------------------------------------------------------

void OStatement_Base::setWarning (const SQLWarning &ex) throw( SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = ex;
}

// -------------------------------------------------------------------------

sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    executeQuery(sql);

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT || m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT_COUNT;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    construct(sql);
    OResultSet* pResult = createResultSet();
    Reference< XResultSet > xRS = pResult;
    initializeResultSet(pResult);

    pResult->OpenImpl();
    return xRS;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    construct(sql);
    OResultSet* pResult = createResultSet();
    Reference< XResultSet > xRS = pResult;
    initializeResultSet(pResult);
    pResult->OpenImpl();

    return pResult->getRowCountResult();
}
// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
OResultSet* OStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbc.driver.file.Statement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::disposing(void)
{
    OStatement_BASE::disposing();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL OStatement::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return OStatement_BASE2::queryInterface( rType);
}
// -----------------------------------------------------------------------------
OSQLAnalyzer* OStatement_Base::createAnalyzer()
{
    return new OSQLAnalyzer();
}
// -----------------------------------------------------------------------------
void OStatement_Base::anylizeSQL()
{
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::anylizeSQL: Analyzer isn't set!");
    // start analysing the statement
    m_pSQLAnalyzer->setOrigColumns(m_xColNames);
    m_pSQLAnalyzer->start(m_pParseTree);

    const OSQLParseNode* pOrderbyClause = m_aSQLIterator.getOrderTree();
    if(pOrderbyClause)
    {
        OSQLParseNode * pOrderingSpecCommalist = pOrderbyClause->getChild(2);
        OSL_ENSURE(SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OResultSet: Fehler im Parse Tree");

        for (sal_uInt32 m = 0; m < pOrderingSpecCommalist->count(); m++)
        {
            OSQLParseNode * pOrderingSpec = pOrderingSpecCommalist->getChild(m);
            OSL_ENSURE(SQL_ISRULE(pOrderingSpec,ordering_spec),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pOrderingSpec->count() == 2,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnRef = pOrderingSpec->getChild(0);
            if(!SQL_ISRULE(pColumnRef,column_ref))
            {
                throw SQLException();
            }
            OSQLParseNode * pAscendingDescending = pOrderingSpec->getChild(1);
            setOrderbyColumn((UINT16)m,pColumnRef,pAscendingDescending);
        }
    }
}
//------------------------------------------------------------------
void OStatement_Base::setOrderbyColumn(UINT16 nOrderbyColumnNo,
                                     OSQLParseNode* pColumnRef,
                                     OSQLParseNode* pAscendingDescending)
{
    if (nOrderbyColumnNo >= m_aOrderbyColumnNumber.size())
    {
        throw SQLException();
    }

    ::rtl::OUString aColumnName;
    if (pColumnRef->count() == 1)
        aColumnName = pColumnRef->getChild(0)->getTokenValue();
    else if (pColumnRef->count() == 3)
    {
        // Nur die Table Range-Variable darf hier vorkommen:
//      if (!(pColumnRef->getChild(0)->getTokenValue() == aTableRange))
//      {
//          aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_INVALID_RANGE_VAR))),
//                      0, String() );
            //  return;
        //  }
        pColumnRef->getChild(2)->parseNodeToStr(aColumnName,m_xDBMetaData,NULL,sal_False,sal_False);
    }
    else
    {
        //  aStatus.SetStatementTooComplex();
        throw SQLException();
    }

    Reference<XColumnLocate> xColLocate(m_xColNames,UNO_QUERY);
    if(!xColLocate.is())
        return;
    // Alles geprueft und wir haben den Namen der Column.
    // Die wievielte Column ist das?
    m_aOrderbyColumnNumber[nOrderbyColumnNo] = xColLocate->findColumn(aColumnName);

    // Ascending or Descending?
    m_aOrderbyAscending[nOrderbyColumnNo]  = (SQL_ISTOKEN(pAscendingDescending,DESC)) ?
                                            FALSE : TRUE;
}

// -----------------------------------------------------------------------------
void OStatement_Base::construct(const ::rtl::OUString& sql)  throw(SQLException, RuntimeException)
{
    ::rtl::OUString aErr;
    m_pParseTree = m_aParser.parseTree(aErr,sql);
    if(m_pParseTree)
    {
        m_aSQLIterator.setParseTree(m_pParseTree);
        m_aSQLIterator.traverseAll();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.empty())
            throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());

        // at this moment we support only one table per select statement
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xTabs.begin()->second,UNO_QUERY);
        if(xTunnel.is())
        {
            if(m_pTable)
                m_pTable->release();
            m_pTable = (OFileTable*)xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId());
            if(m_pTable)
                m_pTable->acquire();
        }
        OSL_ENSURE(m_pTable,"No table!");
        m_xColNames     = m_pTable->getColumns();
        Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
        // set the binding of the resultrow
        m_aRow          = new OValueVector(xNames->getCount());
        (*m_aRow)[0].setBound(sal_True);
        ::std::for_each(m_aRow->begin()+1,m_aRow->end(),TSetBound(sal_False));
        // create teh column mapping
        createColumnMapping();

        m_pSQLAnalyzer  = createAnalyzer();

        OSL_ENSURE(m_pTable,"We need a table object!");
        Reference<XIndexesSupplier> xIndexSup(xTunnel,UNO_QUERY);
        if(xIndexSup.is())
            m_pSQLAnalyzer->setIndexes(xIndexSup->getIndexes());

        anylizeSQL();
    }
    else
        throw SQLException(aErr,*this,::rtl::OUString(),0,Any());
}
// -----------------------------------------------------------------------------
void OStatement_Base::createColumnMapping()
{
    // initialize the column index map (mapping select columns to table columns)
    ::vos::ORef<connectivity::OSQLColumns>  xColumns = m_aSQLIterator.getSelectColumns();
    m_aColMapping.resize(xColumns->size() + 1);
    for (sal_Int32 i=0; i<(sal_Int32)m_aColMapping.size(); ++i)
        m_aColMapping[i] = i;

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
    // now check which columns are bound
    OResultSet::setBoundedColumns(m_aRow,xColumns,xNames,sal_True,m_xDBMetaData,m_aColMapping);
}
// -----------------------------------------------------------------------------
void OStatement_Base::initializeResultSet(OResultSet* _pResult)
{
    _pResult->setSqlAnalyzer(m_pSQLAnalyzer);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setColumnMapping(m_aColMapping);
    //  _pResult->setTable(m_pTable);

}
    }
}
// -----------------------------------------------------------------------------
