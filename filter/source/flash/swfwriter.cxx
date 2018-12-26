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
#include <vcl/gdimtf.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/debug.hxx>

using namespace ::swf;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


static MapMode aTWIPSMode( MapUnit::MapTwip );
static MapMode a100thmmMode( MapUnit::Map100thMM );

static sal_Int32 map100thmm( sal_Int32 n100thMM )
{
    Point aPoint( n100thMM, n100thMM );
    sal_Int32 nX = OutputDevice::LogicToLogic( aPoint,  a100thmmMode, aTWIPSMode ).X();
    return nX;
}


Writer::Writer( sal_Int32 nTWIPWidthOutput, sal_Int32 nTWIPHeightOutput, sal_Int32 nDocWidth, sal_Int32 nDocHeight, sal_Int32 nJPEGcompressMode )
:   mnDocWidth( map100thmm(nDocWidth) ),
    mnDocHeight( map100thmm(nDocHeight) ),
    mnDocXScale( static_cast<double>(nTWIPWidthOutput) / mnDocWidth ),
    mnDocYScale( static_cast<double>(nTWIPHeightOutput) / mnDocHeight ),
    mpClipPolyPolygon( nullptr ),
    mnNextId( 1 ),
    mnFrames( 0 ),
    mnGlobalTransparency(0),
    mnJPEGCompressMode(nJPEGcompressMode)
{
    mpVDev->EnableOutput( false );

    maMovieTempFile.EnableKillingFile();
    maFontsTempFile.EnableKillingFile();

    mpMovieStream = maMovieTempFile.GetStream( StreamMode::WRITE|StreamMode::TRUNC );
    mpFontsStream = maFontsTempFile.GetStream( StreamMode::WRITE|StreamMode::TRUNC );

    // define an invisible button with the size of a page
    tools::Rectangle aRect( 0, 0, static_cast<long>( mnDocWidth * mnDocXScale ), static_cast<long>( mnDocHeight * mnDocYScale ) );
    tools::Polygon aPoly( aRect );
    FillStyle aFill = FillStyle( COL_WHITE );
    sal_uInt16 nWhiteBackgroundShapeId = defineShape( aPoly, aFill );

    ::basegfx::B2DHomMatrix m; // #i73264#
    mnPageButtonId = createID();
    startTag( TAG_DEFINEBUTTON );
    mpTag->addUI16( mnPageButtonId );           // character id for button

    // button records
    mpTag->addUI8( 0x08 );                      // only hit state
    mpTag->addUI16( nWhiteBackgroundShapeId );  // shape id of background rectangle
    mpTag->addUI16( 0 );                        // depth for button DANGER!
    mpTag->addMatrix( m );                      // identity matrix
    mpTag->addUI8( 0 );                         // empty color transform

//  mpTag->addUI8( 0 );                         // end of button records

    // action records
    mpTag->addUI8( 0x06 );                      // ActionPlay
    mpTag->addUI8( 0 );                         // end of action records

    endTag();

    // place a shape that clips shapes depth 2-3 to document boundaries
//  placeShape( mnWhiteBackgroundShapeId, 1, 0, 0, 4 );
}


Writer::~Writer()
{
    mpVDev.disposeAndClear();
}


static void ImplCopySvStreamToXOutputStream( SvStream& rIn, Reference< XOutputStream > const &xOut )
{
    sal_uInt32 nBufferSize = 64*1024;

    sal_uInt32 nSize = rIn.TellEnd();
    rIn.Seek( STREAM_SEEK_TO_BEGIN );

    Sequence< sal_Int8 > aBuffer( std::min( nBufferSize, nSize ) );

    while( nSize )
    {
        if( nSize < nBufferSize )
        {
            nBufferSize = nSize;
            aBuffer.realloc( nSize );
        }

        sal_uInt32 nRead = rIn.ReadBytes(aBuffer.getArray(), nBufferSize);
        DBG_ASSERT( nRead == nBufferSize, "ImplCopySvStreamToXOutputStream failed!" );
        xOut->writeBytes( aBuffer );

        if( nRead == 0 )
            break;

        nSize -= nRead;
    }
}


void Writer::storeTo( Reference< XOutputStream > const &xOutStream )
{
    for (auto & font : maFonts)
    {
        font->write( *mpFontsStream );
        font.reset();
    }
    maFonts.clear();

    // Endtag
    mpMovieStream->WriteUInt16( 0 );

    Tag aHeader( 0xff );

    aHeader.addUI8( 'F' );
    aHeader.addUI8( 'W' );
    aHeader.addUI8( 'S' );
    aHeader.addUI8( 5 );

    sal_uInt32 nSizePos = aHeader.Tell();

    aHeader.WriteUInt32( 0 );

    tools::Rectangle aDocRect( 0, 0, static_cast<long>(mnDocWidth*mnDocXScale), static_cast<long>(mnDocHeight*mnDocYScale) );

    aHeader.addRect( aDocRect );

    // frame delay in 8.8 fixed number of frames per second
    aHeader.addUI8( 0 );
    aHeader.addUI8( 12 );

    aHeader.addUI16( uInt16_(mnFrames) );

    const sal_uInt32 nSize = aHeader.Tell() + mpFontsStream->Tell() + mpMovieStream->Tell();

    aHeader.Seek( nSizePos );
    aHeader.WriteUInt32( nSize );

    ImplCopySvStreamToXOutputStream( aHeader, xOutStream );
    ImplCopySvStreamToXOutputStream( *mpFontsStream, xOutStream );
    ImplCopySvStreamToXOutputStream( *mpMovieStream, xOutStream );
}


sal_uInt16 Writer::startSprite()
{
    sal_uInt16 nShapeId = createID();
    mvSpriteStack.push(mpSprite.release());
    mpSprite.reset(new Sprite( nShapeId ));
    return nShapeId;
}


void Writer::endSprite()
{
    if( mpSprite )
    {
        startTag( TAG_END );
        endTag();

        mpSprite->write( *mpMovieStream );
        mpSprite.reset();

        if (!mvSpriteStack.empty())
        {
            mpSprite.reset( mvSpriteStack.top() );
            mvSpriteStack.pop();
        }
    }
}


void Writer::placeShape( sal_uInt16 nID, sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y )
{
    startTag( TAG_PLACEOBJECT2 );

    BitStream aBits;

    aBits.writeUB( sal_uInt32(0), 1 ); // Has Clip Actions?
    aBits.writeUB( 0, 1 );              // reserved
    aBits.writeUB( sal_uInt32(0), 1 );  // has a name
    aBits.writeUB( 0, 1 );              // no ratio
    aBits.writeUB( 0, 1 );              // no color transform
    aBits.writeUB( 1, 1 );              // has a matrix
    aBits.writeUB( 1, 1 );              // places a character
    aBits.writeUB( 0, 1 );              // does not define a character to be moved

    mpTag->addBits( aBits );
    mpTag->addUI16( nDepth );       // depth
    mpTag->addUI16( nID );          // character Id

    // #i73264#
    const basegfx::B2DHomMatrix aMatrix(basegfx::utils::createTranslateB2DHomMatrix(
        Int16_(static_cast<long>(map100thmm(x)*mnDocXScale)),
        Int16_(static_cast<long>(map100thmm(y)*mnDocYScale))));
    mpTag->addMatrix( aMatrix );        // transformation matrix

    endTag();
}


void Writer::removeShape( sal_uInt16 nDepth )
{
    startTag( TAG_REMOVEOBJECT2 );
    mpTag->addUI16( nDepth );           // depth
    endTag();
}


void Writer::startTag( sal_uInt8 nTagId )
{
    DBG_ASSERT( mpTag == nullptr, "Last tag was not ended");

    mpTag.reset( new Tag( nTagId ) );
}


void Writer::endTag()
{
    sal_uInt8 nTag = mpTag->getTagId();

    if( mpSprite && ( (nTag == TAG_END) || (nTag == TAG_SHOWFRAME) || (nTag == TAG_DOACTION) || (nTag == TAG_STARTSOUND) || (nTag == TAG_PLACEOBJECT) || (nTag == TAG_PLACEOBJECT2) || (nTag == TAG_REMOVEOBJECT2) || (nTag == TAG_FRAMELABEL) ) )
    {
        mpSprite->addTag( std::move(mpTag) );
    }
    else
    {
        mpTag->write( *mpMovieStream );
        mpTag.reset();
    }
}


void Writer::showFrame()
{
    startTag( TAG_SHOWFRAME );
    endTag();

    if(nullptr == mpSprite)
        mnFrames++;
}


sal_uInt16 Writer::defineShape( const GDIMetaFile& rMtf )
{
    mpVDev->SetMapMode( rMtf.GetPrefMapMode() );
    Impl_writeActions( rMtf );

    sal_uInt16 nId = 0;
    if (maShapeIds.empty())
        return nId;

    {
        nId = startSprite();
        sal_uInt16 iDepth = 1;
        for (auto const& shape : maShapeIds)
        {
            placeShape( shape, iDepth++, 0, 0 );
        }
        endSprite();
    }

    maShapeIds.clear();

    return nId;
}


sal_uInt16 Writer::defineShape( const tools::Polygon& rPoly, const FillStyle& rFillStyle )
{
    const tools::PolyPolygon aPolyPoly( rPoly );
    return defineShape( aPolyPoly, rFillStyle );
}


sal_uInt16 Writer::defineShape( const tools::PolyPolygon& rPolyPoly, const FillStyle& rFillStyle )
{
    sal_uInt16 nShapeId = createID();

    // start a DefineShape3 tag
    startTag( TAG_DEFINESHAPE3 );

    mpTag->addUI16( nShapeId );
    mpTag->addRect( rPolyPoly.GetBoundRect() );


    // FILLSTYLEARRAY
    mpTag->addUI8( 1 );         // FillStyleCount

    // FILLSTYLE
    rFillStyle.addTo( mpTag.get() );

    // LINESTYLEARRAY
    mpTag->addUI8( 0 );         // LineStyleCount

    // Number of fill and line index bits to 1
    mpTag->addUI8( 0x11 );

    BitStream aBits;

    const sal_uInt16 nCount = rPolyPoly.Count();
    sal_uInt16 i;
    for( i = 0; i < nCount; i++ )
    {
        const tools::Polygon& rPoly = rPolyPoly[ i ];
        if( rPoly.GetSize() )
            Impl_addPolygon( aBits, rPoly, true );
    }

    Impl_addEndShapeRecord( aBits );

    mpTag->addBits( aBits );
    endTag();

    return nShapeId;
}


sal_uInt16 Writer::defineShape( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nLineWidth, const Color& rLineColor )
{
    sal_uInt16 nShapeId = createID();

    // start a DefineShape3 tag
    startTag( TAG_DEFINESHAPE3 );

    mpTag->addUI16( nShapeId );
    mpTag->addRect( rPolyPoly.GetBoundRect() );


    // FILLSTYLEARRAY
    mpTag->addUI8( 0 );         // FillStyleCount

    // LINESTYLEARRAY
    mpTag->addUI8( 1 );         // LineStyleCount

    // LINESTYLE
    mpTag->addUI16( nLineWidth );   // Width of line in twips
    mpTag->addRGBA( rLineColor );   // Color

    // Number of fill and line index bits to 1
    mpTag->addUI8( 0x11 );

    BitStream aBits;

    const sal_uInt16 nCount = rPolyPoly.Count();
    sal_uInt16 i;
    for( i = 0; i < nCount; i++ )
    {
        const tools::Polygon& rPoly = rPolyPoly[ i ];
        if( rPoly.GetSize() )
            Impl_addPolygon( aBits, rPoly, false );
    }

    Impl_addEndShapeRecord( aBits );

    mpTag->addBits( aBits );
    endTag();

    return nShapeId;
}


void Writer::stop()
{
    startTag( TAG_DOACTION );
    mpTag->addUI8( 0x07 );
    mpTag->addUI8( 0 );
    endTag();
}


void Writer::waitOnClick( sal_uInt16 nDepth )
{
    placeShape( uInt16_( mnPageButtonId ), nDepth, 0, 0 );
    stop();
    showFrame();
    removeShape( nDepth );
}


/** inserts a doaction tag with an ActionGotoFrame */
void Writer::gotoFrame( sal_uInt16 nFrame )
{
    startTag( TAG_DOACTION );
    mpTag->addUI8( 0x81 );
    mpTag->addUI16( 2 );
    mpTag->addUI16( nFrame );
    mpTag->addUI8( 0 );
    endTag();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
