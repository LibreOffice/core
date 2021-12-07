/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <doc.hxx>

class SwNode;
class SdrObject;

namespace sw
{
class SW_DLLPUBLIC ModelTraverseHandler
{
public:
    virtual ~ModelTraverseHandler() {}

    virtual void handleNode(SwNode* pNode) = 0;
    virtual void handleSdrObject(SdrObject* pObject) = 0;
};

class ModelTraverser
{
private:
    std::vector<std::shared_ptr<ModelTraverseHandler>> mpNodeHandler;
    SwDoc* m_pDoc;

public:
    ModelTraverser(SwDoc* pDoc)
        : m_pDoc(pDoc)
    {
    }

    void traverse();

    void addNodeHandler(std::shared_ptr<ModelTraverseHandler> pHandler)
    {
        mpNodeHandler.push_back(pHandler);
    }
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
