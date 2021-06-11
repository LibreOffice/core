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

namespace sw
{
void ModelTraverser::traverse()
{
    if (m_pDoc == nullptr)
        return;

    auto const& pNodes = m_pDoc->GetNodes();
    SwNode* pNode = nullptr;
    for (sal_uLong n = 0; n < pNodes.Count(); ++n)
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
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
