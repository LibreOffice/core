/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <string>
#include <stdlib.h>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/logfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/inputctx.hxx>
#include <basic/sbxobj.hxx>
#include <svl/eitem.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include <svx/ruler.hxx>
#include <editeng/protitem.hxx>
#include <svx/fmshell.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <unotxvw.hxx>
#include <cmdid.h>
#include <swhints.hxx>
#include <swmodule.hxx>
#include <inputwin.hxx>
#include <chartins.hxx>
#include <uivwimp.hxx>
#include <uitool.hxx>
#include <edtwin.hxx>
#include <textsh.hxx>
#include <listsh.hxx>
#include <tabsh.hxx>
#include <grfsh.hxx>
#include <mediash.hxx>
#include <docsh.hxx>
#include <frmsh.hxx>
#include <olesh.hxx>
#include <drawsh.hxx>
#include <drawbase.hxx>
#include <drformsh.hxx>
#include <drwtxtsh.hxx>
#include <beziersh.hxx>
#include <navsh.hxx>
#include <globdoc.hxx>
#include <scroll.hxx>
#include <navipi.hxx>
#include <gloshdl.hxx>
#include <usrpref.hxx>
#include <srcview.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <drawdoc.hxx>
#include <wdocsh.hxx>
#include <wview.hxx>
#include <workctrl.hxx>
#include <wrtsh.hxx>
#include <barcfg.hxx>
#include <pview.hxx>
#include <swdtflvr.hxx>
#include <view.hrc>
#include <globdoc.hrc>
#include <frmui.hrc>
#include <cfgitems.hxx>
#include <prtopt.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/scanner/ScannerContext.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <toolkit/unohlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>

#include "formatclipboard.hxx"
#include <PostItMgr.hxx>
#include <annotsh.hxx>

#include <fldbas.hxx>

#include <unomid.h>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

#define SWVIEWFLAGS ( SFX_VIEW_CAN_PRINT|               \
                      SFX_VIEW_HAS_PRINTOPTIONS)

/*--------------------------------------------------------------------
    Beschreibung:   Statics
 --------------------------------------------------------------------*/

int bDocSzUpdated = 1;

SvxSearchItem*  SwView::pSrchItem   = 0;

sal_uInt16          SwView::nInsertCtrlState        = FN_INSERT_TABLE;
sal_uInt16          SwView::nWebInsertCtrlState     = FN_INSERT_TABLE;
sal_uInt16          SwView::nInsertObjectCtrlState  = SID_INSERT_DIAGRAM;

sal_Bool            SwView::bExtra      = sal_False;
sal_Bool            SwView::bFound      = sal_False;
sal_Bool            SwView::bJustOpened = sal_False;

SvxSearchDialog*    SwView::pSrchDlg    = 0;
SearchAttrItemList* SwView::pSrchList   = 0;
SearchAttrItemList* SwView::pReplList   = 0;

DBG_NAME(viewhdl)

inline SfxDispatcher &SwView::GetDispatcher()
{
    return *GetViewFrame()->GetDispatcher();
}

void SwView::ImpSetVerb( int nSelType )
{
    sal_Bool bResetVerbs = bVerbsActive;
    if ( !GetViewFrame()->GetFrame().IsInPlace() &&
         (nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) & nSelType )
    {
        if ( !pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT) )
        {
            if ( nSelType & nsSelectionType::SEL_OLE )
            {
                SetVerbs( GetWrtShell().GetOLEObject()->getSupportedVerbs() );
                bVerbsActive = sal_True;
                bResetVerbs = sal_False;
            }
        }
    }
    if ( bResetVerbs )
    {
        SetVerbs( Sequence< embed::VerbDescriptor >() );
        bVerbsActive = sal_False;
    }
}

/*--------------------------------------------------------------------
    Description:
    called by the SwEditWin when it gets the focus
 --------------------------------------------------------------------*/
void SwView::GotFocus() const
{
    // if we got the focus, and the form shell *is* on the top of the dispatcher
    // stack, then we need to rebuild the stack (the form shell doesn't belong to
    // the top then)
    const SfxDispatcher& rDispatcher = const_cast< SwView* >( this )->GetDispatcher();
    SfxShell* pTopShell = rDispatcher.GetShell( 0 );
    FmFormShell* pAsFormShell = PTR_CAST( FmFormShell, pTopShell );
    if ( pAsFormShell )
    {
        pAsFormShell->ForgetActiveControl();
        const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
    }
    else if ( mpPostItMgr )
    {
        SwAnnotationShell* pAsAnnotationShell = PTR_CAST( SwAnnotationShell, pTopShell );
        if ( pAsAnnotationShell )
        {
            mpPostItMgr->SetActiveSidebarWin(0);
            const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
        }
    }
    if( GetWrtShellPtr() )
    {
        SwWrtShell& rWrtShell = GetWrtShell();
        rWrtShell.GetDoc()->SetCurrentViewShell( GetWrtShellPtr() );
        rWrtShell.GetDoc()->set( IDocumentSettingAccess::BROWSE_MODE,
                                 rWrtShell.GetViewOptions()->getBrowseMode() );
    }
}

/*--------------------------------------------------------------------
    Description:
    called by the FormShell when a form control is focused. This is
    a request to put the form shell on the top of the dispatcher
    stack
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwView, FormControlActivated)
{
    // if a form control has been activated, and the form shell is not on the top
    // of the dispatcher stack, then we need to activate it
    const SfxDispatcher& rDispatcher = GetDispatcher();
    const SfxShell* pTopShell = rDispatcher.GetShell( 0 );
    const FmFormShell* pAsFormShell = PTR_CAST( FmFormShell, pTopShell );
    if ( !pAsFormShell )
    {
        // if we're editing text currently, cancel this
        SdrView *pSdrView = pWrtShell ? pWrtShell->GetDrawView() : NULL;
        if ( pSdrView && pSdrView->IsTextEdit() )
            pSdrView->SdrEndTextEdit( sal_True );

        const_cast< SwView* >( this )->AttrChangedNotify( pWrtShell );
    }

    return 0L;
}

void SwView::SelectShell()
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//      Achtung: SelectShell fuer die WebView mitpflegen
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

          if(bInDtor)
        return;
    // Entscheidung, ob UpdateTable gerufen werden muss
    sal_Bool bUpdateTable = sal_False;
    const SwFrmFmt* pCurTableFmt = pWrtShell->GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != pLastTableFormat)
    {
        bUpdateTable = sal_True; // kann erst spaeter ausgefuehrt werden
    }
    pLastTableFormat = pCurTableFmt;

    //SEL_TBL and SEL_TBL_CELLS can be changed!
    int nNewSelectionType = (pWrtShell->GetSelectionType()
                                & ~nsSelectionType::SEL_TBL_CELLS);

    if ( pFormShell && pFormShell->IsActiveControl() )
        nNewSelectionType |= nsSelectionType::SEL_FOC_FRM_CTRL;

    if ( nNewSelectionType == nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        if ( nSelectionType & nsSelectionType::SEL_OLE ||
             nSelectionType & nsSelectionType::SEL_GRF )
            //Fuer Grafiken und OLE kann sich natuerlich das Verb aendern!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {

        SfxDispatcher &rDispatcher = GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetToolbarConfig();

        if ( pShell )
        {
            rDispatcher.Flush();        // alle gecachten Shells wirklich loeschen

            //Zur alten Selektion merken welche Toolbar sichtbar war
            sal_uInt16 nId = static_cast< sal_uInt16 >( rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT ));
            if ( nId )
                pBarCfg->SetTopToolbar( nSelectionType, nId );

            SfxShell *pSfxShell;
            for ( sal_uInt16 i = 0; sal_True; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if  (  pSfxShell->ISA( SwBaseShell )
                    || pSfxShell->ISA( SwDrawTextShell )
                    || pSfxShell->ISA( svx::ExtrusionBar )
                    || pSfxShell->ISA( svx::FontworkBar )
                    || pSfxShell->ISA( SwAnnotationShell )
                    )
                {
                    rDispatcher.Pop( *pSfxShell, SFX_SHELL_POP_DELETE );
                }
                else if ( pSfxShell->ISA( FmFormShell ) )
                {
                    rDispatcher.Pop( *pSfxShell );
                }
                else
                    break;
            }
        }

        sal_Bool bInitFormShell = sal_False;
        if (!pFormShell)
        {
            bInitFormShell = sal_True;
            pFormShell = new FmFormShell( this );
            pFormShell->SetControlActivationHandler( LINK( this, SwView, FormControlActivated ) );
            StartListening(*pFormShell);
        }

        sal_Bool bSetExtInpCntxt = sal_False;
        nSelectionType = nNewSelectionType;
        ShellModes eShellMode;

        if ( !( nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL ) )
            rDispatcher.Push( *pFormShell );

        eShellMode = SHELL_MODE_NAVIGATION;
        pShell = new SwNavigationShell( *this );
        rDispatcher.Push( *pShell );

        if ( nSelectionType & nsSelectionType::SEL_OLE )
        {
            eShellMode = SHELL_MODE_OBJECT;
            pShell = new SwOleShell( *this );
            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_FRM
            || nSelectionType & nsSelectionType::SEL_GRF)
        {
            eShellMode = SHELL_MODE_FRAME;
            pShell = new SwFrameShell( *this );
            rDispatcher.Push( *pShell );
            if(nSelectionType & nsSelectionType::SEL_GRF )
            {
                eShellMode = SHELL_MODE_GRAPHIC;
                pShell = new SwGrfShell( *this );
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW )
        {
            eShellMode = SHELL_MODE_DRAW;
            pShell = new SwDrawShell( *this );
            rDispatcher.Push( *pShell );

            if ( nSelectionType & nsSelectionType::SEL_BEZ )
            {
                eShellMode = SHELL_MODE_BEZIER;
                pShell = new SwBezierShell( *this );
                rDispatcher.Push( *pShell );
            }
            else if( nSelectionType & nsSelectionType::SEL_MEDIA )
            {
                eShellMode = SHELL_MODE_MEDIA;
                pShell = new SwMediaShell( *this );
                rDispatcher.Push( *pShell );
            }

            if (nSelectionType & nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE)
            {
                eShellMode = SHELL_MODE_EXTRUDED_CUSTOMSHAPE;
                pShell = new svx::ExtrusionBar(this);
                rDispatcher.Push( *pShell );
            }
            if (nSelectionType & nsSelectionType::SEL_FONTWORK)
            {
                eShellMode = SHELL_MODE_FONTWORK;
                pShell = new svx::FontworkBar(this);
                rDispatcher.Push( *pShell );
            }
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW_FORM )
        {
            eShellMode = SHELL_MODE_DRAW_FORM;
            pShell = new SwDrawFormShell( *this );

            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_DRW_TXT )
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            pShell = new SwDrawTextShell( *this );
            rDispatcher.Push( *pShell );
        }
        else if ( nSelectionType & nsSelectionType::SEL_POSTIT )
        {
            eShellMode = SHELL_MODE_POSTIT;
            pShell = new SwAnnotationShell( *this );
            rDispatcher.Push( *pShell );
        }
        else
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_TEXT;
            sal_uInt32 nHelpId = 0;
            if ( nSelectionType & nsSelectionType::SEL_NUM )
            {
                eShellMode = SHELL_MODE_LIST_TEXT;
                pShell = new SwListShell( *this );
                nHelpId = pShell->GetHelpId();
                rDispatcher.Push( *pShell );
            }
            pShell = new SwTextShell(*this);
            if(nHelpId)
                pShell->SetHelpId(nHelpId);
            rDispatcher.Push( *pShell );
            if ( nSelectionType & nsSelectionType::SEL_TBL )
            {
                eShellMode = eShellMode == SHELL_MODE_LIST_TEXT ? SHELL_MODE_TABLE_LIST_TEXT
                                                        : SHELL_MODE_TABLE_TEXT;
                pShell = new SwTableShell( *this );
                rDispatcher.Push( *pShell );
            }
        }

        if ( nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL )
            rDispatcher.Push( *pFormShell );

        pViewImpl->SetShellMode(eShellMode);
        ImpSetVerb( nSelectionType );

        if( !GetDocShell()->IsReadOnly() )
        {
            if( bSetExtInpCntxt && GetWrtShell().HasReadonlySel() )
                bSetExtInpCntxt = sal_False;

            InputContext aCntxt( GetEditWin().GetInputContext() );
            aCntxt.SetOptions( bSetExtInpCntxt
                                ? (aCntxt.GetOptions() |
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ))
                                : (aCntxt.GetOptions() & ~
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT )) );
            GetEditWin().SetInputContext( aCntxt );
        }

        //Zur neuen Selektion die Toolbar aktivieren, die auch beim letzten Mal
        //aktiviert war
        //Vorher muss ein Flush() sein, betrifft aber lt. MBA nicht das UI und ist
        //kein Performance-Problem
        // TODO/LATER: maybe now the Flush() command is superfluous?!
        rDispatcher.Flush();

        Point aPnt = GetEditWin().OutputToScreenPixel(GetEditWin().GetPointerPosPixel());
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        SdrView* pDView = GetWrtShell().GetDrawView();
        if ( bInitFormShell && pDView )
            pFormShell->SetView(PTR_CAST(FmFormView, pDView));

    }
    //Guenstiger Zeitpunkt fuer die Kommunikation mit OLE-Objekten?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( sal_False );

    // now the table-update
    if(bUpdateTable)
        pWrtShell->UpdateTable();

    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();
}

//Zusammenspiel: AttrChangedNotify() und TimeoutHdl.
//Falls noch Actions offen sind keine Aktualisierung, da der
//Cursor auf der Core-Seite im Wald stehen kann.
//Da wir aber keine Stati mehr liefern koennen und wollen locken wir
//stattdessen den Dispatcher.

extern "C"
{
    static int lcl_CmpIds( const void *pFirst, const void *pSecond)
    {
        return (*(sal_uInt16*)pFirst) - (*(sal_uInt16*)pSecond);
    }
}

IMPL_LINK_NOARG(SwView, AttrChangedNotify)
{
     if ( GetEditWin().IsChainMode() )
        GetEditWin().SetChainMode( sal_False );

    //Opt: Nicht wenn PaintLocked. Beim Unlock wird dafuer nocheinmal ein
    //Notify ausgeloest.
    if( !pWrtShell->IsPaintLocked() && !bNoInterrupt &&
        GetDocShell()->IsReadOnly() )
        _CheckReadonlyState();

    if( !pWrtShell->IsPaintLocked() && !bNoInterrupt )
        _CheckReadonlySelection();

    if( !bAttrChgNotified )
    {
        if ( pWrtShell->BasicActionPend() || bNoInterrupt ||
             GetDispatcher().IsLocked() ||               //do not confuse the SFX
             GetViewFrame()->GetBindings().IsInUpdate() )//do not confuse the SFX
        {
            bAttrChgNotified = sal_True;
            aTimer.Start();

            const SfxPoolItem *pItem;
            if ( SFX_ITEM_SET != GetObjectShell()->GetMedium()->GetItemSet()->
                                    GetItemState( SID_HIDDEN, sal_False, &pItem ) ||
                 !((SfxBoolItem*)pItem)->GetValue() )
            {
                GetViewFrame()->GetBindings().ENTERREGISTRATIONS();
                bAttrChgNotifiedWithRegistrations = sal_True;
            }

        }
        else
            SelectShell();

    }

    // change ui if cursor is at a SwPostItField
    if (mpPostItMgr)
    {
        // only perform the code that is needed to determine, if at the
        // actual cursor position is a post-it field
        mpPostItMgr->SetShadowState( pWrtShell->GetPostItFieldAtCursor() );
    }

    return 0;
}

IMPL_LINK_NOARG(SwView, TimeoutHdl)
{
    DBG_PROFSTART(viewhdl);

    if( pWrtShell->BasicActionPend() || bNoInterrupt )
    {
        aTimer.Start();
        DBG_PROFSTOP(viewhdl);
        return 0;
    }

    if ( bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        bAttrChgNotifiedWithRegistrations = sal_False;
    }

    _CheckReadonlyState();
    _CheckReadonlySelection();

    sal_Bool bOldUndo = pWrtShell->DoesUndo();
    pWrtShell->DoUndo( sal_False );
    SelectShell();
    pWrtShell->DoUndo( bOldUndo );
    bAttrChgNotified = sal_False;
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

    DBG_PROFSTOP(viewhdl);
    return 0;
}

void SwView::_CheckReadonlyState()
{
    SfxDispatcher &rDis = GetDispatcher();
    //Um erkennen zu koennen ob bereits disabled ist!
    SfxItemState eStateRO, eStateProtAll;
    const SfxPoolItem *pItem;
    // von einem nur uns bekannten Slot den Status abfragen.
    // Ansonsten kennen andere den Slot; wie z.B. die BasidIde
    eStateRO = rDis.QueryState( FN_INSERT_BOOKMARK, pItem );
    eStateProtAll = rDis.QueryState( FN_EDIT_REGION, pItem );
    sal_Bool bChgd = sal_False;

    if ( !pWrtShell->IsCrsrReadonly() )
    {
        static sal_uInt16 aROIds[] =
        {
            SID_DELETE,                 FN_BACKSPACE,               FN_SHIFT_BACKSPACE,
            SID_UNDO,
            SID_REDO,                   SID_REPEAT,                 SID_PASTE,
            SID_PASTE_UNFORMATTED,
            SID_PASTE_SPECIAL,            SID_SBA_BRW_INSERT,
            SID_BACKGROUND_COLOR,       FN_INSERT_BOOKMARK,
            SID_CHARMAP,                FN_INSERT_SOFT_HYPHEN,
            FN_INSERT_HARDHYPHEN,       FN_INSERT_HARD_SPACE,       FN_INSERT_BREAK,
            FN_INSERT_LINEBREAK,        FN_INSERT_COLUMN_BREAK,     FN_INSERT_BREAK_DLG,
            FN_DELETE_SENT,             FN_DELETE_BACK_SENT,        FN_DELETE_WORD,
            FN_DELETE_BACK_WORD,        FN_DELETE_LINE,             FN_DELETE_BACK_LINE,
            FN_DELETE_PARA,             FN_DELETE_BACK_PARA,        FN_DELETE_WHOLE_LINE,
            FN_CALCULATE,               FN_FORMAT_RESET,
            FN_POSTIT,                  FN_JAVAEDIT,                SID_ATTR_PARA_ADJUST_LEFT,
            SID_ATTR_PARA_ADJUST_RIGHT, SID_ATTR_PARA_ADJUST_CENTER,SID_ATTR_PARA_ADJUST_BLOCK,
            SID_ATTR_PARA_LINESPACE_10, SID_ATTR_PARA_LINESPACE_15, SID_ATTR_PARA_LINESPACE_20,
            SID_ATTR_CHAR_FONT,         SID_ATTR_CHAR_FONTHEIGHT,   SID_ATTR_CHAR_COLOR_BACKGROUND,
            SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,                     SID_ATTR_CHAR_COLOR_EXT,
            SID_ATTR_CHAR_COLOR,        SID_ATTR_CHAR_WEIGHT,       SID_ATTR_CHAR_POSTURE,
            SID_ATTR_CHAR_OVERLINE,
            SID_ATTR_CHAR_UNDERLINE,    SID_ATTR_FLASH,             SID_ATTR_CHAR_STRIKEOUT,
            FN_UNDERLINE_DOUBLE,        SID_ATTR_CHAR_CONTOUR,      SID_ATTR_CHAR_SHADOWED,
            SID_ATTR_CHAR_AUTOKERN,     SID_ATTR_CHAR_ESCAPEMENT,   FN_SET_SUPER_SCRIPT,
            FN_SET_SUB_SCRIPT,          SID_ATTR_CHAR_CASEMAP,      SID_ATTR_CHAR_LANGUAGE,
            SID_ATTR_CHAR_KERNING,      SID_CHAR_DLG,               SID_ATTR_CHAR_WORDLINEMODE,
            FN_GROW_FONT_SIZE,          FN_SHRINK_FONT_SIZE,        FN_TXTATR_INET,
            FN_FORMAT_DROPCAPS,         SID_ATTR_PARA_ADJUST,       SID_ATTR_PARA_LINESPACE,
            SID_ATTR_PARA_SPLIT,        SID_ATTR_PARA_KEEP,         SID_ATTR_PARA_WIDOWS,
            SID_ATTR_PARA_ORPHANS,
            SID_ATTR_PARA_MODEL,        SID_PARA_DLG,
            FN_SELECT_PARA,             SID_DEC_INDENT,
            SID_INC_INDENT
        };
        static sal_Bool bFirst = sal_True;
        if ( bFirst )
        {
            qsort( (void*)aROIds, sizeof(aROIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
            bFirst = sal_False;
        }
        if ( SFX_ITEM_DISABLED == eStateRO )
        {
            rDis.SetSlotFilter( sal_Bool(2), sizeof(aROIds)/sizeof(sal_uInt16), aROIds );
            bChgd = sal_True;
        }
    }
    else if( pWrtShell->IsAllProtect() )
    {
        if ( SFX_ITEM_DISABLED == eStateProtAll )
        {
            static sal_uInt16 aAllProtIds[] = { SID_SAVEDOC, FN_EDIT_REGION };
            static sal_Bool bAllProtFirst = sal_True;
            if ( bAllProtFirst )
            {
                qsort( (void*)aAllProtIds, sizeof(aAllProtIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
                bAllProtFirst = sal_False;
            }
            rDis.SetSlotFilter( sal_Bool(2),
                                sizeof(aAllProtIds)/sizeof(sal_uInt16),
                                aAllProtIds );
            bChgd = sal_True;
        }
    }
    else if ( SFX_ITEM_DISABLED != eStateRO ||
                SFX_ITEM_DISABLED != eStateProtAll )
    {
        bChgd = sal_True;
        rDis.SetSlotFilter();
    }
    if ( bChgd )
        GetViewFrame()->GetBindings().InvalidateAll(sal_True);
}

void SwView::_CheckReadonlySelection()
{
    sal_uInt32 nDisableFlags = 0;
    SfxDispatcher &rDis = GetDispatcher();

    if( pWrtShell->HasReadonlySel() &&
        ( !pWrtShell->GetDrawView() ||
            !pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount() ))
        nDisableFlags |= SW_DISABLE_ON_PROTECTED_CURSOR;

    if( (SW_DISABLE_ON_PROTECTED_CURSOR & nDisableFlags ) !=
        (SW_DISABLE_ON_PROTECTED_CURSOR & rDis.GetDisableFlags() ) )
    {
        // zusaetzlich am Window den InputContext umsetzen, damit in
        // japanischen / chinesischen Versionen die externe Eingabe
        // ab-/angeschaltet wird. Das aber nur wenn auch die richtige
        // Shell auf dem Stack steht.
        switch( pViewImpl->GetShellMode() )
        {
        case SHELL_MODE_TEXT:
        case SHELL_MODE_LIST_TEXT:
        case SHELL_MODE_TABLE_TEXT:
        case SHELL_MODE_TABLE_LIST_TEXT:
            {
// temporaere Loesung!!! Sollte bei jeder Cursorbewegung
//          den Font von der akt. Einfuegeposition setzen, also ausserhalb
//          dieses if's. Aber TH wertet den Font zur Zeit nicht aus und
//          das besorgen erscheint mir hier zu teuer zu sein.
//          Ausserdem haben wir keinen Font, sondern nur Attribute aus denen
//          die Textformatierung dann den richtigen Font zusammen baut.

                InputContext aCntxt( GetEditWin().GetInputContext() );
                aCntxt.SetOptions( SW_DISABLE_ON_PROTECTED_CURSOR & nDisableFlags
                                    ? (aCntxt.GetOptions() & ~
                                            ( INPUTCONTEXT_TEXT |
                                                INPUTCONTEXT_EXTTEXTINPUT ))
                                    : (aCntxt.GetOptions() |
                                            ( INPUTCONTEXT_TEXT |
                                                INPUTCONTEXT_EXTTEXTINPUT )) );
                GetEditWin().SetInputContext( aCntxt );
            }
            break;
        default:
            ;
        }

    }

    if( nDisableFlags != rDis.GetDisableFlags() )
    {
        rDis.SetDisableFlags( nDisableFlags );
        GetViewFrame()->GetBindings().InvalidateAll( sal_True );
    }
}

SwView::SwView( SfxViewFrame *_pFrame, SfxViewShell* pOldSh )
    : SfxViewShell( _pFrame, SWVIEWFLAGS ),
    aPageStr( SW_RES( STR_PAGE )),
    nNewPage(USHRT_MAX),
    pNumRuleNodeFromDoc(0),
    pEditWin( new SwEditWin( &_pFrame->GetWindow(), *this ) ),
    pWrtShell(0),
    pShell(0),
    pFormShell(0),
    pHScrollbar(0),
    pVScrollbar(0),
    pScrollFill(new ScrollBarBox( &_pFrame->GetWindow(), _pFrame->GetFrame().GetParentFrame() ? 0 : WB_SIZEABLE )),
    pHRuler( new SvxRuler(&GetViewFrame()->GetWindow(), pEditWin,
                    SVXRULER_SUPPORT_TABS |
                    SVXRULER_SUPPORT_PARAGRAPH_MARGINS |
                    SVXRULER_SUPPORT_BORDERS |
                    SVXRULER_SUPPORT_NEGATIVE_MARGINS|
                    SVXRULER_SUPPORT_REDUCED_METRIC,
                    GetViewFrame()->GetBindings(),
                    WB_STDRULER | WB_EXTRAFIELD | WB_BORDER)),
    pVRuler(new SvxRuler(&GetViewFrame()->GetWindow(), pEditWin,
                            SVXRULER_SUPPORT_TABS | SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL|
                                SVXRULER_SUPPORT_BORDERS | SVXRULER_SUPPORT_REDUCED_METRIC,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_EXTRAFIELD | WB_BORDER )),
    pTogglePageBtn(0),
    pPageUpBtn(0),
    pPageDownBtn(0),
    pNaviBtn(0),
    pGlosHdl(0),
    pDrawActual(0),
    pLastTableFormat(0),
    pFormatClipboard(new SwFormatClipboard()),
    mpPostItMgr(0),
    nSelectionType( INT_MAX ),
    nPageCnt(0),
    nDrawSfxId( USHRT_MAX ),
    nFormSfxId( USHRT_MAX ),
    nLastPasteDestination( 0xFFFF ),
    nLeftBorderDistance( 0 ),
    nRightBorderDistance( 0 ),
    bInMailMerge(sal_False),
    bInDtor(sal_False),
    bOldShellWasPagePreView(sal_False),
    bIsPreviewDoubleClick(sal_False)
{
    // According to discussion with MBA and further
    // investigations, no old SfxViewShell will be set as parameter <pOldSh>,
    // if function "New Window" is performed to open an additional view beside
    // an already existing one.
    // If the view is switch from one to another, the 'old' view is given by
    // parameter <pOldSh>.

    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwView::SwView" );

    bCenterCrsr = bTopCrsr = bAllwaysShowSel = bTabColFromDoc = bTabRowFromDoc =
    bSetTabColFromDoc = bSetTabRowFromDoc = bAttrChgNotified = bAttrChgNotifiedWithRegistrations =
    bVerbsActive = bDrawRotate = bInOuterResizePixel = bInInnerResizePixel =
    bPasteState = bPasteSpecialState = bMakeSelectionVisible = sal_False;

    bShowAtResize = bDrawSelMode = bDocSzUpdated = sal_True;

    _CreateScrollbar( sal_True );
    _CreateScrollbar( sal_False );

    pViewImpl = new SwView_Impl(this);
    SetName(rtl::OUString("View"));
    SetWindow( pEditWin );

    aTimer.SetTimeout( 120 );

    SwDocShell* pDocSh = PTR_CAST( SwDocShell, _pFrame->GetObjectShell() );
    sal_Bool bOldModifyFlag = pDocSh->IsEnableSetModified();
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_False );
    OSL_ENSURE( pDocSh, "view without DocShell." );
    SwWebDocShell* pWebDShell = PTR_CAST( SwWebDocShell, pDocSh );

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(0 != pWebDShell);
    SwViewOption aUsrPref( *pUsrPref);

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;
    SvtLinguConfig().GetOptions( aLinguOpt );
    aUsrPref.SetOnlineSpell( aLinguOpt.bIsSpellAuto );

    sal_Bool bOldShellWasSrcView = sal_False;

    // determine if there is an existing view for
    // document
    SfxViewShell* pExistingSh = 0;
    if ( pOldSh )
    {
        pExistingSh = pOldSh;
        // determine type of existing view
        if( pExistingSh->IsA( TYPE( SwPagePreView ) ) )
        {
            sSwViewData = ((SwPagePreView*)pExistingSh)->GetPrevSwViewData();
            sNewCrsrPos = ((SwPagePreView*)pExistingSh)->GetNewCrsrPos();
            nNewPage = ((SwPagePreView*)pExistingSh)->GetNewPage();
            bOldShellWasPagePreView = sal_True;
            bIsPreviewDoubleClick = sNewCrsrPos.Len() > 0 || nNewPage != USHRT_MAX;
        }
        else if( pExistingSh->IsA( TYPE( SwSrcView ) ) )
            bOldShellWasSrcView = sal_True;
    }

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before create WrtShell" );
    if(PTR_CAST( SwView, pExistingSh))
    {
        pWrtShell = new SwWrtShell( *((SwView*)pExistingSh)->pWrtShell,
                                    pEditWin, *this);
    }
    else if( dynamic_cast<SwWrtShell*>( pDocSh->GetDoc()->GetCurrentViewShell() ) )
    {
        pWrtShell = new SwWrtShell( *(SwWrtShell*)pDocSh->GetDoc()->GetCurrentViewShell(),
                                    pEditWin, *this);
    }
    else
    {
        SwDoc& rDoc = *((SwDocShell*)pDocSh)->GetDoc();

        if( !bOldShellWasSrcView && pWebDShell && !bOldShellWasPagePreView )
            aUsrPref.setBrowseMode( sal_True );
        else
            aUsrPref.setBrowseMode( rDoc.get(IDocumentSettingAccess::BROWSE_MODE) );

        //Fuer den BrowseMode wollen wir keinen Factor uebernehmen.
        if( aUsrPref.getBrowseMode() && aUsrPref.GetZoomType() != SVX_ZOOM_PERCENT )
        {
            aUsrPref.SetZoomType( SVX_ZOOM_PERCENT );
            aUsrPref.SetZoom( 100 );
        }
        if(pDocSh->IsPreview())
        {
            aUsrPref.SetZoomType( SVX_ZOOM_WHOLEPAGE );
            aUsrPref.SetViewLayoutBookMode( false );
            aUsrPref.SetViewLayoutColumns( 1 );
        }
        pWrtShell = new SwWrtShell( rDoc, pEditWin, *this, &aUsrPref );
        // creating an SwView from a SwPagePreView needs to
        // add the ViewShell to the ring of the other ViewShell(s)
        if(bOldShellWasPagePreView)
        {
            ViewShell& rPreviewViewShell = *((SwPagePreView*)pExistingSh)->GetViewShell();
            pWrtShell->MoveTo(&rPreviewViewShell);
            // to update the field command et.al. if necessary
            const SwViewOption* pPreViewOpt = rPreviewViewShell.GetViewOptions();
            if( pPreViewOpt->IsFldName() != aUsrPref.IsFldName() ||
                    pPreViewOpt->IsShowHiddenField() != aUsrPref.IsShowHiddenField() ||
                    pPreViewOpt->IsShowHiddenPara() != aUsrPref.IsShowHiddenPara() ||
                    pPreViewOpt->IsShowHiddenChar() != aUsrPref.IsShowHiddenChar() )
                rPreviewViewShell.ApplyViewOptions(aUsrPref);
            // reset design mode at draw view for form
            // shell, if needed.
            if ( ((SwPagePreView*)pExistingSh)->ResetFormDesignMode() &&
                 pWrtShell->HasDrawView() )
            {
                SdrView* pDrawView = pWrtShell->GetDrawView();
                pDrawView->SetDesignMode( ((SwPagePreView*)pExistingSh)->FormDesignModeToReset() );
            }
        }
    }
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after create WrtShell" );

    // assure that modified state of document
    // isn't reset, if document is already modified.
    const bool bIsDocModified = pWrtShell->GetDoc()->IsModified();

    // damit unter anderem das HLineal im
    //              ReadonlyFall nicht angezeigt wird
    aUsrPref.SetReadonly( pWrtShell->GetViewOptions()->IsReadonly() );

    // no margin for OLE!
    Size aBrwsBorder;
    if( SFX_CREATE_MODE_EMBEDDED != pDocSh->GetCreateMode() )
        aBrwsBorder = GetMargin();

    pWrtShell->SetBrowseBorder( aBrwsBorder );

    // Im CTOR duerfen keine Shell wechsel erfolgen, die muessen ueber
    // den Timer "zwischen gespeichert" werden. Sonst raeumt der SFX
    // sie wieder vom Stack!
    sal_Bool bOld = bNoInterrupt;
    bNoInterrupt = sal_True;

    pHRuler->SetActive( sal_True );
    pVRuler->SetActive( sal_True );

    SfxViewFrame* pViewFrame = GetViewFrame();
    if( pViewFrame->GetFrame().GetParentFrame())
    {
        aUsrPref.SetViewHRuler(sal_False);
        aUsrPref.SetViewVRuler(sal_False);
    }

    StartListening( *pViewFrame, sal_True );
    StartListening( *pDocSh, sal_True );

    // Set Zoom-factor from HLineal
    Fraction aZoomFract( aUsrPref.GetZoom(), 100 );
    pHRuler->SetZoom( aZoomFract );
    pVRuler->SetZoom( aZoomFract );
    pHRuler->SetDoubleClickHdl(LINK( this, SwView, ExecRulerClick ));
    FieldUnit eMetric = pUsrPref->GetHScrollMetric();
    pHRuler->SetUnit( eMetric );

    eMetric = pUsrPref->GetVScrollMetric();
    pVRuler->SetUnit( eMetric );

        pHRuler->SetCharWidth( 371 );  // default character width
        pVRuler->SetLineHeight( 551 );  // default line height

    // DocShell setzen
    pDocSh->SetView( this );
    SW_MOD()->SetView( this );

    mpPostItMgr = new SwPostItMgr(this);

    // Die DocSize erfragen und verarbeiten. Ueber die Handler konnte
    // die Shell nicht gefunden werden, weil die Shell innerhalb CTOR-Phase
    // nicht in der SFX-Verwaltung bekannt ist.
    DocSzChgd( pWrtShell->GetDocSize() );

        // AttrChangedNotify Link setzen
    pWrtShell->SetChgLnk(LINK(this, SwView, AttrChangedNotify));

    if( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
        //TODO/LATER: why a cast here?
        //!((SvEmbeddedObject *)pDocSh)->GetVisArea().IsEmpty() )
        //SetVisArea( ((SvEmbeddedObject *)pDocSh)->GetVisArea(),sal_False);
        !pDocSh->GetVisArea(ASPECT_CONTENT).IsEmpty() )
        SetVisArea( pDocSh->GetVisArea(ASPECT_CONTENT),sal_False);

    SAL_WARN_IF(
        officecfg::Office::Common::Undo::Steps::get() <= 0,
        "sw", "/org.openoffice.Office.Common/Undo/Steps <= 0");
    pWrtShell->DoUndo( true );

    const sal_Bool bBrowse = pWrtShell->GetViewOptions()->getBrowseMode();
    // Disable "multiple window"
    SetNewWindowAllowed(!bBrowse);
    // End of disabled multiple window

    ShowVScrollbar(aUsrPref.IsViewVScrollBar());
    ShowHScrollbar(aUsrPref.IsViewHScrollBar());
    pHScrollbar->SetAuto(bBrowse);
    if( aUsrPref.IsViewHRuler() )
        CreateTab();
    if( aUsrPref.IsViewVRuler() )
        CreateVLineal();

    pWrtShell->SetUIOptions( aUsrPref );
    pWrtShell->SetReadOnlyAvailable( aUsrPref.IsCursorInProtectedArea() );
    pWrtShell->ApplyAccessiblityOptions(SW_MOD()->GetAccessibilityOptions());

    if( pWrtShell->GetDoc()->IsUpdateExpFld() )
    {
        if (pWrtShell->GetDoc()->containsUpdatableFields())
        {
            SET_CURR_SHELL( pWrtShell );
            pWrtShell->StartAction();
            pWrtShell->CalcLayout();
            pWrtShell->GetDoc()->UpdateFlds(NULL, false);
            pWrtShell->EndAction();
        }
        pWrtShell->GetDoc()->SetUpdateExpFldStat( sal_False );
    }

    // ggfs. alle Verzeichnisse updaten:
    if( pWrtShell->GetDoc()->IsUpdateTOX() )
    {
        SfxRequest aSfxRequest( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
        Execute( aSfxRequest );
        pWrtShell->GetDoc()->SetUpdateTOX( sal_False );     // wieder zurueck setzen
        pWrtShell->SttEndDoc(sal_True);
    }

    // kein ResetModified, wenn es schone eine View auf dieses Doc gibt
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    // zur Zeit(360) wird die View erst nach dem Ctor eingetragen
    // der folgende Ausdruck funktioniert auch, wenn sich das aendert
    // wenn per Undo nicht mehr die Modifizierung aufhebar ist,
    //              so setze das Modified NICHT zurueck.
    // no reset of modified state, if document
    // was already modified.
    if (!pWrtShell->GetDoc()->GetIDocumentUndoRedo().IsUndoNoResetModified() &&
         ( !pFirst || pFirst == pVFrame ) &&
         !bIsDocModified )
    {
        pWrtShell->ResetModified();
    }

    bNoInterrupt = bOld;

    // wird ein GlobalDoc neu angelegt, soll auch der Navigator erzeugt werden
    if( pDocSh->IsA(SwGlobalDocShell::StaticType()) &&
        !pVFrame->GetChildWindow( SID_NAVIGATOR ))
    {
        SfxBoolItem aNavi(SID_NAVIGATOR, sal_True);
        GetDispatcher().Execute(SID_NAVIGATOR, SFX_CALLMODE_ASYNCHRON, &aNavi, 0L);
    }

    uno::Reference< frame::XFrame >  xFrame = pVFrame->GetFrame().GetFrameInterface();

    uno::Reference< frame::XFrame >  xBeamerFrame = xFrame->findFrame(
            OUString(RTL_CONSTASCII_USTRINGPARAM("_beamer")), frame::FrameSearchFlag::CHILDREN);
    if(xBeamerFrame.is())
    {
        SwDBData aData = pWrtShell->GetDBData();
        SW_MOD()->ShowDBObj( *this, aData );
    }

    // has anybody calls the attrchanged handler in the constructor?
    if( bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        if( aTimer.IsActive() )
            aTimer.Stop();
    }

    aTimer.SetTimeoutHdl(LINK(this, SwView, TimeoutHdl));
    bAttrChgNotified = bAttrChgNotifiedWithRegistrations = sal_False;
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_True );
    InvalidateBorder();

    if( !pHScrollbar->IsVisible( sal_True ) )
        ShowHScrollbar( sal_False );
    if( !pVScrollbar->IsVisible( sal_True ) )
        ShowVScrollbar( sal_False );

    GetViewFrame()->GetWindow().AddChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
}

SwView::~SwView()
{
    GetViewFrame()->GetWindow().RemoveChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
    delete mpPostItMgr;
    mpPostItMgr = 0;

    bInDtor = sal_True;
    pEditWin->Hide(); // damit kein Paint Aerger machen kann!
    // An der SwDocShell den Pointer auf die View ruecksetzen
    SwDocShell* pDocSh = GetDocShell();
    if( pDocSh && pDocSh->GetView() == this )
        pDocSh->SetView( 0 );
    if ( SW_MOD()->GetView() == this )
        SW_MOD()->SetView( 0 );

    if( aTimer.IsActive() && bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    // the last view must end the text edit
    SdrView *pSdrView = pWrtShell ? pWrtShell->GetDrawView() : 0;
    if( pSdrView && pSdrView->IsTextEdit() )
        pSdrView->SdrEndTextEdit( sal_True );

    SetWindow( 0 );

    pViewImpl->Invalidate();
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    delete pScrollFill;
    delete pWrtShell;
    pWrtShell = 0;      // Auf 0 setzen, damit folgende DToren nicht drauf zugreifen
    pShell = 0;
    delete pHScrollbar;
    delete pVScrollbar;
    delete pHRuler;
    delete pVRuler;
    delete pTogglePageBtn;
    delete pPageUpBtn;
    delete pNaviBtn;
    delete pPageDownBtn;
    delete pGlosHdl;
    delete pViewImpl;
    delete pEditWin;
    delete pFormatClipboard;
}

SwDocShell* SwView::GetDocShell()
{
    SfxObjectShell* pDocShell = GetViewFrame()->GetObjectShell();
    return PTR_CAST(SwDocShell, pDocShell);
}

/*--------------------------------------------------------------------
    Beschreibung:   CursorPos merken
 --------------------------------------------------------------------*/
void SwView::WriteUserData( String &rUserData, sal_Bool bBrowse )
{
    //Das Browse-Flag wird vom Sfx durchgereicht, wenn Dokumente gebrowsed
    //werden (nicht zu verwechseln mit dem BrowseMode).
    //Die dann gespeicherten Daten sind nicht persistent!!

    const SwRect& rRect = pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();

    rUserData = String::CreateFromInt32( rRect.Left() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rRect.Top() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( pWrtShell->GetViewOptions()->GetZoom() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rVis.Left() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( rVis.Top() );
    rUserData += ';';
    rUserData += String::CreateFromInt32( bBrowse ? SAL_MIN_INT32 : rVis.Right());
    rUserData += ';';
    rUserData += String::CreateFromInt32( bBrowse ? SAL_MIN_INT32 : rVis.Bottom());
    rUserData += ';';
    rUserData += String::CreateFromInt32(
            (sal_uInt16)pWrtShell->GetViewOptions()->GetZoomType());//eZoom;
    rUserData += ';';
    rUserData += FRMTYPE_NONE == pWrtShell->GetSelFrmType() ? '0' : '1';
}

/*--------------------------------------------------------------------
    Beschreibung: CursorPos setzen
 --------------------------------------------------------------------*/
static bool lcl_IsOwnDocument( SwView& rView )
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        rView.GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps
        = xDPS->getDocumentProperties();
    String Created = xDocProps->getAuthor();
    String Changed = xDocProps->getModifiedBy();
    String FullName = SW_MOD()->GetUserOptions().GetFullName();
    return (FullName.Len() &&
            (Changed.Len() && Changed == FullName )) ||
            (!Changed.Len() && Created.Len() && Created == FullName );
}

void SwView::ReadUserData( const String &rUserData, sal_Bool bBrowse )
{
    if ( comphelper::string::getTokenCount(rUserData, ';') > 1 &&
        //Fuer Dokumente ohne Layout nur im OnlineLayout oder beim
        //Forward/Backward
         (!pWrtShell->IsNewLayout() || pWrtShell->GetViewOptions()->getBrowseMode() || bBrowse) )
    {
        bool bIsOwnDocument = lcl_IsOwnDocument( *this );

        SET_CURR_SHELL(pWrtShell);

        sal_uInt16 nPos = 0;

        // Nein, es ist *keine* gute Idee GetToken gleich im Point-Konstr.
        // aufzurufen, denn welcher Parameter wird zuerst ausgewertet?
        long nX = rUserData.GetToken( 0, ';', nPos ).ToInt32(),
             nY = rUserData.GetToken( 0, ';', nPos ).ToInt32();
        Point aCrsrPos( nX, nY );

        sal_uInt16 nZoomFactor =
            static_cast< sal_uInt16 >( rUserData.GetToken(0, ';', nPos ).ToInt32() );

        long nLeft  = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nTop   = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nRight = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nBottom= rUserData.GetToken(0, ';', nPos ).ToInt32();

        const long nAdd = pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
        if ( nBottom <= (pWrtShell->GetDocSize().Height()+nAdd) )
        {
            pWrtShell->EnableSmooth( sal_False );

            const Rectangle aVis( nLeft, nTop, nRight, nBottom );

            sal_uInt16 nOff = 0;
            SvxZoomType eZoom;
            if( !pWrtShell->GetViewOptions()->getBrowseMode() )
                eZoom = (SvxZoomType) (sal_uInt16)rUserData.GetToken(nOff, ';', nPos ).ToInt32();
            else
            {
                eZoom = SVX_ZOOM_PERCENT;
                ++nOff;
            }

            sal_Bool bSelectObj = (0 != rUserData.GetToken( nOff, ';', nPos ).ToInt32())
                                && pWrtShell->IsObjSelectable( aCrsrPos );

            // restore editing position
            pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
            // set flag value to avoid macro execution.
            bool bSavedFlagValue = pWrtShell->IsMacroExecAllowed();
            pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(bOldShellWasPagePreView || bIsOwnDocument)
            {
                pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                if( bSelectObj )
                {
                    pWrtShell->SelectObj( aCrsrPos );
                    pWrtShell->EnterSelFrmMode( &aCrsrPos );
                }
            }

            // reset flag value
            pWrtShell->SetMacroExecAllowed( bSavedFlagValue );

            // set visible area before applying
            // information from print preview. Otherwise, the applied information
            // is lost.
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(bOldShellWasPagePreView || bIsOwnDocument )
            {
                if ( bBrowse )
                    SetVisArea( aVis.TopLeft() );
                else
                    SetVisArea( aVis );
            }

            //apply information from print preview - if available
            if( sNewCrsrPos.Len() )
            {
                long nXTmp = sNewCrsrPos.GetToken( 0, ';' ).ToInt32(),
                      nYTmp = sNewCrsrPos.GetToken( 1, ';' ).ToInt32();
                Point aCrsrPos2( nXTmp, nYTmp );
                bSelectObj = pWrtShell->IsObjSelectable( aCrsrPos2 );

                pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos2, sal_False );
                if( bSelectObj )
                {
                    pWrtShell->SelectObj( aCrsrPos2 );
                    pWrtShell->EnterSelFrmMode( &aCrsrPos2 );
                }
                pWrtShell->MakeSelVisible();
                sNewCrsrPos.Erase();
            }
            else if(USHRT_MAX != nNewPage)
            {
                pWrtShell->GotoPage(nNewPage, sal_True);
                nNewPage = USHRT_MAX;
            }

            SelectShell();

            pWrtShell->StartAction();
            const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
            if( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom )
                SetZoom( eZoom, nZoomFactor);

            pWrtShell->LockView( sal_True );
            pWrtShell->EndAction();
            pWrtShell->LockView( sal_False );
            pWrtShell->EnableSmooth( sal_True );
        }
    }
}

void SwView::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    if(GetDocShell()->IsPreview()||bIsPreviewDoubleClick)
        return;
    bool bIsOwnDocument = lcl_IsOwnDocument( *this );
    sal_Int32 nLength = rSequence.getLength();
    if (nLength && (!pWrtShell->IsNewLayout() || pWrtShell->GetViewOptions()->getBrowseMode() || bBrowse) )
    {
        SET_CURR_SHELL(pWrtShell);
        const beans::PropertyValue *pValue = rSequence.getConstArray();
        const SwRect& rRect = pWrtShell->GetCharRect();
        const Rectangle &rVis = GetVisArea();
        const SwViewOption* pVOpt = pWrtShell->GetViewOptions();

        long nX = rRect.Left(), nY = rRect.Top(), nLeft = rVis.Left(), nTop = rVis.Top();
        long nRight = bBrowse ? LONG_MIN : rVis.Right(), nBottom = bBrowse ? LONG_MIN : rVis.Bottom();
        sal_Int16 nZoomType = static_cast< sal_Int16 >(pVOpt->GetZoomType());
        sal_Int16 nZoomFactor = static_cast < sal_Int16 > (pVOpt->GetZoom());
        bool bViewLayoutBookMode = pVOpt->IsViewLayoutBookMode();
        sal_Int16 nViewLayoutColumns = pVOpt->GetViewLayoutColumns();

        sal_Bool bSelectedFrame = ( pWrtShell->GetSelFrmType() != FRMTYPE_NONE ),
                 bGotVisibleLeft = sal_False,
                 bGotVisibleTop = sal_False, bGotVisibleRight = sal_False,
                 bGotVisibleBottom = sal_False, bGotZoomType = sal_False,
                 bGotZoomFactor = sal_False, bGotIsSelectedFrame = sal_False,
                 bGotViewLayoutColumns = sal_False, bGotViewLayoutBookMode = sal_False;

        for (sal_Int16 i = 0 ; i < nLength; i++)
        {
            if ( pValue->Name == "ViewLeft" )
            {
               pValue->Value >>= nX;
               nX = MM100_TO_TWIP( nX );
            }
            else if ( pValue->Name == "ViewTop" )
            {
               pValue->Value >>= nY;
               nY = MM100_TO_TWIP( nY );
            }
            else if ( pValue->Name == "VisibleLeft" )
            {
               pValue->Value >>= nLeft;
               nLeft = MM100_TO_TWIP( nLeft );
               bGotVisibleLeft = sal_True;
            }
            else if ( pValue->Name == "VisibleTop" )
            {
               pValue->Value >>= nTop;
               nTop = MM100_TO_TWIP( nTop );
               bGotVisibleTop = sal_True;
            }
            else if ( pValue->Name == "VisibleRight" )
            {
               pValue->Value >>= nRight;
               nRight = MM100_TO_TWIP( nRight );
               bGotVisibleRight = sal_True;
            }
            else if ( pValue->Name == "VisibleBottom" )
            {
               pValue->Value >>= nBottom;
               nBottom = MM100_TO_TWIP( nBottom );
               bGotVisibleBottom = sal_True;
            }
            else if ( pValue->Name == "ZoomType" )
            {
               pValue->Value >>= nZoomType;
               bGotZoomType = sal_True;
            }
            else if ( pValue->Name == "ZoomFactor" )
            {
               pValue->Value >>= nZoomFactor;
               bGotZoomFactor = sal_True;
            }
            else if ( pValue->Name == "ViewLayoutColumns" )
            {
               pValue->Value >>= nViewLayoutColumns;
               bGotViewLayoutColumns = sal_True;
            }
            else if ( pValue->Name == "ViewLayoutBookMode" )
            {
               bViewLayoutBookMode = * (sal_Bool *) pValue->Value.getValue();
               bGotViewLayoutBookMode = sal_True;
            }
            else if ( pValue->Name == "IsSelectedFrame" )
            {
               pValue->Value >>= bSelectedFrame;
               bGotIsSelectedFrame = sal_True;
            }
            pValue++;
        }
        if (bGotVisibleBottom)
        {
            Point aCrsrPos( nX, nY );
            const long nAdd = pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
            if (nBottom <= (pWrtShell->GetDocSize().Height()+nAdd) )
            {
                pWrtShell->EnableSmooth( sal_False );
                const Rectangle aVis( nLeft, nTop, nRight, nBottom );

                SvxZoomType eZoom;
                if ( !pWrtShell->GetViewOptions()->getBrowseMode() )
                    eZoom = static_cast < SvxZoomType > ( nZoomType );
                else
                {
                    eZoom = SVX_ZOOM_PERCENT;
                }
                if (bGotIsSelectedFrame)
                {
                    sal_Bool bSelectObj = (sal_False != bSelectedFrame )
                                        && pWrtShell->IsObjSelectable( aCrsrPos );

                    // set flag value to avoid macro execution.
                    bool bSavedFlagValue = pWrtShell->IsMacroExecAllowed();
                    pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                    pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
                    if(bOldShellWasPagePreView|| bIsOwnDocument)
                    {
                        pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                        if( bSelectObj )
                        {
                            pWrtShell->SelectObj( aCrsrPos );
                            pWrtShell->EnterSelFrmMode( &aCrsrPos );
                        }
                    }

                    // reset flag value
                    pWrtShell->SetMacroExecAllowed( bSavedFlagValue );
                }
                SelectShell();

                // Set ViewLayoutSettings
                const bool bSetViewLayoutSettings = bGotViewLayoutColumns && bGotViewLayoutBookMode &&
                                                    ( pVOpt->GetViewLayoutColumns() != nViewLayoutColumns || pVOpt->IsViewLayoutBookMode() != bViewLayoutBookMode );

                const bool bSetViewSettings = bGotZoomType && bGotZoomFactor &&
                                              ( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom );

                // In case we have a 'fixed' view layout of 2 or more columns,
                // we have to apply the view options *before* starting the action.
                // Otherwsie the SetZoom function cannot work correctly, because
                // the view layout hasn't been calculated.
                const bool bZoomNeedsViewLayout = bSetViewLayoutSettings &&
                                                  1 < nViewLayoutColumns &&
                                                  bSetViewSettings &&
                                                  eZoom != SVX_ZOOM_PERCENT;

                if ( !bZoomNeedsViewLayout )
                    pWrtShell->StartAction();

                if ( bSetViewLayoutSettings )
                    SetViewLayout( nViewLayoutColumns, bViewLayoutBookMode, sal_True );

                if ( bZoomNeedsViewLayout )
                    pWrtShell->StartAction();

                if ( bSetViewSettings )
                    SetZoom( eZoom, nZoomFactor, sal_True );

// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                if(bOldShellWasPagePreView||bIsOwnDocument)
                {
                    if ( bBrowse && bGotVisibleLeft && bGotVisibleTop )
                    {
                        Point aTopLeft(aVis.TopLeft());
                        // make sure the document is still centered
                        const SwTwips lBorder = IsDocumentBorder() ? DOCUMENTBORDER : 2 * DOCUMENTBORDER;
                        SwTwips nEditWidth = GetEditWin().GetOutputSize().Width();
                        if(nEditWidth > (aDocSz.Width() + lBorder ))
                            aTopLeft.X() = ( aDocSz.Width() + lBorder - nEditWidth  ) / 2;
                        else
                        {
                            //check if the values are possible
                            long nXMax = pHScrollbar->GetRangeMax() - pHScrollbar->GetVisibleSize();
                            if( aTopLeft.X() > nXMax )
                                aTopLeft.X() = nXMax < 0 ? 0 : nXMax;
                        }
                        SetVisArea( aTopLeft );
                    }
                    else if (bGotVisibleLeft && bGotVisibleTop && bGotVisibleRight && bGotVisibleBottom )
                        SetVisArea( aVis );
                }

                pWrtShell->LockView( sal_True );
                pWrtShell->EndAction();
                pWrtShell->LockView( sal_False );
                pWrtShell->EnableSmooth( sal_True );
            }
        }
    }
}

#define NUM_VIEW_SETTINGS 12
void SwView::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    const SwRect& rRect = pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();
    Any aAny;

    rSequence.realloc ( NUM_VIEW_SETTINGS );
    sal_Int16 nIndex = 0;
    beans::PropertyValue *pValue = rSequence.getArray();

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    pValue->Name = rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( "ViewId" ) );
    OUStringBuffer sBuffer ( OUString(RTL_CONSTASCII_USTRINGPARAM( "view" ) ) );
    ::sax::Converter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
    pValue->Value <<= sBuffer.makeStringAndClear();
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewLeft" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewTop" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleLeft" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleTop" ) );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleRight" ) );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Right() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "VisibleBottom" ) );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Bottom() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZoomType" ) );
    const sal_Int16 nZoomType = static_cast< sal_Int16 >(pWrtShell->GetViewOptions()->GetZoomType());
    pValue->Value <<= nZoomType;
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewLayoutColumns" ) );
    const sal_Int16 nViewLayoutColumns = static_cast< sal_Int16 >(pWrtShell->GetViewOptions()->GetViewLayoutColumns());
    pValue->Value <<= nViewLayoutColumns;
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ViewLayoutBookMode" ) );
    const sal_Bool bIsViewLayoutBookMode = pWrtShell->GetViewOptions()->IsViewLayoutBookMode();
    pValue->Value.setValue( &bIsViewLayoutBookMode, ::getBooleanCppuType() );
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZoomFactor" ) );
    pValue->Value <<= static_cast < sal_Int16 > (pWrtShell->GetViewOptions()->GetZoom());
    pValue++;nIndex++;

    pValue->Name = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "IsSelectedFrame" ) );
    const sal_Bool bIsSelected = FRMTYPE_NONE == pWrtShell->GetSelFrmType() ? sal_False : sal_True;
    pValue->Value.setValue ( &bIsSelected, ::getBooleanCppuType() );
    nIndex++;

    if ( nIndex < NUM_VIEW_SETTINGS )
        rSequence.realloc ( nIndex );
}
#undef NUM_VIEW_SETTINGS

void SwView::ShowCursor( bool bOn )
{
    //don't scroll the cursor into the visible area
    sal_Bool bUnlockView = !pWrtShell->IsViewLocked();
    pWrtShell->LockView( sal_True );    //lock visible section

    if( !bOn )
        pWrtShell->HideCrsr();
    else if( !pWrtShell->IsFrmSelected() && !pWrtShell->IsObjSelected() )
        pWrtShell->ShowCrsr();

    if( bUnlockView )
        pWrtShell->LockView( sal_False );
}

ErrCode SwView::DoVerb( long nVerb )
{
    if ( !GetViewFrame()->GetFrame().IsInPlace() )
    {
        SwWrtShell &rSh = GetWrtShell();
        const int nSel = rSh.GetSelectionType();
        if ( nSel & nsSelectionType::SEL_OLE )
            rSh.LaunchOLEObj( nVerb );
    }
    return ERRCODE_NONE;
}

/*   only return sal_True for a text selection
*/
sal_Bool SwView::HasSelection( sal_Bool  bText ) const
{
    return bText ? GetWrtShell().SwCrsrShell::HasSelection()
                 : GetWrtShell().HasSelection();
}

String SwView::GetSelectionText( sal_Bool bCompleteWrds )
{
    return GetSelectionTextParam( bCompleteWrds, sal_True );
}

String  SwView::GetSelectionTextParam( sal_Bool bCompleteWrds,
                                       sal_Bool bEraseTrail )
{
    String sReturn;
    if( bCompleteWrds && !GetWrtShell().HasSelection() )
        GetWrtShell().SelWrd();

    GetWrtShell().GetSelectedText( sReturn );
    if( bEraseTrail )
        sReturn = comphelper::string::stripEnd(sReturn, ' ');
    return sReturn;
}

SwGlossaryHdl* SwView::GetGlosHdl()
{
    if(!pGlosHdl)
        pGlosHdl = new SwGlossaryHdl(GetViewFrame(), pWrtShell);
    return pGlosHdl;
}

void SwView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    sal_Bool bCallBase = sal_True;
    if ( rHint.ISA(SfxSimpleHint) )
    {
        sal_uInt32 nId = ((SfxSimpleHint&)rHint).GetId();
        switch ( nId )
        {
            // sub shells will be destroyed by the
            // dispatcher, if the view frame is dying. Thus, reset member <pShell>.
            case SFX_HINT_DYING:
                {
                    if ( &rBC == GetViewFrame() )
                    {
                        ResetSubShell();
                    }
                }
                break;
            case SFX_HINT_MODECHANGED:
                {
                    // Modalmodus-Umschaltung?
                    sal_Bool bModal = GetDocShell()->IsInModalMode();
                    pHRuler->SetActive( !bModal );
                    pVRuler->SetActive( !bModal );
                }

                /* no break here */

            case SFX_HINT_TITLECHANGED:
                if ( GetDocShell()->IsReadOnly() != GetWrtShell().GetViewOptions()->IsReadonly() )
                {
                    SwWrtShell &rSh = GetWrtShell();
                    rSh.SetReadonlyOption( GetDocShell()->IsReadOnly() );

                    if ( rSh.GetViewOptions()->IsViewVRuler() )
                        CreateVLineal();
                    else
                        KillVLineal();
                    if ( rSh.GetViewOptions()->IsViewHRuler() )
                        CreateTab();
                    else
                        KillTab();
                    bool bReadonly = GetDocShell()->IsReadOnly();
                    // if document is to be opened in alive-mode then this has to be regarded while switching from readonly-mode to edit-mode
                    if( !bReadonly )
                    {
                        SwDrawDocument * pDrawDoc = 0;
                        if ( 0 != ( pDrawDoc = dynamic_cast< SwDrawDocument * > (GetDocShell()->GetDoc()->GetDrawModel() ) ) )
                        {
                            if( !pDrawDoc->GetOpenInDesignMode() )
                                break;// don't touch the design mode
                        }
                    }
                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadonly);
                    GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_ASYNCHRON,
                                                &aItem, 0L );
                }
                break;

            case SW_BROADCAST_DRAWVIEWS_CREATED:
                {
                    bCallBase = sal_False;
                    if ( GetFormShell() )
                    {
                        GetFormShell()->SetView(
                            PTR_CAST(FmFormView, GetWrtShell().GetDrawView()) );
                        SfxBoolItem aItem( SID_FM_DESIGN_MODE, !GetDocShell()->IsReadOnly());
                        GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_SYNCHRON,
                                                  &aItem, 0L );
                    }
                }
                break;
        }
    }
    else if(rHint.ISA(FmDesignModeChangedHint))
    {
        sal_Bool bDesignMode = ((FmDesignModeChangedHint&)rHint).GetDesignMode();
        if (!bDesignMode && GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(NULL);
            LeaveDrawCreate();
            AttrChangedNotify(pWrtShell);
        }
    }

    if ( bCallBase )
        SfxViewShell::Notify(rBC, rHint);
}

#if defined WNT || defined UNX

void SwView::ScannerEventHdl( const EventObject& /*rEventObject*/ )
{
    uno::Reference< XScannerManager2 > xScanMgr = SW_MOD()->GetScannerManager();
    if( xScanMgr.is() )
    {
        const ScannerContext    aContext( xScanMgr->getAvailableScanners().getConstArray()[ 0 ] );
        const ScanError         eError = xScanMgr->getError( aContext );

        if( ScanError_ScanErrorNone == eError )
        {
            const uno::Reference< awt::XBitmap > xBitmap( xScanMgr->getBitmap( aContext ) );

            if( xBitmap.is() )
            {
                const BitmapEx aScanBmp( VCLUnoHelper::GetBitmap( xBitmap ) );

                if( !!aScanBmp )
                {
                    Graphic aGrf(aScanBmp);
                    pWrtShell->Insert( aEmptyStr, aEmptyStr, aGrf );
                }
            }
        }
    }
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate( SID_TWAIN_SELECT );
    rBind.Invalidate( SID_TWAIN_TRANSFER );
}
#endif

void    SwView::StopShellTimer()
{
    if(aTimer.IsActive())
    {
        aTimer.Stop();
        if ( bAttrChgNotifiedWithRegistrations )
        {
            GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
            bAttrChgNotifiedWithRegistrations = sal_False;
        }
        SelectShell();
        bAttrChgNotified = sal_False;
    }
}

sal_uInt16  SwView::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    SfxViewFrame* pVFrame = GetViewFrame();
    pVFrame->SetChildWindow( SwInputChild::GetChildWindowId(), sal_False );
    if( pVFrame->GetDispatcher()->IsLocked() )
        pVFrame->GetDispatcher()->Lock(sal_False);

    sal_uInt16 nRet;
    if ( pFormShell &&
         sal_True != (nRet = pFormShell->PrepareClose( bUI, bForBrowsing )) )

    {
        return nRet;
    }
    return SfxViewShell::PrepareClose( bUI, bForBrowsing );
}

// status methods for clipboard.
// Status changes now notified from the clipboard.
sal_Bool SwView::IsPasteAllowed()
{
    sal_uInt16 nPasteDestination = SwTransferable::GetSotDestination( *pWrtShell );
    if( nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            bPasteState = SwTransferable::IsPaste( *pWrtShell, aDataHelper );
            bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *pWrtShell, aDataHelper );
        }
        else
            bPasteState = bPasteSpecialState = sal_False;

        if( 0xFFFF == nLastPasteDestination )  // the init value
            pViewImpl->AddClipboardListener();
        nLastPasteDestination = nPasteDestination;
    }
    return bPasteState;
}

sal_Bool SwView::IsPasteSpecialAllowed()
{
    if ( pFormShell && pFormShell->IsActiveControl() )
        return sal_False;

    sal_uInt16 nPasteDestination = SwTransferable::GetSotDestination( *pWrtShell );
    if( nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            bPasteState = SwTransferable::IsPaste( *pWrtShell, aDataHelper );
            bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *pWrtShell, aDataHelper );
        }
        else
            bPasteState = bPasteSpecialState = sal_False;

        if( 0xFFFF == nLastPasteDestination )  // the init value
            pViewImpl->AddClipboardListener();
    }
    return bPasteSpecialState;
}

void SwView::NotifyDBChanged()
{
    GetViewImpl()->GetUNOObject_Impl()->NotifyDBChanged();
}

/*--------------------------------------------------------------------
    Printing
 --------------------------------------------------------------------*/
SfxObjectShellLock SwView::CreateTmpSelectionDoc()
{
    SwXTextView *const pImpl = GetViewImpl()->GetUNOObject_Impl();
    return pImpl->BuildTmpSelectionDoc();
}

void SwView::AddTransferable(SwTransferable& rTransferable)
{
    GetViewImpl()->AddTransferable(rTransferable);
}

namespace sw {

void InitPrintOptionsFromApplication(SwPrintData & o_rData, bool const bWeb)
{
    o_rData = *SW_MOD()->GetPrtOptions(bWeb);
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
