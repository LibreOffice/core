/*************************************************************************
 *
 *  $RCSfile: FStatement.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:03:35 $
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
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#include <algorithm>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define THROW_SQL(x) \
    OTools::ThrowException(x,m_aStatementHandle,SQL_HANDLE_STMT,*this)

namespace connectivity
{
    namespace file
    {

//------------------------------------------------------------------------------
using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
DBG_NAME( file_OStatement_Base )

//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection ) :  OStatement_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(OStatement_BASE::rBHelper)
    ,rBHelper(OStatement_BASE::rBHelper)
    ,m_pConnection(_pConnection)
    ,m_pParseTree(NULL)
    ,m_aParser(_pConnection->getDriver()->getFactory())
    ,m_aSQLIterator(_pConnection->createCatalog()->getTables(),_pConnection->getMetaData(),NULL,&m_aParser)
    ,m_nMaxFieldSize(0)
    ,m_nMaxRows(0)
    ,m_nQueryTimeOut(0)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::FORWARD_ONLY)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    ,m_pSQLAnalyzer(NULL)
    ,m_xDBMetaData(_pConnection->getMetaData())
    ,m_pTable(NULL)
    ,m_pEvaluationKeySet(NULL)
{
    DBG_CTOR( file_OStatement_Base, NULL );

    m_pConnection->acquire();

    sal_Int32 nAttrib = 0;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),      PROPERTY_ID_CURSORNAME,         nAttrib,&m_aCursorName,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),    PROPERTY_ID_MAXFIELDSIZE,       nAttrib,&m_nMaxFieldSize,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),         PROPERTY_ID_MAXROWS,            nAttrib,&m_nMaxRows,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),    PROPERTY_ID_QUERYTIMEOUT,       nAttrib,&m_nQueryTimeOut,   ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),       PROPERTY_ID_FETCHSIZE,          nAttrib,&m_nFetchSize,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),   PROPERTY_ID_RESULTSETTYPE,      nAttrib,&m_nResultSetType,  ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),  PROPERTY_ID_FETCHDIRECTION,     nAttrib,&m_nFetchDirection, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),PROPERTY_ID_ESCAPEPROCESSING,   nAttrib,&m_bEscapeProcessing,::getCppuBooleanType());

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),        PROPERTY_ID_RESULTSETCONCURRENCY,   nAttrib,&m_nResultSetConcurrency,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
// -----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
    osl_incrementInterlockedCount( &m_refCount );
    disposing();
    delete m_pSQLAnalyzer;

    DBG_DTOR( file_OStatement_Base, NULL );
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
    {
        m_pConnection->release();
        m_pConnection = NULL;
    }

    dispose_ChildImpl();

    if ( m_pParseTree )
    {
        delete m_pParseTree;
        m_pParseTree = NULL;
    }

    OStatement_Base::disposing();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw()
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
void SAL_CALL OStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::disposing(void)
{
    if(m_aEvaluateRow.isValid())
    {
        m_aEvaluateRow->clear();
        m_aEvaluateRow = NULL;
    }
    delete m_pEvaluationKeySet;
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
            setOrderbyColumn(pColumnRef,pAscendingDescending);
        }
    }
}
//------------------------------------------------------------------
void OStatement_Base::setOrderbyColumn( OSQLParseNode* pColumnRef,
                                        OSQLParseNode* pAscendingDescending)
{
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
    try
    {
        m_aOrderbyColumnNumber.push_back(xColLocate->findColumn(aColumnName));
    }
    catch(Exception)
    {
        ::vos::ORef<OSQLColumns> aSelectColumns = m_aSQLIterator.getSelectColumns();
        ::comphelper::UStringMixEqual aCase;
        OSQLColumns::const_iterator aFind = ::connectivity::find(aSelectColumns->begin(),aSelectColumns->end(),aColumnName,aCase);
        if ( aFind == aSelectColumns->end() )
            throw SQLException();
        m_aOrderbyColumnNumber.push_back((aFind - aSelectColumns->begin()) + 1);
    }

    // Ascending or Descending?
    m_aOrderbyAscending.push_back((SQL_ISTOKEN(pAscendingDescending,DESC)) ? SQL_DESC : SQL_ASC);
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

        if ( xTabs.empty() )
            throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid. It contains no valid table."),
                                        static_cast<XWeak*>(this),
                                        makeAny(m_aSQLIterator.getWarning()));
        if ( xTabs.size() > 1 || m_aSQLIterator.getWarning().Message.getLength() )
            throwGenericSQLException(   ::rtl::OUString::createFromAscii("The statement is invalid. It contains more than one table."),
                                        static_cast<XWeak*>(this),
                                        makeAny(m_aSQLIterator.getWarning()));

        // at this moment we support only one table per select statement
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xTabs.begin()->second,UNO_QUERY);
        if(xTunnel.is())
        {
            if(m_pTable)
                m_pTable->release();
            m_pTable = reinterpret_cast<OFileTable*>(xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()));
            if(m_pTable)
                m_pTable->acquire();
        }
        OSL_ENSURE(m_pTable,"No table!");
        m_xColNames     = m_pTable->getColumns();
        Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
        // set the binding of the resultrow
        m_aRow          = new OValueRefVector(xNames->getCount());
        (*m_aRow)[0]->setBound(sal_True);
        ::std::for_each(m_aRow->begin()+1,m_aRow->end(),TSetRefBound(sal_False));

        // set the binding of the resultrow
        m_aEvaluateRow  = new OValueRefVector(xNames->getCount());

        (*m_aEvaluateRow)[0]->setBound(sal_True);
        ::std::for_each(m_aEvaluateRow->begin()+1,m_aEvaluateRow->end(),TSetRefBound(sal_False));

        // set the select row
        m_aSelectRow = new OValueRefVector(m_aSQLIterator.getSelectColumns()->size());
        ::std::for_each(m_aSelectRow->begin(),m_aSelectRow->end(),TSetRefBound(sal_True));

        // create the column mapping
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
    OResultSet::setBoundedColumns(m_aRow,m_aSelectRow,xColumns,xNames,sal_True,m_xDBMetaData,m_aColMapping);
}
// -----------------------------------------------------------------------------
void OStatement_Base::initializeResultSet(OResultSet* _pResult)
{
    GetAssignValues();

    _pResult->setSqlAnalyzer(m_pSQLAnalyzer);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setColumnMapping(m_aColMapping);
    _pResult->setEvaluationRow(m_aEvaluateRow);
    _pResult->setAssignValues(m_aAssignValues);
    _pResult->setSelectRow(m_aSelectRow);

    m_pSQLAnalyzer->bindSelectRow(m_aRow);
    m_pEvaluationKeySet = m_pSQLAnalyzer->bindEvaluationRow(m_aEvaluateRow);    // Werte im Code des Compilers setzen
    _pResult->setEvaluationKeySet(m_pEvaluationKeySet);
}
// -----------------------------------------------------------------------------
void OStatement_Base::GetAssignValues()
{
    if (m_pParseTree == NULL)
    {
        ::dbtools::throwFunctionSequenceException(*this);
        return;
    }

    if (SQL_ISRULE(m_pParseTree,select_statement))
        // Keine zu setzenden Werte bei SELECT
        return;
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        // Row fuer die zu setzenden Werte anlegen (Referenz durch new)
        if(m_aAssignValues.isValid())
            m_aAssignValues->clear();
        sal_Int32 nCount = Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount();
        m_aAssignValues = new OAssignValues(nCount);
        // unbound all
        ::std::for_each(m_aAssignValues->begin()+1,m_aAssignValues->end(),TSetRefBound(sal_False));

        m_aParameterIndexes.resize(nCount+1,SQL_NO_PARAMETER);

        // Liste der Columns-Namen, die in der column_commalist vorkommen (mit ; getrennt):
        ::std::vector<String> aColumnNameList;

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pOptColumnCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pOptColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pOptColumnCommalist,opt_column_commalist),"OResultSet: Fehler im Parse Tree");
        if (pOptColumnCommalist->count() == 0)
        {
            const Sequence< ::rtl::OUString>& aNames = m_xColNames->getElementNames();
            const ::rtl::OUString* pBegin = aNames.getConstArray();
            aColumnNameList.insert(aColumnNameList.begin(),::std::vector<String>::const_iterator(pBegin),::std::vector<String>::const_iterator(pBegin + aNames.getLength()));
        }
        else
        {
            OSL_ENSURE(pOptColumnCommalist->count() == 3,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnCommalist = pOptColumnCommalist->getChild(1);
            OSL_ENSURE(pColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(SQL_ISRULE(pColumnCommalist,column_commalist),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pColumnCommalist->count() > 0,"OResultSet: Fehler im Parse Tree");

            // Alle Columns in der column_commalist ...
            for (sal_uInt32 i = 0; i < pColumnCommalist->count(); i++)
            {
                OSQLParseNode * pCol = pColumnCommalist->getChild(i);
                OSL_ENSURE(pCol != NULL,"OResultSet: Fehler im Parse Tree");
                aColumnNameList.push_back(pCol->getTokenValue());
            }
        }
        if(!aColumnNameList.size())
            throwFunctionSequenceException(*this);

        // Werte ...
        OSQLParseNode * pValuesOrQuerySpec = m_pParseTree->getChild(4);
        OSL_ENSURE(pValuesOrQuerySpec != NULL,"OResultSet: pValuesOrQuerySpec darf nicht NULL sein!");
        OSL_ENSURE(SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec),"OResultSet: ! SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec)");
        OSL_ENSURE(pValuesOrQuerySpec->count() > 0,"OResultSet: pValuesOrQuerySpec->count() <= 0");

        // nur "VALUES" ist erlaubt ...
        if (! SQL_ISTOKEN(pValuesOrQuerySpec->getChild(0),VALUES))
            throwFunctionSequenceException(*this);

        OSL_ENSURE(pValuesOrQuerySpec->count() == 2,"OResultSet: pValuesOrQuerySpec->count() != 2");

        // Liste von Werten
        OSQLParseNode * pInsertAtomCommalist = pValuesOrQuerySpec->getChild(1);
        OSL_ENSURE(pInsertAtomCommalist != NULL,"OResultSet: pInsertAtomCommalist darf nicht NULL sein!");
        OSL_ENSURE(pInsertAtomCommalist->count() > 0,"OResultSet: pInsertAtomCommalist <= 0");

        String aColumnName;
        OSQLParseNode * pRow_Value_Const;
        xub_StrLen nIndex=0;
        for (sal_uInt32 i = 0; i < pInsertAtomCommalist->count(); i++)
        {
            pRow_Value_Const = pInsertAtomCommalist->getChild(i); // row_value_constructor
            if(pRow_Value_Const->count() == 3)  // '(' row_value_const_list ')'
            {
                pRow_Value_Const = pRow_Value_Const->getChild(1); // row_value_const_list
                OSL_ENSURE(pRow_Value_Const != NULL,"OResultSet: pRow_Value_Const darf nicht NULL sein!");
                if(SQL_ISRULE(pRow_Value_Const,parameter))
                {
                    if(pRow_Value_Const->count() == aColumnNameList.size())
                        ParseAssignValues(aColumnNameList,pRow_Value_Const,nIndex++); // kann nur ein Columnname vorhanden sein pro Schleife
                    else
                    {
//                      aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_SYNTAX_ERROR))),
//                      0, String() );
                        throwFunctionSequenceException(*this);
                    }
                }
                else if(pRow_Value_Const->isToken())
                    ParseAssignValues(aColumnNameList,pRow_Value_Const,static_cast<xub_StrLen>(i));
                else
                {
                    if(pRow_Value_Const->count() == aColumnNameList.size())
                    {
                        for (sal_uInt32 j = 0; j < pRow_Value_Const->count(); ++j)
                            ParseAssignValues(aColumnNameList,pRow_Value_Const->getChild(j),nIndex++);
                    }
                    else
                        throwFunctionSequenceException(*this);
                }
            }
            else
            {
                //  aStatus.SetStatementTooComplex();
                throwFunctionSequenceException(*this);
            }
        }
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        if(m_aAssignValues.isValid())
            m_aAssignValues->clear();
        sal_Int32 nCount = Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount();
        m_aAssignValues = new OAssignValues(nCount);
        // unbound all
        ::std::for_each(m_aAssignValues->begin()+1,m_aAssignValues->end(),TSetRefBound(sal_False));

        m_aParameterIndexes.resize(nCount+1,SQL_NO_PARAMETER);

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pAssignmentCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pAssignmentCommalist != NULL,"OResultSet: pAssignmentCommalist == NULL");
        OSL_ENSURE(SQL_ISRULE(pAssignmentCommalist,assignment_commalist),"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(pAssignmentCommalist->count() > 0,"OResultSet: pAssignmentCommalist->count() <= 0");

        // Alle Zuweisungen (Kommaliste) bearbeiten ...
        ::std::vector< String> aList(1);
        for (sal_uInt32 i = 0; i < pAssignmentCommalist->count(); i++)
        {
            OSQLParseNode * pAssignment = pAssignmentCommalist->getChild(i);
            OSL_ENSURE(pAssignment != NULL,"OResultSet: pAssignment == NULL");
            OSL_ENSURE(SQL_ISRULE(pAssignment,assignment),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pAssignment->count() == 3,"OResultSet: pAssignment->count() != 3");

            OSQLParseNode * pCol = pAssignment->getChild(0);
            OSL_ENSURE(pCol != NULL,"OResultSet: pCol == NULL");

            OSQLParseNode * pComp = pAssignment->getChild(1);
            OSL_ENSURE(pComp != NULL,"OResultSet: pComp == NULL");
            OSL_ENSURE(pComp->getNodeType() == SQL_NODE_EQUAL,"OResultSet: pComp->getNodeType() != SQL_NODE_COMPARISON");
            if (pComp->getTokenValue().toChar() != '=')
            {
                //  aStatus.SetInvalidStatement();
                throwFunctionSequenceException(*this);
            }

            OSQLParseNode * pVal = pAssignment->getChild(2);
            OSL_ENSURE(pVal != NULL,"OResultSet: pVal == NULL");
            aList[0] = pCol->getTokenValue();
            ParseAssignValues(aList,pVal,0);
        }

    }
}
// -------------------------------------------------------------------------
void OStatement_Base::ParseAssignValues(const ::std::vector< String>& aColumnNameList,OSQLParseNode* pRow_Value_Constructor_Elem,xub_StrLen nIndex)
{
    OSL_ENSURE(nIndex <= aColumnNameList.size(),"SdbFileCursor::ParseAssignValues: nIndex > aColumnNameList.GetTokenCount()");
    String aColumnName(aColumnNameList[nIndex]);
    OSL_ENSURE(aColumnName.Len() > 0,"OResultSet: Column-Name nicht gefunden");
    OSL_ENSURE(pRow_Value_Constructor_Elem != NULL,"OResultSet: pRow_Value_Constructor_Elem darf nicht NULL sein!");

    if (pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_STRING ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_INTNUM ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_APPROXNUM)
    {
        // Wert setzen:
        SetAssignValue(aColumnName, pRow_Value_Constructor_Elem->getTokenValue());
    }
    else if (SQL_ISTOKEN(pRow_Value_Constructor_Elem,NULL))
    {
        // NULL setzen
        SetAssignValue(aColumnName, String(), TRUE);
    }
    else if (SQL_ISRULE(pRow_Value_Constructor_Elem,parameter))
        parseParamterElem(aColumnName,pRow_Value_Constructor_Elem);
    else
    {
        //  aStatus.SetStatementTooComplex();
        throwFunctionSequenceException(*this);
    }
}
//------------------------------------------------------------------
void OStatement_Base::SetAssignValue(const String& aColumnName,
                                   const String& aValue,
                                   BOOL bSetNull,
                                   UINT32 nParameter)
{
    Reference<XPropertySet> xCol;
    m_xColNames->getByName(aColumnName) >>= xCol;
    sal_Int32 nId = Reference<XColumnLocate>(m_xColNames,UNO_QUERY)->findColumn(aColumnName);
    // Kommt diese Column ueberhaupt in der Datei vor?

    if (!xCol.is())
    {
        // Diese Column gibt es nicht!
//      aStatus.Set(SQL_STAT_ERROR,
//                  String::CreateFromAscii("S0022"),
//                  aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_COLUMN_NOT_FOUND))),
//                  0, String() );
        throwFunctionSequenceException(*this);
    }

    // Value an die Row mit den zuzuweisenden Werten binden:
    //  const ODbVariantRef& xValue = (*aAssignValues)[pFileColumn->GetId()];

    // Alles geprueft und wir haben den Namen der Column.
    // Jetzt eine Value allozieren, den Wert setzen und die Value an die Row binden.
    if (bSetNull)
        (*m_aAssignValues)[nId]->setNull();
    else
    {
        switch (::comphelper::getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
        {
            // Kriterium je nach Typ als String oder double in die Variable packen ...
            case DataType::CHAR:
            case DataType::VARCHAR:
                *(*m_aAssignValues)[nId] = ORowSetValue(aValue);
                // Zeichensatz ist bereits konvertiert, da ja das gesamte Statement konvertiert wurde
                break;

            case DataType::BIT:
                {
                    if (aValue.EqualsIgnoreCaseAscii("TRUE")  || aValue.GetChar(0) == '1')
                        *(*m_aAssignValues)[nId] = sal_True;
                    else if (aValue.EqualsIgnoreCaseAscii("FALSE") || aValue.GetChar(0) == '0')
                        *(*m_aAssignValues)[nId] = sal_False;
                    else
                    {
                        //  aStatus.Set(SQL_STAT_ERROR);    // nyi: genauer!
                        throwFunctionSequenceException(*this);
                    }
                }
                break;
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
            {
                *(*m_aAssignValues)[nId] = ORowSetValue(aValue); // .ToDouble
//              try
//              {
//                  double n = xValue->toDouble();
//                  xValue->setDouble(n);
//              }
//              catch ( ... )
//              {
//                  aStatus.SetDriverNotCapableError();
//              }
            }   break;
            default:
                throwFunctionSequenceException(*this);
        }
    }

    // Parameter-Nr. merken (als User Data)
    // SQL_NO_PARAMETER = kein Parameter.
    m_aAssignValues->setParameterIndex(nId,nParameter);
    if(nParameter != SQL_NO_PARAMETER)
        m_aParameterIndexes[nParameter] = nId;
}
// -----------------------------------------------------------------------------
void OStatement_Base::parseParamterElem(const String& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem)
{
    // do nothing here
}
    }
}
// -----------------------------------------------------------------------------
