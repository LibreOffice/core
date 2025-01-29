/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>
#include <sal/log.hxx>

#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#ifdef MACOSX
#include <osx/salinst.h>
#include <osx/saldata.hxx>
#include <osx/salframe.h>
#else
#include <ios/iosinst.hxx>
#include "headless/svpframe.hxx"
#include "headless/svpinst.hxx"
#include "headless/svpvd.hxx"
#endif
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#include <quartz/utils.h>

std::unique_ptr<SalVirtualDevice> AquaSalInstance::CreateVirtualDevice( SalGraphics& rGraphics,
                                                        tools::Long nDX, tools::Long nDY,
                                                        DeviceFormat eFormat )
{
    // #i92075# can be called first in a thread
    SalData::ensureThreadAutoreleasePool();

#ifdef IOS
    (void)rGraphics;
    std::unique_ptr<SalVirtualDevice> pNew(new AquaSalVirtualDevice( nullptr, nDX, nDY, eFormat ));
    pNew->SetSize( nDX, nDY );
    return pNew;
#else
    return std::unique_ptr<SalVirtualDevice>(new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >(&rGraphics),
                                     nDX, nDY, eFormat ));
#endif
}

AquaSalVirtualDevice::AquaSalVirtualDevice(
    AquaSalGraphics* pGraphic, tools::Long nDX, tools::Long nDY,
    DeviceFormat eFormat )
  : mbGraphicsUsed( false )
  , mnBitmapDepth( 0 )
  , mnWidth(0)
  , mnHeight(0)
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::AquaSalVirtualDevice() this=" << this
              << " size=(" << nDX << "x" << nDY << ") bitcount=" << static_cast<int>(eFormat) );

    // create empty new virtual device

    mbForeignContext = false;           // the mxContext is created within VCL
    mpGraphics = new AquaSalGraphics(); // never fails
    switch (eFormat)
    {
#ifdef IOS
        case DeviceFormat::GRAYSCALE:
            mnBitmapDepth = 8;
            break;
#endif
        default:
            mnBitmapDepth = 0;
            break;
    }
#ifdef IOS
    (void)pGraphic;
#else
    // inherit resolution from reference device
    if( pGraphic )
    {
        AquaSalFrame* pFrame = pGraphic->getGraphicsFrame();
        if( pFrame && AquaSalFrame::isAlive( pFrame ) )
        {
            mpGraphics->setGraphicsFrame( pFrame );
            mpGraphics->copyResolution( *pGraphic );
        }
    }
#endif
    if( nDX && nDY )
    {
        SetSize( nDX, nDY );
    }
    // NOTE: if SetSize does not succeed, we just ignore the nDX and nDY
}

AquaSalVirtualDevice::~AquaSalVirtualDevice()
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::~AquaSalVirtualDevice() this=" << this );

    if( mpGraphics )
    {
        mpGraphics->SetVirDevGraphics( this, nullptr, nullptr );
        delete mpGraphics;
        mpGraphics = nullptr;
    }
    Destroy();
}

SalGraphics* AquaSalVirtualDevice::AcquireGraphics()
{
    if( mbGraphicsUsed || !mpGraphics )
    {
        return nullptr;
    }
    mbGraphicsUsed = true;
    return mpGraphics;
}

void AquaSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphicsUsed = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
