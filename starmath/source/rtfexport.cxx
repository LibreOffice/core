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
#include <filter/msfilter/rtfutil.hxx>

SmRtfExport::SmRtfExport(const SmNode* pIn)
    : m_pTree(pIn)
      , m_pBuffer(0)
{
}

bool SmRtfExport::ConvertFromStarMath(OStringBuffer& rBuffer)
{
    if (!m_pTree)
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
        case NATTRIBUT:
            HandleAttribute( static_cast< const SmAttributNode* >( pNode ), nLevel );
            break;
        case NTEXT:
            HandleText(pNode,nLevel);
            break;
        case NBRACE:
            HandleBrace( static_cast< const SmBraceNode* >( pNode ), nLevel );
            break;
        case NOPER:
            HandleOperator(static_cast<const SmOperNode*>(pNode), nLevel);
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
        OUString aValue(SmTextNode::ConvertSymbolToUnicode(nChar));
        m_pBuffer->append(msfilter::rtfutil::OutString(aValue, RTL_TEXTENCODING_MS_1252));
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

void SmRtfExport::HandleAttribute(const SmAttributNode* pNode, int nLevel)
{
    switch (pNode->Attribute()->GetToken().eType)
    {
        case TCHECK:
        case TACUTE:
        case TGRAVE:
        case TBREVE:
        case TCIRCLE:
        case TVEC:
        case TTILDE:
        case THAT:
        case TDOT:
        case TDDOT:
        case TDDDOT:
        case TWIDETILDE:
        case TWIDEHAT:
        case TWIDEVEC:
        case TBAR:
        {
            m_pBuffer->append("{\\macc ");
            m_pBuffer->append("{\\maccPr ");
            m_pBuffer->append("{\\mchr ");
            OUString aValue(pNode->Attribute()->GetToken().cMathChar);
            m_pBuffer->append(msfilter::rtfutil::OutString(aValue, RTL_TEXTENCODING_MS_1252));
            m_pBuffer->append("}"); // mchr
            m_pBuffer->append("}"); // maccPr
            m_pBuffer->append("{\\me ");
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pBuffer->append("}"); // me
            m_pBuffer->append("}"); // macc
            break;
        }
        case TOVERLINE:
        case TUNDERLINE:
            m_pBuffer->append("{\\mbar ");
            m_pBuffer->append("{\\mbarPr ");
            m_pBuffer->append("{\\mpos ");
            m_pBuffer->append((pNode->Attribute()->GetToken().eType == TUNDERLINE ) ? "bot" : "top");
            m_pBuffer->append("}"); // mpos
            m_pBuffer->append("}"); // mbarPr
            m_pBuffer->append("{\\me ");
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pBuffer->append("}"); // me
            m_pBuffer->append("}"); // mbar
            break;
        case TOVERSTRIKE:
            m_pBuffer->append("{\\mborderBox ");
            m_pBuffer->append("{\\mborderBoxPr ");
            m_pBuffer->append("{\\mhideTop 1}");
            m_pBuffer->append("{\\mhideBot 1}");
            m_pBuffer->append("{\\mhideLeft 1}");
            m_pBuffer->append("{\\mhideRight 1}");
            m_pBuffer->append("{\\mstrikeH 1}");
            m_pBuffer->append("}"); // mborderBoxPr
            m_pBuffer->append("{\\me ");
            HandleNode( pNode->Body(), nLevel + 1 );
            m_pBuffer->append("}"); // me
            m_pBuffer->append("}"); // mborderBox
            break;
        default:
            HandleAllSubNodes( pNode, nLevel );
            break;
    }
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

namespace {
OString mathSymbolToString(const SmNode* node)
{
    assert(node->GetType() == NMATH);
    const SmTextNode* txtnode = static_cast<const SmTextNode*>(node);
    assert(txtnode->GetText().Len() == 1);
    sal_Unicode chr = SmTextNode::ConvertSymbolToUnicode(txtnode->GetText().GetChar(0));
    OUString aValue(chr);
    return msfilter::rtfutil::OutString(aValue, RTL_TEXTENCODING_MS_1252);
}
}

void SmRtfExport::HandleOperator(const SmOperNode* pNode, int nLevel)
{
    SAL_INFO("starmath.rtf", "Operator: " << int(pNode->GetToken().eType));
    switch (pNode->GetToken().eType)
    {
        case TINT:
        case TIINT:
        case TIIINT:
        case TLINT:
        case TLLINT:
        case TLLLINT:
        case TPROD:
        case TCOPROD:
        case TSUM:
        {
            const SmSubSupNode* subsup = pNode->GetSubNode(0)->GetType() == NSUBSUP ? static_cast<const SmSubSupNode*>(pNode->GetSubNode(0)) : 0;
            const SmNode* operation = subsup ? subsup->GetBody() : pNode->GetSubNode(0);
            m_pBuffer->append("{\\mnary ");
            m_pBuffer->append("{\\mnaryPr ");
            m_pBuffer->append("{\\mchr ");
            m_pBuffer->append(mathSymbolToString(operation));
            m_pBuffer->append("}"); // mchr
            if (!subsup || !subsup->GetSubSup(CSUB))
                m_pBuffer->append("{\\msubHide 1}");
            if (!subsup || !subsup->GetSubSup(CSUP))
                m_pBuffer->append("{\\msupHide 1}");
            m_pBuffer->append("}"); // mnaryPr
            if (!subsup || !subsup->GetSubSup(CSUB))
                m_pBuffer->append("{\\msub }");
            else
            {
                m_pBuffer->append("{\\msub ");
                HandleNode(subsup->GetSubSup(CSUB), nLevel + 1);
                m_pBuffer->append("}"); // msub
            }
            if (!subsup || !subsup->GetSubSup( CSUP ))
                m_pBuffer->append("{\\msup }");
            else
            {
                m_pBuffer->append("{\\msup ");
                HandleNode(subsup->GetSubSup(CSUP), nLevel + 1);
                m_pBuffer->append("}"); // msup
            }
            m_pBuffer->append("{\\me ");
            HandleNode(pNode->GetSubNode(1), nLevel + 1); // body
            m_pBuffer->append("}"); // me
            m_pBuffer->append("}"); // mnary
            break;
        }
        case TLIM:
            m_pBuffer->append("{\\mfunc ");
            m_pBuffer->append("{\\mfName ");
            m_pBuffer->append("{\\mlimLow ");
            m_pBuffer->append("{\\me ");
            HandleNode(pNode->GetSymbol(), nLevel + 1);
            m_pBuffer->append("}"); // me
            m_pBuffer->append("{\\mlim ");
            if (const SmSubSupNode* subsup = pNode->GetSubNode(0)->GetType() == NSUBSUP ? static_cast<const SmSubSupNode*>( pNode->GetSubNode(0)) : 0)
                if (subsup->GetSubSup(CSUB))
                    HandleNode(subsup->GetSubSup(CSUB), nLevel + 1);
            m_pBuffer->append("}"); // mlim
            m_pBuffer->append("}"); // mlimLow
            m_pBuffer->append("}"); // mfName
            m_pBuffer->append("{\\me ");
            HandleNode(pNode->GetSubNode(1), nLevel + 1); // body
            m_pBuffer->append("}"); // me
            m_pBuffer->append("}"); // mfunc
            break;
        default:
            SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC << " unhandled oper type");
            break;
    }
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

void SmRtfExport::HandleBrace(const SmBraceNode* pNode, int nLevel)
{
    m_pBuffer->append("{\\md ");
    m_pBuffer->append("{\\mdPr ");
    m_pBuffer->append("{\\mbegChr ");
    m_pBuffer->append(mathSymbolToString(pNode->OpeningBrace()));
    m_pBuffer->append("}"); // mbegChr
    std::vector< const SmNode* > subnodes;
    if (pNode->Body()->GetType() == NBRACEBODY)
    {
        const SmBracebodyNode* body = static_cast<const SmBracebodyNode*>( pNode->Body());
        bool separatorWritten = false; // assume all separators are the same
        for (int i = 0; i < body->GetNumSubNodes(); ++i)
        {
            const SmNode* subnode = body->GetSubNode(i);
            if (subnode->GetType() == NMATH)
            { // do not write, but write what separator it is
                const SmMathSymbolNode* math = static_cast<const SmMathSymbolNode*>(subnode);
                if(!separatorWritten)
                {
                    m_pBuffer->append("{\\msepChr ");
                    m_pBuffer->append(mathSymbolToString(math));
                    m_pBuffer->append("}"); // msepChr
                    separatorWritten = true;
                }
            }
            else
                subnodes.push_back(subnode);
        }
    }
    else
        subnodes.push_back(pNode->Body());
    m_pBuffer->append("{\\mendChr ");
    m_pBuffer->append(mathSymbolToString(pNode->ClosingBrace()));
    m_pBuffer->append("}"); // mendChr
    m_pBuffer->append("}"); // mdPr
    for (unsigned int i = 0; i < subnodes.size(); ++i)
    {
        m_pBuffer->append("{\\me ");
        HandleNode(subnodes[ i ], nLevel + 1);
        m_pBuffer->append("}"); // me
    }
    m_pBuffer->append("}"); // md
}

void SmRtfExport::HandleVerticalBrace(const SmVerticalBraceNode* /*pNode*/, int /*nLevel*/)
{
    SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
