/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <DiagramDialog.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <svx/DiagramDataInterface.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

DiagramDialog::DiagramDialog(weld::Window* pWindow,
                             std::shared_ptr<DiagramDataInterface> pDiagramData)
    : GenericDialogController(pWindow, "cui/ui/diagramdialog.ui", "DiagramDialog")
    , mpDiagramData(pDiagramData)
    , mpBtnOk(m_xBuilder->weld_button("btnOk"))
    , mpBtnCancel(m_xBuilder->weld_button("btnCancel"))
    , mpBtnAdd(m_xBuilder->weld_button("btnAdd"))
    , mpBtnRemove(m_xBuilder->weld_button("btnRemove"))
    , mpTreeDiagram(m_xBuilder->weld_tree_view("treeDiagram"))
    , mpTextAdd(m_xBuilder->weld_text_view("textAdd"))
{
    mpBtnAdd->connect_clicked(LINK(this, DiagramDialog, OnAddClick));
    mpBtnRemove->connect_clicked(LINK(this, DiagramDialog, OnRemoveClick));

    populateTree(nullptr, OUString());

    // expand all items
    weld::TreeView* pTreeDiagram = mpTreeDiagram.get();
    pTreeDiagram->all_foreach([pTreeDiagram](weld::TreeIter& rEntry) {
        pTreeDiagram->expand_row(rEntry);
        return false;
    });
}

IMPL_LINK_NOARG(DiagramDialog, OnAddClick, weld::Button&, void)
{
    OUString sText = mpTextAdd->get_text();
    if (!sText.isEmpty())
    {
        OUString sNodeId = mpDiagramData->addNode(sText);
        std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
        mpTreeDiagram->insert(nullptr, -1, &sText, &sNodeId, nullptr, nullptr, nullptr, false,
                              pEntry.get());
        mpTreeDiagram->select(*pEntry);
        comphelper::dispatchCommand(".uno:RegenerateDiagram", {});
    }
}

IMPL_LINK_NOARG(DiagramDialog, OnRemoveClick, weld::Button&, void)
{
    std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
    if (mpTreeDiagram->get_selected(pEntry.get()))
    {
        if (mpDiagramData->removeNode(mpTreeDiagram->get_id(*pEntry)))
        {
            mpTreeDiagram->remove(*pEntry);
            comphelper::dispatchCommand(".uno:RegenerateDiagram", {});
        }
    }
}

void DiagramDialog::populateTree(const weld::TreeIter* pParent, const OUString& rParentId)
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
