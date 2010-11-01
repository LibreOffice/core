/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SCROLL_HXX
#define _SCROLL_HXX
#include <vcl/scrbar.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
