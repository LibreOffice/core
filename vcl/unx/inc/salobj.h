/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <vcl/sv.h>
#include <vcl/sysdata.hxx>
#include <vcl/salobj.hxx>
#include <vcl/dllapi.h>

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


class X11SalObject : public SalObject
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
