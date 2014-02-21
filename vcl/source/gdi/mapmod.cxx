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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>
#include <vcl/mapmod.hxx>


ImplMapMode::ImplMapMode() :
    maOrigin( 0, 0 ),
    maScaleX( 1, 1 ),
    maScaleY( 1, 1 )
{
    mnRefCount  = 1;
    meUnit      = MAP_PIXEL;
    mbSimple    = false;
}

ImplMapMode::ImplMapMode( const ImplMapMode& rImplMapMode ) :
    maOrigin( rImplMapMode.maOrigin ),
    maScaleX( rImplMapMode.maScaleX ),
    maScaleY( rImplMapMode.maScaleY )
{
    mnRefCount      = 1;
    meUnit          = rImplMapMode.meUnit;
    mbSimple        = false;
}

SvStream& ReadImplMapMode( SvStream& rIStm, ImplMapMode& rImplMapMode )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;

    rIStm.ReadUInt16( nTmp16 ); rImplMapMode.meUnit = (MapUnit) nTmp16;
    ReadPair( rIStm, rImplMapMode.maOrigin );
    ReadFraction( rIStm, rImplMapMode.maScaleX );
    ReadFraction( rIStm, rImplMapMode.maScaleY );
    rIStm.ReadCharAsBool( rImplMapMode.mbSimple );

    return rIStm;
}

SvStream& WriteImplMapMode( SvStream& rOStm, const ImplMapMode& rImplMapMode )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm.WriteUInt16( (sal_uInt16) rImplMapMode.meUnit );
    WritePair( rOStm, rImplMapMode.maOrigin );
    WriteFraction( rOStm, rImplMapMode.maScaleX );
    WriteFraction( rOStm, rImplMapMode.maScaleY );
    rOStm.WriteUChar( rImplMapMode.mbSimple );

    return rOStm;
}

ImplMapMode* ImplMapMode::ImplGetStaticMapMode( MapUnit eUnit )
{
    static long aStaticImplMapModeAry[(MAP_LASTENUMDUMMY)*sizeof(ImplMapMode)/sizeof(long)];

    // #i19496 check for out-of-bounds
     if( eUnit >= MAP_LASTENUMDUMMY )
        return (ImplMapMode*)aStaticImplMapModeAry;

    ImplMapMode* pImplMapMode = ((ImplMapMode*)aStaticImplMapModeAry)+eUnit;
    if ( !pImplMapMode->mbSimple )
    {
        Fraction aDefFraction( 1, 1 );
        pImplMapMode->maScaleX  = aDefFraction;
        pImplMapMode->maScaleY  = aDefFraction;
        pImplMapMode->meUnit    = eUnit;
        pImplMapMode->mbSimple  = true;
    }

    return pImplMapMode;
}

inline void MapMode::ImplMakeUnique()
{
    // If there are other references, copy
    if ( mpImplMapMode->mnRefCount != 1 )
    {
        if ( mpImplMapMode->mnRefCount )
            mpImplMapMode->mnRefCount--;
        mpImplMapMode = new ImplMapMode( *mpImplMapMode );
    }
}

MapMode::MapMode()
{

    mpImplMapMode = ImplMapMode::ImplGetStaticMapMode( MAP_PIXEL );
}

MapMode::MapMode( const MapMode& rMapMode )
{
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFFFFFE, "MapMode: RefCount overflow" );

    // Take over Shared Instance Data and increment refcount
    mpImplMapMode = rMapMode.mpImplMapMode;
    // RefCount == 0 for static objects
    if ( mpImplMapMode->mnRefCount )
        mpImplMapMode->mnRefCount++;
}

MapMode::MapMode( MapUnit eUnit )
{

    mpImplMapMode = ImplMapMode::ImplGetStaticMapMode( eUnit );
}

MapMode::MapMode( MapUnit eUnit, const Point& rLogicOrg,
                  const Fraction& rScaleX, const Fraction& rScaleY )
{

    mpImplMapMode           = new ImplMapMode;
    mpImplMapMode->meUnit   = eUnit;
    mpImplMapMode->maOrigin = rLogicOrg;
    mpImplMapMode->maScaleX = rScaleX;
    mpImplMapMode->maScaleY = rScaleY;
}

MapMode::~MapMode()
{

    // If it's not static ImpData and it's the last reference, delete it,
    // else decrement refcounter
    if ( mpImplMapMode->mnRefCount )
    {
        if ( mpImplMapMode->mnRefCount == 1 )
            delete mpImplMapMode;
        else
            mpImplMapMode->mnRefCount--;
    }
}

void MapMode::SetMapUnit( MapUnit eUnit )
{

    ImplMakeUnique();
    mpImplMapMode->meUnit = eUnit;
}

void MapMode::SetOrigin( const Point& rLogicOrg )
{

    ImplMakeUnique();
    mpImplMapMode->maOrigin = rLogicOrg;
}

void MapMode::SetScaleX( const Fraction& rScaleX )
{

    ImplMakeUnique();
    mpImplMapMode->maScaleX = rScaleX;
}

void MapMode::SetScaleY( const Fraction& rScaleY )
{

    ImplMakeUnique();
    mpImplMapMode->maScaleY = rScaleY;
}

MapMode& MapMode::operator=( const MapMode& rMapMode )
{
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFFFFFE, "MapMode: RefCount overflow" );

    // First increment refcount so that we can reference ourselves
    // RefCount == 0 for static objects
    if ( rMapMode.mpImplMapMode->mnRefCount )
        rMapMode.mpImplMapMode->mnRefCount++;

    // If it's not static ImpData and it's the last reference, delete it,
    // else decrement refcounter
    if ( mpImplMapMode->mnRefCount )
    {
        if ( mpImplMapMode->mnRefCount == 1 )
            delete mpImplMapMode;
        else
            mpImplMapMode->mnRefCount--;
    }

    mpImplMapMode = rMapMode.mpImplMapMode;

    return *this;
}

bool MapMode::operator==( const MapMode& rMapMode ) const
{

    if ( mpImplMapMode == rMapMode.mpImplMapMode )
        return true;

    if ( (mpImplMapMode->meUnit   == rMapMode.mpImplMapMode->meUnit)   &&
         (mpImplMapMode->maOrigin == rMapMode.mpImplMapMode->maOrigin) &&
         (mpImplMapMode->maScaleX == rMapMode.mpImplMapMode->maScaleX) &&
         (mpImplMapMode->maScaleY == rMapMode.mpImplMapMode->maScaleY) )
        return true;
    else
        return false;
}

bool MapMode::IsDefault() const
{

    ImplMapMode* pDefMapMode = ImplMapMode::ImplGetStaticMapMode( MAP_PIXEL );
    if ( mpImplMapMode == pDefMapMode )
        return true;

    if ( (mpImplMapMode->meUnit   == pDefMapMode->meUnit)   &&
         (mpImplMapMode->maOrigin == pDefMapMode->maOrigin) &&
         (mpImplMapMode->maScaleX == pDefMapMode->maScaleX) &&
         (mpImplMapMode->maScaleY == pDefMapMode->maScaleY) )
        return true;
    else
        return false;
}

SvStream& ReadMapMode( SvStream& rIStm, MapMode& rMapMode )
{
    rMapMode.ImplMakeUnique();
    return ReadImplMapMode( rIStm, *rMapMode.mpImplMapMode );
}

SvStream& WriteMapMode( SvStream& rOStm, const MapMode& rMapMode )
{
    return WriteImplMapMode( rOStm, *rMapMode.mpImplMapMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
