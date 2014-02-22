/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "bitmapbackbuffer.hxx"

#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>


namespace vclcanvas
{
    BitmapBackBuffer::BitmapBackBuffer( const BitmapEx&         rBitmap,
                                        const OutputDevice&     rRefDevice ) :
        maBitmap( rBitmap ),
        mpVDev( NULL ),
        mrRefDevice( rRefDevice ),
        mbBitmapContentIsCurrent( false ),
        mbVDevContentIsCurrent( false )
    {
    }

    BitmapBackBuffer::~BitmapBackBuffer()
    {
        
        
        SolarMutexGuard aGuard;

        if( mpVDev )
            delete mpVDev;
    }

    OutputDevice& BitmapBackBuffer::getOutDev()
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    const OutputDevice& BitmapBackBuffer::getOutDev() const
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    void BitmapBackBuffer::clear()
    {
        
        getBitmapReference().Erase(COL_TRANSPARENT);
    }

    BitmapEx& BitmapBackBuffer::getBitmapReference()
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::getBitmapReference(): Both bitmap and VDev are valid?!" );

        if( mbVDevContentIsCurrent && mpVDev )
        {
            
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            const Point aEmptyPoint;
            *maBitmap = mpVDev->GetBitmapEx( aEmptyPoint,
                                             mpVDev->GetOutputSizePixel() );
        }

        
        
        mbBitmapContentIsCurrent = true;
        mbVDevContentIsCurrent   = false;

        return *maBitmap;
    }

    Size BitmapBackBuffer::getBitmapSizePixel() const
    {
        Size aSize = maBitmap->GetSizePixel();

        if( mbVDevContentIsCurrent && mpVDev )
        {
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            aSize = mpVDev->GetOutputSizePixel();
        }

        return aSize;
    }

    void BitmapBackBuffer::createVDev() const
    {
        if( !mpVDev )
        {
            
            
            mpVDev = maBitmap->IsTransparent() ?
                new VirtualDevice( mrRefDevice, 0, 0 ) :
                new VirtualDevice( mrRefDevice );

            OSL_ENSURE( mpVDev,
                        "BitmapBackBuffer::createVDev(): Unable to create VirtualDevice" );

            mpVDev->SetOutputSizePixel( maBitmap->GetSizePixel() );

            
#if defined( MACOSX )
            
            mpVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | mpVDev->GetAntialiasing() );
#else
            
            
            
            mpVDev->SetAntialiasing(mpVDev->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
#endif
        }
    }

    void BitmapBackBuffer::updateVDev() const
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::updateVDev(): Both bitmap and VDev are valid?!" );

        if( mpVDev && mbBitmapContentIsCurrent )
        {
            
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            const Point aEmptyPoint;
            mpVDev->DrawBitmapEx( aEmptyPoint, *maBitmap );
        }

        
        
        mbBitmapContentIsCurrent = false;
        mbVDevContentIsCurrent   = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
