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



#ifndef _BMPWIN_HXX
#define _BMPWIN_HXX

#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#include <vcl/window.hxx>

/*--------------------------------------------------------------------
    Beschreibung:   Extended Page fuer Grafiken
 --------------------------------------------------------------------*/

class BmpWindow : public Window
{
private:
    Graphic     aGraphic;
    BitmapEx    aBmp;
    BitmapEx    aBmpHC;

    sal_Bool        bHorz : 1;
    sal_Bool        bVert : 1;
    sal_Bool        bGraphic : 1;
    sal_Bool        bLeftAlign : 1;

    void Paint(const Rectangle& rRect);

public:
    BmpWindow(Window* pPar, sal_uInt16 nId,
                const Graphic& rGraphic, const BitmapEx& rBmp, const BitmapEx& rBmpHC);
    BmpWindow(Window* pParent, const ResId rResId) :
        Window(pParent, rResId),
        bHorz(sal_False), bVert(sal_False),bGraphic(sal_False), bLeftAlign(sal_True) {}
    ~BmpWindow();
    void MirrorVert(sal_Bool bMirror) { bVert = bMirror; Invalidate(); }
    void MirrorHorz(sal_Bool bMirror) { bHorz = bMirror; Invalidate(); }
    void SetGraphic(const Graphic& rGrf);
};

#endif
