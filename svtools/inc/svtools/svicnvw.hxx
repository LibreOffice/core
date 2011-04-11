/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVICNVW_HXX
#define _SVICNVW_HXX

#include <vcl/image.hxx>
#include <svtools/svlbox.hxx>

class SvImpIconView;
class SvLBoxEntry;
class SvLBoxItem;
class SvLBoxString;
class SvLBoxContextBmp;
class SvPtrarr;

#define WB_ICON         0x0001
#define WB_NAME         0x0002
#define WB_TEXT         0x0004
#define WB_FLOW         0x0008

#define ICNVW_FLAG_POS_LOCKED   0x0001
#define ICNVW_FLAG_USER1        0x1000
#define ICNVW_FLAG_USER2        0x2000
#define ICNVW_FLAG_USER3        0x4000
#define ICNVW_FLAG_USER4        0x8000

enum SvIconViewTextMode
{
    ShowTextFull = 1,
    ShowTextShort,
    ShowTextSmart,          // not implemented
    ShowTextDontKnow        // only for entries (uses config? from the view)
};

class SvIcnVwDataEntry : public SvViewDataEntry
{
friend class SvImpIconView;
    sal_uInt16      nIcnVwFlags;
    SvIconViewTextMode eTextMode;
public:
    Rectangle   aRect;              // Bounding-Rect of the Entry
    Rectangle   aGridRect;          // set in grid mode
    Size        aTextSize;          // set in grid mode only
    sal_uInt16  nX,nY;              // for keyboard control
                SvIcnVwDataEntry();
    virtual     ~SvIcnVwDataEntry();

    sal_Bool        IsEntryPosLocked() const { return (sal_Bool)((nIcnVwFlags & ICNVW_FLAG_POS_LOCKED) !=0); }
    void        ClearVwFlags( sal_uInt16 nMask ) { nIcnVwFlags &= (~nMask); }
    void        SetVwFlags( sal_uInt16 nMask ) { nIcnVwFlags |= nMask; }
    sal_uInt16      GetVwFlags() const { return nIcnVwFlags; }
    SvIconViewTextMode GetTextMode() const { return eTextMode; }
};

#define SV_LISTBOX_ID_ICONVIEW 2

class SvIconView : public SvLBox
{
    friend class SvImpIconView;

    SvImpIconView*  pImp;
    Image           aCollapsedEntryBmp;
    Image           aExpandedEntryBmp;
    sal_uInt16          nIcnVwFlags;
    void            SetModel( SvLBoxTreeList* );

protected:

    using SvLBox::CreateEntry;
    SvLBoxEntry*    CreateEntry( const XubString&, const Image&, const Image&);
    void            ViewDataInitialized( SvLBoxEntry* );
    virtual SvViewData* CreateViewData( SvListEntry* );
    virtual void    InitViewData( SvViewData* pData, SvListEntry* pEntry );

    void            EditingRequest(SvLBoxEntry*,SvLBoxItem*,const Point& );
    void            EditedText( const XubString& );
    void            EditItemText( SvLBoxEntry*,SvLBoxItem*,const Selection& );
    SvLBoxEntry*    pCurEdEntry;
    SvLBoxItem*     pCurEdItem;

    virtual void    WriteDragServerInfo( const Point&, SvLBoxDDInfo* );
    virtual void    ReadDragServerInfo( const Point&, SvLBoxDDInfo* );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    PreparePaint( SvLBoxEntry* );
    virtual void    StateChanged( StateChangedType nStateChange );

public:

    SvIconView( Window* pParent,WinBits nWinStyle=0 );
    SvIconView( Window* pParent,const ResId& rResId );
    ~SvIconView();

    void DisconnectFromModel();

    SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent = 0,
                        sal_Bool bChildsOnDemand = sal_False,
                        sal_uLong nPos=LIST_APPEND );

    SvLBoxEntry*    InsertEntry( const XubString& rText,
                        const Image& rExpandedEntryBmp,
                        const Image& rCollapsedEntryBmp,
                        SvLBoxEntry* pParent = 0,
                        sal_Bool bChildsOnDemand = sal_False,
                        sal_uLong nPos = LIST_APPEND );

    const Image&    GetDefaultExpandedEntryBmp() const { return aExpandedEntryBmp;}
    const Image&    GetDefaultCollapsedEntryBmp() const { return aCollapsedEntryBmp;}
    void            SetDefaultExpandedEntryBmp( const Image& rBmp) { aExpandedEntryBmp=rBmp;}
    void            SetDefaultCollapsedEntryBmp( const Image& rBmp ) { aCollapsedEntryBmp=rBmp;}

    void            SetEntryText(SvLBoxEntry*, const XubString& );
    void            SetExpandedEntryBmp(SvLBoxEntry*, const Image& );
    void            SetCollapsedEntryBmp(SvLBoxEntry*, const Image& );

    XubString       GetEntryText(SvLBoxEntry*) const;
    Image           GetExpandedEntryBmp(SvLBoxEntry*) const;
    Image           GetCollapsedEntryBmp(SvLBoxEntry*) const;

    virtual SvLBoxEntry* CloneEntry( SvLBoxEntry* pSource );

    virtual sal_uInt16  IsA();

    virtual void    RequestingChilds( SvLBoxEntry* pParent );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    void            SetUpdateMode( sal_Bool );

    using SvListView::SetModel;
    virtual void    SetModel( SvLBoxTreeList*, SvLBoxEntry* pParent );
    virtual void    ModelHasCleared();
    virtual void    ModelHasInserted( SvListEntry* pEntry );
    virtual void    ModelHasInsertedTree( SvListEntry* pEntry );
    virtual void    ModelIsMoving(SvListEntry* pSource,
                        SvListEntry* pTargetParent, sal_uLong nChildPos );
    virtual void    ModelHasMoved(SvListEntry* pSource );
    virtual void    ModelIsRemoving( SvListEntry* pEntry );
    virtual void    ModelHasRemoved( SvListEntry* pEntry );
    virtual void    ModelHasEntryInvalidated( SvListEntry* pEntry );

    virtual void    ShowTargetEmphasis( SvLBoxEntry*, sal_Bool bShow );
    using Window::GetDropTarget;
    virtual SvLBoxEntry* GetDropTarget( const Point& );
    virtual Region  GetDragRegion() const;
    // NotifyMoving/Copying is overloaded, since GetDropTarget
    // returns a "magic pointer" if the drop happens in/on an empty
    // area(?) of the IconView
    virtual sal_Bool    NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos);
    virtual sal_Bool    NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos);

    // returns TopLeft of the BoundingRect. Add MapMode.Origin to get the
    // position relative to the window
     Point          GetEntryPosition( SvLBoxEntry* ) const;
    void            SetEntryPosition( SvLBoxEntry*, const Point& rDocPos);
    void            SetEntryPosition( SvLBoxEntry*, const Point& rDocPos,
                                      sal_Bool bAdjustAtGrid );

    void            SetFont( const Font& rFont );
    void            SetDefaultFont();

    using SvLBox::GetEntry;
    SvLBoxEntry*    GetEntry( const Point& rPixPos, sal_Bool  ) const;
    // returns the entry just above pCurEntry (z-wise)
    SvLBoxEntry*    GetNextEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, sal_Bool  ) const;
    // returns the entry just below pCurEntry (z-wise)
    SvLBoxEntry*    GetPrevEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, sal_Bool  ) const;

    SvLBoxEntry*    GetEntryFromLogicPos( const Point& rDocPos ) const;

    virtual void    PaintEntry( SvLBoxEntry* );
    virtual void    PaintEntry( SvLBoxEntry*, const Point& rDocPos );
    Rectangle       GetFocusRect( SvLBoxEntry* );
    void            InvalidateEntry( SvLBoxEntry* );
    void            MakeVisible( SvLBoxEntry* );

    void            SetDragDropMode( DragDropMode );
    void            SetSelectionMode( SelectionMode );

    using SvListView::Select;
    sal_Bool            Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    void            SelectAll( sal_Bool bSelect, sal_Bool bPaint=sal_True );
    virtual void    SetCurEntry( SvLBoxEntry* _pEntry );
    virtual SvLBoxEntry*
                    GetCurEntry() const;

    // locigal coordinates
    void            SelectRect(
                        const Rectangle& rRect,
                        sal_Bool bAdd = sal_False,
                        // inverts the intersection with rRect
                        // (ignored if bAdd == sal_False)
                        SvPtrarr* pOtherRects = 0,
                        short nBorderOffs = -5 );
    sal_uLong           GetSelectionCount() const;

    virtual void    Arrange();
    void            SetSpaceBetweenEntries( long nHor, long Ver );
    long            GetHorSpaceBetweenEntries();
    long            GetVerSpaceBetweenEntries();

    void            EnableInplaceEditing( sal_Bool bEnable );
    void            EditEntry( SvLBoxEntry* pEntry = 0 );
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& );
    virtual sal_Bool    EditedEntry( SvLBoxEntry*, const XubString& rNewText );

    void            SetCurParent( SvLBoxEntry* pNewParent );
    SvLBoxEntry*    GetCurParent() const;

    virtual void    ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, sal_uLong nPos );

    // pass (0, 0) to switch off grid mode
    void            SetGrid( long nDX, long nDY );

    // nDeltaY < 0 : View moves up relative to Doc
    // nDeltaY > 0 : View moves down relative to Doc
    // nDeltaX < 0 : View moves left relative to Doc
    // nDeltaX > 0 : View moves right relative to Doc
    using Window::Scroll;
    virtual void    Scroll( long nDeltaX, long nDeltaY, sal_uInt16 nFlags = 0 );

    virtual void    PrepareCommandEvent( const CommandEvent& );
    virtual void    StartDrag( sal_Int8 nAction, const Point& );
    virtual void    DragFinished( sal_Int8 );
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    using SvLBox::ExecuteDrop;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    void            ShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );
    void            HideDDIcon();
    void            HideShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );

    // to scroll during Drag&Drop
    void            CalcScrollOffsets(
                        const Point& rRefPosPixel,
                        long& rScrollX,
                        long& rScrollY,
                        sal_Bool bInDragDrop = sal_False,
                        sal_uInt16 nBorderWidth = 10 );

    using Window::EndTracking;
    void            EndTracking();
    void            AdjustAtGrid( SvLBoxEntry* pEntry = 0 );
    void            LockEntryPos( SvLBoxEntry* pEntry, sal_Bool bLock = sal_True );
    sal_Bool            IsEntryPosLocked( const SvLBoxEntry* pEntry ) const;

    void            SetTextMode( SvIconViewTextMode, SvLBoxEntry* pEntry = 0 );
    SvIconViewTextMode GetTextMode( const SvLBoxEntry* pEntry = 0 ) const;

    void            ShowFocusRect( const SvLBoxEntry* pEntry );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
