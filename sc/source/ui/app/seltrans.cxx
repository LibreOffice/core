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
#include <svx/fmglob.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdouno.hxx>

#include "seltrans.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "scmod.hxx"
#include "dbfunc.hxx"
#include "docsh.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"
#include "markdata.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

static sal_Bool lcl_IsURLButton( SdrObject* pObject )
{
    sal_Bool bRet = false;

    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObject);
    if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
       {
        uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
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
                    bRet = sal_True;
            }
         }
    }

    return bRet;
}


ScSelectionTransferObj* ScSelectionTransferObj::CreateFromView( ScTabView* pView )
{
    ScSelectionTransferObj* pRet = NULL;

    if ( pView )
    {
        ScSelectionTransferMode eMode = SC_SELTRANS_INVALID;

        SdrView* pSdrView = pView->GetSdrView();
        if ( pSdrView )
        {
            //  handle selection on drawing layer
            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            sal_uLong nMarkCount = rMarkList.GetMarkCount();
            if ( nMarkCount )
            {
                if ( nMarkCount == 1 )
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

                    if ( nSdrObjKind == OBJ_GRAF )
                    {
                        if ( ((SdrGrafObj*)pObj)->GetGraphic().GetType() == GRAPHIC_BITMAP )
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
            ScViewData* pViewData = pView->GetViewData();
            const ScMarkData& rMark = pViewData->GetMarkData();
            //  allow MultiMarked because GetSimpleArea may be able to merge into a simple range
            //  (GetSimpleArea modifies a local copy of MarkData)
            // Also allow simple filtered area.
            ScMarkType eMarkType;
            if ( ( rMark.IsMarked() || rMark.IsMultiMarked() ) &&
                    (((eMarkType = pViewData->GetSimpleArea( aRange )) == SC_MARK_SIMPLE) ||
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

    return pRet;
}


ScSelectionTransferObj::ScSelectionTransferObj( ScTabView* pSource, ScSelectionTransferMode eNewMode ) :
    pView( pSource ),
    eMode( eNewMode ),
    pCellData( NULL ),
    pDrawData( NULL )
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
        pScMod->SetSelectionTransfer( NULL );
    }

    OSL_ENSURE( !pView, "ScSelectionTransferObj dtor: ForgetView not called" );
}

sal_Bool ScSelectionTransferObj::StillValid()
{
    //! check if view still has same cell selection
    //! (but return sal_False if data has changed inbetween)
    return false;
}

void ScSelectionTransferObj::ForgetView()
{
    pView = NULL;
    eMode = SC_SELTRANS_INVALID;

    if (pCellData)
    {
        pCellData->release();
        pCellData = NULL;
    }
    if (pDrawData)
    {
        pDrawData->release();
        pDrawData = NULL;
    }
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
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            AddFormat( SOT_FORMATSTR_ID_PNG );
            AddFormat( SOT_FORMAT_BITMAP );
            AddFormat( SOT_FORMATSTR_ID_HTML );
            AddFormat( SOT_FORMATSTR_ID_SYLK );
            AddFormat( SOT_FORMATSTR_ID_LINK );
            AddFormat( SOT_FORMATSTR_ID_DIF );
            AddFormat( SOT_FORMAT_STRING );
            AddFormat( SOT_FORMAT_RTF );
            if ( eMode == SC_SELTRANS_CELL )
                AddFormat( SOT_FORMATSTR_ID_EDITENGINE );
            break;

        // different graphic formats as in ScDrawTransferObj::AddSupportedFormats:

        case SC_SELTRANS_DRAW_BITMAP:
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMATSTR_ID_SVXB );
            AddFormat( SOT_FORMATSTR_ID_PNG );
            AddFormat( SOT_FORMAT_BITMAP );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            break;

        case SC_SELTRANS_DRAW_GRAPHIC:
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMATSTR_ID_SVXB );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            AddFormat( SOT_FORMATSTR_ID_PNG );
            AddFormat( SOT_FORMAT_BITMAP );
             break;

        case SC_SELTRANS_DRAW_BOOKMARK:
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMATSTR_ID_SOLK );
            AddFormat( SOT_FORMAT_STRING );
            AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
            AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            break;

        case SC_SELTRANS_DRAW_OLE:
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            break;

        case SC_SELTRANS_DRAW_OTHER:
            //  other drawing objects
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            AddFormat( SOT_FORMATSTR_ID_PNG );
            AddFormat( SOT_FORMAT_BITMAP );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            break;

        default:
        {
            // added to avoid warnings
        }
    }
}

void ScSelectionTransferObj::CreateCellData()
{
    OSL_ENSURE( !pCellData, "CreateCellData twice" );
    if ( pView )
    {
        ScViewData* pViewData = pView->GetViewData();
        ScMarkData aNewMark( pViewData->GetMarkData() );    // use local copy for MarkToSimple
        aNewMark.MarkToSimple();

        //  similar to ScViewFunctionSet::BeginDrag
        if ( aNewMark.IsMarked() && !aNewMark.IsMultiMarked() )
        {
            ScDocShell* pDocSh = pViewData->GetDocShell();

            ScRange aSelRange;
            aNewMark.GetMarkArea( aSelRange );
            ScDocShellRef aDragShellRef;
            if ( pDocSh->GetDocument()->HasOLEObjectsInArea( aSelRange, &aNewMark ) )
            {
                aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
                aDragShellRef->DoInitNew(NULL);
            }
            ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);

            ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
            // bApi = sal_True -> no error messages
            // #i18364# bStopEdit = sal_False -> don't end edit mode
            // (this may be called from pasting into the edit line)
            sal_Bool bCopied = pViewData->GetView()->CopyToClip( pClipDoc, false, sal_True, sal_True, false );

            ScDrawLayer::SetGlobalDrawPersist(NULL);

            if ( bCopied )
            {
                TransferableObjectDescriptor aObjDesc;
                pDocSh->FillTransferableObjectDescriptor( aObjDesc );
                aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
                // maSize is set in ScTransferObj ctor

                ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
                uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

                // SetDragHandlePos is not used - there is no mouse position
                //? pTransferObj->SetVisibleTab( nTab );

                SfxObjectShellRef aPersistRef( aDragShellRef );
                pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive

                pTransferObj->SetDragSource( pDocSh, aNewMark );

                pCellData = pTransferObj;
                pCellData->acquire();       // keep ref count up - released in ForgetView
            }
            else
                delete pClipDoc;
        }
    }
    OSL_ENSURE( pCellData, "can't create CellData" );
}

void ScSelectionTransferObj::CreateDrawData()
{
    OSL_ENSURE( !pDrawData, "CreateDrawData twice" );
    if ( pView )
    {
        //  similar to ScDrawView::BeginDrag

        ScDrawView* pDrawView = pView->GetScDrawView();
        if ( pDrawView )
        {
            bool bAnyOle, bOneOle;
            const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
            ScDrawView::CheckOle( rMarkList, bAnyOle, bOneOle );

            //---------------------------------------------------------
            ScDocShellRef aDragShellRef;
            if (bAnyOle)
            {
                aDragShellRef = new ScDocShell;     // ohne Ref lebt die DocShell nicht !!!
                aDragShellRef->DoInitNew(NULL);
            }
            //---------------------------------------------------------

            ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
            SdrModel* pModel = pDrawView->GetMarkedObjModel();
            ScDrawLayer::SetGlobalDrawPersist(NULL);

            ScViewData* pViewData = pView->GetViewData();
            ScDocShell* pDocSh = pViewData->GetDocShell();

            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScDrawTransferObj ctor

            ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
            uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

            SfxObjectShellRef aPersistRef( aDragShellRef );
            pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
            pTransferObj->SetDragSource( pDrawView );       // copies selection

            pDrawData = pTransferObj;
            pDrawData->acquire();       // keep ref count up - released in ForgetView
        }
    }
    OSL_ENSURE( pDrawData, "can't create DrawData" );
}

ScTransferObj* ScSelectionTransferObj::GetCellData()
{
    if ( !pCellData && ( eMode == SC_SELTRANS_CELL || eMode == SC_SELTRANS_CELLS ) )
        CreateCellData();
    return pCellData;
}

ScDrawTransferObj* ScSelectionTransferObj::GetDrawData()
{
    if ( !pDrawData && ( eMode == SC_SELTRANS_DRAW_BITMAP || eMode == SC_SELTRANS_DRAW_GRAPHIC ||
                         eMode == SC_SELTRANS_DRAW_BOOKMARK || eMode == SC_SELTRANS_DRAW_OLE ||
                         eMode == SC_SELTRANS_DRAW_OTHER ) )
        CreateDrawData();
    return pDrawData;
}

sal_Bool ScSelectionTransferObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_Bool bOK = false;

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
        uno::Any aAny = aHelper.GetAny( rFlavor );
        bOK = SetAny( aAny, rFlavor );
    }

    return bOK;
}

void ScSelectionTransferObj::ObjectReleased()
{
    //  called when another selection is set from outside

    ForgetView();

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetSelectionTransfer() == this )
        pScMod->SetSelectionTransfer( NULL );

    TransferableHelper::ObjectReleased();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
