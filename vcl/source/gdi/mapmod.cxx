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
#include <vcl/TypeSerializer.hxx>

struct MapMode::ImplMapMode
{
    Point           maOrigin;
    MapUnit         meUnit;
    // NOTE: these Fraction must NOT have more than 32 bits precision
    // because ReadFraction / WriteFraction do only 32 bits, so more than
    // that cannot be stored in MetaFiles!
    // => call ReduceInaccurate whenever setting these
    Fraction        maScaleX;
    Fraction        maScaleY;
    bool            mbSimple;

    ImplMapMode();
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
}

MapMode::MapMode() : mpImpl(GetGlobalDefault())
{
}

MapMode::MapMode( const MapMode& ) = default;

MapMode::MapMode( MapUnit eUnit )
{
    mpImpl->meUnit = eUnit;
}

MapMode::MapMode( MapUnit eUnit, const Point& rLogicOrg,
                  const Fraction& rScaleX, const Fraction& rScaleY )
{
    mpImpl->meUnit   = eUnit;
    mpImpl->maOrigin = rLogicOrg;
    mpImpl->maScaleX = rScaleX;
    mpImpl->maScaleY = rScaleY;
    mpImpl->maScaleX.ReduceInaccurate(32);
    mpImpl->maScaleY.ReduceInaccurate(32);
    mpImpl->mbSimple = false;
}

MapMode::MapMode(std::nullopt_t) noexcept : mpImpl(std::nullopt)
{
}

MapMode::~MapMode() = default;

void MapMode::SetMapUnit( MapUnit eUnit )
{
    mpImpl->meUnit = eUnit;
}

void MapMode::SetOrigin( const Point& rLogicOrg )
{
    mpImpl->maOrigin = rLogicOrg;
    mpImpl->mbSimple = false;
}

void MapMode::SetScaleX( const Fraction& rScaleX )
{
    mpImpl->maScaleX = rScaleX;
    mpImpl->maScaleX.ReduceInaccurate(32);
    mpImpl->mbSimple = false;
}

void MapMode::SetScaleY( const Fraction& rScaleY )
{
    mpImpl->maScaleY = rScaleY;
    mpImpl->maScaleY.ReduceInaccurate(32);
    mpImpl->mbSimple = false;
}

MapMode& MapMode::operator=( const MapMode& ) = default;

MapMode& MapMode::operator=( MapMode&& ) = default;

bool MapMode::operator==( const MapMode& rMapMode ) const
{
   return mpImpl == rMapMode.mpImpl;
}

bool MapMode::IsDefault() const
{
    return mpImpl.same_object(GetGlobalDefault());
}

MapUnit MapMode::GetMapUnit() const { return mpImpl->meUnit; }

const Point& MapMode::GetOrigin() const { return mpImpl->maOrigin; }

const Fraction& MapMode::GetScaleX() const { return mpImpl->maScaleX; }

const Fraction& MapMode::GetScaleY() const { return mpImpl->maScaleY; }

bool MapMode::IsSimple() const { return mpImpl->mbSimple; }

std::optional<MapMode>::~optional() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
