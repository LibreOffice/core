/*************************************************************************
 *
 *  $RCSfile: hatch.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#define _SV_HATCH_CXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_HATCX_HXX
#include <hatch.hxx>
#endif

DBG_NAME( Hatch );

// --------------
// - ImplHatch -
// --------------

ImplHatch::ImplHatch() :
    mnRefCount  ( 1 ),
    maColor     ( COL_BLACK ),
    meStyle     ( HATCH_SINGLE ),
    mnDistance  ( 1 ),
    mnAngle     ( 0 )
{
}

// -----------------------------------------------------------------------

ImplHatch::ImplHatch( const ImplHatch& rImplHatch ) :
    mnRefCount  ( 1 ),
    maColor     ( rImplHatch.maColor ),
    meStyle     ( rImplHatch.meStyle ),
    mnDistance  ( rImplHatch.mnDistance ),
    mnAngle     ( rImplHatch.mnAngle )
{
}

// ---------
// - Hatch -
// ---------

Hatch::Hatch()
{
    DBG_CTOR( Hatch, NULL );
    mpImplHatch = new ImplHatch;
}

// -----------------------------------------------------------------------

Hatch::Hatch( const Hatch& rHatch )
{
    DBG_CTOR( Hatch, NULL );
    DBG_CHKOBJ( &rHatch, Hatch, NULL );
    mpImplHatch = rHatch.mpImplHatch;
    mpImplHatch->mnRefCount++;
}

// -----------------------------------------------------------------------

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
              long nDistance, USHORT nAngle10 )
{
    DBG_CTOR( Hatch, NULL );
    mpImplHatch = new ImplHatch;
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

// -----------------------------------------------------------------------

Hatch::~Hatch()
{
    DBG_DTOR( Hatch, NULL );
    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;
}

// -----------------------------------------------------------------------

Hatch& Hatch::operator=( const Hatch& rHatch )
{
    DBG_CHKTHIS( Hatch, NULL );
    DBG_CHKOBJ( &rHatch, Hatch, NULL );

    rHatch.mpImplHatch->mnRefCount++;

    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;

    mpImplHatch = rHatch.mpImplHatch;
    return *this;
}

// -----------------------------------------------------------------------

BOOL Hatch::operator==( const Hatch& rHatch ) const
{
    DBG_CHKTHIS( Hatch, NULL );
    DBG_CHKOBJ( &rHatch, Hatch, NULL );

    return( mpImplHatch == rHatch.mpImplHatch ||
            ( mpImplHatch->maColor == rHatch.mpImplHatch->maColor &&
              mpImplHatch->meStyle == rHatch.mpImplHatch->meStyle &&
              mpImplHatch->mnDistance == rHatch.mpImplHatch->mnDistance &&
              mpImplHatch->mnAngle == rHatch.mpImplHatch->mnAngle ) );
}

// -----------------------------------------------------------------------

void Hatch::ImplMakeUnique()
{
    if( mpImplHatch->mnRefCount != 1 )
    {
        if( mpImplHatch->mnRefCount )
            mpImplHatch->mnRefCount--;

        mpImplHatch = new ImplHatch( *mpImplHatch );
    }
}

// -----------------------------------------------------------------------

void Hatch::SetStyle( HatchStyle eStyle )
{
    DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->meStyle = eStyle;
}

// -----------------------------------------------------------------------

void Hatch::SetColor( const Color& rColor )
{
    DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->maColor = rColor;
}

// -----------------------------------------------------------------------

void Hatch::SetDistance( long nDistance )
{
    DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->mnDistance = nDistance;
}

// -----------------------------------------------------------------------

void Hatch::SetAngle( USHORT nAngle10 )
{
    DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->mnAngle = nAngle10;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplHatch& rImplHatch )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;

    rIStm >> nTmp16; rImplHatch.meStyle = (HatchStyle) nTmp16;
    rIStm >> rImplHatch.maColor >> rImplHatch.mnDistance >> rImplHatch.mnAngle;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplHatch& rImplHatch )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (UINT16) rImplHatch.meStyle << rImplHatch.maColor;
    rOStm << rImplHatch.mnDistance << rImplHatch.mnAngle;

    return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Hatch& rHatch )
{
    rHatch.ImplMakeUnique();
    return( rIStm >> *rHatch.mpImplHatch );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Hatch& rHatch )
{
    return( rOStm << *rHatch.mpImplHatch );
}
