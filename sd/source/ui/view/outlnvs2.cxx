/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlnvs2.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:19:57 $
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

#include "OutlineViewShell.hxx"

#include <com/sun/star/presentation/XPresentation2.hpp>

#include "app.hrc"
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
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#include <sfx2/viewfrm.hxx>

#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_FU_BULLET_HXX
#include "fubullet.hxx"
#endif
#ifndef SD_FU_OUTLINE_BULLET_HXX
#include "fuolbull.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_FU_ZOOM_HXX
#include "fuzoom.hxx"
#endif
#ifndef SD_FU_SCALE_HXX
#include "fuscale.hxx"
#endif
#ifndef SD_FU_CHAR_HXX
#include "fuchar.hxx"
#endif
#ifndef SD_FU_INSERT_FILE_HXX
#include "fuinsfil.hxx"
#endif
#ifndef SD_FU_PRESENTATION_OBJECTS_HXX
#include "fuprobjs.hxx"
#endif
#ifndef SD_FU_THESAURUS_HXX
#include "futhes.hxx"
#endif
#ifndef SD_FU_TEMPLATE_HXX
#include "futempl.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_DLG_HXX
#include "fusldlg.hxx"
#endif
#include "zoomlist.hxx"
#ifndef SD_FU_EXPAND_PAGE_HXX
#include "fuexpand.hxx"
#endif
#ifndef SD_FU_SUMMARY_PAGE_HXX
#include "fusumry.hxx"
#endif
#ifndef SD_FU_CUSTOM_SHOW_DLG_HXX
#include "fucushow.hxx"
#endif
#include "drawdoc.hxx"
#include "sdattr.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
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
