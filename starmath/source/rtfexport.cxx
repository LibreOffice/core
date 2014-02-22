/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */


#include "rtfexport.hxx"

#include <rtl/ustring.hxx>
#include <svtools/rtfkeywd.hxx>
#include <filter/msfilter/rtfutil.hxx>

SmRtfExport::SmRtfExport(const SmNode* pIn)
    : SmWordExportBase(pIn)
    , m_pBuffer(0)
    , m_nEncoding(RTL_TEXTENCODING_DONTKNOW)
{
}

bool SmRtfExport::ConvertFromStarMath(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding)
{
    if (!m_pTree)
        return false;
    m_pBuffer = &rBuffer;
    m_nEncoding = nEncoding;
    m_pBuffer->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE LO_STRING_SVTOOLS_RTF_MOMATH " ");
    HandleNode(m_pTree, 0);
    m_pBuffer->append("}"); 
    return true;
}




void SmRtfExport::HandleVerticalStack(const SmNode* pNode, int nLevel)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MEQARR " ");
    int size = pNode->GetNumSubNodes();
    for (int i = 0; i < size; ++i)
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->GetSubNode(i), nLevel + 1);
        m_pBuffer->append("}"); 
    }
    m_pBuffer->append("}"); 
}

void SmRtfExport::HandleText(const SmNode* pNode, int /*nLevel*/)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MR " ");

    if (pNode->GetToken().eType == TTEXT)  
        m_pBuffer->append(LO_STRING_SVTOOLS_RTF_MNOR " ");

    SmTextNode* pTemp=(SmTextNode*)pNode;
    SAL_INFO("starmath.rtf", "Text: " << pTemp->GetText());
    for (sal_Int32 i = 0; i < pTemp->GetText().getLength(); i++)
    {
        sal_uInt16 nChar = pTemp->GetText()[i];
        OUString aValue(SmTextNode::ConvertSymbolToUnicode(nChar));
        m_pBuffer->append(msfilter::rtfutil::OutString(aValue, m_nEncoding));
    }

    m_pBuffer->append("}"); 
}

void SmRtfExport::HandleFractions(const SmNode* pNode, int nLevel, const char* type)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MF " ");
    if (type)
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MFPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MTYPE " ");
        m_pBuffer->append(type);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    OSL_ASSERT(pNode->GetNumSubNodes() == 3);
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MNUM " ");
    HandleNode(pNode->GetSubNode(0), nLevel + 1);
    m_pBuffer->append("}"); 
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MDEN " ");
    HandleNode(pNode->GetSubNode(2), nLevel + 1);
    m_pBuffer->append("}"); 
    m_pBuffer->append("}"); 
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
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MACC " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MACCPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MCHR " ");
        OUString aValue(pNode->Attribute()->GetToken().cMathChar);
        m_pBuffer->append(msfilter::rtfutil::OutString(aValue, m_nEncoding));
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->Body(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    }
    case TOVERLINE:
    case TUNDERLINE:
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MBAR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MBARPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MPOS " ");
        m_pBuffer->append((pNode->Attribute()->GetToken().eType == TUNDERLINE) ? "bot" : "top");
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->Body(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    case TOVERSTRIKE:
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MBORDERBOX " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MBORDERBOXPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MHIDETOP " 1}");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MHIDEBOT " 1}");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MHIDELEFT " 1}");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MHIDERIGHT " 1}");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSTRIKEH " 1}");
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->Body(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    default:
        HandleAllSubNodes(pNode, nLevel);
        break;
    }
}

void SmRtfExport::HandleRoot(const SmRootNode* pNode, int nLevel)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MRAD " ");
    if (const SmNode* argument = pNode->Argument())
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MDEG " ");
        HandleNode(argument, nLevel + 1);
        m_pBuffer->append("}"); 
    }
    else
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MRADPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MDEGHIDE " 1}");
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MDEG " }"); 
    }
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
    HandleNode(pNode->Body(), nLevel + 1);
    m_pBuffer->append("}"); 
    m_pBuffer->append("}"); 
}

namespace
{
OString mathSymbolToString(const SmNode* node, rtl_TextEncoding nEncoding)
{
    assert(node->GetType() == NMATH || node->GetType() == NMATHIDENT);
    const SmTextNode* txtnode = static_cast<const SmTextNode*>(node);
    if (txtnode->GetText().isEmpty())
        return OString();
    assert(txtnode->GetText().getLength() == 1);
    sal_Unicode chr = SmTextNode::ConvertSymbolToUnicode(txtnode->GetText()[0]);
    OUString aValue(chr);
    return msfilter::rtfutil::OutString(aValue, nEncoding);
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
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MNARY " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MNARYPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MCHR " ");
        m_pBuffer->append(mathSymbolToString(operation, m_nEncoding));
        m_pBuffer->append("}"); 
        if (!subsup || !subsup->GetSubSup(CSUB))
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUBHIDE " 1}");
        if (!subsup || !subsup->GetSubSup(CSUP))
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUPHIDE " 1}");
        m_pBuffer->append("}"); 
        if (!subsup || !subsup->GetSubSup(CSUB))
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUB " }");
        else
        {
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUB " ");
            HandleNode(subsup->GetSubSup(CSUB), nLevel + 1);
            m_pBuffer->append("}"); 
        }
        if (!subsup || !subsup->GetSubSup(CSUP))
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUP " }");
        else
        {
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUP " ");
            HandleNode(subsup->GetSubSup(CSUP), nLevel + 1);
            m_pBuffer->append("}"); 
        }
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->GetSubNode(1), nLevel + 1); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    }
    case TLIM:
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MFUNC " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MFNAME " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIMLOW " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->GetSymbol(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIM " ");
        if (const SmSubSupNode* subsup = pNode->GetSubNode(0)->GetType() == NSUBSUP ? static_cast<const SmSubSupNode*>(pNode->GetSubNode(0)) : 0)
            if (subsup->GetSubSup(CSUB))
                HandleNode(subsup->GetSubSup(CSUB), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->GetSubNode(1), nLevel + 1); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    default:
        SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC << " unhandled oper type");
        break;
    }
}

void SmRtfExport::HandleSubSupScriptInternal(const SmSubSupNode* pNode, int nLevel, int flags)
{


    if (flags == 0) 
        return;
    if ((flags & (1 << RSUP | 1 << RSUB)) == (1 << RSUP | 1 << RSUB))
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSSUBSUP " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << RSUP | 1 << RSUB);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUB " ");
        HandleNode(pNode->GetSubSup(RSUB), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUP " ");
        HandleNode(pNode->GetSubSup(RSUP), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else if ((flags & (1 << RSUB)) == 1 << RSUB)
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSSUB " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << RSUB);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUB " ");
        HandleNode(pNode->GetSubSup(RSUB), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else if ((flags & (1 << RSUP)) == 1 << RSUP)
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSSUP " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << RSUP);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUP " ");
        HandleNode(pNode->GetSubSup(RSUP), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else if ((flags & (1 << LSUP | 1 << LSUB)) == (1 << LSUP | 1 << LSUB))
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSPRE " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUB " ");
        HandleNode(pNode->GetSubSup(LSUB), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSUP " ");
        HandleNode(pNode->GetSubSup(LSUP), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << LSUP | 1 << LSUB);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else if ((flags & (1 << CSUB)) == (1 << CSUB))
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIMLOW " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << CSUB);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIM " ");
        HandleNode(pNode->GetSubSup(CSUB), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else if ((flags & (1 << CSUP)) == (1 << CSUP))
    {
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIMUPP " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        flags &= ~(1 << CSUP);
        if (flags == 0)
            HandleNode(pNode->GetBody(), nLevel + 1);
        else
            HandleSubSupScriptInternal(pNode, nLevel, flags);
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIM " ");
        HandleNode(pNode->GetSubSup(CSUP), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
    }
    else
        SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC << " unhandled subsup type");
}

void SmRtfExport::HandleMatrix(const SmMatrixNode* pNode, int nLevel)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MM " ");
    for (int row = 0; row < pNode->GetNumRows(); ++row)
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MMR " ");
        for (int col = 0; col < pNode->GetNumCols(); ++col)
        {
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
            if (const SmNode* node = pNode->GetSubNode(row * pNode->GetNumCols() + col))
                HandleNode(node, nLevel + 1);
            m_pBuffer->append("}"); 
        }
        m_pBuffer->append("}"); 
    }
    m_pBuffer->append("}"); 
}

void SmRtfExport::HandleBrace(const SmBraceNode* pNode, int nLevel)
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MD " ");
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MDPR " ");
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MBEGCHR " ");
    m_pBuffer->append(mathSymbolToString(pNode->OpeningBrace(), m_nEncoding));
    m_pBuffer->append("}"); 
    std::vector< const SmNode* > subnodes;
    if (pNode->Body()->GetType() == NBRACEBODY)
    {
        const SmBracebodyNode* body = static_cast<const SmBracebodyNode*>(pNode->Body());
        bool separatorWritten = false; 
        for (int i = 0; i < body->GetNumSubNodes(); ++i)
        {
            const SmNode* subnode = body->GetSubNode(i);
            if (subnode->GetType() == NMATH || subnode->GetType() == NMATHIDENT)
            {
                
                const SmMathSymbolNode* math = static_cast<const SmMathSymbolNode*>(subnode);
                if (!separatorWritten)
                {
                    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MSEPCHR " ");
                    m_pBuffer->append(mathSymbolToString(math, m_nEncoding));
                    m_pBuffer->append("}"); 
                    separatorWritten = true;
                }
            }
            else
                subnodes.push_back(subnode);
        }
    }
    else
        subnodes.push_back(pNode->Body());
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MENDCHR " ");
    m_pBuffer->append(mathSymbolToString(pNode->ClosingBrace(), m_nEncoding));
    m_pBuffer->append("}"); 
    m_pBuffer->append("}"); 
    for (unsigned int i = 0; i < subnodes.size(); ++i)
    {
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(subnodes[ i ], nLevel + 1);
        m_pBuffer->append("}"); 
    }
    m_pBuffer->append("}"); 
}

void SmRtfExport::HandleVerticalBrace(const SmVerticalBraceNode* pNode, int nLevel)
{
    SAL_INFO("starmath.rtf", "Vertical: " << int(pNode->GetToken().eType));
    switch (pNode->GetToken().eType)
    {
    case TOVERBRACE:
    case TUNDERBRACE:
    {
        bool top = (pNode->GetToken().eType == TOVERBRACE);
        if (top)
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIMUPP " ");
        else
            m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIMLOW " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MGROUPCHR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MGROUPCHRPR " ");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MCHR " ");
        m_pBuffer->append(mathSymbolToString(pNode->Brace(), m_nEncoding));
        m_pBuffer->append("}"); 
        
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MPOS " ").append(top ? "top" : "bot").append("}");
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MVERTJC " ").append(top ? "bot" : "top").append("}");
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_ME " ");
        HandleNode(pNode->Body(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MLIM " ");
        HandleNode(pNode->Script(), nLevel + 1);
        m_pBuffer->append("}"); 
        m_pBuffer->append("}"); 
        break;
    }
    default:
        SAL_INFO("starmath.rtf", "TODO: " << OSL_THIS_FUNC << " unhandled vertical brace type");
        break;
    }
}

void SmRtfExport::HandleBlank()
{
    m_pBuffer->append("{" LO_STRING_SVTOOLS_RTF_MR " ");
    m_pBuffer->append(" ");
    m_pBuffer->append("}"); 
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
