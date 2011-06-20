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
#ifndef _VIEWFUNC_HXX
#define _VIEWFUNC_HXX
#include <tools/solar.h>

class ImageButton;
class Point;
class SfxItemSet;
class SfxPrinter;
class SfxTabPage;
class Size;
class SvxRuler;
class SwScrollbar;
class ViewShell;
class Window;
class SwWrtShell;

// folgende Funktionen stehen im viewprt.cxx
void SetPrinter( IDocumentDeviceAccess*, SfxPrinter*, sal_Bool bWeb );
SfxTabPage* CreatePrintOptionsPage( Window*, const SfxItemSet& );
void SetAppPrintOptions( ViewShell* pSh, sal_Bool bWeb );

// folgende Funktionen stehen im viewport.cxx
void ViewResizePixel( const Window &rRef,
                    const Point &rOfst,
                    const Size &rSize,
                    const Size &rEditSz,
                    const sal_Bool bInner,
                    SwScrollbar& rVScrollbar,
                    SwScrollbar& rHScrollbar,
                    ImageButton* pPageUpBtn,
                    ImageButton* pPageDownBtn,
                    ImageButton* pNaviBtn,
                    Window& rScrollBarBox,
                    SvxRuler* pVLineal = 0,
                    SvxRuler* pHLineal = 0,
                    sal_Bool bWebView = sal_False,
                    sal_Bool bVRulerRight = sal_False );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
