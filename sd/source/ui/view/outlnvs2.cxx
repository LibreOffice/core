/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlnvs2.cxx,v $
 * $Revision: 1.30 $
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

#include "OutlineViewShell.hxx"

#include <com/sun/star/presentation/XPresentation2.hpp>

#include "app.hrc"
#include <svx/hlnkitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svtools/eitem.hxx>
#ifndef _ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <svx/eeitem.hxx>
#include <svx/flditem.hxx>
#include <svx/editstat.hxx>
#include "optsitem.hxx"
#include <svtools/useroptions.hxx>

#include <sfx2/viewfrm.hxx>
#include "Outliner.hxx"
#include "Window.hxx"
#include "OutlineViewShell.hxx"
#include "fubullet.hxx"
#include "fuolbull.hxx"
#include "FrameView.hxx"
#include "fuzoom.hxx"
#include "fuscale.hxx"
#include "fuchar.hxx"
#include "fuinsfil.hxx"
#include "fuprobjs.hxx"
#include "futhes.hxx"
#include "futempl.hxx"
#include "fusldlg.hxx"
#include "zoomlist.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "fucushow.hxx"
#include "drawdoc.hxx"
#include "sdattr.hxx"
#include "ViewShellBase.hxx"
#include "sdabstdlg.hxx"
#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace sd {


/************************************************************************/

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void OutlineViewShell::FuTemporary(SfxRequest &rReq)
{
    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
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
                    default:
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
                SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_ZOOM_OUT:
        {
            SetCurrentFunction( FuZoom::Create(this, GetActiveWindow(), pOlView, GetDoc(), rReq) );
            // Beendet sich selbst, kein Cancel() notwendig!
            rReq.Done();
        }
        break;

        case SID_SIZE_REAL:
        {
            SetZoom( 100 );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                             GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_ZOOM_IN:
        {
            SetZoom( Max( (long) ( GetActiveWindow()->GetZoom() / 2 ), (long) GetActiveWindow()->GetMinZoom() ) );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                             GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
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
            ::Outliner* pOutl = pOutlinerView->GetOutliner();
            pOutl->SetFlatMode( !pOutl->IsFlatMode() );
            Invalidate( SID_COLORVIEW );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SELECTALL:
        {
            ::Outliner* pOutl = pOlView->GetOutliner();
            ULONG nParaCount = pOutl->GetParagraphCount();
            if (nParaCount > 0)
            {
                pOutlinerView->SelectRange( 0, (USHORT) nParaCount );
            }
            Cancel();
        }
        break;

        case SID_PRESENTATION:
        {
            pOlView->PrepareClose();

            Reference< XPresentation2 > xPresentation( GetDoc()->getPresentation() );
            if( xPresentation.is() )
                xPresentation->start();
            rReq.Done();
        }
        break;

        case SID_COLORVIEW:
        {
            ::Outliner* pOutl = pOutlinerView->GetOutliner();
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

        case SID_STYLE_EDIT:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
                Cancel();
            }

            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
        }
        break;
    }

    if(HasCurrentFunction())
        GetCurrentFunction()->Activate();

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

void OutlineViewShell::FuTemporaryModify(SfxRequest &rReq)
{
    OutlineViewModelChangeGuard aGuard( *pOlView );

    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
    USHORT nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                SvxHyperlinkItem* pHLItem =
                (SvxHyperlinkItem*) &pReqArgs->Get(SID_HYPERLINK_SETLINK);

                SvxFieldItem aURLItem(SvxURLField(pHLItem->GetURL(),
                                                  pHLItem->GetName(),
                                                  SVXURLFORMAT_REPR), EE_FEATURE_FIELD);
                ESelection aSel( pOutlinerView->GetSelection() );
                pOutlinerView->InsertField(aURLItem);
                if ( aSel.nStartPos <= aSel.nEndPos )
                    aSel.nEndPos = aSel.nStartPos + 1;
                else
                    aSel.nStartPos = aSel.nEndPos + 1;
                pOutlinerView->SetSelection( aSel );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        case SID_BULLET:
        {
            SetCurrentFunction( FuBullet::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        {
            SetCurrentFunction( FuOutlineBullet::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHAR_DLG:
        {
            SetCurrentFunction( FuChar::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_INSERTFILE:
        {
            SetCurrentFunction( FuInsertFile::Create(this, GetActiveWindow(), pOlView, GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PRESENTATIONOBJECT:
        {
            SetCurrentFunction( FuPresentationObjects::Create(this, GetActiveWindow(), pOlView, GetDoc(), rReq) );
            Cancel();
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

        case SID_SUMMARY_PAGE:
        {
            pOlView->SetSelectedPages();
            SetCurrentFunction( FuSummaryPage::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            pOlView->GetOutliner()->Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            pOlView->SetSelectedPages();
            SetCurrentFunction( FuExpandPage::Create( this, GetActiveWindow(), pOlView, GetDoc(), rReq ) );
            pOlView->GetOutliner()->Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
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
            SvxFieldItem* pFieldItem = 0;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxDateField( Date(), SVXDATETYPE_FIX ), EE_FEATURE_FIELD );
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem = new SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD );
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxExtTimeField( Time(), SVXTIMETYPE_FIX ), EE_FEATURE_FIELD );
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem = new SvxFieldItem( SvxExtTimeField(), EE_FEATURE_FIELD );
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem = new SvxFieldItem(
                            SvxAuthorField(
                                aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() )
                                , EE_FEATURE_FIELD );
                }
                break;

                case SID_INSERT_FLD_PAGE:
                    pFieldItem = new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD );
                break;

                case SID_INSERT_FLD_FILE:
                {
                    String aName;
                    if( GetDocSh()->HasName() )
                        aName = GetDocSh()->GetMedium()->GetName();
                    //else
                    //  aName = GetDocSh()->GetName();
                    pFieldItem = new SvxFieldItem( SvxExtFileField( aName ), EE_FEATURE_FIELD );
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

            if( pFieldItem )
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
                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                AbstractSdModifyFieldDlg* pDlg = pFact ? pFact->CreateSdModifyFieldDlg(GetActiveWindow(), pFldItem->GetField(), pOutlinerView->GetAttribs() ) : 0;
                if( pDlg && (pDlg->Execute() == RET_OK) )
                {
                    SvxFieldData* pField = pDlg->GetField();
                    if( pField )
                    {
                        SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );
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

                    SfxItemSet aSet( pDlg->GetItemSet() );
                    if( aSet.Count() )
                    {
                        pOutlinerView->SetAttribs( aSet );

                        ::Outliner* pOutliner = pOutlinerView->GetOutliner();
                        if( pOutliner )
                            pOutliner->UpdateFields();
                    }
                }
                delete pDlg;
            }

            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
        GetCurrentFunction()->Activate();

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


} // end of namespace sd
