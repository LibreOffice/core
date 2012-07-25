/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */


#include "rtfexport.hxx"

#include <rtl/oustringostreaminserter.hxx>
#include <svtools/rtfkeywd.hxx>

SmRtfExport::SmRtfExport(const SmNode* pIn)
    : m_pTree(pIn)
      , m_pBuffer(0)
{
}

bool SmRtfExport::ConvertFromStarMath(OStringBuffer& rBuffer)
{
    if (m_pTree == NULL)
        return false;
    m_pBuffer = &rBuffer;
    m_pBuffer->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE "\\moMath");
    HandleNode(m_pTree, 0);
    m_pBuffer->append("}");
    return true;
}

// NOTE: This is still work in progress and unfinished, but it already covers a good
// part of the rtf math stuff.

void SmRtfExport::HandleNode(const SmNode* pNode, int nLevel)
{
    SAL_INFO("starmath.rtf", "Node: " << nLevel << " " << int(pNode->GetType()) << " " << pNode->GetNumSubNodes());

    switch(pNode->GetType())
    {
        case NTEXT:
            HandleText(pNode,nLevel);
            break;
        case NBINHOR:
            HandleBinaryOperation(static_cast<const SmBinHorNode*>(pNode), nLevel);
            break;
        case NBINVER:
            HandleFractions(pNode, nLevel);
            break;
        case NMATH:
            HandleMath(pNode, nLevel);
            break;
        case NEXPRESSION:
            HandleAllSubNodes(pNode, nLevel);
            break;
        case NTABLE:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode,nLevel);
            break;
        case NLINE:
            HandleAllSubNodes(pNode, nLevel);
            break;
        default:
            SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC << " unhandled node type");
            break;
    }
}

//Root Node, PILE equivalent, i.e. vertical stack
void SmRtfExport::HandleTable(const SmNode* pNode, int nLevel)
{
    if (nLevel || pNode->GetNumSubNodes() > 1)
        HandleVerticalStack(pNode, nLevel);
    else
        HandleAllSubNodes(pNode, nLevel);
}

void SmRtfExport::HandleAllSubNodes(const SmNode* pNode, int nLevel)
{
    int size = pNode->GetNumSubNodes();
    for (int i = 0; i < size; ++i)
    {
        if (!pNode->GetSubNode(i))
        {
            OSL_FAIL("Subnode is NULL, parent node not handled properly");
            continue;
        }
        HandleNode(pNode->GetSubNode(i), nLevel + 1);
    }
}

void SmRtfExport::HandleVerticalStack(const SmNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleText(const SmNode* pNode, int /*nLevel*/)
{
    m_pBuffer->append("{\\mr ");

    SmTextNode* pTemp=(SmTextNode* )pNode;
    SAL_INFO("starmath.rtf", "Text: " << pTemp->GetText());
    for (xub_StrLen i = 0; i < pTemp->GetText().Len(); i++)
    {
        sal_uInt16 nChar = pTemp->GetText().GetChar(i);
        // TODO special/non-ascii chars?
        m_pBuffer->append(OUStringToOString(OUString(SmTextNode::ConvertSymbolToUnicode(nChar)), RTL_TEXTENCODING_UTF8));
    }

    m_pBuffer->append("}");
}

void SmRtfExport::HandleFractions(const SmNode* pNode, int nLevel, const char* type)
{
    m_pBuffer->append("{\\mf ");
    if (type)
    {
        m_pBuffer->append("{\\mfPr ");
        m_pBuffer->append("{\\mtype ");
        m_pBuffer->append(type);
        m_pBuffer->append("}"); // mtype
        m_pBuffer->append("}"); // mfPr
    }
    OSL_ASSERT(pNode->GetNumSubNodes() == 3);
    m_pBuffer->append("{\\mnum ");
    HandleNode(pNode->GetSubNode(0), nLevel + 1);
    m_pBuffer->append("}"); // mnum
    m_pBuffer->append("{\\mden ");
    HandleNode(pNode->GetSubNode(2), nLevel + 1);
    m_pBuffer->append("}"); // mden
    m_pBuffer->append("}"); // mf
}

void SmRtfExport::HandleUnaryOperation(const SmUnHorNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleBinaryOperation(const SmBinHorNode* pNode, int nLevel)
{
    SAL_INFO("starmath.rtf", "Binary: " << int(pNode->Symbol()->GetToken().eType));
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

void SmRtfExport::HandleAttribute(const SmAttributNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleMath(const SmNode* pNode, int nLevel)
{
    SAL_INFO("starmath.rtf", "Math: " << int(pNode->GetToken().eType));
    switch (pNode->GetToken().eType)
    {
        case TDIVIDEBY:
        case TACUTE:
            // these are handled elsewhere, e.g. when handling BINHOR
            OSL_ASSERT(false);
        default:
            HandleText(pNode, nLevel);
            break;
    }
}

void SmRtfExport::HandleRoot(const SmRootNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleOperator(const SmOperNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleSubSupScript(const SmSubSupNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleSubSupScriptInternal(const SmSubSupNode* /*pNode*/, int /*nLevel*/, int /*flags*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleMatrix(const SmMatrixNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleBrace(const SmBraceNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

void SmRtfExport::HandleVerticalBrace(const SmVerticalBraceNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
