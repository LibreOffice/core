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
#include <unotools/streamwrap.hxx>

#include <svx/unomodel.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/storagehelper.hxx>
#include <comphelper/servicehelper.hxx>

#include <svtools/embedtransfer.hxx>
#include <sot/storage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itempool.hxx>
#include <svl/urlbmk.hxx>
#include <tools/urlobj.hxx>

#include <drwtrans.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <drawview.hxx>
#include <viewdata.hxx>
#include <scmod.hxx>
#include <dragdata.hxx>

#include <editeng/eeitem.hxx>

#include <editeng/fhgtitem.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

constexpr sal_uInt32 SCDRAWTRANS_TYPE_EMBOBJ    = 1;
constexpr sal_uInt32 SCDRAWTRANS_TYPE_DRAWMODEL = 2;
constexpr sal_uInt32 SCDRAWTRANS_TYPE_DOCUMENT  = 3;

ScDrawTransferObj::ScDrawTransferObj( std::unique_ptr<SdrModel> pClipModel, ScDocShell* pContainerShell,
                                        const TransferableObjectDescriptor& rDesc ) :
    m_pModel( std::move(pClipModel) ),
    m_aObjDesc( rDesc ),
    m_bGraphic( false ),
    m_bGrIsBit( false ),
    m_bOleObj( false ),
    m_nDragSourceFlags( ScDragSrc::Undefined ),
    m_bDragWasInternal( false ),
    maShellID(SfxObjectShell::CreateShellID(pContainerShell))
{

    //  check what kind of objects are contained

    SdrPage* pPage = m_pModel->GetPage(0);
    if (pPage)
    {
        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
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
                        m_bOleObj = true;
                }
                catch( uno::Exception& )
                {}
                // aOleData is initialized later
            }

            //  Graphic object

            if (nSdrObjKind == OBJ_GRAF)
            {
                m_bGraphic = true;
                if ( static_cast<SdrGrafObj*>(pObject)->GetGraphic().GetType() == GraphicType::Bitmap )
                    m_bGrIsBit = true;
            }

            //  URL button

            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( pObject );
            if (pUnoCtrl && SdrInventor::FmForm == pUnoCtrl->GetObjInventor())
            {
                const uno::Reference<awt::XControlModel>& xControlModel = pUnoCtrl->GetUnoControlModel();
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
                                    m_pBookmark.reset( new INetBookmark( aAbs, aLabel ) );
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
    SdrView aView(*m_pModel);
    SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
    aView.MarkAllObj(pPv);
    m_aSrcSize = aView.GetAllMarkedRect().GetSize();

    if ( m_bOleObj )              // single OLE object
    {
        SdrOle2Obj* pObj = GetSingleObject();
        if ( pObj && pObj->GetObjRef().is() )
            SvEmbedTransferHelper::FillTransferableObjectDescriptor( m_aObjDesc, pObj->GetObjRef(), pObj->GetGraphic(), pObj->GetAspect() );
    }

    m_aObjDesc.maSize = m_aSrcSize;
    PrepareOLE( m_aObjDesc );

    // remember a unique ID of the source document

    if ( pContainerShell )
    {
        ScDocument& rDoc = pContainerShell->GetDocument();
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
    if ( pScMod->GetDragData().pDrawTransfer == this )
    {
        OSL_FAIL("ScDrawTransferObj wasn't released");
        pScMod->ResetDragObject();
    }

    m_aOleData = TransferableDataHelper();        // clear before releasing the mutex
    m_aDocShellRef.clear();

    m_pModel.reset();
    m_aDrawPersistRef.clear();                    // after the model

    m_pBookmark.reset();
    m_pDragSourceView.reset();
}

ScDrawTransferObj* ScDrawTransferObj::GetOwnClipboard(const uno::Reference<datatransfer::XTransferable2>& xTransferable)
{
    ScDrawTransferObj* pObj = nullptr;
    if (xTransferable.is())
    {
        uno::Reference<XUnoTunnel> xTunnel( xTransferable, uno::UNO_QUERY );
        if ( xTunnel.is() )
        {
            sal_Int64 nHandle = xTunnel->getSomething( getUnoTunnelId() );
            if ( nHandle )
                pObj = dynamic_cast<ScDrawTransferObj*>(reinterpret_cast<TransferableHelper*>( static_cast<sal_IntPtr>(nHandle) ));
        }
    }

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
            SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
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
    if ( m_bGrIsBit )             // single bitmap graphic
    {
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SVXB );
        AddFormat( SotClipboardFormatId::PNG );
        AddFormat( SotClipboardFormatId::BITMAP );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );
    }
    else if ( m_bGraphic )        // other graphic
    {
        // #i25616#
        AddFormat( SotClipboardFormatId::DRAWING );

        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SVXB );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );
        AddFormat( SotClipboardFormatId::PNG );
        AddFormat( SotClipboardFormatId::BITMAP );
    }
    else if ( m_pBookmark )       // url button
    {
//      AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::SOLK );
        AddFormat( SotClipboardFormatId::STRING );
        AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
        AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
        AddFormat( SotClipboardFormatId::DRAWING );
    }
    else if ( m_bOleObj )         // single OLE object
    {
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );

        CreateOLEData();

        if ( m_aOleData.GetTransferable().is() )
        {
            //  get format list from object snapshot
            //  (this must be after inserting the default formats!)

            DataFlavorExVector              aVector( m_aOleData.GetDataFlavorExVector() );

            for( const auto& rItem : aVector )
                AddFormat( rItem );
        }
    }
    else                        // any drawing objects
    {
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::DRAWING );

        // leave out bitmap and metafile if there are only controls
        if ( !lcl_HasOnlyControls( m_pModel.get() ) )
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

    if ( m_bOleObj && nFormat != SotClipboardFormatId::GDIMETAFILE )
    {
        CreateOLEData();

        if( m_aOleData.GetTransferable().is() && m_aOleData.HasFormat( rFlavor ) )
        {
            bOK = SetAny( m_aOleData.GetAny(rFlavor, rDestDoc) );

            return bOK;
        }
    }

    if( HasFormat( nFormat ) )
    {
        if ( nFormat == SotClipboardFormatId::LINKSRCDESCRIPTOR || nFormat == SotClipboardFormatId::OBJECTDESCRIPTOR )
        {
            bOK = SetTransferableObjectDescriptor( m_aObjDesc );
        }
        else if ( nFormat == SotClipboardFormatId::DRAWING )
        {
            bOK = SetObject( m_pModel.get(), SCDRAWTRANS_TYPE_DRAWMODEL, rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::BITMAP
            || nFormat == SotClipboardFormatId::PNG
            || nFormat == SotClipboardFormatId::GDIMETAFILE )
        {
            // #i71538# use complete SdrViews
            // SdrExchangeView aView( pModel );
            SdrView aView(*m_pModel);
            SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
            OSL_ENSURE( pPv, "pPv not there..." );
            aView.MarkAllObj( pPv );
            if ( nFormat == SotClipboardFormatId::GDIMETAFILE )
                bOK = SetGDIMetaFile( aView.GetMarkedObjMetaFile(true) );
            else
                bOK = SetBitmapEx( aView.GetMarkedObjBitmapEx(true), rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::SVXB )
        {
            // only enabled for single graphics object

            SdrPage* pPage = m_pModel->GetPage(0);
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::Flat );
                SdrObject* pObject = aIter.Next();
                if (pObject && pObject->GetObjIdentifier() == OBJ_GRAF)
                {
                    SdrGrafObj* pGraphObj = static_cast<SdrGrafObj*>(pObject);
                    bOK = SetGraphic( pGraphObj->GetGraphic() );
                }
            }
        }
        else if ( nFormat == SotClipboardFormatId::EMBED_SOURCE )
        {
            if ( m_bOleObj )              // single OLE object
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

                SfxObjectShell* pEmbObj = m_aDocShellRef.get();
                bOK = SetObject( pEmbObj, SCDRAWTRANS_TYPE_DOCUMENT, rFlavor );
            }
        }
        else if( m_pBookmark )
        {
            bOK = SetINetBookmark( *m_pBookmark, rFlavor );
        }
    }
    return bOK;
}

bool ScDrawTransferObj::WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
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
                const SfxItemPool& rItemPool = m_pModel->GetItemPool();
                const SvxFontHeightItem& rDefaultFontHeight = rItemPool.GetDefaultItem(EE_CHAR_FONTHEIGHT);

                // SW should have no MasterPages
                OSL_ENSURE(0 == m_pModel->GetMasterPageCount(), "SW with MasterPages (!)");

                for(sal_uInt16 a(0); a < m_pModel->GetPageCount(); a++)
                {
                    const SdrPage* pPage(m_pModel->GetPage(a));
                    SdrObjListIter aIter(pPage, SdrIterMode::DeepNoGroups);

                    while(aIter.IsMore())
                    {
                        SdrObject* pObj = aIter.Next();
                        const SvxFontHeightItem& rItem = pObj->GetMergedItem(EE_CHAR_FONTHEIGHT);

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

                    std::unique_ptr<SvStream> pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), StreamMode::READ );
                    if( pSrcStm )
                    {
                        rxOStm->SetBufferSize( 0xff00 );
                        rxOStm->WriteStream( *pSrcStm );
                        pSrcStm.reset();
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

void ScDrawTransferObj::DragFinished( sal_Int8 nDropAction )
{
    if ( nDropAction == DND_ACTION_MOVE && !m_bDragWasInternal && !(m_nDragSourceFlags & ScDragSrc::Navigator) )
    {
        //  move: delete source objects

        if ( m_pDragSourceView )
            m_pDragSourceView->DeleteMarked();
    }

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetDragData().pDrawTransfer == this )
        pScMod->ResetDragObject();

    m_pDragSourceView.reset();

    TransferableHelper::DragFinished( nDropAction );
}

void ScDrawTransferObj::SetDrawPersist( const SfxObjectShellRef& rRef )
{
    m_aDrawPersistRef = rRef;
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

void ScDrawTransferObj::SetDragSource( const ScDrawView* pView )
{
    m_pDragSourceView.reset(new SdrView(pView->getSdrModelFromSdrView())); // TTTT pView should be reference
    lcl_InitMarks( *m_pDragSourceView, *pView, pView->GetTab() );

    //! add as listener with document, delete pDragSourceView if document gone
}

void ScDrawTransferObj::SetDragSourceObj( SdrObject& rObj, SCTAB nTab )
{
    m_pDragSourceView.reset(new SdrView(rObj.getSdrModelFromSdrObject()));
    m_pDragSourceView->ShowSdrPage(m_pDragSourceView->GetModel()->GetPage(nTab));
    SdrPageView* pPV = m_pDragSourceView->GetSdrPageView();
    m_pDragSourceView->MarkObj(&rObj, pPV); // TTTT MarkObj should take SdrObject&

    //! add as listener with document, delete pDragSourceView if document gone
}

void ScDrawTransferObj::SetDragSourceFlags(ScDragSrc nFlags)
{
    m_nDragSourceFlags = nFlags;
}

void ScDrawTransferObj::SetDragWasInternal()
{
    m_bDragWasInternal = true;
}

const OUString& ScDrawTransferObj::GetShellID() const
{
    return maShellID;
}

SdrOle2Obj* ScDrawTransferObj::GetSingleObject()
{
    //  if single OLE object was copied, get its object

    SdrPage* pPage = m_pModel->GetPage(0);
    if (pPage)
    {
        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
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
    if (m_aOleData.GetTransferable().is())
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

    m_aOleData = TransferableDataHelper(pEmbedTransfer);
}

//  initialize aDocShellRef with a live document from the ClipDoc

void ScDrawTransferObj::InitDocShell()
{
    if ( !m_aDocShellRef.is() )
    {
        ScDocShell* pDocSh = new ScDocShell;
        m_aDocShellRef = pDocSh;      // ref must be there before InitNew

        pDocSh->DoInitNew();

        ScDocument& rDestDoc = pDocSh->GetDocument();
        rDestDoc.InitDrawLayer( pDocSh );

        SdrModel* pDestModel = rDestDoc.GetDrawLayer();
        // #i71538# use complete SdrViews
        // SdrExchangeView aDestView( pDestModel );
        SdrView aDestView(*pDestModel);
        aDestView.ShowSdrPage(aDestView.GetModel()->GetPage(0));
        aDestView.Paste(
            *m_pModel,
            Point(m_aSrcSize.Width()/2, m_aSrcSize.Height()/2),
            nullptr, SdrInsertFlags::NONE);

        // put objects to right layer (see ScViewFunc::PasteDataFormat for SotClipboardFormatId::DRAWING)

        SdrPage* pPage = pDestModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
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

        tools::Rectangle aDestArea( Point(), m_aSrcSize );
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
