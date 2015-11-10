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

using namespace ::swf;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;



static MapMode aTWIPSMode( MAP_TWIP );
static MapMode a100thmmMode( MAP_100TH_MM );

static sal_Int32 map100thmm( sal_Int32 n100thMM )
{
    Point aPoint( n100thMM, n100thMM );
    sal_Int32 nX = OutputDevice::LogicToLogic( aPoint,  a100thmmMode, aTWIPSMode ).X();
    return nX;
}



Writer::Writer( sal_Int32 nTWIPWidthOutput, sal_Int32 nTWIPHeightOutput, sal_Int32 nDocWidthInput, sal_Int32 nDocHeightInput, sal_Int32 nJPEGcompressMode )
:   mpClipPolyPolygon( nullptr ),
    mpTag( nullptr ),
    mpSprite( nullptr ),
    mnNextId( 1 ),
    mnGlobalTransparency(0),
    mnJPEGCompressMode(nJPEGcompressMode)
{
    mpVDev->EnableOutput( false );

    maMovieTempFile.EnableKillingFile();
    maFontsTempFile.EnableKillingFile();

    mpMovieStream = maMovieTempFile.GetStream( StreamMode::WRITE|StreamMode::TRUNC );
    mpFontsStream = maFontsTempFile.GetStream( StreamMode::WRITE|StreamMode::TRUNC );

    mnFrames = 0;

    mnDocWidth = map100thmm( nDocWidthInput );
    mnDocHeight = map100thmm( nDocHeightInput );

    mnDocXScale = (double)nTWIPWidthOutput / mnDocWidth;
    mnDocYScale = (double)nTWIPHeightOutput / mnDocHeight;

    // define an invisible button with the size of a page
    Rectangle aRect( 0, 0, (long)( mnDocWidth * mnDocXScale ), (long)( mnDocHeight * mnDocYScale ) );
    tools::Polygon aPoly( aRect );
    FillStyle aFill = FillStyle( Color(COL_WHITE) );
    mnWhiteBackgroundShapeId = defineShape( aPoly, aFill );

    ::basegfx::B2DHomMatrix m; // #i73264#
    mnPageButtonId = createID();
    startTag( TAG_DEFINEBUTTON );
    mpTag->addUI16( mnPageButtonId );           // character id for button

    // button records
    mpTag->addUI8( 0x08 );                      // only hit state
    mpTag->addUI16( mnWhiteBackgroundShapeId ); // shape id of background rectangle
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
    delete mpSprite;
    delete mpTag;
}



void ImplCopySvStreamToXOutputStream( SvStream& rIn, Reference< XOutputStream > &xOut )
{
    sal_uInt32 nBufferSize = 64*1024;

    rIn.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nSize = rIn.Tell();
    rIn.Seek( STREAM_SEEK_TO_BEGIN );

    Sequence< sal_Int8 > aBuffer( min( nBufferSize, nSize ) );

    while( nSize )
    {
        if( nSize < nBufferSize )
        {
            nBufferSize = nSize;
            aBuffer.realloc( nSize );
        }

        sal_uInt32 nRead = rIn.Read( aBuffer.getArray(), nBufferSize );
        DBG_ASSERT( nRead == nBufferSize, "ImplCopySvStreamToXOutputStream failed!" );
        xOut->writeBytes( aBuffer );

        if( nRead == 0 )
            break;

        nSize -= nRead;
    }
}



void Writer::storeTo( Reference< XOutputStream > &xOutStream )
{
    for(FontMap::iterator i = maFonts.begin(); i != maFonts.end(); ++i)
    {
        FlashFont* pFont = (*i);
        pFont->write( *mpFontsStream );
        delete pFont;
    }

    // Endtag
    mpMovieStream->WriteUInt16( 0 );

    Tag aHeader( 0xff );

    aHeader.addUI8( 'F' );
    aHeader.addUI8( 'W' );
    aHeader.addUI8( 'S' );
    aHeader.addUI8( 5 );

    sal_uInt32 nSizePos = aHeader.Tell();

    aHeader.WriteUInt32( 0 );

    Rectangle aDocRect( 0, 0, static_cast<long>(mnDocWidth*mnDocXScale), static_cast<long>(mnDocHeight*mnDocYScale) );

    aHeader.addRect( aDocRect );

    // frame delay in 8.8 fixed number of frames per second
    aHeader.addUI8( 0 );
    aHeader.addUI8( 12 );

    aHeader.addUI16( _uInt16(mnFrames) );

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
    mvSpriteStack.push(mpSprite);
    mpSprite = new Sprite( nShapeId );
    return nShapeId;
}



void Writer::endSprite()
{
    if( mpSprite )
    {
        startTag( TAG_END );
        endTag();

        mpSprite->write( *mpMovieStream );
        delete mpSprite;

        if (!mvSpriteStack.empty())
        {
            mpSprite = mvSpriteStack.top();
            mvSpriteStack.pop();
        }
        else
            mpSprite = nullptr;
    }
}



void Writer::placeShape( sal_uInt16 nID, sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y, sal_uInt16 nClip, const char* pName )
{
    startTag( TAG_PLACEOBJECT2 );

    BitStream aBits;

    aBits.writeUB( sal_uInt32(nClip != 0), 1 ); // Has Clip Actions?
    aBits.writeUB( 0, 1 );              // reserved
    aBits.writeUB( sal_uInt32(pName != nullptr), 1 ); // has a name
    aBits.writeUB( 0, 1 );              // no ratio
    aBits.writeUB( 0, 1 );              // no color transform
    aBits.writeUB( 1, 1 );              // has a matrix
    aBits.writeUB( 1, 1 );              // places a character
    aBits.writeUB( 0, 1 );              // does not define a character to be moved

    mpTag->addBits( aBits );
    mpTag->addUI16( nDepth );       // depth
    mpTag->addUI16( nID );          // character Id

    // #i73264#
    const basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
        _Int16(static_cast<long>(map100thmm(x)*mnDocXScale)),
        _Int16(static_cast<long>(map100thmm(y)*mnDocYScale))));
    mpTag->addMatrix( aMatrix );        // transformation matrix

    if( pName )
        mpTag->addString( pName );

    if( nClip != 0 )
        mpTag->addUI16( nClip );

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

    mpTag = new Tag( nTagId );
}



void Writer::endTag()
{
    sal_uInt8 nTag = mpTag->getTagId();

    if( mpSprite && ( (nTag == TAG_END) || (nTag == TAG_SHOWFRAME) || (nTag == TAG_DOACTION) || (nTag == TAG_STARTSOUND) || (nTag == TAG_PLACEOBJECT) || (nTag == TAG_PLACEOBJECT2) || (nTag == TAG_REMOVEOBJECT2) || (nTag == TAG_FRAMELABEL) ) )
    {
        mpSprite->addTag( mpTag );
        mpTag = nullptr;
    }
    else
    {
        mpTag->write( *mpMovieStream );
        delete mpTag;
        mpTag = nullptr;
    }
}



void Writer::showFrame()
{
    startTag( TAG_SHOWFRAME );
    endTag();

    if(nullptr == mpSprite)
        mnFrames++;
}



sal_uInt16 Writer::defineShape( const GDIMetaFile& rMtf, sal_Int16 x, sal_Int16 y )
{
    mpVDev->SetMapMode( rMtf.GetPrefMapMode() );
    Impl_writeActions( rMtf );

    sal_uInt16 nId = 0;
    {
        CharacterIdVector::iterator aIter( maShapeIds.begin() );
        const CharacterIdVector::iterator aEnd( maShapeIds.end() );

        bool bHaveShapes = aIter != aEnd;

        if (bHaveShapes)
        {
            nId = startSprite();

            sal_uInt16 iDepth = 1;
            for(; aIter != aEnd; ++aIter)
            {
                placeShape( *aIter, iDepth++, x, y );
            }

            endSprite();
        }
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
    rFillStyle.addTo( mpTag );

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
    placeShape( _uInt16( mnPageButtonId ), nDepth, 0, 0 );
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
