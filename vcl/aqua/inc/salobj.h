/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salobj.h,v $
 * $Revision: 1.12 $
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

#include "vcl/sv.h"
#include "vcl/sysdata.hxx"
#include "vcl/salobj.hxx"

class AquaSalFrame;
class AquaSalObject;


// -----------------
// - SalObjectData -
// -----------------

struct SalObjectData
{
};

class AquaSalObject : public SalObject
{
public:
    AquaSalFrame*               mpFrame;                // parent frame
    NSClipView*                 mpClipView;
    SystemEnvData               maSysData;

    long                        mnClipX;
    long                        mnClipY;
    long                        mnClipWidth;
    long                        mnClipHeight;
    bool                        mbClip;

    long                        mnX;
    long                        mnY;
    long                        mnWidth;
    long                        mnHeight;


    void setClippedPosSize();


    AquaSalObject( AquaSalFrame* pFrame );
    virtual ~AquaSalObject();

    virtual void                    ResetClipRegion();
    virtual USHORT                  GetClipRegionType();
    virtual void                    BeginSetClipRegion( ULONG nRects );
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                    EndSetClipRegion();
    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight );
    virtual void                    Show( BOOL bVisible );
    virtual void                    Enable( BOOL nEnable );
    virtual void                    GrabFocus();
    virtual void                    SetBackground();
    virtual void                    SetBackground( SalColor nSalColor );
    virtual const SystemEnvData*    GetSystemData() const;
};

#endif // _SV_SALOBJ_H
