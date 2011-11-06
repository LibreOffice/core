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



#ifndef SC_NOTEMARK_HXX
#define SC_NOTEMARK_HXX

#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#include <vcl/timer.hxx>
#include "global.hxx"
#include "address.hxx"

class SdrModel;
class SdrObject;

class ScNoteMarker
{
private:
    Window*     pWindow;
    Window*     pRightWin;
    Window*     pBottomWin;
    Window*     pDiagWin;
    ScDocument* pDoc;
    ScAddress   aDocPos;
    String      aUserText;
    Rectangle   aVisRect;
    Timer       aTimer;
    MapMode     aMapMode;
    sal_Bool        bLeft;
    sal_Bool        bByKeyboard;

    Rectangle       aRect;
    SdrModel*       pModel;
    SdrObject*      pObject;
    sal_Bool            bVisible;

    DECL_LINK( TimeHdl, Timer* );

public:
                ScNoteMarker( Window* pWin, Window* pRight, Window* pBottom, Window* pDiagonal,
                                ScDocument* pD, ScAddress aPos, const String& rUser,
                                const MapMode& rMap, sal_Bool bLeftEdge, sal_Bool bForce, sal_Bool bKeyboard );
                ~ScNoteMarker();

    void        Draw();
    void        InvalidateWin();

    ScAddress   GetDocPos() const       { return aDocPos; }
    sal_Bool        IsByKeyboard() const    { return bByKeyboard; }
};



#endif

