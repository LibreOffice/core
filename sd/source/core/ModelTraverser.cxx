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

using namespace css;

namespace sd
{
void ModelTraverser::traverse()
{
    if (!m_pDocument)
        return;

    for (sal_uInt16 nPage = 0; nPage < m_pDocument->GetPageCount(); ++nPage)
    {
        SdrPage* pPage = m_pDocument->GetPage(nPage);
        if (pPage)
        {
            for (const rtl::Reference<SdrObject>& pObject : *pPage)
            {
                for (auto& pNodeHandler : m_pNodeHandler)
                {
                    pNodeHandler->handleSdrObject(pObject.get());
                }
            }
        }
    }
}

void ModelTraverser::addNodeHandler(std::shared_ptr<ModelTraverseHandler> pHandler)
{
    m_pNodeHandler.push_back(pHandler);
}

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
