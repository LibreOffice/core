/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/weld.hxx>
#include <StyleList.hxx>
#include <templdgi.hxx>

void weld::StyleList::setStyleFamily(SfxStyleFamily family)
{
}

void weld::StyleList::setPreview(bool bPreview)
{
}

void weld::StyleList::setFilter(weld::ComboBox& combobox)
{
}

sal_Int8 weld::StyleList::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    // handle drop of content into the treeview to create a new style
    SfxObjectShell* pDocShell = SfxObjectShell::Current();
    if (pDocShell)
    {
        TransferableDataHelper aHelper(rEvt.maDropEvent.Transferable);
        sal_uInt32 nFormatCount = aHelper.GetFormatCount();

        sal_Int8 nRet = DND_ACTION_NONE;

        bool bFormatFound = false;

        for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
        {
            SotClipboardFormatId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if ( aHelper.GetTransferableObjectDescriptor( nId, aDesc ) )
            {
                if ( aDesc.maClassName == pDocShell->GetFactory().GetClassId())
                {
                    Application::PostUserEvent(LINK(this, StyleList, OnAsyncExecuteDrop));

                    bFormatFound = true;
                    nRet =  rEvt.mnAction;
                    break;
                }
            }
        }

        if (bFormatFound)
            return nRet;
    }

    if (!mxStyleList->get_visible())
        return DND_ACTION_NONE;

    if (!bAllowReParentDrop)
        return DND_ACTION_NONE;

    // otherwise if we're dragging with the treeview to set a new parent of the dragged style
    weld::TreeView* pSource = mxStyleList->get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != mxStyleList.get())
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource(mxStyleList->make_iterator());
    if (!mxStyleList->get_selected(xSource.get()))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(mxStyleList->make_iterator());
    if (!mxStyleList->get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
    {
        // if nothing under the mouse, use the last row
        int nChildren = mxStyleList->n_children();
        if (!nChildren)
            return DND_ACTION_NONE;
        if (!mxStyleList->get_iter_first(*xTarget) || !mxStyleList->iter_nth_sibling(*xTarget, nChildren - 1))
            return DND_ACTION_NONE;
        while (mxStyleList->get_row_expanded(*xTarget))
        {
            nChildren = mxStyleList->iter_n_children(*xTarget);
            if (!mxStyleList->iter_children(*xTarget) || !mxStyleList->iter_nth_sibling(*xTarget, nChildren - 1))
                return DND_ACTION_NONE;
        }
    }
    OUString aTargetStyle = mxStyleList->get_text(*xTarget);
    DropHdl(mxStyleList->get_text(*xSource), aTargetStyle);
    mxStyleList->unset_drag_dest_row();
    FillTreeBox();
    SelectStyle(aTargetStyle, false);
    return DND_ACTION_NONE;
}
