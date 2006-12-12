/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vectdlg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:11:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#include "strings.hrc"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include "DrawDocShell.hxx"
#include "vectdlg.hxx"
#include "vectdlg.hrc"

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_OCTREE_HXX //autogen
 #include <vcl/octree.hxx>
#endif
#ifndef _SV_BMPACC_HXX //autogen
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_METAACT_HXX //autogen
#include <vcl/metaact.hxx>
#endif

// -----------
// - Defines -
// -----------

#define VECTORIZE_MAX_EXTENT 512

// ------------------
// - SdVectorizeDlg -
// ------------------

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

// -----------------------------------------------------------------------------

SdVectorizeDlg::~SdVectorizeDlg()
{
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void SdVectorizeDlg::InitPreviewBmp()
{
    const Rectangle aRect( GetRect( aBmpWin.GetSizePixel(), aBmp.GetSizePixel() ) );

    aPreviewBmp = aBmp;
    aPreviewBmp.Scale( aRect.GetSize() );
    aBmpWin.SetGraphic( aPreviewBmp );
}

// -----------------------------------------------------------------------------

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

    aNew.ReduceColors( (USHORT) aNmLayers.GetValue(), BMP_REDUCE_SIMPLE );

    return aNew;
}

// -----------------------------------------------------------------------------

void SdVectorizeDlg::Calculate( Bitmap& rBmp, GDIMetaFile& rMtf )
{
    mpDocSh->SetWaitCursor( TRUE );
    aPrgs.SetValue( 0 );

    Fraction    aScale;
    Bitmap      aTmp( GetPreparedBitmap( rBmp, aScale ) );

    if( !!aTmp )
    {
        const Link aPrgsHdl( LINK( this, SdVectorizeDlg, ProgressHdl ) );
        aTmp.Vectorize( rMtf, (BYTE) aMtReduce.GetValue(), BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES, &aPrgsHdl );

        if( aCbFillHoles.IsChecked() )
        {
            GDIMetaFile         aNewMtf;
            BitmapReadAccess*   pRAcc = aTmp.AcquireReadAccess();

            if( pRAcc )
            {
                const long      nWidth = pRAcc->Width();
                const long      nHeight = pRAcc->Height();
                const long      nTileX = aMtFillHoles.GetValue();
                const long      nTileY = aMtFillHoles.GetValue();
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

                for( ULONG n = 0UL, nCount = rMtf.GetActionCount(); n < nCount; n++ )
                    aNewMtf.AddAction( rMtf.GetAction( n )->Clone() );

                aMap.SetScaleX( aMap.GetScaleX() * aScale );
                aMap.SetScaleY( aMap.GetScaleY() * aScale );
                aNewMtf.SetPrefMapMode( aMap );
                rMtf = aNewMtf;
            }
        }
    }

    aPrgs.SetValue( 0 );
    mpDocSh->SetWaitCursor( FALSE );
}

// -----------------------------------------------------------------------------

void SdVectorizeDlg::AddTile( BitmapReadAccess* pRAcc, GDIMetaFile& rMtf,
                              long nPosX, long nPosY, long nWidth, long nHeight )
{
    ULONG           nSumR = 0UL, nSumG = 0UL, nSumB = 0UL;
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

    const Color aColor( (BYTE) FRound( nSumR * fMult ),
                        (BYTE) FRound( nSumG * fMult ),
                        (BYTE) FRound( nSumB * fMult ) );

    Rectangle   aRect( Point( nPosX, nPosY ), Size( nWidth + 1, nHeight + 1 ) );
    const Size& rMaxSize = rMtf.GetPrefSize();

    aRect = PixelToLogic( aRect, rMtf.GetPrefMapMode() );

    if( aRect.Right() > ( rMaxSize.Width() - 1L ) )
        aRect.Right() = rMaxSize.Width() - 1L;

    if( aRect.Bottom() > ( rMaxSize.Height() - 1L ) )
        aRect.Bottom() = rMaxSize.Height() - 1L;

    rMtf.AddAction( new MetaLineColorAction( aColor, TRUE ) );
    rMtf.AddAction( new MetaFillColorAction( aColor, TRUE ) );
    rMtf.AddAction( new MetaRectAction( aRect ) );
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdVectorizeDlg, ProgressHdl, void*, pData )
{
    aPrgs.SetValue( (USHORT)(ULONG) pData );
    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdVectorizeDlg, ClickPreviewHdl, PushButton*, EMPTYARG )
{
    Calculate( aBmp, aMtf );
    aMtfWin.SetGraphic( aMtf );
    aBtnPreview.Disable();

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdVectorizeDlg, ClickOKHdl, OKButton*, EMPTYARG )
{
    if( aBtnPreview.IsEnabled() )
        Calculate( aBmp, aMtf );

    SaveSettings();
    EndDialog( RET_OK );

    return 0L;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

IMPL_LINK( SdVectorizeDlg, ModifyHdl, void*, EMPTYARG )
{
    aBtnPreview.Enable();
    return 0L;
}

// -----------------------------------------------------------------------------

void SdVectorizeDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream(
                               UniString::CreateFromAscii(
                               RTL_CONSTASCII_STRINGPARAM( SD_OPTION_VECTORIZE ) ),
                               SD_OPTION_LOAD ) );
    UINT16              nLayers;
    UINT16              nReduce;
    UINT16              nFillHoles;
    BOOL                bFillHoles;

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
        bFillHoles = FALSE;
    }

    aNmLayers.SetValue( nLayers );
    aMtReduce.SetValue( nReduce );
    aMtFillHoles.SetValue( nFillHoles );
    aCbFillHoles.Check( bFillHoles );

    ToggleHdl( &aCbFillHoles );
}

// -----------------------------------------------------------------------------

void SdVectorizeDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream(
                              UniString::CreateFromAscii(
                              RTL_CONSTASCII_STRINGPARAM( SD_OPTION_VECTORIZE ) ),
                              SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );
        *xOStm << (UINT16) aNmLayers.GetValue() << (UINT16) aMtReduce.GetValue();
        *xOStm << (UINT16) aMtFillHoles.GetValue() << aCbFillHoles.IsChecked();
    }
}
