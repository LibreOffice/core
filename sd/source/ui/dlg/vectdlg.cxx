/*************************************************************************
 *
 *  $RCSfile: vectdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 16:49:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#include "strings.hrc"
#include "sdresid.hxx"
#include "docshell.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include "docshell.hxx"
#include "vectdlg.hxx"
#include "vectdlg.hrc"

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
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

SdVectorizeDlg::SdVectorizeDlg( Window* pParent, const Bitmap& rBmp, SdDrawDocShell* pDocShell ) :
        ModalDialog     ( pParent, SdResId( DLG_VECTORIZE ) ),
        pDocSh          ( pDocShell ),
        aGrpSettings    ( this, SdResId( GRP_SETTINGS ) ),
        aFtLayers       ( this, SdResId( FT_LAYERS ) ),
        aNmLayers       ( this, SdResId( NM_LAYERS ) ),
        aFtReduce       ( this, SdResId( FT_REDUCE ) ),
        aMtReduce       ( this, SdResId( MT_REDUCE ) ),
        aFtFillHoles    ( this, SdResId( FT_FILLHOLES ) ),
        aMtFillHoles    ( this, SdResId( MT_FILLHOLES ) ),
        aCbFillHoles    ( this, SdResId( CB_FILLHOLES ) ),
        aFtOriginal     ( this, SdResId( FT_ORIGINAL ) ),
        aFtVectorized   ( this, SdResId( FT_VECTORIZED ) ),
        aBmpWin         ( this, SdResId( CTL_BMP ) ),
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
    pDocSh->SetWaitCursor( TRUE );
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
    pDocSh->SetWaitCursor( FALSE );
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

IMPL_LINK( SdVectorizeDlg, ClickPreviewHdl, PushButton*, pBtn )
{
    Calculate( aBmp, aMtf );
    aMtfWin.SetGraphic( aMtf );
    aBtnPreview.Disable();

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdVectorizeDlg, ClickOKHdl, OKButton*, pBtn )
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

IMPL_LINK( SdVectorizeDlg, ModifyHdl, void*, p )
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

