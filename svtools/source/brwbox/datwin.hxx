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

typedef ::std::vector< Rectangle* > RectangleList;

//===================================================================

class ButtonFrame
{
    Rectangle   aRect;
    Rectangle   aInnerRect;
    OUString    aText;
    sal_Bool        bPressed;
    sal_Bool        bCurs;
    sal_Bool        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const OUString &rText,
                            sal_Bool bPress,
                            sal_Bool bCursor,
                            sal_Bool _bDrawDisabled)
                :aRect( rPt, rSz )
                ,aInnerRect( Point( aRect.Left()+1, aRect.Top()+1 ),
                            Size( aRect.GetWidth()-2, aRect.GetHeight()-2 ) )
                ,aText(rText)
                ,bPressed(bPress)
                ,bCurs(bCursor)
                ,m_bDrawDisabled(_bDrawDisabled)
            {
            }

    void    Draw( OutputDevice& rDev );
};

//===================================================================

class BrowserColumn
{
    sal_uInt16              _nId;
    sal_uLong               _nOriginalWidth;
    sal_uLong               _nWidth;
    Image               _aImage;
    OUString            _aTitle;
    sal_Bool                _bFrozen;

public:
                        BrowserColumn( sal_uInt16 nItemId, const Image &rImage,
                                        const OUString& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom );
    virtual            ~BrowserColumn();

    sal_uInt16          GetId() const { return _nId; }

    sal_uLong           Width() { return _nWidth; }
    Image&              GetImage() { return _aImage; }
    OUString&           Title() { return _aTitle; }

    sal_Bool            IsFrozen() const { return _bFrozen; }
    void                Freeze( sal_Bool bFreeze = sal_True ) { _bFrozen = bFreeze; }

    virtual void        Draw( BrowseBox& rBox, OutputDevice& rDev,
                              const Point& rPos, sal_Bool bCurs  );

    void                SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom);
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
    sal_Bool*           pDtorNotify;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // verhindert pseudo-MouseMoves

    OUString        aRealRowCount;  // zur Anzeige im VScrollBar

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
    sal_uInt16          nUpdateLock;    // lock count, dont call Control::Update()!
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
    const OUString& GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( sal_Bool bMode );
    bool            GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate( sal_uInt16 nFlags = 0 );
    void            Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    void            Invalidate( const Region& rRegion, sal_uInt16 nFlags = 0 )
                    { Control::Invalidate( rRegion, nFlags ); }

protected:
    void            StartRowDividerDrag( const Point& _rStartPos );
    sal_Bool            ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent );
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
    sal_uLong           _nTip;
    sal_uLong           _nLastPos;
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
         sal_Bool bFont = sal_True, sal_Bool bForeground = sal_True, sal_Bool bBackground = sal_True );

//===================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
