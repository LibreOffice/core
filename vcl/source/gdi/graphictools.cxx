/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_vcl.hxx"
#include <tools/vcompat.hxx>

#include <vcl/graphictools.hxx>

static ::rtl::OString polyToString( const Polygon& rPoly )
{
    ::rtl::OString aStr;
    sal_uInt16 nVertex;
    for(nVertex=0; nVertex<rPoly.GetSize(); ++nVertex)
    {
        aStr += "(";
        switch( rPoly.GetFlags(nVertex) )
        {
            case POLY_NORMAL:
            case POLY_SMOOTH:
            case POLY_SYMMTR:
                aStr += "n: ";
                break;

            case POLY_CONTROL:
                aStr += "c: ";
                break;

            default:
                OSL_FAIL( "SvtGraphicStroke::polyToString invalid flag");
                break;
        }
        aStr += ::rtl::OString::valueOf( static_cast< double >( rPoly[nVertex].getX() ) );
        aStr += ",";
        aStr += ::rtl::OString::valueOf( static_cast< double >( rPoly[nVertex].getY() ) );
        aStr += ") ";
    }

    return aStr;
}

static ::rtl::OString polyPolyToString( const PolyPolygon& rPolyPoly )
{
    ::rtl::OString aStr;
    sal_uInt16 nPoly;
    for(nPoly=0; nPoly<rPolyPoly.Count(); ++nPoly)
    {
        const Polygon& rPoly = rPolyPoly[nPoly];

        aStr += "{ ";
        aStr += polyToString( rPoly );
        aStr += "} ";
    }

    return aStr;
}

static ::rtl::OString dashToString( const SvtGraphicStroke::DashArray& rDashArray )
{
    ::rtl::OString aStr;

    aStr += "dash: [ ";

    int i, nDashes( rDashArray.size() );
    for(i=0; i<nDashes; ++i)
    {
        aStr += ::rtl::OString::valueOf( rDashArray[i] );
        aStr += " ";
    }

    aStr += "] ";

    return aStr;
}

static ::rtl::OString colorToString( Color aColor )
{
    ::rtl::OString aStr;

    aStr += "color: [ ";
    aStr += ::rtl::OString::valueOf( aColor.GetRed() );
    aStr += " ";
    aStr += ::rtl::OString::valueOf( aColor.GetGreen() );
    aStr += " ";
    aStr += ::rtl::OString::valueOf( aColor.GetBlue() );
    aStr += " ] ";

    return aStr;
}

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

::rtl::OString SvtGraphicStroke::toString() const
{
    ::rtl::OString aStr;

    aStr += polyToString( maPath );
    aStr += "trans: ";
    aStr += ::rtl::OString::valueOf( static_cast< double >(getTransparency()) );
    aStr += " width: ";
    aStr += ::rtl::OString::valueOf( static_cast< double >(getStrokeWidth()) );
    aStr += " cap: ";
    switch( getCapType() )
    {
        case capButt:
            aStr += "butt";
            break;

        case capRound:
            aStr += "round";
            break;

        case capSquare:
            aStr += "square";
            break;

        default:
            OSL_FAIL( "SvtGraphicStroke::toString missing cap type");
            break;
    }
    aStr += " join: ";
    switch( getJoinType() )
    {
        case joinMiter:
            aStr += "miter";
            break;

        case joinRound:
            aStr += "round";
            break;

        case joinBevel:
            aStr += "bevel";
            break;

        case joinNone:
            aStr += "none";
            break;

        default:
            OSL_FAIL( "SvtGraphicStroke::toString missing join type");
            break;
    }
    aStr += " ";

    if( maStartArrow.Count() )
    {
        aStr += "start: ";
        aStr += polyPolyToString( maStartArrow );
        aStr += " ";
    }

    if( maEndArrow.Count() )
    {
        aStr += "end: ";
        aStr += polyPolyToString( maEndArrow );
        aStr += " ";
    }

    aStr += dashToString( maDashArray );

    return aStr;
}

void SvtGraphicStroke::setPath( const Polygon& rPoly )
{
    maPath = rPoly;
}

void SvtGraphicStroke::setStartArrow( const PolyPolygon& rPoly )
{
    maStartArrow = rPoly;
}

void SvtGraphicStroke::setEndArrow( const PolyPolygon& rPoly )
{
    maEndArrow = rPoly;
}

void SvtGraphicStroke::setTransparency( double fTrans )
{
    mfTransparency = fTrans;
}

void SvtGraphicStroke::setStrokeWidth( double fWidth )
{
    mfStrokeWidth = fWidth;
}

void SvtGraphicStroke::setCapType( CapType eType )
{
    maCapType = eType;
}

void SvtGraphicStroke::setJoinType( JoinType eType )
{
    maJoinType = eType;
}

void SvtGraphicStroke::setMiterLimit( double fMiterLimit )
{
    mfMiterLimit = fMiterLimit;
}

void SvtGraphicStroke::setDashArray( const DashArray& rDashArray )
{
    maDashArray = rDashArray;
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
                                int                 aGradientStepCount,
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

SvtGraphicFill::HatchType SvtGraphicFill::getHatchType() const
{
    return maHatchType;
}

Color SvtGraphicFill::getHatchColor() const
{
    return maHatchColor;
}

SvtGraphicFill::GradientType SvtGraphicFill::getGradientType() const
{
    return maGradientType;
}

Color SvtGraphicFill::getGradient1stColor() const
{
    return maGradient1stColor;
}

Color SvtGraphicFill::getGradient2ndColor() const
{
    return maGradient2ndColor;
}

int SvtGraphicFill::getGradientStepCount() const
{
    return maGradientStepCount;
}

void SvtGraphicFill::getGraphic( Graphic& rGraphic ) const
{
    rGraphic = maFillGraphic;
}

::rtl::OString SvtGraphicFill::toString() const
{
    ::rtl::OString aStr;

    aStr += polyPolyToString( maPath );
    aStr += "fill";
    aStr += colorToString( getFillColor() );
    aStr += " trans: ";
    aStr += ::rtl::OString::valueOf( static_cast< double >(getTransparency()) );
    aStr += " rule: ";
    switch( getFillRule() )
    {
        case fillNonZero:
            aStr += "nonzero";
            break;

        case fillEvenOdd:
            aStr += "evenodd";
            break;

        default:
            OSL_FAIL( "SvtGraphicFill::toString missing fill rule");
            break;
    }
    aStr += " type: ";
    switch( getFillType() )
    {
        case fillSolid:
            aStr += "solid";
            break;

        case fillGradient:
            aStr += "gradient";
            break;

        case fillHatch:
            aStr += "hatch";
            break;

        case fillTexture:
            aStr += "bitmap";
            break;

        default:
            OSL_FAIL( "SvtGraphicStroke::toString missing fill type");
            break;
    }

    aStr += " transform: [ ";
    int i;
    for(i=0; i<Transform::MatrixSize; ++i)
        aStr += ::rtl::OString::valueOf( maFillTransform.matrix[i] );
    aStr += " ] ";

    aStr += " hatch: ";
    switch( getHatchType() )
    {
        case hatchSingle:
            aStr += "single";
            break;

        case hatchDouble:
            aStr += "double";
            break;

        case hatchTriple:
            aStr += "triple";
            break;

        default:
            OSL_FAIL( "SvtGraphicStroke::toString missing hatch type");
            break;
    }

    aStr += " hatch";
    aStr += colorToString( getHatchColor() );

    aStr += " gradient: ";
    switch( getGradientType() )
    {
        case gradientLinear:
            aStr += "linear";
            break;

        case gradientRadial:
            aStr += "radial";
            break;

        case gradientRectangular:
            aStr += "rectangular";
            break;

        default:
            OSL_FAIL( "SvtGraphicStroke::toString missing gradient type");
            break;
    }

    aStr += " grad1st";
    aStr += colorToString( getGradient1stColor() );

    aStr += " grad2nd";
    aStr += colorToString( getGradient2ndColor() );

    aStr += " gradstep";
    aStr += ::rtl::OString::valueOf( (sal_Int32)getGradientStepCount() );

    if( maFillGraphic.GetType() != GRAPHIC_NONE )
    {
        aStr += " fillgraphic: ";
        switch( maFillGraphic.GetType() )
        {
            case GRAPHIC_BITMAP:
                aStr += "bitmap";
                break;

            case GRAPHIC_GDIMETAFILE:
                aStr += "metafile";
                break;

            case GRAPHIC_DEFAULT:
                aStr += "default";
                break;

            default:
                OSL_FAIL( "SvtGraphicStroke::toString missing graphic type");
                break;
        }

        aStr += " of ";
        aStr += ::rtl::OString::valueOf( static_cast< sal_Int32 >(maFillGraphic.GetSizeBytes()) );
        aStr += " bytes";
    }

    return aStr;
}

void SvtGraphicFill::setPath( const PolyPolygon& rPath )
{
    maPath = rPath;
}

void SvtGraphicFill::setFillColor( Color aFillColor )
{
    maFillColor = aFillColor;
}

void SvtGraphicFill::setTransparency( double fTransparency )
{
    mfTransparency = fTransparency;
}

void SvtGraphicFill::setFillRule( FillRule aFillRule )
{
    maFillRule = aFillRule;
}

void SvtGraphicFill::setFillType( FillType aFillType )
{
    maFillType = aFillType;
}

void SvtGraphicFill::setTransform( const Transform& rTransform )
{
    maFillTransform = rTransform;
}

void SvtGraphicFill::setTiling( bool bTiling )
{
    mbTiling = bTiling;
}

void SvtGraphicFill::setHatchType( HatchType aHatchType )
{
    maHatchType = aHatchType;
}

void SvtGraphicFill::setHatchColor( Color aHatchColor )
{
    maHatchColor = aHatchColor;
}

void SvtGraphicFill::setGradientType( GradientType aGradType )
{
    maGradientType = aGradType;
}

void SvtGraphicFill::setGradient1stColor( Color aColor )
{
    maGradient1stColor = aColor;
}

void SvtGraphicFill::setGradient2ndColor( Color aColor )
{
    maGradient2ndColor = aColor;
}

void SvtGraphicFill::setGradientStepCount( int aCount )
{
    maGradientStepCount = aCount;
}

void SvtGraphicFill::setGraphic( const Graphic& rGraphic )
{
    maFillGraphic = rGraphic;
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
