/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ilstbox.hxx,v $
 * $Revision: 1.3 $
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

#include <vcl/sv.h>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/lstbox.h>
#include <vcl/timer.hxx>


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
    BOOL        mbIsSelected;
    long        mnFlags;

                ImplEntryType( const XubString& rStr, const Image& rImage ) :
                    maStr( rStr ),
                    maImage( rImage ),
                    mnFlags(0)
                {
                    mbIsSelected = FALSE;
                    mpUserData = NULL;
                }

                ImplEntryType( const XubString& rStr ) :
                    maStr( rStr ),
                    mnFlags(0)
                {
                    mbIsSelected = FALSE;
                    mpUserData = NULL;
                }

                ImplEntryType( const Image& rImage ) :
                    maImage( rImage ),
                    mnFlags( 0 )
                {
                    mbIsSelected = FALSE;
                    mpUserData = NULL;
                }
};

// -----------------
// - ImplEntryList -
// -----------------

class ImplEntryList : private List
{
private:
    Window*         mpWindow;   // For getting the current locale when matching strings
    USHORT          mnLastSelected;
    USHORT          mnSelectionAnchor;
    USHORT          mnImages;

    USHORT          mnMRUCount;
    USHORT          mnMaxMRUCount;

    Link            maSelectionChangedHdl;
    BOOL            mbCallSelectionChangedHdl;

    ImplEntryType*  GetEntry( USHORT nPos ) const { return (ImplEntryType*)List::GetObject( nPos ); }

public:
                    ImplEntryList( Window* pWindow );
                    ~ImplEntryList();

    USHORT                  InsertEntry( USHORT nPos, ImplEntryType* pNewEntry, BOOL bSort );
    void                    RemoveEntry( USHORT nPos );
    const ImplEntryType*    GetEntryPtr( USHORT nPos ) const { return (const ImplEntryType*) GetObject( nPos ); }
    void                    Clear();

    USHORT          FindMatchingEntry( const XubString& rStr, USHORT nStart = 0, BOOL bForward = TRUE, BOOL bLazy = TRUE ) const;
    USHORT          FindEntry( const XubString& rStr, BOOL bSearchMRUArea = FALSE ) const;
    USHORT          FindEntry( const void* pData ) const;

    USHORT          GetEntryCount() const { return (USHORT)List::Count(); }
    BOOL            HasImages() const { return mnImages ? TRUE : FALSE; }

    XubString       GetEntryText( USHORT nPos ) const;

    BOOL            HasEntryImage( USHORT nPos ) const;
    Image           GetEntryImage( USHORT nPos ) const;

    void            SetEntryData( USHORT nPos, void* pNewData );
    void*           GetEntryData( USHORT nPos ) const;

    void            SetEntryFlags( USHORT nPos, long nFlags );
    long            GetEntryFlags( USHORT nPos ) const;

    void            SelectEntry( USHORT nPos, BOOL bSelect );

    USHORT          GetSelectEntryCount() const;
    XubString       GetSelectEntry( USHORT nIndex ) const;
    USHORT          GetSelectEntryPos( USHORT nIndex ) const;
    BOOL            IsEntrySelected( const XubString& rStr ) const;
    BOOL            IsEntryPosSelected( USHORT nIndex ) const;

    void            SetLastSelected( USHORT nPos )  { mnLastSelected = nPos; }
    USHORT          GetLastSelected() const { return mnLastSelected; }

    void            SetSelectionAnchor( USHORT nPos )   { mnSelectionAnchor = nPos; }
    USHORT          GetSelectionAnchor() const { return mnSelectionAnchor; }


    void            SetSelectionChangedHdl( const Link& rLnk )  { maSelectionChangedHdl = rLnk; }
    void            SetCallSelectionChangedHdl( BOOL bCall )    { mbCallSelectionChangedHdl = bCall; }

    void            SetMRUCount( USHORT n ) { mnMRUCount = n; }
    USHORT          GetMRUCount() const     { return mnMRUCount; }

    void            SetMaxMRUCount( USHORT n )  { mnMaxMRUCount = n; }
    USHORT          GetMaxMRUCount() const      { return mnMaxMRUCount; }

    /** An Entry is selectable if its mnFlags does not have the
        LISTBOX_ENTRY_FLAG_DISABLE_SELECTION flag set. */
    bool            IsEntrySelectable( USHORT nPos ) const;

    /** returns the first entry found from the given position nPos that is selectable
        or LISTBOX_ENTRY_NOTFOUND if non is found. If the entry at nPos is not selectable,
        it returns the first selectable entry after nPos if bForward is true and the
        first selectable entry after nPos is bForward is false.
        */
    USHORT          FindFirstSelectable( USHORT nPos, bool bForward = true );
};

// ---------------------
// - ImplListBoxWindow -
// ---------------------

class ImplListBoxWindow : public Control
{
private:
    ImplEntryList*  mpEntryList;     // EntryListe
    Rectangle       maFocusRect;
    String          maSearchStr;
    Timer           maSearchTimeout;

    Size            maUserItemSize;

    USHORT          mnMaxTxtHeight;  // Maximale Hoehe eines Text-Items
    USHORT          mnMaxTxtWidth;   // Maximale Breite eines Text-Items
                                     // Entry ohne Image
    USHORT          mnMaxImgTxtWidth;// Maximale Breite eines Text-Items
                                     // Entry UND Image
    USHORT          mnMaxImgWidth;   // Maximale Breite eines Image-Items
    USHORT          mnMaxImgHeight;  // Maximale Hoehe eines Image-Items
    USHORT          mnMaxWidth;      // Maximale Breite eines Eintrags
    USHORT          mnMaxHeight;     // Maximale Hoehe eines Eintrags

    USHORT          mnMaxVisibleEntries; // Anzahl der sichtbaren Eintraege
    USHORT          mnCurrentPos;    // Position (Focus)
    USHORT          mnTrackingSaveSelection; // Selektion vor Tracking();

    USHORT          mnSeparatorPos; // Separator

    USHORT          mnUserDrawEntry;

    USHORT          mnTop;           // Ausgabe ab Zeile
    USHORT          mnLeft;          // Ausgabe ab Spalte
    USHORT          mnBorder;        // Abstand Rahmen - Text
    USHORT          mnTextHeight;    // Texthoehe

    USHORT          mnSelectModifier;   // Modifiers

    BOOL            mbHasFocusRect:         1,
                    mbSort:                 1,  // ListBox sortiert
                    mbTrack:                1,  // Tracking
                    mbMulti:                1,  // MultiListBox
                    mbStackMode:            1,  // StackSelection
                    mbSimpleMode:           1,  // SimpleMode fuer MultiListBox
                    mbImgsDiffSz:           1,  // Images haben verschiedene Groessen
                    mbTravelSelect:         1,  // TravelSelect
                    mbTrackingSelect:       1,  // Selektiert bei MouseMove
                    mbSelectionChanged:     1,  // Select() nicht zu oft rufen...
                    mbMouseMoveSelect:      1,  // Selektieren bei MouseMove
                    mbGrabFocus:            1,  // Focus bei MBDown grabben
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

protected:
    DECL_LINK(      SearchStringTimeout, Timer* );

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();

    BOOL            SelectEntries( USHORT nSelect, LB_EVENT_TYPE eLET, BOOL bShift = FALSE, BOOL bCtrl = FALSE );
    void            ImplPaint( USHORT nPos, BOOL bErase = FALSE, bool bLayout = false );
    void            ImplDoPaint( const Rectangle& rRect, bool bLayout = false );
    void            ImplCalcMetrics();
    void            ImplCalcEntryMetrics( const ImplEntryType& rEntry, BOOL bUpdateMetrics );
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

    USHORT          InsertEntry( USHORT nPos, ImplEntryType* pNewEntry );
    void            RemoveEntry( USHORT nPos );
    void            Clear();
    void            ResetCurrentPos()               { mnCurrentPos = LISTBOX_ENTRY_NOTFOUND; }
    USHORT          GetCurrentPos() const           { return mnCurrentPos; }
    USHORT          GetDisplayLineCount() const;

    void            DrawEntry( USHORT nPos, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos = FALSE, bool bLayout = false );

    void            SelectEntry( USHORT nPos, BOOL bSelect );
    void            DeselectAll();
    USHORT          GetEntryPosForPoint( const Point& rPoint ) const;

    BOOL            ProcessKeyInput( const KeyEvent& rKEvt );

    void            SetTopEntry( USHORT nTop );
    USHORT          GetTopEntry() const             { return mnTop; }
    using Window::IsVisible;
    BOOL            IsVisible( USHORT nEntry ) const { return ( ( nEntry >= mnTop ) && ( nEntry < (mnTop+mnMaxVisibleEntries) ) ); }

    USHORT          GetLeftIndent() const           { return mnLeft; }
    void            SetLeftIndent( USHORT n );
    void            ScrollHorz( short nDiff );

    void            AllowGrabFocus( BOOL b )        { mbGrabFocus = b; }
    BOOL            IsGrabFocusAllowed() const      { return mbGrabFocus; }

    void            SetSeparatorPos( USHORT n )     { mnSeparatorPos = n; }
    USHORT          GetSeparatorPos() const         { return mnSeparatorPos; }

    void            SetTravelSelect( BOOL bTravelSelect ) { mbTravelSelect = bTravelSelect; }
    BOOL            IsTravelSelect() const          { return mbTravelSelect; }
    BOOL            IsTrackingSelect() const            { return mbTrackingSelect; }

    void            SetUserItemSize( const Size& rSz );
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( BOOL bUserDraw ) { mbUserDrawEnabled = bUserDraw; }
    BOOL            IsUserDrawEnabled() const   { return mbUserDrawEnabled; }

    void            EnableMultiSelection( BOOL bMulti, BOOL bStackMode ) { mbMulti = bMulti; mbStackMode = bStackMode; }
    BOOL            IsMultiSelectionEnabled() const     { return mbMulti; }

    void            SetMultiSelectionSimpleMode( BOOL bSimple ) { mbSimpleMode = bSimple; }
    BOOL            IsMultiSelectionSimpleMode() const          { return mbSimpleMode; }

    void            EnableMouseMoveSelect( BOOL bMouseMoveSelect ) { mbMouseMoveSelect = bMouseMoveSelect; }
    BOOL            IsMouseMoveSelectEnabled() const    { return mbMouseMoveSelect; }
    BOOL            IsMouseMoveSelect() const   { return mbMouseMoveSelect||mbStackMode; }

    Size            CalcSize( USHORT nMaxLines ) const;
    void            CalcMaxVisibleEntries( const Size& rFloatSize);
    Rectangle       GetBoundingRectangle( USHORT nItem ) const;

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

    BOOL            IsSelectionChanged() const { return mbSelectionChanged; }
    USHORT          GetSelectModifier() const { return mnSelectModifier; }

    void            EnableSort( BOOL b ) { mbSort = b; }

    void            SetReadOnly( BOOL bReadOnly )   { mbReadOnly = bReadOnly; }
    BOOL            IsReadOnly() const              { return mbReadOnly; }

    void            ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    USHORT          ImplGetTextStyle() const;

    // pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()       { mbMirroring = TRUE; }
    inline BOOL     IsMirroring() const { return mbMirroring; }
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
    BOOL                mbVScroll       : 1,    // VScroll an oder aus
                        mbHScroll       : 1,    // HScroll an oder aus
                        mbAutoHScroll   : 1;    // AutoHScroll an oder aus
    Link                maScrollHdl;    // Weil der vom ImplListBoxWindow selbst benoetigt wird.

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

    USHORT          InsertEntry( USHORT nPos, const XubString& rStr );
    USHORT          InsertEntry( USHORT nPos, const Image& rImage );
    USHORT          InsertEntry( USHORT nPos, const XubString& rStr, const Image& rImage );
    void            RemoveEntry( USHORT nPos );
    void            SetEntryData( USHORT nPos, void* pNewData ) { maLBWindow.GetEntryList()->SetEntryData( nPos, pNewData ); }
    void            Clear();

    void            SetEntryFlags( USHORT nPos, long nFlags );
    long            GetEntryFlags( USHORT nPos ) const;

    void            SelectEntry( USHORT nPos, BOOL bSelect );
    void            SetNoSelection();
    void            ResetCurrentPos()               { maLBWindow.ResetCurrentPos(); }
    USHORT          GetCurrentPos() const           { return maLBWindow.GetCurrentPos(); }

    BOOL            ProcessKeyInput( const KeyEvent& rKEvt )    { return maLBWindow.ProcessKeyInput( rKEvt ); }
    BOOL            HandleWheelAsCursorTravel( const CommandEvent& rCEvt );

    void            SetSeparatorPos( USHORT n )     { maLBWindow.SetSeparatorPos( n ); }
    USHORT          GetSeparatorPos() const         { return maLBWindow.GetSeparatorPos(); }

    void            SetTopEntry( USHORT nTop )      { maLBWindow.SetTopEntry( nTop ); }
    USHORT          GetTopEntry() const             { return maLBWindow.GetTopEntry(); }
    using Window::IsVisible;
    BOOL            IsVisible( USHORT nEntry ) const { return maLBWindow.IsVisible( nEntry ); }

    USHORT          GetLeftIndent() const           { return maLBWindow.GetLeftIndent(); }
    void            SetLeftIndent( USHORT n )       { maLBWindow.SetLeftIndent( n ); }
    void            ScrollHorz( short nDiff )       { maLBWindow.ScrollHorz( nDiff ); }

    void            SetTravelSelect( BOOL bTravelSelect ) { maLBWindow.SetTravelSelect( bTravelSelect ); }
    BOOL            IsTravelSelect() const          { return maLBWindow.IsTravelSelect(); }
    BOOL            IsTrackingSelect() const            { return maLBWindow.IsTrackingSelect(); }

    void            EnableMultiSelection( BOOL bMulti, BOOL bStackMode ) { maLBWindow.EnableMultiSelection( bMulti, bStackMode ); }
    BOOL            IsMultiSelectionEnabled() const     { return maLBWindow.IsMultiSelectionEnabled(); }

    void            SetMultiSelectionSimpleMode( BOOL bSimple ) { maLBWindow.SetMultiSelectionSimpleMode( bSimple ); }
    BOOL            IsMultiSelectionSimpleMode() const  { return maLBWindow.IsMultiSelectionSimpleMode(); }

    void            SetReadOnly( BOOL b )           { maLBWindow.SetReadOnly( b ); }
    BOOL            IsReadOnly() const              { return maLBWindow.IsReadOnly(); }


    Size            CalcSize( USHORT nMaxLines ) const              { return maLBWindow.CalcSize( nMaxLines ); }
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
    void            SetCallSelectionChangedHdl( BOOL bCall )    { maLBWindow.GetEntryList()->SetCallSelectionChangedHdl( bCall ); }
    BOOL            IsSelectionChanged() const                  { return maLBWindow.IsSelectionChanged(); }
    USHORT          GetSelectModifier() const                   { return maLBWindow.GetSelectModifier(); }

    void            SetMRUEntries( const XubString& rEntries, xub_Unicode cSep );
    XubString       GetMRUEntries( xub_Unicode cSep ) const;
    void            SetMaxMRUCount( USHORT n )                  { maLBWindow.GetEntryList()->SetMaxMRUCount( n ); }
    USHORT          GetMaxMRUCount() const                      { return maLBWindow.GetEntryList()->GetMaxMRUCount(); }
    USHORT          GetDisplayLineCount() const
    { return maLBWindow.GetDisplayLineCount(); }

    // pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()   { maLBWindow.EnableMirroring(); }
};

// -----------------------------
// - ImplListBoxFloatingWindow -
// -----------------------------

class ImplListBoxFloatingWindow : public FloatingWindow
{
private:
    ImplListBox*    mpImplLB;
    Size            maPrefSz;
    USHORT          mnDDLineCount;
    USHORT          mnPopupModeStartSaveSelection;
    BOOL            mbAutoWidth;

protected:
    long            PreNotify( NotifyEvent& rNEvt );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

public:
                    ImplListBoxFloatingWindow( Window* pParent );

    void            SetImplListBox( ImplListBox* pLB )  { mpImplLB = pLB; }

    void            SetPrefSize( const Size& rSz )      { maPrefSz = rSz; }
    const Size&     GetPrefSize() const                 { return maPrefSz; }

    void            SetAutoWidth( BOOL b )              { mbAutoWidth = b; }
    BOOL            IsAutoWidth() const                 { return mbAutoWidth; }

    Size            CalcFloatSize();
    void            StartFloat( BOOL bStartTracking );

    virtual void    SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight, USHORT nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
                        { FloatingWindow::SetPosSizePixel( rNewPos, rNewSize ); }

    void            SetDropDownLineCount( USHORT n ) { mnDDLineCount = n; }
    USHORT          GetDropDownLineCount() const { return mnDDLineCount; }

    USHORT          GetPopupModeStartSaveSelection() const { return mnPopupModeStartSaveSelection; }

    virtual void    Resize();
};

// -----------
// - ImplWin -
// -----------

class ImplWin : public Control
{
private:

    USHORT          mnItemPos;  // wegen UserDraw muss ich wissen, welches Item ich darstelle.
    XubString       maString;
    Image           maImage;
    Image           maImageHC;

    Rectangle       maFocusRect;
    Size            maUserItemSize;

    Link            maMBDownHdl;
    Link            maUserDrawHdl;

    BOOL            mbUserDrawEnabled   : 1,
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

    USHORT          GetItemPos() const { return mnItemPos; }
    void            SetItemPos( USHORT n ) { mnItemPos = n; }

    const XubString& GetString() const { return maString; }
    void            SetString( const XubString& rStr ) { maString = rStr; }

    const Image&    GetImage() const { return maImage; }
    void            SetImage( const Image& rImg ) { maImage = rImg; }

    BOOL            SetModeImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image&    GetModeImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;


    virtual void    MBDown();
    void            SetMBDownHdl( const Link& rLink ) { maMBDownHdl = rLink; }
    const Link&     GetMBDownHdl() const { return maMBDownHdl; }

    void            SetUserDrawHdl( const Link& rLink ) { maUserDrawHdl = rLink; }
    const Link&     GetUserDrawHdl() const              { return maUserDrawHdl; }

    void            SetUserItemSize( const Size& rSz )  { maUserItemSize = rSz; }
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( BOOL bUserDraw )    { mbUserDrawEnabled = bUserDraw; }
    BOOL            IsUserDrawEnabled() const           { return mbUserDrawEnabled; }

    void            DrawEntry( BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos = FALSE, bool bLayout = false );
};

// -----------
// - ImplBtn -
// -----------

class ImplBtn : public PushButton
{
private:
    BOOL            mbDown;

    Link            maMBDownHdl;

public:
                    ImplBtn( Window* pParent, WinBits nWinStyle = 0 );
                    ~ImplBtn() {};

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    virtual void    MBDown();
    void            SetMBDownHdl( const Link& rLink ) { maMBDownHdl = rLink; }
    const Link&     GetMBDownHdl() const { return maMBDownHdl; }
};


void ImplInitFieldSettings( Window* pWin, BOOL bFont, BOOL bForeground, BOOL bBackground );
void ImplInitDropDownButton( PushButton* pButton );

#endif  // _SV_ILSTBOX_HXX
