/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuprlout.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:47:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fuprlout.hxx"

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif

#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "stlpool.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
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
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "eetext.hxx"
#include <svx/editdata.hxx>
#include "sdabstdlg.hxx"

namespace sd
{

#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

TYPEINIT1( FuPresentationLayout, FuPoor );

#define POOL_BUFFER_SIZE        (USHORT)32768
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768
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

    BOOL bError = FALSE;

    // die aktive Seite ermitteln
    USHORT nSelectedPage = SDRPAGE_NOTFOUND;
    for (USHORT nPage = 0; nPage < mpDoc->GetSdPageCount(PK_STANDARD); nPage++)
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
    BOOL bOnMaster = FALSE;
    if( mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        EditMode eEditMode =
            static_cast<DrawViewShell*>(mpViewShell)->GetEditMode();
        if (eEditMode == EM_MASTERPAGE)
            bOnMaster = TRUE;
    }
    BOOL bMasterPage = bOnMaster;
    BOOL bCheckMasters = FALSE;

    // Dialog aufrufen
    BOOL   bLoad = FALSE;           // tauchen neue Masterpages auf?
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

        USHORT nResult = pDlg ? pDlg->Execute() : RET_CANCEL;

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
                bError = TRUE;
        }
        delete pDlg;
    }

    if (!bError)
    {
        mpDocSh->SetWaitCursor( TRUE );

        // Hier werden nur Masterpages ausgewechselt, d.h. die aktuelle Seite
        // bleibt aktuell. Damit beim Ein- und Ausfuegen der Masterpages nicht
        // dauernd via PageOrderChangedHint die Methode ResetActualPage gerufen
        // wird, wird jetzt blockiert.
        // That isn't quitely right. If the masterpageview is active and you are
        // removing a masterpage, it's possible that you are removing the
        // current masterpage. So you have to call ResetActualPage !
        if( mpViewShell->ISA(DrawViewShell) && !bCheckMasters )
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(TRUE);

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
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(FALSE);

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
                    USHORT nPgNum = pSelectedPage->TRG_GetMasterPage().GetPageNum();

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
            BOOL bLayer = pDrawViewSh->IsLayerModeActive();
            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }

        mpDocSh->SetWaitCursor( FALSE );
    }
}

/*************************************************************************
|*
|* Layoutvorlage von einem StyleSheetPool in einen anderen uebertragen
|*
\************************************************************************/

void FuPresentationLayout::TransferLayoutTemplate(String aFromName,
                                                  String aToName,
                                                  SfxStyleSheetBasePool* pFrom,
                                                  SfxStyleSheetBasePool* pTo)
{
    SfxStyleSheetBase* pHis = pFrom->Find(aFromName,SD_STYLE_FAMILY_MASTERPAGE);
    SfxStyleSheetBase* pMy  = pTo->Find(aToName, SD_STYLE_FAMILY_MASTERPAGE);

    DBG_ASSERT(pHis, "neue Layoutvorlage nicht gefunden");

    // gibt's noch nicht: neu anlegen
    if (!pMy)
    {
        pMy = &(pTo->Make(aToName, SD_STYLE_FAMILY_MASTERPAGE));
    }

    // Inhalte neu setzen
    if (pHis)
        pMy->GetItemSet().Set(pHis->GetItemSet());
}


} // end of namespace sd
