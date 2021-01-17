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

    std::unordered_map<OUString, css::uno::Reference<css::uno::XInterface>> maUnoObjectMap;

    void clearChildren(weld::TreeIter const& rParent);

    void fillSheets(weld::TreeIter const& rParent);
    void fillPages(weld::TreeIter const& rParent);
    void fillSlides(weld::TreeIter const& rParent);
    void fillMasterSlides(weld::TreeIter const& rParent);
    void fillParagraphs(weld::TreeIter const& rParent);
    void fillShapes(weld::TreeIter const& rParent);
    void fillTables(weld::TreeIter const& rParent);
    void fillFrames(weld::TreeIter const& rParent);
    void fillGraphicObjects(weld::TreeIter const& rParent);
    void fillOLEObjects(weld::TreeIter const& rParent);
    void fillStyleFamilies(weld::TreeIter const& rParent);

    void insertDocModelToParent(weld::TreeIter const& rParent, OUString const& rName,
                                css::uno::Reference<css::uno::XInterface> const& rInterface);

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
    css::uno::Reference<css::uno::XInterface> getObjectByID(OUString const& rID);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
