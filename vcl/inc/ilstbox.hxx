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

#ifndef INCLUDED_VCL_INC_ILSTBOX_HXX
#define INCLUDED_VCL_INC_ILSTBOX_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/signals2/signal.hpp>
#include <vcl/image.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/lstbox.h>
#include <vcl/timer.hxx>

#include "vcl/quickselectionengine.hxx"

class ScrollBar;
class ScrollBarBox;

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

struct ImplEntryType
{
    OUString    maStr;
    Image       maImage;
    void*       mpUserData;
    bool    mbIsSelected;
    long        mnFlags;
    long        mnHeight;

    ImplEntryType( const OUString& rStr, const Image& rImage ) :
        maStr( rStr ),
        maImage( rImage ),
        mnFlags( 0 ),
        mnHeight( 0 )
    {
        mbIsSelected = false;
        mpUserData = NULL;
    }

    ImplEntryType( const OUString& rStr ) :
        maStr( rStr ),
        mnFlags( 0 ),
        mnHeight( 0 )
    {
        mbIsSelected = false;
        mpUserData = NULL;
    }

    ImplEntryType( const Image& rImage ) :
        maImage( rImage ),
        mnFlags( 0 ),
        mnHeight( 0 )
    {
        mbIsSelected = false;
        mpUserData = NULL;
    }
};

class ImplEntryList
{
private:
    vcl::Window*         mpWindow;   ///< For getting the current locale when matching strings
    sal_Int32       mnLastSelected;
    sal_Int32       mnSelectionAnchor;
    sal_Int32       mnImages;

    sal_Int32       mnMRUCount;
    sal_Int32       mnMaxMRUCount;

    Link            maSelectionChangedHdl;
    bool            mbCallSelectionChangedHdl;
    boost::ptr_vector<ImplEntryType> maEntries;

    ImplEntryType*  GetEntry( sal_Int32  nPos ) const
    {
        if (nPos < 0 || static_cast<size_t>(nPos) >= maEntries.size())
            return NULL;
        return const_cast<ImplEntryType*>(&maEntries[nPos]);
    }

public:
                    ImplEntryList( vcl::Window* pWindow );
                    ~ImplEntryList();

    sal_Int32               InsertEntry( sal_Int32  nPos, ImplEntryType* pNewEntry, bool bSort );
    void                    RemoveEntry( sal_Int32  nPos );
    const ImplEntryType*    GetEntryPtr( sal_Int32  nPos ) const { return (const ImplEntryType*) GetEntry( nPos ); }
    ImplEntryType*          GetMutableEntryPtr( sal_Int32  nPos ) const { return GetEntry( nPos ); }
    void                    Clear();

    sal_Int32           FindMatchingEntry( const OUString& rStr, sal_Int32  nStart = 0, bool bForward = true, bool bLazy = true ) const;
    sal_Int32           FindEntry( const OUString& rStr, bool bSearchMRUArea = false ) const;
    sal_Int32           FindEntry( const void* pData ) const;

    /// helper: add up heights up to index nEndIndex.
    /// GetAddedHeight( 0 ) @return 0
    /// GetAddedHeight( LISTBOX_ENTRY_NOTFOUND ) @return 0
    /// GetAddedHeight( i, k ) with k > i is equivalent -GetAddedHeight( k, i )
    long            GetAddedHeight( sal_Int32  nEndIndex, sal_Int32  nBeginIndex = 0, long nBeginHeight = 0 ) const;
    long            GetEntryHeight( sal_Int32  nPos ) const;

    sal_Int32       GetEntryCount() const { return (sal_Int32 )maEntries.size(); }
    bool            HasImages() const { return mnImages ? true : false; }

    OUString        GetEntryText( sal_Int32  nPos ) const;

    bool            HasEntryImage( sal_Int32  nPos ) const;
    Image           GetEntryImage( sal_Int32  nPos ) const;

    void            SetEntryData( sal_Int32  nPos, void* pNewData );
    void*           GetEntryData( sal_Int32  nPos ) const;

    void            SetEntryFlags( sal_Int32  nPos, long nFlags );
    long            GetEntryFlags( sal_Int32  nPos ) const;

    void            SelectEntry( sal_Int32  nPos, bool bSelect );

    sal_Int32       GetSelectEntryCount() const;
    OUString        GetSelectEntry( sal_Int32  nIndex ) const;
    sal_Int32       GetSelectEntryPos( sal_Int32  nIndex ) const;
    bool            IsEntryPosSelected( sal_Int32  nIndex ) const;

    void            SetLastSelected( sal_Int32  nPos )  { mnLastSelected = nPos; }
    sal_Int32       GetLastSelected() const { return mnLastSelected; }

    void            SetSelectionAnchor( sal_Int32  nPos )   { mnSelectionAnchor = nPos; }
    sal_Int32       GetSelectionAnchor() const { return mnSelectionAnchor; }

    void            SetSelectionChangedHdl( const Link& rLnk )  { maSelectionChangedHdl = rLnk; }
    void            SetCallSelectionChangedHdl( bool bCall )    { mbCallSelectionChangedHdl = bCall; }

    void            SetMRUCount( sal_Int32  n ) { mnMRUCount = n; }
    sal_Int32       GetMRUCount() const     { return mnMRUCount; }

    void            SetMaxMRUCount( sal_Int32  n )  { mnMaxMRUCount = n; }
    sal_Int32       GetMaxMRUCount() const      { return mnMaxMRUCount; }

    /** An Entry is selectable if its mnFlags does not have the
        LISTBOX_ENTRY_FLAG_DISABLE_SELECTION flag set. */
    bool            IsEntrySelectable( sal_Int32  nPos ) const;

    /** @return the first entry found from the given position nPos that is selectable
        or LISTBOX_ENTRY_NOTFOUND if non is found. If the entry at nPos is not selectable,
        it returns the first selectable entry after nPos if bForward is true and the
        first selectable entry after nPos is bForward is false.
        */
    sal_Int32       FindFirstSelectable( sal_Int32  nPos, bool bForward = true );
};

class ImplListBoxWindow : public Control, public ::vcl::ISearchableStringList
{
private:
    ImplEntryList*  mpEntryList;     ///< EntryList
    Rectangle       maFocusRect;

    Size            maUserItemSize;

    long            mnMaxTxtHeight;  ///< Maximum height of a text item
    long            mnMaxTxtWidth;   ///< Maximum width of a text item
                                     ///< Entry without Image
    long            mnMaxImgTxtWidth;///< Maximum width of a text item
                                     ///< Entry AND Image
    long            mnMaxImgWidth;   ///< Maximum width of an image item
    long            mnMaxImgHeight;  ///< Maximum height of an image item
    long            mnMaxWidth;      ///< Maximum width of an entry
    long            mnMaxHeight;     ///< Maximum height of an entry

    sal_Int32       mnCurrentPos;    ///< Position (Focus)
    sal_Int32       mnTrackingSaveSelection; ///< Selection before Tracking();

    sal_Int32       mnSeparatorPos; ///< Separator

    sal_Int32       mnUserDrawEntry;

    sal_Int32       mnTop;           ///< output from line on
    long            mnLeft;          ///< output from column on
    long            mnBorder;        ///< distance border - text
    long            mnTextHeight;    ///< text height
    ProminentEntry  meProminentType; ///< where is the "prominent" entry

    sal_uInt16      mnSelectModifier;   ///< Modifiers

    /// bitfield
    bool mbHasFocusRect : 1;
    bool mbSort : 1;             ///< ListBox sorted
    bool mbTrack : 1;            ///< Tracking
    bool mbMulti : 1;            ///< MultiListBox
    bool mbStackMode : 1;        ///< StackSelection
    bool mbSimpleMode : 1;       ///< SimpleMode for MultiListBox
    bool mbImgsDiffSz : 1;       ///< Images have different sizes
    bool mbTravelSelect : 1;     ///< TravelSelect
    bool mbTrackingSelect : 1;   ///< Selected at a MouseMove
    bool mbSelectionChanged : 1; ///< Do not call Select() too often ...
    bool mbMouseMoveSelect : 1;  ///< Select at MouseMove
    bool mbGrabFocus : 1;        ///< Grab focus at MBDown
    bool mbUserDrawEnabled : 1;  ///< UserDraw possible
    bool mbInUserDraw : 1;       ///< In UserDraw
    bool mbReadOnly : 1;         ///< ReadOnly
    bool mbMirroring : 1;        ///< pb: #106948# explicit mirroring for calc
    bool mbRight : 1;            ///< right align Text output
    bool mbCenter : 1;           ///< center Text output
    bool mbEdgeBlending : 1;

    Link            maScrollHdl;
    Link            maSelectHdl;
    Link            maCancelHdl;
    Link            maDoubleClickHdl;
    Link            maMRUChangedHdl;
    Link            maFocusHdl;
    Link            maListItemSelectHdl;

    ::vcl::QuickSelectionEngine maQuickSelectionEngine;

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;

    bool            SelectEntries( sal_Int32  nSelect, LB_EVENT_TYPE eLET, bool bShift = false, bool bCtrl = false, bool bSelectPosChange = false );
    void            ImplPaint( sal_Int32  nPos, bool bErase = false, bool bLayout = false );
    void            ImplDoPaint( const Rectangle& rRect, bool bLayout = false );
    void            ImplCalcMetrics();
    void            ImplUpdateEntryMetrics( ImplEntryType& rEntry );
    void            ImplCallSelect();

    void            ImplShowFocusRect();
    void            ImplHideFocusRect();

    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
    virtual void  FillLayoutData() const SAL_OVERRIDE;

                    ImplListBoxWindow( vcl::Window* pParent, WinBits nWinStyle );
                    virtual ~ImplListBoxWindow();

    ImplEntryList*  GetEntryList() const { return mpEntryList; }

    sal_Int32       InsertEntry( sal_Int32  nPos, ImplEntryType* pNewEntry );
    void            RemoveEntry( sal_Int32  nPos );
    void            Clear();
    void            ResetCurrentPos()               { mnCurrentPos = LISTBOX_ENTRY_NOTFOUND; }
    sal_Int32       GetCurrentPos() const           { return mnCurrentPos; }
    sal_uInt16      GetDisplayLineCount() const;
    void            SetEntryFlags( sal_Int32  nPos, long nFlags );

    void            DrawEntry( sal_Int32  nPos, bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos = false, bool bLayout = false );

    void            SelectEntry( sal_Int32  nPos, bool bSelect );
    void            DeselectAll();
    sal_Int32       GetEntryPosForPoint( const Point& rPoint ) const;
    sal_Int32       GetLastVisibleEntry() const;

    bool            ProcessKeyInput( const KeyEvent& rKEvt );

    void            SetTopEntry( sal_Int32  nTop );
    sal_Int32       GetTopEntry() const             { return mnTop; }
    /** ShowProminentEntry will set the entry correspoding to nEntryPos
        either at top or in the middle depending on the chosen style*/
    void            ShowProminentEntry( sal_Int32  nEntryPos );
    void            SetProminentEntryType( ProminentEntry eType ) { meProminentType = eType; }
    ProminentEntry  GetProminentEntryType() const { return meProminentType; }
    using Window::IsVisible;
    bool            IsVisible( sal_Int32  nEntry ) const;

    long            GetLeftIndent() const           { return mnLeft; }
    void            SetLeftIndent( long n );
    void            ScrollHorz( long nDiff );

    void            AllowGrabFocus( bool b )        { mbGrabFocus = b; }
    bool            IsGrabFocusAllowed() const      { return mbGrabFocus; }

    void            SetSeparatorPos( sal_Int32  n )     { mnSeparatorPos = n; }
    sal_Int32       GetSeparatorPos() const         { return mnSeparatorPos; }

    void            SetTravelSelect( bool bTravelSelect ) { mbTravelSelect = bTravelSelect; }
    bool            IsTravelSelect() const          { return mbTravelSelect; }
    bool            IsTrackingSelect() const        { return mbTrackingSelect; }

    void            SetUserItemSize( const Size& rSz );
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( bool bUserDraw ) { mbUserDrawEnabled = bUserDraw; }
    bool            IsUserDrawEnabled() const   { return mbUserDrawEnabled; }

    void            EnableMultiSelection( bool bMulti, bool bStackMode ) { mbMulti = bMulti; mbStackMode = bStackMode; }
    bool            IsMultiSelectionEnabled() const     { return mbMulti; }

    void            SetMultiSelectionSimpleMode( bool bSimple ) { mbSimpleMode = bSimple; }
    bool            IsMultiSelectionSimpleMode() const          { return mbSimpleMode; }

    void            EnableMouseMoveSelect( bool bMouseMoveSelect ) { mbMouseMoveSelect = bMouseMoveSelect; }
    bool            IsMouseMoveSelectEnabled() const    { return mbMouseMoveSelect; }
    bool            IsMouseMoveSelect() const   { return mbMouseMoveSelect||mbStackMode; }

    Size            CalcSize(sal_Int32 nMaxLines) const;
    Rectangle       GetBoundingRectangle( sal_Int32  nItem ) const;

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
    void            SetMRUChangedHdl( const Link& rLink )   { maMRUChangedHdl = rLink; }
    const Link&     GetMRUChangedHdl() const                { return maMRUChangedHdl; }
    void            SetFocusHdl( const Link& rLink )    { maFocusHdl = rLink ; }
    const Link&     GetFocusHdl() const             { return maFocusHdl; }

    boost::signals2::signal< void ( UserDrawEvent* ) > userDrawSignal;

    void            SetListItemSelectHdl( const Link& rLink )   { maListItemSelectHdl = rLink ; }
    const Link&     GetListItemSelectHdl() const                { return maListItemSelectHdl; }
    bool            IsSelectionChanged() const { return mbSelectionChanged; }
    sal_uInt16      GetSelectModifier() const { return mnSelectModifier; }

    void            EnableSort( bool b ) { mbSort = b; }

    void            SetReadOnly( bool bReadOnly )   { mbReadOnly = bReadOnly; }
    bool            IsReadOnly() const              { return mbReadOnly; }

    using Control::ImplInitSettings;
    void            ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    sal_uInt16      ImplGetTextStyle() const;

    /// pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()       { mbMirroring = true; }
    inline bool     IsMirroring() const { return mbMirroring; }

    bool GetEdgeBlending() const { return mbEdgeBlending; }
    void SetEdgeBlending(bool bNew) { mbEdgeBlending = bNew; }
    void EnableQuickSelection( const bool& b );

protected:
    // ISearchableStringList
    virtual ::vcl::StringEntryIdentifier    CurrentEntry( OUString& _out_entryText ) const SAL_OVERRIDE;
    virtual ::vcl::StringEntryIdentifier    NextEntry( ::vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const SAL_OVERRIDE;
    virtual void                            SelectEntry( ::vcl::StringEntryIdentifier _entry ) SAL_OVERRIDE;
};

class ImplListBox : public Control
{
private:
    ImplListBoxWindow   maLBWindow;
    ScrollBar*          mpHScrollBar;
    ScrollBar*          mpVScrollBar;
    ScrollBarBox*       mpScrollBarBox;

    /// bitfield
    bool mbVScroll : 1;     // VScroll an oder aus
    bool mbHScroll : 1;     // HScroll an oder aus
    bool mbAutoHScroll : 1; // AutoHScroll an oder aus
    bool mbEdgeBlending : 1;

    Link                maScrollHdl;    // because it is needed by ImplListBoxWindow itself
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDNDListenerContainer;

protected:
    virtual void        GetFocus() SAL_OVERRIDE;
    virtual void        StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual bool        Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void                ImplResizeControls();
    void                ImplCheckScrollBars();
    void                ImplInitScrollBars();

    DECL_LINK(          ScrollBarHdl, ScrollBar* );
    DECL_LINK(          LBWindowScrolled, void* );
    DECL_LINK(          MRUChanged, void* );

public:
                    ImplListBox( vcl::Window* pParent, WinBits nWinStyle );
                    virtual ~ImplListBox();

    const ImplEntryList*    GetEntryList() const            { return maLBWindow.GetEntryList(); }
    ImplListBoxWindow&      GetMainWindow()                 { return maLBWindow; }

    virtual void    Resize() SAL_OVERRIDE;
    virtual const Wallpaper& GetDisplayBackground() const SAL_OVERRIDE;
    virtual vcl::Window*     GetPreferredKeyInputWindow() SAL_OVERRIDE;

    sal_Int32       InsertEntry( sal_Int32  nPos, const OUString& rStr );
    sal_Int32       InsertEntry( sal_Int32  nPos, const OUString& rStr, const Image& rImage );
    void            RemoveEntry( sal_Int32  nPos );
    void            SetEntryData( sal_Int32  nPos, void* pNewData ) { maLBWindow.GetEntryList()->SetEntryData( nPos, pNewData ); }
    void            Clear();

    void            SetEntryFlags( sal_Int32  nPos, long nFlags );

    void            SelectEntry( sal_Int32  nPos, bool bSelect );
    void            SetNoSelection();
    void            ResetCurrentPos()               { maLBWindow.ResetCurrentPos(); }
    sal_Int32       GetCurrentPos() const           { return maLBWindow.GetCurrentPos(); }

    bool            ProcessKeyInput( const KeyEvent& rKEvt )    { return maLBWindow.ProcessKeyInput( rKEvt ); }
    bool            HandleWheelAsCursorTravel( const CommandEvent& rCEvt );

    void            SetSeparatorPos( sal_Int32  n )     { maLBWindow.SetSeparatorPos( n ); }
    sal_Int32       GetSeparatorPos() const         { return maLBWindow.GetSeparatorPos(); }

    void            SetTopEntry( sal_Int32  nTop )      { maLBWindow.SetTopEntry( nTop ); }
    sal_Int32       GetTopEntry() const             { return maLBWindow.GetTopEntry(); }
    void            ShowProminentEntry( sal_Int32  nPos ) { maLBWindow.ShowProminentEntry( nPos ); }
    using Window::IsVisible;
    bool            IsVisible( sal_Int32  nEntry ) const { return maLBWindow.IsVisible( nEntry ); }

    void            SetProminentEntryType( ProminentEntry eType ) { maLBWindow.SetProminentEntryType( eType ); }
    ProminentEntry  GetProminentEntryType() const { return maLBWindow.GetProminentEntryType(); }

    long            GetLeftIndent() const           { return maLBWindow.GetLeftIndent(); }
    void            SetLeftIndent( sal_uInt16 n )       { maLBWindow.SetLeftIndent( n ); }
    void            ScrollHorz( short nDiff )       { maLBWindow.ScrollHorz( nDiff ); }

    void            SetTravelSelect( bool bTravelSelect ) { maLBWindow.SetTravelSelect( bTravelSelect ); }
    bool            IsTravelSelect() const          { return maLBWindow.IsTravelSelect(); }
    bool            IsTrackingSelect() const            { return maLBWindow.IsTrackingSelect(); }

    void            EnableMultiSelection( bool bMulti, bool bStackMode ) { maLBWindow.EnableMultiSelection( bMulti, bStackMode ); }
    bool            IsMultiSelectionEnabled() const     { return maLBWindow.IsMultiSelectionEnabled(); }

    void            SetMultiSelectionSimpleMode( bool bSimple ) { maLBWindow.SetMultiSelectionSimpleMode( bSimple ); }
    bool            IsMultiSelectionSimpleMode() const  { return maLBWindow.IsMultiSelectionSimpleMode(); }

    void            SetReadOnly( bool b )           { maLBWindow.SetReadOnly( b ); }
    bool            IsReadOnly() const              { return maLBWindow.IsReadOnly(); }

    Size            CalcSize( sal_Int32  nMaxLines ) const              { return maLBWindow.CalcSize( nMaxLines ); }
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

    boost::signals2::signal< void ( UserDrawEvent* ) > userDrawSignal;

    void            SetFocusHdl( const Link& rLink )    { maLBWindow.SetFocusHdl( rLink ); }
    const Link&     GetFocusHdl() const             { return maLBWindow.GetFocusHdl(); }
    void            SetListItemSelectHdl( const Link& rLink )   { maLBWindow.SetListItemSelectHdl( rLink ); }
    const Link&     GetListItemSelectHdl() const    { return maLBWindow.GetListItemSelectHdl(); }
    void            SetSelectionChangedHdl( const Link& rLnk )  { maLBWindow.GetEntryList()->SetSelectionChangedHdl( rLnk ); }
    void            SetCallSelectionChangedHdl( bool bCall )    { maLBWindow.GetEntryList()->SetCallSelectionChangedHdl( bCall ); }
    bool            IsSelectionChanged() const                  { return maLBWindow.IsSelectionChanged(); }
    sal_uInt16      GetSelectModifier() const                   { return maLBWindow.GetSelectModifier(); }

    void            SetMRUEntries( const OUString& rEntries, sal_Unicode cSep );
    OUString   GetMRUEntries( sal_Unicode cSep ) const;
    void            SetMaxMRUCount( sal_Int32  n )                  { maLBWindow.GetEntryList()->SetMaxMRUCount( n ); }
    sal_Int32       GetMaxMRUCount() const                      { return maLBWindow.GetEntryList()->GetMaxMRUCount(); }
    sal_uInt16      GetDisplayLineCount() const
    { return maLBWindow.GetDisplayLineCount(); }

    bool GetEdgeBlending() const { return mbEdgeBlending; }
    void SetEdgeBlending(bool bNew);

    /// pb: #106948# explicit mirroring for calc
    inline void     EnableMirroring()   { maLBWindow.EnableMirroring(); }
    inline void     SetDropTraget(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_xDNDListenerContainer){ mxDNDListenerContainer= i_xDNDListenerContainer; }
};

class ImplListBoxFloatingWindow : public FloatingWindow
{
private:
    ImplListBox*    mpImplLB;
    Size            maPrefSz;
    sal_uInt16      mnDDLineCount;
    sal_Int32       mnPopupModeStartSaveSelection;
    bool            mbAutoWidth;

protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
                    ImplListBoxFloatingWindow( vcl::Window* pParent );

    void            SetImplListBox( ImplListBox* pLB )  { mpImplLB = pLB; }

    void            SetPrefSize( const Size& rSz )      { maPrefSz = rSz; }
    const Size&     GetPrefSize() const                 { return maPrefSz; }

    void            SetAutoWidth( bool b )              { mbAutoWidth = b; }
    bool            IsAutoWidth() const                 { return mbAutoWidth; }

    Size            CalcFloatSize();
    void            StartFloat( bool bStartTracking );

    virtual void    setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL ) SAL_OVERRIDE;
    void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) SAL_OVERRIDE
                        { FloatingWindow::SetPosSizePixel( rNewPos, rNewSize ); }

    void            SetDropDownLineCount( sal_uInt16 n ) { mnDDLineCount = n; }
    sal_uInt16      GetDropDownLineCount() const { return mnDDLineCount; }

    sal_Int32       GetPopupModeStartSaveSelection() const { return mnPopupModeStartSaveSelection; }

    virtual void    Resize() SAL_OVERRIDE;
};

class ImplWin : public Control
{
private:

    sal_Int32       mnItemPos;  ///< because of UserDraw I have to know which item I draw
    OUString        maString;
    Image           maImage;

    Rectangle       maFocusRect;
    Size            maUserItemSize;

    /// bitfield
    bool            mbUserDrawEnabled : 1;
    bool            mbInUserDraw : 1;
    bool            mbEdgeBlending : 1;

    void ImplDraw( bool bLayout = false );
protected:
    virtual void  FillLayoutData() const SAL_OVERRIDE;
public:

                    ImplWin( vcl::Window* pParent, WinBits nWinStyle = 0 );
                    virtual ~ImplWin() {};

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    sal_Int32       GetItemPos() const { return mnItemPos; }
    void            SetItemPos( sal_Int32  n ) { mnItemPos = n; }

    const OUString& GetString() const { return maString; }
    void            SetString( const OUString& rStr ) { maString = rStr; }

    const Image&    GetImage() const { return maImage; }
    void            SetImage( const Image& rImg ) { maImage = rImg; }

    virtual void    MBDown();

    boost::signals2::signal< void ( ImplWin* ) > buttonDownSignal;
    boost::signals2::signal< void ( UserDrawEvent* ) > userDrawSignal;

    void            SetUserItemSize( const Size& rSz )  { maUserItemSize = rSz; }
    const Size&     GetUserItemSize() const             { return maUserItemSize; }

    void            EnableUserDraw( bool bUserDraw )    { mbUserDrawEnabled = bUserDraw; }
    bool            IsUserDrawEnabled() const           { return mbUserDrawEnabled; }

    void            DrawEntry( bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos = false, bool bLayout = false );

    bool GetEdgeBlending() const { return mbEdgeBlending; }
    void SetEdgeBlending(bool bNew) { mbEdgeBlending = bNew; }
};

class ImplBtn : public PushButton
{
private:
    bool            mbDown;

public:
                    ImplBtn( vcl::Window* pParent, WinBits nWinStyle = 0 );
                    virtual ~ImplBtn() {};

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MBDown();

    boost::signals2::signal< void ( ImplBtn* ) > buttonDownSignal;
};

void ImplInitFieldSettings( vcl::Window* pWin, bool bFont, bool bForeground, bool bBackground );
void ImplInitDropDownButton( PushButton* pButton );

#endif // INCLUDED_VCL_INC_ILSTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
