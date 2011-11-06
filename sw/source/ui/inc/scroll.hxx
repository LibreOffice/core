/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SCROLL_HXX
#define _SCROLL_HXX
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

class SwScrollbar: public ScrollBar
{
    Size    aDocSz;
    sal_Bool    bHori       :1;     // Horizontal = sal_True, sonst Vertikal
    sal_Bool    bAuto       :1;     // fuer Scrollingmode
    sal_Bool    bThumbEnabled:1;
    sal_Bool    bVisible    :1;     // Show/Hide sollen nur noch dieses Flag setzen
    sal_Bool    bSizeSet    :1;     // wurde die Groesse bereits gesetzt?

    void    AutoShow();

    using Window::Hide;
    using Window::SetPosSizePixel;
    using Window::IsVisible;

public:

    void    ExtendedShow( sal_Bool bVisible = sal_True );
    void    Hide() { Show( sal_False ); }
    void    SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );
    sal_Bool    IsVisible(sal_Bool bReal) const { return bReal ? ScrollBar::IsVisible() : bVisible; }

        // Aenderung der Dokumentgroesse
    void    DocSzChgd(const Size &rNewSize);
        // Aenderung des sichtbaren Bereiches
    void    ViewPortChgd(const Rectangle &rRectangle);
        // was fuer einer ist es denn ??
    sal_Bool    IsHoriScroll() const { return bHori; }

    void    SetAuto(sal_Bool bSet);
    sal_Bool    IsAuto() { return bAuto;}

    SwScrollbar(Window *pParent, sal_Bool bHori = sal_True );
    ~SwScrollbar();
};



#endif
