/*************************************************************************
 *
 *  $RCSfile: editsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/cntritem.hxx>
#include <svx/chardlg.hxx>
#include <svx/crsditem.hxx>
#include <svx/editeng.hxx>
#include <svx/editview.hxx>
#include <svx/eeitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/flditem.hxx>
#include <svx/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/srchitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <so3/pastedlg.hxx>
#include <sot/exchange.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>


#define GLOBALOVERFLOW
#include <segmentc.hxx>


#define _EDITSH_CXX
#include "editsh.hxx"

#include "scresid.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "viewutil.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "namepast.hxx"
#include "reffind.hxx"
#include "tabvwsh.hxx"

#define ScEditShell
#include "scslots.hxx"

SEG_EOFGLOBALS()

#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)

TYPEINIT1( ScEditShell, SfxShell );
#pragma SEG_FUNCDEF(editsh_0a)

SFX_IMPL_INTERFACE(ScEditShell, SfxShell, ScResId(SCSTR_EDITSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_EDIT) );
}


#pragma SEG_FUNCDEF(editsh_01)

ScEditShell::ScEditShell(EditView* pView, ScViewData* pData) :
    pEditView       (pView),
    pViewData       (pData),
    bIsInsertMode   (TRUE)
{
    SetPool( pEditView->GetEditEngine()->GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->GetEditEngine()->GetUndoManager() );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("EditCell")));
}

#pragma SEG_FUNCDEF(editsh_02)

ScEditShell::~ScEditShell()
{
}

#pragma SEG_FUNCDEF(editsh_08)

void ScEditShell::SetEditView(EditView* pView)
{
    pEditView = pView;
    pEditView->SetInsertMode( bIsInsertMode );
    SetPool( pEditView->GetEditEngine()->GetEmptyItemSet().GetPool() );
    SetUndoManager( &pEditView->GetEditEngine()->GetUndoManager() );
}

#pragma SEG_FUNCDEF(editsh_04)

void ScEditShell::Execute( SfxRequest& rReq )
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();
    SfxBindings&        rBindings   = SFX_BINDINGS();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    DBG_ASSERT(pHdl,"kein ScInputHandler");

    EditView* pTopView   = pHdl->GetTopView();      // hat Eingabezeile den Focus?
    EditView* pTableView = pHdl->GetTableView();

    DBG_ASSERT(pTableView,"keine EditView :-(");

    EditEngine* pEngine = pTableView->GetEditEngine();

    pHdl->DataChanging();
    BOOL bSetSelIsRef = FALSE;

    switch ( nSlot )
    {
        case FID_INS_CELL_CONTENTS: // Insert-Taste, weil als Acc definiert
            bIsInsertMode = !pTableView->IsInsertMode();
            pTableView->SetInsertMode( bIsInsertMode );
            if (pTopView)
                pTopView->SetInsertMode( bIsInsertMode );
            SFX_BINDINGS().Invalidate( SID_ATTR_INSERT );
            break;

        case SID_ATTR_INSERT:
            if ( pReqArgs )
            {
                bIsInsertMode = ((const SfxBoolItem&)pReqArgs->Get(nSlot)).GetValue();
                pTableView->SetInsertMode( bIsInsertMode );
                if (pTopView)
                    pTopView->SetInsertMode( bIsInsertMode );
                SFX_BINDINGS().Invalidate( SID_ATTR_INSERT );
            }
            break;

        case SID_COPY:
            pTableView->Copy();
            break;

        case SID_CUT:
            pTableView->Cut();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_PASTE:
            pTableView->PasteSpecial();
            if (pTopView)
                pTopView->Paste();
            break;

        case SID_DELETE:
            pTableView->DeleteSelected();
            if (pTopView)
                pTopView->DeleteSelected();
            break;

        case SID_CELL_FORMAT_RESET:                 // "Standard"
            pTableView->RemoveAttribs(TRUE);        // TRUE: auch Absatz-Attribute
            if (pTopView)
                pTopView->RemoveAttribs(TRUE);
            break;

        case FID_PASTE_CONTENTS:
            {
                SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
                if (pClipObj.Is())
                {
                    SvPasteObjectDialog* pDlg = new SvPasteObjectDialog;
                    pDlg->Insert( FORMAT_STRING, ScResId( SCSTR_CLIP_STRING ) );
                    pDlg->Insert( FORMAT_RTF,    ScResId( SCSTR_CLIP_RTF ) );

                    ULONG nFormat = pDlg->Execute( pViewData->GetDialogParent(), pClipObj );
                    DELETEZ(pDlg);

                    // while the dialog was open, edit mode may have been stopped
                    if (!SC_MOD()->IsInputMode())
                    {
                        Sound::Beep();
                        return;
                    }

                    if (nFormat > 0)
                    {
                        if (FORMAT_STRING == nFormat)
                            pTableView->Paste();
                        else
                            pTableView->PasteSpecial();

                        if (pTopView)
                            pTopView->Paste();
                    }

                    if (pTopView)
                        pTopView->GetWindow()->GrabFocus();
                }
            }
            break;

        case SID_SELECTALL:
            {
                USHORT nPar = pEngine->GetParagraphCount();
                if (nPar)
                {
                    xub_StrLen nLen = pEngine->GetTextLen(nPar-1);
                    pTableView->SetSelection(ESelection(0,0,nPar-1,nLen));
                    if (pTopView)
                        pTopView->SetSelection(ESelection(0,0,nPar-1,nLen));
                }
            }
            break;

        case SID_CHARMAP:
            {
                const SvxFontItem& rItem = (const SvxFontItem&)
                            pTableView->GetAttribs().Get(EE_CHAR_FONTINFO);

                String aString;
                SvxFontItem aNewItem( EE_CHAR_FONTINFO );

                BOOL bOk = ScViewUtil::ExecuteCharMap( rItem, aNewItem, aString );

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    return;
                }

                if (bOk)
                {
                    SfxItemSet aSet( pTableView->GetEmptyItemSet() );
                    aSet.Put( aNewItem );
                    //  SetAttribs an der View selektiert ein Wort, wenn nichts selektiert ist
                    pTableView->GetEditEngine()->QuickSetAttribs( aSet, pTableView->GetSelection() );
                    pTableView->InsertText(aString);
                    if (pTopView)
                        pTopView->InsertText(aString);
                }

                if (pTopView)
                    pTopView->GetWindow()->GrabFocus();
            }
            break;

        case FID_INSERT_NAME:
            {
                ScDocument*     pDoc = pViewData->GetDocument();
                ScNamePasteDlg* pDlg = new ScNamePasteDlg( pViewData->GetDialogParent(),
                                                pDoc->GetRangeName(), FALSE );
                                                // "Liste" disablen

                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    delete pDlg;
                    return;
                }

                if ( nRet == BTN_PASTE_NAME )
                {
                    String aName = pDlg->GetSelectedName();
                    pTableView->InsertText(aName);
                    if (pTopView)
                        pTopView->InsertText(aName);
                }
                delete pDlg;

                if (pTopView)
                    pTopView->GetWindow()->GrabFocus();
            }
            break;

        case SID_CHAR_DLG:
            {
                SfxItemSet aAttrs( pTableView->GetAttribs() );

                SfxObjectShell* pObjSh = pViewData->GetSfxDocShell();
                const SvxFontListItem* pFontListItem =
                    (const SvxFontListItem*) pObjSh->GetItem( SID_ATTR_CHAR_FONTLIST);

                SfxSingleTabDialog* pDlg
                    = new SfxSingleTabDialog( pViewData->GetViewShell()->GetViewFrame(),
                                                pViewData->GetDialogParent(),
                                                aAttrs, RID_SCDLG_EDITCHAR, FALSE );
                SfxTabPage* pPage = SvxCharStdPage::Create( pDlg, aAttrs );
                ((SvxCharStdPage*)pPage)->SetFontList( *pFontListItem );
                pDlg->SetTabPage( pPage );

                short nRet = pDlg->Execute();
                // pDlg is needed below

                // while the dialog was open, edit mode may have been stopped
                if (!SC_MOD()->IsInputMode())
                {
                    Sound::Beep();
                    delete pDlg;
                    return;
                }

                if ( nRet == RET_OK )
                {
                    const SfxItemSet* pOut = pDlg->GetOutputItemSet();
                    pTableView->SetAttribs( *pOut );
                }
                delete pDlg;
            }
            break;

        case SID_TOGGLE_REL:
            {
                BOOL bOk = FALSE;
                if (pEngine->GetParagraphCount() == 1)
                {
                    String aText = pEngine->GetText();
                    ESelection aSel = pEditView->GetSelection();    // aktuelle View

                    ScRefFinder aFinder( aText, pViewData->GetDocument() );
                    aFinder.ToggleRel( aSel.nStartPos, aSel.nEndPos );
                    if (aFinder.GetFound())
                    {
                        String aNew = aFinder.GetText();
                        ESelection aSel( 0,aFinder.GetSelStart(), 0,aFinder.GetSelEnd() );
                        pEngine->SetText( aNew );
                        pTableView->SetSelection( aSel );
                        if ( pTopView )
                        {
                            pTopView->GetEditEngine()->SetText( aNew );
                            pTopView->SetSelection( aSel );
                        }
                        bOk = TRUE;

                        //  Referenz wird selektiert -> beim Tippen nicht ueberschreiben
                        bSetSelIsRef = TRUE;
                    }
                }
                if (!bOk)
                    Sound::Beep();              // keine Referenzen oder mehrere Absätze
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName     = pHyper->GetName();
                    const String& rURL      = pHyper->GetURL();
                    const String& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    BOOL bDone = FALSE;
                    if ( eMode == HLINK_DEFAULT || eMode == HLINK_FIELD )
                    {
                        const SvxURLField* pURLField = GetURLField();
                        if ( pURLField )
                        {
                            //  altes Feld selektieren

                            ESelection aSel = pTableView->GetSelection();
                            aSel.Adjust();
                            aSel.nEndPara = aSel.nStartPara;
                            aSel.nEndPos = aSel.nStartPos + 1;
                            pTableView->SetSelection( aSel );

                            //  neues Feld einfuegen

                            SvxURLField aURLField( rURL, rName, SVXURLFORMAT_REPR );
                            aURLField.SetTargetFrame( rTarget );
                            SvxFieldItem aURLItem( aURLField );
                            pTableView->InsertField( aURLItem );
                            pTableView->SetSelection( aSel );       // select inserted field

                            //  #57254# jetzt doch auch Felder in der Top-View

                            if ( pTopView )
                            {
                                aSel = pTopView->GetSelection();
                                aSel.nEndPara = aSel.nStartPara;
                                aSel.nEndPos = aSel.nStartPos + 1;
                                pTopView->SetSelection( aSel );
                                pTopView->InsertField( aURLItem );
                                pTopView->SetSelection( aSel );     // select inserted field
                            }

                            bDone = TRUE;
                        }
                    }

                    if (!bDone)
                    {
                        pViewData->GetViewShell()->
                            InsertURL( rName, rURL, rTarget, (USHORT) eMode );

                        //  InsertURL an der ViewShell schaltet bei "Button"
                        //  die EditShell ab, darum sofort return

                        return;
                    }
                }
            }
            break;
    }

    pHdl->DataChanged();
    if (bSetSelIsRef)
        pHdl->SetSelIsRef(TRUE);
}

#pragma SEG_FUNCDEF(editsh_05)

void __EXPORT ScEditShell::GetState( SfxItemSet& rSet )
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_ATTR_INSERT:   // Statuszeile
                {
                    if ( pActiveView )
                        rSet.Put( SfxBoolItem( nWhich, pActiveView->IsInsertMode() ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, 42 ) );
                }
                break;

            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    const SvxURLField* pURLField = GetURLField();
                    if ( pURLField )
                    {
                        aHLinkItem.SetName( pURLField->GetRepresentation() );
                        aHLinkItem.SetURL( pURLField->GetURL() );
                        aHLinkItem.SetTargetFrame( pURLField->GetTargetFrame() );
                    }
                    else if ( pActiveView )
                    {
                        // use selected text as name for urls
                        String sReturn = pActiveView->GetSelected();
                        sReturn.Erase(255);
                        sReturn.EraseTrailingChars();
                        aHLinkItem.SetName(sReturn);
                    }
                    rSet.Put(aHLinkItem);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

#pragma SEG_FUNCDEF(editsh_0c)

const SvxURLField* ScEditShell::GetURLField()
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    EditView* pActiveView = pHdl ? pHdl->GetActiveView() : pEditView;
    if ( pActiveView )
    {
        const SvxFieldItem* pFieldItem = pActiveView->GetFieldAtSelection();
        if (pFieldItem)
        {
            const SvxFieldData* pField = pFieldItem->GetField();
            if ( pField && pField->ISA(SvxURLField) )
                return (const SvxURLField*)pField;
        }
    }

    return NULL;
}

#pragma SEG_FUNCDEF(editsh_09)

void __EXPORT ScEditShell::GetClipState( SfxItemSet& rSet )
{
    BOOL bPaste = FALSE;
    SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
    if (pClipObj.Is())
    {
        const SvDataTypeList& rTypeLst = pClipObj->GetTypeList();

        if( rTypeLst.Get( FORMAT_STRING ) || rTypeLst.Get( FORMAT_RTF ) )
            bPaste = TRUE;
    }

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_PASTE:
            case FID_PASTE_CONTENTS:
                if( !bPaste )
                    rSet.DisableItem( nWhich );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void lcl_InvalidateUnder( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
}

#pragma SEG_FUNCDEF(editsh_06)

void ScEditShell::ExecuteAttr(SfxRequest& rReq)
{
    SfxItemSet          aSet( pEditView->GetEmptyItemSet() );
    SfxBindings&        rBindings   = SFX_BINDINGS();
    const SfxItemSet*   pArgs       = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

    switch ( nSlot )
    {
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_COLOR:
            {
                if (pArgs)
                {
                    aSet.Put( pArgs->Get( pArgs->GetPool()->GetWhich( nSlot ) ) );
                    rBindings.Invalidate( nSlot );
                }
            }
            break;

        //  Toggles

        case SID_ATTR_CHAR_WEIGHT:
            {
                BOOL bOld = ((const SvxWeightItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_WEIGHT)).GetValue() > WEIGHT_NORMAL;
                aSet.Put( SvxWeightItem( bOld ? WEIGHT_NORMAL : WEIGHT_BOLD, EE_CHAR_WEIGHT ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_POSTURE:
            {
                BOOL bOld = ((const SvxPostureItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_ITALIC)).GetValue() != ITALIC_NONE;
                aSet.Put( SvxPostureItem( bOld ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ULINE_VAL_NONE:
            aSet.Put( SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE ) );
            lcl_InvalidateUnder( rBindings );
            break;

        case SID_ATTR_CHAR_UNDERLINE:   // Toggles
        case SID_ULINE_VAL_SINGLE:
        case SID_ULINE_VAL_DOUBLE:
        case SID_ULINE_VAL_DOTTED:
            {
                FontUnderline eOld = ((const SvxUnderlineItem&) pEditView->
                                    GetAttribs().Get(EE_CHAR_UNDERLINE)).GetUnderline();
                FontUnderline eNew = eOld;
                switch (nSlot)
                {
                    case SID_ATTR_CHAR_UNDERLINE:
                        eNew = ( eOld != UNDERLINE_NONE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                        break;
                    case SID_ULINE_VAL_SINGLE:
                        eNew = ( eOld == UNDERLINE_SINGLE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                        break;
                    case SID_ULINE_VAL_DOUBLE:
                        eNew = ( eOld == UNDERLINE_DOUBLE ) ? UNDERLINE_NONE : UNDERLINE_DOUBLE;
                        break;
                    case SID_ULINE_VAL_DOTTED:
                        eNew = ( eOld == UNDERLINE_DOTTED ) ? UNDERLINE_NONE : UNDERLINE_DOTTED;
                        break;
                }
                aSet.Put( SvxUnderlineItem( eNew, EE_CHAR_UNDERLINE ) );
                lcl_InvalidateUnder( rBindings );
            }
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            {
                BOOL bOld = ((const SvxCrossedOutItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_STRIKEOUT)).GetValue() != STRIKEOUT_NONE;
                aSet.Put( SvxCrossedOutItem( bOld ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_SHADOWED:
            {
                BOOL bOld = ((const SvxShadowedItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_SHADOW)).GetValue();
                aSet.Put( SvxShadowedItem( !bOld, EE_CHAR_SHADOW ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_ATTR_CHAR_CONTOUR:
            {
                BOOL bOld = ((const SvxContourItem&)pEditView->GetAttribs().
                                Get(EE_CHAR_OUTLINE)).GetValue();
                aSet.Put( SvxContourItem( !bOld, EE_CHAR_OUTLINE ) );
                rBindings.Invalidate( nSlot );
            }
            break;

        case SID_SET_SUPER_SCRIPT:
            {
                SvxEscapement eOld = (SvxEscapement) ((const SvxEscapementItem&)
                        pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT)).GetEnumValue();
                SvxEscapement eNew = (eOld == SVX_ESCAPEMENT_SUPERSCRIPT) ?
                                        SVX_ESCAPEMENT_OFF : SVX_ESCAPEMENT_SUPERSCRIPT;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
        case SID_SET_SUB_SCRIPT:
            {
                SvxEscapement eOld = (SvxEscapement) ((const SvxEscapementItem&)
                        pEditView->GetAttribs().Get(EE_CHAR_ESCAPEMENT)).GetEnumValue();
                SvxEscapement eNew = (eOld == SVX_ESCAPEMENT_SUBSCRIPT) ?
                                        SVX_ESCAPEMENT_OFF : SVX_ESCAPEMENT_SUBSCRIPT;
                aSet.Put( SvxEscapementItem( eNew, EE_CHAR_ESCAPEMENT ) );
                rBindings.Invalidate( nSlot );
            }
            break;
    }

    //
    //  anwenden
    //

    EditEngine* pEngine = pEditView->GetEditEngine();
    BOOL bOld = pEngine->GetUpdateMode();
    pEngine->SetUpdateMode(FALSE);

    pEditView->SetAttribs( aSet );

    pEngine->SetUpdateMode(bOld);
    pEditView->Invalidate();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    pHdl->SetModified();

    rReq.Done();
}

#pragma SEG_FUNCDEF(editsh_07)

void ScEditShell::GetAttrState(SfxItemSet &rSet)
{
    SfxItemSet aAttribs = pEditView->GetAttribs();
    rSet.Put( aAttribs );

    //  Unterstreichung

    SfxItemState eState = aAttribs.GetItemState( EE_CHAR_UNDERLINE, TRUE );
    if ( eState == SFX_ITEM_DONTCARE )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontUnderline eUnderline = ((const SvxUnderlineItem&)
                    aAttribs.Get(EE_CHAR_UNDERLINE)).GetUnderline();
        USHORT nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case UNDERLINE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case UNDERLINE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case UNDERLINE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rSet.Put( SfxBoolItem( nId, TRUE ) );
    }

    //! Testen, ob Klammer-Hervorhebung aktiv ist !!!!
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if ( pHdl && pHdl->IsFormulaMode() )
        rSet.ClearItem( EE_CHAR_WEIGHT );   // hervorgehobene Klammern hier nicht
}

#pragma SEG_FUNCDEF(editsh_0b)

String ScEditShell::GetSelectionText( BOOL bWholeWord )
{
    String aStrSelection;

    if ( bWholeWord )
    {
        EditEngine* pEngine = pEditView->GetEditEngine();
        ESelection  aSel = pEditView->GetSelection();
        String      aStrCurrentDelimiters = pEngine->GetWordDelimiters();

        pEngine->SetWordDelimiters( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(" .,;\"'")) );
        aStrSelection = pEngine->GetWord( aSel.nEndPara, aSel.nEndPos );
        pEngine->SetWordDelimiters( aStrCurrentDelimiters );
    }
    else
    {
        aStrSelection = pEditView->GetSelected();
    }

    return aStrSelection;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.85  2000/09/17 14:09:31  willem.vandorp
    OpenOffice header added.

    Revision 1.84  2000/08/31 16:38:45  willem.vandorp
    Header and footer replaced

    Revision 1.83  2000/07/21 18:21:23  nn
    select url field after inserting

    Revision 1.82  2000/07/21 10:35:15  nn
    #75683# SID_HYPERLINK_GETLINK: use selection as text if no link selected

    Revision 1.81  2000/05/09 18:30:15  nn
    use IMPL_INTERFACE macro without IDL

    Revision 1.80  2000/04/14 08:31:37  nn
    unicode changes

    Revision 1.79  2000/03/31 17:27:08  nn
    #74687# after executing dialogs, check for edit mode

    Revision 1.78  2000/02/11 12:37:23  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.77  1999/09/24 15:59:13  nn
    hlnkitem.hxx moved to svx

    Revision 1.76  1999/06/02 19:41:34  ANK
    #66547# SubShells


      Rev 1.75   02 Jun 1999 21:41:34   ANK
   #66547# SubShells

      Rev 1.74   14 Jan 1999 16:21:56   NN
   #60725# SID_CHARMAP: SetAttribs an der EditEngine statt der View

      Rev 1.73   13 Jan 1999 17:05:52   NN
   #60811# SID_HYPERLINK_SETLINK: Adjust an der Selektion rufen

      Rev 1.72   16 Nov 1998 20:48:28   NN
   #57254# SID_HYPERLINK_SETLINK: auch TopView

      Rev 1.71   05 Dec 1997 19:21:52   NN
   Includes

      Rev 1.70   02 Dec 1997 12:38:40   TJ
   include

      Rev 1.69   24 Nov 1997 20:06:08   NN
   includes

      Rev 1.68   11 Sep 1997 19:57:02   NN
   Hyperlink ersetzen: Einfuegen auf Selektion

      Rev 1.67   04 Sep 1997 19:44:22   RG
   change header

      Rev 1.66   27 Aug 1997 12:19:28   TRI
   VCL Anpassung

      Rev 1.65   05 Aug 1997 14:29:22   TJ
   include svx/srchitem.hxx

      Rev 1.64   01 Aug 1997 11:32:58   NN
   #42327# RemoveAttribs(TRUE)

      Rev 1.63   30 Jul 1997 17:27:10   HJS
   includes

      Rev 1.62   29 Jul 1997 21:23:02   MH
   Syntax

      Rev 1.61   29 Jul 1997 09:55:14   OM
   Hyperlink Statusmethode

      Rev 1.60   16 Jul 1997 21:04:26   NN
   Hypo-Feature: Klammern hervorheben

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


