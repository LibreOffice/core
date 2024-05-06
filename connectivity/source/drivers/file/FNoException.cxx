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

#include <file/FCatalog.hxx>
#include <file/fcomp.hxx>
#include <file/fanalyzer.hxx>
#include <file/FResultSet.hxx>
#include <file/FPreparedStatement.hxx>
#include <connectivity/FValue.hxx>
#include <tools/debug.hxx>
#include <TKeyValue.hxx>

using namespace connectivity;
using namespace connectivity::file;

void OFileCatalog::refreshViews()
{}
void OFileCatalog::refreshGroups()
{}
void OFileCatalog::refreshUsers()
{
}

OPredicateInterpreter::~OPredicateInterpreter()
{
    while(!m_aStack.empty())
    {
        delete m_aStack.top();
        m_aStack.pop();
    }
    //  m_aStack.clear();
}

void OPredicateCompiler::Clean()
{
    m_aCodeList.clear();
}

void OSQLAnalyzer::bindParameterRow(OValueRefRow const & _pRow)
{
    OCodeList& rCodeList    = m_aCompiler->m_aCodeList;
    for (auto const& code : rCodeList)
    {
        OOperandParam* pParam = dynamic_cast<OOperandParam*>(code.get());
        if ( pParam )
            pParam->bindValue(_pRow);
    }
}

void OPreparedStatement::scanParameter(OSQLParseNode* pParseNode,std::vector< OSQLParseNode*>& _rParaNodes)
{
    assert(pParseNode && "OResultSet: internal error: invalid ParseNode");

    // found parameter Name-Rule?
    if (SQL_ISRULE(pParseNode,parameter))
    {
        DBG_ASSERT(pParseNode->count() >= 1,"OResultSet: faulty Parse Tree");
        DBG_ASSERT(pParseNode->getChild(0)->getNodeType() == SQLNodeType::Punctuation,"OResultSet: faulty Parse Tree");

        _rParaNodes.push_back(pParseNode);
        // Further descend not necessary
        return;
    }

    // Further descend in Parse Tree
    for (size_t i = 0; i < pParseNode->count(); i++)
        scanParameter(pParseNode->getChild(i),_rParaNodes);
}

std::unique_ptr<OKeyValue> OResultSet::GetOrderbyKeyValue(OValueRefRow const & _rRow)
{
    sal_uInt32 nBookmarkValue = std::abs((*_rRow)[0]->getValue().getInt32());

    std::unique_ptr<OKeyValue> pKeyValue = OKeyValue::createKeyValue(nBookmarkValue);

    for (auto const& elem : m_aOrderbyColumnNumber)
    {
        OSL_ENSURE(elem < static_cast<sal_Int32>(_rRow->size()),"Invalid index for orderkey values!");
        pKeyValue->pushKey(new ORowSetValueDecorator((*_rRow)[elem]->getValue()));
    }

    return pKeyValue;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
