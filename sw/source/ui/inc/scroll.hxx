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
    sal_Bool    bHori       :1;     // horizontal = salTrue, otherwise vertical
    sal_Bool    bAuto       :1;     // for scrolling mode
    sal_Bool    bVisible    :1;     // show/hide should only set this flag
    sal_Bool    bSizeSet    :1;     // was the size already set?

    void    AutoShow();

    using Window::Hide;
    using Window::IsVisible;

public:

    void    ExtendedShow( sal_Bool bVisible = sal_True );
    void    Hide() { Show( sal_False ); }
    void    SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );
    sal_Bool    IsVisible(sal_Bool bReal) const { return bReal ? ScrollBar::IsVisible() : bVisible; }

        // changing of document size
    void    DocSzChgd(const Size &rNewSize);
        // changing of visible region
    void    ViewPortChgd(const Rectangle &rRectangle);
        // what is it??
    sal_Bool    IsHoriScroll() const { return bHori; }

    void    SetAuto(sal_Bool bSet);
    sal_Bool    IsAuto() { return bAuto;}

    SwScrollbar(Window *pParent, sal_Bool bHori = sal_True );
    ~SwScrollbar();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
