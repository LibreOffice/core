/*************************************************************************
 *
 *  $RCSfile: drviews8.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:18:46 $
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

#include "DrawViewShell.hxx"

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"

#include "misc.hxx"
#include "fuzoom.hxx"
#ifndef SD_FU_DISPLAY_ORDER_HXX
#include "fudspord.hxx"
#endif
#ifndef SD_FU_SLIDE_PARAMETERS_HXX
#include "fuslprms.hxx"
#endif
#ifndef SD_FU_TRANSFORM_HXX
#include "futransf.hxx"
#endif
#ifndef SD_FU_LINE_HXX
#include "fuline.hxx"
#endif
#ifndef SD_FU_AREA_HXX
#include "fuarea.hxx"
#endif
#ifndef SD_FU_CHAR_HXX
#include "fuchar.hxx"
#endif
#ifndef SD_FU_PARAGRAPH_HXX
#include "fuparagr.hxx"
#endif
#ifndef SD_FU_BULLET_HXX
#include "fubullet.hxx"
#endif
#ifndef SD_FU_TEMPLATE_HXX
#include "futempl.hxx"
#endif
#ifndef SD_FU_INSERT_HXX
#include "fuinsert.hxx"
#endif
#ifndef SD_FU_PRESENTATION_LAYOUT_HXX
#include "fuprlout.hxx"
#endif
#ifndef SD_FU_PRESENTATION_TEMPLATES_HXX
#include "fuprtmpl.hxx"
#endif
#ifndef SD_FU_PAGE_HXX
#include "fupage.hxx"
#endif
#ifndef SD_FU_OBJECT_ANIMATION_PARAMETERS_HXX
#include "fuoaprms.hxx"
#endif
#ifndef SD_FU_COPY_HXX
#include "fucopy.hxx"
#endif
#ifndef SD_FU_LINE_END_HXX
#include "fulinend.hxx"
#endif
#ifndef SD_FU_SNAP_LINE_HXX
#include "fusnapln.hxx"
#endif
#ifndef SD_FU_OUTLINE_BULLET_HXX
#include "fuolbull.hxx"
#endif
#ifndef SD_FU_INSERT_FILE_HXX
#include "fuinsfil.hxx"
#endif
#ifndef SD_FU_LINK_HXX
#include "fulink.hxx"
#endif
#ifndef SD_FU_THESAURUS_HXX
#include "futhes.hxx"
#endif
#ifndef SD_FU_TEXT_ATTR_DLG_HXX
#include "futxtatt.hxx"
#endif
#ifndef SD_FU_MEASURE_DLG_HXX
#include "fumeasur.hxx"
#endif
#ifndef SD_FU_CONNECTION_DLG_HXX
#include "fuconnct.hxx"
#endif
#ifndef SD_FU_MORPH_HXX
#include "fumorph.hxx"
#endif
#ifndef SD_FU_VECTORIZE_HXX
#include "fuvect.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "zoomlist.hxx"

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
            pFuActual = new FuLine( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            pFuActual = new FuArea( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            pFuActual = new FuTransform( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Invalidate(SID_RULER_OBJECT);
            Cancel();
        }
        break;

        case SID_CHAR_DLG:  // BASIC
        {
            pFuActual = new FuChar( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            pFuActual = new FuParagraph( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        {
            pFuActual = new FuOutlineBullet( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_BULLET:
        {
            pFuActual = new FuBullet( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            pFuActual = new FuPresentationLayout(this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            Cancel();
        }
        break;

//        case SID_MODEL:
//        {
//            pFuActual = new FuTemplate( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
//            Cancel();
//        }
//        break;

        case SID_PASTE2:
        {
            pFuActual = new FuInsertClipboard( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_GRAPHIC:
        {
            pFuActual = new FuInsertGraphic( this, GetActiveWindow(), pDrView, GetDoc(), rReq );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_INSERT_AVMEDIA:
        {
            pFuActual = new FuInsertAVMedia( this, GetActiveWindow(), pDrView, GetDoc(), rReq );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_APPLET:
        case SID_INSERT_FLOATINGFRAME:
#ifdef STARIMAGE_AVAILABLE
        case SID_INSERT_IMAGE:
#endif
        case SID_INSERT_MATH:
        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        {
            pFuActual = new FuInsertOLE( this, GetActiveWindow(), pDrView, GetDoc(), rReq );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( pDrView->IsPresObjSelected(FALSE, TRUE) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }

                pFuActual = new FuCopy( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            pFuActual = new FuInsertFile( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Done ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_PAGESETUP:  // BASIC ??
        {
            pFuActual = new FuPage( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore (); // es werden eigenstaendige macros generiert !!
        }
        break;

        case SID_ZOOM_OUT:
        case SID_ZOOM_PANNING:
        {
            bZoomOnPage = FALSE;
            pFuActual = new FuZoom(this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            // Beendet sich selbst, kein Cancel() notwendig!
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Ignore ();
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            pFuActual = new FuDisplayOrder(this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            Invalidate( SID_POSITION );
            rReq.Ignore ();
            // Beendet sich selbst, kein Cancel() notwendig!
        }
        break;

        case SID_REVERSE_ORDER:   // BASIC
        {
            pDrView->ReverseOrderOfMarked();
            Invalidate( SID_POSITION );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DIA:
        {
            pFuActual =new FuSlideParameters( this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            Cancel();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            pFuActual =new FuObjectAnimationParameters( this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            pFuActual = new FuLineEnd( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative Werte um Aufruf aus Menue zu signalisieren
            aMousePos = Point(-1,-1);
        case SID_SET_SNAPITEM:
        {
            pFuActual = new FuSnapLine(this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            pFuActual = new FuLink( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            pFuActual = new FuThesaurus( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            pFuActual = new FuTextAttrDlg( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            pFuActual = new FuMeasureDlg( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            pFuActual = new FuConnectionDlg( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSet aDefAttr( GetPool(), SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA );
            GetView()->SetAttributes( aDefAttr, TRUE ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_SELECT:
        {
            BOOL bDone = FALSE;

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
            BOOL bDone = FALSE;

            if( mxScannerManager.is() )
            {
                try
                {
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::scanner::ScannerContext > aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        mxScannerManager->startScan( aContexts.getConstArray()[ 0 ], mxScannerListener );
                        bDone = TRUE;
                    }
                }
                catch( ... )
                {
                }
            }

            if( !bDone )
            {
#ifndef UNX
                const USHORT nId = STR_TWAIN_NO_SOURCE;
#else
                const USHORT nId = STR_TWAIN_NO_SOURCE_UNX;
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
            pFuActual = new FuMorph( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_VECTORIZE:
        {
            pFuActual = new FuVectorize( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
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

void DrawViewShell::ScannerEvent( const ::com::sun::star::lang::EventObject& rEventObject )
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
                    SdrPage*            pPage = pDrView->GetPageViewPvNum( 0 )->GetPage();
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

                    if( ( aBmpSize.Height() > aPageSize.Height() ) || ( aBmpSize.Width()    > aPageSize.Width() ) && aBmpSize.Height() && aPageSize.Height() )
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
                    BOOL        bInsertNewObject = TRUE;

                    if( GetView()->AreObjectsMarked() )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

                        if( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrMark*    pMark = rMarkList.GetMark(0);
                            SdrObject*  pObj = pMark->GetObj();

                            if( pObj->ISA( SdrGrafObj ) )
                            {
                                pGrafObj = static_cast< SdrGrafObj* >( pObj );

                                if( pGrafObj->IsEmptyPresObj() )
                                {
                                    bInsertNewObject = FALSE;
                                    pGrafObj->SetEmptyPresObj(FALSE);
                                    pGrafObj->SetOutlinerParaObject(NULL);
                                    pGrafObj->SetGraphic( Graphic( aScanBmp ) );
                                }
                            }
                        }
                    }

                    if( bInsertNewObject )
                    {
                        pGrafObj = new SdrGrafObj( Graphic( aScanBmp ), aRect );
                        SdrPageView* pPV = GetView()->GetPageViewPvNum(0);
                        GetView()->InsertObject( pGrafObj, *pPV, SDRINSERT_SETDEFLAYER );
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
