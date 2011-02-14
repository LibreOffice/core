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
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"

#include "ViewShellHint.hxx"

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <sfx2/dispatch.hxx>
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#include <vcl/msgbox.hxx>
#include <svx/svddef.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include "app.hrc"
#include "strings.hrc"

#include "misc.hxx"
#include "fuzoom.hxx"
#include "fudspord.hxx"
#include "futransf.hxx"
#include "fuline.hxx"
#include "fuarea.hxx"
#include "fuchar.hxx"
#include "fuparagr.hxx"
#include "fubullet.hxx"
#include "futempl.hxx"
#include "fuinsert.hxx"
#include "fuprlout.hxx"
#include "fupage.hxx"
#ifndef SD_FU_OBJECT_ANIMATION_PARAMETERS_HXX
#include "fuoaprms.hxx"
#endif
#include "fucopy.hxx"
#ifndef SD_FU_LINE_END_HXX
#include "fulinend.hxx"
#endif
#include "fusnapln.hxx"
#include "fuolbull.hxx"
#include "fuinsfil.hxx"
#include "fulink.hxx"
#include "futhes.hxx"
#include "futxtatt.hxx"
#include "fumeasur.hxx"
#include "fuconnct.hxx"
#include "fumorph.hxx"
#include "fuvect.hxx"
#include "sdresid.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "zoomlist.hxx"
#include <vos/mutex.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <vcl/svapp.hxx>

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemp01(SfxRequest& rReq)
{
    switch(rReq.GetSlot())
    {
        case SID_ATTRIBUTES_LINE:  // BASIC
        {
            SetCurrentFunction( FuLine::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            SetCurrentFunction( FuArea::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            SetCurrentFunction( FuTransform::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Invalidate(SID_RULER_OBJECT);
            Cancel();
        }
        break;

        case SID_CHAR_DLG:  // BASIC
        {
            SetCurrentFunction( FuChar::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            SetCurrentFunction( FuParagraph::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        {
            SetCurrentFunction( FuOutlineBullet::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction( FuBullet::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            SetCurrentFunction( FuPresentationLayout::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PASTE_SPECIAL:
        {
            SetCurrentFunction( FuInsertClipboard::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_GRAPHIC:
        {
            SetCurrentFunction( FuInsertGraphic::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_INSERT_AVMEDIA:
        {
            SetCurrentFunction( FuInsertAVMedia::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_MATH:
        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        {
            SetCurrentFunction( FuInsertOLE::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( mpDrawView->IsPresObjSelected(sal_False, sal_True) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                SetCurrentFunction( FuCopy::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
            SetCurrentFunction( FuInsertFile::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
            Cancel();
            rReq.Done ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_SELECT_BACKGROUND:
        case SID_PAGESETUP:  // BASIC ??
        {
            SetCurrentFunction( FuPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore (); // es werden eigenstaendige macros generiert !!
        }
        break;

        case SID_ZOOM_OUT:
        case SID_ZOOM_PANNING:
        {
            mbZoomOnPage = sal_False;
            SetCurrentFunction( FuZoom::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            // Beendet sich selbst, kein Cancel() notwendig!
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Ignore ();
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            SetCurrentFunction( FuDisplayOrder::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Invalidate( SID_POSITION );
            rReq.Ignore ();
            // Beendet sich selbst, kein Cancel() notwendig!
        }
        break;

        case SID_REVERSE_ORDER:   // BASIC
        {
            mpDrawView->ReverseOrderOfMarked();
            Invalidate( SID_POSITION );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            SetCurrentFunction( FuObjectAnimationParameters::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            SetCurrentFunction( FuLineEnd::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative Werte um Aufruf aus Menue zu signalisieren
            maMousePos = Point(-1,-1);
        case SID_SET_SNAPITEM:
        {
            SetCurrentFunction( FuSnapLine::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            SetCurrentFunction( FuLink::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            SetCurrentFunction( FuTextAttrDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            SetCurrentFunction( FuMeasureDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            SetCurrentFunction( FuConnectionDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSet aDefAttr( GetPool(), SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA );
            GetView()->SetAttributes( aDefAttr, sal_True ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_SELECT:
        {
            sal_Bool bDone = sal_False;

            if( mxScannerManager.is() )
            {
                try
                {
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::scanner::ScannerContext >
                        aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        ::com::sun::star::scanner::ScannerContext aContext( aContexts.getConstArray()[ 0 ] );
                        bDone = mxScannerManager->configureScanner( aContext );
                    }
                }
                catch(...)
                {
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            sal_Bool bDone = sal_False;

            if( mxScannerManager.is() )
            {
                try
                {
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::scanner::ScannerContext > aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        mxScannerManager->startScan( aContexts.getConstArray()[ 0 ], mxScannerListener );
                        bDone = sal_True;
                    }
                }
                catch( ... )
                {
                }
            }

            if( !bDone )
            {
#ifndef UNX
                const sal_uInt16 nId = STR_TWAIN_NO_SOURCE;
#else
                const sal_uInt16 nId = STR_TWAIN_NO_SOURCE_UNX;
#endif

                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String( SdResId( nId ) ) ).Execute();
            }
            else
            {
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_TWAIN_SELECT );
                rBindings.Invalidate( SID_TWAIN_TRANSFER );
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_POLYGON_MORPHING:
        {
            SetCurrentFunction( FuMorph::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_VECTORIZE:
        {
            SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        default:
        {
            // switch Anweisung wegen CLOOKS aufgeteilt. Alle case-Anweisungen die
            // eine Fu???? -Funktion aufrufen, sind in die Methode FuTemp01 (drviews8),
            // FuTemp02 (drviewsb) gewandert.
            FuTemp02(rReq);
        }
        break;
    }
}

/*************************************************************************
|*
|* Scanner-Event
|*
\************************************************************************/

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
                    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
                    sal_Bool        bInsertNewObject = sal_True;

                    if( GetView()->AreObjectsMarked() )
                    {
                        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

                        if( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrMark*    pMark = rMarkList.GetMark(0);
                            SdrObject*  pObj = pMark->GetMarkedSdrObj();

                            if( pObj->ISA( SdrGrafObj ) )
                            {
                                pGrafObj = static_cast< SdrGrafObj* >( pObj );

                                if( pGrafObj->IsEmptyPresObj() )
                                {
                                    bInsertNewObject = sal_False;
                                    pGrafObj->SetEmptyPresObj(sal_False);
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
                        GetView()->InsertObjectAtView( pGrafObj, *pPV, SDRINSERT_SETDEFLAYER );
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
