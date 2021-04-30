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
#include "headless/svpframe.hxx"
#include "headless/svpinst.hxx"
#include "headless/svpvd.hxx"
#endif
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#include <quartz/utils.h>

std::unique_ptr<SalVirtualDevice> AquaSalInstance::CreateVirtualDevice( SalGraphics& rGraphics,
                                                        tools::Long &nDX, tools::Long &nDY,
                                                        DeviceFormat eFormat,
                                                        const SystemGraphicsData *pData )
{
    // #i92075# can be called first in a thread
    SalData::ensureThreadAutoreleasePool();

#ifdef IOS
    if( pData )
    {
        return std::unique_ptr<SalVirtualDevice>(new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >(&rGraphics),
                                         nDX, nDY, eFormat, pData ));
    }
    else
    {
        std::unique_ptr<SalVirtualDevice> pNew(new AquaSalVirtualDevice( NULL, nDX, nDY, eFormat, NULL ));
        pNew->SetSize( nDX, nDY );
        return pNew;
    }
#else
    return std::unique_ptr<SalVirtualDevice>(new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >(&rGraphics),
                                     nDX, nDY, eFormat, pData ));
#endif
}

AquaSalVirtualDevice::AquaSalVirtualDevice(
    AquaSalGraphics* pGraphic, tools::Long &nDX, tools::Long &nDY,
    DeviceFormat eFormat, const SystemGraphicsData *pData )
  : mbGraphicsUsed( false )
  , mnBitmapDepth( 0 )
  , mnWidth(0)
  , mnHeight(0)
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::AquaSalVirtualDevice() this=" << this
              << " size=(" << nDX << "x" << nDY << ") bitcount=" << static_cast<int>(eFormat) <<
              " pData=" << pData << " context=" << (pData ? pData->rCGContext : nullptr) );

    if( pGraphic && pData && pData->rCGContext )
    {
        // Create virtual device based on existing SystemGraphicsData
        // We ignore nDx and nDY, as the desired size comes from the SystemGraphicsData.
        // the mxContext is from pData (what "mxContext"? there is no such field anywhere in vcl;)
        mbForeignContext = true;
        mpGraphics = new AquaSalGraphics( /*pGraphic*/ );
        if (nDX == 0)
        {
            nDX = 1;
        }
        if (nDY == 0)
        {
            nDY = 1;
        }
        maLayer.set(CGLayerCreateWithContext(pData->rCGContext, CGSizeMake(nDX, nDY), nullptr));
        // Interrogate the context as to its real size
        if (maLayer.isSet())
        {
            const CGSize aSize = CGLayerGetSize(maLayer.get());
            nDX = static_cast<tools::Long>(aSize.width);
            nDY = static_cast<tools::Long>(aSize.height);
        }
        else
        {
            nDX = 0;
            nDY = 0;
        }

        mpGraphics->SetVirDevGraphics(maLayer, pData->rCGContext);

        SAL_INFO("vcl.virdev", "AquaSalVirtualDevice::AquaSalVirtualDevice() this=" << this <<
                 " (" << nDX << "x" << nDY << ") mbForeignContext=" << (mbForeignContext ? "YES" : "NO"));

    }
    else
    {
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
#ifdef MACOSX
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
}

AquaSalVirtualDevice::~AquaSalVirtualDevice()
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::~AquaSalVirtualDevice() this=" << this );

    if( mpGraphics )
    {
        mpGraphics->SetVirDevGraphics( nullptr, nullptr );
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
