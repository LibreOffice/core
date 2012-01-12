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



#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <vcl/sv.h>
#include <vcl/sysdata.hxx>

#include <vclpluginapi.h>
#include <salobj.hxx>

class SalClipRegion
{

public:

                SalClipRegion();
               ~SalClipRegion();

    void        BeginSetClipRegion( sal_uIntPtr nRects );
    void        UnionClipRegion( long nX, long nY, long nWidth, long nHeight );

    XRectangle *EndSetClipRegion()  {
        return ClipRectangleList;   }
    void        ResetClipRegion()   {
        numClipRectangles = 0;      }
    sal_uInt16      GetClipRegionType() {
        return nClipRegionType;     }
    void        SetClipRegionType( sal_uInt16 nType ) {
        nClipRegionType = nType;    }
    int         GetRectangleCount() {
        return numClipRectangles;   }

private:

    XRectangle* ClipRectangleList;
    int         numClipRectangles;
    int         maxClipRectangles;
    sal_uInt16      nClipRegionType;
};


class VCLPLUG_GEN_PUBLIC X11SalObject : public SalObject
{
public:
    SystemChildData maSystemChildData;
    SalFrame*       mpParent;
    XLIB_Window     maPrimary;
    XLIB_Window     maSecondary;
    Colormap        maColormap;
    SalClipRegion   maClipRegion;
    sal_Bool            mbVisible;

    static VCL_DLLPUBLIC long Dispatch( XEvent* pEvent );
    static VCL_DLLPUBLIC X11SalObject* CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );

    X11SalObject();
    virtual ~X11SalObject();

    // overload all pure virtual methods
     virtual void                   ResetClipRegion();
    virtual sal_uInt16                  GetClipRegionType();
    virtual void                    BeginSetClipRegion( sal_uIntPtr nRects );
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                    EndSetClipRegion();

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight );
    virtual void                    Show( sal_Bool bVisible );
    virtual void                    Enable( sal_Bool nEnable );
    virtual void                    GrabFocus();

    virtual void                    SetBackground();
    virtual void                    SetBackground( SalColor nSalColor );

    virtual const SystemEnvData*    GetSystemData() const;

    virtual void InterceptChildWindowKeyDown( sal_Bool bIntercept );
};

#endif // _SV_SALOBJ_H
