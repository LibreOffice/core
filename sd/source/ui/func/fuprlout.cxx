/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fuprlout.hxx"
#include <vcl/wrkwin.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/smplhint.hxx>
#include <svl/itempool.hxx>
#include <sot/storage.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdundo.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "DrawViewShell.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "stlpool.hxx"
#include "View.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "strings.hrc"
#include "strmname.h"
#include "app.hrc"
#include "DrawDocShell.hxx"
#include "unprlout.hxx"
#include "unchss.hxx"
#include "unmovss.hxx"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include "eetext.hxx"
#include <editeng/editdata.hxx>
#include "sdabstdlg.hxx"

namespace sd
{

#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

TYPEINIT1( FuPresentationLayout, FuPoor );

#define POOL_BUFFER_SIZE        (sal_uInt16)32768
#define DOCUMENT_BUFFER_SIZE    (sal_uInt16)32768
#define DOCUMENT_TOKEN (sal_Unicode('#'))

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPresentationLayout::FuPresentationLayout (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuPresentationLayout::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuPresentationLayout( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuPresentationLayout::DoExecute( SfxRequest& rReq )
{
    // damit nicht Objekte, die gerade editiert werden oder selektiert
    // sind , verschwinden
    mpView->SdrEndTextEdit();

    if(mpView->GetSdrPageView())
    {
        mpView->UnmarkAll();
    }

    sal_Bool bError = sal_False;

    // die aktive Seite ermitteln
    sal_uInt16 nSelectedPage = SDRPAGE_NOTFOUND;
    for (sal_uInt16 nPage = 0; nPage < mpDoc->GetSdPageCount(PK_STANDARD); nPage++)
    {
        if (mpDoc->GetSdPage(nPage, PK_STANDARD)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }

    DBG_ASSERT(nSelectedPage != SDRPAGE_NOTFOUND, "keine selektierte Seite");
    SdPage* pSelectedPage = mpDoc->GetSdPage(nSelectedPage, PK_STANDARD);
    String aOldPageLayoutName(pSelectedPage->GetLayoutName());
    String aOldLayoutName(aOldPageLayoutName);
    aOldLayoutName.Erase(aOldLayoutName.SearchAscii(SD_LT_SEPARATOR));

    // wenn wir auf einer Masterpage sind, gelten die Aenderungen fuer alle
    // Seiten und Notizseiten, die das betreffende Layout benutzen
    sal_Bool bOnMaster = sal_False;
    if( mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        EditMode eEditMode =
            static_cast<DrawViewShell*>(mpViewShell)->GetEditMode();
        if (eEditMode == EM_MASTERPAGE)
            bOnMaster = sal_True;
    }
    sal_Bool bMasterPage = bOnMaster;
    sal_Bool bCheckMasters = sal_False;

    // Dialog aufrufen
    sal_Bool   bLoad = sal_False;           // tauchen neue Masterpages auf?
    String aFile;

    SfxItemSet aSet(mpDoc->GetPool(), ATTR_PRESLAYOUT_START, ATTR_PRESLAYOUT_END);

    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad));
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, bMasterPage ) );
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, bCheckMasters ) );
    aSet.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aOldLayoutName));



    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        if (pArgs->GetItemState(ATTR_PRESLAYOUT_LOAD) == SFX_ITEM_SET)
            bLoad = ((SfxBoolItem&)pArgs->Get(ATTR_PRESLAYOUT_LOAD)).GetValue();
        if( pArgs->GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE ) == SFX_ITEM_SET )
            bMasterPage = ( (SfxBoolItem&) pArgs->Get( ATTR_PRESLAYOUT_MASTER_PAGE ) ).GetValue();
        if( pArgs->GetItemState( ATTR_PRESLAYOUT_CHECK_MASTERS ) == SFX_ITEM_SET )
            bCheckMasters = ( (SfxBoolItem&) pArgs->Get( ATTR_PRESLAYOUT_CHECK_MASTERS ) ).GetValue();
        if (pArgs->GetItemState(ATTR_PRESLAYOUT_NAME) == SFX_ITEM_SET)
            aFile = ((SfxStringItem&)pArgs->Get(ATTR_PRESLAYOUT_NAME)).GetValue();
    }
    else
    {
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractSdPresLayoutDlg* pDlg = pFact ? pFact->CreateSdPresLayoutDlg(mpDocSh, mpViewShell, NULL, aSet ) : 0;

        sal_uInt16 nResult = pDlg ? pDlg->Execute() : RET_CANCEL;

        switch (nResult)
        {
            case RET_OK:
            {
                pDlg->GetAttr(aSet);
                if (aSet.GetItemState(ATTR_PRESLAYOUT_LOAD) == SFX_ITEM_SET)
                    bLoad = ((SfxBoolItem&)aSet.Get(ATTR_PRESLAYOUT_LOAD)).GetValue();
                if( aSet.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE ) == SFX_ITEM_SET )
                    bMasterPage = ( (SfxBoolItem&) aSet.Get( ATTR_PRESLAYOUT_MASTER_PAGE ) ).GetValue();
                if( aSet.GetItemState( ATTR_PRESLAYOUT_CHECK_MASTERS ) == SFX_ITEM_SET )
                    bCheckMasters = ( (SfxBoolItem&) aSet.Get( ATTR_PRESLAYOUT_CHECK_MASTERS ) ).GetValue();
                if (aSet.GetItemState(ATTR_PRESLAYOUT_NAME) == SFX_ITEM_SET)
                    aFile = ((SfxStringItem&)aSet.Get(ATTR_PRESLAYOUT_NAME)).GetValue();
            }
            break;

            default:
                bError = sal_True;
        }
        delete pDlg;
    }

    if (!bError)
    {
        mpDocSh->SetWaitCursor( sal_True );

        // Hier werden nur Masterpages ausgewechselt, d.h. die aktuelle Seite
        // bleibt aktuell. Damit beim Ein- und Ausfuegen der Masterpages nicht
        // dauernd via PageOrderChangedHint die Methode ResetActualPage gerufen
        // wird, wird jetzt blockiert.
        // That isn't quitely right. If the masterpageview is active and you are
        // removing a masterpage, it's possible that you are removing the
        // current masterpage. So you have to call ResetActualPage !
        if( mpViewShell->ISA(DrawViewShell) && !bCheckMasters )
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(sal_True);

        if (bLoad)
        {
            String aFileName = aFile.GetToken( 0, DOCUMENT_TOKEN );
            SdDrawDocument* pTempDoc = mpDoc->OpenBookmarkDoc( aFileName );

            // #69581: If I chosed the standard-template I got no filename and so I get no
            //         SdDrawDocument-Pointer. But the method SetMasterPage is able to handle
            //         a NULL-pointer as a Standard-template ( look at SdDrawDocument::SetMasterPage )
            String aLayoutName;
            if( pTempDoc )
                aLayoutName = aFile.GetToken( 1, DOCUMENT_TOKEN );

            mpDoc->SetMasterPage(nSelectedPage, aLayoutName, pTempDoc, bMasterPage, bCheckMasters);
            mpDoc->CloseBookmarkDoc();
        }
        else
        {
            // MasterPage mit dem LayoutNamen aFile aus aktuellem Doc verwenden
            mpDoc->SetMasterPage(nSelectedPage, aFile, mpDoc, bMasterPage, bCheckMasters);
        }

        // Blockade wieder aufheben
        if (mpViewShell->ISA(DrawViewShell) && !bCheckMasters )
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(sal_False);

        /*************************************************************************
        |* Falls dargestellte Masterpage sichtbar war, neu darstellen
        \************************************************************************/
        if (!bError && nSelectedPage != SDRPAGE_NOTFOUND)
        {
            if (bOnMaster)
            {
                if (mpViewShell->ISA(DrawViewShell))
                {
                    ::sd::View* pView =
                          static_cast<DrawViewShell*>(mpViewShell)->GetView();
                    sal_uInt16 nPgNum = pSelectedPage->TRG_GetMasterPage().GetPageNum();

                    if (static_cast<DrawViewShell*>(mpViewShell)->GetPageKind() == PK_NOTES)
                        nPgNum++;

                    pView->HideSdrPage();
                    pView->ShowSdrPage(pView->GetModel()->GetMasterPage(nPgNum));
                }

                // damit TabBar aktualisiert wird
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_MASTERPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else
            {
                pSelectedPage->SetAutoLayout(pSelectedPage->GetAutoLayout());
            }
        }

        // fake a mode change to repaint the page tab bar
        if( mpViewShell && mpViewShell->ISA( DrawViewShell ) )
        {
            DrawViewShell* pDrawViewSh =
                static_cast<DrawViewShell*>(mpViewShell);
            EditMode eMode = pDrawViewSh->GetEditMode();
            sal_Bool bLayer = pDrawViewSh->IsLayerModeActive();
            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }

        mpDocSh->SetWaitCursor( sal_False );
    }
}

} // end of namespace sd
