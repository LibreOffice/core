/*************************************************************************
 *
 *  $RCSfile: drviews8.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:19 $
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
#pragma hdrstop

#include "app.hrc"
#include "strings.hrc"

#include "misc.hxx"
#include "fuzoom.hxx"
#include "fudspord.hxx"
#include "fuslprms.hxx"
#include "futransf.hxx"
#include "fuline.hxx"
#include "fuarea.hxx"
#include "fuchar.hxx"
#include "fuparagr.hxx"
#include "fubullet.hxx"
#include "futempl.hxx"
#include "fuinsert.hxx"
#include "fuprlout.hxx"
#include "fuprtmpl.hxx"
#include "fupage.hxx"
#include "fuoaprms.hxx"
#include "sdresid.hxx"
#include "fucopy.hxx"
#include "fulinend.hxx"
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
#include "sdwindow.hxx"
#include "drviewsh.hxx"
#include "zoomlist.hxx"
#include "drawview.hxx"



/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuTemp01(SfxRequest& rReq)
{
    switch(rReq.GetSlot())
    {
        case SID_ATTRIBUTES_LINE:  // BASIC
        {
            pFuActual = new FuLine( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            pFuActual = new FuArea( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            pFuActual = new FuTransform( this, pWindow, pDrView, pDoc, rReq );
//          Invalidate(SID_RULER_OBJECT);
            Cancel();
        }
        break;

        case SID_CHAR_DLG:  // BASIC
        {
            pFuActual = new FuChar( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            pFuActual = new FuParagraph( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        {
            pFuActual = new FuOutlineBullet( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_BULLET:
        {
            pFuActual = new FuBullet( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            pFuActual = new FuPresentationLayout(this, pWindow, pDrView, pDoc, rReq);
            Cancel();
        }
        break;

//        case SID_MODEL:
//        {
//            pFuActual = new FuTemplate( this, pWindow, pDrView, pDoc, rReq );
//            Cancel();
//        }
//        break;

        case SID_PASTE2:
        {
            pFuActual = new FuInsertClipboard( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_GRAPHIC:
        {
            pFuActual = new FuInsertGraphic( this, pWindow, pDrView, pDoc, rReq );

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
            pFuActual = new FuInsertOLE( this, pWindow, pDrView, pDoc, rReq );

            Cancel();
            rReq.Ignore ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( pDrView->IsPresObjSelected(FALSE, TRUE) )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }

                pFuActual = new FuCopy( this, pWindow, pDrView, pDoc, rReq );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            pFuActual = new FuInsertFile( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Done ();

            Invalidate(SID_DRAWTBX_INSERT);
        }
        break;

        case SID_PAGESETUP:  // BASIC ??
        {
            pFuActual = new FuPage( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore (); // es werden eigenstaendige macros generiert !!
        }
        break;

        case SID_ZOOM_OUT:
        case SID_ZOOM_PANNING:
        {
            bZoomOnPage = FALSE;
            pFuActual = new FuZoom(this, pWindow, pDrView, pDoc, rReq);
            // Beendet sich selbst, kein Cancel() notwendig!
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Ignore ();
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            pFuActual = new FuDisplayOrder(this, pWindow, pDrView, pDoc, rReq);
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
            pFuActual =new FuSlideParameters( this, pWindow, pDrView, pDoc, rReq);
            Cancel();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            pFuActual =new FuObjectAnimationParameters( this, pWindow, pDrView, pDoc, rReq);
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            pFuActual = new FuLineEnd( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative Werte um Aufruf aus Menue zu signalisieren
            aMousePos = Point(-1,-1);
        case SID_SET_SNAPITEM:
        {
            pFuActual = new FuSnapLine(this, pWindow, pDrView, pDoc, rReq);
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            pFuActual = new FuLink( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            pFuActual = new FuThesaurus( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            pFuActual = new FuTextAttrDlg( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            pFuActual = new FuMeasureDlg( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            pFuActual = new FuConnectionDlg( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSet aDefAttr( GetPool(), SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA );
            pView->SetAttributes( aDefAttr, TRUE ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

#if defined WIN || defined WNT || defined UNX

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

            if( !bDone )
            {
#ifndef UNX
                const USHORT nId = STR_TWAIN_NO_SOURCE;
#else
                const USHORT nId = STR_TWAIN_NO_SOURCE_UNX;
#endif
                InfoBox( pWindow, String( SdResId( nId ) ) ).Execute();
            }
        }
        break;

        case SID_TWAIN_TRANSFER:
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

                InfoBox( pWindow, String( SdResId( nId ) ) ).Execute();
            }
            else
            {
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_TWAIN_SELECT );
                rBindings.Invalidate( SID_TWAIN_TRANSFER );
            }
        }
        break;

#endif

        case SID_POLYGON_MORPHING:
        {
            pFuActual = new FuMorph( this, pWindow, pDrView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_VECTORIZE:
        {
            pFuActual = new FuVectorize( this, pWindow, pDrView, pDoc, rReq );
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

void SdDrawViewShell::ScannerEvent( const ::com::sun::star::lang::EventObject& rEventObject )
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
                    SfxRequest aSfxRequest( SID_TWAIN_TRANSFER, SFX_CALLMODE_SLOT, GetPool() );
                    pFuActual = new FuInsertTwain( this, pWindow, pDrView, pDoc, aSfxRequest, aScanBmp.GetBitmap() );
                    Cancel();
                }
            }
        }
    }

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_TWAIN_SELECT );
    rBindings.Invalidate( SID_TWAIN_TRANSFER );
}


