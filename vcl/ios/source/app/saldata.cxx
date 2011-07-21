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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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
    mnSystemVersion( VER_TIGER ),
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
