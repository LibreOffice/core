/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bmpwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:01:03 $
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

#ifndef _BMPWIN_HXX
#define _BMPWIN_HXX

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif


/*--------------------------------------------------------------------
    Beschreibung:   Extended Page fuer Grafiken
 --------------------------------------------------------------------*/

class BmpWindow : public Window
{
    void Paint(const Rectangle& rRect);
    Graphic aGraphic;
    Bitmap  aBmp;
    BOOL    bHorz : 1;
    BOOL    bVert : 1;
    BOOL    bGraphic : 1;
    BOOL    bLeftAlign : 1;

public:
    BmpWindow(Window* pPar, USHORT nId,
                const Graphic& rGraphic, const Bitmap& rBmp);
    BmpWindow(Window* pParent, const ResId rResId) :
        Window(pParent, rResId), bLeftAlign(TRUE),
        bHorz(FALSE), bVert(FALSE),bGraphic(FALSE){}
    ~BmpWindow();
    void MirrorVert(BOOL bMirror) { bVert = bMirror; Invalidate(); }
    void MirrorHorz(BOOL bMirror) { bHorz = bMirror; Invalidate(); }
    void SetGraphic(const Graphic& rGrf);

};

#endif
