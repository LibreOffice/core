/*************************************************************************
 *
 *  $RCSfile: mapmod.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_MAPMOD_CXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#define private public
#ifndef _SV_MAPMOD_HXX
#include <mapmod.hxx>
#endif
#undef private

// =======================================================================

DBG_NAME( MapMode );

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

static ImplMapMode* ImplGetStaticMapMode( MapUnit eUnit )
{
    // Achtung: Ganz fies und dreckig !!!
#ifdef WIN
    static long _near aStaticImplMapModeAry[(MAP_LASTENUMDUMMY)*sizeof(ImplMapMode)/sizeof(long)];
#else
    static long aStaticImplMapModeAry[(MAP_LASTENUMDUMMY)*sizeof(ImplMapMode)/sizeof(long)];
#endif

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

    mpImplMapMode = ImplGetStaticMapMode( MAP_PIXEL );
}

// -----------------------------------------------------------------------

MapMode::MapMode( const MapMode& rMapMode )
{
    DBG_CTOR( MapMode, NULL );
    DBG_CHKOBJ( &rMapMode, MapMode, NULL );
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFE, "MapMode: RefCount overflow" );

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

    mpImplMapMode = ImplGetStaticMapMode( eUnit );
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
    DBG_ASSERT( rMapMode.mpImplMapMode->mnRefCount < 0xFFFE, "MapMode: RefCount overflow" );

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

    ImplMapMode* pDefMapMode = ImplGetStaticMapMode( MAP_PIXEL );
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
