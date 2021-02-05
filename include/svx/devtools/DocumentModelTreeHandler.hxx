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

#include <svx/svxdllapi.h>
#include <vcl/weld.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <unordered_map>

class DocumentModelTreeHandler
{
private:
    std::unique_ptr<weld::TreeView>& mpDocumentModelTree;
    css::uno::Reference<css::uno::XInterface> mxDocument;

    void clearChildren(weld::TreeIter const& rParent);

public:
    DocumentModelTreeHandler(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
                             css::uno::Reference<css::uno::XInterface> const& xDocument)
        : mpDocumentModelTree(pDocumentModelTree)
        , mxDocument(xDocument)
    {
        mpDocumentModelTree->connect_expanding(
            LINK(this, DocumentModelTreeHandler, ExpandingHandler));
    }

    DECL_LINK(ExpandingHandler, const weld::TreeIter&, bool);

    void inspectDocument();

    static css::uno::Reference<css::uno::XInterface> getObjectByID(OUString const& rID);

    void dispose();
    void selectObject(css::uno::Reference<css::uno::XInterface> const& xInterface);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
