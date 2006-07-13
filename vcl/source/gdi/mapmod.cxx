/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mapmod.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 10:24:20 $
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

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_MAPMOD_HXX
#include <mapmod.hxx>
#endif

// =======================================================================

DBG_NAME( MapMode )

// -----------------------------------------------------------------------

ImplMapMode::ImplMapMode() :
    maOrigin( 0, 0 ),
    maScaleX( 1, 1 ),
    maScaleY( 1, 1 )
{
    mnRefCount  = 1;
    meUnit      = MAP_PIXEL;
    mbSimple    = FALSE;
}

// -----------------------------------------------------------------------

ImplMapMode::ImplMapMode( const ImplMapMode& rImplMapMode ) :
    maOrigin( rImplMapMode.maOrigin ),
    maScaleX( rImplMapMode.maScaleX ),
    maScaleY( rImplMapMode.maScaleY )
{
    mnRefCount      = 1;
    meUnit          = rImplMapMode.meUnit;
    mbSimple        = FALSE;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplMapMode& rImplMapMode )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;

    rIStm >> nTmp16; rImplMapMode.meUnit = (MapUnit) nTmp16;
    rIStm >> rImplMapMode.maOrigin >> rImplMapMode.maScaleX >>
             rImplMapMode.maScaleY >> rImplMapMode.mbSimple;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplMapMode& rImplMapMode )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (UINT16) rImplMapMode.meUnit <<
             rImplMapMode.maOrigin <<
             rImplMapMode.maScaleX <<
             rImplMapMode.maScaleY <<
             rImplMapMode.mbSimple;

    return rOStm;
}

// -----------------------------------------------------------------------

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
        pImplMapMode->mbSimple  = TRUE;
    }

    return pImplMapMode;
}

// -----------------------------------------------------------------------

inline void MapMode::ImplMakeUnique()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpImplMapMode->mnRefCount != 1 )
    {
        if ( mpImplMapMode->mnRefCount )
            mpImplMapMode->mnRefCount--;
        mpImplMapMode = new ImplMapMode( *mpImplMapMode );
    }
}

// -----------------------------------------------------------------------

MapMode::MapMode()
{
    DBG_CTOR( MapMode, NULL );

    mpImplMapMode = ImplMapMode::ImplGetStaticMapMode( MAP_PIXEL );
}

// -----------------------------------------------------------------------

MapMode::MapMode( const MapMode& rMapMode )
{
    DBG_CTOR( MapMode, NULL );
    DBG_CHKOBJ( &rMapMode, MapMode, NULL );
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFFFFFE, "MapMode: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpImplMapMode = rMapMode.mpImplMapMode;
    // RefCount == 0 fuer statische Objekte
    if ( mpImplMapMode->mnRefCount )
        mpImplMapMode->mnRefCount++;
}

// -----------------------------------------------------------------------

MapMode::MapMode( MapUnit eUnit )
{
    DBG_CTOR( MapMode, NULL );

    mpImplMapMode = ImplMapMode::ImplGetStaticMapMode( eUnit );
}

// -----------------------------------------------------------------------

MapMode::MapMode( MapUnit eUnit, const Point& rLogicOrg,
                  const Fraction& rScaleX, const Fraction& rScaleY )
{
    DBG_CTOR( MapMode, NULL );

    mpImplMapMode           = new ImplMapMode;
    mpImplMapMode->meUnit   = eUnit;
    mpImplMapMode->maOrigin = rLogicOrg;
    mpImplMapMode->maScaleX = rScaleX;
    mpImplMapMode->maScaleY = rScaleY;
}

// -----------------------------------------------------------------------

MapMode::~MapMode()
{
    DBG_DTOR( MapMode, NULL );

    // Wenn es keine statischen ImpDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
    if ( mpImplMapMode->mnRefCount )
    {
        if ( mpImplMapMode->mnRefCount == 1 )
            delete mpImplMapMode;
        else
            mpImplMapMode->mnRefCount--;
    }
}

// -----------------------------------------------------------------------

void MapMode::SetMapUnit( MapUnit eUnit )
{
    DBG_CHKTHIS( MapMode, NULL );

    ImplMakeUnique();
    mpImplMapMode->meUnit = eUnit;
}

// -----------------------------------------------------------------------

void MapMode::SetOrigin( const Point& rLogicOrg )
{
    DBG_CHKTHIS( MapMode, NULL );

    ImplMakeUnique();
    mpImplMapMode->maOrigin = rLogicOrg;
}

// -----------------------------------------------------------------------

void MapMode::SetScaleX( const Fraction& rScaleX )
{
    DBG_CHKTHIS( MapMode, NULL );

    ImplMakeUnique();
    mpImplMapMode->maScaleX = rScaleX;
}

// -----------------------------------------------------------------------

void MapMode::SetScaleY( const Fraction& rScaleY )
{
    DBG_CHKTHIS( MapMode, NULL );

    ImplMakeUnique();
    mpImplMapMode->maScaleY = rScaleY;
}

// -----------------------------------------------------------------------

MapMode& MapMode::operator=( const MapMode& rMapMode )
{
    DBG_CHKTHIS( MapMode, NULL );
    DBG_CHKOBJ( &rMapMode, MapMode, NULL );
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFFFFFE, "MapMode: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    // RefCount == 0 fuer statische Objekte
    if ( rMapMode.mpImplMapMode->mnRefCount )
        rMapMode.mpImplMapMode->mnRefCount++;

    // Wenn es keine statischen ImpDaten sind, dann loeschen, wenn es
    // die letzte Referenz ist, sonst Referenzcounter decrementieren
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

// -----------------------------------------------------------------------

BOOL MapMode::operator==( const MapMode& rMapMode ) const
{
    DBG_CHKTHIS( MapMode, NULL );
    DBG_CHKOBJ( &rMapMode, MapMode, NULL );

    if ( mpImplMapMode == rMapMode.mpImplMapMode )
        return TRUE;

    if ( (mpImplMapMode->meUnit   == rMapMode.mpImplMapMode->meUnit)   &&
         (mpImplMapMode->maOrigin == rMapMode.mpImplMapMode->maOrigin) &&
         (mpImplMapMode->maScaleX == rMapMode.mpImplMapMode->maScaleX) &&
         (mpImplMapMode->maScaleY == rMapMode.mpImplMapMode->maScaleY) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL MapMode::IsDefault() const
{
    DBG_CHKTHIS( MapMode, NULL );

    ImplMapMode* pDefMapMode = ImplMapMode::ImplGetStaticMapMode( MAP_PIXEL );
    if ( mpImplMapMode == pDefMapMode )
        return TRUE;

    if ( (mpImplMapMode->meUnit   == pDefMapMode->meUnit)   &&
         (mpImplMapMode->maOrigin == pDefMapMode->maOrigin) &&
         (mpImplMapMode->maScaleX == pDefMapMode->maScaleX) &&
         (mpImplMapMode->maScaleY == pDefMapMode->maScaleY) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, MapMode& rMapMode )
{
    rMapMode.ImplMakeUnique();
    return (rIStm >> *rMapMode.mpImplMapMode);
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const MapMode& rMapMode )
{
    return (rOStm << *rMapMode.mpImplMapMode);
}
