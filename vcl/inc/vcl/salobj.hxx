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

#define SAL_OBJECT_CLIP_INCLUDERECTS            ((sal_uInt16)0x0001)
#define SAL_OBJECT_CLIP_EXCLUDERECTS            ((sal_uInt16)0x0002)
#define SAL_OBJECT_CLIP_ABSOLUTE                ((sal_uInt16)0x0004)

// -------------
// - SalObject -
// -------------

class VCL_PLUGIN_PUBLIC SalObject
{
    void*               m_pInst;
    SALOBJECTPROC       m_pCallback;
    sal_Bool                m_bMouseTransparent:1,
                        m_bEraseBackground:1;
public:
    SalObject() : m_pInst( NULL ), m_pCallback( NULL ), m_bMouseTransparent( sal_False ), m_bEraseBackground( sal_True ) {}
    virtual ~SalObject();

    virtual void                    ResetClipRegion() = 0;
    virtual sal_uInt16                  GetClipRegionType() = 0;
    virtual void                    BeginSetClipRegion( sal_uLong nRects ) = 0;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    EndSetClipRegion() = 0;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    Show( sal_Bool bVisible ) = 0;
    virtual void                    Enable( sal_Bool nEnable ) = 0;
    virtual void                    GrabFocus() = 0;

    virtual void                    SetBackground() = 0;
    virtual void                    SetBackground( SalColor nSalColor ) = 0;

    virtual void                    SetForwardKey( sal_Bool /*bEnable*/ ) {}

    virtual const SystemEnvData*    GetSystemData() const = 0;

    virtual void InterceptChildWindowKeyDown( sal_Bool bIntercept ) = 0;

    void                    SetCallback( void* pInst, SALOBJECTPROC pProc )
    { m_pInst = pInst; m_pCallback = pProc; }
    long                    CallCallback( sal_uInt16 nEvent, const void* pEvent )
    { return m_pCallback ? m_pCallback( m_pInst, this, nEvent, pEvent ) : 0; }
    void                    SetMouseTransparent( sal_Bool bMouseTransparent )
    { m_bMouseTransparent = bMouseTransparent; }
    sal_Bool                    IsMouseTransparent()
    { return m_bMouseTransparent; }
    void                    EnableEraseBackground( sal_Bool bEnable )
    { m_bEraseBackground = bEnable; }
    sal_Bool                    IsEraseBackgroundEnabled()
    { return m_bEraseBackground; }
};

#endif // _SV_SALOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
