/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svtabbx.hxx>

//the default NotifyStartDrag is weird to me, and defaults to enabling all
//possibilities when drag starts, while restricting it to some subset of
//the configured drag drop mode would make more sense to me, but I'm not
//going to change the baseclass

class LclHeaderTabListBox : public SvHeaderTabListBox
{
public:
    LclHeaderTabListBox(vcl::Window* pParent, WinBits nWinStyle)
        : SvHeaderTabListBox(pParent, nWinStyle)
    {
    }

    virtual DragDropMode NotifyStartDrag(TransferDataContainer&, SvTreeListEntry*) override
    {
        return GetDragDropMode();
    }
};

class LclTabListBox : public SvTabListBox
{
    Link<SvTreeListBox*, void> m_aModelChangedHdl;

public:
    LclTabListBox(vcl::Window* pParent, WinBits nWinStyle)
        : SvTabListBox(pParent, nWinStyle)
    {
    }

    void SetModelChangedHdl(const Link<SvTreeListBox*, void>& rLink) { m_aModelChangedHdl = rLink; }

    virtual DragDropMode NotifyStartDrag(TransferDataContainer&, SvTreeListEntry*) override
    {
        return GetDragDropMode();
    }

    virtual void ModelHasCleared() override
    {
        SvTabListBox::ModelHasCleared();
        m_aModelChangedHdl.Call(this);
    }

    virtual void ModelHasInserted(SvTreeListEntry* pEntry) override
    {
        SvTabListBox::ModelHasInserted(pEntry);
        m_aModelChangedHdl.Call(this);
    }

    virtual void ModelHasInsertedTree(SvTreeListEntry* pEntry) override
    {
        SvTabListBox::ModelHasInsertedTree(pEntry);
        m_aModelChangedHdl.Call(this);
    }

    virtual void ModelHasMoved(SvTreeListEntry* pSource) override
    {
        SvTabListBox::ModelHasMoved(pSource);
        m_aModelChangedHdl.Call(this);
    }

    virtual void ModelHasRemoved(SvTreeListEntry* pEntry) override
    {
        SvTabListBox::ModelHasRemoved(pEntry);
        m_aModelChangedHdl.Call(this);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
