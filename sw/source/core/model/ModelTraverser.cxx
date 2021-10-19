/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <ModelTraverser.hxx>
#include <node.hxx>
#include <ndarr.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>

namespace sw
{
void ModelTraverser::traverse()
{
    if (m_pDoc == nullptr)
        return;

    auto const& pNodes = m_pDoc->GetNodes();
    SwNode* pNode = nullptr;

    for (SwNodeOffset n(0); n < pNodes.Count(); ++n)
    {
        pNode = pNodes[n];
        if (pNode)
        {
            for (auto& pNodeHandler : mpNodeHandler)
            {
                pNodeHandler->handleNode(pNode);
            }
        }
    }

    IDocumentDrawModelAccess& rDrawModelAccess = m_pDoc->getIDocumentDrawModelAccess();
    auto* pModel = rDrawModelAccess.GetDrawModel();
    for (sal_uInt16 nPage = 0; nPage < pModel->GetPageCount(); ++nPage)
    {
        SdrPage* pPage = pModel->GetPage(nPage);
        for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
        {
            SdrObject* pObject = pPage->GetObj(nObject);
            if (pObject)
            {
                for (auto& pNodeHandler : mpNodeHandler)
                {
                    pNodeHandler->handleSdrObject(pObject);
                }
            }
        }
    }
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
