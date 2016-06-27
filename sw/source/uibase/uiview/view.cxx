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
#include <o3tl/any.hxx>
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
#include <svx/srchdlg.hxx>
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
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <DocumentFieldsManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <comphelper/propertyvalue.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;

extern bool g_bNoInterrupt;       // in swmodule.cxx

#define SWVIEWFLAGS ( SfxViewShellFlags::CAN_PRINT | SfxViewShellFlags::HAS_PRINTOPTIONS)

// Statics. OMG.

bool bDocSzUpdated = true;

SvxSearchItem*  SwView::m_pSrchItem   = nullptr;

bool            SwView::m_bExtra      = false;
bool            SwView::m_bFound      = false;
bool            SwView::m_bJustOpened = false;

SearchAttrItemList*     SwView::m_pSrchList   = nullptr;
SearchAttrItemList*     SwView::m_pReplList   = nullptr;

SfxDispatcher &SwView::GetDispatcher()
{
    return *GetViewFrame()->GetDispatcher();
}

void SwView::ImpSetVerb( int nSelType )
{
    bool bResetVerbs = m_bVerbsActive;
    if ( !GetViewFrame()->GetFrame().IsInPlace() &&
         (nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) & nSelType )
    {
        if ( m_pWrtShell->IsSelObjProtected(FlyProtectFlags::Content) == FlyProtectFlags::NONE )
        {
            if ( nSelType & nsSelectionType::SEL_OLE )
            {
                SetVerbs( GetWrtShell().GetOLEObject()->getSupportedVerbs() );
                m_bVerbsActive = true;
                bResetVerbs = false;
            }
        }
    }
    if ( bResetVerbs )
    {
        SetVerbs( Sequence< embed::VerbDescriptor >() );
        m_bVerbsActive = false;
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
    FmFormShell* pAsFormShell = dynamic_cast<FmFormShell*>( pTopShell  );
    if ( pAsFormShell )
    {
        pAsFormShell->ForgetActiveControl();
        const_cast< SwView* >( this )->AttrChangedNotify( m_pWrtShell );
    }
    else if ( m_pPostItMgr )
    {
        SwAnnotationShell* pAsAnnotationShell = dynamic_cast<SwAnnotationShell*>( pTopShell  );
        if ( pAsAnnotationShell )
        {
            m_pPostItMgr->SetActiveSidebarWin(nullptr);
            const_cast< SwView* >( this )->AttrChangedNotify( m_pWrtShell );
        }
    }
    if( GetWrtShellPtr() )
    {
        SwWrtShell& rWrtShell = GetWrtShell();
        rWrtShell.GetDoc()->getIDocumentLayoutAccess().SetCurrentViewShell( GetWrtShellPtr() );
        rWrtShell.GetDoc()->getIDocumentSettingAccess().set( DocumentSettingId::BROWSE_MODE,
                                 rWrtShell.GetViewOptions()->getBrowseMode() );
    }
}

// called by the FormShell when a form control is focused. This is
// a request to put the form shell on the top of the dispatcher stack

IMPL_LINK_NOARG_TYPED(SwView, FormControlActivated, LinkParamNone*, void)
{
    // if a form control has been activated, and the form shell is not on the top
    // of the dispatcher stack, then we need to activate it
    const SfxDispatcher& rDispatcher = GetDispatcher();
    const SfxShell* pTopShell = rDispatcher.GetShell( 0 );
    const FmFormShell* pAsFormShell = dynamic_cast<const FmFormShell*>( pTopShell  );
    if ( !pAsFormShell )
    {
        // if we're editing text currently, cancel this
        SdrView *pSdrView = m_pWrtShell ? m_pWrtShell->GetDrawView() : nullptr;
        if ( pSdrView && pSdrView->IsTextEdit() )
            pSdrView->SdrEndTextEdit( true );

        AttrChangedNotify( m_pWrtShell );
    }
}

void SwView::SelectShell()
{
    // Attention: Maintain the SelectShell for the WebView additionally

    if(m_bInDtor)
        return;

    // Decision if the UpdateTable has to be called
    bool bUpdateTable = false;
    const SwFrameFormat* pCurTableFormat = m_pWrtShell->GetTableFormat();
    if(pCurTableFormat && pCurTableFormat != m_pLastTableFormat)
    {
        bUpdateTable = true; // can only be executed later
    }
    m_pLastTableFormat = pCurTableFormat;

    //SEL_TBL and SEL_TBL_CELLS can be ORed!
    int nNewSelectionType = (m_pWrtShell->GetSelectionType()
                                & ~nsSelectionType::SEL_TBL_CELLS);

    if ( m_pFormShell && m_pFormShell->IsActiveControl() )
        nNewSelectionType |= nsSelectionType::SEL_FOC_FRM_CTRL;

    if ( nNewSelectionType == m_nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( false );
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

            for ( sal_uInt16 i = 0; true; ++i )
            {
                SfxShell *pSfxShell = rDispatcher.GetShell( i );
                if  (  dynamic_cast< const SwBaseShell *>( pSfxShell ) !=  nullptr
                    || dynamic_cast< const SwDrawTextShell *>( pSfxShell ) !=  nullptr
                    || dynamic_cast< const svx::ExtrusionBar*>( pSfxShell ) !=  nullptr
                    || dynamic_cast< const svx::FontworkBar*>( pSfxShell ) !=  nullptr
                    || dynamic_cast< const SwAnnotationShell *>( pSfxShell ) !=  nullptr
                    )
                {
                    rDispatcher.Pop( *pSfxShell, SfxDispatcherPopFlags::POP_DELETE );
                }
                else if ( dynamic_cast< const FmFormShell *>( pSfxShell ) !=  nullptr )
                {
                    rDispatcher.Pop( *pSfxShell );
                }
                else
                    break;
            }
        }

        bool bInitFormShell = false;
        if (!m_pFormShell)
        {
            bInitFormShell = true;
            m_pFormShell = new FmFormShell( this );
            m_pFormShell->SetControlActivationHandler( LINK( this, SwView, FormControlActivated ) );
            StartListening(*m_pFormShell);
        }

        bool bSetExtInpCntxt = false;
        m_nSelectionType = nNewSelectionType;
        ShellModes eShellMode;

        if ( !( m_nSelectionType & nsSelectionType::SEL_FOC_FRM_CTRL ) )
            rDispatcher.Push( *m_pFormShell );

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
#if HAVE_FEATURE_AVMEDIA
            else if( m_nSelectionType & nsSelectionType::SEL_MEDIA )
            {
                eShellMode = SHELL_MODE_MEDIA;
                m_pShell = new SwMediaShell( *this );
                rDispatcher.Push( *m_pShell );
            }
#endif
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
            bSetExtInpCntxt = true;
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
            bSetExtInpCntxt = true;
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
                bSetExtInpCntxt = false;

            InputContext aCntxt( GetEditWin().GetInputContext() );
            aCntxt.SetOptions( bSetExtInpCntxt
                                ? (aCntxt.GetOptions() |
                                        ( InputContextFlags::Text |
                                            InputContextFlags::ExtText ))
                                : (aCntxt.GetOptions() & ~
                                        InputContextFlags( InputContextFlags::Text |
                                            InputContextFlags::ExtText )) );
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
            m_pFormShell->SetView(dynamic_cast<FmFormView*>( pDView) );

    }
    // Opportune time for the communication with OLE objects?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( false );

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
        return *static_cast<sal_uInt16 const *>(pFirst) - *static_cast<sal_uInt16 const *>(pSecond);
    }
}

IMPL_LINK_NOARG_TYPED(SwView, AttrChangedNotify, SwCursorShell*, void)
{
     if ( GetEditWin().IsChainMode() )
        GetEditWin().SetChainMode( false );

    //Opt: Not if PaintLocked. During unlock a notify will be once more triggered.
    if( !m_pWrtShell->IsPaintLocked() && !g_bNoInterrupt &&
        GetDocShell()->IsReadOnly() )
        CheckReadonlyState();

    if( !m_pWrtShell->IsPaintLocked() && !g_bNoInterrupt )
        CheckReadonlySelection();

    if( !m_bAttrChgNotified )
    {
        if ( m_pWrtShell->BasicActionPend() || g_bNoInterrupt ||
             GetDispatcher().IsLocked() ||               //do not confuse the SFX
             GetViewFrame()->GetBindings().IsInUpdate() )//do not confuse the SFX
        {
            m_bAttrChgNotified = true;
            m_aTimer.Start();

            const SfxPoolItem *pItem;
            if ( SfxItemState::SET != GetObjectShell()->GetMedium()->GetItemSet()->
                                    GetItemState( SID_HIDDEN, false, &pItem ) ||
                 !static_cast<const SfxBoolItem*>(pItem)->GetValue() )
            {
                GetViewFrame()->GetBindings().ENTERREGISTRATIONS();
                m_bAttrChgNotifiedWithRegistrations = true;
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
}

IMPL_LINK_NOARG_TYPED(SwView, TimeoutHdl, Timer *, void)
{
    if( m_pWrtShell->BasicActionPend() || g_bNoInterrupt )
    {
        m_aTimer.Start();
        return;
    }

    if ( m_bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        m_bAttrChgNotifiedWithRegistrations = false;
    }

    CheckReadonlyState();
    CheckReadonlySelection();

    bool bOldUndo = m_pWrtShell->DoesUndo();
    m_pWrtShell->DoUndo( false );
    SelectShell();
    m_pWrtShell->DoUndo( bOldUndo );
    m_bAttrChgNotified = false;
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();
}

void SwView::CheckReadonlyState()
{
    SfxDispatcher &rDis = GetDispatcher();
    // To be able to recognize if it is already disabled!
    SfxItemState eStateRO, eStateProtAll;
    const SfxPoolItem *pItem;
    // Query the status from a slot which is only known to us.
    // Otherwise the slot is known from other; like the BasicIde
    eStateRO = rDis.QueryState( FN_INSERT_BOOKMARK, pItem );
    eStateProtAll = rDis.QueryState( FN_EDIT_REGION, pItem );
    bool bChgd = false;

    if ( !m_pWrtShell->IsCursorReadonly() )
    {
        static sal_uInt16 aROIds[] =
        {
            SID_DELETE,                 FN_BACKSPACE,               FN_SHIFT_BACKSPACE,
            SID_UNDO,
            SID_REDO,                   SID_REPEAT,                 SID_PASTE,
            SID_PASTE_UNFORMATTED,
            SID_PASTE_SPECIAL,            SID_SBA_BRW_INSERT,
            SID_BACKGROUND_COLOR,       FN_INSERT_BOOKMARK,
            SID_CHARMAP,                SID_EMOJI_CONTROL,          FN_INSERT_SOFT_HYPHEN,
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
        static bool bFirst = true;
        if ( bFirst )
        {
            qsort( static_cast<void*>(aROIds), sizeof(aROIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
            bFirst = false;
        }
        if ( SfxItemState::DISABLED == eStateRO )
        {
            rDis.SetSlotFilter( SfxSlotFilterState::ENABLED_READONLY, sizeof(aROIds)/sizeof(sal_uInt16), aROIds );
            bChgd = true;
        }
    }
    else if( m_pWrtShell->IsAllProtect() )
    {
        if ( SfxItemState::DISABLED == eStateProtAll )
        {
            static sal_uInt16 aAllProtIds[] = { SID_SAVEDOC, FN_EDIT_REGION };
            static bool bAllProtFirst = true;
            if ( bAllProtFirst )
            {
                qsort( static_cast<void*>(aAllProtIds), sizeof(aAllProtIds)/sizeof(sal_uInt16), sizeof(sal_uInt16), lcl_CmpIds );
                bAllProtFirst = false;
            }
            rDis.SetSlotFilter( SfxSlotFilterState::ENABLED_READONLY,
                                sizeof(aAllProtIds)/sizeof(sal_uInt16),
                                aAllProtIds );
            bChgd = true;
        }
    }
    else if ( SfxItemState::DISABLED != eStateRO ||
                SfxItemState::DISABLED != eStateProtAll )
    {
        bChgd = true;
        rDis.SetSlotFilter();
    }
    if ( bChgd )
        GetViewFrame()->GetBindings().InvalidateAll(true);
}

void SwView::CheckReadonlySelection()
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
                                            InputContextFlags( InputContextFlags::Text |
                                                InputContextFlags::ExtText ))
                                    : (aCntxt.GetOptions() |
                                            ( InputContextFlags::Text |
                                                InputContextFlags::ExtText )) );
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
        GetViewFrame()->GetBindings().InvalidateAll( true );
    }
}

SwView::SwView( SfxViewFrame *_pFrame, SfxViewShell* pOldSh )
    : SfxViewShell( _pFrame, SWVIEWFLAGS ),
    m_nNewPage(USHRT_MAX),
    m_nOldPageNum(0),
    m_pNumRuleNodeFromDoc(nullptr),
    m_pEditWin( VclPtr<SwEditWin>::Create( &_pFrame->GetWindow(), *this ) ),
    m_pWrtShell(nullptr),
    m_pShell(nullptr),
    m_pFormShell(nullptr),
    m_pHScrollbar(nullptr),
    m_pVScrollbar(nullptr),
    m_pScrollFill(VclPtr<ScrollBarBox>::Create( &_pFrame->GetWindow(), _pFrame->GetFrame().GetParentFrame() ? 0 : WB_SIZEABLE )),
    m_pVRuler(VclPtr<SvxRuler>::Create(&GetViewFrame()->GetWindow(), m_pEditWin,
                            SvxRulerSupportFlags::TABS | SvxRulerSupportFlags::PARAGRAPH_MARGINS_VERTICAL|
                                SvxRulerSupportFlags::BORDERS | SvxRulerSupportFlags::REDUCED_METRIC,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_EXTRAFIELD | WB_BORDER )),
    m_pTogglePageBtn(nullptr),
    m_pGlosHdl(nullptr),
    m_pDrawActual(nullptr),
    m_pLastTableFormat(nullptr),
    m_pFormatClipboard(new SwFormatClipboard()),
    m_pPostItMgr(nullptr),
    m_nSelectionType( INT_MAX ),
    m_nPageCnt(0),
    m_nDrawSfxId( USHRT_MAX ),
    m_nFormSfxId( USHRT_MAX ),
    m_nLastPasteDestination( static_cast<SotExchangeDest>(0xFFFF) ),
    m_nLeftBorderDistance( 0 ),
    m_nRightBorderDistance( 0 ),
    m_eLastSearchCommand( static_cast<SvxSearchCmd>(0xFFFF) ),
    m_bWheelScrollInProgress(false),
    m_bInMailMerge(false),
    m_bInDtor(false),
    m_bOldShellWasPagePreview(false),
    m_bIsPreviewDoubleClick(false),
    m_bAnnotationMode(false),
    m_nLOKPageUpDownOffset(0)
{
    static bool bRequestDoubleBuffering = getenv("VCL_DOUBLEBUFFERING_ENABLE");
    if (bRequestDoubleBuffering)
        m_pEditWin->RequestDoubleBuffering(true);

    // According to discussion with MBA and further
    // investigations, no old SfxViewShell will be set as parameter <pOldSh>,
    // if function "New Window" is performed to open an additional view beside
    // an already existing one.
    // If the view is switch from one to another, the 'old' view is given by
    // parameter <pOldSh>.

    m_bCenterCursor = m_bTopCursor = m_bAlwaysShowSel = m_bTabColFromDoc = m_bTabRowFromDoc =
    m_bSetTabColFromDoc = m_bSetTabRowFromDoc = m_bAttrChgNotified = m_bAttrChgNotifiedWithRegistrations =
    m_bVerbsActive = m_bDrawRotate = m_bInOuterResizePixel = m_bInInnerResizePixel =
    m_bPasteState = m_bPasteSpecialState = m_bMakeSelectionVisible = false;

    m_bShowAtResize = m_bDrawSelMode = bDocSzUpdated = true;

    CreateScrollbar( true );
    CreateScrollbar( false );

    m_pViewImpl = new SwView_Impl(this);
    SetName("View");
    SetWindow( m_pEditWin );

    m_aTimer.SetTimeout( 120 );

    SwDocShell& rDocSh = dynamic_cast<SwDocShell&>(*_pFrame->GetObjectShell());
    bool bOldModifyFlag = rDocSh.IsEnableSetModified();
    if (bOldModifyFlag)
        rDocSh.EnableSetModified( false );
    // HACK: SwDocShell has some cached font info, VCL informs about font updates,
    // but loading of docs with embedded fonts happens after SwDocShell is created
    // but before SwEditWin (which handles the VCL event) is created. So update
    // manually.
    if (rDocSh.GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        rDocSh.UpdateFontList();
    bool bWebDShell = dynamic_cast<const SwWebDocShell*>(&rDocSh) !=  nullptr;

    const SwMasterUsrPref *pUsrPref = SW_MOD()->GetUsrPref(bWebDShell);
    SwViewOption aUsrPref( *pUsrPref);

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;
    SvtLinguConfig().GetOptions( aLinguOpt );
    aUsrPref.SetOnlineSpell( aLinguOpt.bIsSpellAuto );

    bool bOldShellWasSrcView = false;

    // determine if there is an existing view for
    // document
    SfxViewShell* pExistingSh = nullptr;
    if ( pOldSh )
    {
        pExistingSh = pOldSh;
        // determine type of existing view
        if( dynamic_cast<const SwPagePreview *>(pExistingSh) != nullptr )
        {
            m_sSwViewData = static_cast<SwPagePreview*>(pExistingSh)->GetPrevSwViewData();
            m_sNewCursorPos = static_cast<SwPagePreview*>(pExistingSh)->GetNewCursorPos();
            m_nNewPage = static_cast<SwPagePreview*>(pExistingSh)->GetNewPage();
            m_bOldShellWasPagePreview = true;
            m_bIsPreviewDoubleClick = !m_sNewCursorPos.isEmpty() || m_nNewPage != USHRT_MAX;
        }
        else if( dynamic_cast<const SwSrcView *>(pExistingSh) != nullptr )
            bOldShellWasSrcView = true;
    }

    SAL_INFO( "sw.ui", "before create WrtShell" );
    if (SwView *pView = dynamic_cast<SwView*>(pExistingSh))
    {
        m_pWrtShell = new SwWrtShell(*pView->m_pWrtShell, m_pEditWin, *this);
    }
    else if (SwWrtShell *pWrtShell = dynamic_cast<SwWrtShell*>(rDocSh.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell()))
    {
        m_pWrtShell = new SwWrtShell(*pWrtShell, m_pEditWin, *this);
    }
    else
    {
        SwDoc& rDoc = *static_cast<SwDocShell&>(rDocSh).GetDoc();

        if( !bOldShellWasSrcView && bWebDShell && !m_bOldShellWasPagePreview )
            aUsrPref.setBrowseMode( true );
        else
            aUsrPref.setBrowseMode( rDoc.getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE) );

        //For the BrowseMode we do not assume a factor.
        if( aUsrPref.getBrowseMode() && aUsrPref.GetZoomType() != SvxZoomType::PERCENT )
        {
            aUsrPref.SetZoomType( SvxZoomType::PERCENT );
            aUsrPref.SetZoom( 100 );
        }
        if (rDocSh.IsPreview())
        {
            aUsrPref.SetZoomType( SvxZoomType::WHOLEPAGE );
            aUsrPref.SetViewLayoutBookMode( false );
            aUsrPref.SetViewLayoutColumns( 1 );
        }
        m_pWrtShell = new SwWrtShell( rDoc, m_pEditWin, *this, &aUsrPref );
        // creating an SwView from a SwPagePreview needs to
        // add the SwViewShell to the ring of the other SwViewShell(s)
        if(m_bOldShellWasPagePreview)
        {
            SwViewShell& rPreviewViewShell = *static_cast<SwPagePreview*>(pExistingSh)->GetViewShell();
            m_pWrtShell->MoveTo(&rPreviewViewShell);
            // to update the field command et.al. if necessary
            const SwViewOption* pPreviewOpt = rPreviewViewShell.GetViewOptions();
            if( pPreviewOpt->IsFieldName() != aUsrPref.IsFieldName() ||
                    pPreviewOpt->IsShowHiddenField() != aUsrPref.IsShowHiddenField() ||
                    pPreviewOpt->IsShowHiddenPara() != aUsrPref.IsShowHiddenPara() ||
                    pPreviewOpt->IsShowHiddenChar() != aUsrPref.IsShowHiddenChar() )
                rPreviewViewShell.ApplyViewOptions(aUsrPref);
            // reset design mode at draw view for form
            // shell, if needed.
            if ( static_cast<SwPagePreview*>(pExistingSh)->ResetFormDesignMode() &&
                 m_pWrtShell->HasDrawView() )
            {
                SdrView* pDrawView = m_pWrtShell->GetDrawView();
                pDrawView->SetDesignMode( static_cast<SwPagePreview*>(pExistingSh)->FormDesignModeToReset() );
            }
        }
    }
    SAL_INFO( "sw.ui", "after create WrtShell" );
    m_pHRuler = VclPtr<SwCommentRuler>::Create(m_pWrtShell, &GetViewFrame()->GetWindow(), m_pEditWin,
                SvxRulerSupportFlags::TABS |
                SvxRulerSupportFlags::PARAGRAPH_MARGINS |
                SvxRulerSupportFlags::BORDERS |
                SvxRulerSupportFlags::NEGATIVE_MARGINS|
                SvxRulerSupportFlags::REDUCED_METRIC,
                GetViewFrame()->GetBindings(),
                WB_STDRULER | WB_EXTRAFIELD | WB_BORDER);

    // assure that modified state of document
    // isn't reset, if document is already modified.
    const bool bIsDocModified = m_pWrtShell->GetDoc()->getIDocumentState().IsModified();

    // Thus among other things, the HRuler is not displayed in the read-only case.
    aUsrPref.SetReadonly( m_pWrtShell->GetViewOptions()->IsReadonly() );

    // no margin for OLE!
    Size aBrwsBorder;
    if( SfxObjectCreateMode::EMBEDDED != rDocSh.GetCreateMode() )
        aBrwsBorder = GetMargin();

    m_pWrtShell->SetBrowseBorder( aBrwsBorder );

    // In CTOR no shell changes may take place, which must be temporarily stored
    // with the timer. Otherwise, the SFX removes them from the stack!
    bool bOld = g_bNoInterrupt;
    g_bNoInterrupt = true;

    m_pHRuler->SetActive();
    m_pVRuler->SetActive();

    SfxViewFrame* pViewFrame = GetViewFrame();
    if( pViewFrame->GetFrame().GetParentFrame())
    {
        aUsrPref.SetViewHRuler(false);
        aUsrPref.SetViewVRuler(false);
    }

    StartListening(*pViewFrame, true);
    StartListening(rDocSh, true);

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
    rDocSh.SetView(this);
    SW_MOD()->SetView( this );

    m_pPostItMgr = new SwPostItMgr(this);

    // Check and process the DocSize. Via the handler, the shell could not
    // be found, because the shell is not known in the SFX management
    // within the CTOR phase.
    DocSzChgd( m_pWrtShell->GetDocSize() );

        // Set AttrChangedNotify link
    m_pWrtShell->SetChgLnk(LINK(this, SwView, AttrChangedNotify));

    if (rDocSh.GetCreateMode() == SfxObjectCreateMode::EMBEDDED &&
        !rDocSh.GetVisArea(ASPECT_CONTENT).IsEmpty())
        SetVisArea(rDocSh.GetVisArea(ASPECT_CONTENT),false);

    SAL_WARN_IF(
        officecfg::Office::Common::Undo::Steps::get() <= 0,
        "sw", "/org.openoffice.Office.Common/Undo/Steps <= 0");
    m_pWrtShell->DoUndo();

    const bool bBrowse = m_pWrtShell->GetViewOptions()->getBrowseMode();
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

    if( m_pWrtShell->GetDoc()->getIDocumentState().IsUpdateExpField() )
    {
        if (m_pWrtShell->GetDoc()->GetDocumentFieldsManager().containsUpdatableFields())
        {
            SET_CURR_SHELL( m_pWrtShell );
            m_pWrtShell->StartAction();
            m_pWrtShell->CalcLayout();
            m_pWrtShell->GetDoc()->getIDocumentFieldsAccess().UpdateFields(false);
            m_pWrtShell->EndAction();
        }
        m_pWrtShell->GetDoc()->getIDocumentState().SetUpdateExpFieldStat( false );
    }

    // Update all tables if necessary:
    if( m_pWrtShell->GetDoc()->IsUpdateTOX() )
    {
        SfxRequest aSfxRequest( FN_UPDATE_TOX, SfxCallMode::SLOT, GetPool() );
        Execute( aSfxRequest );
        m_pWrtShell->GetDoc()->SetUpdateTOX( false );     // reset again
        m_pWrtShell->SttEndDoc(true);
    }

    // No ResetModified, if there is already a view to this doc.
    SfxViewFrame* pVFrame = GetViewFrame();
    SfxViewFrame* pFirst = SfxViewFrame::GetFirst(&rDocSh);
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

    g_bNoInterrupt = bOld;

    // If a new GlobalDoc will be created, the navigator will also be generated.
    if( dynamic_cast<const SwGlobalDocShell*>(&rDocSh) != nullptr &&
        !pVFrame->GetChildWindow( SID_NAVIGATOR ))
    {
        SfxBoolItem aNavi(SID_NAVIGATOR, true);
        GetDispatcher().ExecuteList(SID_NAVIGATOR, SfxCallMode::ASYNCHRON, { &aNavi });
    }

    uno::Reference< frame::XFrame >  xFrame = pVFrame->GetFrame().GetFrameInterface();

    uno::Reference< frame::XFrame >  xBeamerFrame = xFrame->findFrame(
            "_beamer", frame::FrameSearchFlag::CHILDREN);
    if(xBeamerFrame.is())
    {
        SwDBData aData = m_pWrtShell->GetDBData();
        SwModule::ShowDBObj( *this, aData );
    }

    // has anybody calls the attrchanged handler in the constructor?
    if( m_bAttrChgNotifiedWithRegistrations )
    {
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();
        if( m_aTimer.IsActive() )
            m_aTimer.Stop();
    }

    m_aTimer.SetTimeoutHdl(LINK(this, SwView, TimeoutHdl));
    m_bAttrChgNotified = m_bAttrChgNotifiedWithRegistrations = false;
    if (bOldModifyFlag)
        rDocSh.EnableSetModified();
    InvalidateBorder();

    if( !m_pHScrollbar->IsVisible( true ) )
        ShowHScrollbar( false );
    if( !m_pVScrollbar->IsVisible( true ) )
        ShowVScrollbar( false );

    GetViewFrame()->GetWindow().AddChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
}

SwView::~SwView()
{
    // Notify other LOK views that we are going away.
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", "false");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", "");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", "EMPTY");

    GetViewFrame()->GetWindow().RemoveChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
    delete m_pPostItMgr;
    m_pPostItMgr = nullptr;

    m_bInDtor = true;
    m_pEditWin->Hide(); // prevent problems with painting

    // Set pointer in SwDocShell to the view again
    SwDocShell* pDocSh = GetDocShell();
    if( pDocSh && pDocSh->GetView() == this )
        pDocSh->SetView( nullptr );
    if ( SW_MOD()->GetView() == this )
        SW_MOD()->SetView( nullptr );

    if( m_aTimer.IsActive() && m_bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    // the last view must end the text edit
    SdrView *pSdrView = m_pWrtShell ? m_pWrtShell->GetDrawView() : nullptr;
    if( pSdrView && pSdrView->IsTextEdit() )
        pSdrView->SdrEndTextEdit( true );

    SetWindow( nullptr );

    m_pViewImpl->Invalidate();
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    m_pScrollFill.disposeAndClear();
    delete m_pWrtShell;
    m_pWrtShell = nullptr;      // Set to 0, so that it is not accessible by the following dtors cannot.
    m_pShell = nullptr;
    m_pHScrollbar.disposeAndClear();
    m_pVScrollbar.disposeAndClear();
    m_pHRuler.disposeAndClear();
    m_pVRuler.disposeAndClear();
    m_pTogglePageBtn.disposeAndClear();
    delete m_pGlosHdl;
    delete m_pViewImpl;

    // If this was enabled in the ctor for the frame, then disable it here.
    static bool bRequestDoubleBuffering = getenv("VCL_DOUBLEBUFFERING_ENABLE");
    if (bRequestDoubleBuffering)
        m_pEditWin->RequestDoubleBuffering(false);
    m_pEditWin.disposeAndClear();

    delete m_pFormatClipboard;
}

SwDocShell* SwView::GetDocShell()
{
    SfxObjectShell* pDocShell = GetViewFrame()->GetObjectShell();
    return dynamic_cast<SwDocShell*>( pDocShell );
}

// Remember CursorPos

void SwView::WriteUserData( OUString &rUserData, bool bBrowse )
{
    // The browse flag will be passed from Sfx when documents are browsed
    // (not to be confused with the BrowseMode).
    // Then that stored data are not persistent!

    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();

    rUserData = OUString::number( rRect.Left() );
    rUserData += ";";
    rUserData += OUString::number( rRect.Top() );
    rUserData += ";";
    rUserData += OUString::number( m_pWrtShell->GetViewOptions()->GetZoom() );
    rUserData += ";";
    rUserData += OUString::number( rVis.Left() );
    rUserData += ";";
    rUserData += OUString::number( rVis.Top() );
    rUserData += ";";
    rUserData += OUString::number( bBrowse ? SAL_MIN_INT32 : rVis.Right());
    rUserData += ";";
    rUserData += OUString::number( bBrowse ? SAL_MIN_INT32 : rVis.Bottom());
    rUserData += ";";
    rUserData += OUString::number(
            (sal_uInt16)m_pWrtShell->GetViewOptions()->GetZoomType());//eZoom;
    rUserData += ";";
    rUserData += FrameTypeFlags::NONE == m_pWrtShell->GetSelFrameType() ? OUString("0") : OUString("1");
}

// Set CursorPos

static bool lcl_IsOwnDocument( SwView& rView )
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        rView.GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps
        = xDPS->getDocumentProperties();
    OUString Created = xDocProps->getAuthor();
    OUString Changed = xDocProps->getModifiedBy();
    OUString FullName = SW_MOD()->GetUserOptions().GetFullName();
    return (!FullName.isEmpty() &&
            (!Changed.isEmpty() && Changed == FullName )) ||
            (Changed.isEmpty() && !Created.isEmpty() && Created == FullName );
}

void SwView::ReadUserData( const OUString &rUserData, bool bBrowse )
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
        long nX = rUserData.getToken( 0, ';', nPos ).toInt32(),
             nY = rUserData.getToken( 0, ';', nPos ).toInt32();
        Point aCursorPos( nX, nY );

        sal_uInt16 nZoomFactor =
            static_cast< sal_uInt16 >( rUserData.getToken(0, ';', nPos ).toInt32() );

        long nLeft  = rUserData.getToken(0, ';', nPos ).toInt32(),
             nTop   = rUserData.getToken(0, ';', nPos ).toInt32(),
             nRight = rUserData.getToken(0, ';', nPos ).toInt32(),
             nBottom= rUserData.getToken(0, ';', nPos ).toInt32();

        const long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
        if ( nBottom <= (m_pWrtShell->GetDocSize().Height()+nAdd) )
        {
            m_pWrtShell->EnableSmooth( false );

            const Rectangle aVis( nLeft, nTop, nRight, nBottom );

            sal_Int32 nOff = 0;
            SvxZoomType eZoom;
            if( !m_pWrtShell->GetViewOptions()->getBrowseMode() )
                eZoom = (SvxZoomType) (sal_uInt16)rUserData.getToken(nOff, ';', nPos ).toInt32();
            else
            {
                eZoom = SvxZoomType::PERCENT;
                ++nOff;
            }

            bool bSelectObj = (0 != rUserData.getToken( nOff, ';', nPos ).toInt32())
                                && m_pWrtShell->IsObjSelectable( aCursorPos );

            // restore editing position
            m_pViewImpl->SetRestorePosition(aCursorPos, bSelectObj);
            // set flag value to avoid macro execution.
            bool bSavedFlagValue = m_pWrtShell->IsMacroExecAllowed();
            m_pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(m_bOldShellWasPagePreview || bIsOwnDocument)
            {
                m_pWrtShell->SwCursorShell::SetCursor( aCursorPos, !bSelectObj );
                if( bSelectObj )
                {
                    m_pWrtShell->SelectObj( aCursorPos );
                    m_pWrtShell->EnterSelFrameMode( &aCursorPos );
                }
            }

            // reset flag value
            m_pWrtShell->SetMacroExecAllowed( bSavedFlagValue );

            // set visible area before applying
            // information from print preview. Otherwise, the applied information
            // is lost.
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
            if(m_bOldShellWasPagePreview || bIsOwnDocument )
            {
                if ( bBrowse )
                    SetVisArea( aVis.TopLeft() );
                else
                    SetVisArea( aVis );
            }

            //apply information from print preview - if available
            if( !m_sNewCursorPos.isEmpty() )
            {
                long nXTmp = m_sNewCursorPos.getToken( 0, ';' ).toInt32(),
                     nYTmp = m_sNewCursorPos.getToken( 1, ';' ).toInt32();
                Point aCursorPos2( nXTmp, nYTmp );
                bSelectObj = m_pWrtShell->IsObjSelectable( aCursorPos2 );

                m_pWrtShell->SwCursorShell::SetCursor( aCursorPos2 );
                if( bSelectObj )
                {
                    m_pWrtShell->SelectObj( aCursorPos2 );
                    m_pWrtShell->EnterSelFrameMode( &aCursorPos2 );
                }
                m_pWrtShell->MakeSelVisible();
                m_sNewCursorPos.clear();
            }
            else if(USHRT_MAX != m_nNewPage)
            {
                m_pWrtShell->GotoPage(m_nNewPage, true);
                m_nNewPage = USHRT_MAX;
            }

            SelectShell();

            m_pWrtShell->StartAction();
            const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
            if( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom )
                SetZoom( eZoom, nZoomFactor);

            m_pWrtShell->LockView( true );
            m_pWrtShell->EndAction();
            m_pWrtShell->LockView( false );
            m_pWrtShell->EnableSmooth( true );
        }
    }
}

void SwView::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& rSequence )
{
    if(GetDocShell()->IsPreview()||m_bIsPreviewDoubleClick)
        return;
    bool bIsOwnDocument = lcl_IsOwnDocument( *this );
    sal_Int32 nLength = rSequence.getLength();
    if (nLength)
    {
        SET_CURR_SHELL(m_pWrtShell);
        const beans::PropertyValue *pValue = rSequence.getConstArray();
        const SwRect& rRect = m_pWrtShell->GetCharRect();
        const Rectangle &rVis = GetVisArea();
        const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();

        sal_Int64 nX = rRect.Left(), nY = rRect.Top(), nLeft = rVis.Left(), nTop = rVis.Top();
        sal_Int64 nRight = LONG_MIN;
        sal_Int64 nBottom = LONG_MIN;
        sal_Int16 nZoomType = static_cast< sal_Int16 >(pVOpt->GetZoomType());
        sal_Int16 nZoomFactor = static_cast < sal_Int16 > (pVOpt->GetZoom());
        bool bViewLayoutBookMode = pVOpt->IsViewLayoutBookMode();
        sal_Int16 nViewLayoutColumns = pVOpt->GetViewLayoutColumns();

        bool bSelectedFrame = ( m_pWrtShell->GetSelFrameType() != FrameTypeFlags::NONE ),
                 bGotVisibleLeft = false,
                 bGotVisibleTop = false, bGotVisibleRight = false,
                 bGotVisibleBottom = false, bGotZoomType = false,
                 bGotZoomFactor = false, bGotIsSelectedFrame = false,
                 bGotViewLayoutColumns = false, bGotViewLayoutBookMode = false;

        for (sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue->Name == "ViewLeft" )
            {
               pValue->Value >>= nX;
               nX = convertMm100ToTwip( nX );
            }
            else if ( pValue->Name == "ViewTop" )
            {
               pValue->Value >>= nY;
               nY = convertMm100ToTwip( nY );
            }
            else if ( pValue->Name == "VisibleLeft" )
            {
               pValue->Value >>= nLeft;
               nLeft = convertMm100ToTwip( nLeft );
               bGotVisibleLeft = true;
            }
            else if ( pValue->Name == "VisibleTop" )
            {
               pValue->Value >>= nTop;
               nTop = convertMm100ToTwip( nTop );
               bGotVisibleTop = true;
            }
            else if ( pValue->Name == "VisibleRight" )
            {
               pValue->Value >>= nRight;
               nRight = convertMm100ToTwip( nRight );
               bGotVisibleRight = true;
            }
            else if ( pValue->Name == "VisibleBottom" )
            {
               pValue->Value >>= nBottom;
               nBottom = convertMm100ToTwip( nBottom );
               bGotVisibleBottom = true;
            }
            else if ( pValue->Name == "ZoomType" )
            {
               pValue->Value >>= nZoomType;
               bGotZoomType = true;
            }
            else if ( pValue->Name == "ZoomFactor" )
            {
               pValue->Value >>= nZoomFactor;
               bGotZoomFactor = true;
            }
            else if ( pValue->Name == "ViewLayoutColumns" )
            {
               pValue->Value >>= nViewLayoutColumns;
               bGotViewLayoutColumns = true;
            }
            else if ( pValue->Name == "ViewLayoutBookMode" )
            {
               bViewLayoutBookMode = *o3tl::doAccess<bool>(pValue->Value);
               bGotViewLayoutBookMode = true;
            }
            else if ( pValue->Name == "IsSelectedFrame" )
            {
               pValue->Value >>= bSelectedFrame;
               bGotIsSelectedFrame = true;
            }
            // Fallback to common SdrModel processing
            else GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->ReadUserDataSequenceValue(pValue);
            pValue++;
        }
        if (bGotVisibleBottom)
        {
            Point aCursorPos( nX, nY );
            const long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
            if (nBottom <= (m_pWrtShell->GetDocSize().Height()+nAdd) )
            {
                m_pWrtShell->EnableSmooth( false );
                const Rectangle aVis( nLeft, nTop, nRight, nBottom );

                SvxZoomType eZoom;
                if ( !m_pWrtShell->GetViewOptions()->getBrowseMode() )
                    eZoom = static_cast < SvxZoomType > ( nZoomType );
                else
                {
                    eZoom = SvxZoomType::PERCENT;
                }
                if (bGotIsSelectedFrame)
                {
                    bool bSelectObj = bSelectedFrame && m_pWrtShell->IsObjSelectable( aCursorPos );

                    // set flag value to avoid macro execution.
                    bool bSavedFlagValue = m_pWrtShell->IsMacroExecAllowed();
                    m_pWrtShell->SetMacroExecAllowed( false );
// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                    m_pViewImpl->SetRestorePosition(aCursorPos, bSelectObj);
                    if(m_bOldShellWasPagePreview|| bIsOwnDocument)
                    {
                        m_pWrtShell->SwCursorShell::SetCursor( aCursorPos, !bSelectObj );

                        // Update the shell to toggle Header/Footer edit if needed
                        bool bInHeader = true;
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
                            m_pWrtShell->SelectObj( aCursorPos );
                            m_pWrtShell->EnterSelFrameMode( &aCursorPos );
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
                                                  eZoom != SvxZoomType::PERCENT;

                if ( !bZoomNeedsViewLayout )
                    m_pWrtShell->StartAction();

                if ( bSetViewLayoutSettings )
                    SetViewLayout( nViewLayoutColumns, bViewLayoutBookMode, true );

                if ( bZoomNeedsViewLayout )
                    m_pWrtShell->StartAction();

                if ( bSetViewSettings )
                    SetZoom( eZoom, nZoomFactor, true );

// os: changed: The user data has to be read if the view is switched back from page preview
// go to the last editing position when opening own files
                if(m_bOldShellWasPagePreview||bIsOwnDocument)
                {
                    if ( bGotVisibleLeft && bGotVisibleTop )
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

                m_pWrtShell->LockView( true );
                m_pWrtShell->EndAction();
                m_pWrtShell->LockView( false );
                m_pWrtShell->EnableSmooth( true );
            }
        }
    }
}

void SwView::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& rSequence )
{
    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const Rectangle& rVis = GetVisArea();

    std::vector<beans::PropertyValue> aVector;

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    OUStringBuffer sBuffer ( OUString( "view" ) );
    ::sax::Converter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
    aVector.push_back(comphelper::makePropertyValue("ViewId", sBuffer.makeStringAndClear()));

    aVector.push_back(comphelper::makePropertyValue("ViewLeft", convertTwipToMm100 ( rRect.Left() )));

    aVector.push_back(comphelper::makePropertyValue("ViewTop", convertTwipToMm100 ( rRect.Top() )));

    aVector.push_back(comphelper::makePropertyValue("VisibleLeft", convertTwipToMm100 ( rVis.Left() )));

    aVector.push_back(comphelper::makePropertyValue("VisibleTop", convertTwipToMm100 ( rVis.Top() )));

    aVector.push_back(comphelper::makePropertyValue("VisibleRight", convertTwipToMm100 ( rVis.Right() )));

    aVector.push_back(comphelper::makePropertyValue("VisibleBottom", convertTwipToMm100 ( rVis.Bottom() )));

    const sal_Int16 nZoomType = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetZoomType());
    aVector.push_back(comphelper::makePropertyValue("ZoomType", nZoomType));

    const sal_Int16 nViewLayoutColumns = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetViewLayoutColumns());
    aVector.push_back(comphelper::makePropertyValue("ViewLayoutColumns", nViewLayoutColumns));

    aVector.push_back(comphelper::makePropertyValue("ViewLayoutBookMode", m_pWrtShell->GetViewOptions()->IsViewLayoutBookMode()));

    aVector.push_back(comphelper::makePropertyValue("ZoomFactor", static_cast < sal_Int16 > (m_pWrtShell->GetViewOptions()->GetZoom())));

    aVector.push_back(comphelper::makePropertyValue("IsSelectedFrame", FrameTypeFlags::NONE != m_pWrtShell->GetSelFrameType()));

    rSequence = comphelper::containerToSequence(aVector);

    // Common SdrModel processing
    GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->WriteUserDataSequence(rSequence);
}

void SwView::ShowCursor( bool bOn )
{
    //don't scroll the cursor into the visible area
    bool bUnlockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( true );    //lock visible section

    if( !bOn )
        m_pWrtShell->HideCursor();
    else if( !m_pWrtShell->IsFrameSelected() && !m_pWrtShell->IsObjSelected() )
        m_pWrtShell->ShowCursor();

    if( bUnlockView )
        m_pWrtShell->LockView( false );
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

//   only return true for a text selection

bool SwView::HasSelection( bool  bText ) const
{
    return bText ? GetWrtShell().SwCursorShell::HasSelection()
                 : GetWrtShell().HasSelection();
}

OUString SwView::GetSelectionText( bool bCompleteWrds )
{
    return GetSelectionTextParam( bCompleteWrds, true );
}

OUString SwView::GetSelectionTextParam( bool bCompleteWrds, bool bEraseTrail )
{
    OUString sReturn;
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
    bool bCallBase = true;
    if ( dynamic_cast<const SfxSimpleHint*>(&rHint) )
    {
        sal_uInt32 nId = static_cast<const SfxSimpleHint&>(rHint).GetId();
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
                    bool bModal = GetDocShell()->IsInModalMode();
                    m_pHRuler->SetActive( !bModal );
                    m_pVRuler->SetActive( !bModal );
                }

                SAL_FALLTHROUGH;

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
                        SwDrawModel * pDrawDoc = GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                        if (pDrawDoc)
                        {
                            if( !pDrawDoc->GetOpenInDesignMode() )
                                break;// don't touch the design mode
                        }
                    }
                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadonly);
                    GetDispatcher().ExecuteList(SID_FM_DESIGN_MODE,
                            SfxCallMode::ASYNCHRON, { &aItem });
                }
                break;

            case SW_BROADCAST_DRAWVIEWS_CREATED:
                {
                    bCallBase = false;
                    if ( GetFormShell() )
                    {
                        GetFormShell()->SetView(
                            dynamic_cast<FmFormView*>( GetWrtShell().GetDrawView())  );
                        SfxBoolItem aItem( SID_FM_DESIGN_MODE, !GetDocShell()->IsReadOnly());
                        GetDispatcher().ExecuteList(SID_FM_DESIGN_MODE,
                                SfxCallMode::SYNCHRON, { &aItem });
                    }
                }
                break;
        }
    }
    else if(dynamic_cast<const FmDesignModeChangedHint*>(&rHint))
    {
        bool bDesignMode = static_cast<const FmDesignModeChangedHint&>(rHint).GetDesignMode();
        if (!bDesignMode && GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(nullptr);
            LeaveDrawCreate();
            AttrChangedNotify(m_pWrtShell);
        }
    }

    if ( bCallBase )
        SfxViewShell::Notify(rBC, rHint);
}

#if defined(_WIN32) || defined UNX

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
                    m_pWrtShell->Insert( OUString(), OUString(), aGrf );
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
            m_bAttrChgNotifiedWithRegistrations = false;
        }
        SelectShell();
        m_bAttrChgNotified = false;
    }
}

bool SwView::PrepareClose( bool bUI )
{
    SfxViewFrame* pVFrame = GetViewFrame();
    pVFrame->SetChildWindow( SwInputChild::GetChildWindowId(), false );
    if( pVFrame->GetDispatcher()->IsLocked() )
        pVFrame->GetDispatcher()->Lock(false);

    if ( m_pFormShell && !m_pFormShell->PrepareClose( bUI ) )
    {
        return false;
    }
    return SfxViewShell::PrepareClose( bUI );
}

// status methods for clipboard.
// Status changes now notified from the clipboard.
bool SwView::IsPasteAllowed()
{
    SotExchangeDest nPasteDestination = SwTransferable::GetSotDestination( *m_pWrtShell );
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
            m_bPasteState = m_bPasteSpecialState = false;

        if( static_cast<SotExchangeDest>(0xFFFF) == m_nLastPasteDestination )  // the init value
            m_pViewImpl->AddClipboardListener();
        m_nLastPasteDestination = nPasteDestination;
    }
    return m_bPasteState;
}

bool SwView::IsPasteSpecialAllowed()
{
    if ( m_pFormShell && m_pFormShell->IsActiveControl() )
        return false;

    SotExchangeDest nPasteDestination = SwTransferable::GetSotDestination( *m_pWrtShell );
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
            m_bPasteState = m_bPasteSpecialState = false;

        if( static_cast<SotExchangeDest>(0xFFFF) == m_nLastPasteDestination )  // the init value
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
    SwXTextView *const pTempImpl = GetViewImpl()->GetUNOObject_Impl();
    return pTempImpl->BuildTmpSelectionDoc();
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
