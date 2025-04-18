/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <memory>
#include "drawdoc.hxx"

class SdrObject;
class SdrPage;

namespace sd
{
/**
 * Interface for the visitor class, which handles each visited SdrObject
 * in the DOM.
 */
class ModelTraverseHandler
{
public:
    virtual ~ModelTraverseHandler() {}
    virtual void handleSdrObject(SdrObject* pObject) = 0;
};

/** Options to change how the traverser is traversing the tree, what is included and what not */
struct TraverserOptions
{
    bool mbPages = true;
    bool mbMasterPages = false;
};

/**
 * Traverses the DOM and calls a handler for each object (SdrObject) it
 * encounters.
 */
class ModelTraverser
{
private:
    std::vector<std::shared_ptr<ModelTraverseHandler>> m_pNodeHandler;
    SdDrawDocument* m_pDocument;
    TraverserOptions m_aTraverserOptions;

    void traverseObjects(SdrPage const& rPage);
    void traversePages();
    void traverseMasterPages();

public:
    ModelTraverser(SdDrawDocument* pDocument, TraverserOptions const& rTraverserOptions)
        : m_pDocument(pDocument)
        , m_aTraverserOptions(rTraverserOptions)
    {
    }

    void traverse();
    void addNodeHandler(std::shared_ptr<ModelTraverseHandler> pHandler);
};

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
