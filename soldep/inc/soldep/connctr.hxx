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


#ifndef _FMRWRK_CONNCTR_HXX
#define _FMRWRK_CONNCTR_HXX


#include <vcl/window.hxx>
#include <soldep/listmacr.hxx>
#include <tools/stream.hxx>

class DepWin;
class ObjectWin;

static Rectangle aEmptyRect( Point( 0, 0), Point( 0, 0));

class Connector
{
    ObjectWin *mpStartWin, *mpEndWin;
    sal_uIntPtr mnStartId, mnEndId;
    Point mStart, mEnd, mCenter;
    DepWin* mpParent;

    sal_Bool bVisible;
    sal_Bool mbHideIndependend;
    static sal_Bool msbHideMode;


public:
    double len;
    Connector( DepWin* pParent, WinBits nWinStyle );
    ~Connector();

    void    Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin, sal_Bool bVis = sal_False );
    Point   GetMiddle();
    void    Paint( const Rectangle& rRect );
    void    UpdatePosition( ObjectWin* pWin, sal_Bool bPaint = sal_True );
    sal_uInt16  Save( SvFileStream& rOutFile );
    sal_uInt16  Load( SvFileStream& rInFile );
    sal_uIntPtr GetStartId(){ return mnStartId; };
    sal_uIntPtr GetEndId(){ return mnEndId; };
    ObjectWin*  GetStartWin(){ return mpStartWin; };
    ObjectWin*  GetEndWin(){ return mpEndWin; };
    ObjectWin*  GetOtherWin( ObjectWin* pWin );
    sal_uIntPtr GetOtherId( sal_uIntPtr nId );
    sal_uIntPtr GetLen();
    sal_Bool    IsStart( ObjectWin* pWin );
    void    SetHideIndependend( sal_Bool bHide) { mbHideIndependend = bHide; };

    sal_Bool    IsVisible() { return bVisible; }
    void    SetVisibility( sal_Bool visible ) { bVisible = visible; }
    void    UpdateVisibility();
    void    SetHideMode(sal_Bool bHide) { msbHideMode = bHide; };
    void    DrawOutput( OutputDevice* pDevice, const Point& rOffset  );
};

DECLARE_LIST( ConnectorList, Connector* )

#endif
