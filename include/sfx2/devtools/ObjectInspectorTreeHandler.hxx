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

class ObjectInspectorTreeHandler
{
private:
    std::unique_ptr<weld::TreeView>& mpObjectInspectorTree;
    std::unique_ptr<weld::Label>& mpClassNameLabel;

    void clearObjectInspectorChildren(weld::TreeIter const& rParent);

public:
    ObjectInspectorTreeHandler(std::unique_ptr<weld::TreeView>& pObjectInspectorTree,
                               std::unique_ptr<weld::Label>& pClassNameLabel)
        : mpObjectInspectorTree(pObjectInspectorTree)
        , mpClassNameLabel(pClassNameLabel)
    {
        mpObjectInspectorTree->connect_expanding(
            LINK(this, ObjectInspectorTreeHandler, ExpandingHandler));
    }

    DECL_LINK(ExpandingHandler, const weld::TreeIter&, bool);

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);

    void dispose();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
