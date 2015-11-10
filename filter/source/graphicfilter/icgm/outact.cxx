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


#include <outact.hxx>
#include <vcl/gradient.hxx>

using namespace ::com::sun::star;

CGMOutAct::CGMOutAct( CGM& rCGM )
{
    mpCGM = &rCGM;
    mnCurrentPage = 0;
    mnGroupActCount = mnGroupLevel = 0;
    mpGroupLevel = new sal_uInt32[CGM_OUTACT_MAX_GROUP_LEVEL] ();
    mpPoints = reinterpret_cast<Point*>(new sal_Int8[ 0x2000 * sizeof( Point ) ]);
    mpFlags = new sal_uInt8[ 0x2000 ];

    mnIndex = 0;
    mpGradient = nullptr;
}

CGMOutAct::~CGMOutAct()
{
    delete[] reinterpret_cast<sal_Int8*>(mpPoints);
    delete[] mpFlags;
    delete[] mpGroupLevel;
    delete mpGradient;
}

void CGMOutAct::BeginFigure()
{
    if ( mnIndex )
        EndFigure();

    BeginGroup();
    mnIndex = 0;
}

void CGMOutAct::CloseRegion()
{
    if ( mnIndex > 2 )
    {
        NewRegion();
        DrawPolyPolygon( maPolyPolygon );
        maPolyPolygon.Clear();
    }
}

void CGMOutAct::NewRegion()
{
    if ( mnIndex > 2 )
    {
        tools::Polygon aPolygon( mnIndex, mpPoints, mpFlags );
        maPolyPolygon.Insert( aPolygon );
    }
    mnIndex = 0;
}

void CGMOutAct::EndFigure()
{
    NewRegion();
    DrawPolyPolygon( maPolyPolygon );
    maPolyPolygon.Clear();
    EndGroup();
    mnIndex = 0;
}

void CGMOutAct::RegPolyLine( tools::Polygon& rPolygon, bool bReverse )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    if ( nPoints )
    {
        if ( bReverse )
        {
            for ( sal_uInt16 i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( nPoints - i - 1 );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( nPoints - i - 1 );
            }
        }
        else
        {
            for ( sal_uInt16 i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( i );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( i );
            }
        }
        mnIndex = mnIndex + nPoints;
    }
}

void CGMOutAct::SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 /*nType*/ )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->XOffset = ( (sal_uInt16)nHorzOfs & 0x7f );
    mpGradient->YOffset = ( (sal_uInt16)nVertOfs & 0x7f );
}

void CGMOutAct::SetGradientAngle( long nAngle )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->Angle = sal::static_int_cast< sal_Int16 >(nAngle);
}

void CGMOutAct::SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->StartColor = nColorFrom;
    mpGradient->EndColor = nColorTo;
}

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
            mpGradient->Style = awt::GradientStyle_RADIAL;          // CONICAL
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
