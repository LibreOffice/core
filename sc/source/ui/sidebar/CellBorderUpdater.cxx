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

#include "precompiled_sc.hxx"

#include <CellBorderUpdater.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>

namespace sc { namespace sidebar {

CellBorderUpdater::CellBorderUpdater(
    sal_uInt16 nTbxBtnId,
    ToolBox& rTbx)
:   mnBtnId(nTbxBtnId),
    mrTbx(rTbx)
{
}

CellBorderUpdater::~CellBorderUpdater()
{
}

void CellBorderUpdater::UpdateCellBorder(bool bTop, bool bBot, bool bLeft, bool bRight, Image aImg, bool bVer, bool bHor)
{
    BitmapEx            aBmpEx( aImg.GetBitmapEx() );
    Bitmap              aBmp( aBmpEx.GetBitmap() );
    BitmapWriteAccess*  pBmpAcc = aBmp.AcquireWriteAccess();
    const Size maBmpSize = aBmp.GetSizePixel();

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

        pBmpAcc->SetLineColor( ::Application::GetSettings().GetStyleSettings().GetFieldTextColor() ) ;
        pBmpAcc->SetFillColor( COL_BLACK);

        if(maBmpSize.Width() == 43 && maBmpSize.Height() == 43)
        {
            Point aTL(2, 1), aTR(42,1), aBL(2, 41), aBR(42, 41), aHL(2,21), aHR(42, 21), aVT(22,1), aVB(22, 41);
            if( pMskAcc )
            {
                pMskAcc->SetLineColor( COL_BLACK );
                pMskAcc->SetFillColor( COL_BLACK );
            }
            if(bLeft)
            {
                pBmpAcc->DrawLine( aTL,aBL );
                if( pMskAcc )
                    pMskAcc->DrawLine( aTL,aBL );
            }
            if(bRight)
            {
                pBmpAcc->DrawLine( aTR,aBR );
                if( pMskAcc )
                    pMskAcc->DrawLine( aTR,aBR );
            }
            if(bTop)
            {
                pBmpAcc->DrawLine( aTL,aTR );
                if( pMskAcc )
                    pMskAcc->DrawLine( aTL,aTR );
            }
            if(bBot)
            {
                pBmpAcc->DrawLine( aBL,aBR );
                if( pMskAcc )
                    pMskAcc->DrawLine( aBL,aBR );
            }
            if(bVer)
            {
                pBmpAcc->DrawLine( aVT,aVB );
                if( pMskAcc )
                    pMskAcc->DrawLine( aVT,aVB );
            }
            if(bHor)
            {
                pBmpAcc->DrawLine( aHL,aHR );
                if( pMskAcc )
                    pMskAcc->DrawLine( aHL,aHR );
            }
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

        mrTbx.SetItemImage( mnBtnId, Image( aBmpEx ) );
    }
}

} } // end of namespace svx::sidebar

// eof
