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
#include <svx/svdogrp.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdundo.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svx/diagram/datamodel.hxx>
#include <svx/diagram/IDiagramHelper.hxx>

DiagramDialog::DiagramDialog(weld::Window* pWindow, SdrObjGroup& rDiagram)
    : GenericDialogController(pWindow, u"cui/ui/diagramdialog.ui"_ustr, u"DiagramDialog"_ustr)
    , m_rDiagram(rDiagram)
    , m_nUndos(0)
    , mpBtnCancel(m_xBuilder->weld_button(u"btnCancel"_ustr))
    , mpBtnAdd(m_xBuilder->weld_button(u"btnAdd"_ustr))
    , mpBtnRemove(m_xBuilder->weld_button(u"btnRemove"_ustr))
    , mpTreeDiagram(m_xBuilder->weld_tree_view(u"treeDiagram"_ustr))
    , mpTextAdd(m_xBuilder->weld_text_view(u"textAdd"_ustr))
{
    mpBtnCancel->connect_clicked(LINK(this, DiagramDialog, OnAddCancel));
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

IMPL_LINK_NOARG(DiagramDialog, OnAddCancel, weld::Button&, void)
{
    // If the user cancels the dialog, undo all changes done so far. It may
    // even be feasible to then delete the redo-stack, since it stays
    // available (?) - but it does no harm either...
    while (0 != m_nUndos)
    {
        comphelper::dispatchCommand(u".uno:Undo"_ustr, {});
        m_nUndos--;
    }

    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(DiagramDialog, OnAddClick, weld::Button&, void)
{
    if (!m_rDiagram.isDiagram())
        return;

    OUString sText = mpTextAdd->get_text();
    const std::shared_ptr< svx::diagram::IDiagramHelper >& pDiagramHelper(m_rDiagram.getDiagramHelper());

    if (pDiagramHelper && !sText.isEmpty())
    {
        SdrModel& rDrawModel(m_rDiagram.getSdrModelFromSdrObject());
        const bool bUndo(rDrawModel.IsUndoEnabled());
        svx::diagram::DiagramDataStatePtr aStartState;

        if (bUndo)
        {
            // rescue all start state Diagram-defining data
            aStartState = pDiagramHelper->extractDiagramDataState();
        }

        OUString sNodeId = pDiagramHelper->addNode(sText);

        if (bUndo)
        {
            // create undo action. That will internally secure the
            // current Diagram-defining data as end state
            rDrawModel.AddUndo(
                rDrawModel.GetSdrUndoFactory().CreateUndoDiagramModelData(m_rDiagram, aStartState));
            m_nUndos++;
        }

        std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
        mpTreeDiagram->insert(nullptr, -1, &sText, &sNodeId, nullptr, nullptr, false, pEntry.get());
        mpTreeDiagram->select(*pEntry);
        comphelper::dispatchCommand(u".uno:RegenerateDiagram"_ustr, {});
    }
}

IMPL_LINK_NOARG(DiagramDialog, OnRemoveClick, weld::Button&, void)
{
    if (!m_rDiagram.isDiagram())
        return;

    std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
    const std::shared_ptr< svx::diagram::IDiagramHelper >& pDiagramHelper(m_rDiagram.getDiagramHelper());

    if (pDiagramHelper && mpTreeDiagram->get_selected(pEntry.get()))
    {
        SdrModel& rDrawModel(m_rDiagram.getSdrModelFromSdrObject());
        const bool bUndo(rDrawModel.IsUndoEnabled());
        svx::diagram::DiagramDataStatePtr aStartState;

        if (bUndo)
        {
            // rescue all start state Diagram-defining data
            aStartState = pDiagramHelper->extractDiagramDataState();
        }

        if (pDiagramHelper->removeNode(mpTreeDiagram->get_id(*pEntry)))
        {
            if (bUndo)
            {
                // create undo action. That will internally secure the
                // current Diagram-defining data as end state
                rDrawModel.AddUndo(rDrawModel.GetSdrUndoFactory().CreateUndoDiagramModelData(
                    m_rDiagram, aStartState));
                m_nUndos++;
            }

            mpTreeDiagram->remove(*pEntry);
            comphelper::dispatchCommand(u".uno:RegenerateDiagram"_ustr, {});
        }
    }
}

void DiagramDialog::populateTree(const weld::TreeIter* pParent, const OUString& rParentId)
{
    if (!m_rDiagram.isDiagram())
        return;

    const std::shared_ptr< svx::diagram::IDiagramHelper >& pDiagramHelper(m_rDiagram.getDiagramHelper());

    if (!pDiagramHelper)
        return;

    auto aItems = pDiagramHelper->getChildren(rParentId);
    for (auto& aItem : aItems)
    {
        std::unique_ptr<weld::TreeIter> pEntry(mpTreeDiagram->make_iterator());
        mpTreeDiagram->insert(pParent, -1, &aItem.second, &aItem.first, nullptr, nullptr, false,
                              pEntry.get());
        populateTree(pEntry.get(), aItem.first);
    }
}

DiagramDialog::~DiagramDialog() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
