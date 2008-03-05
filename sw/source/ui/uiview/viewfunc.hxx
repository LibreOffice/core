/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfunc.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:48:39 $
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
#ifndef _VIEWFUNC_HXX
#define _VIEWFUNC_HXX
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class ImageButton;
class Point;
class PrintDialog;
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
PrintDialog* CreatePrintDialog( Window* , USHORT, SwWrtShell* );
void SetPrinter( IDocumentDeviceAccess*, SfxPrinter*, BOOL bWeb );
SfxTabPage* CreatePrintOptionsPage( Window*, const SfxItemSet& );
void SetAppPrintOptions( ViewShell* pSh, BOOL bWeb );

// folgende Funktionen stehen im viewport.cxx
void ViewResizePixel( const Window &rRef,
                    const Point &rOfst,
                    const Size &rSize,
                    const Size &rEditSz,
                    const BOOL bInner,
                    SwScrollbar& rVScrollbar,
                    SwScrollbar& rHScrollbar,
                    ImageButton* pPageUpBtn,
                    ImageButton* pPageDownBtn,
                    ImageButton* pNaviBtn,
                    Window& rScrollBarBox,
                    SvxRuler* pVLineal = 0,
                    SvxRuler* pHLineal = 0,
                    BOOL bWebView = FALSE,
                    BOOL bVRulerRight = FALSE );


#endif
