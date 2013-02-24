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


#include <svx/tbxcolorupdate.hxx>
#include <svx/svxids.hrc>

#include <vcl/toolbox.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/debug.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= ToolboxButtonColorUpdater
    //====================================================================
    /* Note:
       The initial color shown on the button is set in /core/svx/source/tbxctrls/tbxcolorupdate.cxx
       (ToolboxButtonColorUpdater::ToolboxButtonColorUpdater()) .
       The initial color used by the button is set in /core/svx/source/tbxcntrls/tbcontrl.cxx
       (SvxColorExtToolBoxControl::SvxColorExtToolBoxControl())
       and in case of writer for text(background)color also in /core/sw/source/ui/docvw/edtwin.cxx
       (SwEditWin::aTextBackColor and SwEditWin::aTextBackColor)
     */

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
        switch( mnSlotId )
        {
            case SID_ATTR_CHAR_COLOR  :
            case SID_ATTR_CHAR_COLOR2 :
                Update( COL_RED );
                break;
            case SID_FRAME_LINECOLOR  :
                Update( COL_BLUE );
                break;
            case SID_ATTR_CHAR_COLOR_BACKGROUND :
            case SID_BACKGROUND_COLOR :
                Update( COL_YELLOW );
                break;
            default :
                Update( COL_TRANSPARENT );
        }
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

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
