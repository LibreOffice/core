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

#include "swfwriter.hxx"
#include <vcl/virdev.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/debug.hxx>

#include <math.h>

using namespace ::swf;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


static sal_uInt16 getMaxBitsUnsigned( sal_uInt32 nValue )
{
    sal_uInt16 nBits = 0;

    while( nValue )
    {
        nBits++;
        nValue >>= 1;
    }

    return nBits;
}


sal_uInt16 getMaxBitsSigned( sal_Int32 nValue )
{
    if( nValue < 0 )
        nValue *= -1;

    return getMaxBitsUnsigned( static_cast< sal_uInt32 >(nValue) ) + 1;
}


BitStream::BitStream()
{
    mnBitPos = 8;
    mnCurrentByte = 0;
}


void BitStream::writeUB( sal_uInt32 nValue, sal_uInt16 nBits )
{
    while( nBits != 0 )
    {
        mnCurrentByte |= nValue << (32 - nBits) >> (32 - mnBitPos);

        if ( nBits > mnBitPos )
        {
            nBits = nBits - mnBitPos;
            mnBitPos = 0;
        }
        else
        {
            mnBitPos = sal::static_int_cast<sal_uInt8>( mnBitPos - nBits );
            nBits = 0;
        }

        if( 0 == mnBitPos )
            pad();
    }
}


void BitStream::writeSB( sal_Int32 nValue, sal_uInt16 nBits )
{
    writeUB( static_cast< sal_uInt32 >(nValue), nBits );
}


void BitStream::writeFB( sal_uInt32 nValue, sal_uInt16 nBits )
{
    writeUB( nValue, nBits );
}


void BitStream::pad()
{
    if( 8 != mnBitPos )
    {
        maData.push_back( mnCurrentByte );
        mnCurrentByte = 0;
        mnBitPos = 8;
    }
}


void BitStream::writeTo( SvStream& out )
{
    pad();

    for (auto const& data : maData)
    {
        out.WriteUChar(data);
    }
}


sal_uInt32 BitStream::getOffset() const
{
    return maData.size();
}


Tag::Tag( sal_uInt8 nTagId )
{
    mnTagId = nTagId;
}


void Tag::write( SvStream &out )
{
    sal_uInt32 nSz = TellEnd();
    Seek( STREAM_SEEK_TO_BEGIN );

    if( mnTagId != 0xff )
    {
        bool bLarge = nSz > 62;

        sal_uInt16 nCode = ( mnTagId << 6 ) | ( bLarge ? 0x3f : uInt16_(nSz) );

        out.WriteUChar( nCode );
        out.WriteUChar( nCode >> 8 );

        if( bLarge )
        {
            sal_uInt32 nTmp = nSz;

            out.WriteUChar( nTmp );
            nTmp >>= 8;
            out.WriteUChar( nTmp );
            nTmp >>= 8;
            out.WriteUChar( nTmp );
            nTmp >>= 8;
            out.WriteUChar( nTmp );
        }
    }

    out.WriteBytes( GetData(), nSz );
}
#if 0


void Tag::addI32( sal_Int32 nValue )
{
    addUI32( static_cast<sal_uInt32>( nValue ) );
}
#endif


void Tag::addUI32( sal_uInt32 nValue )
{
    WriteUInt32( nValue );
}
#if 0


void Tag::addI16( sal_Int16 nValue )
{
    addUI16( static_cast<sal_uInt16>( nValue ) );
}
#endif


void Tag::addUI16( sal_uInt16 nValue )
{
    WriteUChar( nValue );
    WriteUChar( nValue >> 8 );
}


void Tag::addUI8( sal_uInt8 nValue )
{
    WriteUChar( nValue );
}


void Tag::addBits( BitStream& rIn )
{
    rIn.writeTo( *this );
}


void Tag::addRGBA( const Color& rColor )
{
    addUI8( rColor.GetRed() );
    addUI8( rColor.GetGreen() );
    addUI8( rColor.GetBlue() );
    addUI8( 0xff - rColor.GetTransparency() );
}


void Tag::addRGB( const Color& rColor )
{
    addUI8( rColor.GetRed() );
    addUI8( rColor.GetGreen() );
    addUI8( rColor.GetBlue() );
}


void Tag::addRect( const tools::Rectangle& rRect )
{
    writeRect( *this, rRect );
}


void Tag::writeRect( SvStream& rOut, const tools::Rectangle& rRect )
{
    BitStream aBits;

    sal_Int32 minX, minY, maxX, maxY;

    if( rRect.Left() < rRect.Right() )
    {
        minX = rRect.Left();
        maxX = rRect.Right();
    }
    else
    {
        maxX = rRect.Left();
        minX = rRect.Right();
    }


    if( rRect.Top() < rRect.Bottom() )
    {
        minY = rRect.Top();
        maxY = rRect.Bottom();
    }
    else
    {
        maxY = rRect.Top();
        minY = rRect.Bottom();
    }

    // AS: Figure out the maximum number of bits required to represent any of the
    //  rectangle coordinates.  Since minX or minY could be negative, they could
    //  actually require more bits than maxX or maxY.
    // AS: Christian, can they be negative, or is that a wasted check?
    // CL: I think so, f.e. for shapes that have the top and/or left edge outside
    //         the page origin
    sal_uInt8 nBits1 = sal::static_int_cast<sal_uInt8>( std::max( getMaxBitsSigned( minX ), getMaxBitsSigned( minY ) ) );
    sal_uInt8 nBits2 = sal::static_int_cast<sal_uInt8>( std::max( getMaxBitsSigned( maxX ), getMaxBitsSigned( maxY ) ) );
    sal_uInt8 nBitsMax = std::max( nBits1, nBits2 );

    aBits.writeUB( nBitsMax, 5 );
    aBits.writeSB( minX, nBitsMax );
    aBits.writeSB( maxX, nBitsMax );
    aBits.writeSB( minY, nBitsMax );
    aBits.writeSB( maxY, nBitsMax );

    aBits.writeTo( rOut );
}


void Tag::addMatrix( const ::basegfx::B2DHomMatrix& rMatrix ) // #i73264#
{
    writeMatrix( *this, rMatrix );
}


void Tag::writeMatrix( SvStream& rOut, const ::basegfx::B2DHomMatrix& rMatrix ) // #i73264#
{

    BitStream aBits;

    const bool bHasScale = rMatrix.get(0, 0) != 1.0 || rMatrix.get(1, 1) != 1.0;

    aBits.writeUB( int(bHasScale), 1 );

    if( bHasScale )
    {
        sal_uInt8 nScaleBits = 31;

        aBits.writeUB( nScaleBits, 5 );
        aBits.writeFB( getFixed( rMatrix.get(0, 0) ), nScaleBits ); // Scale X
        aBits.writeFB( getFixed( rMatrix.get(1, 1) ), nScaleBits ); // Scale Y
    }

    const bool bHasRotate = rMatrix.get(0, 1) != 0.0 || rMatrix.get(1, 0) != 0.0;

    aBits.writeUB( int(bHasRotate), 1 );

    if( bHasRotate )
    {
        sal_uInt8 nRotateBits = 31;

        aBits.writeUB( nRotateBits, 5 );
        aBits.writeFB( getFixed( rMatrix.get(0, 1) ), nRotateBits );    // RotateSkew0
        aBits.writeFB( getFixed( rMatrix.get(1, 0) ), nRotateBits );    // RotateSkew1
    }

    sal_uInt8 nTranslateBits = 16;

    aBits.writeUB( nTranslateBits, 5 );
    aBits.writeSB( static_cast<sal_Int16>(rMatrix.get(0, 2)), nTranslateBits );      // Translate X
    aBits.writeSB( static_cast<sal_Int16>(rMatrix.get(1, 2)), nTranslateBits );      // Translate Y

    aBits.writeTo( rOut );
}


void Tag::addStream( SvStream& rIn )
{
    (*this).WriteStream( rIn );
}


Sprite::Sprite( sal_uInt16 nId )
: mnId( nId ), mnFrames(0)
{
}


Sprite::~Sprite()
{
}


void Sprite::write( SvStream& out )
{
    SvMemoryStream aTmp;
    for (auto const& tag : maTags)
        tag->write( aTmp );

    if( !mnFrames )
        mnFrames = 1;

    aTmp.Seek(0);

    Tag aTag( TAG_DEFINESPRITE );
    aTag.addUI16( mnId );
    aTag.addUI16( uInt16_( mnFrames ) );
    aTag.addStream( aTmp );
    aTag.write( out );
}


void Sprite::addTag( std::unique_ptr<Tag> pNewTag )
{
    if( pNewTag->getTagId() == TAG_SHOWFRAME )
        mnFrames++;

    maTags.push_back( std::move(pNewTag) );
}


sal_uInt32 swf::getFixed( double fValue )
{
    sal_Int16 nUpper = static_cast<sal_Int16>(floor(fValue));
    sal_uInt16 nLower = static_cast<sal_uInt16>((fValue - floor(fValue))*0x10000);

    sal_uInt32 temp = static_cast<sal_Int32>(nUpper)<<16;
    temp |= nLower;

    return temp;
}


/** constructs a new flash font for the given VCL Font */
FlashFont::FlashFont( const vcl::Font& rFont, sal_uInt16 nId )
: maFont( rFont ), mnNextIndex(0), mnId( nId )
{
}


FlashFont::~FlashFont()
{
}


/** gets the glyph id for the given character. The glyphs are created on demand */
sal_uInt16 FlashFont::getGlyph( sal_uInt16 nChar, VirtualDevice* pVDev )
{
    // see if we already created a glyph for this character
    std::map<sal_uInt16, sal_uInt16>::iterator aIter( maGlyphIndex.find(nChar) );
    if( aIter != maGlyphIndex.end() )
    {
        return aIter->second;
    }

    // if not, we create one now

    maGlyphIndex[nChar] = mnNextIndex;

    vcl::Font aOldFont( pVDev->GetFont() );
    vcl::Font aNewFont( aOldFont );
    aNewFont.SetAlignment( ALIGN_BASELINE );
    pVDev->SetFont( aNewFont );
    aOldFont.SetOrientation(0);

    // let the virtual device convert the character to polygons
    tools::PolyPolygon aPolyPoly;
    pVDev->GetTextOutline( aPolyPoly, OUString(sal_Unicode(nChar)) );

    maGlyphOffsets.push_back( uInt16_( maGlyphData.getOffset() ) );

    // Number of fill and line index bits set to 1
    maGlyphData.writeUB( 0x11, 8 );

    const sal_uInt16 nCount = aPolyPoly.Count();
    sal_uInt16 i,n;
    for( i = 0; i < nCount; i++ )
    {
        tools::Polygon& rPoly = aPolyPoly[ i ];

        const sal_uInt16 nSize = rPoly.GetSize();
        if( nSize )
        {
            // convert polygon to flash EM_SQUARE (1024x1024)
            for( n = 0; n < nSize; n++ )
            {
                Point aPoint( rPoly[n] );
                aPoint.setX( static_cast<long>((double(aPoint.X()) * 1024.0 ) / double(aOldFont.GetFontHeight())) );
                aPoint.setY( static_cast<long>((double(aPoint.Y()) * 1024.0 ) / double(aOldFont.GetFontHeight())) );
                rPoly[n] = aPoint;
            }
            Writer::Impl_addPolygon( maGlyphData, rPoly, true );
        }
    }
    Writer::Impl_addEndShapeRecord( maGlyphData );

    maGlyphData.pad();

    pVDev->SetFont( aOldFont );

    return mnNextIndex++;
}


void FlashFont::write( SvStream& out )
{
    Tag aTag( TAG_DEFINEFONT );

    aTag.addUI16( mnId );

    sal_uInt16 nGlyphs = uInt16_( maGlyphOffsets.size() );
    sal_uInt16 nOffset = nGlyphs * sizeof( sal_uInt16 );

    for (auto const& glyphOffset : maGlyphOffsets)
        aTag.addUI16( nOffset + glyphOffset );

    aTag.addBits( maGlyphData );

    aTag.write( out );
}


/** this c'tor creates a solid fill style */
FillStyle::FillStyle( const Color& rSolidColor )
    : meType(solid )
    , mnBitmapId(0)
    , maColor(rSolidColor)
{
}


/** this c'tor creates a tiled or clipped bitmap fill style */
FillStyle::FillStyle( sal_uInt16 nBitmapId, bool bClipped, const ::basegfx::B2DHomMatrix& rMatrix ) // #i73264#
:   meType( bClipped ? clipped_bitmap : tiled_bitmap ),
    maMatrix( rMatrix ),
    mnBitmapId( nBitmapId )
{
}


static FillStyle::FillStyleType Impl_getFillStyleType( const Gradient& rGradient )
{
    switch( rGradient.GetStyle() )
    {
    case GradientStyle::Elliptical:
    case GradientStyle::Radial:
        return FillStyle::radial_gradient;
//  case GradientStyle::Axial:
//  case GradientStyle::Square:
//  case GradientStyle::Rect:
//  case GradientStyle::Linear:
    default:
        return FillStyle::linear_gradient;
    }
}


/** this c'tor creates a linear or radial gradient fill style */
FillStyle::FillStyle( const tools::Rectangle& rBoundRect, const Gradient& rGradient )
    : meType(Impl_getFillStyleType(rGradient))
    , mnBitmapId(0)
    , maGradient(rGradient)
    , maBoundRect(rBoundRect)
{
}


void FillStyle::addTo( Tag* pTag ) const
{
    pTag->addUI8( sal::static_int_cast<sal_uInt8>( meType ) );
    switch( meType )
    {
    case solid:
        pTag->addRGBA( maColor );
        break;
    case linear_gradient:
    case radial_gradient:
        Impl_addGradient( pTag );
        break;
    case tiled_bitmap:
    case clipped_bitmap:
        pTag->addUI16( mnBitmapId );
        pTag->addMatrix( maMatrix );
        break;
    }
}


struct GradRecord
{
    sal_uInt8   mnRatio;
    Color       maColor;

    GradRecord( sal_uInt8 nRatio, const Color& rColor ) : mnRatio( nRatio ), maColor( rColor ) {}
};

// TODO: better emulation of our gradients
void FillStyle::Impl_addGradient( Tag* pTag ) const
{
    std::vector< struct GradRecord > aGradientRecords;
    basegfx::B2DHomMatrix m(basegfx::utils::createRotateB2DHomMatrix((maGradient.GetAngle() - 900) * F_PI1800));

    switch( maGradient.GetStyle() )
    {
    case GradientStyle::Elliptical:
    case GradientStyle::Radial:
        {
            aGradientRecords.emplace_back( 0x00, maGradient.GetEndColor() );
            aGradientRecords.emplace_back( 0xff, maGradient.GetStartColor() );

            double tx = ( maGradient.GetOfsX() * 32768.0 ) / 100.0;
            double ty = ( maGradient.GetOfsY() * 32768.0 ) / 100.0;
            double scalex = static_cast<double>(maBoundRect.GetWidth()) / 32768.0;
            double scaley = static_cast<double>(maBoundRect.GetHeight()) / 32768.0;

            m.scale( 1.2, 1.2 );

            if( scalex > scaley )
            {
                double scale_move = scaley / scalex;

                m.translate( tx, scale_move * ty );


                m.scale( scalex, scalex );
            }
            else
            {
                double scale_move = scalex / scaley;

                m.translate( scale_move * tx, ty );


                m.scale( scaley, scaley );
            }

        }
        break;
    case GradientStyle::Axial:
        {
            aGradientRecords.emplace_back( 0x00, maGradient.GetEndColor() );
            aGradientRecords.emplace_back( 0x80, maGradient.GetStartColor() );
            aGradientRecords.emplace_back( 0xff, maGradient.GetEndColor() );
            double scalex = static_cast<double>(maBoundRect.GetWidth()) / 32768.0;
            double scaley = static_cast<double>(maBoundRect.GetHeight()) / 32768.0;
            m.translate( 32768.0 / 2.0, 32768.0 / 2.0 );
            m.scale( scalex, scaley );
        }
        break;
    case GradientStyle::Square:
    case GradientStyle::Rect:
    case GradientStyle::Linear:
        {
            aGradientRecords.emplace_back( 0x00, maGradient.GetStartColor() );
            aGradientRecords.emplace_back( 0xff, maGradient.GetEndColor() );
            double scalex = static_cast<double>(maBoundRect.GetWidth()) / 32768.0;
            double scaley = static_cast<double>(maBoundRect.GetHeight()) / 32768.0;

            m.scale( scalex, scaley );

            m.translate( maBoundRect.GetWidth() / 2.0, maBoundRect.GetHeight() / 2.0 );
        }
        break;
    case  GradientStyle::FORCE_EQUAL_SIZE: break;
    }

    m.translate( maBoundRect.Left(), maBoundRect.Top() );

    pTag->addMatrix( m );

    DBG_ASSERT( aGradientRecords.size() < 8, "Illegal FlashGradient!" );

    pTag->addUI8( static_cast<sal_uInt8>( aGradientRecords.size() ) );

    for (auto const& gradientRecord : aGradientRecords)
    {
        pTag->addUI8( gradientRecord.mnRatio );
        pTag->addRGBA( gradientRecord.maColor );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
