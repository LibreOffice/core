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

#ifndef INCLUDED_VCL_MAPMOD_HXX
#define INCLUDED_VCL_MAPMOD_HXX

#include <vcl/dllapi.h>
#include <tools/mapunit.hxx>
#include <o3tl/cow_wrapper.hxx>

#include <ostream>

class Point;
class Fraction;
class SvStream;

class SAL_WARN_UNUSED VCL_DLLPUBLIC MapMode
{
    friend class        OutputDevice;

public:
    struct SAL_DLLPRIVATE ImplMapMode;

    MapMode();
    MapMode( const MapMode& rMapMode );
    explicit MapMode( MapUnit eUnit );
    MapMode( MapUnit eUnit, const Point& rLogicOrg,
        const Fraction& rScaleX, const Fraction& rScaleY );
    ~MapMode();

    void            SetMapUnit( MapUnit eUnit );
    MapUnit         GetMapUnit() const;

    void            SetOrigin( const Point& rOrigin );
    const Point&    GetOrigin() const;

    void            SetScaleX( const Fraction& rScaleX );
    const Fraction& GetScaleX() const;
    void            SetScaleY( const Fraction& rScaleY );
    const Fraction& GetScaleY() const;

    MapMode&        operator=( const MapMode& rMapMode );
    MapMode&        operator=( MapMode&& rMapMode );
    bool            operator==( const MapMode& rMapMode ) const;
    bool            operator!=( const MapMode& rMapMode ) const
    {
        return !(MapMode::operator==( rMapMode ));
    }
    bool            IsDefault() const;

    // tdf#117984 needs to be thread-safe due to being used e.g. in Bitmaps
    // vcl::ScopedBitmapAccess in parallelized 3D renderer
    typedef o3tl::cow_wrapper< ImplMapMode, o3tl::ThreadSafeRefCountingPolicy > ImplType;

    // If only the map unit is set.
    bool IsSimple() const;

private:
    ImplType        mpImplMapMode;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & rStream, const MapMode& rMode)
{
    rStream << "MapMode(" << static_cast<unsigned>(rMode.GetMapUnit()) << ",(" << rMode.GetScaleX() << "," << rMode.GetScaleY() << ")@(" << rMode.GetOrigin() << "))";
    return rStream;
}

#endif // INCLUDED_VCL_MAPMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
