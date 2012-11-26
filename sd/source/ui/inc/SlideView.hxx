/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    SlideView (
        SdDrawDocument* pDoc,
        ::Window* pWindow,
        SlideViewShell* pSlideVShell);
    virtual ~SlideView (void);

    void                Select( sal_uInt16 nSdPageNum, bool bSelect );
    void                SelectAllSlides( bool bSelect );

    void                MoveFocus( SlideViewFocusMove eMove );
    sal_uInt16              GetFocusPage() const;
    bool                HasFocus() const;

    sal_uInt16              ChangePagesPerRow( sal_uInt16 nNum );
    sal_uInt16              GetPagesPerRow() const { return nPagesPerRow; }

    virtual void        InvalidateOneWin( ::Window& rWin );
    virtual void        InvalidateOneWin( ::Window& rWin, const basegfx::B2DRange& rRange );

    void                SetAllowInvalidate( bool bFlag );
    bool                IsInvalidateAllowed() const;

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
                                    sal_uInt32 nPage = SDRPAGE_NOTFOUND,
                                    SdrLayerID aLayer = SDRLAYER_NOTFOUND);
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                     ::sd::Window* pTargetWindow = NULL,
                                     sal_uInt32 nPage = SDRPAGE_NOTFOUND,
                                     SdrLayerID aLayer = SDRLAYER_NOTFOUND);

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
    bool                bInPaint;
    bool                bInDelayedPaint;

                        DECL_LINK( PaintDelayed, Timer * );
    void CreateSlideTransferable (::Window* pWindow, bool bDrag);
};

} // end of namespace sd

#endif
