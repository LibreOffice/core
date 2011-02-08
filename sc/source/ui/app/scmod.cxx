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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <comphelper/processfactory.hxx>





// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/flditem.hxx>
#include <editeng/outliner.hxx>
#include <basic/sbstar.hxx>

#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#include <svx/hyprlink.hxx>
#include "IAnyRefDialog.hxx"

#include <svtools/ehdl.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/status.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/macrconf.hxx>
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



#include "scmod.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "docoptio.hxx"
#include "appoptio.hxx"
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
#include "anyrefdg.hxx"
#include "dwfunctr.hxx"
#include "formdata.hxx"
#include "tpprint.hxx"
#include "transobj.hxx"
#include "detfunc.hxx"
#include "preview.hxx"

#include <svx/xmlsecctrl.hxx>


#define ScModule
#include "scslots.hxx"

#include "scabstdlg.hxx"

#define SC_IDLE_MIN     150
#define SC_IDLE_MAX     3000
#define SC_IDLE_STEP    75
#define SC_IDLE_COUNT   50

static USHORT nIdleCount = 0;

//------------------------------------------------------------------

SFX_IMPL_INTERFACE( ScModule, SfxShell, ScResId(RID_APPTITLE) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
                                ScResId(RID_OBJECTBAR_APP) );
    SFX_STATUSBAR_REGISTRATION( ScResId(SCCFG_STATUSBAR) );     // nur ID wichtig
}

//------------------------------------------------------------------

ScModule::ScModule( SfxObjectFactory* pFact ) :
    SfxModule( SfxApplication::CreateResManager( "sc" ), FALSE, pFact, NULL ),
    pSelTransfer( NULL ),
    pMessagePool( NULL ),
    pRefInputHandler( NULL ),
    pViewCfg( NULL ),
    pDocCfg( NULL ),
    pAppCfg( NULL ),
    pInputCfg( NULL ),
    pPrintCfg( NULL ),
    pNavipiCfg( NULL ),
    pAddInCfg( NULL ),
    pColorConfig( NULL ),
    pAccessOptions( NULL ),
    pCTLOptions( NULL ),
    pUserOptions( NULL ),
    pErrorHdl( NULL ),
    pSvxErrorHdl( NULL ),
    pFormEditData( NULL ),
    nCurRefDlgId( 0 ),
    bIsWaterCan( FALSE ),
    bIsInEditCommand( FALSE ),
    bIsInExecuteDrop( FALSE ),
    mbIsInSharedDocLoading( false ),
    mbIsInSharedDocSaving( false )
{
    //  im ctor ist der ResManager (DLL-Daten) noch nicht initialisiert!

    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc")));       // fuer Basic

    ResetDragObject();
    SetClipObject( NULL, NULL );

    //  InputHandler braucht nicht mehr angelegt zu werden

    //  ErrorHandler anlegen - war in Init()
    //  zwischen OfficeApplication::Init und ScGlobal::Init
    SvxErrorHandler::Get();
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

    StartListening( *SFX_APP() );       // for SFX_HINT_DEINITIALIZING
}

ScModule::~ScModule()
{
    DBG_ASSERT( !pSelTransfer, "Selection Transfer object not deleted" );

    //  InputHandler braucht nicht mehr geloescht zu werden (gibt keinen an der App mehr)

    SfxItemPool::Free(pMessagePool);

    DELETEZ( pFormEditData );

    delete pErrorHdl;
//  delete pSvxErrorHdl;

    ScGlobal::Clear();      // ruft auch ScDocumentPool::DeleteVersionMaps();

    DeleteCfg();            // wurde mal aus Exit() gerufen
}

//------------------------------------------------------------------
void ScModule::ConfigurationChanged( utl::ConfigurationBroadcaster* p, sal_uInt32 )
{
    if ( p == pColorConfig || p == pAccessOptions )
    {
        //  Test if detective objects have to be updated with new colors
        //  (if the detective colors haven't been used yet, there's nothing to update)
        if ( ScDetectiveFunc::IsColorsInitialized() )
        {
            const svtools::ColorConfig& rColors = GetColorConfig();
            BOOL bArrows =
                ( ScDetectiveFunc::GetArrowColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVE).nColor ||
                  ScDetectiveFunc::GetErrorColor() != (ColorData)rColors.GetColorValue(svtools::CALCDETECTIVEERROR).nColor );
            BOOL bComments =
                ( ScDetectiveFunc::GetCommentColor() != (ColorData)rColors.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor );
            if ( bArrows || bComments )
            {
                ScDetectiveFunc::InitializeColors();        // get the new colors

                //  update detective objects in all open documents
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

        //  force all views to repaint, using the new options

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
                    pHdl->ForgetLastPattern();  // EditEngine BackgroundColor may change
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
        //  for all documents: set digit language for printer, recalc output factor, update row heights
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

        //  for all views (table and preview): update digit language
        SfxViewShell* pSh = SfxViewShell::GetFirst();
        while ( pSh )
        {
            if ( pSh->ISA( ScTabViewShell ) )
            {
                ScTabViewShell* pViewSh = (ScTabViewShell*)pSh;

                //  set ref-device for EditEngine (re-evaluates digit settings)
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
        ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
        if ( nHintId == SFX_HINT_DEINITIALIZING )
        {
            //  ConfigItems must be removed before ConfigManager
            DeleteCfg();
        }
    }
}

//------------------------------------------------------------------

void ScModule::DeleteCfg()
{
    DELETEZ( pViewCfg ); // Speichern passiert vor Exit() automatisch
    DELETEZ( pDocCfg );
    DELETEZ( pAppCfg );
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

//------------------------------------------------------------------

#define TEXT_WIDTH(s)   rStatusBar.GetTextWidth((s))

void ScModule::FillStatusBar(StatusBar& rStatusBar)
{
    // Dokumentposition (Tabelle x / y)
    rStatusBar.InsertItem( SID_STATUS_DOCPOS,
                            TEXT_WIDTH( String().Fill( 10, 'X' ) ),
                            SIB_LEFT|SIB_AUTOSIZE );
    rStatusBar.SetHelpId( SID_STATUS_DOCPOS, SID_STATUS_DOCPOS );

    // Seitenvorlage
    rStatusBar.InsertItem( SID_STATUS_PAGESTYLE,
                            TEXT_WIDTH( String().Fill( 15, 'X' ) ),
                            SIB_LEFT|SIB_AUTOSIZE );
    rStatusBar.SetHelpId( SID_STATUS_PAGESTYLE, SID_STATUS_PAGESTYLE );

    // Einfuege-/Ueberschreibmodus
    rStatusBar.InsertItem( SID_ATTR_INSERT,
                            SvxInsertStatusBarControl::GetDefItemWidth(rStatusBar),
                            SIB_CENTER );
    rStatusBar.SetHelpId( SID_ATTR_INSERT, SID_ATTR_INSERT );

    // Selektionsmodus
    rStatusBar.InsertItem( SID_STATUS_SELMODE,
                            SvxSelectionModeControl::GetDefItemWidth(rStatusBar),
                            SIB_CENTER );
    rStatusBar.SetHelpId( SID_STATUS_SELMODE, SID_STATUS_SELMODE );

    // Dokument geaendert
    rStatusBar.InsertItem( SID_DOC_MODIFIED,
                            SvxModifyControl::GetDefItemWidth(rStatusBar));

    // signatures
    rStatusBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rStatusBar ), SIB_USERDRAW );
    rStatusBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);


    rStatusBar.SetHelpId( SID_DOC_MODIFIED, SID_DOC_MODIFIED );

    // den aktuellen Kontext anzeigen Uhrzeit / FramePos / TabellenInfo / Errors
    rStatusBar.InsertItem( SID_ATTR_SIZE,
                            SvxPosSizeStatusBarControl::GetDefItemWidth(rStatusBar),
                            SIB_AUTOSIZE|SIB_LEFT|SIB_USERDRAW);
    rStatusBar.SetHelpId( SID_ATTR_SIZE, SID_ATTR_SIZE );

    // Ma"sstab
    rStatusBar.InsertItem( SID_ATTR_ZOOM,
        SvxZoomStatusBarControl::GetDefItemWidth(rStatusBar),
        SIB_CENTER );
    rStatusBar.SetHelpId( SID_ATTR_ZOOM, SID_ATTR_ZOOM );

    // ZoomSlider
    rStatusBar.InsertItem( SID_ATTR_ZOOMSLIDER,
        TEXT_WIDTH( String().Fill( 15, 'X' ) ),
        SIB_CENTER );
    rStatusBar.SetHelpId( SID_ATTR_ZOOMSLIDER, SID_ATTR_ZOOMSLIDER );
}

#undef TEXT_WIDTH

//------------------------------------------------------------------
//
//      von der Applikation verschoben:
//
//------------------------------------------------------------------

void ScModule::Execute( SfxRequest& rReq )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : NULL;

    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

    switch ( nSlot )
    {
        case SID_CHOOSE_DESIGN:
            {
                String aMacroName =
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Template.Samples.ShowStyles"));
                SfxApplication* pApp = SFX_APP();
                pApp->EnterBasicCall();
                pApp->GetMacroConfig()->Call( NULL, aMacroName, pApp->GetBasicManager() );
                pApp->LeaveBasicCall();
            }
            break;
        case SID_EURO_CONVERTER:
            {
                String aMacroName =
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Euro.ConvertRun.Main"));
                SfxApplication* pApp = SFX_APP();
                pApp->EnterBasicCall();
                pApp->GetMacroConfig()->Call( NULL, aMacroName, pApp->GetBasicManager() );
                pApp->LeaveBasicCall();
            }
            break;
        case SID_AUTOSPELL_CHECK:
            {
                BOOL bSet;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( nSlot, TRUE, &pItem ) )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                else
                {                       //  Toggle
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
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( nSlot, TRUE, &pItem ) )
                {
                    FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
                    switch( eUnit )
                    {
                        case FUNIT_MM:      // nur die Einheiten, die auch im Dialog stehen
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
                BOOL bNew = !aNewOpts.GetAutoComplete();
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
                BOOL bNew = !aNewOpts.GetDetectiveAuto();
                SFX_REQUEST_ARG( rReq, pAuto, SfxBoolItem, SID_DETECTIVE_AUTO, sal_False );
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
                DBG_ASSERT(rItem.ISA(SfxUInt16Item),"falscher Parameter");

                ScAppOptions aNewOpts( GetAppOptions() );
                aNewOpts.SetStatusFunc( rItem.GetValue() );
                SetAppOptions( aNewOpts );

                if (pBindings)
                {
                    pBindings->Invalidate( SID_TABLE_CELL );
                    pBindings->Update( SID_TABLE_CELL );            // sofort

                    pBindings->Invalidate( SID_PSZ_FUNCTION );
                    pBindings->Update( SID_PSZ_FUNCTION );
                    // falls Menue gleich wieder aufgeklappt wird
                }
            }
            break;

        case SID_ATTR_LANGUAGE:
        case SID_ATTR_CHAR_CJK_LANGUAGE:
        case SID_ATTR_CHAR_CTL_LANGUAGE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( GetPool().GetWhich(nSlot), TRUE, &pItem ) )
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
                                pInputHandler->UpdateSpellSettings();   // EditEngine-Flags
                            ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
                            if ( pViewSh )
                                pViewSh->UpdateDrawTextOutliner();      // EditEngine-Flags

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
                com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XExecutableDialog > xDialog(::comphelper::getProcessServiceFactory()->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ui.XSLTFilterDialog"))), com::sun::star::uno::UNO_QUERY);
                if( xDialog.is() )
                {
                    xDialog->execute();
                }
            }
            catch( ::com::sun::star::uno::RuntimeException& )
            {
            }
        }
        break;

        default:
            DBG_ERROR( "ScApplication: Unknown Message." );
            break;
    }
}

void ScModule::GetState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
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
                rSet.Put( SfxUInt16Item( nWhich, sal::static_int_cast<UINT16>(GetAppOptions().GetAppMetric()) ) );
                break;
            case SID_AUTOSPELL_CHECK:
                {
                    BOOL bAuto;
                    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
                    if ( pDocSh )
                        bAuto = pDocSh->GetDocument()->GetDocOptions().IsAutoSpell();
                    else
                    {
                        USHORT nDummyLang, nDummyCjk, nDummyCtl;
                        GetSpellSettings( nDummyLang, nDummyCjk, nDummyCtl, bAuto );
                    }
                    rSet.Put( SfxBoolItem( nWhich, bAuto ) );
                }
                break;
            case SID_ATTR_LANGUAGE:
            case ATTR_CJK_FONT_LANGUAGE:        // WID for SID_ATTR_CHAR_CJK_LANGUAGE
            case ATTR_CTL_FONT_LANGUAGE:        // WID for SID_ATTR_CHAR_CTL_LANGUAGE
                {
                    ScDocShell* pDocSh = PTR_CAST(ScDocShell, SfxObjectShell::Current());
                    ScDocument* pDoc = pDocSh ? pDocSh->GetDocument() : NULL;
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
        nWhich = aIter.NextWhich();
    }
}


void ScModule::HideDisabledSlots( SfxItemSet& rSet )
{
    if( SfxViewFrame* pViewFrm = SfxViewFrame::Current() )
    {
        SfxBindings& rBindings = pViewFrm->GetBindings();
        SfxWhichIter aIter( rSet );
        for( USHORT nWhich = aIter.FirstWhich(); nWhich != 0; nWhich = aIter.NextWhich() )
        {
            ScViewUtil::HideDisabledSlot( rSet, rBindings, nWhich );
            // always disable the slots
            rSet.DisableItem( nWhich );
        }
    }
}


//------------------------------------------------------------------

void ScModule::ResetDragObject()
{
    aDragData.pCellTransfer = NULL;
    aDragData.pDrawTransfer = NULL;

    aDragData.aLinkDoc.Erase();
    aDragData.aLinkTable.Erase();
    aDragData.aLinkArea.Erase();
    aDragData.pJumpLocalDoc = NULL;
    aDragData.aJumpTarget.Erase();
    aDragData.aJumpText.Erase();
}

void ScModule::SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    ResetDragObject();
    aDragData.pCellTransfer = pCellObj;
    aDragData.pDrawTransfer = pDrawObj;
}

void ScModule::SetDragLink( const String& rDoc, const String& rTab, const String& rArea )
{
    ResetDragObject();

    aDragData.aLinkDoc   = rDoc;
    aDragData.aLinkTable = rTab;
    aDragData.aLinkArea  = rArea;
}

void ScModule::SetDragJump( ScDocument* pLocalDoc, const String& rTarget, const String& rText )
{
    ResetDragObject();

    aDragData.pJumpLocalDoc = pLocalDoc;
    aDragData.aJumpTarget = rTarget;
    aDragData.aJumpText = rText;
}

//------------------------------------------------------------------

void ScModule::SetClipObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj )
{
    DBG_ASSERT( !pCellObj || !pDrawObj, "SetClipObject: not allowed to set both objects" );

    aClipData.pCellClipboard = pCellObj;
    aClipData.pDrawClipboard = pDrawObj;
}

ScDocument* ScModule::GetClipDoc()
{
    //  called from document

    ScTransferObj* pObj = ScTransferObj::GetOwnClipboard( NULL );
    if (pObj)
        return pObj->GetDocument();

    return NULL;
}

//------------------------------------------------------------------

void ScModule::SetSelectionTransfer( ScSelectionTransferObj* pNew )
{
    pSelTransfer = pNew;
}

//------------------------------------------------------------------

void ScModule::InitFormEditData()
{
    pFormEditData = new ScFormEditData;
}

void ScModule::ClearFormEditData()
{
    DELETEZ( pFormEditData );
}

//------------------------------------------------------------------

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

void ScModule::InsertEntryToLRUList(USHORT nFIndex)
{
    if(nFIndex != 0)
    {
        const ScAppOptions& rAppOpt = GetAppOptions();
        USHORT nLRUFuncCount = Min( rAppOpt.GetLRUFuncListCount(), (USHORT)LRU_MAX );
        USHORT* pLRUListIds = rAppOpt.GetLRUFuncList();

        USHORT  aIdxList[LRU_MAX];
        USHORT  n = 0;
        BOOL    bFound = FALSE;

        while ((n < LRU_MAX) && n<nLRUFuncCount)                        // alte Liste abklappern
        {
            if (!bFound && (pLRUListIds[n]== nFIndex))
                bFound = TRUE;                                          // erster! Treffer
            else if (bFound)
                aIdxList[n  ] = pLRUListIds[n];                         // hinter Treffer kopieren
            else if ((n+1) < LRU_MAX)
                aIdxList[n+1] = pLRUListIds[n];                         // vor Treffer verschieben
            n++;
        }
        if (!bFound && (n < LRU_MAX))                                   // Eintrag nicht gefunden?
            n++;                                                        //  einen mehr
        aIdxList[0] = nFIndex;                                          // Current on Top

        ScAppOptions aNewOpts(rAppOpt);                                 // an App melden
        aNewOpts.SetLRUFuncList(aIdxList, n);
        SetAppOptions(aNewOpts);

        RecentFunctionsChanged();
    }
}

void ScModule::RecentFunctionsChanged()
{
    //  update function list window
    USHORT nFuncListID = ScFunctionChildWindow::GetChildWindowId();

    //! notify all views
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

USHORT ScModule::GetOptDigitLanguage()
{
    SvtCTLOptions::TextNumerals eNumerals = GetCTLOptions().GetCTLTextNumerals();
    return ( eNumerals == SvtCTLOptions::NUMERALS_ARABIC ) ? LANGUAGE_ENGLISH_US :
           ( eNumerals == SvtCTLOptions::NUMERALS_HINDI)   ? LANGUAGE_ARABIC_SAUDI_ARABIA :
                                                             LANGUAGE_SYSTEM;
}

//------------------------------------------------------------------
//
//                          Optionen
//
//------------------------------------------------------------------

//
//      ModifyOptions - Items aus Calc-Options-Dialog
//                      und SID_AUTOSPELL_CHECK
//

#define IS_AVAILABLE(w,item) (SFX_ITEM_SET==rOptSet.GetItemState((w),TRUE,&item))

void ScModule::ModifyOptions( const SfxItemSet& rOptSet )
{
    USHORT nOldSpellLang, nOldCjkLang, nOldCtlLang;
    BOOL bOldAutoSpell;
    GetSpellSettings( nOldSpellLang, nOldCjkLang, nOldCtlLang, bOldAutoSpell );

    if (!pAppCfg)
        GetAppOptions();
    DBG_ASSERT( pAppCfg, "AppOptions not initialised :-(" );

    if (!pInputCfg)
        GetInputOptions();
    DBG_ASSERT( pInputCfg, "InputOptions not initialised :-(" );

    //--------------------------------------------------------------

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    SfxBindings* pBindings = pViewFrm ? &pViewFrm->GetBindings() : NULL;

    ScTabViewShell*         pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    ScDocShell*             pDocSh  = PTR_CAST(ScDocShell, SfxObjectShell::Current());
    ScDocument*             pDoc    = pDocSh ? pDocSh->GetDocument() : NULL;
    const SfxPoolItem*      pItem   = NULL;
    BOOL                    bRepaint            = FALSE;
    BOOL                    bUpdateMarks        = FALSE;
    BOOL                    bUpdateRefDev       = FALSE;
    BOOL                    bCalcAll            = FALSE;
    BOOL                    bSaveAppOptions     = FALSE;
    BOOL                    bSaveInputOptions   = FALSE;

    //--------------------------------------------------------------------------

    //  SFX_APP()->SetOptions( rOptSet );

    //  Linguistik nicht mehr

    if ( IS_AVAILABLE(SID_ATTR_METRIC,pItem) )
    {
        PutItem( *pItem );
        pAppCfg->SetAppMetric( (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue() );
        bSaveAppOptions = TRUE;
    }

    if ( IS_AVAILABLE(SCITEM_USERLIST,pItem) )
    {
        ScGlobal::SetUserList( ((const ScUserListItem*)pItem)->GetUserList() );
        bSaveAppOptions = TRUE;
    }

    if ( IS_AVAILABLE(SID_SC_OPT_SYNCZOOM,pItem) )
    {
        pAppCfg->SetSynchronizeZoom( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
        bSaveAppOptions = TRUE;
    }

    //============================================
    // ViewOptions
    //============================================

    if ( IS_AVAILABLE(SID_SCVIEWOPTIONS,pItem) )
    {
        const ScViewOptions& rNewOpt = ((const ScTpViewItem*)pItem)->GetViewOptions();

        if ( pViewSh )
        {
            ScViewData*             pViewData = pViewSh->GetViewData();
            const ScViewOptions&    rOldOpt   = pViewData->GetOptions();

            BOOL bAnchorList = ( rOldOpt.GetOption( VOPT_ANCHOR ) !=
                                 rNewOpt.GetOption( VOPT_ANCHOR ) );

            if ( rOldOpt != rNewOpt )
            {
                pViewData->SetOptions( rNewOpt );   // veraendert rOldOpt
                pViewData->GetDocument()->SetViewOptions( rNewOpt );
                pDocSh->SetDocumentModified();
                bRepaint = TRUE;
            }
            if ( bAnchorList )
                pViewSh->UpdateAnchorHandles();
        }
        SetViewOptions( rNewOpt );
        if (pBindings)
            pBindings->Invalidate(SID_HELPLINES_MOVE);
    }

    //============================================
    // GridOptions, Auswertung nach ViewOptions,
    // da GridOptions Member der ViewOptions ist!
    //============================================

    if ( IS_AVAILABLE(SID_ATTR_GRID_OPTIONS,pItem) )
    {
        ScGridOptions aNewGridOpt( (const SvxOptionsGrid&)((const SvxGridItem&)*pItem) );

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
                bRepaint = TRUE;
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


    //============================================
    // DocOptions
    //============================================

    if ( IS_AVAILABLE(SID_SCDOCOPTIONS,pItem) )
    {
        const ScDocOptions& rNewOpt = ((const ScTpCalcItem*)pItem)->GetDocOptions();

        if ( pDoc )
        {
            const ScDocOptions& rOldOpt = pDoc->GetDocOptions();
            ScOptionsUtil::KeyBindingType eKeyOld = rOldOpt.GetKeyBindingType();
            ScOptionsUtil::KeyBindingType eKeyNew = rNewOpt.GetKeyBindingType();
            if (eKeyOld != eKeyNew)
                pDocSh->ResetKeyBindings(eKeyNew);

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

    // nach den eigentlichen DocOptions auch noch die TabDistance setzen
    if ( IS_AVAILABLE(SID_ATTR_DEFTABSTOP,pItem) )
    {
        USHORT nTabDist = ((SfxUInt16Item*)pItem)->GetValue();
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

    //  AutoSpell nach den Doc-Options (weil Member)

    if ( IS_AVAILABLE(SID_AUTOSPELL_CHECK,pItem) )              // an Doc-Options
    {
        BOOL bDoAutoSpell = ((const SfxBoolItem*)pItem)->GetValue();

        if (pDoc)
        {
            ScDocOptions aNewOpt = pDoc->GetDocOptions();
            if ( aNewOpt.IsAutoSpell() != bDoAutoSpell )
            {
                aNewOpt.SetAutoSpell( bDoAutoSpell );
                pDoc->SetDocOptions( aNewOpt );

                if (bDoAutoSpell)
                    pDoc->SetOnlineSpellPos( ScAddress(0,0,0) );    // vorne anfangen
                else
                {
                    WaitObject aWait( pDocSh->GetActiveDialogParent() );
                    pDoc->RemoveAutoSpellObj();     //  Edit-Text-Objekte wieder zurueckwandeln
                }

                //#92038#; don't set document modified, because this flag is no longer saved
//              pDocSh->SetDocumentModified();

                bRepaint = TRUE;            //  weil HideAutoSpell evtl. ungueltig
                                            //! alle Views painten ???
            }
        }

        if ( bOldAutoSpell != bDoAutoSpell )
            SetAutoSpellProperty( bDoAutoSpell );
        if ( pDocSh )
            pDocSh->PostPaintGridAll();                     // wegen Markierungen
        ScInputHandler* pInputHandler = GetInputHdl();
        if ( pInputHandler )
            pInputHandler->UpdateSpellSettings();           // EditEngine-Flags
        if ( pViewSh )
            pViewSh->UpdateDrawTextOutliner();              // EditEngine-Flags

        if (pBindings)
            pBindings->Invalidate( SID_AUTOSPELL_CHECK );
    }

    //============================================
    // InputOptions
    //============================================

    if ( IS_AVAILABLE(SID_SC_INPUT_SELECTIONPOS,pItem) )
    {
        pInputCfg->SetMoveDir( ((const SfxUInt16Item*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_SELECTION,pItem) )
    {
        pInputCfg->SetMoveSelection( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_EDITMODE,pItem) )
    {
        pInputCfg->SetEnterEdit( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_FMT_EXPAND,pItem) )
    {
        pInputCfg->SetExtendFormat( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_RANGEFINDER,pItem) )
    {
        pInputCfg->SetRangeFinder( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_REF_EXPAND,pItem) )
    {
        pInputCfg->SetExpandRefs( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_MARK_HEADER,pItem) )
    {
        pInputCfg->SetMarkHeader( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
        bUpdateMarks = TRUE;
    }
    if ( IS_AVAILABLE(SID_SC_INPUT_TEXTWYSIWYG,pItem) )
    {
        BOOL bNew = ((const SfxBoolItem*)pItem)->GetValue();
        if ( bNew != pInputCfg->GetTextWysiwyg() )
        {
            pInputCfg->SetTextWysiwyg( bNew );
            bSaveInputOptions = TRUE;
            bUpdateRefDev = TRUE;
        }
    }
    if( IS_AVAILABLE( SID_SC_INPUT_REPLCELLSWARN, pItem ) )
    {
        pInputCfg->SetReplaceCellsWarn( ((const SfxBoolItem*)pItem)->GetValue() );
        bSaveInputOptions = TRUE;
    }

    //============================================
    // PrintOptions
    //============================================

    if ( IS_AVAILABLE(SID_SCPRINTOPTIONS,pItem) )
    {
        const ScPrintOptions& rNewOpt = ((const ScTpPrintItem*)pItem)->GetPrintOptions();
        SetPrintOptions( rNewOpt );

        //  broadcast causes all previews to recalc page numbers
        SFX_APP()->Broadcast( SfxSimpleHint( SID_SCPRINTOPTIONS ) );
    }

    //----------------------------------------------------------

    if ( bSaveAppOptions )
        pAppCfg->OptionsChanged();

    if ( bSaveInputOptions )
        pInputCfg->OptionsChanged();

    // Neuberechnung anstossen?

    if ( pDoc && bCalcAll )
    {
        WaitObject aWait( pDocSh->GetActiveDialogParent() );
        pDoc->CalcAll();
        if ( pViewSh )
            pViewSh->UpdateCharts( TRUE );
        else
            ScDBFunc::DoUpdateCharts( ScAddress(), pDoc, TRUE );
        if (pBindings)
            pBindings->Invalidate( SID_ATTR_SIZE ); //SvxPosSize-StatusControl-Update
    }

    if ( pViewSh && bUpdateMarks )
        pViewSh->UpdateAutoFillMark();

    // View neuzeichnen?

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
            pBindings->Invalidate( FID_TOGGLEHEADERS ); // -> Checks im Menue
            pBindings->Invalidate( FID_TOGGLESYNTAX );
        }
    }

    // update ref device (for all documents)

    if ( bUpdateRefDev )
    {
        //  for all documents: recalc output factor, update row heights
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

        //  for all (tab-) views:
        TypeId aScType = TYPE(ScTabViewShell);
        SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
        while ( pSh )
        {
            ScTabViewShell* pOneViewSh = (ScTabViewShell*)pSh;

            //  set ref-device for EditEngine
            ScInputHandler* pHdl = GetInputHdl(pOneViewSh);
            if (pHdl)
                pHdl->UpdateRefDevice();

            //  update view scale
            ScViewData* pViewData = pOneViewSh->GetViewData();
            pOneViewSh->SetZoom( pViewData->GetZoomX(), pViewData->GetZoomY(), FALSE );

            //  repaint
            pOneViewSh->PaintGrid();
            pOneViewSh->PaintTop();
            pOneViewSh->PaintLeft();

            pSh = SfxViewShell::GetNext( *pSh, &aScType );
        }
    }
}

#undef IS_AVAILABLE

//------------------------------------------------------------------
//
//                      Input-Handler
//
//------------------------------------------------------------------

ScInputHandler* ScModule::GetInputHdl( ScTabViewShell* pViewSh, BOOL bUseRef )
{
    if ( pRefInputHandler && bUseRef )
        return pRefInputHandler;

    ScInputHandler* pHdl = NULL;
    if ( !pViewSh )
    {
        // in case a UIActive embedded object has no ViewShell ( UNO component )
        // the own calc view shell will be set as current, but no handling should happen

        ScTabViewShell* pCurViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        if ( pCurViewSh && !pCurViewSh->GetUIActiveClient() )
            pViewSh = pCurViewSh;
    }

    if ( pViewSh )
        pHdl = pViewSh->GetInputHandler();      // Viewshell hat jetzt immer einen

    //  #57989# wenn keine ViewShell uebergeben oder aktiv, kann NULL herauskommen
    DBG_ASSERT( pHdl || !pViewSh, "GetInputHdl: kein InputHandler gefunden" );
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

BOOL ScModule::IsEditMode()
{
    ScInputHandler* pHdl = GetInputHdl();
    return pHdl && pHdl->IsEditMode();
}

BOOL ScModule::IsInputMode()
{
    ScInputHandler* pHdl = GetInputHdl();
    return pHdl && pHdl->IsInputMode();
}

BOOL ScModule::InputKeyEvent( const KeyEvent& rKEvt, BOOL bStartEdit )
{
    ScInputHandler* pHdl = GetInputHdl();
    return ( pHdl ? pHdl->KeyInput( rKEvt, bStartEdit ) : FALSE );
}

void ScModule::InputEnterHandler( BYTE nBlockMode )
{
    if ( !SFX_APP()->IsDowning() )                                  // nicht beim Programmende
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

//------------------------------------------------------------------------
//  Olk's Krempel:

void ScModule::InputGetSelection( xub_StrLen& rStart, xub_StrLen& rEnd )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputGetSelection( rStart, rEnd );
}

void ScModule::InputSetSelection( xub_StrLen nStart, xub_StrLen nEnd )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputSetSelection( nStart, nEnd );
}

void ScModule::InputReplaceSelection( const String& rStr )
{
    ScInputHandler* pHdl = GetInputHdl();
    if (pHdl)
        pHdl->InputReplaceSelection( rStr );
}

String ScModule::InputGetFormulaStr()
{
    ScInputHandler* pHdl = GetInputHdl();
    String aStr;
    if ( pHdl )
        aStr = pHdl->InputGetFormulaStr();
    return aStr;
}

void ScModule::ActivateInputWindow( const String* pStrFormula, BOOL bMatrix )
{
    ScInputHandler* pHdl = GetInputHdl();
    if ( pHdl )
    {
        ScInputWindow* pWin = pHdl->GetInputWindow();
        if ( pStrFormula )
        {
            // Formel uebernehmen
            if ( pWin )
            {
                pWin->SetFuncString( *pStrFormula, FALSE );
                // SetSumAssignMode wegen FALSE nicht noetig
            }
            BYTE nMode = bMatrix ? SC_ENTER_MATRIX : SC_ENTER_NORMAL;
            pHdl->EnterHandler( nMode );

            //  ohne Invalidate bleibt die Selektion stehen, wenn die Formel unveraendert ist
            if (pWin)
                pWin->TextInvalidate();
        }
        else
        {
            // Abbrechen
            if ( pWin )
            {
                pWin->SetFuncString( EMPTY_STRING, FALSE );
                // SetSumAssignMode wegen FALSE nicht noetig
            }
            pHdl->CancelHandler();
        }
    }
}

//------------------------------------------------------------------
//
//                  Referenz - Dialoge
//
//------------------------------------------------------------------

void ScModule::SetRefDialog( USHORT nId, BOOL bVis, SfxViewFrame* pViewFrm )
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    if(nCurRefDlgId==0 || (nId==nCurRefDlgId && !bVis))
    {
        if ( !pViewFrm )
            pViewFrm = SfxViewFrame::Current();

        // #79379# bindings update causes problems with update of stylist if
        // current style family has changed
        //if ( pViewFrm )
        //  pViewFrm->GetBindings().Update();       // to avoid trouble in LockDispatcher

        nCurRefDlgId = bVis ? nId : 0 ;             // before SetChildWindow

        if ( pViewFrm )
        {
            //  store the dialog id also in the view shell
            SfxViewShell* pViewSh = pViewFrm->GetViewShell();
            if ( pViewSh && pViewSh->ISA( ScTabViewShell ) )
                ((ScTabViewShell*)pViewSh)->SetCurRefDlgId( nCurRefDlgId );
            else
            {
                // no ScTabViewShell - possible for example from a Basic macro
                bVis = FALSE;
                nCurRefDlgId = 0;   // don't set nCurRefDlgId if no dialog is created
            }

            pViewFrm->SetChildWindow( nId, bVis );
        }

        SfxApplication* pSfxApp = SFX_APP();
        pSfxApp->Broadcast( SfxSimpleHint( FID_REFMODECHANGED ) );
    }
}

SfxChildWindow* lcl_GetChildWinFromAnyView( USHORT nId )
{
    //  first try the current view

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    // #i46999# current view frame can be null (for example, when closing help)
    SfxChildWindow* pChildWnd = pViewFrm ? pViewFrm->GetChildWindow( nId ) : NULL;
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

    return NULL;                    // none found
}

BOOL ScModule::IsModalMode(SfxObjectShell* pDocSh)
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    BOOL bIsModal = FALSE;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            bIsModal = pChildWnd->IsVisible() &&
                !( pRefDlg->IsRefInputMode() && pRefDlg->IsDocAllowed(pDocSh) );
        }
        else
        {
            // in 592 and above, the dialog isn't visible in other views
            //  if the dialog is open but can't be accessed, disable input

            bIsModal = TRUE;
        }

        //  pChildWnd kann 0 sein, wenn der Dialog nach dem Umschalten
        //  von einer anderen Shell noch nicht erzeugt wurde (z.B. in GetFocus)
    }
    else if (pDocSh)
    {
        ScInputHandler* pHdl = GetInputHdl();
        if ( pHdl )
            bIsModal = pHdl->IsModalMode(pDocSh);
    }

    return bIsModal;
}

BOOL ScModule::IsTableLocked()
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    BOOL bLocked = FALSE;

    //  bisher nur bei ScAnyRefDlg

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
            bLocked = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow())->IsTableLocked();
        else
            bLocked = TRUE;     // for other views, see IsModalMode
    }

    return bLocked;
}

BOOL ScModule::IsRefDialogOpen()
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    BOOL bIsOpen = FALSE;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
            bIsOpen = pChildWnd->IsVisible();
        else
            bIsOpen = TRUE;     // for other views, see IsModalMode
    }

    return bIsOpen;
}

BOOL ScModule::IsFormulaMode()
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    BOOL bIsFormula = FALSE;

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            bIsFormula = pChildWnd->IsVisible() && pRefDlg->IsRefInputMode();
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if ( pHdl )
            bIsFormula = pHdl->IsFormulaMode();
    }

    if (bIsInEditCommand)
        bIsFormula = TRUE;

    return bIsFormula;
}

void lcl_MarkedTabs( const ScMarkData& rMark, SCTAB& rStartTab, SCTAB& rEndTab )
{
    if (rMark.GetSelectCount() > 1)
    {
        BOOL bFirst = TRUE;
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (rMark.GetTableSelect(i))
            {
                if (bFirst)
                    rStartTab = i;
                rEndTab = i;
                bFirst = FALSE;
            }
    }
}

void ScModule::SetReference( const ScRange& rRef, ScDocument* pDoc,
                                    const ScMarkData* pMarkData )
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    //  in Ref-Dialogen wird hiermit auch das Zoom-In ausgeloest,
    //  wenn Start und Ende der Ref unterschiedlich sind

    ScRange aNew = rRef;
    aNew.Justify();                 // immer "richtig herum"

    if( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        DBG_ASSERT( pChildWnd, "NoChildWin" );
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

            //  hide the (color) selection now instead of later from LoseFocus,
            //  don't abort the ref input that causes this call (bDoneRefMode = FALSE)
            pRefDlg->HideReference( FALSE );
            pRefDlg->SetReference( aNew, pDoc );
        }
    }
    else
    {
        ScInputHandler* pHdl = GetInputHdl();
        if (pHdl)
            pHdl->SetReference( aNew, pDoc );
        else
        {
            DBG_ERROR("SetReference ohne Empfaenger");
        }
    }
}

void ScModule::AddRefEntry()                        // "Mehrfachselektion"
{
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        DBG_ASSERT( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            pRefDlg->AddRefEntry();
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
    //! move reference dialog handling to view
    //! (only keep function autopilot here for references to other documents)

    //  in Ref-Dialogen wird hiermit auch das Zoom-In wieder aufgehoben

    //! ShowRefFrame am InputHdl, wenn der Funktions-AP offen ist ???

    if ( nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( nCurRefDlgId );
        DBG_ASSERT( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            pRefDlg->SetActive();
        }
    }
}

//------------------------------------------------------------------
//
//                  Idle / Online-Spelling
//
//------------------------------------------------------------------

void ScModule::AnythingChanged()
{
    ULONG nOldTime = aIdleTimer.GetTimeout();
    if ( nOldTime != SC_IDLE_MIN )
        aIdleTimer.SetTimeout( SC_IDLE_MIN );

    nIdleCount = 0;
}

void lcl_CheckNeedsRepaint( ScDocShell* pDocShell )
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

IMPL_LINK( ScModule, IdleHandler, Timer*, EMPTYARG )
{
    if ( Application::AnyInput( INPUT_MOUSEANDKEYBOARD ) )
    {
        aIdleTimer.Start();         // Timeout unveraendert
        return 0;
    }

    BOOL bMore = FALSE;
    ScDocShell* pDocSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();

        BOOL bLinks = pDoc->IdleCheckLinks();
        BOOL bWidth = pDoc->IdleCalcTextWidth();
        BOOL bSpell = pDoc->ContinueOnlineSpelling();
        if ( bSpell )
            aSpellTimer.Start();                    // da ist noch was

        bMore = bLinks || bWidth || bSpell;         // ueberhaupt noch was?

        //  While calculating a Basic formula, a paint event may have occurred,
        //  so check the bNeedsRepaint flags for this document's views
        if (bWidth)
            lcl_CheckNeedsRepaint( pDocSh );
    }

    ULONG nOldTime = aIdleTimer.GetTimeout();
    ULONG nNewTime = nOldTime;
    if ( bMore )
    {
        nNewTime = SC_IDLE_MIN;
        nIdleCount = 0;
    }
    else
    {
        //  SC_IDLE_COUNT mal mit initialem Timeout, dann hochzaehlen

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

IMPL_LINK( ScModule, SpellTimerHdl, Timer*, EMPTYARG )
{
    if ( Application::AnyInput( INPUT_KEYBOARD ) )
    {
        aSpellTimer.Start();
        return 0;                   // dann spaeter wieder...
    }

    ScDocShell* pDocSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        if ( pDoc->ContinueOnlineSpelling() )
            aSpellTimer.Start();
    }
    return 0;
}

    //virtuelle Methoden fuer den Optionendialog
SfxItemSet*  ScModule::CreateItemSet( USHORT nId )
{
    SfxItemSet*  pRet = 0;
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
                            // TP_USERLISTS:
                            SCITEM_USERLIST,        SCITEM_USERLIST,
                            // TP_PRINT:
                            SID_SCPRINTOPTIONS, SID_SCPRINTOPTIONS,
                            // TP_GRID:
                            SID_ATTR_GRID_OPTIONS, SID_ATTR_GRID_OPTIONS,
                            //
                            SID_ATTR_METRIC,        SID_ATTR_METRIC,
                            SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                            0 );

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

        //  SFX_APP()->GetOptions( aSet );

        pRet->Put( SfxUInt16Item( SID_ATTR_METRIC,
                        sal::static_int_cast<UINT16>(GetAppOptions().GetAppMetric()) ) );

        // TP_CALC
        pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP,
                        aCalcOpt.GetTabDistance()));
        pRet->Put( ScTpCalcItem( SID_SCDOCOPTIONS, aCalcOpt ) );

        // TP_VIEW
        pRet->Put( ScTpViewItem( SID_SCVIEWOPTIONS, aViewOpt ) );
        pRet->Put( SfxBoolItem( SID_SC_OPT_SYNCZOOM, GetAppOptions().GetSynchronizeZoom() ) );

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
        pRet->Put( SfxBoolItem( SID_SC_INPUT_MARK_HEADER,
                    rInpOpt.GetMarkHeader() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_TEXTWYSIWYG,
                    rInpOpt.GetTextWysiwyg() ) );
        pRet->Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN,
                    rInpOpt.GetReplaceCellsWarn() ) );

        // RID_SC_TP_PRINT
        pRet->Put( ScTpPrintItem( SID_SCPRINTOPTIONS, GetPrintOptions() ) );

        // TP_GRID
        SvxGridItem* pSvxGridItem = aViewOpt.CreateGridItem();
        pRet->Put( *pSvxGridItem );
        delete pSvxGridItem;

        // TP_USERLISTS
        if ( pUL )
            aULItem.SetUserList( *pUL );
        pRet->Put( aULItem );

    }
    return pRet;
}

void ScModule::ApplyItemSet( USHORT nId, const SfxItemSet& rSet )
{
    if(SID_SC_EDITOPTIONS == nId)
    {
        ModifyOptions( rSet );
    }
}

SfxTabPage*  ScModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage* pRet = NULL;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");
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
        case RID_OFA_TP_INTERNATIONAL:
        {
            SfxAbstractDialogFactory* pSfxFact = SfxAbstractDialogFactory::Create();
            if ( pSfxFact )
            {
                ::CreateTabPage fnCreatePage = pSfxFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
        }
    }

    DBG_ASSERT( pRet, "ScModule::CreateTabPage(): no valid ID for TabPage!" );

    return pRet;
}

//------------------------------------------------------------------

IMPL_LINK( ScModule, CalcFieldValueHdl, EditFieldInfo*, pInfo )
{
    //! mit ScFieldEditEngine zusammenfassen !!!

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
            String aURL = pURLField->GetURL();

            switch ( pURLField->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
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
            DBG_ERROR("unbekannter Feldbefehl");
            pInfo->SetRepresentation(String('?'));
        }
    }

    return 0;
}

BOOL ScModule::RegisterRefWindow( USHORT nSlotId, Window *pWnd )
{
    std::list<Window*> & rlRefWindow = m_mapRefWindow[nSlotId];

    if( std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd ) == rlRefWindow.end() )
    {
        rlRefWindow.push_back( pWnd );
        return TRUE;
    }

    return FALSE;
}

BOOL  ScModule::UnregisterRefWindow( USHORT nSlotId, Window *pWnd )
{
    std::map<USHORT, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return FALSE;

    std::list<Window*> & rlRefWindow = iSlot->second;

    std::list<Window*>::iterator i = std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );

    if( i == rlRefWindow.end() )
        return FALSE;

    rlRefWindow.erase( i );

    if( !rlRefWindow.size() )
        m_mapRefWindow.erase( nSlotId );

    return TRUE;
}

BOOL  ScModule::IsAliveRefDlg( USHORT nSlotId, Window *pWnd )
{
    std::map<USHORT, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return FALSE;

    std::list<Window*> & rlRefWindow = iSlot->second;

    return rlRefWindow.end() != std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );
}

Window *  ScModule::Find1RefWindow( USHORT nSlotId, Window *pWndAncestor )
{
    if (!pWndAncestor)
        return NULL;

    std::map<USHORT, std::list<Window*> >::iterator iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return NULL;

    std::list<Window*> & rlRefWindow = iSlot->second;

    while( Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for( std::list<Window*>::iterator i = rlRefWindow.begin(); i!=rlRefWindow.end(); ++i )
        if ( pWndAncestor->IsWindowOrChild( *i, (*i)->IsSystemWindow() ) )
            return *i;

    return NULL;
}

Window *  ScModule::Find1RefWindow( Window *pWndAncestor )
{
    if (!pWndAncestor)
        return NULL;

    while( Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for( std::map<USHORT, std::list<Window*> >::iterator i = m_mapRefWindow.begin();
        i!=m_mapRefWindow.end(); ++i )
        for( std::list<Window*>::iterator j = i->second.begin(); j!=i->second.end(); ++j )
            if ( pWndAncestor->IsWindowOrChild( *j, (*j)->IsSystemWindow() ) )
                return *j;

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
