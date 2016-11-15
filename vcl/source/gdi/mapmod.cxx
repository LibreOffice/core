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

#include <vcl/mapmod.hxx>

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>


// - ImplMapMode -

struct MapMode::ImplMapMode
{
    sal_uLong       mnRefCount;
    MapUnit         meUnit;
    Point           maOrigin;
    // NOTE: these Fraction must NOT have more than 32 bits precision
    // because ReadFraction / WriteFraction do only 32 bits, so more than
    // that cannot be stored in MetaFiles!
    // => call ReduceInaccurate whenever setting these
    Fraction        maScaleX;
    Fraction        maScaleY;
    bool            mbSimple;

    friend SvStream& ReadImplMapMode(SvStream& rIStm, ImplMapMode& rMapMode);
    friend SvStream& WriteImplMapMode(SvStream& rOStm, const ImplMapMode& rMapMode);

    static ImplMapMode* ImplGetStaticMapMode( MapUnit eUnit );

    ImplMapMode();
    ImplMapMode(const ImplMapMode& rImpMapMode);
};

MapMode::ImplMapMode::ImplMapMode() :
    maOrigin( 0, 0 ),
    maScaleX( 1, 1 ),
    maScaleY( 1, 1 )
{
    mnRefCount  = 1;
    meUnit      = MAP_PIXEL;
    mbSimple    = false;
}

MapMode::ImplMapMode::ImplMapMode( const ImplMapMode& rImplMapMode ) :
    maOrigin( rImplMapMode.maOrigin ),
    maScaleX( rImplMapMode.maScaleX ),
    maScaleY( rImplMapMode.maScaleY )
{
    mnRefCount      = 1;
    meUnit          = rImplMapMode.meUnit;
    mbSimple        = false;
}

SvStream& ReadImplMapMode(SvStream& rIStm, MapMode::ImplMapMode& rImplMapMode)
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;

    rIStm.ReadUInt16( nTmp16 ); rImplMapMode.meUnit = (MapUnit) nTmp16;
    ReadPair( rIStm, rImplMapMode.maOrigin );
    ReadFraction( rIStm, rImplMapMode.maScaleX );
    ReadFraction( rIStm, rImplMapMode.maScaleY );
    rIStm.ReadCharAsBool( rImplMapMode.mbSimple );

    return rIStm;
}

SvStream& WriteImplMapMode(SvStream& rOStm, const MapMode::ImplMapMode& rImplMapMode)
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 1 );

    rOStm.WriteUInt16( rImplMapMode.meUnit );
    WritePair( rOStm, rImplMapMode.maOrigin );
    WriteFraction( rOStm, rImplMapMode.maScaleX );
    WriteFraction( rOStm, rImplMapMode.maScaleY );
    rOStm.WriteBool( rImplMapMode.mbSimple );

    return rOStm;
}

MapMode::ImplMapMode *
MapMode::ImplMapMode::ImplGetStaticMapMode(MapUnit eUnit)
{
    static long aStaticImplMapModeAry[(MAP_LASTENUMDUMMY)*sizeof(ImplMapMode)/sizeof(long)];

    // #i19496 check for out-of-bounds
     if( eUnit >= MAP_LASTENUMDUMMY )
        return reinterpret_cast<ImplMapMode*>(aStaticImplMapModeAry);

    ImplMapMode* pImplMapMode = reinterpret_cast<ImplMapMode*>(aStaticImplMapModeAry)+eUnit;
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
    mpImplMapMode->maScaleX.ReduceInaccurate(32);
    mpImplMapMode->maScaleY.ReduceInaccurate(32);
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
    mpImplMapMode->mbSimple = false;
}

void MapMode::SetScaleX( const Fraction& rScaleX )
{

    ImplMakeUnique();
    mpImplMapMode->maScaleX = rScaleX;
    mpImplMapMode->maScaleX.ReduceInaccurate(32);
    mpImplMapMode->mbSimple = false;
}

void MapMode::SetScaleY( const Fraction& rScaleY )
{

    ImplMakeUnique();
    mpImplMapMode->maScaleY = rScaleY;
    mpImplMapMode->maScaleY.ReduceInaccurate(32);
    mpImplMapMode->mbSimple = false;
}

double MapMode::GetUnitMultiplier() const
{
    double  nMul;
    switch ( GetMapUnit() )
    {
        case MAP_PIXEL :
        case MAP_SYSFONT :
        case MAP_APPFONT :

        case MAP_100TH_MM :
            nMul = 1;
            break;
        case MAP_10TH_MM :
            nMul = 10;
            break;
        case MAP_MM :
            nMul = 100;
            break;
        case MAP_CM :
            nMul = 1000;
            break;
        case MAP_1000TH_INCH :
            nMul = 2.54;
            break;
        case MAP_100TH_INCH :
            nMul = 25.4;
            break;
        case MAP_10TH_INCH :
            nMul = 254;
            break;
        case MAP_INCH :
            nMul = 2540;
            break;
        case MAP_TWIP :
            nMul = 1.76388889;
            break;
        case MAP_POINT :
            nMul = 35.27777778;
            break;
        default:
            nMul = 1.0;
            break;
    }
    return nMul;
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


MapUnit MapMode::GetMapUnit() const { return mpImplMapMode->meUnit; }

const Point& MapMode::GetOrigin() const { return mpImplMapMode->maOrigin; }

const Fraction& MapMode::GetScaleX() const { return mpImplMapMode->maScaleX; }

const Fraction& MapMode::GetScaleY() const { return mpImplMapMode->maScaleY; }

bool MapMode::IsSimple() const { return mpImplMapMode->mbSimple; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
