/*************************************************************************
 *
 *  $RCSfile: sdview5.cxx,v $
 *
 *  $Revision: 1.3 $
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

#if defined (WIN) || defined (WNT)
#include <tools/svwin.h>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif

#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#include "app.hrc"
#include "sdwindow.hxx"
#include "docshell.hxx"
#include "drviewsh.hxx"
#include "graphpro.hxx"
#include "fuinsfil.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "sdpage.hxx"
#include "sdview.hxx"
#include "slidview.hxx"

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

/*************************************************************************
|*
|* Graphik einfuegen
|* Wird ein leeres Graphikobjekt uebergeben, so wird dieses gefuellt.
|* Andernfalls wird ein an der gegebenen Position vorhandenes Objekt
|* gefuellt. Ist an der Position kein Objekt vorhanden, so wird ein neues
|* Objekt erzeugt und ein Pointer auf dieses Objekt zurueckgegeben.
|*
\************************************************************************/

SdrGrafObj* SdView::InsertGraphic(const Graphic& rGraphic, DropAction& rAction,
                                  const Point& rPos, SdrObject* pObj,
                                  ImageMap* pImageMap)
{
    EndTextEdit();
    eAction = rAction;

    // Liegt ein Objekt an der Position rPos?
    SdrGrafObj* pNewGrafObj = NULL;
    SdrPageView* pPV = GetPageViewPvNum(0);

    if (this->ISA(SdSlideView))
    {
        pPV = HitPage(rPos);
    }

    SdrObject* pPickObj = pObj;

    if ( !pPickObj && pPV )
    {
        SdrPageView* pPageView = pPV;
        PickObj(rPos, pPickObj, pPageView);
    }

    if (eAction == DROP_LINK && pPickObj && pPV )
    {
        /**********************************************************************
        * Objekt neu attributieren
        **********************************************************************/
        if (pPickObj->ISA(SdrGrafObj))
        {
            /******************************************************************
            * Das Graphik-Objekt bekommt eine neue Graphik
            ******************************************************************/
            // Das Objekt wird mit der Bitmap gefuellt
            pNewGrafObj = (SdrGrafObj*) pPickObj->Clone();
            pNewGrafObj->SetGraphic(rGraphic);

            if ( pNewGrafObj->IsEmptyPresObj() )
            {
                Rectangle aRect( pNewGrafObj->GetLogicRect() );
                pNewGrafObj->AdjustToMaxRect( aRect, FALSE );
                pNewGrafObj->SetOutlinerParaObject(NULL);
                pNewGrafObj->SetEmptyPresObj(FALSE);
            }

            SdPage* pPage = (SdPage*) pPickObj->GetPage();

            if (pPage && pPage->GetPresObjKind(pPickObj) == PRESOBJ_GRAPHIC)
            {
                // Neues PresObj in die Liste eintragen
                pNewGrafObj->SetUserCall(pPickObj->GetUserCall());
                pPage->GetPresObjList()->Remove(pPickObj);
                pPage->GetPresObjList()->Insert(pNewGrafObj, LIST_APPEND);
            }

            if (pImageMap)
                pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));

            BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
            ReplaceObject(pPickObj, *pPV, pNewGrafObj);
            EndUndo();
        }
        else if (pPickObj->IsClosedObj() && !pPickObj->ISA(SdrOle2Obj))
        {
            /******************************************************************
            * Das Objekt wird mit der Graphik gefuellt
            ******************************************************************/
            BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
            AddUndo(new SdrUndoAttrObj(*pPickObj));
            EndUndo();

            XOBitmap aXOBitmap( rGraphic.GetBitmap() );
            SfxItemSet aSet(pDocSh->GetPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);
            aSet.Put(XFillStyleItem(XFILL_BITMAP));
            aSet.Put(XFillBitmapItem(&pDocSh->GetPool(), aXOBitmap));

//-/            pPickObj->SetAttributes(aSet, FALSE);
//-/            SdrBroadcastItemChange aItemChange(*pPickObj);
            pPickObj->SetItemSetAndBroadcast(aSet);
//-/            pPickObj->BroadcastItemChange(aItemChange);
        }
    }
    else if ( pPV )
    {
        /**********************************************************************
        * Es wird ein neues Objekt erzeugt
        **********************************************************************/
        Size aSize;
        if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
            aSize = pViewSh->GetActiveWindow()->PixelToLogic( rGraphic.GetPrefSize(), MAP_100TH_MM );
        else
            aSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                rGraphic.GetPrefMapMode(),
                                                MapMode( MAP_100TH_MM ) );

        pNewGrafObj = new SdrGrafObj( rGraphic, Rectangle( rPos, aSize ) );
        SdrPage* pPage = pPV->GetPage();
        Size aPageSize( pPage->GetSize() );
        aPageSize.Width()  -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();
        pNewGrafObj->AdjustToMaxRect( Rectangle( pPV->GetOffset(), aPageSize ), TRUE );

        ULONG nOptions = SDRINSERT_SETDEFLAYER;
        if ((pViewSh && pViewSh->GetIPClient() &&
             pViewSh->GetIPClient()->IsInPlaceActive()) ||
             this->ISA(SdSlideView))
        {
            nOptions |= SDRINSERT_DONTMARK;
        }

        BOOL bIsPresTarget = FALSE;

        if (eAction == DROP_MOVE && pPickObj && (pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall()) )
        {
            SdPage* pPage = (SdPage*) pPickObj->GetPage();
            if ( pPage && pPage->IsMasterPage() )
                bIsPresTarget = pPage->GetPresObjList()->GetPos(pPickObj) != LIST_ENTRY_NOTFOUND;
        }

        if (eAction == DROP_MOVE && pPickObj && !bIsPresTarget)
        {
            /******************************************************
            * Objekt ersetzen
            ******************************************************/
            if (pImageMap)
            {
                pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));
            }

            Rectangle aPickObjRect(pPickObj->GetBoundRect());
            Size aPickObjSize(aPickObjRect.GetSize());
            Rectangle aObjRect(pNewGrafObj->GetBoundRect());
            Size aObjSize(aObjRect.GetSize());

            Fraction aScaleWidth(aPickObjSize.Width(), aObjSize.Width());
            Fraction aScaleHeight(aPickObjSize.Height(), aObjSize.Height());
            pNewGrafObj->NbcResize(aObjRect.TopLeft(), aScaleWidth, aScaleHeight);

            Point aVec = aPickObjRect.TopLeft() - aObjRect.TopLeft();
            pNewGrafObj->NbcMove(Size(aVec.X(), aVec.Y()));

            BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
            pNewGrafObj->NbcSetLayer(pPickObj->GetLayer());
            SdrPage* pPage = pPV->GetPage();
            pPage->InsertObject(pNewGrafObj);
            AddUndo(new SdrUndoNewObj(*pNewGrafObj));
            AddUndo(new SdrUndoDelObj(*pPickObj));
            pPage->RemoveObject(pPickObj->GetOrdNum());
            EndUndo();

            // Damit in der Gallery nicht geloescht wird
            eAction = DROP_COPY;
        }
        else
        {
            InsertObject(pNewGrafObj, *pPV, nOptions);

            if (pImageMap)
            {
                pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));
            }
        }
    }
    rAction = eAction;

    return (pNewGrafObj);
}


/*************************************************************************
|*
|* Timer-Handler fuer InsertFile beim Drop()
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdView, DropInsertFileHdl, Timer*, pTimer )
{
    BOOL bOK = FALSE;
    const SfxFilter* pFilter = NULL;

    SfxMedium aSfxMedium(aDropFile, (STREAM_READ | STREAM_SHARE_DENYNONE), FALSE);
    ErrCode nErr = SFX_APP()->GetFilterMatcher().
                              GuessFilter(aSfxMedium, &pFilter, SFX_FILTER_IMPORT,
                                          SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

    if (pFilter && !nErr)
    {
        GraphicFilter* pGraphicFilter = GetGrfFilter();
        String aFilterName = pFilter->GetFilterName();
        USHORT nFormat = pGraphicFilter->GetImportFormatNumber(aFilterName);

        if (aFilterName.EqualsAscii( "Text" )               ||
            aFilterName.EqualsAscii( "Rich Text Format" )   ||
            aFilterName.EqualsAscii( "HTML" )               ||
            aDropFile.ToLowerAscii().SearchAscii(".sdd") != STRING_NOTFOUND ||  //TODO: birnig!
            aDropFile.ToLowerAscii().SearchAscii(".sda") != STRING_NOTFOUND )
        {
            /******************************************************************
            * Eigenes Format, Text oder RTF
            ******************************************************************/
            bOK = TRUE;
            SdWindow* pWin = pViewSh->GetActiveWindow();
            SfxRequest aReq(SID_INSERTFILE, 0, pDoc->GetItemPool());
            SfxStringItem aItem1(ID_VAL_DUMMY0, aDropFile);
            SfxStringItem aItem2(ID_VAL_DUMMY1, pFilter->GetFilterName());
            aReq.AppendItem (aItem1);
            aReq.AppendItem (aItem2);
            FuInsertFile* pFunc = new FuInsertFile(pViewSh, pWin, this,
                                                   pDoc, aReq);
            delete pFunc;
        }
        else if (nFormat != GRFILTER_FORMAT_DONTKNOW)
        {
            /******************************************************************
            * Graphik-Format
            ******************************************************************/
            FilterProgress  aFilterProgress(pGraphicFilter, pViewSh->GetDocSh());
            Graphic aGraphic;

            // keine native Tempdatei anlegen (DummyLink setzen)
            aGraphic.SetLink( GfxLink() );

            SvFileStream aIStm( aDropFile, STREAM_READ | STREAM_SHARE_DENYNONE );

            if( !pGraphicFilter->ImportGraphic(aGraphic, aDropFile, aIStm, nFormat ) )
            {
                bOK = TRUE;
                SdrGrafObj* pGrafObj = InsertGraphic(aGraphic, eAction, aDropPos, NULL, NULL);

                if (pGrafObj)
                {
                    pGrafObj->SetGraphicLink(aDropFile, aFilterName);
                }
            }
        }
    }

    if (!bOK)
    {
        String aTmpStr;
        INetBookmark aINetBookmark(aTmpStr, aTmpStr);

        if (eAction == DROP_LINK ||
            !INetBookmark::DragServerHasFormat(0) || !aINetBookmark.PasteDragServer(0))
        {
            ((SdDrawViewShell*) pViewSh)->InsertURLButton(aDropFile, aDropFile,
                                                          String(), &aDropPos);
        }
        else
        {
            /**********************************************************************
            * Datei als OLE-Objekt einfuegen
            **********************************************************************/
            SvInPlaceObjectRef  aIPObj;
            SvStorageRef        aStor = new SvStorage( String(), STREAM_STD_READWRITE );
            String              aName;

            if (pViewSh)
            {
#ifndef SO3
                aIPObj = &SvInPlaceObject::ClassFactory()->CreateAndInit(aDropFile, aStor);
#else
                aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(aDropFile, aStor);
#endif
                if ( aIPObj.Is() )
                {
                    Size        aSize(aIPObj->GetVisArea(ASPECT_CONTENT).GetSize());
                    Rectangle   aRect;

                    if (!aSize.Width() || !aSize.Height())
                    {
                        aSize.Width()   = 1410;
                        aSize.Height()  = 1000;

                        aRect = Rectangle(aDropPos, aSize);
                    }
                    else
                    {
                        aRect = Rectangle(aDropPos, aSize);
                    }

                    aName = pDocSh->InsertObject(aIPObj, String())->GetObjName();

                    SdrOle2Obj* pOleObj = new SdrOle2Obj(aIPObj, aName, aRect);

                    ULONG nOptions = SDRINSERT_SETDEFLAYER;

                    if (pViewSh && pViewSh->GetIPClient() &&
                        pViewSh->GetIPClient()->IsInPlaceActive())
                    {
                        nOptions |= SDRINSERT_DONTMARK;
                    }

                    InsertObject(pOleObj, *GetPageViewPvNum(0), nOptions);

                    pOleObj->SetLogicRect(aRect);
                    aIPObj->SetVisAreaSize(aRect.GetSize());
                }
            }
        }
    }

    return 0;
}
IMPL_LINK_INLINE_END( SdView, DropInsertFileHdl, Timer*, pTimer )


/*************************************************************************
|*
|* Timer-Handler fuer Errorhandling beim Drop()
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdView, DropErrorHdl, Timer*, pTimer )
{
    InfoBox( pViewSh->GetActiveWindow(),
             String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
    return 0;
}
IMPL_LINK_INLINE_END( SdView, DropErrorHdl, Timer*, pTimer )


#ifdef WNT
#pragma optimize ( "", on )
#endif

/*************************************************************************
|*
|* Redraw sperren oder erlauben
|*
\************************************************************************/

void SdView::LockRedraw(BOOL bLock)
{
    if (bLock)
    {
        nLockRedrawSmph++;
        DBG_ASSERT(nLockRedrawSmph, "Ueberlauf im LockRedraw");
    }
    else
    {
        DBG_ASSERT(nLockRedrawSmph, "Unterlauf im LockRedraw");
        nLockRedrawSmph--;

        // alle gespeicherten Redraws ausfuehren
        if (!nLockRedrawSmph)
        {
            while (pLockedRedraws && pLockedRedraws->Count())
            {
                SdViewRedrawRec* pRec = (SdViewRedrawRec*)pLockedRedraws->First();
                OutputDevice* pCurrentOut = pRec->pOut;
                Rectangle aBoundRect(pRec->aRect);
                pLockedRedraws->Remove(pRec);
                delete pRec;

                pRec = (SdViewRedrawRec*)pLockedRedraws->First();
                while (pRec)
                {
                    if (pRec->pOut == pCurrentOut)
                    {
                        aBoundRect.Union(pRec->aRect);
                        pLockedRedraws->Remove(pRec);
                        delete pRec;
                        pRec = (SdViewRedrawRec*)pLockedRedraws->GetCurObject();
                    }
                    else
                    {
                        pRec = (SdViewRedrawRec*)pLockedRedraws->Next();
                    }
                }

                InitRedraw(pCurrentOut, Region(aBoundRect));
            }
            delete pLockedRedraws;
            pLockedRedraws = NULL;
        }
    }
}


/*************************************************************************
|*
|* StyleSheet aus der Sleketion besorgen
|*
\************************************************************************/

SfxStyleSheet* SdView::GetStyleSheet() const
{
    return SdrView::GetStyleSheet();
}



