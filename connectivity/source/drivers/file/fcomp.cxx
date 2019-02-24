/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <file/fcomp.hxx>
#include <tools/debug.hxx>
#include <TConnection.hxx>
#include <connectivity/sqlparse.hxx>
#include <file/fanalyzer.hxx>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <file/FStringFunctions.hxx>
#include <file/FDateFunctions.hxx>
#include <file/FNumericFunctions.hxx>
#include <file/FConnection.hxx>
#include <sqlbison.hxx>
#include <strings.hrc>

using namespace connectivity;
using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace com::sun::star;

OPredicateCompiler::OPredicateCompiler(OSQLAnalyzer* pAnalyzer)//,OCursor& rCurs)
                     : m_pAnalyzer(pAnalyzer)
                     , m_nParamCounter(0)
{
}


OPredicateCompiler::~OPredicateCompiler()
{
    Clean();
}

void OPredicateCompiler::dispose()
{
    Clean();
    m_orgColumns        = nullptr;
}

void OPredicateCompiler::start(OSQLParseNode const * pSQLParseNode)
{
    if (!pSQLParseNode)
        return;

    m_nParamCounter = 0;
    // analyse Parse Tree (depending on Statement-type)
    // and set pointer on WHERE-clause:
    OSQLParseNode * pWhereClause = nullptr;

    if (SQL_ISRULE(pSQLParseNode,select_statement))
    {
        OSQLParseNode * pOrderbyClause = nullptr;
        DBG_ASSERT(pSQLParseNode->count() >= 4,"OFILECursor: Error in Parse Tree");

        OSQLParseNode * pTableExp = pSQLParseNode->getChild(3);
        DBG_ASSERT(pTableExp != nullptr,"Error in Parse Tree");
        DBG_ASSERT(SQL_ISRULE(pTableExp,table_exp)," Error in Parse Tree");
        DBG_ASSERT(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"Error in Parse Tree");

        // check that we don't use anything other than count(*) as function
        OSQLParseNode* pSelection = pSQLParseNode->getChild(2);
        if ( SQL_ISRULE(pSelection,scalar_exp_commalist) )
        {
            for (size_t i = 0; i < pSelection->count(); i++)
            {
                OSQLParseNode *pColumnRef = pSelection->getChild(i)->getChild(0);
                if ( SQL_ISRULE(pColumnRef,general_set_fct) && pColumnRef->count() != 4 )
                {
                    m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_COMPLEX_COUNT,nullptr);
                }
            }
        }


        pWhereClause    = pTableExp->getChild(1);
        pOrderbyClause  = pTableExp->getChild(ORDER_BY_CHILD_POS);
        (void)pOrderbyClause;
    }
    else if (SQL_ISRULE(pSQLParseNode,update_statement_searched))
    {
        DBG_ASSERT(pSQLParseNode->count() == 5,"OFILECursor: Error in Parse Tree");
        pWhereClause = pSQLParseNode->getChild(4);
    }
    else if (SQL_ISRULE(pSQLParseNode,delete_statement_searched))
    {
        DBG_ASSERT(pSQLParseNode->count() == 4,"Error in Parse Tree");
        pWhereClause = pSQLParseNode->getChild(3);
    }
    else
            // Other Statement. no selection-criteria
        return;

    if (SQL_ISRULE(pWhereClause,where_clause))
    {
        // a where-clause is not allowed to be empty:
        DBG_ASSERT(pWhereClause->count() == 2,"OFILECursor: Error in Parse Tree");

        OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
        DBG_ASSERT(pComparisonPredicate != nullptr,"OFILECursor: Error in Parse Tree");

        execute( pComparisonPredicate );
    }
    else
    {
        // The where-clause is optionally in the majority of cases, i.e. it might be an "optional-where-clause".
        DBG_ASSERT(SQL_ISRULE(pWhereClause,opt_where_clause),"OPredicateCompiler: Error in Parse Tree");
    }
}


OOperand* OPredicateCompiler::execute(OSQLParseNode const * pPredicateNode)
{
    OOperand* pOperand = nullptr;
    if (pPredicateNode->count() == 3 &&                         // Expression is bracketed
        SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pPredicateNode->getChild(2),")"))
    {
        execute(pPredicateNode->getChild(1));
    }
    else if ((SQL_ISRULE(pPredicateNode,search_condition) || SQL_ISRULE(pPredicateNode,boolean_term))
                            &&          // AND/OR-linkage:
                            pPredicateNode->count() == 3)
    {
        execute(pPredicateNode->getChild(0));                           // process the left branch
        execute(pPredicateNode->getChild(2));                           // process the right branch

        if (SQL_ISTOKEN(pPredicateNode->getChild(1),OR))                // OR-Operator
        {
            m_aCodeList.emplace_back(new OOp_OR);
        }
        else if (SQL_ISTOKEN(pPredicateNode->getChild(1),AND))      // AND-Operator
            m_aCodeList.emplace_back(new OOp_AND);
        else
        {
            OSL_FAIL("OPredicateCompiler: Error in Parse Tree");
        }
    }
    else if (SQL_ISRULE(pPredicateNode,boolean_factor))
    {
        execute(pPredicateNode->getChild(1));
        m_aCodeList.emplace_back(new OOp_NOT);
    }
    else if (SQL_ISRULE(pPredicateNode,comparison_predicate))
    {
        execute_COMPARE(pPredicateNode);
    }
    else if (SQL_ISRULE(pPredicateNode,like_predicate))
    {
        execute_LIKE(pPredicateNode);
    }
    else if (SQL_ISRULE(pPredicateNode,between_predicate))
    {
        execute_BETWEEN(pPredicateNode);
    }
    else if (SQL_ISRULE(pPredicateNode,test_for_null))
    {
        execute_ISNULL(pPredicateNode);
    }
    else if(SQL_ISRULE(pPredicateNode,num_value_exp))
    {
        execute(pPredicateNode->getChild(0));                           // process the left branch
        execute(pPredicateNode->getChild(2));                           // process the right branch
        if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"+"))
        {
            m_aCodeList.emplace_back(new OOp_ADD);
        }
        else if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"-"))
            m_aCodeList.emplace_back(new OOp_SUB);
        else
        {
            OSL_FAIL("OPredicateCompiler: Error in Parse Tree num_value_exp");
        }
    }
    else if(SQL_ISRULE(pPredicateNode,term))
    {
        execute(pPredicateNode->getChild(0));                           // process the left branch
        execute(pPredicateNode->getChild(2));                           // process the right branch
        if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"*"))
        {
            m_aCodeList.emplace_back(new OOp_MUL);
        }
        else if (SQL_ISPUNCTUATION(pPredicateNode->getChild(1),"/"))
            m_aCodeList.emplace_back(new OOp_DIV);
        else
        {
            OSL_FAIL("OPredicateCompiler: Error in Parse Tree num_value_exp");
        }
    }
    else
        pOperand = execute_Operand(pPredicateNode);                     // now only simple operands will be processed

    return pOperand;
}


void OPredicateCompiler::execute_COMPARE(OSQLParseNode const * pPredicateNode)
{
    DBG_ASSERT(pPredicateNode->count() == 3,"OFILECursor: Error in Parse Tree");

    if ( !(SQL_ISRULE(pPredicateNode->getChild(0),column_ref)               ||
          pPredicateNode->getChild(2)->getNodeType() == SQLNodeType::String     ||
          pPredicateNode->getChild(2)->getNodeType() == SQLNodeType::IntNum     ||
          pPredicateNode->getChild(2)->getNodeType() == SQLNodeType::ApproxNum  ||
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
        m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_TOO_COMPLEX,nullptr);
        return;
    }

    sal_Int32 ePredicateType( SQLFilterOperator::EQUAL );
    OSQLParseNode *pPrec = pPredicateNode->getChild(1);

    if (pPrec->getNodeType() == SQLNodeType::Equal)
        ePredicateType = SQLFilterOperator::EQUAL;
    else if (pPrec->getNodeType() == SQLNodeType::NotEqual)
        ePredicateType = SQLFilterOperator::NOT_EQUAL;
    else if (pPrec->getNodeType() == SQLNodeType::Less)
        ePredicateType = SQLFilterOperator::LESS;
    else if (pPrec->getNodeType() == SQLNodeType::LessEq)
        ePredicateType = SQLFilterOperator::LESS_EQUAL;
    else if (pPrec->getNodeType() == SQLNodeType::GreatEq)
        ePredicateType = SQLFilterOperator::GREATER_EQUAL;
    else if (pPrec->getNodeType() == SQLNodeType::Great)
        ePredicateType = SQLFilterOperator::GREATER;
    else
        OSL_FAIL( "OPredicateCompiler::execute_COMPARE: unexpected node type!" );

    execute(pPredicateNode->getChild(0));
    execute(pPredicateNode->getChild(2));
    m_aCodeList.emplace_back( new OOp_COMPARE(ePredicateType) );
}


void OPredicateCompiler::execute_LIKE(OSQLParseNode const * pPredicateNode)
{
    DBG_ASSERT(pPredicateNode->count() == 2,"OFILECursor: Error in Parse Tree");
    const OSQLParseNode* pPart2 = pPredicateNode->getChild(1);

    sal_Unicode cEscape = L'\0';
    const bool bNotLike = pPart2->getChild(0)->isToken();

    OSQLParseNode* pAtom        = pPart2->getChild(pPart2->count()-2);
    OSQLParseNode* pOptEscape   = pPart2->getChild(pPart2->count()-1);

    if (!(pAtom->getNodeType() == SQLNodeType::String   ||
          SQL_ISRULE(pAtom,parameter)               ||
          // odbc date
          SQL_ISRULE(pAtom,set_fct_spec)            ||
          SQL_ISRULE(pAtom,position_exp)            ||
          SQL_ISRULE(pAtom,char_substring_fct)      ||
          // upper, lower etc.
          SQL_ISRULE(pAtom,fold)) )
    {
        m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_TOO_COMPLEX,nullptr);
        return;
    }

    if (pOptEscape->count() != 0)
    {
        if (pOptEscape->count() != 2)
        {
            m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_INVALID_LIKE_STRING,nullptr);
        }
        OSQLParseNode *pEscNode = pOptEscape->getChild(1);
        if (pEscNode->getNodeType() != SQLNodeType::String)
        {
            m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_INVALID_LIKE_STRING,nullptr);
        }
        else
            cEscape = pEscNode->getTokenValue().toChar();
    }

    execute(pPredicateNode->getChild(0));
    execute(pAtom);

    OBoolOperator* pOperator = bNotLike
                                    ? new OOp_NOTLIKE(cEscape)
                                    : new OOp_LIKE(cEscape);
    m_aCodeList.emplace_back(pOperator);
}

void OPredicateCompiler::execute_BETWEEN(OSQLParseNode const * pPredicateNode)
{
    DBG_ASSERT(pPredicateNode->count() == 2,"OFILECursor: Error in Parse Tree");

    OSQLParseNode* pColumn = pPredicateNode->getChild(0);
    const OSQLParseNode* pPart2 = pPredicateNode->getChild(1);
    OSQLParseNode* p1stValue = pPart2->getChild(2);
    OSQLParseNode* p2ndtValue = pPart2->getChild(4);

    if (
            !(p1stValue->getNodeType() == SQLNodeType::String || SQL_ISRULE(p1stValue,parameter))
        &&  !(p2ndtValue->getNodeType() == SQLNodeType::String || SQL_ISRULE(p2ndtValue,parameter))
        )
    {
        m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_INVALID_BETWEEN,nullptr);
    }

    bool bNot = SQL_ISTOKEN(pPart2->getChild(0),NOT);

    OOperand* pColumnOp = execute(pColumn);
    OOperand* pOb1 = execute(p1stValue);
    OBoolOperator* pOperator = new OOp_COMPARE(bNot ? SQLFilterOperator::LESS_EQUAL : SQLFilterOperator::GREATER);
    m_aCodeList.emplace_back(pOperator);

    execute(pColumn);
    OOperand* pOb2 = execute(p2ndtValue);
    pOperator = new OOp_COMPARE(bNot ? SQLFilterOperator::GREATER_EQUAL : SQLFilterOperator::LESS);
    m_aCodeList.emplace_back(pOperator);

    if ( pColumnOp && pOb1 && pOb2 )
    {
        switch(pColumnOp->getDBType())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                pOb1->setValue(pOb1->getValue().getString());
                pOb2->setValue(pOb2->getValue().getString());
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                pOb1->setValue(static_cast<double>(pOb1->getValue()));
                pOb2->setValue(static_cast<double>(pOb2->getValue()));
                break;
            case DataType::FLOAT:
                pOb1->setValue(static_cast<float>(pOb1->getValue()));
                pOb2->setValue(static_cast<float>(pOb2->getValue()));
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                pOb1->setValue(static_cast<double>(pOb1->getValue()));
                pOb2->setValue(static_cast<double>(pOb2->getValue()));
                break;
            case DataType::DATE:
                pOb1->setValue(static_cast<util::Date>(pOb1->getValue()));
                pOb2->setValue(static_cast<util::Date>(pOb2->getValue()));
                break;
            case DataType::TIME:
                pOb1->setValue(static_cast<util::Time>(pOb1->getValue()));
                pOb2->setValue(static_cast<util::Time>(pOb2->getValue()));
                break;
            case DataType::TIMESTAMP:
                pOb1->setValue(static_cast<util::DateTime>(pOb1->getValue()));
                pOb2->setValue(static_cast<util::DateTime>(pOb2->getValue()));
                break;
        }
    }


    OBoolOperator* pBoolOp = nullptr;
    if ( bNot )
        pBoolOp = new OOp_OR;
    else
        pBoolOp = new OOp_AND;
    m_aCodeList.emplace_back(pBoolOp);
}

void OPredicateCompiler::execute_ISNULL(OSQLParseNode const * pPredicateNode)
{
    DBG_ASSERT(pPredicateNode->count() == 2,"OFILECursor: Error in Parse Tree");
    const OSQLParseNode* pPart2 = pPredicateNode->getChild(1);
    DBG_ASSERT(SQL_ISTOKEN(pPart2->getChild(0),IS),"OFILECursor: Error in Parse Tree");

    sal_Int32 ePredicateType;
    if (SQL_ISTOKEN(pPart2->getChild(1),NOT))
        ePredicateType = SQLFilterOperator::NOT_SQLNULL;
    else
        ePredicateType = SQLFilterOperator::SQLNULL;

    execute(pPredicateNode->getChild(0));
    OBoolOperator* pOperator = (ePredicateType == SQLFilterOperator::SQLNULL) ?
                                new OOp_ISNULL : new OOp_ISNOTNULL;
    m_aCodeList.emplace_back(pOperator);
}

OOperand* OPredicateCompiler::execute_Operand(OSQLParseNode const * pPredicateNode)
{
    OOperand* pOperand = nullptr;

    if (SQL_ISRULE(pPredicateNode,column_ref))
    {
        OUString aColumnName;
        if (pPredicateNode->count() == 1)
        {
            aColumnName = pPredicateNode->getChild(0)->getTokenValue();
        }
        else if (pPredicateNode->count() == 3)
        {
            if(SQL_ISRULE(pPredicateNode->getChild(2),column_val))
                aColumnName = pPredicateNode->getChild(2)->getChild(0)->getTokenValue();
            else
                aColumnName = pPredicateNode->getChild(2)->getTokenValue();
        }

        if(!m_orgColumns->hasByName(aColumnName))
        {
            const OUString sError( m_pAnalyzer->getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_INVALID_COLUMNNAME,
                    "$columnname$", aColumnName
                 ) );
            ::dbtools::throwGenericSQLException( sError, nullptr );
        }
        css::uno::Reference< css::beans::XPropertySet> xCol;
        try
        {
            if (m_orgColumns->getByName(aColumnName) >>= xCol)
            {
                pOperand = OSQLAnalyzer::createOperandAttr(Reference< XColumnLocate>(m_orgColumns,UNO_QUERY)->findColumn(aColumnName),xCol);
            }
            else
            {// Column doesn't exist in the Result-set
                const OUString sError( m_pAnalyzer->getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_INVALID_COLUMNNAME,
                    "$columnname$", aColumnName
                 ) );
                ::dbtools::throwGenericSQLException( sError, nullptr );
            }
        }
        catch(Exception &)
        {
            OSL_FAIL("OPredicateCompiler::execute_Operand Exception");
        }
    }
    else if (SQL_ISRULE(pPredicateNode,parameter))
    {
        pOperand = new OOperandParam(pPredicateNode, ++m_nParamCounter);
    }
    else if (pPredicateNode->getNodeType() == SQLNodeType::String ||
             pPredicateNode->getNodeType() == SQLNodeType::IntNum ||
             pPredicateNode->getNodeType() == SQLNodeType::ApproxNum ||
             pPredicateNode->getNodeType() == SQLNodeType::Name ||
             SQL_ISTOKEN(pPredicateNode,TRUE) ||
             SQL_ISTOKEN(pPredicateNode,FALSE) ||
             SQL_ISRULE(pPredicateNode,parameter))
    {
        pOperand = new OOperandConst(*pPredicateNode, pPredicateNode->getTokenValue());
    }
    else if((pPredicateNode->count() == 2) &&
            (SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"+") || SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"-")) &&
            pPredicateNode->getChild(1)->getNodeType() == SQLNodeType::IntNum)
    { // if -1 or +1 is there
        OUString aValue = pPredicateNode->getChild(0)->getTokenValue() + pPredicateNode->getChild(1)->getTokenValue();
        pOperand = new OOperandConst(*pPredicateNode->getChild(1), aValue);
    }
    else if( SQL_ISRULE(pPredicateNode,set_fct_spec) && SQL_ISPUNCTUATION(pPredicateNode->getChild(0),"{") )
    {
        const OSQLParseNode* pODBCNode = pPredicateNode->getChild(1);
        const OSQLParseNode* pODBCNodeChild = pODBCNode->getChild(0);

        // Odbc Date or time
        if (pODBCNodeChild->getNodeType() == SQLNodeType::Keyword && (
            SQL_ISTOKEN(pODBCNodeChild,D) ||
            SQL_ISTOKEN(pODBCNodeChild,T) ||
            SQL_ISTOKEN(pODBCNodeChild,TS) ))
        {
            OUString sDateTime = pODBCNode->getChild(1)->getTokenValue();
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
            m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_TOO_COMPLEX,nullptr);

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
        m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_TOO_COMPLEX,nullptr);
    }
    if (pOperand)
        m_aCodeList.emplace_back(pOperand);
    return pOperand;
}


bool OPredicateInterpreter::evaluate(OCodeList& rCodeList)
{
    static bool bResult;

    if (!(rCodeList[0]))
        return true;        // no Predicate

    for (auto const& code : rCodeList)
    {
        OOperand* pOperand = dynamic_cast<OOperand* >(code.get());
        if (pOperand)
            m_aStack.push(pOperand);
        else
            static_cast<OOperator *>(code.get())->Exec(m_aStack);
    }

    OOperand* pOperand = m_aStack.top();
    m_aStack.pop();

    DBG_ASSERT(m_aStack.empty(), "Stack error");
    DBG_ASSERT(pOperand, "Stack error");

    bResult = pOperand->isValid();
    if (typeid(OOperandResult) == typeid(*pOperand))
        delete pOperand;
    return bResult;
}

void OPredicateInterpreter::evaluateSelection(OCodeList& rCodeList, ORowSetValueDecoratorRef const & _rVal)
{
    if (!(rCodeList[0]))
        return ;        // no Predicate

    for (auto const& code : rCodeList)
    {
        OOperand* pOperand = dynamic_cast<OOperand* >(code.get());
        if (pOperand)
            m_aStack.push(pOperand);
        else
            static_cast<OOperator *>(code.get())->Exec(m_aStack);
    }

    OOperand* pOperand = m_aStack.top();
    m_aStack.pop();

    DBG_ASSERT(m_aStack.empty(), "Stack error");
    DBG_ASSERT(pOperand, "Stack error");

    (*_rVal) = pOperand->getValue();
    if (typeid(OOperandResult) == typeid(*pOperand))
        delete pOperand;
}

void OPredicateCompiler::execute_Fold(OSQLParseNode const * pPredicateNode)
{
    DBG_ASSERT(pPredicateNode->count() >= 4,"OFILECursor: Error in Parse Tree");

    bool bUpper = SQL_ISTOKEN(pPredicateNode->getChild(0),UPPER);

    execute(pPredicateNode->getChild(2));
    OOperator* pOperator = nullptr;
    if ( bUpper )
        pOperator = new OOp_Upper;
    else
        pOperator = new OOp_Lower;

    m_aCodeList.emplace_back(pOperator);
}

void OPredicateCompiler::executeFunction(OSQLParseNode const * pPredicateNode)
{
    OOperator* pOperator = nullptr;

    OSL_ENSURE(pPredicateNode->getChild(0)->isToken(),"The first one must be the name of the function!");
    sal_Int32 nTokenId = pPredicateNode->getChild(0)->getTokenID();
    switch ( nTokenId )
    {
        case SQL_TOKEN_CHAR_LENGTH:
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
        case SQL_TOKEN_LN:
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
                case SQL_TOKEN_LENGTH:
                case SQL_TOKEN_OCTET_LENGTH:
                    pOperator = new OOp_CharLength;
                    break;
                case SQL_TOKEN_ASCII:
                    pOperator = new OOp_Ascii;
                    break;
                case SQL_TOKEN_LCASE:
                    pOperator = new OOp_Lower;
                    break;

                case SQL_TOKEN_LTRIM:
                    pOperator = new OOp_LTrim;
                    break;
                case SQL_TOKEN_RTRIM:
                    pOperator = new OOp_RTrim;
                    break;
                case SQL_TOKEN_SPACE:
                    pOperator = new OOp_Space;
                    break;
                case SQL_TOKEN_UCASE:
                    pOperator = new OOp_Upper;
                    break;
                case SQL_TOKEN_ABS:
                    pOperator = new OOp_Abs;
                    break;
                case SQL_TOKEN_ACOS:
                    pOperator = new OOp_ACos;
                    break;
                case SQL_TOKEN_ASIN:
                    pOperator = new OOp_ASin;
                    break;
                case SQL_TOKEN_ATAN:
                    pOperator = new OOp_ATan;
                    break;
                case SQL_TOKEN_CEILING:
                    pOperator = new OOp_Ceiling;
                    break;
                case SQL_TOKEN_COS:
                    pOperator = new OOp_Cos;
                    break;
                case SQL_TOKEN_DEGREES:
                    pOperator = new OOp_Degrees;
                    break;
                case SQL_TOKEN_EXP:
                    pOperator = new OOp_Exp;
                    break;
                case SQL_TOKEN_FLOOR:
                    pOperator = new OOp_Floor;
                    break;
                case SQL_TOKEN_LOG10:
                    pOperator = new OOp_Log10;
                    break;
                case SQL_TOKEN_LN:
                    pOperator = new OOp_Ln;
                    break;
                case SQL_TOKEN_RADIANS:
                    pOperator = new OOp_Radians;
                    break;
                case SQL_TOKEN_SIGN:
                    pOperator = new OOp_Sign;
                    break;
                case SQL_TOKEN_SIN:
                    pOperator = new OOp_Sin;
                    break;
                case SQL_TOKEN_SQRT:
                    pOperator = new OOp_Sqrt;
                    break;
                case SQL_TOKEN_TAN:
                    pOperator = new OOp_Tan;
                    break;
                case SQL_TOKEN_DAYOFWEEK:
                    pOperator = new OOp_DayOfWeek;
                    break;
                case SQL_TOKEN_DAYOFMONTH:
                    pOperator = new OOp_DayOfMonth;
                    break;
                case SQL_TOKEN_DAYOFYEAR:
                    pOperator = new OOp_DayOfYear;
                    break;
                case SQL_TOKEN_MONTH:
                    pOperator = new OOp_Month;
                    break;
                case SQL_TOKEN_DAYNAME:
                    pOperator = new OOp_DayName;
                    break;
                case SQL_TOKEN_MONTHNAME:
                    pOperator = new OOp_MonthName;
                    break;
                case SQL_TOKEN_QUARTER:
                    pOperator = new OOp_Quarter;
                    break;
                case SQL_TOKEN_YEAR:
                    pOperator = new OOp_Year;
                    break;
                case SQL_TOKEN_HOUR:
                    pOperator = new OOp_Hour;
                    break;
                case SQL_TOKEN_MINUTE:
                    pOperator = new OOp_Minute;
                    break;
                case SQL_TOKEN_SECOND:
                    pOperator = new OOp_Second;
                    break;
                default:
                    OSL_FAIL("Error in switch!");
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
        case SQL_TOKEN_LOG:
        case SQL_TOKEN_POWER:
        case SQL_TOKEN_ATAN2:
        case SQL_TOKEN_PI:
        case SQL_TOKEN_CURDATE:
        case SQL_TOKEN_CURTIME:
        case SQL_TOKEN_NOW:
        case SQL_TOKEN_WEEK:
            {
                m_aCodeList.emplace_back(new OStopOperand);
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (size_t i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));

                switch( nTokenId )
                {
                    case SQL_TOKEN_CHAR:
                        pOperator = new OOp_Char;
                        break;
                    case SQL_TOKEN_CONCAT:
                        pOperator = new OOp_Concat;
                        break;
                    case SQL_TOKEN_INSERT:
                        pOperator = new OOp_Insert;
                        break;
                    case SQL_TOKEN_LEFT:
                        pOperator = new OOp_Left;
                        break;
                    case SQL_TOKEN_LOCATE:
                    case SQL_TOKEN_LOCATE_2:
                        pOperator = new OOp_Locate;
                        break;
                    case SQL_TOKEN_REPEAT:
                        pOperator = new OOp_Repeat;
                        break;
                    case SQL_TOKEN_REPLACE:
                        pOperator = new OOp_Replace;
                        break;
                    case SQL_TOKEN_RIGHT:
                        pOperator = new OOp_Right;
                        break;
                    case SQL_TOKEN_MOD:
                        pOperator = new OOp_Mod;
                        break;
                    case SQL_TOKEN_ROUND:
                        pOperator = new OOp_Round;
                        break;
                    case SQL_TOKEN_LOGF:
                    case SQL_TOKEN_LOG:
                        pOperator = new OOp_Log;
                        break;
                    case SQL_TOKEN_POWER:
                        pOperator = new OOp_Pow;
                        break;
                    case SQL_TOKEN_ATAN2:
                        pOperator = new OOp_ATan2;
                        break;
                    case SQL_TOKEN_PI:
                        pOperator = new OOp_Pi;
                        break;
                    case SQL_TOKEN_CURDATE:
                        pOperator = new OOp_CurDate;
                        break;
                    case SQL_TOKEN_CURTIME:
                        pOperator = new OOp_CurTime;
                        break;
                    case SQL_TOKEN_NOW:
                        pOperator = new OOp_Now;
                        break;
                    case SQL_TOKEN_WEEK:
                        pOperator = new OOp_Week;
                        break;
                    default:
                        OSL_FAIL("Error in switch!");
                }
            }
            break;

        case SQL_TOKEN_SUBSTRING:
            m_aCodeList.emplace_back(new OStopOperand);
            if ( pPredicateNode->count() == 4 ) //char_substring_fct
            {
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (size_t i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));
            }
            else
            {
                execute(pPredicateNode->getChild(2));
                execute(pPredicateNode->getChild(4));
                execute(pPredicateNode->getChild(5)->getChild(1));
            }
            pOperator = new OOp_SubString;
            break;

        case SQL_TOKEN_POSITION:
            m_aCodeList.emplace_back(new OStopOperand);
            if ( pPredicateNode->count() == 4 ) //position_exp
            {
                OSQLParseNode* pList = pPredicateNode->getChild(2);
                for (size_t i=0; i < pList->count(); ++i)
                    execute(pList->getChild(i));
            }
            else
            {
                execute(pPredicateNode->getChild(2));
                execute(pPredicateNode->getChild(4));
            }
            pOperator = new OOp_Locate;
            break;
        default:
            m_pAnalyzer->getConnection()->throwGenericSQLException(STR_QUERY_FUNCTION_NOT_SUPPORTED,nullptr);
    }

    m_aCodeList.emplace_back(pOperator);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
