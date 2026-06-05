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

/*
    TODO:
        - delete anchor in SelectionEngine when selecting manually
        - SelectAll( false ) => only repaint the deselected entries
*/

#include <accessibility/accessiblelistbox.hxx>

#include <vcl/toolkit/treelistbox.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/help.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builder.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/decoview.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <sot/formats.hxx>
#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/viewdataentry.hxx>
#include <accel.hxx>
#include <svimpbox.hxx>
#include <window.h>

#include <set>
#include <string.h>
#include <vector>

using namespace css::accessibility;

// Drag&Drop
static VclPtr<SvTreeListBox> g_pDDSource;
static VclPtr<SvTreeListBox> g_pDDTarget;

#define SVLBOX_ACC_RETURN 1
#define SVLBOX_ACC_ESCAPE 2

class SvInplaceEdit2
{
    Link<SvInplaceEdit2&, void> m_aCallBackHdl;
    Accelerator m_aAccReturn;
    Accelerator m_aAccEscape;
    Idle m_aIdle{ "svtools::SvInplaceEdit2 aIdle" };
    VclPtr<Edit> m_pEdit;
    bool m_bCanceled;
    bool m_bAlreadyInCallBack;

    void        CallCallBackHdl_Impl();
    DECL_LINK( Timeout_Impl, Timer *, void );
    DECL_LINK( ReturnHdl_Impl, Accelerator&, void );
    DECL_LINK( EscapeHdl_Impl, Accelerator&, void );

public:
                SvInplaceEdit2( vcl::Window* pParent, const Point& rPos, const Size& rSize,
                   const OUString& rData, const Link<SvInplaceEdit2&,void>& rNotifyEditEnd,
                   const Selection& );
               ~SvInplaceEdit2();
    bool        KeyInput( const KeyEvent& rKEvt );
    void        LoseFocus();
    bool EditingCanceled() const { return m_bCanceled; }
    OUString    GetText() const;
    OUString const & GetSavedValue() const;
    void        StopEditing( bool bCancel );
    void        Hide();
    const VclPtr<Edit>& GetEditWidget() const { return m_pEdit; };

    void RemoveEscapeAccel() { Application::RemoveAccel(&m_aAccEscape); }
    void InsertEscapeAccel() { Application::InsertAccel(&m_aAccEscape); }
};
// ***************************************************************

namespace {

class MyEdit_Impl : public Edit
{
    SvInplaceEdit2* pOwner;
public:
                 MyEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* pOwner );
    virtual     ~MyEdit_Impl() override { disposeOnce(); }
    virtual void dispose() override { pOwner = nullptr; Edit::dispose(); }
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void LoseFocus() override;
    virtual void Command(const CommandEvent& rCEvt) override;
};

}

MyEdit_Impl::MyEdit_Impl( vcl::Window* pParent, SvInplaceEdit2* _pOwner ) :

    Edit( pParent, WB_LEFT ),

    pOwner( _pOwner )

{
}

void MyEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if( !pOwner->KeyInput( rKEvt ))
        Edit::KeyInput( rKEvt );
}

void MyEdit_Impl::LoseFocus()
{
    if (pOwner)
        pOwner->LoseFocus();
}

void MyEdit_Impl::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        pOwner->RemoveEscapeAccel();  // so escape ends the popup
        Edit::Command(rCEvt);
        pOwner->InsertEscapeAccel();
    }
    else
        Edit::Command(rCEvt);
}

SvInplaceEdit2::SvInplaceEdit2(vcl::Window* pParent, const Point& rPos, const Size& rSize,
                               const OUString& rData,
                               const Link<SvInplaceEdit2&, void>& rNotifyEditEnd,
                               const Selection& rSelection)
    : m_aCallBackHdl(rNotifyEditEnd)
    , m_bCanceled(false)
    , m_bAlreadyInCallBack(false)
{
    m_pEdit = VclPtr<MyEdit_Impl>::Create(pParent, this);

    vcl::Font aFont( pParent->GetFont() );
    aFont.SetTransparent( false );
    Color aColor( pParent->GetBackground().GetColor() );
    aFont.SetFillColor(aColor );
    m_pEdit->SetFont(aFont);
    m_pEdit->SetBackground(pParent->GetBackground());
    m_pEdit->SetPosPixel(rPos);
    m_pEdit->SetSizePixel(rSize);
    m_pEdit->SetText(rData);
    m_pEdit->SetSelection(rSelection);
    m_pEdit->SaveValue();

    m_aAccReturn.InsertItem(SVLBOX_ACC_RETURN, vcl::KeyCode(KEY_RETURN));
    m_aAccEscape.InsertItem(SVLBOX_ACC_ESCAPE, vcl::KeyCode(KEY_ESCAPE));

    m_aAccReturn.SetActivateHdl(LINK(this, SvInplaceEdit2, ReturnHdl_Impl));
    m_aAccEscape.SetActivateHdl(LINK(this, SvInplaceEdit2, EscapeHdl_Impl));
    Application::InsertAccel(&m_aAccReturn);
    Application::InsertAccel(&m_aAccEscape);

    m_pEdit->Show();
    m_pEdit->GrabFocus();
}

SvInplaceEdit2::~SvInplaceEdit2()
{
    if (!m_bAlreadyInCallBack)
    {
        Application::RemoveAccel(&m_aAccReturn);
        Application::RemoveAccel(&m_aAccEscape);
    }
    m_pEdit.disposeAndClear();
}

OUString const& SvInplaceEdit2::GetSavedValue() const { return m_pEdit->GetSavedValue(); }

void SvInplaceEdit2::Hide() { m_pEdit->Hide(); }

IMPL_LINK_NOARG(SvInplaceEdit2, ReturnHdl_Impl, Accelerator&, void)
{
    m_bCanceled = false;
    CallCallBackHdl_Impl();
}

IMPL_LINK_NOARG(SvInplaceEdit2, EscapeHdl_Impl, Accelerator&, void)
{
    m_bCanceled = true;
    CallCallBackHdl_Impl();
}

bool SvInplaceEdit2::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            m_bCanceled = true;
            CallCallBackHdl_Impl();
            return true;

        case KEY_RETURN:
            m_bCanceled = false;
            CallCallBackHdl_Impl();
            return true;
    }
    return false;
}

void SvInplaceEdit2::StopEditing( bool bCancel )
{
    if (!m_bAlreadyInCallBack)
    {
        m_bCanceled = bCancel;
        CallCallBackHdl_Impl();
    }
}

void SvInplaceEdit2::LoseFocus()
{
    if (!m_bAlreadyInCallBack && !m_pEdit->IsActivePopup()
        && ((!Application::GetFocusWindow()) || !m_pEdit->IsChild(Application::GetFocusWindow())))
    {
        m_bCanceled = false;
        m_aIdle.SetPriority(TaskPriority::REPAINT);
        m_aIdle.SetInvokeHandler(LINK(this, SvInplaceEdit2, Timeout_Impl));
        m_aIdle.Start();
    }
}

IMPL_LINK_NOARG(SvInplaceEdit2, Timeout_Impl, Timer *, void)
{
    CallCallBackHdl_Impl();
}

void SvInplaceEdit2::CallCallBackHdl_Impl()
{
    m_aIdle.Stop();
    if (!m_bAlreadyInCallBack)
    {
        m_bAlreadyInCallBack = true;
        Application::RemoveAccel(&m_aAccReturn);
        Application::RemoveAccel(&m_aAccEscape);
        m_pEdit->Hide();
        m_aCallBackHdl.Call(*this);
    }
}

OUString SvInplaceEdit2::GetText() const { return m_pEdit->GetText(); }

// ***************************************************************
// class SvLBoxTab
// ***************************************************************


SvLBoxTab::SvLBoxTab()
{
    nPos = 0;
    nFlags = SvLBoxTabFlags::NONE;
}

SvLBoxTab::SvLBoxTab( tools::Long nPosition, SvLBoxTabFlags nTabFlags )
{
    nPos = nPosition;
    nFlags = nTabFlags;
}

SvLBoxTab::SvLBoxTab( const SvLBoxTab& rTab )
{
    nPos = rTab.nPos;
    nFlags = rTab.nFlags;
}

tools::Long SvLBoxTab::CalcOffset( tools::Long nItemWidth, tools::Long nTabWidth )
{
    tools::Long nOffset = 0;
    if ( nFlags & SvLBoxTabFlags::ADJUST_RIGHT )
    {
        nOffset = nTabWidth - nItemWidth;
        if( nOffset < 0 )
            nOffset = 0;
    }
    else if ( nFlags & SvLBoxTabFlags::ADJUST_CENTER )
    {
        if( nFlags & SvLBoxTabFlags::FORCE )
        {
            // correct implementation of centering
            nOffset = ( nTabWidth - nItemWidth ) / 2;
            if( nOffset < 0 )
                nOffset = 0;
        }
        else
        {
            // historically grown, wrong calculation of tabs which is needed by
            // Abo-Tabbox, Tools/Options/Customize etc.
            nItemWidth++;
            nOffset = -( nItemWidth / 2 );
        }
    }
    return nOffset;
}

// ***************************************************************
// class SvLBoxItem
// ***************************************************************


SvLBoxItem::SvLBoxItem()
    : mbDisabled(false)
{
}

SvLBoxItem::~SvLBoxItem()
{
}

int SvLBoxItem::GetWidth(const SvTreeListBox& rView, const SvTreeListEntry* pEntry) const
{
    const SvViewDataItem& rViewData = rView.GetViewDataItem(pEntry, *this);
    int nWidth = rViewData.mnWidth;
    if (nWidth == -1)
    {
        nWidth = CalcWidth(rView);
        const_cast<SvViewDataItem&>(rViewData).mnWidth = nWidth;
    }
    return nWidth;
}

int SvLBoxItem::GetHeight(const SvTreeListBox& rView, const SvTreeListEntry* pEntry) const
{
    const SvViewDataItem& rViewData = rView.GetViewDataItem(pEntry, *this);
    return rViewData.mnHeight;
}

int SvLBoxItem::GetWidth(const SvTreeListBox& rView, const SvViewDataEntry* pData,
                         sal_uInt16 nItemPos) const
{
    const SvViewDataItem& rIData = pData->GetItem(nItemPos);
    int nWidth = rIData.mnWidth;
    if (nWidth == -1)
    {
        nWidth = CalcWidth(rView);
        const_cast<SvViewDataItem&>(rIData).mnWidth = nWidth;
    }
    return nWidth;
}

int SvLBoxItem::GetHeight(const SvViewDataEntry* pData, sal_uInt16 nItemPos)
{
    const SvViewDataItem& rIData = pData->GetItem(nItemPos);
    return rIData.mnHeight;
}

int SvLBoxItem::CalcWidth(const SvTreeListBox& /*rView*/) const
{
    return 0;
}

struct SvTreeListBoxImpl
{
    bool m_bDoingQuickSelection:1;

    vcl::QuickSelectionEngine m_aQuickSelectionEngine;

    explicit SvTreeListBoxImpl(SvTreeListBox& _rBox) :
        m_bDoingQuickSelection(false),
        m_aQuickSelectionEngine(_rBox) {}
};

#define SV_LBOX_DEFAULT_INDENT_PIXEL 20

SvTreeListBox::SvTreeListBox(vcl::Window* pParent, WinBits nWinStyle) :
    Control(pParent, nWinStyle | WB_CLIPCHILDREN),
    DropTargetHelper(this),
    DragSourceHelper(this),
    m_nVisibleCount(0),
    m_nSelectionCount(0),
    m_bVisPositionsValid(false),
    mpImpl(new SvTreeListBoxImpl(*this)),
    mbContextBmpExpanded(false),
    mbQuickSearch(false),
    mbActivateOnSingleClick(false),
    mbCustomEntryRenderer(false),
    mbHoverSelection(false),
    mbSelectingByHover(false),
    mbIsTextColumEnabled(false),
    mnClicksToToggle(0), //at default clicking on a row won't toggle its default checkbox
    m_eSelMode(SelectionMode::NONE),
    m_nMinWidthInChars(0),
    mnDragAction(DND_ACTION_COPYMOVE | DND_ACTION_LINK),
    mbCenterAndClipText(false)
{
    m_pModel.reset(new SvTreeList(*this));

    // insert root entry
    SvTreeListEntry* pEntry = m_pModel->m_pRootItem.get();
    SvViewDataEntry aViewData;
    aViewData.SetExpanded(true);
    m_DataTable.insert(std::make_pair(pEntry, std::move(aViewData)));

    m_nImpFlags = SvTreeListBoxFlags::NONE;
    m_pTargetEntry = nullptr;
    m_nDragDropMode = DragDropMode::NONE;
    m_pModel->SetCloneLink(LINK(this, SvTreeListBox, CloneHdl_Impl));
    m_pHdlEntry = nullptr;
    m_eSelMode = SelectionMode::Single;
    m_nDragDropMode = DragDropMode::NONE;
    SetType(WindowType::TREELISTBOX);

    m_pCheckButtonData = nullptr;
    m_pEdEntry = nullptr;
    m_pEdItem = nullptr;
    m_nEntryHeight = 0;
    m_pEdCtrl = nullptr;
    m_nFirstSelTab = 0;
    m_nLastSelTab = 0;
    m_nFocusWidth = -1;
    mnCheckboxItemWidth = 0;

    m_nTreeFlags = SvTreeFlags::RECALCTABS;
    m_nIndent = SV_LBOX_DEFAULT_INDENT_PIXEL;
    m_nEntryHeightOffs = SV_ENTRYHEIGHTOFFS_PIXEL;
    m_pImpl.reset(new SvImpLBox(*this, *m_pModel, GetStyle()));

    mbContextBmpExpanded = true;
    m_nContextBmpWidthMax = 0;

    SetFont( GetFont() );
    AdjustEntryHeightAndRecalc();

    SetSpaceBetweenEntries( 0 );
    GetOutDev()->SetLineColor();
    InitSettings();
    ImplInitStyle();
    SetTabs();

    SetSublistOpenWithLeftRight();
}

void SvTreeListBox::Clear()
{
    if (m_pModel)
        m_pModel->Clear(); // Model calls SvTreeListBox::ModelHasCleared()
}

IMPL_LINK(SvTreeListBox, CloneHdl_Impl, SvTreeListEntry&, rEntry, SvTreeListEntry*)
{
    return CloneEntry(rEntry);
}

void SvTreeListBox::Insert(SvTreeListEntry* pEntry, SvTreeListEntry* pParent, sal_uInt32 nPos)
{
    m_pModel->Insert(pEntry, pParent, nPos);
}

void SvTreeListBox::Insert(SvTreeListEntry* pEntry, sal_uInt32 nRootPos)
{
    m_pModel->Insert(pEntry, nRootPos);
}

bool SvTreeListBox::ExpandingHdl()
{
    return !m_aExpandingHdl.IsSet() || m_aExpandingHdl.Call(this);
}

void SvTreeListBox::ExpandedHdl()
{
    m_aExpandedHdl.Call(this);
}

void SvTreeListBox::SelectHdl()
{
    m_aSelectHdl.Call(this);
}

void SvTreeListBox::DeselectHdl()
{
    m_aDeselectHdl.Call(this);
}

bool SvTreeListBox::DoubleClickHdl()
{
    return !m_aDoubleClickHdl.IsSet() || m_aDoubleClickHdl.Call(this);
}

bool SvTreeListBox::CheckDragAndDropMode( SvTreeListBox const * pSource, sal_Int8 nAction )
{
    if ( pSource != this )
        return false; // no drop

    if (!(m_nDragDropMode & DragDropMode::CTRL_MOVE))
        return false; // D&D locked within list

    if( DND_ACTION_MOVE == nAction )
    {
        if (!(m_nDragDropMode & DragDropMode::CTRL_MOVE))
            return false; // no local move
    }
    else
        return false; // no local copy

    return true;
}


/*
    NotifyMoving/Copying
    ====================

    default behavior:

    1. target doesn't have children
        - entry becomes sibling of target. entry comes after target
          (->Window: below the target)
    2. target is an expanded parent
        - entry inserted at the beginning of the target childlist
    3. target is a collapsed parent
        - entry is inserted at the end of the target childlist
*/
TriState SvTreeListBox::NotifyMoving(
    SvTreeListEntry*  pTarget,       // D&D dropping position in GetModel()
    const SvTreeListEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvTreeListEntry*& rpNewParent,   // new target parent
    sal_uInt32&        rNewChildPos)  // position in childlist of target parent
{
    DBG_ASSERT(pEntry,"NotifyMoving:SourceEntry?");
    if( !pTarget )
    {
        rpNewParent = nullptr;
        rNewChildPos = 0;
        return TRISTATE_TRUE;
    }
    if ( !pTarget->HasChildren() && !pTarget->HasChildrenOnDemand() )
    {
        // case 1
        rpNewParent = GetParent( pTarget );
        rNewChildPos = SvTreeList::GetRelPos( pTarget ) + 1;
        rNewChildPos += m_nCurEntrySelPos;
        m_nCurEntrySelPos++;
    }
    else
    {
        // cases 2 & 3
        rpNewParent = pTarget;
        if( IsExpanded(pTarget))
            rNewChildPos = 0;
        else
            rNewChildPos = TREELIST_APPEND;
    }
    return TRISTATE_TRUE;
}

TriState SvTreeListBox::NotifyCopying(
    SvTreeListEntry*  pTarget,       // D&D dropping position in GetModel()
    const SvTreeListEntry*  pEntry,        // entry that we want to move, from
                                 // GetSourceListBox()->GetModel()
    SvTreeListEntry*& rpNewParent,   // new target parent
    sal_uInt32&        rNewChildPos)  // position in childlist of target parent
{
    return NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
}

sal_uInt32 SvTreeListBox::GetSelectionCount() const { return m_nSelectionCount; }

bool SvTreeListBox::HasViewData() const { return m_DataTable.size() > 1; } // There's always a ROOT

void SvTreeListBox::ExpandListEntry(SvTreeListEntry* pEntry)
{
    assert(pEntry && "Expand:View/Entry?");
    SvViewDataEntry* pViewData = GetViewData(pEntry);
    if (!pViewData)
        return;

    if (pViewData->IsExpanded())
        return;

    DBG_ASSERT(!pEntry->m_Children.empty(),
               "SvTreeList::Expand: We expected to have child entries.");

    pViewData->SetExpanded(true);
    SvTreeListEntry* pParent = pEntry->pParent;
    // if parent is visible, invalidate status data
    if (IsExpanded(pParent))
    {
        m_bVisPositionsValid = false;
        m_nVisibleCount = 0;
    }
}

void SvTreeListBox::CollapseListEntry(SvTreeListEntry* pEntry)
{
    assert(pEntry && "Collapse:View/Entry?");
    SvViewDataEntry* pViewData = GetViewData(pEntry);
    if (!pViewData)
        return;

    if (!pViewData->IsExpanded())
        return;

    DBG_ASSERT(!pEntry->m_Children.empty(),
               "SvTreeList::Collapse: We expected to have child entries.");

    pViewData->SetExpanded(false);

    SvTreeListEntry* pParent = pEntry->pParent;
    if (IsExpanded(pParent))
    {
        m_nVisibleCount = 0;
        m_bVisPositionsValid = false;
    }
}

bool SvTreeListBox::SelectListEntry(SvTreeListEntry* pEntry, bool bSelect)
{
    DBG_ASSERT(pEntry, "Select:View/Entry?");

    SvViewDataEntry* pViewData = GetViewData(pEntry);
    if (!pViewData)
        return false;

    if (bSelect)
    {
        if (pViewData->IsSelected() || !pViewData->IsSelectable())
            return false;
        else
        {
            pViewData->SetSelected(true);
            m_nSelectionCount++;
        }
    }
    else
    {
        if (!pViewData->IsSelected())
            return false;
        else
        {
            pViewData->SetSelected(false);
            m_nSelectionCount--;
        }
    }
    return true;
}

void SvTreeListBox::Reset()
{
    m_DataTable.clear();
    m_nSelectionCount = 0;
    m_nVisibleCount = 0;
    m_bVisPositionsValid = false;
    if (m_pModel)
    {
        // insert root entry
        SvTreeListEntry* pEntry = m_pModel->m_pRootItem.get();
        SvViewDataEntry aViewData;
        aViewData.SetExpanded(true);
        m_DataTable.insert(std::make_pair(pEntry, std::move(aViewData)));
    }
}

void SvTreeListBox::ActionMoving(SvTreeListEntry* pEntry)
{
    SvTreeListEntry* pParent = pEntry->pParent;
    assert(pParent && "Model not consistent");
    if (pParent != m_pModel->m_pRootItem.get() && pParent->m_Children.size() == 1)
    {
        const auto iter = m_DataTable.find(pParent);
        assert(iter != m_DataTable.end());
        SvViewDataEntry& rViewData = iter->second;
        rViewData.SetExpanded(false);
    }
    // preliminary
    m_nVisibleCount = 0;
    m_bVisPositionsValid = false;
}

void SvTreeListBox::ActionMoved()
{
    m_nVisibleCount = 0;
    m_bVisPositionsValid = false;
}

void SvTreeListBox::ActionInserted(SvTreeListEntry* pEntry)
{
    DBG_ASSERT(pEntry, "Insert:No Entry");
    SvViewDataEntry aData;
    InitViewData(&aData, pEntry);
    std::pair<SvDataTable::iterator, bool> aSuccess
        = m_DataTable.insert(std::make_pair(pEntry, std::move(aData)));
    DBG_ASSERT(aSuccess.second, "Entry already in View");
    if (m_nVisibleCount && m_pModel->IsEntryVisible(this, pEntry))
    {
        m_nVisibleCount = 0;
        m_bVisPositionsValid = false;
    }
}

void SvTreeListBox::ActionInsertedTree(SvTreeListEntry* pEntry)
{
    if (m_pModel->IsEntryVisible(this, pEntry))
    {
        m_nVisibleCount = 0;
        m_bVisPositionsValid = false;
    }
    // iterate over entry and its children
    SvTreeListEntry* pCurEntry = pEntry;
    sal_uInt16 nRefDepth = m_pModel->GetDepth(pCurEntry);
    while (pCurEntry)
    {
        DBG_ASSERT(m_DataTable.find(pCurEntry) != m_DataTable.end(), "Entry already in Table");
        SvViewDataEntry aViewData;
        InitViewData(&aViewData, pEntry);
        m_DataTable.insert(std::make_pair(pCurEntry, std::move(aViewData)));
        pCurEntry = m_pModel->Next(pCurEntry);
        if (pCurEntry && m_pModel->GetDepth(pCurEntry) <= nRefDepth)
            pCurEntry = nullptr;
    }
}

void SvTreeListBox::RemoveViewData(SvTreeListEntry* pParent)
{
    for (auto const& it : pParent->m_Children)
    {
        SvTreeListEntry& rEntry = *it;
        m_DataTable.erase(&rEntry);
        if (rEntry.HasChildren())
            RemoveViewData(&rEntry);
    }
}

void SvTreeListBox::ActionRemoving(SvTreeListEntry* pEntry)
{
    assert(pEntry && "Remove:No Entry");
    const auto iter = m_DataTable.find(pEntry);
    assert(iter != m_DataTable.end());
    SvViewDataEntry& rViewData = iter->second;
    sal_uInt32 nSelRemoved = 0;
    if (rViewData.IsSelected())
        nSelRemoved = 1 + m_pModel->GetChildSelectionCount(this, pEntry);
    m_nSelectionCount -= nSelRemoved;
    sal_uInt32 nVisibleRemoved = 0;
    if (m_pModel->IsEntryVisible(this, pEntry))
        nVisibleRemoved = 1 + m_pModel->GetVisibleChildCount(this, pEntry);
    if (m_nVisibleCount)
    {
#ifdef DBG_UTIL
        if (m_nVisibleCount < nVisibleRemoved)
        {
            OSL_FAIL("nVisibleRemoved bad");
        }
#endif
        m_nVisibleCount -= nVisibleRemoved;
    }
    m_bVisPositionsValid = false;

    m_DataTable.erase(pEntry);
    RemoveViewData(pEntry);

    SvTreeListEntry* pCurEntry = pEntry->pParent;
    if (pCurEntry && pCurEntry != m_pModel->m_pRootItem.get() && pCurEntry->m_Children.size() == 1)
    {
        SvDataTable::iterator itr = m_DataTable.find(pCurEntry);
        assert(itr != m_DataTable.end() && "Entry not in Table");
        SvViewDataEntry& rViewData2 = itr->second;
        rViewData2.SetExpanded(false);
    }
}

bool SvTreeListBox::IsExpanded(SvTreeListEntry* pEntry) const
{
    DBG_ASSERT(pEntry, "IsExpanded:No Entry");
    SvDataTable::const_iterator itr = m_DataTable.find(pEntry);
    if (itr == m_DataTable.end())
        return false;
    return itr->second.IsExpanded();
}

bool SvTreeListBox::IsAllExpanded(SvTreeListEntry* pEntry) const
{
    assert(pEntry && "IsAllExpanded:No Entry");
    if (!IsExpanded(pEntry))
        return false;
    const SvTreeListEntries& rChildren = pEntry->GetChildEntries();
    for (auto& rChild : rChildren)
    {
        if (rChild->HasChildren() || rChild->HasChildrenOnDemand())
        {
            if (!IsAllExpanded(rChild.get()))
                return false;
        }
    }
    return true;
}

bool SvTreeListBox::IsSelected(const SvTreeListEntry* pEntry) const
{
    DBG_ASSERT(pEntry, "IsExpanded:No Entry");
    SvDataTable::const_iterator itr = m_DataTable.find(const_cast<SvTreeListEntry*>(pEntry));
    if (itr == m_DataTable.end())
        return false;
    return itr->second.IsSelected();
}

void SvTreeListBox::SetEntryFocus(SvTreeListEntry* pEntry, bool bFocus)
{
    DBG_ASSERT(pEntry, "SetEntryFocus:No Entry");
    SvDataTable::iterator itr = m_DataTable.find(pEntry);
    assert(itr != m_DataTable.end() && "Entry not in Table");
    itr->second.SetFocus(bFocus);
}

const SvViewDataEntry* SvTreeListBox::GetViewData(const SvTreeListEntry* pEntry) const
{
    SvDataTable::const_iterator itr = m_DataTable.find(const_cast<SvTreeListEntry*>(pEntry));
    assert(itr != m_DataTable.end() && "Entry not in model or wrong view");
    if (itr == m_DataTable.end())
        return nullptr;
    return &itr->second;
}

SvViewDataEntry* SvTreeListBox::GetViewData(SvTreeListEntry* pEntry)
{
    return const_cast<SvViewDataEntry*>(std::as_const(*this).GetViewData(pEntry));
}

SvTreeListEntry* SvTreeListBox::FirstChild(const SvTreeListEntry* pParent) const
{
    return m_pModel->FirstChild(pParent);
}

sal_uInt32 SvTreeListBox::GetEntryPos(const SvTreeListEntry* pEntry) const
{
    sal_uInt32 nPos = 0;
    SvTreeListEntry* pTmpEntry = First();
    while (pTmpEntry)
    {
        if (pTmpEntry == pEntry)
            return nPos;
        pTmpEntry = Next(pTmpEntry);
        ++nPos;
    }
    return 0xffffffff;
}

// return: all entries copied
bool SvTreeListBox::CopySelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget )
{
    m_nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    bool bSuccess = true;
    std::vector<SvTreeListEntry*> aList;
    bool bClone = ( pSource->GetModel() != GetModel() );

    // cache selection to simplify iterating over the selection when doing a D&D
    // exchange within the same listbox
    SvTreeListEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are copied automatically
        pSource->SelectChildren( pSourceEntry, false );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    for (auto const& elem : aList)
    {
        pSourceEntry = elem;
        SvTreeListEntry* pNewParent = nullptr;
        sal_uInt32 nInsertionPos = TREELIST_APPEND;
        TriState nOk = NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        if ( nOk )
        {
            if ( bClone )
            {
                sal_uInt32 nCloneCount = 0;
                pSourceEntry = m_pModel->Clone(*pSourceEntry, nCloneCount);
                m_pModel->InsertTree(pSourceEntry, pNewParent, nInsertionPos);
            }
            else
            {
                sal_uInt32 nListPos = m_pModel->Copy(*pSourceEntry, pNewParent, nInsertionPos);
                pSourceEntry = GetEntry( pNewParent, nListPos );
            }
        }
        else
            bSuccess = false;

        if (nOk == TRISTATE_INDET)  // HACK: make visible moved entry
            MakeVisible( pSourceEntry );
    }
    return bSuccess;
}

// return: all entries were moved
bool SvTreeListBox::MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvTreeListEntry* pTarget, bool bAllowCopyFallback )
{
    m_nCurEntrySelPos = 0; // selection counter for NotifyMoving/Copying
    bool bSuccess = true;
    std::vector<SvTreeListEntry*> aList;
    bool bClone = ( pSource->GetModel() != GetModel() );

    SvTreeListEntry* pSourceEntry = pSource->FirstSelected();
    while ( pSourceEntry )
    {
        // children are automatically moved
        pSource->SelectChildren( pSourceEntry, false );
        aList.push_back( pSourceEntry );
        pSourceEntry = pSource->NextSelected( pSourceEntry );
    }

    for (auto const& elem : aList)
    {
        pSourceEntry = elem;
        SvTreeListEntry* pNewParent = nullptr;
        sal_uInt32 nInsertionPos = TREELIST_APPEND;
        TriState nOk = NotifyMoving(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        TriState nCopyOk = nOk;
        if ( !nOk && bAllowCopyFallback )
        {
            nInsertionPos = TREELIST_APPEND;
            nCopyOk = NotifyCopying(pTarget,pSourceEntry,pNewParent,nInsertionPos);
        }

        if ( nOk || nCopyOk )
        {
            if ( bClone )
            {
                sal_uInt32 nCloneCount = 0;
                pSourceEntry = m_pModel->Clone(*pSourceEntry, nCloneCount);
                m_pModel->InsertTree(pSourceEntry, pNewParent, nInsertionPos);
            }
            else
            {
                if ( nOk )
                    m_pModel->Move(pSourceEntry, pNewParent, nInsertionPos);
                else
                    m_pModel->Copy(*pSourceEntry, pNewParent, nInsertionPos);
            }
        }
        else
            bSuccess = false;

        if (nOk == TRISTATE_INDET)  // HACK: make moved entry visible
            MakeVisible( pSourceEntry );
    }
    return bSuccess;
}

void SvTreeListBox::RemoveSelection()
{
    std::vector<const SvTreeListEntry*> aList;
    // cache selection, as the implementation deselects everything on the first
    // remove
    SvTreeListEntry* pEntry = FirstSelected();
    while ( pEntry )
    {
        aList.push_back( pEntry );
        if ( pEntry->HasChildren() )
            // remove deletes all children automatically
            SelectChildren(pEntry, false);
        pEntry = NextSelected( pEntry );
    }

    for (auto const& elem : aList)
        m_pModel->Remove(elem);
}

void SvTreeListBox::RemoveEntry(SvTreeListEntry const* pEntry) { m_pModel->Remove(pEntry); }

void SvTreeListBox::RecalcViewData()
{
    SvTreeListEntry* pEntry = First();
    while( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCurPos = 0;
        while ( nCurPos < nCount )
        {
            SvLBoxItem& rItem = pEntry->GetItem( nCurPos );
            rItem.InitViewData(*this, pEntry);
            nCurPos++;
        }
        pEntry = Next( pEntry );
    }
}

void SvTreeListBox::ImplShowTargetEmphasis( SvTreeListEntry* pEntry, bool bShow)
{
    if (bShow && (m_nImpFlags & SvTreeListBoxFlags::TARGEMPH_VIS))
        return;
    if (!bShow && !(m_nImpFlags & SvTreeListBoxFlags::TARGEMPH_VIS))
        return;
    m_pImpl->PaintDDCursor(pEntry, bShow);
    if( bShow )
        m_nImpFlags |= SvTreeListBoxFlags::TARGEMPH_VIS;
    else
        m_nImpFlags &= ~SvTreeListBoxFlags::TARGEMPH_VIS;
}

void SvTreeListBox::OnCurrentEntryChanged()
{
    if ( !mpImpl->m_bDoingQuickSelection )
        mpImpl->m_aQuickSelectionEngine.Reset();
}

SvTreeListEntry* SvTreeListBox::GetEntry( SvTreeListEntry* pParent, sal_uInt32 nPos ) const
{
    return m_pModel->GetEntry(pParent, nPos);
}

SvTreeListEntry* SvTreeListBox::GetEntry( sal_uInt32 nRootPos ) const
{
    return m_pModel->GetEntry(nRootPos);
}

SvTreeListEntry* SvTreeListBox::GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const
{

    SvTreeListEntry* pEntry = nullptr;
    SvTreeListEntry* pParent = nullptr;
    for (auto const& elem : _rPath)
    {
        pEntry = GetEntry( pParent, elem );
        if ( !pEntry )
            break;
        pParent = pEntry;
    }

    return pEntry;
}

void SvTreeListBox::FillEntryPath( SvTreeListEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const
{

    if ( !pEntry )
        return;

    SvTreeListEntry* pParentEntry = GetParent( pEntry );
    while ( true )
    {
        sal_uInt32 i, nCount = GetLevelChildCount( pParentEntry );
        for ( i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pTemp = GetEntry( pParentEntry, i );
            DBG_ASSERT( pEntry, "invalid entry" );
            if ( pEntry == pTemp )
            {
                _rPath.push_front( static_cast<sal_Int32>(i) );
                break;
            }
        }

        if ( pParentEntry )
        {
            pEntry = pParentEntry;
            pParentEntry = GetParent( pParentEntry );
        }
        else
            break;
    }
}

SvTreeListEntry* SvTreeListBox::GetParent( SvTreeListEntry* pEntry ) const
{
    return m_pModel->GetParent(pEntry);
}

sal_uInt32 SvTreeListBox::GetChildCount( SvTreeListEntry const * pParent ) const
{
    return m_pModel->GetChildCount(pParent);
}

sal_uInt32 SvTreeListBox::GetLevelChildCount( const SvTreeListEntry* _pParent ) const
{

    //if _pParent is 0, then pEntry is the first child of the root.
    SvTreeListEntry* pEntry = FirstChild( _pParent );

    if( !pEntry )//there is only root, root don't have children
        return 0;

    if( !_pParent )//root and children of root
        return pEntry->pParent->m_Children.size();

    return _pParent->m_Children.size();
}

SvViewDataEntry* SvTreeListBox::GetViewDataEntry( SvTreeListEntry const * pEntry ) const
{
    return const_cast<SvViewDataEntry*>(GetViewData(pEntry));
}

SvViewDataItem& SvTreeListBox::GetViewDataItem(SvTreeListEntry const* pEntry,
                                               const SvLBoxItem& rItem)
{
    return const_cast<SvViewDataItem&>(
        static_cast<const SvTreeListBox*>(this)->GetViewDataItem(pEntry, rItem));
}

const SvViewDataItem& SvTreeListBox::GetViewDataItem(const SvTreeListEntry* pEntry,
                                                     const SvLBoxItem& rItem) const
{
    const SvViewDataEntry* pEntryData = GetViewData(pEntry);
    assert(pEntryData && "Entry not in View");
    sal_uInt16 nItemPos = pEntry->GetPos(rItem);
    return pEntryData->GetItem(nItemPos);
}

OUString SvTreeListBox::GetEntryTooltip(SvTreeListEntry& rEntry) const
{
    const OUString sToolTip = m_aTooltipHdl.Call(rEntry);
    if (!sToolTip.isEmpty())
        return sToolTip;

    return rEntry.GetToolTip();
}

void SvTreeListBox::InitViewData( SvViewDataEntry* pData, SvTreeListEntry* pEntry )
{
    pData->Init(pEntry->ItemCount());
    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCurPos = 0;
    while( nCurPos < nCount )
    {
        SvLBoxItem& rItem = pEntry->GetItem(nCurPos);
        SvViewDataItem& rItemData = pData->GetItem(nCurPos);
        rItem.InitViewData(*this, pEntry, &rItemData);
        nCurPos++;
    }
}

void SvTreeListBox::EnableSelectionAsDropTarget( bool bEnable )
{
    sal_uInt16 nRefDepth;
    SvTreeListEntry* pTemp;

    SvTreeListEntry* pSelEntry = FirstSelected();
    while( pSelEntry )
    {
        if ( !bEnable )
        {
            pSelEntry->nEntryFlags |= SvTLEntryFlags::DISABLE_DROP;
            nRefDepth = m_pModel->GetDepth(pSelEntry);
            pTemp = Next( pSelEntry );
            while (pTemp && m_pModel->GetDepth(pTemp) > nRefDepth)
            {
                pTemp->nEntryFlags |= SvTLEntryFlags::DISABLE_DROP;
                pTemp = Next( pTemp );
            }
        }
        else
        {
            pSelEntry->nEntryFlags &= ~SvTLEntryFlags::DISABLE_DROP;
            nRefDepth = m_pModel->GetDepth(pSelEntry);
            pTemp = Next( pSelEntry );
            while (pTemp && m_pModel->GetDepth(pTemp) > nRefDepth)
            {
                pTemp->nEntryFlags &= ~SvTLEntryFlags::DISABLE_DROP;
                pTemp = Next( pTemp );
            }
        }
        pSelEntry = NextSelected( pSelEntry );
    }
}

// ******************************************************************
// InplaceEditing
// ******************************************************************

VclPtr<Edit> SvTreeListBox::GetEditWidget() const
{
    return m_pEdCtrl ? m_pEdCtrl->GetEditWidget() : nullptr;
}

void SvTreeListBox::EditText( const OUString& rStr, const tools::Rectangle& rRect,
    const Selection& rSel )
{
    m_pEdCtrl.reset();
    m_nImpFlags |= SvTreeListBoxFlags::IN_EDT;
    m_nImpFlags &= ~SvTreeListBoxFlags::EDTEND_CALLED;
    HideFocus();
    m_pEdCtrl.reset(new SvInplaceEdit2(this, rRect.TopLeft(), rRect.GetSize(), rStr,
                                       LINK(this, SvTreeListBox, TextEditEndedHdl_Impl), rSel));
}

IMPL_LINK_NOARG(SvTreeListBox, TextEditEndedHdl_Impl, SvInplaceEdit2&, void)
{
    if ( m_nImpFlags & SvTreeListBoxFlags::EDTEND_CALLED ) // avoid nesting
        return;
    m_nImpFlags |= SvTreeListBoxFlags::EDTEND_CALLED;
    OUString aStr;
    if (!m_pEdCtrl->EditingCanceled())
        aStr = m_pEdCtrl->GetText();
    else
        aStr = m_pEdCtrl->GetSavedValue();
    EditedText( aStr );
    // Hide may only be called after the new text was put into the entry, so
    // that we don't call the selection handler in the GetFocus of the listbox
    // with the old entry text.
    m_pEdCtrl->Hide();
    m_nImpFlags &= ~SvTreeListBoxFlags::IN_EDT;
    GrabFocus();
}

void SvTreeListBox::CancelTextEditing()
{
    if (m_pEdCtrl)
        m_pEdCtrl->StopEditing(true);
    m_nImpFlags &= ~SvTreeListBoxFlags::IN_EDT;
}

void SvTreeListBox::EndEditing( bool bCancel )
{
    if (m_pEdCtrl)
        m_pEdCtrl->StopEditing(bCancel);
    m_nImpFlags &= ~SvTreeListBoxFlags::IN_EDT;
}

vcl::StringEntryIdentifier SvTreeListBox::CurrentEntry( OUString& _out_entryText ) const
{
    // always accept the current entry if there is one
    SvTreeListEntry* pEntry( GetCurEntry() );
    if (pEntry)
    {
        _out_entryText = GetEntryText(pEntry);
        return pEntry;
    }

    pEntry = FirstSelected();
    if ( !pEntry )
        pEntry = First();

    if ( pEntry )
        _out_entryText = GetEntryText( pEntry );

    return pEntry;
}

vcl::StringEntryIdentifier SvTreeListBox::NextEntry(vcl::StringEntryIdentifier _pCurrentSearchEntry, OUString& _out_entryText) const
{
    SvTreeListEntry* pEntry = const_cast< SvTreeListEntry* >( static_cast< const SvTreeListEntry* >( _pCurrentSearchEntry ) );

    if  (   (   ( GetChildCount( pEntry ) > 0 )
            ||  ( pEntry->HasChildrenOnDemand() )
            )
        &&  !IsExpanded( pEntry )
        )
    {
        SvTreeListEntry* pNextSiblingEntry = pEntry->NextSibling();
        if ( !pNextSiblingEntry )
            pEntry = Next( pEntry );
        else
            pEntry = pNextSiblingEntry;
    }
    else
    {
        pEntry = Next( pEntry );
    }

    if ( !pEntry )
        pEntry = First();

    if ( pEntry )
        _out_entryText = GetEntryText( pEntry );

    return pEntry;
}

void SvTreeListBox::SelectEntry(vcl::StringEntryIdentifier _pEntry)
{
    SvTreeListEntry* pEntry = const_cast< SvTreeListEntry* >( static_cast< const SvTreeListEntry* >( _pEntry ) );
    DBG_ASSERT( pEntry, "SvTreeListBox::SelectSearchEntry: invalid entry!" );
    if ( !pEntry )
        return;

    SelectAll( false );
    SetCurEntry( pEntry );
    Select( pEntry );
}

bool SvTreeListBox::HandleKeyInput( const KeyEvent& _rKEvt )
{
    if ( _rKEvt.GetKeyCode().IsMod1() )
        return false;

    if (mbQuickSearch)
    {
        mpImpl->m_bDoingQuickSelection = true;
        const bool bHandled = mpImpl->m_aQuickSelectionEngine.HandleKeyEvent( _rKEvt );
        mpImpl->m_bDoingQuickSelection = false;
        if ( bHandled )
            return true;
    }

    return false;
}


//JP 28.3.2001: new Drag & Drop API
sal_Int8 SvTreeListBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if (rEvt.mbLeaving || !CheckDragAndDropMode(g_pDDSource, rEvt.mnAction))
    {
        ImplShowTargetEmphasis(m_pTargetEntry, false);
    }
    else if (m_nDragDropMode == DragDropMode::NONE)
    {
        SAL_WARN( "svtools.contnr", "SvTreeListBox::QueryDrop(): no target" );
    }
    else
    {
        SvTreeListEntry* pEntry = GetDropTarget( rEvt.maPosPixel );
        if( !IsDropFormatSupported( SotClipboardFormatId::TREELISTBOX ) )
        {
            SAL_WARN( "svtools.contnr", "SvTreeListBox::QueryDrop(): no format" );
        }
        else
        {
            DBG_ASSERT(g_pDDSource, "SvTreeListBox::QueryDrop(): SourceBox == 0");
            if (!( pEntry && g_pDDSource->GetModel() == GetModel()
                    && DND_ACTION_MOVE == rEvt.mnAction
                    && (pEntry->nEntryFlags & SvTLEntryFlags::DISABLE_DROP)))
            {
                nRet = rEvt.mnAction;
            }
        }

        // **** draw emphasis ****
        if( DND_ACTION_NONE == nRet )
            ImplShowTargetEmphasis(m_pTargetEntry, false);
        else if (pEntry != m_pTargetEntry || !(m_nImpFlags & SvTreeListBoxFlags::TARGEMPH_VIS))
        {
            ImplShowTargetEmphasis(m_pTargetEntry, false);
            m_pTargetEntry = pEntry;
            ImplShowTargetEmphasis(m_pTargetEntry, true);
        }
    }
    return nRet;
}

sal_Int8 SvTreeListBox::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    assert(g_pDDSource);
    g_pDDSource->EnableSelectionAsDropTarget();

    ImplShowTargetEmphasis(m_pTargetEntry, false);
    g_pDDTarget = this;

    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

    sal_Int8 nRet;
    if( aData.HasFormat( SotClipboardFormatId::TREELISTBOX ))
        nRet = rEvt.mnAction;
    else
        nRet = DND_ACTION_NONE;

    if( DND_ACTION_NONE != nRet )
    {
        nRet = DND_ACTION_NONE;

        SvTreeListEntry* pTarget = m_pTargetEntry; // may be 0!

        if( DND_ACTION_COPY == rEvt.mnAction )
        {
            if (CopySelection(g_pDDSource, pTarget))
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_MOVE == rEvt.mnAction )
        {
            if (MoveSelectionCopyFallbackPossible( g_pDDSource, pTarget, false ))
                nRet = rEvt.mnAction;
        }
        else if( DND_ACTION_COPYMOVE == rEvt.mnAction )
        {
            if (MoveSelectionCopyFallbackPossible(g_pDDSource, pTarget, true))
                nRet = rEvt.mnAction;
        }
    }
    return nRet;
}

/**
 * This sets the global variables used to determine the
 * in-process drag source.
 */
void SvTreeListBox::SetupDragOrigin()
{
    g_pDDSource = this;
    g_pDDTarget = nullptr;
}

void SvTreeListBox::StartDrag( sal_Int8, const Point& rPosPixel )
{
    if(!isDisposed())
    {
        // tdf#143114 do not start drag when a Button/Checkbox is in
        // drag-before-ButtonUp mode (CaptureMouse() active)
        if (m_pImpl->IsCaptureOnButtonActive())
            return;
    }

    m_nOldDragMode = GetDragDropMode();
    if (m_nOldDragMode == DragDropMode::NONE)
        return;

    ReleaseMouse();

    SvTreeListEntry* pEntry = GetEntry( rPosPixel ); // GetDropTarget( rPos );
    if( !pEntry )
    {
        DragFinished( DND_ACTION_NONE );
        return;
    }

    rtl::Reference<TransferDataContainer> xContainer = m_xTransferHelper;

    if (!xContainer)
    {
        xContainer.set(new TransferDataContainer);
        // apparently some (unused) content is needed
        xContainer->CopyAnyData( SotClipboardFormatId::TREELISTBOX,
                                    "unused", SAL_N_ELEMENTS("unused") );
    }

    m_nDragDropMode = NotifyStartDrag();
    if (m_nDragDropMode == DragDropMode::NONE || 0 == GetSelectionCount())
    {
        m_nDragDropMode = m_nOldDragMode;
        DragFinished( DND_ACTION_NONE );
        return;
    }

    SetupDragOrigin();

    bool bOldUpdateMode = Control::IsUpdateMode();
    Control::SetUpdateMode( true );
    PaintImmediately();
    Control::SetUpdateMode( bOldUpdateMode );

    // Disallow using the selection and its children as drop targets.
    // Important: If the selection of the SourceListBox is changed in the
    // DropHandler, the entries have to be allowed as drop targets again:
    // (GetSourceListBox()->EnableSelectionAsDropTarget( true, true );)
    EnableSelectionAsDropTarget( false );

    // Removal happens in the DragFinishHdl_Impl callback, which also calls DragFinish.
    // Removal also happens in dispose() (which also gets called from the dtor),
    // so it can't be called for a deleted object.
    AddBoxToDDList_Impl(*this);
    xContainer->StartDrag(this, mnDragAction, LINK(this, SvTreeListBox, DragFinishHdl_Impl));
}

void SvTreeListBox::SetDragHelper(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants)
{
    m_xTransferHelper = rHelper;
    mnDragAction = eDNDConstants;
}

void SvTreeListBox::DragFinished( sal_Int8
#ifndef UNX
nAction
#endif
)
{
    EnableSelectionAsDropTarget();

#ifndef UNX
    if (   (nAction == DND_ACTION_MOVE)
        && (   (g_pDDTarget && (g_pDDTarget->GetModel() != GetModel()))
            || !g_pDDTarget))
    {
        RemoveSelection();
    }
#endif

    UnsetDropTarget();
    g_pDDSource = nullptr;
    g_pDDTarget = nullptr;
    m_nDragDropMode = m_nOldDragMode;
}

void SvTreeListBox::UnsetDropTarget()
{
    if (m_pTargetEntry)
    {
        ImplShowTargetEmphasis(m_pTargetEntry, false);
        m_pTargetEntry = nullptr;
    }
}

DragDropMode SvTreeListBox::NotifyStartDrag()
{
    return DragDropMode(0xffff);
}

namespace
{
    // void* to avoid loplugin:vclwidgets, we don't need ownership here
    std::set<const void*> gSortLBoxes;
}

void SvTreeListBox::AddBoxToDDList_Impl( const SvTreeListBox& rB )
{
    gSortLBoxes.insert( &rB );
}

void SvTreeListBox::RemoveBoxFromDDList_Impl( const SvTreeListBox& rB )
{
    gSortLBoxes.erase( &rB );
}

IMPL_LINK( SvTreeListBox, DragFinishHdl_Impl, sal_Int8, nAction, void )
{
    auto &rSortLBoxes = gSortLBoxes;
    auto it = rSortLBoxes.find(this);
    if( it != rSortLBoxes.end() )
    {
        DragFinished( nAction );
        rSortLBoxes.erase( it );
    }
}

/*
    Bugs/TODO

    - calculate rectangle when editing in-place (bug with some fonts)
    - SetSpaceBetweenEntries: offset is not taken into account in SetEntryHeight
*/

OUString SvTreeListBox::SearchEntryTextWithHeadTitle( SvTreeListEntry* pEntry )
{
    assert(pEntry);
    OUStringBuffer sRet;

    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    while( nCur < nCount )
    {
        SvLBoxItem& rItem = pEntry->GetItem( nCur );
        if ( (rItem.GetType() == SvLBoxItemType::String) &&
             !static_cast<SvLBoxString&>( rItem ).GetText().isEmpty() )
        {
            sRet.append(static_cast<SvLBoxString&>( rItem ).GetText() + ",");
        }
        nCur++;
    }

    if (!sRet.isEmpty())
        sRet.remove(sRet.getLength() - 1, 1);
    return sRet.makeStringAndClear();
}

SvTreeListBox::~SvTreeListBox()
{
    disposeOnce();
}

void SvTreeListBox::dispose()
{
    if (IsMouseCaptured())
        ReleaseMouse();

    if (m_pImpl)
    {
        m_pImpl->CallEventListeners(VclEventId::ObjectDying);
        m_pImpl.reset();
    }
    if( mpImpl )
    {
        m_aTabs.clear();

        m_pEdCtrl.reset();

        m_pModel.reset();

        SvTreeListBox::RemoveBoxFromDDList_Impl( *this );

        if (this == g_pDDSource)
            g_pDDSource = nullptr;
        if (this == g_pDDTarget)
            g_pDDTarget = nullptr;
        mpImpl.reset();
    }

    m_DataTable.clear();

    DropTargetHelper::dispose();
    DragSourceHelper::dispose();
    Control::dispose();
}

void SvTreeListBox::SetNoAutoCurEntry( bool b )
{
    m_pImpl->SetNoAutoCurEntry(b);
}

void SvTreeListBox::SetSublistOpenWithLeftRight()
{
    m_pImpl->m_bSubLstOpLR = true;
}

void SvTreeListBox::Resize()
{
    if( IsEditingActive() )
        EndEditing( true );

    Control::Resize();

    m_pImpl->Resize();
    m_nFocusWidth = -1;
    m_pImpl->ShowCursor(false);
    m_pImpl->ShowCursor(true);
}


namespace {

/* Cases:

   A) entries have bitmaps
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
   B) entries don't have bitmaps  (=>via WindowBits because of D&D!)
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
*/
enum class TreeListButtonType
{
    NO_BUTTONS,
    NODE_BUTTONS,
    NODE_AND_CHECK_BUTTONS,
    CHECK_BUTTONS,
};

}

#define TABFLAGS_TEXT (SvLBoxTabFlags::DYNAMIC |        \
                       SvLBoxTabFlags::ADJUST_LEFT |    \
                       SvLBoxTabFlags::EDITABLE |       \
                       SvLBoxTabFlags::SHOW_SELECTION)

#define TABFLAGS_CONTEXTBMP (SvLBoxTabFlags::DYNAMIC | SvLBoxTabFlags::ADJUST_CENTER)

#define TABFLAGS_CHECKBTN (SvLBoxTabFlags::DYNAMIC |        \
                           SvLBoxTabFlags::ADJUST_CENTER)

#define TAB_STARTPOS    2

// take care of GetTextOffset when doing changes
void SvTreeListBox::SetTabs()
{
    // Moved to SvTreeListBox::Paint to make inplace editing work for X11 in the enhancement patch
    // tdf#139663 Rename objects from tree view in navigator.
    // if( IsEditingActive() )
    //   EndEditing( true );
    m_nTreeFlags &= ~SvTreeFlags::RECALCTABS;
    m_nFocusWidth = -1;
    const WinBits nStyle( GetStyle() );
    bool bHasButtons = (nStyle & WB_HASBUTTONS)!=0;
    bool bHasButtonsAtRoot = (nStyle & (WB_HASLINESATROOT |
                                              WB_HASBUTTONSATROOT))!=0;
    tools::Long nStartPos = TAB_STARTPOS;
    tools::Long nNodeWidthPixel = GetExpandedNodeBmp().GetSizePixel().Width();

    // pCheckButtonData->Width() knows nothing about the native checkbox width,
    // so we have mnCheckboxItemWidth which becomes valid when something is added.
    tools::Long nCheckWidth = 0;
    if (m_nTreeFlags & SvTreeFlags::CHKBTN)
        nCheckWidth = mnCheckboxItemWidth;
    tools::Long nCheckWidthDIV2 = nCheckWidth / 2;

    tools::Long nContextWidth = m_nContextBmpWidthMax;
    tools::Long nContextWidthDIV2 = nContextWidth / 2;

    // Remember hidden state of tabs
    std::vector<bool> hiddenState(m_aTabs.size());
    for (size_t n = 0; n < m_aTabs.size(); ++n)
        hiddenState[n] = m_aTabs[n]->IsHidden();

    m_aTabs.clear();

    TreeListButtonType eButtonType = TreeListButtonType::NO_BUTTONS;
    if (!(m_nTreeFlags & SvTreeFlags::CHKBTN))
    {
        if( bHasButtons )
            eButtonType = TreeListButtonType::NODE_BUTTONS;
    }
    else
    {
        if( bHasButtons )
            eButtonType = TreeListButtonType::NODE_AND_CHECK_BUTTONS;
        else
            eButtonType = TreeListButtonType::CHECK_BUTTONS;
    }

    switch(eButtonType)
    {
        case TreeListButtonType::NO_BUTTONS:
            nStartPos += nContextWidthDIV2;  // because of centering
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if (m_nContextBmpWidthMax)
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case TreeListButtonType::NODE_BUTTONS:
            if( bHasButtonsAtRoot )
                nStartPos += (m_nIndent + (nNodeWidthPixel / 2));
            else
                nStartPos += nContextWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            // add an indent if the context bitmap can't be centered without touching the expander
            if (m_nContextBmpWidthMax > m_nIndent + (nNodeWidthPixel / 2))
                nStartPos += m_nIndent;
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if (m_nContextBmpWidthMax)
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case TreeListButtonType::NODE_AND_CHECK_BUTTONS:
            if( bHasButtonsAtRoot )
                nStartPos += (m_nIndent + nNodeWidthPixel);
            else
                nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if (m_nContextBmpWidthMax)
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case TreeListButtonType::CHECK_BUTTONS:
            nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if (m_nContextBmpWidthMax)
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;
    }

    for (size_t n = 0; n < std::min(m_aTabs.size(), hiddenState.size()); ++n)
    {
        if (hiddenState[n])
            m_aTabs[n]->nFlags |= SvLBoxTabFlags::HIDDEN;
        else
            m_aTabs[n]->nFlags &= ~SvLBoxTabFlags::HIDDEN;
    }

    m_pImpl->NotifyTabsChanged();
}

void SvTreeListBox::InitEntry(SvTreeListEntry& rEntry, const OUString& aStr,
                              const Image& aCollEntryBmp, const Image& aExpEntryBmp)
{
    if (m_nTreeFlags & SvTreeFlags::CHKBTN)
    {
        rEntry.AddItem(std::make_unique<SvLBoxButton>(m_pCheckButtonData));
    }

    rEntry.AddItem(
        std::make_unique<SvLBoxContextBmp>(aCollEntryBmp, aExpEntryBmp, mbContextBmpExpanded));

    rEntry.AddItem(std::make_unique<SvLBoxString>(aStr));
}

OUString SvTreeListBox::GetEntryText(SvTreeListEntry* pEntry) const
{
    assert(pEntry);
    SvLBoxString* pItem = static_cast<SvLBoxString*>(pEntry->GetFirstItem(SvLBoxItemType::String));
    if (pItem) // There may be entries without text items, e.g. in IconView
        return pItem->GetText();
    return {};
}

const Image& SvTreeListBox::GetExpandedEntryBmp(const SvTreeListEntry* pEntry)
{
    assert(pEntry);
    const SvLBoxContextBmp* pItem = static_cast<const SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));
    assert(pItem);
    return pItem->GetBitmap2( );
}

const Image& SvTreeListBox::GetCollapsedEntryBmp( const SvTreeListEntry* pEntry )
{
    assert(pEntry);
    const SvLBoxContextBmp* pItem = static_cast<const SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));
    assert(pItem);
    return pItem->GetBitmap1( );
}

IMPL_LINK( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData, void )
{
    m_pHdlEntry = pData->GetActEntry();
    CheckButtonHdl();
}

SvTreeListEntry* SvTreeListBox::InsertEntry(
    const OUString& rText,
    SvTreeListEntry* pParent,
    bool bChildrenOnDemand, sal_uInt32 nPos,
    OUString* pUser
)
{
    m_nTreeFlags |= SvTreeFlags::MANINS;

    const Image& rDefExpBmp = m_pImpl->GetDefaultEntryExpBmp();
    const Image& rDefColBmp = m_pImpl->GetDefaultEntryColBmp();

    m_aCurInsertedExpBmp = rDefExpBmp;
    m_aCurInsertedColBmp = rDefColBmp;

    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->SetUserData( pUser );
    InitEntry(*pEntry, rText, rDefColBmp, rDefExpBmp);
    pEntry->EnableChildrenOnDemand( bChildrenOnDemand );

    if( !pParent )
        Insert( pEntry, nPos );
    else
        Insert( pEntry, pParent, nPos );

    m_aPrevInsertedExpBmp = rDefExpBmp;
    m_aPrevInsertedColBmp = rDefColBmp;

    m_nTreeFlags &= ~SvTreeFlags::MANINS;

    return pEntry;
}

void SvTreeListBox::SetEntryText(SvTreeListEntry* pEntry, const OUString& rStr)
{
    SvLBoxString* pItem = static_cast<SvLBoxString*>(pEntry->GetFirstItem(SvLBoxItemType::String));
    assert(pItem);
    pItem->SetText(rStr);
    pItem->InitViewData(*this, pEntry);
    GetModel()->InvalidateEntry( pEntry );
}

void SvTreeListBox::SetExpandedEntryBmp( SvTreeListEntry* pEntry, const Image& aBmp )
{
    SvLBoxContextBmp* pItem = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));

    assert(pItem);
    pItem->SetBitmap2( aBmp );

    ModelHasEntryInvalidated(pEntry);
    CalcEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = m_pImpl->UpdateContextBmpWidthVector(pEntry, static_cast<short>(aSize.Width()));
    if (nWidth > m_nContextBmpWidthMax)
    {
        m_nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::SetCollapsedEntryBmp(SvTreeListEntry* pEntry,const Image& aBmp )
{
    SvLBoxContextBmp* pItem = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));

    assert(pItem);
    pItem->SetBitmap1( aBmp );

    ModelHasEntryInvalidated(pEntry);
    CalcEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = m_pImpl->UpdateContextBmpWidthVector(pEntry, static_cast<short>(aSize.Width()));
    if (nWidth > m_nContextBmpWidthMax)
    {
        m_nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::CheckBoxInserted(SvTreeListEntry* pEntry)
{
    SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
    if( pItem )
    {
        auto nWidth = pItem->GetWidth(*this, pEntry);
        if( mnCheckboxItemWidth < nWidth )
        {
            mnCheckboxItemWidth = nWidth;
            m_nTreeFlags |= SvTreeFlags::RECALCTABS;
        }
    }
}

void SvTreeListBox::ImpEntryInserted( SvTreeListEntry* pEntry )
{
    SvTreeListEntry* pParent = m_pModel->GetParent(pEntry);
    if( pParent )
    {
        SvTLEntryFlags nFlags = pParent->GetFlags();
        nFlags &= ~SvTLEntryFlags::NO_NODEBMP;
        pParent->SetFlags( nFlags );
    }

    if (!((m_nTreeFlags & SvTreeFlags::MANINS) && (m_aPrevInsertedExpBmp == m_aCurInsertedExpBmp)
          && (m_aPrevInsertedColBmp == m_aCurInsertedColBmp)))
    {
        Size aSize = GetCollapsedEntryBmp( pEntry ).GetSizePixel();
        if (aSize.Width() > m_nContextBmpWidthMax)
        {
            m_nContextBmpWidthMax = static_cast<short>(aSize.Width());
            m_nTreeFlags |= SvTreeFlags::RECALCTABS;
        }
        aSize = GetExpandedEntryBmp( pEntry ).GetSizePixel();
        if (aSize.Width() > m_nContextBmpWidthMax)
        {
            m_nContextBmpWidthMax = static_cast<short>(aSize.Width());
            m_nTreeFlags |= SvTreeFlags::RECALCTABS;
        }
    }
    CalcEntryHeight( pEntry );

    if (!(m_nTreeFlags & SvTreeFlags::CHKBTN))
        return;

    CheckBoxInserted(pEntry);
}

void SvTreeListBox::SetCheckButtonState( SvTreeListEntry* pEntry, SvButtonState eState)
{
    if (!(m_nTreeFlags & SvTreeFlags::CHKBTN))
        return;

    SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
    if(!pItem)
        return ;
    switch( eState )
    {
        case SvButtonState::Checked:
            pItem->SetStateChecked();
            break;

        case SvButtonState::Unchecked:
            pItem->SetStateUnchecked();
            break;

        case SvButtonState::Tristate:
            pItem->SetStateTristate();
            break;
    }
    InvalidateEntry( pEntry );
}

SvButtonState SvTreeListBox::GetCheckButtonState( SvTreeListEntry* pEntry ) const
{
    SvButtonState eState = SvButtonState::Unchecked;
    if (pEntry && (m_nTreeFlags & SvTreeFlags::CHKBTN))
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
        if(!pItem)
            return SvButtonState::Tristate;
        SvItemStateFlags nButtonFlags = pItem->GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }
    return eState;
}

bool SvTreeListBox::GetCheckButtonEnabled(SvTreeListEntry* pEntry) const
{
    if (pEntry && (m_nTreeFlags & SvTreeFlags::CHKBTN))
    {
        SvLBoxButton* pItem
            = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
        if (pItem)
            return pItem->isEnable();
    }
    return false;
}

void SvTreeListBox::CheckButtonHdl()
{
    if (m_pCheckButtonData)
        CallEventListeners(VclEventId::CheckboxToggle,
                           static_cast<void*>(m_pCheckButtonData->GetActEntry()));
}


// TODO: Currently all data is cloned so that they conform to the default tree
// view format. Actually, the model should be used as a reference here. This
// leads to us _not_ calling SvTreeListEntry::Clone, but only its base class
// SvTreeListEntry.

SvTreeListEntry* SvTreeListBox::CloneEntry(const SvTreeListEntry& rSource)
{
    OUString aStr;
    Image aCollEntryBmp;
    Image aExpEntryBmp;

    const SvLBoxString* pStringItem
        = static_cast<const SvLBoxString*>(rSource.GetFirstItem(SvLBoxItemType::String));
    if( pStringItem )
        aStr = pStringItem->GetText();
    const SvLBoxContextBmp* pBmpItem
        = static_cast<const SvLBoxContextBmp*>(rSource.GetFirstItem(SvLBoxItemType::ContextBmp));
    if( pBmpItem )
    {
        aCollEntryBmp = pBmpItem->GetBitmap1( );
        aExpEntryBmp  = pBmpItem->GetBitmap2( );
    }
    SvTreeListEntry* pClone = new SvTreeListEntry;
    InitEntry(*pClone, aStr, aCollEntryBmp, aExpEntryBmp);
    pClone->Clone(rSource);
    pClone->EnableChildrenOnDemand(rSource.HasChildrenOnDemand());
    pClone->SetUserData(rSource.GetUserData());

    return pClone;
}

const Image& SvTreeListBox::GetDefaultExpandedEntryBmp( ) const
{
    return m_pImpl->GetDefaultEntryExpBmp();
}

const Image& SvTreeListBox::GetDefaultCollapsedEntryBmp( ) const
{
    return m_pImpl->GetDefaultEntryColBmp();
}

void SvTreeListBox::SetDefaultExpandedEntryBmp( const Image& aBmp )
{
    Size aSize = aBmp.GetSizePixel();
    if (aSize.Width() > m_nContextBmpWidthMax)
        m_nContextBmpWidthMax = static_cast<short>(aSize.Width());
    SetTabs();

    m_pImpl->SetDefaultEntryExpBmp(aBmp);
}

void SvTreeListBox::SetDefaultCollapsedEntryBmp( const Image& aBmp )
{
    Size aSize = aBmp.GetSizePixel();
    if (aSize.Width() > m_nContextBmpWidthMax)
        m_nContextBmpWidthMax = static_cast<short>(aSize.Width());
    SetTabs();

    m_pImpl->SetDefaultEntryColBmp(aBmp);
}

void SvTreeListBox::EnableCheckButton(SvLBoxButtonData& rData)
{
    m_pCheckButtonData = &rData;
    m_nTreeFlags |= SvTreeFlags::CHKBTN;
    rData.SetLink( LINK(this, SvTreeListBox, CheckButtonClick));

    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

const Image& SvTreeListBox::GetDefaultExpandedNodeImage( )
{
    return SvImpLBox::GetDefaultExpandedNodeImage( );
}

const Image& SvTreeListBox::GetDefaultCollapsedNodeImage( )
{
    return SvImpLBox::GetDefaultCollapsedNodeImage( );
}

void SvTreeListBox::SetNodeDefaultImages()
{
    SetExpandedNodeBmp(GetDefaultExpandedNodeImage());
    SetCollapsedNodeBmp(GetDefaultCollapsedNodeImage());
    SetTabs();
}

bool SvTreeListBox::EditingEntry( SvTreeListEntry* )
{
    return true;
}

bool SvTreeListBox::EditedEntry(SvTreeListEntry&, const SvLBoxItem&, const OUString&)
{
    return true;
}

void SvTreeListBox::EnableInplaceEditing( bool bOn )
{
    if (bOn)
        m_nImpFlags |= SvTreeListBoxFlags::EDT_ENABLED;
    else
        m_nImpFlags &= ~SvTreeListBoxFlags::EDT_ENABLED;
}

void SvTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    // under OS/2, we get key up/down even while editing
    if( IsEditingActive() )
        return;

    if (!m_pImpl->KeyInput(rKEvt))
    {
        bool bHandled = HandleKeyInput( rKEvt );
        if ( !bHandled )
            Control::KeyInput( rKEvt );
    }
}

void SvTreeListBox::RequestingChildren( SvTreeListEntry* pParent )
{
    if( !pParent->HasChildren() )
        InsertEntry( u"<dummy>"_ustr, pParent );
}

void SvTreeListBox::GetFocus()
{
    //If there is no item in the tree, draw focus.
    if( !First())
    {
        Invalidate();
    }
    m_pImpl->GetFocus();
    Control::GetFocus();

    SvTreeListEntry* pEntry = FirstSelected();
    if ( !pEntry )
    {
        pEntry = m_pImpl->GetCurEntry();
    }
    if (m_pImpl->m_pCursor)
    {
        if (pEntry != m_pImpl->m_pCursor)
            pEntry = m_pImpl->m_pCursor;
    }
    if ( pEntry )
        CallEventListeners(VclEventId::ListboxTreeFocus, pEntry);
}

void SvTreeListBox::LoseFocus()
{
    // If there is no item in the tree, delete visual focus.
    if ( !First() )
        Invalidate();
    if (m_pImpl)
        m_pImpl->LoseFocus();
    Control::LoseFocus();
}

void SvTreeListBox::ModelHasCleared()
{
    m_pImpl->m_pCursor = nullptr; // else we crash in GetFocus when editing in-place
    m_pTargetEntry = nullptr;
    m_pEdCtrl.reset();
    m_pImpl->Clear();
    m_nFocusWidth = -1;

    m_nContextBmpWidthMax = 0;
    SetDefaultExpandedEntryBmp( GetDefaultExpandedEntryBmp() );
    SetDefaultCollapsedEntryBmp( GetDefaultCollapsedEntryBmp() );

    if (!(m_nTreeFlags & SvTreeFlags::FIXEDHEIGHT))
        m_nEntryHeight = 0;
    AdjustEntryHeight();
    AdjustEntryHeight( GetDefaultExpandedEntryBmp() );
    AdjustEntryHeight( GetDefaultCollapsedEntryBmp() );
}

bool SvTreeListBox::PosOverBody(const Point& rPos) const
{
    if (rPos.X() < 0 || rPos.Y() < 0)
        return false;
    Size aSize(GetSizePixel());
    if (rPos.X() > aSize.Width() || rPos.Y() > aSize.Height())
        return false;
    if (m_pImpl->m_aVerSBar->IsVisible())
    {
        tools::Rectangle aRect(m_pImpl->m_aVerSBar->GetPosPixel(),
                               m_pImpl->m_aVerSBar->GetSizePixel());
        if (aRect.Contains(rPos))
            return false;
    }
    if (m_pImpl->m_aHorSBar->IsVisible())
    {
        tools::Rectangle aRect(m_pImpl->m_aHorSBar->GetPosPixel(),
                               m_pImpl->m_aHorSBar->GetSizePixel());
        if (aRect.Contains(rPos))
            return false;
    }
    return true;
}

void SvTreeListBox::ScrollOutputArea( short nDeltaEntries )
{
    if (!nDeltaEntries || !m_pImpl->m_aVerSBar->IsVisible())
        return;

    tools::Long nThumb = m_pImpl->m_aVerSBar->GetThumbPos();
    tools::Long nMax = m_pImpl->m_aVerSBar->GetRange().Max();

    if( nDeltaEntries < 0 )
    {
        // move window up
        nDeltaEntries *= -1;
        tools::Long nVis = m_pImpl->m_aVerSBar->GetVisibleSize();
        tools::Long nTemp = nThumb + nVis;
        if( nDeltaEntries > (nMax - nTemp) )
            nDeltaEntries = static_cast<short>(nMax - nTemp);
        m_pImpl->PageDown(static_cast<sal_uInt16>(nDeltaEntries));
    }
    else
    {
        if( nDeltaEntries > nThumb )
            nDeltaEntries = static_cast<short>(nThumb);
        m_pImpl->PageUp(static_cast<sal_uInt16>(nDeltaEntries));
    }
    m_pImpl->SyncVerThumb();
}

void SvTreeListBox::ScrollToAbsPos( tools::Long nPos )
{
    m_pImpl->ScrollToAbsPos(nPos);
}

void SvTreeListBox::SetSelectionMode( SelectionMode eSelectMode )
{
    m_eSelMode = eSelectMode;
    m_pImpl->SetSelectionMode(eSelectMode);
}

void SvTreeListBox::SetDragDropMode( DragDropMode nDDMode )
{
    m_nDragDropMode = nDDMode;
    m_pImpl->SetDragDropMode(nDDMode);
}

void SvTreeListBox::CalcEntryHeight( SvTreeListEntry const * pEntry )
{
    short nHeightMax=0;
    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    SvViewDataEntry* pViewData = GetViewDataEntry( pEntry );
    while( nCur < nCount )
    {
        auto nHeight = SvLBoxItem::GetHeight(pViewData, nCur);
        if( nHeight > nHeightMax )
            nHeightMax = nHeight;
        nCur++;
    }

    if (nHeightMax > m_nEntryHeight)
    {
        m_nEntryHeight = nHeightMax;
        Control::SetFont( GetFont() );
        m_pImpl->SetEntryHeight();
    }
}

void SvTreeListBox::SetEntryHeight( short nHeight )
{
    if (nHeight > m_nEntryHeight)
    {
        m_nEntryHeight = nHeight;
        if (m_nEntryHeight)
            m_nTreeFlags |= SvTreeFlags::FIXEDHEIGHT;
        else
            m_nTreeFlags &= ~SvTreeFlags::FIXEDHEIGHT;
        Control::SetFont( GetFont() );
        m_pImpl->SetEntryHeight();
    }
}

void SvTreeListBox::SetEntryWidth( short nWidth )
{
    m_nEntryWidth = nWidth;
}

void SvTreeListBox::AdjustEntryHeight( const Image& rBmp )
{
    const Size aSize( rBmp.GetSizePixel() );
    if (aSize.Height() > m_nEntryHeight)
    {
        m_nEntryHeight = static_cast<short>(aSize.Height()) + m_nEntryHeightOffs;
        m_pImpl->SetEntryHeight();
    }
}

void SvTreeListBox::AdjustEntryHeight()
{
    tools::Long nHeight = GetTextHeight();
    if (nHeight > m_nEntryHeight)
    {
        m_nEntryHeight = static_cast<short>(nHeight) + m_nEntryHeightOffs;
        m_pImpl->SetEntryHeight();
    }
}

bool SvTreeListBox::Expand( SvTreeListEntry* pParent )
{
    m_pHdlEntry = pParent;
    bool bExpanded = false;
    SvTLEntryFlags nFlags;

    if( pParent->HasChildrenOnDemand() )
        RequestingChildren( pParent );
    bool bExpandAllowed = pParent->HasChildren() && ExpandingHdl();
    // double check if the expander callback ended up removing all children
    if (pParent->HasChildren())
    {
        if (bExpandAllowed)
        {
            bExpanded = true;
            ExpandListEntry( pParent );
            m_pImpl->EntryExpanded(pParent);
            m_pHdlEntry = pParent;
            ExpandedHdl();
        }
        nFlags = pParent->GetFlags();
        nFlags &= ~SvTLEntryFlags::NO_NODEBMP;
        nFlags |= SvTLEntryFlags::HAD_CHILDREN;
        pParent->SetFlags( nFlags );
    }
    else
    {
        nFlags = pParent->GetFlags();
        nFlags |= SvTLEntryFlags::NO_NODEBMP;
        pParent->SetFlags( nFlags );
        GetModel()->InvalidateEntry( pParent ); // repaint
    }

    // #i92103#
    if ( bExpanded )
    {
        CallEventListeners(VclEventId::ItemExpanded, pParent);
    }

    return bExpanded;
}

bool SvTreeListBox::Collapse( SvTreeListEntry* pParent )
{
    m_pHdlEntry = pParent;
    bool bCollapsed = false;

    if( ExpandingHdl() )
    {
        bCollapsed = true;
        m_pImpl->CollapsingEntry(pParent);
        CollapseListEntry( pParent );
        m_pImpl->EntryCollapsed(pParent);
        m_pHdlEntry = pParent;
        ExpandedHdl();
    }

    // #i92103#
    if ( bCollapsed )
    {
        CallEventListeners(VclEventId::ItemCollapsed, pParent);
    }

    return bCollapsed;
}

bool SvTreeListBox::Select( SvTreeListEntry* pEntry, bool bSelect )
{
    DBG_ASSERT(pEntry,"Select: Null-Ptr");
    bool bRetVal = SelectListEntry( pEntry, bSelect );
    DBG_ASSERT(IsSelected(pEntry)==bSelect,"Select failed");
    if( bRetVal )
    {
        m_pImpl->EntrySelected(pEntry, bSelect);
        m_pHdlEntry = pEntry;
        if( bSelect )
        {
            SelectHdl();
            CallEventListeners( VclEventId::ListboxTreeSelect, pEntry);
        }
        else
            DeselectHdl();
    }
    return bRetVal;
}

sal_uInt32 SvTreeListBox::SelectChildren( const SvTreeListEntry* pParent, bool bSelect )
{
    m_pImpl->DestroyAnchor();
    sal_uInt32 nRet = 0;
    if( !pParent->HasChildren() )
        return 0;
    sal_uInt16 nRefDepth = m_pModel->GetDepth(pParent);
    SvTreeListEntry* pChild = FirstChild( pParent );
    do {
        nRet++;
        Select( pChild, bSelect );
        pChild = Next( pChild );
    } while (pChild && m_pModel->GetDepth(pChild) > nRefDepth);
    return nRet;
}

void SvTreeListBox::SelectAll( bool bSelect )
{
    m_pImpl->SelAllDestrAnch(bSelect,
                             true, // delete anchor,
                             true); // even when using SelectionMode::Single, deselect the cursor
}

void SvTreeListBox::ModelHasInsertedTree( SvTreeListEntry* pEntry )
{
    sal_uInt16 nRefDepth = m_pModel->GetDepth(pEntry);
    SvTreeListEntry* pTmp = pEntry;
    do
    {
        ImpEntryInserted( pTmp );
        pTmp = Next( pTmp );
    } while (pTmp && nRefDepth < m_pModel->GetDepth(pTmp));
    m_pImpl->TreeInserted(pEntry);
}

void SvTreeListBox::ModelHasInserted( SvTreeListEntry* pEntry )
{
    ImpEntryInserted( pEntry );
    m_pImpl->EntryInserted(pEntry);
}

void SvTreeListBox::ModelIsMoving(SvTreeListEntry* pSource )
{
    m_pImpl->MovingEntry(pSource);
}

void SvTreeListBox::ModelHasMoved( SvTreeListEntry* pSource )
{
    m_pImpl->EntryMoved(pSource);
}

void SvTreeListBox::ModelIsRemoving( SvTreeListEntry* pEntry )
{
    if (m_pEdEntry == pEntry)
        m_pEdEntry = nullptr;

    m_pImpl->RemovingEntry(pEntry);
}

void SvTreeListBox::ModelHasRemoved( SvTreeListEntry* pEntry  )
{
    // WARNING WARNING WARNING
    // The supplied pointer should have been deleted
    // before this call. Be careful not to use it!!!
    if (pEntry == m_pHdlEntry)
        m_pHdlEntry = nullptr;

    if (pEntry == m_pTargetEntry)
        m_pTargetEntry = nullptr;

    m_pImpl->EntryRemoved();
}

void SvTreeListBox::SetCollapsedNodeBmp( const Image& rBmp)
{
    AdjustEntryHeight( rBmp );
    m_pImpl->SetCollapsedNodeBmp(rBmp);
}

void SvTreeListBox::SetExpandedNodeBmp( const Image& rBmp )
{
    AdjustEntryHeight( rBmp );
    m_pImpl->SetExpandedNodeBmp(rBmp);
}


void SvTreeListBox::SetFont( const vcl::Font& rFont )
{
    vcl::Font aTempFont( rFont );
    vcl::Font aOrigFont( GetFont() );
    aTempFont.SetTransparent( true );
    if (aTempFont == aOrigFont)
        return;
    Control::SetFont( aTempFont );

    aTempFont.SetColor(aOrigFont.GetColor());
    aTempFont.SetFillColor(aOrigFont.GetFillColor());
    aTempFont.SetTransparent(aOrigFont.IsTransparent());

    if (aTempFont == aOrigFont)
        return;

    AdjustEntryHeightAndRecalc();
}

void SvTreeListBox::AdjustEntryHeightAndRecalc()
{
    AdjustEntryHeight();
    // always invalidate, else things go wrong in SetEntryHeight
    RecalcViewData();
}

void SvTreeListBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Control::Paint(rRenderContext, rRect);
    if (m_nTreeFlags & SvTreeFlags::RECALCTABS)
    {
        if (IsEditingActive())
            EndEditing(true);
        SetTabs();
    }
    m_pImpl->Paint(rRenderContext, rRect);

    //Add visual focus draw
    if (First())
        return;

    if (HasFocus())
    {
        tools::Long nHeight = rRenderContext.GetTextHeight();
        tools::Rectangle aRect(Point(0, 0), Size(GetSizePixel().Width(), nHeight));
        ShowFocus(aRect);
    }
    else
    {
        HideFocus();
    }
}

void SvTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    // tdf#143114 remember the *correct* starting entry
    m_pImpl->m_pCursorOld
        = (rMEvt.IsLeft() && (m_nTreeFlags & SvTreeFlags::CHKBTN) && mnClicksToToggle > 0)
              ? GetEntry(rMEvt.GetPosPixel())
              : nullptr;

    m_pImpl->MouseButtonDown(rMEvt);
}

void SvTreeListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    // tdf#116675 clicking on an entry should toggle its checkbox
    // tdf#143114 use the already created starting entry and if it exists
    if (nullptr != m_pImpl->m_pCursorOld)
    {
        const Point aPnt = rMEvt.GetPosPixel();
        SvTreeListEntry* pEntry = GetEntry(aPnt);

        // compare if MouseButtonUp *is* on the same entry, regardless of scrolling
        // or other things
        if (pEntry && pEntry->m_Items.size() > 0 && 1 == mnClicksToToggle
            && pEntry == m_pImpl->m_pCursorOld)
        {
            SvLBoxItem* pItem = GetItem(pEntry, aPnt.X());
            // if the checkbox button was clicked, that will be toggled later, do not toggle here
            // anyway users probably don't want to toggle the checkbox by clickink on another button
            if (!pItem || pItem->GetType() != SvLBoxItemType::Button)
            {
                SvLBoxButton* pItemCheckBox
                    = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
                if (pItemCheckBox && pItemCheckBox->isEnable() && GetItemPos(pEntry, 0).first < aPnt.X() - GetMapMode().GetOrigin().X())
                {
                    pItemCheckBox->ClickHdl(pEntry);
                    InvalidateEntry(pEntry);
                }
            }
        }
    }

    m_pImpl->MouseButtonUp(rMEvt);
}

void SvTreeListBox::MouseMove( const MouseEvent& rMEvt )
{
    m_pImpl->MouseMove(rMEvt);
}

void SvTreeListBox::SetUpdateMode( bool bUpdate )
{
    m_pImpl->SetUpdateMode(bUpdate);
}

void SvTreeListBox::SetSpaceBetweenEntries( short nOffsLogic )
{
    if (nOffsLogic != m_nEntryHeightOffs)
    {
        m_nEntryHeight = m_nEntryHeight - m_nEntryHeightOffs;
        m_nEntryHeightOffs = nOffsLogic;
        m_nEntryHeight = m_nEntryHeight + nOffsLogic;
        AdjustEntryHeightAndRecalc();
        m_pImpl->SetEntryHeight();
    }
}

void SvTreeListBox::SetCurEntry( SvTreeListEntry* pEntry )
{
    m_pImpl->SetCurEntry(pEntry);
}

Image const & SvTreeListBox::GetExpandedNodeBmp( ) const
{
    return m_pImpl->GetExpandedNodeBmp();
}

Point SvTreeListBox::GetEntryPosition(const SvTreeListEntry* pEntry) const
{
    return m_pImpl->GetEntryPosition(pEntry);
}

void SvTreeListBox::MakeVisible( SvTreeListEntry* pEntry )
{
    m_pImpl->MakeVisible(pEntry);
}

void SvTreeListBox::MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop )
{
    m_pImpl->MakeVisible(pEntry, bMoveToTop);
}

void SvTreeListBox::ModelHasEntryInvalidated( SvTreeListEntry* pEntry )
{

    // reinitialize the separate items of the entries
    sal_uInt16 nCount = pEntry->ItemCount();
    for( sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++ )
    {
        SvLBoxItem& rItem = pEntry->GetItem( nIdx );
        rItem.InitViewData(*this, pEntry);
    }

    // repaint
    m_pImpl->InvalidateEntry(pEntry);
}

void SvTreeListBox::EditItemText(SvTreeListEntry& rEntry, SvLBoxString& rItem,
                                 const Selection& rSelection)
{
    if (IsSelected(&rEntry))
    {
        m_pImpl->ShowCursor(false);
        SelectListEntry(&rEntry, false);
        m_pImpl->InvalidateEntry(&rEntry);
        SelectListEntry(&rEntry, true);
        m_pImpl->ShowCursor(true);
    }
    m_pEdEntry = &rEntry;
    m_pEdItem = &rItem;
    SvLBoxTab* pTab = GetTab(rEntry, rItem);
    DBG_ASSERT(pTab,"EditItemText:Tab not found");

    auto nItemHeight(rItem.GetHeight(*this, &rEntry));
    Point aPos = GetEntryPosition(&rEntry);
    aPos.AdjustY((m_nEntryHeight - nItemHeight) / 2);
    aPos.setX(GetTabPos(&rEntry, pTab));
    tools::Long nOutputWidth = m_pImpl->GetOutputSize().Width();
    Size aSize( nOutputWidth - aPos.X(), nItemHeight );
    sal_uInt16 nPos = std::find_if( m_aTabs.begin(), m_aTabs.end(),
                        [pTab](const std::unique_ptr<SvLBoxTab>& p) { return p.get() == pTab; })
                      - m_aTabs.begin();
    if( nPos+1 < static_cast<sal_uInt16>(m_aTabs.size()) )
    {
        SvLBoxTab* pRightTab = m_aTabs[nPos + 1].get();
        tools::Long nRight = GetTabPos(&rEntry, pRightTab);
        if( nRight <= nOutputWidth )
            aSize.setWidth( nRight - aPos.X() );
    }
    Point aOrigin( GetMapMode().GetOrigin() );
    aPos += aOrigin; // convert to win coordinates
    aSize.AdjustWidth( -(aOrigin.X()) );
    tools::Rectangle aRect( aPos, aSize );
    EditText(rItem.GetText(), aRect, rSelection);
}

void SvTreeListBox::EditEntry( SvTreeListEntry* pEntry )
{
    m_pImpl->m_aEditClickPos = Point(-1, -1);
    ImplEditEntry( pEntry );
}

void SvTreeListBox::ImplEditEntry( SvTreeListEntry* pEntry )
{
    if( IsEditingActive() )
        EndEditing();
    if( !pEntry )
        pEntry = GetCurEntry();
    if( !pEntry )
        return;

    tools::Long nClickX = m_pImpl->m_aEditClickPos.X();
    bool bIsMouseTriggered = nClickX >= 0;

    SvLBoxString* pItem = nullptr;
    sal_uInt16 nCount = pEntry->ItemCount();
    tools::Long nTabPos, nNextTabPos = 0;
    for( sal_uInt16 i = 0 ; i < nCount ; i++ )
    {
        SvLBoxItem& rTmpItem = pEntry->GetItem( i );
        if (rTmpItem.GetType() != SvLBoxItemType::String)
            continue;

        SvLBoxTab* pTab = GetTab(*pEntry, rTmpItem);
        nNextTabPos = -1;
        if( i < nCount - 1 )
        {
            SvLBoxItem& rNextItem = pEntry->GetItem( i + 1 );
            SvLBoxTab* pNextTab = GetTab(*pEntry, rNextItem);
            nNextTabPos = pNextTab->GetPos();
        }

        if( pTab && pTab->IsEditable() )
        {
            nTabPos = pTab->GetPos();
            if( !bIsMouseTriggered || (nClickX > nTabPos && (nNextTabPos == -1 || nClickX < nNextTabPos ) ) )
            {
                pItem = static_cast<SvLBoxString*>( &rTmpItem );
                break;
            }
        }
    }

    if( pItem && EditingEntry( pEntry ) )
    {
        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        SelectAll( false );
        MakeVisible( pEntry );
        EditItemText(*pEntry, *pItem, aSel);
    }
}

void SvTreeListBox::EditedText( const OUString& rStr )

{
    if (m_pEdEntry) // we have to check if this entry is null that means that it is removed while editing
    {
        assert(m_pEdItem);
        if (EditedEntry(*m_pEdEntry, *m_pEdItem, rStr))
        {
            m_pEdItem->SetText(rStr);
            m_pModel->InvalidateEntry(m_pEdEntry);
        }
        if( GetSelectionCount() == 0 )
            Select(m_pEdEntry);
        if( GetSelectionMode() == SelectionMode::Multiple && !GetCurEntry() )
            SetCurEntry(m_pEdEntry);
    }
}

SvTreeListEntry* SvTreeListBox::GetDropTarget( const Point& rPos )
{
    // scroll
    if( rPos.Y() < 12 )
    {
        ImplShowTargetEmphasis(m_pTargetEntry, false);
        ScrollOutputArea( +1 );
    }
    else
    {
        Size aSize(m_pImpl->GetOutputSize());
        if( rPos.Y() > aSize.Height() - 12 )
        {
            ImplShowTargetEmphasis(m_pTargetEntry, false);
            ScrollOutputArea( -1 );
        }
    }

    SvTreeListEntry* pTarget = m_pImpl->GetEntry(rPos);
    // when dropping in a vacant space, use the last entry
    if( !pTarget )
        return LastVisible();
    else if( (GetDragDropMode() & DragDropMode::ENABLE_TOP) &&
             pTarget == First() && rPos.Y() < 6 )
        return nullptr;

    return pTarget;
}


SvTreeListEntry* SvTreeListBox::GetEntry( const Point& rPos, bool bHit ) const
{
    SvTreeListEntry* pEntry = m_pImpl->GetEntry(rPos);
    if( pEntry && bHit )
    {
        tools::Long nLine = m_pImpl->GetEntryLine(pEntry);
        if (!(m_pImpl->EntryReallyHit(*pEntry, rPos, nLine)))
            return nullptr;
    }
    return pEntry;
}

SvTreeListEntry* SvTreeListBox::GetCurEntry() const
{
    return m_pImpl ? m_pImpl->GetCurEntry() : nullptr;
}

void SvTreeListBox::ImplInitStyle()
{
    const WinBits nWindowStyle = GetStyle();

    m_nTreeFlags |= SvTreeFlags::RECALCTABS;
    if (nWindowStyle & WB_SORT)
    {
        GetModel()->SetSortMode(SvSortMode::Ascending);
        GetModel()->SetCompareHdl(LINK(this, SvTreeListBox, DefaultCompare));
    }
    else
    {
        GetModel()->SetSortMode(SvSortMode::None);
        GetModel()->SetCompareHdl(Link<const SvSortData&,sal_Int32>());
    }
    m_pImpl->SetStyle(nWindowStyle);
    m_pImpl->Resize();
    Invalidate();
}

void SvTreeListBox::InvalidateEntry(SvTreeListEntry* pEntry)
{
    DBG_ASSERT(pEntry,"InvalidateEntry:No Entry");
    if (pEntry)
    {
        GetModel()->InvalidateEntry(pEntry);
    }
}

void SvTreeListBox::PaintEntry1(SvTreeListEntry& rEntry, tools::Long nLine, vcl::RenderContext& rRenderContext)
{
    tools::Rectangle aRect; // multi purpose

    bool bHorSBar = m_pImpl->HasHorScrollBar();

    m_pImpl->UpdateContextBmpWidthMax(&rEntry);

    if (m_nTreeFlags & SvTreeFlags::RECALCTABS)
        SetTabs();

    short nTempEntryHeight = GetEntryHeight();
    tools::Long nWidth = m_pImpl->GetOutputSize().Width();

    // Did we turn on the scrollbar within PreparePaints? If yes, we have to set
    // the ClipRegion anew.
    if (!bHorSBar && m_pImpl->HasHorScrollBar())
        rRenderContext.SetClipRegion(vcl::Region(m_pImpl->GetClipRegionRect()));

    Point aEntryPos(rRenderContext.GetMapMode().GetOrigin());
    aEntryPos.setX( aEntryPos.X() * -1 ); // conversion document coordinates
    tools::Long nMaxRight = nWidth + aEntryPos.X() - 1;

    Color aBackupTextColor(rRenderContext.GetTextColor());
    vcl::Font aBackupFont(rRenderContext.GetFont());
    Color aBackupColor = rRenderContext.GetFillColor();

    bool bCurFontIsSel = false;
    // if a ClipRegion was set from outside, we don't have to reset it
    const WinBits nWindowStyle = GetStyle();
    const bool bHideSelection = (nWindowStyle & WB_HIDESELECTION) !=0 && !HasFocus();
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();

    vcl::Font aHighlightFont(rRenderContext.GetFont());
    const Color aHighlightTextColor(rSettings.GetHighlightTextColor());
    aHighlightFont.SetColor(aHighlightTextColor);

    Size aRectSize(0, nTempEntryHeight);

    SvViewDataEntry* pViewDataEntry = GetViewDataEntry( &rEntry );
    const bool bSeparator(rEntry.IsSeparator());

    const auto nMaxContextBmpWidthBeforeIndentIsNeeded
        = m_nIndent + GetExpandedNodeBmp().GetSizePixel().Width() / 2;
    const bool bHasButtonsAtRoot = nWindowStyle & WB_HASBUTTONSATROOT;

    const size_t nTabCount = m_aTabs.size();

    // Get image information

    sal_uInt16 nFirstDynTabPos(0);
    SvLBoxTab* pFirstDynamicTab = GetFirstDynamicTab(nFirstDynTabPos);

    const Image* pImg = nullptr;
    Point aImagePos;
    Size aImageSize;
    bool bDefaultImage = false;
    bool bExpanded = false;

    const bool bNeedsImage = (!(rEntry.GetFlags() & SvTLEntryFlags::NO_NODEBMP))
                             && (nWindowStyle & WB_HASBUTTONS) && pFirstDynamicTab
                             && (rEntry.HasChildren() || rEntry.HasChildrenOnDemand());
    if (bNeedsImage)
    {
        tools::Long nDynTabPos = GetTabPos(&rEntry, pFirstDynamicTab);
        nDynTabPos += m_pImpl->m_nNodeBmpTabDistance;
        nDynTabPos += m_pImpl->m_nNodeBmpWidth / 2;
        nDynTabPos += 4; // 4 pixels of buffer, so the node bitmap is not too close to the next tab

        // find first tab and check if the node bitmap extends into it
        sal_uInt16 nNextTab = nFirstDynTabPos;
        SvLBoxTab* pNextTab;
        do
        {
            nNextTab++;
            pNextTab = nNextTab < nTabCount ? m_aTabs[nNextTab].get() : nullptr;
        } while (pNextTab && pNextTab->IsDynamic());

        if (pNextTab && (GetTabPos(&rEntry, pNextTab) <= nDynTabPos))
            return;

        if (!((nWindowStyle & WB_HASBUTTONSATROOT) || m_pModel->GetDepth(&rEntry) > 0))
            return;

        aImagePos = Point(GetTabPos(&rEntry, pFirstDynamicTab), nLine);
        aImagePos.AdjustX(m_pImpl->m_nNodeBmpTabDistance);

        bExpanded = IsExpanded(&rEntry);
        if (bExpanded)
            pImg = &m_pImpl->GetExpandedNodeBmp();
        else
            pImg = &m_pImpl->GetCollapsedNodeBmp();
        bDefaultImage = bExpanded ? *pImg == GetDefaultExpandedNodeImage()
                                  : *pImg == GetDefaultCollapsedNodeImage();
        aImageSize = pImg->GetSizePixel();
        aImagePos.AdjustY((nTempEntryHeight - aImageSize.Height()) / 2);
    }

    // Draw items

    const size_t nItemCount = rEntry.ItemCount();

    const bool bEntryHighlighted = pViewDataEntry->IsHighlighted();
    // We need to track, if the area for the image area has selection background; otherwise,
    // the symbol may be drawn using aHighlightTextColor (usually white) on white background
    bool bImageHighlighted = false;

    for (size_t nCurIndex = 0; nCurIndex < nTabCount && nCurIndex < nItemCount; ++nCurIndex)
    {
        SvLBoxTab* pTab = m_aTabs[nCurIndex].get();

        if (pTab->IsHidden())
            continue;

        const size_t nNextTab = nCurIndex + 1;
        SvLBoxTab* pNextTab = nNextTab < nTabCount ? m_aTabs[nNextTab].get() : nullptr;
        SvLBoxItem& rItem = rEntry.GetItem(nCurIndex);

        SvLBoxTabFlags nFlags = pTab->nFlags;
        Size aSize(rItem.GetWidth(*this, pViewDataEntry, nCurIndex),
                   SvLBoxItem::GetHeight(pViewDataEntry, nCurIndex));
        tools::Long nTabPos = GetTabPos(&rEntry, pTab);

        tools::Long nNextTabPos;
        if (pNextTab)
            nNextTabPos = GetTabPos(&rEntry, pNextTab);
        else
        {
            nNextTabPos = nMaxRight;
            if (nTabPos > nMaxRight)
                nNextTabPos += 50;
        }

        tools::Long nX;
        if( pTab->nFlags & SvLBoxTabFlags::ADJUST_RIGHT )
            // avoid cutting the right edge off the tab separation
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), (nNextTabPos - SV_TAB_BORDER - 1) - nTabPos);
        else
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), nNextTabPos - nTabPos);

        // add an indent if the context bitmap can't be centered without touching the expander
        if (nCurIndex == 0 && !(m_nTreeFlags & SvTreeFlags::CHKBTN) && bHasButtonsAtRoot &&
                pTab->nFlags & SvLBoxTabFlags::ADJUST_CENTER &&
                !(pTab->nFlags & SvLBoxTabFlags::FORCE) &&
                aSize.Width() > nMaxContextBmpWidthBeforeIndentIsNeeded)
            nX += m_nIndent;

        aEntryPos.setX( nX );
        aEntryPos.setY( nLine );

        // set background pattern/color

        Wallpaper aWallpaper = rRenderContext.GetBackground();

        bool bSelTab = bool(nFlags & SvLBoxTabFlags::SHOW_SELECTION);

        if (bEntryHighlighted && bSelTab)
        {
            Color aNewWallColor = rSettings.GetHighlightColor();
            // if the face color is bright then the deactivate color is also bright
            // -> so you can't see any deactivate selection
            if (bHideSelection && !rSettings.GetFaceColor().IsBright()
               && aWallpaper.GetColor().IsBright() != rSettings.GetDeactiveColor().IsBright())
            {
                aNewWallColor = rSettings.GetDeactiveColor();
            }
            // set font color to highlight
            if (!bCurFontIsSel)
            {
                rRenderContext.SetTextColor(aHighlightTextColor);
                rRenderContext.SetFont(aHighlightFont);
                bCurFontIsSel = true;
            }
            aWallpaper.SetColor(aNewWallColor);
        }
        else  // no selection
        {
            if (bCurFontIsSel || rEntry.GetTextColor())
            {
                bCurFontIsSel = false;
                if (const auto & xCustomTextColor = rEntry.GetTextColor())
                    rRenderContext.SetTextColor(*xCustomTextColor);
                else
                    rRenderContext.SetTextColor(aBackupTextColor);
                rRenderContext.SetFont(aBackupFont);
            }
        }

        // draw background
        if (!(m_nTreeFlags & SvTreeFlags::USESEL))
        {
            // only draw the area that is used by the item
            aRectSize.setWidth( aSize.Width() );
            aRect.SetPos(aEntryPos);
            aRect.SetSize(aRectSize);
        }
        else
        {
            // draw from the current to the next tab
            if (nCurIndex != 0)
                aRect.SetLeft( nTabPos );
            else
                // if we're in the 0th tab, always draw from column 0 --
                // else we get problems with centered tabs
                aRect.SetLeft( 0 );
            aRect.SetTop( nLine );
            aRect.SetBottom( nLine + nTempEntryHeight - 1 );
            if (pNextTab)
            {
                tools::Long nRight;
                nRight = GetTabPos(&rEntry, pNextTab) - 1;
                if (nRight > nMaxRight)
                    nRight = nMaxRight;
                aRect.SetRight( nRight );
            }
            else
            {
                aRect.SetRight( nMaxRight );
            }
        }
        // A custom selection that starts at a tab position > 0, do not fill
        // the background of the 0th item, else e.g. we might not be able to
        // realize tab listboxes with lines.
        if (!(nCurIndex == 0 && (m_nTreeFlags & SvTreeFlags::USESEL) && m_nFirstSelTab))
        {
            Color aBackgroundColor = aWallpaper.GetColor();
            if (aBackgroundColor != COL_TRANSPARENT)
            {
                rRenderContext.SetFillColor(aBackgroundColor);
                // this case may occur for smaller horizontal resizes
                if (aRect.Left() < aRect.Right())
                {
                    rRenderContext.DrawRect(aRect);
                    if (!bImageHighlighted && bNeedsImage && bDefaultImage && bEntryHighlighted)
                    {
                        if (!aRect.GetIntersection({ aImagePos, aImageSize }).IsEmpty())
                            bImageHighlighted = true;
                    }
                }
            }
        }
        // draw item
        // center vertically
        aEntryPos.AdjustY((nTempEntryHeight - aSize.Height()) / 2 );

        rItem.Paint(aEntryPos, *this, rRenderContext, pViewDataEntry, rEntry);

        // division line between tabs (but not if this is a separator line)
        if (!bSeparator && pNextTab && rItem.GetType() == SvLBoxItemType::String &&
            // not at the right edge of the window!
            aRect.Right() < nMaxRight)
        {
            aRect.SetLeft( aRect.Right() - SV_TAB_BORDER );
            rRenderContext.DrawRect(aRect);
        }

        rRenderContext.SetFillColor(aBackupColor);
    }

    if (pViewDataEntry->IsDragTarget())
    {
        auto popIt = rRenderContext.ScopedPush();
        rRenderContext.SetLineColor(rSettings.GetDeactiveColor());
        rRenderContext.SetFillColor(rSettings.GetDeactiveColor());

        const bool bAsTree = GetStyle() & (WB_HASLINES | WB_HASLINESATROOT);
        if (bAsTree)
        {
            rRenderContext.DrawRect(tools::Rectangle(Point(0, nLine + nTempEntryHeight - 2), Size(nWidth, 2)));
            rRenderContext.DrawRect(tools::Rectangle(Point(0, nLine), Size(nWidth, 2)));
        }
        else
        {
            rRenderContext.DrawRect(tools::Rectangle(Point(0, nLine), Size(nWidth, 2)));
        }
    }

    if (bCurFontIsSel || rEntry.GetTextColor())
    {
        rRenderContext.SetTextColor(aBackupTextColor);
        rRenderContext.SetFont(aBackupFont);
    }

    if (!bNeedsImage)
        return;

    if (!bDefaultImage)
    {
        // If it's a custom image then draw what was explicitly set to use
        DrawImageFlags nStyle = DrawImageFlags::NONE;
        if (!IsEnabled())
            nStyle |= DrawImageFlags::Disable;
        rRenderContext.DrawImage(aImagePos, *pImg, nStyle);
    }
    else
    {
        bool bNativeOK = false;
        // native
        if (rRenderContext.IsNativeControlSupported(ControlType::ListNode, ControlPart::Entire))
        {
            ImplControlValue aControlValue;
            tools::Rectangle aCtrlRegion(aImagePos,  pImg->GetSizePixel());
            ControlState nState = ControlState::NONE;

            if (IsEnabled())
                nState |= ControlState::ENABLED;

            if (bExpanded)
                aControlValue.setTristateVal(ButtonValue::On); //expanded node
            else
            {
                if ((!rEntry.HasChildren()) && rEntry.HasChildrenOnDemand() &&
                    (!(rEntry.GetFlags() & SvTLEntryFlags::HAD_CHILDREN)))
                {
                    aControlValue.setTristateVal( ButtonValue::DontKnow ); //don't know
                }
                else
                {
                    aControlValue.setTristateVal( ButtonValue::Off ); //collapsed node
                }
            }

            bNativeOK = rRenderContext.DrawNativeControl(ControlType::ListNode, ControlPart::Entire, aCtrlRegion, nState, aControlValue, OUString());
        }
        if (!bNativeOK)
        {
            DecorationView aDecoView(&rRenderContext);
            DrawSymbolFlags nSymbolStyle = DrawSymbolFlags::NONE;
            if (!IsEnabled())
                nSymbolStyle |= DrawSymbolFlags::Disable;

            Color aCol = aBackupTextColor;
            if (bImageHighlighted)
                aCol = aHighlightTextColor;

            SymbolType eSymbol = bExpanded ? SymbolType::SPIN_DOWN : SymbolType::SPIN_RIGHT;
            aDecoView.DrawSymbol(tools::Rectangle(aImagePos, pImg->GetSizePixel()), eSymbol, aCol, nSymbolStyle);
        }
    }
}

void SvTreeListBox::DrawCustomEntry(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const SvTreeListEntry& rEntry)
{
    m_aCustomRenderHdl.Call(
        std::tuple<vcl::RenderContext&, const tools::Rectangle&, const SvTreeListEntry&>(
            rRenderContext, rRect, rEntry));
}

Size SvTreeListBox::MeasureCustomEntry(vcl::RenderContext& rRenderContext, const SvTreeListEntry& rEntry) const
{
    return m_aCustomMeasureHdl.Call(
        std::pair<vcl::RenderContext&, const SvTreeListEntry&>(rRenderContext, rEntry));
}

tools::Rectangle SvTreeListBox::GetFocusRect(const SvTreeListEntry* pEntry, tools::Long nLine )
{
    m_pImpl->UpdateContextBmpWidthMax(pEntry);

    Size aSize;
    tools::Rectangle aRect;
    aRect.SetTop( nLine );
    aSize.setHeight( GetEntryHeight() );

    tools::Long nRealWidth = m_pImpl->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    sal_uInt16 nCurTab;
    SvLBoxTab* pTab = GetFirstTab( SvLBoxTabFlags::SHOW_SELECTION, nCurTab );
    tools::Long nTabPos = 0;
    if( pTab )
        nTabPos = GetTabPos( pEntry, pTab );
    tools::Long nNextTabPos;
    if (pTab && nCurTab < m_aTabs.size() - 1)
    {
        SvLBoxTab* pNextTab = m_aTabs[nCurTab + 1].get();
        nNextTabPos = GetTabPos( pEntry, pNextTab );
    }
    else
    {
        nNextTabPos = nRealWidth;
        if( nTabPos > nRealWidth )
            nNextTabPos += 50;
    }

    bool bUserSelection = bool(m_nTreeFlags & SvTreeFlags::USESEL);
    if( !bUserSelection )
    {
        if( pTab && nCurTab < pEntry->ItemCount() )
        {
            const SvLBoxItem& rItem = pEntry->GetItem( nCurTab );
            aSize.setWidth(rItem.GetWidth(*this, pEntry));
            if( !aSize.Width() )
                aSize.setWidth( 15 );
            tools::Long nX = nTabPos; //GetTabPos( pEntry, pTab );
            // alignment
            nX += pTab->CalcOffset( aSize.Width(), nNextTabPos - nTabPos );
            aRect.SetLeft( nX );
            // make sure that first and last letter aren't cut off slightly
            aRect.SetSize( aSize );
            if( aRect.Left() > 0 )
                aRect.AdjustLeft( -1 );
            aRect.AdjustRight( 1 );
        }
    }
    else
    {
        // if SelTab != 0, we have to calculate also
        if (m_nFocusWidth == -1 || m_nFirstSelTab)
        {
            SvLBoxTab* pLastTab = nullptr; // default to select whole width

            sal_uInt16 nLastTab;
            GetLastTab(SvLBoxTabFlags::SHOW_SELECTION, nLastTab);
            sal_uInt32 nTrailingTab = nLastTab + 1;
            if (nTrailingTab < m_aTabs.size()) // is there another one?
                pLastTab = m_aTabs[nTrailingTab].get();

            aSize.setWidth( pLastTab ? pLastTab->GetPos() : 0x0fffffff );
            m_nFocusWidth = static_cast<short>(aSize.Width());
            if( pTab )
                m_nFocusWidth = m_nFocusWidth - static_cast<short>(nTabPos); //pTab->GetPos();
        }
        else
        {
            aSize.setWidth(m_nFocusWidth);
            if( pTab )
            {
                if( nCurTab )
                    aSize.AdjustWidth(nTabPos );
                else
                    aSize.AdjustWidth(pTab->GetPos() ); // Tab0 always from the leftmost position
            }
        }
        // if selection starts with 0th tab, draw from column 0 on
        if( nCurTab != 0 )
        {
            aRect.SetLeft( nTabPos );
            aSize.AdjustWidth( -nTabPos );
        }
        aRect.SetSize( aSize );
    }
    // adjust right edge because of clipping
    if( aRect.Right() >= nRealWidth )
    {
        aRect.SetRight( nRealWidth-1 );
        m_nFocusWidth = static_cast<short>(aRect.GetWidth());
    }
    return aRect;
}

tools::Long SvTreeListBox::GetTabPos(const SvTreeListEntry* pEntry, const SvLBoxTab* pTab) const
{
    assert(pTab);
    tools::Long nPos = pTab->GetPos();
    if( pTab->IsDynamic() )
    {
        sal_uInt16 nDepth = m_pModel->GetDepth(pEntry);
        nDepth = nDepth * static_cast<sal_uInt16>(m_nIndent);
        nPos += nDepth;
    }
    return nPos + (pEntry->GetExtraIndent() * m_nIndent);
}

std::pair<tools::Long, tools::Long> SvTreeListBox::GetItemPos(SvTreeListEntry* pEntry, sal_uInt16 nTabIdx)
{
    sal_uInt16 nTabCount = m_aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    if (nTabIdx >= nItemCount || nTabIdx >= nTabCount)
        return std::make_pair(-1, -1);

    SvLBoxTab* pTab = m_aTabs.front().get();
    SvLBoxItem* pItem = &pEntry->GetItem(nTabIdx);
    sal_uInt16 nNextItem = nTabIdx + 1;

    tools::Long nRealWidth = m_pImpl->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    SvLBoxTab* pNextTab = nNextItem < nTabCount ? m_aTabs[nNextItem].get() : nullptr;
    tools::Long nStart = GetTabPos(pEntry, pTab);

    tools::Long nNextTabPos;
    if (pNextTab)
        nNextTabPos = GetTabPos(pEntry, pNextTab);
    else
    {
        nNextTabPos = nRealWidth;
        if (nStart > nRealWidth)
            nNextTabPos += 50;
    }

    auto nItemWidth(pItem->GetWidth(*this, pEntry));
    nStart += pTab->CalcOffset(nItemWidth, nNextTabPos - nStart);
    auto nLen = nItemWidth;
    if (pNextTab)
    {
        tools::Long nTabWidth = GetTabPos(pEntry, pNextTab) - nStart;
        if (nTabWidth < nLen)
            nLen = nTabWidth;
    }
    return std::make_pair(nStart, nLen);
}

tools::Long SvTreeListBox::getPreferredDimensions(std::vector<tools::Long> &rWidths) const
{
    tools::Long nHeight = 0;
    rWidths.clear();
    SvTreeListEntry* pEntry = First();
    while (pEntry)
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCurPos = 0;
        if (nCount > rWidths.size())
            rWidths.resize(nCount);
        while (nCurPos < nCount)
        {
            SvLBoxItem& rItem = pEntry->GetItem( nCurPos );
            auto nWidth = rItem.GetWidth(*this, pEntry);
            if (nWidth)
            {
                nWidth += SV_TAB_BORDER * 2;
                if (nWidth > rWidths[nCurPos])
                   rWidths[nCurPos] = nWidth;
            }
            ++nCurPos;
        }
        pEntry = Next( pEntry );
        nHeight += GetEntryHeight();
    }
    return nHeight;
}

Size SvTreeListBox::GetOptimalSize() const
{
    std::vector<tools::Long> aWidths;
    Size aRet(0, getPreferredDimensions(aWidths));
    for (tools::Long aWidth : aWidths)
        aRet.AdjustWidth(aWidth );

    sal_Int32 nLeftBorder(0), nTopBorder(0), nRightBorder(0), nBottomBorder(0);
    GetBorder(nLeftBorder, nTopBorder, nRightBorder, nBottomBorder);
    aRet.AdjustWidth(nLeftBorder + nRightBorder);
    aRet.AdjustHeight(nTopBorder + nBottomBorder);

    tools::Long nMinWidth = m_nMinWidthInChars * approximate_char_width();
    aRet.setWidth( std::max(aRet.Width(), nMinWidth) );

    if (GetStyle() & WB_VSCROLL)
        aRet.AdjustWidth(GetSettings().GetStyleSettings().GetScrollBarSize());

    return aRet;
}

void SvTreeListBox::SetForceMakeVisible(bool bEnable) { m_pImpl->SetForceMakeVisible(bEnable); }

SvLBoxItem* SvTreeListBox::GetItem(SvTreeListEntry* pEntry,tools::Long nX,SvLBoxTab** ppTab)
{
    SvLBoxItem* pItemClicked = nullptr;
    sal_uInt16 nTabCount = m_aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    SvLBoxTab* pTab = m_aTabs.front().get();
    SvLBoxItem* pItem = &pEntry->GetItem(0);
    sal_uInt16 nNextItem = 1;
    nX -= GetMapMode().GetOrigin().X();
    tools::Long nRealWidth = m_pImpl->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    while (true)
    {
        SvLBoxTab* pNextTab = nNextItem < nTabCount ? m_aTabs[nNextItem].get() : nullptr;
        tools::Long nStart = GetTabPos(pEntry, pTab);

        tools::Long nNextTabPos;
        if (pNextTab)
            nNextTabPos = GetTabPos(pEntry, pNextTab);
        else
        {
            nNextTabPos = nRealWidth;
            if (nStart > nRealWidth)
                nNextTabPos += 50;
        }

        auto nItemWidth(pItem->GetWidth(*this, pEntry));
        nStart += pTab->CalcOffset(nItemWidth, nNextTabPos - nStart);
        auto nLen = nItemWidth;
        if (pNextTab)
        {
            tools::Long nTabWidth = GetTabPos(pEntry, pNextTab) - nStart;
            if (nTabWidth < nLen)
                nLen = nTabWidth;
        }

        if (nX >= nStart && nX < (nStart + nLen))
        {
            pItemClicked = pItem;
            if (ppTab)
            {
                *ppTab = pTab;
                break;
            }
        }
        if (nNextItem >= nItemCount || nNextItem >= nTabCount)
            break;
        pTab = m_aTabs[nNextItem].get();
        pItem = &pEntry->GetItem(nNextItem);
        nNextItem++;
    }
    return pItemClicked;
}

SvLBoxItem* SvTreeListBox::GetItem(SvTreeListEntry* pEntry,tools::Long nX )
{
    SvLBoxTab* pDummyTab;
    return GetItem(pEntry, nX, &pDummyTab);
}

void SvTreeListBox::AddTab(tools::Long nTabPos, SvLBoxTabFlags nFlags )
{
    m_nFocusWidth = -1;
    SvLBoxTab* pTab = new SvLBoxTab( nTabPos, nFlags );
    m_aTabs.emplace_back(pTab);
    if (m_nTreeFlags & SvTreeFlags::USESEL)
    {
        sal_uInt16 nPos = m_aTabs.size() - 1;
        if (nPos >= m_nFirstSelTab && nPos <= m_nLastSelTab)
            pTab->nFlags |= SvLBoxTabFlags::SHOW_SELECTION;
        else
            // string items usually have to be selected -- turn this off
            // explicitly
            pTab->nFlags &= ~SvLBoxTabFlags::SHOW_SELECTION;
    }
}


SvLBoxTab* SvTreeListBox::GetFirstDynamicTab( sal_uInt16& rPos ) const
{
    sal_uInt16 nCurTab = 0;
    sal_uInt16 nTabCount = m_aTabs.size();
    while( nCurTab < nTabCount )
    {
        SvLBoxTab* pTab = m_aTabs[nCurTab].get();
        if( pTab->nFlags & SvLBoxTabFlags::DYNAMIC )
        {
            rPos = nCurTab;
            return pTab;
        }
        nCurTab++;
    }
    return nullptr;
}

SvLBoxTab* SvTreeListBox::GetFirstDynamicTab() const
{
    sal_uInt16 nDummy;
    return GetFirstDynamicTab( nDummy );
}

SvLBoxTab* SvTreeListBox::GetTab(const SvTreeListEntry& rEntry, const SvLBoxItem& rItem) const
{
    sal_uInt16 nPos = rEntry.GetPos(rItem);
    return m_aTabs[nPos].get();
}

Size SvTreeListBox::GetOutputSizePixel() const
{
    Size aSize = m_pImpl->GetOutputSize();
    return aSize;
}

void SvTreeListBox::NotifyScrolled()
{
    m_aScrolledHdl.Call( this );
}

void SvTreeListBox::ImplInvalidate( const vcl::Region* pRegion, InvalidateFlags nInvalidateFlags )
{
    if (!m_pImpl)
        return;
    if (m_nFocusWidth == -1)
        // to make sure that the control doesn't show the wrong focus rectangle
        // after painting
        m_pImpl->RecalcFocusRect();
    Control::ImplInvalidate( pRegion, nInvalidateFlags );
    m_pImpl->Invalidate();
}

void SvTreeListBox::SetHighlightRange( sal_uInt16 nStart, sal_uInt16 nEnd)
{
    m_nTreeFlags |= SvTreeFlags::USESEL;
    if( nStart > nEnd )
        std::swap(nStart, nEnd);
    // select all tabs that lie within the area
    m_nTreeFlags |= SvTreeFlags::RECALCTABS;
    m_nFirstSelTab = nStart;
    m_nLastSelTab = nEnd;
    m_pImpl->RecalcFocusRect();
}

void SvTreeListBox::Command(const CommandEvent& rCEvt)
{
    if (!ImplGetWindowImpl()->maCommandHdl.Call(rCEvt))
        m_pImpl->Command(rCEvt);
    //pass at least alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
        Control::Command(rCEvt);
}

SvLBoxTab* SvTreeListBox::GetFirstTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rPos )
{
    sal_uInt16 nTabCount = m_aTabs.size();
    for( sal_uInt16 nPos = 0; nPos < nTabCount; nPos++ )
    {
        SvLBoxTab* pTab = m_aTabs[nPos].get();
        if( pTab->nFlags & nFlagMask )
        {
            rPos = nPos;
            return pTab;
        }
    }
    rPos = 0xffff;
    return nullptr;
}

void SvTreeListBox::GetLastTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos )
{
    sal_uInt16 nPos = static_cast<sal_uInt16>(m_aTabs.size());
    while( nPos )
    {
        --nPos;
        SvLBoxTab* pTab = m_aTabs[nPos].get();
        if( pTab->nFlags & nFlagMask )
        {
            rTabPos = nPos;
            return;
        }
    }
    rTabPos = 0xffff;
}

void SvTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    const Point pos(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    if (SvTreeListEntry* entry = GetEntry(pos))
    {
        const OUString tooltip = GetEntryTooltip(*entry);
        if (!tooltip.isEmpty())
        {
            const Size size(GetOutputSizePixel().Width(), GetEntryHeight());
            tools::Rectangle screenRect(OutputToScreenPixel(GetEntryPosition(entry)), size);
            Help::ShowQuickHelp(this, screenRect, tooltip);
            return;
        }
    }

    if (!m_pImpl->RequestHelp(rHEvt))
        Control::RequestHelp( rHEvt );
}

sal_Int32 SvTreeListBox::DefaultCompare(const SvLBoxString* pLeftText, const SvLBoxString* pRightText)
{
    OUString aLeft = pLeftText ? pLeftText->GetText() : OUString();
    OUString aRight = pRightText ? pRightText->GetText() : OUString();
    m_pImpl->UpdateStringSorter();
    return m_pImpl->m_pStringSorter->compare(aLeft, aRight);
}

IMPL_LINK( SvTreeListBox, DefaultCompare, const SvSortData&, rData, sal_Int32 )
{
    const SvTreeListEntry* pLeft = rData.pLeft;
    const SvTreeListEntry* pRight = rData.pRight;
    const SvLBoxString* pLeftText = static_cast<const SvLBoxString*>(pLeft->GetFirstItem(SvLBoxItemType::String));
    const SvLBoxString* pRightText = static_cast<const SvLBoxString*>(pRight->GetFirstItem(SvLBoxItemType::String));
    return DefaultCompare(pLeftText, pRightText);
}

void SvTreeListBox::ModelNotification(SvListAction nActionId, SvTreeListEntry* pEntry)
{
    SolarMutexGuard aSolarGuard;

    if( nActionId == SvListAction::CLEARING )
        CancelTextEditing();

    switch (nActionId)
    {
        case SvListAction::INSERTED:
            ActionInserted(pEntry);
            ModelHasInserted(pEntry);
            break;
        case SvListAction::INSERTED_TREE:
            ActionInsertedTree(pEntry);
            ModelHasInsertedTree(pEntry);
            break;
        case SvListAction::REMOVING:
            ModelIsRemoving(pEntry);
            ActionRemoving(pEntry);
            break;
        case SvListAction::REMOVED:
            ModelHasRemoved(pEntry);
            break;
        case SvListAction::MOVING:
            ModelIsMoving(pEntry);
            ActionMoving(pEntry);
            break;
        case SvListAction::MOVED:
            ActionMoved();
            ModelHasMoved(pEntry);
            break;
        case SvListAction::CLEARING:
            Reset();
            ModelHasCleared(); // sic! for compatibility reasons!
            break;
        case SvListAction::CLEARED:
            break;
        case SvListAction::INVALIDATE_ENTRY:
            // no action for the base class
            ModelHasEntryInvalidated(pEntry);
            break;
        case SvListAction::RESORTED:
            m_bVisPositionsValid = false;
            break;
        case SvListAction::RESORTING:
            break;
        default:
            OSL_FAIL("unknown ActionId");
    }

    switch( nActionId )
    {
        case SvListAction::INSERTED:
        {
            SvLBoxContextBmp* pBmpItem
                = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));
            if ( !pBmpItem )
                break;
            const Image& rBitmap1( pBmpItem->GetBitmap1() );
            const Image& rBitmap2( pBmpItem->GetBitmap2() );
            short nMaxWidth = short( std::max( rBitmap1.GetSizePixel().Width(), rBitmap2.GetSizePixel().Width() ) );
            nMaxWidth = m_pImpl->UpdateContextBmpWidthVector(pEntry, nMaxWidth);
            if (nMaxWidth > m_nContextBmpWidthMax)
            {
                m_nContextBmpWidthMax = nMaxWidth;
                SetTabs();
            }
            if (get_width_request() == -1)
                queue_resize();
        }
        break;

        case SvListAction::RESORTING:
            SetUpdateMode( false );
            break;

        case SvListAction::RESORTED:
            // after a selection: show first entry and also keep the selection
            MakeVisible(m_pModel->First(), true);
            SetUpdateMode( true );
            break;

        case SvListAction::CLEARED:
            if( IsUpdateMode() )
                PaintImmediately();
            break;

        default: break;
    }
}

SvTreeListEntry* SvTreeListBox::GetFirstEntryInView() const
{
    return GetEntry( Point() );
}

SvTreeListEntry* SvTreeListBox::GetNextEntryInView(SvTreeListEntry* pEntry ) const
{
    SvTreeListEntry* pNext = NextVisible( pEntry );
    if( pNext )
    {
        Point aPos( GetEntryPosition(pNext) );
        const Size& rSize = m_pImpl->GetOutputSize();
        if( aPos.Y() < 0 || aPos.Y() >= rSize.Height() )
            return nullptr;
    }
    return pNext;
}


void SvTreeListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        m_nEntryHeight = 0;   // _together_ with true of 1. par (bFont) of InitSettings() a zero-height
                              //  forces complete recalc of heights!
        InitSettings();
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvTreeListBox::StateChanged( StateChangedType eType )
{
    if( eType == StateChangedType::Enable )
        Invalidate( InvalidateFlags::Children );

    Control::StateChanged( eType );

    if ( eType == StateChangedType::Style )
        ImplInitStyle();
}

void SvTreeListBox::ApplySettings(vcl::RenderContext& rRenderContext)
{
    SetPointFont(rRenderContext, GetPointFont(*GetOutDev()));

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
    rRenderContext.SetTextFillColor();
    rRenderContext.SetBackground(rStyleSettings.GetFieldColor());

    // always try to re-create default-SvLBoxButtonData
    if (m_pCheckButtonData)
        m_pCheckButtonData->SetDefaultImages(*this);
}

void SvTreeListBox::InitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    SetPointFont(*GetOutDev(), aFont);
    AdjustEntryHeightAndRecalc();

    SetTextColor(rStyleSettings.GetFieldTextColor());
    SetTextFillColor();

    SetBackground(rStyleSettings.GetFieldColor());

    // always try to re-create default-SvLBoxButtonData
    if (m_pCheckButtonData)
        m_pCheckButtonData->SetDefaultImages(*this);
}

rtl::Reference<comphelper::OAccessible> SvTreeListBox::CreateAccessible()
{
    rtl::Reference<comphelper::OAccessible> pAccParent = GetAccessibleParent();
    if (pAccParent.is())
        return new AccessibleListBox(*this, pAccParent);

    return {};
}

tools::Rectangle SvTreeListBox::GetBoundingRect(const SvTreeListEntry* pEntry)
{
    Point aPos = GetEntryPosition( pEntry );
    tools::Rectangle aRect = GetFocusRect( pEntry, aPos.Y() );
    return aRect;
}

void SvTreeListBox::set_min_width_in_chars(sal_Int32 nChars)
{
    m_nMinWidthInChars = nChars;
    queue_resize();
}

bool SvTreeListBox::set_property(const OUString &rKey, const OUString &rValue)
{
    if (rKey == "min-width-chars")
    {
        set_min_width_in_chars(rValue.toInt32());
    }
    else if (rKey == "enable-tree-lines")
    {
        auto nStyle = GetStyle();
        nStyle &= ~(WB_HASLINES | WB_HASLINESATROOT);
        if (toBool(rValue))
            nStyle |= (WB_HASLINES | WB_HASLINESATROOT);
        SetStyle(nStyle);
    }
    else if (rKey == "show-expanders")
    {
        auto nStyle = GetStyle();
        nStyle &= ~(WB_HASBUTTONS | WB_HASBUTTONSATROOT);
        if (toBool(rValue))
            nStyle |= (WB_HASBUTTONS | WB_HASBUTTONSATROOT);
        SetStyle(nStyle);
    }
    else if (rKey == "enable-search")
    {
        SetQuickSearch(toBool(rValue));
    }
    else if (rKey == "activate-on-single-click")
    {
        SetActivateOnSingleClick(toBool(rValue));
    }
    else if (rKey == "hover-selection")
    {
        SetHoverSelection(toBool(rValue));
    }
    else if (rKey == "reorderable")
    {
        if (toBool(rValue))
            SetDragDropMode(DragDropMode::CTRL_MOVE | DragDropMode::ENABLE_TOP);
    }
    else if (rKey == "text-column")
    {
        SetTextColumnEnabled(toBool(rValue));
    }
    else
        return Control::set_property(rKey, rValue);
    return true;
}

void SvTreeListBox::EnableRTL(bool bEnable)
{
    Control::EnableRTL(bEnable);
    m_pImpl->m_aHorSBar->EnableRTL(bEnable);
    m_pImpl->m_aVerSBar->EnableRTL(bEnable);
    m_pImpl->m_aScrBarBox->EnableRTL(bEnable);
}

FactoryFunction SvTreeListBox::GetUITestFactory() const
{
    return TreeListUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
