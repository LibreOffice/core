/*************************************************************************
 *
 *  $RCSfile: fcomp.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:26:30 $
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

#ifndef _CONNECTIVITY_FILE_FCOMP_HXX_
#include "file/fcomp.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#include "file/fanalyzer.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#include "file/FStringFunctions.hxx"
#include "file/FDateFunctions.hxx"
#include "file/FNumericFunctions.hxx"

using namespace connectivity;
using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

DBG_NAME(OPredicateCompiler);
//------------------------------------------------------------------
OPredicateCompiler::OPredicateCompiler(OSQLAnalyzer* pAnalyzer)//,OCursor& rCurs)
                     // : m_rCursor(rCurs)
                     : m_pAnalyzer(pAnalyzer)
                     , m_bORCondition(FALSE)
                     , m_nParamCounter(0)
{
    DBG_CTOR(OPredicateCompiler,NULL);
}

//------------------------------------------------------------------
OPredicateCompiler::~OPredicateCompiler()
{
    Clean();
    DBG_DTOR(OPredicateCompiler,NULL);
}
// -----------------------------------------------------------------------------
void OPredicateCompiler::dispose()
{
    Clean();
    m_orgColumns        = NULL;
    m_xIndexes          = NULL;
}
//------------------------------------------------------------------
//  inline OCursor& OPredicateCompiler::Cursor() const {return m_rCursor;}
//------------------------------------------------------------------
void OPredicateCompiler::start(OSQLParseNode* pSQLParseNode)
{
    if (!pSQLParseNode)
        return;

    m_nParamCounter = 0;
    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf WHERE-Klausel setzen:
    OSQLParseNode * pWhereClause = NULL;
    OSQLParseNode * pOrderbyClause = NULL;

    if (SQL_ISRULE(pSQLParseNode,select_statement))
    {
        DBG_ASSERT(pSQLParseNode->count() >= 4,"OFILECursor: Fehler im Parse Tree");

        OSQLParseNode * pTableExp = pSQLParseNode->getChild(3);
        DBG_ASSERT(pTableExp != NULL,"Fehler im Parse Tree");
        DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp)," Fehler im Parse Tree");
        DBG_ASSERT(pTableExp->count() == 5,"Fehler im Parse Tree");

        // check that we don't use anything other than count(*) as function
        OSQLParseNode* pSelection = pSQLParseNode->getChild(2);
        if ( SQL_ISRULE(pSelection,scalar_exp_commalist) )
        {
            for (sal_uInt32 i = 0; i < pSelection->count(); i++)
            {
                OSQLParseNode *pColumnRef = pSelection->getChild(i)->getChild(0);
                if ( SQL_ISRULE(pColumnRef,general_set_fct) && pColumnRef->count() != 4 )
                {
                    ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Statement to complex. Only \"COUNT(*)\" is supported.")),NULL);
                }
            }
        }


        pWhereClause    = pTableExp->getChild(1);
        pOrderbyClause  = pTableExp->getChild(4);
    }
    else if (SQL_ISRULE(pSQLParseNode,update_statement_searched))
    {
        DBG_ASSERT(pSQLParseNode->count() == 5,"OFILECursor: Fehler im Parse Tree");
        pWhereClause = pSQLParseNode->getChild(4);
    }
    else if (SQL_ISRULE(pSQLParseNode,delete_statement_searched))
    {
        DBG_ASSERT(pSQLParseNode->count() == 4,"Fehler im Parse Tree");
        pWhereClause = pSQLParseNode->getChild(3);
    }
    else
            // Anderes Statement. Keine Selektionskriterien.
        return;

    if (SQL_ISRULE(pWhereClause,where_clause))
    {
        // Wenn es aber eine where_clause ist, dann darf sie nicht leer sein:
        DBG_ASSERT(pWhereClause->count() == 2,"OFILECursor: Fehler im Parse Tree");

        OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
        DBG_ASSERT(pComparisonPredicate != NULL,"OFILECursor: Fehler im Parse Tree");

        execute( pComparisonPredicate );
    }
    else
    {
        // Die Where Clause ist meistens optional, d. h. es koennte sich auch
        // um "optional_where_clause" handeln.
        DBG_ASSERT(SQL_ISRULE(pWhereClause,opt_where_clause),"OPredicateCompiler: Fehler im Parse Tree");
    }
}

//------------------------------------------------------------------
OOperand* OPredicateCompiler::execute(OSQLParseNode* pPredicateNode)
{
    OOperand* pOperand = NULL;
    if (pPredicateNode->count() == 3 &&                         // Ausdruck is geklammert
        SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pPredicateNode->getChild(2),")"))
    {
        execute(pPredicateNode->getChild(1));
    }
    else if ((SQL_ISRULE(pPredicateNode,search_condition) || (SQL_ISRULE(pPredicateNode,boolean_term)))
                            &&          // AND/OR-Verknuepfung:
             pPredicateNode->count() == 3)
    {
        execute(pPredicateNode->getChild(0));                           // Bearbeiten des linken Zweigs
        execute(pPredicateNode->getChild(2));                           // Bearbeiten des rechten Zweigs

        if (SQL_ISTOKEN(pPredicateNode->getChild(1),OR))                // OR-Operator
        {
            m_aCodeList.push_back(new OOp_OR());
            m_bORCondition = sal_True;
        }
        else if (SQL_ISTOKEN(pPredicateNode->getChild(1),AND))      // AND-Operator
            m_aCodeList.push_back(new OOp_AND());
        else
        {
            DBG_ERROR("OPredicateCompiler: Fehler im Parse Tree");
        }
    }
    else if (SQL_ISRULE(pPredicateNode,comparison_predicate))
    {
        execute_COMPARE(pPredicateNode);
    }
    else if (SQL_ISRULE(pPredicateNode,like_predicate))
    {
        execute_LIKE(pPredicateNode);
    }
    else if (SQL_ISRULE(pPredicateNode,test_for_null))
    {
        execute_ISNULL(pPredicateNode);
    }
    else if(SQL_ISRULE(pPredicateNode,num_value_exp))
    {
        execute(pPredicateNode->getChild(0));                           // Bearbeiten des linken Zweigs
        execute(pPredicateNode->getChild(2));                           // Bearbeiten des rechten Zweigs
        if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"+"))
        {
            m_aCodeList.push_back(new OOp_ADD());
        }
        else if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"-"))
            m_aCodeList.push_back(new OOp_SUB());
        else
        {
            DBG_ERROR("OPredicateCompiler: Fehler im Parse Tree num_value_exp");
        }
    }
    else if(SQL_ISRULE(pPredicateNode,term))
    {
        execute(pPredicateNode->getChild(0));                           // Bearbeiten des linken Zweigs
        execute(pPredicateNode->getChild(2));                           // Bearbeiten des rechten Zweigs
        if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"*"))
        {
            m_aCodeList.push_back(new OOp_MUL());
        }
        else if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"/"))
            m_aCodeList.push_back(new OOp_DIV());
        else
        {
            DBG_ERROR("OPredicateCompiler: Fehler im Parse Tree num_value_exp");
        }
    }
    else
        pOperand = execute_Operand(pPredicateNode);                     // jetzt werden nur einfache Operanden verarbeitet

    return pOperand;
}

//------------------------------------------------------------------
OOperand* OPredicateCompiler::execute_COMPARE(OSQLParseNode* pPredicateNode)  throw(SQLException, RuntimeException)
{
    DBG_ASSERT(pPredicateNode->count() == 3,"OFILECursor: Fehler im Parse Tree");

    if ( !(SQL_ISRULE(pPredicateNode->getChild(0),column_ref)               ||
          pPredicateNode->getChild(2)->getNodeType() == SQL_NODE_STRING     ||
          pPredicateNode->getChild(2)->getNodeType() == SQL_NODE_INTNUM     ||
          pPredicateNode->getChild(2)->getNodeType() == SQL_NODE_APPROXNUM  ||
          SQL_ISTOKEN(pPredicateNode->getChild(2),TRUE)                     ||
          SQL_ISTOKEN(pPredicateNode->getChild(2),FALSE)                    ||
          SQL_ISRULE(pPredicateNode->getChild(2),parameter)                 ||
          // odbc date
          SQL_ISRULE(pPredicateNode->getChild(2),set_fct_spec)              ||
          SQL_ISRULE(pPredicateNode->getChild(2),position_exp)              ||
          SQL_ISRULE(pPredicateNode->getChild(2),char_substring_fct)        ||
          // upper, lower etc.
          SQL_ISRULE(pPredicateNode->getChild(2),fold)) )
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Statement to complex"),NULL);
        return NULL;
    }

    OSQLPredicateType ePredicateType;
    OSQLParseNode *pPrec = pPredicateNode->getChild(1);

    if (pPrec->getNodeType() == SQL_NODE_EQUAL)
        ePredicateType = SQL_PRED_EQUAL;
    else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
        ePredicateType = SQL_PRED_NOTEQUAL;
    else if (pPrec->getNodeType() == SQL_NODE_LESS)
        ePredicateType = SQL_PRED_LESS;
    else if (pPrec->getNodeType() == SQL_NODE_LESSEQ)
        ePredicateType = SQL_PRED_LESSOREQUAL;
    else if (pPrec->getNodeType() == SQL_NODE_GREATEQ)
        ePredicateType = SQL_PRED_GREATEROREQUAL;
    else if (pPrec->getNodeType() == SQL_NODE_GREAT)
        ePredicateType = SQL_PRED_GREATER;

    OOperand* pOb = execute(pPredicateNode->getChild(0));
    OOperand* pOperand  = execute(pPredicateNode->getChild(2));

    OBoolOperator* pOperator = new OOp_COMPARE(ePredicateType);
    //pOb->PreProcess(pOperator, pOperand);

    m_aCodeList.push_back(pOperator);

    // wenn es sich um eine Vergleichsoperation auf datum/Zeit handelt, dann
    // erfolgt jetzt bereits eine Umwandlung fuer die Konstante
//  if (pOb)
//  {
//      switch (pPredicateNode->getChild(2)->getNodeType())
//      {
//          case SQL_NODE_STRING:
//          {
//              OOperandConst* pConst = PTR_CAST(OOperandConst,m_aCodeList[m_aCodeList.size() - 2]);
//              switch (pOb->getDBType())
//              {
//                  case DataType::DECIMAL:
//                  case DataType::NUMERIC:
//                  case DataType::REAL:
//                  case DataType::DOUBLE:
//                  case DataType::TIMESTAMP:
//                  case DataType::DATE:
//                  case DataType::TIME:
//                  {
//                      try
//                      {
//                          pConst->setValue(makeAny(pConst->getValue().getDouble()));
//                      }
//                      catch( Exception&)
//                      {
//                          ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Datatype mismatch"),NULL);
//                      }
//                  }   break;
//                  case DataType::TINYINT:
//                  case DataType::SMALLINT:
//                  case DataType::INTEGER:
//                  case DataType::BIT:
//                      ;
//                      ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Datatype mismatch"),NULL);
//              }
//          }
//      }
//  }
    return NULL;
}

//------------------------------------------------------------------
OOperand* OPredicateCompiler::execute_LIKE(OSQLParseNode* pPredicateNode) throw(SQLException, RuntimeException)
{
    DBG_ASSERT(pPredicateNode->count() >= 4,"OFILECursor: Fehler im Parse Tree");

    if (!SQL_ISRULE(pPredicateNode->getChild(0),column_ref))
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
        return NULL;
    }

    OSQLPredicateType ePredicateType;
    OSQLParseNode *pAtom;
    OSQLParseNode *pOptEscape;
    sal_Unicode cEscape = L'\0';

    if (pPredicateNode->count() == 5)
        ePredicateType = SQL_PRED_NOTLIKE;
    else
        ePredicateType = SQL_PRED_LIKE;

    pAtom       = pPredicateNode->getChild(pPredicateNode->count()-2);
    pOptEscape  = pPredicateNode->getChild(pPredicateNode->count()-1);

    if (!(pAtom->getNodeType() == SQL_NODE_STRING || SQL_ISRULE(pAtom,parameter)))
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
        return NULL;
    }
    if (pOptEscape->count() != 0)
    {
        if (pOptEscape->count() != 2)
        {
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
            return NULL;
        }
        OSQLParseNode *pEscNode = pOptEscape->getChild(1);
        if (pEscNode->getNodeType() != SQL_NODE_STRING)
        {
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
            return NULL;
        }
        else
            cEscape = pEscNode->getTokenValue().toChar();
    }

    OOperand* pOb = execute(pPredicateNode->getChild(0));
    OOperand* pOperand  = execute(pAtom);
    OBoolOperator* pOperator = (ePredicateType == SQL_PRED_LIKE)
                                                    ? new OOp_LIKE(cEscape)
                                                    : new OOp_NOTLIKE(cEscape);
    m_aCodeList.push_back(pOperator);
    return NULL;
}

//------------------------------------------------------------------
OOperand* OPredicateCompiler::execute_ISNULL(OSQLParseNode* pPredicateNode) throw(SQLException, RuntimeException)
{
    if (!SQL_ISRULE(pPredicateNode->getChild(0),column_ref))
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
        return NULL;
    }

    DBG_ASSERT(pPredicateNode->count() >= 3,"OFILECursor: Fehler im Parse Tree");
    DBG_ASSERT(SQL_ISTOKEN(pPredicateNode->getChild(1),IS),"OFILECursor: Fehler im Parse Tree")

    OSQLPredicateType ePredicateType;
    if (SQL_ISTOKEN(pPredicateNode->getChild(2),NOT))
        ePredicateType = SQL_PRED_ISNOTNULL;
    else
        ePredicateType = SQL_PRED_ISNULL;

    OOperand* pOb = execute(pPredicateNode->getChild(0));
    OBoolOperator* pOperator = (ePredicateType == SQL_PRED_ISNULL) ?
                                new OOp_ISNULL() : new OOp_ISNOTNULL();

    //pOb->PreProcess(pOperator);


    m_aCodeList.push_back(pOperator);
    return NULL;
}
//------------------------------------------------------------------
OOperand* OPredicateCompiler::execute_Operand(OSQLParseNode* pPredicateNode) throw(SQLException, RuntimeException)
{
    OOperand* pOperand = NULL;

    if (SQL_ISRULE(pPredicateNode,column_ref))
    {
        ::rtl::OUString aColumnName;
        if (pPredicateNode->count() == 1)
        {
            aColumnName = pPredicateNode->getChild(0)->getTokenValue();
        }
        else if (pPredicateNode->count() == 3)
        {
            ::rtl::OUString aTableName = pPredicateNode->getChild(0)->getTokenValue();
            if(SQL_ISRULE(pPredicateNode->getChild(2),column_val))
                aColumnName = pPredicateNode->getChild(2)->getChild(0)->getTokenValue();
            else
                aColumnName = pPredicateNode->getChild(2)->getTokenValue();
        }

        if(!m_orgColumns->hasByName(aColumnName))
        {
            ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Column not found: ");
            sMsg += aColumnName;
            throw SQLException(sMsg,NULL,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
        }
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCol;
        try
        {
            if (m_orgColumns->getByName(aColumnName) >>= xCol)
            {
                pOperand = m_pAnalyzer->createOperandAttr(Reference< XColumnLocate>(m_orgColumns,UNO_QUERY)->findColumn(aColumnName),xCol,m_xIndexes);
            }
            else
            {// Column existiert nicht im Resultset
                ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
            }
        }
        catch(Exception &)
        {
            OSL_ENSURE(0,"OPredicateCompiler::execute_Operand Exception");
        }
    }
    else if (SQL_ISRULE(pPredicateNode,parameter))
    {
        pOperand = new OOperandParam(pPredicateNode, ++m_nParamCounter);
    }
    else if (pPredicateNode->getNodeType() == SQL_NODE_STRING ||
             pPredicateNode->getNodeType() == SQL_NODE_INTNUM ||
             pPredicateNode->getNodeType() == SQL_NODE_APPROXNUM ||
             pPredicateNode->getNodeType() == SQL_NODE_NAME ||
             SQL_ISTOKEN(pPredicateNode,TRUE) ||
             SQL_ISTOKEN(pPredicateNode,FALSE) ||
             SQL_ISRULE(pPredicateNode,parameter))
    {
        pOperand = new OOperandConst(*pPredicateNode, pPredicateNode->getTokenValue());
    }
    else if((pPredicateNode->count() == 2) &&
            (SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"+") || SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"-")) &&
            pPredicateNode->getChild(1)->getNodeType() == SQL_NODE_INTNUM)
    { // falls -1 bzw. +1 vorhanden ist
        ::rtl::OUString aValue(pPredicateNode->getChild(0)->getTokenValue());
        aValue += pPredicateNode->getChild(1)->getTokenValue();
        pOperand = new OOperandConst(*pPredicateNode->getChild(1), aValue);
    }
    else if( SQL_ISRULE(pPredicateNode,set_fct_spec) && SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"{") )
    {
        const OSQLParseNode* pODBCNode = pPredicateNode->getChild(1);
        const OSQLParseNode* pODBCNodeChild = pODBCNode->getChild(0);

        // Odbc Date or time
        if (pODBCNodeChild->getNodeType() == SQL_NODE_KEYWORD && (
            SQL_ISTOKEN(pODBCNodeChild,D) ||
            SQL_ISTOKEN(pODBCNodeChild,T) ||
            SQL_ISTOKEN(pODBCNodeChild,TS) ))
        {
            ::rtl::OUString sDateTime = pODBCNode->getChild(1)->getTokenValue();
            pOperand = new OOperandConst(*pODBCNode->getChild(1), sDateTime);
            if(SQL_ISTOKEN(pODBCNodeChild,D))
            {
                pOperand->setValue(::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(sDateTime)));
            }
            else if(SQL_ISTOKEN(pODBCNodeChild,T))
            {
                pOperand->setValue(::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(sDateTime)));
            }
            else if(SQL_ISTOKEN(pODBCNodeChild,TS))
            {
                pOperand->setValue(::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDateTime(sDateTime)));
            }
        }
        else
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Statement to complex"),NULL);

    }
    else if( SQL_ISRULE(pPredicateNode,fold) )
    {
        execute_Fold(pPredicateNode);
    }
    else if(    SQL_ISRULE(pPredicateNode,set_fct_spec)
            ||  SQL_ISRULE(pPredicateNode,position_exp)
            ||  SQL_ISRULE(pPredicateNode,char_substring_fct)
            )
    {
        executeFunction(pPredicateNode);
    }
    else if( SQL_ISRULE(pPredicateNode,length_exp) )
    {
        executeFunction(pPredicateNode->getChild(0));
    }
    else
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Statement to complex"),NULL);
    }
    if (pOperand)
        m_aCodeList.push_back(pOperand);
    return pOperand;
}

////////////////////////////////////////////////////////////////////////////////////////
sal_Bool OPredicateInterpreter::evaluate(OCodeList& rCodeList)
{
    static sal_Bool bResult;

    OCodeList::iterator aIter = rCodeList.begin();
    if (!(*aIter))
        return sal_True;        // kein Prädikat

    for(;aIter != rCodeList.end();++aIter)
    {
        OOperand* pOperand = PTR_CAST(OOperand,(*aIter));
        if (pOperand)
            m_aStack.push(pOperand);
        else
            ((OOperator *)(*aIter))->Exec(m_aStack);
    }

    OOperand* pOperand = m_aStack.top();
    m_aStack.pop();

    DBG_ASSERT(m_aStack.size() == 0, "StackFehler");
    DBG_ASSERT(pOperand, "StackFehler");

    bResult = pOperand->isValid();
    if (IS_TYPE(OOperandResult,pOperand))
        delete pOperand;
    return bResult;
}
// -----------------------------------------------------------------------------
void OPredicateInterpreter::evaluateSelection(OCodeList& rCodeList,ORowSetValueDecoratorRef& _rVal)
{
    OCodeList::iterator aIter = rCodeList.begin();
    if (!(*aIter))
        return ;        // kein Prädikat

    for(;aIter != rCodeList.end();++aIter)
    {
        OOperand* pOperand = PTR_CAST(OOperand,(*aIter));
        if (pOperand)
            m_aStack.push(pOperand);
        else
            ((OOperator *)(*aIter))->Exec(m_aStack);
    }

    OOperand* pOperand = m_aStack.top();
    m_aStack.pop();

    DBG_ASSERT(m_aStack.size() == 0, "StackFehler");
    DBG_ASSERT(pOperand, "StackFehler");

    (*_rVal) = pOperand->getValue();
    if (IS_TYPE(OOperandResult,pOperand))
        delete pOperand;
}
// -----------------------------------------------------------------------------
OOperand* OPredicateCompiler::execute_Fold(OSQLParseNode* pPredicateNode)   throw(SQLException, RuntimeException)
{
    /*if (!SQL_ISRULE(pPredicateNode->getChild(0),column_ref))
    {
        ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Invalid Statement"),NULL);
        return NULL;
    }*/

    DBG_ASSERT(pPredicateNode->count() >= 4,"OFILECursor: Fehler im Parse Tree");

    sal_Bool bUpper = SQL_ISTOKEN(pPredicateNode->getChild(0),UPPER);

    OOperand* pOb = execute(pPredicateNode->getChild(2));
    OOperator* pOperator = NULL;
    if ( bUpper )
        pOperator = new OOp_Upper();
    else
        pOperator = new OOp_Lower();

    //pOb->PreProcess(pOperator);


    m_aCodeList.push_back(pOperator);
    return NULL;
}
// -----------------------------------------------------------------------------
OOperand* OPredicateCompiler::executeFunction(OSQLParseNode* pPredicateNode)    throw(SQLException, RuntimeException)
{
    OOperator* pOperator = NULL;

    OSL_ENSURE(pPredicateNode->getChild(0)->isToken(),"The first one must be the name of the function!");
    sal_Int32 nTokenId = pPredicateNode->getChild(0)->getTokenID();
    switch ( nTokenId )
    {
        case SQL_TOKEN_CHAR_LENGTH:
        case SQL_TOKEN_CHARACTER_LENGTH:
        case SQL_TOKEN_LENGTH:
        case SQL_TOKEN_OCTET_LENGTH:
        case SQL_TOKEN_ASCII:
        case SQL_TOKEN_LCASE:
        case SQL_TOKEN_LTRIM:
        case SQL_TOKEN_RTRIM:
        case SQL_TOKEN_SPACE:
        case SQL_TOKEN_UCASE:
        case SQL_TOKEN_ABS:
        case SQL_TOKEN_ACOS:
        case SQL_TOKEN_ASIN:
        case SQL_TOKEN_ATAN:
        case SQL_TOKEN_CEILING:
        case SQL_TOKEN_COS:
        case SQL_TOKEN_DEGREES:
        case SQL_TOKEN_EXP:
        case SQL_TOKEN_FLOOR:
        case SQL_TOKEN_LOG10:
        case SQL_TOKEN_RADIANS:
        case SQL_TOKEN_SIGN:
        case SQL_TOKEN_SIN:
        case SQL_TOKEN_SQRT:
        case SQL_TOKEN_TAN:
        case SQL_TOKEN_DAYNAME:
        case SQL_TOKEN_DAYOFMONTH:
        case SQL_TOKEN_DAYOFWEEK:
        case SQL_TOKEN_DAYOFYEAR:
        case SQL_TOKEN_HOUR:
        case SQL_TOKEN_MINUTE:
        case SQL_TOKEN_MONTH:
        case SQL_TOKEN_MONTHNAME:
        case SQL_TOKEN_QUARTER:
        case SQL_TOKEN_SECOND:
        case SQL_TOKEN_YEAR:

            execute(pPredicateNode->getChild(2));

            switch( nTokenId )
            {
                case SQL_TOKEN_CHAR_LENGTH:
                case SQL_TOKEN_CHARACTER_LENGTH:
                case SQL_TOKEN_LENGTH:
                case SQL_TOKEN_OCTET_LENGTH:
                    pOperator = new OOp_CharLength();
                    break;
                case SQL_TOKEN_ASCII:
                    pOperator = new OOp_Ascii();
                    break;
                case SQL_TOKEN_LCASE:
                    pOperator = new OOp_Lower();
                    break;

                case SQL_TOKEN_LTRIM:
                    pOperator = new OOp_LTrim();
                    break;
                case SQL_TOKEN_RTRIM:
                    pOperator = new OOp_RTrim();
                    break;
                case SQL_TOKEN_SPACE:
                    pOperator = new OOp_Space();
                    break;
                case SQL_TOKEN_UCASE:
                    pOperator = new OOp_Upper();
                    break;
                case SQL_TOKEN_ABS:
                    pOperator = new OOp_Abs();
                    break;
                case SQL_TOKEN_ACOS:
                    pOperator = new OOp_ACos();
                    break;
                case SQL_TOKEN_ASIN:
                    pOperator = new OOp_ASin();
                    break;
                case SQL_TOKEN_ATAN:
                    pOperator = new OOp_ATan();
                    break;
                case SQL_TOKEN_CEILING:
                    pOperator = new OOp_Ceiling();
                    break;
                case SQL_TOKEN_COS:
                    pOperator = new OOp_Cos();
                    break;
                case SQL_TOKEN_DEGREES:
                    pOperator = new OOp_Degrees();
                    break;
                case SQL_TOKEN_EXP:
                    pOperator = new OOp_Exp();
                    break;
                case SQL_TOKEN_FLOOR:
                    pOperator = new OOp_Floor();
                    break;
                case SQL_TOKEN_LOG10:
                    pOperator = new OOp_Log10();
                    break;
                case SQL_TOKEN_RADIANS:
                    pOperator = new OOp_Radians();
                    break;
                case SQL_TOKEN_SIGN:
                    pOperator = new OOp_Sign();
                    break;
                case SQL_TOKEN_SIN:
                    pOperator = new OOp_Sin();
                    break;
                case SQL_TOKEN_SQRT:
                    pOperator = new OOp_Sqrt();
                    break;
                case SQL_TOKEN_TAN:
                    pOperator = new OOp_Tan();
                    break;
                case SQL_TOKEN_DAYOFWEEK:
                    pOperator = new OOp_DayOfWeek();
                    break;
                case SQL_TOKEN_DAYOFMONTH:
                    pOperator = new OOp_DayOfMonth();
                    break;
                case SQL_TOKEN_DAYOFYEAR:
                    pOperator = new OOp_DayOfYear();
                    break;
                case SQL_TOKEN_MONTH:
                    pOperator = new OOp_Month();
                    break;
                case SQL_TOKEN_DAYNAME:
                    pOperator = new OOp_DayName();
                    break;
                case SQL_TOKEN_MONTHNAME:
                    pOperator = new OOp_MonthName();
                    break;
                case SQL_TOKEN_QUARTER:
                    pOperator = new OOp_Quarter();
                    break;
                case SQL_TOKEN_YEAR:
                    pOperator = new OOp_Year();
                    break;
                case SQL_TOKEN_HOUR:
                    pOperator = new OOp_Hour();
                    break;
                case SQL_TOKEN_MINUTE:
                    pOperator = new OOp_Minute();
                    break;
                case SQL_TOKEN_SECOND:
                    pOperator = new OOp_Second();
                    break;
                default:
                    OSL_ENSURE(0,"Error in switch!");
            }
            break;
        case SQL_TOKEN_CHAR:
        case SQL_TOKEN_CONCAT:
        case SQL_TOKEN_INSERT:
        case SQL_TOKEN_LEFT:
        case SQL_TOKEN_LOCATE:
        case SQL_TOKEN_LOCATE_2:
        case SQL_TOKEN_REPEAT:
        case SQL_TOKEN_REPLACE:
        case SQL_TOKEN_RIGHT:
        case SQL_TOKEN_MOD:
        case SQL_TOKEN_ROUND:
        case SQL_TOKEN_LOGF:
        case SQL_TOKEN_POWER:
        case SQL_TOKEN_ATAN2:
        case SQL_TOKEN_PI:
        case SQL_TOKEN_CURDATE:
        case SQL_TOKEN_CURTIME:
        case SQL_TOKEN_NOW:
        case SQL_TOKEN_WEEK:
            {
                m_aCodeList.push_back(new OStopOperand);
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (sal_Int32 i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));

                switch( nTokenId )
                {
                    case SQL_TOKEN_CHAR:
                        pOperator = new OOp_Char();
                        break;
                    case SQL_TOKEN_CONCAT:
                        pOperator = new OOp_Concat();
                        break;
                    case SQL_TOKEN_INSERT:
                        pOperator = new OOp_Insert();
                        break;
                    case SQL_TOKEN_LEFT:
                        pOperator = new OOp_Left();
                        break;
                    case SQL_TOKEN_LOCATE:
                    case SQL_TOKEN_LOCATE_2:
                        pOperator = new OOp_Locate();
                        break;
                    case SQL_TOKEN_REPEAT:
                        pOperator = new OOp_Repeat();
                        break;
                    case SQL_TOKEN_REPLACE:
                        pOperator = new OOp_Replace();
                        break;
                    case SQL_TOKEN_RIGHT:
                        pOperator = new OOp_Right();
                        break;
                    case SQL_TOKEN_MOD:
                        pOperator = new OOp_Mod();
                        break;
                    case SQL_TOKEN_ROUND:
                        pOperator = new OOp_Round();
                        break;
                    case SQL_TOKEN_LOGF:
                        pOperator = new OOp_Log();
                        break;
                    case SQL_TOKEN_POWER:
                        pOperator = new OOp_Pow();
                        break;
                    case SQL_TOKEN_ATAN2:
                        pOperator = new OOp_ATan2();
                        break;
                    case SQL_TOKEN_PI:
                        pOperator = new OOp_Pi();
                        break;
                    case SQL_TOKEN_CURDATE:
                        pOperator = new OOp_CurDate();
                        break;
                    case SQL_TOKEN_CURTIME:
                        pOperator = new OOp_CurTime();
                        break;
                    case SQL_TOKEN_NOW:
                        pOperator = new OOp_Now();
                        break;
                    case SQL_TOKEN_WEEK:
                        pOperator = new OOp_Week();
                        break;
                    default:
                        OSL_ENSURE(0,"Error in switch!");
                }
            }
            break;

        case SQL_TOKEN_SUBSTRING:
            m_aCodeList.push_back(new OStopOperand);
            if ( pPredicateNode->count() == 4 ) //char_substring_fct
            {
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (sal_Int32 i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));
            }
            else
            {
                execute(pPredicateNode->getChild(2));
                execute(pPredicateNode->getChild(4));
                execute(pPredicateNode->getChild(5)->getChild(1));
            }
            pOperator = new OOp_SubString();
            break;

        case SQL_TOKEN_POSITION:
            m_aCodeList.push_back(new OStopOperand);
            if ( pPredicateNode->count() == 4 ) //position_exp
            {
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (sal_Int32 i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));
            }
            else
            {
                execute(pPredicateNode->getChild(2));
                execute(pPredicateNode->getChild(4));
            }
            pOperator = new OOp_Locate();
            break;
        default:
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Function not supported, yet."),NULL);
    }

    m_aCodeList.push_back(pOperator);
    return NULL;
}
// -----------------------------------------------------------------------------


