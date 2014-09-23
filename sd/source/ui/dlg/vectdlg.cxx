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
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>

#define VECTORIZE_MAX_EXTENT 512

SdVectorizeDlg::SdVectorizeDlg(vcl::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell)
    : ModalDialog(pParent, "VectorizeDialog", "modules/sdraw/ui/vectorize.ui")
    , mpDocSh(pDocShell)
    , aBmp(rBmp)
{
    get(m_pNmLayers, "colors");
    get(m_pMtReduce, "points");
    get(m_pCbFillHoles, "fillholes");
    get(m_pFtFillHoles, "tilesft");
    get(m_pMtFillHoles, "tiles");
    get(m_pBmpWin, "source");
    get(m_pMtfWin, "vectorized");

    Size aSize(LogicToPixel(Size(92, 100), MAP_APPFONT));
    m_pBmpWin->set_width_request(aSize.Width());
    m_pMtfWin->set_width_request(aSize.Width());
    m_pBmpWin->set_height_request(aSize.Height());
    m_pMtfWin->set_height_request(aSize.Height());

    get(m_pPrgs, "progressbar");
    get(m_pBtnPreview, "preview");
    get(m_pBtnOK, "ok");

    m_pBtnPreview->SetClickHdl( LINK( this, SdVectorizeDlg, ClickPreviewHdl ) );
    m_pBtnOK->SetClickHdl( LINK( this, SdVectorizeDlg, ClickOKHdl ) );
    m_pNmLayers->SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    m_pMtReduce->SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    m_pMtFillHoles->SetModifyHdl( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    m_pCbFillHoles->SetToggleHdl( LINK( this, SdVectorizeDlg, ToggleHdl ) );

    // disable 3D border
    m_pBmpWin->SetBorderStyle(WINDOW_BORDER_MONO);
    m_pMtfWin->SetBorderStyle(WINDOW_BORDER_MONO);

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
    const Rectangle aRect( GetRect( m_pBmpWin->GetSizePixel(), aBmp.GetSizePixel() ) );

    aPreviewBmp = aBmp;
    aPreviewBmp.Scale( aRect.GetSize() );
    m_pBmpWin->SetGraphic( aPreviewBmp );
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

    aNew.ReduceColors( (sal_uInt16) m_pNmLayers->GetValue(), BMP_REDUCE_SIMPLE );

    return aNew;
}

void SdVectorizeDlg::Calculate( Bitmap& rBmp, GDIMetaFile& rMtf )
{
    mpDocSh->SetWaitCursor( true );
    m_pPrgs->SetValue( 0 );

    Fraction    aScale;
    Bitmap      aTmp( GetPreparedBitmap( rBmp, aScale ) );

    if( !!aTmp )
    {
        const Link aPrgsHdl( LINK( this, SdVectorizeDlg, ProgressHdl ) );
        aTmp.Vectorize( rMtf, (sal_uInt8) m_pMtReduce->GetValue(), BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES, &aPrgsHdl );

        if( m_pCbFillHoles->IsChecked() )
        {
            GDIMetaFile         aNewMtf;
            BitmapReadAccess*   pRAcc = aTmp.AcquireReadAccess();

            if( pRAcc )
            {
                const long      nWidth = pRAcc->Width();
                const long      nHeight = pRAcc->Height();
                const long      nTileX = static_cast<long>(m_pMtFillHoles->GetValue());
                const long      nTileY = static_cast<long>(m_pMtFillHoles->GetValue());
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

    m_pPrgs->SetValue( 0 );
    mpDocSh->SetWaitCursor( false );
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

    rMtf.AddAction( new MetaLineColorAction( aColor, true ) );
    rMtf.AddAction( new MetaFillColorAction( aColor, true ) );
    rMtf.AddAction( new MetaRectAction( aRect ) );
}

IMPL_LINK( SdVectorizeDlg, ProgressHdl, void*, pData )
{
    m_pPrgs->SetValue( (sal_uInt16)(sal_uLong) pData );
    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickPreviewHdl)
{
    Calculate( aBmp, aMtf );
    m_pMtfWin->SetGraphic( aMtf );
    m_pBtnPreview->Disable();

    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickOKHdl)
{
    if( m_pBtnPreview->IsEnabled() )
        Calculate( aBmp, aMtf );

    SaveSettings();
    EndDialog( RET_OK );

    return 0L;
}

IMPL_LINK( SdVectorizeDlg, ToggleHdl, CheckBox*, pCb )
{
    if( pCb->IsChecked() )
    {
        m_pFtFillHoles->Enable();
        m_pMtFillHoles->Enable();
    }
    else
    {
        m_pFtFillHoles->Disable();
        m_pMtFillHoles->Disable();
    }

    ModifyHdl( NULL );

    return 0L;
}

IMPL_LINK_NOARG(SdVectorizeDlg, ModifyHdl)
{
    m_pBtnPreview->Enable();
    return 0L;
}

void SdVectorizeDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream(
                               OUString( SD_OPTION_VECTORIZE ) ,
                               SD_OPTION_LOAD ) );
    sal_uInt16              nLayers;
    sal_uInt16              nReduce;
    sal_uInt16              nFillHoles;
    bool                bFillHoles;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );
        xIStm->ReadUInt16( nLayers ).ReadUInt16( nReduce ).ReadUInt16( nFillHoles ).ReadCharAsBool( bFillHoles );
    }
    else
    {
        nLayers = 8;
        nReduce = 0;
        nFillHoles = 32;
        bFillHoles = false;
    }

    m_pNmLayers->SetValue( nLayers );
    m_pMtReduce->SetValue( nReduce );
    m_pMtFillHoles->SetValue( nFillHoles );
    m_pCbFillHoles->Check( bFillHoles );

    ToggleHdl(m_pCbFillHoles);
}

void SdVectorizeDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream(
                              OUString(SD_OPTION_VECTORIZE)  ,
                              SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );
        xOStm->WriteUInt16( (sal_uInt16) m_pNmLayers->GetValue() ).WriteUInt16( (sal_uInt16) m_pMtReduce->GetValue() );
        xOStm->WriteUInt16( (sal_uInt16) m_pMtFillHoles->GetValue() ).WriteUChar( m_pCbFillHoles->IsChecked() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
