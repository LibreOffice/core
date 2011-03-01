/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/tbxcolorupdate.hxx>
#include <svx/svxids.hrc>

#include <vcl/toolbox.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/debug.hxx>

#define IMAGE_COL_TRANSPARENT       COL_LIGHTMAGENTA

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= ToolboxButtonColorUpdater
    //====================================================================

    ToolboxButtonColorUpdater::ToolboxButtonColorUpdater(
        USHORT nId,
        USHORT nTbxBtnId,
        ToolBox* ptrTbx,
        USHORT nMode ) :
        mnDrawMode        ( nMode ),
        mnBtnId           ( nTbxBtnId ),
        mnSlotId           ( nId ),
        mpTbx             ( ptrTbx ),
        maCurColor        ( COL_TRANSPARENT )
    {
        if (mnSlotId == SID_BACKGROUND_COLOR)
            mnDrawMode = TBX_UPDATER_MODE_CHAR_COLOR_NEW;
        DBG_ASSERT( ptrTbx, "ToolBox not found :-(" );
        mbWasHiContrastMode = ptrTbx ? ( ptrTbx->GetSettings().GetStyleSettings().GetHighContrastMode() ) : FALSE;
        Update(mnSlotId == SID_ATTR_CHAR_COLOR2 ? COL_BLACK : COL_GRAY);
    }

    // -----------------------------------------------------------------------

    ToolboxButtonColorUpdater::~ToolboxButtonColorUpdater()
    {
    }

    // -----------------------------------------------------------------------

    void ToolboxButtonColorUpdater::Update( const Color& rColor )
    {
        Image       aImage( mpTbx->GetItemImage( mnBtnId ) );
        const bool  bSizeChanged = ( maBmpSize != aImage.GetSizePixel() );
        const bool  bDisplayModeChanged = ( mbWasHiContrastMode != mpTbx->GetSettings().GetStyleSettings().GetHighContrastMode() );
        Color       aColor( rColor );

        // !!! #109290# Workaround for SetFillColor with COL_AUTO
        if( aColor.GetColor() == COL_AUTO )
            aColor = Color( COL_TRANSPARENT );

        if( ( maCurColor != aColor ) || bSizeChanged || bDisplayModeChanged )
        {
            BitmapEx            aBmpEx( aImage.GetBitmapEx() );
            Bitmap              aBmp( aBmpEx.GetBitmap() );
            BitmapWriteAccess*  pBmpAcc = aBmp.AcquireWriteAccess();

            maBmpSize = aBmp.GetSizePixel();

            if( pBmpAcc )
            {
                Bitmap              aMsk;
                BitmapWriteAccess*  pMskAcc;
                const Point         aNullPnt;

                if( aBmpEx.IsAlpha() )
                    pMskAcc = ( aMsk = aBmpEx.GetAlpha().GetBitmap() ).AcquireWriteAccess();
                else if( aBmpEx.IsTransparent() )
                    pMskAcc = ( aMsk = aBmpEx.GetMask() ).AcquireWriteAccess();
                else
                    pMskAcc = NULL;

                mbWasHiContrastMode = mpTbx->GetSettings().GetStyleSettings().GetHighContrastMode();

                if( mnDrawMode == TBX_UPDATER_MODE_CHAR_COLOR_NEW && ( COL_TRANSPARENT != aColor.GetColor() ) )
                    pBmpAcc->SetLineColor( aColor );
                else if( mpTbx->GetBackground().GetColor().IsDark() )
                    pBmpAcc->SetLineColor( Color( COL_WHITE ) );
                else
                    pBmpAcc->SetLineColor( Color( COL_BLACK ) );

                pBmpAcc->SetFillColor( maCurColor = aColor );

                if( TBX_UPDATER_MODE_CHAR_COLOR_NEW == mnDrawMode || TBX_UPDATER_MODE_NONE == mnDrawMode )
                {
                    if( TBX_UPDATER_MODE_CHAR_COLOR_NEW == mnDrawMode )
                    {
                        if( maBmpSize.Width() <= 16 )
                            maUpdRect = Rectangle( Point( 0,12 ), Size( maBmpSize.Width(), 4 ) );
                        else
                            maUpdRect = Rectangle( Point( 1, maBmpSize.Height() - 7 ), Size( maBmpSize.Width() - 2 ,6 ) );
                    }
                    else
                    {
                        if( maBmpSize.Width() <= 16 )
                            maUpdRect = Rectangle( Point( 7, 7 ), Size( 8, 8 ) );
                        else
                            maUpdRect = Rectangle( Point( maBmpSize.Width() - 12, maBmpSize.Height() - 12 ), Size( 11, 11 ) );
                    }

                    pBmpAcc->DrawRect( maUpdRect );

                    if( pMskAcc )
                    {
                        if( COL_TRANSPARENT == aColor.GetColor() )
                        {
                            pMskAcc->SetLineColor( COL_BLACK );
                            pMskAcc->SetFillColor( COL_WHITE );
                        }
                        else
                            pMskAcc->SetFillColor( COL_BLACK );

                        pMskAcc->DrawRect( maUpdRect );
                    }
                }
                else
                {
                    OSL_FAIL( "ToolboxButtonColorUpdater::Update: TBX_UPDATER_MODE_CHAR_COLOR / TBX_UPDATER_MODE_CHAR_BACKGROUND" );
                    // !!! DrawChar( aVirDev, aColor );
                }

                aBmp.ReleaseAccess( pBmpAcc );

                if( pMskAcc )
                    aMsk.ReleaseAccess( pMskAcc );

                if( aBmpEx.IsAlpha() )
                    aBmpEx = BitmapEx( aBmp, AlphaMask( aMsk ) );
                else if( aBmpEx.IsTransparent() )
                    aBmpEx = BitmapEx( aBmp, aMsk );
                else
                    aBmpEx = aBmp;

                mpTbx->SetItemImage( mnBtnId, Image( aBmpEx ) );
            }
        }
    }

    // -----------------------------------------------------------------------

    void ToolboxButtonColorUpdater::DrawChar( VirtualDevice& rVirDev, const Color& rCol )
    {
        Font aOldFont = rVirDev.GetFont();
        Font aFont = aOldFont;
        Size aSz = aFont.GetSize();
        aSz.Height() = maBmpSize.Height();
        aFont.SetSize( aSz );
        aFont.SetWeight( WEIGHT_BOLD );

        if ( mnDrawMode == TBX_UPDATER_MODE_CHAR_COLOR )
        {
            aFont.SetColor( rCol );
            aFont.SetFillColor( Color( IMAGE_COL_TRANSPARENT ) );
        }
        else
        {
            rVirDev.SetLineColor();
            rVirDev.SetFillColor( rCol );
            Rectangle aRect( Point(0,0), maBmpSize );
            rVirDev.DrawRect( aRect );
            aFont.SetFillColor( rCol );
        }
        rVirDev.SetFont( aFont );
        Size aTxtSize(rVirDev.GetTextWidth( 'A' ), rVirDev.GetTextHeight());
        Point aPos( ( maBmpSize.Width() - aTxtSize.Width() ) / 2,
                    ( maBmpSize.Height() - aTxtSize.Height() ) / 2 );

        rVirDev.DrawText( aPos, 'A' );
        rVirDev.SetFont( aOldFont );
    }

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
