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

#define SDTRANSFER_OBJECTTYPE_DRAWMODEL         static_cast<SotClipboardFormatId>(0x00000001)
#define SDTRANSFER_OBJECTTYPE_DRAWOLE           static_cast<SotClipboardFormatId>(0x00000002)

SdTransferable::SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, bool bInitOnGetData )
:   mpPageDocShell( nullptr )
,   mpOLEDataHelper( nullptr )
,   mpObjDesc( nullptr )
,   mpSdView( pWorkView )
,   mpSdViewIntern( pWorkView )
,   mpSdDrawDocument( nullptr )
,   mpSdDrawDocumentIntern( nullptr )
,   mpSourceDoc( pSrcDoc )
,   mpVDev( nullptr )
,   mpBookmark( nullptr )
,   mpGraphic( nullptr )
,   mpImageMap( nullptr )
,   mbInternalMove( false )
,   mbOwnDocument( false )
,   mbOwnView( false )
,   mbLateInit( bInitOnGetData )
,   mbPageTransferable( false )
,   mbPageTransferablePersistent( false )
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
    SolarMutexGuard g;

    if( mpSourceDoc )
        EndListening( *mpSourceDoc );

    if( mpSdView )
        EndListening( *const_cast< sd::View *>( mpSdView) );

    ObjectReleased();

    if( mbOwnView )
        delete mpSdViewIntern;

    delete mpOLEDataHelper;

    if( maDocShellRef.is() )
    {
        SfxObjectShell* pObj = maDocShellRef.get();
        ::sd::DrawDocShell* pDocSh = static_cast< ::sd::DrawDocShell*>(pObj);
        pDocSh->DoClose();
    }

    maDocShellRef.clear();

    if( mbOwnDocument )
        delete mpSdDrawDocumentIntern;

    delete mpGraphic;
    delete mpBookmark;
    delete mpImageMap;

    mpVDev.disposeAndClear();
    delete mpObjDesc;

    //call explicitly at end of dtor to be covered by above SolarMutex
    maUserData.clear();
}

void SdTransferable::CreateObjectReplacement( SdrObject* pObj )
{
    if( pObj )
    {
        delete mpOLEDataHelper;
        mpOLEDataHelper = nullptr;
        delete mpGraphic;
        mpGraphic = nullptr;
        delete mpBookmark;
        mpBookmark = nullptr;
        delete mpImageMap;
        mpImageMap = nullptr;

        if( nullptr!= dynamic_cast< const SdrOle2Obj* >( pObj ) )
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
        else if( dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr && (mpSourceDoc && !SdDrawDocument::GetAnimationInfo( pObj )) )
        {
            mpGraphic = new Graphic( static_cast< SdrGrafObj* >( pObj )->GetTransformedGraphic() );
        }
        else if( pObj->IsUnoObj() && SdrInventor::FmForm == pObj->GetObjInventor() && ( pObj->GetObjIdentifier() == (sal_uInt16) OBJ_FM_BUTTON ) )
        {
            SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pObj );

            if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
            {
                Reference< css::awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

                if( !xControlModel.is() )
                    return;

                Reference< css::beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY );

                if( !xPropSet.is() )
                    return;

                css::form::FormButtonType  eButtonType;
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
        else if( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr )
        {
            const OutlinerParaObject* pPara;

            if( (pPara = static_cast< SdrTextObj* >( pObj )->GetOutlinerParaObject()) != nullptr )
            {
                const SvxFieldItem* pField;

                if( (pField = pPara->GetTextObject().GetField()) != nullptr )
                {
                    const SvxFieldData* pData = pField->GetField();

                    if( pData && dynamic_cast< const SvxURLField *>( pData ) !=  nullptr )
                    {
                        const SvxURLField* pURL = static_cast<const SvxURLField*>(pData);

                        // #i63399# This special code identifies TextFrames which have just an URL
                        // as content and directly add this to the clipboard, probably to avoid adding
                        // an unnecessary DrawObject to the target where paste may take place. This is
                        // wanted only for SdrObjects with no fill and no line, else it is necessary to
                        // use the whole SdrObect. Test here for Line/FillStyle and take shortcut only
                        // when both are unused
                        if(!pObj->HasFillStyle() && !pObj->HasLineStyle())
                        {
                            mpBookmark = new INetBookmark( pURL->GetURL(), pURL->GetRepresentation() );
                        }
                    }
                }
            }
        }

        SdIMapInfo* pInfo = SdDrawDocument::GetIMapInfo( pObj );

        if( pInfo )
            mpImageMap = new ImageMap( pInfo->GetImageMap() );

        mbIsUnoObj = pObj && pObj->IsUnoObj();
    }
}

void SdTransferable::CreateData()
{
    if( mpSdDrawDocument && !mpSdViewIntern )
    {
        mbOwnView = true;

        SdPage* pPage = mpSdDrawDocument->GetSdPage(0, PageKind::Standard);

        if( 1 == pPage->GetObjCount() )
            CreateObjectReplacement( pPage->GetObj( 0 ) );

        mpVDev = VclPtr<VirtualDevice>::Create( *Application::GetDefaultDevice() );
        mpVDev->SetMapMode( MapMode( mpSdDrawDocumentIntern->GetScaleUnit(), Point(), mpSdDrawDocumentIntern->GetScaleFraction(), mpSdDrawDocumentIntern->GetScaleFraction() ) );
        mpSdViewIntern = new ::sd::View( *mpSdDrawDocumentIntern, mpVDev );
        mpSdViewIntern->EndListening(*mpSdDrawDocumentIntern );
        mpSdViewIntern->hideMarkHandles();
        SdrPageView* pPageView = mpSdViewIntern->ShowSdrPage(pPage);
        mpSdViewIntern->MarkAllObj(pPageView);
    }
    else if( mpSdView && !mpSdDrawDocumentIntern )
    {
        const SdrMarkList& rMarkList = mpSdView->GetMarkedObjectList();

        if( rMarkList.GetMarkCount() == 1 )
            CreateObjectReplacement( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(this);
        mpSdDrawDocumentIntern = static_cast<SdDrawDocument*>( mpSdView->GetMarkedObjModel() );
        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(nullptr);

        if( !maDocShellRef.is() && mpSdDrawDocumentIntern->GetDocSh() )
            maDocShellRef = mpSdDrawDocumentIntern->GetDocSh();

        if( !maDocShellRef.is() )
        {
            OSL_FAIL( "SdTransferable::CreateData(), failed to create a model with persist, clipboard operation will fail for OLE objects!" );
            mbOwnDocument = true;
        }

        // Use dimension of source page
        SdrPageView*        pPgView = mpSdView->GetSdrPageView();
        SdPage*             pOldPage = static_cast<SdPage*>( pPgView->GetPage() );
        SdrModel*           pOldModel = mpSdView->GetModel();
        SdStyleSheetPool*   pOldStylePool = static_cast<SdStyleSheetPool*>( pOldModel->GetStyleSheetPool() );
        SdStyleSheetPool*   pNewStylePool = static_cast<SdStyleSheetPool*>( mpSdDrawDocumentIntern->GetStyleSheetPool() );
        SdPage*             pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PageKind::Standard );
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
        SdPage* pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PageKind::Standard );

        if( 1 == mpSdDrawDocumentIntern->GetPageCount() )
        {
            // #112978# need to use GetAllMarkedBoundRect instead of GetAllMarkedRect to get
            // fat lines correctly
            Point   aOrigin( ( maVisArea = mpSdViewIntern->GetAllMarkedBoundRect() ).TopLeft() );
            Size    aVector( -aOrigin.X(), -aOrigin.Y() );

            for( size_t nObj = 0, nObjCount = pPage->GetObjCount(); nObj < nObjCount; ++nObj )
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

static bool lcl_HasOnlyControls( SdrModel* pModel )
{
    bool bOnlyControls = false;         // default if there are no objects

    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, SdrIterMode::DeepNoGroups );
            SdrObject* pObj = aIter.Next();
            if ( pObj )
            {
                bOnlyControls = true;   // only set if there are any objects at all
                while ( pObj )
                {
                    if (dynamic_cast< const SdrUnoObj *>( pObj ) ==  nullptr)
                    {
                        bOnlyControls = false;
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
            if( dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) ) != nullptr )
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
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );

        if( mpOLEDataHelper )
        {
            AddFormat( SotClipboardFormatId::EMBED_SOURCE );

            DataFlavorExVector              aVector( mpOLEDataHelper->GetDataFlavorExVector() );
            DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

            while( aIter != aEnd )
                AddFormat( *aIter++ );
        }
        else if( mpGraphic )
        {
            // #i25616#
            AddFormat( SotClipboardFormatId::DRAWING );

            AddFormat( SotClipboardFormatId::SVXB );

            if( mpGraphic->GetType() == GraphicType::Bitmap )
            {
                AddFormat( SotClipboardFormatId::PNG );
                AddFormat( SotClipboardFormatId::BITMAP );
                AddFormat( SotClipboardFormatId::GDIMETAFILE );
            }
            else
            {
                AddFormat( SotClipboardFormatId::GDIMETAFILE );
                AddFormat( SotClipboardFormatId::PNG );
                AddFormat( SotClipboardFormatId::BITMAP );
            }
        }
        else if( mpBookmark )
        {
            AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
            AddFormat( SotClipboardFormatId::STRING );
        }
        else
        {
            AddFormat( SotClipboardFormatId::EMBED_SOURCE );
            AddFormat( SotClipboardFormatId::DRAWING );
            if( !mpSdDrawDocument || !lcl_HasOnlyControls( mpSdDrawDocument ) )
            {
                AddFormat( SotClipboardFormatId::GDIMETAFILE );
                AddFormat( SotClipboardFormatId::PNG );
                AddFormat( SotClipboardFormatId::BITMAP );
            }

            if( lcl_HasOnlyOneTable( mpSdDrawDocument ) ) {
                AddFormat( SotClipboardFormatId::RTF );
                AddFormat( SotClipboardFormatId::RICHTEXT );
            }
        }

        if( mpImageMap )
            AddFormat( SotClipboardFormatId::SVIM );
    }
}

bool SdTransferable::GetData( const DataFlavor& rFlavor, const OUString& rDestDoc )
{
    if (SD_MOD()==nullptr)
        return false;

    SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );
    bool        bOK = false;

    CreateData();

    if( nFormat == SotClipboardFormatId::RTF && lcl_HasOnlyOneTable( mpSdDrawDocument ) )
    {
        bOK = SetTableRTF( mpSdDrawDocument, rFlavor );
    }
    else if( mpOLEDataHelper && mpOLEDataHelper->HasFormat( rFlavor ) )
    {
        SdrSwapGraphicsMode nOldSwapMode(SdrSwapGraphicsMode::DEFAULT);

        if( mpSdDrawDocumentIntern )
        {
            nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( SdrSwapGraphicsMode::PURGE );
        }

        // TODO/LATER: support all the graphical formats, the embedded object scenario should not have separated handling
        if( nFormat == SotClipboardFormatId::GDIMETAFILE && mpGraphic )
            bOK = SetGDIMetaFile( mpGraphic->GetGDIMetaFile(), rFlavor );
        else
            bOK = SetAny( mpOLEDataHelper->GetAny(rFlavor, rDestDoc), rFlavor );

        if( mpSdDrawDocumentIntern )
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
    }
    else if( HasFormat( nFormat ) )
    {
        if( ( nFormat == SotClipboardFormatId::LINKSRCDESCRIPTOR || nFormat == SotClipboardFormatId::OBJECTDESCRIPTOR ) && mpObjDesc )
        {
            bOK = SetTransferableObjectDescriptor( *mpObjDesc, rFlavor );
        }
        else if( nFormat == SotClipboardFormatId::DRAWING )
        {
            SfxObjectShellRef aOldRef( maDocShellRef );

            maDocShellRef.clear();

            if( mpSdViewIntern )
            {
                SdDrawDocument& rInternDoc = mpSdViewIntern->GetDoc();
                rInternDoc.CreatingDataObj(this);
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( mpSdViewIntern->GetMarkedObjModel() );
                rInternDoc.CreatingDataObj(nullptr);

                bOK = SetObject( pDoc, SDTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );

                if( maDocShellRef.is() )
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
        else if( nFormat == SotClipboardFormatId::GDIMETAFILE )
        {
            if (mpSdViewIntern)
            {
                const bool bToggleOnlineSpell = mpSdDrawDocumentIntern && mpSdDrawDocumentIntern->GetOnlineSpell();
                if (bToggleOnlineSpell)
                    mpSdDrawDocumentIntern->SetOnlineSpell(false);
                bOK = SetGDIMetaFile( mpSdViewIntern->GetMarkedObjMetaFile( true ), rFlavor );
                if (bToggleOnlineSpell)
                    mpSdDrawDocumentIntern->SetOnlineSpell(true);
            }
        }
        else if( SotClipboardFormatId::BITMAP == nFormat || SotClipboardFormatId::PNG == nFormat )
        {
            if (mpSdViewIntern)
            {
                const bool bToggleOnlineSpell = mpSdDrawDocumentIntern && mpSdDrawDocumentIntern->GetOnlineSpell();
                if (bToggleOnlineSpell)
                    mpSdDrawDocumentIntern->SetOnlineSpell(false);
                bOK = SetBitmapEx( mpSdViewIntern->GetMarkedObjBitmapEx(true), rFlavor );
                if (bToggleOnlineSpell)
                    mpSdDrawDocumentIntern->SetOnlineSpell(true);
            }
        }
        else if( ( nFormat == SotClipboardFormatId::STRING ) && mpBookmark )
        {
            bOK = SetString( mpBookmark->GetURL(), rFlavor );
        }
        else if( ( nFormat == SotClipboardFormatId::SVXB ) && mpGraphic )
        {
            bOK = SetGraphic( *mpGraphic, rFlavor );
        }
        else if( ( nFormat == SotClipboardFormatId::SVIM ) && mpImageMap )
        {
            bOK = SetImageMap( *mpImageMap, rFlavor );
        }
        else if( mpBookmark )
        {
            bOK = SetINetBookmark( *mpBookmark, rFlavor );
        }
        else if( nFormat == SotClipboardFormatId::EMBED_SOURCE )
        {
            if( mpSdDrawDocumentIntern )
            {
                SdrSwapGraphicsMode nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
                mpSdDrawDocumentIntern->SetSwapGraphicsMode( SdrSwapGraphicsMode::PURGE );

                if( !maDocShellRef.is() )
                {
                    maDocShellRef = new ::sd::DrawDocShell(
                        mpSdDrawDocumentIntern,
                        SfxObjectCreateMode::EMBEDDED,
                        true,
                        mpSdDrawDocumentIntern->GetDocumentType());
                    mbOwnDocument = false;
                    maDocShellRef->DoInitNew();
                }

                maDocShellRef->SetVisArea( maVisArea );
                bOK = SetObject( maDocShellRef.get(), SDTRANSFER_OBJECTTYPE_DRAWOLE, rFlavor );

                mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
            }
        }
    }

    return bOK;
}

bool SdTransferable::WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pObject, SotClipboardFormatId nObjectType, const DataFlavor& )
{
    bool bRet = false;

    switch( nObjectType )
    {
        case SDTRANSFER_OBJECTTYPE_DRAWMODEL:
        {
            try
            {
                static const bool bDontBurnInStyleSheet = ( getenv( "AVOID_BURN_IN_FOR_GALLERY_THEME" ) != nullptr );
                SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(pObject);
                if ( !bDontBurnInStyleSheet )
                    pDoc->BurnInStyleSheetAttributes();
                rxOStm->SetBufferSize( 16348 );

                Reference< XComponent > xComponent( new SdXImpressDocument( pDoc, true ) );
                pDoc->setUnoModel( Reference< XInterface >::query( xComponent ) );

                {
                    css::uno::Reference<css::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                    if( SvxDrawingLayerExport( pDoc, xDocOut, xComponent, (pDoc->GetDocumentType() == DocumentType::Impress) ? "com.sun.star.comp.Impress.XMLClipboardExporter" : "com.sun.star.comp.DrawingLayer.XMLExporter" ) )
                        rxOStm->Commit();
                }

                xComponent->dispose();
                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            catch( Exception& )
            {
                OSL_FAIL( "sd::SdTransferable::WriteObject(), exception catched!" );
                bRet = false;
            }
        }
        break;

        case SDTRANSFER_OBJECTTYPE_DRAWOLE:
        {
            SfxObjectShell*   pEmbObj = static_cast<SfxObjectShell*>(pObject);
            ::utl::TempFile     aTempFile;
            aTempFile.EnableKillingFile();

            try
            {
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false );
                // mba: no relative URLs for clipboard!
                SfxMedium aMedium( xWorkStore, OUString() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), StreamMode::READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    rxOStm->WriteStream( *pSrcStm );
                    delete pSrcStm;
                }

                bRet = true;
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
        const_cast< ::sd::View* >(mpSdView)->DragFinished( nDropAction );
}

void SdTransferable::ObjectReleased()
{
    if( this == SD_MOD()->pTransferClip )
        SD_MOD()->pTransferClip = nullptr;

    if( this == SD_MOD()->pTransferDrag )
        SD_MOD()->pTransferDrag = nullptr;

    if( this == SD_MOD()->pTransferSelection )
        SD_MOD()->pTransferSelection = nullptr;
}

void SdTransferable::SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc )
{
    delete mpObjDesc;
    mpObjDesc = new TransferableObjectDescriptor( rObjDesc );
    PrepareOLE( rObjDesc );
}

void SdTransferable::SetPageBookmarks( const std::vector<OUString> &rPageBookmarks, bool bPersistent )
{
    if( mpSourceDoc )
    {
        if( mpSdViewIntern )
            mpSdViewIntern->HideSdrPage();

        mpSdDrawDocument->ClearModel(false);

        mpPageDocShell = nullptr;

        maPageBookmarks.clear();

        if( bPersistent )
        {
            mpSdDrawDocument->CreateFirstPages(mpSourceDoc);
            mpSdDrawDocument->InsertBookmarkAsPage( rPageBookmarks, nullptr, false, true, 1, true,
                                                    mpSourceDoc->GetDocSh(), true, true, false );
        }
        else
        {
            mpPageDocShell = mpSourceDoc->GetDocSh();
            maPageBookmarks = rPageBookmarks;
        }

        if( mpSdViewIntern )
        {
            SdPage* pPage = mpSdDrawDocument->GetSdPage( 0, PageKind::Standard );

            if( pPage )
            {
                mpSdViewIntern->MarkAllObj( mpSdViewIntern->ShowSdrPage( pPage ) );
            }
        }

        // set flags for page transferable; if ( mbPageTransferablePersistent == sal_False ),
        // don't offer any formats => it's just for internal purposes
        mbPageTransferable = true;
        mbPageTransferablePersistent = bPersistent;
    }
}

sal_Int64 SAL_CALL SdTransferable::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
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

void SdTransferable::AddUserData (const std::shared_ptr<UserData>& rpData)
{
    maUserData.push_back(rpData);
}

sal_Int32 SdTransferable::GetUserDataCount() const
{
    return maUserData.size();
}

std::shared_ptr<SdTransferable::UserData> SdTransferable::GetUserData (const sal_Int32 nIndex) const
{
    if (nIndex>=0 && nIndex<sal_Int32(maUserData.size()))
        return maUserData[nIndex];
    else
        return std::shared_ptr<UserData>();
}

namespace
{
    class theSdTransferableUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdTransferableUnoTunnelId > {};
}

const css::uno::Sequence< sal_Int8 >& SdTransferable::getUnoTunnelId()
{
    return theSdTransferableUnoTunnelId::get().getSeq();
}

SdTransferable* SdTransferable::getImplementation( const Reference< XInterface >& rxData ) throw()
{
    try
    {
        Reference< css::lang::XUnoTunnel > xUnoTunnel( rxData, UNO_QUERY_THROW );
        return reinterpret_cast<SdTransferable*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SdTransferable::getUnoTunnelId()) ) );
    }
    catch( const css::uno::Exception& )
    {
    }
    return nullptr;
}

void SdTransferable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );
    if( pSdrHint )
    {
        if( SdrHintKind::ModelCleared == pSdrHint->GetKind() )
        {
            EndListening(*mpSourceDoc);
            mpSourceDoc = nullptr;
        }
    }
    else
    {
        if( rHint.GetId() == SfxHintId::Dying )
        {
            if( &rBC == mpSourceDoc )
                mpSourceDoc = nullptr;
            if( &rBC == mpSdViewIntern )
                mpSdViewIntern = nullptr;
            if( &rBC == mpSdView )
                mpSdView = nullptr;
        }
    }
}

void SdTransferable::SetView(const ::sd::View* pView)
{
    if (mpSdView)
        EndListening(*const_cast<sd::View*>(mpSdView));
    mpSdView = pView;
    if (mpSdView)
        StartListening(*const_cast<sd::View*>(mpSdView));
}

bool SdTransferable::SetTableRTF( SdDrawDocument* pModel, const DataFlavor& rFlavor)
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
                return SetAny( Any( Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) ) ), rFlavor );
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
