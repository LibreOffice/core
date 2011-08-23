/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <outact.hxx>
#include <vcl/gradient.hxx>

using namespace ::com::sun::star;

// ---------------------------------------------------------------

CGMOutAct::CGMOutAct( CGM& rCGM )
{
    mpCGM = &rCGM;
    mnCurrentPage = 0;
    mnGroupActCount = mnGroupLevel = 0;
    mpGroupLevel = new sal_uInt32[ CGM_OUTACT_MAX_GROUP_LEVEL ];
    mpPoints = (Point*)new sal_Int8[ 0x2000 * sizeof( Point ) ];
    mpFlags = new BYTE[ 0x2000 ];

    mnIndex = 0;
    mpGradient = NULL;
};

// ---------------------------------------------------------------

CGMOutAct::~CGMOutAct()
{
    delete[] (sal_Int8*) mpPoints;
    delete[] mpFlags;
    delete[] mpGroupLevel;

    if ( mpGradient )
        delete mpGradient;
};

// ---------------------------------------------------------------

void CGMOutAct::BeginFigure()
{
    if ( mnIndex )
        EndFigure();

    BeginGroup();
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::CloseRegion()
{
    if ( mnIndex > 2 )
    {
        NewRegion();
        DrawPolyPolygon( maPolyPolygon );
        maPolyPolygon.Clear();
    }
}

// ---------------------------------------------------------------

void CGMOutAct::NewRegion()
{
    if ( mnIndex > 2 )
    {
        Polygon aPolygon( mnIndex, mpPoints, mpFlags );
        maPolyPolygon.Insert( aPolygon );
    }
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::EndFigure()
{
    NewRegion();
    DrawPolyPolygon( maPolyPolygon );
    maPolyPolygon.Clear();
    EndGroup();
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::RegPolyLine( Polygon& rPolygon, sal_Bool bReverse )
{
    USHORT nPoints = rPolygon.GetSize();
    if ( nPoints )
    {
        if ( bReverse )
        {
            for ( USHORT i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( nPoints - i - 1 );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( nPoints - i - 1 );
            }
        }
        else
        {
            for ( USHORT i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( i );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( i );
            }
        }
        mnIndex = mnIndex + nPoints;
    }
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 /*nType*/ )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->XOffset = ( (sal_uInt16)nHorzOfs & 0x7f );
    mpGradient->YOffset = ( (sal_uInt16)nVertOfs & 0x7f );
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientAngle( long nAngle )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->Angle = sal::static_int_cast< sal_Int16 >(nAngle);
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->StartColor = nColorFrom;
    mpGradient->EndColor = nColorTo;
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientStyle( sal_uInt32 nStyle, double /*fRatio*/ )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    switch ( nStyle )
    {
        case 0xff :
        {
            mpGradient->Style = awt::GradientStyle_AXIAL;
        }
        break;
        case 4 :
        {
            mpGradient->Style = awt::GradientStyle_RADIAL;			// CONICAL
        }
        break;
        case 3 :
        {
            mpGradient->Style = awt::GradientStyle_RECT;
        }
        break;
        case 2 :
        {
            mpGradient->Style = awt::GradientStyle_ELLIPTICAL;
        }
        break;
        default :
        {
            mpGradient->Style = awt::GradientStyle_LINEAR;
        }
    }
}


