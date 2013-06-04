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

#ifndef _SVP_SVPVD_HXX
#define _SVP_SVPVD_HXX

#include <salvd.hxx>
#include "svpelement.hxx"

#include <list>

#ifdef IOS
#define SvpSalGraphics AquaSalGraphics
#endif

class SvpSalGraphics;

class SvpSalVirtualDevice : public SalVirtualDevice, public SvpElement
{
    sal_uInt16                          m_nBitCount;
    basebmp::BitmapDeviceSharedPtr      m_aDevice;
    std::list< SvpSalGraphics* >        m_aGraphics;

public:
    SvpSalVirtualDevice( sal_uInt16 nBitCount ) : SvpElement(), m_nBitCount(nBitCount) {}
    virtual ~SvpSalVirtualDevice();

    // SvpElement
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aDevice; }

    // SalVirtualDevice
    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

    virtual sal_Bool        SetSize( long nNewDX, long nNewDY );
    virtual sal_Bool        SetSizeUsingBuffer( long nNewDX, long nNewDY, const basebmp::RawMemorySharedArray &pBuffer );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#endif // _SVP_SVPVD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
