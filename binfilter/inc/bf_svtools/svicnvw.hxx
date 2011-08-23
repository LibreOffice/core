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

#include <bf_svtools/svlbox.hxx>

namespace binfilter
{

class SvPtrarr;
class SvImpIconView;
class SvLBoxEntry;
class SvLBoxItem;
class SvLBoxString;
class SvLBoxContextBmp;

#define WB_ICON			0x0001
#define WB_NAME	        0x0002
#define WB_TEXT			0x0004
#define WB_FLOW			0x0008

#define ICNVW_FLAG_POS_LOCKED	0x0001
#define ICNVW_FLAG_USER1		0x1000
#define ICNVW_FLAG_USER2		0x2000
#define ICNVW_FLAG_USER3		0x4000
#define ICNVW_FLAG_USER4		0x8000

enum SvIconViewTextMode
{
    ShowTextFull = 1,
    ShowTextShort,
    ShowTextSmart,			// not implemented
    ShowTextDontKnow		// nur fuer Eintraege (->Einstellung der View
                            // wird genommen)
};

class SvIcnVwDataEntry : public SvViewDataEntry
{
friend class SvImpIconView;
    USHORT		nIcnVwFlags;
    SvIconViewTextMode eTextMode;
public:
    Rectangle 	aRect;				// Bounding-Rect des Entries
    Rectangle 	aGridRect;			// gesetzt im Grid-Modus
    Size		aTextSize;          // nur gesetzt im Grid-Modus
    USHORT 		nX,nY;				// fuer Tastatursteuerung
                SvIcnVwDataEntry();
    virtual 	~SvIcnVwDataEntry();

    BOOL		IsEntryPosLocked() const { return (BOOL)((nIcnVwFlags & ICNVW_FLAG_POS_LOCKED) !=0); }
    void		ClearVwFlags( USHORT nMask ) { nIcnVwFlags &= (~nMask); }
    void		SetVwFlags( USHORT nMask ) { nIcnVwFlags |= nMask; }
    USHORT		GetVwFlags() const { return nIcnVwFlags; }
    SvIconViewTextMode GetTextMode() const { return eTextMode; }
};

#define SV_LISTBOX_ID_ICONVIEW 2

class SvIconView : public SvLBox
{
    friend class SvImpIconView;

    SvImpIconView*	pImp;
    Image          	aCollapsedEntryBmp;
    Image          	aExpandedEntryBmp;
    WinBits			nWinBits;
    USHORT			nIcnVwFlags;
    void			SetModel( SvLBoxTreeList* );

protected:

    using SvLBox::CreateEntry;
    SvLBoxEntry*	CreateEntry( const XubString&, const Image&, const Image&);
    void			ViewDataInitialized( SvLBoxEntry* );
    virtual	SvViewData* CreateViewData( SvListEntry* );
    virtual void	InitViewData( SvViewData* pData, SvListEntry* pEntry );

    void			EditingRequest(SvLBoxEntry*,SvLBoxItem*,const Point& );
    void			EditedText( const XubString& );
    void			EditItemText( SvLBoxEntry*,SvLBoxItem*,const Selection& );
    SvLBoxEntry*	pCurEdEntry;
    SvLBoxItem*		pCurEdItem;

    virtual void 	WriteDragServerInfo( const Point&, SvLBoxDDInfo* );
    virtual void 	ReadDragServerInfo( const Point&, SvLBoxDDInfo* );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void	PreparePaint( SvLBoxEntry* );

public:

    SvIconView( Window* pParent,WinBits nWinStyle=0 );
    SvIconView( Window* pParent,const ResId& rResId );
    ~SvIconView();

    void DisconnectFromModel();

    SvLBoxEntry*	InsertEntry( const XubString& rText, SvLBoxEntry* pParent = 0,
                        BOOL bChildsOnDemand = FALSE,
                        ULONG nPos=LIST_APPEND );

    SvLBoxEntry*	InsertEntry( const XubString& rText,
                        const Image& rExpandedEntryBmp,
                        const Image& rCollapsedEntryBmp,
                        SvLBoxEntry* pParent = 0,
                        BOOL bChildsOnDemand = FALSE,
                        ULONG nPos = LIST_APPEND );

    const Image&	GetDefaultExpandedEntryBmp() const { return aExpandedEntryBmp;}
    const Image&	GetDefaultCollapsedEntryBmp() const { return aCollapsedEntryBmp;}
    void			SetDefaultExpandedEntryBmp( const Image& rBmp) { aExpandedEntryBmp=rBmp;}
    void          	SetDefaultCollapsedEntryBmp( const Image& rBmp ) { aCollapsedEntryBmp=rBmp;}

    void			SetEntryText(SvLBoxEntry*, const XubString& );
    void			SetExpandedEntryBmp(SvLBoxEntry*, const Image& );
    void			SetCollapsedEntryBmp(SvLBoxEntry*, const Image& );

    XubString 		GetEntryText(SvLBoxEntry*) const;
    Image			GetExpandedEntryBmp(SvLBoxEntry*) const;
    Image			GetCollapsedEntryBmp(SvLBoxEntry*) const;

    virtual SvLBoxEntry* CloneEntry( SvLBoxEntry* pSource );

    virtual USHORT	IsA();

    virtual void	RequestingChilds( SvLBoxEntry* pParent );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    void 			SetUpdateMode( BOOL );

    using SvListView::SetModel;
    virtual void	SetModel( SvLBoxTreeList*, SvLBoxEntry* pParent );
    virtual void 	ModelHasCleared();
    virtual void 	ModelHasInserted( SvListEntry* pEntry );
    virtual void 	ModelHasInsertedTree( SvListEntry* pEntry );
    virtual void 	ModelIsMoving(SvListEntry* pSource,
                        SvListEntry* pTargetParent, ULONG nChildPos );
    virtual void 	ModelHasMoved(SvListEntry* pSource );
    virtual void 	ModelIsRemoving( SvListEntry* pEntry );
    virtual void 	ModelHasRemoved( SvListEntry* pEntry );
    virtual void 	ModelHasEntryInvalidated( SvListEntry* pEntry );

    virtual void	ShowTargetEmphasis( SvLBoxEntry*, BOOL bShow );
    using Window::GetDropTarget;
    virtual SvLBoxEntry* GetDropTarget( const Point& );
    virtual Region	GetDragRegion() const;
    // NotifyMoving/Copying ueberladen, da wir bei GetDropTarget
    // einen "magic pointer" zurueckgeben, falls in einen leeren
    // Bereich auf der IconView gedroppt wird.
    virtual BOOL	NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos);
    virtual BOOL	NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos);

    // gibt TopLeft des BoundingRects zurueck
    // Konvertierung in Fensterposition: MapMode.Origin addieren
    Point			GetEntryPosition( SvLBoxEntry* ) const;
    void			SetEntryPosition( SvLBoxEntry*, const Point& rDocPos);
    void			SetEntryPosition( SvLBoxEntry*, const Point& rDocPos,
                                      BOOL bAdjustAtGrid );

    void			SetFont( const Font& rFont );
    void			SetDefaultFont();

    using SvLBox::GetEntry;
    SvLBoxEntry*	GetEntry( const Point& rPixPos, BOOL  ) const;
    // Gibt den naechsten ueber pCurEntry liegenden Eintrag (ZOrder)
    SvLBoxEntry*	GetNextEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, BOOL  ) const;
    // Gibt den naechsten unter pCurEntry liegenden Eintrag (ZOrder)
    SvLBoxEntry*	GetPrevEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, BOOL  ) const;

    SvLBoxEntry*	GetEntryFromLogicPos( const Point& rDocPos ) const;

    void			SetWindowBits( WinBits nWinStyle );
    virtual void	PaintEntry( SvLBoxEntry* );
    virtual void	PaintEntry( SvLBoxEntry*, const Point& rDocPos );
    Rectangle		GetFocusRect( SvLBoxEntry* );
    void			InvalidateEntry( SvLBoxEntry* );
    void			MakeVisible( SvLBoxEntry* );

    void			SetDragDropMode( DragDropMode );
    void 			SetSelectionMode( SelectionMode );

    using SvListView::Select;
    BOOL            Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );
    void			SelectAll( BOOL bSelect, BOOL bPaint=TRUE );
    virtual void    SetCurEntry( SvLBoxEntry* _pEntry );
    virtual SvLBoxEntry*
                    GetCurEntry() const;

    // logische Koordinaten
    void			SelectRect(
                        const Rectangle& rRect,
                        BOOL bAdd = FALSE,
                        // die Schnittmenge mit rRect wird invertiert
                        // wird bei bAdd == FALSE ignoriert
                        SvPtrarr* pOtherRects = 0,
                        short nBorderOffs = -5 );
    ULONG			GetSelectionCount() const;

    virtual void	Arrange();
    void			SetSpaceBetweenEntries( long nHor, long Ver );
    long			GetHorSpaceBetweenEntries();
    long			GetVerSpaceBetweenEntries();

    void			EnableInplaceEditing( BOOL bEnable );
    void			EditEntry( SvLBoxEntry* pEntry = 0 );
    virtual BOOL	EditingEntry( SvLBoxEntry* pEntry, Selection& );
    virtual BOOL	EditedEntry( SvLBoxEntry*, const XubString& rNewText );

    void			SetCurParent( SvLBoxEntry* pNewParent );
    SvLBoxEntry*	GetCurParent() const;

    virtual void	ModelNotification( USHORT nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, ULONG nPos );

    // 0,0: Grid-Modus ausschalten
    void			SetGrid( long nDX, long nDY );

    // nDeltaY < 0 : View bewegt sich auf dem Doc nach oben
    // nDeltaY > 0 : View bewegt sich auf dem Doc nach unten
    // nDeltaX < 0 : View bewegt sich auf dem Doc nach links
    // nDeltaX > 0 : View bewegt sich auf dem Doc nach rechts
    using Window::Scroll;
    virtual void	Scroll( long nDeltaX, long nDeltaY, USHORT nFlags = 0 );

    virtual void	PrepareCommandEvent( const CommandEvent& );
    virtual void	StartDrag( sal_Int8 nAction, const Point& );
    virtual void	DragFinished( sal_Int8 );
    virtual sal_Int8	AcceptDrop( const AcceptDropEvent& rEvt );
    using SvLBox::ExecuteDrop;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    void 			ShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );
    void 			HideDDIcon();
    void			HideShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );

    // fuers Scrollen beim D&D
    void			CalcScrollOffsets(
                        const Point& rRefPosPixel,
                        long& rScrollX,
                        long& rScrollY,
                        BOOL bInDragDrop = FALSE,
                        USHORT nBorderWidth = 10 );

    using Window::EndTracking;
    void			EndTracking();
    void			AdjustAtGrid( SvLBoxEntry* pEntry = 0 );
    void			LockEntryPos( SvLBoxEntry* pEntry, BOOL bLock = TRUE );
    BOOL			IsEntryPosLocked( const SvLBoxEntry* pEntry ) const;

    void			SetTextMode( SvIconViewTextMode, SvLBoxEntry* pEntry = 0 );
    SvIconViewTextMode GetTextMode( const SvLBoxEntry* pEntry = 0 ) const;

    void			ShowFocusRect( const SvLBoxEntry* pEntry );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
