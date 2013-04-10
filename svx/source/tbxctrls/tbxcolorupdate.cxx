/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        sal_uInt16 nId,
        sal_uInt16 nTbxBtnId,
        ToolBox* ptrTbx,
        sal_uInt16 nMode ) :
        mnDrawMode        ( nMode ),
        mnBtnId           ( nTbxBtnId ),
        mnSlotId           ( nId ),
        mpTbx             ( ptrTbx ),
        maCurColor        ( COL_TRANSPARENT )
    {
        if (mnSlotId == SID_BACKGROUND_COLOR)
            mnDrawMode = TBX_UPDATER_MODE_CHAR_COLOR_NEW;
        DBG_ASSERT( ptrTbx, "ToolBox not found :-(" );
        mbWasHiContrastMode = ptrTbx ? ( ptrTbx->GetSettings().GetStyleSettings().GetHighContrastMode() ) : sal_False;
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
            BitmapWriteAccess*  pBmpAcc = aBmp.IsEmpty() ? NULL : aBmp.AcquireWriteAccess();

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
                        else if(76 == maBmpSize.Width() && 12 == maBmpSize.Height())
                        {
                            maUpdRect.Left() = 22;
                            maUpdRect.Top() = 2;
                            maUpdRect.Right() = 73;
                            maUpdRect.Bottom() = 9;
                        }
                        else if(30 == maBmpSize.Width() && 16 == maBmpSize.Height())
                        {
                            maUpdRect.Left() = 17;
                            maUpdRect.Top() = 2;
                            maUpdRect.Right() = 27;
                            maUpdRect.Bottom() = 13;
                        }
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
                    DBG_ERROR( "ToolboxButtonColorUpdater::Update: TBX_UPDATER_MODE_CHAR_COLOR / TBX_UPDATER_MODE_CHAR_BACKGROUND" );
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
