/*************************************************************************
 *
 *  $RCSfile: sdview3.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-08 12:59:27 $
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

#pragma hdrstop


#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _EDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SVX_XEXCH_HXX
#include <svx/xexch.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#define ITEMID_COLOR                    EE_CHAR_COLOR
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDETC_HXX //autogen
#include <svx/svdetc.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _EDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _TRANSBND_HXX
#include <so3/transbnd.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
#ifndef _E3D_UNDO_HXX
#include <svx/e3dundo.hxx>
#endif
#include <svx/dbexch.hrc>

#include "docshell.hxx"
#include "fupoor.hxx"
#include "sdwindow.hxx"
#include "sdview.hxx"
#include "sdxfer.hxx"
#include "sdpage.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "slidvish.hxx"
#include "strmname.h"

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

#define CHECK_FORMAT_TRANS( _def_Type ) ( ( nFormat == (_def_Type) || !nFormat ) && aDataHelper.HasFormat( _def_Type ) )

/*************************************************************************
|*
|* Paste
|*
\************************************************************************/

BOOL SdView::InsertData( const Reference< XTransferable >& rxTransferable,
                         const Point& rPos, sal_Int8& rDnDAction, BOOL bDrag,
                         ULONG nFormat, USHORT nPage, USHORT nLayer )
{
    aDropPos = rPos;
    nAction = rDnDAction;
    bIsDropAllowed = FALSE;

    TransferableDataHelper  aDataHelper( rxTransferable );
    SdrObject*              pPickObj = NULL;
    SdPage*                 pPage = NULL;
    SdWindow*               pWin = pViewSh->GetActiveWindow();
    ImageMap*               pImageMap = NULL;
    BOOL                    bMtf = FALSE;
    BOOL                    bReturn = FALSE;
    BOOL                    bLink = ( ( nAction & DND_ACTION_LINK ) != 0 );
    BOOL                    bCopy = ( ( ( nAction & DND_ACTION_COPY ) != 0 ) || bLink );
    ULONG                   nEditEngineFormat = EditEngine::RegisterClipboardFormatName();
    ULONG                   nPasteOptions = SDRINSERT_SETDEFLAYER;

    if( pViewSh->ISA( SdSlideViewShell ) || ( pViewSh && pViewSh->GetIPClient() && pViewSh->GetIPClient()->IsInPlaceActive() ) )
        nPasteOptions |= SDRINSERT_DONTMARK;

    if( bDrag )
    {
        SdrPageView* pPV = NULL;
        PickObj( rPos, pPickObj, pPV );
    }

    if( nPage != SDRPAGE_NOTFOUND )
        pPage = (SdPage*) pDoc->GetPage( nPage );

    // !!!Clipboard
//  SdTransferable* pOwnData = NULL;
    SdTransferable* pOwnData = ( SD_MOD()->pTransferClip == (SdTransferable*) rxTransferable.get() ) ? SD_MOD()->pTransferClip : NULL;

    if( !pOwnData )
    {
        // ImageMap?
        if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVIM ) )
        {
            SotStorageStreamRef xStm;

            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVIM, xStm ) )
            {
                pImageMap = new ImageMap;
                *xStm >> *pImageMap;
            }
        }
    }

    if( pOwnData && !nFormat )
    {
        // own data
        const SdView* pSourceView = pOwnData->GetView();

        if( pSourceView )
        {
            if( pSourceView == this )
            {
                // same view
                if( nLayer != SDRLAYER_NOTFOUND )
                {
                    // drop on layer tab bar
                    SdrLayerAdmin&  rLayerAdmin = pDoc->GetLayerAdmin();
                    SdrLayer*       pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                    SdrPageView*    pPV = GetPageViewPvNum( 0 );
                    String          aLayer( pLayer->GetName() );

                    if( !pPV->IsLayerLocked( aLayer ) )
                    {
                        pOwnData->SetInternalMove( TRUE );
                        aMark.ForceSort();

                        for( ULONG nM = 0; nM < aMark.GetMarkCount(); nM++ )
                        {
                            SdrMark*    pM = aMark.GetMark( nM );
                            SdrObject*  pO = pM->GetObj();

                            if( pO )
                                pO->SetLayer( (SdrLayerID) nLayer );
                        }

                        bReturn = TRUE;
                    }
                }
                else
                {
                    SdrPageView*    pPV = GetPageViewPvNum( 0 );
                    BOOL            bDropOnTabBar = TRUE;

                    if( !pPage && pPV->GetPage()->GetPageNum() != nDragSrcPgNum )
                    {
                        pPage = (SdPage*) pPV->GetPage();
                        bDropOnTabBar = FALSE;
                    }

                    if( pPage )
                    {
                        // drop on other page
                        String aActiveLayer( GetActiveLayer() );

                        if( !pPV->IsLayerLocked( aActiveLayer ) )
                        {
                            if( !IsPresObjSelected() )
                            {
                                SdrMarkList* pMarkList;

                                if( nDragSrcPgNum != SDRPAGE_NOTFOUND && nDragSrcPgNum != pPV->GetPage()->GetPageNum() )
                                {
                                    // source != destination => saved mark list is used
                                    pMarkList = pDragSrcMarkList;
                                }
                                else
                                    // actual mark list is used
                                    pMarkList = new SdrMarkList( aMark );

                                pMarkList->ForceSort();

                                for( ULONG nM = 0; nM < pMarkList->GetMarkCount(); nM++ )
                                {
                                    SdrMark*    pM = pMarkList->GetMark( nM );
                                    SdrObject*  pObj = pM->GetObj()->Clone();

                                    if( pObj )
                                    {
                                        if( !bDropOnTabBar )
                                        {
                                            Rectangle   aRect( pObj->GetLogicRect() );
                                            Point       aPos( aRect.TopLeft() );
                                            Size        aSize( aRect.GetSize() );
                                            Size        aVector( aDropPos.X() - pOwnData->GetStartPos().X(),
                                                                 aDropPos.Y() - pOwnData->GetStartPos().Y() );

                                            aPos.X() += aVector.Width();
                                            aPos.Y() += aVector.Height();
                                            aRect.SetPos( aPos );
                                            pObj->SetLogicRect( aRect );
                                        }

                                        pPage->InsertObject(pObj);

                                        BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                                        AddUndo( new SdrUndoNewObj( *pObj ) );
                                        EndUndo();
                                    }
                                }

                                if( pMarkList != pDragSrcMarkList )
                                    delete pMarkList;

                                bReturn = TRUE;
                            }
                            else
                            {
                                aDropErrorTimer.Start();
                                bReturn = FALSE;
                            }
                        }
                    }
                    else
                    {
                        pOwnData->SetInternalMove( TRUE );
                        MoveAllMarked( Size( aDropPos.X() - pOwnData->GetStartPos().X(),
                                             aDropPos.Y() - pOwnData->GetStartPos().Y() ), bCopy );
                        bReturn = TRUE;
                    }
                }
            }
            else
            {
                // different views
                if( !pSourceView->IsPresObjSelected() )
                {
                    // model is owned by from AllocModel() created DocShell
                    SdDrawDocument* pSourceDoc = (SdDrawDocument*) pSourceView->GetModel();
                    pSourceDoc->CreatingDataObj( TRUE );
                    SdDrawDocument* pModel = (SdDrawDocument*) pSourceView->GetAllMarkedModel();
                    bReturn = Paste( *pModel, aDropPos, pPage, nPasteOptions );

                    if( bLink )
                    {
                        SdrObject*      pObj = NULL;
                        SdPage*         pPage = pModel->GetSdPage( 0, PK_STANDARD );
                        SdrObjListIter  aIter( *pPage, IM_DEEPWITHGROUPS );
                        String          aDocName( pSourceDoc->GetDocSh()->GetMedium()->GetName() );

                        while( aIter.IsMore() )
                        {
                            pObj = aIter.Next();

                            String aName( pObj->GetName() );

                            if( aName.Len() )
                            {
                                SdrObject* pNewObj = pDoc->GetObj( aName );

                                if( pNewObj )
                                {
                                    if( pNewObj->ISA( SdrObjGroup ) )
                                        ( (SdrObjGroup*) pNewObj )->SetGroupLink( aDocName, aName );
                                }
                            }
                        }
                    }

                    if( !pPage )
                        pPage = (SdPage*) GetPageViewPvNum( 0 )->GetPage();

                    String aLayout( pPage->GetLayoutName() );
                    aLayout.Erase( aLayout.SearchAscii( SD_LT_SEPARATOR ) );
                    pPage->SetPresentationLayout( aLayout, FALSE, FALSE );
                    pSourceDoc->CreatingDataObj( FALSE );
                }
                else
                {
                    aDropErrorTimer.Start();
                    bReturn = FALSE;
                }
            }
        }
        else
        {
            if( !pPage )
                pPage = (SdPage*) GetPageViewPvNum( 0 )->GetPage();

            // internal paste
            SdrModel*   pWorkModel = (SdrModel*) pOwnData->GetWorkDocument();
            SdrPage*    pWorkPage = pWorkModel->GetPage( 0 ); pPage->SetRectsDirty();
            Size        aSize( pWorkPage->GetAllObjBoundRect().GetSize() );

            aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
            aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );

            bReturn = Paste( *pWorkModel, aDropPos, pPage, nPasteOptions );

            String aLayout( pPage->GetLayoutName() );
            aLayout.Erase( aLayout.SearchAscii( SD_LT_SEPARATOR ) );
            pPage->SetPresentationLayout( aLayout, FALSE, FALSE );
       }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_DRAWING ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
        {
            BOOL bChanged = FALSE;

            xStm->SetVersion( SOFFICE_FILEFORMAT_50 );
            FmFormModel* pModel = new FmFormModel( SvtPathOptions().GetPalettePath(), NULL, pDocSh );
            xStm->Seek( 0 );
            pModel->SetStreamingSdrModel( TRUE );
            pModel->GetItemPool().Load( *xStm );
            *xStm >> *pModel;
            pModel->SetStreamingSdrModel( FALSE );

            if( pModel->GetPage( 0 )->GetObjCount() == 1 )
            {
                // only one object
                SdrObject*      pObj = pModel->GetPage( 0 )->GetObj( 0 );
                SdrObject*      pPickObj = NULL;
                SdrPageView*    pPV = NULL;
                BOOL            bPickObj = PickObj( rPos, pPickObj, pPV );

                if( ( nAction & DND_ACTION_MOVE ) && pPickObj && pObj )
                {
                    // replace object
                    SdrObject*  pNewObj = pObj->Clone();
                    Rectangle   aPickObjRect( pPickObj->GetBoundRect() );
                    Size        aPickObjSize( aPickObjRect.GetSize() );
                    Point       aVec( aPickObjRect.TopLeft() );
                    Rectangle   aObjRect( pNewObj->GetBoundRect() );
                    Size        aObjSize( aObjRect.GetSize() );

                    Fraction aScaleWidth( aPickObjSize.Width(), aObjSize.Width() );
                    Fraction aScaleHeight( aPickObjSize.Height(), aObjSize.Height() );
                    pNewObj->NbcResize( aObjRect.TopLeft(), aScaleWidth, aScaleHeight );

                    aVec -= aObjRect.TopLeft();
                    pNewObj->NbcMove( Size( aVec.X(), aVec.Y() ) );

                    BegUndo( String( SdResId(STR_UNDO_DRAGDROP ) ) );
                    pNewObj->NbcSetLayer( pPickObj->GetLayer() );
                    SdrPage* pPage = GetPageViewPvNum( 0 )->GetPage();
                    pPage->InsertObject( pNewObj );
                    AddUndo( new SdrUndoNewObj( *pNewObj ) );
                    AddUndo( new SdrUndoDelObj( *pPickObj ) );
                    pPage->RemoveObject( pPickObj->GetOrdNum() );
                    EndUndo();
                    bChanged = TRUE;
                    nAction = DND_ACTION_COPY;
                }
                else if( ( nAction & DND_ACTION_LINK ) && pPickObj && pObj && !pPickObj->ISA( SdrGrafObj ) && !pPickObj->ISA( SdrOle2Obj ) )
                {
                    SfxItemSet aSet( pDoc->GetPool() );

                    // set new attributes to object
                    BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
                    AddUndo( new SdrUndoAttrObj( *pPickObj ) );
                    aSet.Put( pObj->GetItemSet() );

                    // Eckenradius soll nicht uebernommen werden.
                    // In der Gallery stehen Farbverlauefe (Rechtecke)
                    // welche den Eckenradius == 0 haben. Dieser soll
                    // nicht auf das Objekt uebertragen werden.
                    aSet.ClearItem( SDRATTR_ECKENRADIUS );

                    pPickObj->SetItemSetAndBroadcast( aSet );

                    if( pPickObj->ISA( E3dObject ) && pObj->ISA( E3dObject ) )
                    {
                        // Zusaetzlich 3D Attribute handeln
                        SfxItemSet aNewSet( pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );
                        SfxItemSet aOldSet( pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );

                        aOldSet.Put(pPickObj->GetItemSet());
                        aNewSet.Put( pObj->GetItemSet() );

                        AddUndo( new E3dAttributesUndoAction( *pDoc, this, (E3dObject*) pPickObj, aNewSet, aOldSet, FALSE ) );
                        pPickObj->SetItemSetAndBroadcast( aNewSet );
                    }

                    EndUndo();
                    bChanged = TRUE;
                }
            }

            if( !bChanged )
            {
                // insert object

                SdrPage* pInsertPage = pModel->GetPage( 0 );

                pInsertPage->SetRectsDirty();

                if( pOwnData )
                {
                    Size aSize( pInsertPage->GetAllObjBoundRect().GetSize() );

                    aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                    aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                }

                bReturn = Paste( *pModel, aDropPos, pPage, nPasteOptions );
            }
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SVXB ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ) )
        {
            Graphic aGraphic;

            *xStm >> aGraphic;

            if( pOwnData )
            {
                SdrPage* pWorkPage = ( (SdrModel*) pOwnData->GetWorkDocument() )->GetPage( 0 );

                pWorkPage->SetRectsDirty();

                Size aSize( pWorkPage->GetSize() );

                aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            InsertGraphic( aGraphic, nAction, aDropPos, NULL, pImageMap );
            bReturn = TRUE;
        }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE ) )
    {
        String aString;

        if( aDataHelper.GetString( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE, aString ) )
        {
            SdrObject* pObj = CreateFieldControl( aString );

            if( pObj )
            {
                Rectangle   aRect( pObj->GetLogicRect() );
                Size        aSize( aRect.GetSize() );

                aDropPos.X() -= ( aSize.Width() >> 1 );
                aDropPos.Y() -= ( aSize.Height() >> 1 );

                aRect.SetPos( aDropPos );
                pObj->SetLogicRect( aRect );
                InsertObject( pObj, *GetPageViewPvNum( 0 ), SDRINSERT_SETDEFLAYER );
                bReturn = TRUE;
            }
        }
    }
    else if( !bLink &&
             ( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBED_SOURCE ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ) &&
               aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
    {
        SotStorageStreamRef             xStm;
        TransferableObjectDescriptor    aObjDesc;

        if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) &&
            aDataHelper.GetSotStorageStream( nFormat, xStm ) )
        {
            DocumentType eDocType = pDoc->GetDocumentType();
            SvStorageRef xStore( new SvStorage( *xStm ) );

            if( pDoc->GetDocSh() && ( pDoc->GetDocSh()->GetClassName() == aObjDesc.maClassName ) )
            {
                SdDrawDocShellRef xDocShRef( new SdDrawDocShell( SFX_CREATE_MODE_EMBEDDED, TRUE, eDocType ) );

                if( xDocShRef->DoLoad( xStore ) )
                {
                    SdrModel*   pModel = xDocShRef->GetDoc();
                    SdPage*     pPage = (SdPage*) pModel->GetPage( 0 );

                    pPage->SetRectsDirty();

                    if( pOwnData )
                    {
                        Size aSize( pPage->GetAllObjBoundRect().GetSize() );

                        aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                        aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                    }

                    bReturn = Paste( *pModel, aDropPos, pPage, nPasteOptions );

                    if( !pPage )
                        pPage = (SdPage*) GetPageViewPvNum( 0 )->GetPage();

                    String aLayout(pPage->GetLayoutName());
                    aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
                    pPage->SetPresentationLayout( aLayout, FALSE, FALSE );
                }

                xDocShRef->DoClose();
                xDocShRef.Clear();

            }
            else
            {
                SvInPlaceObjectRef xIPObj = &( (SvFactory*) SvInPlaceObject::ClassFactory() )->CreateAndLoad( xStore );

                if( xIPObj.Is() )
                {
                    String aName( pDocSh->InsertObject( xIPObj, String() )->GetObjName() );

                    if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                        xIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aObjDesc.maSize, MAP_100TH_MM, xIPObj->GetMapUnit() ) );

                    Size aSize( xIPObj->GetVisArea().GetSize() );

                    if( !aSize.Width() || !aSize.Height() )
                    {
                        aSize.Width()  = 14100;
                        aSize.Height() = 10000;
                        xIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, xIPObj->GetMapUnit() ) );
                        aSize = xIPObj->GetVisArea().GetSize();
                    }

                    aSize = OutputDevice::LogicToLogic( aSize, xIPObj->GetMapUnit(), MAP_100TH_MM );
                    Size aMaxSize( pDoc->GetMaxObjSize() );

                    aDropPos.X() -= Min( aSize.Width(), aMaxSize.Width() ) >> 1;
                    aDropPos.Y() -= Min( aSize.Height(), aMaxSize.Height() ) >> 1;

                    Rectangle       aRect( aDropPos, aSize );
                    SdrOle2Obj*     pObj = new SdrOle2Obj( xIPObj, aName, aRect );
                    SdrPageView*    pPV = GetPageViewPvNum( 0 );
                    ULONG           nOptions = SDRINSERT_SETDEFLAYER;

                    if( pViewSh && pViewSh->GetIPClient() && pViewSh->GetIPClient()->IsInPlaceActive() )
                        nOptions |= SDRINSERT_DONTMARK;

                    InsertObject( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                    bReturn = TRUE;
                }
            }
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_GDIMETAFILE ) )
    {
        GDIMetaFile aMtf;

        if( aDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
        {
            Point aInsertPos( rPos );

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                SdrModel*   pWorkModel = (SdrModel*) pOwnData->GetWorkDocument();
                SdrPage*    pPage = pWorkModel->GetPage( 0 );

                pPage->SetRectsDirty();

                Size aSize( pPage->GetAllObjBoundRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            InsertGraphic( aMtf, nAction, aInsertPos, NULL, pImageMap );
            bReturn = TRUE;
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_BITMAP ) )
    {
        Bitmap aBmp;

        if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ) )
        {
            Point aInsertPos( rPos );

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                SdrModel*   pWorkModel = (SdrModel*) pOwnData->GetWorkDocument();
                SdrPage*    pPage = pWorkModel->GetPage( 0 );

                pPage->SetRectsDirty();

                Size aSize( pPage->GetAllObjBoundRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            InsertGraphic( aBmp, nAction, aInsertPos, NULL, pImageMap );
            bReturn = TRUE;
        }
    }
    else if( pPickObj && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_XFA ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) )
        {
            XFillExchangeData aFillData;

            *xStm >> aFillData;

            BegUndo( String( SdResId( STR_UNDO_DRAGDROP ) ) );
            AddUndo( new SdrUndoAttrObj( *pPickObj ) );
            EndUndo();

            XFillAttrSetItem*   pSetItem = aFillData.GetXFillAttrSetItem();
            SfxItemSet          rSet = pSetItem->GetItemSet();
            XFillStyle          eFill= ( (XFillStyleItem&) rSet.Get( XATTR_FILLSTYLE ) ).GetValue();

            if( eFill == XFILL_SOLID || eFill == XFILL_NONE )
            {
                const XFillColorItem&   rColItem = (XFillColorItem&) rSet.Get( XATTR_FILLCOLOR );
                Color                   aColor( rColItem.GetValue() );
                String                  aName( rColItem.GetName() );
                SfxItemSet              aSet( pDoc->GetPool() );
                BOOL                    bClosed = pPickObj->IsClosedObj();
                SdWindow*               pWin = pViewSh->GetActiveWindow();
                USHORT                  nHitLog = (USHORT) pWin->PixelToLogic( Size( HITPIX, 0 ) ).Width();
                const long              n2HitLog = nHitLog << 1;
                Point                   aHitPosR( rPos );
                Point                   aHitPosL( rPos );
                Point                   aHitPosT( rPos );
                Point                   aHitPosB( rPos );
                const SetOfByte*        pVisiLayer = &GetPageViewPvNum(0)->GetVisibleLayers();

                aHitPosR.X() += n2HitLog;
                aHitPosL.X() -= n2HitLog;
                aHitPosT.Y() += n2HitLog;
                aHitPosB.Y() -= n2HitLog;

                if( bClosed                                          &&
                    pPickObj->IsHit( aHitPosR, nHitLog, pVisiLayer ) &&
                    pPickObj->IsHit( aHitPosL, nHitLog, pVisiLayer ) &&
                    pPickObj->IsHit( aHitPosT, nHitLog, pVisiLayer ) &&
                    pPickObj->IsHit( aHitPosB, nHitLog, pVisiLayer ) )
                {
                    // area fill
                    if(eFill == XFILL_SOLID )
                        aSet.Put(XFillColorItem(aName, aColor));

                    aSet.Put( XFillStyleItem( eFill ) );
                }
                else
                    aSet.Put( XLineColorItem( aName, aColor ) );

                // Textfarbe hinzufuegen
                pPickObj->SetItemSetAndBroadcast( aSet );
            }
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_HTML ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_HTML, xStm ) )
        {
            xStm->Seek( 0 );
            bReturn = SdrView::Paste( *xStm, EE_FORMAT_HTML, aDropPos, pPage, nPasteOptions );
        }
    }
    else if( !bLink && ( CHECK_FORMAT_TRANS( FORMAT_RTF ) || CHECK_FORMAT_TRANS( nEditEngineFormat ) ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( nFormat, xStm ) )
        {
            EETextFormat nFmt = EE_FORMAT_RTF;

            xStm->Seek( 0 );

            if( CHECK_FORMAT_TRANS( nEditEngineFormat ) )
                nFmt = EE_FORMAT_BIN;

            OutlinerView* pOLV = GetTextEditOutlinerView();

            if( pOLV )
            {
                Rectangle   aRect( pOLV->GetOutputArea() );
                   Point        aPos( pOLV->GetWindow()->PixelToLogic( aDropPos ) );

                if( aRect.IsInside( aPos ) )
                {
                    pOLV->Read( *xStm, nFmt, FALSE, pDocSh->GetHeaderAttributes() );
                    bReturn = TRUE;
                }
            }

            if( !bReturn )
                bReturn = SdrView::Paste( *xStm, nFmt, aDropPos, pPage, nPasteOptions );
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( FORMAT_STRING ) )
    {
        // Falls auch ein URL-Format gedropt wurde, nehmen
        // wir natuerlich dieses und kehren somit sofort zurueck,
        // da beim Aufrufer dieser URL eingefuegt wird
        String          aTmpStr;
        INetBookmark    aINetBookmark( aTmpStr, aTmpStr );

        if( !INetBookmark::DragServerHasFormat( 0 ) || !aINetBookmark.PasteDragServer( 0 ) )
        {
            String aStr;

            if( aDataHelper.GetString( FORMAT_STRING, aStr ) )
            {
                OutlinerView* pOLV = GetTextEditOutlinerView();

                if( pOLV )
                {
                    pOLV->InsertText( aStr );
                    bReturn = TRUE;
                }
                else
                    bReturn = SdrView::Paste( aStr, aDropPos, pPage, nPasteOptions );
            }
        }
    }
    else if( CHECK_FORMAT_TRANS( FORMAT_FILE ) )
    {
        if( aDataHelper.GetString( FORMAT_FILE, aDropFile ) )
            aDropInsertFileTimer.Start();

        bReturn = TRUE;
    }

    MarkListHasChanged();
    bIsDropAllowed = TRUE;
    rDnDAction = nAction;
    delete pImageMap;

    return bReturn;
}
