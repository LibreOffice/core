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

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <tools/mapunit.hxx>

class SvStream;

// --------------
// - ImplMapMode -
// --------------

class   OutputDevice;

class ImplMapMode
{
    friend class    MapMode;
    friend class    OutputDevice;

private:
    sal_uLong           mnRefCount;
    MapUnit         meUnit;
    Point           maOrigin;
    Fraction        maScaleX;
    Fraction        maScaleY;
    bool            mbSimple;

    friend SvStream& ReadImplMapMode( SvStream& rIStm, ImplMapMode& rMapMode );
    friend SvStream& WriteImplMapMode( SvStream& rOStm, const ImplMapMode& rMapMode );

    static ImplMapMode* ImplGetStaticMapMode( MapUnit eUnit );
public:
                    ImplMapMode();
                    ImplMapMode( const ImplMapMode& rImpMapMode );
};

// -----------
// - MapMode -
// -----------

class VCL_DLLPUBLIC MapMode
{
    friend class        OutputDevice;

private:
    ImplMapMode*        mpImplMapMode;

    SAL_DLLPRIVATE void ImplMakeUnique();

public:
                    MapMode();
                    MapMode( const MapMode& rMapMode );
                    MapMode( MapUnit eUnit );
                    MapMode( MapUnit eUnit, const Point& rLogicOrg,
                             const Fraction& rScaleX, const Fraction& rScaleY );
                    ~MapMode();

    void            SetMapUnit( MapUnit eUnit );
    MapUnit         GetMapUnit() const
                        { return mpImplMapMode->meUnit; }

    void            SetOrigin( const Point& rOrigin );
    const Point&    GetOrigin() const
                        { return mpImplMapMode->maOrigin; }

    void            SetScaleX( const Fraction& rScaleX );
    const Fraction& GetScaleX() const
                        { return mpImplMapMode->maScaleX; }
    void            SetScaleY( const Fraction& rScaleY );
    const Fraction& GetScaleY() const
                        { return mpImplMapMode->maScaleY; }

    MapMode&        operator=( const MapMode& rMapMode );
    bool            operator==( const MapMode& rMapMode ) const;
    bool            operator!=( const MapMode& rMapMode ) const
                        { return !(MapMode::operator==( rMapMode )); }
    bool            IsDefault() const;
    bool            IsSameInstance( const MapMode& rMapMode ) const
                        { return (mpImplMapMode == rMapMode.mpImplMapMode); }

    friend VCL_DLLPUBLIC SvStream& ReadMapMode( SvStream& rIStm, MapMode& rMapMode );
    friend VCL_DLLPUBLIC SvStream& WriteMapMode( SvStream& rOStm, const MapMode& rMapMode );
};

#endif // INCLUDED_VCL_MAPMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
