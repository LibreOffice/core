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

#include <file/fanalyzer.hxx>
#include <connectivity/sqlparse.hxx>
#include <tools/debug.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/dbexception.hxx>
#include <file/FConnection.hxx>
#include <strings.hrc>

using namespace ::connectivity;
using namespace ::connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

OSQLAnalyzer::OSQLAnalyzer(OConnection* _pConnection)
               :m_pConnection(_pConnection)
               ,m_bHasSelectionCode(false)
               ,m_bSelectionFirstTime(true)
{
    m_aCompiler = new OPredicateCompiler(this);
    m_aInterpreter = new OPredicateInterpreter(m_aCompiler);
}


OSQLAnalyzer::~OSQLAnalyzer()
{
}


void OSQLAnalyzer::start(OSQLParseNode const * pSQLParseNode)
{
    if (SQL_ISRULE(pSQLParseNode,select_statement))
    {
        DBG_ASSERT(pSQLParseNode->count() >= 4,"OFILECursor: Error in Parse Tree");

        // check that we don't use anything other than count(*) as function
        OSQLParseNode* pSelection = pSQLParseNode->getChild(2);
        if ( SQL_ISRULE(pSelection,scalar_exp_commalist) )
        {
            for (size_t i = 0; i < pSelection->count(); i++)
            {
                OSQLParseNode *pColumnRef = pSelection->getChild(i)->getChild(0);
                if (    ( SQL_ISRULE(pColumnRef,set_fct_spec) && pColumnRef->count() == 4 )
                    ||  SQL_ISRULE(pColumnRef,char_value_fct)
                    ||  SQL_ISRULE(pColumnRef,char_substring_fct)
                    ||  SQL_ISRULE(pColumnRef,position_exp)
                    ||  SQL_ISRULE(pColumnRef,fold)
                    ||  SQL_ISRULE(pColumnRef,length_exp)
                    ||  SQL_ISRULE(pColumnRef,num_value_exp)
                    ||  SQL_ISRULE(pColumnRef,term)
                    ||  SQL_ISRULE(pColumnRef,factor)
                    ||  SQL_ISRULE(pColumnRef,set_fct_spec) )
                {
                    ::rtl::Reference<OPredicateCompiler>        pCompiler = new OPredicateCompiler(this);
                    pCompiler->setOrigColumns(m_aCompiler->getOrigColumns());
                    ::rtl::Reference<OPredicateInterpreter> pInterpreter = new OPredicateInterpreter(pCompiler);
                    pCompiler->execute( pColumnRef );
                    m_aSelectionEvaluations.push_back( TPredicates(pCompiler,pInterpreter) );
                }
                else if ( SQL_ISRULE(pColumnRef,general_set_fct) && pColumnRef->count() != 4 )
                {
                    m_pConnection->throwGenericSQLException(STR_QUERY_COMPLEX_COUNT,nullptr);
                }
                else
                {
                    if  (   SQL_ISPUNCTUATION( pColumnRef, "*" )
                        ||  (   SQL_ISRULE( pColumnRef, column_ref )
                            &&  ( pColumnRef->count() == 3 )
                            &&  ( pColumnRef->getChild(0)->getNodeType() == SQLNodeType::Name )
                            &&  SQL_ISPUNCTUATION( pColumnRef->getChild(1), "." )
                            &&  SQL_ISRULE( pColumnRef->getChild(2), column_val )
                            &&  SQL_ISPUNCTUATION( pColumnRef->getChild(2)->getChild(0), "*" )
                            )
                        )
                    {
                        // push one element for each column of our table
                        const Reference< XNameAccess > xColumnNames( m_aCompiler->getOrigColumns() );
                        const Sequence< OUString > aColumnNames( xColumnNames->getElementNames() );
                        for ( sal_Int32 j=0; j<aColumnNames.getLength(); ++j )
                            m_aSelectionEvaluations.push_back( TPredicates() );
                    }
                    else
                        m_aSelectionEvaluations.push_back( TPredicates() );
                }
            }
        }
    }

    m_aCompiler->start(pSQLParseNode);
}


void OSQLAnalyzer::bindRow(OCodeList& rCodeList,const OValueRefRow& _pRow)
{
    for (auto const& code : rCodeList)
    {
        OOperandAttr* pAttr = dynamic_cast<OOperandAttr*>(code.get());
        if (pAttr)
        {
            pAttr->bindValue(_pRow);
        }
    }
}

void OSQLAnalyzer::bindSelectRow(const OValueRefRow& _pRow)
{
    // first the select part
    for (auto const& selectionEval : m_aSelectionEvaluations)
    {
        if ( selectionEval.first.is() )
            bindRow(selectionEval.first->m_aCodeList,_pRow);
    }
}

void OSQLAnalyzer::bindEvaluationRow(OValueRefRow const & _pRow)
{
    bindRow(m_aCompiler->m_aCodeList,_pRow);
}

OOperandAttr* OSQLAnalyzer::createOperandAttr(sal_Int32 _nPos,
                                              const Reference< XPropertySet>& _xCol)
{
    return new OOperandAttr(static_cast<sal_uInt16>(_nPos),_xCol);
}

bool OSQLAnalyzer::hasRestriction() const
{
    return m_aCompiler->hasCode();
}

bool OSQLAnalyzer::hasFunctions() const
{
    if ( m_bSelectionFirstTime )
    {
        m_bSelectionFirstTime = false;
        for (auto const& selectionEval : m_aSelectionEvaluations)
        {
            if ( selectionEval.first.is() )
            {
                m_bHasSelectionCode = selectionEval.first->hasCode();
                if (m_bHasSelectionCode)
                    break;
            }
        }
    }
    return m_bHasSelectionCode;
}

void OSQLAnalyzer::setSelectionEvaluationResult(OValueRefRow const & _pRow,const std::vector<sal_Int32>& _rColumnMapping)
{
    sal_Int32 nPos = 1;
    for (auto const& selectionEval : m_aSelectionEvaluations)
    {
        if ( selectionEval.second.is() )
        {
            // the first column (index 0) is for convenience only. The first real select column is no 1.
            sal_Int32   map = nPos;
            if ( nPos < static_cast< sal_Int32 >( _rColumnMapping.size() ) )
                map = _rColumnMapping[nPos];
            if ( map > 0 )
                selectionEval.second->startSelection( (_pRow->get())[map] );
        }
        ++nPos;
    }
}

void OSQLAnalyzer::dispose()
{
    m_aCompiler->dispose();
    for (auto const& selectionEval : m_aSelectionEvaluations)
    {
        if ( selectionEval.first.is() )
            selectionEval.first->dispose();
    }
}

void OSQLAnalyzer::setOrigColumns(const css::uno::Reference< css::container::XNameAccess>& rCols)
{
    m_aCompiler->setOrigColumns(rCols);
    for (auto const& selectionEval : m_aSelectionEvaluations)
    {
        if ( selectionEval.first.is() )
            selectionEval.first->setOrigColumns(rCols);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
