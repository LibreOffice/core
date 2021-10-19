/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SearchResultLocator.hxx>
#include <node.hxx>
#include <drawdoc.hxx>
#include <frame.hxx>
#include <cntfrm.hxx>
#include <viewsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/stream.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>

namespace sw::search
{
void SearchResultLocator::findOne(LocationResult& rResult, SearchIndexData const& rSearchIndexData)
{
    if (rSearchIndexData.meType == NodeType::WriterNode)
    {
        SwNodes const& rNodes = mpDocument->GetNodes();
        if (rSearchIndexData.mnNodeIndex >= rNodes.Count())
            return;
        SwNode* pNode = rNodes[rSearchIndexData.mnNodeIndex];

        auto* pContentNode = pNode->GetContentNode();
        auto* pShell = mpDocument->getIDocumentLayoutAccess().GetCurrentViewShell();

        if (pContentNode && pShell)
        {
            const SwFrame* pFrame
                = pContentNode->getLayoutFrame(pShell->GetLayout(), nullptr, nullptr);
            SwRect const& rArea = pFrame->getFrameArea();

            rResult.mbFound = true;
            rResult.maRectangles.emplace_back(rArea.Left(), rArea.Top(),
                                              rArea.Left() + rArea.Width(),
                                              rArea.Top() + rArea.Height());
        }
    }
    else if (rSearchIndexData.meType == NodeType::CommonNode)
    {
        IDocumentDrawModelAccess& rDrawModelAccess = mpDocument->getIDocumentDrawModelAccess();
        auto* pModel = rDrawModelAccess.GetDrawModel();
        for (sal_uInt16 nPage = 0; nPage < pModel->GetPageCount(); ++nPage)
        {
            SdrPage* pPage = pModel->GetPage(nPage);
            for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
            {
                SdrObject* pObject = pPage->GetObj(nObject);
                if (pObject)
                {
                    if (pObject->GetName() == rSearchIndexData.maObjectName)
                    {
                        auto aRect = pObject->GetLogicRect();
                        rResult.mbFound = true;
                        rResult.maRectangles.emplace_back(aRect.Left(), aRect.Top(),
                                                          aRect.Left() + aRect.GetWidth(),
                                                          aRect.Top() + aRect.GetHeight());
                    }
                }
            }
        }
    }
}

LocationResult SearchResultLocator::find(std::vector<SearchIndexData> const& rSearchIndexDataVector)
{
    LocationResult aResult;
    for (auto const& rSearchIndexData : rSearchIndexDataVector)
        findOne(aResult, rSearchIndexData);

    return aResult;
}

/** Trying to parse the payload as JSON
 *
 *  Returns true if parsing was successful and the payload was identified as JSON, else false
 */
bool SearchResultLocator::tryParseJSON(const char* pPayload,
                                       std::vector<sw::search::SearchIndexData>& rDataVector)
{
    boost::property_tree::ptree aTree;
    std::stringstream aStream(pPayload);
    try
    {
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (const boost::property_tree::json_parser_error& /*exception*/)
    {
        return false;
    }

    for (const auto& rEachNode : boost::make_iterator_range(aTree.equal_range("")))
    {
        auto const& rEach = rEachNode.second;

        std::string sType = rEach.get<std::string>("node_type", "");

        auto eNodeType = sw::search::NodeType::Undefined;
        if (sType == "writer")
            eNodeType = sw::search::NodeType::WriterNode;
        else if (sType == "common")
            eNodeType = sw::search::NodeType::CommonNode;

        std::string sJsonObjectName = rEach.get<std::string>("object_name", "");

        SwNodeOffset nIndex(rEach.get<sal_Int32>("index", -1));

        // Don't add search data elements that don't have valid data
        if (eNodeType != sw::search::NodeType::Undefined && nIndex >= SwNodeOffset(0))
        {
            OUString sObjectName;
            if (!sJsonObjectName.empty())
            {
                OString sObjectNameOString(sJsonObjectName.c_str());
                sObjectName = OStringToOUString(sObjectNameOString, RTL_TEXTENCODING_UTF8);
            }

            rDataVector.emplace_back(eNodeType, nIndex, sObjectName);
        }
    }

    return true;
}

/** Trying to parse the payload as XML
 *
 *  Returns true if parsing was successful and the payload was identified as XML, else false
 */
bool SearchResultLocator::tryParseXML(const char* pPayload,
                                      std::vector<sw::search::SearchIndexData>& rDataVector)
{
    const OString aPayloadString(pPayload);

    SvMemoryStream aStream(const_cast<char*>(aPayloadString.getStr()), aPayloadString.getLength(),
                           StreamMode::READ);

    tools::XmlWalker aWalker;

    if (!aWalker.open(&aStream))
        return false;

    if (aWalker.name() != "indexing")
        return true;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "paragraph")
        {
            OString sType = aWalker.attribute("node_type");
            OString sIndex = aWalker.attribute("index");
            OString sObjectName = aWalker.attribute("object_name");

            if (!sType.isEmpty() && !sIndex.isEmpty())
            {
                sw::search::SearchIndexData aData;
                aData.mnNodeIndex = SwNodeOffset(sIndex.toInt32());
                auto eNodeType = sw::search::NodeType::Undefined;
                if (sType == "writer")
                    eNodeType = sw::search::NodeType::WriterNode;
                else if (sType == "common")
                    eNodeType = sw::search::NodeType::CommonNode;

                aData.meType = eNodeType;
                if (!sObjectName.isEmpty())
                    aData.maObjectName = OStringToOUString(sObjectName, RTL_TEXTENCODING_UTF8);
                rDataVector.push_back(aData);
            }
        }
        aWalker.next();
    }
    aWalker.parent();
    return true;
}

LocationResult SearchResultLocator::findForPayload(const char* pPayload)
{
    std::vector<sw::search::SearchIndexData> aDataVector;

    // Try parse the payload as JSON, if not recognised as JSON, try parse
    // it as XML
    tryParseJSON(pPayload, aDataVector) || tryParseXML(pPayload, aDataVector);

    return find(aDataVector);
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
