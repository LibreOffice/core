/*************************************************************************
 *
 *  $RCSfile: sdgrffilter.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:57:06 $
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

#include <unotools/localfilehelper.hxx>
#include <tools/errinf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/impgrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xoutbmp.hxx>

#ifndef MAC
#ifndef SVX_LIGHT
#include "../../ui/inc/strings.hrc"
#include "../../ui/inc/graphpro.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "../../ui/inc/DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../../ui/inc/DrawDocShell.hxx"
#endif
#ifndef SD_CLIENT_VIEW_HXX
#include "../../ui/inc/ClientView.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "../../ui/inc/FrameView.hxx"
#endif
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#include "strings.hrc"
#include "graphpro.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_CLIENT_VIEW_HXX
#include "ClientView.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#endif //!SVX_LIGHT
#endif //!MAC


#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdgrffilter.hxx"


#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif


// ---------------
// - SdPPTFilter -
// ---------------

SdGRFFilter::SdGRFFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdGRFFilter::~SdGRFFilter()
{
}

// -----------------------------------------------------------------------------

GDIMetaFile SdGRFFilter::ImplRemoveClipRegionActions( const GDIMetaFile& rMtf )
{
    GDIMetaFile     aMtf;
    const ULONG     nActionCount = rMtf.GetActionCount();

    aMtf.SetPrefSize( rMtf.GetPrefSize() );
    aMtf.SetPrefMapMode( rMtf.GetPrefMapMode() );

    // Actions untersuchen und ClipRegion-Action herausnehmen
    for ( ULONG nAction = 0; nAction < nActionCount; nAction++ )
    {
        MetaAction* pCopyAction = ( (GDIMetaFile&) rMtf ).CopyAction( nAction );

        if( pCopyAction )
        {
            switch( pCopyAction->GetType() )
            {
                case( META_CLIPREGION_ACTION ) :
                    delete pCopyAction;
                break;

                default:
                    aMtf.AddAction( pCopyAction );
                break;
            }
        }
    }

    return aMtf;
}

// -----------------------------------------------------------------------------

BitmapEx SdGRFFilter::ImplGetBitmapFromMetaFile( const GDIMetaFile& rMtf, BOOL bTransparent, const Size* pSizePixel )
{
    Graphic     aGraphic( rMtf );
    BitmapEx    aBmpEx;

    if( bTransparent )
    {
        Graphic aMaskGraphic( rMtf.GetMonochromeMtf( COL_BLACK ) );
        Bitmap  aMaskBmp( aMaskGraphic.GetBitmap( pSizePixel) );

        aMaskBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
        aBmpEx = BitmapEx( aGraphic.GetBitmap( pSizePixel ), aMaskBmp );
    }
    else
        aBmpEx = BitmapEx( aGraphic.GetBitmap( pSizePixel ) );

    aBmpEx.SetPrefMapMode( rMtf.GetPrefMapMode() );
    aBmpEx.SetPrefSize( rMtf.GetPrefSize() );

    return aBmpEx;
}

// -----------------------------------------------------------------------------

void SdGRFFilter::HandleGraphicFilterError( USHORT nFilterError, ULONG nStreamError )
{
    USHORT nId;

    switch( nFilterError )
    {
        case GRFILTER_OPENERROR:
            nId = STR_IMPORT_GRFILTER_OPENERROR;
            break;
        case GRFILTER_IOERROR:
            nId = STR_IMPORT_GRFILTER_IOERROR;
            break;
        case GRFILTER_FORMATERROR:
            nId = STR_IMPORT_GRFILTER_FORMATERROR;
            break;
        case GRFILTER_VERSIONERROR:
            nId = STR_IMPORT_GRFILTER_VERSIONERROR;
            break;
        case GRFILTER_FILTERERROR:
            nId = STR_IMPORT_GRFILTER_FILTERERROR;
            break;
        case GRFILTER_TOOBIG:
            nId = STR_IMPORT_GRFILTER_TOOBIG;
            break;
        default:
            nId = STR_IMPORT_GRFILTER_FILTERERROR;
            break;
    }

    if( ERRCODE_NONE != nStreamError )
        ErrorHandler::HandleError( nStreamError );
    else if( STR_IMPORT_GRFILTER_IOERROR == nId )
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    else
    {
        ErrorBox aErrorBox( NULL, WB_OK, String( SdResId( nId ) ) );
        aErrorBox.Execute();
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Import()
{
    Graphic         aGraphic;
    const String    aFileName( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    GraphicFilter*  pGraphicFilter = GetGrfFilter();
    const USHORT    nFilter = pGraphicFilter->GetImportFormatNumberForTypeName( mrMedium.GetFilter()->GetTypeName() );
    sal_Bool        bRet = sal_False;

    // ggf. Filterdialog ausfuehren
    if ( !pGraphicFilter->HasImportDialog( nFilter ) || pGraphicFilter->DoImportDialog( NULL, nFilter ) )
    {
        SvStream*       pIStm = mrMedium.GetInStream();
        FilterProgress* pFilterProgress = mbShowProgress ? new FilterProgress( pGraphicFilter, &(SfxObjectShell&) mrDocShell ) : NULL;
        USHORT          nReturn = pIStm ? pGraphicFilter->ImportGraphic( aGraphic, aFileName, *pIStm, nFilter ) : 1;

        if( nReturn )
            HandleGraphicFilterError( nReturn, pGraphicFilter->GetLastError().nStreamError );
        else
        {
            if( mrDocument.GetPageCount() == 0L )
                mrDocument.CreateFirstPages();

            SdPage*     pPage = mrDocument.GetSdPage( 0, PK_STANDARD );
            Point       aPos;
            Size        aPagSize( pPage->GetSize() );
            Size        aGrfSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                  aGraphic.GetPrefMapMode(), MAP_100TH_MM ) );

            aPagSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
            aPagSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

            // scale to fit page
            if ( ( aGrfSize.Height() > aPagSize.Height() ) || (aGrfSize.Width() > aPagSize.Width() ) &&
                 aGrfSize.Height() && aPagSize.Height() )
            {
                double fGrfWH = (double) aGrfSize.Width() / aGrfSize.Height();
                double fWinWH = (double) aPagSize.Width() / aPagSize.Height();

                // Grafik an Pagesize anpassen (skaliert)
                if( fGrfWH < fWinWH )
                {
                    aGrfSize.Width() = (long) ( aPagSize.Height() * fGrfWH );
                    aGrfSize.Height() = aPagSize.Height();
                }
                else if( fGrfWH > 0.F )
                {
                    aGrfSize.Width() = aPagSize.Width();
                    aGrfSize.Height()= (long) ( aPagSize.Width() / fGrfWH );
                }
            }

            // Ausgaberechteck fuer Grafik setzen
            aPos.X() = ( ( aPagSize.Width() - aGrfSize.Width() ) >> 1 ) + pPage->GetLftBorder();
            aPos.Y() = ( ( aPagSize.Height() - aGrfSize.Height() ) >> 1 )  + pPage->GetUppBorder();

            pPage->InsertObject( new SdrGrafObj( aGraphic, Rectangle( aPos, aGrfSize ) ) );
            bRet = sal_True;
        }

        delete pFilterProgress;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Export()
{
    const String        aTypeName( mrMedium.GetFilter()->GetTypeName() );
    SfxItemSet*         pSet = mrMedium.GetItemSet();
    VirtualDevice       aVDev;
    Graphic             aGraphic;
    sd::DrawViewShell*  pDrawViewShell = static_cast< ::sd::DrawViewShell* >( ( ( mrDocShell.GetViewShell() && mrDocShell.GetViewShell()->ISA(::sd::DrawViewShell ) ) ? mrDocShell.GetViewShell() : NULL ) );
    const Fraction      aFrac( mrDocument.GetScaleFraction() );
    const MapMode       aMap( mrDocument.GetScaleUnit(), Point(), aFrac, aFrac );
    GraphicFilter*      pGraphicFilter = GetGrfFilter();
    SdPage*             pPage = NULL;
    const USHORT        nFilter = pGraphicFilter->GetExportFormatNumberForTypeName( aTypeName );
    USHORT              nPage;
    PageKind            ePageKind = PK_STANDARD;
    BOOL                bVectorType = !pGraphicFilter->IsExportPixelFormat( nFilter );
    BOOL                bTranslucent = pGraphicFilter->GetExportFormatShortName( nFilter ).ToLowerAscii().EqualsAscii( "gif" );
    BOOL                bSelection = FALSE;
    sal_Bool            bRet = sal_False;

    if( pSet && ( SFX_ITEM_SET == pSet->GetItemState( SID_SELECTION ) ) && static_cast< const SfxBoolItem& >( pSet->Get( SID_SELECTION ) ).GetValue() )
        bSelection = TRUE;

    if( pDrawViewShell )
    {
        ePageKind = pDrawViewShell->GetPageKind();

        if( PK_HANDOUT == ePageKind )
            pPage = mrDocument.GetSdPage( 0, PK_HANDOUT );
        else
            pPage = pDrawViewShell->GetActualPage();
    }
    else
        pPage = mrDocument.GetSdPage( 0, PK_STANDARD );

    if( pPage )
    {
        FilterProgress* pFilterProgress = mbShowProgress ? new FilterProgress( pGraphicFilter, &(SfxObjectShell&) mrDocShell ) : NULL;

        if( pFilterProgress )
            pFilterProgress->SetState( 10 );

        // translucent speichern?
        if( bTranslucent )
        {
            FilterConfigItem aConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/GIF" ) ) );
            bTranslucent = aConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Translucent" ) ), 1 ) != 0;
        }
        else
            bTranslucent = FALSE;

        // 'richtige' Zeichenseitennummer ermitteln
        if( pPage->GetPageNum() )
            nPage = ( pPage->GetPageNum() - 1 ) >> 1;
        else
            nPage = 0;

        // 'richtige' Seite besorgen
        pPage = mrDocument.GetSdPage( nPage, ePageKind );

        if( !bSelection || !pDrawViewShell )
        {
            // export the whole page
            const Size  aSize( pPage->GetSize() );
            const Point aNewOrg( pPage->GetLftBorder(), pPage->GetUppBorder() );
            const Size  aNewSize( aSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder(),
                                  aSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder() );

            if ( !bVectorType && !bTranslucent )
            {
                const Size      aSizePix( aVDev.LogicToPixel( aNewSize, aMap ) );
                const long      nWidthPix = ( aSizePix.Width() > 2048 || aSizePix.Height() > 2048 ) ? 2048 : 0;
                ::sd::View* pView = new ::sd::View( &mrDocument, &aVDev );
                VirtualDevice*  pVDev = pView->CreatePageVDev( nPage, ePageKind, nWidthPix );

                if( pVDev )
                {
                    aGraphic = pVDev->GetBitmap( Point(), pVDev->GetOutputSize() );
                    aGraphic.SetPrefMapMode( aMap );
                    aGraphic.SetPrefSize( aNewSize );
                    delete pVDev;
                }

                delete pView;
            }
            else
            {
                GDIMetaFile aMtf;

                aVDev.SetMapMode( aMap );
                aVDev.EnableOutput( FALSE );
                aMtf.Record( &aVDev );

                ::sd::ClientView* pView = new ::sd::ClientView( &mrDocShell, &aVDev, NULL );

                pView->SetBordVisible( FALSE );
                pView->SetPageVisible( FALSE );
                pView->ShowPage( pPage, Point() );

                const Rectangle aClipRect( aNewOrg, aNewSize );
                MapMode         aVMap( aMap );

                SdrPageView* pPageView  = pView->GetPageView( pPage );
                ::sd::FrameView* pFrameView = mrDocShell.GetFrameView();

                pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
                pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

                aVDev.Push();
                aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                aVDev.SetRelativeMapMode( aVMap );
                aVDev.IntersectClipRegion( aClipRect );
                /* #103186# because of:

                  > SdrPageView:
                  > // rReg bezieht sich auf's OutDev, nicht auf die Page
                  > void CompleteRedraw( ... );

                  and setting the origin to -aNewOrg we have to use aNewOrg
                  instead of (0,0) for the Clip-Region to CompleteRedraw
                */
                pView->CompleteRedraw( &aVDev, Region( Rectangle( aNewOrg, aNewSize ) ) );
                aVDev.Pop();

                aMtf.Stop();
                aMtf.WindStart();
                aMtf.SetPrefMapMode( aMap );
                aMtf.SetPrefSize( aNewSize );

                aGraphic = Graphic( ImplRemoveClipRegionActions( aMtf ) );

                if( bTranslucent )
                    aGraphic = ImplGetBitmapFromMetaFile( aGraphic.GetGDIMetaFile(), TRUE );

                delete pView;
            }
        }
        else
        {
            // export selected objects
            ::sd::View* pView = pDrawViewShell->GetView();

            if( pView )
            {
                if( !bVectorType )
                {
                    const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();
                    BOOL                bGraf = FALSE;

                    if( rMarkList.GetMarkCount() == 1 )
                    {
                        SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                        if( pObj && pObj->ISA( SdrGrafObj ) && !( (SdrGrafObj*) pObj )->HasText() )
                        {
                            aGraphic = ( (SdrGrafObj*) pObj )->GetTransformedGraphic();
                            bGraf = TRUE;
                        }
                    }

                    if( !bGraf )
                        aGraphic = ImplGetBitmapFromMetaFile( pView->GetAllMarkedMetaFile(), bTranslucent );
                }
                else
                    aGraphic = pView->GetAllMarkedMetaFile();
            }
        }

        const Size aGraphSize( aGraphic.GetPrefSize() );

        if ( ( aGraphSize.Width() > 1 ) && ( aGraphSize.Height() > 1 ) )
        {
            String aTmp;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( mrMedium.GetPhysicalName(), aTmp );
            const INetURLObject aURL( aTmp );

            mrMedium.Close();

            const SfxPoolItem *pItem=0;
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;
            if ( pSet->GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                ((SfxUnoAnyItem*)pItem)->GetValue() >>= aFilterData;
            }
            const USHORT nRet = pGraphicFilter->ExportGraphic( aGraphic, aURL, nFilter, sal_False, &aFilterData );

            if( nRet )
                SdGRFFilter::HandleGraphicFilterError( nRet, pGraphicFilter->GetLastError().nStreamError );
            else
                bRet = sal_True;
        }
        else
        {
            ErrorBox aErrBox( NULL, WB_OK, String( SdResId( STR_EXPORT_EMPTYGRAPHIC ) ) );
            aErrBox.Execute();
        }

        delete pFilterProgress;
    }

    return bRet;
}
