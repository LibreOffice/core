/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salobj.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:48:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
