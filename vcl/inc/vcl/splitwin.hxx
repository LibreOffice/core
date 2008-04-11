/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: splitwin.hxx,v $
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

#ifndef _SV_SPLITWIN_HXX
#define _SV_SPLITWIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>

class Wallpaper;
struct ImplSplitSet;

// -----------------------
// - SplitWindowItemBits -
// -----------------------

typedef USHORT SplitWindowItemBits;

// -------------------------------
// - Bits fuer SplitWindow-Items -
// -------------------------------

#define SWIB_FIXED                  ((SplitWindowItemBits)0x0001)
#define SWIB_RELATIVESIZE           ((SplitWindowItemBits)0x0002)
#define SWIB_PERCENTSIZE            ((SplitWindowItemBits)0x0004)
#define SWIB_COLSET                 ((SplitWindowItemBits)0x0008)
#define SWIB_INVISIBLE              ((SplitWindowItemBits)0x0010)

// ---------------------
// - SplitWindow-Types -
// ---------------------

#define SPLITWINDOW_APPEND          ((USHORT)0xFFFF)
#define SPLITWINDOW_ITEM_NOTFOUND   ((USHORT)0xFFFF)

// ---------------
// - SplitWindow -
// ---------------

class VCL_DLLPUBLIC SplitWindow : public DockingWindow
{
private:
    ImplSplitSet*       mpMainSet;
    ImplSplitSet*       mpBaseSet;
    ImplSplitSet*       mpSplitSet;
    long*               mpLastSizes;
    Rectangle           maDragRect;
    long                mnDX;
    long                mnDY;
    long                mnLeftBorder;
    long                mnTopBorder;
    long                mnRightBorder;
    long                mnBottomBorder;
    long                mnMaxSize;
    long                mnMouseOff;
    long                mnMStartPos;
    long                mnMSplitPos;
    WinBits             mnWinStyle;
    WindowAlign         meAlign;
    USHORT              mnSplitTest;
    USHORT              mnSplitPos;
    USHORT              mnMouseModifier;
    BOOL                mbDragFull:1,
                        mbHorz:1,
                        mbBottomRight:1,
                        mbCalc:1,
                        mbRecalc:1,
                        mbInvalidate:1,
                        mbSizeable:1,
                        mbBorder:1,
                        mbAutoHide:1,
                        mbFadeIn:1,
                        mbFadeOut:1,
                        mbAutoHideIn:1,
                        mbAutoHideDown:1,
                        mbFadeInDown:1,
                        mbFadeOutDown:1,
                        mbAutoHidePressed:1,
                        mbFadeInPressed:1,
                        mbFadeOutPressed:1,
                        mbFadeNoButtonMode:1,
                        mbNoAlign:1;
    Link                maStartSplitHdl;
    Link                maSplitHdl;
    Link                maSplitResizeHdl;
    Link                maAutoHideHdl;
    Link                maFadeInHdl;
    Link                maFadeOutHdl;

    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplInitSettings();
    SAL_DLLPRIVATE void ImplCalcLayout();
    SAL_DLLPRIVATE void ImplUpdate();
    SAL_DLLPRIVATE void ImplUpdateSet( ImplSplitSet* pSet );
    SAL_DLLPRIVATE void ImplSetWindowSize( long nDelta );
    SAL_DLLPRIVATE void ImplSplitMousePos( Point& rMousePos );
    SAL_DLLPRIVATE void ImplGetButtonRect( Rectangle& rRect, long nEx, BOOL bTest ) const;
    SAL_DLLPRIVATE void ImplGetAutoHideRect( Rectangle& rRect, BOOL bTest = FALSE ) const;
    SAL_DLLPRIVATE void ImplGetFadeInRect( Rectangle& rRect, BOOL bTest = FALSE ) const;
    SAL_DLLPRIVATE void ImplGetFadeOutRect( Rectangle& rRect, BOOL bTest = FALSE ) const;
    SAL_DLLPRIVATE void ImplDrawButtonRect( const Rectangle& rRect, long nSize );
    SAL_DLLPRIVATE void ImplDrawAutoHide( BOOL bInPaint );
    SAL_DLLPRIVATE void ImplDrawFadeIn( BOOL bInPaint );
    SAL_DLLPRIVATE void ImplDrawFadeOut( BOOL bInPaint );
    SAL_DLLPRIVATE void ImplNewAlign();
    SAL_DLLPRIVATE void ImplDrawGrip( const Rectangle& rRect, BOOL bHorz, BOOL bLeft );
    SAL_DLLPRIVATE void ImplDrawFadeArrow( const Point& rPt, BOOL bHorz, BOOL bLeft );
    SAL_DLLPRIVATE void ImplStartSplit( const MouseEvent& rMEvt );

    static SAL_DLLPRIVATE void ImplDrawBorder( SplitWindow* pWin );
    static SAL_DLLPRIVATE void ImplDrawBorderLine( SplitWindow* pWin );
    static SAL_DLLPRIVATE void ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, BOOL bHide,
                                             BOOL bRows, BOOL bDown = TRUE );
    static SAL_DLLPRIVATE void ImplDrawBack( SplitWindow* pWindow, ImplSplitSet* pSet );
    static SAL_DLLPRIVATE void ImplDrawBack( SplitWindow* pWindow, const Rectangle& rRect,
                                             const Wallpaper* pWall, const Bitmap* pBitmap );
    static SAL_DLLPRIVATE USHORT ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, USHORT& rFoundPos,
                                                BOOL bRows, BOOL bDown = TRUE );
    static SAL_DLLPRIVATE USHORT ImplTestSplit( SplitWindow* pWindow, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, USHORT& rFoundPos );
    static SAL_DLLPRIVATE void ImplDrawSplitTracking( SplitWindow* pThis, const Point& rPos );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      SplitWindow (const SplitWindow &);
    SAL_DLLPRIVATE      SplitWindow & operator= (const SplitWindow &);
public:
                        SplitWindow( Window* pParent, WinBits nStyle = 0 );
                        SplitWindow( Window* pParent, const ResId& rResId );
                        ~SplitWindow();

    virtual void        StartSplit();
    virtual void        Split();
    virtual void        SplitResize();
    virtual void        AutoHide();
    virtual void        FadeIn();
    virtual void        FadeOut();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Move();
    virtual void        Resize();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );

    void                InsertItem( USHORT nId, Window* pWindow, long nSize,
                                    USHORT nPos = SPLITWINDOW_APPEND, USHORT nSetId = 0,
                                    SplitWindowItemBits nBits = 0 );
    void                InsertItem( USHORT nId, long nSize,
                                    USHORT nPos = SPLITWINDOW_APPEND, USHORT nSetId = 0,
                                    SplitWindowItemBits nBits = 0 );
    void                MoveItem( USHORT nId, USHORT nNewPos, USHORT nNewSetId = 0 );
    void                RemoveItem( USHORT nId, BOOL bHide = TRUE );
    void                Clear();

    void                SetBaseSet( USHORT nSetId = 0 );
    USHORT              GetBaseSet() const;

    void                SetSplitSize( USHORT nSetId, long nSplitSize,
                                      BOOL bWithChilds = FALSE );
    long                GetSplitSize( USHORT nSetId ) const;
    void                SetItemBackground( USHORT nSetId );
    void                SetItemBackground( USHORT nSetId, const Wallpaper& rWallpaper );
    Wallpaper           GetItemBackground( USHORT nSetId ) const;
    BOOL                IsItemBackground( USHORT nSetId ) const;
    void                SetItemBitmap( USHORT nSetId, const Bitmap& rBitmap );
    Bitmap              GetItemBitmap( USHORT nSetId ) const;

    void                SplitItem( USHORT nId, long nNewSize,
                                   BOOL bPropSmall = FALSE,
                                   BOOL bPropGreat = FALSE );
    void                SetItemSize( USHORT nId, long nNewSize );
    long                GetItemSize( USHORT nId ) const;
    long                GetItemSize( USHORT nId, SplitWindowItemBits nBits ) const;
    void                SetItemBits( USHORT nId, SplitWindowItemBits nNewBits );
    SplitWindowItemBits GetItemBits( USHORT nId ) const;
    Window*             GetItemWindow( USHORT nId ) const;
    USHORT              GetSet( USHORT nId ) const;
    BOOL                GetSet( USHORT nId, USHORT& rSetId, USHORT& rPos ) const;
    USHORT              GetItemId( Window* pWindow ) const;
    USHORT              GetItemId( const Point& rPos ) const;
    USHORT              GetItemPos( USHORT nId, USHORT nSetId = 0 ) const;
    USHORT              GetItemId( USHORT nPos, USHORT nSetId = 0 ) const;
    USHORT              GetItemCount( USHORT nSetId = 0 ) const;
    BOOL                IsItemValid( USHORT nId ) const;

    void                SetNoAlign( BOOL bNoAlign );
    BOOL                IsNoAlign() const { return mbNoAlign; }
    void                SetAlign( WindowAlign eNewAlign = WINDOWALIGN_TOP );
    WindowAlign         GetAlign() const { return meAlign; }
    BOOL                IsHorizontal() const { return mbHorz; }

    BOOL                IsSplitting() const { return IsTracking(); }

    void                SetMaxSizePixel( long nNewMaxSize ) { mnMaxSize = nNewMaxSize; }
    long                GetMaxSizePixel() const { return mnMaxSize; }

    static Size         CalcWindowSizePixel( const Size& rSize,
                                             WindowAlign eAlign,
                                             WinBits nWinStyle,
                                             BOOL bExtra = FALSE );
    Size                CalcWindowSizePixel( const Size& rSize )
                            { return CalcWindowSizePixel( rSize, meAlign, mnWinStyle, (mbAutoHide || mbFadeOut) ); }

    Size                CalcLayoutSizePixel( const Size& aNewSize );

    void                ShowAutoHideButton( BOOL bShow = TRUE );
    BOOL                IsAutoHideButtonVisible() const { return mbAutoHide; }
    void                ShowFadeInHideButton( BOOL bShow = TRUE );
    void                ShowFadeInButton( BOOL bShow = TRUE ) { ShowFadeInHideButton( bShow ); }
    BOOL                IsFadeInButtonVisible() const { return mbFadeIn; }
    void                ShowFadeOutButton( BOOL bShow = TRUE );
    BOOL                IsFadeOutButtonVisible() const { return mbFadeOut; }
    long                GetFadeInSize() const;
    BOOL                IsFadeNoButtonMode() const { return mbFadeNoButtonMode; }

    void                SetAutoHideState( BOOL bAutoHide );
    BOOL                GetAutoHideState() const { return mbAutoHideIn; }

    Rectangle           GetAutoHideRect() const;
    Rectangle           GetFadeInRect() const;
    Rectangle           GetFadeOutRect() const;

    void                SetStartSplitHdl( const Link& rLink ) { maStartSplitHdl = rLink; }
    const Link&         GetStartSplitHdl() const { return maStartSplitHdl; }
    void                SetSplitHdl( const Link& rLink ) { maSplitHdl = rLink; }
    const Link&         GetSplitHdl() const { return maSplitHdl; }
    void                SetSplitResizeHdl( const Link& rLink ) { maSplitResizeHdl = rLink; }
    const Link&         GetSplitResizeHdl() const { return maSplitResizeHdl; }
    void                SetAutoHideHdl( const Link& rLink ) { maAutoHideHdl = rLink; }
    const Link&         GetAutoHideHdl() const { return maAutoHideHdl; }
    void                SetFadeInHdl( const Link& rLink ) { maFadeInHdl = rLink; }
    const Link&         GetFadeInHdl() const { return maFadeInHdl; }
    void                SetFadeOutHdl( const Link& rLink ) { maFadeOutHdl = rLink; }
    const Link&         GetFadeOutHdl() const { return maFadeOutHdl; }
};

#endif  // _SV_SPLITWIN_HXX
