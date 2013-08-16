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

#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <sot/storage.hxx>
#include <svl/itempool.hxx>
#include <editeng/editobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <sot/formats.hxx>
#include <svl/urlbmk.hxx>
#include <editeng/outliner.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/streamwrap.hxx>

#include <svx/svdotable.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <svtools/embedtransfer.hxx>
#include "DrawDocShell.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "imapinfo.hxx"
#include "sdxfer.hxx"
#include "unomodel.hxx"
#include <vcl/virdev.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

#define SDTRANSFER_OBJECTTYPE_DRAWMODEL         0x00000001
#define SDTRANSFER_OBJECTTYPE_DRAWOLE           0x00000002

SdTransferable::SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, sal_Bool bInitOnGetData )
:   mpPageDocShell( NULL )
,   mpOLEDataHelper( NULL )
,   mpObjDesc( NULL )
,   mpSdView( pWorkView )
,   mpSdViewIntern( pWorkView )
,   mpSdDrawDocument( NULL )
,   mpSdDrawDocumentIntern( NULL )
,   mpSourceDoc( pSrcDoc )
,   mpVDev( NULL )
,   mpBookmark( NULL )
,   mpGraphic( NULL )
,   mpImageMap( NULL )
,   mbInternalMove( sal_False )
,   mbOwnDocument( sal_False )
,   mbOwnView( sal_False )
,   mbLateInit( bInitOnGetData )
,   mbPageTransferable( sal_False )
,   mbPageTransferablePersistent( sal_False )
,   mbIsUnoObj( false )
,   maUserData()
{
    if( mpSourceDoc )
        StartListening( *mpSourceDoc );

    if( pWorkView )
        StartListening( *pWorkView );

    if( !mbLateInit )
        CreateData();
}

SdTransferable::~SdTransferable()
{
    if( mpSourceDoc )
        EndListening( *mpSourceDoc );

    if( mpSdView )
        EndListening( *const_cast< sd::View *>( mpSdView) );

    SolarMutexGuard aSolarGuard;

    ObjectReleased();

    if( mbOwnView )
        delete mpSdViewIntern;

    delete mpOLEDataHelper;

    if( maDocShellRef.Is() )
    {
        SfxObjectShell* pObj = maDocShellRef;
        ::sd::DrawDocShell* pDocSh = static_cast< ::sd::DrawDocShell*>(pObj);
        pDocSh->DoClose();
    }

    maDocShellRef.Clear();

    if( mbOwnDocument )
        delete mpSdDrawDocumentIntern;

    delete mpGraphic;
    delete mpBookmark;
    delete mpImageMap;

    delete mpVDev;
    delete mpObjDesc;

}

void SdTransferable::CreateObjectReplacement( SdrObject* pObj )
{
    if( pObj )
    {
        delete mpOLEDataHelper, mpOLEDataHelper = NULL;
        delete mpGraphic, mpGraphic = NULL;
        delete mpBookmark, mpBookmark = NULL;
        delete mpImageMap, mpImageMap = NULL;

        if( pObj->ISA( SdrOle2Obj ) )
        {
            try
            {
                uno::Reference < embed::XEmbeddedObject > xObj = static_cast< SdrOle2Obj* >( pObj )->GetObjRef();
                uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if( xObj.is() && xPersist.is() && xPersist->hasEntry() )
                {
                    mpOLEDataHelper = new TransferableDataHelper( new SvEmbedTransferHelper( xObj, static_cast< SdrOle2Obj* >( pObj )->GetGraphic(), static_cast< SdrOle2Obj* >( pObj )->GetAspect() ) );

                    // TODO/LATER: the standalone handling of the graphic should not be used any more in future
                    // The EmbedDataHelper should bring the graphic in future
                    const Graphic* pObjGr = static_cast< SdrOle2Obj* >( pObj )->GetGraphic();
                    if ( pObjGr )
                        mpGraphic = new Graphic( *pObjGr );
                }
            }
            catch( uno::Exception& )
            {}
        }
        else if( pObj->ISA( SdrGrafObj ) && (mpSourceDoc && !mpSourceDoc->GetAnimationInfo( pObj )) )
        {
            mpGraphic = new Graphic( static_cast< SdrGrafObj* >( pObj )->GetTransformedGraphic() );
        }
        else if( pObj->IsUnoObj() && FmFormInventor == pObj->GetObjInventor() && ( pObj->GetObjIdentifier() == (sal_uInt16) OBJ_FM_BUTTON ) )
        {
            SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pObj );

            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                Reference< ::com::sun::star::awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

                if( !xControlModel.is() )
                    return;

                Reference< ::com::sun::star::beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY );

                if( !xPropSet.is() )
                    return;

                ::com::sun::star::form::FormButtonType  eButtonType;
                Any                                     aTmp( xPropSet->getPropertyValue( "ButtonType" ) );

                if( aTmp >>= eButtonType )
                {
                    OUString aLabel, aURL;

                    xPropSet->getPropertyValue( "Label" ) >>= aLabel;
                    xPropSet->getPropertyValue( "TargetURL" ) >>= aURL;

                    mpBookmark = new INetBookmark( aURL, aLabel );
                }
            }
        }
        else if( pObj->ISA( SdrTextObj ) )
        {
            const OutlinerParaObject* pPara;

            if( (pPara = static_cast< SdrTextObj* >( pObj )->GetOutlinerParaObject()) != 0 )
            {
                const SvxFieldItem* pField;

                if( (pField = pPara->GetTextObject().GetField()) != 0 )
                {
                    const SvxFieldData* pData = pField->GetField();

                    if( pData && pData->ISA( SvxURLField ) )
                    {
                        const SvxURLField* pURL = (SvxURLField*) pData;

                        mpBookmark = new INetBookmark( pURL->GetURL(), pURL->GetRepresentation() );
                    }
                }
            }
        }

        SdIMapInfo* pInfo = static_cast< SdDrawDocument* >( pObj->GetModel() )->GetIMapInfo( static_cast< SdrObject* >( pObj ) );

        if( pInfo )
            mpImageMap = new ImageMap( pInfo->GetImageMap() );

        mbIsUnoObj = pObj && pObj->IsUnoObj();
    }
}

void SdTransferable::CreateData()
{
    if( mpSdDrawDocument && !mpSdViewIntern )
    {
        mbOwnView = sal_True;

        SdPage* pPage = mpSdDrawDocument->GetSdPage(0, PK_STANDARD);

        if( 1 == pPage->GetObjCount() )
            CreateObjectReplacement( pPage->GetObj( 0 ) );

        mpVDev = new VirtualDevice( *Application::GetDefaultDevice() );
        mpVDev->SetMapMode( MapMode( mpSdDrawDocumentIntern->GetScaleUnit(), Point(), mpSdDrawDocumentIntern->GetScaleFraction(), mpSdDrawDocumentIntern->GetScaleFraction() ) );
        mpSdViewIntern = new ::sd::View( *mpSdDrawDocumentIntern, mpVDev );
        mpSdViewIntern->EndListening(*mpSdDrawDocumentIntern );
        mpSdViewIntern->hideMarkHandles();
        SdrPageView* pPageView = mpSdViewIntern->ShowSdrPage(pPage);
        ((SdrMarkView*)mpSdViewIntern)->MarkAllObj(pPageView);
    }
    else if( mpSdView && !mpSdDrawDocumentIntern )
    {
        const SdrMarkList& rMarkList = mpSdView->GetMarkedObjectList();

        if( rMarkList.GetMarkCount() == 1 )
            CreateObjectReplacement( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(this);
        mpSdDrawDocumentIntern = (SdDrawDocument*) mpSdView->GetMarkedObjModel();
        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(0);

        if( !maDocShellRef.Is() && mpSdDrawDocumentIntern->GetDocSh() )
            maDocShellRef = mpSdDrawDocumentIntern->GetDocSh();

        if( !maDocShellRef.Is() )
        {
            OSL_FAIL( "SdTransferable::CreateData(), failed to create a model with persist, clipboard operation will fail for OLE objects!" );
            mbOwnDocument = sal_True;
        }

        // Use dimension of source page
        SdrPageView*        pPgView = mpSdView->GetSdrPageView();
        SdPage*             pOldPage = (SdPage*) pPgView->GetPage();
        SdrModel*           pOldModel = mpSdView->GetModel();
        SdStyleSheetPool*   pOldStylePool = (SdStyleSheetPool*) pOldModel->GetStyleSheetPool();
        SdStyleSheetPool*   pNewStylePool = (SdStyleSheetPool*) mpSdDrawDocumentIntern->GetStyleSheetPool();
        SdPage*             pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );
        OUString            aOldLayoutName( pOldPage->GetLayoutName() );

        pPage->SetSize( pOldPage->GetSize() );
        pPage->SetLayoutName( aOldLayoutName );
        pNewStylePool->CopyGraphicSheets( *pOldStylePool );
        pNewStylePool->CopyCellSheets( *pOldStylePool );
        pNewStylePool->CopyTableStyles( *pOldStylePool );
        sal_Int32 nPos = aOldLayoutName.indexOf( SD_LT_SEPARATOR );
        if( nPos != -1 )
            aOldLayoutName = aOldLayoutName.copy( 0, nPos );
        SdStyleSheetVector aCreatedSheets;
        pNewStylePool->CopyLayoutSheets( aOldLayoutName, *pOldStylePool, aCreatedSheets );
    }

    // set VisArea and adjust objects if necessary
    if( maVisArea.IsEmpty() &&
        mpSdDrawDocumentIntern && mpSdViewIntern &&
        mpSdDrawDocumentIntern->GetPageCount() )
    {
        SdPage* pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );

        if( 1 == mpSdDrawDocumentIntern->GetPageCount() )
        {
            // #112978# need to use GetAllMarkedBoundRect instead of GetAllMarkedRect to get
            // fat lines correctly
            Point   aOrigin( ( maVisArea = mpSdViewIntern->GetAllMarkedBoundRect() ).TopLeft() );
            Size    aVector( -aOrigin.X(), -aOrigin.Y() );

            for( sal_uLong nObj = 0, nObjCount = pPage->GetObjCount(); nObj < nObjCount; nObj++ )
            {
                SdrObject* pObj = pPage->GetObj( nObj );
                pObj->NbcMove( aVector );
            }
        }
        else
            maVisArea.SetSize( pPage->GetSize() );

        // output is at the zero point
        maVisArea.SetPos( Point() );
    }
}

static sal_Bool lcl_HasOnlyControls( SdrModel* pModel )
{
    sal_Bool bOnlyControls = sal_False;         // default if there are no objects

    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObj = aIter.Next();
            if ( pObj )
            {
                bOnlyControls = sal_True;   // only set if there are any objects at all
                while ( pObj )
                {
                    if (!pObj->ISA(SdrUnoObj))
                    {
                        bOnlyControls = sal_False;
                        break;
                    }
                    pObj = aIter.Next();
                }
            }
        }
    }

    return bOnlyControls;
}

static bool lcl_HasOnlyOneTable( SdrModel* pModel )
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
            if( dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) ) != 0 )
                return true;
        }
    }
    return false;
}

void SdTransferable::AddSupportedFormats()
{
    if( !mbPageTransferable || mbPageTransferablePersistent )
    {
        if( !mbLateInit )
            CreateData();

        if( mpObjDesc )
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        if( mpOLEDataHelper )
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

            DataFlavorExVector              aVector( mpOLEDataHelper->GetDataFlavorExVector() );
            DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

            while( aIter != aEnd )
                AddFormat( *aIter++ );
        }
        else if( mpGraphic )
        {
            // #i25616#
            AddFormat( SOT_FORMATSTR_ID_DRAWING );

            AddFormat( SOT_FORMATSTR_ID_SVXB );

            if( mpGraphic->GetType() == GRAPHIC_BITMAP )
            {
                AddFormat( SOT_FORMAT_BITMAP );
                AddFormat( SOT_FORMAT_GDIMETAFILE );
            }
            else
            {
                AddFormat( SOT_FORMAT_GDIMETAFILE );
                AddFormat( SOT_FORMAT_BITMAP );
            }
        }
        else if( mpBookmark )
        {
            AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
            AddFormat( FORMAT_STRING );
        }
        else
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            if( !mpSdDrawDocument || !lcl_HasOnlyControls( mpSdDrawDocument ) )
            {
                AddFormat( SOT_FORMAT_GDIMETAFILE );
                AddFormat( SOT_FORMAT_BITMAP );
            }

            if( lcl_HasOnlyOneTable( mpSdDrawDocument ) )
                AddFormat( SOT_FORMAT_RTF );
        }

        if( mpImageMap )
            AddFormat( SOT_FORMATSTR_ID_SVIM );
    }
}

sal_Bool SdTransferable::GetData( const DataFlavor& rFlavor )
{
    if (SD_MOD()==NULL)
        return sal_False;

    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bOK = sal_False;

    CreateData();

    if( nFormat == SOT_FORMAT_RTF && lcl_HasOnlyOneTable( mpSdDrawDocument ) )
    {
        bOK = SetTableRTF( mpSdDrawDocument, rFlavor );
    }
    else if( mpOLEDataHelper && mpOLEDataHelper->HasFormat( rFlavor ) )
    {
        sal_uLong nOldSwapMode = 0;

        if( mpSdDrawDocumentIntern )
        {
            nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
        }

        // TODO/LATER: support all the graphical formats, the embedded object scenario should not have separated handling
        if( nFormat == FORMAT_GDIMETAFILE && mpGraphic )
            bOK = SetGDIMetaFile( mpGraphic->GetGDIMetaFile(), rFlavor );
        else
            bOK = SetAny( mpOLEDataHelper->GetAny( rFlavor ), rFlavor );

        if( mpSdDrawDocumentIntern )
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
    }
    else if( HasFormat( nFormat ) )
    {
        if( ( nFormat == SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR || nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) && mpObjDesc )
        {
            bOK = SetTransferableObjectDescriptor( *mpObjDesc, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_DRAWING )
        {
            SfxObjectShellRef aOldRef( maDocShellRef );

            maDocShellRef.Clear();

            if( mpSdViewIntern )
            {
                SdDrawDocument& rInternDoc = mpSdViewIntern->GetDoc();
                rInternDoc.CreatingDataObj(this);
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( mpSdViewIntern->GetMarkedObjModel() );
                rInternDoc.CreatingDataObj(0);

                bOK = SetObject( pDoc, SDTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );

                if( maDocShellRef.Is() )
                {
                    maDocShellRef->DoClose();
                }
                else
                {
                    delete pDoc;
                }
            }

            maDocShellRef = aOldRef;
        }
        else if( nFormat == FORMAT_GDIMETAFILE )
        {
            if( mpSdViewIntern )
                bOK = SetGDIMetaFile( mpSdViewIntern->GetMarkedObjMetaFile( true ), rFlavor );
        }
        else if( FORMAT_BITMAP == nFormat || SOT_FORMATSTR_ID_PNG == nFormat )
        {
            if( mpSdViewIntern )
                bOK = SetBitmapEx( mpSdViewIntern->GetMarkedObjBitmapEx(true), rFlavor );
        }
        else if( ( nFormat == FORMAT_STRING ) && mpBookmark )
        {
            bOK = SetString( mpBookmark->GetURL(), rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVXB ) && mpGraphic )
        {
            bOK = SetGraphic( *mpGraphic, rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVIM ) && mpImageMap )
        {
            bOK = SetImageMap( *mpImageMap, rFlavor );
        }
        else if( mpBookmark )
        {
            bOK = SetINetBookmark( *mpBookmark, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
        {
            sal_uLong nOldSwapMode = 0;

            if( mpSdDrawDocumentIntern )
            {
                nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
                mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
            }

            if( !maDocShellRef.Is() )
            {
                maDocShellRef = new ::sd::DrawDocShell(
                    mpSdDrawDocumentIntern,
                    SFX_CREATE_MODE_EMBEDDED,
                    sal_True,
                    mpSdDrawDocumentIntern->GetDocumentType());
                mbOwnDocument = sal_False;
                maDocShellRef->DoInitNew( NULL );
            }

            maDocShellRef->SetVisArea( maVisArea );
            bOK = SetObject( &maDocShellRef, SDTRANSFER_OBJECTTYPE_DRAWOLE, rFlavor );

            if( mpSdDrawDocumentIntern )
                mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
        }
    }

    return bOK;
}

sal_Bool SdTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pObject, sal_uInt32 nObjectType, const DataFlavor& )
{
    sal_Bool bRet = sal_False;

    switch( nObjectType )
    {
        case( SDTRANSFER_OBJECTTYPE_DRAWMODEL ):
        {
            try
            {
                static const sal_Bool bDontBurnInStyleSheet = ( getenv( "AVOID_BURN_IN_FOR_GALLERY_THEME" ) != NULL );
                SdDrawDocument* pDoc = (SdDrawDocument*) pObject;
                if ( !bDontBurnInStyleSheet )
                    pDoc->BurnInStyleSheetAttributes();
                rxOStm->SetBufferSize( 16348 );

                Reference< XComponent > xComponent( new SdXImpressDocument( pDoc, sal_True ) );
                pDoc->setUnoModel( Reference< XInterface >::query( xComponent ) );

                {
                    com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                    if( SvxDrawingLayerExport( pDoc, xDocOut, xComponent, (pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? "com.sun.star.comp.Impress.XMLClipboardExporter" : "com.sun.star.comp.DrawingLayer.XMLExporter" ) )
                        rxOStm->Commit();
                }

                xComponent->dispose();
                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            catch( Exception& )
            {
                OSL_FAIL( "sd::SdTransferable::WriteObject(), exception catched!" );
                bRet = sal_False;
            }
        }
        break;

        case( SDTRANSFER_OBJECTTYPE_DRAWOLE ):
        {
            SfxObjectShell*   pEmbObj = (SfxObjectShell*) pObject;
            ::utl::TempFile     aTempFile;
            aTempFile.EnableKillingFile();

            try
            {
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, sal_False );
                // mba: no relative ULRs for clipboard!
                SfxMedium aMedium( xWorkStore, OUString() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, sal_False );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    *rxOStm << *pSrcStm;
                    delete pSrcStm;
                }

                bRet = sal_True;
                rxOStm->Commit();
            }
            catch ( Exception& )
            {}
        }

        break;

        default:
        break;
    }

    return bRet;
}

void SdTransferable::DragFinished( sal_Int8 nDropAction )
{
    if( mpSdView )
        ( (::sd::View*) mpSdView )->DragFinished( nDropAction );
}

void SdTransferable::ObjectReleased()
{
    if( this == SD_MOD()->pTransferClip )
        SD_MOD()->pTransferClip = NULL;

    if( this == SD_MOD()->pTransferDrag )
        SD_MOD()->pTransferDrag = NULL;

    if( this == SD_MOD()->pTransferSelection )
        SD_MOD()->pTransferSelection = NULL;
}

void SdTransferable::SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc )
{
    delete mpObjDesc;
    mpObjDesc = new TransferableObjectDescriptor( rObjDesc );
    PrepareOLE( rObjDesc );
}

void SdTransferable::SetPageBookmarks( const std::vector<OUString> &rPageBookmarks, sal_Bool bPersistent )
{
    if( mpSourceDoc )
    {
        if( mpSdViewIntern )
            mpSdViewIntern->HideSdrPage();

        mpSdDrawDocument->ClearModel(sal_False);

        mpPageDocShell = NULL;

        maPageBookmarks.clear();

        if( bPersistent )
        {
            mpSdDrawDocument->CreateFirstPages(mpSourceDoc);
            mpSdDrawDocument->InsertBookmarkAsPage( rPageBookmarks, NULL, sal_False, sal_True, 1, sal_True,
                                                    mpSourceDoc->GetDocSh(), sal_True, sal_True, sal_False );
        }
        else
        {
            mpPageDocShell = mpSourceDoc->GetDocSh();
            maPageBookmarks = rPageBookmarks;
        }

        if( mpSdViewIntern && mpSdDrawDocument )
        {
            SdPage* pPage = mpSdDrawDocument->GetSdPage( 0, PK_STANDARD );

            if( pPage )
            {
                ( (SdrMarkView*) mpSdViewIntern )->MarkAllObj( (SdrPageView*) mpSdViewIntern->ShowSdrPage( pPage ) );
            }
        }

        // set flags for page transferable; if ( mbPageTransferablePersistent == sal_False ),
        // don't offer any formats => it's just for internal puposes
        mbPageTransferable = sal_True;
        mbPageTransferablePersistent = bPersistent;
    }
}

sal_Int64 SAL_CALL SdTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
    {
        nRet = 0;
    }

    return nRet;
}


SdDrawDocument* SdTransferable::GetSourceDoc (void) const
{
    return mpSourceDoc;
}

void SdTransferable::AddUserData (const ::boost::shared_ptr<UserData>& rpData)
{
    maUserData.push_back(rpData);
}

sal_Int32 SdTransferable::GetUserDataCount (void) const
{
    return maUserData.size();
}

::boost::shared_ptr<SdTransferable::UserData> SdTransferable::GetUserData (const sal_Int32 nIndex) const
{
    if (nIndex>=0 && nIndex<sal_Int32(maUserData.size()))
        return maUserData[nIndex];
    else
        return ::boost::shared_ptr<UserData>();
}

namespace
{
    class theSdTransferableUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdTransferableUnoTunnelId > {};
}

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdTransferable::getUnoTunnelId()
{
    return theSdTransferableUnoTunnelId::get().getSeq();
}

SdTransferable* SdTransferable::getImplementation( const Reference< XInterface >& rxData ) throw()
{
    try
    {
        Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( rxData, UNO_QUERY_THROW );
        return reinterpret_cast<SdTransferable*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SdTransferable::getUnoTunnelId()) ) );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
    return NULL;
}

void SdTransferable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );
    if( pSdrHint )
    {
        if( HINT_MODELCLEARED == pSdrHint->GetKind() )
        {
            EndListening(*mpSourceDoc);
            mpSourceDoc = 0;
        }
    }
    else
    {
        const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint * >(&rHint);
        if(pSimpleHint && (pSimpleHint->GetId() == SFX_HINT_DYING) )
        {
            if( &rBC == mpSourceDoc )
                mpSourceDoc = 0;
            if( &rBC == mpSdViewIntern )
                mpSdViewIntern = 0;
            if( &rBC == mpSdView )
                mpSdView = 0;
        }
    }
}

sal_Bool SdTransferable::SetTableRTF( SdDrawDocument* pModel, const DataFlavor& rFlavor)
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
            sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) );
            if( pTableObj )
            {
                SvMemoryStream aMemStm( 65535, 65535 );
                sdr::table::SdrTableObj::ExportAsRTF( aMemStm, *pTableObj );
                return SetAny( Any( Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) ) ), rFlavor );
            }
        }
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
