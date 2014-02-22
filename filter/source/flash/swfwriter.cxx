/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
:   mpClipPolyPolygon( NULL ),
    mpTag( NULL ),
    mpSprite( NULL ),
    mnNextId( 1 ),
    mnGlobalTransparency(0),
    mnJPEGCompressMode(nJPEGcompressMode)
{
    mpVDev = new VirtualDevice;
    mpVDev->EnableOutput( false );

    maMovieTempFile.EnableKillingFile();
    maFontsTempFile.EnableKillingFile();

    mpMovieStream = maMovieTempFile.GetStream( STREAM_WRITE|STREAM_TRUNC );
    mpFontsStream = maFontsTempFile.GetStream( STREAM_WRITE|STREAM_TRUNC );

    mnFrames = 0;

    mnDocWidth = map100thmm( nDocWidthInput );
    mnDocHeight = map100thmm( nDocHeightInput );

    mnDocXScale = (double)nTWIPWidthOutput / mnDocWidth;
    mnDocYScale = (double)nTWIPHeightOutput / mnDocHeight;

#ifndef AUGUSTUS
    
    Rectangle aRect( 0, 0, (long)( mnDocWidth * mnDocXScale ), (long)( mnDocHeight * mnDocYScale ) );
    Polygon aPoly( aRect );
    FillStyle aFill = FillStyle( Color(COL_WHITE) );
    mnWhiteBackgroundShapeId = defineShape( aPoly, aFill );

    ::basegfx::B2DHomMatrix m; 
    mnPageButtonId = createID();
    startTag( TAG_DEFINEBUTTON );
    mpTag->addUI16( mnPageButtonId );           

    
    mpTag->addUI8( 0x08 );                      
    mpTag->addUI16( mnWhiteBackgroundShapeId ); 
    mpTag->addUI16( 0 );                        
    mpTag->addMatrix( m );                      
    mpTag->addUI8( 0 );                         



    
    mpTag->addUI8( 0x06 );                      
    mpTag->addUI8( 0 );                         

    endTag();

    

#endif
}



Writer::~Writer()
{
    delete mpVDev;
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

    
    mpMovieStream->WriteUInt16( (sal_uInt16)0 );

    Tag aHeader( 0xff );

    aHeader.addUI8( 'F' );
    aHeader.addUI8( 'W' );
    aHeader.addUI8( 'S' );
    aHeader.addUI8( 5 );

    sal_uInt32 nSizePos = aHeader.Tell();

    aHeader.WriteUInt32( (sal_uInt32)0 );

    Rectangle aDocRect( 0, 0, static_cast<long>(mnDocWidth*mnDocXScale), static_cast<long>(mnDocHeight*mnDocYScale) );

    aHeader.addRect( aDocRect );

    
    aHeader.addUI8( 0 );
    aHeader.addUI8( 12 );

    aHeader.addUI16( _uInt16(mnFrames) );

    const sal_uInt32 nSize = aHeader.Tell() + mpFontsStream->Tell() + mpMovieStream->Tell();

    aHeader.Seek( nSizePos );
    aHeader.WriteUInt32( (sal_uInt32)nSize );

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
            mpSprite = NULL;
    }
}



void Writer::placeShape( sal_uInt16 nID, sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y, sal_uInt16 nClip, const char* pName )
{
    startTag( TAG_PLACEOBJECT2 );

    BitStream aBits;

    aBits.writeUB( sal_uInt32(nClip != 0), 1 ); 
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( sal_uInt32(pName != NULL), 1 ); 
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 1, 1 );              
    aBits.writeUB( 1, 1 );              
    aBits.writeUB( 0, 1 );              

    mpTag->addBits( aBits );
    mpTag->addUI16( nDepth );       
    mpTag->addUI16( nID );          

    
    const basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
        _Int16(static_cast<long>(map100thmm(x)*mnDocXScale)),
        _Int16(static_cast<long>(map100thmm(y)*mnDocYScale))));
    mpTag->addMatrix( aMatrix );        

    if( pName )
        mpTag->addString( pName );

    if( nClip != 0 )
        mpTag->addUI16( nClip );

    endTag();
}

#ifdef THEFUTURE


void Writer::moveShape( sal_uInt16 nDepth, sal_Int32 x, sal_Int32 y )
{
    startTag( TAG_PLACEOBJECT2 );

    BitStream aBits;
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 1, 1 );              
    aBits.writeUB( 0, 1 );              
    aBits.writeUB( 1, 1 );              

    mpTag->addBits( aBits );
    mpTag->addUI16( nDepth );           

    
    const basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
        _Int16(static_cast<long>(map100thmm(x)*mnDocXScale)),
        _Int16(static_cast<long>(map100thmm(y)*mnDocYScale))));
    mpTag->addMatrix( aMatrix );        

    endTag();
}
#endif



void Writer::removeShape( sal_uInt16 nDepth )
{
    startTag( TAG_REMOVEOBJECT2 );
    mpTag->addUI16( nDepth );           
    endTag();
}



void Writer::startTag( sal_uInt8 nTagId )
{
    DBG_ASSERT( mpTag == NULL, "Last tag was not ended");

    mpTag = new Tag( nTagId );
}



void Writer::endTag()
{
    sal_uInt8 nTag = mpTag->getTagId();

    if( mpSprite && ( (nTag == TAG_END) || (nTag == TAG_SHOWFRAME) || (nTag == TAG_DOACTION) || (nTag == TAG_STARTSOUND) || (nTag == TAG_PLACEOBJECT) || (nTag == TAG_PLACEOBJECT2) || (nTag == TAG_REMOVEOBJECT2) || (nTag == TAG_FRAMELABEL) ) )
    {
        mpSprite->addTag( mpTag );
        mpTag = NULL;
    }
    else
    {
        mpTag->write( *mpMovieStream );
        delete mpTag;
        mpTag = NULL;
    }
}



sal_uInt16 Writer::createID()
{
    return mnNextId++;
}



void Writer::showFrame()
{
    startTag( TAG_SHOWFRAME );
    endTag();

    if(NULL == mpSprite)
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

        sal_Bool bHaveShapes = aIter != aEnd;

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



sal_uInt16 Writer::defineShape( const Polygon& rPoly, const FillStyle& rFillStyle )
{
    const PolyPolygon aPolyPoly( rPoly );
    return defineShape( aPolyPoly, rFillStyle );
}



sal_uInt16 Writer::defineShape( const PolyPolygon& rPolyPoly, const FillStyle& rFillStyle )
{
    sal_uInt16 nShapeId = createID();

    
    startTag( TAG_DEFINESHAPE3 );

    mpTag->addUI16( nShapeId );
    mpTag->addRect( rPolyPoly.GetBoundRect() );


    
    mpTag->addUI8( 1 );         

    
    rFillStyle.addTo( mpTag );

    
    mpTag->addUI8( 0 );         

    
    mpTag->addUI8( 0x11 );

    BitStream aBits;

    const sal_uInt16 nCount = rPolyPoly.Count();
    sal_uInt16 i;
    for( i = 0; i < nCount; i++ )
    {
        const Polygon& rPoly = rPolyPoly[ i ];
        if( rPoly.GetSize() )
            Impl_addPolygon( aBits, rPoly, true );
    }

    Impl_addEndShapeRecord( aBits );

    mpTag->addBits( aBits );
    endTag();

    return nShapeId;
}



sal_uInt16 Writer::defineShape( const PolyPolygon& rPolyPoly, sal_uInt16 nLineWidth, const Color& rLineColor )
{
    sal_uInt16 nShapeId = createID();

    
    startTag( TAG_DEFINESHAPE3 );

    mpTag->addUI16( nShapeId );
    mpTag->addRect( rPolyPoly.GetBoundRect() );


    
    mpTag->addUI8( 0 );         

    
    mpTag->addUI8( 1 );         

    
    mpTag->addUI16( nLineWidth );   
    mpTag->addRGBA( rLineColor );   

    
    mpTag->addUI8( 0x11 );

    BitStream aBits;

    const sal_uInt16 nCount = rPolyPoly.Count();
    sal_uInt16 i;
    for( i = 0; i < nCount; i++ )
    {
        const Polygon& rPoly = rPolyPoly[ i ];
        if( rPoly.GetSize() )
            Impl_addPolygon( aBits, rPoly, false );
    }

    Impl_addEndShapeRecord( aBits );

    mpTag->addBits( aBits );
    endTag();

    return nShapeId;
}

#ifdef AUGUSTUS
enum {NO_COMPRESSION, ADPCM_COMPRESSION, MP3_COMPRESSION } COMPRESSION_TYPE;
sal_Bool Writer::streamSound( const char * filename )
{
    SF_INFO      info;
    SNDFILE *sf = sf_open(filename, SFM_READ, &info);

    if (NULL == sf)
        return sal_False;
    else
    {
        
        m_lame_flags = lame_init();

        
        
        

        lame_set_num_channels(m_lame_flags,1);
        lame_set_in_samplerate(m_lame_flags,22050);
        lame_set_brate(m_lame_flags,48);
        lame_set_mode(m_lame_flags,MONO);
        lame_set_quality(m_lame_flags,2);   /* 2=high  5 = medium  7=low */

        
        
        
        

        
        

        int ret_code = lame_init_params(m_lame_flags);

        if (ret_code < 0)
            throw 0;

        int samples_per_frame = 22050 / 12; 
        int mp3buffer_size = static_cast<int>(samples_per_frame*1.25 + 7200 + 7200);


        startTag(TAG_SOUNDSTREAMHEAD2);

        mpTag->addUI8(2<<2 | 1<<1 | 0<<0);  

        BitStream bs;

        bs.writeUB(MP3_COMPRESSION,4);
        bs.writeUB(2, 2);  
        bs.writeUB(1, 1);  
        bs.writeUB(0, 1);  

        mpTag->addBits(bs);

        mpTag->addUI16(samples_per_frame);
        endTag();

        short *sample_buff = new short[static_cast<int>(info.frames)];
        sf_readf_short(sf, sample_buff, info.frames);

        unsigned char* mp3buffer = new unsigned char[mp3buffer_size];








//




        for (int samples_written = 0; samples_written < info.frames; samples_written += samples_per_frame)
        {
            startTag(TAG_SOUNDSTREAMBLOCK);

            int samples_to_write = std::min((int)info.frames - samples_written, samples_per_frame);

            
            
            int ret = lame_encode_buffer(m_lame_flags, sample_buff + samples_written,
                                            sample_buff + samples_written,
                                            samples_to_write, mp3buffer, mp3buffer_size);

            if (ret < 0)
                throw 0;





            if (mp3buffer_size - ret < 7200)
                throw 0;

            int ret2 = lame_encode_flush(m_lame_flags, mp3buffer + ret, mp3buffer_size - ret);

            if (ret2 < 0)
                throw 0;


            SvMemoryStream strm(mp3buffer, ret + ret2, STREAM_READWRITE);

            mpTag->addUI16(samples_to_write);
            mpTag->addUI16(0);
            mpTag->addStream(strm);

            endTag();

            showFrame();
        }


        delete[] mp3buffer;

        delete[] sample_buff;
        sf_close(sf);

        
        lame_close(m_lame_flags);
    }

    return sal_True;
}
#endif 




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
