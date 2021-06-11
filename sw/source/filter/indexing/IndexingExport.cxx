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

namespace sw
{
namespace
{
class IndexingNodeHandler : public ModelTraverseHandler
{
private:
    tools::XmlWriter& m_rXmlWriter;

public:
    IndexingNodeHandler(tools::XmlWriter& rXmlWriter)
        : m_rXmlWriter(rXmlWriter)
    {
    }

    void handleNode(SwNode* pNode) override
    {
        if (!pNode->IsTextNode())
            return;

        SwTextNode* pTextNode = pNode->GetTextNode();
        const OUString& rString
            = pTextNode->GetText().replaceAll(OUStringChar(CH_TXTATR_BREAKWORD), "");
        m_rXmlWriter.startElement("paragraph");
        m_rXmlWriter.attribute("index", pTextNode->GetIndex());
        m_rXmlWriter.content(rString);
        m_rXmlWriter.endElement();
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
