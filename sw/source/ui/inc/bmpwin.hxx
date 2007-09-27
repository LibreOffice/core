/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bmpwin.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:54:18 $
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
#ifndef _BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

/*--------------------------------------------------------------------
    Beschreibung:   Extended Page fuer Grafiken
 --------------------------------------------------------------------*/

class BmpWindow : public Window
{
private:
    Graphic     aGraphic;
    BitmapEx    aBmp;
    BitmapEx    aBmpHC;

    BOOL        bHorz : 1;
    BOOL        bVert : 1;
    BOOL        bGraphic : 1;
    BOOL        bLeftAlign : 1;

    void Paint(const Rectangle& rRect);

public:
    BmpWindow(Window* pPar, USHORT nId,
                const Graphic& rGraphic, const BitmapEx& rBmp, const BitmapEx& rBmpHC);
    BmpWindow(Window* pParent, const ResId rResId) :
        Window(pParent, rResId),
        bHorz(FALSE), bVert(FALSE),bGraphic(FALSE), bLeftAlign(TRUE) {}
    ~BmpWindow();
    void MirrorVert(BOOL bMirror) { bVert = bMirror; Invalidate(); }
    void MirrorHorz(BOOL bMirror) { bHorz = bMirror; Invalidate(); }
    void SetGraphic(const Graphic& rGrf);
};

#endif
