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
