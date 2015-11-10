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

#ifndef INCLUDED_VCL_INC_SALOBJ_HXX
#define INCLUDED_VCL_INC_SALOBJ_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <salwtype.hxx>

struct SystemEnvData;

// SalObject types

#define SAL_OBJECT_CLIP_INCLUDERECTS            ((sal_uInt16)0x0001)
#define SAL_OBJECT_CLIP_EXCLUDERECTS            ((sal_uInt16)0x0002)
#define SAL_OBJECT_CLIP_ABSOLUTE                ((sal_uInt16)0x0004)

class VCL_PLUGIN_PUBLIC SalObject
{
    void*               m_pInst;
    SALOBJECTPROC       m_pCallback;
    bool                m_bMouseTransparent:1,
                        m_bEraseBackground:1;
public:
            SalObject() : m_pInst( nullptr ), m_pCallback( nullptr ), m_bMouseTransparent( false ), m_bEraseBackground( true ) {}
            virtual ~SalObject();

    virtual void                    ResetClipRegion() = 0;
    virtual sal_uInt16              GetClipRegionType() = 0;
    virtual void                    BeginSetClipRegion( sal_uLong nRects ) = 0;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    EndSetClipRegion() = 0;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) = 0;
    virtual void                    Show( bool bVisible ) = 0;
    virtual void                    Enable( bool /* nEnable */ ) {} // overridden by WinSalObject
    virtual void                    GrabFocus() {}

    virtual void                    SetForwardKey( bool /* bEnable */ ) {}

    virtual const SystemEnvData*    GetSystemData() const = 0;

    void                            SetCallback( void* pInst, SALOBJECTPROC pProc )
                                        { m_pInst = pInst; m_pCallback = pProc; }
    long                            CallCallback( sal_uInt16 nEvent, const void* pEvent )
                                        { return m_pCallback ? m_pCallback( m_pInst, this, nEvent, pEvent ) : 0; }

    void                            SetMouseTransparent( bool bMouseTransparent )
                                        { m_bMouseTransparent = bMouseTransparent; }
    bool                            IsMouseTransparent()
                                        { return m_bMouseTransparent; }

    void                            EnableEraseBackground( bool bEnable )
                                        { m_bEraseBackground = bEnable; }
    bool                            IsEraseBackgroundEnabled()
                                        { return m_bEraseBackground; }
};

#endif // INCLUDED_VCL_INC_SALOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
