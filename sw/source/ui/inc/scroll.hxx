/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scroll.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:22:59 $
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
#ifndef _SCROLL_HXX
#define _SCROLL_HXX
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

class SwScrollbar: public ScrollBar
{
    Size    aDocSz;
    BOOL    bHori       :1;     // Horizontal = TRUE, sonst Vertikal
    BOOL    bAuto       :1;     // fuer Scrollingmode
    BOOL    bThumbEnabled:1;
    BOOL    bVisible    :1;     // Show/Hide sollen nur noch dieses Flag setzen
    BOOL    bSizeSet    :1;     // wurde die Groesse bereits gesetzt?

    void    AutoShow();

    using Window::Hide;
    using Window::SetPosSizePixel;
    using Window::IsVisible;

public:

    void    ExtendedShow( BOOL bVisible = TRUE );
    void    Hide() { Show( FALSE ); }
    void    SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );
    BOOL    IsVisible(BOOL bReal) const { return bReal ? ScrollBar::IsVisible() : bVisible; }

        // Aenderung der Dokumentgroesse
    void    DocSzChgd(const Size &rNewSize);
        // Aenderung des sichtbaren Bereiches
    void    ViewPortChgd(const Rectangle &rRectangle);
        // was fuer einer ist es denn ??
    BOOL    IsHoriScroll() const { return bHori; }

    void    SetAuto(BOOL bSet);
    BOOL    IsAuto() { return bAuto;}

    SwScrollbar(Window *pParent, BOOL bHori = TRUE );
    ~SwScrollbar();
};



#endif
