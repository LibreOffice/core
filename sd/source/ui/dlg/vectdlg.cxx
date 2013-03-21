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

#include <vcl/vclenum.hxx>
#include <vcl/wrkwin.hxx>

#include "strings.hrc"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include "vectdlg.hxx"
#include "vectdlg.hrc"
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>

#define VECTORIZE_MAX_EXTENT 512

SdVectorizeDlg::SdVectorizeDlg(
    Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell ) :
        ModalDialog     ( pParent, SdResId( DLG_VECTORIZE ) ),
        mpDocSh          ( pDocShell ),
        aGrpSettings    ( this, SdResId( GRP_SETTINGS ) ),
        aFtLayers       ( this, SdResId( FT_LAYERS ) ),
        aNmLayers       ( this, SdResId( NM_LAYERS ) ),
        aFtReduce       ( this, SdResId( FT_REDUCE ) ),
        aMtReduce       ( this, SdResId( MT_REDUCE ) ),
        aFtFillHoles    ( this, SdResId( FT_FILLHOLES ) ),
        aMtFillHoles    ( this, SdResId( MT_FILLHOLES ) ),
        aCbFillHoles    ( this, SdResId( CB_FILLHOLES ) ),
        aFtOriginal     ( this, SdResId( FT_ORIGINAL ) ),
        aBmpWin         ( this, SdResId( CTL_BMP ) ),
        aFtVectorized   ( this, SdResId( FT_VECTORIZED ) ),
        aMtfWin         ( this, SdResId( CTL_WMF ) ),
        aGrpPrgs        ( this, SdResId( GRP_PRGS ) ),
        aPrgs           ( this, SdResId( WND_PRGS ) ),
        aBtnOK          ( this, SdResId( BTN_OK ) ),
        aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, SdResId( BTN_HELP ) ),
        aBtnPreview     ( this, SdResId( BTN_PREVIEW ) ),
        aBmp            ( rBmp )
{
    FreeResource();

    aBtnPreview.SetClickHdl( LINK( this, SdVectorizeDlg, ClickPreviewHdl ) );
    aBtnOK.SetClickHdl( LINK( this, SdVectorizeDlg, ClickOKHdl ) );
    aNmLayers.SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    aMtReduce.SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    aMtFillHoles.SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    aCbFillHoles.SetToggleHdl( LINK( this, SdVectorizeDlg, ToggleHdl ) );

    // disable 3D border
    aBmpWin.SetBorderStyle(WINDOW_BORDER_MONO);
    aMtfWin.SetBorderStyle(WINDOW_BORDER_MONO);

    LoadSettings();
    InitPreviewBmp();
}

SdVectorizeDlg::~SdVectorizeDlg()
{
}

Rectangle SdVectorizeDlg::GetRect( const Size& rDispSize, const Size& rBmpSize ) const
{
    Rectangle aRect;

    if( rBmpSize.Width() && rBmpSize.Height() && rDispSize.Width() && rDispSize.Height() )
    {
        Size         aBmpSize( rBmpSize );
        const double fGrfWH = (double) aBmpSize.Width() / aBmpSize.Height();
        const double fWinWH = (double) rDispSize.Width() / rDispSize.Height();

        if( fGrfWH < fWinWH )
        {
            aBmpSize.Width() = (long) ( rDispSize.Height() * fGrfWH );
            aBmpSize.Height()= rDispSize.Height();
        }
        else
        {
            aBmpSize.Width() = rDispSize.Width();
            aBmpSize.Height()= (long) ( rDispSize.Width() / fGrfWH);
        }

        const Point aBmpPos( ( rDispSize.Width()  - aBmpSize.Width() ) >> 1,
                             ( rDispSize.Height() - aBmpSize.Height() ) >> 1 );

        aRect = Rectangle( aBmpPos, aBmpSize );
    }

    return aRect;
}

void SdVectorizeDlg::InitPreviewBmp()
{
    const Rectangle aRect( GetRect( aBmpWin.GetSizePixel(), aBmp.GetSizePixel() ) );

    aPreviewBmp = aBmp;
    aPreviewBmp.Scale( aRect.GetSize() );
    aBmpWin.SetGraphic( aPreviewBmp );
}

Bitmap SdVectorizeDlg::GetPreparedBitmap( Bitmap& rBmp, Fraction& rScale )
{
    Bitmap      aNew( rBmp );
    const Size  aSizePix( aNew.GetSizePixel() );

    if( aSizePix.Width() > VECTORIZE_MAX_EXTENT || aSizePix.Height() > VECTORIZE_MAX_EXTENT )
    {
        const Rectangle aRect( GetRect( Size( VECTORIZE_MAX_EXTENT, VECTORIZE_MAX_EXTENT ), aSizePix ) );
        rScale = Fraction( aSizePix.Width(), aRect.GetWidth() );
        aNew.Scale( aRect.GetSize() );
    }
    else
        rScale = Fraction( 1, 1 );

    aNew.ReduceColors( (sal_uInt16) aNmLayers.GetValue(), BMP_REDUCE_SIMPLE );

    return aNew;
}

void SdVectorizeDlg::Calculate( Bitmap& rBmp, GDIMetaFile& rMtf )
{
    mpDocSh->SetWaitCursor( sal_True );
    aPrgs.SetValue( 0 );

    Fraction    aScale;
    Bitmap      aTmp( GetPreparedBitmap( rBmp, aScale ) );

    if( !!aTmp )
    {
        const Link aPrgsHdl( LINK( this, SdVectorizeDlg, ProgressHdl ) );
        aTmp.Vectorize( rMtf, (sal_uInt8) aMtReduce.GetValue(), BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES, &aPrgsHdl );

        if( aCbFillHoles.IsChecked() )
        {
            GDIMetaFile         aNewMtf;
            BitmapReadAccess*   pRAcc = aTmp.AcquireReadAccess();

            if( pRAcc )
            {
                const long      nWidth = pRAcc->Width();
                const long      nHeight = pRAcc->Height();
                const long      nTileX = static_cast<long>(aMtFillHoles.GetValue());
                const long      nTileY = static_cast<long>(aMtFillHoles.GetValue());
                const long      nCountX = nWidth / nTileX;
                const long      nCountY = nHeight / nTileY;
                const long      nRestX = nWidth % nTileX;
                const long      nRestY = nHeight % nTileY;

                MapMode aMap( rMtf.GetPrefMapMode() );
                aNewMtf.SetPrefSize( rMtf.GetPrefSize() );
                aNewMtf.SetPrefMapMode( aMap );

                for( long nTY = 0; nTY < nCountY; nTY++ )
                {
                    const long nY = nTY * nTileY;

                    for( long nTX = 0; nTX < nCountX; nTX++ )
                        AddTile( pRAcc, aNewMtf, nTX * nTileX, nTY * nTileY, nTileX, nTileY );

                    if( nRestX )
                        AddTile( pRAcc, aNewMtf, nCountX * nTileX, nY, nRestX, nTileY );
                }

                if( nRestY )
                {
                    const long nY = nCountY * nTileY;

                    for( long nTX = 0; nTX < nCountX; nTX++ )
                        AddTile( pRAcc, aNewMtf, nTX * nTileX, nY, nTileX, nRestY );

                    if( nRestX )
                        AddTile( pRAcc, aNewMtf, nCountX * nTileX, nCountY * nTileY, nRestX, nRestY );
                }


                aTmp.ReleaseAccess( pRAcc );

                for( size_t n = 0, nCount = rMtf.GetActionSize(); n < nCount; n++ )
                    aNewMtf.AddAction( rMtf.GetAction( n )->Clone() );

                aMap.SetScaleX( aMap.GetScaleX() * aScale );
                aMap.SetScaleY( aMap.GetScaleY() * aScale );
                aNewMtf.SetPrefMapMode( aMap );
                rMtf = aNewMtf;
            }
        }
    }

    aPrgs.SetValue( 0 );
    mpDocSh->SetWaitCursor( sal_False );
}

void SdVectorizeDlg::AddTile( BitmapReadAccess* pRAcc, GDIMetaFile& rMtf,
                              long nPosX, long nPosY, long nWidth, long nHeight )
{
    sal_uLong           nSumR = 0UL, nSumG = 0UL, nSumB = 0UL;
    const long      nRight = nPosX + nWidth - 1L;
    const long      nBottom = nPosY + nHeight - 1L;
    const double    fMult = 1.0 / ( nWidth * nHeight );

    for( long nY = nPosY; nY <= nBottom; nY++ )
    {
        for( long nX = nPosX; nX <= nRight; nX++ )
        {
            const BitmapColor aPixel( pRAcc->GetColor( nY, nX ) );

            nSumR += aPixel.GetRed();
            nSumG += aPixel.GetGreen();
            nSumB += aPixel.GetBlue();
        }
    }

    const Color aColor( (sal_uInt8) FRound( nSumR * fMult ),
                        (sal_uInt8) FRound( nSumG * fMult ),
                        (sal_uInt8) FRound( nSumB * fMult ) );

    Rectangle   aRect( Point( nPosX, nPosY ), Size( nWidth + 1, nHeight + 1 ) );
    const Size& rMaxSize = rMtf.GetPrefSize();

    aRect = PixelToLogic( aRect, rMtf.GetPrefMapMode() );

    if( aRect.Right() > ( rMaxSize.Width() - 1L ) )
        aRect.Right() = rMaxSize.Width() - 1L;

    if( aRect.Bottom() > ( rMaxSize.Height() - 1L ) )
        aRect.Bottom() = rMaxSize.Height() - 1L;

    rMtf.AddAction( new MetaLineColorAction( aColor, sal_True ) );
    rMtf.AddAction( new MetaFillColorAction( aColor, sal_True ) );
    rMtf.AddAction( new MetaRectAction( aRect ) );
}

IMPL_LINK( SdVectorizeDlg, ProgressHdl, void*, pData )
{
    aPrgs.SetValue( (sal_uInt16)(sal_uLong) pData );
    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickPreviewHdl)
{
    Calculate( aBmp, aMtf );
    aMtfWin.SetGraphic( aMtf );
    aBtnPreview.Disable();

    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickOKHdl)
{
    if( aBtnPreview.IsEnabled() )
        Calculate( aBmp, aMtf );

    SaveSettings();
    EndDialog( RET_OK );

    return 0L;
}

IMPL_LINK( SdVectorizeDlg, ToggleHdl, CheckBox*, pCb )
{
    if( pCb->IsChecked() )
    {
        aFtFillHoles.Enable();
        aMtFillHoles.Enable();
    }
    else
    {
        aFtFillHoles.Disable();
        aMtFillHoles.Disable();
    }

    ModifyHdl( NULL );

    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ModifyHdl)
{
    aBtnPreview.Enable();
    return 0L;
}

void SdVectorizeDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream(
                               rtl::OUString( SD_OPTION_VECTORIZE ) ,
                               SD_OPTION_LOAD ) );
    sal_uInt16              nLayers;
    sal_uInt16              nReduce;
    sal_uInt16              nFillHoles;
    sal_Bool                bFillHoles;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );
        *xIStm >> nLayers >> nReduce >> nFillHoles >> bFillHoles;
    }
    else
    {
        nLayers = 8;
        nReduce = 0;
        nFillHoles = 32;
        bFillHoles = sal_False;
    }

    aNmLayers.SetValue( nLayers );
    aMtReduce.SetValue( nReduce );
    aMtFillHoles.SetValue( nFillHoles );
    aCbFillHoles.Check( bFillHoles );

    ToggleHdl( &aCbFillHoles );
}

void SdVectorizeDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream(
                              rtl::OUString(SD_OPTION_VECTORIZE)  ,
                              SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );
        *xOStm << (sal_uInt16) aNmLayers.GetValue() << (sal_uInt16) aMtReduce.GetValue();
        *xOStm << (sal_uInt16) aMtFillHoles.GetValue() << aCbFillHoles.IsChecked();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
