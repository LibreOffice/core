/*************************************************************************
 *
 *  $RCSfile: sdview3.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:50:43 $
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
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
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

#include <svx/dbexch.hrc>

#include "docshell.hxx"
#include "fupoor.hxx"
#include "sdwindow.hxx"
#include "sdview.hxx"
#include "dragserv.hxx"
#include "sdpage.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "slidvish.hxx"
#include "strmname.h"

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef _E3D_UNDO_HXX
#include <svx/e3dundo.hxx>
#endif

// statisches Flag, das anzeigt, ob momentan gedropt
// werden darf
extern BOOL bIsDropAllowed;


/*************************************************************************
|*
|* Paste
|*
\************************************************************************/

#ifdef WNT
#pragma optimize ( "", off )
#endif



BOOL SdView::InsertData(SvDataObjectRef pDataObject, const Point& rPos,
                        DropAction& rAction, BOOL bDrag, ULONG nFormat,
                        USHORT nPage, USHORT nLayer)
{
    aDropPos = rPos;
    eAction = rAction;

    SdPage*     pPage = NULL;
    SdWindow*   pWin = pViewSh->GetActiveWindow();
    ImageMap*   pImageMap = NULL;
    BOOL        bMtf = FALSE;
    BOOL        bReturn = FALSE;
    BOOL        bLink = (eAction == DROP_LINK ? TRUE : FALSE);
    BOOL        bCopy = (eAction == DROP_COPY || bLink ? TRUE : FALSE);
    ULONG nSBAFieldFormat = Exchange::RegisterFormatName( String( RTL_CONSTASCII_USTRINGPARAM( SBA_FIELDEXCHANGE_FORMAT )));
    ULONG nEditEngineFormat = EditEngine::RegisterClipboardFormatName();

    ULONG nPasteOptions = SDRINSERT_SETDEFLAYER;

    if ((pViewSh && pViewSh->GetIPClient() &&
         pViewSh->GetIPClient()->IsInPlaceActive()) ||
        pViewSh->ISA(SdSlideViewShell))
    {
        nPasteOptions |= SDRINSERT_DONTMARK;
    }

    SdrObject* pPickObj = NULL;

    if (bDrag)
    {
        // Ist ein Objekt getroffen worden?
        SdrPageView* pPV = NULL;
        PickObj(rPos, pPickObj, pPV);
    }

    // weiteres Droppen waehrend des Insertens verhindern
    bIsDropAllowed = FALSE;

    if (nPage != SDRPAGE_NOTFOUND)
    {
        pPage = (SdPage*) pDoc->GetPage(nPage);
    }

    SdDataObject* pOwnData = NULL;
    if( ( SvDataObject*) pDataObject == SD_MOD()->pDragData )
    {
        pOwnData = SD_MOD()->pDragData;
    }
    else if( ( SvDataObject*) pDataObject == SD_MOD()->pClipboardData )
    {
        pOwnData = SD_MOD()->pClipboardData;
    }

    // ggf. holen wir uns erst einmal die ImageMap
    if ( !pOwnData || nFormat )
    {
        SvData aData(SOT_FORMATSTR_ID_SVIM);

        if ( pDataObject->GetData( &aData ) )
            aData.GetData( (SvDataCopyStream**) &pImageMap, ImageMap::StaticType(), TRANSFER_MOVE );
    }

    const SvDataTypeList& rTypeList = pDataObject->GetTypeList();

    if (pOwnData && nFormat==0)
    {
        /**********************************************************************
        * Eigene Daten
        **********************************************************************/
        SdView* pSourceView = NULL;

        // Pointer auf Source-View
        pSourceView = pOwnData->pSdView;

        if (pSourceView)
        {
            /******************************************************************
            * Internes Drop
            ******************************************************************/

            if (pSourceView == this)
            {
                /**************************************************************
                * View intern
                **************************************************************/
                if (nLayer != SDRLAYER_NOTFOUND)
                {
                    /**********************************************************
                    * Drop auf Layer-TabBar
                    **********************************************************/
                    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                    SdrLayer* pLayer = rLayerAdmin.GetLayerPerID(nLayer);
                    SdrPageView* pPV = GetPageViewPvNum(0);
                    String aLayer = pLayer->GetName();

                    if (!pPV->IsLayerLocked(aLayer))
                    {
                        pOwnData->bInternalMove = TRUE;
                        aMark.ForceSort();

                        for (ULONG nM=0; nM<aMark.GetMarkCount(); nM++)
                        {
                            SdrMark* pM=aMark.GetMark(nM);
                            SdrObject* pO=pM->GetObj();

                            if (pO!=NULL)
                            {
                                pO->SetLayer( (SdrLayerID) nLayer);
                            }
                        }

                        bReturn = TRUE;
                    }
                }
                else
                {
                    BOOL bDropOnTabBar = TRUE;
                    SdrPageView* pPV = GetPageViewPvNum(0);

                    if (!pPage && pPV->GetPage()->GetPageNum() != nDragSrcPgNum)
                    {
                        pPage = (SdPage*) pPV->GetPage();
                        bDropOnTabBar = FALSE;
                    }

                    if (pPage)
                    {
                        /******************************************************
                        * Drop auf eine andere Seite (TabBar oder Seite)
                        ******************************************************/
                        String aActiveLayer = GetActiveLayer();

                        if (!pPV->IsLayerLocked(aActiveLayer))
                        {
                            if ( !IsPresObjSelected() )
                            {
                                SdrMarkList* pMarkList;

                                if (nDragSrcPgNum != SDRPAGE_NOTFOUND &&
                                    nDragSrcPgNum != pPV->GetPage()->GetPageNum())
                                {
                                    // Source- und Destination-Page unterschiedlich:
                                    // es wird die gemerkte MarkList verwendet
                                    pMarkList = pDragSrcMarkList;
                                }
                                else
                                {
                                    // Es wird die aktuelle MarkList verwendet
                                    pMarkList = new SdrMarkList(aMark);
                                }

                                pMarkList->ForceSort();

                                for (ULONG nM=0; nM<pMarkList->GetMarkCount(); nM++)
                                {
                                    SdrMark* pM = pMarkList->GetMark(nM);
                                    SdrObject* pObj = pM->GetObj()->Clone();

                                    if (pObj)
                                    {
                                        if (!bDropOnTabBar)
                                        {
                                            // Position setzen
                                            Rectangle aRect = pObj->GetLogicRect();
                                            Size aSize = aRect.GetSize();
                                            Size aVector(aDropPos.X() -
                                                         pOwnData->aStartPos.X(),
                                                         aDropPos.Y() -
                                                         pOwnData->aStartPos.Y());
                                            Point aPos(aRect.TopLeft());
                                            aPos.X() += aVector.Width();
                                            aPos.Y() += aVector.Height();
                                            aRect.SetPos(aPos);
                                            pObj->SetLogicRect(aRect);
                                        }

                                        pPage->InsertObject(pObj);

                                        BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                                        AddUndo(new SdrUndoNewObj(*pObj));
                                        EndUndo();
                                    }
                                }

                                if (pMarkList != pDragSrcMarkList)
                                {
                                    delete pMarkList;
                                }

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
                        pOwnData->bInternalMove = TRUE;

                        Size aVector (aDropPos.X() - pOwnData->aStartPos.X(),
                                      aDropPos.Y() - pOwnData->aStartPos.Y() );
                        MoveAllMarked(aVector, bCopy);
                        bReturn = TRUE;
                    }
                }
            }
            else
            {
                /**************************************************************
                * Unterschiedliche Views
                **************************************************************/
                if ( !pSourceView->IsPresObjSelected() )
                {
                    // Model gehoert der in AllocModel() erzeugten DocShell
                    SdDrawDocument* pSourceDoc = (SdDrawDocument*) pSourceView->GetModel();
                    pSourceDoc->CreatingDataObj(TRUE);
                    SdDrawDocument* pModel = (SdDrawDocument*) pSourceView->
                                             GetAllMarkedModel();
                    bReturn = Paste(*pModel, aDropPos, pPage, nPasteOptions);

                    if (bLink)
                    {
                        SdrObject* pObj = NULL;
                        SdPage* pPage = pModel->GetSdPage(0, PK_STANDARD);
                        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);
                        String aDocName = pSourceDoc->GetDocSh()->GetMedium()->
                                          GetName();

                        while (aIter.IsMore())
                        {
                            pObj = aIter.Next();
                            String aName(pObj->GetName());

                            if (aName.Len())
                            {
                                SdrObject* pNewObj = pDoc->GetObj(aName);

                                if (pNewObj)
                                {
                                    // Objekt gefunden
                                    if (pNewObj->ISA(SdrObjGroup))
                                    {
                                        ( (SdrObjGroup*) pNewObj)->
                                        SetGroupLink(aDocName, aName);
                                    }
                                }
                            }
                        }
                    }

                    if (!pPage)
                    {
                        pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
                    }

                    String aLayout(pPage->GetLayoutName());
                    aLayout.Erase( aLayout.SearchAscii( SD_LT_SEPARATOR ));
                    pPage->SetPresentationLayout(aLayout, FALSE, FALSE);

                    pSourceDoc->CreatingDataObj(FALSE);
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
            /******************************************************************
            * Internes Paste
            ******************************************************************/
            SdrModel* pModel = pOwnData->pSdDrawDocument;
            pModel->GetPage(0)->SetRectsDirty();
            Size aSize = pModel->GetPage(0)->GetAllObjBoundRect().GetSize();
            aDropPos.X() = aSize.Width()  / 2 + pOwnData->aStartPos.X();
            aDropPos.Y() = aSize.Height() / 2 + pOwnData->aStartPos.Y();
            bReturn = Paste(*pModel, aDropPos, pPage, nPasteOptions);

            if (!pPage)
            {
                pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
            }

            String aLayout(pPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
            pPage->SetPresentationLayout(aLayout, FALSE, FALSE);
       }
    }
    else if ( rTypeList.Get(SOT_FORMATSTR_ID_DRAWING) &&
              (nFormat==SOT_FORMATSTR_ID_DRAWING || nFormat==0))
    {
        SvData aData(SOT_FORMATSTR_ID_DRAWING);

        if ( pDataObject->GetData(&aData) )
        {
            SvStorageStreamRef xStream;

            if ( aData.GetData(xStream) )       // Setzt auch die BufferSize
            {
                xStream->SetVersion(SOFFICE_FILEFORMAT_NOW);
                FmFormModel* pModel = new FmFormModel(SFX_APP()->GetAppIniManager()
                                      ->Get(SFX_KEY_PALETTE_PATH), NULL, pDocSh);
                xStream->Seek(0);
                pModel->SetStreamingSdrModel(TRUE);
                pModel->GetItemPool().Load(*xStream);
                *xStream >> *pModel;

                BOOL bChanged = FALSE;

                if (pModel->GetPage(0)->GetObjCount() == 1)
                {
                    /**********************************************************
                    * Genau ein Objekt im DataObj
                    **********************************************************/
                    SdrObject* pObj = pModel->GetPage(0)->GetObj(0);
                    SdrObject* pPickObj = NULL;
                    SdrPageView* pPV = NULL;
                    BOOL bPickObj = PickObj(rPos, pPickObj, pPV);

                    if (eAction == DROP_MOVE && pPickObj && pObj)
                    {
                        /******************************************************
                        * Objekt ersetzen
                        ******************************************************/
                        SdrObject* pNewObj = pObj->Clone();
                        Rectangle aPickObjRect(pPickObj->GetBoundRect());
                        Size aPickObjSize(aPickObjRect.GetSize());
                        Rectangle aObjRect(pNewObj->GetBoundRect());
                        Size aObjSize(aObjRect.GetSize());

                        Fraction aScaleWidth(aPickObjSize.Width(), aObjSize.Width());
                        Fraction aScaleHeight(aPickObjSize.Height(), aObjSize.Height());
                        pNewObj->NbcResize(aObjRect.TopLeft(), aScaleWidth, aScaleHeight);

                        Point aVec = aPickObjRect.TopLeft() - aObjRect.TopLeft();
                        pNewObj->NbcMove(Size(aVec.X(), aVec.Y()));

                        BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                        pNewObj->NbcSetLayer(pPickObj->GetLayer());
                        SdrPage* pPage = GetPageViewPvNum(0)->GetPage();
                        pPage->InsertObject(pNewObj);
                        AddUndo(new SdrUndoNewObj(*pNewObj));
                        AddUndo(new SdrUndoDelObj(*pPickObj));
                        pPage->RemoveObject(pPickObj->GetOrdNum());
                        EndUndo();
                        bChanged = TRUE;

                        // Damit in der Gallery nicht geloescht wird
                        eAction = DROP_COPY;
                    }
                    else if (eAction == DROP_LINK && pPickObj && pObj &&
                             !pPickObj->ISA(SdrGrafObj)               &&
                             !pPickObj->ISA(SdrOle2Obj))
                    {
                        /******************************************************
                        * Objekt neu attributieren
                        ******************************************************/
                        BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                        AddUndo(new SdrUndoAttrObj(*pPickObj));
                        SfxItemSet aSet(pDoc->GetPool());

//-/                        pObj->TakeAttributes(aSet, TRUE, FALSE);
                        aSet.Put(pObj->GetItemSet());

                        // Eckenradius soll nicht uebernommen werden.
                        // In der Gallery stehen Farbverlauefe (Rechtecke)
                        // welche den Eckenradius == 0 haben. Dieser soll
                        // nicht auf das Objekt uebertragen werden.
                        aSet.ClearItem(SDRATTR_ECKENRADIUS);

//-/                        pPickObj->SetAttributes(aSet, FALSE);
//-/                        SdrBroadcastItemChange aItemChange(*pPickObj);
                        pPickObj->SetItemSetAndBroadcast(aSet);
//-/                        pPickObj->BroadcastItemChange(aItemChange);

                        if(pPickObj->ISA(E3dObject) && pObj->ISA(E3dObject))
                        {
                            // Zusaetzlich 3D Attribute handeln
                            SfxItemSet aNewSet(pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0);
                            SfxItemSet aOldSet(pDoc->GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END, 0);

//-/                            pPickObj->TakeAttributes(aOldSet, TRUE, TRUE);
                            aOldSet.Put(pPickObj->GetItemSet());

//-/                            pObj->TakeAttributes(aNewSet, TRUE, TRUE);
                            aNewSet.Put(pObj->GetItemSet());

                            AddUndo(new E3dAttributesUndoAction(*pDoc, this,
                                (E3dObject*)pPickObj, aNewSet, aOldSet, FALSE));

//-/                            pPickObj->SetAttributes(aNewSet, FALSE);
//-/                            SdrBroadcastItemChange aItemChange(*pPickObj);
                            pPickObj->SetItemSetAndBroadcast(aNewSet);
//-/                            pPickObj->BroadcastItemChange(aItemChange);
                        }
                        EndUndo();
                        bChanged = TRUE;
                    }
                }

                if (!bChanged)
                {
                    // Objekt einfuegen
                    pModel->GetPage(0)->SetRectsDirty();

                    if (pOwnData)
                    {
                        Size aSize = pModel->GetPage(0)->GetAllObjBoundRect().GetSize();
                        aDropPos.X() = aSize.Width()  / 2 + pOwnData->aStartPos.X();
                        aDropPos.Y() = aSize.Height() / 2 + pOwnData->aStartPos.Y();
                    }

                    bReturn = Paste(*pModel, aDropPos, pPage, nPasteOptions);
                }

                pModel->SetStreamingSdrModel(FALSE);
            }
        }
    }
    else if ( rTypeList.Get(SOT_FORMATSTR_ID_SVXB)    &&
              ( nFormat == SOT_FORMATSTR_ID_SVXB || nFormat==0 ) &&
              (!bLink || pPickObj) )
    {
        SvData aData( SOT_FORMATSTR_ID_SVXB );

        if ( pDataObject->GetData( &aData ) )
        {
            Graphic* pGraphic = NULL;

            if ( aData.GetData( (SvDataCopyStream**) &pGraphic,
                                Graphic::StaticType(), TRANSFER_MOVE ) )
            {
                if (pOwnData)
                {
                    SdrModel* pModel = pOwnData->pSdDrawDocument;
                    pModel->GetPage(0)->SetRectsDirty();
                    Size aSize = pModel->GetPage(0)->GetAllObjBoundRect().GetSize();
                    aDropPos.X() = aSize.Width()  / 2 + pOwnData->aStartPos.X();
                    aDropPos.Y() = aSize.Height() / 2 + pOwnData->aStartPos.Y();
                }

                InsertGraphic(*pGraphic, eAction, aDropPos, NULL, pImageMap);
                bReturn = TRUE;
            }

            if ( pGraphic )
                delete pGraphic;
        }
    }
    else if ( rTypeList.Get(nSBAFieldFormat)  &&
              (nFormat==nSBAFieldFormat || nFormat==0) )
    {
        // Datenbankfeld einfuegen
        SvData aData(nSBAFieldFormat);

        if ( pDataObject->GetData(&aData) )
        {
            String aString;

            if ( aData.GetData(aString) )
            {
                SdrObject* pObj = CreateFieldControl(aString);

                if (pObj)
                {
                    bReturn = TRUE;
                    Rectangle aRect(pObj->GetLogicRect());
                    aDropPos.X() -= aRect.GetSize().Width()  / 2;
                    aDropPos.Y() -= aRect.GetSize().Height() / 2;
                    aRect.SetPos(aDropPos);
                    pObj->SetLogicRect(aRect);
                    InsertObject(pObj, *GetPageViewPvNum(0), SDRINSERT_SETDEFLAYER);
                }
            }
        }
    }
    else if (!bLink &&
             (((nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE     ||
                nFormat == SOT_FORMATSTR_ID_EMBEDDED_OBJ     ||
                nFormat == SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ||
                nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE_OLE) && rTypeList.Get(nFormat)) ||
             (nFormat == 0 &&
              (rTypeList.Get(SOT_FORMATSTR_ID_EMBED_SOURCE)     ||
               rTypeList.Get(SOT_FORMATSTR_ID_EMBEDDED_OBJ)     ||
               rTypeList.Get(SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE) ||
               rTypeList.Get(SOT_FORMATSTR_ID_EMBED_SOURCE_OLE)))))
    {
        BOOL bStorFilled = FALSE;
        bLink = FALSE;
        SvStorageRef aStor = new SvStorage( String(), STREAM_STD_READWRITE );

        BOOL bDrawFmt = FALSE;
        BOOL bImpressFmt = SvFactory::IsOwnFormat(bStorFilled, pDataObject, SOT_FORMATSTR_ID_STARIMPRESS_50, aStor);

        if( !bImpressFmt )
            bDrawFmt = SvFactory::IsOwnFormat(bStorFilled, pDataObject, SOT_FORMATSTR_ID_STARDRAW_50, aStor);

        DocumentType eDocType = pDoc->GetDocumentType();

        if( ( bDrawFmt    && eDocType == DOCUMENT_TYPE_DRAW ) ||
            ( bImpressFmt && eDocType == DOCUMENT_TYPE_IMPRESS ) )
        {
            if (bStorFilled)
            {
                // Eigenes Format (OLE)
                SdDrawDocShellRef xDocShRef = new SdDrawDocShell(
                                              SFX_CREATE_MODE_EMBEDDED, TRUE,
                                              pDoc->GetDocumentType());

                if (xDocShRef->DoLoad(aStor))
                {
                    SdrModel* pModel = xDocShRef->GetDoc();
                    pModel->GetPage(0)->SetRectsDirty();

                    if (pOwnData)
                    {
                        Size aSize = pModel->GetPage(0)->GetAllObjBoundRect().GetSize();
                        aDropPos.X() = aSize.Width()  / 2 + pOwnData->aStartPos.X();
                        aDropPos.Y() = aSize.Height() / 2 + pOwnData->aStartPos.Y();
                    }

                    bReturn = Paste(*pModel, aDropPos, pPage, nPasteOptions);

                    if (!pPage)
                    {
                        pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
                    }

                    String aLayout(pPage->GetLayoutName());
                    aLayout.Erase(aLayout.SearchAscii(SD_LT_SEPARATOR));
                    pPage->SetPresentationLayout(aLayout, FALSE, FALSE);
                }

                xDocShRef->DoClose();
                xDocShRef.Clear();
            }
        }
        else
        {
#ifndef SO3
            SvInPlaceObjectRef aIPObj = &SvInPlaceObject::ClassFactory()->
                                         CreateAndInit(pDataObject, aStor,
                                                       bLink, bStorFilled);
#else
            SvInPlaceObjectRef aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->
                                         CreateAndInit(pDataObject, aStor,
                                                       bLink, bStorFilled);
#endif
            if (aIPObj.Is())
            {
                String aName = pDocSh->InsertObject(aIPObj, String() )->GetObjName();

                Size aSize;
                SvObjectDescriptor aDesc (pDataObject);
                Size aDescSize( aDesc.GetSize() );  // ist stets in 1/100mm

                if (aDescSize.Width() && aDescSize.Height())
                {
                    aIPObj->SetVisAreaSize(OutputDevice::LogicToLogic(aDescSize, MAP_100TH_MM, aIPObj->GetMapUnit()));
                }

                aSize = aIPObj->GetVisArea().GetSize();

                if (aSize.Height() == 0 || aSize.Width() == 0)
                {
                    // Rechteck mit ausgewogenem Kantenverhaeltnis
                    aSize.Width()  = 14100;
                    aSize.Height() = 10000;
                    aIPObj->SetVisAreaSize( OutputDevice::LogicToLogic(
                                     aSize, MAP_100TH_MM, aIPObj->GetMapUnit() ) );
                    aSize = aIPObj->GetVisArea().GetSize();
                }

                aSize = OutputDevice::LogicToLogic(aSize, aIPObj->GetMapUnit(), MAP_100TH_MM);

                Size aMaxSize( pDoc->GetMaxObjSize() );
                aSize.Height() = Min(aSize.Height(), aMaxSize.Height());
                aSize.Width()  = Min(aSize.Width(), aMaxSize.Width());

                aDropPos.X() -= aSize.Width()  / 2;
                aDropPos.Y() -= aSize.Height() / 2;
                Rectangle aRect (aDropPos, aSize);
                SdrOle2Obj* pObj = new SdrOle2Obj(aIPObj, aName, aRect);
                SdrPageView* pPV = GetPageViewPvNum(0);

                ULONG nOptions = SDRINSERT_SETDEFLAYER;

                if (pViewSh && pViewSh->GetIPClient() &&
                    pViewSh->GetIPClient()->IsInPlaceActive())
                {
                    nOptions |= SDRINSERT_DONTMARK;
                }

                InsertObject(pObj, *pPV, nOptions);

                if ( pImageMap )
                    pObj->InsertUserData( new SdIMapInfo( *pImageMap ) );

                bReturn = TRUE;
            }
        }
    }
    else if (((bMtf = (rTypeList.Get(FORMAT_GDIMETAFILE) &&
              (nFormat==FORMAT_GDIMETAFILE || nFormat==0))) ||
              (rTypeList.Get(FORMAT_BITMAP) &&
              (nFormat==FORMAT_BITMAP || nFormat==0)))  &&
              (!bLink || pPickObj) )
    {
        Point aInsPos = rPos;

        if (pOwnData && pOwnData->pSdDrawDocument)
        {
            SdrModel* pModel = pOwnData->pSdDrawDocument;
            pModel->GetPage(0)->SetRectsDirty();
            Size aSize = pModel->GetPage(0)->GetAllObjBoundRect().GetSize();
            aInsPos.X() = aSize.Width()  / 2 + pOwnData->aStartPos.X();
            aInsPos.Y() = aSize.Height() / 2 + pOwnData->aStartPos.Y();
        }

        if (bMtf)
        {
            SvData aData (FORMAT_GDIMETAFILE);

            if (pDataObject->GetData(&aData))
            {
                GDIMetaFile* pMtf = NULL;

                if (aData.GetData(&pMtf, TRANSFER_MOVE))
                {
                    InsertGraphic(Graphic(*pMtf), eAction, aInsPos, NULL, pImageMap);
                    bReturn = TRUE;
                }

                if ( pMtf )
                    delete pMtf;
            }
        }
        else
        {
            SvData aData (FORMAT_BITMAP);

            if (pDataObject->GetData(&aData))
            {
                Bitmap* pBmp = NULL;

                if (aData.GetData(&pBmp, TRANSFER_MOVE))
                {
                    InsertGraphic(Graphic(*pBmp), eAction, aInsPos, NULL, pImageMap);
                    bReturn = TRUE;
                }

                if ( pBmp )
                    delete pBmp;
            }
        }

    }
    else if ( pPickObj &&
             (nFormat == SOT_FORMATSTR_ID_XFA || nFormat==0) )
    {
        SvData aData(SOT_FORMATSTR_ID_XFA);
        XFillExchangeData* pFillData = NULL;

        if ( pDataObject->GetData( &aData ) )
        {
            if (aData.GetData( (SvDataCopyStream**) &pFillData, XFillExchangeData::StaticType(), TRANSFER_MOVE))
            {
                BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
                AddUndo(new SdrUndoAttrObj(*pPickObj));
                EndUndo();

                XFillAttrSetItem* pSetItem = pFillData->GetXFillAttrSetItem();
                SfxItemSet rSet = pSetItem->GetItemSet();

                XFillStyle eFill= ((XFillStyleItem&) rSet.Get(XATTR_FILLSTYLE)).GetValue();

                if (eFill == XFILL_SOLID || eFill == XFILL_NONE)
                {
                    const XFillColorItem& rColItem = (XFillColorItem&) rSet.Get(XATTR_FILLCOLOR);
                    Color aColor = rColItem.GetValue();
                    String aName = rColItem.GetName();

                    SfxItemSet aSet(pDoc->GetPool());

                    BOOL bClosed = pPickObj->IsClosedObj();
                    SdWindow* pWin = pViewSh->GetActiveWindow();
                    USHORT nHitLog = USHORT ( pWin->PixelToLogic(Size(HITPIX,0)).Width() );
                    const long  n2HitLog = nHitLog * 2;
                    Point aHitPosR( rPos );
                    Point aHitPosL( rPos );
                    Point aHitPosT( rPos );
                    Point aHitPosB( rPos );

                    aHitPosR.X() += n2HitLog;
                    aHitPosL.X() -= n2HitLog;
                    aHitPosT.Y() += n2HitLog;
                    aHitPosB.Y() -= n2HitLog;

                    const SetOfByte* pVisiLayer = &GetPageViewPvNum(0)->GetVisibleLayers();

                    if (bClosed                                          &&
                        pPickObj->IsHit( aHitPosR, nHitLog, pVisiLayer ) &&
                        pPickObj->IsHit( aHitPosL, nHitLog, pVisiLayer ) &&
                        pPickObj->IsHit( aHitPosT, nHitLog, pVisiLayer ) &&
                        pPickObj->IsHit( aHitPosB, nHitLog, pVisiLayer ) )
                    {
                        // Flaechenfuellung
                        if (eFill == XFILL_SOLID)
                        {
                            aSet.Put(XFillColorItem(aName, aColor));
                        }

                        aSet.Put(XFillStyleItem(eFill));
                    }
                    else
                    {
                        // Linienstil hinzufuegen
                        aSet.Put(XLineColorItem(aName, aColor));
                    }

                    // Textfarbe hinzufuegen
//                    aSet.Put(SvxColorItem(aColor, ITEMID_COLOR));

//-/                    pPickObj->SetAttributes(aSet, FALSE);
//-/                    SdrBroadcastItemChange aItemChange(*pPickObj);
                    pPickObj->SetItemSetAndBroadcast(aSet);
//-/                    pPickObj->BroadcastItemChange(aItemChange);
                }
            }
        }
    }
    else if (rTypeList.Get(SOT_FORMATSTR_ID_HTML) &&
             (nFormat==SOT_FORMATSTR_ID_HTML || nFormat==0) && !bLink )
    {
        SvData aData(SOT_FORMATSTR_ID_HTML);

        if ( pDataObject->GetData(&aData) )
        {
            SvStorageStreamRef xStream;

            if ( aData.GetData(xStream) )    // Setzt auch die BufferSize
            {
                xStream->Seek(0);
                bReturn = SdrView::Paste(*xStream, EE_FORMAT_HTML, aDropPos, pPage, nPasteOptions);
            }
        }
    }
    else if ( ( rTypeList.Get( FORMAT_RTF ) && ( nFormat == FORMAT_RTF || nFormat==0 ) && !bLink ) ||
              ( rTypeList.Get( nEditEngineFormat ) && (nFormat == nEditEngineFormat || nFormat==0 ) && !bLink ) )
    {
        SvData aData( nFormat );

        if ( pDataObject->GetData(&aData) )
        {
            SvStorageStreamRef xStream;

            if ( aData.GetData(xStream) )    // Setzt auch die BufferSize
            {
                xStream->Seek(0);

                EETextFormat nFmt = EE_FORMAT_RTF;

                if( nFormat == nEditEngineFormat || ( nFormat == 0 && rTypeList.Get( nEditEngineFormat ) ) )
                    nFmt = EE_FORMAT_BIN;

                OutlinerView* pOLV = GetTextEditOutlinerView();

                if( pOLV )
                {
                    Rectangle aRect( pOLV->GetOutputArea() );
                       Point aPos = pOLV->GetWindow()->PixelToLogic( aDropPos );

                    if ( aRect.IsInside(aPos) )
                    {
                        pOLV->Read( *xStream, nFmt, FALSE, pDocSh->GetHeaderAttributes() );
                        bReturn = TRUE;
                    }
                }

                if( !bReturn )
                {
                    bReturn = SdrView::Paste(*xStream, nFmt, aDropPos, pPage, nPasteOptions);
                }
            }
        }
    }
    else if (rTypeList.Get(FORMAT_STRING)  &&
              (nFormat==FORMAT_STRING || nFormat==0) && !bLink )
    {
        // Falls auch ein URL-Format gedropt wurde, nehmen
        // wir natuerlich dieses und kehren somit sofort zurueck,
        // da beim Aufrufer dieser URL eingefuegt wird
        String aTmpStr;
        INetBookmark aINetBookmark(aTmpStr, aTmpStr);

        if (!INetBookmark::DragServerHasFormat(0) ||
            !aINetBookmark.PasteDragServer(0))
        {
            SvData aData (FORMAT_STRING);

            if ( pDataObject->GetData(&aData) )
            {
                String aString;

                if ( aData.GetData(aString) )
                {
                    OutlinerView* pOLV = GetTextEditOutlinerView();

                    if( pOLV )
                    {
                        pOLV->InsertText( aString );
                        bReturn = TRUE;
                    }
                    else
                    {
                        // Direkt Basisklasse rufen (Bug im MS C++)
                        bReturn = SdrView::Paste(aString, aDropPos, pPage, nPasteOptions);
                    }
                }
            }
        }
    }
    else if (rTypeList.Get(FORMAT_FILE) &&
             (nFormat==FORMAT_FILE || nFormat==0))
    {
        SvData aData(FORMAT_FILE);
        BOOL bGetData = pDataObject->GetData(&aData);
        aDropFile.Erase();
        BOOL bGetFile = aData.GetData(aDropFile);

//      if ( bGetData && bGetFile && aDropFile.Len() )
//      {
//          USHORT nItemCount = DragServer::GetItemCount();
            USHORT nItemCount = 1;

            for (USHORT nItem = 0; nItem < nItemCount; nItem++)
            {
                aDropFile = DragServer::PasteFile(nItem);
                aDropInsertFileTimer.Start();
            }

            bReturn = TRUE;
//      }
    }

    MarkListHasChanged();

    //Droppen wieder ermoeglichen
    bIsDropAllowed = TRUE;
    rAction = eAction;

    if ( pImageMap )
        delete pImageMap;

    return (bReturn);
}

