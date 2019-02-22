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
#include <vcl/transfer.hxx>
#include <memory>
#include <vector>


#define MIN_COLUMNWIDTH  2


class ButtonFrame
{
    tools::Rectangle const   aRect;
    tools::Rectangle const   aInnerRect;
    OUString const    aText;
    bool const        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const OUString &rText,
                            bool _bDrawDisabled)
                :aRect( rPt, rSz )
                ,aInnerRect( Point( aRect.Left()+1, aRect.Top()+1 ),
                            Size( aRect.GetWidth()-2, aRect.GetHeight()-2 ) )
                ,aText(rText)
                ,m_bDrawDisabled(_bDrawDisabled)
            {
            }

    void    Draw( OutputDevice& rDev );
};


class BrowserColumn final
{
    sal_uInt16 const    _nId;
    sal_uLong           _nOriginalWidth;
    sal_uLong           _nWidth;
    OUString            _aTitle;
    bool                _bFrozen;

public:
                        BrowserColumn( sal_uInt16 nItemId,
                                        const OUString& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom );
                        ~BrowserColumn();

    sal_uInt16          GetId() const { return _nId; }

    sal_uLong           Width() { return _nWidth; }
    OUString&           Title() { return _aTitle; }

    bool                IsFrozen() const { return _bFrozen; }
    void                Freeze() { _bFrozen = true; }

    void                Draw( BrowseBox const & rBox, OutputDevice& rDev,
                              const Point& rPos  );

    void                SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom);
    void                ZoomChanged(const Fraction& rNewZoom);
};


class BrowserDataWin
            :public Control
            ,public DragSourceHelper
            ,public DropTargetHelper
{
public:
    VclPtr<BrowserHeader> pHeaderBar;     // only for BrowserMode::HEADERBAR_NEW
    VclPtr<ScrollBarBox>  pCornerWin;     // Window in the corner btw the ScrollBars
    bool            bInDtor;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // prevents pseudo-MouseMoves

    OUString        aRealRowCount;  // to show in VScrollBar

    std::vector<tools::Rectangle> aInvalidRegion; // invalidated Rectangles during !UpdateMode
    bool            bInPaint;       // TRUE while in Paint
    bool            bInCommand;     // TRUE while in Command
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
    bool            bCallingDropCallback; // we're in a callback to AcceptDrop or ExecuteDrop currently
    sal_uInt16          nUpdateLock;    // lock count, don't call Control::Update()!
    short           nCursorHidden;  // new counter for DoHide/ShowCursor

    long            m_nDragRowDividerLimit;
    long            m_nDragRowDividerOffset;

public:
                    explicit BrowserDataWin( BrowseBox* pParent );
    virtual         ~BrowserDataWin() override;
    virtual void    dispose() override;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    Command( const CommandEvent& rEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rEvt ) override;
    virtual void    MouseMove( const MouseEvent& rEvt ) override;
                    DECL_LINK( RepeatedMouseMove, Timer *, void );

    virtual void    MouseButtonUp( const MouseEvent& rEvt ) override;
    virtual void    KeyInput( const KeyEvent& rEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;

    // DropTargetHelper overridables
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    // DragSourceHelper overridables
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel ) override;


    BrowseEvent     CreateBrowseEvent( const Point& rPosPixel );
    BrowseBox*      GetParent() const
                         { return static_cast<BrowseBox*>( Window::GetParent() ); }
    const OUString& GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( bool bMode );
    bool            GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE ) override;
    void            Invalidate( const tools::Rectangle& rRect, InvalidateFlags nFlags = InvalidateFlags::NONE ) override;
    using Control::Invalidate;

protected:
    void            StartRowDividerDrag( const Point& _rStartPos );
    bool            ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent );
};


class BrowserScrollBar: public ScrollBar
{
    sal_uLong           _nLastPos;
    VclPtr<BrowserDataWin> _pDataWin;

public:
                    BrowserScrollBar( vcl::Window* pParent, WinBits nStyle,
                                      BrowserDataWin *pDataWin )
                    :   ScrollBar( pParent, nStyle ),
                        _nLastPos( ULONG_MAX ),
                        _pDataWin( pDataWin )
                    {}
   virtual          ~BrowserScrollBar() override;
   virtual void     dispose() override;
                    //ScrollBar( vcl::Window* pParent, const ResId& rResId );

    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    EndScroll() override;
};


void InitSettings_Impl( vcl::Window *pWin );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
