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

#include <o3tl/hash_combine.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/TypeSerializer.hxx>

struct MapMode::ImplMapMode
{
    MapUnit         meUnit;
    bool            mbSimple;
    Point           maOrigin;
    // NOTE: these Fraction must NOT have more than 32 bits precision
    // because ReadFraction / WriteFraction do only 32 bits, so more than
    // that cannot be stored in MetaFiles!
    // => call ReduceInaccurate whenever setting these
    Fraction        maScaleX;
    Fraction        maScaleY;

    ImplMapMode();
    ImplMapMode(MapUnit eMapUnit);
    ImplMapMode(const ImplMapMode& rImpMapMode);

    bool operator==( const ImplMapMode& rImpMapMode ) const;
};

MapMode::ImplMapMode::ImplMapMode() :
    maOrigin( 0, 0 ),
    maScaleX( 1, 1 ),
    maScaleY( 1, 1 )
{
    meUnit   = MapUnit::MapPixel;
    mbSimple = true;
}

MapMode::ImplMapMode::ImplMapMode(MapUnit eMapUnit) :
    maOrigin( 0, 0 ),
    maScaleX( 1, 1 ),
    maScaleY( 1, 1 )
{
    meUnit   = eMapUnit;
    mbSimple = true;
}

MapMode::ImplMapMode::ImplMapMode( const ImplMapMode& ) = default;

bool MapMode::ImplMapMode::operator==( const ImplMapMode& rImpMapMode ) const
{
    return meUnit == rImpMapMode.meUnit
        && maOrigin == rImpMapMode.maOrigin
        && maScaleX == rImpMapMode.maScaleX
        && maScaleY == rImpMapMode.maScaleY;
}

namespace
{

    MapMode::ImplType& GetGlobalDefault()
    {
        static MapMode::ImplType gDefault;
        return gDefault;
    }
    MapMode::ImplType GetUnitDefault(MapUnit mapUnit)
    {
        static const MapMode::ImplType defaults[]
        {
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::Map100thMM) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::Map10thMM) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapMM) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapCM) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::Map1000thInch) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::Map100thInch) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::Map10thInch) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapInch) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapPoint) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapTwip) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapPixel) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapSysFont) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapAppFont) ),
            MapMode::ImplType( MapMode::ImplMapMode(MapUnit::MapRelative) ),
        };
        if (mapUnit <= MapUnit::MapRelative) {
            return MapMode::ImplType(defaults[static_cast<int>(mapUnit)]); // [-loplugin:redundantfcast] false positive
        }
        // sometimes the SvmReader stuff will generate a bogus mapunit value
        return MapMode::ImplType(MapMode::ImplMapMode(mapUnit));
    }
}

MapMode::MapMode() : mpImplMapMode(GetGlobalDefault())
{
}

MapMode::MapMode( const MapMode& ) = default;

MapMode::MapMode( MapUnit eUnit ) : mpImplMapMode(GetUnitDefault(eUnit))
{
}

MapMode::MapMode( MapUnit eUnit, const Point& rLogicOrg,
                  const Fraction& rScaleX, const Fraction& rScaleY )
{
    mpImplMapMode->meUnit   = eUnit;
    mpImplMapMode->maOrigin = rLogicOrg;
    mpImplMapMode->maScaleX = rScaleX;
    mpImplMapMode->maScaleY = rScaleY;
    mpImplMapMode->mbSimple = false;
}

MapMode::~MapMode() = default;

void MapMode::SetMapUnit( MapUnit eUnit )
{
    mpImplMapMode->meUnit = eUnit;
}

void MapMode::SetOrigin( const Point& rLogicOrg )
{
    mpImplMapMode->maOrigin = rLogicOrg;
    mpImplMapMode->mbSimple = false;
}

void MapMode::SetScaleX( const Fraction& rScaleX )
{
    mpImplMapMode->maScaleX = rScaleX;
    mpImplMapMode->mbSimple = false;
}

void MapMode::SetScaleY( const Fraction& rScaleY )
{
    mpImplMapMode->maScaleY = rScaleY;
    mpImplMapMode->mbSimple = false;
}

MapMode& MapMode::operator=( const MapMode& ) = default;

MapMode& MapMode::operator=( MapMode&& ) = default;

bool MapMode::operator==( const MapMode& rMapMode ) const
{
   return mpImplMapMode == rMapMode.mpImplMapMode;
}

bool MapMode::IsDefault() const
{
    return mpImplMapMode.same_object(GetGlobalDefault());
}

size_t MapMode::GetHashValue() const
{
    size_t hash = 0;
    o3tl::hash_combine( hash, mpImplMapMode->meUnit );
    o3tl::hash_combine( hash, mpImplMapMode->maOrigin.GetHashValue());
    o3tl::hash_combine( hash, mpImplMapMode->maScaleX.GetHashValue());
    o3tl::hash_combine( hash, mpImplMapMode->maScaleY.GetHashValue());
    o3tl::hash_combine( hash, mpImplMapMode->mbSimple );
    return hash;
}

MapUnit MapMode::GetMapUnit() const { return mpImplMapMode->meUnit; }

const Point& MapMode::GetOrigin() const { return mpImplMapMode->maOrigin; }

const Fraction& MapMode::GetScaleX() const { return mpImplMapMode->maScaleX; }

const Fraction& MapMode::GetScaleY() const { return mpImplMapMode->maScaleY; }

bool MapMode::IsSimple() const { return mpImplMapMode->mbSimple; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
