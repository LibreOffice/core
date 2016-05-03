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

#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <comphelper/processfactory.hxx>

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/flditem.hxx>
#include <editeng/outliner.hxx>
#include <basic/sbstar.hxx>

#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#include "IAnyRefDialog.hxx"
#include "anyrefdg.hxx"

#include <svtools/ehdl.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/status.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <editeng/langitem.hxx>
#include <svtools/colorcfg.hxx>

#include <svl/whiter.hxx>
#include <svx/selctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/modctrl.hxx>
#include <svx/pszctrl.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <vcl/msgbox.hxx>
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
#include <com/sun/star/lang/Locale.hpp>

#include "scmod.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "docoptio.hxx"
#include "appoptio.hxx"
#include "defaultsoptions.hxx"
#include "formulaopt.hxx"
#include "inputopt.hxx"
#include "printopt.hxx"
#include "navicfg.hxx"
#include "addincfg.hxx"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "cfgids.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "msgpool.hxx"
#include "scresid.hxx"
#include "dwfunctr.hxx"
#include "formdata.hxx"
#include "tpprint.hxx"
#include "tpdefaults.hxx"
#include "transobj.hxx"
#include "detfunc.hxx"
#include "preview.hxx"
#include "dragdata.hxx"
#include "clipdata.hxx"
#include "markdata.hxx"

#include <svx/xmlsecctrl.hxx>

#define ScModule
#include "scslots.hxx"

#include "scabstdlg.hxx"
#include <formula/errorcodes.hxx>
#include "formulagroup.hxx"
#include <documentlinkmgr.hxx>

#define SC_IDLE_MIN     150
#define SC_IDLE_MAX     3000
#define SC_IDLE_STEP    75
#define SC_IDLE_COUNT   50

static sal_uInt16 nIdleCount = 0;

SFX_IMPL_INTERFACE(ScModule, SfxShell)

void ScModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
                                            RID_OBJECTBAR_APP);

    GetStaticInterface()->RegisterStatusBar(ScResId(SCCFG_STATUSBAR));
}

ScModule::ScModule( SfxObjectFactory* pFact ) :
    SfxModule( ResMgr::CreateResMgr( "sc" ), false, pFact, nullptr ),
    mpDragData(new ScDragData),
    mpClipData(new ScClipData),
    pSelTransfer( nullptr ),
    pMessagePool( nullptr ),
    pRefInputHandler( nullptr ),
    pViewCfg( nullptr ),
    pDocCfg( nullptr ),
    pAppCfg( nullptr ),
    pDefaultsCfg( nullptr ),
    pFormulaCfg( nullptr ),
    pInputCfg( nullptr ),
    pPrintCfg( nullptr ),
    pNavipiCfg( nullptr ),
    pAddInCfg( nullptr ),
    pColorConfig( nullptr ),
    pAccessOptions( nullptr ),
    pCTLOptions( nullptr ),
    pUserOptions( nullptr ),
    pErrorHdl( nullptr ),
    pFormEditData( nullptr ),
    nCurRefDlgId( 0 ),
    bIsWaterCan( false ),
    bIsInEditCommand( false ),
    bIsInExecuteDrop( false ),
    mbIsInSharedDocLoading( false ),
    mbIsInSharedDocSaving( false )
{
    // The ResManager (DLL data) is not yet initalized in the ctor!
    SetName("StarCalc"); // for Basic

    ResetDragObject();
    SetClipObject( nullptr, nullptr );

    // InputHandler does not need to be created

    // Create ErrorHandler - was in Init()
    // Between OfficeApplication::Init and ScGlobal::Init
    SvxErrorHandler::ensure();
    pErrorHdl    = new SfxErrorHandler( RID_ERRHDLSC,
                                        ERRCODE_AREA_SC,
                                        ERRCODE_AREA_APP2-1,
                                        GetResMgr() );

    aSpellIdle.SetPriority(SchedulerPriority::LOWER);
    aSpellIdle.SetIdleHdl( LINK( this, ScModule, SpellTimerHdl ) );
    aIdleTimer.SetTimeout(SC_IDLE_MIN);
    aIdleTimer.SetTimeoutHdl( LINK( this, ScModule, IdleHandler ) );
    aIdleTimer.Start();

    pMessagePool = new ScMessagePool;
    pMessagePool->FreezeIdRanges();
    SetPool( pMessagePool );
    ScGlobal::InitTextHeight( pMessagePool );

    StartListening( *SfxGetpApp() );       // for SFX_HINT_DEINITIALIZING
}

ScModule::~ScModule()
{
    OSL_ENSURE( !pSelTransfer, "Selection Transfer object not deleted" );

    // InputHandler does not need to be deleted (there's none in the App anymore)

    SfxItemPool::Free(pMessagePool);

    DELETEZ( pFormEditData );

    delete mpDragData;
    delete mpClipData;
    delete pErrorHdl;

    ScGlobal::Clear(); // Also calls ScDocumentPool::DeleteVersionMaps();

    DeleteCfg(); // Called from Exit()
}

void ScModule::ConfigurationChanged( utl::ConfigurationBroadcaster* p, sal_uInt32 )
{
    if ( p == pColorConfig || p == pAccessOptions )
    {
        // Test if detective objects have to be updated with new colors
        // (if the detective colors haven't been used yet, there's nothing to update)
        if ( ScDetectiveFunc::IsColorsInitialized() )
        {
            const svtools::ColorConfig& rColors = GetColorConfig();
            bool bArrows =
                ( ScDetectiveFunc::GetArrowColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVE).nColor ||
                  ScDetectiveFunc::GetErrorColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVEERROR).nColor );
            bool bComments =
                ( ScDetectiveFunc::GetCommentColor() != (ColorData)rColors.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor );
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
    else if ( p == pCTLOptions )
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
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DEINITIALIZING )
    {
        // ConfigItems must be removed before ConfigManager
        DeleteCfg();
    }
}

void ScModule::DeleteCfg()
{
    DELETEZ( pViewCfg ); // Saving happens automatically before Exit()
    DELETEZ( pDocCfg );
    DELETEZ( pAppCfg );
    DELETEZ( pDefaultsCfg );
    DELETEZ( pFormulaCfg );
    DELETEZ( pInputCfg );
    DELETEZ( pPrintCfg );
    DELETEZ( pNavipiCfg );
    DELETEZ( pAddInCfg );

    if ( pColorConfig )
    {
        pColorConfig->RemoveListener(this);
        DELETEZ( pColorConfig );
    }
    if ( pAccessOptions )
    {
        pAccessOptions->RemoveListener(this);
        DELETEZ( pAccessOptions );
    }
    if ( pCTLOptions )
    {
        pCTLOptions->RemoveListener(this);
        DELETEZ( pCTLOptions );
    }
    if( pUserOptions )
    {
        DELETEZ( pUserOptions );
    }
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
            {
                OUString aMacroName("Template.Samples.ShowStyles");
                SfxApplication::CallAppBasic( aMacroName );
            }
            break;
        case SID_EURO_CONVERTER:
            {
                OUString aMacroName("Euro.ConvertRun.Main");
                SfxApplication::CallAppBasic( aMacroName );
            }
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

                SfxItemSet aSet( GetPool(), SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK );
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
                    FieldUnit eUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                    switch( eUnit )
                    {
                        case FUNIT_MM:      // Just the units that are also in the dialog
                        case FUNIT_CM:
                        case FUNIT_INCH:
                        case FUNIT_PICA:
                        case FUNIT_POINT:
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
                OSL_ENSURE(dynamic_cast<const SfxUInt16Item*>(&p) !=  nullptr,"wrong Parameter");
                const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(p);

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
                DBG_UNHANDLED_EXCEPTION();
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
                rSet.Put( SfxUInt16Item( nWhich, GetAppOptions().GetStatusFunc() ) );
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
    mpDragData->pCellTransfer = nullptr;
    mpDragData->pDrawTransfer = nullptr;
    mpDragData->pJumpLocalDoc = nullptr;
    (mpDragData->aLinkDoc).clear();
    (mpDragData->aLinkTable).clear();
    (mpDragData->aLinkArea).clear();
    (mpDragData->aJumpTarget).clear();
    (mpDragData->aJumpText).clear();
}

void ScModule::SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    ResetDragObject();
    mpDragData->pCellTransfer = pCellObj;
    mpDragData->pDrawTransfer = pDrawObj;
}

void ScModule::SetDragLink(
    const OUString& rDoc, const OUString& rTab, const OUString& rArea )
{
    ResetDragObject();
    mpDragData->aLinkDoc   = rDoc;
    mpDragData->aLinkTable = rTab;
    mpDragData->aLinkArea  = rArea;
}

void ScModule::SetDragJump(
    ScDocument* pLocalDoc, const OUString& rTarget, const OUString& rText )
{
    ResetDragObject();

    mpDragData->pJumpLocalDoc = pLocalDoc;
    mpDragData->aJumpTarget = rTarget;
    mpDragData->aJumpText = rText;
}

void ScModule::SetClipObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    OSL_ENSURE( !pCellObj || !pDrawObj, "SetClipObject: not allowed to set both objects" );

    mpClipData->pCellClipboard = pCellObj;
    mpClipData->pDrawClipboard = pDrawObj;
}

ScDocument* ScModule::GetClipDoc()
{
    // called from document
    ScTransferObj* pObj = ScTransferObj::GetOwnClipboard( nullptr );
    if (pObj)
        return pObj->GetDocument();

    return nullptr;
}

void ScModule::SetSelectionTransfer( ScSelectionTransferObj* pNew )
{
    pSelTransfer = pNew;
}

void ScModule::InitFormEditData()
{
    pFormEditData = new ScFormEditData;
}

void ScModule::ClearFormEditData()
{
    DELETEZ( pFormEditData );
}

void ScModule::SetViewOptions( const ScViewOptions& rOpt )
{
    if ( !pViewCfg )
        pViewCfg = new ScViewCfg;

    pViewCfg->SetOptions( rOpt );
}

const ScViewOptions& ScModule::GetViewOptions()
{
    if ( !pViewCfg )
        pViewCfg = new ScViewCfg;

    return *pViewCfg;
}

void ScModule::SetDocOptions( const ScDocOptions& rOpt )
{
    if ( !pDocCfg )
        pDocCfg = new ScDocCfg;

    pDocCfg->SetOptions( rOpt );
}

const ScDocOptions& ScModule::GetDocOptions()
{
    if ( !pDocCfg )
        pDocCfg = new ScDocCfg;

    return *pDocCfg;
}

#ifndef LRU_MAX
#define LRU_MAX 10
#endif

void ScModule::InsertEntryToLRUList(sal_uInt16 nFIndex)
{
    if(nFIndex != 0)
    {
        const ScAppOptions& rAppOpt = GetAppOptions();
        sal_uInt16 nLRUFuncCount = std::min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
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

        RecentFunctionsChanged();
    }
}

void ScModule::RecentFunctionsChanged()
{
    // update function list window
    sal_uInt16 nFuncListID = ScFunctionChildWindow::GetChildWindowId();

    //! notify all views
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm && pViewFrm->HasChildWindow(nFuncListID))
    {
        ScFunctionChildWindow* pWnd = static_cast<ScFunctionChildWindow*>(pViewFrm->GetChildWindow(nFuncListID));
        if (!pWnd)
            return;
        ScFunctionDockWin* pFuncList = static_cast<ScFunctionDockWin*>(pWnd->GetWindow());
        pFuncList->InitLRUList();
    }
}

void ScModule::SetAppOptions( const ScAppOptions& rOpt )
{
    if ( !pAppCfg )
        pAppCfg = new ScAppCfg;

    pAppCfg->SetOptions( rOpt );
}

void global_InitAppOptions()
{
    SC_MOD()->GetAppOptions();
}

const ScAppOptions& ScModule::GetAppOptions()
{
    if ( !pAppCfg )
        pAppCfg = new ScAppCfg;

    return *pAppCfg;
}

void ScModule::SetDefaultsOptions( const ScDefaultsOptions& rOpt )
{
    if ( !pDefaultsCfg )
        pDefaultsCfg = new ScDefaultsCfg;

    pDefaultsCfg->SetOptions( rOpt );
}

const ScDefaultsOptions& ScModule::GetDefaultsOptions()
{
    if ( !pDefaultsCfg )
        pDefaultsCfg = new ScDefaultsCfg;

    return *pDefaultsCfg;
}

void ScModule::SetFormulaOptions( const ScFormulaOptions& rOpt )
{
    if ( !pFormulaCfg )
        pFormulaCfg = new ScFormulaCfg;

    pFormulaCfg->SetOptions( rOpt );
}

const ScFormulaOptions& ScModule::GetFormulaOptions()
{
    if ( !pFormulaCfg )
        pFormulaCfg = new ScFormulaCfg;

    return *pFormulaCfg;
}

void ScModule::SetInputOptions( const ScInputOptions& rOpt )
{
    if ( !pInputCfg )
        pInputCfg = new ScInputCfg;

    pInputCfg->SetOptions( rOpt );
}

const ScInputOptions& ScModule::GetInputOptions()
{
    if ( !pInputCfg )
        pInputCfg = new ScInputCfg;

    return *pInputCfg;
}

void ScModule::SetPrintOptions( const ScPrintOptions& rOpt )
{
    if ( !pPrintCfg )
        pPrintCfg = new ScPrintCfg;

    pPrintCfg->SetOptions( rOpt );
}

const ScPrintOptions& ScModule::GetPrintOptions()
{
    if ( !pPrintCfg )
        pPrintCfg = new ScPrintCfg;

    return *pPrintCfg;
}

ScNavipiCfg& ScModule::GetNavipiCfg()
{
    if ( !pNavipiCfg )
        pNavipiCfg = new ScNavipiCfg;

    return *pNavipiCfg;
}

ScAddInCfg& ScModule::GetAddInCfg()
{
    if ( !pAddInCfg )
        pAddInCfg = new ScAddInCfg;

    return *pAddInCfg;
}

svtools::ColorConfig& ScModule::GetColorConfig()
{
    if ( !pColorConfig )
    {
        pColorConfig = new svtools::ColorConfig;
        pColorConfig->AddListener(this);
    }

    return *pColorConfig;
}

SvtAccessibilityOptions& ScModule::GetAccessOptions()
{
    if ( !pAccessOptions )
    {
        pAccessOptions = new SvtAccessibilityOptions;
        pAccessOptions->AddListener(this);
    }

    return *pAccessOptions;
}

SvtCTLOptions& ScModule::GetCTLOptions()
{
    if ( !pCTLOptions )
    {
        pCTLOptions = new SvtCTLOptions;
        pCTLOptions->AddListener(this);
    }

    return *pCTLOptions;
}

SvtUserOptions&  ScModule::GetUserOptions()
{
    if( !pUserOptions )
    {
        pUserOptions = new SvtUserOptions;
    }
    return *pUserOptions;
}

sal_uInt16 ScModule::GetOptDigitLanguage()
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
    sal_uInt16 nOldSpellLang, nOldCjkLang, nOldCtlLang;
    bool bOldAutoSpell;
    GetSpellSettings( nOldSpellLang, nOldCjkLang, nOldCtlLang, bOldAutoSpell );

    if (!pAppCfg)
        GetAppOptions();
    OSL_ENSURE( pAppCfg, "AppOptions not initialised :-(" );

    if (!pInputCfg)
        GetInputOptions();
    OSL_ENSURE( pInputCfg, "InputOptions not initialised :-(" );

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
        pAppCfg->SetAppMetric( (FieldUnit)static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SCITEM_USERLIST, &pItem))
    {
        ScGlobal::SetUserList( static_cast<const ScUserListItem*>(pItem)->GetUserList() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SID_SC_OPT_SYNCZOOM, &pItem))
    {
        pAppCfg->SetSynchronizeZoom( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SID_SC_OPT_KEY_BINDING_COMPAT, &pItem))
    {
        sal_uInt16 nVal = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        ScOptionsUtil::KeyBindingType eOld = pAppCfg->GetKeyBindingType();
        ScOptionsUtil::KeyBindingType eNew = static_cast<ScOptionsUtil::KeyBindingType>(nVal);
        if (eOld != eNew)
        {
            pAppCfg->SetKeyBindingType(eNew);
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

        if (!pFormulaCfg || (*pFormulaCfg != rOpt))
            // Formula options have changed. Repaint the column headers.
            bRepaint = true;

        if (pFormulaCfg && pFormulaCfg->GetUseEnglishFuncName() != rOpt.GetUseEnglishFuncName())
        {
            // Re-compile formula cells with error as the error may have been
            // caused by unresolved function names.
            bCompileErrorCells = true;
        }

        // Recalc for interpreter options changes.
        if (pFormulaCfg && pFormulaCfg->GetCalcConfig() != rOpt.GetCalcConfig())
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
            aNewOpt.GetCalcConfig().MergeDocumentSpecific( pFormulaCfg->GetCalcConfig());
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
        pInputCfg->SetMoveDir( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_SELECTION,&pItem) )
    {
        pInputCfg->SetMoveSelection( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_EDITMODE,&pItem) )
    {
        pInputCfg->SetEnterEdit( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_FMT_EXPAND,&pItem) )
    {
        pInputCfg->SetExtendFormat( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_RANGEFINDER,&pItem) )
    {
        pInputCfg->SetRangeFinder( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_REF_EXPAND,&pItem) )
    {
        pInputCfg->SetExpandRefs( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if (rOptSet.HasItem(SID_SC_OPT_SORT_REF_UPDATE, &pItem))
    {
        pInputCfg->SetSortRefUpdate(static_cast<const SfxBoolItem*>(pItem)->GetValue());
        bSaveInputOptions = true;
    }

    if ( rOptSet.HasItem(SID_SC_INPUT_MARK_HEADER,&pItem) )
    {
        pInputCfg->SetMarkHeader( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
        bUpdateMarks = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_TEXTWYSIWYG,&pItem) )
    {
        bool bNew = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        if ( bNew != pInputCfg->GetTextWysiwyg() )
        {
            pInputCfg->SetTextWysiwyg( bNew );
            bSaveInputOptions = true;
            bUpdateRefDev = true;
        }
    }
    if( rOptSet.HasItem( SID_SC_INPUT_REPLCELLSWARN, &pItem ) )
    {
        pInputCfg->SetReplaceCellsWarn( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    if( rOptSet.HasItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, &pItem ) )
    {
        pInputCfg->SetLegacyCellSelection( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    // PrintOptions
    if ( rOptSet.HasItem(SID_SCPRINTOPTIONS,&pItem) )
    {
        const ScPrintOptions& rNewOpt = static_cast<const ScTpPrintItem*>(pItem)->GetPrintOptions();
        SetPrintOptions( rNewOpt );

        // broadcast causes all previews to recalc page numbers
        SfxGetpApp()->Broadcast( SfxSimpleHint( SID_SCPRINTOPTIONS ) );
    }

    if ( bSaveAppOptions )
        pAppCfg->OptionsChanged();

    if ( bSaveInputOptions )
        pInputCfg->OptionsChanged();

    // Kick off recalculation?
    if (pDoc && bCompileErrorCells)
    {
        // Re-compile cells with name error, and recalc if at least one cell
        // has been re-compiled.  In the future we may want to find a way to
        // recalc only those that are affected.
        if (pDoc->CompileErrorCells(ScErrorCodes::errNoName))
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
    if ( pRefInputHandler && bUseRef )
        return pRefInputHandler;

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

void ScModule::ViewShellChanged()
{
    ScInputHandler* pHdl   = GetInputHdl();
    ScTabViewShell* pShell = ScTabViewShell::GetActiveViewShell();
    if ( pShell && pHdl )
        pShell->UpdateInputHandler();
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

void ScModule::InputSelection( EditView* pView )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputSelection( pView );
}

void ScModule::InputChanged( EditView* pView )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputChanged( pView );
}

void ScModule::ViewShellGone( ScTabViewShell* pViewSh )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->ViewShellGone( pViewSh );
}

void ScModule::SetRefInputHdl( ScInputHandler* pNew )
{
    pRefInputHandler = pNew;
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
    if(nCurRefDlgId==0 || (nId==nCurRefDlgId && !bVis))
    {
        if ( !pViewFrm )
            pViewFrm = SfxViewFrame::Current();

        // bindings update causes problems with update of stylist if
        // current style family has changed
        //if ( pViewFrm )
        //  pViewFrm->GetBindings().Update();       // to avoid trouble in LockDispatcher

        nCurRefDlgId = bVis ? nId : 0 ;             // before SetChildWindow

        if ( pViewFrm )
        {
            //  store the dialog id also in the view shell
            SfxViewShell* pViewSh = pViewFrm->GetViewShell();
            if (ScTabViewShell* pTabViewSh = dynamic_cast<ScTabViewShell*>(pViewSh))
                pTabViewSh->SetCurRefDlgId(nCurRefDlgId);
            else
            {
                // no ScTabViewShell - possible for example from a Basic macro
                bVis = false;
                nCurRefDlgId = 0;   // don't set nCurRefDlgId if no dialog is created
            }

            pViewFrm->SetChildWindow( nId, bVis );
        }

        SfxApplication* pSfxApp = SfxGetpApp();
        pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
    }
}

static SfxChildWindow* lcl_GetChildWinFromAnyView( sal_uInt16 nId )
{
    // First, try the current view
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    // #i46999# current view frame can be null (for example, when closing help)
    SfxChildWindow* pChildWnd = pViewFrm ? pViewFrm->GetChildWindow( nId ) : nullptr;
    if ( pChildWnd )
        return pChildWnd;           // found in the current view

    //  if not found there, get the child window from any open view
    //  it can be open only in one view because nCurRefDlgId is global

    pViewFrm = SfxViewFrame::GetFirst();
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

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsModal = pChildWnd->IsVisible() && pRefDlg &&
                !( pRefDlg->IsRefInputMode() && pRefDlg->IsDocAllowed(pDocSh) );
        }
        else
        {
            // in 592 and above, the dialog isn't visible in other views
            //  if the dialog is open but can't be accessed, disable input
            bIsModal = true;
        }

        //  pChildWnd can be 0 if the dialog has not been created by another Shell yet after
        //  switching over(e.g. in GetFocus())
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
    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
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

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
            bIsOpen = pChildWnd->IsVisible();
        else
            bIsOpen = true;     // for other views, see IsModalMode
    }

    return bIsOpen;
}

bool ScModule::IsFormulaMode()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsFormula = false;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsFormula = pChildWnd->IsVisible() && pRefDlg && pRefDlg->IsRefInputMode();
        }
        else
            bIsFormula = true;
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if ( pHdl )
            bIsFormula = pHdl->IsFormulaMode();
    }

    if (bIsInEditCommand)
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

    if( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            if ( nCurRefDlgId == SID_OPENDLG_CONSOLIDATE && pMarkData )
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
 * Multiple selecton
 */
void ScModule::AddRefEntry()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
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
    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
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
    sal_uLong nOldTime = aIdleTimer.GetTimeout();
    if ( nOldTime != SC_IDLE_MIN )
        aIdleTimer.SetTimeout( SC_IDLE_MIN );

    nIdleCount = 0;
}

static void lcl_CheckNeedsRepaint( ScDocShell* pDocShell )
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

IMPL_LINK_NOARG_TYPED(ScModule, IdleHandler, Timer *, void)
{
    if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD ) )
    {
        aIdleTimer.Start(); // Timeout unchanged
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
                aSpellIdle.Start();
                bMore = true;
            }
        }
    }

    sal_uLong nOldTime = aIdleTimer.GetTimeout();
    sal_uLong nNewTime = nOldTime;
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
        aIdleTimer.SetTimeout( nNewTime );


    aIdleTimer.Start();
}

IMPL_LINK_NOARG_TYPED(ScModule, SpellTimerHdl, Idle *, void)
{
    if ( Application::AnyInput( VclInputFlags::KEYBOARD ) )
    {
        aSpellIdle.Start();
        return; // Later again ...
    }

    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    if (pViewSh)
    {
        if (pViewSh->ContinueOnlineSpelling())
            aSpellIdle.Start();
    }
}

/**
 * Virtual methods for the OptionsDialog
 */
SfxItemSet*  ScModule::CreateItemSet( sal_uInt16 nId )
{
    SfxItemSet*  pRet = nullptr;
    if(SID_SC_EDITOPTIONS == nId)
    {
        pRet = new SfxItemSet( GetPool(),
                            // TP_CALC:
                            SID_SCDOCOPTIONS,       SID_SCDOCOPTIONS,
                            // TP_VIEW:
                            SID_SCVIEWOPTIONS,      SID_SCVIEWOPTIONS,
                            SID_SC_OPT_SYNCZOOM,    SID_SC_OPT_SYNCZOOM,
                            // TP_INPUT:
                            SID_SC_INPUT_SELECTION,SID_SC_INPUT_MARK_HEADER,
                            SID_SC_INPUT_TEXTWYSIWYG,SID_SC_INPUT_TEXTWYSIWYG,
                            SID_SC_INPUT_REPLCELLSWARN,SID_SC_INPUT_REPLCELLSWARN,
                            SID_SC_INPUT_LEGACY_CELL_SELECTION,SID_SC_OPT_SORT_REF_UPDATE,
                            // TP_USERLISTS:
                            SCITEM_USERLIST,        SCITEM_USERLIST,
                            // TP_PRINT:
                            SID_SCPRINTOPTIONS, SID_SCPRINTOPTIONS,
                            // TP_GRID:
                            SID_ATTR_GRID_OPTIONS, SID_ATTR_GRID_OPTIONS,

                            SID_ATTR_METRIC,        SID_ATTR_METRIC,
                            SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                            // TP_COMPATIBILITY
                            SID_SC_OPT_KEY_BINDING_COMPAT, SID_SC_OPT_KEY_BINDING_COMPAT,
                            // TP_DEFAULTS
                            SID_SCDEFAULTSOPTIONS, SID_SCDEFAULTSOPTIONS,
                            // TP_FORMULA
                            SID_SCFORMULAOPTIONS, SID_SCFORMULAOPTIONS,
                            0 );

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
        pRet->Put( ScTpViewItem( SID_SCVIEWOPTIONS, aViewOpt ) );
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
        pRet->Put( ScTpPrintItem( SID_SCPRINTOPTIONS, GetPrintOptions() ) );

        // TP_GRID
        SvxGridItem* pSvxGridItem = aViewOpt.CreateGridItem();
        pRet->Put( *pSvxGridItem );
        delete pSvxGridItem;

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
        pRet->Put( ScTpDefaultsItem( SID_SCDEFAULTSOPTIONS, GetDefaultsOptions() ) );

        // TP_FORMULA
        ScFormulaOptions aOptions = GetFormulaOptions();
        if (pDocSh)
        {
            ScCalcConfig aConfig( aOptions.GetCalcConfig());
            aConfig.MergeDocumentSpecific( pDocSh->GetDocument().GetCalcConfig());
            aOptions.SetCalcConfig( aConfig);
        }
        pRet->Put( ScTpFormulaItem( SID_SCFORMULAOPTIONS, aOptions ) );
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

VclPtr<SfxTabPage> ScModule::CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
{
    VclPtr<SfxTabPage> pRet;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
    switch(nId)
    {
        case SID_SC_TP_LAYOUT:
                                {
                                    ::CreateTabPage ScTpLayoutOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_LAYOUT );
                                    if ( ScTpLayoutOptionsCreate )
                                        pRet = (*ScTpLayoutOptionsCreate) (pParent, &rSet);
                                }
                                break;
        case SID_SC_TP_CONTENT:
                                {
                                    ::CreateTabPage ScTpContentOptionsCreate = pFact->GetTabPageCreatorFunc(RID_SCPAGE_CONTENT);
                                    if ( ScTpContentOptionsCreate )
                                        pRet = (*ScTpContentOptionsCreate)(pParent, &rSet);
                                }
                                break;
        case SID_SC_TP_GRID:            pRet = SvxGridTabPage::Create(pParent, rSet); break;
        case SID_SC_TP_USERLISTS:
                                {
                                    ::CreateTabPage ScTpUserListsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_USERLISTS );
                                    if ( ScTpUserListsCreate )
                                            pRet = (*ScTpUserListsCreate)( pParent, &rSet);
                                }
                                break;
        case SID_SC_TP_CALC:
                                {
                                    ::CreateTabPage ScTpCalcOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_CALC );
                                    if ( ScTpCalcOptionsCreate )
                                            pRet = (*ScTpCalcOptionsCreate)(pParent, &rSet);
                                }
                                break;
        case SID_SC_TP_FORMULA:
        {
            ::CreateTabPage ScTpFormulaOptionsCreate = pFact->GetTabPageCreatorFunc (RID_SCPAGE_FORMULA);
            if (ScTpFormulaOptionsCreate)
                pRet = (*ScTpFormulaOptionsCreate)(pParent, &rSet);
        }
        break;
        case SID_SC_TP_COMPATIBILITY:
        {
            ::CreateTabPage ScTpCompatOptionsCreate = pFact->GetTabPageCreatorFunc (RID_SCPAGE_COMPATIBILITY);
            if (ScTpCompatOptionsCreate)
                pRet = (*ScTpCompatOptionsCreate)(pParent, &rSet);
        }
        break;
        case SID_SC_TP_CHANGES:
                                {
                                   ::CreateTabPage ScRedlineOptionsTabPageCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_OPREDLINE );
                                   if ( ScRedlineOptionsTabPageCreate )
                                           pRet =(*ScRedlineOptionsTabPageCreate)(pParent, &rSet);
                                }
                        break;
        case RID_SC_TP_PRINT:
                                {
                                   ::CreateTabPage ScTpPrintOptionsCreate =    pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
                                   if ( ScTpPrintOptionsCreate )
                                          pRet = (*ScTpPrintOptionsCreate)( pParent, &rSet);
                                }
            break;
        case RID_SC_TP_DEFAULTS:
            {
                ::CreateTabPage ScTpDefaultsOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_DEFAULTS );
                if ( ScTpDefaultsOptionsCreate )
                    pRet = (*ScTpDefaultsOptionsCreate)( pParent, &rSet);
            }
            break;
    }

    OSL_ENSURE( pRet, "ScModule::CreateTabPage(): no valid ID for TabPage!" );

    return pRet;
}

IMPL_LINK_TYPED( ScModule, CalcFieldValueHdl, EditFieldInfo*, pInfo, void )
{
    //TODO: Merge with ScFieldEditEngine!
    if (!pInfo)
        return;

    const SvxFieldItem& rField = pInfo->GetField();
    const SvxFieldData* pField = rField.GetField();

    if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
    {
        // URLField
        OUString aURL = pURLField->GetURL();

        switch ( pURLField->GetFormat() )
        {
            case SVXURLFORMAT_APPDEFAULT: //TODO: Settable in the App?
            case SVXURLFORMAT_REPR:
            {
                pInfo->SetRepresentation( pURLField->GetRepresentation() );
            }
            break;

            case SVXURLFORMAT_URL:
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

bool ScModule::RegisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    std::list<VclPtr<vcl::Window> > & rlRefWindow = m_mapRefWindow[nSlotId];

    if( std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd ) == rlRefWindow.end() )
    {
        rlRefWindow.push_back( pWnd );
        return true;
    }

    return false;
}

bool  ScModule::UnregisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return false;

    std::list<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    auto i = std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );

    if( i == rlRefWindow.end() )
        return false;

    rlRefWindow.erase( i );

    if( rlRefWindow.empty() )
        m_mapRefWindow.erase( nSlotId );

    return true;
}

vcl::Window *  ScModule::Find1RefWindow( sal_uInt16 nSlotId, vcl::Window *pWndAncestor )
{
    if (!pWndAncestor)
        return nullptr;

    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return nullptr;

    std::list<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    while( vcl::Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for( auto i = rlRefWindow.begin(); i!=rlRefWindow.end(); ++i )
        if ( pWndAncestor->IsWindowOrChild( *i, (*i)->IsSystemWindow() ) )
            return *i;

    return nullptr;
}

using namespace com::sun::star;

#define LINGUPROP_AUTOSPELL "IsSpellAuto"

void ScModule::GetSpellSettings( sal_uInt16& rDefLang, sal_uInt16& rCjkLang, sal_uInt16& rCtlLang,
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

bool ScModule::HasThesaurusLanguage( sal_uInt16 nLang )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
