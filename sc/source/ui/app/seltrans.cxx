/*************************************************************************
 *
 *  $RCSfile: seltrans.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:26:38 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
#include "dbfunc.hxx"   // for CopyToClip
#include "docsh.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

BOOL lcl_IsURLButton( SdrObject* pObject )
{
    BOOL bRet = FALSE;

    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObject);
    if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
       {
        uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
        DBG_ASSERT( xControlModel.is(), "uno control without model" );
        if ( xControlModel.is() )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

            rtl::OUString sPropButtonType = rtl::OUString::createFromAscii( "ButtonType" );
            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                    bRet = TRUE;
            }
         }
    }

    return bRet;
}

// static

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
            ULONG nMarkCount = rMarkList.GetMarkCount();
            if ( nMarkCount )
            {
                if ( nMarkCount == 1 )
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                    UINT16 nSdrObjKind = pObj->GetObjIdentifier();

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
            if ( ( rMark.IsMarked() || rMark.IsMultiMarked() ) && pViewData->GetSimpleArea( aRange ) )
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

    DBG_ASSERT( !pView, "ScSelectionTransferObj dtor: ForgetView not called" );
}

BOOL ScSelectionTransferObj::StillValid()
{
    //! check if view still has same cell selection
    //! (but return FALSE if data has changed inbetween)
    return FALSE;
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
            AddFormat( SOT_FORMAT_BITMAP );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            break;

        case SC_SELTRANS_DRAW_GRAPHIC:
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
            AddFormat( SOT_FORMATSTR_ID_SVXB );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
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
            AddFormat( SOT_FORMAT_BITMAP );
            AddFormat( SOT_FORMAT_GDIMETAFILE );
            break;
    }
}

void ScSelectionTransferObj::CreateCellData()
{
    DBG_ASSERT( !pCellData, "CreateCellData twice" );
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
            // bApi = TRUE -> no error mesages
            // #i18364# bStopEdit = FALSE -> don't end edit mode
            // (this may be called from pasting into the edit line)
            BOOL bCopied = pViewData->GetView()->CopyToClip( pClipDoc, FALSE, TRUE, TRUE, FALSE );

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

                SvEmbeddedObjectRef aPersistRef( aDragShellRef );
                pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive

                pTransferObj->SetDragSource( pDocSh, aNewMark );

                pCellData = pTransferObj;
                pCellData->acquire();       // keep ref count up - released in ForgetView
            }
            else
                delete pClipDoc;
        }
    }
    DBG_ASSERT( pCellData, "can't create CellData" );
}

//! make static member of ScDrawView
extern void lcl_CheckOle( const SdrMarkList& rMarkList, BOOL& rAnyOle, BOOL& rOneOle );

void ScSelectionTransferObj::CreateDrawData()
{
    DBG_ASSERT( !pDrawData, "CreateDrawData twice" );
    if ( pView )
    {
        //  similar to ScDrawView::BeginDrag

        ScDrawView* pDrawView = pView->GetScDrawView();
        if ( pDrawView )
        {
            BOOL bAnyOle, bOneOle;
            const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
            lcl_CheckOle( rMarkList, bAnyOle, bOneOle );

            //---------------------------------------------------------
            ScDocShellRef aDragShellRef;
            if (bAnyOle)
            {
                aDragShellRef = new ScDocShell;     // ohne Ref lebt die DocShell nicht !!!
                aDragShellRef->DoInitNew(NULL);
            }
            //---------------------------------------------------------

            ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
            SdrModel* pModel = pDrawView->GetAllMarkedModel();
            ScDrawLayer::SetGlobalDrawPersist(NULL);

            ScViewData* pViewData = pView->GetViewData();
            ScDocShell* pDocSh = pViewData->GetDocShell();

            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScDrawTransferObj ctor

            ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
            uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

            SvEmbeddedObjectRef aPersistRef( aDragShellRef );
            pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
            pTransferObj->SetDragSource( pDrawView );       // copies selection

            pDrawData = pTransferObj;
            pDrawData->acquire();       // keep ref count up - released in ForgetView
        }
    }
    DBG_ASSERT( pDrawData, "can't create DrawData" );
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
    sal_Bool bOK = sal_False;

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


