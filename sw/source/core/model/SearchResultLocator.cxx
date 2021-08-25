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
    else if (rSearchIndexData.meType == NodeType::SdrObject)
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
                        auto aRect = o3tl::convert(pObject->GetLogicRect(), o3tl::Length::mm100,
                                                   o3tl::Length::twip);
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

LocationResult SearchResultLocator::findForPayload(const char* pPayload)
{
    LocationResult aResult;

    const OString aPayloadString(pPayload);

    SvMemoryStream aStream(const_cast<char*>(aPayloadString.getStr()), aPayloadString.getLength(),
                           StreamMode::READ);
    tools::XmlWalker aWalker;

    if (!aWalker.open(&aStream))
        return aResult;

    if (aWalker.name() == "indexing")
    {
        std::vector<sw::search::SearchIndexData> aDataVector;
        aWalker.children();
        while (aWalker.isValid())
        {
            if (aWalker.name() == "paragraph")
            {
                OString sType = aWalker.attribute("type");
                OString sIndex = aWalker.attribute("index");

                if (!sType.isEmpty() && !sIndex.isEmpty())
                {
                    sw::search::SearchIndexData aData;
                    aData.mnNodeIndex = sIndex.toInt32();
                    aData.meType = sw::search::NodeType(sType.toInt32());

                    aDataVector.push_back(aData);
                }
            }
            aWalker.next();
        }
        aWalker.parent();

        if (!aDataVector.empty())
        {
            for (auto const& rSearchIndexData : aDataVector)
                findOne(aResult, rSearchIndexData);
        }
    }

    return aResult;
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
