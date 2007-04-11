/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mapmod.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:59:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_MAPMOD_HXX
#define _SV_MAPMOD_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

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
