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
#include <vcl/bitmapaccess.hxx>
#include <vcl/metaact.hxx>
#include <vcl/BitmapSimpleColorQuantizationFilter.hxx>

#include <DrawDocShell.hxx>
#include <sdmod.hxx>
#include <sdiocmpt.hxx>
#include <vectdlg.hxx>

#define VECTORIZE_MAX_EXTENT 512

SdVectorizeDlg::SdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell)
    : GenericDialogController(pParent, "modules/sdraw/ui/vectorize.ui", "VectorizeDialog")
    , m_pDocSh(pDocShell)
    , aBmp(rBmp)
    , m_xNmLayers(m_xBuilder->weld_spin_button("colors"))
    , m_xMtReduce(m_xBuilder->weld_metric_spin_button("points", FieldUnit::PIXEL))
    , m_xFtFillHoles(m_xBuilder->weld_label("tilesft"))
    , m_xMtFillHoles(m_xBuilder->weld_metric_spin_button("tiles", FieldUnit::PIXEL))
    , m_xCbFillHoles(m_xBuilder->weld_check_button("fillholes"))
    , m_xBmpWin(new weld::CustomWeld(*m_xBuilder, "source", m_aBmpWin))
    , m_xMtfWin(new weld::CustomWeld(*m_xBuilder, "vectorized", m_aMtfWin))
    , m_xPrgs(m_xBuilder->weld_progress_bar("progressbar"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
    , m_xBtnPreview(m_xBuilder->weld_button("preview"))
{
    const int nWidth = m_xFtFillHoles->get_approximate_digit_width() * 32;
    const int nHeight = m_xFtFillHoles->get_text_height() * 16;
    m_xBmpWin->set_size_request(nWidth, nHeight);
    m_xMtfWin->set_size_request(nWidth, nHeight);

    m_xBtnPreview->connect_clicked( LINK( this, SdVectorizeDlg, ClickPreviewHdl ) );
    m_xBtnOK->connect_clicked( LINK( this, SdVectorizeDlg, ClickOKHdl ) );
    m_xNmLayers->connect_value_changed( LINK( this, SdVectorizeDlg, ModifyHdl ) );
    m_xMtReduce->connect_value_changed( LINK( this, SdVectorizeDlg, MetricModifyHdl ) );
    m_xMtFillHoles->connect_value_changed( LINK( this, SdVectorizeDlg, MetricModifyHdl ) );
    m_xCbFillHoles->connect_toggled( LINK( this, SdVectorizeDlg, ToggleHdl ) );

    LoadSettings();
    InitPreviewBmp();
}

SdVectorizeDlg::~SdVectorizeDlg()
{
}

::tools::Rectangle SdVectorizeDlg::GetRect( const Size& rDispSize, const Size& rBmpSize )
{
    ::tools::Rectangle aRect;

    if( rBmpSize.Width() && rBmpSize.Height() && rDispSize.Width() && rDispSize.Height() )
    {
        Size         aBmpSize( rBmpSize );
        const double fGrfWH = static_cast<double>(aBmpSize.Width()) / aBmpSize.Height();
        const double fWinWH = static_cast<double>(rDispSize.Width()) / rDispSize.Height();

        if( fGrfWH < fWinWH )
        {
            aBmpSize.setWidth( static_cast<long>( rDispSize.Height() * fGrfWH ) );
            aBmpSize.setHeight( rDispSize.Height() );
        }
        else
        {
            aBmpSize.setWidth( rDispSize.Width() );
            aBmpSize.setHeight( static_cast<long>( rDispSize.Width() / fGrfWH) );
        }

        const Point aBmpPos( ( rDispSize.Width()  - aBmpSize.Width() ) >> 1,
                             ( rDispSize.Height() - aBmpSize.Height() ) >> 1 );

        aRect = ::tools::Rectangle( aBmpPos, aBmpSize );
    }

    return aRect;
}

void SdVectorizeDlg::InitPreviewBmp()
{
    const ::tools::Rectangle aRect( GetRect( m_aBmpWin.GetOutputSizePixel(), aBmp.GetSizePixel() ) );

    aPreviewBmp = aBmp;
    aPreviewBmp.Scale( aRect.GetSize() );
    m_aBmpWin.SetGraphic( aPreviewBmp );
}

Bitmap SdVectorizeDlg::GetPreparedBitmap( Bitmap const & rBmp, Fraction& rScale )
{
    Bitmap      aNew( rBmp );
    const Size  aSizePix( aNew.GetSizePixel() );

    if( aSizePix.Width() > VECTORIZE_MAX_EXTENT || aSizePix.Height() > VECTORIZE_MAX_EXTENT )
    {
        const ::tools::Rectangle aRect( GetRect( Size( VECTORIZE_MAX_EXTENT, VECTORIZE_MAX_EXTENT ), aSizePix ) );
        rScale = Fraction( aSizePix.Width(), aRect.GetWidth() );
        aNew.Scale( aRect.GetSize() );
    }
    else
        rScale = Fraction( 1, 1 );

    BitmapEx aNewBmpEx(aNew);
    BitmapFilter::Filter(aNewBmpEx, BitmapSimpleColorQuantizationFilter(m_xNmLayers->get_value()));
    aNew = aNewBmpEx.GetBitmap();

    return aNew;
}

void SdVectorizeDlg::Calculate( Bitmap const & rBmp, GDIMetaFile& rMtf )
{
    m_pDocSh->SetWaitCursor( true );
    m_xPrgs->set_percentage(0);

    Fraction    aScale;
    Bitmap      aTmp( GetPreparedBitmap( rBmp, aScale ) );

    if( !!aTmp )
    {
        const Link<long,void> aPrgsHdl( LINK( this, SdVectorizeDlg, ProgressHdl ) );
        aTmp.Vectorize( rMtf, static_cast<sal_uInt8>(m_xMtReduce->get_value(FieldUnit::NONE)), &aPrgsHdl );

        if (m_xCbFillHoles->get_active())
        {
            GDIMetaFile                 aNewMtf;
            Bitmap::ScopedReadAccess    pRAcc(aTmp);

            if( pRAcc )
            {
                const long      nWidth = pRAcc->Width();
                const long      nHeight = pRAcc->Height();
                const long      nTileX = m_xMtFillHoles->get_value(FieldUnit::NONE);
                const long      nTileY = m_xMtFillHoles->get_value(FieldUnit::NONE);
                assert(nTileX && "div-by-zero");
                const long      nCountX = nWidth / nTileX;
                assert(nTileY && "div-by-zero");
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
                        AddTile( pRAcc.get(), aNewMtf, nTX * nTileX, nTY * nTileY, nTileX, nTileY );

                    if( nRestX )
                        AddTile( pRAcc.get(), aNewMtf, nCountX * nTileX, nY, nRestX, nTileY );
                }

                if( nRestY )
                {
                    const long nY = nCountY * nTileY;

                    for( long nTX = 0; nTX < nCountX; nTX++ )
                        AddTile( pRAcc.get(), aNewMtf, nTX * nTileX, nY, nTileX, nRestY );

                    if( nRestX )
                        AddTile( pRAcc.get(), aNewMtf, nCountX * nTileX, nCountY * nTileY, nRestX, nRestY );
                }

                pRAcc.reset();

                for( size_t n = 0, nCount = rMtf.GetActionSize(); n < nCount; n++ )
                    aNewMtf.AddAction( rMtf.GetAction( n )->Clone() );

                aMap.SetScaleX( aMap.GetScaleX() * aScale );
                aMap.SetScaleY( aMap.GetScaleY() * aScale );
                aNewMtf.SetPrefMapMode( aMap );
                rMtf = aNewMtf;
            }
        }
    }

    m_xPrgs->set_percentage(0);
    m_pDocSh->SetWaitCursor( false );
}

void SdVectorizeDlg::AddTile( BitmapReadAccess const * pRAcc, GDIMetaFile& rMtf,
                              long nPosX, long nPosY, long nWidth, long nHeight )
{
    sal_uLong           nSumR = 0, nSumG = 0, nSumB = 0;
    const long      nRight = nPosX + nWidth - 1;
    const long      nBottom = nPosY + nHeight - 1;
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

    const Color aColor( static_cast<sal_uInt8>(FRound( nSumR * fMult )),
                        static_cast<sal_uInt8>(FRound( nSumG * fMult )),
                        static_cast<sal_uInt8>(FRound( nSumB * fMult )) );

    ::tools::Rectangle   aRect( Point( nPosX, nPosY ), Size( nWidth + 1, nHeight + 1 ) );
    const Size& rMaxSize = rMtf.GetPrefSize();

    aRect = Application::GetDefaultDevice()->PixelToLogic(aRect, rMtf.GetPrefMapMode());

    if( aRect.Right() > ( rMaxSize.Width() - 1 ) )
        aRect.SetRight( rMaxSize.Width() - 1 );

    if( aRect.Bottom() > ( rMaxSize.Height() - 1 ) )
        aRect.SetBottom( rMaxSize.Height() - 1 );

    rMtf.AddAction( new MetaLineColorAction( aColor, true ) );
    rMtf.AddAction( new MetaFillColorAction( aColor, true ) );
    rMtf.AddAction( new MetaRectAction( aRect ) );
}

IMPL_LINK( SdVectorizeDlg, ProgressHdl, long, nData, void )
{
    m_xPrgs->set_percentage(nData);
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickPreviewHdl, weld::Button&, void)
{
    Calculate( aBmp, aMtf );
    m_aMtfWin.SetGraphic( aMtf );
    m_xBtnPreview->set_sensitive(false);
}

IMPL_LINK_NOARG(SdVectorizeDlg, ClickOKHdl, weld::Button&, void)
{
    if (m_xBtnPreview->get_sensitive())
        Calculate( aBmp, aMtf );

    SaveSettings();
    m_xDialog->response(RET_OK);
}

IMPL_LINK( SdVectorizeDlg, ToggleHdl, weld::ToggleButton&, rCb, void )
{
    if (rCb.get_active())
    {
        m_xFtFillHoles->set_sensitive(true);
        m_xMtFillHoles->set_sensitive(true);
    }
    else
    {
        m_xFtFillHoles->set_sensitive(false);
        m_xMtFillHoles->set_sensitive(false);
    }

    m_xBtnPreview->set_sensitive(true);
}

IMPL_LINK_NOARG(SdVectorizeDlg, ModifyHdl, weld::SpinButton&, void)
{
    m_xBtnPreview->set_sensitive(true);
}

IMPL_LINK_NOARG(SdVectorizeDlg, MetricModifyHdl, weld::MetricSpinButton&, void)
{
    m_xBtnPreview->set_sensitive(true);
}

void SdVectorizeDlg::LoadSettings()
{
    tools::SvRef<SotStorageStream>  xIStm( SD_MOD()->GetOptionStream(
                               SD_OPTION_VECTORIZE ,
                               SD_OPTION_LOAD ) );
    sal_uInt16              nLayers;
    sal_uInt16              nReduce;
    sal_uInt16              nFillHoles;
    bool                bFillHoles;

    if( xIStm.is() )
    {
        SdIOCompat aCompat( *xIStm, StreamMode::READ );
        xIStm->ReadUInt16( nLayers ).ReadUInt16( nReduce ).ReadUInt16( nFillHoles ).ReadCharAsBool( bFillHoles );
    }
    else
    {
        nLayers = 8;
        nReduce = 0;
        nFillHoles = 32;
        bFillHoles = false;
    }

    m_xNmLayers->set_value(nLayers);
    m_xMtReduce->set_value(nReduce, FieldUnit::NONE);
    m_xMtFillHoles->set_value(nFillHoles, FieldUnit::NONE);
    m_xCbFillHoles->set_active(bFillHoles);

    ToggleHdl(*m_xCbFillHoles);
}

void SdVectorizeDlg::SaveSettings() const
{
    tools::SvRef<SotStorageStream> xOStm( SD_MOD()->GetOptionStream(
                              SD_OPTION_VECTORIZE  ,
                              SD_OPTION_STORE ) );

    if( xOStm.is() )
    {
        SdIOCompat aCompat( *xOStm, StreamMode::WRITE, 1 );
        xOStm->WriteUInt16( m_xNmLayers->get_value() ).WriteUInt16(m_xMtReduce->get_value(FieldUnit::NONE));
        xOStm->WriteUInt16( m_xMtFillHoles->get_value(FieldUnit::NONE) ).WriteBool(m_xCbFillHoles->get_active());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
