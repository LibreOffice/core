/*************************************************************************
 *
 *  $RCSfile: sdview4.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:23:10 $
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

#include "View.hxx"

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
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
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
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
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#include "app.hrc"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "graphpro.hxx"
#ifndef SD_FU_INSERT_FILE_HXX
#include "fuinsfil.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "sdpage.hxx"
#ifndef SD_SLIDE_VIEW_HXX
#include "SlideView.hxx"
#endif

namespace sd {

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

SdrGrafObj* View::InsertGraphic( const Graphic& rGraphic, sal_Int8& rAction,
                                   const Point& rPos, SdrObject* pObj, ImageMap* pImageMap )
{
    EndTextEdit();
    nAction = rAction;

    // Liegt ein Objekt an der Position rPos?
    SdrGrafObj*     pNewGrafObj = NULL;
    SdrPageView*    pPV = GetPageViewPvNum(0);
    SdrObject*      pPickObj = pObj;

    if( this->ISA(SlideView))
        pPV = HitPage( rPos );

    if( !pPickObj && pPV )
    {
        SdrPageView* pPageView = pPV;
        PickObj(rPos, pPickObj, pPageView);
    }

    if( nAction == DND_ACTION_LINK && pPickObj && pPV )
    {
        if( pPickObj->ISA( SdrGrafObj ) )
        {
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
                pPage->RemovePresObj(pPickObj);
                pPage->InsertPresObj(pNewGrafObj, PRESOBJ_GRAPHIC);
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
            pPickObj->SetMergedItemSetAndBroadcast(aSet);
        }
    }
    else if ( pPV )
    {
        // create  new object
        Size aSize;

        if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
            aSize = pViewSh->GetActiveWindow()->PixelToLogic( rGraphic.GetPrefSize(), MAP_100TH_MM );
        else
        {
            aSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                rGraphic.GetPrefMapMode(),
                                                MapMode( MAP_100TH_MM ) );
        }

        pNewGrafObj = new SdrGrafObj( rGraphic, Rectangle( rPos, aSize ) );
        SdrPage* pPage = pPV->GetPage();
        Size aPageSize( pPage->GetSize() );
        aPageSize.Width()  -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();
        pNewGrafObj->AdjustToMaxRect( Rectangle( pPV->GetOffset(), aPageSize ), TRUE );

        ULONG   nOptions = SDRINSERT_SETDEFLAYER;
        BOOL    bIsPresTarget = FALSE;

        if ((pViewSh
                && pViewSh->GetViewShell()!=NULL
                && pViewSh->GetViewShell()->GetIPClient()
                && pViewSh->GetViewShell()->GetIPClient()->IsInPlaceActive())
            || this->ISA(SlideView))
            nOptions |= SDRINSERT_DONTMARK;

        if( ( nAction & DND_ACTION_MOVE ) && pPickObj && (pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall()) )
        {
            SdPage* pPage = (SdPage*) pPickObj->GetPage();

            if ( pPage && pPage->IsMasterPage() )
                bIsPresTarget = pPage->IsPresObj(pPickObj);
        }

        if( ( nAction & DND_ACTION_MOVE ) && pPickObj && !bIsPresTarget )
        {
            // replace object
            if (pImageMap)
                pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));

            Rectangle aPickObjRect(pPickObj->GetCurrentBoundRect());
            Size aPickObjSize(aPickObjRect.GetSize());
            Rectangle aObjRect(pNewGrafObj->GetCurrentBoundRect());
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
            nAction = DND_ACTION_COPY;
        }
        else
        {
            InsertObject(pNewGrafObj, *pPV, nOptions);

            if( pImageMap )
                pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));
        }
    }

    rAction = nAction;

    return pNewGrafObj;
}

/*************************************************************************
|*
|* Timer-Handler fuer InsertFile beim Drop()
|*
\************************************************************************/

IMPL_LINK( View, DropInsertFileHdl, Timer*, pTimer )
{
    ::std::vector< String >::const_iterator aIter( aDropFileVector.begin() );

    while( aIter != aDropFileVector.end() )
    {
        String          aCurrentDropFile( *aIter );
        INetURLObject   aURL( aCurrentDropFile );
        BOOL            bOK = FALSE;

        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        {
            String aURLStr;
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aCurrentDropFile, aURLStr );
            aURL = INetURLObject( aURLStr );
        }

        GraphicFilter*  pGraphicFilter = GetGrfFilter();
        Graphic         aGraphic;

        aCurrentDropFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

        if( !Sound::IsSoundFile( aCurrentDropFile ) )
        {
            FilterProgress* pFilterProgress = new FilterProgress( pGraphicFilter, pViewSh->GetDocSh() );

            if( !pGraphicFilter->ImportGraphic( aGraphic, aURL ) )
            {
                sal_Int8    nTempAction = ( aIter == aDropFileVector.begin() ) ? nAction : 0;
                SdrGrafObj* pGrafObj = InsertGraphic( aGraphic, nTempAction, aDropPos, NULL, NULL );

                if( pGrafObj )
                    pGrafObj->SetGraphicLink( aCurrentDropFile, String() );

                // return action from first inserted graphic
                if( aIter == aDropFileVector.begin() )
                    nAction = nTempAction;

                bOK = TRUE;
            }

            delete pFilterProgress;

            if( !bOK )
            {
                const SfxFilter*        pFoundFilter = NULL;
                SfxMedium               aSfxMedium( aCurrentDropFile, STREAM_READ | STREAM_SHARE_DENYNONE, FALSE );
                ErrCode                 nErr = SFX_APP()->GetFilterMatcher().GuessFilter(  aSfxMedium, &pFoundFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

                if( pFoundFilter && !nErr )
                {
                    ::std::vector< String > aFilterVector;
                    const String            aFilterName( pFoundFilter->GetFilterName() );
                    const String            aLowerAsciiFileName( aCurrentDropFile.ToLowerAscii() );

                    FuInsertFile::GetSupportedFilterVector( aFilterVector );

                    if( ( ::std::find( aFilterVector.begin(), aFilterVector.end(), pFoundFilter->GetMimeType() ) != aFilterVector.end() ) ||
                        aFilterName.SearchAscii( "Text" ) != STRING_NOTFOUND ||
                        aFilterName.SearchAscii( "Rich" ) != STRING_NOTFOUND ||
                        aFilterName.SearchAscii( "RTF" ) != STRING_NOTFOUND ||
                        aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".sdd") != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".sda") != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".sxd") != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".sxi") != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".std") != STRING_NOTFOUND ||
                        aLowerAsciiFileName.SearchAscii(".sti") != STRING_NOTFOUND )
                    {
                        ::sd::Window* pWin = pViewSh->GetActiveWindow();
                        SfxRequest      aReq(SID_INSERTFILE, 0, pDoc->GetItemPool());
                        SfxStringItem   aItem1( ID_VAL_DUMMY0, aCurrentDropFile ), aItem2( ID_VAL_DUMMY1, pFoundFilter->GetFilterName() );

                        aReq.AppendItem( aItem1 );
                        aReq.AppendItem( aItem2 );
                        delete( new FuInsertFile( pViewSh, pWin, this, pDoc, aReq ) );
                        bOK = TRUE;
                    }
                }
            }
        }

        if( !bOK )
        {
            if( Sound::IsSoundFile( aCurrentDropFile ) || ( nAction & DND_ACTION_LINK ) )
                static_cast< DrawViewShell* >( pViewSh )->InsertURLButton( aCurrentDropFile, aCurrentDropFile, String(), &aDropPos );
            else
            {
                if( pViewSh )
                {
                    String              aName;
                    SvStorageRef        aStor( new SvStorage( String(), STREAM_STD_READWRITE ) );
                    SvInPlaceObjectRef  aIPObj( &static_cast< SvFactory* >( SvInPlaceObject::ClassFactory() )->CreateAndInit( aCurrentDropFile, aStor ) );

                    if( aIPObj.Is() )
                    {
                        Size        aSize( aIPObj->GetVisArea( ASPECT_CONTENT ).GetSize() );
                        Rectangle   aRect;

                        if (!aSize.Width() || !aSize.Height())
                        {
                            aSize.Width()   = 1410;
                            aSize.Height()  = 1000;
                        }

                        aRect = Rectangle( aDropPos, aSize );
                        aName = pDocSh->InsertObject( aIPObj, String() )->GetObjName();

                        SdrOle2Obj* pOleObj = new SdrOle2Obj( aIPObj, aName, aRect );
                        ULONG       nOptions = SDRINSERT_SETDEFLAYER;

                        if (pViewSh != NULL)
                        {
                            OSL_ASSERT (pViewSh->GetViewShell()!=NULL);
                            SfxInPlaceClient* pIpClient =
                                pViewSh->GetViewShell()->GetIPClient();
                            if (pIpClient!=NULL && pIpClient->IsInPlaceActive())
                                nOptions |= SDRINSERT_DONTMARK;
                        }

                        InsertObject( pOleObj, *GetPageViewPvNum(0), nOptions );
                        pOleObj->SetLogicRect( aRect );
                        aIPObj->SetVisAreaSize( aRect.GetSize() );
                    }
                }
            }
        }

        ++aIter;
    }

    return 0;
}

/*************************************************************************
|*
|* Timer-Handler fuer Errorhandling beim Drop()
|*
\************************************************************************/

IMPL_LINK( View, DropErrorHdl, Timer*, pTimer )
{
    InfoBox( pViewSh->GetActiveWindow(), String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
    return 0;
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

/*************************************************************************
|*
|* Redraw sperren oder erlauben
|*
\************************************************************************/

void View::LockRedraw(BOOL bLock)
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

                CompleteRedraw(pCurrentOut, Region(aBoundRect));
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

SfxStyleSheet* View::GetStyleSheet() const
{
    return SdrView::GetStyleSheet();
}

} // end of namespace sd
