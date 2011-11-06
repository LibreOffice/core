/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>
#ifndef _SV_HATCX_HXX
#include <vcl/hatch.hxx>
#endif

DBG_NAME( Hatch )

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
              long nDistance, sal_uInt16 nAngle10 )
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

sal_Bool Hatch::operator==( const Hatch& rHatch ) const
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

void Hatch::SetAngle( sal_uInt16 nAngle10 )
{
    DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->mnAngle = nAngle10;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplHatch& rImplHatch )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;

    rIStm >> nTmp16; rImplHatch.meStyle = (HatchStyle) nTmp16;
    rIStm >> rImplHatch.maColor >> rImplHatch.mnDistance >> rImplHatch.mnAngle;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplHatch& rImplHatch )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (sal_uInt16) rImplHatch.meStyle << rImplHatch.maColor;
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
