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

#ifndef _SFXDATWIN_HXX
#define _SFXDATWIN_HXX

#include <svtools/brwbox.hxx>
#include <svtools/brwhead.hxx>
#include <vcl/timer.hxx>
#include <vcl/image.hxx>
#include <svtools/transfer.hxx>
#include <vector>

//===================================================================

#define MIN_COLUMNWIDTH  2
#define DRAG_CRITICAL    4

typedef ::std::vector< Rectangle* > RectangleList;

//===================================================================

class ButtonFrame
{
    Rectangle   aRect;
    Rectangle   aInnerRect;
    String      aText;
    BOOL        bPressed;
    BOOL        bCurs;
    BOOL        bAbbr;
    BOOL        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const String &rText,
                            BOOL bPress = FALSE,
                            BOOL bCursor = FALSE,
                            BOOL bAbbreviate = TRUE,
                            BOOL _bDrawDisabled = FALSE)
                :aRect( rPt, rSz )
                ,aInnerRect( Point( aRect.Left()+1, aRect.Top()+1 ),
                            Size( aRect.GetWidth()-2, aRect.GetHeight()-2 ) )
                ,aText(rText)
                ,bPressed(bPress)
                ,bCurs(bCursor)
                ,bAbbr(bAbbreviate)
                ,m_bDrawDisabled(_bDrawDisabled)
            {
            }

    void    Draw( OutputDevice& rDev );
};

//===================================================================

class BrowserColumn
{
    USHORT              _nId;
    ULONG               _nOriginalWidth;
    ULONG               _nWidth;
    Image               _aImage;
    String              _aTitle;
    BOOL                _bFrozen;
    HeaderBarItemBits   _nFlags;

public:
                        BrowserColumn( USHORT nItemId, const Image &rImage,
                                        const String& rTitle, ULONG nWidthPixel, const Fraction& rCurrentZoom,
                                        HeaderBarItemBits nFlags );
    virtual            ~BrowserColumn();

    USHORT              GetId() const { return _nId; }

    ULONG               Width() { return _nWidth; }
    Image&              GetImage() { return _aImage; }
    String&             Title() { return _aTitle; }
    HeaderBarItemBits&  Flags() { return _nFlags; }

    BOOL                IsFrozen() const { return _bFrozen; }
    void                Freeze( BOOL bFreeze = TRUE ) { _bFrozen = bFreeze; }

    virtual void        Draw( BrowseBox& rBox, OutputDevice& rDev,
                              const Point& rPos, BOOL bCurs  );

    void                SetWidth(ULONG nNewWidthPixel, const Fraction& rCurrentZoom);
    void                ZoomChanged(const Fraction& rNewZoom);
};

//===================================================================

class BrowserDataWin
            :public Control
            ,public DragSourceHelper
            ,public DropTargetHelper
{
public:
    BrowserHeader*  pHeaderBar;     // only for BROWSER_HEADERBAR_NEW
    Window*         pEventWin;      // Window of forwarded events
    ScrollBarBox*   pCornerWin;     // Window in the corner btw the ScrollBars
    BOOL*           pDtorNotify;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // verhindert pseudo-MouseMoves

    String          aRealRowCount;  // zur Anzeige im VScrollBar

    RectangleList   aInvalidRegion; // invalidated Rectangles during !UpdateMode
    bool            bInPaint;       // TRUE while in Paint
    bool            bInCommand;     // TRUE while in Command
    bool            bNoScrollBack;  // nur vorwaerts scrollen
    bool            bNoHScroll;     // kein horizontaler Scrollbar
    bool            bNoVScroll;     // no vertical scrollbar
    bool            bAutoHScroll;   // autohide horizontaler Scrollbar
    bool            bAutoVScroll;   // autohide horizontaler Scrollbar
    bool            bUpdateMode;    // nicht SV-UpdateMode wegen Invalidate()
    bool            bAutoSizeLastCol; // last column always fills up window
    bool            bResizeOnPaint;   // outstanding resize-event
    bool            bUpdateOnUnlock;  // Update() while locked
    bool            bInUpdateScrollbars;  // Rekursionsschutz
    bool            bHadRecursion;        // Rekursion war aufgetreten
    bool            bOwnDataChangedHdl;   // dont change colors in DataChanged
    bool            bCallingDropCallback; // we're in a callback to AcceptDrop or ExecuteDrop curently
    USHORT          nUpdateLock;    // lock count, dont call Control::Update()!
    short           nCursorHidden;  // new conuter for DoHide/ShowCursor

    long            m_nDragRowDividerLimit;
    long            m_nDragRowDividerOffset;

public:
                    BrowserDataWin( BrowseBox* pParent );
                    ~BrowserDataWin();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Command( const CommandEvent& rEvt );
    virtual void    MouseButtonDown( const MouseEvent& rEvt );
    virtual void    MouseMove( const MouseEvent& rEvt );
                    DECL_LINK( RepeatedMouseMove, void * );

    virtual void    MouseButtonUp( const MouseEvent& rEvt );
    virtual void    KeyInput( const KeyEvent& rEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    // DropTargetHelper overridables
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper overridables
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel );


    BrowseEvent     CreateBrowseEvent( const Point& rPosPixel );
    void            Repaint();
    BrowseBox*      GetParent() const
                         { return (BrowseBox*) Window::GetParent(); }
    const String&   GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( BOOL bMode );
    bool            GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate( USHORT nFlags = 0 );
    void            Invalidate( const Rectangle& rRect, USHORT nFlags = 0 );
    void            Invalidate( const Region& rRegion, USHORT nFlags = 0 )
                    { Control::Invalidate( rRegion, nFlags ); }

protected:
    void            StartRowDividerDrag( const Point& _rStartPos );
    BOOL            ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent );
};

//-------------------------------------------------------------------

inline void BrowserDataWin::Repaint()
{
    if ( GetUpdateMode() )
        Update();
    Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}

//===================================================================

class BrowserScrollBar: public ScrollBar
{
    ULONG           _nTip;
    ULONG           _nLastPos;
    BrowserDataWin* _pDataWin;

public:
                    BrowserScrollBar( Window* pParent, WinBits nStyle,
                                      BrowserDataWin *pDataWin )
                    :   ScrollBar( pParent, nStyle ),
                        _nTip( 0 ),
                        _nLastPos( ULONG_MAX ),
                        _pDataWin( pDataWin )
                    {}
                    //ScrollBar( Window* pParent, const ResId& rResId );

    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    EndScroll();
};

//===================================================================

void InitSettings_Impl( Window *pWin,
         BOOL bFont = TRUE, BOOL bForeground = TRUE, BOOL bBackground = TRUE );

//===================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
