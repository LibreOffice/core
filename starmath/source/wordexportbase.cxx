/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "wordexportbase.hxx"
#include <node.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

SmWordExportBase::SmWordExportBase(const SmNode* pIn)
    : m_pTree(pIn)
{
}

SmWordExportBase::~SmWordExportBase() = default;

void SmWordExportBase::HandleNode(const SmNode* pNode, int nLevel)
{
    SAL_INFO("starmath.wordbase",
             "Node: " << nLevel << " " << int(pNode->GetType()) << " " << pNode->GetNumSubNodes());
    switch (pNode->GetType())
    {
        case SmNodeType::Attribute:
            HandleAttribute(static_cast<const SmAttributeNode*>(pNode), nLevel);
            break;
        case SmNodeType::Text:
            HandleText(pNode, nLevel);
            break;
        case SmNodeType::VerticalBrace:
            HandleVerticalBrace(static_cast<const SmVerticalBraceNode*>(pNode), nLevel);
            break;
        case SmNodeType::Brace:
            HandleBrace(static_cast<const SmBraceNode*>(pNode), nLevel);
            break;
        case SmNodeType::Oper:
            HandleOperator(static_cast<const SmOperNode*>(pNode), nLevel);
            break;
        case SmNodeType::UnHor:
            HandleUnaryOperation(static_cast<const SmUnHorNode*>(pNode), nLevel);
            break;
        case SmNodeType::BinHor:
            HandleBinaryOperation(static_cast<const SmBinHorNode*>(pNode), nLevel);
            break;
        case SmNodeType::BinVer:
            HandleFractions(pNode, nLevel, nullptr);
            break;
        case SmNodeType::Root:
            HandleRoot(static_cast<const SmRootNode*>(pNode), nLevel);
            break;
        case SmNodeType::Special:
        {
            auto pText = static_cast<const SmTextNode*>(pNode);
            //if the token str and the result text are the same then this
            //is to be seen as text, else assume it's a mathchar
            if (pText->GetText() == pText->GetToken().aText)
                HandleText(pText, nLevel);
            else
                HandleMath(pText, nLevel);
            break;
        }
        case SmNodeType::Math:
        case SmNodeType::MathIdent:
            HandleMath(pNode, nLevel);
            break;
        case SmNodeType::SubSup:
            HandleSubSupScript(static_cast<const SmSubSupNode*>(pNode), nLevel);
            break;
        case SmNodeType::Expression:
            HandleAllSubNodes(pNode, nLevel);
            break;
        case SmNodeType::Table:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode, nLevel);
            break;
        case SmNodeType::Matrix:
            HandleMatrix(static_cast<const SmMatrixNode*>(pNode), nLevel);
            break;
        case SmNodeType::Line:
        {
            // TODO
            HandleAllSubNodes(pNode, nLevel);
        }
        break;
#if 0
    case SmNodeType::Align:
        HandleMAlign(pNode,nLevel);
        break;
#endif
        case SmNodeType::Place:
            // explicitly do nothing, MSOffice treats that as a placeholder if item is missing
            break;
        case SmNodeType::Blank:
            HandleBlank();
            break;
        default:
            HandleAllSubNodes(pNode, nLevel);
            break;
    }
}

//Root Node, PILE equivalent, i.e. vertical stack
void SmWordExportBase::HandleTable(const SmNode* pNode, int nLevel)
{
    //The root of the starmath is a table, if
    //we convert this them each iteration of
    //conversion from starmath to Word will
    //add an extra unnecessary level to the
    //Word output stack which would grow
    //without bound in a multi step conversion
    if (nLevel || pNode->GetNumSubNodes() > 1)
        HandleVerticalStack(pNode, nLevel);
    else
        HandleAllSubNodes(pNode, nLevel);
}

void SmWordExportBase::HandleAllSubNodes(const SmNode* pNode, int nLevel)
{
    int size = pNode->GetNumSubNodes();
    for (int i = 0; i < size; ++i)
    {
        // TODO remove when all types of nodes are handled properly
        if (pNode->GetSubNode(i) == nullptr)
        {
            SAL_WARN("starmath.wordbase", "Subnode is NULL, parent node not handled properly");
            continue;
        }
        HandleNode(pNode->GetSubNode(i), nLevel + 1);
    }
}

void SmWordExportBase::HandleUnaryOperation(const SmUnHorNode* pNode, int nLevel)
{
    // update HandleMath() when adding new items
    SAL_INFO("starmath.wordbase", "Unary: " << int(pNode->GetToken().eType));

    HandleAllSubNodes(pNode, nLevel);
}

void SmWordExportBase::HandleBinaryOperation(const SmBinHorNode* pNode, int nLevel)
{
    SAL_INFO("starmath.wordbase", "Binary: " << int(pNode->Symbol()->GetToken().eType));
    // update HandleMath() when adding new items
    switch (pNode->Symbol()->GetToken().eType)
    {
        case TDIVIDEBY:
            return HandleFractions(pNode, nLevel, "lin");
        default:
            HandleAllSubNodes(pNode, nLevel);
            break;
    }
}

void SmWordExportBase::HandleMath(const SmNode* pNode, int nLevel)
{
    SAL_INFO("starmath.wordbase", "Math: " << int(pNode->GetToken().eType));
    switch (pNode->GetToken().eType)
    {
        case TDIVIDEBY:
        case TACUTE:
            OSL_ASSERT(false);
            [[fallthrough]]; // the above are handled elsewhere, e.g. when handling BINHOR
        default:
            HandleText(pNode, nLevel);
            break;
    }
}

void SmWordExportBase::HandleSubSupScript(const SmSubSupNode* pNode, int nLevel)
{
    // set flags to a bitfield of which sub/sup items exists
    int flags = (pNode->GetSubSup(CSUB) != nullptr ? (1 << CSUB) : 0)
                | (pNode->GetSubSup(CSUP) != nullptr ? (1 << CSUP) : 0)
                | (pNode->GetSubSup(RSUB) != nullptr ? (1 << RSUB) : 0)
                | (pNode->GetSubSup(RSUP) != nullptr ? (1 << RSUP) : 0)
                | (pNode->GetSubSup(LSUB) != nullptr ? (1 << LSUB) : 0)
                | (pNode->GetSubSup(LSUP) != nullptr ? (1 << LSUP) : 0);
    HandleSubSupScriptInternal(pNode, nLevel, flags);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
