/*************************************************************************
 *
 *  $RCSfile: docshel3.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-28 11:55:47 $
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
#include "app.hrc"

#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#define ITEMID_SEARCH           SID_SEARCH_ITEM

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _OFF_OFAITEM_HXX //autogen
#include <offmgr/ofaitem.hxx>
#endif
#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif
#ifdef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _OFA_OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#pragma hdrstop

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "app.hxx"
#include "docshell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "fuspell.hxx"
#include "fusearch.hxx"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "fuexport.hxx"
#include "fuslshow.hxx"
#include "fupngdlg.hxx"

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* SFX-Requests bearbeiten
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::Execute( SfxRequest& rReq )
{
    if (pViewShell)
    {
        FuSlideShow* pFuSlideShow = pViewShell->GetSlideShow();

        if (pFuSlideShow && !pFuSlideShow->IsLivePresentation())
        {
            // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
            return;
        }
    }

    switch ( rReq.GetSlot() )
    {
        case SID_SPELLING:
        {
            if ( pViewShell )
            {
                SfxErrorContext aContext(ERRCTX_SVX_LINGU_SPELLING, String(),
                    GetWindow(), RID_SVXERRCTX, DIALOG_MGR() );

                {
                    SdView* pView = pViewShell->GetView();

                    if ( pView->IsTextEdit() )
                    {
                        pView->EndTextEdit();
                    }

                    delete pFuActual;
                    pFuActual = new FuSpell( pViewShell,
                    pViewShell->GetActiveWindow(), pView, pDoc, rReq );
                    ( (FuSpell*) pFuActual)->StartSpelling();
                    delete pFuActual;
                    pFuActual = NULL;
                }
            }
            rReq.Done();
        }
        break;

        case SID_SEARCH_ITEM:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxSearchItem* pSearchItem =
                (const SvxSearchItem*) &pReqArgs->Get(ITEMID_SEARCH);

                // ein Zuweisungsoperator am SearchItem waer nicht schlecht...
                SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                delete pAppSearchItem;
                pAppSearchItem = (SvxSearchItem*) pSearchItem->Clone();
                SD_MOD()->SetSearchItem(pAppSearchItem);
            }

            rReq.Done();
        }
        break;

        case FID_SEARCH_ON:
        {
            // Keine Aktion noetig
            rReq.Done();
        }
        break;

        case FID_SEARCH_OFF:
        {
            if ( pFuActual && pFuActual->ISA(FuSearch) )
            {
                // Suchen&Ersetzen in allen DocShells beenden
                SfxObjectShell* pFirstShell = SfxObjectShell::GetFirst();
                SfxObjectShell* pShell = pFirstShell;

                while (pShell)
                {
                    if (pShell->ISA(SdDrawDocShell))
                    {
                        ( (SdDrawDocShell*) pShell)->CancelSearching();
                    }

                    pShell = SfxObjectShell::GetNext(*pShell);

                    if (pShell == pFirstShell)
                    {
                        pShell = NULL;
                    }
                }

                delete pFuActual;
                pFuActual = NULL;
                Invalidate();
                rReq.Done();
            }
        }
        break;

        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                if ( !pFuActual || !pFuActual->ISA(FuSearch) )
                {
                    delete pFuActual;
                    SdView* pView = pViewShell->GetView();
                    pFuActual = new FuSearch( pViewShell, pViewShell->GetActiveWindow(),
                                              pView, pDoc, rReq );
                }

                if ( pFuActual && pFuActual->ISA(FuSearch) )
                {
                    const SvxSearchItem* pSearchItem =
                    (const SvxSearchItem*) &pReqArgs->Get(ITEMID_SEARCH);

                    // ein Zuweisungsoperator am SearchItem waer nicht schlecht...
                    SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                    delete pAppSearchItem;
                    pAppSearchItem = (SvxSearchItem*)pSearchItem->Clone();
                    SD_MOD()->SetSearchItem(pAppSearchItem);

                    ( (FuSearch*) pFuActual)->SearchAndReplace(pSearchItem);
                }
            }

            rReq.Done();
        }
        break;

        case SID_CLOSEDOC:
        {
//            SfxObjectShell::DoClose();
            ExecuteSlot(rReq, SfxObjectShell::GetInterface());
        }
        break;

        case SID_PACKNGO :
        {
            pFuActual = new FuPackNGoDlg( pViewShell, pViewShell->GetActiveWindow(),
                                          pViewShell->GetView(), pDoc, rReq );
            ( ( FuPackNGoDlg *) pFuActual )->DoExecute ();

            delete pFuActual;
            pFuActual = NULL;

            rReq.Done();
        }
        break;

        case SID_PUBLISH:
        {
            if ( pViewShell )
            {
                pFuActual = new FuExport( pViewShell, pViewShell->GetActiveWindow(),
                                          pViewShell->GetView(), pDoc, rReq );

                pFuActual->Activate();
                pFuActual->Deactivate();

                delete pFuActual;
                pFuActual = NULL;

                rReq.Done();
            }
        }
        break;

        case SID_GET_COLORTABLE:
        {
            //  passende ColorTable ist per PutItem gesetzt worden
            SvxColorTableItem* pColItem = (SvxColorTableItem*) GetItem( SID_COLOR_TABLE );
            XColorTable* pTable = pColItem->GetColorTable();
            rReq.SetReturnValue( OfaPtrItem( SID_GET_COLORTABLE, pTable ) );
        }
        break;

        case SID_VERSION:
        {
            const ULONG nOldSwapMode = pDoc->GetSwapGraphicsMode();

            pDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );
            ExecuteSlot( rReq, SfxObjectShell::GetInterface() );
            pDoc->SetSwapGraphicsMode( nOldSwapMode );
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* Suchmaske fuer Organizer
|*
\************************************************************************/

void SdDrawDocShell::SetOrganizerSearchMask(SfxStyleSheetBasePool* pBasePool) const
{
    pBasePool->SetSearchMask(SFX_STYLE_FAMILY_PARA,
                             SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}





