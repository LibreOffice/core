/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mapmod.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_MAPMOD_HXX
#define _SV_MAPMOD_HXX

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <vcl/mapunit.hxx>

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
    ULONG           mnRefCount;
    MapUnit         meUnit;
    Point           maOrigin;
    Fraction        maScaleX;
    Fraction        maScaleY;
    BOOL            mbSimple;

    friend SvStream& operator>>( SvStream& rIStm, ImplMapMode& rMapMode );
    friend SvStream& operator<<( SvStream& rOStm, const ImplMapMode& rMapMode );

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
    BOOL            operator==( const MapMode& rMapMode ) const;
    BOOL            operator!=( const MapMode& rMapMode ) const
                        { return !(MapMode::operator==( rMapMode )); }
    BOOL            IsDefault() const;
    BOOL            IsSameInstance( const MapMode& rMapMode ) const
                        { return (mpImplMapMode == rMapMode.mpImplMapMode); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, MapMode& rMapMode );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const MapMode& rMapMode );
};

#endif  // _SV_MAPMOD_HXX
