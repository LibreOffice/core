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

#ifndef _SV_ILSTBOX_HXX
#define _SV_ILSTBOX_HXX

#include <boost/ptr_container/ptr_vector.hpp>

#include <tools/solar.h>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/lstbox.h>
#include <vcl/timer.hxx>

#include "vcl/quickselectionengine.hxx"

class ScrollBar;
class ScrollBarBox;

// -----------------
// - ListBox-Types -
// -----------------

#define HORZ_SCROLL         4
#define IMG_TXT_DISTANCE    6

enum LB_EVENT_TYPE
{
    LET_MBDOWN,
    LET_TRACKING,
    LET_TRACKING_END,
    LET_KEYMOVE,
    LET_KEYSPACE
};

// -----------------
// - ImplEntryType -
// -----------------

struct ImplEntryType
{
    XubString   maStr;
    Image       maImage;
    void*       mpUserData;
    sal_Bool        mbIsSelected;
    long        mnFlags;
    long        mnHeight;

                ImplEntryType( const XubString& rStr, const Image& rImage ) :
                    maStr( rStr ),
                    maImage( rImage ),
                    mnFlags( 0 ),
                    mnHeight( 0 )
                {
                    mbIsSelected = sal_False;
                    mpUserData = NULL;
                }

                ImplEntryType( const XubString& rStr ) :
                    maStr( rStr ),
                    mnFlags( 0 ),
                    mnHeight( 0 )
                {
                    mbIsSelected = sal_False;
                    mpUserData = NULL;
                }

                ImplEntryType( const Image& rImage ) :
                    maImage( rImage ),
                    mnFlags( 0 ),
                    mnHeight( 0 )
                {
                    mbIsSelected = sal_False;
                    mpUserData = NULL;
                }
};

// -----------------
// - ImplEntryList -
// -----------------

class ImplEntryList
{
private:
    Window*         mpWindow;   // For getting the current locale when matching strings
    sal_uInt16          mnLastSelected;
    sal_uInt16          mnSelectionAnchor;
    sal_uInt16          mnImages;

    sal_uInt16          mnMRUCount;
    sal_uInt16          mnMaxMRUCount;

    Link            maSelectionChangedHdl;
    sal_Bool            mbCallSelectionChangedHdl;
    boost::ptr_vector<ImplEntryType> maEntries;

    ImplEntryType*  GetEntry( sal_uInt16 nPos ) const
    {
        if (nPos >= maEntries.size())
            return NULL;
        return const_cast<ImplEntryType*>(&maEntries[nPos]);
    }

public:
                    ImplEntryList( Window* pWindow );
                    ~ImplEntryList();

    sal_uInt16                  InsertEntry( sal_uInt16 nPos, ImplEntryType* pNewEntry, sal_Bool bSort );
    void                    RemoveEntry( sal_uInt16 nPos );
    const ImplEntryType*    GetEntryPtr( sal_uInt16 nPos ) const { return (const ImplEntryType*) GetEntry( nPos ); }
    ImplEntryType*          GetMutableEntryPtr( sal_uInt16 nPos ) const { return GetEntry( nPos ); }
    void                    Clear();

    sal_uInt16          FindMatchingEntry( const XubString& rStr, sal_uInt16 nStart = 0, sal_Bool bForward = sal_True, sal_Bool bLazy = sal_True ) const;
    sal_uInt16          FindEntry( const XubString& rStr, sal_Bool bSearchMRUArea = sal_False ) const;
    sal_uInt16          FindEntry( const void* pData ) const;

    // helper: add up heights up to index nEndIndex.
    // GetAddedHeight( 0 ) returns 0
    // GetAddedHeight( LISTBOX_ENTRY_NOTFOUND ) returns 0
    // GetAddedHeight( i, k ) with k > i is equivalent -GetAddedHeight( k, i )
    long            GetAddedHeight( sal_uInt16 nEndIndex, sal_uInt16 nBeginIndex = 0, long nBeginHeight = 0 ) const;
    long            GetEntryHeight( sal_uInt16 nPos ) const;

    sal_uInt16          GetEntryCount() const { return (sal_uInt16)maEntries.size(); }
    sal_Bool            HasImages() const { return mnImages ? sal_True : sal_False; }

    XubString       GetEntryText( sal_uInt16 nPos ) const;

    sal_Bool            HasEntryImage( sal_uInt16 nPos ) const;
    Image           GetEntryImage( sal_uInt16 nPos ) const;

    void            SetEntryData( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData( sal_uInt16 nPos ) const;

    void            SetEntryFlags( sal_uInt16 nPos, long nFlags );
    long            GetEntryFlags( sal_uInt16 nPos ) const;

    void            SelectEntry( sal_uInt16 nPos, sal_Bool bSelect );

    sal_uInt16          GetSelectEntryCount() const;
    XubString       GetSelectEntry( sal_uInt16 nIndex ) const;
    sal_uInt16          GetSelectEntryPos( sal_uInt16 nIndex ) const;
    sal_Bool            IsEntryPosSelected( sal_uInt16 nIndex ) const;

    void            SetLastSelected( sal_uInt16 nPos )  { mnLastSelected = nPos; }
    sal_uInt16          GetLastSelected() const { return mnLastSelected; }

    void            SetSelectionAnchor( sal_uInt16 nPos )   { mnSelectionAnchor = nPos; }
    sal_uInt16          GetSelectionAnchor() const { return mnSelectionAnchor; }


    void            SetSelectionChangedHdl( const Link& rLnk )  { maSelectionChangedHdl = rLnk; }
    void            SetCallSelectionChangedHdl( sal_Bool bCall )    { mbCallSelectionChangedHdl = bCall; }

    void            SetMRUCount( sal_uInt16 n ) { mnMRUCount = n; }
    sal_uInt16          GetMRUCount() const     { return mnMRUCount; }

    void            SetMaxMRUCount( sal_uInt16 n )  { mnMaxMRUCount = n; }
    sal_uInt16          GetMaxMRUCount() const      { return mnMaxMRUCount; }

    /** An Entry is selectable if its mnFlags does not have the
        LISTBOX_ENTRY_FLAG_DISABLE_SELECTION flag set. */
    bool            IsEntrySelectable( sal_uInt16 nPos ) const;

    /** returns the first entry found from the given position nPos that is selectable
        or LISTBOX_ENTRY_NOTFOUND if non is found. If the entry at nPos is not selectable,
        it returns the first selectable entry after nPos if bForward is true and the
        first selectable entry after nPos is bForward is false.
        */
    sal_uInt16          FindFirstSelectable( sal_uInt16 nPos, bool bForward = true );

    void take_properties(ImplEntryList &rOther);
};

// ---------------------
// - ImplListBoxWindow -
// ---------------------

class ImplListBoxWindow : public Control, public ::vcl::ISearchableStringList
{
private:
    ImplEntryList*  mpEntryList;     // EntryList
    Rectangle       maFocusRect;

    Size            maUserItemSize;

    long            mnMaxTxtHeight;  // Maximum height of a text item
    long            mnMaxTxtWidth;   // Maximum width of a text item
                                     // Entry without Image
    long            mnMaxImgTxtWidth;// Maximum width of a text item
                                     // Entry AND Image
    long            mnMaxImgWidth;   // Maximum width of an image item
    long            mnMaxImgHeight;  // Maximum height of an image item
    long            mnMaxWidth;      // Maximum width of an entry
    long            mnMaxHeight;     // Maximum heigth of an entry

    sal_uInt16          mnCurrentPos;    // Position (Focus)
    sal_uInt16          mnTrackingSaveSelection; // Selection before Tracking();

    sal_uInt16          mnSeparatorPos; // Separator

    sal_uInt16          mnUserDrawEntry;

    sal_uInt16      mnTop;           // output from line on
    long            mnLeft;          // output from column on
    long            mnBorder;        // distance border - text
    long            mnTextHeight;    // text height
    ProminentEntry  meProminentType; // where is the "prominent" entry

    sal_uInt16          mnSelectModifier;   // Modifiers

    sal_Bool            mbHasFocusRect:         1,
                    mbSort:                 1,  // ListBox sorted
                    mbTrack:                1,  // Tracking
                    mbMulti:                1,  // MultiListBox
                    mbStackMode:            1,  // StackSelection
                    mbSimpleMode:           1,  // SimpleMode for MultiListBox
                    mbImgsDiffSz:           1,  // Images have different sizes
                    mbTravelSelect:         1,  // TravelSelect
                    mbTrackingSelect:       1,  // Selected at a MouseMove
                    mbSelectionChanged:     1,  // Do not call Select() too often ...
                    mbMouseMoveSelect:      1,  // Select at MouseMove
                    mbGrabFocus:            1,  // Grab focus at MBDown
                    mbUserDrawEnabled:      1,  // UserDraw possible
                    mbInUserDraw:           1,  // In UserDraw
                    mbReadOnly:             1,  // ReadOnly
                    mbMirroring:            1,  // pb: #106948# explicit mirroring for calc
                    mbRight:                1,  // right align Text output
                    mbCenter:               1;  // center Text output

    Link            maScrollHdl;
    Link            maSelectHdl;
    Link            maCancelHdl;
    Link            maDoubleClickHdl;
    Link            maUserDrawHdl;
    Link            maMRUChangedHdl;

    ::vcl::QuickSelectionEngine
                    maQuickSelectionEngine;

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();

    sal_Bool            SelectEntries( sal_uInt16 nSelect, LB_EVENT_TYPE eLET, sal_Bool bShift = sal_False, sal_Bool bCtrl = sal_False );
    void            ImplPaint( sal_uInt16 nPos, sal_Bool bErase = sal_False, bool bLayout = false );
    void            ImplDoPaint( const Rectangle& rRect, bool bLayout = false );
    void            ImplCalcMetrics();
    void            ImplUpdateEntryMetrics( ImplEntryType& rEntry );
    void            ImplCallSelect();

    void            ImplShowFocusRect();
    void            ImplHideFocusRect();


    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    virtual void  FillLayoutData() const;

                    ImplListBoxWindow( Window* pParent, WinBits nWinStyle );
                    ~ImplListBoxWindow();

    ImplEntryList*  GetEntryList() const { return mpEntryList; }

    sal_uInt16          InsertEntry( sal_uInt16 nPos, ImplEntryType* pNewEntry );
    void            RemoveEntry( sal_uInt16 nPos );
    void            Clear();
    void            ResetCurrentPos()               { mnCurrentPos = LISTBOX_ENTRY_NOTFOUND; }
    sal_uInt16          GetCurrentPos() const           { return mnCurrentPos; }
    sal_uInt16          GetDisplayLineCount() const;
    void            SetEntryFlags( sal_uInt16 nPos, long nFlags );

    void            DrawEntry( sal_uInt16 nPos, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos = sal_False, bool bLayout = false );

    void            SelectEntry( sal_uInt16 nPos, sal_Bool bSelect );
    void            DeselectAll();
    sal_uInt16          GetEntryPosForPoint( const Point& rPoint ) const;
    sal_uInt16          GetLastVisibleEntry() const;

    sal_Bool            ProcessKeyInput( const KeyEvent& rKEvt );

    void            SetTopEntry( sal_uInt16 nTop );
    sal_uInt16          GetTopEntry() const             { return mnTop; }
    // ShowProminentEntry will set the entry correspoding to nEntryPos
    // either at top or in the middle depending on the chosen style
    void            ShowProminentEntry( sal_uInt16 nEntryPos );
    void            SetProminentEntryType( ProminentEntry eType ) { meProminentType = eType; }
    ProminentEntry  GetProminentEntryType() const { return meProminentType; }
    using Window::IsVisible;
    sal_Bool            IsVisible( sal_uInt16 nEntry ) const;

    long            GetLeftIndent() const           { return mnLeft; }
    void            SetLeftIndent( long n );
    void            ScrollHorz( long nDiff );

    void            AllowGrabFocus( sal_Bool b )        { mbGrabFocus = b; }
    sal_Bool            IsGrabFocusAllowed() const      { return mbGrabFocus; }

    void            SetSeparatorPos( sal_uInt16 n )     { mnSeparatorPos = n; }
    sal_uInt16          GetSeparatorPos() const         { return mnSeparatorPos; }

    void            SetTravelSelect( sal_Bool bTravelSelect ) { mbTravelSelect = bTravelSelect; }
    sal_Bool            IsTravelSelect() const          { return mbTravelSelect; }
    sal_Bool            IsTrackingSelect() const            { return mbTrackingSelect; }

    void            SetUserItemSize( const Size& rSz );
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( sal_Bool bUserDraw ) { mbUserDrawEnabled = bUserDraw; }
    sal_Bool            IsUserDrawEnabled() const   { return mbUserDrawEnabled; }

    void            EnableMultiSelection( sal_Bool bMulti, sal_Bool bStackMode ) { mbMulti = bMulti; mbStackMode = bStackMode; }
    sal_Bool            IsMultiSelectionEnabled() const     { return mbMulti; }

    void            SetMultiSelectionSimpleMode( sal_Bool bSimple ) { mbSimpleMode = bSimple; }
    sal_Bool            IsMultiSelectionSimpleMode() const          { return mbSimpleMode; }

    void            EnableMouseMoveSelect( sal_Bool bMouseMoveSelect ) { mbMouseMoveSelect = bMouseMoveSelect; }
    sal_Bool            IsMouseMoveSelectEnabled() const    { return mbMouseMoveSelect; }
    sal_Bool            IsMouseMoveSelect() const   { return mbMouseMoveSelect||mbStackMode; }

    Size            CalcSize( sal_uInt16 nMaxLines ) const;
    Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const;

    long            GetEntryHeight() const              { return mnMaxHeight; }
    long            GetMaxEntryWidth() const            { return mnMaxWidth; }

    void            SetScrollHdl( const Link& rLink )   { maScrollHdl = rLink; }
    const Link&     GetScrollHdl() const                { return maScrollHdl; }
    void            SetSelectHdl( const Link& rLink )   { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                { return maSelectHdl; }
    void            SetCancelHdl( const Link& rLink )   { maCancelHdl = rLink; }
    const Link&     GetCancelHdl() const                { return maCancelHdl; }
    void            SetDoubleClickHdl( const Link& rLink )  { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const               { return maDoubleClickHdl; }
    void            SetUserDrawHdl( const Link& rLink ) { maUserDrawHdl = rLink; }
    const Link&     GetUserDrawHdl() const              { return maUserDrawHdl; }
    void            SetMRUChangedHdl( const Link& rLink )   { maMRUChangedHdl = rLink; }
    const Link&     GetMRUChangedHdl() const                { return maMRUChangedHdl; }

    sal_Bool            IsSelectionChanged() const { return mbSelectionChanged; }
    sal_uInt16          GetSelectModifier() const { return mnSelectModifier; }

    void            EnableSort( sal_Bool b ) { mbSort = b; }

    void            SetReadOnly( sal_Bool bReadOnly )   { mbReadOnly = bReadOnly; }
    sal_Bool            IsReadOnly() const              { return mbReadOnly; }

    using Control::ImplInitSettings;
    void            ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    sal_uInt16          ImplGetTextStyle() const;

    // pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()       { mbMirroring = sal_True; }
    inline sal_Bool     IsMirroring() const { return mbMirroring; }

    /*
     * Takes ownership of the rOther properties
     */
    virtual void take_properties(Window &rOther);

protected:
    // ISearchableStringList
    virtual ::vcl::StringEntryIdentifier    CurrentEntry( String& _out_entryText ) const;
    virtual ::vcl::StringEntryIdentifier    NextEntry( ::vcl::StringEntryIdentifier _currentEntry, String& _out_entryText ) const;
    virtual void                            SelectEntry( ::vcl::StringEntryIdentifier _entry );
};

// ---------------
// - ImplListBox -
// ---------------

class ImplListBox : public Control
{
private:
    ImplListBoxWindow   maLBWindow;
    ScrollBar*          mpHScrollBar;
    ScrollBar*          mpVScrollBar;
    ScrollBarBox*       mpScrollBarBox;
    sal_Bool                mbVScroll       : 1,    // VScroll an oder aus
                        mbHScroll       : 1,    // HScroll an oder aus
                        mbAutoHScroll   : 1;    // AutoHScroll an oder aus
    Link                maScrollHdl;    // Weil der vom ImplListBoxWindow selbst benoetigt wird.
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDNDListenerContainer;

protected:
    virtual void        GetFocus();
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    long                Notify( NotifyEvent& rNEvt );

    void                ImplResizeControls();
    void                ImplCheckScrollBars();
    void                ImplInitScrollBars();

    DECL_LINK(          ScrollBarHdl, ScrollBar* );
    DECL_LINK(          LBWindowScrolled, void* );
    DECL_LINK(          MRUChanged, void* );

public:
                    ImplListBox( Window* pParent, WinBits nWinStyle );
                    ~ImplListBox();

    const ImplEntryList*    GetEntryList() const            { return maLBWindow.GetEntryList(); }
    ImplListBoxWindow*      GetMainWindow()                 { return &maLBWindow; }

    virtual void    Resize();
    virtual const Wallpaper& GetDisplayBackground() const;
    virtual Window*     GetPreferredKeyInputWindow();

    sal_uInt16          InsertEntry( sal_uInt16 nPos, const XubString& rStr );
    sal_uInt16          InsertEntry( sal_uInt16 nPos, const Image& rImage );
    sal_uInt16          InsertEntry( sal_uInt16 nPos, const XubString& rStr, const Image& rImage );
    void            RemoveEntry( sal_uInt16 nPos );
    void            SetEntryData( sal_uInt16 nPos, void* pNewData ) { maLBWindow.GetEntryList()->SetEntryData( nPos, pNewData ); }
    void            Clear();

    void            SetEntryFlags( sal_uInt16 nPos, long nFlags );

    void            SelectEntry( sal_uInt16 nPos, sal_Bool bSelect );
    void            SetNoSelection();
    void            ResetCurrentPos()               { maLBWindow.ResetCurrentPos(); }
    sal_uInt16          GetCurrentPos() const           { return maLBWindow.GetCurrentPos(); }

    sal_Bool            ProcessKeyInput( const KeyEvent& rKEvt )    { return maLBWindow.ProcessKeyInput( rKEvt ); }
    sal_Bool            HandleWheelAsCursorTravel( const CommandEvent& rCEvt );

    void            SetSeparatorPos( sal_uInt16 n )     { maLBWindow.SetSeparatorPos( n ); }
    sal_uInt16          GetSeparatorPos() const         { return maLBWindow.GetSeparatorPos(); }

    void            SetTopEntry( sal_uInt16 nTop )      { maLBWindow.SetTopEntry( nTop ); }
    sal_uInt16          GetTopEntry() const             { return maLBWindow.GetTopEntry(); }
    void            ShowProminentEntry( sal_uInt16 nPos ) { maLBWindow.ShowProminentEntry( nPos ); }
    using Window::IsVisible;
    sal_Bool            IsVisible( sal_uInt16 nEntry ) const { return maLBWindow.IsVisible( nEntry ); }

    void            SetProminentEntryType( ProminentEntry eType ) { maLBWindow.SetProminentEntryType( eType ); }
    ProminentEntry  GetProminentEntryType() const { return maLBWindow.GetProminentEntryType(); }

    long            GetLeftIndent() const           { return maLBWindow.GetLeftIndent(); }
    void            SetLeftIndent( sal_uInt16 n )       { maLBWindow.SetLeftIndent( n ); }
    void            ScrollHorz( short nDiff )       { maLBWindow.ScrollHorz( nDiff ); }

    void            SetTravelSelect( sal_Bool bTravelSelect ) { maLBWindow.SetTravelSelect( bTravelSelect ); }
    sal_Bool            IsTravelSelect() const          { return maLBWindow.IsTravelSelect(); }
    sal_Bool            IsTrackingSelect() const            { return maLBWindow.IsTrackingSelect(); }

    void            EnableMultiSelection( sal_Bool bMulti, sal_Bool bStackMode ) { maLBWindow.EnableMultiSelection( bMulti, bStackMode ); }
    sal_Bool            IsMultiSelectionEnabled() const     { return maLBWindow.IsMultiSelectionEnabled(); }

    void            SetMultiSelectionSimpleMode( sal_Bool bSimple ) { maLBWindow.SetMultiSelectionSimpleMode( bSimple ); }
    sal_Bool            IsMultiSelectionSimpleMode() const  { return maLBWindow.IsMultiSelectionSimpleMode(); }

    void            SetReadOnly( sal_Bool b )           { maLBWindow.SetReadOnly( b ); }
    sal_Bool            IsReadOnly() const              { return maLBWindow.IsReadOnly(); }


    Size            CalcSize( sal_uInt16 nMaxLines ) const              { return maLBWindow.CalcSize( nMaxLines ); }
    long            GetEntryHeight() const          { return maLBWindow.GetEntryHeight(); }
    long            GetMaxEntryWidth() const        { return maLBWindow.GetMaxEntryWidth(); }

    void            SetScrollHdl( const Link& rLink )   { maScrollHdl = rLink; }
    const Link&     GetScrollHdl() const                { return maScrollHdl; }
    void            SetSelectHdl( const Link& rLink )   { maLBWindow.SetSelectHdl( rLink ); }
    const Link&     GetSelectHdl() const                { return maLBWindow.GetSelectHdl(); }
    void            SetCancelHdl( const Link& rLink )   { maLBWindow.SetCancelHdl( rLink ); }
    const Link&     GetCancelHdl() const                { return maLBWindow.GetCancelHdl(); }
    void            SetDoubleClickHdl( const Link& rLink )  { maLBWindow.SetDoubleClickHdl( rLink ); }
    const Link&     GetDoubleClickHdl() const               { return maLBWindow.GetDoubleClickHdl(); }
    void            SetUserDrawHdl( const Link& rLink ) { maLBWindow.SetUserDrawHdl( rLink ); }
    const Link&     GetUserDrawHdl() const              { return maLBWindow.GetUserDrawHdl(); }

    void            SetSelectionChangedHdl( const Link& rLnk )  { maLBWindow.GetEntryList()->SetSelectionChangedHdl( rLnk ); }
    void            SetCallSelectionChangedHdl( sal_Bool bCall )    { maLBWindow.GetEntryList()->SetCallSelectionChangedHdl( bCall ); }
    sal_Bool            IsSelectionChanged() const                  { return maLBWindow.IsSelectionChanged(); }
    sal_uInt16          GetSelectModifier() const                   { return maLBWindow.GetSelectModifier(); }

    void            SetMRUEntries( const rtl::OUString& rEntries, sal_Unicode cSep );
    rtl::OUString   GetMRUEntries( sal_Unicode cSep ) const;
    void            SetMaxMRUCount( sal_uInt16 n )                  { maLBWindow.GetEntryList()->SetMaxMRUCount( n ); }
    sal_uInt16          GetMaxMRUCount() const                      { return maLBWindow.GetEntryList()->GetMaxMRUCount(); }
    sal_uInt16          GetDisplayLineCount() const
    { return maLBWindow.GetDisplayLineCount(); }

    // pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()   { maLBWindow.EnableMirroring(); }
    inline void     SetDropTraget(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_xDNDListenerContainer){ mxDNDListenerContainer= i_xDNDListenerContainer; }

    virtual void take_properties(Window &rOther);
};

// -----------------------------
// - ImplListBoxFloatingWindow -
// -----------------------------

class ImplListBoxFloatingWindow : public FloatingWindow
{
private:
    ImplListBox*    mpImplLB;
    Size            maPrefSz;
    sal_uInt16          mnDDLineCount;
    sal_uInt16          mnPopupModeStartSaveSelection;
    sal_Bool            mbAutoWidth;

protected:
    long            PreNotify( NotifyEvent& rNEvt );

public:
                    ImplListBoxFloatingWindow( Window* pParent );

    void            SetImplListBox( ImplListBox* pLB )  { mpImplLB = pLB; }

    void            SetPrefSize( const Size& rSz )      { maPrefSz = rSz; }
    const Size&     GetPrefSize() const                 { return maPrefSz; }

    void            SetAutoWidth( sal_Bool b )              { mbAutoWidth = b; }
    sal_Bool            IsAutoWidth() const                 { return mbAutoWidth; }

    Size            CalcFloatSize();
    void            StartFloat( sal_Bool bStartTracking );

    virtual void    SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { FloatingWindow::SetPosSizePixel( rNewPos, rNewSize ); }

    void            SetDropDownLineCount( sal_uInt16 n ) { mnDDLineCount = n; }
    sal_uInt16          GetDropDownLineCount() const { return mnDDLineCount; }

    sal_uInt16          GetPopupModeStartSaveSelection() const { return mnPopupModeStartSaveSelection; }

    virtual void    Resize();
};

// -----------
// - ImplWin -
// -----------

class ImplWin : public Control
{
private:

    sal_uInt16      mnItemPos;  // because of UserDraw I have to know which item I draw
    XubString       maString;
    Image           maImage;

    Rectangle       maFocusRect;
    Size            maUserItemSize;

    Link            maMBDownHdl;
    Link            maUserDrawHdl;

    sal_Bool        mbUserDrawEnabled   : 1,
                    mbInUserDraw        : 1;


    void ImplDraw( bool bLayout = false );
protected:
    virtual void  FillLayoutData() const;
public:

                    ImplWin( Window* pParent, WinBits nWinStyle = 0 );
                    ~ImplWin() {};

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual long    PreNotify( NotifyEvent& rNEvt );

    sal_uInt16          GetItemPos() const { return mnItemPos; }
    void            SetItemPos( sal_uInt16 n ) { mnItemPos = n; }

    const XubString& GetString() const { return maString; }
    void            SetString( const XubString& rStr ) { maString = rStr; }

    const Image&    GetImage() const { return maImage; }
    void            SetImage( const Image& rImg ) { maImage = rImg; }

    virtual void    MBDown();
    void            SetMBDownHdl( const Link& rLink ) { maMBDownHdl = rLink; }
    const Link&     GetMBDownHdl() const { return maMBDownHdl; }

    void            SetUserDrawHdl( const Link& rLink ) { maUserDrawHdl = rLink; }
    const Link&     GetUserDrawHdl() const              { return maUserDrawHdl; }

    void            SetUserItemSize( const Size& rSz )  { maUserItemSize = rSz; }
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( sal_Bool bUserDraw )    { mbUserDrawEnabled = bUserDraw; }
    sal_Bool            IsUserDrawEnabled() const           { return mbUserDrawEnabled; }

    void            DrawEntry( sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos = sal_False, bool bLayout = false );
};

// -----------
// - ImplBtn -
// -----------

class ImplBtn : public PushButton
{
private:
    sal_Bool            mbDown;

    Link            maMBDownHdl;

public:
                    ImplBtn( Window* pParent, WinBits nWinStyle = 0 );
                    ~ImplBtn() {};

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    virtual void    MBDown();
    void            SetMBDownHdl( const Link& rLink ) { maMBDownHdl = rLink; }
    const Link&     GetMBDownHdl() const { return maMBDownHdl; }
};


void ImplInitFieldSettings( Window* pWin, sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
void ImplInitDropDownButton( PushButton* pButton );

#endif  // _SV_ILSTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
