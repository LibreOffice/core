/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: connctr.hxx,v $
 * $Revision: 1.3 $
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


#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <bootstrp/listmacr.hxx>
#include <tools/stream.hxx>

class DepWin;
class ObjectWin;

static Rectangle aEmptyRect( Point( 0, 0), Point( 0, 0));

class Connector
{
    ObjectWin *mpStartWin, *mpEndWin;
    ULONG mnStartId, mnEndId;
    Point mStart, mEnd, mCenter;
    DepWin* mpParent;

    BOOL bVisible;
    BOOL mbHideIndependend;
    static BOOL msbHideMode;


public:
    double len;
    Connector( DepWin* pParent, WinBits nWinStyle );
    ~Connector();

    void    Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin, BOOL bVis = FALSE );
    Point   GetMiddle();
    void    Paint( const Rectangle& rRect );
    void    UpdatePosition( ObjectWin* pWin, BOOL bPaint = TRUE );
    USHORT  Save( SvFileStream& rOutFile );
    USHORT  Load( SvFileStream& rInFile );
    ULONG   GetStartId(){ return mnStartId; };
    ULONG   GetEndId(){ return mnEndId; };
    ObjectWin*  GetStartWin(){ return mpStartWin; };
    ObjectWin*  GetEndWin(){ return mpEndWin; };
    ObjectWin*  GetOtherWin( ObjectWin* pWin );
    ULONG   GetOtherId( ULONG nId );
    ULONG   GetLen();
    BOOL    IsStart( ObjectWin* pWin );
    void    SetHideIndependend( BOOL bHide) { mbHideIndependend = bHide; };

    BOOL    IsVisible() { return bVisible; }
    void    SetVisibility( BOOL visible ) { bVisible = visible; }
    void    UpdateVisibility();
    void    SetHideMode(BOOL bHide) { msbHideMode = bHide; };
    void    DrawOutput( OutputDevice* pDevice, const Point& rOffset  );
};

DECLARE_LIST( ConnectorList, Connector* )

#endif
