/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <DiagramDialog.hxx>

#include <svx/DiagramDataInterface.hxx>

DiagramDialog::DiagramDialog(weld::Window* pWindow,
                             std::shared_ptr<DiagramDataInterface> pDiagramData)
    : GenericDialogController(pWindow, "cui/ui/diagramdialog.ui", "DiagramDialog")
    , mpDiagramData(pDiagramData)
    , mpBtnOk(m_xBuilder->weld_button("btnOk"))
    , mpBtnCancel(m_xBuilder->weld_button("btnCancel"))
    , mpTreeDiagram(m_xBuilder->weld_tree_view("treeDiagram"))
{
    populateTree(nullptr, OUString());

    // expand all items
    weld::TreeView* pTreeDiagram = mpTreeDiagram.get();
    pTreeDiagram->all_foreach([pTreeDiagram](weld::TreeIter& rEntry) {
        pTreeDiagram->expand_row(rEntry);
        return false;
    });
}

void DiagramDialog::populateTree(weld::TreeIter* pParent, const OUString& rParentId)
{
    auto aItems = mpDiagramData->getChildren(rParentId);
    for (auto& aItem : aItems)
    {
        std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
        mpTreeDiagram->insert(pParent, -1, &aItem.second, &aItem.first, nullptr, nullptr, nullptr,
                              false, pEntry.get());
        populateTree(pEntry.get(), aItem.first);
    }
}

DiagramDialog::~DiagramDialog() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
