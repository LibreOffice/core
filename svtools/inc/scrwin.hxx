/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scrwin.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:26:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SCRWIN_HXX
#define _SCRWIN_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

class DataChangedEvent;

// -------------------------
// - ScrollableWindow-Type -
// -------------------------

typedef USHORT ScrollableWindowFlags;

#define SCRWIN_THUMBDRAGGING 1
#define SCRWIN_VCENTER       2
#define SCRWIN_HCENTER       4
#define SCRWIN_DEFAULT       (SCRWIN_THUMBDRAGGING | SCRWIN_VCENTER | SCRWIN_HCENTER)

// --------------------
// - ScrollableWindow -
// --------------------

class SVT_DLLPUBLIC ScrollableWindow: public Window
{
private:
    Point           aPixOffset;         // offset to virtual window (pixel)
    Size            aTotPixSz;          // total size of virtual window (pixel)
    long            nLinePixH;          // size of a line/column (pixel)
    long            nColumnPixW;

    ScrollBar       aVScroll;           // the scrollbars
    ScrollBar       aHScroll;
    ScrollBarBox    aCornerWin;         // window in the bottom right corner
    BOOL            bScrolling:1,       // user controlled scrolling
                    bHandleDragging:1,  // scroll window while dragging
                    bHCenter:1,
                    bVCenter:1;

#ifdef _SVT_SCRWIN_CXX
    SVT_DLLPRIVATE void         ImpInitialize( ScrollableWindowFlags nFlags );
    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar * );
    DECL_DLLPRIVATE_LINK( EndScrollHdl, ScrollBar * );
#endif

public:
                    ScrollableWindow( Window* pParent, WinBits nBits = 0,
                                      ScrollableWindowFlags = SCRWIN_DEFAULT );
                    ScrollableWindow( Window* pParent, const ResId& rId,
                                      ScrollableWindowFlags = SCRWIN_DEFAULT );

    virtual void    Resize();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    DataChanged( const DataChangedEvent& rDEvt );

    virtual void    StartScroll();
    virtual void    EndScroll( long nDeltaX, long nDeltaY );

    using OutputDevice::SetMapMode;
    virtual void    SetMapMode( const MapMode& rNewMapMode );
    virtual MapMode GetMapMode() const;

    void            SetTotalSize( const Size& rNewSize );
    Size            GetTotalSize() { return PixelToLogic( aTotPixSz ); }

    void            SetVisibleSize( const Size& rNewSize );
    BOOL            MakeVisible( const Rectangle& rTarget, BOOL bSloppy = FALSE );
    Rectangle       GetVisibleArea() const;

    void            SetLineSize( ULONG nHorz, ULONG nVert );
    using Window::Scroll;
    virtual void    Scroll( long nDeltaX, long nDeltaY, USHORT nFlags = 0 );
    void            ScrollLines( long nLinesX, long nLinesY );
    void            ScrollPages( long nPagesX, ULONG nOverlapX,
                                 long nPagesY, ULONG nOverlapY );

private:
    SVT_DLLPRIVATE Size         GetOutputSizePixel() const;
    SVT_DLLPRIVATE Size         GetOutputSize() const;
};

#endif
