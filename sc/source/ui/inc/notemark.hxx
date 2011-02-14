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

