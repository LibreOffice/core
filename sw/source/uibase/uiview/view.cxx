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

#include <sal/config.h>

#include <string_view>

#include <config_features.h>
#include <config_wasm_strip.h>

#include <stdlib.h>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <o3tl/any.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/graph.hxx>
#include <vcl/inputctx.hxx>
#include <svl/eitem.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/event.hxx>
#include <sfx2/infobar.hxx>
#include <svx/ruler.hxx>
#include <svx/srchdlg.hxx>
#include <svx/fmshell.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/fmview.hxx>
#include <unotxvw.hxx>
#include <cmdid.h>
#include <svl/hint.hxx>
#include <swmodule.hxx>
#include <inputwin.hxx>
#include <uivwimp.hxx>
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
#include <gloshdl.hxx>
#include <usrpref.hxx>
#include <srcview.hxx>
#include <strings.hrc>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <DocumentFieldsManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <drawdoc.hxx>
#include <wdocsh.hxx>
#include <wrtsh.hxx>
#include <barcfg.hxx>
#include <pview.hxx>
#include <swdtflvr.hxx>
#include <prtopt.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/scanner/ScannerContext.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <sal/log.hxx>

#include <formatclipboard.hxx>
#include <PostItMgr.hxx>
#include <annotsh.hxx>
#include <swruler.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <comphelper/propertyvalue.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svtools/embedhlp.hxx>
#include <tools/UnitConversion.hxx>
#include <svx/svdoutl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;

#define SWVIEWFLAGS SfxViewShellFlags::HAS_PRINTOPTIONS

// Statics. OMG.

bool bDocSzUpdated = true;

SvxSearchItem*  SwView::s_pSrchItem   = nullptr;

bool            SwView::s_bExtra      = false;
bool            SwView::s_bFound      = false;
bool            SwView::s_bJustOpened = false;

std::unique_ptr<SearchAttrItemList>  SwView::s_xSearchList;
std::unique_ptr<SearchAttrItemList>  SwView::s_xReplaceList;

SfxDispatcher &SwView::GetDispatcher()
{
    return *GetViewFrame()->GetDispatcher();
}

void SwView::ImpSetVerb( SelectionType nSelType )
{
    bool bResetVerbs = m_bVerbsActive;
    if ( !GetViewFrame()->GetFrame().IsInPlace() &&
         (SelectionType::Ole|SelectionType::Graphic) & nSelType )
    {
        FlyProtectFlags eProtectFlags = m_pWrtShell->IsSelObjProtected(FlyProtectFlags::Content);
        if (eProtectFlags == FlyProtectFlags::NONE || nSelType & SelectionType::Ole)
        {
            if ( nSelType & SelectionType::Ole )
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
        const_cast< SwView* >( this )->AttrChangedNotify(nullptr);
    }
    else if ( m_pPostItMgr )
    {
        SwAnnotationShell* pAsAnnotationShell = dynamic_cast<SwAnnotationShell*>( pTopShell  );
        if ( pAsAnnotationShell )
        {
            m_pPostItMgr->SetActiveSidebarWin(nullptr);
            const_cast< SwView* >( this )->AttrChangedNotify(nullptr);
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

IMPL_LINK_NOARG(SwView, FormControlActivated, LinkParamNone*, void)
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

        AttrChangedNotify(nullptr);
    }
}

namespace
{
uno::Reference<frame::XLayoutManager> getLayoutManager(const SfxViewFrame& rViewFrame)
{
    uno::Reference<frame::XLayoutManager> xLayoutManager;
    uno::Reference<beans::XPropertySet> xPropSet(rViewFrame.GetFrame().GetFrameInterface(),
                                                 uno::UNO_QUERY);
    if (xPropSet.is())
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue("LayoutManager"), uno::UNO_QUERY);
        }
        catch (const Exception& e)
        {
            SAL_WARN("sw.ui", "Failure getting layout manager: " + e.Message);
        }
    }
    return xLayoutManager;
}
}

void SwView::ShowUIElement(const OUString& sElementURL) const
{
    if (auto xLayoutManager = getLayoutManager(*GetViewFrame()))
    {
        if (!xLayoutManager->getElement(sElementURL).is())
        {
            xLayoutManager->createElement(sElementURL);
            xLayoutManager->showElement(sElementURL);
        }
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
    SelectionType nNewSelectionType = m_pWrtShell->GetSelectionType()
                                & ~SelectionType::TableCell;

    if ( m_pFormShell && m_pFormShell->IsActiveControl() )
        nNewSelectionType |= SelectionType::FormControl;

    if ( nNewSelectionType == m_nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( false );
        if ( m_nSelectionType & SelectionType::Ole ||
             m_nSelectionType & SelectionType::Graphic )
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
            ToolbarId eId = rDispatcher.GetObjectBarId(SFX_OBJECTBAR_OBJECT);
            if (eId != ToolbarId::None)
                pBarCfg->SetTopToolbar(m_nSelectionType, eId);

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
        ShellMode eShellMode;

        if ( !( m_nSelectionType & SelectionType::FormControl ) )
            rDispatcher.Push( *m_pFormShell );

        m_pShell = new SwNavigationShell( *this );
        rDispatcher.Push( *m_pShell );

        if ( m_nSelectionType & SelectionType::Ole )
        {
            eShellMode = ShellMode::Object;
            m_pShell = new SwOleShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & SelectionType::Frame
            || m_nSelectionType & SelectionType::Graphic)
        {
            eShellMode = ShellMode::Frame;
            m_pShell = new SwFrameShell( *this );
            rDispatcher.Push( *m_pShell );
            if(m_nSelectionType & SelectionType::Graphic )
            {
                eShellMode = ShellMode::Graphic;
                m_pShell = new SwGrfShell( *this );
                rDispatcher.Push( *m_pShell );
            }
        }
        else if ( m_nSelectionType & SelectionType::DrawObject )
        {
            eShellMode = ShellMode::Draw;
            m_pShell = new SwDrawShell( *this );
            rDispatcher.Push( *m_pShell );

            if ( m_nSelectionType & SelectionType::Ornament )
            {
                eShellMode = ShellMode::Bezier;
                m_pShell = new SwBezierShell( *this );
                rDispatcher.Push( *m_pShell );
            }
#if HAVE_FEATURE_AVMEDIA
            else if( m_nSelectionType & SelectionType::Media )
            {
                eShellMode = ShellMode::Media;
                m_pShell = new SwMediaShell( *this );
                rDispatcher.Push( *m_pShell );
            }
#endif
            if (m_nSelectionType & SelectionType::ExtrudedCustomShape)
            {
                eShellMode = ShellMode::ExtrudedCustomShape;
                m_pShell = new svx::ExtrusionBar(this);
                rDispatcher.Push( *m_pShell );
            }
            if (m_nSelectionType & SelectionType::FontWork)
            {
                eShellMode = ShellMode::FontWork;
                m_pShell = new svx::FontworkBar(this);
                rDispatcher.Push( *m_pShell );
            }
        }
        else if ( m_nSelectionType & SelectionType::DbForm )
        {
            eShellMode = ShellMode::DrawForm;
            m_pShell = new SwDrawFormShell( *this );

            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & SelectionType::DrawObjectEditMode )
        {
            bSetExtInpCntxt = true;
            eShellMode = ShellMode::DrawText;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            m_pShell = new SwDrawTextShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else if ( m_nSelectionType & SelectionType::PostIt )
        {
            eShellMode = ShellMode::PostIt;
            m_pShell = new SwAnnotationShell( *this );
            rDispatcher.Push( *m_pShell );
        }
        else
        {
            bSetExtInpCntxt = true;
            eShellMode = ShellMode::Text;
            if ( m_nSelectionType & SelectionType::NumberList )
            {
                eShellMode = ShellMode::ListText;
                m_pShell = new SwListShell( *this );
                rDispatcher.Push( *m_pShell );
            }
            m_pShell = new SwTextShell(*this);
            rDispatcher.Push( *m_pShell );
            if ( m_nSelectionType & SelectionType::Table )
            {
                eShellMode = eShellMode == ShellMode::ListText ? ShellMode::TableListText
                                                        : ShellMode::TableText;
                m_pShell = new SwTableShell( *this );
                rDispatcher.Push( *m_pShell );
            }
        }

        if ( m_nSelectionType & SelectionType::FormControl )
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

        // Show Mail Merge toolbar initially for documents with Database fields
        if (!m_bInitOnceCompleted && GetWrtShell().IsAnyDatabaseFieldInDoc())
            ShowUIElement("private:resource/toolbar/mailmerge");

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

    m_bInitOnceCompleted = true;
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

IMPL_LINK_NOARG(SwView, AttrChangedNotify, LinkParamNone*, void)
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
        if (m_pWrtShell->ActionPend() || g_bNoInterrupt ||
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

IMPL_LINK_NOARG(SwView, TimeoutHdl, Timer *, void)
{
    if (m_pWrtShell->ActionPend() || g_bNoInterrupt)
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
            SID_PASTE_UNFORMATTED,      FN_PASTE_NESTED_TABLE,      FN_TABLE_PASTE_ROW_BEFORE,
            FN_TABLE_PASTE_COL_BEFORE,  SID_PASTE_SPECIAL,          SID_SBA_BRW_INSERT,
            SID_BACKGROUND_COLOR,       FN_INSERT_BOOKMARK,         SID_CHARMAP_CONTROL,
            SID_CHARMAP,                SID_EMOJI_CONTROL,          FN_INSERT_SOFT_HYPHEN,
            FN_INSERT_HARDHYPHEN,       FN_INSERT_HARD_SPACE,       FN_INSERT_NNBSP,
            FN_INSERT_BREAK,            FN_INSERT_LINEBREAK,        FN_INSERT_COLUMN_BREAK,
            FN_INSERT_BREAK_DLG,
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
            SID_ULINE_VAL_SINGLE,       SID_ULINE_VAL_DOUBLE,       SID_ULINE_VAL_DOTTED,
            SID_ATTR_CHAR_CONTOUR,      SID_ATTR_CHAR_SHADOWED,
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
            qsort( static_cast<void*>(aROIds), SAL_N_ELEMENTS(aROIds), sizeof(sal_uInt16), lcl_CmpIds );
            bFirst = false;
        }
        if ( SfxItemState::DISABLED == eStateRO )
        {
            rDis.SetSlotFilter( SfxSlotFilterState::ENABLED_READONLY, aROIds );
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
                qsort( static_cast<void*>(aAllProtIds), SAL_N_ELEMENTS(aAllProtIds), sizeof(sal_uInt16), lcl_CmpIds );
                bAllProtFirst = false;
            }
            rDis.SetSlotFilter( SfxSlotFilterState::ENABLED_READONLY, aAllProtIds );
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
    SfxDisableFlags nDisableFlags = SfxDisableFlags::NONE;
    SfxDispatcher &rDis = GetDispatcher();

    if( m_pWrtShell->HasReadonlySel() &&
        ( !m_pWrtShell->GetDrawView() ||
            !m_pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount() ))
        nDisableFlags |= SfxDisableFlags::SwOnProtectedCursor;

    if( (SfxDisableFlags::SwOnProtectedCursor & nDisableFlags ) !=
        (SfxDisableFlags::SwOnProtectedCursor & rDis.GetDisableFlags() ) )
    {
        // Additionally move at the Window the InputContext, so that
        // in japanese / chinese versions the external input will be
        // turned on or off. This but only if the correct shell is on
        // the stack.
        switch( m_pViewImpl->GetShellMode() )
        {
        case ShellMode::Text:
        case ShellMode::ListText:
        case ShellMode::TableText:
        case ShellMode::TableListText:
            {
// Temporary solution!!! Should set the font of the current insertion point
//         at each cursor movement, so outside of this "if". But TH does not
//         evaluates the font at this time and the "purchase" appears to me
//         as too expensive.
//         Moreover, we don't have a font, but only attributes from which the
//         text formatting and the correct font will be build together.

                InputContext aCntxt( GetEditWin().GetInputContext() );
                aCntxt.SetOptions( SfxDisableFlags::SwOnProtectedCursor & nDisableFlags
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
    m_aTimer( "sw::SwView m_aTimer" ),
    m_nNewPage(USHRT_MAX),
    m_nOldPageNum(0),
    m_pNumRuleNodeFromDoc(nullptr),
    m_pEditWin( VclPtr<SwEditWin>::Create( &_pFrame->GetWindow(), *this ) ),
    m_pShell(nullptr),
    m_pFormShell(nullptr),
    m_pHScrollbar(nullptr),
    m_pVScrollbar(nullptr),
    m_pScrollFill(VclPtr<ScrollBarBox>::Create( &_pFrame->GetWindow(), WB_SIZEABLE )),
    m_pVRuler(VclPtr<SvxRuler>::Create(&GetViewFrame()->GetWindow(), m_pEditWin,
                            SvxRulerSupportFlags::TABS | SvxRulerSupportFlags::PARAGRAPH_MARGINS_VERTICAL|
                                SvxRulerSupportFlags::BORDERS | SvxRulerSupportFlags::REDUCED_METRIC,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_EXTRAFIELD | WB_BORDER )),
    m_pLastTableFormat(nullptr),
    m_pFormatClipboard(new SwFormatClipboard()),
    m_nSelectionType( SelectionType::All ),
    m_nPageCnt(0),
    m_nDrawSfxId( USHRT_MAX ),
    m_nFormSfxId( USHRT_MAX ),
    m_eFormObjKind(OBJ_NONE),
    m_nLastPasteDestination( static_cast<SotExchangeDest>(0xFFFF) ),
    m_nLeftBorderDistance( 0 ),
    m_nRightBorderDistance( 0 ),
    m_eLastSearchCommand( static_cast<SvxSearchCmd>(0xFFFF) ),
    m_bWheelScrollInProgress(false),
    m_bCenterCursor(false),
    m_bTopCursor(false),
    m_bTabColFromDoc(false),
    m_bTabRowFromDoc(false),
    m_bSetTabColFromDoc(false),
    m_bSetTabRowFromDoc(false),
    m_bAttrChgNotified(false),
    m_bAttrChgNotifiedWithRegistrations(false),
    m_bVerbsActive(false),
    m_bDrawRotate(false),
    m_bDrawSelMode(true),
    m_bShowAtResize(true),
    m_bInOuterResizePixel(false),
    m_bInInnerResizePixel(false),
    m_bPasteState(false),
    m_bPasteSpecialState(false),
    m_bInMailMerge(false),
    m_bInDtor(false),
    m_bOldShellWasPagePreview(false),
    m_bIsPreviewDoubleClick(false),
    m_bMakeSelectionVisible(false),
    m_bForceChangesToolbar(true),
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

    bDocSzUpdated = true;

    CreateScrollbar( true );
    CreateScrollbar( false );

    m_pViewImpl.reset(new SwView_Impl(this));
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
        if (SwPagePreview* pPagePreview = dynamic_cast<SwPagePreview *>(pExistingSh))
        {
            m_sSwViewData = pPagePreview->GetPrevSwViewData();
            m_sNewCursorPos = pPagePreview->GetNewCursorPos();
            m_nNewPage = pPagePreview->GetNewPage();
            m_bOldShellWasPagePreview = true;
            m_bIsPreviewDoubleClick = !m_sNewCursorPos.isEmpty() || m_nNewPage != USHRT_MAX;
        }
        else if (dynamic_cast<const SwSrcView *>(pExistingSh) != nullptr)
            bOldShellWasSrcView = true;
    }

    SAL_INFO( "sw.ui", "before create WrtShell" );
    if (SwView *pView = dynamic_cast<SwView*>(pExistingSh))
    {
        m_pWrtShell.reset(new SwWrtShell(*pView->m_pWrtShell, m_pEditWin, *this));
    }
    else if (SwWrtShell *pWrtShell = dynamic_cast<SwWrtShell*>(rDocSh.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell()))
    {
        m_pWrtShell.reset(new SwWrtShell(*pWrtShell, m_pEditWin, *this));
    }
    else
    {
        SwDoc& rDoc = *rDocSh.GetDoc();

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
        m_pWrtShell.reset(new SwWrtShell(rDoc, m_pEditWin, *this, &aUsrPref));
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
    m_pHRuler = VclPtr<SwCommentRuler>::Create(m_pWrtShell.get(), &GetViewFrame()->GetWindow(), m_pEditWin,
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

    StartListening(*pViewFrame, DuplicateHandling::Prevent);
    StartListening(rDocSh, DuplicateHandling::Prevent);

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
    m_xGlueDocShell.reset(new SwViewGlueDocShell(*this, rDocSh));
    m_pPostItMgr.reset(new SwPostItMgr(this));

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
        "sw.ui", "/org.openoffice.Office.Common/Undo/Steps <= 0");
    if (!utl::ConfigManager::IsFuzzing() && 0 < officecfg::Office::Common::Undo::Steps::get())
    {
        m_pWrtShell->DoUndo();
    }

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
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    m_pWrtShell->ApplyAccessibilityOptions(SW_MOD()->GetAccessibilityOptions());
#endif

    if( m_pWrtShell->GetDoc()->getIDocumentState().IsUpdateExpField() )
    {
        if (m_pWrtShell->GetDoc()->GetDocumentFieldsManager().containsUpdatableFields())
        {
            CurrShell aCurr(m_pWrtShell.get());
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

    m_aTimer.SetInvokeHandler(LINK(this, SwView, TimeoutHdl));
    m_bAttrChgNotified = m_bAttrChgNotifiedWithRegistrations = false;
    if (bOldModifyFlag)
        rDocSh.EnableSetModified();
    InvalidateBorder();

    if( !m_pHScrollbar->IsVisible( true ) )
        ShowHScrollbar( false );
    if( !m_pVScrollbar->IsVisible( true ) )
        ShowVScrollbar( false );

    if (m_pWrtShell && m_pWrtShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
        m_pWrtShell->InvalidateOutlineContentVisibility();

    GetViewFrame()->GetWindow().AddChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
}

SwViewGlueDocShell::SwViewGlueDocShell(SwView& rView, SwDocShell& rDocSh)
    : m_rView(rView)
{
    // Set DocShell
    rDocSh.SetView(&m_rView);
    SW_MOD()->SetView(&m_rView);
}

SwViewGlueDocShell::~SwViewGlueDocShell()
{
    SwDocShell* pDocSh = m_rView.GetDocShell();
    if (pDocSh && pDocSh->GetView() == &m_rView)
        pDocSh->SetView(nullptr);
    if (SW_MOD()->GetView() == &m_rView)
        SW_MOD()->SetView(nullptr);
}

SwView::~SwView()
{
    // Notify other LOK views that we are going away.
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", "false");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", "");
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", "EMPTY");

    // Need to remove activated field's button before disposing EditWin.
    GetWrtShell().getIDocumentMarkAccess()->ClearFieldActivation();

    GetViewFrame()->GetWindow().RemoveChildEventListener( LINK( this, SwView, WindowChildEventListener ) );
    m_pPostItMgr.reset();

    m_bInDtor = true;
    m_pEditWin->Hide(); // prevent problems with painting

    // Set pointer in SwDocShell to the view again
    m_xGlueDocShell.reset();

    if( m_aTimer.IsActive() && m_bAttrChgNotifiedWithRegistrations )
        GetViewFrame()->GetBindings().LEAVEREGISTRATIONS();

    // the last view must end the text edit
    SdrView *pSdrView = m_pWrtShell ? m_pWrtShell->GetDrawView() : nullptr;
    if( pSdrView && pSdrView->IsTextEdit() )
        pSdrView->SdrEndTextEdit( true );
    else if (pSdrView)
    {
        pSdrView->DisposeUndoManager();
    }

    SetWindow( nullptr );

    m_pViewImpl->Invalidate();
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    m_pScrollFill.disposeAndClear();
    m_pWrtShell.reset(); // reset here so that it is not accessible by the following dtors.
    m_pHScrollbar.disposeAndClear();
    m_pVScrollbar.disposeAndClear();
    m_pHRuler.disposeAndClear();
    m_pVRuler.disposeAndClear();
    m_pGlosHdl.reset();
    m_pViewImpl.reset();

    // If this was enabled in the ctor for the frame, then disable it here.
    static bool bRequestDoubleBuffering = getenv("VCL_DOUBLEBUFFERING_ENABLE");
    if (bRequestDoubleBuffering)
        m_pEditWin->RequestDoubleBuffering(false);
    m_pEditWin.disposeAndClear();

    m_pFormatClipboard.reset();
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
    const tools::Rectangle& rVis = GetVisArea();

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
            static_cast<sal_uInt16>(m_pWrtShell->GetViewOptions()->GetZoomType()));//eZoom;
    rUserData += ";";
    rUserData += FrameTypeFlags::NONE == m_pWrtShell->GetSelFrameType() ? std::u16string_view(u"0") : std::u16string_view(u"1");
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
    return !FullName.isEmpty()
           && (Changed == FullName || (Changed.isEmpty() && Created == FullName));
}

void SwView::ReadUserData( const OUString &rUserData, bool bBrowse )
{
    if ( !(rUserData.indexOf(';')>=0 && // more than one token
        // For document without layout only in the onlinelayout or
        // while forward/backward
         (!m_pWrtShell->IsNewLayout() || m_pWrtShell->GetViewOptions()->getBrowseMode() || bBrowse)) )
        return;

    bool bIsOwnDocument = lcl_IsOwnDocument( *this );

    CurrShell aCurr(m_pWrtShell.get());

    sal_Int32 nPos = 0;

    // No it is *not* a good idea to call GetToken within Point constr. immediately,
    // because which parameter is evaluated first?
    tools::Long nX = rUserData.getToken( 0, ';', nPos ).toInt32(),
         nY = rUserData.getToken( 0, ';', nPos ).toInt32();
    Point aCursorPos( nX, nY );

    sal_uInt16 nZoomFactor =
        static_cast< sal_uInt16 >( rUserData.getToken(0, ';', nPos ).toInt32() );

    tools::Long nLeft  = rUserData.getToken(0, ';', nPos ).toInt32(),
         nTop   = rUserData.getToken(0, ';', nPos ).toInt32(),
         nRight = rUserData.getToken(0, ';', nPos ).toInt32(),
         nBottom= rUserData.getToken(0, ';', nPos ).toInt32();

    const tools::Long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
    if ( nBottom > (m_pWrtShell->GetDocSize().Height()+nAdd) )
        return;

    m_pWrtShell->EnableSmooth( false );

    const tools::Rectangle aVis( nLeft, nTop, nRight, nBottom );

    sal_Int32 nOff = 0;
    SvxZoomType eZoom;
    if( !m_pWrtShell->GetViewOptions()->getBrowseMode() )
        eZoom = static_cast<SvxZoomType>(o3tl::narrowing<sal_uInt16>(rUserData.getToken(nOff, ';', nPos ).toInt32()));
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
        sal_Int32 nIdx{ 0 };
        const tools::Long nXTmp = m_sNewCursorPos.getToken( 0, ';', nIdx ).toInt32();
        const tools::Long nYTmp = m_sNewCursorPos.getToken( 0, ';', nIdx ).toInt32();
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

void SwView::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& rSequence )
{
    if(GetDocShell()->IsPreview()||m_bIsPreviewDoubleClick)
        return;
    bool bIsOwnDocument = lcl_IsOwnDocument( *this );

    CurrShell aCurr(m_pWrtShell.get());
    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const tools::Rectangle &rVis = GetVisArea();
    const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();

    sal_Int64 nX = rRect.Left(), nY = rRect.Top(), nLeft = rVis.Left(), nTop = rVis.Top();
    sal_Int64 nRight = nLeft;
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
             bGotViewLayoutColumns = false, bGotViewLayoutBookMode = false,
             bBrowseMode = false, bGotBrowseMode = false;
    bool bKeepRatio = pVOpt->IsKeepRatio();
    bool bGotKeepRatio = false;

    for (const beans::PropertyValue& rValue : rSequence)
    {
        if ( rValue.Name == "ViewLeft" )
        {
           rValue.Value >>= nX;
           nX = o3tl::toTwips(nX, o3tl::Length::mm100);
        }
        else if ( rValue.Name == "ViewTop" )
        {
           rValue.Value >>= nY;
           nY = o3tl::toTwips(nY, o3tl::Length::mm100);
        }
        else if ( rValue.Name == "VisibleLeft" )
        {
           rValue.Value >>= nLeft;
           nLeft = o3tl::toTwips(nLeft, o3tl::Length::mm100);
           bGotVisibleLeft = true;
        }
        else if ( rValue.Name == "VisibleTop" )
        {
           rValue.Value >>= nTop;
           nTop = o3tl::toTwips(nTop, o3tl::Length::mm100);
           bGotVisibleTop = true;
        }
        else if ( rValue.Name == "VisibleRight" )
        {
           rValue.Value >>= nRight;
           nRight = o3tl::toTwips(nRight, o3tl::Length::mm100);
           bGotVisibleRight = true;
        }
        else if ( rValue.Name == "VisibleBottom" )
        {
           rValue.Value >>= nBottom;
           nBottom = o3tl::toTwips(nBottom, o3tl::Length::mm100);
           bGotVisibleBottom = true;
        }
        else if ( rValue.Name == "ZoomType" )
        {
           rValue.Value >>= nZoomType;
           bGotZoomType = true;
        }
        else if ( rValue.Name == "ZoomFactor" )
        {
           rValue.Value >>= nZoomFactor;
           bGotZoomFactor = true;
        }
        else if ( rValue.Name == "ViewLayoutColumns" )
        {
           rValue.Value >>= nViewLayoutColumns;
           bGotViewLayoutColumns = true;
        }
        else if ( rValue.Name == "ViewLayoutBookMode" )
        {
           bViewLayoutBookMode = *o3tl::doAccess<bool>(rValue.Value);
           bGotViewLayoutBookMode = true;
        }
        else if ( rValue.Name == "IsSelectedFrame" )
        {
           rValue.Value >>= bSelectedFrame;
           bGotIsSelectedFrame = true;
        }
        else if (rValue.Name == "ShowOnlineLayout")
        {
           rValue.Value >>= bBrowseMode;
           bGotBrowseMode = true;
        }
        else if (rValue.Name == "KeepRatio")
        {
            rValue.Value >>= bKeepRatio;
            bGotKeepRatio = true;
        }
        // Fallback to common SdrModel processing
        else
           GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->ReadUserDataSequenceValue(&rValue);
    }
    if (bGotBrowseMode)
    {
        // delegate further
        GetViewImpl()->GetUNOObject_Impl()->getViewSettings()->setPropertyValue("ShowOnlineLayout", uno::Any(bBrowseMode));
    }

    SelectShell();

    if (!bGotVisibleBottom)
        return;

    Point aCursorPos( nX, nY );
    const tools::Long nAdd = m_pWrtShell->GetViewOptions()->getBrowseMode() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
    if (nBottom > (m_pWrtShell->GetDocSize().Height()+nAdd) )
        return;

    m_pWrtShell->EnableSmooth( false );
    const tools::Rectangle aVis( nLeft, nTop, nRight, nBottom );

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
                    m_pWrtShell->SetShowHeaderFooterSeparator( FrameControlType::Footer, true );
                    m_pWrtShell->SetShowHeaderFooterSeparator( FrameControlType::Header, false );
                }
                else
                {
                    m_pWrtShell->SetShowHeaderFooterSeparator( FrameControlType::Header, true );
                    m_pWrtShell->SetShowHeaderFooterSeparator( FrameControlType::Footer, false );
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

    if (bGotKeepRatio && bKeepRatio != pVOpt->IsKeepRatio())
    {
        // Got a custom value, then it makes sense to trigger notifications.
        SwViewOption aUsrPref(*pVOpt);
        aUsrPref.SetKeepRatio(bKeepRatio);
        SW_MOD()->ApplyUsrPref(aUsrPref, this);
    }

    // Set ViewLayoutSettings
    const bool bSetViewLayoutSettings = bGotViewLayoutColumns && bGotViewLayoutBookMode &&
                                        ( pVOpt->GetViewLayoutColumns() != nViewLayoutColumns || pVOpt->IsViewLayoutBookMode() != bViewLayoutBookMode );

    const bool bSetViewSettings = bGotZoomType && bGotZoomFactor &&
                                  ( pVOpt->GetZoom() != nZoomFactor || pVOpt->GetZoomType() != eZoom );

    // In case we have a 'fixed' view layout of 2 or more columns,
    // we have to apply the view options *before* starting the action.
    // Otherwise the SetZoom function cannot work correctly, because
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
            SwTwips nEditWidth = GetEditWin().GetOutDev()->GetOutputSize().Width();
            if(nEditWidth > (m_aDocSz.Width() + lBorder ))
                aTopLeft.setX( ( m_aDocSz.Width() + lBorder - nEditWidth  ) / 2 );
            else
            {
                //check if the values are possible
                tools::Long nXMax = m_pHScrollbar->GetRangeMax() - m_pHScrollbar->GetVisibleSize();
                if( aTopLeft.X() > nXMax )
                    aTopLeft.setX( nXMax < 0 ? 0 : nXMax );
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

void SwView::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& rSequence )
{
    const SwRect& rRect = m_pWrtShell->GetCharRect();
    const tools::Rectangle& rVis = GetVisArea();

    std::vector<beans::PropertyValue> aVector;

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    aVector.push_back(comphelper::makePropertyValue("ViewId", "view" + OUString::number(nViewID)));

    aVector.push_back(comphelper::makePropertyValue("ViewLeft", convertTwipToMm100 ( rRect.Left() )));

    aVector.push_back(comphelper::makePropertyValue("ViewTop", convertTwipToMm100 ( rRect.Top() )));

    auto visibleLeft = convertTwipToMm100 ( rVis.Left() );
    aVector.push_back(comphelper::makePropertyValue("VisibleLeft", visibleLeft));

    auto visibleTop = convertTwipToMm100 ( rVis.Top() );
    aVector.push_back(comphelper::makePropertyValue("VisibleTop", visibleTop));

    auto visibleRight = rVis.IsWidthEmpty() ? visibleLeft : convertTwipToMm100 ( rVis.Right() );
    aVector.push_back(comphelper::makePropertyValue("VisibleRight", visibleRight));

    auto visibleBottom = rVis.IsHeightEmpty() ? visibleTop : convertTwipToMm100 ( rVis.Bottom() );
    aVector.push_back(comphelper::makePropertyValue("VisibleBottom", visibleBottom));

    const sal_Int16 nZoomType = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetZoomType());
    aVector.push_back(comphelper::makePropertyValue("ZoomType", nZoomType));

    const sal_Int16 nViewLayoutColumns = static_cast< sal_Int16 >(m_pWrtShell->GetViewOptions()->GetViewLayoutColumns());
    aVector.push_back(comphelper::makePropertyValue("ViewLayoutColumns", nViewLayoutColumns));

    aVector.push_back(comphelper::makePropertyValue("ViewLayoutBookMode", m_pWrtShell->GetViewOptions()->IsViewLayoutBookMode()));

    aVector.push_back(comphelper::makePropertyValue("ZoomFactor", static_cast < sal_Int16 > (m_pWrtShell->GetViewOptions()->GetZoom())));

    aVector.push_back(comphelper::makePropertyValue("IsSelectedFrame", FrameTypeFlags::NONE != m_pWrtShell->GetSelFrameType()));

    aVector.push_back(
        comphelper::makePropertyValue("KeepRatio", m_pWrtShell->GetViewOptions()->IsKeepRatio()));

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

ErrCode SwView::DoVerb(sal_Int32 nVerb)
{
    if ( !GetViewFrame()->GetFrame().IsInPlace() )
    {
        SwWrtShell &rSh = GetWrtShell();
        const SelectionType nSel = rSh.GetSelectionType();
        if ( nSel & SelectionType::Ole )
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
        m_pGlosHdl.reset(new SwGlossaryHdl(GetViewFrame(), m_pWrtShell.get()));
    return m_pGlosHdl.get();
}

void SwView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    bool bCallBase = true;
    if(auto pChangedHint = dynamic_cast<const FmDesignModeChangedHint*>(&rHint))
    {
        bool bDesignMode = pChangedHint->GetDesignMode();
        if (!bDesignMode && GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(nullptr);
            LeaveDrawCreate();
            AttrChangedNotify(nullptr);
        }
    }
    else
    {
        SfxHintId nId = rHint.GetId();

        switch ( nId )
        {
            // sub shells will be destroyed by the
            // dispatcher, if the view frame is dying. Thus, reset member <pShell>.
            case SfxHintId::Dying:
                {
                    if ( &rBC == GetViewFrame() )
                    {
                        ResetSubShell();
                    }
                }
                break;
            case SfxHintId::ModeChanged:
                {
                    // Modal mode change-over?
                    bool bModal = GetDocShell()->IsInModalMode();
                    m_pHRuler->SetActive( !bModal );
                    m_pVRuler->SetActive( !bModal );
                }

                [[fallthrough]];

            case SfxHintId::TitleChanged:
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

            case SfxHintId::SwDrawViewsCreated:
                {
                    bCallBase = false;
                    if ( GetFormShell() )
                    {
                        GetFormShell()->SetView(dynamic_cast<FmFormView*>(GetWrtShell().GetDrawView()));
                        SfxBoolItem aItem( SID_FM_DESIGN_MODE, !GetDocShell()->IsReadOnly());
                        GetDispatcher().ExecuteList(SID_FM_DESIGN_MODE,
                                SfxCallMode::SYNCHRON, { &aItem });
                    }
                }
                break;
            case SfxHintId::RedlineChanged:
                {
                    static sal_uInt16 const aSlotRedLine[] = {
                        FN_REDLINE_ACCEPT_DIRECT,
                        FN_REDLINE_REJECT_DIRECT,
                        FN_REDLINE_NEXT_CHANGE,
                        FN_REDLINE_PREV_CHANGE,
                        FN_REDLINE_ACCEPT_ALL,
                        FN_REDLINE_REJECT_ALL,
                        0
                    };
                    GetViewFrame()->GetBindings().Invalidate(aSlotRedLine);
                }
                break;
            default: break;
        }
    }

    if ( bCallBase )
        SfxViewShell::Notify(rBC, rHint);
}

#if defined(_WIN32) || defined UNX

void SwView::ScannerEventHdl()
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

                if( !aScanBmp.IsEmpty() )
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

bool SwView::IsPasteSpreadsheet(bool bHasOwnTableCopied)
{
    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        &GetEditWin()) );
    if( aDataHelper.GetXTransferable().is() )
    {
        if (bHasOwnTableCopied && SwTransferable::IsPasteOwnFormat( aDataHelper ))
            return true;
        return aDataHelper.HasFormat( SotClipboardFormatId::SYLK ) || aDataHelper.HasFormat( SotClipboardFormatId::SYLK_BIGCAPS );
    }
    return false;
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

tools::Rectangle SwView::getLOKVisibleArea() const
{
    SwViewShell* pVwSh = GetWrtShellPtr();
    if (pVwSh)
        return pVwSh->getLOKVisibleArea();
    else
        return tools::Rectangle();
}

void SwView::flushPendingLOKInvalidateTiles()
{
    SwWrtShell* pSh = GetWrtShellPtr();
    assert(pSh);
    pSh->FlushPendingLOKInvalidateTiles();
}

OString SwView::getLOKPayload(int nType, int nViewId, bool* ignore) const
{
    SwWrtShell* pSh = GetWrtShellPtr();
    assert(pSh);
    return pSh->getLOKPayload(nType, nViewId, ignore);
}

OUString SwView::GetDataSourceName() const
{
    uno::Reference<lang::XMultiServiceFactory> xFactory(GetDocShell()->GetModel(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    OUString sDataSourceName = "";
    xSettings->getPropertyValue("CurrentDatabaseDataSource") >>= sDataSourceName;

    return sDataSourceName;
}

bool SwView::IsDataSourceAvailable(const OUString sDataSourceName)
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< XDatabaseContext> xDatabaseContext = DatabaseContext::create(xContext);

    return xDatabaseContext->hasByName(sDataSourceName);
}

namespace sw {

void InitPrintOptionsFromApplication(SwPrintData & o_rData, bool const bWeb)
{
    o_rData = *SW_MOD()->GetPrtOptions(bWeb);
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
