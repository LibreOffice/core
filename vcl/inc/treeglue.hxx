/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/toolkit/svtabbx.hxx>
#include "svimpbox.hxx"

//the default NotifyStartDrag is weird to me, and defaults to enabling all
//possibilities when drag starts, while restricting it to some subset of
//the configured drag drop mode would make more sense to me, but I'm not
//going to change the baseclass

class LclHeaderTabListBox final : public SvHeaderTabListBox
{
private:
    Link<SvTreeListEntry*, bool> m_aEditingEntryHdl;
    Link<std::pair<SvTreeListEntry*, OUString>, bool> m_aEditedEntryHdl;

public:
    LclHeaderTabListBox(vcl::Window* pParent, WinBits nWinStyle)
        : SvHeaderTabListBox(pParent, nWinStyle)
    {
    }

    void SetEditingEntryHdl(const Link<SvTreeListEntry*, bool>& rLink)
    {
        m_aEditingEntryHdl = rLink;
    }

    void SetEditedEntryHdl(const Link<std::pair<SvTreeListEntry*, OUString>, bool>& rLink)
    {
        m_aEditedEntryHdl = rLink;
    }

    virtual DragDropMode NotifyStartDrag() override { return GetDragDropMode(); }

    virtual bool EditingEntry(SvTreeListEntry* pEntry) override
    {
        return m_aEditingEntryHdl.Call(pEntry);
    }

    virtual bool EditedEntry(SvTreeListEntry* pEntry, const OUString& rNewText) override
    {
        return m_aEditedEntryHdl.Call(std::pair<SvTreeListEntry*, OUString>(pEntry, rNewText));
    }
};

class LclTabListBox final : public SvTabListBox
{
    Link<SvTreeListBox*, void> m_aModelChangedHdl;
    Link<SvTreeListBox*, bool> m_aStartDragHdl;
    Link<SvTreeListBox*, void> m_aEndDragHdl;
    Link<SvTreeListEntry*, bool> m_aEditingEntryHdl;
    Link<std::pair<SvTreeListEntry*, OUString>, bool> m_aEditedEntryHdl;

public:
    LclTabListBox(vcl::Window* pParent, WinBits nWinStyle)
        : SvTabListBox(pParent, nWinStyle)
    {
    }

    void SetModelChangedHdl(const Link<SvTreeListBox*, void>& rLink) { m_aModelChangedHdl = rLink; }
    void SetStartDragHdl(const Link<SvTreeListBox*, bool>& rLink) { m_aStartDragHdl = rLink; }
    void SetEndDragHdl(const Link<SvTreeListBox*, void>& rLink) { m_aEndDragHdl = rLink; }
    void SetEditingEntryHdl(const Link<SvTreeListEntry*, bool>& rLink)
    {
        m_aEditingEntryHdl = rLink;
    }
    void SetEditedEntryHdl(const Link<std::pair<SvTreeListEntry*, OUString>, bool>& rLink)
    {
        m_aEditedEntryHdl = rLink;
    }

    virtual DragDropMode NotifyStartDrag() override { return GetDragDropMode(); }

    virtual void StartDrag(sal_Int8 nAction, const Point& rPosPixel) override
    {
        if (m_aStartDragHdl.Call(this))
            return;
        SvTabListBox::StartDrag(nAction, rPosPixel);
    }

    virtual void DragFinished(sal_Int8 nDropAction) override
    {
        SvTabListBox::DragFinished(nDropAction);
        m_aEndDragHdl.Call(this);
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

    SvTreeListEntry* GetTargetAtPoint(const Point& rPos, bool bHighLightTarget)
    {
        SvTreeListEntry* pOldTargetEntry = pTargetEntry;
        pTargetEntry = PosOverBody(rPos) ? pImpl->GetEntry(rPos) : nullptr;
        if (pOldTargetEntry != pTargetEntry)
            ImplShowTargetEmphasis(pOldTargetEntry, false);

        // scroll
        if (rPos.Y() < 12)
        {
            ImplShowTargetEmphasis(pTargetEntry, false);
            ScrollOutputArea(+1);
        }
        else
        {
            Size aSize(pImpl->GetOutputSize());
            if (rPos.Y() > aSize.Height() - 12)
            {
                ImplShowTargetEmphasis(pTargetEntry, false);
                ScrollOutputArea(-1);
            }
        }

        if (pTargetEntry && bHighLightTarget)
            ImplShowTargetEmphasis(pTargetEntry, true);
        return pTargetEntry;
    }

    virtual SvTreeListEntry* GetDropTarget(const Point& rPos) override
    {
        return GetTargetAtPoint(rPos, true);
    }

    virtual bool EditingEntry(SvTreeListEntry* pEntry) override
    {
        return m_aEditingEntryHdl.Call(pEntry);
    }

    virtual bool EditedEntry(SvTreeListEntry* pEntry, const OUString& rNewText) override
    {
        return m_aEditedEntryHdl.Call(std::pair<SvTreeListEntry*, OUString>(pEntry, rNewText));
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
