/*************************************************************************
 *
 *  $RCSfile: scrwin.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:52 $
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

#ifndef _SCRWIN_HXX
#define _SCRWIN_HXX

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

class ScrollableWindow: public Window
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
    void            ImpInitialize( ScrollableWindowFlags nFlags );
    DECL_LINK( ScrollHdl, ScrollBar * );
    DECL_LINK( EndScrollHdl, ScrollBar * );
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

    void            SetMapMode( const MapMode& rNewMapMode );
    MapMode         GetMapMode() const;

    void            SetTotalSize( const Size& rNewSize );
    Size            GetTotalSize() { return PixelToLogic( aTotPixSz ); }

    void            SetVisibleSize( const Size& rNewSize );
    BOOL            MakeVisible( const Rectangle& rTarget, BOOL bSloppy = FALSE );
    Rectangle       GetVisibleArea() const;

    void            SetLineSize( ULONG nHorz, ULONG nVert );
    void            Scroll( long nDeltaX, long nDeltaY );
    void            Scroll( long nDeltaX, long nDeltaY,
                            const Rectangle& rRect );
    void            ScrollLines( long nLinesX, long nLinesY );
    void            ScrollPages( long nPagesX, ULONG nOverlapX,
                                 long nPagesY, ULONG nOverlapY );

private:
    void            SetOutputSizePixel( const Size& rSize );
    Size            GetOutputSizePixel() const;
    Size            GetOutputSize() const;
};

#endif
