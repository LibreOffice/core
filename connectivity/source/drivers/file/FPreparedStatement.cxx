/*************************************************************************
 *
 *  $RCSfile: FPreparedStatement.cxx,v $
 *
 *  $Revision: 1.23 $
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

#include <stdio.h>

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_
#include "file/FPreparedStatement.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSETMETADATA_HXX_
#include "file/FResultSetMetaData.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
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

using namespace connectivity;
using namespace comphelper;
using namespace ::dbtools;
using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbc.driver.file.PreparedStatement","com.sun.star.sdbc.PreparedStatement");

OPreparedStatement::OPreparedStatement( OConnection* _pConnection)
    : OStatement_BASE2( _pConnection )
    ,m_pResultSet(NULL)
    ,m_pEvaluationKeySet(NULL)
{
}
// -------------------------------------------------------------------------
void OPreparedStatement::disposing()
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
    if(m_aEvaluateRow.isValid())
    {
        m_aEvaluateRow->clear();
        m_aEvaluateRow = NULL;
    }
    delete m_pEvaluationKeySet;
    m_xParamColumns = NULL;
}
// -------------------------------------------------------------------------
void OPreparedStatement::construct(const ::rtl::OUString& sql)  throw(SQLException, RuntimeException)
{
    OStatement_Base::construct(sql);

    m_aParameterRow = new OValueVector();
    m_aParameterRow->push_back(sal_Int32(0));
    m_xParamColumns = new OSQLColumns();

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
    // set the binding of the resultrow
    m_aEvaluateRow  = new OValueVector(xNames->getCount());

    (*m_aEvaluateRow)[0].setBound(sal_True);
    ::std::for_each(m_aEvaluateRow->begin()+1,m_aEvaluateRow->end(),TSetBound(sal_False));

    // describe all parameters need for the resultset
    describeParameter();

    OResultSet::setBoundedColumns(m_aEvaluateRow,m_xParamColumns,xNames,sal_False,m_xDBMetaData,m_aColMapping);

    GetAssignValues();

    m_pResultSet = createResultSet();
    m_pResultSet->acquire();
    m_xResultSet = Reference<XResultSet>(m_pResultSet);
    initializeResultSet(m_pResultSet);
}
// -------------------------------------------------------------------------

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
        ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aSQLIterator.getSelectColumns(),m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearMyResultSet();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    initResultSet();

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT || m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT_COUNT;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    initResultSet();

    return m_pResultSet ? m_pResultSet->getRowCountResult() : sal_Int32(0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
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

    return initResultSet();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aData));
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if(m_aAssignValues.isValid())
        (*m_aAssignValues)[m_aParameterIndexes[parameterIndex]].setNull();
    else
        (*m_aParameterRow)[parameterIndex].setNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    throwInvalidIndexException(*this);
    //  setObject (parameterIndex, x, sqlType, 0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    setBinaryStream(parameterIndex,x,length );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readSomeBytes(aSeq,length);
    setParameter(parameterIndex,aSeq);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_aParameterRow->clear();
    m_aParameterRow->push_back(sal_Int32(0));
}
// -------------------------------------------------------------------------
OResultSet* OPreparedStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}
// -----------------------------------------------------------------------------
Reference<XResultSet> OPreparedStatement::initResultSet()
{
    m_pResultSet->clear();
    Reference<XResultSet> xRs(m_pResultSet);

//  UINT16 nCount = m_aAssignValues.isValid() ? m_aAssignValues->size() : 1; // 1 ist wichtig für die Kriterien
//  for (UINT16 j = 1; j < nCount; ++j)
//  {
//      UINT32 nParameter = (*m_aAssignValues).getParameterIndex(j);
//      if (nParameter != SQL_NO_PARAMETER)
//          (*m_aAssignValues)[j] = (*m_aParameterRow)[(UINT16)nParameter];
//  }

    m_pResultSet->OpenImpl();

    return xRs;
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkAndResizeParameters(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    if(m_aAssignValues.isValid() && (parameterIndex < 1 || parameterIndex >= m_aParameterIndexes.size()))
        throwInvalidIndexException(*this);
    else if ((sal_Int32)(*m_aParameterRow).size() <= parameterIndex)
        (*m_aParameterRow).resize(parameterIndex+1);
}
// -----------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if(m_aAssignValues.isValid())
        (*m_aAssignValues)[m_aParameterIndexes[parameterIndex]] = x;
    else
        (*m_aParameterRow)[parameterIndex] = x;
}
// -----------------------------------------------------------------------------
void OPreparedStatement::GetAssignValues()
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
                else
                {
                    if(pRow_Value_Const->count() == aColumnNameList.size())
                    {
                        for (sal_uInt32 j = 0; j < pRow_Value_Const->count(); ++j)
                            ParseAssignValues(aColumnNameList,pRow_Value_Const->getChild(j),nIndex++);
                    }
                    else
                    {
//                      aStatus.Set(SQL_STAT_ERROR,
//                      String::CreateFromAscii("S1000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_STAT_SYNTAX_ERROR))),
//                      0, String() );
                        throwFunctionSequenceException(*this);
                    }

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
void OPreparedStatement::ParseAssignValues(const ::std::vector< String>& aColumnNameList,OSQLParseNode* pRow_Value_Constructor_Elem,xub_StrLen nIndex)
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
    {
        // Parameter hinzufuegen, Typ ... entsprechend der Column, der der Wert zugewiesen wird
        Reference<XPropertySet> xCol;
        m_xColNames->getByName(aColumnName) >>= xCol;
        sal_Int32 nParameter = -1;
        if(m_xParamColumns.isValid())
        {
            OSQLColumns::const_iterator aIter = find(m_xParamColumns->begin(),m_xParamColumns->end(),aColumnName,::comphelper::UStringMixEqual(m_pTable->isCaseSensitive()));
            if(aIter != m_xParamColumns->end())
                nParameter = m_xParamColumns->size() - (m_xParamColumns->end() - aIter) + 1;// +1 because the rows start at 1
        }
        if(nParameter == -1)
            nParameter = AddParameter(pRow_Value_Constructor_Elem,xCol);
        // Nr. des Parameters in der Variablen merken:
        SetAssignValue(aColumnName, String(), TRUE, nParameter);
    }
    else
    {
        //  aStatus.SetStatementTooComplex();
        throwFunctionSequenceException(*this);
    }
}
//------------------------------------------------------------------
void OPreparedStatement::SetAssignValue(const String& aColumnName,
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
        (*m_aAssignValues)[nId].setNull();
    else
    {
        switch (getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
        {
            // Kriterium je nach Typ als String oder double in die Variable packen ...
            case DataType::CHAR:
            case DataType::VARCHAR:
                (*m_aAssignValues)[nId] = aValue;
                // Zeichensatz ist bereits konvertiert, da ja das gesamte Statement konvertiert wurde
                break;

            case DataType::BIT:
                {
                    if (aValue.EqualsIgnoreCaseAscii("TRUE")  || aValue.GetChar(0) == '1')
                        (*m_aAssignValues)[nId] = sal_True;
                    else if (aValue.EqualsIgnoreCaseAscii("FALSE") || aValue.GetChar(0) == '0')
                        (*m_aAssignValues)[nId] = sal_False;
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
                (*m_aAssignValues)[nId] = aValue; // .ToDouble
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

//------------------------------------------------------------------
UINT32 OPreparedStatement::AddParameter(OSQLParseNode * pParameter, const Reference<XPropertySet>& _xCol)
{

    // Nr. des neu hinzuzufuegenden Parameters:
    UINT32 nParameter = m_xParamColumns->size()+1;

    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument ist kein Parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: Fehler im Parse Tree");
    OSQLParseNode * pMark = pParameter->getChild(0);

    ::rtl::OUString sParameterName;
//  if (SQL_ISPUNCTUATION(pMark,"?"))
//      aParameterName = '?';
//  else if (SQL_ISPUNCTUATION(pMark,":"))
//      aParameterName = pParameter->getChild(1)->getTokenValue();
//  else if (SQL_ISPUNCTUATION(pMark,"["))
//      aParameterName = pParameter->getChild(1)->getTokenValue();
//  else
//  {
//      OSL_ASSERT("OResultSet: Fehler im Parse Tree");
//  }

    // Parameter-Column aufsetzen:
    sal_Int32 eType = DataType::VARCHAR;
    UINT32 nPrecision = 255;
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
void OPreparedStatement::describeColumn(OSQLParseNode* _pParameter,OSQLParseNode* _pNode,const OSQLTable& _xTable)
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
            ::std::vector< OSQLParseNode*>::const_iterator aIter = aParseNodes.begin();
            for (;aIter != aParseNodes.end();++aIter )
            {
                describeColumn(*aIter,(*aIter)->getParent()->getChild(0),xTable);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::initializeResultSet(OResultSet* _pResult)
{
    OStatement_Base::initializeResultSet(_pResult);
    m_pResultSet->setParameterColumns(m_xParamColumns);
    m_pResultSet->setParameterRow(m_aParameterRow);
    m_pResultSet->setAssignValues(m_aAssignValues);
    m_pResultSet->setEvaluationRow(m_aEvaluateRow);

    // Parameter substituieren (AssignValues und Kriterien):
    if (!m_xParamColumns->empty())
    {
        // Zunächst AssignValues
        USHORT nParaCount=0; // gibt die aktuelle Anzahl der bisher gesetzen Parameter an

        // Nach zu substituierenden Parametern suchen:
        UINT16 nCount = m_aAssignValues.isValid() ? m_aAssignValues->size() : 1; // 1 ist wichtig für die Kriterien
        for (UINT16 j = 1; j < nCount; j++)
        {
            UINT32 nParameter = (*m_aAssignValues).getParameterIndex(j);
            if (nParameter == SQL_NO_PARAMETER)
                continue;   // dieser AssignValue ist kein Parameter

            ++nParaCount; // ab hier ist der Parameter gueltig
            // Parameter ersetzen. Wenn Parameter nicht verfuegbar,
            //  Value auf NULL setzen.
            //  (*m_aAssignValues)[j] = (*m_aParameterRow)[(UINT16)nParameter];
        }

        if (m_aParameterRow.isValid() &&  nParaCount < m_aParameterRow->size())
        {
            m_pSQLAnalyzer->bindParameterRow(m_aParameterRow);
        }
    }

    m_pEvaluationKeySet = m_pSQLAnalyzer->bindResultRow(m_aEvaluateRow);    // Werte im Code des Compilers setzen
    m_pResultSet->setEvaluationKeySet(m_pEvaluationKeySet);
}
// -----------------------------------------------------------------------------


