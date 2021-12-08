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
#include <vcl/transfer.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvTreeListBox;
class SvTreeListEntry;
struct SvViewDataItem;
class SvViewDataEntry;
class SvInplaceEdit2;
class SvLBoxString;
class SvImpLBox;
class SvLBoxButtonData;
class Timer;
class Edit;

namespace utl {
    class AccessibleStateSetHelper;
}

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
};
namespace o3tl
{
    template<> struct typed_flags<SvLBoxTabFlags> : is_typed_flags<SvLBoxTabFlags, 0x007f> {};
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
            ~SvLBoxTab();

    SvLBoxTabFlags nFlags;

    bool    IsDynamic() const { return bool(nFlags & SvLBoxTabFlags::DYNAMIC); }
    void    SetPos( tools::Long nNewPos) { nPos = nNewPos; }
    tools::Long    GetPos() const { return nPos; }
    tools::Long    CalcOffset( tools::Long nItemLength, tools::Long nTabWidth );
    bool    IsEditable() const { return bool(nFlags & SvLBoxTabFlags::EDITABLE); }
};

class VCL_DLLPUBLIC SvLBoxItem
{
protected:
    bool mbDisabled;

public:
                        SvLBoxItem();
    virtual             ~SvLBoxItem();
    virtual SvLBoxItemType GetType() const = 0;
    virtual int CalcWidth(const SvTreeListBox* pView) const;
    int GetWidth(const SvTreeListBox* pView, const SvTreeListEntry* pEntry) const;
    int GetWidth(const SvTreeListBox* pView, const SvViewDataEntry* pData, sal_uInt16 nItemPos) const;
    int GetHeight(const SvTreeListBox* pView, const SvTreeListEntry* pEntry) const;
    static int GetHeight(const SvViewDataEntry* pData, sal_uInt16 nItemPos);
    void Enable(bool bEnabled) { mbDisabled = !bEnabled; }

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext, const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) = 0;

    virtual void InitViewData(SvTreeListBox* pView, SvTreeListEntry* pEntry,
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
    APP_COPY        = 0x0004,
    // Entries may be dropped via the uppermost Entry
    // The DropTarget is 0 in that case
    ENABLE_TOP      = 0x0010,
    ALL             = 0x0015,
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

class VCL_DLLPUBLIC SvTreeListBox
                :public Control
                ,public SvListView
                ,public DropTargetHelper
                ,public DragSourceHelper
                ,public vcl::ISearchableStringList
{
    friend class SvImpLBox;
    friend class SvLBoxString;
    friend class IconViewImpl;
    friend class TreeControlPeer;
    friend class SalInstanceIconView;
    friend class SalInstanceTreeView;
    friend class SalInstanceEntryTreeView;

    std::unique_ptr<SvTreeListBoxImpl> mpImpl;
    Link<SvTreeListBox*,void>  aScrolledHdl;
    Link<SvTreeListBox*,void>  aExpandedHdl;
    Link<SvTreeListBox*,bool>  aExpandingHdl;
    Link<SvTreeListBox*,void>  aSelectHdl;
    Link<SvTreeListBox*,void>  aDeselectHdl;
    Link<const CommandEvent&, bool> aPopupMenuHdl;
    Link<const HelpEvent&, bool> aTooltipHdl;
    Link<svtree_render_args, void> aCustomRenderHdl;
    Link<svtree_measure_args, Size> aCustomMeasureHdl;

    Image           aPrevInsertedExpBmp;
    Image           aPrevInsertedColBmp;
    Image           aCurInsertedExpBmp;
    Image           aCurInsertedColBmp;

    short           nContextBmpWidthMax;
    short           nEntryHeightOffs;
    short           nIndent;
    short           nFocusWidth;
    sal_uInt16      nFirstSelTab;
    sal_uInt16      nLastSelTab;
    tools::Long mnCheckboxItemWidth;
    bool mbContextBmpExpanded;
    bool mbAlternatingRowColors;
    bool mbUpdateAlternatingRows;
    bool mbQuickSearch; // Enables type-ahead search in the check list box.
    bool mbActivateOnSingleClick; // Make single click "activate" a row like a double-click normally does
    bool mbHoverSelection; // Make mouse over a row "select" a row like a single-click normally does
    bool mbSelectingByHover; // true during "Select" if it was due to hover
    sal_Int8        mnClicksToToggle; // 0 == Click on a row not toggle its checkbox.
                                      // 1 == Every click on row toggle its checkbox.
                                      // 2 == First click select, second click toggle.

    SvTreeListEntry*    pHdlEntry;

    DragDropMode    nDragDropMode;
    DragDropMode    nOldDragMode;
    SelectionMode   eSelMode;
    sal_Int32       nMinWidthInChars;

    sal_Int8        mnDragAction;

    SvTreeListEntry*        pEdEntry;
    SvLBoxItem*             pEdItem;

    rtl::Reference<TransferDataContainer> m_xTransferHelper;

protected:
    std::unique_ptr<SvImpLBox>              pImpl;
    short                   nColumns;
    short                   nEntryHeight;
    short                   nEntryWidth;
    bool                    mbCenterAndClipText;

    Link<SvTreeListBox*,bool> aDoubleClickHdl;
    SvTreeListEntry*        pTargetEntry;
    SvLBoxButtonData*       pCheckButtonData;
    std::vector<std::unique_ptr<SvLBoxTab>> aTabs;
    SvTreeFlags             nTreeFlags;
    SvTreeListBoxFlags      nImpFlags;
    // Move/CopySelection: Position of the current Entry in SelectionList
    sal_uInt16              nCurEntrySelPos;

private:
    DECL_DLLPRIVATE_LINK( CheckButtonClick, SvLBoxButtonData *, void );
    DECL_DLLPRIVATE_LINK( TextEditEndedHdl_Impl, SvInplaceEdit2&, void );
    // Handler that is called by TreeList to clone an Entry
    DECL_DLLPRIVATE_LINK( CloneHdl_Impl, SvTreeListEntry*, SvTreeListEntry* );

    // Handler and methods for Drag - finished handler.
    // The Handle retrieved by GetDragFinishedHdl can be set on the
    // TransferDataContainer. This link is a callback for the DragFinished
    // call. The AddBox method is called from the GetDragFinishedHdl() and the
    // remove is called in the link callback and in the dtor. So it can't be
    // called for a deleted object.
    VCL_DLLPRIVATE static void AddBoxToDDList_Impl( const SvTreeListBox& rB );
    VCL_DLLPRIVATE static void RemoveBoxFromDDList_Impl( const SvTreeListBox& rB );
    DECL_DLLPRIVATE_LINK( DragFinishHdl_Impl, sal_Int8, void );

    // after a checkbox entry is inserted, use this to get its width to support
    // autowidth for the 1st checkbox column
    VCL_DLLPRIVATE void CheckBoxInserted(SvTreeListEntry* pEntry);

    VCL_DLLPRIVATE void DrawCustomEntry(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const SvTreeListEntry& rEntry);
    VCL_DLLPRIVATE Size MeasureCustomEntry(vcl::RenderContext& rRenderContext, const SvTreeListEntry& rEntry) const;

    /** Handles the given key event.

        At the moment this merely does typeahead if typeahead is enabled.

        @return
            <TRUE/> if the event has been consumed, <FALSE/> otherwise.
    */
    VCL_DLLPRIVATE bool HandleKeyInput(const KeyEvent& rKEvt);

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

    virtual sal_uInt32 Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uInt32 nPos=TREELIST_APPEND);
    virtual sal_uInt32 Insert( SvTreeListEntry* pEntry,sal_uInt32 nRootPos = TREELIST_APPEND );

    // In-place editing
    std::unique_ptr<SvInplaceEdit2>  pEdCtrl;
    void            EditText( const OUString&, const tools::Rectangle&,const Selection&);
    void            CancelTextEditing();

    // InitViewData is called right after CreateViewData
    // The Entry is has not yet been added to the View in InitViewData!
    virtual void InitViewData( SvViewDataEntry*, SvTreeListEntry* pEntry ) override;
    // Calls InitViewData for all Items
    void            RecalcViewData();

    // Handler and methods for Drag - finished handler. This link can be set
    // to the TransferDataContainer. The AddBox/RemoveBox methods must be
    // called before the StartDrag call.
    // The Remove will be called from the handler, which then calls DragFinish.
    // The Remove is also called in the DTOR of the SvTreeListBox -
    // so it can't be called for a deleted object.
    Link<sal_Int8,void> GetDragFinishedHdl() const;

    // For asynchronous D'n'D
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt, SvTreeListBox* pSourceView );

    void            OnCurrentEntryChanged();

    // ISearchableStringList
    virtual vcl::StringEntryIdentifier    CurrentEntry( OUString& _out_entryText ) const override;
    virtual vcl::StringEntryIdentifier    NextEntry( vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const override;
    virtual void                            SelectEntry( vcl::StringEntryIdentifier _entry ) override;

public:

    SvTreeListBox( vcl::Window* pParent, WinBits nWinStyle=0 );
    virtual ~SvTreeListBox() override;
    virtual void dispose() override;

    SvTreeList* GetModel() const
    {
        return pModel.get();
    }

    sal_uInt32 GetEntryCount() const
    {
        return pModel ? pModel->GetEntryCount() : 0;
    }
    SvTreeListEntry* First() const
    {
        return pModel ? pModel->First() : nullptr;
    }
    SvTreeListEntry* Next( SvTreeListEntry* pEntry ) const
    {
         return pModel->Next(pEntry);
    }
    SvTreeListEntry* Prev( SvTreeListEntry* pEntry ) const
    {
        return pModel->Prev(pEntry);
    }
    SvTreeListEntry* Last() const
    {
        return pModel ? pModel->Last() : nullptr;
    }

    SvTreeListEntry* FirstChild( SvTreeListEntry* pParent ) const;

    bool            CopySelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget );
    bool            MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvTreeListEntry* pTarget, bool bAllowCopyFallback );
    void            RemoveSelection();
    /**
     * Removes the entry along with all of its descendants
     */
    void            RemoveEntry(SvTreeListEntry const * pEntry);

    DragDropMode    GetDragDropMode() const { return nDragDropMode; }
    SelectionMode   GetSelectionMode() const { return eSelMode; }

    // pParent == 0 -> Root level
    SvTreeListEntry* GetEntry( SvTreeListEntry* pParent, sal_uInt32 nPos ) const;
    SvTreeListEntry* GetEntry( sal_uInt32 nRootPos ) const;

    SvTreeListEntry*    GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const;
    void            FillEntryPath( SvTreeListEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const;

    using Window::GetParent;
    SvTreeListEntry* GetParent( SvTreeListEntry* pEntry ) const;

    using Window::GetChildCount;
    sal_uInt32          GetChildCount( SvTreeListEntry const * pParent ) const;
    sal_uInt32          GetLevelChildCount( SvTreeListEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvTreeListEntry const * pEntry ) const;
    SvViewDataItem*  GetViewDataItem(SvTreeListEntry const *, SvLBoxItem const *);
    const SvViewDataItem*  GetViewDataItem(const SvTreeListEntry*, const SvLBoxItem*) const;

    bool IsInplaceEditingEnabled() const { return bool(nImpFlags & SvTreeListBoxFlags::EDT_ENABLED); }
    bool IsEditingActive() const { return bool(nImpFlags & SvTreeListBoxFlags::IN_EDT); }
    void EndEditing( bool bCancel = false );

    void            Clear();

    bool            TextCenterAndClipEnabled() const { return mbCenterAndClipText; }

    void            SetSelectHdl( const Link<SvTreeListBox*,void>& rNewHdl ) {aSelectHdl=rNewHdl; }
    void            SetDeselectHdl( const Link<SvTreeListBox*,void>& rNewHdl ) {aDeselectHdl=rNewHdl; }
    void            SetDoubleClickHdl(const Link<SvTreeListBox*,bool>& rNewHdl) {aDoubleClickHdl=rNewHdl;}
    void            SetExpandingHdl(const Link<SvTreeListBox*,bool>& rNewHdl){aExpandingHdl=rNewHdl;}
    void            SetExpandedHdl(const Link<SvTreeListBox*,void>& rNewHdl){aExpandedHdl=rNewHdl;}
    void SetPopupMenuHdl(const Link<const CommandEvent&, bool>& rLink) { aPopupMenuHdl = rLink; }
    void SetTooltipHdl(const Link<const HelpEvent&, bool>& rLink) { aTooltipHdl = rLink; }
    void SetCustomRenderHdl(const Link<svtree_render_args, void>& rLink) { aCustomRenderHdl = rLink; }
    void SetCustomMeasureHdl(const Link<svtree_measure_args, Size>& rLink) { aCustomMeasureHdl = rLink; }

    void            ExpandedHdl();
    bool            ExpandingHdl();
    void            SelectHdl();
    void            DeselectHdl();
    bool            DoubleClickHdl();
    SvTreeListEntry*    GetHdlEntry() const { return pHdlEntry; }

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

    SvTreeListEntry*         CloneEntry( SvTreeListEntry* pSource );

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

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    /** Fills the StateSet of one entry. */
    void FillAccessibleEntryStateSet( SvTreeListEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Calculate and return the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    tools::Rectangle    GetBoundingRect(const SvTreeListEntry* pEntry);

    SvTreeFlags         GetTreeFlags() const {return nTreeFlags;}

    static OUString     SearchEntryTextWithHeadTitle(SvTreeListEntry* pEntry);

    void set_min_width_in_chars(sal_Int32 nChars);

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;

protected:

    VCL_DLLPRIVATE void         SetEntryHeight( SvTreeListEntry const * pEntry );
                   void         AdjustEntryHeight( const Image& rBmp );
    VCL_DLLPRIVATE void         AdjustEntryHeight();

    VCL_DLLPRIVATE void         ImpEntryInserted( SvTreeListEntry* pEntry );
    VCL_DLLPRIVATE void         PaintEntry1( SvTreeListEntry&, tools::Long nLine, vcl::RenderContext& rRenderContext );

    VCL_DLLPRIVATE void         InitTreeView();
    VCL_DLLPRIVATE SvLBoxItem*  GetItem_Impl( SvTreeListEntry*, tools::Long nX, SvLBoxTab** ppTab );
    VCL_DLLPRIVATE void         ImplInitStyle();

    void            SetupDragOrigin();
    void            EditItemText( SvTreeListEntry* pEntry, SvLBoxString* pItem,
                        const Selection& );
    void            EditedText(const OUString&);

    // Recalculate all tabs depending on TreeListStyle and Bitmap sizes
    // Is called automatically when inserting/changing Bitmaps, changing the Model etc.
    virtual void    SetTabs();
    void            AddTab( tools::Long nPos, SvLBoxTabFlags nFlags );
    sal_uInt16      TabCount() const { return aTabs.size(); }
    SvLBoxTab*      GetFirstDynamicTab() const;
    SvLBoxTab*      GetFirstDynamicTab( sal_uInt16& rTabPos ) const;
    SvLBoxTab*      GetFirstTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    void            GetLastTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetTab( SvTreeListEntry const *, SvLBoxItem const * ) const;
    void            ClearTabList();

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&);

    void            NotifyScrolled();
    void            SetScrolledHdl( const Link<SvTreeListBox*,void>& rLink ) { aScrolledHdl = rLink; }
    tools::Long            GetXOffset() const { return GetMapMode().GetOrigin().X(); }

    virtual void    Command( const CommandEvent& rCEvt ) override;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            InitSettings();

    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

    void            CallImplEventListeners(VclEventId nEvent, void* pData);

    void            ImplEditEntry( SvTreeListEntry* pEntry );

    void            AdjustEntryHeightAndRecalc();

    // true if rPos is over the SvTreeListBox body, i.e. not over a
    // scrollbar
    VCL_DLLPRIVATE bool PosOverBody(const Point& rPos) const;
public:

    void            SetNoAutoCurEntry( bool b );

    void            EnableCheckButton( SvLBoxButtonData* );
    void            SetCheckButtonData( SvLBoxButtonData* );
    void            SetNodeBitmaps( const Image& rCollapsedNodeBmp, const Image& rExpandedNodeBmp );

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
    void    SetNodeDefaultImages( )
    {
        SetNodeBitmaps(
            GetDefaultCollapsedNodeImage( ),
            GetDefaultExpandedNodeImage( )
        );
    }

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uInt32 nPos=TREELIST_APPEND, void* pUserData = nullptr);

    const Image&    GetDefaultExpandedEntryBmp( ) const;
    const Image&    GetDefaultCollapsedEntryBmp( ) const;

    void            SetDefaultExpandedEntryBmp( const Image& rBmp );
    void            SetDefaultCollapsedEntryBmp( const Image& rBmp );

    void            SetCheckButtonState( SvTreeListEntry*, SvButtonState );
    SvButtonState   GetCheckButtonState( SvTreeListEntry* ) const;

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
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    void            SetUpdateMode( bool );

    virtual void    ModelHasCleared() override;
    virtual void    ModelHasInserted( SvTreeListEntry* pEntry ) override;
    virtual void    ModelHasInsertedTree( SvTreeListEntry* pEntry ) override;
    virtual void    ModelIsMoving(SvTreeListEntry* pSource ) override;
    virtual void    ModelHasMoved(SvTreeListEntry* pSource ) override;
    virtual void    ModelIsRemoving( SvTreeListEntry* pEntry ) override;
    virtual void    ModelHasRemoved( SvTreeListEntry* pEntry ) override;
    void            ModelHasEntryInvalidated( SvTreeListEntry* pEntry ) override;

    void            ScrollOutputArea( short nDeltaEntries );

    short           GetColumnsCount() const { return nColumns; }
    short           GetEntryHeight() const  { return nEntryHeight; }
    void            SetEntryHeight( short nHeight );
    short           GetEntryWidth() const { return nEntryWidth; }
    void            SetEntryWidth( short nWidth );
    Size            GetOutputSizePixel() const;
    short           GetIndent() const { return nIndent; }
    void            SetSpaceBetweenEntries( short nSpace );
    Point           GetEntryPosition(const SvTreeListEntry*) const;
    void            MakeVisible( SvTreeListEntry* pEntry );
    void            MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop );

    void            SetCollapsedNodeBmp( const Image& );
    void            SetExpandedNodeBmp( const Image& );
    Image const &   GetExpandedNodeBmp( ) const;

    void            SetFont( const vcl::Font& rFont );

    SvTreeListEntry*    GetEntry( const Point& rPos, bool bHit = false ) const;

    virtual tools::Rectangle GetFocusRect(const SvTreeListEntry*, tools::Long nLine );
    // Respects indentation
    sal_IntPtr      GetTabPos(const SvTreeListEntry*, const SvLBoxTab*) const;
    void            InvalidateEntry( SvTreeListEntry* );
    SvLBoxItem*     GetItem( SvTreeListEntry*, tools::Long nX, SvLBoxTab** ppTab);
    SvLBoxItem*     GetItem( SvTreeListEntry*, tools::Long nX );
    std::pair<tools::Long, tools::Long> GetItemPos(SvTreeListEntry* pEntry, sal_uInt16 nTabIdx);

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    bool            Expand( SvTreeListEntry* pParent );
    bool            Collapse( SvTreeListEntry* pParent );
    bool            Select( SvTreeListEntry* pEntry, bool bSelect=true );
    sal_uInt32      SelectChildren( SvTreeListEntry* pParent, bool bSelect );
    void            SelectAll( bool bSelect );

    void SetCurEntry( SvTreeListEntry* _pEntry );
    SvTreeListEntry* GetCurEntry() const;

    using Window::Invalidate;
    virtual void    Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE) override;
    virtual void    Invalidate( const tools::Rectangle&, InvalidateFlags nFlags = InvalidateFlags::NONE ) override;

    void            SetHighlightRange(sal_uInt16 nFirstTab=0, sal_uInt16 nLastTab=0xffff);

    sal_Int32       DefaultCompare(const SvLBoxString* pLeftText, const SvLBoxString* pRightText);

    DECL_DLLPRIVATE_LINK( DefaultCompare, const SvSortData&, sal_Int32 );
    virtual void    ModelNotification( SvListAction nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uInt32 nPos ) override;

    SvTreeListEntry*    GetFirstEntryInView() const;
    SvTreeListEntry*    GetNextEntryInView(SvTreeListEntry*) const;
    void            ScrollToAbsPos( tools::Long nPos );

    tools::Long            getPreferredDimensions(std::vector<tools::Long> &rWidths) const;

    virtual Size    GetOptimalSize() const override;

    void            SetAlternatingRowColors( const bool bEnable );

    // Enables type-ahead search in the check list box.
    void            SetQuickSearch(bool bEnable) { mbQuickSearch = bEnable; }

    // Make single click "activate" a row like a double-click normally does
    void            SetActivateOnSingleClick(bool bEnable) { mbActivateOnSingleClick = bEnable; }
    bool            GetActivateOnSingleClick() const { return mbActivateOnSingleClick; }

    // Make mouse over a row "select" a row like a single-click normally does
    void            SetHoverSelection(bool bEnable) { mbHoverSelection = bEnable; }
    bool            GetHoverSelection() const { return mbHoverSelection; }

    // only true during Select if the Select is due to a Hover
    bool            IsSelectDueToHover() const { return mbSelectingByHover; }

    // Set when clicks toggle the checkbox of the row.
    void            SetClicksToToggle(sal_Int8 nCount) { mnClicksToToggle = nCount; }

    void            SetForceMakeVisible(bool bEnable);

    virtual FactoryFunction GetUITestFactory() const override;

    void            SetDragHelper(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
