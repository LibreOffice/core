/*************************************************************************
 *
 *  $RCSfile: sdview3.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:01:44 $
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

#include "View.hxx"

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _FILELIST_HXX
#include <sot/filelist.hxx>
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
#ifndef SVX_UNOMODEL_HXX
#include <svx/unomodel.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#include "DrawDocShell.hxx"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdxfer.hxx"
#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#include "strmname.h"
#include "unomodel.hxx"
#include "ViewClipboard.hxx"

#include <tools/stream.hxx>
#include <vcl/cvtgrf.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

namespace sd {

#define CHECK_FORMAT_TRANS( _def_Type ) ( ( nFormat == (_def_Type) || !nFormat ) && aDataHelper.HasFormat( _def_Type ) )

/*************************************************************************
|*
|* Paste
|*
\************************************************************************/

// #83525#
struct ImpRememberOrigAndClone
{
    SdrObject*      pOrig;
    SdrObject*      pClone;
};

SdrObject* ImpGetClone(Container& aConnectorContainer, SdrObject* pConnObj)
{
    for(sal_uInt32 a(0); a < aConnectorContainer.Count(); a++)
    {
        if(pConnObj == ((ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a))->pOrig)
            return ((ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a))->pClone;
    }
    return 0L;
}

// #90129# restrict movement to WorkArea
void ImpCheckInsertPos(Point& rPos, const Size& rSize, const Rectangle& rWorkArea)
{
    if(!rWorkArea.IsEmpty())
    {
        Rectangle aMarkRect(Point(rPos.X() - (rSize.Width() / 2), rPos.Y() - (rSize.Height() / 2)), rSize);

        if(!aMarkRect.IsInside(rWorkArea))
        {
            if(aMarkRect.Left() < rWorkArea.Left())
            {
                rPos.X() += rWorkArea.Left() - aMarkRect.Left();
            }

            if(aMarkRect.Right() > rWorkArea.Right())
            {
                rPos.X() -= aMarkRect.Right() - rWorkArea.Right();
            }

            if(aMarkRect.Top() < rWorkArea.Top())
            {
                rPos.Y() += rWorkArea.Top() - aMarkRect.Top();
            }

            if(aMarkRect.Bottom() > rWorkArea.Bottom())
            {
                rPos.Y() -= aMarkRect.Bottom() - rWorkArea.Bottom();
            }
        }
    }
}

BOOL View::InsertData( const TransferableDataHelper& rDataHelper,
                         const Point& rPos, sal_Int8& rDnDAction, BOOL bDrag,
                         ULONG nFormat, USHORT nPage, USHORT nLayer )
{
    aDropPos = rPos;
    nAction = rDnDAction;
    bIsDropAllowed = FALSE;

    TransferableDataHelper  aDataHelper( rDataHelper );
    SdrObject*              pPickObj = NULL;
    SdPage*                 pPage = NULL;
    ::sd::Window* pWin = pViewSh->GetActiveWindow();
    ImageMap*               pImageMap = NULL;
    BOOL                    bMtf = FALSE;
    BOOL                    bReturn = FALSE;
    BOOL                    bLink = ( ( nAction & DND_ACTION_LINK ) != 0 );
    BOOL                    bCopy = ( ( ( nAction & DND_ACTION_COPY ) != 0 ) || bLink );
    ULONG                   nPasteOptions = SDRINSERT_SETDEFLAYER;

    if (pViewSh != NULL)
    {
        OSL_ASSERT (pViewSh->GetViewShell()!=NULL);
        SfxInPlaceClient* pIpClient = pViewSh->GetViewShell()->GetIPClient();
        if( pViewSh->ISA(SlideViewShell)
            || (pIpClient!=NULL && pIpClient->IsInPlaceActive()))
        nPasteOptions |= SDRINSERT_DONTMARK;
    }

    if( bDrag )
    {
        SdrPageView* pPV = NULL;
        PickObj( rPos, pPickObj, pPV );
    }

    if( nPage != SDRPAGE_NOTFOUND )
        pPage = (SdPage*) pDoc->GetPage( nPage );

    SdTransferable* pOwnData = NULL;
    SdTransferable* pImplementation = SdTransferable::getImplementation( aDataHelper.GetTransferable() );

    // try to get own transfer data
    if( pImplementation )
    {
        if( SD_MOD()->pTransferClip == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferClip;
        else if( SD_MOD()->pTransferDrag == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferDrag;
        else if( SD_MOD()->pTransferSelection == (SdTransferable*) pImplementation )
            pOwnData = SD_MOD()->pTransferSelection;
    }

    // ImageMap?
    if( !pOwnData && aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVIM ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVIM, xStm ) )
        {
            pImageMap = new ImageMap;
            *xStm >> *pImageMap;
        }
    }

    if( pOwnData && !nFormat )
    {
        const View* pSourceView = pOwnData->GetView();


        if( pOwnData->GetDocShell() && pOwnData->IsPageTransferable() && ISA( View ) )
        {
            mpClipboard->HandlePageDrop (*pOwnData);
        }
        else if( pSourceView )
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
                        SortMarkedObjects();

                        for( ULONG nM = 0; nM < GetMarkedObjectCount(); nM++ )
                        {
                            SdrMark*    pM = GetSdrMarkByIndex( nM );
                            SdrObject*  pO = pM->GetObj();

                            if( pO )
                            {
                                // #i11702#
                                BegUndo(String(SdResId(STR_MODIFYLAYER)));
                                AddUndo(new SdrUndoObjectLayerChange(*pO, pO->GetLayer(), (SdrLayerID)nLayer));
                                EndUndo();

                                pO->SetLayer( (SdrLayerID) nLayer );
                            }
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
                                    pMarkList = new SdrMarkList( GetMarkedObjectList());

                                pMarkList->ForceSort();

                                // #83525# stuff to remember originals and clones
                                Container   aConnectorContainer(0);
                                sal_uInt32  a, nConnectorCount(0L);
                                Point       aCurPos;

                                // calculate real position of current
                                // source objects, if necessary (#103207)
                                if( pOwnData == SD_MOD()->pTransferSelection )
                                {
                                    Rectangle aCurBoundRect;

                                    if( pMarkList->TakeBoundRect( pPV, aCurBoundRect ) )
                                        aCurPos = aCurBoundRect.TopLeft();
                                    else
                                        aCurPos = pOwnData->GetStartPos();
                                }
                                else
                                    aCurPos = pOwnData->GetStartPos();

                                const Size aVector( aDropPos.X() - aCurPos.X(), aDropPos.Y() - aCurPos.Y() );

                                for(a = 0; a < pMarkList->GetMarkCount(); a++)
                                {
                                    SdrMark* pM = pMarkList->GetMark(a);
                                    SdrObject* pObj = pM->GetObj()->Clone();

                                    if(pObj)
                                    {
                                        if(!bDropOnTabBar)
                                        {
                                            // #83525# do a NbcMove(...) instead of setting SnapRects here
                                            pObj->NbcMove(aVector);
                                        }

                                        pPage->InsertObject(pObj);

                                        BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                                        AddUndo(new SdrUndoNewObj(*pObj));
                                        EndUndo();

                                        // #83525#
                                        ImpRememberOrigAndClone* pRem = new ImpRememberOrigAndClone;
                                        pRem->pOrig = pM->GetObj();
                                        pRem->pClone = pObj;
                                        aConnectorContainer.Insert(pRem, CONTAINER_APPEND);

                                        if(pObj->ISA(SdrEdgeObj))
                                            nConnectorCount++;
                                    }
                                }

                                // #83525# try to re-establish connections at clones
                                if(nConnectorCount)
                                {
                                    for(a = 0; a < aConnectorContainer.Count(); a++)
                                    {
                                        ImpRememberOrigAndClone* pRem = (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);

                                        if(pRem->pClone->ISA(SdrEdgeObj))
                                        {
                                            SdrEdgeObj* pOrigEdge = (SdrEdgeObj*)pRem->pOrig;
                                            SdrEdgeObj* pCloneEdge = (SdrEdgeObj*)pRem->pClone;

                                            // test first connection
                                            SdrObjConnection& rConn0 = pOrigEdge->GetConnection(FALSE);
                                            SdrObject* pConnObj = rConn0.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(FALSE, pConnClone);
                                                    pCloneEdge->GetConnection(FALSE).SetConnectorId(rConn0.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn0.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.X() += aVector.A();
                                                            aPosition.Y() += aVector.B();
                                                            pCloneEdge->SetTailPoint(FALSE, aPosition);
                                                        }
                                                    }
                                                }
                                            }

                                            // test second connection
                                            SdrObjConnection& rConn1 = pOrigEdge->GetConnection(TRUE);
                                            pConnObj = rConn1.GetObject();
                                            if(pConnObj)
                                            {
                                                SdrObject* pConnClone = ImpGetClone(aConnectorContainer, pConnObj);
                                                if(pConnClone)
                                                {
                                                    // if dest obj was cloned, too, re-establish connection
                                                    pCloneEdge->ConnectToNode(TRUE, pConnClone);
                                                    pCloneEdge->GetConnection(TRUE).SetConnectorId(rConn1.GetConnectorId());
                                                }
                                                else
                                                {
                                                    // set position of connection point of original connected object
                                                    const SdrGluePointList* pGlueList = pConnObj->GetGluePointList();
                                                    if(pGlueList)
                                                    {
                                                        sal_uInt16 nInd = pGlueList->FindGluePoint(rConn1.GetConnectorId());

                                                        if(SDRGLUEPOINT_NOTFOUND != nInd)
                                                        {
                                                            const SdrGluePoint& rGluePoint = (*pGlueList)[nInd];
                                                            Point aPosition = rGluePoint.GetAbsolutePos(*pConnObj);
                                                            aPosition.X() += aVector.A();
                                                            aPosition.Y() += aVector.B();
                                                            pCloneEdge->SetTailPoint(TRUE, aPosition);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // #83525# cleanup remember classes
                                for(a = 0; a < aConnectorContainer.Count(); a++)
                                    delete (ImpRememberOrigAndClone*)aConnectorContainer.GetObject(a);

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
                    pSourceDoc->CreatingDataObj( pOwnData );
                    SdDrawDocument* pModel = (SdDrawDocument*) pSourceView->GetAllMarkedModel();
                    bReturn = Paste( *pModel, aDropPos, pPage, nPasteOptions );

                    if( bLink )
                    {
                        SdrObject*      pObj = NULL;
                        SdPage*         pWorkPage = pModel->GetSdPage( 0, PK_STANDARD );
                        SdrObjListIter  aIter( *pWorkPage, IM_DEEPWITHGROUPS );
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
                    pSourceDoc->CreatingDataObj( NULL );
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
            SdDrawDocument* pWorkModel = (SdDrawDocument*) pOwnData->GetWorkDocument();
            SdPage*         pWorkPage = (SdPage*) pWorkModel->GetSdPage( 0, PK_STANDARD );

            pWorkPage->SetRectsDirty();

            // #104148# Use SnapRect, not BoundRect
            Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

            aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
            aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );

            // delete pages, that are not of any interest for us
            for( long i = ( pWorkModel->GetPageCount() - 1 ); i >= 0; i-- )
            {
                SdPage* pPage = (SdPage*) pWorkModel->GetPage( (USHORT) i );

                if( pPage->GetPageKind() != PK_STANDARD )
                    pWorkModel->DeletePage( (USHORT) i );
            }

            bReturn = Paste( *pWorkModel, aDropPos, pPage, nPasteOptions );

            if( !pPage )
                pPage = (SdPage*) GetPageViewPvNum( 0 )->GetPage();

            String aLayout(pPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
            pPage->SetPresentationLayout( aLayout, FALSE, FALSE );
       }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_DRAWING ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
        {
            BOOL bChanged = FALSE;

            SdDrawDocument* pModel = new SdDrawDocument( DOCUMENT_TYPE_IMPRESS, pDocSh );
            pModel->GetItemPool().SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
//          pModel->GetItemPool().FreezeIdRanges();

            pModel->SetStreamingSdrModel( TRUE );

            Reference< XComponent > xComponent( new SdXImpressDocument( pModel, sal_True ) );
            pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );

            xStm->Seek( 0 );

            com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ) );
            bReturn = SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Impress.XMLImporter" );

            pModel->SetStreamingSdrModel( FALSE );

            if( bReturn )
            {
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
                        Rectangle   aPickObjRect( pPickObj->GetCurrentBoundRect() );
                        Size        aPickObjSize( aPickObjRect.GetSize() );
                        Point       aVec( aPickObjRect.TopLeft() );
                        Rectangle   aObjRect( pNewObj->GetCurrentBoundRect() );
                        Size        aObjSize( aObjRect.GetSize() );

                        Fraction aScaleWidth( aPickObjSize.Width(), aObjSize.Width() );
                        Fraction aScaleHeight( aPickObjSize.Height(), aObjSize.Height() );
                        pNewObj->NbcResize( aObjRect.TopLeft(), aScaleWidth, aScaleHeight );

                        aVec -= aObjRect.TopLeft();
                        pNewObj->NbcMove( Size( aVec.X(), aVec.Y() ) );

                        BegUndo( String( SdResId(STR_UNDO_DRAGDROP ) ) );
                        pNewObj->NbcSetLayer( pPickObj->GetLayer() );
                        SdrPage* pWorkPage = GetPageViewPvNum( 0 )->GetPage();
                        pWorkPage->InsertObject( pNewObj );
                        AddUndo( new SdrUndoNewObj( *pNewObj ) );
                        AddUndo( new SdrUndoDelObj( *pPickObj ) );
                        pWorkPage->RemoveObject( pPickObj->GetOrdNum() );
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
                        aSet.Put( pObj->GetMergedItemSet() );

                        // Eckenradius soll nicht uebernommen werden.
                        // In der Gallery stehen Farbverlauefe (Rechtecke)
                        // welche den Eckenradius == 0 haben. Dieser soll
                        // nicht auf das Objekt uebertragen werden.
                        aSet.ClearItem( SDRATTR_ECKENRADIUS );

                        pPickObj->SetMergedItemSetAndBroadcast( aSet );

                        if( pPickObj->ISA( E3dObject ) && pObj->ISA( E3dObject ) )
                        {
                            // Zusaetzlich 3D Attribute handeln
                            SfxItemSet aNewSet( pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );
                            SfxItemSet aOldSet( pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0 );

                            aOldSet.Put(pPickObj->GetMergedItemSet());
                            aNewSet.Put( pObj->GetMergedItemSet() );

                            AddUndo( new E3dAttributesUndoAction( *pDoc, this, (E3dObject*) pPickObj, aNewSet, aOldSet, FALSE ) );
                            pPickObj->SetMergedItemSetAndBroadcast( aNewSet );
                        }

                        EndUndo();
                        bChanged = TRUE;
                    }
                }
            }

            if( !bChanged )
            {
                SdrPage* pWorkPage = pModel->GetPage( 0 );

                pWorkPage->SetRectsDirty();

                if( pOwnData )
                {
                    // #104148# Use SnapRect, not BoundRect
                    Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                    aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                    aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                }

                bReturn = Paste( *pModel, aDropPos, pPage, nPasteOptions );
            }
        }
    }
    else if( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE ) )
    {
        ::rtl::OUString aOUString;

        if( aDataHelper.GetString( SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE, aOUString ) )
        {
            SdrObject* pObj = CreateFieldControl( aOUString );

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
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) )  &&
               aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
    {
        SotStorageStreamRef             xStm;
        TransferableObjectDescriptor    aObjDesc;

        if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) &&
            ( aDataHelper.GetSotStorageStream( nFormat ? nFormat : SOT_FORMATSTR_ID_EMBED_SOURCE, xStm ) ||
              aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_EMBEDDED_OBJ, xStm ) ) )
        {
            SvStorageRef xStore( new SvStorage( *xStm ) );

            if( pDoc->GetDocSh() && ( pDoc->GetDocSh()->GetClassName() == aObjDesc.maClassName ) )
            {
                ::sd::DrawDocShellRef xDocShRef( new ::sd::DrawDocShell( SFX_CREATE_MODE_EMBEDDED, TRUE, pDoc->GetDocumentType() ) );

                if( xDocShRef->DoLoad( xStore ) )
                {
                    SdDrawDocument* pModel = (SdDrawDocument*) xDocShRef->GetDoc();
                    SdPage*         pWorkPage = (SdPage*) pModel->GetSdPage( 0, PK_STANDARD );

                    pWorkPage->SetRectsDirty();

                    if( pOwnData )
                    {
                        // #104148# Use SnapRect, not BoundRect
                        Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                        aDropPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                        aDropPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
                    }

                    // delete pages, that are not of any interest for us
                    for( long i = ( pModel->GetPageCount() - 1 ); i >= 0; i-- )
                    {
                        SdPage* pPage = (SdPage*) pModel->GetPage( (USHORT) i );

                        if( pPage->GetPageKind() != PK_STANDARD )
                            pModel->DeletePage( (USHORT) i );
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
                SvInPlaceObjectRef  xIPObj( &( (SvFactory*) SvInPlaceObject::ClassFactory() )->CreateAndLoad( xStore ) );

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

                    if (pViewSh!=NULL)
                    {
                        OSL_ASSERT (pViewSh->GetViewShell()!=NULL);
                        SfxInPlaceClient* pIpClient
                            = pViewSh->GetViewShell()->GetIPClient();
                        if (pIpClient!=NULL && pIpClient->IsInPlaceActive())
                            nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObject( pObj, *pPV, nOptions );

                    if( pImageMap )
                        pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                    bReturn = TRUE;
                }
            }
        }
    }
    else if( !bLink &&
             ( CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ) ||
               CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ) &&
               aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE ) )
    {
        SotStorageStreamRef             xStm;
        TransferableObjectDescriptor    aObjDesc;

        if( aDataHelper.GetTransferableObjectDescriptor(
            SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE, aObjDesc ))
        {
            UniString aEmptyStr;
            SvStorage* xStore = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
            SvInPlaceObjectRef xIPObj= &((SvFactory*)SvInPlaceObject::ClassFactory())
                ->CreateAndInit( aDataHelper.GetTransferable(), xStore);

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

                if (pViewSh != NULL)
                {
                    OSL_ASSERT (pViewSh->GetViewShell()!=NULL);
                    SfxInPlaceClient* pIpClient =
                        pViewSh->GetViewShell()->GetIPClient();
                    if (pIpClient!=NULL && pIpClient->IsInPlaceActive())
                        nOptions |= SDRINSERT_DONTMARK;
                }

                InsertObject( pObj, *pPV, nOptions );

                if( pImageMap )
                    pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                bReturn = TRUE;

            }
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_SVXB ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ) )
        {
            Point   aInsertPos( rPos );
            Graphic aGraphic;

            *xStm >> aGraphic;

            if( pOwnData && pOwnData->GetWorkDocument() )
            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetRectsDirty();

                // #104148# Use SnapRect, not BoundRect
                Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            // #90129# restrict movement to WorkArea
            Size aImageMapSize = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(),
                aGraphic.GetPrefMapMode(), MapMode(MAP_100TH_MM));

            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aGraphic, nAction, aInsertPos, NULL, pImageMap );
            bReturn = TRUE;
        }
    }
    else if( ( !bLink || pPickObj ) && CHECK_FORMAT_TRANS( FORMAT_GDIMETAFILE ) )
    {
        GDIMetaFile aMtf;

        if( aDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
        {


SvFileStream    aSvOutputStream( String( RTL_CONSTASCII_USTRINGPARAM( "/tmp/test.png" ) ), STREAM_WRITE | STREAM_TRUNC );
Graphic         aMtfGraphic( aMtf );
Size            aPreviewSizePixel( OutputDevice::LogicToLogic( aMtf.GetPrefSize(), aMtf.GetPrefMapMode(), MAP_PIXEL ) );

if( aPreviewSizePixel.Width() && aPreviewSizePixel.Height() )
{
    const double fWH = static_cast< double >( aPreviewSizePixel.Width() ) / static_cast< double >( aPreviewSizePixel.Height() );

    if( fWH <= 1.0 )
        aPreviewSizePixel.Width() = static_cast< long >( 128.0 * fWH ), aPreviewSizePixel.Height() = 128;
    else
        aPreviewSizePixel.Width() = 128, aPreviewSizePixel.Height() = static_cast< long >( 128.0 / fWH );

    if( GraphicConverter::Export( aSvOutputStream, aMtfGraphic.GetBitmapEx( &aPreviewSizePixel ), CVT_PNG ) )
    {
        // handle errror case here
    }
    else
    {
        // Success
    }
}



            Point aInsertPos( rPos );

            if( pOwnData && pOwnData->GetWorkDocument() )

            {
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetRectsDirty();

                // #104148# Use SnapRect, not BoundRect
                Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            // #90129# restrict movement to WorkArea
            Size aImageMapSize(aMtf.GetPrefSize());
            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

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
                const SdDrawDocument*   pWorkModel = pOwnData->GetWorkDocument();
                SdrPage*                pWorkPage = (SdrPage*) ( ( pWorkModel->GetPageCount() > 1 ) ?
                                                    pWorkModel->GetSdPage( 0, PK_STANDARD ) :
                                                    pWorkModel->GetPage( 0 ) );

                pWorkPage->SetRectsDirty();

                // #104148# Use SnapRect, not BoundRect
                Size aSize( pWorkPage->GetAllObjSnapRect().GetSize() );

                aInsertPos.X() = pOwnData->GetStartPos().X() + ( aSize.Width() >> 1 );
                aInsertPos.Y() = pOwnData->GetStartPos().Y() + ( aSize.Height() >> 1 );
            }

            // #90129# restrict movement to WorkArea
            Size aImageMapSize(aBmp.GetPrefSize());
            ImpCheckInsertPos(aInsertPos, aImageMapSize, GetWorkArea());

            InsertGraphic( aBmp, nAction, aInsertPos, NULL, pImageMap );
            bReturn = TRUE;
        }
    }
    else if( pPickObj && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_XFA ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_XFA, xStm ) )
        {
            XFillExchangeData aFillData( XFillAttrSetItem( &pDoc->GetPool() ) );

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
                ::sd::Window* pWin = pViewSh->GetActiveWindow();
                USHORT nHitLog = (USHORT) pWin->PixelToLogic(
                    Size(FuPoor::HITPIX, 0 ) ).Width();
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
                pPickObj->SetMergedItemSetAndBroadcast( aSet );
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
    else if( !bLink && CHECK_FORMAT_TRANS( SOT_FORMATSTR_ID_EDITENGINE ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_EDITENGINE, xStm ) )
        {
            OutlinerView* pOLV = GetTextEditOutlinerView();

            xStm->Seek( 0 );

            if( pOLV )
            {
                Rectangle   aRect( pOLV->GetOutputArea() );
                   Point       aPos( pOLV->GetWindow()->PixelToLogic( aDropPos ) );

                if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                {
                    pOLV->Read( *xStm, EE_FORMAT_BIN, FALSE, pDocSh->GetHeaderAttributes() );
                    bReturn = TRUE;
                }
            }

            if( !bReturn )
                bReturn = SdrView::Paste( *xStm, EE_FORMAT_BIN, aDropPos, pPage, nPasteOptions );
        }
    }
    else if( !bLink && CHECK_FORMAT_TRANS( FORMAT_RTF ) )
    {
        SotStorageStreamRef xStm;

        if( aDataHelper.GetSotStorageStream( FORMAT_RTF, xStm ) )
        {
            OutlinerView* pOLV = GetTextEditOutlinerView();

            xStm->Seek( 0 );

            if( pOLV )
            {
                Rectangle   aRect( pOLV->GetOutputArea() );
                   Point       aPos( pOLV->GetWindow()->PixelToLogic( aDropPos ) );

                if( aRect.IsInside( aPos ) || ( !bDrag && IsTextEdit() ) )
                {
                    pOLV->Read( *xStm, EE_FORMAT_RTF, FALSE, pDocSh->GetHeaderAttributes() );
                    bReturn = TRUE;
                }
            }

            if( !bReturn )
                bReturn = SdrView::Paste( *xStm, EE_FORMAT_RTF, aDropPos, pPage, nPasteOptions );
        }
    }
    else if( CHECK_FORMAT_TRANS( FORMAT_FILE_LIST ) )
    {
        FileList aDropFileList;

        if( aDataHelper.GetFileList( FORMAT_FILE_LIST, aDropFileList ) )
        {
            aDropFileVector.clear();

            for( ULONG i = 0, nCount = aDropFileList.Count(); i < nCount; i++ )
                aDropFileVector.push_back( aDropFileList.GetFile( i ) );

            aDropInsertFileTimer.Start();
        }

        bReturn = TRUE;
    }
    else if( CHECK_FORMAT_TRANS( FORMAT_FILE ) )
    {
        String aDropFile;

        if( aDataHelper.GetString( FORMAT_FILE, aDropFile ) )
        {
            aDropFileVector.clear();
            aDropFileVector.push_back( aDropFile );
            aDropInsertFileTimer.Start();
        }

        bReturn = TRUE;
    }
    else if( !bLink && CHECK_FORMAT_TRANS( FORMAT_STRING ) )
    {
        if( ( FORMAT_STRING == nFormat ) ||
            ( !aDataHelper.HasFormat( SOT_FORMATSTR_ID_SOLK ) &&
              !aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
              !aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILENAME ) ) )
        {
            ::rtl::OUString aOUString;

            if( aDataHelper.GetString( FORMAT_STRING, aOUString ) )
            {
                OutlinerView* pOLV = GetTextEditOutlinerView();

                if( pOLV )
                {
                    pOLV->InsertText( aOUString );
                    bReturn = TRUE;
                }

                if( !bReturn )
                    bReturn = SdrView::Paste( aOUString, aDropPos, pPage, nPasteOptions );
            }
        }
    }

    MarkListHasChanged();
    bIsDropAllowed = TRUE;
    rDnDAction = nAction;
    delete pImageMap;

    return bReturn;
}

} // end of namespace sd
