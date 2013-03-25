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

#include <tools/vcompat.hxx>

#include <vcl/graphictools.hxx>

////////////////////////////////////////////////////////////////////////////

SvtGraphicFill::Transform::Transform()
{
    matrix[0] = 1.0; matrix[1] = 0.0; matrix[2] = 0.0;
    matrix[3] = 0.0; matrix[4] = 1.0; matrix[5] = 0.0;
}

////////////////////////////////////////////////////////////////////////////

SvtGraphicStroke::SvtGraphicStroke() :
    maPath(),
    maStartArrow(),
    maEndArrow(),
    mfTransparency(),
    mfStrokeWidth(),
    maCapType(),
    maJoinType(),
    mfMiterLimit( 3.0 ),
    maDashArray()
{
}

SvtGraphicStroke::SvtGraphicStroke( const Polygon&      rPath,
                                    const PolyPolygon&  rStartArrow,
                                    const PolyPolygon&  rEndArrow,
                                    double              fTransparency,
                                    double              fStrokeWidth,
                                    CapType             aCap,
                                    JoinType            aJoin,
                                    double              fMiterLimit,
                                    const DashArray&    rDashArray  ) :
    maPath( rPath ),
    maStartArrow( rStartArrow ),
    maEndArrow( rEndArrow ),
    mfTransparency( fTransparency ),
    mfStrokeWidth( fStrokeWidth ),
    maCapType( aCap ),
    maJoinType( aJoin ),
    mfMiterLimit( fMiterLimit ),
    maDashArray( rDashArray )
{
}

void SvtGraphicStroke::getPath( Polygon& rPath ) const
{
    rPath = maPath;
}

void SvtGraphicStroke::getStartArrow( PolyPolygon& rPath ) const
{
    rPath = maStartArrow;
}

void SvtGraphicStroke::getEndArrow( PolyPolygon& rPath ) const
{
    rPath = maEndArrow;
}

double SvtGraphicStroke::getTransparency() const
{
    return mfTransparency;
}

double SvtGraphicStroke::getStrokeWidth() const
{
    return mfStrokeWidth;
}

SvtGraphicStroke::CapType SvtGraphicStroke::getCapType() const
{
    return maCapType;
}

SvtGraphicStroke::JoinType SvtGraphicStroke::getJoinType() const
{
    return maJoinType;
}

double SvtGraphicStroke::getMiterLimit() const
{
    return mfMiterLimit;
}

void SvtGraphicStroke::getDashArray( DashArray& rDashArray ) const
{
    rDashArray = maDashArray;
}

void SvtGraphicStroke::setPath( const Polygon& rPoly )
{
    maPath = rPoly;
}

void SvtGraphicStroke::scale( double fXScale, double fYScale )
{
    // Clearly scaling stroke-width for fat lines is rather a problem
    maPath.Scale( fXScale, fYScale );

    double fScale = sqrt (fabs (fXScale * fYScale) ); // clearly not ideal.
    mfStrokeWidth *= fScale;
    mfMiterLimit *= fScale;

    maStartArrow.Scale( fXScale, fYScale );
    maEndArrow.Scale( fXScale, fYScale );
}

SvStream& operator<<( SvStream& rOStm, const SvtGraphicStroke& rClass )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rClass.maPath.Write( rOStm );
    rClass.maStartArrow.Write( rOStm );
    rClass.maEndArrow.Write( rOStm );
    rOStm << rClass.mfTransparency;
    rOStm << rClass.mfStrokeWidth;
    sal_uInt16 nTmp = sal::static_int_cast<sal_uInt16>( rClass.maCapType );
    rOStm << nTmp;
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maJoinType );
    rOStm << nTmp;
    rOStm << rClass.mfMiterLimit;

    rOStm << static_cast<sal_uInt32>(rClass.maDashArray.size());
    size_t i;
    for(i=0; i<rClass.maDashArray.size(); ++i)
        rOStm << rClass.maDashArray[i];

    return rOStm;
}

SvStream& operator>>( SvStream& rIStm, SvtGraphicStroke& rClass )
{
    VersionCompat aCompat( rIStm, STREAM_READ );

    rClass.maPath.Read( rIStm );
    rClass.maStartArrow.Read( rIStm );
    rClass.maEndArrow.Read( rIStm );
    rIStm >> rClass.mfTransparency;
    rIStm >> rClass.mfStrokeWidth;
    sal_uInt16 nTmp;
    rIStm >> nTmp;
    rClass.maCapType = SvtGraphicStroke::CapType(nTmp);
    rIStm >> nTmp;
    rClass.maJoinType = SvtGraphicStroke::JoinType(nTmp);
    rIStm >> rClass.mfMiterLimit;

    sal_uInt32 nSize;
    rIStm >> nSize;
    rClass.maDashArray.resize(nSize);
    size_t i;
    for(i=0; i<rClass.maDashArray.size(); ++i)
        rIStm >> rClass.maDashArray[i];

    return rIStm;
}


/////////////////////////////////////////////////////////////////////////////

SvtGraphicFill::SvtGraphicFill() :
    maPath(),
    maFillColor( COL_BLACK ),
    mfTransparency(),
    maFillRule(),
    maFillType(),
    maFillTransform(),
    maHatchType(),
    maHatchColor( COL_BLACK ),
    maGradientType(),
    maGradient1stColor( COL_BLACK ),
    maGradient2ndColor( COL_BLACK ),
    maGradientStepCount( gradientStepsInfinite ),
    maFillGraphic()
{
}

SvtGraphicFill::SvtGraphicFill( const PolyPolygon&  rPath,
                                Color               aFillColor,
                                double              fTransparency,
                                FillRule            aFillRule,
                                FillType            aFillType,
                                const Transform&    aFillTransform,
                                bool                bTiling,
                                HatchType           aHatchType,
                                Color               aHatchColor,
                                GradientType        aGradientType,
                                Color               aGradient1stColor,
                                Color               aGradient2ndColor,
                                sal_Int32           aGradientStepCount,
                                const Graphic&      aFillGraphic ) :
    maPath( rPath ),
    maFillColor( aFillColor ),
    mfTransparency( fTransparency ),
    maFillRule( aFillRule ),
    maFillType( aFillType ),
    maFillTransform( aFillTransform ),
    mbTiling( bTiling ),
    maHatchType( aHatchType ),
    maHatchColor( aHatchColor ),
    maGradientType( aGradientType ),
    maGradient1stColor( aGradient1stColor ),
    maGradient2ndColor( aGradient2ndColor ),
    maGradientStepCount( aGradientStepCount ),
    maFillGraphic( aFillGraphic )
{
}

void SvtGraphicFill::getPath( PolyPolygon& rPath ) const
{
    rPath = maPath;
}

Color SvtGraphicFill::getFillColor() const
{
    return maFillColor;
}

double SvtGraphicFill::getTransparency() const
{
    return mfTransparency;
}

SvtGraphicFill::FillRule SvtGraphicFill::getFillRule() const
{
    return maFillRule;
}

SvtGraphicFill::FillType SvtGraphicFill::getFillType() const
{
    return maFillType;
}

void SvtGraphicFill::getTransform( Transform& rTrans ) const
{
    rTrans = maFillTransform;
}

bool SvtGraphicFill::IsTiling() const
{
    return mbTiling;
}

bool SvtGraphicFill::isTiling() const
{
    return mbTiling;
}

SvtGraphicFill::GradientType SvtGraphicFill::getGradientType() const
{
    return maGradientType;
}

void SvtGraphicFill::getGraphic( Graphic& rGraphic ) const
{
    rGraphic = maFillGraphic;
}

void SvtGraphicFill::setPath( const PolyPolygon& rPath )
{
    maPath = rPath;
}

SvStream& operator<<( SvStream& rOStm, const SvtGraphicFill& rClass )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rClass.maPath.Write( rOStm );
    rOStm << rClass.maFillColor;
    rOStm << rClass.mfTransparency;
    sal_uInt16 nTmp = sal::static_int_cast<sal_uInt16>( rClass.maFillRule );
    rOStm << nTmp;
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maFillType );
    rOStm << nTmp;
    int i;
    for(i=0; i<SvtGraphicFill::Transform::MatrixSize; ++i)
        rOStm << rClass.maFillTransform.matrix[i];
    nTmp = rClass.mbTiling;
    rOStm << nTmp;
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maHatchType );
    rOStm << nTmp;
    rOStm << rClass.maHatchColor;
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maGradientType );
    rOStm << nTmp;
    rOStm << rClass.maGradient1stColor;
    rOStm << rClass.maGradient2ndColor;
    rOStm << rClass.maGradientStepCount;
    rOStm << rClass.maFillGraphic;

    return rOStm;
}

SvStream& operator>>( SvStream& rIStm, SvtGraphicFill& rClass )
{
    VersionCompat aCompat( rIStm, STREAM_READ );

    rClass.maPath.Read( rIStm );
    rIStm >> rClass.maFillColor;
    rIStm >> rClass.mfTransparency;
    sal_uInt16 nTmp;
    rIStm >> nTmp;
    rClass.maFillRule = SvtGraphicFill::FillRule( nTmp );
    rIStm >> nTmp;
    rClass.maFillType = SvtGraphicFill::FillType( nTmp );
    int i;
    for(i=0; i<SvtGraphicFill::Transform::MatrixSize; ++i)
        rIStm >> rClass.maFillTransform.matrix[i];
    rIStm >> nTmp;
    rClass.mbTiling = nTmp;
    rIStm >> nTmp;
    rClass.maHatchType = SvtGraphicFill::HatchType( nTmp );
    rIStm >> rClass.maHatchColor;
    rIStm >> nTmp;
    rClass.maGradientType = SvtGraphicFill::GradientType( nTmp );
    rIStm >> rClass.maGradient1stColor;
    rIStm >> rClass.maGradient2ndColor;
    rIStm >> rClass.maGradientStepCount;
    rIStm >> rClass.maFillGraphic;

    return rIStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
