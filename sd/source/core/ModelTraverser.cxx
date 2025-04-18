/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ModelTraverser.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>

namespace sd
{
void ModelTraverser::traverse()
{
    if (!m_pDocument)
        return;

    if (m_aTraverserOptions.mbPages)
        traversePages();

    if (m_aTraverserOptions.mbMasterPages)
        traverseMasterPages();
}

void ModelTraverser::traverseObjects(SdrPage const& rPage)
{
    SdrObjListIter aIterator(&rPage, SdrIterMode::DeepWithGroups);
    while (aIterator.IsMore())
    {
        SdrObject* pObject = aIterator.Next();
        if (!pObject)
            continue;
        for (auto& pNodeHandler : m_pNodeHandler)
            pNodeHandler->handleSdrObject(pObject);
    }
}

void ModelTraverser::traversePages()
{
    for (sal_uInt16 nPage = 0; nPage < m_pDocument->GetPageCount(); ++nPage)
    {
        SdrPage* pPage = m_pDocument->GetPage(nPage);
        if (pPage)
            traverseObjects(*pPage);
    }
}

void ModelTraverser::traverseMasterPages()
{
    for (sal_uInt16 nMasterPage = 0; nMasterPage < m_pDocument->GetMasterPageCount(); ++nMasterPage)
    {
        SdrPage* pMasterPage = m_pDocument->GetMasterPage(nMasterPage);
        if (pMasterPage)
            traverseObjects(*pMasterPage);
    }
}

void ModelTraverser::addNodeHandler(std::shared_ptr<ModelTraverseHandler> pHandler)
{
    m_pNodeHandler.push_back(pHandler);
}

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
