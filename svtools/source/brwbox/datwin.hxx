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

#ifndef INCLUDED_SVTOOLS_SOURCE_BRWBOX_DATWIN_HXX
#define INCLUDED_SVTOOLS_SOURCE_BRWBOX_DATWIN_HXX

#include <svtools/brwbox.hxx>
#include <svtools/brwhead.hxx>
#include <vcl/timer.hxx>
#include <vcl/image.hxx>
#include <svtools/transfer.hxx>
#include <vector>



#define MIN_COLUMNWIDTH  2

typedef ::std::vector< Rectangle* > RectangleList;



class ButtonFrame
{
    Rectangle   aRect;
    Rectangle   aInnerRect;
    OUString    aText;
    bool        bPressed;
    bool        bCurs;
    bool        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const OUString &rText,
                            bool bPress,
                            bool bCursor,
                            bool _bDrawDisabled)
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



class BrowserColumn
{
    sal_uInt16          _nId;
    sal_uLong           _nOriginalWidth;
    sal_uLong           _nWidth;
    Image               _aImage;
    OUString            _aTitle;
    bool                _bFrozen;

public:
                        BrowserColumn( sal_uInt16 nItemId, const Image &rImage,
                                        const OUString& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom );
    virtual            ~BrowserColumn();

    sal_uInt16          GetId() const { return _nId; }

    sal_uLong           Width() { return _nWidth; }
    Image&              GetImage() { return _aImage; }
    OUString&           Title() { return _aTitle; }

    bool                IsFrozen() const { return _bFrozen; }
    void                Freeze( bool bFreeze = true ) { _bFrozen = bFreeze; }

    virtual void        Draw( BrowseBox& rBox, OutputDevice& rDev,
                              const Point& rPos, bool bCurs  );

    void                SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom);
    void                ZoomChanged(const Fraction& rNewZoom);
};



class BrowserDataWin
            :public Control
            ,public DragSourceHelper
            ,public DropTargetHelper
{
public:
    BrowserHeader*  pHeaderBar;     // only for BROWSER_HEADERBAR_NEW
    vcl::Window*         pEventWin;      // Window of forwarded events
    ScrollBarBox*   pCornerWin;     // Window in the corner btw the ScrollBars
    bool            bInDtor;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // prevents pseudo-MouseMoves

    OUString        aRealRowCount;  // to show in VScrollBar

    RectangleList   aInvalidRegion; // invalidated Rectangles during !UpdateMode
    bool            bInPaint;       // TRUE while in Paint
    bool            bInCommand;     // TRUE while in Command
    bool            bNoScrollBack;  // only scroll forward
    bool            bNoHScroll;     // no horizontal scrollbar
    bool            bNoVScroll;     // no vertical scrollbar
    bool            bAutoHScroll;   // autohide horizontaler Scrollbar
    bool            bAutoVScroll;   // autohide horizontaler Scrollbar
    bool            bUpdateMode;    // not SV-UpdateMode because of Invalidate()
    bool            bAutoSizeLastCol; // last column always fills up window
    bool            bResizeOnPaint;   // outstanding resize-event
    bool            bUpdateOnUnlock;  // Update() while locked
    bool            bInUpdateScrollbars;  // prevents recursions
    bool            bHadRecursion;        // a recursion occurred
    bool            bOwnDataChangedHdl;   // dont change colors in DataChanged
    bool            bCallingDropCallback; // we're in a callback to AcceptDrop or ExecuteDrop curently
    sal_uInt16          nUpdateLock;    // lock count, dont call Control::Update()!
    short           nCursorHidden;  // new counter for DoHide/ShowCursor

    long            m_nDragRowDividerLimit;
    long            m_nDragRowDividerOffset;

public:
                    BrowserDataWin( BrowseBox* pParent );
                    virtual ~BrowserDataWin();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rEvt ) SAL_OVERRIDE;
                    DECL_LINK( RepeatedMouseMove, void * );

    virtual void    MouseButtonUp( const MouseEvent& rEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rEvt ) SAL_OVERRIDE;
    virtual void    Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;

    // DropTargetHelper overridables
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

    // DragSourceHelper overridables
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel ) SAL_OVERRIDE;


    BrowseEvent     CreateBrowseEvent( const Point& rPosPixel );
    void            Repaint();
    BrowseBox*      GetParent() const
                         { return static_cast<BrowseBox*>( Window::GetParent() ); }
    const OUString& GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( bool bMode );
    bool            GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate( sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;
    void            Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 ) SAL_OVERRIDE;
    void            Invalidate( const vcl::Region& rRegion, sal_uInt16 nFlags = 0 ) SAL_OVERRIDE
                    { Control::Invalidate( rRegion, nFlags ); }

protected:
    void            StartRowDividerDrag( const Point& _rStartPos );
    bool            ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent );
};



inline void BrowserDataWin::Repaint()
{
    if ( GetUpdateMode() )
        Update();
    Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}



class BrowserScrollBar: public ScrollBar
{
    sal_uLong           _nTip;
    sal_uLong           _nLastPos;
    BrowserDataWin* _pDataWin;

public:
                    BrowserScrollBar( vcl::Window* pParent, WinBits nStyle,
                                      BrowserDataWin *pDataWin )
                    :   ScrollBar( pParent, nStyle ),
                        _nTip( 0 ),
                        _nLastPos( ULONG_MAX ),
                        _pDataWin( pDataWin )
                    {}
                    //ScrollBar( vcl::Window* pParent, const ResId& rResId );

    virtual void    Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void    EndScroll() SAL_OVERRIDE;
};



void InitSettings_Impl( vcl::Window *pWin,
         bool bFont = true, bool bForeground = true, bool bBackground = true );



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
