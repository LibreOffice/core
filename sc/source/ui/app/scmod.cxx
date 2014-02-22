/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include "formula/errorcodes.hxx"
#include "formulagroup.hxx"
#include <documentlinkmgr.hxx>

#define SC_IDLE_MIN     150
#define SC_IDLE_MAX     3000
#define SC_IDLE_STEP    75
#define SC_IDLE_COUNT   50

static sal_uInt16 nIdleCount = 0;



SFX_IMPL_INTERFACE( ScModule, SfxShell, ScResId(RID_APPTITLE) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
                                ScResId(RID_OBJECTBAR_APP) );
    SFX_STATUSBAR_REGISTRATION( ScResId(SCCFG_STATUSBAR) );     
}



ScModule::ScModule( SfxObjectFactory* pFact ) :
    SfxModule( SfxApplication::CreateResManager( "sc" ), false, pFact, NULL ),
    mpDragData(new ScDragData),
    mpClipData(new ScClipData),
    pSelTransfer( NULL ),
    pMessagePool( NULL ),
    pRefInputHandler( NULL ),
    pViewCfg( NULL ),
    pDocCfg( NULL ),
    pAppCfg( NULL ),
    pDefaultsCfg( NULL ),
    pFormulaCfg( NULL ),
    pInputCfg( NULL ),
    pPrintCfg( NULL ),
    pNavipiCfg( NULL ),
    pAddInCfg( NULL ),
    pColorConfig( NULL ),
    pAccessOptions( NULL ),
    pCTLOptions( NULL ),
    pUserOptions( NULL ),
    pErrorHdl( NULL ),
    pFormEditData( NULL ),
    nCurRefDlgId( 0 ),
    bIsWaterCan( false ),
    bIsInEditCommand( false ),
    bIsInExecuteDrop( false ),
    mbIsInSharedDocLoading( false ),
    mbIsInSharedDocSaving( false )
{
    
    SetName(OUString("StarCalc"));       

    ResetDragObject();
    SetClipObject( NULL, NULL );

    

    
    
    SvxErrorHandler::ensure();
    pErrorHdl    = new SfxErrorHandler( RID_ERRHDLSC,
                                        ERRCODE_AREA_SC,
                                        ERRCODE_AREA_APP2-1,
                                        GetResMgr() );

    aSpellTimer.SetTimeout(10);
    aSpellTimer.SetTimeoutHdl( LINK( this, ScModule, SpellTimerHdl ) );
    aIdleTimer.SetTimeout(SC_IDLE_MIN);
    aIdleTimer.SetTimeoutHdl( LINK( this, ScModule, IdleHandler ) );
    aIdleTimer.Start();

    pMessagePool = new ScMessagePool;
    pMessagePool->FreezeIdRanges();
    SetPool( pMessagePool );
    ScGlobal::InitTextHeight( pMessagePool );

    StartListening( *SFX_APP() );       
}

ScModule::~ScModule()
{
    OSL_ENSURE( !pSelTransfer, "Selection Transfer object not deleted" );

    

    SfxItemPool::Free(pMessagePool);

    DELETEZ( pFormEditData );

    delete mpDragData;
    delete mpClipData;
    delete pErrorHdl;

    ScGlobal::Clear();      

    DeleteCfg();            
}


void ScModule::ConfigurationChanged( utl::ConfigurationBroadcaster* p, sal_uInt32 )
{
    if ( p == pColorConfig || p == pAccessOptions )
    {
        
        
        if ( ScDetectiveFunc::IsColorsInitialized() )
        {
            const svtools::ColorConfig& rColors = GetColorConfig();
            sal_Bool bArrows =
                ( ScDetectiveFunc::GetArrowColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVE).nColor ||
                  ScDetectiveFunc::GetErrorColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVEERROR).nColor );
            sal_Bool bComments =
                ( ScDetectiveFunc::GetCommentColor() != (ColorData)rColors.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor );
            if ( bArrows || bComments )
            {
                ScDetectiveFunc::InitializeColors();        

                
                SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
                while ( pObjSh )
                {
                    if ( pObjSh->Type() == TYPE(ScDocShell) )
                    {
                        ScDocShell* pDocSh = ((ScDocShell*)pObjSh);
                        if ( bArrows )
                            ScDetectiveFunc( pDocSh->GetDocument(), 0 ).UpdateAllArrowColors();
                        if ( bComments )
                            ScDetectiveFunc::UpdateAllComments( *pDocSh->GetDocument() );
                    }
                    pObjSh = SfxObjectShell::GetNext( *pObjSh );
                }
            }
        }

        

        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while(pViewShell)
        {
            if ( pViewShell->ISA(ScTabViewShell) )
            {
                ScTabViewShell* pViewSh = (ScTabViewShell*)pViewShell;
                pViewSh->PaintGrid();
                pViewSh->PaintTop();
                pViewSh->PaintLeft();
                pViewSh->PaintExtras();

                ScInputHandler* pHdl = pViewSh->GetInputHandler();
                if ( pHdl )
                    pHdl->ForgetLastPattern();  
            }
            else if ( pViewShell->ISA(ScPreviewShell) )
            {
                Window* pWin = pViewShell->GetWindow();
                if (pWin)
                    pWin->Invalidate();
            }
            pViewShell = SfxViewShell::GetNext( *pViewShell );
        }
    }
    else if ( p == pCTLOptions )
    {
        
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
        while ( pObjSh )
        {
            if ( pObjSh->Type() == TYPE(ScDocShell) )
            {
                ScDocShell* pDocSh = ((ScDocShell*)pObjSh);
                OutputDevice* pPrinter = pDocSh->GetPrinter();
                if ( pPrinter )
                    pPrinter->SetDigitLanguage( GetOptDigitLanguage() );

                pDocSh->CalcOutputFactor();

                SCTAB nTabCount = pDocSh->GetDocument()->GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    pDocSh->AdjustRowHeight( 0, MAXROW, nTab );
            }
            pObjSh = SfxObjectShell::GetNext( *pObjSh );
        }

        
        SfxViewShell* pSh = SfxViewShell::GetFirst();
        while ( pSh )
        {
            if ( pSh->ISA( ScTabViewShell ) )
            {
                ScTabViewShell* pViewSh = (ScTabViewShell*)pSh;

                
                ScInputHandler* pHdl = GetInputHdl(pViewSh);
                if (pHdl)
                    pHdl->UpdateRefDevice();

                pViewSh->DigitLanguageChanged();
                pViewSh->PaintGrid();
            }
            else if ( pSh->ISA( ScPreviewShell ) )
            {
                ScPreviewShell* pPreviewSh = (ScPreviewShell*)pSh;
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
    if ( rHint.ISA(SfxSimpleHint) )
    {
        sal_uLong nHintId = ((SfxSimpleHint&)rHint).GetId();
        if ( nHintId == SFX_HINT_DEINITIALIZING )
        {
            
            DeleteCfg();
        }
    }
}



void ScModule::DeleteCfg()
{
    DELETEZ( pViewCfg ); 
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


//

//


void ScModule::Execute( SfxRequest& rReq )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : NULL;

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
                sal_Bool bSet;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                else
                {                       
                    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
                    if ( pDocSh )
                        bSet = !pDocSh->GetDocument()->GetDocOptions().IsAutoSpell();
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
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                {
                    FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
                    switch( eUnit )
                    {
                        case FUNIT_MM:      
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
                SFX_REQUEST_ARG( rReq, pAuto, SfxBoolItem, SID_DETECTIVE_AUTO, false );
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
                const SfxUInt16Item& rItem = (const SfxUInt16Item&)pReqArgs->Get(SID_PSZ_FUNCTION);
                OSL_ENSURE(rItem.ISA(SfxUInt16Item),"falscher Parameter");

                ScAppOptions aNewOpts( GetAppOptions() );
                aNewOpts.SetStatusFunc( rItem.GetValue() );
                SetAppOptions( aNewOpts );

                if (pBindings)
                {
                    pBindings->Invalidate( SID_TABLE_CELL );
                    pBindings->Update( SID_TABLE_CELL );            

                    pBindings->Invalidate( SID_PSZ_FUNCTION );
                    pBindings->Update( SID_PSZ_FUNCTION );
                    
                }
            }
            break;

        case SID_ATTR_LANGUAGE:
        case SID_ATTR_CHAR_CJK_LANGUAGE:
        case SID_ATTR_CHAR_CTL_LANGUAGE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( GetPool().GetWhich(nSlot), true, &pItem ) )
                {
                    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
                    ScDocument* pDoc = pDocSh ? pDocSh->GetDocument() : NULL;
                    if ( pDoc )
                    {
                        LanguageType eNewLang = ((SvxLanguageItem*)pItem)->GetLanguage();
                        LanguageType eLatin, eCjk, eCtl;
                        pDoc->GetLanguage( eLatin, eCjk, eCtl );
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

                            pDoc->SetLanguage( eLatin, eCjk, eCtl );

                            ScInputHandler* pInputHandler = GetInputHdl();
                            if ( pInputHandler )
                                pInputHandler->UpdateSpellSettings();   
                            ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
                            if ( pViewSh )
                                pViewSh->UpdateDrawTextOutliner();      

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
            catch( ::com::sun::star::uno::RuntimeException& )
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
    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
    bool bTabView = pDocSh && (pDocSh->GetBestViewShell(true) != NULL);

    SfxWhichIter aIter(rSet);
    for (sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich())
    {
        if (!bTabView)
        {
            
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
                rSet.Put( SfxBoolItem( nWhich, pDocSh->GetDocument()->GetDocOptions().IsAutoSpell()) );
                break;
            case SID_ATTR_LANGUAGE:
            case ATTR_CJK_FONT_LANGUAGE:        
            case ATTR_CTL_FONT_LANGUAGE:        
                {
                    ScDocument* pDoc = pDocSh->GetDocument();
                    if ( pDoc )
                    {
                        LanguageType eLatin, eCjk, eCtl;
                        pDoc->GetLanguage( eLatin, eCjk, eCtl );
                        LanguageType eLang = ( nWhich == ATTR_CJK_FONT_LANGUAGE ) ? eCjk :
                                            ( ( nWhich == ATTR_CTL_FONT_LANGUAGE ) ? eCtl : eLatin );
                        rSet.Put( SvxLanguageItem( eLang, nWhich ) );
                    }
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
            
            rSet.DisableItem( nWhich );
        }
    }
}




void ScModule::ResetDragObject()
{
    mpDragData->pCellTransfer = NULL;
    mpDragData->pDrawTransfer = NULL;
    mpDragData->pJumpLocalDoc = NULL;
    mpDragData->aLinkDoc = OUString();
    mpDragData->aLinkTable = OUString();
    mpDragData->aLinkArea = OUString();
    mpDragData->aJumpTarget = OUString();
    mpDragData->aJumpText = OUString();
}

const ScDragData& ScModule::GetDragData() const
{
    return *mpDragData;
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

const ScClipData& ScModule::GetClipData() const
{
    return *mpClipData;
}

void ScModule::SetClipObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    OSL_ENSURE( !pCellObj || !pDrawObj, "SetClipObject: not allowed to set both objects" );

    mpClipData->pCellClipboard = pCellObj;
    mpClipData->pDrawClipboard = pDrawObj;
}

ScDocument* ScModule::GetClipDoc()
{
    

    ScTransferObj* pObj = ScTransferObj::GetOwnClipboard( NULL );
    if (pObj)
        return pObj->GetDocument();

    return NULL;
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
        sal_Bool    bFound = false;

        while ((n < LRU_MAX) && n<nLRUFuncCount)                        
        {
            if (!bFound && (pLRUListIds[n]== nFIndex))
                bFound = sal_True;                                          
            else if (bFound)
                aIdxList[n  ] = pLRUListIds[n];                         
            else if ((n+1) < LRU_MAX)
                aIdxList[n+1] = pLRUListIds[n];                         
            n++;
        }
        if (!bFound && (n < LRU_MAX))                                   
            n++;                                                        
        aIdxList[0] = nFIndex;                                          

        ScAppOptions aNewOpts(rAppOpt);                                 
        aNewOpts.SetLRUFuncList(aIdxList, n);
        SetAppOptions(aNewOpts);

        RecentFunctionsChanged();
    }
}

void ScModule::RecentFunctionsChanged()
{
    
    sal_uInt16 nFuncListID = ScFunctionChildWindow::GetChildWindowId();

    
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->HasChildWindow(nFuncListID) )
    {
        ScFunctionChildWindow* pWnd =(ScFunctionChildWindow*)pViewFrm->GetChildWindow( nFuncListID );

        ScFunctionDockWin* pFuncList=(ScFunctionDockWin*)pWnd->GetWindow();

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


//

//


//


//

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
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : NULL;

    ScTabViewShell*         pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    ScDocShell*             pDocSh  = PTR_CAST(ScDocShell, SfxObjectShell::Current());
    ScDocument*             pDoc    = pDocSh ? pDocSh->GetDocument() : NULL;
    const SfxPoolItem*      pItem   = NULL;
    bool bRepaint = false;
    bool bUpdateMarks = false;
    bool bUpdateRefDev = false;
    bool bCalcAll = false;
    bool bSaveAppOptions = false;
    bool bSaveInputOptions = false;
    bool bCompileErrorCells = false;

    

    

    

    if (rOptSet.HasItem(SID_ATTR_METRIC, &pItem))
    {
        PutItem( *pItem );
        pAppCfg->SetAppMetric( (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue() );
        bSaveAppOptions = true;
    }

    if (rOptSet.HasItem(SCITEM_USERLIST, &pItem))
    {
        ScGlobal::SetUserList( ((const ScUserListItem*)pItem)->GetUserList() );
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
            pDocSh->ResetKeyBindings(eNew);
        }
    }

    
    
    

    if (rOptSet.HasItem(SID_SCDEFAULTSOPTIONS, &pItem))
    {
        const ScDefaultsOptions& rOpt = ((const ScTpDefaultsItem*)pItem)->GetDefaultsOptions();
        SetDefaultsOptions( rOpt );
    }

    
    
    

    if (rOptSet.HasItem(SID_SCFORMULAOPTIONS, &pItem))
    {
        const ScFormulaOptions& rOpt = ((const ScTpFormulaItem*)pItem)->GetFormulaOptions();

        if (!pFormulaCfg || (*pFormulaCfg != rOpt))
            
            bRepaint = true;

        if (pFormulaCfg && pFormulaCfg->GetUseEnglishFuncName() != rOpt.GetUseEnglishFuncName())
        {
            
            
            bCompileErrorCells = true;
        }

        SetFormulaOptions( rOpt );

        if ( pDocSh )
        {
            pDocSh->SetFormulaOptions( rOpt );
            pDocSh->SetDocumentModified();
        }
    }

    
    
    

    if (rOptSet.HasItem(SID_SCVIEWOPTIONS, &pItem))
    {
        const ScViewOptions& rNewOpt = ((const ScTpViewItem*)pItem)->GetViewOptions();

        if ( pViewSh )
        {
            ScViewData*             pViewData = pViewSh->GetViewData();
            const ScViewOptions&    rOldOpt   = pViewData->GetOptions();

            sal_Bool bAnchorList = ( rOldOpt.GetOption( VOPT_ANCHOR ) !=
                                 rNewOpt.GetOption( VOPT_ANCHOR ) );

            if ( rOldOpt != rNewOpt )
            {
                pViewData->SetOptions( rNewOpt );   
                pViewData->GetDocument()->SetViewOptions( rNewOpt );
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
            pBindings->Invalidate(SID_SCGRIDSHOW);
        }
    }

    
    
    
    

    if ( rOptSet.HasItem(SID_ATTR_GRID_OPTIONS,&pItem) )
    {
        ScGridOptions aNewGridOpt( (const SvxGridItem&)*pItem );

        if ( pViewSh )
        {
            ScViewData*          pViewData = pViewSh->GetViewData();
            ScViewOptions        aNewViewOpt( pViewData->GetOptions() );
            const ScGridOptions& rOldGridOpt = aNewViewOpt.GetGridOptions();

            if ( rOldGridOpt != aNewGridOpt )
            {
                aNewViewOpt.SetGridOptions( aNewGridOpt );
                pViewData->SetOptions( aNewViewOpt );
                pViewData->GetDocument()->SetViewOptions( aNewViewOpt );
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


    
    
    

    if ( rOptSet.HasItem(SID_SCDOCOPTIONS,&pItem) )
    {
        const ScDocOptions& rNewOpt = ((const ScTpCalcItem*)pItem)->GetDocOptions();

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

    
    if ( rOptSet.HasItem(SID_ATTR_DEFTABSTOP,&pItem) )
    {
        sal_uInt16 nTabDist = ((SfxUInt16Item*)pItem)->GetValue();
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

    

    if ( rOptSet.HasItem(SID_AUTOSPELL_CHECK,&pItem) )              
    {
        bool bDoAutoSpell = ((const SfxBoolItem*)pItem)->GetValue();

        if (pDoc)
        {
            ScDocOptions aNewOpt = pDoc->GetDocOptions();
            if ( aNewOpt.IsAutoSpell() != bDoAutoSpell )
            {
                aNewOpt.SetAutoSpell( bDoAutoSpell );
                pDoc->SetDocOptions( aNewOpt );

                if (pViewSh)
                    pViewSh->EnableAutoSpell(bDoAutoSpell);

                bRepaint = true;            
                                            
            }
        }

        if ( bOldAutoSpell != bDoAutoSpell )
            SetAutoSpellProperty( bDoAutoSpell );
        if ( pDocSh )
            pDocSh->PostPaintGridAll();                     
        ScInputHandler* pInputHandler = GetInputHdl();
        if ( pInputHandler )
            pInputHandler->UpdateSpellSettings();           
        if ( pViewSh )
            pViewSh->UpdateDrawTextOutliner();              

        if (pBindings)
            pBindings->Invalidate( SID_AUTOSPELL_CHECK );
    }

    
    
    

    if ( rOptSet.HasItem(SID_SC_INPUT_SELECTIONPOS,&pItem) )
    {
        pInputCfg->SetMoveDir( ((const SfxUInt16Item*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_SELECTION,&pItem) )
    {
        pInputCfg->SetMoveSelection( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_EDITMODE,&pItem) )
    {
        pInputCfg->SetEnterEdit( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_FMT_EXPAND,&pItem) )
    {
        pInputCfg->SetExtendFormat( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_RANGEFINDER,&pItem) )
    {
        pInputCfg->SetRangeFinder( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_REF_EXPAND,&pItem) )
    {
        pInputCfg->SetExpandRefs( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_MARK_HEADER,&pItem) )
    {
        pInputCfg->SetMarkHeader( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
        bUpdateMarks = true;
    }
    if ( rOptSet.HasItem(SID_SC_INPUT_TEXTWYSIWYG,&pItem) )
    {
        sal_Bool bNew = ((const SfxBoolItem*)pItem)->GetValue();
        if ( bNew != ( pInputCfg->GetTextWysiwyg() ? 1 : 0 ) )
        {
            pInputCfg->SetTextWysiwyg( bNew );
            bSaveInputOptions = true;
            bUpdateRefDev = true;
        }
    }
    if( rOptSet.HasItem( SID_SC_INPUT_REPLCELLSWARN, &pItem ) )
    {
        pInputCfg->SetReplaceCellsWarn( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    if( rOptSet.HasItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, &pItem ) )
    {
        pInputCfg->SetLegacyCellSelection( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = true;
    }

    
    
    

    if ( rOptSet.HasItem(SID_SCPRINTOPTIONS,&pItem) )
    {
        const ScPrintOptions& rNewOpt = ((const ScTpPrintItem*)pItem)->GetPrintOptions();
        SetPrintOptions( rNewOpt );

        
        SFX_APP()->Broadcast( SfxSimpleHint( SID_SCPRINTOPTIONS ) );
    }

    

    if ( bSaveAppOptions )
        pAppCfg->OptionsChanged();

    if ( bSaveInputOptions )
        pInputCfg->OptionsChanged();

    

    if (pDoc && bCompileErrorCells)
    {
        
        
        
        if (pDoc->CompileErrorCells(ScErrorCodes::errNoName))
            bCalcAll = true;
    }

    if ( pDoc && bCalcAll )
    {
        WaitObject aWait( pDocSh->GetActiveDialogParent() );
        pDoc->CalcAll();
        if ( pViewSh )
            pViewSh->UpdateCharts( true );
        else
            ScDBFunc::DoUpdateCharts( ScAddress(), pDoc, true );
        if (pBindings)
            pBindings->Invalidate( SID_ATTR_SIZE ); 
    }

    if ( pViewSh && bUpdateMarks )
        pViewSh->UpdateAutoFillMark();

    

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
            pBindings->Invalidate( FID_TOGGLEHEADERS ); 
            pBindings->Invalidate( FID_TOGGLESYNTAX );
        }
    }

    

    if ( bUpdateRefDev )
    {
        
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
        while ( pObjSh )
        {
            if ( pObjSh->Type() == TYPE(ScDocShell) )
            {
                ScDocShell* pOneDocSh = ((ScDocShell*)pObjSh);
                pOneDocSh->CalcOutputFactor();
                SCTAB nTabCount = pOneDocSh->GetDocument()->GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    pOneDocSh->AdjustRowHeight( 0, MAXROW, nTab );
            }
            pObjSh = SfxObjectShell::GetNext( *pObjSh );
        }

        
        TypeId aScType = TYPE(ScTabViewShell);
        SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
        while ( pSh )
        {
            ScTabViewShell* pOneViewSh = (ScTabViewShell*)pSh;

            
            ScInputHandler* pHdl = GetInputHdl(pOneViewSh);
            if (pHdl)
                pHdl->UpdateRefDevice();

            
            ScViewData* pViewData = pOneViewSh->GetViewData();
            pOneViewSh->SetZoom( pViewData->GetZoomX(), pViewData->GetZoomY(), false );

            
            pOneViewSh->PaintGrid();
            pOneViewSh->PaintTop();
            pOneViewSh->PaintLeft();

            pSh = SfxViewShell::GetNext( *pSh, &aScType );
        }
    }
}


//

//


ScInputHandler* ScModule::GetInputHdl( ScTabViewShell* pViewSh, bool bUseRef )
{
    if ( pRefInputHandler && bUseRef )
        return pRefInputHandler;

    ScInputHandler* pHdl = NULL;
    if ( !pViewSh )
    {
        
        

        ScTabViewShell* pCurViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        if ( pCurViewSh && !pCurViewSh->GetUIActiveClient() )
            pViewSh = pCurViewSh;
    }

    if ( pViewSh )
        pHdl = pViewSh->GetInputHandler();      

    
    OSL_ENSURE( pHdl || !pViewSh, "GetInputHdl: kein InputHandler gefunden" );
    return pHdl;
}

void ScModule::ViewShellChanged()
{
    ScInputHandler* pHdl   = GetInputHdl();
    ScTabViewShell* pShell = ScTabViewShell::GetActiveViewShell();
    if ( pShell && pHdl )
        pShell->UpdateInputHandler();
}

void ScModule::SetInputMode( ScInputMode eMode )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->SetMode( eMode );
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
    return ( pHdl ? pHdl->KeyInput( rKEvt, bStartEdit ) : false );
}

void ScModule::InputEnterHandler( sal_uInt8 nBlockMode )
{
    if ( !SFX_APP()->IsDowning() )                                  
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

ScInputHandler* ScModule::GetRefInputHdl()
{
    return pRefInputHandler;
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
            
            if ( pWin )
            {
                pWin->SetFuncString( *pStrFormula, false );
                
            }
            sal_uInt8 nMode = bMatrix ? SC_ENTER_MATRIX : SC_ENTER_NORMAL;
            pHdl->EnterHandler( nMode );

            
            if (pWin)
                pWin->TextInvalidate();
        }
        else
        {
            
            if ( pWin )
            {
                pWin->SetFuncString( EMPTY_OUSTRING, false );
                
            }
            pHdl->CancelHandler();
        }
    }
}


//

//


void ScModule::SetRefDialog( sal_uInt16 nId, bool bVis, SfxViewFrame* pViewFrm )
{
    
    

    if(nCurRefDlgId==0 || (nId==nCurRefDlgId && !bVis))
    {
        if ( !pViewFrm )
            pViewFrm = SfxViewFrame::Current();

        
        
        
        

        nCurRefDlgId = bVis ? nId : 0 ;             

        if ( pViewFrm )
        {
            
            SfxViewShell* pViewSh = pViewFrm->GetViewShell();
            if ( pViewSh && pViewSh->ISA( ScTabViewShell ) )
                ((ScTabViewShell*)pViewSh)->SetCurRefDlgId( nCurRefDlgId );
            else
            {
                
                bVis = false;
                nCurRefDlgId = 0;   
            }

            pViewFrm->SetChildWindow( nId, bVis );
        }

        SfxApplication* pSfxApp = SFX_APP();
        pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
    }
}

static SfxChildWindow* lcl_GetChildWinFromAnyView( sal_uInt16 nId )
{
    

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    
    SfxChildWindow* pChildWnd = pViewFrm ? pViewFrm->GetChildWindow( nId ) : NULL;
    if ( pChildWnd )
        return pChildWnd;           

    
    

    pViewFrm = SfxViewFrame::GetFirst();
    while ( pViewFrm )
    {
        pChildWnd = pViewFrm->GetChildWindow( nId );
        if ( pChildWnd )
            return pChildWnd;       

        pViewFrm = SfxViewFrame::GetNext( *pViewFrm );
    }

    return NULL;                    
}

bool ScModule::IsModalMode(SfxObjectShell* pDocSh)
{
    
    

    bool bIsModal = false;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            bIsModal = pChildWnd->IsVisible() &&
                !( pRefDlg->IsRefInputMode() && pRefDlg->IsDocAllowed(pDocSh) );
        }
        else if(pModalDlg)
        {
            bIsModal = pModalDlg->IsVisible() && !(pModalDlg->IsRefInputMode() && pModalDlg->IsDocAllowed(pDocSh) );
        }
        else
        {
            
            

            bIsModal = true;
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
    
    

    bool bLocked = false;

    

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        if ( pChildWnd )
            bLocked = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow())->IsTableLocked();
        else if( pModalDlg )
            bLocked = pModalDlg->IsTableLocked();
        else
            bLocked = true;     
    }

    return bLocked;
}

bool ScModule::IsRefDialogOpen()
{
    
    

    bool bIsOpen = false;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        if ( pChildWnd )
            bIsOpen = pChildWnd->IsVisible();
        else if(pModalDlg)
            bIsOpen = pModalDlg->IsVisible();
        else
            bIsOpen = true;     
    }

    return bIsOpen;
}

bool ScModule::IsFormulaMode()
{
    
    

    bool bIsFormula = false;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            bIsFormula = pChildWnd->IsVisible() && pRefDlg->IsRefInputMode();
        }
        else if(pModalDlg)
        {
            bIsFormula = pModalDlg->IsVisible() && pModalDlg->IsRefInputMode();
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
    
    

    
    

    ScRange aNew = rRef;
    aNew.Justify();                 

    if( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        OSL_ENSURE( pChildWnd || pModalDlg, "NoChildWin" );
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

            
            
            pRefDlg->HideReference( false );
            pRefDlg->SetReference( aNew, pDoc );
        }
        else if(pModalDlg)
        {
            
            
            pModalDlg->HideReference( false );
            pModalDlg->SetReference( aNew, pDoc );
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if (pHdl)
            pHdl->SetReference( aNew, pDoc );
        else
        {
            OSL_FAIL("SetReference ohne Empfaenger");
        }
    }
}

void ScModule::AddRefEntry()                        
{
    
    

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        OSL_ENSURE( pChildWnd || pModalDlg, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            pRefDlg->AddRefEntry();
        }
        else if(pModalDlg)
            pModalDlg->AddRefEntry();
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
    
    

    

    

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        ScAnyRefModalDlg* pModalDlg = GetCurrentAnyRefDlg();
        OSL_ENSURE( pChildWnd || pModalDlg, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            pRefDlg->SetActive();
        }
        else
            pModalDlg->SetActive();
    }
}


//

//


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
        ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
        if ( pViewSh )
            pViewSh->CheckNeedsRepaint();
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
    }
}

IMPL_LINK_NOARG(ScModule, IdleHandler)
{
    if ( Application::AnyInput( VCL_INPUT_MOUSEANDKEYBOARD ) )
    {
        aIdleTimer.Start();         
        return 0;
    }

    bool bMore = false;
    bool bAutoSpell = false;
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(SfxObjectShell::Current());

    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        bAutoSpell = pDoc->GetDocOptions().IsAutoSpell();
        if (pDocSh->IsReadOnly())
            bAutoSpell = false;

        sc::DocumentLinkManager& rLinkMgr = pDoc->GetDocLinkManager();
        bool bLinks = rLinkMgr.idleCheckLinks();
        sal_Bool bWidth = pDoc->IdleCalcTextWidth();

        bMore = bLinks || bWidth;         

        
        
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
                aSpellTimer.Start();
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
    return 0;
}

IMPL_LINK_NOARG(ScModule, SpellTimerHdl)
{
    if ( Application::AnyInput( VCL_INPUT_KEYBOARD ) )
    {
        aSpellTimer.Start();
        return 0;                   
    }

    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    if (pViewSh)
    {
        if (pViewSh->ContinueOnlineSpelling())
            aSpellTimer.Start();
    }
    return 0;
}

    
SfxItemSet*  ScModule::CreateItemSet( sal_uInt16 nId )
{
    SfxItemSet*  pRet = 0;
    if(SID_SC_EDITOPTIONS == nId)
    {
        pRet = new SfxItemSet( GetPool(),
                            
                            SID_SCDOCOPTIONS,       SID_SCDOCOPTIONS,
                            
                            SID_SCVIEWOPTIONS,      SID_SCVIEWOPTIONS,
                            SID_SC_OPT_SYNCZOOM,    SID_SC_OPT_SYNCZOOM,
                            
                            SID_SC_INPUT_SELECTION,SID_SC_INPUT_MARK_HEADER,
                            SID_SC_INPUT_TEXTWYSIWYG,SID_SC_INPUT_TEXTWYSIWYG,
                            SID_SC_INPUT_REPLCELLSWARN,SID_SC_INPUT_REPLCELLSWARN,
                            SID_SC_INPUT_LEGACY_CELL_SELECTION,SID_SC_INPUT_LEGACY_CELL_SELECTION,
                            
                            SCITEM_USERLIST,        SCITEM_USERLIST,
                            
                            SID_SCPRINTOPTIONS, SID_SCPRINTOPTIONS,
                            
                            SID_ATTR_GRID_OPTIONS, SID_ATTR_GRID_OPTIONS,
                            //
                            SID_ATTR_METRIC,        SID_ATTR_METRIC,
                            SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                            
                            SID_SC_OPT_KEY_BINDING_COMPAT, SID_SC_OPT_KEY_BINDING_COMPAT,
                            
                            SID_SCDEFAULTSOPTIONS, SID_SCDEFAULTSOPTIONS,
                            
                            SID_SCFORMULAOPTIONS, SID_SCFORMULAOPTIONS,
                            0 );

        const ScAppOptions& rAppOpt = GetAppOptions();

        ScDocShell*     pDocSh = PTR_CAST(ScDocShell,
                                            SfxObjectShell::Current());
        ScDocOptions    aCalcOpt = pDocSh
                            ? pDocSh->GetDocument()->GetDocOptions()
                            : GetDocOptions();

        ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,
                                            SfxViewShell::Current());
        ScViewOptions   aViewOpt = pViewSh
                            ? pViewSh->GetViewData()->GetOptions()
                            : GetViewOptions();

        ScUserListItem  aULItem( SCITEM_USERLIST );
        ScUserList*     pUL = ScGlobal::GetUserList();

        

        pRet->Put( SfxUInt16Item( SID_ATTR_METRIC,
                        sal::static_int_cast<sal_uInt16>(rAppOpt.GetAppMetric()) ) );

        
        pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP,
                        aCalcOpt.GetTabDistance()));
        pRet->Put( ScTpCalcItem( SID_SCDOCOPTIONS, aCalcOpt ) );

        
        pRet->Put( ScTpViewItem( SID_SCVIEWOPTIONS, aViewOpt ) );
        pRet->Put( SfxBoolItem( SID_SC_OPT_SYNCZOOM, rAppOpt.GetSynchronizeZoom() ) );

        
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
        pRet->Put( SfxBoolItem( SID_SC_INPUT_MARK_HEADER,
                    rInpOpt.GetMarkHeader() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_TEXTWYSIWYG,
                    rInpOpt.GetTextWysiwyg() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN,
                    rInpOpt.GetReplaceCellsWarn() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_LEGACY_CELL_SELECTION,
                    rInpOpt.GetLegacyCellSelection() ) );


        
        pRet->Put( ScTpPrintItem( SID_SCPRINTOPTIONS, GetPrintOptions() ) );

        
        SvxGridItem* pSvxGridItem = aViewOpt.CreateGridItem();
        pRet->Put( *pSvxGridItem );
        delete pSvxGridItem;

        
        if ( pUL )
        {
            aULItem.SetUserList( *pUL );
            pRet->Put(aULItem);
        }

        
        pRet->Put( SfxUInt16Item( SID_SC_OPT_KEY_BINDING_COMPAT,
                                   rAppOpt.GetKeyBindingType() ) );

        
        pRet->Put( ScTpDefaultsItem( SID_SCDEFAULTSOPTIONS, GetDefaultsOptions() ) );

        
        pRet->Put( ScTpFormulaItem( SID_SCFORMULAOPTIONS, GetFormulaOptions() ) );
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

SfxTabPage*  ScModule::CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage* pRet = NULL;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
    switch(nId)
    {
        case SID_SC_TP_LAYOUT:
                                {
                                    ::CreateTabPage ScTpLayoutOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_LAYOUT );
                                    if ( ScTpLayoutOptionsCreate )
                                        pRet =  (*ScTpLayoutOptionsCreate) (pParent, rSet);
                                }
                                break;
        case SID_SC_TP_CONTENT:
                                {
                                    ::CreateTabPage ScTpContentOptionsCreate = pFact->GetTabPageCreatorFunc(RID_SCPAGE_CONTENT);
                                    if ( ScTpContentOptionsCreate )
                                        pRet = (*ScTpContentOptionsCreate)(pParent, rSet);
                                }
                                break;
        case SID_SC_TP_GRID:            pRet = SvxGridTabPage::Create(pParent, rSet); break;
        case SID_SC_TP_USERLISTS:
                                {
                                    ::CreateTabPage ScTpUserListsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_USERLISTS );
                                    if ( ScTpUserListsCreate )
                                            pRet = (*ScTpUserListsCreate)( pParent, rSet);
                                }
                                break;
        case SID_SC_TP_CALC:
                                {
                                                    ::CreateTabPage ScTpCalcOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_CALC );
                                                    if ( ScTpCalcOptionsCreate )
                                                        pRet = (*ScTpCalcOptionsCreate)(pParent, rSet);
                                }
                                break;
        case SID_SC_TP_FORMULA:
        {
            ::CreateTabPage ScTpFormulaOptionsCreate = pFact->GetTabPageCreatorFunc (RID_SCPAGE_FORMULA);
            if (ScTpFormulaOptionsCreate)
                pRet = (*ScTpFormulaOptionsCreate)(pParent, rSet);
        }
        break;
        case SID_SC_TP_COMPATIBILITY:
        {
            ::CreateTabPage ScTpCompatOptionsCreate = pFact->GetTabPageCreatorFunc (RID_SCPAGE_COMPATIBILITY);
            if (ScTpCompatOptionsCreate)
                pRet = (*ScTpCompatOptionsCreate)(pParent, rSet);
        }
        break;
        case SID_SC_TP_CHANGES:
                                {
                                            ::CreateTabPage ScRedlineOptionsTabPageCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_OPREDLINE );
                                            if ( ScRedlineOptionsTabPageCreate )
                                                    pRet =(*ScRedlineOptionsTabPageCreate)(pParent, rSet);
                                }
                        break;
        case RID_SC_TP_PRINT:
                                {
                                    ::CreateTabPage ScTpPrintOptionsCreate =    pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
                                    if ( ScTpPrintOptionsCreate )
                                        pRet = (*ScTpPrintOptionsCreate)( pParent, rSet);
                                }
            break;
        case RID_SC_TP_DEFAULTS:
            {
                ::CreateTabPage ScTpDefaultsOptionsCreate = pFact->GetTabPageCreatorFunc( RID_SCPAGE_DEFAULTS );
                if ( ScTpDefaultsOptionsCreate )
                    pRet = (*ScTpDefaultsOptionsCreate)( pParent, rSet);
            }
            break;
    }

    OSL_ENSURE( pRet, "ScModule::CreateTabPage(): no valid ID for TabPage!" );

    return pRet;
}

IMPL_LINK( ScModule, CalcFieldValueHdl, EditFieldInfo*, pInfo )
{
    

    if (pInfo)
    {
        const SvxFieldItem& rField = pInfo->GetField();
        const SvxFieldData* pField = rField.GetField();

        if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/

            const SvxURLField* pURLField = (const SvxURLField*) pField;
            OUString aURL = pURLField->GetURL();

            switch ( pURLField->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: 
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
            pInfo->SetTxtColor( GetColorConfig().GetColorValue(eEntry).nColor );
        }
        else
        {
            OSL_FAIL("unbekannter Feldbefehl");
            pInfo->SetRepresentation(OUString('?'));
        }
    }

    return 0;
}

bool ScModule::RegisterRefWindow( sal_uInt16 nSlotId, Window *pWnd )
{
    std::list<Window*> & rlRefWindow = m_mapRefWindow[nSlotId];

    if( std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd ) == rlRefWindow.end() )
    {
        rlRefWindow.push_back( pWnd );
        return true;
    }

    return false;
}

bool  ScModule::UnregisterRefWindow( sal_uInt16 nSlotId, Window *pWnd )
{
    std::map<sal_uInt16, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return false;

    std::list<Window*> & rlRefWindow = iSlot->second;

    std::list<Window*>::iterator i = std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );

    if( i == rlRefWindow.end() )
        return false;

    rlRefWindow.erase( i );

    if( rlRefWindow.empty() )
        m_mapRefWindow.erase( nSlotId );

    return true;
}

bool  ScModule::IsAliveRefDlg( sal_uInt16 nSlotId, Window *pWnd )
{
    std::map<sal_uInt16, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return false;

    std::list<Window*> & rlRefWindow = iSlot->second;

    return rlRefWindow.end() != std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );
}

Window *  ScModule::Find1RefWindow( sal_uInt16 nSlotId, Window *pWndAncestor )
{
    if (!pWndAncestor)
        return NULL;

    std::map<sal_uInt16, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return NULL;

    std::list<Window*> & rlRefWindow = iSlot->second;

    while( Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for( std::list<Window*>::iterator i = rlRefWindow.begin(); i!=rlRefWindow.end(); ++i )
        if ( pWndAncestor->IsWindowOrChild( *i, (*i)->IsSystemWindow() ) )
            return *i;

    return NULL;
}

ScAnyRefModalDlg* ScModule::GetCurrentAnyRefDlg()
{
    if(!maAnyRefDlgStack.empty())
        return maAnyRefDlgStack.top();

    return NULL;
}

void ScModule::PushNewAnyRefDlg( ScAnyRefModalDlg* pNewDlg )
{
    maAnyRefDlgStack.push( pNewDlg );

    
    
    if(maAnyRefDlgStack.size() != 1)
        return;

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while(pViewShell)
    {
        if ( pViewShell->ISA(ScTabViewShell) )
        {
            ScTabViewShell* pViewSh = (ScTabViewShell*)pViewShell;
            pViewSh->SetInRefMode( true );
        }
        pViewShell = SfxViewShell::GetNext( *pViewShell );
    }
}

void ScModule::PopAnyRefDlg()
{
    maAnyRefDlgStack.pop();

    if(maAnyRefDlgStack.empty())
    {
        
        

        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while(pViewShell)
        {
            if ( pViewShell->ISA(ScTabViewShell) )
            {
                ScTabViewShell* pViewSh = (ScTabViewShell*)pViewShell;
                pViewSh->SetInRefMode( false );
            }
            pViewShell = SfxViewShell::GetNext( *pViewShell );
        }

    }
}

using namespace com::sun::star;

#define LINGUPROP_AUTOSPELL         "IsSpellAuto"

void ScModule::GetSpellSettings( sal_uInt16& rDefLang, sal_uInt16& rCjkLang, sal_uInt16& rCtlLang,
        bool& rAutoSpell )
{
    
    
    SvtLinguConfig aConfig;

    SvtLinguOptions aOptions;
    aConfig.GetOptions( aOptions );

    rDefLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN);
    rCjkLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN);
    rCtlLang = MsLangId::resolveSystemLanguageByScriptType(aOptions.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
    rAutoSpell = aOptions.bIsSpellAuto;
}

void ScModule::SetAutoSpellProperty( bool bSet )
{
    
    
    SvtLinguConfig aConfig;

    uno::Any aAny;
    aAny <<= bSet;
    aConfig.SetProperty( OUString( LINGUPROP_AUTOSPELL ), aAny );
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
