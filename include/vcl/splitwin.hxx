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

#ifndef _SV_SPLITWIN_HXX
#define _SV_SPLITWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>

class Wallpaper;
struct ImplSplitSet;

// -----------------------
// - SplitWindowItemBits -
// -----------------------

typedef sal_uInt16 SplitWindowItemBits;

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

#define SPLITWINDOW_APPEND          ((sal_uInt16)0xFFFF)
#define SPLITWINDOW_ITEM_NOTFOUND   ((sal_uInt16)0xFFFF)

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
    sal_uInt16              mnSplitTest;
    sal_uInt16              mnSplitPos;
    sal_uInt16              mnMouseModifier;
    sal_Bool                mbDragFull:1,
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
    SAL_DLLPRIVATE void ImplSetWindowSize( long nDelta );
    SAL_DLLPRIVATE void ImplSplitMousePos( Point& rMousePos );
    SAL_DLLPRIVATE void ImplGetButtonRect( Rectangle& rRect, long nEx, sal_Bool bTest ) const;
    SAL_DLLPRIVATE void ImplGetAutoHideRect( Rectangle& rRect, sal_Bool bTest = sal_False ) const;
    SAL_DLLPRIVATE void ImplGetFadeInRect( Rectangle& rRect, sal_Bool bTest = sal_False ) const;
    SAL_DLLPRIVATE void ImplGetFadeOutRect( Rectangle& rRect, sal_Bool bTest = sal_False ) const;
    SAL_DLLPRIVATE void ImplDrawButtonRect( const Rectangle& rRect, long nSize );
    SAL_DLLPRIVATE void ImplDrawAutoHide( sal_Bool bInPaint );
    SAL_DLLPRIVATE void ImplDrawFadeIn( sal_Bool bInPaint );
    SAL_DLLPRIVATE void ImplDrawFadeOut( sal_Bool bInPaint );
    SAL_DLLPRIVATE void ImplNewAlign();
    SAL_DLLPRIVATE void ImplDrawGrip( const Rectangle& rRect, sal_Bool bHorz, sal_Bool bLeft );
    SAL_DLLPRIVATE void ImplDrawFadeArrow( const Point& rPt, sal_Bool bHorz, sal_Bool bLeft );
    SAL_DLLPRIVATE void ImplStartSplit( const MouseEvent& rMEvt );

    static SAL_DLLPRIVATE void ImplDrawBorder( SplitWindow* pWin );
    static SAL_DLLPRIVATE void ImplDrawBorderLine( SplitWindow* pWin );
    static SAL_DLLPRIVATE void ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, sal_Bool bHide,
                                             sal_Bool bRows, sal_Bool bDown = sal_True );
    static SAL_DLLPRIVATE void ImplDrawBack( SplitWindow* pWindow, ImplSplitSet* pSet );
    static SAL_DLLPRIVATE void ImplDrawBack( SplitWindow* pWindow, const Rectangle& rRect,
                                             const Wallpaper* pWall, const Bitmap* pBitmap );
    static SAL_DLLPRIVATE sal_uInt16 ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos,
                                                sal_Bool bRows, sal_Bool bDown = sal_True );
    static SAL_DLLPRIVATE sal_uInt16 ImplTestSplit( SplitWindow* pWindow, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos );
    static SAL_DLLPRIVATE void ImplDrawSplitTracking( SplitWindow* pThis, const Point& rPos );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      SplitWindow (const SplitWindow &);
    SAL_DLLPRIVATE      SplitWindow & operator= (const SplitWindow &);
public:
                        SplitWindow( Window* pParent, WinBits nStyle = 0 );
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

    void                InsertItem( sal_uInt16 nId, Window* pWindow, long nSize,
                                    sal_uInt16 nPos = SPLITWINDOW_APPEND, sal_uInt16 nSetId = 0,
                                    SplitWindowItemBits nBits = 0 );
    void                InsertItem( sal_uInt16 nId, long nSize,
                                    sal_uInt16 nPos = SPLITWINDOW_APPEND, sal_uInt16 nSetId = 0,
                                    SplitWindowItemBits nBits = 0 );
    void                RemoveItem( sal_uInt16 nId, sal_Bool bHide = sal_True );
    void                Clear();

    void                SplitItem( sal_uInt16 nId, long nNewSize,
                                   sal_Bool bPropSmall = sal_False,
                                   sal_Bool bPropGreat = sal_False );
    void                SetItemSize( sal_uInt16 nId, long nNewSize );
    long                GetItemSize( sal_uInt16 nId ) const;
    /** Set a range that limits the (variable part of the) size with an
        upper and a lower bound (both are valid values themselves.)
        @param nId
            Id of the item for which the size limits are set.
        @param aRange
            Values of -1 define missing bounds, thus setting a range (-1,-1)
            (the default) removes the size limitiation.
    */
    void                SetItemSizeRange (sal_uInt16 nId, const Range aRange);
    /** Return the current size limits for the specified item.
    */
    long                GetItemSize( sal_uInt16 nId, SplitWindowItemBits nBits ) const;
    sal_uInt16              GetSet( sal_uInt16 nId ) const;
    sal_uInt16              GetItemId( Window* pWindow ) const;
    sal_uInt16              GetItemId( const Point& rPos ) const;
    sal_uInt16              GetItemPos( sal_uInt16 nId, sal_uInt16 nSetId = 0 ) const;
    sal_uInt16              GetItemId( sal_uInt16 nPos, sal_uInt16 nSetId = 0 ) const;
    sal_uInt16              GetItemCount( sal_uInt16 nSetId = 0 ) const;
    sal_Bool                IsItemValid( sal_uInt16 nId ) const;

    sal_Bool                IsNoAlign() const { return mbNoAlign; }
    void                SetAlign( WindowAlign eNewAlign = WINDOWALIGN_TOP );
    WindowAlign         GetAlign() const { return meAlign; }
    sal_Bool                IsHorizontal() const { return mbHorz; }

    sal_Bool                IsSplitting() const { return IsTracking(); }

    void                SetMaxSizePixel( long nNewMaxSize ) { mnMaxSize = nNewMaxSize; }
    long                GetMaxSizePixel() const { return mnMaxSize; }

    Size                CalcLayoutSizePixel( const Size& aNewSize );

    void                ShowAutoHideButton( sal_Bool bShow = sal_True );
    sal_Bool                IsAutoHideButtonVisible() const { return mbAutoHide; }
    void                ShowFadeInHideButton( sal_Bool bShow = sal_True );
    void                ShowFadeInButton( sal_Bool bShow = sal_True ) { ShowFadeInHideButton( bShow ); }
    sal_Bool                IsFadeInButtonVisible() const { return mbFadeIn; }
    void                ShowFadeOutButton( sal_Bool bShow = sal_True );
    sal_Bool                IsFadeOutButtonVisible() const { return mbFadeOut; }
    long                GetFadeInSize() const;
    sal_Bool                IsFadeNoButtonMode() const { return mbFadeNoButtonMode; }

    void                SetAutoHideState( sal_Bool bAutoHide );
    sal_Bool                GetAutoHideState() const { return mbAutoHideIn; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
