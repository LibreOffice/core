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

#include <officecfg/Office/Common.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/bitmap/Vectorizer.hxx>
#include <vcl/metaact.hxx>
#include <vcl/BitmapSimpleColorQuantizationFilter.hxx>
#include <vcl/svapp.hxx>

#include <DrawDocShell.hxx>
#include <vectdlg.hxx>

#define VECTORIZE_MAX_EXTENT 512

SdVectorizeDlg::SdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell)
    : GenericDialogController(pParent, u"modules/sdraw/ui/vectorize.ui"_ustr, u"VectorizeDialog"_ustr)
    , m_pDocSh(pDocShell)
    , aBmp(rBmp)
    , m_aBmpWin(m_xDialog.get())
    , m_aMtfWin(m_xDialog.get())
    , m_xNmLayers(m_xBuilder->weld_spin_button(u"colors"_ustr))
    , m_xMtReduce(m_xBuilder->weld_metric_spin_button(u"points"_ustr, FieldUnit::PIXEL))
    , m_xFtFillHoles(m_xBuilder->weld_label(u"tilesft"_ustr))
    , m_xMtFillHoles(m_xBuilder->weld_metric_spin_button(u"tiles"_ustr, FieldUnit::PIXEL))
    , m_xCbFillHoles(m_xBuilder->weld_check_button(u"fillholes"_ustr))
    , m_xBmpWin(new weld::CustomWeld(*m_xBuilder, u"source"_ustr, m_aBmpWin))
    , m_xMtfWin(new weld::CustomWeld(*m_xBuilder, u"vectorized"_ustr, m_aMtfWin))
    , m_xPrgs(m_xBuilder->weld_progress_bar(u"progressbar"_ustr))
    , m_xBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnPreview(m_xBuilder->weld_button(u"preview"_ustr))
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
            aBmpSize.setWidth( static_cast<tools::Long>( rDispSize.Height() * fGrfWH ) );
            aBmpSize.setHeight( rDispSize.Height() );
        }
        else
        {
            aBmpSize.setWidth( rDispSize.Width() );
            aBmpSize.setHeight( static_cast<tools::Long>( rDispSize.Width() / fGrfWH) );
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
    m_aBmpWin.SetGraphic(BitmapEx(aPreviewBmp));
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

    Fraction aScale;
    BitmapEx aBitmapEx(GetPreparedBitmap(rBmp, aScale));

    if (!aBitmapEx.IsEmpty())
    {
        const Link<::tools::Long,void> aPrgsHdl( LINK( this, SdVectorizeDlg, ProgressHdl ) );
        sal_uInt8 nReduce = sal_uInt8(m_xMtReduce->get_value(FieldUnit::NONE));
        vcl::Vectorizer aVecotrizer(nReduce);
        aVecotrizer.vectorize(aBitmapEx, rMtf);
        aVecotrizer.setProgressCallback(&aPrgsHdl);

        if (m_xCbFillHoles->get_active())
        {
            GDIMetaFile aNewMtf;
            BitmapScopedReadAccess pRAcc(aBitmapEx.GetBitmap());

            if( pRAcc )
            {
                const tools::Long      nWidth = pRAcc->Width();
                const tools::Long      nHeight = pRAcc->Height();
                const tools::Long      nTileX = m_xMtFillHoles->get_value(FieldUnit::NONE);
                const tools::Long      nTileY = m_xMtFillHoles->get_value(FieldUnit::NONE);
                assert(nTileX && "div-by-zero");
                const tools::Long      nCountX = nWidth / nTileX;
                assert(nTileY && "div-by-zero");
                const tools::Long      nCountY = nHeight / nTileY;
                const tools::Long      nRestX = nWidth % nTileX;
                const tools::Long      nRestY = nHeight % nTileY;

                MapMode aMap( rMtf.GetPrefMapMode() );
                aNewMtf.SetPrefSize( rMtf.GetPrefSize() );
                aNewMtf.SetPrefMapMode( aMap );

                for( tools::Long nTY = 0; nTY < nCountY; nTY++ )
                {
                    const tools::Long nY = nTY * nTileY;

                    for( tools::Long nTX = 0; nTX < nCountX; nTX++ )
                        AddTile( pRAcc.get(), aNewMtf, nTX * nTileX, nTY * nTileY, nTileX, nTileY );

                    if( nRestX )
                        AddTile( pRAcc.get(), aNewMtf, nCountX * nTileX, nY, nRestX, nTileY );
                }

                if( nRestY )
                {
                    const tools::Long nY = nCountY * nTileY;

                    for( tools::Long nTX = 0; nTX < nCountX; nTX++ )
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
                              tools::Long nPosX, tools::Long nPosY, tools::Long nWidth, tools::Long nHeight )
{
    sal_uLong           nSumR = 0, nSumG = 0, nSumB = 0;
    const tools::Long      nRight = nPosX + nWidth - 1;
    const tools::Long      nBottom = nPosY + nHeight - 1;
    const double    fMult = 1.0 / ( nWidth * nHeight );

    for( tools::Long nY = nPosY; nY <= nBottom; nY++ )
    {
        for( tools::Long nX = nPosX; nX <= nRight; nX++ )
        {
            const BitmapColor aPixel( pRAcc->GetColor( nY, nX ) );

            nSumR += aPixel.GetRed();
            nSumG += aPixel.GetGreen();
            nSumB += aPixel.GetBlue();
        }
    }

    const Color aColor( basegfx::fround<sal_uInt8>( nSumR * fMult ),
                        basegfx::fround<sal_uInt8>( nSumG * fMult ),
                        basegfx::fround<sal_uInt8>( nSumB * fMult ) );

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

IMPL_LINK( SdVectorizeDlg, ProgressHdl, tools::Long, nData, void )
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

IMPL_LINK( SdVectorizeDlg, ToggleHdl, weld::Toggleable&, rCb, void )
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
    m_xNmLayers->set_value(officecfg::Office::Common::Vectorize::ColorCount::get());
    m_xMtReduce->set_value(officecfg::Office::Common::Vectorize::PointReduce::get(), FieldUnit::NONE);
    m_xCbFillHoles->set_active(officecfg::Office::Common::Vectorize::FillHole::get());
    m_xMtFillHoles->set_value(officecfg::Office::Common::Vectorize::TileExtent::get(), FieldUnit::NONE);

    ToggleHdl(*m_xCbFillHoles);
}

void SdVectorizeDlg::SaveSettings() const
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Vectorize::ColorCount::set(m_xNmLayers->get_value(),batch);
    officecfg::Office::Common::Vectorize::PointReduce::set(m_xMtReduce->get_value(FieldUnit::NONE),batch);
    officecfg::Office::Common::Vectorize::FillHole::set(m_xCbFillHoles->get_active(),batch);
    officecfg::Office::Common::Vectorize::TileExtent::set(m_xMtFillHoles->get_value(FieldUnit::NONE),batch);
    batch->commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
