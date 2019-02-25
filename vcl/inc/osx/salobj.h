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

#ifndef INCLUDED_VCL_INC_OSX_SALOBJ_H
#define INCLUDED_VCL_INC_OSX_SALOBJ_H

#include <vcl/sysdata.hxx>
#include <salobj.hxx>

class AquaSalFrame;
class AquaSalObject;


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

    AquaSalObject( AquaSalFrame* pFrame, SystemWindowData const * pWinData );
    virtual ~AquaSalObject() override;

    virtual void                    ResetClipRegion() override;
    virtual void                    BeginSetClipRegion( sal_uInt32 nRects ) override;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void                    EndSetClipRegion() override;
    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void                    Show( bool bVisible ) override;
    virtual const SystemEnvData*    GetSystemData() const override;
};

#endif // INCLUDED_VCL_INC_OSX_SALOBJ_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
