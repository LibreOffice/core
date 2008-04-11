/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salobj.hxx,v $
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

#ifndef _SV_SALOBJ_HXX
#define _SV_SALOBJ_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <vcl/salwtype.hxx>

struct SystemEnvData;

// -------------------
// - SalObject-Types -
// -------------------

#define SAL_OBJECT_CLIP_INCLUDERECTS            ((USHORT)0x0001)
#define SAL_OBJECT_CLIP_EXCLUDERECTS            ((USHORT)0x0002)
#define SAL_OBJECT_CLIP_ABSOLUTE                ((USHORT)0x0004)

// -------------
// - SalObject -
// -------------

class VCL_DLLPUBLIC SalObject
{
    void*               m_pInst;
    SALOBJECTPROC       m_pCallback;
    BOOL                m_bMouseTransparent:1,
                        m_bEraseBackground:1;
public:
    SalObject() : m_pInst( NULL ), m_pCallback( NULL ), m_bMouseTransparent( FALSE ), m_bEraseBackground( TRUE ) {}
    virtual ~SalObject();

    virtual void                    ResetClipRegion() = 0;
    virtual USHORT                  GetClipRegionType() = 0;
    virtual void                    BeginSetClipRegion( ULONG nRects ) = 0;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    EndSetClipRegion() = 0;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    Show( BOOL bVisible ) = 0;
    virtual void                    Enable( BOOL nEnable ) = 0;
    virtual void                    GrabFocus() = 0;

    virtual void                    SetBackground() = 0;
    virtual void                    SetBackground( SalColor nSalColor ) = 0;

    virtual const SystemEnvData*    GetSystemData() const = 0;

    void                    SetCallback( void* pInst, SALOBJECTPROC pProc )
    { m_pInst = pInst; m_pCallback = pProc; }
    long                    CallCallback( USHORT nEvent, const void* pEvent )
    { return m_pCallback ? m_pCallback( m_pInst, this, nEvent, pEvent ) : 0; }
    void                    SetMouseTransparent( BOOL bMouseTransparent )
    { m_bMouseTransparent = bMouseTransparent; }
    BOOL                    IsMouseTransparent()
    { return m_bMouseTransparent; }
    void                    EnableEraseBackground( BOOL bEnable )
    { m_bEraseBackground = bEnable; }
    BOOL                    IsEraseBackgroundEnabled()
    { return m_bEraseBackground; }
};

#endif // _SV_SALOBJ_HXX
