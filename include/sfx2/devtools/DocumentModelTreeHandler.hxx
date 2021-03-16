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

#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <unordered_map>

/** Document model tree handler
 *
 * Handles the DOM tree part of DevTools, which includes interaction with
 * the DOM tree view UI elements and the DOM model.
 */
class DocumentModelTreeHandler
{
private:
    std::unique_ptr<weld::TreeView>& mpDocumentModelTree;
    css::uno::Reference<css::uno::XInterface> mxDocument;

    // Clears all children of a tree node, where the parent is
    // identified by the input tree iter.
    void clearChildren(weld::TreeIter const& rParent);

    // Clear all tree view nodes.
    void clearAll();

public:
    DocumentModelTreeHandler(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
                             css::uno::Reference<css::uno::XInterface> const& xDocument);

    DECL_LINK(ExpandingHandler, const weld::TreeIter&, bool);

    void inspectDocument();

    static css::uno::Reference<css::uno::XInterface> getObjectByID(OUString const& rID);

    void dispose();

    // selects the input object if it exists in the DOM tree view
    void selectObject(css::uno::Reference<css::uno::XInterface> const& xInterface);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
