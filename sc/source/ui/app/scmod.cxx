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

#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <scitems.hxx>
#include <sfx2/app.hxx>

#include <editeng/flditem.hxx>
#include <editeng/outliner.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#include <IAnyRefDialog.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <editeng/langitem.hxx>
#include <svtools/colorcfg.hxx>

#include <svl/whiter.hxx>
#include <svx/dialogs.hrc>
#include <svl/inethist.hxx>
#include <vcl/waitobj.hxx>
#include <svx/svxerr.hxx>
#include <tools/diagnose_ex.h>

#include <editeng/unolingu.hxx>
#include <unotools/lingucfg.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <ooo/vba/XSinkCaller.hpp>

#include <scmod.hxx>
#include <global.hxx>
#include <viewopti.hxx>
#include <docoptio.hxx>
#include <appoptio.hxx>
#include <defaultsoptions.hxx>
#include <formulaopt.hxx>
#include <inputopt.hxx>
#include <printopt.hxx>
#include <navicfg.hxx>
#include <addincfg.hxx>
#include <tabvwsh.hxx>
#include <prevwsh.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <uiitems.hxx>
#include <sc.hrc>
#include <scerrors.hrc>
#include <scstyles.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <bitmaps.hlst>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <msgpool.hxx>
#include <formdata.hxx>
#include <detfunc.hxx>
#include <preview.hxx>
#include <dragdata.hxx>
#include <markdata.hxx>
#include <transobj.hxx>

#define ShellClass_ScModule
#include <scslots.hxx>

#include <scabstdlg.hxx>
#include <formula/errorcodes.hxx>
#include <documentlinkmgr.hxx>

#define SC_IDLE_MIN     150
#define SC_IDLE_MAX     3000
#define SC_IDLE_STEP    75
#define SC_IDLE_COUNT   50

static sal_uInt16 nIdleCount = 0;

SFX_IMPL_INTERFACE(ScModule, SfxShell)

void ScModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Client | SfxVisibilityFlags::Viewer,
                                            ToolbarId::Objectbar_App);

    GetStaticInterface()->RegisterStatusBar(StatusBarId::CalcStatusBar);
}

ScModule::ScModule( SfxObjectFactory* pFact ) :
    SfxModule("sc", {pFact}),
    m_aIdleTimer("sc ScModule IdleTimer"),
    m_aSpellIdle("sc ScModule SpellIdle"),
    m_pDragData(new ScDragData),
    m_pSelTransfer( nullptr ),
    m_pMessagePool( nullptr ),
    m_pRefInputHandler( nullptr ),
    m_nCurRefDlgId( 0 ),
    m_bIsWaterCan( false ),
    m_bIsInEditCommand( false ),
    m_bIsInExecuteDrop( false ),
    m_bIsInSharedDocLoading( false ),
    m_bIsInSharedDocSaving( false )
{
    // The ResManager (DLL data) is not yet initialized in the ctor!
    SetName("StarCalc"); // for Basic

    ResetDragObject();

    // InputHandler does not need to be created

    // Create ErrorHandler - was in Init()
    // Between OfficeApplication::Init and ScGlobal::Init
    SvxErrorHandler::ensure();
    m_pErrorHdl.reset( new SfxErrorHandler(RID_ERRHDLSC,
                                       ErrCodeArea::Sc,
                                       ErrCodeArea::Sc,
                                       GetResLocale()) );

    m_aSpellIdle.SetInvokeHandler( LINK( this, ScModule, SpellTimerHdl ) );

    m_aIdleTimer.SetTimeout(SC_IDLE_MIN);
    m_aIdleTimer.SetInvokeHandler( LINK( this, ScModule, IdleHandler ) );
    m_aIdleTimer.Start();

    m_pMessagePool = new ScMessagePool;
    m_pMessagePool->FreezeIdRanges();
    SetPool( m_pMessagePool );
    ScGlobal::InitTextHeight( m_pMessagePool );

    StartListening( *SfxGetpApp() );       // for SfxHintId::Deinitializing
}

ScModule::~ScModule()
{
    OSL_ENSURE( !m_pSelTransfer, "Selection Transfer object not deleted" );

    // InputHandler does not need to be deleted (there's none in the App anymore)

    SfxItemPool::Free(m_pMessagePool);

    m_pFormEditData.reset();

    m_pDragData.reset();
    m_pErrorHdl.reset();

    ScGlobal::Clear(); // Also calls ScDocumentPool::DeleteVersionMaps();

    DeleteCfg(); // Called from Exit()
}

void ScModule::ConfigurationChanged( utl::ConfigurationBroadcaster* p, ConfigurationHints )
{
    if ( p == m_pColorConfig.get() || p == m_pAccessOptions.get() )
    {
        // Test if detective objects have to be updated with new colors
        // (if the detective colors haven't been used yet, there's nothing to update)
        if ( ScDetectiveFunc::IsColorsInitialized() )
        {
            const svtools::ColorConfig& rColors = GetColorConfig();
            bool bArrows =
                ( ScDetectiveFunc::GetArrowColor() != rColors.GetColorValue(svtools::CALCDETECTIVE).nColor ||
                  ScDetectiveFunc::GetErrorColor() != rColors.GetColorValue(svtools::CALCDETECTIVEERROR).nColor );
            bool bComments =
                ( ScDetectiveFunc::GetCommentColor() != rColors.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor );
            if ( bArrows || bComments )
            {
                ScDetectiveFunc::InitializeColors(); // get the new colors

                // update detective objects in all open documents
                SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
                while ( pObjSh )
                {
                    if ( dynamic_cast<const ScDocShell * >(pObjSh) != nullptr )
                    {
                        ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);
                        if ( bArrows )
                            ScDetectiveFunc( &pDocSh->GetDocument(), 0 ).UpdateAllArrowColors();
                        if ( bComments )
                            ScDetectiveFunc::UpdateAllComments( pDocSh->GetDocument() );
                    }
                    pObjSh = SfxObjectShell::GetNext( *pObjSh );
                }
            }
        }

        // force all views to repaint, using the new options
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while(pViewShell)
        {
            if (ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(pViewShell))
            {
                pViewSh->PaintGrid();
                pViewSh->PaintTop();
                pViewSh->PaintLeft();
                pViewSh->PaintExtras();

                ScInputHandler* pHdl = pViewSh->GetInputHandler();
                if ( pHdl )
                    pHdl->ForgetLastPattern(); // EditEngine BackgroundColor may change
            }
            else if ( dynamic_cast<const ScPreviewShell*>( pViewShell) !=  nullptr )
            {
                vcl::Window* pWin = pViewShell->GetWindow();
                if (pWin)
                    pWin->Invalidate();
            }
            pViewShell = SfxViewShell::GetNext( *pViewShell );
        }
    }
    else if ( p == m_pCTLOptions.get() )
    {
        // for all documents: set digit language for printer, recalc output factor, update row heights
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
        while ( pObjSh )
        {
            if ( dynamic_cast<const ScDocShell *>(pObjSh) != nullptr )
            {
                ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);
                OutputDevice* pPrinter = pDocSh->GetPrinter();
                if ( pPrinter )
                    pPrinter->SetDigitLanguage( GetOptDigitLanguage() );

                pDocSh->CalcOutputFactor();

                SCTAB nTabCount = pDocSh->GetDocument().GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    pDocSh->AdjustRowHeight( 0, MAXROW, nTab );
            }
            pObjSh = SfxObjectShell::GetNext( *pObjSh );
        }

        // for all views (table and preview): update digit language
        SfxViewShell* pSh = SfxViewShell::GetFirst();
        while ( pSh )
        {
            if (ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(pSh))
            {
                // set ref-device for EditEngine (re-evaluates digit settings)
                ScInputHandler* pHdl = GetInputHdl(pViewSh);
                if (pHdl)
                    pHdl->UpdateRefDevice();

                pViewSh->DigitLanguageChanged();
                pViewSh->PaintGrid();
            }
            else if (ScPreviewShell* pPreviewSh = dynamic_cast<ScPreviewShell*>(pSh))
            {
                ScPreview* pPreview = pPreviewSh->GetPreview();

                pPreview->SetDigitLanguage( GetOptDigitLanguage() );
                pPreview->Invalidate();
            }

            pSh = SfxViewShell::GetNext( *pSh );
        }
    }
}

void ScModule::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Deinitializing )
    {
        // ConfigItems must be removed before ConfigManager
        DeleteCfg();
    }
}

void ScModule::DeleteCfg()
{
    m_pViewCfg.reset(); // Saving happens automatically before Exit()
    m_pDocCfg.reset();
    m_pAppCfg.reset();
    m_pDefaultsCfg.reset();
    m_pFormulaCfg.reset();
    m_pInputCfg.reset();
    m_pPrintCfg.reset();
    m_pNavipiCfg.reset();
    m_pAddInCfg.reset();

    if ( m_pColorConfig )
    {
        m_pColorConfig->RemoveListener(this);
        m_pColorConfig.reset();
    }
    if ( m_pAccessOptions )
    {
        m_pAccessOptions->RemoveListener(this);
        m_pAccessOptions.reset();
    }
    if ( m_pCTLOptions )
    {
        m_pCTLOptions->RemoveListener(this);
        m_pCTLOptions.reset();
    }
    m_pUserOptions.reset();
}

// Moved here from the App

void ScModule::Execute( SfxRequest& rReq )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : nullptr;

    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    switch ( nSlot )
    {
        case SID_CHOOSE_DESIGN:
            SfxApplication::CallAppBasic( "Template.Samples.ShowStyles" );
            break;
        case SID_EURO_CONVERTER:
            SfxApplication::CallAppBasic( "Euro.ConvertRun.Main" );
            break;
        case SID_AUTOSPELL_CHECK:
            {
                bool bSet;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                {   // Toggle
                    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
                    if ( pDocSh )
                        bSet = !pDocSh->GetDocument().GetDocOptions().IsAutoSpell();
                    else
                        bSet = !GetDocOptions().IsAutoSpell();
                }

                SfxItemSet aSet( GetPool(), svl::Items<SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK>{} );
                aSet.Put( SfxBoolItem( SID_AUTOSPELL_CHECK, bSet ) );
                ModifyOptions( aSet );
                rReq.Done();
            }
            break;

        case SID_ATTR_METRIC:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                {
                    FieldUnit eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
                    switch( eUnit )
                    {
                        case FieldUnit::MM:      // Just the units that are also in the dialog
                        case FieldUnit::CM:
                        case FieldUnit::INCH:
                        case FieldUnit::PICA:
                        case FieldUnit::POINT:
                            {
                                PutItem( *pItem );
                                ScAppOptions aNewOpts( GetAppOptions() );
                                aNewOpts.SetAppMetric( eUnit );
                                SetAppOptions( aNewOpts );
                                rReq.Done();
                            }
                            break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
            break;

        case FID_AUTOCOMPLETE:
            {
                ScAppOptions aNewOpts( GetAppOptions() );
                bool bNew = !aNewOpts.GetAutoComplete();
                aNewOpts.SetAutoComplete( bNew );
                SetAppOptions( aNewOpts );
                ScInputHandler::SetAutoComplete( bNew );
                if (pBindings)
                    pBindings->Invalidate( FID_AUTOCOMPLETE );
                rReq.Done();
            }
            break;

        case SID_DETECTIVE_AUTO:
            {
                ScAppOptions aNewOpts( GetAppOptions() );
                bool bNew = !aNewOpts.GetDetectiveAuto();
                const SfxBoolItem* pAuto = rReq.GetArg<SfxBoolItem>(SID_DETECTIVE_AUTO);
                if ( pAuto )
                    bNew = pAuto->GetValue();

                aNewOpts.SetDetectiveAuto( bNew );
                SetAppOptions( aNewOpts );
                if (pBindings)
                    pBindings->Invalidate( SID_DETECTIVE_AUTO );
                rReq.AppendItem( SfxBoolItem( SID_DETECTIVE_AUTO, bNew ) );
                rReq.Done();
            }
            break;

        case SID_PSZ_FUNCTION:
            if (pReqArgs)
            {
                auto const & p = pReqArgs->Get(SID_PSZ_FUNCTION);
                OSL_ENSURE(dynamic_cast<const SfxUInt32Item*>(&p) !=  nullptr,"wrong Parameter");
                const SfxUInt32Item& rItem = static_cast<const SfxUInt32Item&>(p);

                ScAppOptions aNewOpts( GetAppOptions() );
                aNewOpts.SetStatusFunc( rItem.GetValue() );
                SetAppOptions( aNewOpts );

                if (pBindings)
                {
                    pBindings->Invalidate( SID_TABLE_CELL );
                    pBindings->Update( SID_TABLE_CELL ); // Immediately

                    pBindings->Invalidate( SID_PSZ_FUNCTION );
                    pBindings->Update( SID_PSZ_FUNCTION );
                    // If the menu is opened again immediately
                }
            }
            break;

        case SID_ATTR_LANGUAGE:
        case SID_ATTR_CHAR_CJK_LANGUAGE:
        case SID_ATTR_CHAR_CTL_LANGUAGE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( GetPool().GetWhich(nSlot), true, &pItem ) )
                {
                    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
                    if ( pDocSh )
                    {
                        ScDocument& rDoc = pDocSh->GetDocument();
                        LanguageType eNewLang = static_cast<const SvxLanguageItem*>(pItem)->GetLanguage();
                        LanguageType eLatin, eCjk, eCtl;
                        rDoc.GetLanguage( eLatin, eCjk, eCtl );
                        LanguageType eOld = ( nSlot == SID_ATTR_CHAR_CJK_LANGUAGE ) ? eCjk :
                                            ( ( nSlot == SID_ATTR_CHAR_CTL_LANGUAGE ) ? eCtl : eLatin );
                        if ( eNewLang != eOld )
                        {
                            if ( nSlot == SID_ATTR_CHAR_CJK_LANGUAGE )
                                eCjk = eNewLang;
                            else if ( nSlot == SID_ATTR_CHAR_CTL_LANGUAGE )
                                eCtl = eNewLang;
                            else
                                eLatin = eNewLang;

                            rDoc.SetLanguage( eLatin, eCjk, eCtl );

                            ScInputHandler* pInputHandler = GetInputHdl();
                            if ( pInputHandler )
                                pInputHandler->UpdateSpellSettings(); // EditEngine flags
                            ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
                            if ( pViewSh )
                                pViewSh->UpdateDrawTextOutliner(); // EditEngine flags

                            pDocSh->SetDocumentModified();
                        }
                    }
                }
            }
            break;

        case FID_FOCUS_POSWND:
            {
                ScInputHandler* pHdl = GetInputHdl();
                if (pHdl)
                {
                    ScInputWindow* pWin = pHdl->GetInputWindow();
                    if (pWin)
                        pWin->PosGrabFocus();
                }
                rReq.Done();
            }
            break;

        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                css::uno::Reference < css::ui::dialogs::XExecutableDialog > xDialog = css::ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext());
                xDialog->execute();
            }
            catch( css::uno::RuntimeException& )
            {
                DBG_UNHANDLED_EXCEPTION("sc.ui");
            }
        }
        break;

        default:
            OSL_FAIL( "ScApplication: Unknown Message." );
            break;
    }
}

void ScModule::GetState( SfxItemSet& rSet )
{
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
    bool bTabView = pDocSh && (pDocSh->GetBestViewShell() != nullptr);

    SfxWhichIter aIter(rSet);
    for (sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich())
    {
        if (!bTabView)
        {
            // Not in the normal calc view shell (most likely in preview shell). Disable all actions.
            rSet.DisableItem(nWhich);
            continue;
        }

        switch ( nWhich )
        {
            case FID_AUTOCOMPLETE:
                rSet.Put( SfxBoolItem( nWhich, GetAppOptions().GetAutoComplete() ) );
                break;
            case SID_DETECTIVE_AUTO:
                rSet.Put( SfxBoolItem( nWhich, GetAppOptions().GetDetectiveAuto() ) );
                break;
            case SID_PSZ_FUNCTION:
                rSet.Put( SfxUInt32Item( nWhich, GetAppOptions().GetStatusFunc() ) );
                break;
            case SID_ATTR_METRIC:
                rSet.Put( SfxUInt16Item( nWhich, sal::static_int_cast<sal_uInt16>(GetAppOptions().GetAppMetric()) ) );
                break;
            case SID_AUTOSPELL_CHECK:
                rSet.Put( SfxBoolItem( nWhich, pDocSh->GetDocument().GetDocOptions().IsAutoSpell()) );
                break;
            case SID_ATTR_LANGUAGE:
            case ATTR_CJK_FONT_LANGUAGE:        // WID for SID_ATTR_CHAR_CJK_LANGUAGE
            case ATTR_CTL_FONT_LANGUAGE:        // WID for SID_ATTR_CHAR_CTL_LANGUAGE
                {
                    LanguageType eLatin, eCjk, eCtl;
                    pDocSh->GetDocument().GetLanguage( eLatin, eCjk, eCtl );
                    LanguageType eLang = ( nWhich == ATTR_CJK_FONT_LANGUAGE ) ? eCjk :
                                        ( ( nWhich == ATTR_CTL_FONT_LANGUAGE ) ? eCtl : eLatin );
                    rSet.Put( SvxLanguageItem( eLang, nWhich ) );
                }
                break;
        }
    }
}

void ScModule::HideDisabledSlots( SfxItemSet& rSet )
{
    if( SfxViewFrame* pViewFrm = SfxViewFrame::Current() )
    {
        SfxBindings& rBindings = pViewFrm->GetBindings();
        SfxWhichIter aIter( rSet );
        for( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich != 0; nWhich = aIter.NextWhich() )
        {
            ScViewUtil::HideDisabledSlot( rSet, rBindings, nWhich );
            // always disable the slots
            rSet.DisableItem( nWhich );
        }
    }
}

void ScModule::ResetDragObject()
{
    m_pDragData->pCellTransfer = nullptr;
    m_pDragData->pDrawTransfer = nullptr;
    m_pDragData->pJumpLocalDoc = nullptr;
    m_pDragData->aLinkDoc.clear();
    m_pDragData->aLinkTable.clear();
    m_pDragData->aLinkArea.clear();
    m_pDragData->aJumpTarget.clear();
    m_pDragData->aJumpText.clear();
}

void ScModule::SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    ResetDragObject();
    m_pDragData->pCellTransfer = pCellObj;
    m_pDragData->pDrawTransfer = pDrawObj;
}

void ScModule::SetDragLink(
    const OUString& rDoc, const OUString& rTab, const OUString& rArea )
{
    ResetDragObject();
    m_pDragData->aLinkDoc   = rDoc;
    m_pDragData->aLinkTable = rTab;
    m_pDragData->aLinkArea  = rArea;
}

void ScModule::SetDragJump(
    ScDocument* pLocalDoc, const OUString& rTarget, const OUString& rText )
{
    ResetDragObject();

    m_pDragData->pJumpLocalDoc = pLocalDoc;
    m_pDragData->aJumpTarget = rTarget;
    m_pDragData->aJumpText = rText;
}

ScDocument* ScModule::GetClipDoc()
{
    // called from document
    SfxViewFrame* pViewFrame = nullptr;
    ScTabViewShell* pViewShell = nullptr;
    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable;

    if ((pViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current())))
        xTransferable.set(ScTabViewShell::GetClipData(pViewShell->GetViewData().GetActiveWin()));
    else if ((pViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::GetFirst())))
        xTransferable.set(ScTabViewShell::GetClipData(pViewShell->GetViewData().GetActiveWin()));
    else if ((pViewFrame = SfxViewFrame::GetFirst()))
    {
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> xClipboard =
            pViewFrame->GetWindow().GetClipboard();
        xTransferable.set(xClipboard.is() ? xClipboard->getContents() : nullptr, css::uno::UNO_QUERY);
    }

    const ScTransferObj* pObj = ScTransferObj::GetOwnClipboard(xTransferable);
    if (pObj)
    {
        ScDocument* pDoc = pObj->GetDocument();
        assert((!pDoc || pDoc->IsClipboard()) && "Document is not clipboard, how can that be?");
        return pDoc;
    }

    return nullptr;
}

void ScModule::SetSelectionTransfer( ScSelectionTransferObj* pNew )
{
    m_pSelTransfer = pNew;
}

void ScModule::InitFormEditData()
{
    m_pFormEditData.reset( new ScFormEditData );
}

void ScModule::ClearFormEditData()
{
    m_pFormEditData.reset();
}

void ScModule::SetViewOptions( const ScViewOptions& rOpt )
{
    if ( !m_pViewCfg )
        m_pViewCfg.reset(new ScViewCfg);

    m_pViewCfg->SetOptions( rOpt );
}

const ScViewOptions& ScModule::GetViewOptions()
{
    if ( !m_pViewCfg )
        m_pViewCfg.reset( new ScViewCfg );

    return *m_pViewCfg;
}

void ScModule::SetDocOptions( const ScDocOptions& rOpt )
{
    if ( !m_pDocCfg )
        m_pDocCfg.reset( new ScDocCfg );

    m_pDocCfg->SetOptions( rOpt );
}

const ScDocOptions& ScModule::GetDocOptions()
{
    if ( !m_pDocCfg )
        m_pDocCfg.reset( new ScDocCfg );

    return *m_pDocCfg;
}

#ifndef LRU_MAX
#define LRU_MAX 10
#endif

void ScModule::InsertEntryToLRUList(sal_uInt16 nFIndex)
{
    if(nFIndex != 0)
    {
        const ScAppOptions& rAppOpt = GetAppOptions();
        sal_uInt16 nLRUFuncCount = std::min( rAppOpt.GetLRUFuncListCount(), sal_uInt16(LRU_MAX) );
        sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();

        sal_uInt16  aIdxList[LRU_MAX];
        sal_uInt16  n = 0;
        bool    bFound = false;

        while ((n < LRU_MAX) && n<nLRUFuncCount)                        // Iterate through old list
        {
            if (!bFound && (pLRUListIds[n]== nFIndex))
                bFound = true;                                          // First hit!
            else if (bFound)
                aIdxList[n  ] = pLRUListIds[n];                         // Copy after hit
            else if ((n+1) < LRU_MAX)
                aIdxList[n+1] = pLRUListIds[n];                         // Move before hit
            n++;
        }
        if (!bFound && (n < LRU_MAX))                                   // Entry not found?
            n++;                                                        // One more
        aIdxList[0] = nFIndex;                                          // Current on Top

        ScAppOptions aNewOpts(rAppOpt);                                 // Let App know
        aNewOpts.SetLRUFuncList(aIdxList, n);
        SetAppOptions(aNewOpts);
    }
}

void ScModule::SetAppOptions( const ScAppOptions& rOpt )
{
    if ( !m_pAppCfg )
        m_pAppCfg.reset( new ScAppCfg );

    m_pAppCfg->SetOptions( rOpt );
}

void global_InitAppOptions()
{
    SC_MOD()->GetAppOptions();
}

const ScAppOptions& ScModule::GetAppOptions()
{
    if ( !m_pAppCfg )
        m_pAppCfg.reset( new ScAppCfg );

    return *m_pAppCfg;
}

void ScModule::SetDefaultsOptions( const ScDefaultsOptions& rOpt )
{
    if ( !m_pDefaultsCfg )
        m_pDefaultsCfg.reset( new ScDefaultsCfg );

    m_pDefaultsCfg->SetOptions( rOpt );
}

const ScDefaultsOptions& ScModule::GetDefaultsOptions()
{
    if ( !m_pDefaultsCfg )
        m_pDefaultsCfg.reset( new ScDefaultsCfg );

    return *m_pDefaultsCfg;
}

void ScModule::SetFormulaOptions( const ScFormulaOptions& rOpt )
{
    if ( !m_pFormulaCfg )
        m_pFormulaCfg.reset( new ScFormulaCfg );

    m_pFormulaCfg->SetOptions( rOpt );
}

const ScFormulaOptions& ScModule::GetFormulaOptions()
{
    if ( !m_pFormulaCfg )
        m_pFormulaCfg.reset( new ScFormulaCfg );

    return *m_pFormulaCfg;
}

void ScModule::SetInputOptions( const ScInputOptions& rOpt )
{
    if ( !m_pInputCfg )
        m_pInputCfg.reset( new ScInputCfg );

    m_pInputCfg->SetOptions( rOpt );
}

const ScInputOptions& ScModule::GetInputOptions()
{
    if ( !m_pInputCfg )
        m_pInputCfg.reset( new ScInputCfg );

    return *m_pInputCfg;
}

void ScModule::SetPrintOptions( const ScPrintOptions& rOpt )
{
    if ( !m_pPrintCfg )
        m_pPrintCfg.reset( new ScPrintCfg );

    m_pPrintCfg->SetOptions( rOpt );
}

const ScPrintOptions& ScModule::GetPrintOptions()
{
    if ( !m_pPrintCfg )
        m_pPrintCfg.reset( new ScPrintCfg );

    return *m_pPrintCfg;
}

ScNavipiCfg& ScModule::GetNavipiCfg()
{
    if ( !m_pNavipiCfg )
        m_pNavipiCfg.reset( new ScNavipiCfg );

    return *m_pNavipiCfg;
}

ScAddInCfg& ScModule::GetAddInCfg()
{
    if ( !m_pAddInCfg )
        m_pAddInCfg.reset( new ScAddInCfg );

    return *m_pAddInCfg;
}

svtools::ColorConfig& ScModule::GetColorConfig()
{
    if ( !m_pColorConfig )
    {
        m_pColorConfig.reset( new svtools::ColorConfig );
        m_pColorConfig->AddListener(this);
    }

    return *m_pColorConfig;
}

SvtAccessibilityOptions& ScModule::GetAccessOptions()
{
    if ( !m_pAccessOptions )
    {
        m_pAccessOptions.reset( new SvtAccessibilityOptions );
        m_pAccessOptions->AddListener(this);
    }

    return *m_pAccessOptions;
}

SvtCTLOptions& ScModule::GetCTLOptions()
{
    if ( !m_pCTLOptions )
    {
        m_pCTLOptions.reset( new SvtCTLOptions );
        m_pCTLOptions->AddListener(this);
    }

    return *m_pCTLOptions;
}

SvtUserOptions&  ScModule::GetUserOptions()
{
    if( !m_pUserOptions )
    {
        m_pUserOptions.reset( new SvtUserOptions );
    }
    return *m_pUserOptions;
}

LanguageType ScModule::GetOptDigitLanguage()
{
    SvtCTLOptions::TextNumerals eNumerals = GetCTLOptions().GetCTLTextNumerals();
    return ( eNumerals == SvtCTLOptions::NUMERALS_ARABIC ) ? LANGUAGE_ENGLISH_US :
           ( eNumerals == SvtCTLOptions::NUMERALS_HINDI)   ? LANGUAGE_ARABIC_SAUDI_ARABIA :
                                                             LANGUAGE_SYSTEM;
}

/**
 * Options
 *
 * Items from Calc options dialog and SID_AUTOSPELL_CHECK
 */
void ScModule::ModifyOptions( const SfxItemSet& rOptSet )
{
    LanguageType nOldSpellLang, nOldCjkLang, nOldCtlLang;
    bool bOldAutoSpell;
    GetSpellSettings( nOldSpellLang, nOldCjkLang, nOldCtlLang, bOldAutoSpell );

    if (!m_pAppCfg)
        GetAppOptions();
    OSL_ENSURE( m_pAppCfg, "AppOptions not initialised :-(" );

    if (!m_pInputCfg)
        GetInputOptions();
    OSL_ENSURE( m_pInputCfg, "InputOptions not initialised :-(" );

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : nullptr;

    ScTabViewShell*         pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
    ScDocShell*             pDocSh  = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
    ScDocument*             pDoc    = pDocSh ? &pDocSh->GetDocument() : nullptr;
    const SfxPoolItem*      pItem   = nullptr;
    bool bRepaint = false;
    bool bUpdateMarks = false;
    bool bUpdateRefDev = false;
    bool bCalcAll = false;
    bool bSaveAppOptions = false;
    bool bSaveInputOptions = false;
    bool bCompileErrorCells = false;

    //  SfxGetpApp()->SetOptions( rOptSet );

    // No more linguistics
    if (rOptSet.HasItem(SID_ATTR_METRIC, &pItem))
    {
        PutItem( *pItem );
        m_pAppCfg->SetAppMetric( static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue()) );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SCITEM_USERLIST, &pItem))
    {
        ScGlobal::SetUserList( static_cast<const ScUserListItem*>(pItem)->GetUserList() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SID_SC_OPT_SYNCZOOM, &pItem))
    {
        m_pAppCfg->SetSynchronizeZoom( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SID_SC_OPT_KEY_BINDING_COMPAT, &pItem))
    {
        sal_uInt16 nVal = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        ScOptionsUtil::KeyBindingType eOld = m_pAppCfg->GetKeyBindingType();
        ScOptionsUtil::KeyBindingType eNew = static_cast<ScOptionsUtil::KeyBindingType>(nVal);
        if (eOld != eNew)
        {
            m_pAppCfg->SetKeyBindingType(eNew);
            bSaveAppOptions = true;
            ScDocShell::ResetKeyBindings(eNew);
        }
    }

    // DefaultsOptions
    if (rOptSet.HasItem(SID_SCDEFAULTSOPTIONS, &pItem))
    {
        const ScDefaultsOptions& rOpt = static_cast<const ScTpDefaultsItem*>(pItem)->GetDefaultsOptions();
        SetDefaultsOptions( rOpt );
    }

    // FormulaOptions
    if (rOptSet.HasItem(SID_SCFORMULAOPTIONS, &pItem))
    {
        const ScFormulaOptions& rOpt = static_cast<const ScTpFormulaItem*>(pItem)->GetFormulaOptions();

        if (!m_pFormulaCfg || (*m_pFormulaCfg != rOpt))
            // Formula options have changed. Repaint the column headers.
            bRepaint = true;

        if (m_pFormulaCfg && m_pFormulaCfg->GetUseEnglishFuncName() != rOpt.GetUseEnglishFuncName())
        {
            // Re-compile formula cells with error as the error may have been
            // caused by unresolved function names.
            bCompileErrorCells = true;
        }

        // Recalc for interpreter options changes.
        if (m_pFormulaCfg && m_pFormulaCfg->GetCalcConfig() != rOpt.GetCalcConfig())
            bCalcAll = true;

        if ( pDocSh )
        {
            pDocSh->SetFormulaOptions( rOpt );
            pDocSh->SetDocumentModified();
        }

        // ScDocShell::SetFormulaOptions() may check for changed settings, so
        // set the new options here after that has been called.
        if (!bCalcAll || rOpt.GetWriteCalcConfig())
        {
            // CalcConfig is new, didn't change or is global, simply set all.
            SetFormulaOptions( rOpt );
        }
        else
        {
            // If "only for current document" was checked, reset those affected
            // by that setting to previous values.
            ScFormulaOptions aNewOpt( rOpt);
            aNewOpt.GetCalcConfig().MergeDocumentSpecific( m_pFormulaCfg->GetCalcConfig());
            SetFormulaOptions( aNewOpt);
        }
    }

    // ViewOptions
    if (rOptSet.HasItem(SID_SCVIEWOPTIONS, &pItem))
    {
        const ScViewOptions& rNewOpt = static_cast<const ScTpViewItem*>(pItem)->GetViewOptions();

        if ( pViewSh )
        {
            ScViewData&             rViewData = pViewSh->GetViewData();
            const ScViewOptions&    rOldOpt   = rViewData.GetOptions();

            bool bAnchorList = ( rOldOpt.GetOption( VOPT_ANCHOR ) !=
                                 rNewOpt.GetOption( VOPT_ANCHOR ) );

            if ( rOldOpt != rNewOpt )
            {
                rViewData.SetOptions( rNewOpt ); // Changes rOldOpt
                rViewData.GetDocument()->SetViewOptions( rNewOpt );
                if (pDocSh)
                    pDocSh->SetDocumentModified();
                bRepaint = true;
            }
            if ( bAnchorList )
                pViewSh->UpdateAnchorHandles();
        }
        SetViewOptions( rNewOpt );
        if (pBindings)
        {
            pBindings->Invalidate(SID_HELPLINES_MOVE);
        }
    }

    // GridOptions
    // Evaluate after ViewOptions, as GridOptions is a member of ViewOptions
    if ( rOptSet.HasItem(SID_ATTR_GRID_OPTIONS,&pItem) )
    {
        ScGridOptions aNewGridOpt( static_cast<const SvxGridItem&>(*pItem ));

        if ( pViewSh )
        {
            ScViewData&          rViewData = pViewSh->GetViewData();
            ScViewOptions        aNewViewOpt( rViewData.GetOptions() );
            const ScGridOptions& rOldGridOpt = aNewViewOpt.GetGridOptions();

            if ( rOldGridOpt != aNewGridOpt )
            {
                aNewViewOpt.SetGridOptions( aNewGridOpt );
                rViewData.SetOptions( aNewViewOpt );
                rViewData.GetDocument()->SetViewOptions( aNewViewOpt );
                pDocSh->SetDocumentModified();
                bRepaint = true;
            }
        }
        ScViewOptions aNewViewOpt ( GetViewOptions() );
        aNewViewOpt.SetGridOptions( aNewGridOpt );
        SetViewOptions( aNewViewOpt );
        if (pBindings)
        {
            pBindings->Invalidate(SID_GRID_VISIBLE);
            pBindings->Invalidate(SID_GRID_USE);
        }
    }

    // DocOptions
    if ( rOptSet.HasItem(SID_SCDOCOPTIONS,&pItem) )
    {
        const ScDocOptions& rNewOpt = static_cast<const ScTpCalcItem*>(pItem)->GetDocOptions();

        if ( pDoc )
        {
            const ScDocOptions& rOldOpt = pDoc->GetDocOptions();

            bRepaint = ( bRepaint || ( rOldOpt != rNewOpt )   );
            bCalcAll =   bRepaint &&
                         (  rOldOpt.IsIter()       != rNewOpt.IsIter()
                         || rOldOpt.GetIterCount() != rNewOpt.GetIterCount()
                         || rOldOpt.GetIterEps()   != rNewOpt.GetIterEps()
                         || rOldOpt.IsIgnoreCase() != rNewOpt.IsIgnoreCase()
                         || rOldOpt.IsCalcAsShown() != rNewOpt.IsCalcAsShown()
                         || (rNewOpt.IsCalcAsShown() &&
                            rOldOpt.GetStdPrecision() != rNewOpt.GetStdPrecision())
                         || rOldOpt.IsMatchWholeCell() != rNewOpt.IsMatchWholeCell()
                         || rOldOpt.GetYear2000()   != rNewOpt.GetYear2000()
                         || rOldOpt.IsFormulaRegexEnabled() != rNewOpt.IsFormulaRegexEnabled()
                         || rOldOpt.IsFormulaWildcardsEnabled() != rNewOpt.IsFormulaWildcardsEnabled()
                         );
            pDoc->SetDocOptions( rNewOpt );
            pDocSh->SetDocumentModified();
        }
        SetDocOptions( rNewOpt );
    }

    // Set TabDistance after the actual DocOptions
    if ( rOptSet.HasItem(SID_ATTR_DEFTABSTOP,&pItem) )
    {
        sal_uInt16 nTabDist = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        ScDocOptions aOpt(GetDocOptions());
        aOpt.SetTabDistance(nTabDist);
        SetDocOptions( aOpt );

        if ( pDoc )
        {
            ScDocOptions aDocOpt(pDoc->GetDocOptions());
            aDocOpt.SetTabDistance(nTabDist);
            pDoc->SetDocOptions( aDocOpt );
            pDocSh->SetDocumentModified();
            if(pDoc->GetDrawLayer())
                pDoc->GetDrawLayer()->SetDefaultTabulator(nTabDist);
        }
    }

    // AutoSpell after the DocOptions (due to being a member)
    if ( rOptSet.HasItem(SID_AUTOSPELL_CHECK,&pItem) ) // At DocOptions
    {
        bool bDoAutoSpell = static_cast<const SfxBoolItem*>(pItem)->GetValue();

        if (pDoc)
        {
            ScDocOptions aNewOpt = pDoc->GetDocOptions();
            if ( aNewOpt.IsAutoSpell() != bDoAutoSpell )
            {
                aNewOpt.SetAutoSpell( bDoAutoSpell );
                pDoc->SetDocOptions( aNewOpt );

                if (pViewSh)
                    pViewSh->EnableAutoSpell(bDoAutoSpell);

                bRepaint = true;            // Because HideAutoSpell might be invalid
                                            //TODO: Paint all Views?
            }
        }

        if ( bOldAutoSpell != bDoAutoSpell )
            SetAutoSpellProperty( bDoAutoSpell );
        if ( pDocSh )
            pDocSh->PostPaintGridAll();                     // Due to marks
        ScInputHandler* pInputHandler = GetInputHdl();
        if ( pInputHandler )
            pInputHandler->UpdateSpellSettings();           // EditEngine flags
        if ( pViewSh )
            pViewSh->UpdateDrawTextOutliner();              // EditEngine flags

        if (pBindings)
            pBindings->Invalidate( SID_AUTOSPELL_CHECK );
    }

    // InputOptions
    if ( rOptSet.HasItem(SID_SC_INPUT_SELECTIONPOS,&pItem) )
    {
        m_pInputCfg->SetMoveDir( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_SELECTION,&pItem) )
    {
        m_pInputCfg->SetMoveSelection( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_EDITMODE,&pItem) )
    {
        m_pInputCfg->SetEnterEdit( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_FMT_EXPAND,&pItem) )
    {
        m_pInputCfg->SetExtendFormat( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_RANGEFINDER,&pItem) )
    {
        m_pInputCfg->SetRangeFinder( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_REF_EXPAND,&pItem) )
    {
        m_pInputCfg->SetExpandRefs( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if (rOptSet.HasItem(SID_SC_OPT_SORT_REF_UPDATE, &pItem))
    {
        m_pInputCfg->SetSortRefUpdate(static_cast<const SfxBoolItem*>(pItem)->GetValue());
        bSaveInputOptions = true;
    }

    if ( rOptSet.HasItem(SID_SC_INPUT_MARK_HEADER,&pItem) )
    {
        m_pInputCfg->SetMarkHeader( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
        bUpdateMarks = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_TEXTWYSIWYG,&pItem) )
    {
        bool bNew = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        if ( bNew != m_pInputCfg->GetTextWysiwyg() )
        {
            m_pInputCfg->SetTextWysiwyg( bNew );
            bSaveInputOptions = true;
            bUpdateRefDev = true;
        }
    }
    if( rOptSet.HasItem( SID_SC_INPUT_REPLCELLSWARN, &pItem ) )
    {
        m_pInputCfg->SetReplaceCellsWarn( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    if( rOptSet.HasItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, &pItem ) )
    {
        m_pInputCfg->SetLegacyCellSelection( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    // PrintOptions
    if ( rOptSet.HasItem(SID_SCPRINTOPTIONS,&pItem) )
    {
        const ScPrintOptions& rNewOpt = static_cast<const ScTpPrintItem*>(pItem)->GetPrintOptions();
        SetPrintOptions( rNewOpt );

        // broadcast causes all previews to recalc page numbers
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScPrintOptions ) );
    }

    if ( bSaveAppOptions )
        m_pAppCfg->OptionsChanged();

    if ( bSaveInputOptions )
        m_pInputCfg->OptionsChanged();

    // Kick off recalculation?
    if (pDoc && bCompileErrorCells)
    {
        // Re-compile cells with name error, and recalc if at least one cell
        // has been re-compiled.  In the future we may want to find a way to
        // recalc only those that are affected.
        if (pDoc->CompileErrorCells(FormulaError::NoName))
            bCalcAll = true;
    }

    if ( pDoc && bCalcAll )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        pDoc->CalcAll();
        if ( pViewSh )
            pViewSh->UpdateCharts( true );
        else
            ScDBFunc::DoUpdateCharts( ScAddress(), pDoc, true );
        if (pBindings)
            pBindings->Invalidate( SID_ATTR_SIZE ); //SvxPosSize StatusControl Update
    }

    if ( pViewSh && bUpdateMarks )
        pViewSh->UpdateAutoFillMark();

    // Repaint View?
    if ( pViewSh && bRepaint )
    {
        pViewSh->UpdateFixPos();
        pViewSh->PaintGrid();
        pViewSh->PaintTop();
        pViewSh->PaintLeft();
        pViewSh->PaintExtras();
        pViewSh->InvalidateBorder();
        if (pBindings)
        {
            pBindings->Invalidate( FID_TOGGLEHEADERS ); // -> Checks in menu
            pBindings->Invalidate( FID_TOGGLESYNTAX );
        }
    }

    // update ref device (for all documents)
    if ( bUpdateRefDev )
    {
        // for all documents: recalc output factor, update row heights
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
        while ( pObjSh )
        {
            if ( dynamic_cast<const ScDocShell *>(pObjSh) != nullptr )
            {
                ScDocShell* pOneDocSh = static_cast<ScDocShell*>(pObjSh);
                pOneDocSh->CalcOutputFactor();
                SCTAB nTabCount = pOneDocSh->GetDocument().GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    pOneDocSh->AdjustRowHeight( 0, MAXROW, nTab );
            }
            pObjSh = SfxObjectShell::GetNext( *pObjSh );
        }

        // for all (tab-) views:
        SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
        while ( pSh )
        {
            ScTabViewShell* pOneViewSh = static_cast<ScTabViewShell*>(pSh);

            // set ref-device for EditEngine
            ScInputHandler* pHdl = GetInputHdl(pOneViewSh);
            if (pHdl)
                pHdl->UpdateRefDevice();

            // update view scale
            ScViewData& rViewData = pOneViewSh->GetViewData();
            pOneViewSh->SetZoom( rViewData.GetZoomX(), rViewData.GetZoomY(), false );

            // repaint
            pOneViewSh->PaintGrid();
            pOneViewSh->PaintTop();
            pOneViewSh->PaintLeft();

            pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
        }
    }
}

/**
 * Input-Handler
 */
ScInputHandler* ScModule::GetInputHdl( ScTabViewShell* pViewSh, bool bUseRef )
{
    if ( m_pRefInputHandler && bUseRef )
        return m_pRefInputHandler;

    ScInputHandler* pHdl = nullptr;
    if ( !pViewSh )
    {
        // in case a UIActive embedded object has no ViewShell (UNO component)
        // the own calc view shell will be set as current, but no handling should happen
        ScTabViewShell* pCurViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
        if ( pCurViewSh && !pCurViewSh->GetUIActiveClient() )
            pViewSh = pCurViewSh;
    }

    if ( pViewSh )
        pHdl = pViewSh->GetInputHandler(); // Viewshell always has one, from now on

    // If no ViewShell passed or active, we can get NULL
    OSL_ENSURE( pHdl || !pViewSh, "GetInputHdl: no InputHandler found!" );
    return pHdl;
}

void ScModule::ViewShellChanged(bool bStopEditing /*=true*/)
{
    ScInputHandler* pHdl   = GetInputHdl();
    ScTabViewShell* pShell = ScTabViewShell::GetActiveViewShell();
    if ( pShell && pHdl )
        pShell->UpdateInputHandler(false, bStopEditing);
}

void ScModule::SetInputMode( ScInputMode eMode, const OUString* pInitText )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->SetMode(eMode, pInitText);
}

bool ScModule::IsEditMode()
{
    ScInputHandler* pHdl = GetInputHdl();
    return pHdl && pHdl->IsEditMode();
}

bool ScModule::IsInputMode()
{
    ScInputHandler* pHdl = GetInputHdl();
    return pHdl && pHdl->IsInputMode();
}

bool ScModule::InputKeyEvent( const KeyEvent& rKEvt, bool bStartEdit )
{
    ScInputHandler* pHdl = GetInputHdl();
    return pHdl && pHdl->KeyInput( rKEvt, bStartEdit );
}

void ScModule::InputEnterHandler( ScEnterMode nBlockMode )
{
    if ( !SfxGetpApp()->IsDowning() ) // Not when quitting the program
    {
        ScInputHandler* pHdl = GetInputHdl();
        if (pHdl)
            pHdl->EnterHandler( nBlockMode );
    }
}

void ScModule::InputCancelHandler()
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->CancelHandler();
}

void ScModule::InputSelection( const EditView* pView )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputSelection( pView );
}

void ScModule::InputChanged( const EditView* pView )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputChanged( pView, false );
}

void ScModule::ViewShellGone( const ScTabViewShell* pViewSh )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->ViewShellGone( pViewSh );
}

void ScModule::SetRefInputHdl( ScInputHandler* pNew )
{
    m_pRefInputHandler = pNew;
}

void ScModule::InputGetSelection( sal_Int32& rStart, sal_Int32& rEnd )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputGetSelection( rStart, rEnd );
}

void ScModule::InputSetSelection( sal_Int32 nStart, sal_Int32 nEnd )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputSetSelection( nStart, nEnd );
}

void ScModule::InputReplaceSelection( const OUString& rStr )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputReplaceSelection( rStr );
}

void ScModule::InputTurnOffWinEngine()
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputTurnOffWinEngine();
}

OUString ScModule::InputGetFormulaStr()
{
    ScInputHandler* pHdl = GetInputHdl();
    OUString aStr;
    if ( pHdl )
        aStr = pHdl->GetFormString();
    return aStr;
}

void ScModule::ActivateInputWindow( const OUString* pStrFormula, bool bMatrix )
{
    ScInputHandler* pHdl = GetInputHdl();
    if ( pHdl )
    {
        ScInputWindow* pWin = pHdl->GetInputWindow();
        if ( pStrFormula )
        {
            // Take over formula
            if ( pWin )
            {
                pWin->SetFuncString( *pStrFormula, false );
                // SetSumAssignMode due to sal_False not necessary
            }
            ScEnterMode nMode = bMatrix ? ScEnterMode::MATRIX : ScEnterMode::NORMAL;
            pHdl->EnterHandler( nMode );

            // Without Invalidate the selection remains active, if the formula has not changed
            if (pWin)
                pWin->TextInvalidate();
        }
        else
        {
            // Cancel
            if ( pWin )
            {
                pWin->SetFuncString( EMPTY_OUSTRING, false );
                // SetSumAssignMode due to sal_False no necessary
            }
            pHdl->CancelHandler();
        }
    }
}

/**
 * Reference dialogs
 */
void ScModule::SetRefDialog( sal_uInt16 nId, bool bVis, SfxViewFrame* pViewFrm )
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    if(m_nCurRefDlgId==0 || (nId==m_nCurRefDlgId && !bVis))
    {
        if ( !pViewFrm )
            pViewFrm = SfxViewFrame::Current();

        // bindings update causes problems with update of stylist if
        // current style family has changed
        //if ( pViewFrm )
        //  pViewFrm->GetBindings().Update();       // to avoid trouble in LockDispatcher

        m_nCurRefDlgId = bVis ? nId : 0 ;             // before SetChildWindow

        if ( pViewFrm )
        {
            //  store the dialog id also in the view shell
            SfxViewShell* pViewSh = pViewFrm->GetViewShell();
            if (ScTabViewShell* pTabViewSh = dynamic_cast<ScTabViewShell*>(pViewSh))
                pTabViewSh->SetCurRefDlgId(m_nCurRefDlgId);
            else
            {
                // no ScTabViewShell - possible for example from a Basic macro
                bVis = false;
                m_nCurRefDlgId = 0;   // don't set nCurRefDlgId if no dialog is created
            }

            pViewFrm->SetChildWindow( nId, bVis );
        }

        SfxApplication* pSfxApp = SfxGetpApp();
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
    }
}

static SfxChildWindow* lcl_GetChildWinFromCurrentView( sal_uInt16 nId )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    // #i46999# current view frame can be null (for example, when closing help)
    return pViewFrm ? pViewFrm->GetChildWindow( nId ) : nullptr;
}

static SfxChildWindow* lcl_GetChildWinFromAnyView( sal_uInt16 nId )
{
    // First, try the current view
    SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( nId );
    if ( pChildWnd )
        return pChildWnd;           // found in the current view

    //  if not found there, get the child window from any open view
    //  it can be open only in one view because nCurRefDlgId is global

    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while ( pViewFrm )
    {
        pChildWnd = pViewFrm->GetChildWindow( nId );
        if ( pChildWnd )
            return pChildWnd;       // found in any view

        pViewFrm = SfxViewFrame::GetNext( *pViewFrm );
    }

    return nullptr;                    // none found
}

bool ScModule::IsModalMode(SfxObjectShell* pDocSh)
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsModal = false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsModal = pChildWnd->IsVisible() && pRefDlg &&
                !( pRefDlg->IsRefInputMode() && pRefDlg->IsDocAllowed(pDocSh) );
        }
    }
    else if (pDocSh)
    {
        ScInputHandler* pHdl = GetInputHdl();
        if ( pHdl )
            bIsModal = pHdl->IsModalMode(pDocSh);
    }

    return bIsModal;
}

bool ScModule::IsTableLocked()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bLocked = false;

    // Up until now just for ScAnyRefDlg
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg(dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow()));
            assert(pRefDlg);
            if(pRefDlg)
            {
                bLocked = pRefDlg->IsTableLocked();
            }
        }
        else
            bLocked = true;     // for other views, see IsModalMode
    }

    return bLocked;
}

bool ScModule::IsRefDialogOpen()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsOpen = false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
            bIsOpen = pChildWnd->IsVisible();
    }

    return bIsOpen;
}

bool ScModule::IsFormulaMode()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsFormula = false;

    // formula mode in online is not usable in collaborative mode,
    // this is a workaround for disabling formula mode in online
    // when there is more than a single view
    if (comphelper::LibreOfficeKit::isActive() && SfxViewShell::GetActiveShells() > 1)
            return false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsFormula = pChildWnd->IsVisible() && pRefDlg && pRefDlg->IsRefInputMode();
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if ( pHdl )
            bIsFormula = pHdl->IsFormulaMode();
    }

    if (m_bIsInEditCommand)
        bIsFormula = true;

    return bIsFormula;
}

static void lcl_MarkedTabs( const ScMarkData& rMark, SCTAB& rStartTab, SCTAB& rEndTab )
{
    if (rMark.GetSelectCount() > 1)
    {
        rEndTab = rMark.GetLastSelected();
        rStartTab = rMark.GetFirstSelected();
    }
}

void ScModule::SetReference( const ScRange& rRef, ScDocument* pDoc,
                                    const ScMarkData* pMarkData )
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents

    // In RefDialogs we also trigger the ZoomIn, if the Ref's Start and End are different
    ScRange aNew = rRef;
    aNew.PutInOrder(); // Always in the right direction

    if( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            if ( m_nCurRefDlgId == SID_OPENDLG_CONSOLIDATE && pMarkData )
            {
                SCTAB nStartTab = aNew.aStart.Tab();
                SCTAB nEndTab   = aNew.aEnd.Tab();
                lcl_MarkedTabs( *pMarkData, nStartTab, nEndTab );
                aNew.aStart.SetTab(nStartTab);
                aNew.aEnd.SetTab(nEndTab);
            }

            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                // hide the (color) selection now instead of later from LoseFocus,
                // don't abort the ref input that causes this call (bDoneRefMode = sal_False)
                pRefDlg->HideReference( false );
                pRefDlg->SetReference( aNew, pDoc );
            }
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if (pHdl)
            pHdl->SetReference( aNew, pDoc );
        else
        {
            OSL_FAIL("SetReference without receiver");
        }
    }
}

/**
 * Multiple selection
 */
void ScModule::AddRefEntry()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                pRefDlg->AddRefEntry();
            }
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if (pHdl)
            pHdl->AddRefEntry();
    }
}

void ScModule::EndReference()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents

    // We also annul the ZoomIn again in RefDialogs

    //FIXME: ShowRefFrame at InputHdl, if the Function AutoPilot is open?
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                pRefDlg->SetActive();
            }
        }
    }
}

/**
 * Idle/OnlineSpelling
 */
void ScModule::AnythingChanged()
{
    sal_uInt64 nOldTime = m_aIdleTimer.GetTimeout();
    if ( nOldTime != SC_IDLE_MIN )
        m_aIdleTimer.SetTimeout( SC_IDLE_MIN );

    nIdleCount = 0;
}

static void lcl_CheckNeedsRepaint( const ScDocShell* pDocShell )
{
    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
    while ( pFrame )
    {
        SfxViewShell* p = pFrame->GetViewShell();
        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
        if ( pViewSh )
            pViewSh->CheckNeedsRepaint();
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
    }
}

IMPL_LINK_NOARG(ScModule, IdleHandler, Timer *, void)
{
    if ( Application::AnyInput( VclInputFlags::MOUSE | VclInputFlags::KEYBOARD ) )
    {
        m_aIdleTimer.Start(); // Timeout unchanged
        return;
    }

    bool bMore = false;
    bool bAutoSpell = false;
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(SfxObjectShell::Current());

    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        bAutoSpell = rDoc.GetDocOptions().IsAutoSpell();
        if (pDocSh->IsReadOnly())
            bAutoSpell = false;

        sc::DocumentLinkManager& rLinkMgr = rDoc.GetDocLinkManager();
        bool bLinks = rLinkMgr.idleCheckLinks();
        bool bWidth = rDoc.IdleCalcTextWidth();

        bMore = bLinks || bWidth; // Still something at all?

        // While calculating a Basic formula, a paint event may have occurred,
        // so check the bNeedsRepaint flags for this document's views
        if (bWidth)
            lcl_CheckNeedsRepaint( pDocSh );
    }

    if (bAutoSpell)
    {
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        if (pViewSh)
        {
            bool bSpell = pViewSh->ContinueOnlineSpelling();
            if (bSpell)
            {
                m_aSpellIdle.Start();
                bMore = true;
            }
        }
    }

    sal_uInt64 nOldTime = m_aIdleTimer.GetTimeout();
    sal_uInt64 nNewTime = nOldTime;
    if ( bMore )
    {
        nNewTime = SC_IDLE_MIN;
        nIdleCount = 0;
    }
    else
    {
        // Set SC_IDLE_COUNT to initial Timeout - increase afterwards
        if ( nIdleCount < SC_IDLE_COUNT )
            ++nIdleCount;
        else
        {
            nNewTime += SC_IDLE_STEP;
            if ( nNewTime > SC_IDLE_MAX )
                nNewTime = SC_IDLE_MAX;
        }
    }
    if ( nNewTime != nOldTime )
        m_aIdleTimer.SetTimeout( nNewTime );


    m_aIdleTimer.Start();
}

IMPL_LINK_NOARG(ScModule, SpellTimerHdl, Timer *, void)
{
    if ( Application::AnyInput( VclInputFlags::KEYBOARD ) )
    {
        m_aSpellIdle.Start();
        return; // Later again ...
    }

    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    if (pViewSh)
    {
        if (pViewSh->ContinueOnlineSpelling())
            m_aSpellIdle.Start();
    }
}

/**
 * Virtual methods for the OptionsDialog
 */
std::unique_ptr<SfxItemSet> ScModule::CreateItemSet( sal_uInt16 nId )
{
    std::unique_ptr<SfxItemSet> pRet;
    if(SID_SC_EDITOPTIONS == nId)
    {
        pRet = std::make_unique<SfxItemSet>(
            GetPool(),
            svl::Items<
                // TP_USERLISTS:
                SCITEM_USERLIST, SCITEM_USERLIST,
                // TP_GRID:
                SID_ATTR_GRID_OPTIONS, SID_ATTR_GRID_OPTIONS,
                SID_ATTR_METRIC, SID_ATTR_METRIC,
                SID_ATTR_DEFTABSTOP, SID_ATTR_DEFTABSTOP,
                // TP_INPUT:
                SID_SC_INPUT_LEGACY_CELL_SELECTION, SID_SC_OPT_SORT_REF_UPDATE,
                // TP_FORMULA, TP_DEFAULTS:
                SID_SCFORMULAOPTIONS, SID_SCDEFAULTSOPTIONS,
                // TP_VIEW, TP_CALC:
                SID_SCVIEWOPTIONS, SID_SCDOCOPTIONS,
                // TP_PRINT:
                SID_SCPRINTOPTIONS, SID_SCPRINTOPTIONS,
                // TP_INPUT:
                SID_SC_INPUT_SELECTION, SID_SC_INPUT_MARK_HEADER,
                SID_SC_INPUT_TEXTWYSIWYG, SID_SC_INPUT_TEXTWYSIWYG,
                SID_SC_INPUT_REPLCELLSWARN, SID_SC_INPUT_REPLCELLSWARN,
                // TP_VIEW:
                SID_SC_OPT_SYNCZOOM, SID_SC_OPT_KEY_BINDING_COMPAT>{});

        const ScAppOptions& rAppOpt = GetAppOptions();

        ScDocShell*     pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );
        ScDocOptions    aCalcOpt = pDocSh
                            ? pDocSh->GetDocument().GetDocOptions()
                            : GetDocOptions();

        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( SfxViewShell::Current() );
        ScViewOptions   aViewOpt = pViewSh
                            ? pViewSh->GetViewData().GetOptions()
                            : GetViewOptions();

        ScUserListItem  aULItem( SCITEM_USERLIST );
        ScUserList*     pUL = ScGlobal::GetUserList();

        //  SfxGetpApp()->GetOptions( aSet );

        pRet->Put( SfxUInt16Item( SID_ATTR_METRIC,
                        sal::static_int_cast<sal_uInt16>(rAppOpt.GetAppMetric()) ) );

        // TP_CALC
        pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP,
                        aCalcOpt.GetTabDistance()));
        pRet->Put( ScTpCalcItem( SID_SCDOCOPTIONS, aCalcOpt ) );

        // TP_VIEW
        pRet->Put( ScTpViewItem( aViewOpt ) );
        pRet->Put( SfxBoolItem( SID_SC_OPT_SYNCZOOM, rAppOpt.GetSynchronizeZoom() ) );

        // TP_INPUT
        const ScInputOptions& rInpOpt = GetInputOptions();
        pRet->Put( SfxUInt16Item( SID_SC_INPUT_SELECTIONPOS,
                    rInpOpt.GetMoveDir() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_SELECTION,
                    rInpOpt.GetMoveSelection() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_EDITMODE,
                    rInpOpt.GetEnterEdit() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_FMT_EXPAND,
                    rInpOpt.GetExtendFormat() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_RANGEFINDER,
                    rInpOpt.GetRangeFinder() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_REF_EXPAND,
                    rInpOpt.GetExpandRefs() ) );
        pRet->Put( SfxBoolItem(SID_SC_OPT_SORT_REF_UPDATE, rInpOpt.GetSortRefUpdate()));
        pRet->Put( SfxBoolItem( SID_SC_INPUT_MARK_HEADER,
                    rInpOpt.GetMarkHeader() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_TEXTWYSIWYG,
                    rInpOpt.GetTextWysiwyg() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN,
                    rInpOpt.GetReplaceCellsWarn() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_LEGACY_CELL_SELECTION,
                    rInpOpt.GetLegacyCellSelection() ) );

        // RID_SC_TP_PRINT
        pRet->Put( ScTpPrintItem( GetPrintOptions() ) );

        // TP_GRID
        std::unique_ptr<SvxGridItem> pSvxGridItem = aViewOpt.CreateGridItem();
        pRet->Put( *pSvxGridItem );
        pSvxGridItem.reset();

        // TP_USERLISTS
        if ( pUL )
        {
            aULItem.SetUserList( *pUL );
            pRet->Put(aULItem);
        }

        // TP_COMPATIBILITY
        pRet->Put( SfxUInt16Item( SID_SC_OPT_KEY_BINDING_COMPAT,
                                   rAppOpt.GetKeyBindingType() ) );

        // TP_DEFAULTS
        pRet->Put( ScTpDefaultsItem( GetDefaultsOptions() ) );

        // TP_FORMULA
        ScFormulaOptions aOptions = GetFormulaOptions();
        if (pDocSh)
        {
            ScCalcConfig aConfig( aOptions.GetCalcConfig());
            aConfig.MergeDocumentSpecific( pDocSh->GetDocument().GetCalcConfig());
            aOptions.SetCalcConfig( aConfig);
        }
        pRet->Put( ScTpFormulaItem( aOptions ) );
    }
    return pRet;
}

void ScModule::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    if(SID_SC_EDITOPTIONS == nId)
    {
        ModifyOptions( rSet );
    }
}

VclPtr<SfxTabPage> ScModule::CreateTabPage( sal_uInt16 nId, TabPageParent pParent, const SfxItemSet& rSet )
{
    VclPtr<SfxTabPage> pRet;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    switch(nId)
    {
        case SID_SC_TP_LAYOUT:
        {
            ::CreateTabPage ScTpLayoutOptionsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_LAYOUT);
            if (ScTpLayoutOptionsCreate)
                pRet = (*ScTpLayoutOptionsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_CONTENT:
        {
            ::CreateTabPage ScTpContentOptionsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_CONTENT);
            if (ScTpContentOptionsCreate)
                pRet = (*ScTpContentOptionsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_GRID:
            pRet = SvxGridTabPage::Create(pParent, rSet);
            break;
        case SID_SC_TP_USERLISTS:
        {
            ::CreateTabPage ScTpUserListsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_USERLISTS);
            if (ScTpUserListsCreate)
                pRet = (*ScTpUserListsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_CALC:
        {
            ::CreateTabPage ScTpCalcOptionsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_CALC);
            if (ScTpCalcOptionsCreate)
                pRet = (*ScTpCalcOptionsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_FORMULA:
        {
            ::CreateTabPage ScTpFormulaOptionsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_FORMULA);
            if (ScTpFormulaOptionsCreate)
                pRet = (*ScTpFormulaOptionsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_COMPATIBILITY:
        {
            ::CreateTabPage ScTpCompatOptionsCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_COMPATIBILITY);
            if (ScTpCompatOptionsCreate)
                pRet = (*ScTpCompatOptionsCreate)(pParent, &rSet);
            break;
        }
        case SID_SC_TP_CHANGES:
        {
            ::CreateTabPage ScRedlineOptionsTabPageCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_CHANGES);
            if (ScRedlineOptionsTabPageCreate)
                pRet =(*ScRedlineOptionsTabPageCreate)(pParent, &rSet);
            break;
        }
        case RID_SC_TP_PRINT:
        {
            ::CreateTabPage ScTpPrintOptionsCreate = pFact->GetTabPageCreatorFunc(RID_SC_TP_PRINT);
            if (ScTpPrintOptionsCreate)
                pRet = (*ScTpPrintOptionsCreate)(pParent, &rSet);
            break;
        }
        case RID_SC_TP_DEFAULTS:
        {
            ::CreateTabPage ScTpDefaultsOptionsCreate = pFact->GetTabPageCreatorFunc(RID_SC_TP_DEFAULTS);
            if (ScTpDefaultsOptionsCreate)
                pRet = (*ScTpDefaultsOptionsCreate)(pParent, &rSet);
            break;
        }
    }

    OSL_ENSURE( pRet, "ScModule::CreateTabPage(): no valid ID for TabPage!" );

    return pRet;
}

IMPL_LINK( ScModule, CalcFieldValueHdl, EditFieldInfo*, pInfo, void )
{
    //TODO: Merge with ScFieldEditEngine!
    if (!pInfo)
        return;

    const SvxFieldItem& rField = pInfo->GetField();
    const SvxFieldData* pField = rField.GetField();

    if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
    {
        // URLField
        const OUString& aURL = pURLField->GetURL();

        switch ( pURLField->GetFormat() )
        {
            case SvxURLFormat::AppDefault: //TODO: Settable in the App?
            case SvxURLFormat::Repr:
            {
                pInfo->SetRepresentation( pURLField->GetRepresentation() );
            }
            break;

            case SvxURLFormat::Url:
            {
                pInfo->SetRepresentation( aURL );
            }
            break;
        }

        svtools::ColorConfigEntry eEntry =
            INetURLHistory::GetOrCreate()->QueryUrl( aURL ) ? svtools::LINKSVISITED : svtools::LINKS;
        pInfo->SetTextColor( GetColorConfig().GetColorValue(eEntry).nColor );
    }
    else
    {
        OSL_FAIL("Unknown Field");
        pInfo->SetRepresentation(OUString('?'));
    }
}

void ScModule::RegisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    std::vector<VclPtr<vcl::Window> > & rlRefWindow = m_mapRefWindow[nSlotId];

    if( std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd ) == rlRefWindow.end() )
    {
        rlRefWindow.emplace_back(pWnd );
    }

}

void  ScModule::UnregisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return;

    std::vector<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    auto i = std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );

    if( i == rlRefWindow.end() )
        return;

    rlRefWindow.erase( i );

    if( rlRefWindow.empty() )
        m_mapRefWindow.erase( nSlotId );
}

vcl::Window *  ScModule::Find1RefWindow( sal_uInt16 nSlotId, vcl::Window *pWndAncestor )
{
    if (!pWndAncestor)
        return nullptr;

    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return nullptr;

    std::vector<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    while( vcl::Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for (auto const& refWindow : rlRefWindow)
        if ( pWndAncestor->IsWindowOrChild( refWindow, refWindow->IsSystemWindow() ) )
            return refWindow;

    return nullptr;
}

using namespace com::sun::star;

#define LINGUPROP_AUTOSPELL "IsSpellAuto"

void ScModule::GetSpellSettings( LanguageType& rDefLang, LanguageType& rCjkLang, LanguageType& rCtlLang,
        bool& rAutoSpell )
{
    // use SvtLinguConfig instead of service LinguProperties to avoid
    // loading the linguistic component
    SvtLinguConfig aConfig;

    SvtLinguOptions aOptions;
    aConfig.GetOptions( aOptions );

    rDefLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage, css::i18n::ScriptType::LATIN);
    rCjkLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN);
    rCtlLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);
    rAutoSpell = aOptions.bIsSpellAuto;
}

void ScModule::SetAutoSpellProperty( bool bSet )
{
    // use SvtLinguConfig instead of service LinguProperties to avoid
    // loading the linguistic component
    SvtLinguConfig aConfig;

    aConfig.SetProperty( OUString( LINGUPROP_AUTOSPELL ), uno::Any(bSet) );
}

bool ScModule::HasThesaurusLanguage( LanguageType nLang )
{
    if ( nLang == LANGUAGE_NONE )
        return false;

    bool bHasLang = false;
    try
    {
        uno::Reference< linguistic2::XThesaurus > xThes(LinguMgr::GetThesaurus());
        if ( xThes.is() )
            bHasLang = xThes->hasLocale( LanguageTag::convertToLocale( nLang ) );
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("Error in Thesaurus");
    }

    return bHasLang;
}

std::unique_ptr<SfxStyleFamilies> ScModule::CreateStyleFamilies()
{
    std::unique_ptr<SfxStyleFamilies> pStyleFamilies(new SfxStyleFamilies);

    pStyleFamilies->emplace_back(SfxStyleFamilyItem(SfxStyleFamily::Para,
                                                    ScResId(STR_STYLE_FAMILY_CELL),
                                                    Image(StockImage::Yes, BMP_STYLES_FAMILY_CELL),
                                                    RID_CELLSTYLEFAMILY, SC_MOD()->GetResLocale()));

    pStyleFamilies->emplace_back(SfxStyleFamilyItem(SfxStyleFamily::Page,
                                                    ScResId(STR_STYLE_FAMILY_PAGE),
                                                    Image(StockImage::Yes, BMP_STYLES_FAMILY_PAGE),
                                                    RID_PAGESTYLEFAMILY, SC_MOD()->GetResLocale()));

    return pStyleFamilies;
}

void ScModule::RegisterAutomationApplicationEventsCaller(css::uno::Reference< ooo::vba::XSinkCaller > const& xCaller)
{
    mxAutomationApplicationEventsCaller = xCaller;
}

void ScModule::CallAutomationApplicationEventSinks(const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments)
{
    if (mxAutomationApplicationEventsCaller.is())
        mxAutomationApplicationEventsCaller->CallSinks(Method, Arguments);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
