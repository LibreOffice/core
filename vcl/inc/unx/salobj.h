/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <tools/solar.h>
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
    SystemEnvData maSystemChildData;
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
    virtual void                    GrabFocus();

    virtual const SystemEnvData*    GetSystemData() const;
};

#endif // _SV_SALOBJ_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
