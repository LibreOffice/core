/*************************************************************************
 *
 *  $RCSfile: drtxtob.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-24 20:15:44 $
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

//------------------------------------------------------------------------

// T
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX

//SV
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX

#if defined  WIN
#define _MENUBTN_HXX
#endif


//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _TABBAR_HXX
#define _VALUESET_HXX
#define _STDMENU_HXX
#define _STDCTRL_HXX
// #define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX

//SVTOOLS
#define _SVTREELIST_HXX
#define _FILTER_HXX
#define _SVLBOXITM_HXX
#define _SVTREEBOX_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX



//svdraw.hxx
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet

#define _SDR_NOEXTDEV           // ExtOutputDevice
//#define   _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
//#define _SDR_NOTEXTOBJ          // SdrTextObj
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
//#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
//#define _SDR_NOOLE2OBJ          // SdrOle2Obj


// si.hxx:
//#define _SI_HXX
#define _SIDLL_HXX

#define SI_NOITEMS
//#define SI_NODRW
#define SI_NOOTHERFORMS
#define SI_NOSBXCONTROLS
#define SI_NOCONTROL
#define _VCATTR_HXX
#define _VCONT_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
//#define _VCDRWOBJ_HXX
#define _VCBRW_HXX

//-------------------------------------------------------------------------

#include "scitems.hxx"
#include "eetext.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/adjitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/editeng.hxx>
#include <svx/escpitem.hxx>
#include <svx/flditem.hxx>
#include <svx/fontitem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/lspcitem.hxx>
#include <svx/svdoutl.hxx>
#include <svx/postitem.hxx>
#include <svx/scripttypeitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/srchitem.hxx>
#include <svx/textanim.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "sc.hrc"
#include "globstr.hrc"
#include "drtxtob.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "viewutil.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"

#define ScDrawTextObjectBar
#include "scslots.hxx"

SFX_IMPL_INTERFACE( ScDrawTextObjectBar, SfxShell, ScResId(SCSTR_DRAWTEXTSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_TEXT_TOOLBOX) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_DRAWTEXT) );
    SFX_OBJECTMENU_REGISTRATION( SID_OBJECTMENU0, ScResId(RID_OBJECTMENU_DRAWTEXT) );
    SFX_CHILDWINDOW_REGISTRATION( ScGetFontWorkId() );
}

TYPEINIT1( ScDrawTextObjectBar, SfxShell );



// abschalten der nicht erwuenschten Acceleratoren:

void ScDrawTextObjectBar::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

ScDrawTextObjectBar::ScDrawTextObjectBar(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData(pData)
{
    SetPool( pViewData->GetScDrawView()->GetDefaultAttr().GetPool() );

    //  UndoManager wird beim Umschalten in den Edit-Modus umgesetzt...
    SetUndoManager( pViewData->GetSfxDocShell()->GetUndoManager() );

    SetHelpId( HID_SCSHELL_DRTXTOB );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("DrawText")));
}

__EXPORT ScDrawTextObjectBar::~ScDrawTextObjectBar()
{
}

//========================================================================
//
//          Funktionen
//
//========================================================================

void __EXPORT ScDrawTextObjectBar::Execute( SfxRequest &rReq )
{
    SdrView* pView = pViewData->GetScDrawView();
    OutlinerView* pOutView = pView->GetTextEditOutlinerView();
    Outliner* pOutliner = pView->GetTextEditOutliner();

    if (!pOutView || !pOutliner)
    {
        ExecuteGlobal( rReq );              // auf ganze Objekte
        return;
    }

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    switch ( rReq.GetSlot() )
    {
        case SID_COPY:
            pOutView->Copy();
            break;

        case SID_CUT:
            pOutView->Cut();
            break;

        case SID_PASTE:
            pOutView->PasteSpecial();
            break;

        case FID_PASTE_CONTENTS:
            {
                ExecutePasteContents( rReq );
/*
                SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
                if (pClipObj.Is())
                {
                    SvPasteObjectDialog* pDlg = new SvPasteObjectDialog;
                    pDlg->Insert( FORMAT_STRING,        ResId( SCSTR_CLIP_STRING ) );
                    pDlg->Insert( FORMAT_RTF,           ResId( SCSTR_CLIP_RTF ) );
                    ULONG nFormat = pDlg->Execute( Application::GetAppWindow(), pClipObj );
                    if (nFormat == FORMAT_STRING)
                        pOutView->Paste();
                    else
                        pOutView->PasteSpecial();
                    delete pDlg;
                }
*/
            }
            break;

        case SID_SELECTALL:
            {
                ULONG nCount = pOutliner->GetParagraphCount();
                ESelection aSel( 0,0,(USHORT)nCount,0 );
                pOutView->SetSelection( aSel );
            }
            break;

        case SID_CHARMAP:
            {
                const SvxFontItem& rItem = (const SvxFontItem&)
                            pOutView->GetAttribs().Get(EE_CHAR_FONTINFO);

                String aString;
                SvxFontItem aNewItem( EE_CHAR_FONTINFO );

                if (ScViewUtil::ExecuteCharMap( rItem, aNewItem, aString ))
                {
                    SfxItemSet aSet( pOutliner->GetEmptyItemSet() );
                    aSet.Put( aNewItem );
                    //  SetAttribs an der View selektiert ein Wort, wenn nichts selektiert ist
                    pOutView->GetOutliner()->QuickSetAttribs( aSet, pOutView->GetSelection() );
                    pOutView->InsertText(aString);
                }

                Invalidate( SID_ATTR_CHAR_FONT );
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
                    if ( pOutView && ( eMode == HLINK_DEFAULT || eMode == HLINK_FIELD ) )
                    {
                        const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
                        if (pFieldItem)
                        {
                            const SvxFieldData* pField = pFieldItem->GetField();
                            if ( pField && pField->ISA(SvxURLField) )
                            {
                                //  altes Feld selektieren

                                ESelection aSel = pOutView->GetSelection();
                                aSel.Adjust();
                                aSel.nEndPara = aSel.nStartPara;
                                aSel.nEndPos = aSel.nStartPos + 1;
                                pOutView->SetSelection( aSel );
                            }
                        }

                        //  neues Feld einfuegen

                        SvxURLField aURLField( rURL, rName, SVXURLFORMAT_REPR );
                        aURLField.SetTargetFrame( rTarget );
                        SvxFieldItem aURLItem( aURLField );
                        pOutView->InsertField( aURLItem );

                        //  select new field

                        ESelection aSel = pOutView->GetSelection();
                        if ( aSel.nStartPos == aSel.nEndPos && aSel.nStartPos > 0 )
                        {
                            //  Cursor is behind the inserted field -> extend selection to the left

                            --aSel.nStartPos;
                            pOutView->SetSelection( aSel );
                        }

                        bDone = TRUE;
                    }

                    if (!bDone)
                        ExecuteGlobal( rReq );      // normal an der View

                    //  InsertURL an der ViewShell schaltet bei "Text" die DrawShell ab !!!
                }
            }
            break;
    }
}

void __EXPORT ScDrawTextObjectBar::GetState( SfxItemSet& rSet )
{
    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    BOOL bHasFontWork = pViewFrm->HasChildWindow(SID_FONTWORK);
    BOOL bDisableFontWork = FALSE;

    if (IsNoteEdit())
    {
        //  Funktionen, die bei Notizen disabled sind:

        rSet.DisableItem( SID_TEXT_STANDARD );
        rSet.DisableItem( SID_DRAWTEXT_ATTR_DLG );

        if(!bHasFontWork)
            bDisableFontWork = TRUE;
    }

    if ( bDisableFontWork )
        rSet.DisableItem( SID_FONTWORK  );
    else
        rSet.Put(SfxBoolItem(SID_FONTWORK, bHasFontWork));

    if ( rSet.GetItemState( SID_HYPERLINK_GETLINK ) != SFX_ITEM_UNKNOWN )
    {
        SvxHyperlinkItem aHLinkItem;
        SdrView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if ( pOutView )
        {
            BOOL bField = FALSE;
            const SvxFieldItem* pFieldItem = pOutView->GetFieldAtSelection();
            if (pFieldItem)
            {
                const SvxFieldData* pField = pFieldItem->GetField();
                if ( pField && pField->ISA(SvxURLField) )
                {
                    const SvxURLField* pURLField = (const SvxURLField*) pField;
                    aHLinkItem.SetName( pURLField->GetRepresentation() );
                    aHLinkItem.SetURL( pURLField->GetURL() );
                    aHLinkItem.SetTargetFrame( pURLField->GetTargetFrame() );
                    bField = TRUE;
                }
            }
            if (!bField)
            {
                // use selected text as name for urls
                String sReturn = pOutView->GetSelected();
                sReturn.Erase(255);
                sReturn.EraseTrailingChars();
                aHLinkItem.SetName(sReturn);
            }
        }
        rSet.Put(aHLinkItem);
    }
}

void __EXPORT ScDrawTextObjectBar::GetClipState( SfxItemSet& rSet )
{
    SdrView* pView = pViewData->GetScDrawView();
    if ( !pView->GetTextEditOutlinerView() )
    {
        GetGlobalClipState( rSet );
        return;
    }

    BOOL bPaste = HasPasteContents();
/*
    BOOL bPaste = FALSE;
    SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
    if (pClipObj.Is())
    {
        const SvDataTypeList& rTypeLst = pClipObj->GetTypeList();

        if( rTypeLst.Get( FORMAT_STRING ) || rTypeLst.Get( FORMAT_RTF ) )
            bPaste = TRUE;
    }
*/

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

//========================================================================
//
//          Attribute
//
//========================================================================

void __EXPORT ScDrawTextObjectBar::ExecuteToggle( SfxRequest &rReq )
{
    //  Unterstreichung

    SdrView* pView = pViewData->GetScDrawView();

    const SfxItemSet* pArgs = rReq.GetArgs();
    USHORT nSlot = rReq.GetSlot();

    SfxItemSet aSet( pView->GetDefaultAttr() );

    SfxItemSet aViewAttr(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aViewAttr);

    //  Unterstreichung
    FontUnderline eOld = ((const SvxUnderlineItem&) aViewAttr.
                                        Get(EE_CHAR_UNDERLINE)).GetUnderline();
    FontUnderline eNew = eOld;
    switch (nSlot)
    {
        case SID_ULINE_VAL_NONE:
            eNew = UNDERLINE_NONE;
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
        default:
            break;
    }
    aSet.Put( SvxUnderlineItem( eNew, EE_CHAR_UNDERLINE ) );

    pView->SetAttributes( aSet );
    rReq.Done();
    pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
}

void lcl_RemoveFields( OutlinerView& rOutView )
{
    //! Outliner should have RemoveFields with a selection

    Outliner* pOutliner = rOutView.GetOutliner();
    if (!pOutliner) return;

    ESelection aOldSel = rOutView.GetSelection();
    ESelection aSel = aOldSel;
    aSel.Adjust();
    xub_StrLen nNewEnd = aSel.nEndPos;

    BOOL bUpdate = pOutliner->GetUpdateMode();
    BOOL bChanged = FALSE;

    //! GetPortions and GetAttribs should be const!
    EditEngine& rEditEng = (EditEngine&)pOutliner->GetEditEngine();

    ULONG nParCount = pOutliner->GetParagraphCount();
    for (ULONG nPar=0; nPar<nParCount; nPar++)
        if ( nPar >= aSel.nStartPara && nPar <= aSel.nEndPara )
        {
            SvUShorts aPortions;
            rEditEng.GetPortions( nPar, aPortions );
            //! GetPortions should use xub_StrLen instead of USHORT

            for ( USHORT nPos = aPortions.Count(); nPos; )
            {
                --nPos;
                USHORT nEnd = aPortions.GetObject( nPos );
                USHORT nStart = nPos ? aPortions.GetObject( nPos - 1 ) : 0;
                // fields are single characters
                if ( nEnd == nStart+1 &&
                     ( nPar > aSel.nStartPara || nStart >= aSel.nStartPos ) &&
                     ( nPar < aSel.nEndPara   || nEnd   <= aSel.nEndPos ) )
                {
                    ESelection aFieldSel( nPar, nStart, nPar, nEnd );
                    SfxItemSet aSet = rEditEng.GetAttribs( aFieldSel );
                    if ( aSet.GetItemState( EE_FEATURE_FIELD ) == SFX_ITEM_ON )
                    {
                        if (!bChanged)
                        {
                            if (bUpdate)
                                pOutliner->SetUpdateMode( FALSE );
                            String aName = ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );
                            pOutliner->GetUndoManager().EnterListAction( aName, aName );
                            bChanged = TRUE;
                        }

                        String aFieldText = rEditEng.GetText( aFieldSel );
                        pOutliner->QuickInsertText( aFieldText, aFieldSel );
                        if ( nPar == aSel.nEndPara )
                        {
                            nNewEnd += aFieldText.Len();
                            --nNewEnd;
                        }
                    }
                }
            }
        }

    if (bUpdate && bChanged)
    {
        pOutliner->GetUndoManager().LeaveListAction();
        pOutliner->SetUpdateMode( TRUE );
    }

    if ( aOldSel.IsEqual( aSel ) )          // aSel is adjusted
        aOldSel.nEndPos = nNewEnd;
    else
        aOldSel.nStartPos = nNewEnd;        // if aOldSel is backwards
    rOutView.SetSelection( aOldSel );
}

void __EXPORT ScDrawTextObjectBar::ExecuteAttr( SfxRequest &rReq )
{
    SdrView*            pView = pViewData->GetScDrawView();
    const SfxItemSet*   pArgs = rReq.GetArgs();
    USHORT              nSlot = rReq.GetSlot();

    BOOL bArgsInReq = ( pArgs != NULL );
    if ( !bArgsInReq )
    {
        SfxItemSet aEditAttr(pView->GetModel()->GetItemPool());
        pView->GetAttributes(aEditAttr);
        SfxItemSet  aNewAttr( *aEditAttr.GetPool(), aEditAttr.GetRanges() );
        BOOL        bDone = TRUE;

        switch ( nSlot )
        {
            case SID_TEXT_STANDARD: // Harte Textattributierung loeschen
            {
                OutlinerView* pOutView = pView->IsTextEdit() ?
                                pView->GetTextEditOutlinerView() : NULL;
                if ( pOutView )
                    pOutView->Paint( Rectangle() );

                SfxItemSet aEmptyAttr( *aEditAttr.GetPool(), EE_ITEMS_START, EE_ITEMS_END );
                pView->SetAttributes( aEmptyAttr, TRUE );

                if ( pOutView )
                {
                    lcl_RemoveFields( *pOutView );
                    pOutView->ShowCursor();
                }

                rReq.Done( aEmptyAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
                bDone = FALSE; // bereits hier passiert
            }
            break;

            case SID_CHAR_DLG:                      // Dialog-Button
            case SID_ATTR_CHAR_FONT:                // Controller nicht angezeigt
            case SID_ATTR_CHAR_FONTHEIGHT:
                bDone = ExecuteCharDlg( aEditAttr, aNewAttr );
                break;

            case SID_PARA_DLG:
                bDone = ExecuteParaDlg( aEditAttr, aNewAttr );
                break;

            case SID_ATTR_CHAR_WEIGHT:
                aNewAttr.Put( (const SvxWeightItem&)aEditAttr.Get( EE_CHAR_WEIGHT ) );
                break;

            case SID_ATTR_CHAR_POSTURE:
                aNewAttr.Put( (const SvxPostureItem&)aEditAttr.Get( EE_CHAR_ITALIC ) );
                break;

            case SID_ATTR_CHAR_UNDERLINE:
                aNewAttr.Put( (const SvxUnderlineItem&)aEditAttr.Get( EE_CHAR_UNDERLINE ) );
                break;

            case SID_ATTR_CHAR_CONTOUR:
                aNewAttr.Put( (const SvxContourItem&)aEditAttr.Get( EE_CHAR_OUTLINE ) );
                break;

            case SID_ATTR_CHAR_SHADOWED:
                aNewAttr.Put( (const SvxShadowedItem&)aEditAttr.Get( EE_CHAR_SHADOW ) );
                break;

            case SID_ATTR_CHAR_STRIKEOUT:
                aNewAttr.Put( (const SvxCrossedOutItem&)aEditAttr.Get( EE_CHAR_STRIKEOUT ) );
                break;

            case SID_ATTR_PARA_ADJUST_LEFT:
                aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
                break;

            case SID_ATTR_PARA_ADJUST_CENTER:
                aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
                break;

            case SID_ATTR_PARA_ADJUST_RIGHT:
                aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
                break;

            case SID_ATTR_PARA_ADJUST_BLOCK:
                aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_BLOCK, EE_PARA_JUST ) );
                break;

            case SID_ATTR_PARA_LINESPACE_10:
                {
                    SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_LINE, EE_PARA_SBL );
                    aItem.SetPropLineSpace( 100 );
                    aNewAttr.Put( aItem );
                }
                break;

            case SID_ATTR_PARA_LINESPACE_15:
                {
                    SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL );
                    aItem.SetPropLineSpace( 150 );
                    aNewAttr.Put( aItem );
                }
                break;

            case SID_ATTR_PARA_LINESPACE_20:
                {
                    SvxLineSpacingItem aItem( SVX_LINESPACE_TWO_LINES, EE_PARA_SBL );
                    aItem.SetPropLineSpace( 200 );
                    aNewAttr.Put( aItem );
                }
                break;

            case SID_SET_SUPER_SCRIPT:
                {
                    SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                    SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                                    aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                        aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                    else
                        aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
                    aNewAttr.Put( aItem );
                }
                break;
            case SID_SET_SUB_SCRIPT:
                {
                    SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                    SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                                    aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                    if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                        aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                    else
                        aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
                    aNewAttr.Put( aItem );
                }
                break;

            case SID_DRAWTEXT_ATTR_DLG:
                {
                    SvxTextTabDialog* pDlg =
                        new SvxTextTabDialog( pViewData->GetDialogParent(),
                                               &aEditAttr,
                                               pView );

                    bDone = ( RET_OK == pDlg->Execute() );

                    if ( bDone )
                        aNewAttr.Put( *pDlg->GetOutputItemSet() );

                    delete pDlg;
                }
                break;
        }

        if ( bDone ) // wurden Attribute geaendert?
        {
            rReq.Done( aNewAttr );
            pArgs = rReq.GetArgs();
        }
    }

    if ( pArgs )
    {
        if ( bArgsInReq &&
            ( nSlot == SID_ATTR_CHAR_FONT || nSlot == SID_ATTR_CHAR_FONTHEIGHT ||
              nSlot == SID_ATTR_CHAR_WEIGHT || nSlot == SID_ATTR_CHAR_POSTURE ) )
        {
            // font items from toolbox controller have to be applied for the right script type

            USHORT nScript = pView->GetScriptType();

            SfxItemPool& rPool = GetPool();
            SvxScriptSetItem aSetItem( nSlot, rPool );
            USHORT nWhich = rPool.GetWhich( nSlot );
            aSetItem.PutItemForScriptType( nScript, pArgs->Get( nWhich ) );

            pView->SetAttributes( aSetItem.GetItemSet() );
        }
        else
        {
            // use args directly

            pView->SetAttributes( *pArgs );
        }
        pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
    }
}

void __EXPORT ScDrawTextObjectBar::GetAttrState( SfxItemSet& rDestSet )
{
    if ( IsNoteEdit() )
    {
            //  Notizen haben keine Text-Attribute !!!
        SfxWhichIter aIter(rDestSet);
        USHORT nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            rDestSet.DisableItem( nWhich );
            nWhich = aIter.NextWhich();
        }
        return;
    }


    SdrView* pView = pViewData->GetScDrawView();
    SfxItemSet aAttrSet(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aAttrSet);

    //  direkte Attribute

    rDestSet.Put( aAttrSet );

    //  choose font info according to selection script type

    USHORT nScript = pView->GetScriptType();

    if ( rDestSet.GetItemState( EE_CHAR_FONTINFO ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_FONTINFO, nScript );
    if ( rDestSet.GetItemState( EE_CHAR_FONTHEIGHT ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_FONTHEIGHT, nScript );
    if ( rDestSet.GetItemState( EE_CHAR_WEIGHT ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_WEIGHT, nScript );
    if ( rDestSet.GetItemState( EE_CHAR_ITALIC ) != SFX_ITEM_UNKNOWN )
        ScViewUtil::PutItemScript( rDestSet, aAttrSet, EE_CHAR_ITALIC, nScript );

    //  Ausrichtung

    SvxAdjust eAdj = ((const SvxAdjustItem&)aAttrSet.Get(EE_PARA_JUST)).GetAdjust();
    switch( eAdj )
    {
        case SVX_ADJUST_LEFT:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, TRUE ) );
            break;
        case SVX_ADJUST_CENTER:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, TRUE ) );
            break;
        case SVX_ADJUST_RIGHT:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, TRUE ) );
            break;
        case SVX_ADJUST_BLOCK:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, TRUE ) );
            break;
    }

    //  Zeilenabstand

    USHORT nLineSpace = (USHORT)
                ((const SvxLineSpacingItem&)aAttrSet.
                        Get( EE_PARA_SBL )).GetPropLineSpace();
    switch( nLineSpace )
    {
        case 100:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_10, TRUE ) );
            break;
        case 150:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_15, TRUE ) );
            break;
        case 200:
            rDestSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_20, TRUE ) );
            break;
    }

    //  hoch-/tiefgestellt

    SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                    aAttrSet.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        rDestSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, TRUE ) );
    else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        rDestSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, TRUE ) );

    //  Unterstreichung

    SfxItemState eState = aAttrSet.GetItemState( EE_CHAR_UNDERLINE, TRUE );
    if ( eState == SFX_ITEM_DONTCARE )
    {
        rDestSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rDestSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontUnderline eUnderline = ((const SvxUnderlineItem&)
                    aAttrSet.Get(EE_CHAR_UNDERLINE)).GetUnderline();
        USHORT nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case UNDERLINE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case UNDERLINE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case UNDERLINE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rDestSet.Put( SfxBoolItem( nId, TRUE ) );
    }
}



