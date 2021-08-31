/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <IndexingExport.hxx>

#include <ndtxt.hxx>
#include <ndole.hxx>
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <swtable.hxx>
#include <deque>

namespace sw
{
namespace
{
class IndexingNodeHandler : public ModelTraverseHandler
{
private:
    tools::XmlWriter& m_rXmlWriter;

    std::deque<SwNode*> maNodeStack;

public:
    IndexingNodeHandler(tools::XmlWriter& rXmlWriter)
        : m_rXmlWriter(rXmlWriter)
    {
    }

    void handleNode(SwNode* pNode) override
    {
        if (pNode->IsOLENode())
        {
            handleOLENode(pNode->GetOLENode());
        }
        else if (pNode->IsGrfNode())
        {
            handleGraphicNode(pNode->GetGrfNode());
        }
        else if (pNode->IsTextNode())
        {
            handleTextNode(pNode->GetTextNode());
        }
        else if (pNode->IsTableNode())
        {
            handleTableNode(pNode->GetTableNode());
        }

        if (pNode->IsEndNode())
        {
            handleEndNode(pNode->GetEndNode());
        }
    }

    void handleOLENode(SwOLENode* pOleNode)
    {
        auto pFrameFormat = pOleNode->GetFlyFormat();
        m_rXmlWriter.startElement("ole");
        m_rXmlWriter.attribute("alt", pOleNode->GetTitle());
        m_rXmlWriter.attribute("name", pFrameFormat->GetName());
        m_rXmlWriter.endElement();
    }

    void handleGraphicNode(SwGrfNode* pGraphicNode)
    {
        auto pFrameFormat = pGraphicNode->GetFlyFormat();
        m_rXmlWriter.startElement("graphic");
        m_rXmlWriter.attribute("alt", pGraphicNode->GetTitle());
        m_rXmlWriter.attribute("name", pFrameFormat->GetName());
        m_rXmlWriter.endElement();
    }

    void handleTextNode(SwTextNode* pTextNode)
    {
        const OUString& rString
            = pTextNode->GetText().replaceAll(OUStringChar(CH_TXTATR_BREAKWORD), "");
        m_rXmlWriter.startElement("paragraph");
        m_rXmlWriter.attribute("index", pTextNode->GetIndex());
        m_rXmlWriter.content(rString);
        m_rXmlWriter.endElement();
    }

    void handleSdrObject(SdrObject* pObject) override
    {
        if (pObject->GetName().isEmpty())
            return;
        m_rXmlWriter.startElement("shape");
        m_rXmlWriter.attribute("name", pObject->GetName());
        m_rXmlWriter.attribute("alt", pObject->GetTitle());
        m_rXmlWriter.attribute("description", pObject->GetDescription());

        SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
        if (pTextObject)
        {
            OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
            const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();
            for (sal_Int32 nParagraph = 0; nParagraph < aEdit.GetParagraphCount(); ++nParagraph)
            {
                OUString sText = aEdit.GetText(nParagraph);

                m_rXmlWriter.startElement("paragraph");
                m_rXmlWriter.attribute("index", nParagraph);
                m_rXmlWriter.content(sText);
                m_rXmlWriter.endElement();
            }
        }

        m_rXmlWriter.endElement();
    }

    void handleTableNode(SwTableNode* pTableNode)
    {
        const SwTableFormat* pFormat = pTableNode->GetTable().GetFrameFormat();
        OUString sName = pFormat->GetName();

        m_rXmlWriter.startElement("table");
        m_rXmlWriter.attribute("index", pTableNode->GetIndex());
        m_rXmlWriter.attribute("name", sName);

        maNodeStack.push_back(pTableNode);
    }

    void handleEndNode(SwEndNode* pEndNode)
    {
        if (!maNodeStack.empty() && pEndNode->StartOfSectionNode() == maNodeStack.back())
        {
            maNodeStack.pop_back();
            m_rXmlWriter.endElement();
        }
    }
};

} // end anonymous namespace

IndexingExport::IndexingExport(SvStream& rStream, SwDoc* pDoc)
    : m_aModelTraverser(pDoc)
    , m_aXmlWriter(&rStream)
{
}

bool IndexingExport::runExport()
{
    bool bResult = m_aXmlWriter.startDocument(2);
    if (!bResult)
        return false;

    m_aXmlWriter.startElement("indexing");
    m_aModelTraverser.addNodeHandler(std::make_shared<IndexingNodeHandler>(m_aXmlWriter));
    m_aModelTraverser.traverse();
    m_aXmlWriter.endElement();

    m_aXmlWriter.endDocument();

    return true;
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
