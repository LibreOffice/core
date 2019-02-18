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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/form/FormButtonType.hpp>

#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdouno.hxx>

#include <seltrans.hxx>
#include <transobj.hxx>
#include <drwtrans.hxx>
#include <scmod.hxx>
#include <dbfunc.hxx>
#include <docsh.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include <markdata.hxx>

using namespace com::sun::star;

static bool lcl_IsURLButton( SdrObject* pObject )
{
    bool bRet = false;

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
            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                    bRet = true;
            }
         }
    }

    return bRet;
}

ScSelectionTransferObj* ScSelectionTransferObj::CreateFromView( ScTabView* pView )
{
    ScSelectionTransferObj* pRet = nullptr;

    try
    {
        if ( pView )
        {
            ScSelectionTransferMode eMode = SC_SELTRANS_INVALID;

            SdrView* pSdrView = pView->GetSdrView();
            if ( pSdrView )
            {
                //  handle selection on drawing layer
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                const size_t nMarkCount = rMarkList.GetMarkCount();
                if ( nMarkCount )
                {
                    if ( nMarkCount == 1 )
                    {
                        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

                        if ( nSdrObjKind == OBJ_GRAF )
                        {
                            if ( static_cast<SdrGrafObj*>(pObj)->GetGraphic().GetType() == GraphicType::Bitmap )
                                eMode = SC_SELTRANS_DRAW_BITMAP;
                            else
                                eMode = SC_SELTRANS_DRAW_GRAPHIC;
                        }
                        else if ( nSdrObjKind == OBJ_OLE2 )
                            eMode = SC_SELTRANS_DRAW_OLE;
                        else if ( lcl_IsURLButton( pObj ) )
                            eMode = SC_SELTRANS_DRAW_BOOKMARK;
                    }

                    if ( eMode == SC_SELTRANS_INVALID )
                        eMode = SC_SELTRANS_DRAW_OTHER;     // something selected but no special selection
                }
            }
            if ( eMode == SC_SELTRANS_INVALID )             // no drawing object selected
            {
                ScRange aRange;
                ScViewData& rViewData = pView->GetViewData();
                const ScMarkData& rMark = rViewData.GetMarkData();
                //  allow MultiMarked because GetSimpleArea may be able to merge into a simple range
                //  (GetSimpleArea modifies a local copy of MarkData)
                // Also allow simple filtered area.
                ScMarkType eMarkType;
                if ( ( rMark.IsMarked() || rMark.IsMultiMarked() ) &&
                        (((eMarkType = rViewData.GetSimpleArea( aRange )) == SC_MARK_SIMPLE) ||
                         (eMarkType == SC_MARK_SIMPLE_FILTERED)) )
                {
                    //  only for "real" selection, cursor alone isn't used
                    if ( aRange.aStart == aRange.aEnd )
                        eMode = SC_SELTRANS_CELL;
                    else
                        eMode = SC_SELTRANS_CELLS;
                }
            }

            if ( eMode != SC_SELTRANS_INVALID )
                pRet = new ScSelectionTransferObj( pView, eMode );
        }
    }
    catch (...)
    {
    }

    return pRet;
}

ScSelectionTransferObj::ScSelectionTransferObj( ScTabView* pSource, ScSelectionTransferMode eNewMode ) :
    pView( pSource ),
    eMode( eNewMode )
{
    //! store range for StillValid
}

ScSelectionTransferObj::~ScSelectionTransferObj()
{
    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetSelectionTransfer() == this )
    {
        //  this is reached when the object wasn't really copied to the selection
        //  (CopyToSelection has no effect under Windows)

        ForgetView();
        pScMod->SetSelectionTransfer( nullptr );
    }

    OSL_ENSURE( !pView, "ScSelectionTransferObj dtor: ForgetView not called" );
}

void ScSelectionTransferObj::ForgetView()
{
    pView = nullptr;
    eMode = SC_SELTRANS_INVALID;

    mxCellData.clear();
    mxDrawData.clear();
}

void ScSelectionTransferObj::AddSupportedFormats()
{
    //  AddSupportedFormats must work without actually creating the
    //  "real" transfer object

    switch (eMode)
    {
        case SC_SELTRANS_CELL:
        case SC_SELTRANS_CELLS:
            //  same formats as in ScTransferObj::AddSupportedFormats
            AddFormat( SotClipboardFormatId::EMBED_SOURCE );
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
            AddFormat( SotClipboardFormatId::HTML );
            AddFormat( SotClipboardFormatId::SYLK );
            AddFormat( SotClipboardFormatId::LINK );
            AddFormat( SotClipboardFormatId::DIF );
            AddFormat( SotClipboardFormatId::STRING );
            AddFormat( SotClipboardFormatId::STRING_TSVC );
            AddFormat( SotClipboardFormatId::RTF );
            AddFormat( SotClipboardFormatId::RICHTEXT );
            if ( eMode == SC_SELTRANS_CELL )
            {
                AddFormat( SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT );
            }
            break;

        // different graphic formats as in ScDrawTransferObj::AddSupportedFormats:

        case SC_SELTRANS_DRAW_BITMAP:
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::SVXB );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            break;

        case SC_SELTRANS_DRAW_GRAPHIC:
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::SVXB );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
             break;

        case SC_SELTRANS_DRAW_BOOKMARK:
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::SOLK );
            AddFormat( SotClipboardFormatId::STRING );
            AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
            AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
            AddFormat( SotClipboardFormatId::DRAWING );
            break;

        case SC_SELTRANS_DRAW_OLE:
            AddFormat( SotClipboardFormatId::EMBED_SOURCE );
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            break;

        case SC_SELTRANS_DRAW_OTHER:
            //  other drawing objects
            AddFormat( SotClipboardFormatId::EMBED_SOURCE );
            AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
            AddFormat( SotClipboardFormatId::DRAWING );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            break;

        default:
        {
            // added to avoid warnings
        }
    }
}

void ScSelectionTransferObj::CreateCellData()
{
    OSL_ENSURE( !mxCellData.is(), "CreateCellData twice" );
    if ( pView )
    {
        ScViewData& rViewData = pView->GetViewData();
        ScMarkData aNewMark( rViewData.GetMarkData() );    // use local copy for MarkToSimple
        aNewMark.MarkToSimple();

        //  similar to ScViewFunctionSet::BeginDrag
        if ( aNewMark.IsMarked() && !aNewMark.IsMultiMarked() )
        {
            ScDocShell* pDocSh = rViewData.GetDocShell();

            ScRange aSelRange;
            aNewMark.GetMarkArea( aSelRange );
            ScDocShellRef aDragShellRef;
            if ( pDocSh->GetDocument().HasOLEObjectsInArea( aSelRange, &aNewMark ) )
            {
                aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
                aDragShellRef->DoInitNew();
            }
            ScDrawLayer::SetGlobalDrawPersist( aDragShellRef.get() );

            ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));
            // bApi = sal_True -> no error messages
            // #i18364# bStopEdit = sal_False -> don't end edit mode
            // (this may be called from pasting into the edit line)
            bool bCopied = rViewData.GetView()->CopyToClip( pClipDoc.get(), false, true, true, false );

            ScDrawLayer::SetGlobalDrawPersist(nullptr);

            if ( bCopied )
            {
                TransferableObjectDescriptor aObjDesc;
                pDocSh->FillTransferableObjectDescriptor( aObjDesc );
                aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
                // maSize is set in ScTransferObj ctor

                rtl::Reference<ScTransferObj> pTransferObj = new ScTransferObj( std::move(pClipDoc), aObjDesc );

                // SetDragHandlePos is not used - there is no mouse position
                //? pTransferObj->SetVisibleTab( nTab );

                SfxObjectShellRef aPersistRef( aDragShellRef.get() );
                pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive

                pTransferObj->SetDragSource( pDocSh, aNewMark );

                mxCellData = pTransferObj;
            }
        }
    }
    OSL_ENSURE( mxCellData.is(), "can't create CellData" );
}

void ScSelectionTransferObj::CreateDrawData()
{
    OSL_ENSURE( !mxDrawData.is(), "CreateDrawData twice" );
    if ( pView )
    {
        //  similar to ScDrawView::BeginDrag

        ScDrawView* pDrawView = pView->GetScDrawView();
        if ( pDrawView )
        {
            bool bAnyOle, bOneOle;
            const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
            ScDrawView::CheckOle( rMarkList, bAnyOle, bOneOle );

            ScDocShellRef aDragShellRef;
            if (bAnyOle)
            {
                aDragShellRef = new ScDocShell; // Without Ref the DocShell does not live
                aDragShellRef->DoInitNew();
            }

            ScDrawLayer::SetGlobalDrawPersist( aDragShellRef.get() );
            std::unique_ptr<SdrModel> pModel(pDrawView->CreateMarkedObjModel());
            ScDrawLayer::SetGlobalDrawPersist(nullptr);

            ScViewData& rViewData = pView->GetViewData();
            ScDocShell* pDocSh = rViewData.GetDocShell();

            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScDrawTransferObj ctor

            rtl::Reference<ScDrawTransferObj> pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, aObjDesc );

            SfxObjectShellRef aPersistRef( aDragShellRef.get() );
            pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
            pTransferObj->SetDragSource( pDrawView );       // copies selection

            mxDrawData = pTransferObj;
        }
    }
    OSL_ENSURE( mxDrawData.is(), "can't create DrawData" );
}

ScTransferObj* ScSelectionTransferObj::GetCellData()
{
    if ( !mxCellData.is() && ( eMode == SC_SELTRANS_CELL || eMode == SC_SELTRANS_CELLS ) )
        CreateCellData();
    return mxCellData.get();
}

ScDrawTransferObj* ScSelectionTransferObj::GetDrawData()
{
    if ( !mxDrawData.is() && ( eMode == SC_SELTRANS_DRAW_BITMAP || eMode == SC_SELTRANS_DRAW_GRAPHIC ||
                               eMode == SC_SELTRANS_DRAW_BOOKMARK || eMode == SC_SELTRANS_DRAW_OLE ||
                               eMode == SC_SELTRANS_DRAW_OTHER ) )
        CreateDrawData();
    return mxDrawData.get();
}

bool ScSelectionTransferObj::GetData(
    const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc )
{
    bool bOK = false;

    uno::Reference<datatransfer::XTransferable> xSource;
    switch (eMode)
    {
        case SC_SELTRANS_CELL:
        case SC_SELTRANS_CELLS:
            xSource = GetCellData();
            break;
        case SC_SELTRANS_DRAW_BITMAP:
        case SC_SELTRANS_DRAW_GRAPHIC:
        case SC_SELTRANS_DRAW_BOOKMARK:
        case SC_SELTRANS_DRAW_OLE:
        case SC_SELTRANS_DRAW_OTHER:
            xSource = GetDrawData();
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    if ( xSource.is() )
    {
        TransferableDataHelper aHelper( xSource );
        uno::Any aAny = aHelper.GetAny(rFlavor, rDestDoc);
        bOK = SetAny( aAny );
    }

    return bOK;
}

void ScSelectionTransferObj::ObjectReleased()
{
    //  called when another selection is set from outside

    ForgetView();

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetSelectionTransfer() == this )
        pScMod->SetSelectionTransfer( nullptr );

    TransferableHelper::ObjectReleased();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
