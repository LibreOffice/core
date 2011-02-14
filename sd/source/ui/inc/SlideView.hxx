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

#ifndef SD_SLIDE_VIEW_HXX
#define SD_SLIDE_VIEW_HXX

#include "View.hxx"

class SdDrawDocument;
class BitmapCache;
class SdPage;

namespace sd {

class SlideViewShell;
class Window;

// ----------------------
// - SlideViewFocusMove -
// ----------------------

enum SlideViewFocusMove
{
    SLIDEVIEWFOCUSMOVE_NONE = 0,
    SLIDEVIEWFOCUSMOVE_TOGGLE = 1,
    SLIDEVIEWFOCUSMOVE_SELECT = 2,
    SLIDEVIEWFOCUSMOVE_LEFT = 3,
    SLIDEVIEWFOCUSMOVE_TOP = 4,
    SLIDEVIEWFOCUSMOVE_RIGHT = 5,
    SLIDEVIEWFOCUSMOVE_BOTTOM = 6
};

// ---------------
// - SdSlideView -
// ---------------

class SlideView
    : public ::sd::View
{
public:
    TYPEINFO();

    SlideView (
        SdDrawDocument* pDoc,
        ::Window* pWindow,
        SlideViewShell* pSlideVShell);
    virtual ~SlideView (void);

    void                Select( sal_uInt16 nSdPageNum, sal_Bool bSelect );
    void                SelectAllSlides( sal_Bool bSelect );

    void                MoveFocus( SlideViewFocusMove eMove );
    sal_uInt16              GetFocusPage() const;
    sal_Bool                HasFocus() const;

    sal_uInt16              ChangePagesPerRow( sal_uInt16 nNum );
    sal_uInt16              GetPagesPerRow() const { return nPagesPerRow; }

    virtual void        InvalidateOneWin( ::Window& rWin );
    virtual void        InvalidateOneWin( ::Window& rWin, const Rectangle& rRect );

    void                SetAllowInvalidate( sal_Bool bFlag );
    sal_Bool                IsInvalidateAllowed() const;

    void                Paint(const Rectangle& rRect, OutputDevice* pOut);
    void                DrawSelectionRect(sal_uInt16 nPage);

    Point               CalcPagePos( sal_uInt16 nPageNo ) const;
    Rectangle           GetPageArea( sal_uInt16 nPageNo ) const;
    sal_uLong               GetPageGap() const;
    Rectangle           GetFadeIconArea( sal_uInt16 nPageNo ) const;
    SdPage*             GetHitPage( const Point& rPos ) const;
    SdPage*             GetFadePage( const Point& rPos ) const;
    SdPage*             GetNearestPage( const Point& rPos ) const;

    void                DeleteMarked();
    void                MoveMarked( sal_uInt16 nTargetPage );

    void                    AddToCache( SdPage* pPage, const Bitmap& rBitmap, long nZoom );
    const GraphicObject*    GetFromCache( SdPage* pPage, long& rZoom, long nZoomTolerance ) const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void        DoCut( ::Window* pWindow = NULL );
    virtual void        DoCopy( ::Window* pWindow = NULL );
    virtual void        DoPaste( ::Window* pWindow = NULL );

    virtual void        StartDrag( const Point& rDragPt, ::Window* pWindow );
    virtual void        DragFinished( sal_Int8 nDropAction );

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow = NULL,
                                    sal_uInt16 nPage = SDRPAGE_NOTFOUND,
                                    sal_uInt16 nLayer = SDRPAGE_NOTFOUND );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                     ::sd::Window* pTargetWindow = NULL,
                                     sal_uInt16 nPage = SDRPAGE_NOTFOUND,
                                     sal_uInt16 nLayer = SDRPAGE_NOTFOUND );

    void                UpdateAllPages();

private:
    Timer               aDelayedPaintTimer;
    List                aDelayedPaints;
    SlideViewShell*     pSlideViewShell;
    BitmapCache*        pCache;
    VirtualDevice*      mpVDev;
    sal_uInt16              nAllowInvalidateSmph;
    sal_uInt16              nPagesPerRow;
    sal_uInt16              nFocusPage;
    sal_Bool                bInPaint;
    sal_Bool                bInDelayedPaint;

                        DECL_LINK( PaintDelayed, Timer * );
    void CreateSlideTransferable (::Window* pWindow, sal_Bool bDrag);
};

} // end of namespace sd

#endif
