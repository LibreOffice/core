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

#ifndef __SVTOOLS_TREELISTBOX_HXX__
#define __SVTOOLS_TREELISTBOX_HXX__

#include "svtools/svtdllapi.h"

#include <deque>
#include <vector>

#include <vcl/ctrl.hxx>
#include <vcl/seleng.hxx>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>
#include <vcl/accel.hxx>
#include <vcl/mnemonicengine.hxx>
#include <vcl/quickselectionengine.hxx>
#include <vcl/image.hxx>
#include <tools/gen.hxx>
#include <tools/contnr.hxx>
#include <svtools/treelist.hxx>
#include <svtools/transfer.hxx>

class Application;
class SvTreeListBox;
class SvTreeListEntry;
struct SvViewDataItem;
class SvViewDataEntry;
class SvInplaceEdit2;
class SvLBoxString;
class SvLBoxButton;
class SvLBoxContextBmp;
class SvLBoxBmp;
class SvImpLBox;
class SvLBoxButtonData;
struct SvLBoxDDInfo;

namespace utl {
    class AccessibleStateSetHelper;
}

enum SvLBoxButtonKind
{
    SvLBoxButtonKind_enabledCheckbox,
    SvLBoxButtonKind_disabledCheckbox,
    SvLBoxButtonKind_staticImage
};

enum SvButtonState { SV_BUTTON_UNCHECKED, SV_BUTTON_CHECKED, SV_BUTTON_TRISTATE };

// *********************************************************************
// *************************** Tabulators ******************************
// *********************************************************************

#define SV_LBOXTAB_DYNAMIC          0x0001 // Item's output column changes according to the Child Depth
#define SV_LBOXTAB_ADJUST_RIGHT     0x0002 // Item's right margin at the tabulator
#define SV_LBOXTAB_ADJUST_LEFT      0x0004 // Left margin
#define SV_LBOXTAB_ADJUST_CENTER    0x0008 // Center the item at the tabulator
#define SV_LBOXTAB_ADJUST_NUMERIC   0x0010 // Decimal point at the tabulator (strings)

// Is not supported anymore! The focus is now controlled by selection!
#define SV_LBOXTAB_SHOW_FOCUS       0x0020 // Visualize focus

#define SV_LBOXTAB_SHOW_SELECTION   0x0040 // Visualize selection state
                                           // Item needs to be able to return the surrounding polygon (D'n'D cursor)
#define SV_LBOXTAB_EDITABLE         0x0100 // Item editable at the tabulator
#define SV_LBOXTAB_PUSHABLE         0x0200 // Item acts like a Button
#define SV_LBOXTAB_INV_ALWAYS       0x0400 // Always delete the background
#define SV_LBOXTAB_FORCE            0x0800 // Switch off the default calculation of the first tabulator
                                           // (on which Abo Tabpage/Extras/Option/Customize, etc. rely on)
                                           // The first tab's position corresponds precisely to the Flags set
                                           // and column widths

#define SV_TAB_BORDER 8

#define SV_ENTRYHEIGHTOFFS_PIXEL 2

#define TREEFLAG_CHKBTN         0x0001
#define TREEFLAG_USESEL         0x0002
#define TREEFLAG_MANINS         0x0004
#define TREEFLAG_RECALCTABS     0x0008

typedef sal_Int64   ExtendedWinBits;

// disable the behavior of automatically selecting a "CurEntry" upon painting the control
#define EWB_NO_AUTO_CURENTRY        0x00000001

#define SV_ITEM_ID_LBOXSTRING       1
#define SV_ITEM_ID_LBOXBMP          2
#define SV_ITEM_ID_LBOXBUTTON       3
#define SV_ITEM_ID_LBOXCONTEXTBMP   4

class SvLBoxTab
{
    long    nPos;
    void*   pUserData;
public:
            SvLBoxTab();
            SvLBoxTab( long nPos, sal_uInt16 nFlags=SV_LBOXTAB_ADJUST_LEFT );
            SvLBoxTab( const SvLBoxTab& );
            ~SvLBoxTab();

    sal_uInt16  nFlags;

    void    SetUserData( void* pPtr ) { pUserData = pPtr; }
    void*   GetUserData() const { return pUserData; }
    sal_Bool    IsDynamic() const { return (sal_Bool)((nFlags & SV_LBOXTAB_DYNAMIC)!=0); }
    void    SetPos( long nNewPos) { nPos = nNewPos; }
    long    GetPos() const { return nPos; }
    long    CalcOffset( long nItemLength, long nTabWidth );
    sal_Bool    IsEditable() const { return (sal_Bool)((nFlags & SV_LBOXTAB_EDITABLE)!=0); }
    sal_Bool    IsPushable() const { return (sal_Bool)((nFlags & SV_LBOXTAB_PUSHABLE)!=0); }
};

// *********************************************************************
// ****************************** Items ********************************
// *********************************************************************

class SVT_DLLPUBLIC SvLBoxItem
{
public:
                        SvLBoxItem( SvTreeListEntry*, sal_uInt16 nFlags );
                        SvLBoxItem();
    virtual             ~SvLBoxItem();
    virtual sal_uInt16 GetType() const = 0;
    const Size&         GetSize(const SvTreeListBox* pView, const SvTreeListEntry* pEntry) const;
    const Size&         GetSize(const SvViewDataEntry* pData, sal_uInt16 nItemPos) const;

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) = 0;

    virtual void        InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry,
                            // If != 0: this Pointer must be used!
                            // If == 0: it needs to be retrieved via the View
                            SvViewDataItem* pViewData = 0) = 0;
    virtual SvLBoxItem* Create() const = 0;
    // View-dependent data is not cloned
    virtual void        Clone( SvLBoxItem* pSource ) = 0;
};

inline SvLBoxItem* new_clone(const SvLBoxItem& rSrc)
{
    SvLBoxItem* p = rSrc.Create();
    p->Clone(const_cast<SvLBoxItem*>(&rSrc));
    return p;
}

inline void delete_clone(const SvLBoxItem* p)
{
    delete p;
}

// *********************************************************************
// ****************************** SvTreeListBox ************************
// *********************************************************************

#define WB_FORCE_SELECTION          ((WinBits)0x8000)

#define DragDropMode sal_uInt16
#define SV_DRAGDROP_NONE            (DragDropMode)0x0000
#define SV_DRAGDROP_CTRL_MOVE       (DragDropMode)0x0001
#define SV_DRAGDROP_CTRL_COPY       (DragDropMode)0x0002
#define SV_DRAGDROP_APP_MOVE        (DragDropMode)0x0004
#define SV_DRAGDROP_APP_COPY        (DragDropMode)0x0008
#define SV_DRAGDROP_APP_DROP        (DragDropMode)0x0010
// Entries may be dropped via the uppermost Entry
// The DropTarget is 0 in that case
#define SV_DRAGDROP_ENABLE_TOP      (DragDropMode)0x0020

#define SVLISTBOX_ID_LBOX 0   // for SvTreeListBox::GetType()

#define SVLBOX_IN_EDT           0x0001
#define SVLBOX_EDT_ENABLED      0x0002
#define SVLBOX_IS_EXPANDING     0x0004
#define SVLBOX_IS_TRAVELSELECT  0x0008
#define SVLBOX_TARGEMPH_VIS     0x0010
#define SVLBOX_EDTEND_CALLED    0x0020

struct SvTreeListBoxImpl;

class SVT_DLLPUBLIC SvTreeListBox
                :public Control
                ,public SvListView
                ,public DropTargetHelper
                ,public DragSourceHelper
                ,public ::vcl::IMnemonicEntryList
                ,public ::vcl::ISearchableStringList
{
    friend class SvImpLBox;
    friend class TreeControlPeer;

    SvTreeListBoxImpl* mpImpl;
    SvImpLBox*      pImp;
    Link            aCheckButtonHdl;
    Link            aScrolledHdl;
    Link            aExpandedHdl;
    Link            aExpandingHdl;
    Link            aSelectHdl;
    Link            aDeselectHdl;

    Accelerator     aInpEditAcc;
    Image           aPrevInsertedExpBmp;
    Image           aPrevInsertedColBmp;
    Image           aCurInsertedExpBmp;
    Image           aCurInsertedColBmp;

    short           nContextBmpWidthMax;
    short           nEntryHeight;
    short           nEntryHeightOffs;
    short           nIndent;
    short           nFocusWidth;
    sal_uInt16      nFirstSelTab;
    sal_uInt16      nLastSelTab;
    long mnCheckboxItemWidth;
    bool mbContextBmpExpanded;

    SvTreeListEntry*    pHdlEntry;
    SvLBoxItem*     pHdlItem;

    DragDropMode    nDragDropMode;
    DragDropMode    nOldDragMode;
    SelectionMode   eSelMode;
    sal_Int8        nDragOptions;

    SvTreeListEntry*        pEdEntry;
    SvLBoxItem*         pEdItem;

protected:
    Link            aDoubleClickHdl;
    SvTreeListEntry*    pTargetEntry;
    SvLBoxButtonData*   pCheckButtonData;
    std::vector<SvLBoxTab*> aTabs;
    sal_uInt16      nTreeFlags;
    sal_uInt16      nImpFlags;
    // Move/CopySelection: Position of the current Entry in SelectionList
    sal_uInt16      nCurEntrySelPos;

private:
    void SetBaseModel(SvTreeList* pNewModel);

    DECL_DLLPRIVATE_LINK( CheckButtonClick, SvLBoxButtonData * );
    DECL_DLLPRIVATE_LINK( TextEditEndedHdl_Impl, void * );
    // Handler that is called by TreeList to clone an Entry
    DECL_DLLPRIVATE_LINK( CloneHdl_Impl, SvTreeListEntry* );

    // Handler and methods for Drag - finished handler.
    // The Handle retrieved by GetDragFinishedHdl can be set on the
    // TransferDataContainer. This link is a callback for the DragFinished
    // call. The AddBox method is called from the GetDragFinishedHdl() and the
    // remove is called in the link callback and in the dtor. So it can't be
    // called for a deleted object.
    SVT_DLLPRIVATE static void AddBoxToDDList_Impl( const SvTreeListBox& rB );
    SVT_DLLPRIVATE static void RemoveBoxFromDDList_Impl( const SvTreeListBox& rB );
    DECL_DLLPRIVATE_STATIC_LINK( SvTreeListBox, DragFinishHdl_Impl, sal_Int8* );

protected:

    sal_Bool            CheckDragAndDropMode( SvTreeListBox* pSource, sal_Int8 );
    void            ImplShowTargetEmphasis( SvTreeListEntry* pEntry, sal_Bool bShow);
    void            EnableSelectionAsDropTarget( sal_Bool bEnable = sal_True,
                                                 sal_Bool bWithChildren = sal_True );
    // Standard impl returns 0; must be overloaded by derived classes which support D'n'D
    using Window::GetDropTarget;
    virtual SvTreeListEntry* GetDropTarget( const Point& );

    // Put View-specific data into the Dragserver
    // Is called at the SourceView (in BeginDrag Handler)
    virtual void WriteDragServerInfo( const Point&, SvLBoxDDInfo* );
    // Is called at the TargetView (in Drop Handler)
    virtual void ReadDragServerInfo( const Point&,SvLBoxDDInfo* );

    // Invalidate children on enable/disable
    virtual void StateChanged( StateChangedType eType );

    virtual sal_uLong Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uLong nPos=LIST_APPEND);
    virtual sal_uLong Insert( SvTreeListEntry* pEntry,sal_uLong nRootPos = LIST_APPEND );

    // In-place editing
    SvInplaceEdit2*  pEdCtrl;
    void            EditText( const OUString&, const Rectangle&,const Selection&);
    void            EditText( const OUString&, const Rectangle&,const Selection&, sal_Bool bMulti);
    void            EditTextMultiLine( const OUString&, const Rectangle&,const Selection&);
    void            CancelTextEditing();
    sal_Bool            EditingCanceled() const;
    bool            IsEmptyTextAllowed() const;

    // Return value must be derived from SvViewDataEntry!
    virtual SvViewDataEntry* CreateViewData( SvTreeListEntry* );
    // InitViewData is called right after CreateViewData
    // The Entry is has not yet been added to the View in InitViewData!
    virtual void InitViewData( SvViewDataEntry*, SvTreeListEntry* pEntry );
    // Calls InitViewData for all Items
    void            RecalcViewData();
    // Callback of RecalcViewData
    virtual void    ViewDataInitialized( SvTreeListEntry* );

    // Handler and methods for Drag - finished handler. This link can be set
    // to the TransferDataContainer. The AddBox/RemoveBox methods must be
    // called before the StartDrag call.
    // The Remove will be called from the handler, which then calls DragFinish.
    // The Remove is also called in the DTOR of the SvTreeListBox -
    // so it can't be called for a deleted object.
    Link GetDragFinishedHdl() const;

    // For asynchronous D'n'D
    sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt, SvTreeListBox* pSourceView );

    void            OnCurrentEntryChanged();

    // IMnemonicEntryList
    virtual const void* FirstSearchEntry( OUString& _rEntryText ) const;
    virtual const void* NextSearchEntry( const void* _pCurrentSearchEntry, OUString& _rEntryText ) const;
    virtual void        SelectSearchEntry( const void* _pEntry );
    virtual void        ExecuteSearchEntry( const void* _pEntry ) const;

    // ISearchableStringList
    virtual ::vcl::StringEntryIdentifier    CurrentEntry( OUString& _out_entryText ) const;
    virtual ::vcl::StringEntryIdentifier    NextEntry( ::vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const;
    virtual void                            SelectEntry( ::vcl::StringEntryIdentifier _entry );

public:

    SvTreeListBox( Window* pParent, WinBits nWinStyle=0 );
    SvTreeListBox( Window* pParent, const ResId& rResId );
    virtual ~SvTreeListBox();

    SvTreeList* GetModel() const { return pModel; }
    using SvListView::SetModel;
    void SetModel(SvTreeList* pNewModel);

    sal_uLong           GetEntryCount() const {return pModel->GetEntryCount();}
    SvTreeListEntry*    First() const { return pModel->First(); }
    SvTreeListEntry*    Next( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const { return pModel->Next(pEntry,pDepth); }
    SvTreeListEntry*    Prev( SvTreeListEntry* pEntry, sal_uInt16* pDepth=0 ) const { return pModel->Prev(pEntry,pDepth); }
    SvTreeListEntry*    Last() const { return pModel->Last(); }

    SvTreeListEntry*    FirstChild( SvTreeListEntry* pParent ) const;
    SvTreeListEntry*    NextSibling( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*    PrevSibling( SvTreeListEntry* pEntry ) const;

    sal_Bool            CopySelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget );
    sal_Bool            MoveSelection( SvTreeListBox* pSource, SvTreeListEntry* pTarget );
    sal_Bool            MoveSelectionCopyFallbackPossible( SvTreeListBox* pSource, SvTreeListEntry* pTarget, sal_Bool bAllowCopyFallback );
    void            RemoveSelection();

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
    sal_uLong           GetChildCount( SvTreeListEntry* pParent ) const;
    sal_uLong           GetLevelChildCount( SvTreeListEntry* pParent ) const;

    SvViewDataEntry* GetViewDataEntry( SvTreeListEntry* pEntry ) const;
    SvViewDataItem*  GetViewDataItem(SvTreeListEntry*, SvLBoxItem*);
    const SvViewDataItem*  GetViewDataItem(const SvTreeListEntry*, const SvLBoxItem*) const;

    bool IsInplaceEditingEnabled() const { return ((nImpFlags & SVLBOX_EDT_ENABLED) != 0); }
    bool IsEditingActive() const { return ((nImpFlags & SVLBOX_IN_EDT) != 0); }
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
    void            EnableEntryMnemonics( bool _bEnable = true );
    bool            IsEntryMnemonicsEnabled() const;

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

    void            SetSelectHdl( const Link& rNewHdl ) {aSelectHdl=rNewHdl; }
    void            SetDeselectHdl( const Link& rNewHdl ) {aDeselectHdl=rNewHdl; }
    void            SetDoubleClickHdl(const Link& rNewHdl) {aDoubleClickHdl=rNewHdl;}
    const Link&     GetSelectHdl() const { return aSelectHdl; }
    const Link&     GetDeselectHdl() const { return aDeselectHdl; }
    const Link&     GetDoubleClickHdl() const { return aDoubleClickHdl; }
    void            SetExpandingHdl(const Link& rNewHdl){aExpandingHdl=rNewHdl;}
    void            SetExpandedHdl(const Link& rNewHdl){aExpandedHdl=rNewHdl;}
    const Link&     GetExpandingHdl() const { return aExpandingHdl; }

    virtual void    ExpandedHdl();
    virtual long    ExpandingHdl();
    virtual void    SelectHdl();
    virtual void    DeselectHdl();
    virtual sal_Bool    DoubleClickHdl();
    sal_Bool            IsTravelSelect() const { return (sal_Bool)((nImpFlags&SVLBOX_IS_TRAVELSELECT)!=0);}
    SvTreeListEntry*    GetHdlEntry() const { return pHdlEntry; }
    SvLBoxItem*     GetHdlItem() const;

    // Is called for an Entry that gets expanded with the Flag
    // ENTRYFLAG_CHILDREN_ON_DEMAND set.
    virtual void RequestingChildren( SvTreeListEntry* pParent );

    // Drag & Drop
    // New D'n'D API
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData,
                                         SvTreeListEntry* );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual sal_Bool        NotifyAcceptDrop( SvTreeListEntry* );

    void            SetDragOptions( sal_Int8 nOptions ) { nDragOptions = nOptions; }
    sal_Int8        GetDragOptions() const { return nDragOptions; }

    SvTreeListBox*         GetSourceView() const;

    virtual void    NotifyRemoving( SvTreeListEntry* );
    virtual SvTreeListEntry* CloneEntry( SvTreeListEntry* pSource );
    virtual SvTreeListEntry* CreateEntry() const; // To create new Entries

    // Return value: sal_True == Ok, sal_False == Cancel
    virtual sal_Bool    NotifyMoving(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in this->GetModel()
        SvTreeListEntry*  pEntry,        // Entry to be moved from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uLong&        rNewChildPos); // The TargetParent's position in Childlist

    // Return value: sal_True == Ok, sal_False == Cancel
    virtual sal_Bool    NotifyCopying(
        SvTreeListEntry*  pTarget,       // D'n'D DropPosition in this->GetModel()
        SvTreeListEntry*  pEntry,        // Entry to be copied from GetSourceListBox()->GetModel()
        SvTreeListEntry*& rpNewParent,   // New TargetParent
        sal_uLong&        rNewChildPos); // The TargetParent's position in Childlist

    // ACCESSIBILITY ==========================================================

    /** Creates and returns the accessible object of the Box. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    /** Fills the StateSet with all states (except DEFUNC, done by the accessible object). */
    virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Fills the StateSet of one entry. */
    virtual void FillAccessibleEntryStateSet( SvTreeListEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const;

    /** Calculate and return the bounding rectangle of an entry.
        @param pEntry
            The entry.
        @return  The bounding rectangle of an entry. */
    virtual Rectangle   GetBoundingRect( SvTreeListEntry* pEntry );

    /** Enables, that one cell of a tablistbox entry can be focused */
    void EnableCellFocus();

protected:
    using SvListView::Expand;
    using SvListView::Collapse;
    using SvListView::Select;
    using SvListView::SelectAll;

    SVT_DLLPRIVATE short        GetHeightOffset( const Image& rBmp, Size& rLogicSize);
    SVT_DLLPRIVATE short        GetHeightOffset( const Font& rFont, Size& rLogicSize);

    SVT_DLLPRIVATE void         SetEntryHeight( SvTreeListEntry* pEntry );
    SVT_DLLPRIVATE void         AdjustEntryHeight( const Image& rBmp );
    SVT_DLLPRIVATE void         AdjustEntryHeight( const Font& rFont );

    SVT_DLLPRIVATE void         ImpEntryInserted( SvTreeListEntry* pEntry );
    SVT_DLLPRIVATE long         PaintEntry1( SvTreeListEntry*, long nLine,
                                sal_uInt16 nTabFlagMask=0xffff,
                                sal_Bool bHasClipRegion=sal_False );

    SVT_DLLPRIVATE void         InitTreeView();
    SVT_DLLPRIVATE SvLBoxItem*  GetItem_Impl( SvTreeListEntry*, long nX, SvLBoxTab** ppTab,
                        sal_uInt16 nEmptyWidth );
    SVT_DLLPRIVATE void         ImplInitStyle();

protected:

    void            EditItemText( SvTreeListEntry* pEntry, SvLBoxString* pItem,
                        const Selection& );
    void            EditedText(const OUString&);

    // Recalculate all tabs depending on TreeListStyle and Bitmap sizes
    // Is called automatically when inserting/changing Bitmaps, changing the Model etc.
    virtual void    SetTabs();
    void            SetTabs_Impl();
    void            AddTab( long nPos,sal_uInt16 nFlags=SV_LBOXTAB_ADJUST_LEFT,
                        void* pUserData = 0 );
    sal_uInt16      TabCount() const { return aTabs.size(); }
    SvLBoxTab*      GetFirstDynamicTab() const;
    SvLBoxTab*      GetFirstDynamicTab( sal_uInt16& rTabPos ) const;
    SvLBoxTab*      GetFirstTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetLastTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos );
    SvLBoxTab*      GetTab( SvTreeListEntry*, SvLBoxItem* ) const;
    void            ClearTabList();

    virtual void InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind);

    virtual void    NotifyBeginScroll();
    virtual void    NotifyEndScroll();
    // nLines == 0 => horizontal Scrolling
    virtual void    NotifyScrolling( long nLines );
    virtual void    NotifyScrolled();
    void            SetScrolledHdl( const Link& rLink ) { aScrolledHdl = rLink; }
    const Link&     GetScrolledHdl() const { return aScrolledHdl; }
    long            GetXOffset() const { return GetMapMode().GetOrigin().X(); }

    // Is called _before_ Areas in the Control are invalidated.
    // This can be used to hide Elements which are painted from outside into the Control
    virtual void    NotifyInvalidating();

    virtual void    Command( const CommandEvent& rCEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    CursorMoved( SvTreeListEntry* pNewCursor );
    virtual void    PreparePaint( SvTreeListEntry* );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            InitSettings(sal_Bool bFont,sal_Bool bForeground,sal_Bool bBackground);
    sal_Bool            IsCellFocusEnabled() const;
    bool            SetCurrentTabPos( sal_uInt16 _nNewPos );
    sal_uInt16          GetCurrentTabPos() const;
    void            CallImplEventListeners(sal_uLong nEvent, void* pData);

    void            ImplEditEntry( SvTreeListEntry* pEntry );

    sal_Bool        AreChildrenTransient() const;
    void            SetChildrenNotTransient();

    void            AdjustEntryHeightAndRecalc( const Font& rFont );
public:

    void                SetExtendedWinBits( ExtendedWinBits _nBits );

    void            DisconnectFromModel();

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
    inline  void    SetNodeDefaultImages( )
    {
        SetNodeBitmaps(
            GetDefaultCollapsedNodeImage( ),
            GetDefaultExpandedNodeImage( )
        );
    }

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos=LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvTreeListEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos = LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    const Image&    GetDefaultExpandedEntryBmp( ) const;
    const Image&    GetDefaultCollapsedEntryBmp( ) const;

    void            SetDefaultExpandedEntryBmp( const Image& rBmp );
    void            SetDefaultCollapsedEntryBmp( const Image& rBmp );

    void            SetCheckButtonState( SvTreeListEntry*, SvButtonState );
    void            SetCheckButtonInvisible( SvTreeListEntry* );
    SvButtonState   GetCheckButtonState( SvTreeListEntry* ) const;

    void            SetEntryText(SvTreeListEntry*, const OUString& );
    void            SetExpandedEntryBmp( SvTreeListEntry* _pEntry, const Image& _rImage );
    void            SetCollapsedEntryBmp( SvTreeListEntry* _pEntry, const Image& _rImage );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const;
    OUString        SearchEntryText( SvTreeListEntry* pEntry ) const;
    const Image&    GetExpandedEntryBmp(const SvTreeListEntry* _pEntry ) const;
    const Image&    GetCollapsedEntryBmp(const SvTreeListEntry* _pEntry ) const;

    void            SetCheckButtonHdl( const Link& rLink )  { aCheckButtonHdl=rLink; }
    Link            GetCheckButtonHdl() const { return aCheckButtonHdl; }
    virtual void    CheckButtonHdl();

    void            SetSublistOpenWithReturn( sal_Bool bMode = sal_True );      // open/close sublist with return/enter
    void            SetSublistOpenWithLeftRight( sal_Bool bMode = sal_True );   // open/close sublist with cursor left/right

    void            EnableInplaceEditing( bool bEnable );
    // Edits the Entry's first StringItem, 0 == Cursor
    void            EditEntry( SvTreeListEntry* pEntry = NULL );
    virtual sal_Bool    EditingEntry( SvTreeListEntry* pEntry, Selection& );
    virtual sal_Bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    void            SetUpdateMode( sal_Bool );

    virtual void    ModelHasCleared();
    virtual void    ModelHasInserted( SvTreeListEntry* pEntry );
    virtual void    ModelHasInsertedTree( SvTreeListEntry* pEntry );
    virtual void    ModelIsMoving(SvTreeListEntry* pSource,
                        SvTreeListEntry* pTargetParent, sal_uLong nChildPos );
    virtual void    ModelHasMoved(SvTreeListEntry* pSource );
    virtual void    ModelIsRemoving( SvTreeListEntry* pEntry );
    virtual void    ModelHasRemoved( SvTreeListEntry* pEntry );
    void ModelHasEntryInvalidated( SvTreeListEntry* pEntry );

    void            ShowTargetEmphasis( SvTreeListEntry*, sal_Bool bShow );
    void            ScrollOutputArea( short nDeltaEntries );

    short           GetEntryHeight() const  { return nEntryHeight; }
    void            SetEntryHeight( short nHeight, sal_Bool bAlways = sal_False );
    Size            GetOutputSizePixel() const;
    short           GetIndent() const { return nIndent; }
    void            SetIndent( short nIndent );
    void            SetSpaceBetweenEntries( short nSpace );
    short           GetSpaceBetweenEntries() const {return nEntryHeightOffs;}
    Point           GetEntryPosition( SvTreeListEntry* ) const;
    void            ShowEntry( SvTreeListEntry* );  // !!!OBSOLETE, use MakeVisible
    void            MakeVisible( SvTreeListEntry* pEntry );
    void            MakeVisible( SvTreeListEntry* pEntry, sal_Bool bMoveToTop );

    void            SetCollapsedNodeBmp( const Image& );
    void            SetExpandedNodeBmp( const Image& );
    Image           GetExpandedNodeBmp( ) const;

    void            SetFont( const Font& rFont );

    using Window::SetCursor;
    void            SetCursor( SvTreeListEntry* pEntry, sal_Bool bForceNoSelect = sal_False );

    SvTreeListEntry*    GetEntry( const Point& rPos, sal_Bool bHit = sal_False ) const;

    void            PaintEntry( SvTreeListEntry* );
    long            PaintEntry( SvTreeListEntry*, long nLine,
                                sal_uInt16 nTabFlagMask=0xffff );
    virtual Rectangle GetFocusRect( SvTreeListEntry*, long nLine );
    // Respects indentation
    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* );
    void            InvalidateEntry( SvTreeListEntry* );
    SvLBoxItem*     GetItem( SvTreeListEntry*, long nX, SvLBoxTab** ppTab);
    SvLBoxItem*     GetItem( SvTreeListEntry*, long nX );

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    sal_Bool Expand( SvTreeListEntry* pParent );
    sal_Bool Collapse( SvTreeListEntry* pParent );
    virtual sal_Bool Select( SvTreeListEntry* pEntry, sal_Bool bSelect=sal_True );
    sal_uLong SelectChildren( SvTreeListEntry* pParent, sal_Bool bSelect );
    virtual void SelectAll( sal_Bool bSelect, sal_Bool bPaint = sal_True );

    void SetCurEntry( SvTreeListEntry* _pEntry );
    SvTreeListEntry* GetCurEntry() const;

    using Window::Invalidate;
    virtual void    Invalidate( sal_uInt16 nFlags = 0);
    virtual void    Invalidate( const Rectangle&, sal_uInt16 nFlags = 0 );

    void            SetHighlightRange(sal_uInt16 nFirstTab=0,sal_uInt16 nLastTab=0xffff);

    // A Parent's Children are turned into Children of the Parent which comes next in hierarchy
    void            RemoveParentKeepChildren( SvTreeListEntry* pParent );

    DECL_LINK( DefaultCompare, SvSortData* );
    virtual void    ModelNotification( sal_uInt16 nActionId, SvTreeListEntry* pEntry1,
                        SvTreeListEntry* pEntry2, sal_uLong nPos );

    void            EndSelection();
    void            RepaintScrollBars() const;
    ScrollBar*      GetVScroll();
    ScrollBar*      GetHScroll();
    void            EnableAsyncDrag( sal_Bool b );

    SvTreeListEntry*    GetFirstEntryInView() const;
    SvTreeListEntry*    GetNextEntryInView(SvTreeListEntry*) const;
    SvTreeListEntry*    GetLastEntryInView() const;
    void            ScrollToAbsPos( long nPos );

    void            ShowFocusRect( const SvTreeListEntry* pEntry );
    void            InitStartEntry();

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

    void            EnableContextMenuHandling( void );
    void            EnableContextMenuHandling( sal_Bool bEnable );
    sal_Bool        IsContextMenuHandlingEnabled( void ) const;

    void            EnableList( bool _bEnable );

    long getPreferredDimensions(std::vector<long> &rWidths) const;

    virtual Size GetOptimalSize() const;
};

#define SV_LBOX_DD_FORMAT "SV_LBOX_DD_FORMAT"
struct SvLBoxDDInfo
{
    Application*    pApp;
    SvTreeListBox*         pSource;
    SvTreeListEntry*    pDDStartEntry;
    // Relative position in the Entry at DragBeginn (IconView)
    long            nMouseRelX,nMouseRelY;
    sal_uLong           nRes1,nRes2,nRes3,nRes4;
};

class SvInplaceEdit2
{
    Link        aCallBackHdl;
    Accelerator aAccReturn;
    Accelerator aAccEscape;
    Timer       aTimer;
    Edit*       pEdit;
    sal_Bool        bCanceled;
    sal_Bool        bAlreadyInCallBack;

    void        CallCallBackHdl_Impl();
    DECL_LINK( Timeout_Impl, void * );
    DECL_LINK( ReturnHdl_Impl, void * );
    DECL_LINK( EscapeHdl_Impl, void * );

public:
                SvInplaceEdit2( Window* pParent, const Point& rPos, const Size& rSize,
                   const OUString& rData, const Link& rNotifyEditEnd,
                   const Selection&, sal_Bool bMultiLine = sal_False );
               ~SvInplaceEdit2();
    sal_Bool        KeyInput( const KeyEvent& rKEvt );
    void        LoseFocus();
    sal_Bool        EditingCanceled() const { return bCanceled; }
    OUString    GetText() const;
    OUString    GetSavedValue() const;
    void        StopEditing( sal_Bool bCancel = sal_False );
    void        Hide();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
