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

#include "DrawViewShell.hxx"

#include "ViewShellHint.hxx"

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <vcl/msgbox.hxx>
#include <svx/svddef.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include "app.hrc"
#include "strings.hrc"

#include "sdresid.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "zoomlist.hxx"
#include <osl/mutex.hxx>
#include <tools/helpers.hxx>
#include <vcl/svapp.hxx>

namespace sd {

void DrawViewShell::ScannerEvent( const ::com::sun::star::lang::EventObject& )
{
    if( mxScannerManager.is() )
    {
        const ::com::sun::star::scanner::ScannerContext aContext( mxScannerManager->getAvailableScanners().getConstArray()[ 0 ] );
        const ::com::sun::star::scanner::ScanError      eError = mxScannerManager->getError( aContext );

        if( ::com::sun::star::scanner::ScanError_ScanErrorNone == eError )
        {
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBitmap( mxScannerManager->getBitmap( aContext ) );

            if( xBitmap.is() )
            {
                const BitmapEx aScanBmp( VCLUnoHelper::GetBitmap( xBitmap ) );

                if( !!aScanBmp )
                {
                    const SolarMutexGuard aGuard;
                    SdrPage*            pPage = mpDrawView->GetSdrPageView()->GetPage();
                    Size                aBmpSize( aScanBmp.GetPrefSize() ), aPageSize( pPage->GetSize() );
                    const MapMode       aMap100( MAP_100TH_MM );

                    if( !aBmpSize.Width() || !aBmpSize.Height() )
                        aBmpSize = aScanBmp.GetSizePixel();

                    if( aScanBmp.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                        aBmpSize = GetActiveWindow()->PixelToLogic( aBmpSize, aMap100 );
                    else
                        aBmpSize = OutputDevice::LogicToLogic( aBmpSize, aScanBmp.GetPrefMapMode(), aMap100 );

                    aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
                    aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

                    if( ( ( aBmpSize.Height() > aPageSize.Height() ) || ( aBmpSize.Width() > aPageSize.Width() ) ) && aBmpSize.Height() && aPageSize.Height() )
                    {
                        double fGrfWH = (double) aBmpSize.Width() / aBmpSize.Height();
                        double fWinWH = (double) aPageSize.Width() / aPageSize.Height();

                        if( fGrfWH < fWinWH )
                        {
                            aBmpSize.Width() = FRound( aPageSize.Height() * fGrfWH );
                            aBmpSize.Height()= aPageSize.Height();
                        }
                        else if( fGrfWH > 0.F )
                        {
                            aBmpSize.Width() = aPageSize.Width();
                            aBmpSize.Height()= FRound( aPageSize.Width() / fGrfWH );
                        }
                    }

                    Point aPnt ( ( aPageSize.Width() - aBmpSize.Width() ) >> 1, ( aPageSize.Height() - aBmpSize.Height() ) >> 1 );
                    aPnt += Point( pPage->GetLftBorder(), pPage->GetUppBorder() );
                    Rectangle   aRect( aPnt, aBmpSize );
                    SdrGrafObj* pGrafObj = NULL;
                    bool        bInsertNewObject = true;

                    if( GetView()->AreObjectsMarked() )
                    {
                        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

                        if( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrMark*    pMark = rMarkList.GetMark(0);
                            SdrObject*  pObj = pMark->GetMarkedSdrObj();

                            if( dynamic_cast< SdrGrafObj *>( pObj ) !=  nullptr )
                            {
                                pGrafObj = static_cast< SdrGrafObj* >( pObj );

                                if( pGrafObj->IsEmptyPresObj() )
                                {
                                    bInsertNewObject = false;
                                    pGrafObj->SetEmptyPresObj(false);
                                    pGrafObj->SetOutlinerParaObject(NULL);
                                    pGrafObj->SetGraphic( Graphic( aScanBmp ) );
                                }
                            }
                        }
                    }

                    if( bInsertNewObject )
                    {
                        pGrafObj = new SdrGrafObj( Graphic( aScanBmp ), aRect );
                        SdrPageView* pPV = GetView()->GetSdrPageView();
                        GetView()->InsertObjectAtView( pGrafObj, *pPV, SdrInsertFlags::SETDEFLAYER );
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
