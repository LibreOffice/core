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
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/streamwrap.hxx>

#include <svx/unomodel.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/servicehelper.hxx>

#include <svtools/embedtransfer.hxx>
#include <sot/storage.hxx>
#include <vcl/virdev.hxx>
#include <svx/fmglob.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdxcgv.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itempool.hxx>
#include <svl/urlbmk.hxx>
#include <tools/urlobj.hxx>
#include <osl/mutex.hxx>

#include "drwtrans.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "scmod.hxx"
#include "chartlis.hxx"
#include "rangeutl.hxx"
#include <formula/grammar.hxx>
#include "dragdata.hxx"
#include "clipdata.hxx"

#include "scitems.hxx"

#include <editeng/eeitem.hxx>

#include <editeng/fhgtitem.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

#define SCDRAWTRANS_TYPE_EMBOBJ         SotClipboardFormatId::STRING
#define SCDRAWTRANS_TYPE_DRAWMODEL      SotClipboardFormatId::BITMAP
#define SCDRAWTRANS_TYPE_DOCUMENT       SotClipboardFormatId::GDIMETAFILE

ScDrawTransferObj::ScDrawTransferObj( SdrModel* pClipModel, ScDocShell* pContainerShell,
                                        const TransferableObjectDescriptor& rDesc ) :
    pModel( pClipModel ),
    aObjDesc( rDesc ),
    pBookmark( nullptr ),
    bGraphic( false ),
    bGrIsBit( false ),
    bOleObj( false ),
    pDragSourceView( nullptr ),
    nDragSourceFlags( ScDragSrc::Undefined ),
    bDragWasInternal( false ),
    nSourceDocID( 0 ),
    maShellID(SfxObjectShell::CreateShellID(pContainerShell))
{

    //  check what kind of objects are contained

    SdrPage* pPage = pModel->GetPage(0);
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        if (pObject && !aIter.Next())               // exactly one object?
        {

            //  OLE object

            sal_uInt16 nSdrObjKind = pObject->GetObjIdentifier();
            if (nSdrObjKind == OBJ_OLE2)
            {
                // if object has no persistence it must be copied as a part of document
                try
                {
                    uno::Reference< embed::XEmbedPersist > xPersObj( static_cast<SdrOle2Obj*>(pObject)->GetObjRef(), uno::UNO_QUERY );
                    if ( xPersObj.is() && xPersObj->hasEntry() )
                        bOleObj = true;
                }
                catch( uno::Exception& )
                {}
                // aOleData is initialized later
            }

            //  Graphic object

            if (nSdrObjKind == OBJ_GRAF)
            {
                bGraphic = true;
                if ( static_cast<SdrGrafObj*>(pObject)->GetGraphic().GetType() == GraphicType::Bitmap )
                    bGrIsBit = true;
            }

            //  URL button

            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( pObject );
            if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
            {
                uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
                OSL_ENSURE( xControlModel.is(), "uno control without model" );
                if ( xControlModel.is() )
                {
                    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                    uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

                    OUString sPropButtonType( "ButtonType" );
                    OUString sPropTargetURL( "TargetURL" );
                    OUString sPropLabel( "Label" );

                    if(xInfo->hasPropertyByName( sPropButtonType ))
                    {
                        uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                        form::FormButtonType eTmp;
                        if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                        {
                            // URL
                            if(xInfo->hasPropertyByName( sPropTargetURL ))
                            {
                                aAny = xPropSet->getPropertyValue( sPropTargetURL );
                                OUString sTmp;
                                if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                                {
                                    OUString aUrl = sTmp;
                                    OUString aAbs;
                                    const SfxMedium* pMedium;
                                    if (pContainerShell && (pMedium = pContainerShell->GetMedium()) != nullptr)
                                    {
                                        bool bWasAbs = true;
                                        aAbs = pMedium->GetURLObject().smartRel2Abs( aUrl, bWasAbs ).
                                                    GetMainURL(INetURLObject::DecodeMechanism::NONE);
                                        // full path as stored INetBookmark must be encoded
                                    }
                                    else
                                        aAbs = aUrl;

                                    // Label
                                    OUString aLabel;
                                    if(xInfo->hasPropertyByName( sPropLabel ))
                                    {
                                        aAny = xPropSet->getPropertyValue( sPropLabel );
                                        if ( (aAny >>= sTmp) && !sTmp.isEmpty() )
                                        {
                                            aLabel = sTmp;
                                        }
                                    }
                                    pBookmark = new INetBookmark( aAbs, aLabel );
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //  get size for object descriptor

    // #i71538# use complete SdrViews
    // SdrExchangeView aView(pModel);
    SdrView aView(pModel);
    SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
    aView.MarkAllObj(pPv);
    aSrcSize = aView.GetAllMarkedRect().GetSize();

    if ( bOleObj )              // single OLE object
    {
        SdrOle2Obj* pObj = GetSingleObject();
        if ( pObj && pObj->GetObjRef().is() )
            SvEmbedTransferHelper::FillTransferableObjectDescriptor( aObjDesc, pObj->GetObjRef(), pObj->GetGraphic(), pObj->GetAspect() );
    }

    aObjDesc.maSize = aSrcSize;
    PrepareOLE( aObjDesc );

    // remember a unique ID of the source document

    if ( pContainerShell )
    {
        ScDocument& rDoc = pContainerShell->GetDocument();
        nSourceDocID = rDoc.GetDocumentID();
        if ( pPage )
        {
            ScChartHelper::FillProtectedChartRangesVector( m_aProtectedChartRangesVector, &rDoc, pPage );
        }
    }
}

ScDrawTransferObj::~ScDrawTransferObj()
{
    SolarMutexGuard aSolarGuard;

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetClipData().pDrawClipboard == this )
    {
        OSL_FAIL("ScDrawTransferObj wasn't released");
        pScMod->SetClipObject( nullptr, nullptr );
    }
    if ( pScMod->GetDragData().pDrawTransfer == this )
    {
        OSL_FAIL("ScDrawTransferObj wasn't released");
        pScMod->ResetDragObject();
    }

    aOleData = TransferableDataHelper();        // clear before releasing the mutex
    aDocShellRef.clear();

    delete pModel;
    aDrawPersistRef.clear();                    // after the model

    delete pBookmark;
    delete pDragSourceView;
}

ScDrawTransferObj* ScDrawTransferObj::GetOwnClipboard( vcl::Window* )
{
    ScDrawTransferObj* pObj = SC_MOD()->GetClipData().pDrawClipboard;
    return pObj;
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
                    if (dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr)
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

void ScDrawTransferObj::AddSupportedFormats()
{
    if ( bGrIsBit )             // single bitmap graphic
    {
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SVXB );
        AddFormat( SotClipboardFormatId::PNG );
        AddFormat( SotClipboardFormatId::BITMAP );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );
    }
    else if ( bGraphic )        // other graphic
    {
        // #i25616#
        AddFormat( SotClipboardFormatId::DRAWING );

        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SVXB );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );
        AddFormat( SotClipboardFormatId::PNG );
        AddFormat( SotClipboardFormatId::BITMAP );
    }
    else if ( pBookmark )       // url button
    {
//      AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SOLK );
        AddFormat( SotClipboardFormatId::STRING );
        AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
        AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
        AddFormat( SotClipboardFormatId::DRAWING );
    }
    else if ( bOleObj )         // single OLE object
    {
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );

        CreateOLEData();

        if ( aOleData.GetTransferable().is() )
        {
            //  get format list from object snapshot
            //  (this must be after inserting the default formats!)

            DataFlavorExVector              aVector( aOleData.GetDataFlavorExVector() );
            DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

            while( aIter != aEnd )
                AddFormat( *aIter++ );
        }
    }
    else                        // any drawing objects
    {
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::DRAWING );

        // leave out bitmap and metafile if there are only controls
        if ( !lcl_HasOnlyControls( pModel ) )
        {
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
        }
    }

//  if( pImageMap )
//      AddFormat( SotClipboardFormatId::SVIM );
}

bool ScDrawTransferObj::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc )
{
    bool bOK = false;
    SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );

    if ( bOleObj && nFormat != SotClipboardFormatId::GDIMETAFILE )
    {
        CreateOLEData();

        if( aOleData.GetTransferable().is() && aOleData.HasFormat( rFlavor ) )
        {
            SdrSwapGraphicsMode nOldSwapMode(SdrSwapGraphicsMode::DEFAULT);

            if( pModel )
            {
                nOldSwapMode = pModel->GetSwapGraphicsMode();
                pModel->SetSwapGraphicsMode( SdrSwapGraphicsMode::PURGE );
            }

            bOK = SetAny( aOleData.GetAny(rFlavor, rDestDoc), rFlavor );

            if( pModel )
                pModel->SetSwapGraphicsMode( nOldSwapMode );

            return bOK;
        }
    }

    if( HasFormat( nFormat ) )
    {
        if ( nFormat == SotClipboardFormatId::LINKSRCDESCRIPTOR || nFormat == SotClipboardFormatId::OBJECTDESCRIPTOR )
        {
            bOK = SetTransferableObjectDescriptor( aObjDesc, rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::DRAWING )
        {
            bOK = SetObject( pModel, SCDRAWTRANS_TYPE_DRAWMODEL, rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::BITMAP
            || nFormat == SotClipboardFormatId::PNG
            || nFormat == SotClipboardFormatId::GDIMETAFILE )
        {
            // #i71538# use complete SdrViews
            // SdrExchangeView aView( pModel );
            SdrView aView( pModel );
            SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
            OSL_ENSURE( pPv, "pPv not there..." );
            aView.MarkAllObj( pPv );
            if ( nFormat == SotClipboardFormatId::GDIMETAFILE )
                bOK = SetGDIMetaFile( aView.GetMarkedObjMetaFile(true), rFlavor );
            else
                bOK = SetBitmapEx( aView.GetMarkedObjBitmapEx(true), rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::SVXB )
        {
            // only enabled for single graphics object

            SdrPage* pPage = pModel->GetPage(0);
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, SdrIterMode::Flat );
                SdrObject* pObject = aIter.Next();
                if (pObject && pObject->GetObjIdentifier() == OBJ_GRAF)
                {
                    SdrGrafObj* pGraphObj = static_cast<SdrGrafObj*>(pObject);
                    bOK = SetGraphic( pGraphObj->GetGraphic(), rFlavor );
                }
            }
        }
        else if ( nFormat == SotClipboardFormatId::EMBED_SOURCE )
        {
            if ( bOleObj )              // single OLE object
            {
                SdrOle2Obj* pObj = GetSingleObject();
                if ( pObj && pObj->GetObjRef().is() )
                {
                    bOK = SetObject( pObj->GetObjRef().get(), SCDRAWTRANS_TYPE_EMBOBJ, rFlavor );
                }
            }
            else                        // create object from contents
            {
                //TODO/LATER: needs new Format, because now single OLE and "this" are different
                InitDocShell();         // set aDocShellRef

                SfxObjectShell* pEmbObj = aDocShellRef.get();
                bOK = SetObject( pEmbObj, SCDRAWTRANS_TYPE_DOCUMENT, rFlavor );
            }
        }
        else if( pBookmark )
        {
            bOK = SetINetBookmark( *pBookmark, rFlavor );
        }
    }
    return bOK;
}

bool ScDrawTransferObj::WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId,
                                        const css::datatransfer::DataFlavor& /* rFlavor */ )
{
    // called from SetObject, put data into stream

    bool bRet = false;
    switch (nUserObjectId)
    {
        case SCDRAWTRANS_TYPE_DRAWMODEL:
            {
                SdrModel* pDrawModel = static_cast<SdrModel*>(pUserObject);
                rxOStm->SetBufferSize( 0xff00 );

                // for the changed pool defaults from drawing layer pool set those
                // attributes as hard attributes to preserve them for saving
                const SfxItemPool& rItemPool = pModel->GetItemPool();
                const SvxFontHeightItem& rDefaultFontHeight = static_cast<const SvxFontHeightItem&>(rItemPool.GetDefaultItem(EE_CHAR_FONTHEIGHT));

                // SW should have no MasterPages
                OSL_ENSURE(0 == pModel->GetMasterPageCount(), "SW with MasterPages (!)");

                for(sal_uInt16 a(0); a < pModel->GetPageCount(); a++)
                {
                    const SdrPage* pPage = pModel->GetPage(a);
                    SdrObjListIter aIter(*pPage, SdrIterMode::DeepNoGroups);

                    while(aIter.IsMore())
                    {
                        SdrObject* pObj = aIter.Next();
                        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(pObj->GetMergedItem(EE_CHAR_FONTHEIGHT));

                        if(rItem.GetHeight() == rDefaultFontHeight.GetHeight())
                        {
                            pObj->SetMergedItem(rDefaultFontHeight);
                        }
                    }
                }

                {
                    css::uno::Reference<css::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                    if( SvxDrawingLayerExport( pDrawModel, xDocOut ) )
                        rxOStm->Commit();
                }

                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        case SCDRAWTRANS_TYPE_EMBOBJ:
            {
                // impl. for "single OLE"
                embed::XEmbeddedObject* pEmbObj = static_cast<embed::XEmbeddedObject*>(pUserObject);

                ::utl::TempFile     aTempFile;
                aTempFile.EnableKillingFile();
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                uno::Reference < embed::XEmbedPersist > xPers( static_cast<embed::XVisualObject*>(pEmbObj), uno::UNO_QUERY );
                if ( xPers.is() )
                {
                    try
                    {
                        uno::Sequence < beans::PropertyValue > aSeq;
                        OUString aDummyName("Dummy");
                        xPers->storeToEntry( xWorkStore, aDummyName, aSeq, aSeq );
                        if ( xWorkStore->isStreamElement( aDummyName ) )
                        {
                            uno::Reference < io::XOutputStream > xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                            uno::Reference < io::XStream > xNewStream = xWorkStore->openStreamElement( aDummyName, embed::ElementModes::READ );
                            ::comphelper::OStorageHelper::CopyInputToOutput( xNewStream->getInputStream(), xDocOut );
                        }
                        else
                        {
                            uno::Reference < io::XStream > xDocStr( new utl::OStreamWrapper( *rxOStm ) );
                            uno::Reference< embed::XStorage > xDocStg = ::comphelper::OStorageHelper::GetStorageFromStream( xDocStr );
                            uno::Reference < embed::XStorage > xNewStg = xWorkStore->openStorageElement( aDummyName, embed::ElementModes::READ );
                            xNewStg->copyToStorage( xDocStg );
                            uno::Reference < embed::XTransactedObject > xTrans( xDocStg, uno::UNO_QUERY );
                            if ( xTrans.is() )
                                xTrans->commit();
                        }

                        rxOStm->Commit();
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }

                break;
            }
        case SCDRAWTRANS_TYPE_DOCUMENT:
            {
                // impl. for "DocShell"
                SfxObjectShell*   pEmbObj = static_cast<SfxObjectShell*>(pUserObject);

                try
                {
                    ::utl::TempFile     aTempFile;
                    aTempFile.EnableKillingFile();
                    uno::Reference< embed::XStorage > xWorkStore =
                        ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                    // write document storage
                    pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false );

                    // mba: no relative URLs for clipboard!
                    SfxMedium aMedium( xWorkStore, OUString() );
                    pEmbObj->DoSaveObjectAs( aMedium, false );
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

                    xWorkStore->dispose();
                    xWorkStore.clear();
                    rxOStm->Commit();
                }
                catch ( uno::Exception& )
                {}

                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        default:
            OSL_FAIL("unknown object id");
    }
    return bRet;
}

void ScDrawTransferObj::ObjectReleased()
{
    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetClipData().pDrawClipboard == this )
        pScMod->SetClipObject( nullptr, nullptr );

    TransferableHelper::ObjectReleased();
}

void ScDrawTransferObj::DragFinished( sal_Int8 nDropAction )
{
    if ( nDropAction == DND_ACTION_MOVE && !bDragWasInternal && !(nDragSourceFlags & ScDragSrc::Navigator) )
    {
        //  move: delete source objects

        if ( pDragSourceView )
            pDragSourceView->DeleteMarked();
    }

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetDragData().pDrawTransfer == this )
        pScMod->ResetDragObject();

    DELETEZ( pDragSourceView );

    TransferableHelper::DragFinished( nDropAction );
}

void ScDrawTransferObj::SetDrawPersist( const SfxObjectShellRef& rRef )
{
    aDrawPersistRef = rRef;
}

static void lcl_InitMarks( SdrMarkView& rDest, const SdrMarkView& rSource, SCTAB nTab )
{
    rDest.ShowSdrPage(rDest.GetModel()->GetPage(nTab));
    SdrPageView* pDestPV = rDest.GetSdrPageView();
    OSL_ENSURE(pDestPV,"PageView ?");

    const SdrMarkList& rMarkList = rSource.GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        SdrObject* pObj = pMark->GetMarkedSdrObj();

        rDest.MarkObj(pObj, pDestPV);
    }
}

void ScDrawTransferObj::SetDragSource( ScDrawView* pView )
{
    DELETEZ( pDragSourceView );
    pDragSourceView = new SdrView( pView->GetModel() );
    lcl_InitMarks( *pDragSourceView, *pView, pView->GetTab() );

    //! add as listener with document, delete pDragSourceView if document gone
}

void ScDrawTransferObj::SetDragSourceObj( SdrObject* pObj, SCTAB nTab )
{
    DELETEZ( pDragSourceView );
    pDragSourceView = new SdrView( pObj->GetModel() );
    pDragSourceView->ShowSdrPage(pDragSourceView->GetModel()->GetPage(nTab));
    SdrPageView* pPV = pDragSourceView->GetSdrPageView();
    pDragSourceView->MarkObj(pObj, pPV);

    //! add as listener with document, delete pDragSourceView if document gone
}

void ScDrawTransferObj::SetDragSourceFlags(ScDragSrc nFlags)
{
    nDragSourceFlags = nFlags;
}

void ScDrawTransferObj::SetDragWasInternal()
{
    bDragWasInternal = true;
}

const OUString& ScDrawTransferObj::GetShellID() const
{
    return maShellID;
}

SdrOle2Obj* ScDrawTransferObj::GetSingleObject()
{
    //  if single OLE object was copied, get its object

    SdrPage* pPage = pModel->GetPage(0);
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        if (pObject && pObject->GetObjIdentifier() == OBJ_OLE2)
        {
            return static_cast<SdrOle2Obj*>(pObject);
        }
    }

    return nullptr;
}

void ScDrawTransferObj::CreateOLEData()
{
    if (aOleData.GetTransferable().is())
        // Already created.
        return;

    SdrOle2Obj* pObj = GetSingleObject();
    if (!pObj || !pObj->GetObjRef().is())
        // No OLE object present.
        return;

    SvEmbedTransferHelper* pEmbedTransfer =
        new SvEmbedTransferHelper(
            pObj->GetObjRef(), pObj->GetGraphic(), pObj->GetAspect());

    pEmbedTransfer->SetParentShellID(maShellID);

    aOleData = TransferableDataHelper(pEmbedTransfer);
}

//  initialize aDocShellRef with a live document from the ClipDoc

void ScDrawTransferObj::InitDocShell()
{
    if ( !aDocShellRef.is() )
    {
        ScDocShell* pDocSh = new ScDocShell;
        aDocShellRef = pDocSh;      // ref must be there before InitNew

        pDocSh->DoInitNew();

        ScDocument& rDestDoc = pDocSh->GetDocument();
        rDestDoc.InitDrawLayer( pDocSh );

        SdrModel* pDestModel = rDestDoc.GetDrawLayer();
        // #i71538# use complete SdrViews
        // SdrExchangeView aDestView( pDestModel );
        SdrView aDestView( pDestModel );
        aDestView.ShowSdrPage(aDestView.GetModel()->GetPage(0));
        aDestView.Paste(
            *pModel,
            Point(aSrcSize.Width()/2, aSrcSize.Height()/2),
            nullptr, SdrInsertFlags::NONE);

        // put objects to right layer (see ScViewFunc::PasteDataFormat for SotClipboardFormatId::DRAWING)

        SdrPage* pPage = pDestModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( dynamic_cast<const SdrUnoObj*>( pObject) !=  nullptr )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                else
                    pObject->NbcSetLayer(SC_LAYER_FRONT);
                pObject = aIter.Next();
            }
        }

        Point aTmpPoint;
        Rectangle aDestArea( aTmpPoint, aSrcSize );
        pDocSh->SetVisArea( aDestArea );

        ScViewOptions aViewOpt( rDestDoc.GetViewOptions() );
        aViewOpt.SetOption( VOPT_GRID, false );
        rDestDoc.SetViewOptions( aViewOpt );

        ScViewData aViewData( pDocSh, nullptr );
        aViewData.SetTabNo( 0 );
        aViewData.SetScreen( aDestArea );
        aViewData.SetCurX( 0 );
        aViewData.SetCurY( 0 );
        pDocSh->UpdateOle(&aViewData, true);
    }
}

namespace
{
    class theScDrawTransferObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScDrawTransferObjUnoTunnelId > {};
}

const css::uno::Sequence< sal_Int8 >& ScDrawTransferObj::getUnoTunnelId()
{
    return theScDrawTransferObjUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL ScDrawTransferObj::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;
    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = reinterpret_cast< sal_Int64 >( this );
    }
    else
        nRet = TransferableHelper::getSomething(rId);
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
