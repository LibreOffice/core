/*************************************************************************
 *
 *  $RCSfile: fuprlout.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-18 14:33:02 $
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
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif

#include "fuprlout.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "drviewsh.hxx"
#include "frmview.hxx"
#include "stlpool.hxx"
#include "sdview.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "strings.hrc"
#include "strmname.h"
#include "app.hrc"
#include "docshell.hxx"
#include "unprlout.hxx"
#include "unchss.hxx"
#include "unmovss.hxx"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "sdpreslt.hxx"
#include "drawview.hxx"
#include "eetext.hxx"
#include <svx/editdata.hxx>

#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

TYPEINIT1( FuPresentationLayout, FuPoor );

#define POOL_BUFFER_SIZE        (USHORT)32768
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPresentationLayout::FuPresentationLayout(SdViewShell* pViewSh,
                                           SdWindow* pWin, SdView* pView,
                                           SdDrawDocument* pDoc,
                                           SfxRequest& rReq)
     : FuPoor(pViewSh, pWin, pView, pDoc, rReq)

{
    // damit nicht Objekte, die gerade editiert werden oder selektiert
    // sind , verschwinden
    pView->EndTextEdit();

    USHORT nPgViewCount = pView->GetPageViewCount();
    for (USHORT nPgView = 0; nPgView < nPgViewCount; nPgView++)
    {
        pView->UnmarkAll();
    }

    BOOL bError = FALSE;

    // die aktive Seite ermitteln
    USHORT nSelectedPage = SDRPAGE_NOTFOUND;
    for (USHORT nPage = 0; nPage < pDoc->GetSdPageCount(PK_STANDARD); nPage++)
    {
        if (pDoc->GetSdPage(nPage, PK_STANDARD)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }

    DBG_ASSERT(nSelectedPage != SDRPAGE_NOTFOUND, "keine selektierte Seite");
    SdPage* pSelectedPage = pDoc->GetSdPage(nSelectedPage, PK_STANDARD);
    String aOldPageLayoutName(pSelectedPage->GetLayoutName());
    String aOldLayoutName(aOldPageLayoutName);
    aOldLayoutName.Erase(aOldLayoutName.SearchAscii(SD_LT_SEPARATOR));

    // wenn wir auf einer Masterpage sind, gelten die Aenderungen fuer alle
    // Seiten und Notizseiten, die das betreffende Layout benutzen
    BOOL bOnMaster = FALSE;
    if (pViewSh->ISA(SdDrawViewShell))
    {
        EditMode eEditMode = ((SdDrawViewShell*)pViewSh)->GetEditMode();
        if (eEditMode == EM_MASTERPAGE)
            bOnMaster = TRUE;
    }
    BOOL bMasterPage = bOnMaster;
    BOOL bCheckMasters = FALSE;

    // Dialog aufrufen
    BOOL   bLoad = FALSE;           // tauchen neue Masterpages auf?
    String aFile;
    SfxItemSet aSet(pDoc->GetPool(), ATTR_PRESLAYOUT_START, ATTR_PRESLAYOUT_END);

    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad));
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, bMasterPage ) );
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, bCheckMasters ) );
    aSet.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aOldLayoutName));

    SdPresLayoutDlg* pDlg = new SdPresLayoutDlg( pDocSh, pViewSh, NULL, aSet);
    USHORT nResult = pDlg->Execute();

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

    if (!bError)
    {
        pDocSh->SetWaitCursor( TRUE );

        // Hier werden nur Masterpages ausgewechselt, d.h. die aktuelle Seite
        // bleibt aktuell. Damit beim Ein- und Ausfuegen der Masterpages nicht
        // dauernd via PageOrderChangedHint die Methode ResetActualPage gerufen
        // wird, wird jetzt blockiert.
        // That isn't quitely right. If the masterpageview is active and you are
        // removing a masterpage, it's possible that you are removing the
        // current masterpage. So you have to call ResetActualPage !
        if( pViewShell->ISA(SdDrawViewShell) && !bCheckMasters )
            ((SdDrawView*)pView)->BlockPageOrderChangedHint(TRUE);

        if (bLoad)
        {
            SdDrawDocument* pTempDoc = pDoc->OpenBookmarkDoc(aFile);

            // #69581: If I chosed the standard-template I got no filename and so I get no
            //         SdDrawDocument-Pointer. But the method SetMasterPage is able to handle
            //         a NULL-pointer as a Standard-template ( look at SdDrawDocument::SetMasterPage )

            // Erste MasterPage aus pTempDoc verwenden
            pDoc->SetMasterPage(nSelectedPage, String(), pTempDoc, bMasterPage, bCheckMasters);
            pDoc->CloseBookmarkDoc();
        }
        else
        {
            // MasterPage mit dem LayoutNamen aFile aus aktuellem Doc verwenden
            pDoc->SetMasterPage(nSelectedPage, aFile, pDoc, bMasterPage, bCheckMasters);
        }

        // Blockade wieder aufheben
        if (pViewShell->ISA(SdDrawViewShell) && !bCheckMasters )
            ((SdDrawView*)pView)->BlockPageOrderChangedHint(FALSE);

        /*************************************************************************
        |* Falls dargestellte Masterpage sichtbar war, neu darstellen
        \************************************************************************/
        if (!bError && nSelectedPage != SDRPAGE_NOTFOUND)
        {
            if (bOnMaster)
            {
                if (pViewShell->ISA(SdDrawViewShell))
                {
                    SdView* pView = ((SdDrawViewShell*)pViewShell)->GetView();
                    USHORT nPgNum = pSelectedPage->GetMasterPage(0)->GetPageNum();

                    if (((SdDrawViewShell*)pViewShell)->GetPageKind() == PK_NOTES)
                        nPgNum++;

                    pView->HideAllPages();
                    pView->ShowMasterPagePgNum(nPgNum, Point());
                }

                // damit TabBar aktualisiert wird
                pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_MASTERPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else
            {
                pSelectedPage->SetAutoLayout(pSelectedPage->GetAutoLayout(), TRUE);
            }
        }

        // fake a mode change to repaint the page tab bar
        if( pViewShell && pViewShell->ISA( SdDrawViewShell ) )
        {
            SdDrawViewShell* pDrawViewSh = (SdDrawViewShell*)pViewShell;
            EditMode eMode = pDrawViewSh->GetEditMode();
            BOOL bLayer = pDrawViewSh->GetLayerMode();
            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }

        pDocSh->SetWaitCursor( FALSE );
    }
}



/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPresentationLayout::~FuPresentationLayout()
{
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
    SfxStyleSheetBase* pHis = pFrom->Find(aFromName,SD_LT_FAMILY);
    SfxStyleSheetBase* pMy  = pTo->Find(aToName, SD_LT_FAMILY);

    DBG_ASSERT(pHis, "neue Layoutvorlage nicht gefunden");

    // gibt's noch nicht: neu anlegen
    if (!pMy)
    {
        pMy = &(pTo->Make(aToName, SD_LT_FAMILY));
    }

    // Inhalte neu setzen
    if (pHis)
        pMy->GetItemSet().Set(pHis->GetItemSet());
}


