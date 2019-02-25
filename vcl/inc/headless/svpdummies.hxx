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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPDUMMIES_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPDUMMIES_HXX

#include <vcl/sysdata.hxx>
#include <unx/gensys.h>
#include <salobj.hxx>
#include <salimestatus.hxx>
#include <salsys.hxx>

class SalGraphics;

class VCL_DLLPUBLIC SvpSalObject : public SalObject
{
public:
    SystemEnvData m_aSystemChildData;

    SvpSalObject();
    virtual ~SvpSalObject() override;

    // override all pure virtual methods
    virtual void                    ResetClipRegion() override;
    virtual void                    BeginSetClipRegion( sal_uInt32 nRects ) override;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void                    EndSetClipRegion() override;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void                    Show( bool bVisible ) override;

    virtual const SystemEnvData*    GetSystemData() const override;
};

class VCL_DLLPUBLIC SvpSalSystem : public SalGenericSystem
{
public:
    SvpSalSystem() {}
    virtual ~SvpSalSystem() override;
    // get info about the display
    virtual unsigned int GetDisplayScreenCount() override;
    virtual tools::Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen ) override;

    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::vector< OUString >& rButtons ) override;
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPDUMMIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
