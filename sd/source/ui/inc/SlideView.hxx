/*************************************************************************
 *
 *  $RCSfile: SlideView.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:10:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDE_VIEW_HXX
#define SD_SLIDE_VIEW_HXX

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif

class SdDrawDocument;
class BitmapCache;
class SdPage;

namespace sd {

class ShowView;
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

    void                Select( USHORT nSdPageNum, BOOL bSelect );
    void                SelectAll( BOOL bSelect );

    void                MoveFocus( SlideViewFocusMove eMove );
    USHORT              GetFocusPage() const;
    BOOL                HasFocus() const;

    USHORT              ChangePagesPerRow( USHORT nNum );
    USHORT              GetPagesPerRow() const { return nPagesPerRow; }

    virtual void        InvalidateOneWin( ::Window& rWin );
    virtual void        InvalidateOneWin( ::Window& rWin, const Rectangle& rRect );

    void                SetMultiPageView( BOOL bFlag ) { bPageTwice = bFlag; }
    void                SetAllowInvalidate( BOOL bFlag );
    BOOL                IsInvalidateAllowed() const;

    void                Paint(const Rectangle& rRect, OutputDevice* pOut);
    void                DrawSelectionRect(USHORT nPage);

    Point               CalcPagePos( USHORT nPageNo ) const;
    Rectangle           GetPageArea( USHORT nPageNo ) const;
    ULONG               GetPageGap() const;
    Rectangle           GetFadeIconArea( USHORT nPageNo ) const;
    SdPage*             GetHitPage( const Point& rPos ) const;
    SdPage*             GetFadePage( const Point& rPos ) const;
    SdPage*             GetNearestPage( const Point& rPos ) const;

    void                ArrangePages();
    void                DeleteMarked();
    void                MoveMarked( USHORT nTargetPage );

    void                    AddToCache( SdPage* pPage, const Bitmap& rBitmap, long nZoom );
    const GraphicObject*    GetFromCache( SdPage* pPage, long& rZoom, long nZoomTolerance ) const;

    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );

    virtual void        DoCut( ::Window* pWindow = NULL );
    virtual void        DoCopy( ::Window* pWindow = NULL );
    virtual void        DoPaste( ::Window* pWindow = NULL );

    virtual void        StartDrag( const Point& rDragPt, ::Window* pWindow );
    virtual void        DragFinished( sal_Int8 nDropAction );

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow = NULL,
                                    USHORT nPage = SDRPAGE_NOTFOUND,
                                    USHORT nLayer = SDRPAGE_NOTFOUND );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                     ::sd::Window* pTargetWindow = NULL,
                                     USHORT nPage = SDRPAGE_NOTFOUND,
                                     USHORT nLayer = SDRPAGE_NOTFOUND );

    void                UpdateAllPages();

private:
    Timer               aDelayedPaintTimer;
    List                aDelayedPaints;
    SlideViewShell* pSlideViewShell;
    BitmapCache*        pCache;
    ShowView*           pShowView;
    VirtualDevice*      pVDev;
    USHORT              nAllowInvalidateSmph;
    USHORT              nPagesPerRow;
    USHORT              nFocusPage;
    BOOL                bInPaint;
    BOOL                bInDelayedPaint;

                        DECL_LINK( PaintDelayed, Timer * );
    void CreateSlideTransferable (::Window* pWindow, BOOL bDrag);
};

} // end of namespace sd

#endif
