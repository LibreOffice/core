/*************************************************************************
 *
 *  $RCSfile: outlnvs2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-13 09:57:09 $
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
#define ITEMID_HYPERLINK    SID_HYPERLINK_SETLINK
#ifndef _SVX_HLNKITEM_HXX
#include <svx/hlnkitem.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX
#include <svx/adritem.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#include "sdoutl.hxx"
#include "sdwindow.hxx"
#include "outlnvsh.hxx"
#include "fubullet.hxx"
#include "fuolbull.hxx"
#include "frmview.hxx"
#include "fuzoom.hxx"
#include "fuscale.hxx"
#include "fuchar.hxx"
#include "fuinsfil.hxx"
#include "fuprobjs.hxx"
#include "futhes.hxx"
#include "futempl.hxx"
#include "fusldlg.hxx"
#include "zoomlist.hxx"
#include "prevchld.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "fucushow.hxx"
#include "dlgfield.hxx"
#include "drawdoc.hxx"

#include "preview.hxx"

/************************************************************************/

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void SdOutlineViewShell::FuTemporary(SfxRequest &rReq)
{
    if (pFuActual)
    {
        pFuActual->Deactivate();

        if (pFuActual != pFuOld)
        {
            delete pFuActual;
        }

        pFuActual = NULL;
    }

    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );
    USHORT nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if ( pArgs )
            {
                SvxZoomType eZT = ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetType();
                switch( eZT )
                {
                    case SVX_ZOOM_PERCENT:
                        SetZoom( (long) ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetValue() );
                        Invalidate( SID_ATTR_ZOOM );
                        break;

                    /* Gibt es hier z.Z. nicht
                    case SVX_ZOOM_OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SFX_CALLMODE_ASYNCHRON );
                        break;

                    case SVX_ZOOM_PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SFX_CALLMODE_ASYNCHRON );
                        break;

                    case SVX_ZOOM_WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON );
                        break;
                    */
                }
                rReq.Done();
            }
            else
            {
                // hier den Zoom-Dialog oeffnen
                pFuActual = new FuScale( this, pWindow, pOlView, pDoc, rReq );
            }
            Cancel();
        }
        break;

        case SID_ZOOM_OUT:
        {
            pFuActual = new FuZoom(this, pWindow, pOlView, pDoc, rReq);
            // Beendet sich selbst, kein Cancel() notwendig!
            rReq.Done();
        }
        break;

        case SID_SIZE_REAL:
        {
            SetZoom( 100 );
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                             pWindow->GetOutputSizePixel()) );
            pZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_ZOOM_IN:
        {
            SetZoom( Max( (long) ( pWindow->GetZoom() / 2 ), (long) pWindow->GetMinZoom() ) );
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                             pWindow->GetOutputSizePixel()) );
            pZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Invalidate( SID_ZOOM_OUT);
            Invalidate( SID_ZOOM_IN );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_COLLAPSE_ALL:
        {
            pOutlinerView->CollapseAll();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_COLLAPSE:
        {
            pOutlinerView->Collapse();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_EXPAND_ALL:
        {
            pOutlinerView->ExpandAll();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_EXPAND:
        {
            pOutlinerView->Expand();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_FORMAT:
        {
            Outliner* pOutl = pOutlinerView->GetOutliner();
            pOutl->SetFlatMode( !pOutl->IsFlatMode() );
            Invalidate( SID_COLORVIEW );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_BULLET:
        {
            pFuActual = new FuBullet( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        {
            pFuActual = new FuOutlineBullet( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_THESAURUS:
        {
            pFuActual = new FuThesaurus( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHAR_DLG:
        {
            pFuActual = new FuChar( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_INSERTFILE:
        {
            pFuActual = new FuInsertFile(this, pWindow, pOlView, pDoc, rReq);
            Cancel();
        }
        break;

        case SID_PRESENTATIONOBJECT:
        {
            pFuActual = new FuPresentationObjects(this, pWindow, pOlView, pDoc, rReq);
            Cancel();
        }
        break;

        case SID_SELECTALL:
        {
            Outliner* pOutl = pOlView->GetOutliner();
            ULONG nParaCount = pOutl->GetParagraphCount();
            if (nParaCount > 0)
            {
                pOutlinerView->SelectRange(0L, nParaCount);
            }
            Cancel();
        }
        break;

        case SID_PRESENTATION:
        {
            // den Outliner-Inhalt ins Draw-Model schreiben
            pOlView->PrepareClose();

            // Zum Zeichentisch wechseln
            pFrameView->SetPresentationViewShellId(SID_VIEWSHELL2);
            pFrameView->SetSlotId(SID_PRESENTATION);
            pFrameView->SetPageKind(PK_STANDARD);
            GetViewFrame()->GetDispatcher()->Execute(
                SID_VIEWSHELL0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
            rReq.Done();
        }
        break;

        case SID_COLORVIEW:
        {
            Outliner* pOutl = pOutlinerView->GetOutliner();
            ULONG nCntrl = pOutl->GetControlWord();

            if ( !(nCntrl & EE_CNTRL_NOCOLORS) )
            {
                // Farbansicht ist eingeschaltet: ausschalten
                pOutl->SetControlWord(nCntrl | EE_CNTRL_NOCOLORS);
            }
            else
            {
                // Farbansicht ist ausgeschaltet: einschalten
                pOutl->SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);
            }

            InvalidateWindows();
            Invalidate( SID_COLORVIEW );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_PREVIEW_WIN:
        {
            BOOL bPreview = FALSE;

            if ( rReq.GetArgs() )
            {
                bPreview = ((const SfxBoolItem&) (rReq.GetArgs()->Get(SID_PREVIEW_WIN))).GetValue();
                GetViewFrame()->SetChildWindow(SdPreviewChildWindow::GetChildWindowId(),
                                          bPreview);
            }
            else
            {
                USHORT nId = SdPreviewChildWindow::GetChildWindowId();
                bPreview = !SfxBoolItem(SID_PREVIEW_WIN, GetViewFrame()->HasChildWindow(nId)).GetValue();
                GetViewFrame()->ToggleChildWindow(SdPreviewChildWindow::GetChildWindowId() );
            }

            pFrameView->SetShowPreviewInOutlineMode(bPreview);

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate(SID_PREVIEW_WIN);
            rBindings.Invalidate(SID_PREVIEW_QUALITY_COLOR);
            rBindings.Invalidate(SID_PREVIEW_QUALITY_GRAYSCALE);
            rBindings.Invalidate(SID_PREVIEW_QUALITY_BLACKWHITE);

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_SET_DEFAULT:
        {
            // 1. Selektion merken (kriegt die eselige EditEngine nicht selbst
            //    auf die Reihe!)
            // 2. Update auf False (sonst flackert's noch staerker
            // an allen selektierten Absaetzen:
            //  a. deren Vorlage nochmal setzen, um absatzweite harte Attribute
            //     zu entfernen
            //  b. harte Zeichenattribute loeschen
            // 3. Update auf True und Selektion wieder setzen
            /*
            ESelection aEsel= pOutlinerView->GetSelection();
            Outliner* pOutl = pOutlinerView->GetOutliner();
            pOutl->SetUpdateMode(FALSE);
            List* pSelectedParas = pOutlinerView->CreateSelectionList();
            Paragraph* pPara = (Paragraph*)pSelectedParas->First();
            while (pPara)
            {
                ULONG nParaPos = pOutl->GetAbsPos(pPara);
                String aName;
                SfxStyleFamily aFamily;
                pOutl->GetStyleSheet(nParaPos, aName, aFamily);
                pOutl->SetStyleSheet(nParaPos, aName, aFamily);
                pOutl->QuickRemoveCharAttribs(nParaPos);
                pPara = (Paragraph*)pSelectedParas->Next();
            }
            delete pSelectedParas;
            pOutl->SetUpdateMode(TRUE);
            pOutlinerView->SetSelection(aEsel);
            */
            pOutlinerView->RemoveAttribs(TRUE); // TRUE = auch Absatzattribute
            Cancel();
            rReq.Done();
        }
        break;

        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxFieldItem* pFieldItem = pOutlinerView->GetFieldAtSelection();

                if (pFieldItem && pFieldItem->GetField()->ISA(SvxURLField))
                {
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOutlinerView->GetSelection();
                    aSel.nEndPos++;
                    pOutlinerView->SetSelection(aSel);
                }

                SvxHyperlinkItem* pHLItem =
                (SvxHyperlinkItem*) &pReqArgs->Get(ITEMID_HYPERLINK);

                SvxFieldItem aURLItem(SvxURLField(pHLItem->GetURL(),
                                                  pHLItem->GetName(),
                                                  SVXURLFORMAT_REPR));
                pOutlinerView->InsertField(aURLItem);
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_FILE:
        {
            SvxFieldItem* pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxDateField( Date(), SVXDATETYPE_FIX ) );
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem = new SvxFieldItem( SvxDateField() );
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxExtTimeField( Time(), SVXTIMETYPE_FIX ) );
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem = new SvxFieldItem( SvxExtTimeField() );
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvxAddressItem aAdrItem;
                    pFieldItem = new SvxFieldItem( SvxAuthorField( aAdrItem ) );
                }
                break;

                case SID_INSERT_FLD_PAGE:
                    pFieldItem = new SvxFieldItem( SvxPageField() );
                break;

                case SID_INSERT_FLD_FILE:
                {
                    String aName;
                    if( pDocSh->HasName() )
                        aName = pDocSh->GetMedium()->GetName();
                    //else
                    //  aName = pDocSh->GetName();
                    pFieldItem = new SvxFieldItem( SvxExtFileField( aName ) );
                }
                break;
            }

            const SvxFieldItem* pOldFldItem = pOutlinerView->GetFieldAtSelection();

            if( pOldFldItem && ( pOldFldItem->GetField()->ISA( SvxURLField ) ||
                                pOldFldItem->GetField()->ISA( SvxDateField ) ||
                                pOldFldItem->GetField()->ISA( SvxTimeField ) ||
                                pOldFldItem->GetField()->ISA( SvxExtTimeField ) ||
                                pOldFldItem->GetField()->ISA( SvxExtFileField ) ||
                                pOldFldItem->GetField()->ISA( SvxAuthorField ) ||
                                pOldFldItem->GetField()->ISA( SvxPageField ) ) )
            {
                // Feld selektieren, so dass es beim Insert geloescht wird
                ESelection aSel = pOutlinerView->GetSelection();
                if( aSel.nStartPos == aSel.nEndPos )
                    aSel.nEndPos++;
                pOutlinerView->SetSelection( aSel );
            }

            pOutlinerView->InsertField( *pFieldItem );

            delete pFieldItem;

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            const SvxFieldItem* pFldItem = pOutlinerView->GetFieldAtSelection();

            if( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                                pFldItem->GetField()->ISA( SvxAuthorField ) ||
                                pFldItem->GetField()->ISA( SvxExtFileField ) ||
                                pFldItem->GetField()->ISA( SvxExtTimeField ) ) )
            {
                // Dialog...
                SdModifyFieldDlg aDlg( pWindow, pFldItem->GetField() );
                if( aDlg.Execute() == RET_OK )
                {
                    SvxFieldData* pField = aDlg.GetField();
                    if( pField )
                    {
                        SvxFieldItem aFieldItem( *pField );
                        //pOLV->DeleteSelected(); <-- fehlt leider !
                        // Feld selektieren, so dass es beim Insert geloescht wird
                        ESelection aSel = pOutlinerView->GetSelection();
                        BOOL bSel = TRUE;
                        if( aSel.nStartPos == aSel.nEndPos )
                        {
                            bSel = FALSE;
                            aSel.nEndPos++;
                        }
                        pOutlinerView->SetSelection( aSel );

                        pOutlinerView->InsertField( aFieldItem );

                        // Selektion wird wieder in den Ursprungszustand gebracht
                        if( !bSel )
                            aSel.nEndPos--;
                        pOutlinerView->SetSelection( aSel );

                        delete pField;
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_STYLE_EDIT:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if( rReq.GetArgs() )
            {
                pFuActual = new FuTemplate( this, pWindow, pOlView, pDoc, rReq );
                Cancel();
            }

            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            pFuActual = new FuSlideShowDlg( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            pFuActual = new FuCustomShowDlg( this, pWindow, pOlView, pDoc, rReq );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            pOlView->SetSelectedPages();
            pFuActual = new FuSummaryPage( this, pWindow, pOlView, pDoc, rReq );
            pOlView->GetOutliner()->Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            pOlView->SetSelectedPages();
            pFuActual = new FuExpandPage( this, pWindow, pOlView, pDoc, rReq );
            pOlView->GetOutliner()->Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
        }
        break;
    }

    if (pFuActual)
    {
        pFuActual->Activate();
    }

    Invalidate( SID_OUTLINE_COLLAPSE_ALL );
    Invalidate( SID_OUTLINE_COLLAPSE );
    Invalidate( SID_OUTLINE_EXPAND_ALL );
    Invalidate( SID_OUTLINE_EXPAND );

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OUTLINE_LEFT );
    rBindings.Invalidate( SID_OUTLINE_RIGHT );
    rBindings.Invalidate( SID_OUTLINE_UP );
    rBindings.Invalidate( SID_OUTLINE_DOWN );

    Invalidate( SID_OUTLINE_FORMAT );
    Invalidate( SID_COLORVIEW );
    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
}


