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

#include <DrawViewShell.hxx>

#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include <Window.hxx>
#include <drawview.hxx>
#include <tools/helpers.hxx>
#include <vcl/svapp.hxx>

namespace sd {

void DrawViewShell::ScannerEvent()
{
    if( mxScannerManager.is() )
    {
        const css::scanner::ScannerContext aContext( mxScannerManager->getAvailableScanners().getConstArray()[ 0 ] );
        const css::scanner::ScanError      eError = mxScannerManager->getError( aContext );

        if( css::scanner::ScanError_ScanErrorNone == eError )
        {
            const css::uno::Reference< css::awt::XBitmap > xBitmap( mxScannerManager->getBitmap( aContext ) );

            if( xBitmap.is() )
            {
                const BitmapEx aScanBmp( VCLUnoHelper::GetBitmap( xBitmap ) );

                if( !aScanBmp.IsEmpty() )
                {
                    const SolarMutexGuard aGuard;
                    SdrPage*            pPage = mpDrawView->GetSdrPageView()->GetPage();
                    Size                aBmpSize( aScanBmp.GetPrefSize() ), aPageSize( pPage->GetSize() );
                    const MapMode       aMap100( MapUnit::Map100thMM );

                    if( !aBmpSize.Width() || !aBmpSize.Height() )
                        aBmpSize = aScanBmp.GetSizePixel();

                    if( aScanBmp.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
                        aBmpSize = GetActiveWindow()->PixelToLogic( aBmpSize, aMap100 );
                    else
                        aBmpSize = OutputDevice::LogicToLogic( aBmpSize, aScanBmp.GetPrefMapMode(), aMap100 );

                    aPageSize.AdjustWidth( -(pPage->GetLeftBorder() + pPage->GetRightBorder()) );
                    aPageSize.AdjustHeight( -(pPage->GetUpperBorder() + pPage->GetLowerBorder()) );

                    if( ( ( aBmpSize.Height() > aPageSize.Height() ) || ( aBmpSize.Width() > aPageSize.Width() ) ) && aBmpSize.Height() && aPageSize.Height() )
                    {
                        double fGrfWH = static_cast<double>(aBmpSize.Width()) / aBmpSize.Height();
                        double fWinWH = static_cast<double>(aPageSize.Width()) / aPageSize.Height();

                        if( fGrfWH < fWinWH )
                        {
                            aBmpSize.setWidth( basegfx::fround<::tools::Long>( aPageSize.Height() * fGrfWH ) );
                            aBmpSize.setHeight( aPageSize.Height() );
                        }
                        else if( fGrfWH > 0.F )
                        {
                            aBmpSize.setWidth( aPageSize.Width() );
                            aBmpSize.setHeight( basegfx::fround<::tools::Long>( aPageSize.Width() / fGrfWH ) );
                        }
                    }

                    Point aPnt ( ( aPageSize.Width() - aBmpSize.Width() ) >> 1, ( aPageSize.Height() - aBmpSize.Height() ) >> 1 );
                    aPnt += Point( pPage->GetLeftBorder(), pPage->GetUpperBorder() );
                    ::tools::Rectangle   aRect( aPnt, aBmpSize );
                    bool        bInsertNewObject = true;

                    if( GetView()->GetMarkedObjectList().GetMarkCount() != 0 )
                    {
                        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

                        if( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrMark*    pMark = rMarkList.GetMark(0);
                            SdrObject*  pObj = pMark->GetMarkedSdrObj();

                            if( auto pGrafObj = dynamic_cast< SdrGrafObj *>( pObj ) )
                            {
                                if( pGrafObj->IsEmptyPresObj() )
                                {
                                    bInsertNewObject = false;
                                    pGrafObj->SetEmptyPresObj(false);
                                    pGrafObj->SetOutlinerParaObject(std::nullopt);
                                    pGrafObj->SetGraphic( Graphic( aScanBmp ) );
                                }
                            }
                        }
                    }

                    if( bInsertNewObject )
                    {
                        rtl::Reference<SdrGrafObj> pGrafObj = new SdrGrafObj(
                            GetView()->getSdrModelFromSdrView(),
                            Graphic(aScanBmp),
                            aRect);
                        SdrPageView* pPV = GetView()->GetSdrPageView();
                        GetView()->InsertObjectAtView( pGrafObj.get(), *pPV, SdrInsertFlags::SETDEFLAYER );
                    }
                }
            }
        }
    }

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_TWAIN_SELECT );
    rBindings.Invalidate( SID_TWAIN_TRANSFER );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
