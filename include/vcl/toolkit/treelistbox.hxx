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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/dllapi.h>

#include <deque>
#include <memory>
#include <vector>

#include <vcl/ctrl.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/image.hxx>
#include <tools/gen.hxx>
#include <tools/contnr.hxx>
#include <vcl/toolkit/treelist.hxx>
#include <vcl/toolkit/viewdataentry.hxx>
#include <vcl/transfer.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvTreeListBox;
class SvTreeListEntry;
class SvInplaceEdit2;
class SvLBoxString;
class SvImpLBox;
class SvLBoxButtonData;
class Timer;
class Edit;

enum class SvButtonState { Unchecked, Checked, Tristate };

// *********************************************************************
// *************************** Tabulators ******************************
// *********************************************************************

enum class SvLBoxTabFlags
{
    NONE             = 0x0000,
    DYNAMIC          = 0x0001, // Item's output column changes according to the Child Depth
    ADJUST_RIGHT     = 0x0002, // Item's right margin at the tabulator
    ADJUST_LEFT      = 0x0004, // Left margin
    ADJUST_CENTER    = 0x0008, // Center the item at the tabulator

    SHOW_SELECTION   = 0x0010, // Visualize selection state
                                           // Item needs to be able to return the surrounding polygon (D'n'D cursor)
    EDITABLE         = 0x0020, // Item editable at the tabulator
    FORCE            = 0x0040, // Switch off the default calculation of the first tabulator
                               // (on which Abo Tabpage/Extras/Option/Customize, etc. rely on)
                               // The first tab's position corresponds precisely to the Flags set
                               // and column widths
    HIDDEN           = 0x0080, // Tab is not visible

    ADJUST_FLAGS = ADJUST_RIGHT | ADJUST_LEFT | ADJUST_CENTER,
};
namespace o3tl
{
    template<> struct typed_flags<SvLBoxTabFlags> : is_typed_flags<SvLBoxTabFlags, 0x00ff> {};
}

#define SV_TAB_BORDER 8

#define SV_ENTRYHEIGHTOFFS_PIXEL 2

enum class SvTreeFlags
{
    CHKBTN         = 0x01,
    USESEL         = 0x02,
    MANINS         = 0x04,
    RECALCTABS     = 0x08,
    FIXEDHEIGHT    = 0x10,
};
namespace o3tl
{
    template<> struct typed_flags<SvTreeFlags> : is_typed_flags<SvTreeFlags, 0x1f> {};
}

enum class SvLBoxItemType {String, Button, ContextBmp};

class SvLBoxTab
{
    tools::Long    nPos;
public:
            SvLBoxTab();
            SvLBoxTab( tools::Long nPos, SvLBoxTabFlags nFlags );
            SvLBoxTab( const SvLBoxTab& );

    SvLBoxTabFlags nFlags;

    bool    IsDynamic() const { return bool(nFlags & SvLBoxTabFlags::DYNAMIC); }
    void    SetPos( tools::Long nNewPos) { nPos = nNewPos; }
    tools::Long    GetPos() const { return nPos; }
    tools::Long    CalcOffset( tools::Long nItemLength, tools::Long nTabWidth );
    bool    IsEditable() const { return bool(nFlags & SvLBoxTabFlags::EDITABLE); }
    bool    IsHidden() const { return bool(nFlags & SvLBoxTabFlags::HIDDEN); }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SvLBoxItem
{
protected:
    bool mbDisabled;

public:
                        SvLBoxItem();
    virtual             ~SvLBoxItem();
    virtual SvLBoxItemType GetType() const = 0;
    virtual int CalcWidth(const SvTreeListBox& rView) const;
    int GetWidth(const SvTreeListBox& rView, const SvTreeListEntry* pEntry) const;
    int GetWidth(const SvTreeListBox& rView, const SvViewDataEntry* pData,
                 sal_uInt16 nItemPos) const;
    int GetHeight(const SvTreeListBox& rView, const SvTreeListEntry* pEntry) const;
    static int GetHeight(const SvViewDataEntry* pData, sal_uInt16 nItemPos);
    void Enable(bool bEnabled) { mbDisabled = !bEnabled; }
    bool isEnable() const { return !mbDisabled; }

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext, const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) = 0;

    virtual void InitViewData(SvTreeListBox& rView, SvTreeListEntry* pEntry,
                            // If != 0: this Pointer must be used!
                            // If == 0: it needs to be retrieved via the View
                            SvViewDataItem* pViewData = nullptr) = 0;
    // View-dependent data is not cloned
    virtual std::unique_ptr<SvLBoxItem> Clone(SvLBoxItem const * pSource) const = 0;
};

enum class DragDropMode
{
    NONE            = 0x0000,
    CTRL_MOVE       = 0x0001,
    // Entries may be dropped via the uppermost Entry
    // The DropTarget is 0 in that case
    ENABLE_TOP      = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<DragDropMode> : is_typed_flags<DragDropMode, 0x0015> {};
}

enum class SvTreeListBoxFlags
{
    NONE             = 0x0000,
    IN_EDT           = 0x0001,
    EDT_ENABLED      = 0x0002,
    TARGEMPH_VIS     = 0x0004,
    EDTEND_CALLED    = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<SvTreeListBoxFlags> : is_typed_flags<SvTreeListBoxFlags, 0x000f> {};
}

struct SvTreeListBoxImpl;

typedef std::pair<vcl::RenderContext&, const SvTreeListEntry&> svtree_measure_args;
typedef std::tuple<vcl::RenderContext&, const tools::Rectangle&, const SvTreeListEntry&> svtree_render_args;

struct EntryItemText
{
    SvTreeListEntry& m_rEntry;
    const SvLBoxItem& m_rItem;
    OUString m_sText;

    EntryItemText(SvTreeListEntry& rEntry, const SvLBoxItem& rItem, const OUString& rText)
        : m_rEntry(rEntry)
        , m_rItem(rItem)
        , m_sText(rText)
    {
    }
};

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvTreeListBox : public Control,
                                                           public DropTargetHelper,
                                                           public DragSourceHelper,
                                                           public vcl::ISearchableStringList
{
    friend class SvImpLBox;
    friend class SvLBoxString;
    friend class IconViewImpl;
    friend class TreeControlPeer;
    friend class SalInstanceIconView;
    friend class SalInstanceTreeView;
    friend class SalInstanceEntryTreeView;
    friend class SvTreeList;
    friend class JSTreeView;

    using SvDataTable = std::unordered_map<SvTreeListEntry*, SvViewDataEntry>;
    SvDataTable m_DataTable; // Mapping SvTreeListEntry -> ViewData

    sal_uInt32 m_nVisibleCount;
    sal_uInt32 m_nSelectionCount;
    bool m_bVisPositionsValid;

    std::unique_ptr<SvTreeList> m_pModel;

    std::unique_ptr<SvTreeListBoxImpl> mpImpl;
    Link<SvTreeListBox*, void> m_aScrolledHdl;
    Link<SvTreeListBox*, void> m_aExpandedHdl;
    Link<SvTreeListBox*, bool> m_aExpandingHdl;
    Link<SvTreeListBox*, void> m_aSelectHdl;
    Link<SvTreeListBox*, void> m_aDeselectHdl;
    Link<SvTreeListEntry&, OUString> m_aTooltipHdl;
    Link<svtree_render_args, void> m_aCustomRenderHdl;
    Link<svtree_measure_args, Size> m_aCustomMeasureHdl;

    Image m_aPrevInsertedExpBmp;
    Image m_aPrevInsertedColBmp;
    Image m_aCurInsertedExpBmp;
    Image m_aCurInsertedColBmp;

    short m_nContextBmpWidthMax;
    short m_nEntryHeightOffs;
    short m_nIndent;
    short m_nFocusWidth;
    sal_uInt16 m_nFirstSelTab;
    sal_uInt16 m_nLastSelTab;
    tools::Long mnCheckboxItemWidth;
    bool mbContextBmpExpanded;
    bool mbQuickSearch; // Enables type-ahead search in the check list box.
    bool mbActivateOnSingleClick; // Make single click "activate" a row like a double-click normally does
    bool mbCustomEntryRenderer; // Used to define if the list entries are updated on demand
    bool mbHoverSelection; // Make mouse over a row "select" a row like a single-click normally does
    bool mbSelectingByHover; // true during "Select" if it was due to hover
    bool mbIsTextColumEnabled; // true if the property name text-column is enabled
    sal_Int8        mnClicksToToggle; // 0 == Click on a row not toggle its checkbox.
                                      // 1 == Every click on row toggle its checkbox.
                                      // 2 == First click select, second click toggle.

    SvTreeListEntry* m_pHdlEntry;

    DragDropMode m_nDragDropMode;
    DragDropMode m_nOldDragMode;
    SelectionMode m_eSelMode;
    sal_Int32 m_nMinWidthInChars;

    sal_Int8        mnDragAction;

    SvTreeListEntry* m_pEdEntry;
    SvLBoxString* m_pEdItem;

    rtl::Reference<TransferDataContainer> m_xTransferHelper;

protected:
    std::unique_ptr<SvImpLBox> m_pImpl;
    short m_nEntryHeight;
    short m_nEntryWidth;
    bool                    mbCenterAndClipText;

    Link<SvTreeListBox*, bool> m_aDoubleClickHdl;
    SvTreeListEntry* m_pTargetEntry;
    SvLBoxButtonData* m_pCheckButtonData;
    std::vector<std::unique_ptr<SvLBoxTab>> m_aTabs;
    SvTreeFlags m_nTreeFlags;
    SvTreeListBoxFlags m_nImpFlags;
    // Move/CopySelection: Position of the current Entry in SelectionList
    sal_uInt16 m_nCurEntrySelPos;

    std::unique_ptr<SvInplaceEdit2> m_pEdCtrl;

private:
    DECL_DLLPRIVATE_LINK( CheckButtonClick, SvLBoxButtonData *, void );
    DECL_DLLPRIVATE_LINK( TextEditEndedHdl_Impl, SvInplaceEdit2&, void );
    // Handler that is called by TreeList to clone an Entry
    DECL_DLLPRIVATE_LINK(CloneHdl_Impl, SvTreeListEntry&, SvTreeListEntry*);

    void ExpandListEntry(SvTreeListEntry* pParent);
    void CollapseListEntry(SvTreeListEntry* pParent);
    bool SelectListEntry(SvTreeListEntry* pEntry, bool bSelect);

    void Reset();

    void RemoveViewData(SvTreeListEntry* pParent);

    void ActionMoving(SvTreeListEntry* pEntry);
    void ActionMoved();
    void ActionInserted(SvTreeListEntry* pEntry);
    void ActionInsertedTree(SvTreeListEntry* pEntry);
    void ActionRemoving(SvTreeListEntry* pEntry);

    // Handler and methods for Drag - finished handler.
    // The Handle retrieved by GetDragFinishedHdl can be set on the
    // TransferDataContainer. This link is a callback for the DragFinished
    // call. The AddBox method is called from the GetDragFinishedHdl() and the
    // remove is called in the link callback and in the dtor. So it can't be
    // called for a deleted object.
    SAL_DLLPRIVATE static void AddBoxToDDList_Impl( const SvTreeListBox& rB );
    SAL_DLLPRIVATE static void RemoveBoxFromDDList_Impl( const SvTreeListBox& rB );
    DECL_DLLPRIVATE_LINK( DragFinishHdl_Impl, sal_Int8, void );

    // after a checkbox entry is inserted, use this to get its width to support
    // autowidth for the 1st checkbox column
    SAL_DLLPRIVATE void CheckBoxInserted(SvTreeListEntry* pEntry);

    SAL_DLLPRIVATE void DrawCustomEntry(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const SvTreeListEntry& rEntry);
    SAL_DLLPRIVATE Size MeasureCustomEntry(vcl::RenderContext& rRenderContext, const SvTreeListEntry& rEntry) const;

    /** Handles the given key event.

        At the moment this merely does typeahead if typeahead is enabled.

        @return
            <TRUE/> if the event has been consumed, <FALSE/> otherwise.
    */
    SAL_DLLPRIVATE bool HandleKeyInput(const KeyEvent& rKEvt);

    void UnsetDropTarget();

protected:

    bool            CheckDragAndDropMode( SvTreeListBox const * pSource, sal_Int8 );
    void            ImplShowTargetEmphasis( SvTreeListEntry* pEntry, bool bShow);
    void            EnableSelectionAsDropTarget( bool bEnable = true );
    // Standard impl returns 0; derived classes which support D'n'D must override
    using Window::GetDropTarget;
    virtual SvTreeListEntry* GetDropTarget( const Point& );

    // Invalidate children on enable/disable
    virtual void StateChanged( StateChangedType eType ) override;

    virtual void Insert(SvTreeListEntry* pEntry, SvTreeListEntry* pParent,
                        sal_uInt32 nPos = TREELIST_APPEND);
    virtual void Insert(SvTreeListEntry* pEntry, sal_uInt32 nRootPos = TREELIST_APPEND);

    // In-place editing
    void            EditText( const OUString&, const tools::Rectangle&,const Selection&);
    void            CancelTextEditing();

    // InitViewData is called right after CreateViewData
    // The Entry is has not yet been added to the View in InitViewData!
    virtual void InitViewData(SvViewDataEntry*, SvTreeListEntry* pEntry);
    // Calls InitViewData for all Items
    void            RecalcViewData();

    void            OnCurrentEntryChanged();

    // ISearchableStringList
    virtual vcl::StringEntryIdentifier    CurrentEntry( OUString& _out_entryText ) const override;
    virtual vcl::StringEntryIdentifier    NextEntry( vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const override;
    virtual void                            SelectEntry( vcl::StringEntryIdentifier _entry ) override;

public:

    SvTreeListBox( vcl::Window* pParent, WinBits nWinStyle=0 );
    virtual ~SvTreeListBox() override;
    virtual void dispose() override;

    sal_uInt32 GetVisibleCount() const
    {
        return m_pModel->GetVisibleCount(const_cast<SvTreeListBox*>(this));
    }

    SvTreeListEntry* FirstVisible() const { return m_pModel->FirstVisible(); }

    SvTreeListEntry* NextVisible(SvTreeListEntry* pEntry) const
    {
        return m_pModel->NextVisible(this, pEntry);
    }

    SvTreeListEntry* PrevVisible(SvTreeListEntry* pEntry) const
    {
        return m_pModel->PrevVisible(this, pEntry);
    }

    SvTreeListEntry* LastVisible() const { return m_pModel->LastVisible(this); }

    SvTreeListEntry* NextVisible(SvTreeListEntry* pEntry, sal_uInt16& rDelta) const
    {
        return m_pModel->NextVisible(this, pEntry, rDelta);
    }

    SvTreeListEntry* PrevVisible(SvTreeListEntry* pEntry, sal_uInt16& rDelta) const
    {
        return m_pModel->PrevVisible(this, pEntry, rDelta);
    }

    sal_uInt32 GetSelectionCount() const;

    SvTreeListEntry* FirstSelected() const { return m_pModel->FirstSelected(this); }

    SvTreeListEntry* NextSelected(SvTreeListEntry* pEntry) const
    {
        return m_pModel->NextSelected(this, pEntry);
    }

    SvTreeListEntry* GetEntryAtAbsPos(sal_uInt32 nAbsPos) const
    {
        return m_pModel->GetEntryAtAbsPos(nAbsPos);
    }

    SvTreeListEntry* GetEntryAtVisPos(sal_uInt32 nVisPos) const
    {
        return m_pModel->GetEntryAtVisPos(this, nVisPos);
    }

    sal_uInt32 GetAbsPos(SvTreeListEntry const* pEntry) const
    {
        return m_pModel->GetAbsPos(pEntry);
    }

    sal_uInt32 GetVisiblePos(SvTreeListEntry const* pEntry) const
    {
        return m_pModel->GetVisiblePos(this, pEntry);
    }

    sal_uInt32 GetVisibleChildCount(SvTreeListEntry* pParent) const
    {
        return m_pModel->GetVisibleChildCount(this, pParent);
    }

    bool IsEntryVisible(SvTreeListEntry* pEntry) const
    {
        return m_pModel->IsEntryVisible(this, pEntry);
    }

    bool IsExpanded(SvTreeListEntry* pEntry) const;
    bool IsAllExpanded(SvTreeListEntry* pEntry) const;
    bool IsSelected(const SvTreeListEntry* pEntry) const;
    void SetEntryFocus(SvTreeListEntry* pEntry, bool bFocus);
    const SvViewDataEntry* GetViewData(const SvTreeListEntry* pEntry) const;
    SvViewDataEntry* GetViewData(SvTreeListEntry* pEntry);
    bool HasViewData() const;

    SvTreeList* GetModel() const { return m_pModel.get(); }

    sal_uInt32 GetEntryCount() const { return m_pModel ? m_pModel->GetEntryCount() : 0; }
    SvTreeListEntry* First() const { return m_pModel ? m_pModel->First() : nullptr; }
    SvTreeListEntry* Next(SvTreeListEntry* pEntry) const { return m_pModel->Next(pEntry); }
    SvTreeListEntry* Last() const { return m_pModel ? m_pModel->Last() : nullptr; }

    SvTreeListEntry* FirstChild(const SvTreeListEntry* pParent) const;

    sal_uInt32 GetEntryPos(const SvTreeListEntry* pEntry) const;

    bool            CopySelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget );
    bool            MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvTreeListEntry* pTarget, bool bAllowCopyFallback );
    void            RemoveSelection();
    /**
     * Removes the entry along with all of its descendants
     */
    void            RemoveEntry(SvTreeListEntry const * pEntry);

    DragDropMode GetDragDropMode() const { return m_nDragDropMode; }
    SelectionMode GetSelectionMode() const { return m_eSelMode; }

    // pParent == 0 -> Root level
    SvTreeListEntry* GetEntry( SvTreeListEntry* pParent, sal_uInt32 nPos ) const;
    SvTreeListEntry* GetEntry( sal_uInt32 nRootPos ) const;

    SvTreeListEntry*    GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const;
    void            FillEntryPath( SvTreeListEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const;

    using Window::GetParent;
    SvTreeListEntry* GetParent( SvTreeListEntry* pEntry ) const;

    using Window::GetChildCount;
    sal_uInt32          GetChildCount( SvTreeListEntry const * pParent ) const;
    sal_uInt32          GetLevelChildCount( const SvTreeListEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvTreeListEntry const * pEntry ) const;
    SvViewDataItem& GetViewDataItem(SvTreeListEntry const*, const SvLBoxItem&);
    const SvViewDataItem& GetViewDataItem(const SvTreeListEntry*, const SvLBoxItem&) const;

    OUString GetEntryTooltip(SvTreeListEntry& rEntry) const;

    VclPtr<Edit> GetEditWidget() const; // for UITest
    bool IsInplaceEditingEnabled() const
    {
        return bool(m_nImpFlags & SvTreeListBoxFlags::EDT_ENABLED);
    }
    bool IsEditingActive() const { return bool(m_nImpFlags & SvTreeListBoxFlags::IN_EDT); }
    void EndEditing( bool bCancel = false );

    void            Clear();

    bool            TextCenterAndClipEnabled() const { return mbCenterAndClipText; }

    void SetSelectHdl(const Link<SvTreeListBox*, void>& rNewHdl) { m_aSelectHdl = rNewHdl; }
    void SetDeselectHdl(const Link<SvTreeListBox*, void>& rNewHdl) { m_aDeselectHdl = rNewHdl; }
    void SetDoubleClickHdl(const Link<SvTreeListBox*, bool>& rNewHdl)
    {
        m_aDoubleClickHdl = rNewHdl;
    }
    void SetExpandingHdl(const Link<SvTreeListBox*, bool>& rNewHdl) { m_aExpandingHdl = rNewHdl; }
    void SetExpandedHdl(const Link<SvTreeListBox*, void>& rNewHdl) { m_aExpandedHdl = rNewHdl; }
    void SetTooltipHdl(const Link<SvTreeListEntry&, OUString>& rLink) { m_aTooltipHdl = rLink; }
    void SetCustomRenderHdl(const Link<svtree_render_args, void>& rLink)
    {
        m_aCustomRenderHdl = rLink;
    }
    void SetCustomMeasureHdl(const Link<svtree_measure_args, Size>& rLink)
    {
        m_aCustomMeasureHdl = rLink;
    }

    void            ExpandedHdl();
    bool            ExpandingHdl();
    void            SelectHdl();
    void            DeselectHdl();
    bool            DoubleClickHdl();
    SvTreeListEntry* GetHdlEntry() const { return m_pHdlEntry; }

    // Is called for an Entry that gets expanded with the Flag
    // ENTRYFLAG_CHILDREN_ON_DEMAND set.
    virtual void             RequestingChildren( SvTreeListEntry* pParent );

    // Drag & Drop
    // New D'n'D API
    virtual sal_Int8         AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8         ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void             StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual DragDropMode     NotifyStartDrag();
    virtual void             DragFinished( sal_Int8 nDropAction );

    SvTreeListEntry* CloneEntry(const SvTreeListEntry& rSource);

    // Return value: TRISTATE_TRUE == Ok, TRISTATE_FALSE == Cancel, TRISTATE_INDET == Ok and Make visible moved entry
    TriState NotifyMoving(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in GetModel()
        const SvTreeListEntry*  pEntry,        // Entry to be moved from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uInt32&        rNewChildPos); // The TargetParent's position in Childlist

    // Return value: TRISTATE_TRUE == Ok, TRISTATE_FALSE == Cancel, TRISTATE_INDET == Ok and Make visible moved entry
    TriState    NotifyCopying(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in GetModel()
        const SvTreeListEntry*  pEntry,        // Entry to be copied from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uInt32&        rNewChildPos); // The TargetParent's position in Childlist

    /** Creates and returns the accessible object of the Box. */
    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() override;

    /** Calculate and return the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    tools::Rectangle    GetBoundingRect(const SvTreeListEntry* pEntry);

    SvTreeFlags GetTreeFlags() const { return m_nTreeFlags; }

    static OUString     SearchEntryTextWithHeadTitle(SvTreeListEntry* pEntry);

    void set_min_width_in_chars(sal_Int32 nChars);

    virtual bool set_property(const OUString &rKey, const OUString &rValue) override;

    SAL_DLLPRIVATE void SetCollapsedNodeBmp( const Image& );
    SAL_DLLPRIVATE void SetExpandedNodeBmp( const Image& );
    SAL_DLLPRIVATE Image const & GetExpandedNodeBmp( ) const;

protected:

    virtual void                CalcEntryHeight(SvTreeListEntry const* pEntry);
                   void         AdjustEntryHeight( const Image& rBmp );
    SAL_DLLPRIVATE void         AdjustEntryHeight();

    SAL_DLLPRIVATE void         ImpEntryInserted( SvTreeListEntry* pEntry );
    SAL_DLLPRIVATE void         PaintEntry1( SvTreeListEntry&, tools::Long nLine, vcl::RenderContext& rRenderContext );

    SAL_DLLPRIVATE void         ImplInitStyle();

    void            SetupDragOrigin();
    void EditItemText(SvTreeListEntry& rEntry, SvLBoxString& rItem, const Selection&);
    void            EditedText(const OUString&);

    // Recalculate all tabs depending on TreeListStyle and Bitmap sizes
    // Is called automatically when inserting/changing Bitmaps, changing the Model etc.
    virtual void    SetTabs();
    void            AddTab( tools::Long nPos, SvLBoxTabFlags nFlags );
    sal_uInt16 TabCount() const { return m_aTabs.size(); }
    SvLBoxTab*      GetFirstDynamicTab() const;
    SvLBoxTab*      GetFirstDynamicTab( sal_uInt16& rTabPos ) const;
    SvLBoxTab*      GetFirstTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    void            GetLastTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab* GetTab(const SvTreeListEntry&, const SvLBoxItem&) const;

    virtual void InitEntry(SvTreeListEntry& rEntry, const OUString&, const Image&, const Image&);

    void            NotifyScrolled();
    void SetScrolledHdl(const Link<SvTreeListBox*, void>& rLink) { m_aScrolledHdl = rLink; }
    tools::Long            GetXOffset() const { return GetMapMode().GetOrigin().X(); }

    virtual void    Command( const CommandEvent& rCEvt ) override;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            InitSettings();

    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

    void            ImplEditEntry( SvTreeListEntry* pEntry );

    void            AdjustEntryHeightAndRecalc();

    // true if rPos is over the SvTreeListBox body, i.e. not over a
    // scrollbar
    SAL_DLLPRIVATE bool PosOverBody(const Point& rPos) const;
public:

    void            SetNoAutoCurEntry( bool b );

    void            EnableCheckButton(SvLBoxButtonData&);

    /** Returns the default image which clients should use for expanded nodes, to have a consistent user
        interface experience in the whole product.
    */
    static const Image& GetDefaultExpandedNodeImage( );

    /** Returns the default image which clients should use for expanded nodes, to have a consistent user
        interface experience in the whole product.
    */
    static const Image& GetDefaultCollapsedNodeImage( );

    /** Sets default bitmaps for collapsed and expanded nodes.
    */
    void    SetNodeDefaultImages();

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uInt32 nPos=TREELIST_APPEND, OUString* pUserData = nullptr);

    const Image&    GetDefaultExpandedEntryBmp( ) const;
    const Image&    GetDefaultCollapsedEntryBmp( ) const;

    void            SetDefaultExpandedEntryBmp( const Image& rBmp );
    void            SetDefaultCollapsedEntryBmp( const Image& rBmp );

    void            SetCheckButtonState( SvTreeListEntry*, SvButtonState );
    SvButtonState   GetCheckButtonState( SvTreeListEntry* ) const;
    bool GetCheckButtonEnabled(SvTreeListEntry* pEntry) const;

    void            SetEntryText(SvTreeListEntry*, const OUString& );
    void            SetExpandedEntryBmp( SvTreeListEntry* _pEntry, const Image& _rImage );
    void            SetCollapsedEntryBmp( SvTreeListEntry* _pEntry, const Image& _rImage );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const;
    static const Image&    GetExpandedEntryBmp(const SvTreeListEntry* _pEntry );
    static const Image&    GetCollapsedEntryBmp(const SvTreeListEntry* _pEntry );

    void            CheckButtonHdl();

    void            SetSublistOpenWithLeftRight();   // open/close sublist with cursor left/right

    void            EnableInplaceEditing( bool bEnable );
    // Edits the Entry's first StringItem, 0 == Cursor
    void            EditEntry( SvTreeListEntry* pEntry );
    virtual bool    EditingEntry( SvTreeListEntry* pEntry );
    virtual bool EditedEntry(SvTreeListEntry& rEntry, const SvLBoxItem& rItem,
                             const OUString& rNewText);

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    void            SetUpdateMode( bool );

    virtual void ModelHasCleared();
    virtual void ModelHasInserted(SvTreeListEntry* pEntry);
    virtual void ModelHasInsertedTree(SvTreeListEntry* pEntry);
    virtual void ModelIsMoving(SvTreeListEntry* pSource);
    virtual void ModelHasMoved(SvTreeListEntry* pSource);
    virtual void ModelIsRemoving(SvTreeListEntry* pEntry);
    virtual void ModelHasRemoved(SvTreeListEntry* pEntry);
    void ModelHasEntryInvalidated(SvTreeListEntry* pEntry);

    void            ScrollOutputArea( short nDeltaEntries );

    short GetEntryHeight() const { return m_nEntryHeight; }
    void            SetEntryHeight( short nHeight );
    short GetEntryWidth() const { return m_nEntryWidth; }
    void            SetEntryWidth( short nWidth );
    Size            GetOutputSizePixel() const;
    short GetIndent() const { return m_nIndent; }
    void            SetSpaceBetweenEntries( short nSpace );
    Point           GetEntryPosition(const SvTreeListEntry*) const;
    void            MakeVisible( SvTreeListEntry* pEntry );
    void            MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop );

    void            SetFont( const vcl::Font& rFont );

    SvTreeListEntry*    GetEntry( const Point& rPos, bool bHit = false ) const;

    virtual tools::Rectangle GetFocusRect(const SvTreeListEntry*, tools::Long nLine );
    // Respects indentation
    tools::Long     GetTabPos(const SvTreeListEntry*, const SvLBoxTab*) const;
    void            InvalidateEntry( SvTreeListEntry* );
    SvLBoxItem*     GetItem( SvTreeListEntry*, tools::Long nX, SvLBoxTab** ppTab);
    SvLBoxItem*     GetItem( SvTreeListEntry*, tools::Long nX );
    std::pair<tools::Long, tools::Long> GetItemPos(SvTreeListEntry* pEntry, sal_uInt16 nTabIdx);

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    bool            Expand( SvTreeListEntry* pParent );
    bool            Collapse( SvTreeListEntry* pParent );
    bool            Select( SvTreeListEntry* pEntry, bool bSelect=true );
    sal_uInt32      SelectChildren( const SvTreeListEntry* pParent, bool bSelect );
    void            SelectAll( bool bSelect );

    void SetCurEntry( SvTreeListEntry* _pEntry );
    SvTreeListEntry* GetCurEntry() const;

    virtual void    ImplInvalidate( const vcl::Region* rRegion, InvalidateFlags nFlags ) override;

    void            SetHighlightRange(sal_uInt16 nFirstTab=0, sal_uInt16 nLastTab=0xffff);

    sal_Int32       DefaultCompare(const SvLBoxString* pLeftText, const SvLBoxString* pRightText);

    DECL_DLLPRIVATE_LINK( DefaultCompare, const SvSortData&, sal_Int32 );
    void ModelNotification(SvListAction nActionId, SvTreeListEntry* pEntry);

    SvTreeListEntry*    GetFirstEntryInView() const;
    SvTreeListEntry*    GetNextEntryInView(SvTreeListEntry*) const;
    void            ScrollToAbsPos( tools::Long nPos );

    tools::Long            getPreferredDimensions(std::vector<tools::Long> &rWidths) const;

    virtual Size    GetOptimalSize() const override;

    // Enables type-ahead search in the check list box.
    void            SetQuickSearch(bool bEnable) { mbQuickSearch = bEnable; }

    // Make single click "activate" a row like a double-click normally does
    void            SetActivateOnSingleClick(bool bEnable) { mbActivateOnSingleClick = bEnable; }
    bool            GetActivateOnSingleClick() const { return mbActivateOnSingleClick; }

    void            SetCustomEntryRenderer(bool bEnable) { mbCustomEntryRenderer = bEnable; }
    bool            GetCustomEntryRenderer() const { return mbCustomEntryRenderer; }

    // Make mouse over a row "select" a row like a single-click normally does
    void            SetHoverSelection(bool bEnable) { mbHoverSelection = bEnable; }
    bool            GetHoverSelection() const { return mbHoverSelection; }

    // to get enable or disable the text-column
    void           SetTextColumnEnabled(bool bEnable) { mbIsTextColumEnabled = bEnable; }
    bool           IsTextColumnEnabled() const { return mbIsTextColumEnabled; }

    // only true during Select if the Select is due to a Hover
    bool            IsSelectDueToHover() const { return mbSelectingByHover; }

    // Set when clicks toggle the checkbox of the row.
    void            SetClicksToToggle(sal_Int8 nCount) { mnClicksToToggle = nCount; }

    void            SetForceMakeVisible(bool bEnable);

    virtual FactoryFunction GetUITestFactory() const override;

    void            SetDragHelper(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants);

    virtual void    EnableRTL(bool bEnable = true) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
