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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/graphictools.hxx>

SvtGraphicFill::Transform::Transform()
{
    matrix[0] = 1.0; matrix[1] = 0.0; matrix[2] = 0.0;
    matrix[3] = 0.0; matrix[4] = 1.0; matrix[5] = 0.0;
}

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

SvtGraphicStroke::SvtGraphicStroke( const tools::Polygon& rPath,
                                    const tools::PolyPolygon&  rStartArrow,
                                    const tools::PolyPolygon&  rEndArrow,
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

void SvtGraphicStroke::getPath( tools::Polygon& rPath ) const
{
    rPath = maPath;
}

void SvtGraphicStroke::getStartArrow( tools::PolyPolygon& rPath ) const
{
    rPath = maStartArrow;
}

void SvtGraphicStroke::getEndArrow( tools::PolyPolygon& rPath ) const
{
    rPath = maEndArrow;
}


void SvtGraphicStroke::getDashArray( DashArray& rDashArray ) const
{
    rDashArray = maDashArray;
}

void SvtGraphicStroke::setPath( const tools::Polygon& rPoly )
{
    maPath = rPoly;
}

void SvtGraphicStroke::setStartArrow( const tools::PolyPolygon& rPoly )
{
    maStartArrow = rPoly;
}

void SvtGraphicStroke::setEndArrow( const tools::PolyPolygon& rPoly )
{
    maEndArrow = rPoly;
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

SvStream& WriteSvtGraphicStroke( SvStream& rOStm, const SvtGraphicStroke& rClass )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 1 );

    rClass.maPath.Write( rOStm );
    rClass.maStartArrow.Write( rOStm );
    rClass.maEndArrow.Write( rOStm );
    rOStm.WriteDouble( rClass.mfTransparency );
    rOStm.WriteDouble( rClass.mfStrokeWidth );
    sal_uInt16 nTmp = sal::static_int_cast<sal_uInt16>( rClass.maCapType );
    rOStm.WriteUInt16( nTmp );
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maJoinType );
    rOStm.WriteUInt16( nTmp );
    rOStm.WriteDouble( rClass.mfMiterLimit );

    rOStm.WriteUInt32( rClass.maDashArray.size() );
    size_t i;
    for(i=0; i<rClass.maDashArray.size(); ++i)
        rOStm.WriteDouble( rClass.maDashArray[i] );

    return rOStm;
}

SvStream& ReadSvtGraphicStroke( SvStream& rIStm, SvtGraphicStroke& rClass )
{
    VersionCompat aCompat( rIStm, StreamMode::READ );

    rClass.maPath.Read( rIStm );
    rClass.maStartArrow.Read( rIStm );
    rClass.maEndArrow.Read( rIStm );
    rIStm.ReadDouble( rClass.mfTransparency );
    rIStm.ReadDouble( rClass.mfStrokeWidth );
    sal_uInt16 nTmp;
    rIStm.ReadUInt16( nTmp );
    rClass.maCapType = SvtGraphicStroke::CapType(nTmp);
    rIStm.ReadUInt16( nTmp );
    rClass.maJoinType = SvtGraphicStroke::JoinType(nTmp);
    rIStm.ReadDouble( rClass.mfMiterLimit );

    sal_uInt32 nSize;
    rIStm.ReadUInt32( nSize );
    rClass.maDashArray.resize(nSize);
    size_t i;
    for(i=0; i<rClass.maDashArray.size(); ++i)
        rIStm.ReadDouble( rClass.maDashArray[i] );

    return rIStm;
}

SvtGraphicFill::SvtGraphicFill() :
    maPath(),
    maFillColor( COL_BLACK ),
    mfTransparency(),
    maFillRule(),
    maFillType(),
    maFillTransform(),
    mbTiling( false ),
    maHatchType(),
    maHatchColor( COL_BLACK ),
    maGradientType(),
    maGradient1stColor( COL_BLACK ),
    maGradient2ndColor( COL_BLACK ),
    maGradientStepCount( gradientStepsInfinite ),
    maFillGraphic()
{
}

SvtGraphicFill::SvtGraphicFill( const tools::PolyPolygon&  rPath,
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

void SvtGraphicFill::getPath( tools::PolyPolygon& rPath ) const
{
    rPath = maPath;
}


void SvtGraphicFill::getTransform( Transform& rTrans ) const
{
    rTrans = maFillTransform;
}


void SvtGraphicFill::getGraphic( Graphic& rGraphic ) const
{
    rGraphic = maFillGraphic;
}

void SvtGraphicFill::setPath( const tools::PolyPolygon& rPath )
{
    maPath = rPath;
}

SvStream& WriteSvtGraphicFill( SvStream& rOStm, const SvtGraphicFill& rClass )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 1 );

    rClass.maPath.Write( rOStm );
    TypeSerializer aSerializer(rOStm);
    aSerializer.writeColor(rClass.maFillColor);
    rOStm.WriteDouble( rClass.mfTransparency );
    sal_uInt16 nTmp = sal::static_int_cast<sal_uInt16>( rClass.maFillRule );
    rOStm.WriteUInt16( nTmp );
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maFillType );
    rOStm.WriteUInt16( nTmp );
    int i;
    for(i=0; i<SvtGraphicFill::Transform::MatrixSize; ++i)
        rOStm.WriteDouble( rClass.maFillTransform.matrix[i] );
    nTmp = sal_uInt16(rClass.mbTiling);
    rOStm.WriteUInt16( nTmp );
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maHatchType );
    rOStm.WriteUInt16( nTmp );
    aSerializer.writeColor(rClass.maHatchColor);
    nTmp = sal::static_int_cast<sal_uInt16>( rClass.maGradientType );
    rOStm.WriteUInt16( nTmp );
    aSerializer.writeColor(rClass.maGradient1stColor);
    aSerializer.writeColor(rClass.maGradient2ndColor);
    rOStm.WriteInt32( rClass.maGradientStepCount );
    aSerializer.writeGraphic(rClass.maFillGraphic);

    return rOStm;
}

SvStream& ReadSvtGraphicFill( SvStream& rIStm, SvtGraphicFill& rClass )
{
    VersionCompat aCompat( rIStm, StreamMode::READ );

    rClass.maPath.Read( rIStm );

    TypeSerializer aSerializer(rIStm);
    aSerializer.readColor(rClass.maFillColor);
    rIStm.ReadDouble( rClass.mfTransparency );
    sal_uInt16 nTmp;
    rIStm.ReadUInt16( nTmp );
    rClass.maFillRule = SvtGraphicFill::FillRule( nTmp );
    rIStm.ReadUInt16( nTmp );
    rClass.maFillType = SvtGraphicFill::FillType( nTmp );
    for (int i = 0; i < SvtGraphicFill::Transform::MatrixSize; ++i)
        rIStm.ReadDouble( rClass.maFillTransform.matrix[i] );
    rIStm.ReadUInt16( nTmp );
    rClass.mbTiling = nTmp;
    rIStm.ReadUInt16( nTmp );
    rClass.maHatchType = SvtGraphicFill::HatchType( nTmp );
    aSerializer.readColor(rClass.maHatchColor);
    rIStm.ReadUInt16( nTmp );
    rClass.maGradientType = SvtGraphicFill::GradientType( nTmp );
    aSerializer.readColor(rClass.maGradient1stColor);
    aSerializer.readColor(rClass.maGradient2ndColor);
    rIStm.ReadInt32( rClass.maGradientStepCount );
    aSerializer.readGraphic(rClass.maFillGraphic);

    return rIStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
