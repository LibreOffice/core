/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_INC_SCROLL_HXX
#define INCLUDED_SW_SOURCE_UI_INC_SCROLL_HXX
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
    void    Hide() { Show( false ); }
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
