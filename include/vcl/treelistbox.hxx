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

#ifndef INCLUDED_VCL_TREELISTBOX_HXX
#define INCLUDED_VCL_TREELISTBOX_HXX

#include <vcl/dllapi.h>

#include <deque>
#include <memory>
#include <vector>

#include <vcl/ctrl.hxx>
#include <vcl/accel.hxx>
#include <vcl/mnemonicengine.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/image.hxx>
#include <tools/gen.hxx>
#include <tools/contnr.hxx>
#include <vcl/treelist.hxx>
#include <vcl/transfer.hxx>
#include <vcl/idle.hxx>
#include <vcl/menu.hxx>
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

enum class SvLBoxButtonKind
{
    EnabledCheckbox,
    DisabledCheckbox,
    StaticImage
};

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

enum class SvTreeAccRoleType
{
    NONE   = 0x00,
    TREE   = 0x02
};

enum class SvLBoxItemType {String, Button, ContextBmp};

class VCL_DLLPUBLIC SvLBoxTab
{
    long    nPos;
public:
            SvLBoxTab();
            SvLBoxTab( long nPos, SvLBoxTabFlags nFlags );
            SvLBoxTab( const SvLBoxTab& );
            ~SvLBoxTab();

    SvLBoxTabFlags nFlags;

    bool    IsDynamic() const { return bool(nFlags & SvLBoxTabFlags::DYNAMIC); }
    void    SetPos( long nNewPos) { nPos = nNewPos; }
    long    GetPos() const { return nPos; }
    long    CalcOffset( long nItemLength, long nTabWidth );
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
    const Size&         GetSize(const SvTreeListBox* pView, const SvTreeListEntry* pEntry) const;
    static const Size&  GetSize(const SvViewDataEntry* pData, sal_uInt16 nItemPos);
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
    CTRL_COPY       = 0x0002,
    APP_MOVE        = 0x0004,
    APP_COPY        = 0x0008,
    APP_DROP        = 0x0010,
    // Entries may be dropped via the uppermost Entry
    // The DropTarget is 0 in that case
    ENABLE_TOP      = 0x0020,
    ALL             = 0x003f,
};
namespace o3tl
{
    template<> struct typed_flags<DragDropMode> : is_typed_flags<DragDropMode, 0x003f> {};
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

class VCL_DLLPUBLIC SvTreeListBox
                :public Control
                ,public SvListView
                ,public DropTargetHelper
                ,public DragSourceHelper
                ,public vcl::IMnemonicEntryList
                ,public vcl::ISearchableStringList
{
    friend class SvImpLBox;
    friend class IconViewImpl;
    friend class TreeControlPeer;
    friend class SalInstanceTreeView;
    friend class SalInstanceEntryTreeView;

    std::unique_ptr<SvTreeListBoxImpl> mpImpl;
    Link<SvTreeListBox*,void>  aCheckButtonHdl;
    Link<SvTreeListBox*,void>  aScrolledHdl;
    Link<SvTreeListBox*,void>  aExpandedHdl;
    Link<SvTreeListBox*,bool>  aExpandingHdl;
    Link<SvTreeListBox*,void>  aSelectHdl;
    Link<SvTreeListBox*,void>  aDeselectHdl;

    Image           aPrevInsertedExpBmp;
    Image           aPrevInsertedColBmp;
    Image           aCurInsertedExpBmp;
    Image           aCurInsertedColBmp;

    short           nContextBmpWidthMax;
    short           nEntryHeightOffs;
    short           nIndent;
    short           nFocusWidth;
    SvTreeAccRoleType nAllItemAccRoleType;
    sal_uInt16      nFirstSelTab;
    sal_uInt16      nLastSelTab;
    long mnCheckboxItemWidth;
    bool mbContextBmpExpanded;
    bool mbAlternatingRowColors;
    bool mbUpdateAlternatingRows;
    bool mbQuickSearch;     // Enables type-ahead search in the check list box.

    SvTreeListEntry*    pHdlEntry;

    DragDropMode    nDragDropMode;
    DragDropMode    nOldDragMode;
    SelectionMode   eSelMode;
    sal_Int8        nDragOptions;
    sal_Int32       nMinWidthInChars;

    SvTreeListEntry*        pEdEntry;
    SvLBoxItem*             pEdItem;

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

protected:

    bool            CheckDragAndDropMode( SvTreeListBox const * pSource, sal_Int8 );
    void            ImplShowTargetEmphasis( SvTreeListEntry* pEntry, bool bShow);
    void            EnableSelectionAsDropTarget( bool bEnable = true );
    // Standard impl returns 0; derived classes which support D'n'D must override
    using Window::GetDropTarget;
    virtual SvTreeListEntry* GetDropTarget( const Point& );

    // Invalidate children on enable/disable
    virtual void StateChanged( StateChangedType eType ) override;

    virtual sal_uLong Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uLong nPos=TREELIST_APPEND);
    virtual sal_uLong Insert( SvTreeListEntry* pEntry,sal_uLong nRootPos = TREELIST_APPEND );

    // In-place editing
    std::unique_ptr<SvInplaceEdit2>  pEdCtrl;
    void            EditText( const OUString&, const tools::Rectangle&,const Selection&);
    void            CancelTextEditing();
    bool            EditingCanceled() const;

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

    // IMnemonicEntryList
    virtual const void* FirstSearchEntry( OUString& _rEntryText ) const override;
    virtual const void* NextSearchEntry( const void* _pCurrentSearchEntry, OUString& _rEntryText ) const override;
    virtual void        SelectSearchEntry( const void* _pEntry ) override;
    virtual void        ExecuteSearchEntry( const void* _pEntry ) const override;

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

    sal_uLong GetEntryCount() const
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
    SvTreeListEntry* GetEntry( SvTreeListEntry* pParent, sal_uLong nPos ) const;
    SvTreeListEntry* GetEntry( sal_uLong nRootPos ) const;

    SvTreeListEntry*    GetEntryFromPath( const ::std::deque< sal_Int32 >& _rPath ) const;
    void            FillEntryPath( SvTreeListEntry* pEntry, ::std::deque< sal_Int32 >& _rPath ) const;

    using Window::GetParent;
    const SvTreeListEntry* GetParent( const SvTreeListEntry* pEntry ) const;
    SvTreeListEntry* GetParent( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*    GetRootLevelParent(SvTreeListEntry* pEntry ) const;

    using Window::GetChildCount;
    sal_uLong           GetChildCount( SvTreeListEntry const * pParent ) const;
    sal_uLong           GetLevelChildCount( SvTreeListEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvTreeListEntry const * pEntry ) const;
    SvViewDataItem*  GetViewDataItem(SvTreeListEntry const *, SvLBoxItem const *);
    const SvViewDataItem*  GetViewDataItem(const SvTreeListEntry*, const SvLBoxItem*) const;

    bool IsInplaceEditingEnabled() const { return bool(nImpFlags & SvTreeListBoxFlags::EDT_ENABLED); }
    bool IsEditingActive() const { return bool(nImpFlags & SvTreeListBoxFlags::IN_EDT); }
    void EndEditing( bool bCancel = false );
    void ForbidEmptyText();

    void            Clear();

    /** Enables or disables mnemonic characters in the entry texts.

        If mnemonics are enabled, then entries are selected and made current when
        there mnemonic character is pressed. If there are multiple entries with the
        same mnemonic, the selection cycles between them.

        Entries with an collapsed ancestor are not included in the calculation of
        mnemonics. That is, if you press the accelerator key of an invisible
        entry, then this entry is *not* selected.

        Be aware that enabling mnemonics gets more expensive as you add to the list.
    */
    void            EnableEntryMnemonics();
    bool            IsEntryMnemonicsEnabled() const;

    bool            TextCenterAndClipEnabled() const { return mbCenterAndClipText; }

    /** Handles the given key event.

        At the moment this merely checks for accelerator keys, if entry mnemonics
        are enabled.

        This method may come in handy if you want to use keyboard acceleration
        while the control does not have the focus.

        If the key event describes the pressing of a shortcut for an entry,
        then SelectSearchEntry resp. ExecuteSearchEntry are called.

        @see IMnemonicEntryList
        @see MnemonicEngine

        @return
            <TRUE/> if the event has been consumed, <FALSE/> otherwise.
    */
    bool            HandleKeyInput( const KeyEvent& rKEvt );

    void            SetSelectHdl( const Link<SvTreeListBox*,void>& rNewHdl ) {aSelectHdl=rNewHdl; }
    void            SetDeselectHdl( const Link<SvTreeListBox*,void>& rNewHdl ) {aDeselectHdl=rNewHdl; }
    void            SetDoubleClickHdl(const Link<SvTreeListBox*,bool>& rNewHdl) {aDoubleClickHdl=rNewHdl;}
    const Link<SvTreeListBox*,void>&   GetSelectHdl() const { return aSelectHdl; }
    const Link<SvTreeListBox*,void>&   GetDeselectHdl() const { return aDeselectHdl; }
    const Link<SvTreeListBox*,bool>&   GetDoubleClickHdl() const { return aDoubleClickHdl; }
    void            SetExpandingHdl(const Link<SvTreeListBox*,bool>& rNewHdl){aExpandingHdl=rNewHdl;}
    void            SetExpandedHdl(const Link<SvTreeListBox*,void>& rNewHdl){aExpandedHdl=rNewHdl;}

    virtual void    ExpandedHdl();
    virtual bool    ExpandingHdl();
    virtual void    SelectHdl();
    virtual void    DeselectHdl();
    virtual bool    DoubleClickHdl();
    SvTreeListEntry*    GetHdlEntry() const { return pHdlEntry; }

    // Is called for an Entry that gets expanded with the Flag
    // ENTRYFLAG_CHILDREN_ON_DEMAND set.
    virtual void             RequestingChildren( SvTreeListEntry* pParent );

    // Drag & Drop
    // New D'n'D API
    virtual sal_Int8         AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8         ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void             StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual DragDropMode     NotifyStartDrag( TransferDataContainer& rData,
                                         SvTreeListEntry* );
    virtual void             DragFinished( sal_Int8 nDropAction );
    virtual bool             NotifyAcceptDrop( SvTreeListEntry* );

    void                     SetDragOptions( sal_Int8 nOptions ) { nDragOptions = nOptions; }

    virtual SvTreeListEntry* CloneEntry( SvTreeListEntry* pSource );
    virtual SvTreeListEntry* CreateEntry() const; // To create new Entries

    // Return value: TRISTATE_TRUE == Ok, TRISTATE_FALSE == Cancel, TRISTATE_INDET == Ok and Make visible moved entry
    virtual TriState NotifyMoving(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in GetModel()
        SvTreeListEntry*  pEntry,        // Entry to be moved from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uLong&        rNewChildPos); // The TargetParent's position in Childlist

    // Return value: TRISTATE_TRUE == Ok, TRISTATE_FALSE == Cancel, TRISTATE_INDET == Ok and Make visible moved entry
    virtual TriState    NotifyCopying(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in GetModel()
        SvTreeListEntry*  pEntry,        // Entry to be copied from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uLong&        rNewChildPos); // The TargetParent's position in Childlist

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    /** Fills the StateSet of one entry. */
    void FillAccessibleEntryStateSet( SvTreeListEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Calculate and return the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    tools::Rectangle           GetBoundingRect( SvTreeListEntry* pEntry );

    /** Enables, that one cell of a tablistbox entry can be focused */
    void                EnableCellFocus();

                        // For overwriting accessible role for all entries - normally 0, so each entry can be different
    void                SetAllEntriesAccessibleRoleType( SvTreeAccRoleType n ) { nAllItemAccRoleType = n; }
    SvTreeAccRoleType   GetAllEntriesAccessibleRoleType() const { return nAllItemAccRoleType; }

    SvTreeFlags         GetTreeFlags() const {return nTreeFlags;}

    static OUString     SearchEntryTextWithHeadTitle(SvTreeListEntry* pEntry);
    virtual OUString    GetEntryAltText(SvTreeListEntry* pEntry) const;
    virtual OUString    GetEntryLongDescription(SvTreeListEntry* pEntry) const;

    void set_min_width_in_chars(sal_Int32 nChars);

    virtual bool set_property(const OString &rKey, const OUString &rValue) override;

protected:

    VCL_DLLPRIVATE void         SetEntryHeight( SvTreeListEntry const * pEntry );
                   void         AdjustEntryHeight( const Image& rBmp );
    VCL_DLLPRIVATE void         AdjustEntryHeight();

    VCL_DLLPRIVATE void         ImpEntryInserted( SvTreeListEntry* pEntry );
    VCL_DLLPRIVATE void         PaintEntry1( SvTreeListEntry&, long nLine, vcl::RenderContext& rRenderContext );

    VCL_DLLPRIVATE void         InitTreeView();
    VCL_DLLPRIVATE SvLBoxItem*  GetItem_Impl( SvTreeListEntry*, long nX, SvLBoxTab** ppTab );
    VCL_DLLPRIVATE void         ImplInitStyle();

    void            SetupDragOrigin();
    void            EditItemText( SvTreeListEntry* pEntry, SvLBoxString* pItem,
                        const Selection& );
    void            EditedText(const OUString&);

    // Recalculate all tabs depending on TreeListStyle and Bitmap sizes
    // Is called automatically when inserting/changing Bitmaps, changing the Model etc.
    virtual void    SetTabs();
    void            AddTab( long nPos, SvLBoxTabFlags nFlags );
    sal_uInt16      TabCount() const { return aTabs.size(); }
    SvLBoxTab*      GetFirstDynamicTab() const;
    SvLBoxTab*      GetFirstDynamicTab( sal_uInt16& rTabPos ) const;
    SvLBoxTab*      GetFirstTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    void            GetLastTab( SvLBoxTabFlags nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetTab( SvTreeListEntry const *, SvLBoxItem const * ) const;
    void            ClearTabList();

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind);

    virtual void    NotifyEndScroll();
    virtual void    NotifyScrolled();
    void            SetScrolledHdl( const Link<SvTreeListBox*,void>& rLink ) { aScrolledHdl = rLink; }
    long            GetXOffset() const { return GetMapMode().GetOrigin().X(); }

    virtual void    Command( const CommandEvent& rCEvt ) override;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    PreparePaint(vcl::RenderContext& rRenderContext, SvTreeListEntry& rEntry);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            InitSettings();

    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

    bool            IsCellFocusEnabled() const;
    bool            SetCurrentTabPos( sal_uInt16 _nNewPos );
    sal_uInt16      GetCurrentTabPos() const;
    void            CallImplEventListeners(VclEventId nEvent, void* pData);

    void            ImplEditEntry( SvTreeListEntry* pEntry );

    bool            AreChildrenTransient() const;
    void            SetChildrenNotTransient();

    void            AdjustEntryHeightAndRecalc();
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
                                         sal_uLong nPos=TREELIST_APPEND, void* pUserData = nullptr,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind::EnabledCheckbox );

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uLong nPos = TREELIST_APPEND, void* pUserData = nullptr,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind::EnabledCheckbox );

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

    void            SetCheckButtonHdl( const Link<SvTreeListBox*,void>& rLink )  { aCheckButtonHdl=rLink; }
    const Link<SvTreeListBox*,void>& GetCheckButtonHdl() const { return aCheckButtonHdl; }
    virtual void    CheckButtonHdl();

    void            SetSublistOpenWithReturn();      // open/close sublist with return/enter
    void            SetSublistOpenWithLeftRight();   // open/close sublist with cursor left/right
    void            SetSublistDontOpenWithDoubleClick( bool bDontOpen ); // set mouse double click open/close sublist behavior

    void            EnableInplaceEditing( bool bEnable );
    // Edits the Entry's first StringItem, 0 == Cursor
    void            EditEntry( SvTreeListEntry* pEntry );
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& );
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
    virtual void    ModelIsMoving(SvTreeListEntry* pSource,
                        SvTreeListEntry* pTargetParent, sal_uLong nChildPos ) override;
    virtual void    ModelHasMoved(SvTreeListEntry* pSource ) override;
    virtual void    ModelIsRemoving( SvTreeListEntry* pEntry ) override;
    virtual void    ModelHasRemoved( SvTreeListEntry* pEntry ) override;
    void            ModelHasEntryInvalidated( SvTreeListEntry* pEntry ) override;

    void            ScrollOutputArea( short nDeltaEntries );

    short           GetColumnsCount() const { return nColumns; }
    short           GetEntryHeight() const  { return nEntryHeight; }
    void            SetEntryHeight( short nHeight, bool bForce = false );
    short           GetEntryWidth() const { return nEntryWidth; }
    void            SetEntryWidth( short nWidth );
    Size            GetOutputSizePixel() const;
    short           GetIndent() const { return nIndent; }
    void            SetIndent( short nIndent );
    // Place the expander checkitem at the optimal indent for hierarchical lists
    void            SetOptimalImageIndent() { SetIndent(12); }
    void            SetSpaceBetweenEntries( short nSpace );
    short           GetSpaceBetweenEntries() const {return nEntryHeightOffs;}
    Point           GetEntryPosition( SvTreeListEntry* ) const;
    void            MakeVisible( SvTreeListEntry* pEntry );
    void            MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop );

    void            SetCollapsedNodeBmp( const Image& );
    void            SetExpandedNodeBmp( const Image& );
    Image const &   GetExpandedNodeBmp( ) const;

    void            SetFont( const vcl::Font& rFont );

    using Window::SetCursor;
    void            SetCursor( SvTreeListEntry* pEntry, bool bForceNoSelect = false );

    SvTreeListEntry*    GetEntry( const Point& rPos, bool bHit = false ) const;

    virtual tools::Rectangle GetFocusRect( SvTreeListEntry*, long nLine );
    // Respects indentation
    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* );
    void            InvalidateEntry( SvTreeListEntry* );
    SvLBoxItem*     GetItem( SvTreeListEntry*, long nX, SvLBoxTab** ppTab);
    SvLBoxItem*     GetItem( SvTreeListEntry*, long nX );

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    virtual bool    Expand( SvTreeListEntry* pParent );
    virtual bool    Collapse( SvTreeListEntry* pParent );
    virtual bool    Select( SvTreeListEntry* pEntry, bool bSelect=true );
    sal_uLong       SelectChildren( SvTreeListEntry* pParent, bool bSelect );
    void            SelectAll( bool bSelect, bool bPaint = true );

    void SetCurEntry( SvTreeListEntry* _pEntry );
    SvTreeListEntry* GetCurEntry() const;

    using Window::Invalidate;
    virtual void    Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE) override;
    virtual void    Invalidate( const tools::Rectangle&, InvalidateFlags nFlags = InvalidateFlags::NONE ) override;

    void            SetHighlightRange(sal_uInt16 nFirstTab=0, sal_uInt16 nLastTab=0xffff);

    // A Parent's Children are turned into Children of the Parent which comes next in hierarchy
    void            RemoveParentKeepChildren( SvTreeListEntry* pParent );

    sal_Int32       DefaultCompare(const SvLBoxString* pLeftText, const SvLBoxString* pRightText);

    DECL_LINK( DefaultCompare, const SvSortData&, sal_Int32 );
    virtual void    ModelNotification( SvListAction nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uLong nPos ) override;

    void            EndSelection();
    ScrollBar*      GetVScroll();
    ScrollBar*      GetHScroll();
    void            EnableAsyncDrag( bool b );

    SvTreeListEntry*    GetFirstEntryInView() const;
    SvTreeListEntry*    GetNextEntryInView(SvTreeListEntry*) const;
    SvTreeListEntry*    GetPrevEntryInView(SvTreeListEntry*) const;
    SvTreeListEntry*    GetLastEntryInView() const;
    void            ScrollToAbsPos( long nPos );

    void            ShowFocusRect( const SvTreeListEntry* pEntry );
    void            InitStartEntry();

    virtual VclPtr<PopupMenu> CreateContextMenu();
    virtual void    ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

    void            EnableContextMenuHandling();

    long            getPreferredDimensions(std::vector<long> &rWidths) const;

    virtual Size    GetOptimalSize() const override;

    void            SetAlternatingRowColors( const bool bEnable );

    // Enables type-ahead search in the check list box.
    void            SetQuickSearch(bool bEnable) { mbQuickSearch = bEnable; }

    void            SetForceMakeVisible(bool bEnable);

    virtual FactoryFunction GetUITestFactory() const override;
};

class SvInplaceEdit2
{
    Link<SvInplaceEdit2&,void> const aCallBackHdl;
    Accelerator   aAccReturn;
    Accelerator   aAccEscape;
    Idle          aIdle;
    VclPtr<Edit>  pEdit;
    bool          bCanceled;
    bool          bAlreadyInCallBack;

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
    bool        EditingCanceled() const { return bCanceled; }
    OUString    GetText() const;
    OUString const & GetSavedValue() const;
    void        StopEditing( bool bCancel );
    void        Hide();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
