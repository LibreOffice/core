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


#include "ios/saldata.hxx"
#include "ios/saluimenu.h"
#include "ios/salinst.h"

oslThreadKey SalData::s_aAutoReleaseKey = 0;

static void SAL_CALL releasePool( void* pPool )
{
    if( pPool )
        [(NSAutoreleasePool*)pPool release];
}

SalData::SalData()
:
    mpTimerProc( NULL ),
    mpFirstInstance( NULL ),
    mpFirstObject( NULL ),
    mpFirstVD( NULL ),
    mpFirstPrinter( NULL ),
    mpFontList( NULL ),
    mxRGBSpace( CGColorSpaceCreateDeviceRGB( ) ),
    mxGraySpace( CGColorSpaceCreateDeviceGray( ) ),
    mxP50Space( NULL ),
    mxP50Pattern( NULL ),
    mnDPIX( 0 ),
    mnDPIY( 0 )
{
    if( s_aAutoReleaseKey == 0 )
        s_aAutoReleaseKey = osl_createThreadKey( releasePool );
}

SalData::~SalData()
{
    CGPatternRelease( mxP50Pattern );
    CGColorSpaceRelease( mxP50Space );
    CGColorSpaceRelease( mxRGBSpace );
    CGColorSpaceRelease( mxGraySpace );
    if( s_aAutoReleaseKey )
    {
        // release the last pool
        NSAutoreleasePool* pPool = nil;
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( pPool )
        {
            osl_setThreadKeyData( s_aAutoReleaseKey, NULL );
            [pPool release];
        }

        osl_destroyThreadKey( s_aAutoReleaseKey );
        s_aAutoReleaseKey = 0;
    }
}

void SalData::ensureThreadAutoreleasePool()
{
    NSAutoreleasePool* pPool = nil;
    if( s_aAutoReleaseKey )
    {
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( ! pPool )
        {
            pPool = [[NSAutoreleasePool alloc] init];
            osl_setThreadKeyData( s_aAutoReleaseKey, pPool );
        }
    }
    else
    {
        OSL_FAIL( "no autorelease key" );
    }
}

void SalData::drainThreadAutoreleasePool()
{
    NSAutoreleasePool* pPool = nil;
    if( s_aAutoReleaseKey )
    {
        pPool = reinterpret_cast<NSAutoreleasePool*>( osl_getThreadKeyData( s_aAutoReleaseKey ) );
        if( pPool )
        {
            // osl_setThreadKeyData( s_aAutoReleaseKey, NULL );
            // [pPool release];
            [pPool drain];
        }
        else
        {
            pPool = [[NSAutoreleasePool alloc] init];
            osl_setThreadKeyData( s_aAutoReleaseKey, pPool );
        }
    }
    else
    {
        OSL_FAIL( "no autorelease key" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
