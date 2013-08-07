/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <string>
#include <stdlib.h>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
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
#include <toolkit/helper/vclunohelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>

#include "formatclipboard.hxx"
#include <PostItMgr.hxx>
#include <annotsh.hxx>
#include <swruler.hxx>

#include <fldbas.hxx>

#include <unomid.h>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;

extern bool bNoInterrupt;       // in mainwn.cxx

#define SWVIEWFLAGS ( SFX_VIEW_CAN_PRINT|               \
                      SFX_VIEW_HAS_PRINTOPTIONS)

// Statics

int bDocSzUpdated = 1;

SvxSearchItem*  SwView::m_pSrchItem   = 0;

sal_uInt16          SwView::m_nInsertCtrlState        = FN_INSERT_TABLE;
sal_uInt16          SwView::m_nWebInsertCtrlState     = FN_INSERT_TABLE;
sal_uInt16          SwView::m_nInsertObjectCtrlState  = SID_INSERT_DIAGRAM;

bool            SwView::m_bExtra      = false;
sal_Bool            SwView::m_bFound      = sal_False;
bool            SwView::m_bJustOpened = false;

SvxSearchDialog*    SwView::m_pSrchDlg    = 0;
SearchAttrItemList* SwView::m_pSrchList   = 0;
SearchAttrItemList* SwView::m_pReplList   = 0;

DBG_NAME(viewhdl)

inline SfxDispatcher &SwView::GetDispatcher()
{
    return *GetViewFrame()->GetDispatcher();
}

void SwView::ImpSetVerb( int nSelType )
{
    sal_Bool bResetVerbs = m_bVerbsActive;
    if ( !GetViewFrame()->GetFrame().IsInPlace() &&
         (nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) & nSelType )
    {
        if ( !m_pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT) )
        {
            if ( nSelType & nsSelectionType::SEL_OLE )
            {
                SetVerbs( GetWrtShell().GetOLEObject()->getSupportedVerbs() );
                m_bVerbsActive = sal_True;
                bResetVerbs = sal_False;
            }
        }
    }
    if ( bResetVerbs )
    {
        SetVerbs( Sequence< embed::VerbDescriptor >() );
        m_bVerbsActive = sal_False;
    }
}

// Called by the SwEditWin when it gets the focus.

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
        const_cast< SwView* >( this )->AttrChangedNotify( m_pWrtShell );
    }
    else if ( m_pPostItMgr )
    {
        SwAnnotationShell* pAsAnnotationShell = PTR_CAST( SwAnnotationShell, pTopShell );
        if ( pAsAnnotationShell )
        {
            m_pPostItMgr->SetActiveSidebarWin(0);
            const_cast< SwView* >( this )->AttrChangedNotify( m_pWrtShell );
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

// called by the FormShell when a form control is focused. This is
// a request to put the form shell on the top of the dispatcher stack

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
        SdrView *pSdrView = m_pWrtShell ? m_pWrtShell->GetDrawView() : NULL;
        if ( pSdrView && pSdrView->IsTextEdit() )
            pSdrView->SdrEndTextEdit( sal_True );

        const_cast< SwView* >( this )->AttrChangedNotify( m_pWrtShell );
    }

    return 0L;
}

void SwView::SelectShell()
{


// Attention: Maintain the SelectShell for the WebView additionally


          if(m_bInDtor)
        return;
    // Decision if the UpdateTable has to be called
    sal_Bool bUpdateTable = sal_False;
    const SwFrmFmt* pCurTableFmt = m_pWrtShell->GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != m_pLastTableFormat)
    {
        bUpdateTable = sal_True; // can only be executed later
    }
    m_pLastTableFormat = pCurTableFmt;

    //SEL_TBL and SEL_TBL_CELLS can be ORed!
    int nNewSelectionType = (m_pWrtShell->GetSelectionType()
                                & ~nsSelectionType::SEL_TBL_CELLS);

    if ( m_pFormShell && m_pFormShell->IsActiveControl() )
        nNewSelectionType |= nsSelectionType::SEL_FOC_FRM_CTRL;

    if ( nNewSelectionType == m_nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        if ( m_nSelectionType & nsSelectionType::SEL_OLE ||
             m_nSelectionType & nsSelectionType::SEL_GRF )
            // For graphs and OLE the verb can be modified of course!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {

        SfxDispatcher &rDispatcher = GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetToolbarConfig();

        if ( m_pShell )
        {
            rDispatcher.Flush();        // Really erase all cached shells
            //Remember to the old selection which toolbar was visible
            sal_uInt16 nId = static_cast< sal_uInt16 >( rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT ));
            if ( nId )
                pBarCfg->SetTopToolbar( m_nSelectionType, nId );

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
        if (!m_pFormShell)
        {
            bInitFormShell = sal_True;
            m_pFormShell = new FmFormShell( this );
            m_pFormShell->SetControlActivationHandler( LINK( this, SwView, FormControlActivated ) );
            StartListening(*m_pFormShell);
        }

        sal_Bool bSetExtInpCntxt = sal_False;
        m_nSelectionType = nNewSelectionType;
        ShellModes eShellMode;

        if ( !( m_nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL ) )
            rDispatcher.Push( *m_pFormShell );

        eShellMode = SHELL_MODE_NAVIGATION;
        m_pShell = new SwNavigationShell( *this );
        rDispatcher.Push( *m_pShell );

        if ( m_nSelectionType & nsSelectionType::SEL_OLE )
        {
            eShellMode = SHELL_MODE_OBJECT;
            m_pShell = new SwOleShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & nsSelectionType::SEL_FRM
            || m_nSelectionType & nsSelectionType::SEL_GRF)
        {
            eShellMode = SHELL_MODE_FRAME;
            m_pShell = new SwFrameShell( *this );
            rDispatcher.Push( *m_pShell );
            if(m_nSelectionType & nsSelectionType::SEL_GRF )
            {
                eShellMode = SHELL_MODE_GRAPHIC;
                m_pShell = new SwGrfShell( *this );
                rDispatcher.Push( *m_pShell );
            }
        }
        else if ( m_nSelectionType & nsSelectionType::SEL_DRW )
        {
            eShellMode = SHELL_MODE_DRAW;
            m_pShell = new SwDrawShell( *this );
            rDispatcher.Push( *m_pShell );

            if ( m_nSelectionType & nsSelectionType::SEL_BEZ )
            {
                eShellMode = SHELL_MODE_BEZIER;
                m_pShell = new SwBezierShell( *this );
                rDispatcher.Push( *m_pShell );
            }
            else if( m_nSelectionType & nsSelectionType::SEL_MEDIA )
            {
                eShellMode = SHELL_MODE_MEDIA;
                m_pShell = new SwMediaShell( *this );
                rDispatcher.Push( *m_pShell );
            }

            if (m_nSelectionType & nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE)
            {
                eShellMode = SHELL_MODE_EXTRUDED_CUSTOMSHAPE;
                m_pShell = new svx::ExtrusionBar(this);
                rDispatcher.Push( *m_pShell );
            }
            if (m_nSelectionType & nsSelectionType::SEL_FONTWORK)
            {
                eShellMode = SHELL_MODE_FONTWORK;
                m_pShell = new svx::FontworkBar(this);
                rDispatcher.Push( *m_pShell );
            }
        }
        else if ( m_nSelectionType & nsSelectionType::SEL_DRW_FORM )
        {
            eShellMode = SHELL_MODE_DRAW_FORM;
            m_pShell = new SwDrawFormShell( *this );

            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & nsSelectionType::SEL_DRW_TXT )
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            m_pShell = new SwDrawTextShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & nsSelectionType::SEL_POSTIT )
        {
            eShellMode = SHELL_MODE_POSTIT;
            m_pShell = new SwAnnotationShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else
        {
            bSetExtInpCntxt = sal_True;
            eShellMode = SHELL_MODE_TEXT;
            sal_uInt32 nHelpId = 0;
            if ( m_nSelectionType & nsSelectionType::SEL_NUM )
            {
                eShellMode = SHELL_MODE_LIST_TEXT;
                m_pShell = new SwListShell( *this );
                nHelpId = m_pShell->GetHelpId();
                rDispatcher.Push( *m_pShell );
            }
            m_pShell = new SwTextShell(*this);
            if(nHelpId)
                m_pShell->SetHelpId(nHelpId);
            rDispatcher.Push( *m_pShell );
            if ( m_nSelectionType & nsSelectionType::SEL_TBL )
            {
                eShellMode = eShellMode == SHELL_MODE_LIST_TEXT ? SHELL_MODE_TABLE_LIST_TEXT
                                                        : SHELL_MODE_TABLE_TEXT;
                m_pShell = new SwTableShell( *this );
                rDispatcher.Push( *m_pShell );
            }
        }

        if ( m_nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL )
            rDispatcher.Push( *m_pFormShell );

        m_pViewImpl->SetShellMode(eShellMode);
        ImpSetVerb( m_nSelectionType );

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

        // Activate the toolbar to the new selection which also was active last time.
        // Before a flush () must be, but does not affect the UI according to MBA and
        // is not a performance problem.
        // TODO/LATER: maybe now the Flush() command is superfluous?!
        rDispatcher.Flush();

        Point aPnt = GetEditWin().OutputToScreenPixel(GetEditWin().GetPointerPosPixel());
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        SdrView* pDView = GetWrtShell().GetDrawView();
        if ( bInitFormShell && pDView )
            m_pFormShell->SetView(PTR_CAST(FmFormView, pDView));

    }
    // Opportune time for the communication with OLE objects?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( sal_False );

    // now the table-update
    if(bUpdateTable)
        m_pWrtShell->UpdateTable();

    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();
}

// Interaction: AttrChangedNotify() and TimeoutHdl.
// No Update if actions are still open, since the cursor on the core side
// can be somewhere in no man's land.
// But since we can no longer supply status and we want instead lock
// the dispatcher.

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

    //Opt: Not if PaintLocked. During unlock a notify will be once more triggered.
    if( !m_pWrtShell->IsPaintLocked() && !bNoInterrupt &&
        GetDocShell()->IsReadOnly() )
        _CheckReadonlyState();

    if( !m_pWrtShell->IsPaintLocked() && !bNoInterrupt )
        _CheckReadonlySelection();

    if( !m_bAttrChgNotified )
    {
        if ( m_pWrtShell->BasicActionPend() || bNoInterrupt ||
             GetDispatcher().IsLocked() ||               //do not confuse the SFX
             GetViewFrame()->GetBindings().IsInUpdate() )//do not confuse the SFX
        {
            m_bAttrChgNotified = sal_True;
            m_aTimer.Start();

            const SfxPoolItem *pItem;
            if ( SFX_ITEM_SET != GetObjectShell()->GetMedium()->GetItemSet()->
                                    GetItemState( SID_HIDDEN, sal_False, &pItem ) ||
                 !((SfxBoolItem*)pItem)->GetValue() )
            {
                GetViewFrame()->GetBindings().ENTERREGISTRATIONS();
                m_bAttrChgNotifiedWithRegistrations = sal_True;
            }

        }
        else
            SelectShell();

    }

    // change ui if cursor is at a SwPostItField
    if (m_pPostItMgr)
    {
        // only perform the code that is needed to determine, if at the
        // actual cursor position is a post-it field
        m_pPostItMgr->SetShadowState( m_pWrtShell->GetPostItFieldAtCursor() );
    }

    return 0;
}

IMPL_LINK_NOARG(SwView, TimeoutHdl)
{
    DBG_PROFSTART(viewhdl);

    if( m_pWrtShell->BasicActionPend() || bNoInterrupt )
    {
        m_aTimer.Start();
        DBG_PROFSTOP(viewhdl);
        return 0;
    }

    if ( m_bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        m_bAttrChgNotifiedWithRegistrations = sal_False;
    }

    _CheckReadonlyState();
    _CheckReadonlySelection();

    sal_Bool bOldUndo = m_pWrtShell->DoesUndo();
    m_pWrtShell->DoUndo( sal_False );
    SelectShell();
    m_pWrtShell->DoUndo( bOldUndo );
    m_bAttrChgNotified = sal_False;
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

    DBG_PROFSTOP(viewhdl);
    return 0;
}

void SwView::_CheckReadonlyState()
{
    SfxDispatcher &rDis = GetDispatcher();
    // To be able to recognize if it is already disabled!
    SfxItemState eStateRO, eStateProtAll;
    const SfxPoolItem *pItem;
    // Query the status from a slot which is only known to us.
    // Otherwise the slot is known from other; like the BasidIde
    eStateRO = rDis.QueryState( FN_INSERT_BOOKMARK, pItem );
    eStateProtAll = rDis.QueryState( FN_EDIT_REGION, pItem );
    sal_Bool bChgd = sal_False;

    if ( !m_pWrtShell->IsCrsrReadonly() )
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
    else if( m_pWrtShell->IsAllProtect() )
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

    if( m_pWrtShell->HasReadonlySel(m_bAnnotationMode) &&
        ( !m_pWrtShell->GetDrawView() ||
            !m_pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount() ))
        nDisableFlags |= SW_DISABLE_ON_PROTECTED_CURSOR;

    if( (SW_DISABLE_ON_PROTECTED_CURSOR & nDisableFlags ) !=
        (SW_DISABLE_ON_PROTECTED_CURSOR & rDis.GetDisableFlags() ) )
    {
        // Additionally move at the Window the InputContext, so that
        // in japanese / chinese versions the external input will be
        // turned on or off. This but only if the correct shell is on
        // the stack.
        switch( m_pViewImpl->GetShellMode() )
        {
        case SHELL_MODE_TEXT:
        case SHELL_MODE_LIST_TEXT:
        case SHELL_MODE_TABLE_TEXT:
        case SHELL_MODE_TABLE_LIST_TEXT:
            {
// Temporary solution!!! Should set the font of the current insertion point
//         at each cursor movement, so outside of this "if". But TH does not
//         evaluates the font at this time and the "purchase" appears to me
//         as too expensive.
//         Moreover, we don't have a font, but only attributes from which the
//         text formatting and the correct font will be build together.

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
    m_aPageStr( SW_RES( STR_PAGE )),
    m_nNewPage(USHRT_MAX),
    m_pNumRuleNodeFromDoc(0),
    m_pEditWin( new SwEditWin( &_pFrame->GetWindow(), *this ) ),
    m_pWrtShell(0),
    m_pShell(0),
    m_pFormShell(0),
    m_pHScrollbar(0),
    m_pVScrollbar(0),
    m_pScrollFill(new ScrollBarBox( &_pFrame->GetWindow(), _pFrame->GetFrame().GetParentFrame() ? 0 : WB_SIZEABLE )),
    m_pVRuler(new SvxRuler(&GetViewFrame()->GetWindow(), m_pEditWin,
                            SVXRULER_SUPPORT_TABS | SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL|
                                SVXRULER_SUPPORT_BORDERS | SVXRULER_SUPPORT_REDUCED_METRIC,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_EXTRAFIELD | WB_BORDER )),
    m_pTogglePageBtn(0),
    m_pPageUpBtn(0),
    m_pPageDownBtn(0),
    m_pNaviBtn(0),
    m_pGlosHdl(0),
    m_pDrawActual(0),
    m_pLastTableFormat(0),
    m_pFormatClipboard(new SwFormatClipboard()),
    m_pPostItMgr(0),
    m_nSelectionType( INT_MAX ),
    m_nPageCnt(0),
    m_nDrawSfxId( USHRT_MAX ),
    m_nFormSfxId( USHRT_MAX ),
    m_nLastPasteDestination( 0xFFFF ),
    m_nLeftBorderDistance( 0 ),
    m_nRightBorderDistance( 0 ),
    m_bWheelScrollInProgress(false),
    m_bInMailMerge(sal_False),
    m_bInDtor(sal_False),
    m_bOldShellWasPagePreView(sal_False),
    m_bIsPreviewDoubleClick(sal_False),
    m_bAnnotationMode(false)
{
    // According to discussion with MBA and further
    // investigations, no old SfxViewShell will be set as parameter <pOldSh>,
    // if function "New Window" is performed to open an additional view beside
    // an already existing one.
    // If the view is switch from one to another, the 'old' view is given by
    // parameter <pOldSh>.

    m_bCenterCrsr = m_bTopCrsr = m_bAlwaysShowSel = m_bTabColFromDoc = m_bTabRowFromDoc =
    m_bSetTabColFromDoc = m_bSetTabRowFromDoc = m_bAttrChgNotified = m_bAttrChgNotifiedWithRegistrations =
    m_bVerbsActive = m_bDrawRotate = m_bInOuterResizePixel = m_bInInnerResizePixel =
    m_bPasteState = m_bPasteSpecialState = m_bMakeSelectionVisible = sal_False;

    m_bShowAtResize = m_bDrawSelMode = bDocSzUpdated = sal_True;

    _CreateScrollbar( sal_True );
    _CreateScrollbar( sal_False );

    m_pViewImpl = new SwView_Impl(this);
    SetName(OUString("View"));
    SetWindow( m_pEditWin );

    m_aTimer.SetTimeout( 120 );

    SwDocShell* pDocSh = PTR_CAST( SwDocShell, _pFrame->GetObjectShell() );
    sal_Bool bOldModifyFlag = pDocSh->IsEnableSetModified();
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_False );
    // HACK: SwDocShell has some cached font info, VCL informs about font updates,
    // but loading of docs with embedded fonts happens after SwDocShell is created
    // but before SwEditWin (which handles the VCL event) is created. So update
    // manually.
    if( pDocSh->GetDoc()->get( IDocumentSettingAccess::EMBED_FONTS ))
        pDocSh->UpdateFontList();
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
            m_sSwViewData = ((SwPagePreView*)pExistingSh)->GetPrevSwViewData();
            m_sNewCrsrPos = ((SwPagePreView*)pExistingSh)->GetNewCrsrPos();
            m_nNewPage = ((SwPagePreView*)pExistingSh)->GetNewPage();
            m_bOldShellWasPagePreView = sal_True;
            m_bIsPreviewDoubleClick = m_sNewCrsrPos.Len() > 0 || m_nNewPage != USHRT_MAX;
        }
        else if( pExistingSh->IsA( TYPE( SwSrcView ) ) )
            bOldShellWasSrcView = sal_True;
    }

    SAL_INFO( "sw.ui", "before create WrtShell" );
    if(PTR_CAST( SwView, pExistingSh))
    {
        m_pWrtShell = new SwWrtShell( *((SwView*)pExistingSh)->m_pWrtShell,
                                    m_pEditWin, *this);
    }
    else if( dynamic_cast<SwWrtShell*>( pDocSh->GetDoc()->GetCurrentViewShell() ) )
    {
        m_pWrtShell = new SwWrtShell( *(SwWrtShell*)pDocSh->GetDoc()->GetCurrentViewShell(),
                                    m_pEditWin, *this);
    }
    else
    {
        SwDoc& rDoc = *((SwDocShell*)pDocSh)->GetDoc();

        if( !bOldShellWasSrcView && pWebDShell && !m_bOldShellWasPagePreView )
            aUsrPref.setBrowseMode( sal_True );
        else
            aUsrPref.setBrowseMode( rDoc.get(IDocumentSettingAccess::BROWSE_MODE) );

        //For the BrowseMode we do not assume a factor.
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
        m_pWrtShell = new SwWrtShell( rDoc, m_pEditWin, *this, &aUsrPref );
        // creating an SwView from a SwPagePreView needs to
        // add the ViewShell to the ring of the other ViewShell(s)
        if(m_bOldShellWasPagePreView)
        {
            ViewShell& rPreviewViewShell = *((SwPagePreView*)pExistingSh)->GetViewShell();
            m_pWrtShell->MoveTo(&rPreviewViewShell);
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
                 m_pWrtShell->HasDrawView() )
            {
                SdrView* pDrawView = m_pWrtShell->GetDrawView();
                pDrawView->SetDesignMode( ((SwPagePreView*)pExistingSh)->FormDesignModeToReset() );
            }
        }
    }
    SAL_INFO( "sw.ui", "after create WrtShell" );
    m_pHRuler = new SwCommentRuler(m_pWrtShell, &GetViewFrame()->GetWindow(), m_pEditWin,
                SVXRULER_SUPPORT_TABS |
                SVXRULER_SUPPORT_PARAGRAPH_MARGINS |
                SVXRULER_SUPPORT_BORDERS |
                SVXRULER_SUPPORT_NEGATIVE_MARGINS|
                SVXRULER_SUPPORT_REDUCED_METRIC,
                GetViewFrame()->GetBindings(),
                WB_STDRULER | WB_EXTRAFIELD | WB_BORDER);

    // assure that modified state of document
    // isn't reset, if document is already modified.
    const bool bIsDocModified = m_pWrtShell->GetDoc()->IsModified();

    // Thus among other things, the HRuler is not displayed in the read-only case.
    aUsrPref.SetReadonly( m_pWrtShell->GetViewOptions()->IsReadonly() );

    // no margin for OLE!
    Size aBrwsBorder;
    if( SFX_CREATE_MODE_EMBEDDED != pDocSh->GetCreateMode() )
        aBrwsBorder = GetMargin();

    m_pWrtShell->SetBrowseBorder( aBrwsBorder );

    // In CTOR no shell changes may take place, which must be temporarily stored
    // with the timer. Otherwise, the SFX removes them from the stack!
    bool bOld = bNoInterrupt;
    bNoInterrupt = true;

    m_pHRuler->SetActive( sal_True );
    m_pVRuler->SetActive( sal_True );

    SfxViewFrame* pViewFrame = GetViewFrame();
    if( pViewFrame->GetFrame().GetParentFrame())
    {
        aUsrPref.SetViewHRuler(sal_False);
        aUsrPref.SetViewVRuler(sal_False);
    }

    StartListening( *pViewFrame, sal_True );
    StartListening( *pDocSh, sal_True );

    // Set Zoom-factor from HRuler
    Fraction aZoomFract( aUsrPref.GetZoom(), 100 );
    m_pHRuler->SetZoom( aZoomFract );
    m_pVRuler->SetZoom( aZoomFract );
    m_pHRuler->SetDoubleClickHdl(LINK( this, SwView, ExecRulerClick ));
    FieldUnit eMetric = pUsrPref->GetHScrollMetric();
    m_pHRuler->SetUnit( eMetric );

    eMetric = pUsrPref->GetVScrollMetric();
    m_pVRuler->SetUnit( eMetric );

        m_pHRuler->SetCharWidth( 371 );  // default character width
        m_pVRuler->SetLineHeight( 551 );  // default line height

    // Set DocShell
    pDocSh->SetView( this );
    SW_MOD()->SetView( this );

    m_pPostItMgr = new SwPostItMgr(this);

    // Check and process the DocSize. Via the handler, the shell could not
    // be found, because the shell is not known in the SFX management
    // within the CTOR phase.
    DocSzChgd( m_pWrtShell->GetDocSize() );

        // Set AttrChangedNotify link
    m_pWrtShell->SetChgLnk(LINK(this, SwView, AttrChangedNotify));

    if( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
        //TODO/LATER: why a cast here?
        //!((SvEmbeddedObject *)pDocSh)->GetVisArea().IsEmpty() )
        //SetVisArea( ((SvEmbeddedObject *)pDocSh)->GetVisArea(),sal_False);
        !pDocSh->GetVisArea(ASPECT_CONTENT).IsEmpty() )
        SetVisArea( pDocSh->GetVisArea(ASPECT_CONTENT),sal_False);

    SAL_WARN_IF(
        officecfg::Office::Common::Undo::Steps::get() <= 0,
        "sw", "/org.openoffice.Office.Common/Undo/Steps <= 0");
    m_pWrtShell->DoUndo( true );

    const sal_Bool bBrowse = m_pWrtShell->GetViewOptions()->getBrowseMode();
    // Disable "multiple window"
    SetNewWindowAllowed(!bBrowse);
    // End of disabled multiple window

    m_bVScrollbarEnabled = aUsrPref.IsViewVScrollBar();
    m_bHScrollbarEnabled = aUsrPref.IsViewHScrollBar();
    m_pHScrollbar->SetAuto(bBrowse);
    if( aUsrPref.IsViewHRuler() )
        CreateTab();
    if( aUsrPref.IsViewVRuler() )
        CreateVRuler();

    m_pWrtShell->SetUIOptions( aUsrPref );
    m_pWrtShell->SetReadOnlyAvailable( aUsrPref.IsCursorInProtectedArea() );
    m_pWrtShell->ApplyAccessiblityOptions(SW_MOD()->GetAccessibilityOptions());

    if( m_pWrtShell->GetDoc()->IsUpdateExpFld() )
    {
        if (m_pWrtShell->GetDoc()->containsUpdatableFields())
        {
            SET_CURR_SHELL( m_pWrtShell );
            m_pWrtShell->StartAction();
            m_pWrtShell->CalcLayout();
            m_pWrtShell->GetDoc()->UpdateFlds(NULL, false);
            m_pWrtShell->EndAction();
        }
        m_pWrtShell->GetDoc()->SetUpdateExpFldStat( sal_False );
    }

    // Update all tables if necessary:
    if( m_pWrtShell->GetDoc()->IsUpdateTOX() )
    {
        SfxRequest aSfxRequest( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
        Execute( aSfxRequest );
        m_pWrtShell->GetDoc()->SetUpdateTOX( sal_False );     // reset again
        m_pWrtShell->SttEndDoc(sal_True);
    }

    // No ResetModified, if there is already a view to this doc.
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(pDocSh);
    // Currently(360) the view is registered firstly after the CTOR,
    // the following expression is also working if this changes.
    // If the modification cannot be canceled by undo, then do NOT set
    // the modify back.
    // no reset of modified state, if document
    // was already modified.
    if (!m_pWrtShell->GetDoc()->GetIDocumentUndoRedo().IsUndoNoResetModified() &&
         ( !pFirst || pFirst == pVFrame ) &&
         !bIsDocModified )
    {
        m_pWrtShell->ResetModified();
    }

    bNoInterrupt = bOld;

    // If a new GlobalDoc will be created, the navigator will also be generated.
    if( pDocSh->IsA(SwGlobalDocShell::StaticType()) &&
        !pVFrame->GetChildWindow( SID_NAVIGATOR ))
    {
        SfxBoolItem aNavi(SID_NAVIGATOR, sal_True);
        GetDispatcher().Execute(SID_NAVIGATOR, SFX_CALLMODE_ASYNCHRON, &aNavi, 0L);
    }

    uno::Reference< frame::XFrame >  xFrame = pVFrame->GetFrame().GetFrameInterface();

    uno::Reference< frame::XFrame >  xBeamerFrame = xFrame->findFrame(
            OUString("_beamer"), frame::FrameSearchFlag::CHILDREN);
    if(xBeamerFrame.is())
    {
        SwDBData aData = m_pWrtShell->GetDBData();
        SW_MOD()->ShowDBObj( *this, aData );
    }

    // has anybody calls the attrchanged handler in the constructor?
    if( m_bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        if( m_aTimer.IsActive() )
            m_aTimer.Stop();
    }

    m_aTimer.SetTimeoutHdl(LINK(this, SwView, TimeoutHdl));
    m_bAttrChgNotified = m_bAttrChgNotifiedWithRegistrations = sal_False;
    if(bOldModifyFlag)
        pDocSh->EnableSetModified( sal_True );
    InvalidateBorder();

    if( !m_pHScrollbar->IsVisible( sal_True ) )
        ShowHScrollbar( sal_False );
    if( !m_pVScrollbar->IsVisible( sal_True ) )
        ShowVScrollbar( sal_False );

    GetViewFrame()->GetWindow().AddChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
}

SwView::~SwView()
{
    GetViewFrame()->GetWindow().RemoveChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
    delete m_pPostItMgr;
    m_pPostItMgr = 0;

    m_bInDtor = sal_True;
    m_pEditWin->Hide(); // damit kein Paint Aerger machen kann!
    // An der SwDocShell den Pointer auf die View ruecksetzen

    SwDocShell* pDocSh = GetDocShell();
    if( pDocSh && pDocSh->GetView() == this )
        pDocSh->SetView( 0 );
    if ( SW_MOD()->GetView() == this )
        SW_MOD()->SetView( 0 );

    if( m_aTimer.IsActive() && m_bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    // the last view must end the text edit
    SdrView *pSdrView = m_pWrtShell ? m_pWrtShell->GetDrawView() : 0;
    if( pSdrView && pSdrView->IsTextEdit() )
        pSdrView->SdrEndTextEdit( sal_True );

    SetWindow( 0 );

    m_pViewImpl->Invalidate();
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    delete m_pScrollFill;
    delete m_pWrtShell;
    m_pWrtShell = 0;      // Set to 0, so that it is not accessable by the following dtors cannot.
    m_pShell = 0;
    delete m_pHScrollbar;
    delete m_pVScrollbar;
    delete m_pHRuler;
    delete m_pVRuler;
    delete m_pTogglePageBtn;
    delete m_pPageUpBtn;
    delete m_pNaviBtn;
    delete m_pPageDownBtn;
    delete m_pGlosHdl;
    delete m_pViewImpl;
    delete m_pEditWin;
    delete m_pFormatClipboard;
}

SwDocShell* SwView::GetDocShell()
{
    SfxObjectShell* pDocShell = GetViewFrame()->GetObjectShell();
    return PTR_CAST(SwDocShell, pDocShell);
}

// Remember CursorPos

void SwView::WriteUserData( String &rUserData, sal_Bool bBrowse )
{
    // The browse flag will be passed from Sfx when documents are browsed
    // (not to be confused with the BrowseMode).
    // Then that stored data are not persistent!

    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();

    rUserData = OUString::number( rRect.Left() );
    rUserData += ';';
    rUserData += OUString::number( rRect.Top() );
    rUserData += ';';
    rUserData += OUString::number( m_pWrtShell->GetViewOptions()->GetZoom() );
    rUserData += ';';
    rUserData += OUString::number( rVis.Left() );
    rUserData += ';';
    rUserData += OUString::number( rVis.Top() );
    rUserData += ';';
    rUserData += OUString::number( bBrowse ? SAL_MIN_INT32 : rVis.Right());
    rUserData += ';';
    rUserData += OUString::number( bBrowse ? SAL_MIN_INT32 : rVis.Bottom());
    rUserData += ';';
    rUserData += OUString::number(
            (sal_uInt16)m_pWrtShell->GetViewOptions()->GetZoomType());//eZoom;
    rUserData += ';';
    rUserData += FRMTYPE_NONE == m_pWrtShell->GetSelFrmType() ? '0' : '1';
}

// Set CursorPos

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
        // For document without layout only in the onlinelayout or
        // while forward/backward
         (!m_pWrtShell->IsNewLayout() || m_pWrtShell->GetViewOptions()->getBrowseMode() || bBrowse) )
    {
        bool bIsOwnDocument = lcl_IsOwnDocument( *this );

        SET_CURR_SHELL(m_pWrtShell);

        sal_Int32 nPos = 0;

        // No it is *no* good idea to call GetToken within Point-Konstr. immediately,
        // because which parameter is evaluated first?
        long nX = rUserData.GetToken( 0, ';', nPos ).ToInt32(),
             nY = rUserData.GetToken( 0, ';', nPos ).ToInt32();
        Point aCrsrPos( nX, nY );

        sal_uInt16 nZoomFactor =
            static_cast< sal_uInt16 >( rUserData.GetToken(0, ';', nPos ).ToInt32() );

        long nLeft  = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nTop   = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nRight = rUserData.GetToken(0, ';', nPos ).ToInt32(),
             nBottom= rUserData.GetToken(0, ';', nPos ).ToInt32();

        const long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
        if ( nBottom <= (m_pWrtShell->GetDocSize().Height()+nAdd) )
        {
            m_pWrtShell->EnableSmooth( sal_False );

            const Rectangle aVis( nLeft, nTop, nRight, nBottom );

            sal_uInt16 nOff = 0;
            SvxZoomType eZoom;
            if( !m_pWrtShell->GetViewOptions()->getBrowseMode() )
                eZoom = (SvxZoomType) (sal_uInt16)rUserData.GetToken(nOff, ';', nPos ).ToInt32();
            else
            {
                eZoom = SVX_ZOOM_PERCENT;
                ++nOff;
            }

            sal_Bool bSelectObj = (0 != rUserData.GetToken( nOff, ';', nPos ).ToInt32())
                                && m_pWrtShell->IsObjSelectable( aCrsrPos );

            // restore editing position
            m_pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
            // set flag value to avoid macro execution.
            bool bSavedFlagValue = m_pWrtShell->IsMacroExecAllowed();
            m_pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(m_bOldShellWasPagePreView || bIsOwnDocument)
            {
                m_pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                if( bSelectObj )
                {
                    m_pWrtShell->SelectObj( aCrsrPos );
                    m_pWrtShell->EnterSelFrmMode( &aCrsrPos );
                }
            }

            // reset flag value
            m_pWrtShell->SetMacroExecAllowed( bSavedFlagValue );

            // set visible area before applying
            // information from print preview. Otherwise, the applied information
            // is lost.
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(m_bOldShellWasPagePreView || bIsOwnDocument )
            {
                if ( bBrowse )
                    SetVisArea( aVis.TopLeft() );
                else
                    SetVisArea( aVis );
            }

            //apply information from print preview - if available
            if( m_sNewCrsrPos.Len() )
            {
                long nXTmp = m_sNewCrsrPos.GetToken( 0, ';' ).ToInt32(),
                      nYTmp = m_sNewCrsrPos.GetToken( 1, ';' ).ToInt32();
                Point aCrsrPos2( nXTmp, nYTmp );
                bSelectObj = m_pWrtShell->IsObjSelectable( aCrsrPos2 );

                m_pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos2, sal_False );
                if( bSelectObj )
                {
                    m_pWrtShell->SelectObj( aCrsrPos2 );
                    m_pWrtShell->EnterSelFrmMode( &aCrsrPos2 );
                }
                m_pWrtShell->MakeSelVisible();
                m_sNewCrsrPos.Erase();
            }
            else if(USHRT_MAX != m_nNewPage)
            {
                m_pWrtShell->GotoPage(m_nNewPage, sal_True);
                m_nNewPage = USHRT_MAX;
            }

            SelectShell();

            m_pWrtShell->StartAction();
            const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
            if( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom )
                SetZoom( eZoom, nZoomFactor);

            m_pWrtShell->LockView( sal_True );
            m_pWrtShell->EndAction();
            m_pWrtShell->LockView( sal_False );
            m_pWrtShell->EnableSmooth( sal_True );
        }
    }
}

void SwView::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    if(GetDocShell()->IsPreview()||m_bIsPreviewDoubleClick)
        return;
    bool bIsOwnDocument = lcl_IsOwnDocument( *this );
    sal_Int32 nLength = rSequence.getLength();
    if (nLength && (!m_pWrtShell->IsNewLayout() || m_pWrtShell->GetViewOptions()->getBrowseMode() || bBrowse) )
    {
        SET_CURR_SHELL(m_pWrtShell);
        const beans::PropertyValue *pValue = rSequence.getConstArray();
        const SwRect& rRect = m_pWrtShell->GetCharRect();
        const Rectangle &rVis = GetVisArea();
        const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();

        long nX = rRect.Left(), nY = rRect.Top(), nLeft = rVis.Left(), nTop = rVis.Top();
        long nRight = bBrowse ? LONG_MIN : rVis.Right(), nBottom = bBrowse ? LONG_MIN : rVis.Bottom();
        sal_Int16 nZoomType = static_cast< sal_Int16 >(pVOpt->GetZoomType());
        sal_Int16 nZoomFactor = static_cast < sal_Int16 > (pVOpt->GetZoom());
        bool bViewLayoutBookMode = pVOpt->IsViewLayoutBookMode();
        sal_Int16 nViewLayoutColumns = pVOpt->GetViewLayoutColumns();

        sal_Bool bSelectedFrame = ( m_pWrtShell->GetSelFrmType() != FRMTYPE_NONE ),
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
            const long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
            if (nBottom <= (m_pWrtShell->GetDocSize().Height()+nAdd) )
            {
                m_pWrtShell->EnableSmooth( sal_False );
                const Rectangle aVis( nLeft, nTop, nRight, nBottom );

                SvxZoomType eZoom;
                if ( !m_pWrtShell->GetViewOptions()->getBrowseMode() )
                    eZoom = static_cast < SvxZoomType > ( nZoomType );
                else
                {
                    eZoom = SVX_ZOOM_PERCENT;
                }
                if (bGotIsSelectedFrame)
                {
                    sal_Bool bSelectObj = (sal_False != bSelectedFrame )
                                        && m_pWrtShell->IsObjSelectable( aCrsrPos );

                    // set flag value to avoid macro execution.
                    bool bSavedFlagValue = m_pWrtShell->IsMacroExecAllowed();
                    m_pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                    m_pViewImpl->SetRestorePosition(aCrsrPos, bSelectObj);
                    if(m_bOldShellWasPagePreView|| bIsOwnDocument)
                    {
                        m_pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );

                        // Update the shell to toggle Header/Footer edit if needed
                        sal_Bool bInHeader = sal_True;
                        if ( m_pWrtShell->IsInHeaderFooter( &bInHeader ) )
                        {
                            if ( !bInHeader )
                            {
                                m_pWrtShell->SetShowHeaderFooterSeparator( Footer, true );
                                m_pWrtShell->SetShowHeaderFooterSeparator( Header, false );
                            }
                            else
                            {
                                m_pWrtShell->SetShowHeaderFooterSeparator( Header, true );
                                m_pWrtShell->SetShowHeaderFooterSeparator( Footer, false );
                            }

                            // Force repaint
                            m_pWrtShell->GetWin()->Invalidate();
                        }
                        if ( m_pWrtShell->IsInHeaderFooter() != m_pWrtShell->IsHeaderFooterEdit() )
                            m_pWrtShell->ToggleHeaderFooterEdit();

                        if( bSelectObj )
                        {
                            m_pWrtShell->SelectObj( aCrsrPos );
                            m_pWrtShell->EnterSelFrmMode( &aCrsrPos );
                        }
                    }

                    // reset flag value
                    m_pWrtShell->SetMacroExecAllowed( bSavedFlagValue );
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
                    m_pWrtShell->StartAction();

                if ( bSetViewLayoutSettings )
                    SetViewLayout( nViewLayoutColumns, bViewLayoutBookMode, sal_True );

                if ( bZoomNeedsViewLayout )
                    m_pWrtShell->StartAction();

                if ( bSetViewSettings )
                    SetZoom( eZoom, nZoomFactor, sal_True );

// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                if(m_bOldShellWasPagePreView||bIsOwnDocument)
                {
                    if ( bBrowse && bGotVisibleLeft && bGotVisibleTop )
                    {
                        Point aTopLeft(aVis.TopLeft());
                        // make sure the document is still centered
                        const SwTwips lBorder = IsDocumentBorder() ? DOCUMENTBORDER : 2 * DOCUMENTBORDER;
                        SwTwips nEditWidth = GetEditWin().GetOutputSize().Width();
                        if(nEditWidth > (m_aDocSz.Width() + lBorder ))
                            aTopLeft.X() = ( m_aDocSz.Width() + lBorder - nEditWidth  ) / 2;
                        else
                        {
                            //check if the values are possible
                            long nXMax = m_pHScrollbar->GetRangeMax() - m_pHScrollbar->GetVisibleSize();
                            if( aTopLeft.X() > nXMax )
                                aTopLeft.X() = nXMax < 0 ? 0 : nXMax;
                        }
                        SetVisArea( aTopLeft );
                    }
                    else if (bGotVisibleLeft && bGotVisibleTop && bGotVisibleRight && bGotVisibleBottom )
                        SetVisArea( aVis );
                }

                m_pWrtShell->LockView( sal_True );
                m_pWrtShell->EndAction();
                m_pWrtShell->LockView( sal_False );
                m_pWrtShell->EnableSmooth( sal_True );
            }
        }
    }
}

#define NUM_VIEW_SETTINGS 12
void SwView::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();
    Any aAny;

    rSequence.realloc ( NUM_VIEW_SETTINGS );
    sal_Int16 nIndex = 0;
    beans::PropertyValue *pValue = rSequence.getArray();

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    pValue->Name = OUString( "ViewId" );
    OUStringBuffer sBuffer ( OUString( "view" ) );
    ::sax::Converter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
    pValue->Value <<= sBuffer.makeStringAndClear();
    pValue++;nIndex++;

    pValue->Name = OUString( "ViewLeft" );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString( "ViewTop" );
    pValue->Value <<= TWIP_TO_MM100 ( rRect.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString( "VisibleLeft" );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Left() );
    pValue++;nIndex++;

    pValue->Name = OUString( "VisibleTop" );
    pValue->Value <<= TWIP_TO_MM100 ( rVis.Top() );
    pValue++;nIndex++;

    pValue->Name = OUString( "VisibleRight" );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Right() );
    pValue++;nIndex++;

    pValue->Name = OUString( "VisibleBottom" );
    pValue->Value <<= TWIP_TO_MM100 ( bBrowse ? LONG_MIN : rVis.Bottom() );
    pValue++;nIndex++;

    pValue->Name = OUString( "ZoomType" );
    const sal_Int16 nZoomType = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetZoomType());
    pValue->Value <<= nZoomType;
    pValue++;nIndex++;

    pValue->Name = OUString( "ViewLayoutColumns" );
    const sal_Int16 nViewLayoutColumns = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetViewLayoutColumns());
    pValue->Value <<= nViewLayoutColumns;
    pValue++;nIndex++;

    pValue->Name = OUString( "ViewLayoutBookMode" );
    const sal_Bool bIsViewLayoutBookMode = m_pWrtShell->GetViewOptions()->IsViewLayoutBookMode();
    pValue->Value.setValue( &bIsViewLayoutBookMode, ::getBooleanCppuType() );
    pValue++;nIndex++;

    pValue->Name = OUString( "ZoomFactor" );
    pValue->Value <<= static_cast < sal_Int16 > (m_pWrtShell->GetViewOptions()->GetZoom());
    pValue++;nIndex++;

    pValue->Name = OUString( "IsSelectedFrame" );
    const sal_Bool bIsSelected = FRMTYPE_NONE == m_pWrtShell->GetSelFrmType() ? sal_False : sal_True;
    pValue->Value.setValue ( &bIsSelected, ::getBooleanCppuType() );
    nIndex++;

    if ( nIndex < NUM_VIEW_SETTINGS )
        rSequence.realloc ( nIndex );
}
#undef NUM_VIEW_SETTINGS

void SwView::ShowCursor( bool bOn )
{
    //don't scroll the cursor into the visible area
    sal_Bool bUnlockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( sal_True );    //lock visible section

    if( !bOn )
        m_pWrtShell->HideCrsr();
    else if( !m_pWrtShell->IsFrmSelected() && !m_pWrtShell->IsObjSelected() )
        m_pWrtShell->ShowCrsr();

    if( bUnlockView )
        m_pWrtShell->LockView( sal_False );
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

//   only return sal_True for a text selection

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
    if(!m_pGlosHdl)
        m_pGlosHdl = new SwGlossaryHdl(GetViewFrame(), m_pWrtShell);
    return m_pGlosHdl;
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
                    // Modal mode change-over?
                    sal_Bool bModal = GetDocShell()->IsInModalMode();
                    m_pHRuler->SetActive( !bModal );
                    m_pVRuler->SetActive( !bModal );
                }

                /* no break here */

            case SFX_HINT_TITLECHANGED:
                if ( GetDocShell()->IsReadOnly() != GetWrtShell().GetViewOptions()->IsReadonly() )
                {
                    SwWrtShell &rSh = GetWrtShell();
                    rSh.SetReadonlyOption( GetDocShell()->IsReadOnly() );

                    if ( rSh.GetViewOptions()->IsViewVRuler() )
                        CreateVRuler();
                    else
                        KillVRuler();
                    if ( rSh.GetViewOptions()->IsViewHRuler() )
                        CreateTab();
                    else
                        KillTab();
                    bool bReadonly = GetDocShell()->IsReadOnly();
                    // if document is to be opened in alive-mode then this has to be
                    // regarded while switching from readonly-mode to edit-mode
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
            AttrChangedNotify(m_pWrtShell);
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
                    m_pWrtShell->Insert( aEmptyStr, aEmptyStr, aGrf );
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
    if(m_aTimer.IsActive())
    {
        m_aTimer.Stop();
        if ( m_bAttrChgNotifiedWithRegistrations )
        {
            GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
            m_bAttrChgNotifiedWithRegistrations = sal_False;
        }
        SelectShell();
        m_bAttrChgNotified = sal_False;
    }
}

sal_uInt16  SwView::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    SfxViewFrame* pVFrame = GetViewFrame();
    pVFrame->SetChildWindow( SwInputChild::GetChildWindowId(), sal_False );
    if( pVFrame->GetDispatcher()->IsLocked() )
        pVFrame->GetDispatcher()->Lock(sal_False);

    sal_uInt16 nRet;
    if ( m_pFormShell &&
         sal_True != (nRet = m_pFormShell->PrepareClose( bUI, bForBrowsing )) )

    {
        return nRet;
    }
    return SfxViewShell::PrepareClose( bUI, bForBrowsing );
}

// status methods for clipboard.
// Status changes now notified from the clipboard.
sal_Bool SwView::IsPasteAllowed()
{
    sal_uInt16 nPasteDestination = SwTransferable::GetSotDestination( *m_pWrtShell );
    if( m_nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            m_bPasteState = SwTransferable::IsPaste( *m_pWrtShell, aDataHelper );
            m_bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *m_pWrtShell, aDataHelper );
        }
        else
            m_bPasteState = m_bPasteSpecialState = sal_False;

        if( 0xFFFF == m_nLastPasteDestination )  // the init value
            m_pViewImpl->AddClipboardListener();
        m_nLastPasteDestination = nPasteDestination;
    }
    return m_bPasteState;
}

sal_Bool SwView::IsPasteSpecialAllowed()
{
    if ( m_pFormShell && m_pFormShell->IsActiveControl() )
        return sal_False;

    sal_uInt16 nPasteDestination = SwTransferable::GetSotDestination( *m_pWrtShell );
    if( m_nLastPasteDestination != nPasteDestination )
    {
        TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
        if( aDataHelper.GetXTransferable().is() )
        {
            m_bPasteState = SwTransferable::IsPaste( *m_pWrtShell, aDataHelper );
            m_bPasteSpecialState = SwTransferable::IsPasteSpecial(
                                                    *m_pWrtShell, aDataHelper );
        }
        else
            m_bPasteState = m_bPasteSpecialState = sal_False;

        if( 0xFFFF == m_nLastPasteDestination )  // the init value
            m_pViewImpl->AddClipboardListener();
    }
    return m_bPasteSpecialState;
}

void SwView::NotifyDBChanged()
{
    GetViewImpl()->GetUNOObject_Impl()->NotifyDBChanged();
}

// Printing

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
