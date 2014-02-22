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

#include "docsh.hxx"

#include "scitems.hxx"
#include <editeng/justifyitem.hxx>
#include <comphelper/classids.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svl/PasswordHelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
#include "svl/urihelper.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include "scabstdlg.hxx"
#include <sot/formats.hxx>

#include "formulacell.hxx"
#include "postit.hxx"
#include "global.hxx"
#include "filter.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "docfunc.hxx"
#include "imoptdlg.hxx"
#include "impex.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "brdcst.hxx"
#include "stlpool.hxx"
#include "autostyl.hxx"
#include "attrib.hxx"
#include "asciiopt.hxx"
#include "waitoff.hxx"
#include "docpool.hxx"
#include "progress.hxx"
#include "pntlock.hxx"
#include "docuno.hxx"
#include "appoptio.hxx"
#include "detdata.hxx"
#include "printfun.hxx"
#include "dociter.hxx"
#include "cellform.hxx"
#include "chartlis.hxx"
#include "hints.hxx"
#include "xmlwrap.hxx"
#include "drwlayer.hxx"
#include "refreshtimer.hxx"
#include "dbdata.hxx"
#include "scextopt.hxx"
#include "compiler.hxx"
#include "cfgids.hxx"
#include "warnpassword.hxx"
#include "optsolver.hxx"
#include "sheetdata.hxx"
#include "tabprotection.hxx"
#include "docparam.hxx"
#include "docshimp.hxx"
#include "sizedev.hxx"
#include "refreshtimerprotector.hxx"

#include <officecfg/Office/Calc.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/configmgr.hxx>
#include "uiitems.hxx"
#include "cellsuno.hxx"
#include "dpobject.hxx"
#include "markdata.hxx"
#include "orcusfilters.hxx"
#include <datastream.hxx>
#include <documentlinkmgr.hxx>

#include <config_telepathy.h>

#if ENABLE_TELEPATHY
#include "sccollaboration.hxx"
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::boost::shared_ptr;
using ::std::vector;





static const sal_Char pFilterSc50[]     = "StarCalc 5.0";
static const sal_Char pFilterSc40[]     = "StarCalc 4.0";
static const sal_Char pFilterSc30[]     = "StarCalc 3.0";
static const sal_Char pFilterSc10[]     = "StarCalc 1.0";
static const sal_Char pFilterXML[]      = "StarOffice XML (Calc)";
static const sal_Char pFilterAscii[]        = "Text - txt - csv (StarCalc)";
static const sal_Char pFilterLotus[]        = "Lotus";
static const sal_Char pFilterQPro6[]        = "Quattro Pro 6.0";
static const sal_Char pFilterExcel4[]   = "MS Excel 4.0";
static const sal_Char pFilterEx4Temp[]  = "MS Excel 4.0 Vorlage/Template";
static const sal_Char pFilterExcel5[]   = "MS Excel 5.0/95";
static const sal_Char pFilterEx5Temp[]  = "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char pFilterExcel95[]  = "MS Excel 95";
static const sal_Char pFilterEx95Temp[] = "MS Excel 95 Vorlage/Template";
static const sal_Char pFilterExcel97[]  = "MS Excel 97";
static const sal_Char pFilterEx97Temp[] = "MS Excel 97 Vorlage/Template";
static const sal_Char pFilterDBase[]        = "dBase";
static const sal_Char pFilterDif[]      = "DIF";
static const sal_Char pFilterSylk[]     = "SYLK";
static const sal_Char pFilterHtml[]     = "HTML (StarCalc)";
static const sal_Char pFilterHtmlWebQ[] = "calc_HTML_WebQuery";
static const sal_Char pFilterRtf[]      = "Rich Text Format (StarCalc)";



#define ScDocShell
#include "scslots.hxx"


SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell, ScResId(SCSTR_DOCSHELL))
{
}


SFX_IMPL_OBJECTFACTORY( ScDocShell, SvGlobalName(SO3_SC_CLASSID), SFXOBJECTSHELL_STD_NORMAL, "scalc" )

TYPEINIT1( ScDocShell, SfxObjectShell ); 



void ScDocShell::FillClass( SvGlobalName* pClassName,
                                        sal_uInt32* pFormat,
                                        OUString* /* pAppName */,
                                        OUString* pFullTypeName,
                                        OUString* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
    if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_60;
        *pFullTypeName  = OUString( ScResId( SCSTR_LONG_SCDOC_NAME ) );
        *pShortTypeName = OUString( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = bTemplate ? SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE : SOT_FORMATSTR_ID_STARCALC_8;
        *pFullTypeName  = "calc8";
        *pShortTypeName = ScResId(SCSTR_SHORT_SCDOC_NAME).toString();
    }
    else
    {
        OSL_FAIL("Which version?");
    }
}



void ScDocShell::DoEnterHandler()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        if (pViewSh->GetViewData()->GetDocShell() == this)
            SC_MOD()->InputEnterHandler();
}



SCTAB ScDocShell::GetSaveTab()
{
    SCTAB nTab = 0;
    ScTabViewShell* pSh = GetBestViewShell();
    if (pSh)
    {
        const ScMarkData& rMark = pSh->GetViewData()->GetMarkData();
        nTab = rMark.GetFirstSelected();
    }
    return nTab;
}

sal_uInt16 ScDocShell::GetHiddenInformationState( sal_uInt16 nStates )
{
    
    sal_uInt16 nState = SfxObjectShell::GetHiddenInformationState( nStates );

    if ( nStates & HIDDENINFORMATION_RECORDEDCHANGES )
    {
        if ( aDocument.GetChangeTrack() && aDocument.GetChangeTrack()->GetFirst() )
          nState |= HIDDENINFORMATION_RECORDEDCHANGES;
    }
    if ( nStates & HIDDENINFORMATION_NOTES )
    {
        SCTAB nTableCount = aDocument.GetTableCount();
        bool bFound = false;
        for (SCTAB nTab = 0; nTab < nTableCount && !bFound; ++nTab)
        {
            if (aDocument.HasTabNotes(nTab)) 
                bFound = true;
        }

        if (bFound)
            nState |= HIDDENINFORMATION_NOTES;
    }

    return nState;
}

void ScDocShell::BeforeXMLLoading()
{
    aDocument.EnableIdle(false);

    
    OSL_ENSURE(pModificator == NULL, "The Modificator should not exist");
    pModificator = new ScDocShellModificator( *this );

    aDocument.SetImportingXML( true );
    aDocument.EnableExecuteLink( false );   
    aDocument.EnableUndo( false );
    
    aDocument.SetInsertingFromOtherDoc( true );
}

void ScDocShell::AfterXMLLoading(bool bRet)
{
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
    {
        UpdateLinks();
        
        aDocument.SetInsertingFromOtherDoc( false );
        if ( bRet )
        {
            ScChartListenerCollection* pChartListener = aDocument.GetChartListenerCollection();
            if (pChartListener)
                pChartListener->UpdateDirtyCharts();

            
            SCTAB nTabCount = aDocument.GetTableCount();
            for (SCTAB i = 0; i < nTabCount; ++i)
            {
                if (aDocument.IsLinked( i ))
                {
                    OUString aName;
                    aDocument.GetName(i, aName);
                    OUString aLinkTabName = aDocument.GetLinkTab(i);
                    sal_Int32 nLinkTabNameLength = aLinkTabName.getLength();
                    sal_Int32 nNameLength = aName.getLength();
                    if (nLinkTabNameLength < nNameLength)
                    {

                        
                        const sal_Unicode* pNameBuffer = aName.getStr();
                        if ( *pNameBuffer == '\'' && 
                            ScGlobal::UnicodeStrChr( pNameBuffer, SC_COMPILER_FILE_TAB_SEP ) )
                        {
                            OUStringBuffer aDocURLBuffer;
                            sal_Bool bQuote = sal_True; 
                            ++pNameBuffer;
                            while ( bQuote && *pNameBuffer )
                            {
                                if ( *pNameBuffer == '\'' && *(pNameBuffer-1) != '\\' )
                                    bQuote = false;
                                else if( !(*pNameBuffer == '\\' && *(pNameBuffer+1) == '\'') )
                                    aDocURLBuffer.append(*pNameBuffer); 
                                ++pNameBuffer;
                            }


                            if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  
                            {
                                sal_Int32 nIndex = nNameLength - nLinkTabNameLength;
                                INetURLObject aINetURLObject(aDocURLBuffer.makeStringAndClear());
                                if(aName == aLinkTabName.copy(nIndex, nLinkTabNameLength) &&
                                    (aName[nIndex - 1] == '#') && 
                                    !aINetURLObject.HasError()) 
                                {
                                    aName = ScGlobal::GetDocTabName( aDocument.GetLinkDoc( i ), aDocument.GetLinkTab( i ) );
                                    aDocument.RenameTab(i, aName, true, true);
                                }
                                
                            }
                            
                        }
                        
                    }
                    
                }
            }

            
            
            ScDPCollection* pDPCollection = aDocument.GetDPCollection();
            if ( pDPCollection )
            {
                size_t nDPCount = pDPCollection->GetCount();
                for (size_t nDP=0; nDP<nDPCount; ++nDP)
                {
                    ScDPObject* pDPObj = (*pDPCollection)[nDP];
                    if (pDPObj->GetName().isEmpty())
                        pDPObj->SetName( pDPCollection->CreateNewName() );
                }
            }
        }
    }
    else
        aDocument.SetInsertingFromOtherDoc( false );

    aDocument.SetImportingXML( false );
    aDocument.EnableExecuteLink( true );
    aDocument.EnableUndo( true );
    bIsEmpty = false;

    if (pModificator)
    {
        bool bRecalcState = aDocument.GetHardRecalcState();
        
        
        aDocument.SetHardRecalcState(true);
        delete pModificator;
        aDocument.SetHardRecalcState(bRecalcState);
        pModificator = NULL;
    }
    else
    {
        OSL_FAIL("The Modificator should exist");
    }

    aDocument.EnableIdle(true);
}

namespace {

class LoadMediumGuard
{
public:
    explicit LoadMediumGuard(ScDocument* pDoc) :
        mpDoc(pDoc)
    {
        mpDoc->SetLoadingMedium(true);
    }

    ~LoadMediumGuard()
    {
        mpDoc->SetLoadingMedium(false);
    }
private:
    ScDocument* mpDoc;
};

void processDataStream( ScDocShell& rShell, const sc::ImportPostProcessData& rData )
{
    if (!rData.mpDataStream)
        return;

    const sc::ImportPostProcessData::DataStream& r = *rData.mpDataStream;
    if (!r.maRange.IsValid())
        return;

    
    
    

    ScRange aTopRange = r.maRange;
    aTopRange.aEnd.SetRow(aTopRange.aStart.Row());
    sal_Int32 nLimit = r.maRange.aEnd.Row() - r.maRange.aStart.Row() + 1;
    if (r.maRange.aEnd.Row() == MAXROW)
        
        nLimit = 0;

    sc::DataStream::MoveType eMove =
        r.meInsertPos == sc::ImportPostProcessData::DataStream::InsertTop ?
        sc::DataStream::MOVE_DOWN : sc::DataStream::RANGE_DOWN;

    sc::DataStream* pStrm = new sc::DataStream(&rShell, r.maURL, aTopRange, nLimit, eMove, 0);
    pStrm->SetRefreshOnEmptyLine(r.mbRefreshOnEmpty);
    sc::DocumentLinkManager& rMgr = rShell.GetDocument()->GetDocLinkManager();
    rMgr.setDataStream(pStrm);
}

}

bool ScDocShell::LoadXML( SfxMedium* pLoadMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    LoadMediumGuard aLoadGuard(&aDocument);

    

    

    BeforeXMLLoading();

    
    
    
    aDocument.SetXMLFromWrapper( true );

    ScXMLImportWrapper aImport( aDocument, pLoadMedium, xStor );

    sal_Bool bRet(false);
    ErrCode nError = ERRCODE_NONE;
    aDocument.EnableAdjustHeight(false);
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Import(false, nError);
    else
        bRet = aImport.Import(true, nError);

    if ( nError )
        pLoadMedium->SetError( nError, OUString( OSL_LOG_PREFIX ) );

    processDataStream(*this, aImport.GetImportPostProcessData());

    
    
    
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();

    Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    ScRecalcOptions nRecalcMode =
        static_cast<ScRecalcOptions>(officecfg::Office::Calc::Formula::Load::ODFRecalcMode::get(xContext));

    bool bHardRecalc = false;
    if (nRecalcMode == RECALC_ASK)
    {
        OUString sProductName(utl::ConfigManager::getProductName());
        if (aDocument.IsUserInteractionEnabled() && xDocProps->getGenerator().indexOf(sProductName) == -1)
        {
            
            
            QueryBox aBox(
                GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                ScGlobal::GetRscString(STR_QUERY_FORMULA_RECALC_ONLOAD_ODS));
            aBox.SetCheckBoxText(ScGlobal::GetRscString(STR_ALWAYS_PERFORM_SELECTED));

            bHardRecalc = aBox.Execute() == RET_YES;

            if (aBox.GetCheckBoxState())
            {
                
                boost::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create() );
                officecfg::Office::Calc::Formula::Load::ODFRecalcMode::set(sal_Int32(0), batch);
                ScFormulaOptions aOpt = SC_MOD()->GetFormulaOptions();
                aOpt.SetODFRecalcOptions(bHardRecalc ? RECALC_ALWAYS : RECALC_NEVER);
                SC_MOD()->SetFormulaOptions(aOpt);

                batch->commit();
            }
        }
    }
    else if (nRecalcMode == RECALC_ALWAYS)
        bHardRecalc = true;

    if (bHardRecalc)
        DoHardRecalc(false);
    else
    {
        
        aDocument.Broadcast(ScHint(SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS));
    }

    aDocument.SetXMLFromWrapper( false );
    AfterXMLLoading(bRet);

    aDocument.EnableAdjustHeight(true);
    return bRet;
}

bool ScDocShell::SaveXML( SfxMedium* pSaveMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    aDocument.EnableIdle(false);

    ScXMLImportWrapper aImport( aDocument, pSaveMedium, xStor );
    sal_Bool bRet(false);
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Export(false);
    else
        bRet = aImport.Export(true);

    aDocument.EnableIdle(true);

    return bRet;
}

bool ScDocShell::SaveCurrentChart( SfxMedium& rMedium )
{
    bool bRet = false;

    try
    {

        uno::Reference< lang::XComponent > xCurrentComponent = frame::Desktop::create( comphelper::getProcessComponentContext() )->getCurrentComponent();

        uno::Reference< frame::XStorable2 > xStorable( xCurrentComponent, uno::UNO_QUERY_THROW );

        uno::Reference< frame::XModel > xChartDoc ( xCurrentComponent, uno::UNO_QUERY_THROW );

        ScXMLChartExportWrapper aExport( xChartDoc, rMedium );
        bRet = aExport.Export();
    }
    catch(...)
    {
        SAL_WARN("sc", "exception thrown while saving chart. Bug!!!");
    }
    return bRet;
}

sal_Bool ScDocShell::Load( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    
    
    InitOptions(true);

    GetUndoManager()->Clear();

    sal_Bool bRet = SfxObjectShell::Load( rMedium );
    if( bRet )
    {
        if (GetMedium())
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
            nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
        }

        {
            
            
            aDocument.MakeTable(0);
            aDocument.GetStyleSheetPool()->CreateStandardStyles();
            aDocument.UpdStlShtPtrsFrmNms();

            bRet = LoadXML( &rMedium, NULL );
        }
    }

    if (!bRet && !rMedium.GetError())
        rMedium.SetError( SVSTREAM_FILEFORMAT_ERROR, OUString( OSL_LOG_PREFIX ) );

    if (rMedium.GetError())
        SetError( rMedium.GetError(), OUString( OSL_LOG_PREFIX ) );

    InitItems();
    CalcOutputFactor();

    
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = false;
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
    return bRet;
}

void ScDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (rHint.ISA(ScTablesHint) )
    {
        const ScTablesHint& rScHint = static_cast< const ScTablesHint& >( rHint );
        if (rScHint.GetId() == SC_TAB_INSERTED)
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = aDocument.GetVbaEventProcessor();
            if ( xVbaEvents.is() ) try
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= rScHint.GetTab1();
                xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_NEWSHEET, aArgs );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    if (rHint.ISA(SfxSimpleHint)) 
    {
        sal_uLong nSlot = ((const SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case SFX_HINT_TITLECHANGED:
                aDocument.SetName( SfxShell::GetName() );
                
                SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED )); 
                break;
        }
    }
    else if (rHint.ISA(SfxStyleSheetHint)) 
        NotifyStyle((const SfxStyleSheetHint&) rHint);
    else if (rHint.ISA(ScAutoStyleHint))
    {
        

        
        
        

        ScAutoStyleHint& rStlHint = (ScAutoStyleHint&)rHint;
        ScRange aRange = rStlHint.GetRange();
        OUString aName1 = rStlHint.GetStyle1();
        OUString aName2 = rStlHint.GetStyle2();
        sal_uInt32 nTimeout = rStlHint.GetTimeout();

        if (!pAutoStyleList)
            pAutoStyleList = new ScAutoStyleList(this);
        pAutoStyleList->AddInitial( aRange, aName1, nTimeout, aName2 );
    }
    else if ( rHint.ISA( SfxEventHint ) )
    {
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch ( nEventId )
        {
            case SFX_EVENT_LOADFINISHED:
                {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    
                    if ( HasSharedXMLFlagSet() && !SC_MOD()->IsInSharedDocLoading() && !IsReadOnly() )
                    {
                        if ( SwitchToShared( sal_True, false ) )
                        {
                            ScViewData* pViewData = GetViewData();
                            ScTabView* pTabView = ( pViewData ? dynamic_cast< ScTabView* >( pViewData->GetView() ) : NULL );
                            if ( pTabView )
                            {
                                pTabView->UpdateLayerLocks();
                            }
                        }
                        else
                        {
                            
                            
                            SetReadOnlyUI( sal_True );
                        }
                    }
#endif
                }
                break;
            case SFX_EVENT_VIEWCREATED:
                {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocLoading() )
                    {
                        ScAppOptions aAppOptions = SC_MOD()->GetAppOptions();
                        if ( aAppOptions.GetShowSharedDocumentWarning() )
                        {
                            WarningBox aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                ScGlobal::GetRscString( STR_SHARED_DOC_WARNING ) );
                            aBox.SetDefaultCheckBoxText();
                            aBox.Execute();
                            sal_Bool bChecked = aBox.GetCheckBoxState();
                            if ( bChecked )
                            {
                                aAppOptions.SetShowSharedDocumentWarning( !bChecked );
                                SC_MOD()->SetAppOptions( aAppOptions );
                            }
                        }
                    }
#endif
                    try
                    {
                        uno::Reference< uno::XComponentContext > xContext(
                            comphelper::getProcessComponentContext() );
                        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
                            xContext->getServiceManager(),
                            uno::UNO_QUERY_THROW );
                        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xServiceManager, uno::UNO_QUERY_THROW );
                        uno::Reference< container::XEnumeration> xEnum = xEnumAccess->createContentEnumeration(
                            OUString( "com.sun.star.sheet.SpreadsheetDocumentJob" ) );
                        if ( xEnum.is() )
                        {
                            while ( xEnum->hasMoreElements() )
                            {
                                uno::Any aAny = xEnum->nextElement();
                                uno::Reference< lang::XSingleComponentFactory > xFactory;
                                aAny >>= xFactory;
                                if ( xFactory.is() )
                                {
                                    uno::Reference< task::XJob > xJob( xFactory->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                                    uno::Sequence< beans::NamedValue > aArgsForJob(1);
                                    ScViewData* pViewData = GetViewData();
                                    SfxViewShell* pViewShell = ( pViewData ? pViewData->GetViewShell() : NULL );
                                    SfxViewFrame* pViewFrame = ( pViewShell ? pViewShell->GetViewFrame() : NULL );
                                    SfxFrame* pFrame = ( pViewFrame ? &pViewFrame->GetFrame() : NULL );
                                    uno::Reference< frame::XController > xController = ( pFrame ? pFrame->GetController() : 0 );
                                    uno::Reference< sheet::XSpreadsheetView > xSpreadsheetView( xController, uno::UNO_QUERY_THROW );
                                    aArgsForJob[0] = beans::NamedValue( OUString( "SpreadsheetView" ),
                                            uno::makeAny( xSpreadsheetView ) );
                                    xJob->execute( aArgsForJob );
                                }
                            }
                        }
                    }
                    catch ( uno::Exception & )
                    {
                    }
                }
                break;
            case SFX_EVENT_SAVEDOC:
                {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocSaving() )
                    {
                        bool bSuccess = false;
                        bool bRetry = true;
                        while ( bRetry )
                        {
                            bRetry = false;
                            uno::Reference< frame::XModel > xModel;
                            try
                            {
                                
                                xModel.set( LoadSharedDocument(), uno::UNO_QUERY_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                
                                bool bShared = false;
                                ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
                                ScDocShell* pSharedDocShell = ( pDocObj ? dynamic_cast< ScDocShell* >( pDocObj->GetObjectShell() ) : NULL );
                                if ( pSharedDocShell )
                                {
                                    bShared = pSharedDocShell->HasSharedXMLFlagSet();
                                }

                                
                                bool bOwnEntry = false;
                                bool bEntriesNotAccessible = false;
                                try
                                {
                                    ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                                    bOwnEntry = aControlFile.HasOwnEntry();
                                }
                                catch ( uno::Exception& )
                                {
                                    bEntriesNotAccessible = true;
                                }

                                if ( bShared && bOwnEntry )
                                {
                                    uno::Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY_THROW );

                                    if ( xStorable->isReadonly() )
                                    {
                                        xCloseable->close( sal_True );

                                        OUString aUserName( ScGlobal::GetRscString( STR_UNKNOWN_USER ) );
                                        bool bNoLockAccess = false;
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            uno::Sequence< OUString > aData = aLockFile.GetLockData();
                                            if ( aData.getLength() > LOCKFILE_SYSUSERNAME_ID )
                                            {
                                                if ( !aData[LOCKFILE_OOOUSERNAME_ID].isEmpty() )
                                                {
                                                    aUserName = aData[LOCKFILE_OOOUSERNAME_ID];
                                                }
                                                else if ( !aData[LOCKFILE_SYSUSERNAME_ID].isEmpty() )
                                                {
                                                    aUserName = aData[LOCKFILE_SYSUSERNAME_ID];
                                                }
                                            }
                                        }
                                        catch ( uno::Exception& )
                                        {
                                            bNoLockAccess = true;
                                        }

                                        if ( bNoLockAccess )
                                        {
                                            
                                            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                        }
                                        else
                                        {
                                            OUString aMessage( ScGlobal::GetRscString( STR_FILE_LOCKED_SAVE_LATER ) );
                                            aMessage = aMessage.replaceFirst( "%1", aUserName );

                                            WarningBox aBox( GetActiveDialogParent(), WinBits( WB_RETRY_CANCEL | WB_DEF_RETRY ), aMessage );
                                            if ( aBox.Execute() == RET_RETRY )
                                            {
                                                bRetry = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        
                                        bool bSaveToShared = false;
                                        if ( pSharedDocShell )
                                        {
                                            bSaveToShared = MergeSharedDocument( pSharedDocShell );
                                        }

                                        
                                        xCloseable->close( sal_True );

                                        

                                        
                                        if ( bSaveToShared )
                                        {
                                            bool bChangedViewSettings = false;
                                            ScChangeViewSettings* pChangeViewSet = aDocument.GetChangeViewSettings();
                                            if ( pChangeViewSet && pChangeViewSet->ShowChanges() )
                                            {
                                                pChangeViewSet->SetShowChanges( false );
                                                pChangeViewSet->SetShowAccepted( false );
                                                aDocument.SetChangeViewSettings( *pChangeViewSet );
                                                bChangedViewSettings = true;
                                            }

                                            uno::Reference< frame::XStorable > xStor( GetModel(), uno::UNO_QUERY_THROW );
                                            
                                            uno::Sequence< beans::PropertyValue > aValues(1);
                                            aValues[0].Name = "FilterName";
                                            aValues[0].Value <<= OUString( GetMedium()->GetFilter()->GetFilterName() );

                                            SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, false);
                                            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
                                            {
                                                aValues.realloc( 2 );
                                                aValues[1].Name = "Password";
                                                aValues[1].Value <<= pPasswordItem->GetValue();
                                            }

                                            SC_MOD()->SetInSharedDocSaving( true );
                                            xStor->storeToURL( GetSharedFileURL(), aValues );
                                            SC_MOD()->SetInSharedDocSaving( false );

                                            if ( bChangedViewSettings )
                                            {
                                                pChangeViewSet->SetShowChanges( true );
                                                pChangeViewSet->SetShowAccepted( true );
                                                aDocument.SetChangeViewSettings( *pChangeViewSet );
                                            }
                                        }

                                        bSuccess = true;
                                        GetUndoManager()->Clear();
                                    }
                                }
                                else
                                {
                                    xCloseable->close( sal_True );

                                    if ( bEntriesNotAccessible )
                                    {
                                        
                                        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                    }
                                    else
                                    {
                                        WarningBox aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                            ScGlobal::GetRscString( STR_DOC_NOLONGERSHARED ) );
                                        aBox.Execute();

                                        SfxBindings* pBindings = GetViewBindings();
                                        if ( pBindings )
                                        {
                                            pBindings->ExecuteSynchron( SID_SAVEASDOC );
                                        }
                                    }
                                }
                            }
                            catch ( uno::Exception& )
                            {
                                OSL_FAIL( "SFX_EVENT_SAVEDOC: caught exception\n" );
                                SC_MOD()->SetInSharedDocSaving( false );

                                try
                                {
                                    uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
                                    xClose->close( sal_True );
                                }
                                catch ( uno::Exception& )
                                {
                                }
                            }
                        }

                        if ( !bSuccess )
                            SetError( ERRCODE_IO_ABORT, OUString( OSL_LOG_PREFIX ) ); 
                    }
#endif

                    if (pSheetSaveData)
                        pSheetSaveData->SetInSupportedSave(true);
                }
                break;
            case SFX_EVENT_SAVEASDOC:
                {
                    if ( GetDocument()->GetExternalRefManager()->containsUnsavedReferences() )
                    {
                        WarningBox aBox( GetActiveDialogParent(), WinBits( WB_YES_NO ),
                                ScGlobal::GetRscString( STR_UNSAVED_EXT_REF ) );

                        if( RET_NO == aBox.Execute())
                        {
                            SetError( ERRCODE_IO_ABORT, OUString( OSL_LOG_PREFIX ) ); 
                        }
                    }
                } 
            case SFX_EVENT_SAVETODOC:
                
                
                
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(true);
                break;
            case SFX_EVENT_SAVEDOCDONE:
            case SFX_EVENT_SAVEASDOCDONE:
                {
                    
                    UseSheetSaveEntries();      
                } 
            case SFX_EVENT_SAVETODOCDONE:
                
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(false);
                break;
            default:
                {
                }
                break;
        }
    }
}


sal_Bool ScDocShell::LoadFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    WaitObject aWait( GetActiveDialogParent() );

    sal_Bool bRet = false;

    if (GetMedium())
    {
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
        nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
    }

    
    
    bRet = LoadXML( &rMedium, NULL );
    InitItems();

    SfxObjectShell::LoadFrom( rMedium );

    return bRet;
}

static void lcl_parseHtmlFilterOption(const OUString& rOption, LanguageType& rLang, bool& rDateConvert)
{
    OUStringBuffer aBuf;
    std::vector< OUString > aTokens;
    sal_Int32 n = rOption.getLength();
    const sal_Unicode* p = rOption.getStr();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const sal_Unicode c = p[i];
        if (c == ' ')
        {
            if (!aBuf.isEmpty())
                aTokens.push_back( aBuf.makeStringAndClear() );
        }
        else
            aBuf.append(c);
    }

    if (!aBuf.isEmpty())
        aTokens.push_back( aBuf.makeStringAndClear() );

    rLang = LanguageType( 0 );
    rDateConvert = false;

    if (aTokens.size() > 0)
        rLang = static_cast<LanguageType>(aTokens[0].toInt32());
    if (aTokens.size() > 1)
        rDateConvert = static_cast<bool>(aTokens[1].toInt32());
}

sal_Bool ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);

    sal_Bool bRet = false; 
                           

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    GetUndoManager()->Clear();

    
    sal_Bool bSetColWidths = false;
    sal_Bool bSetSimpleTextColWidths = false;
    ScColWidthParam aColWidthParam[MAXCOLCOUNT];
    ScRange aColWidthRange;
    
    sal_Bool bSetRowHeights = false;

    vector<ScDocRowHeightUpdater::TabRanges> aRecalcRowRangesArray;

    
    
    rMedium.GetPhysicalName();  

    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
    nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;

    const SfxFilter* pFilter = rMedium.GetFilter();
    if (pFilter)
    {
        OUString aFltName = pFilter->GetFilterName();

        sal_Bool bCalc3 = ( aFltName.equalsAscii(pFilterSc30) );
        sal_Bool bCalc4 = ( aFltName.equalsAscii(pFilterSc40) );
        if (!bCalc3 && !bCalc4)
            aDocument.SetInsertingFromOtherDoc( true );

        if (aFltName.equalsAscii(pFilterXML))
            bRet = LoadXML( &rMedium, NULL );
        else if (aFltName.equalsAscii(pFilterSc10))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError = ScFormatFilter::Get().ScImportStarCalc10( *pStream, &aDocument );
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError, OUString( OSL_LOG_PREFIX ));
                }
                else
                    bRet = sal_True;
            }
        }
        else if (aFltName.equalsAscii(pFilterLotus))
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                
                
                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
            }

            FltError eError = ScFormatFilter::Get().ScImportLotus123( rMedium, &aDocument,
                                                ScGlobal::GetCharsetValue(sItStr));
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OUString( OSL_LOG_PREFIX ));

                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = sal_True;
            }
            else
                bRet = sal_True;
            bSetColWidths = sal_True;
            bSetRowHeights = sal_True;
        }
        else if ( aFltName.equalsAscii(pFilterExcel4) || aFltName.equalsAscii(pFilterExcel5) ||
                   aFltName.equalsAscii(pFilterExcel95) || aFltName.equalsAscii(pFilterExcel97) ||
                   aFltName.equalsAscii(pFilterEx4Temp) || aFltName.equalsAscii(pFilterEx5Temp) ||
                   aFltName.equalsAscii(pFilterEx95Temp) || aFltName.equalsAscii(pFilterEx97Temp) )
        {
            EXCIMPFORMAT eFormat = EIF_AUTO;
            if ( aFltName.equalsAscii(pFilterExcel4) || aFltName.equalsAscii(pFilterEx4Temp) )
                eFormat = EIF_BIFF_LE4;
            else if ( aFltName.equalsAscii(pFilterExcel5) || aFltName.equalsAscii(pFilterExcel95) ||
                      aFltName.equalsAscii(pFilterEx5Temp) || aFltName.equalsAscii(pFilterEx95Temp) )
                eFormat = EIF_BIFF5;
            else if ( aFltName.equalsAscii(pFilterExcel97) || aFltName.equalsAscii(pFilterEx97Temp) )
                eFormat = EIF_BIFF8;

            MakeDrawLayer(); 
            CalcOutputFactor(); 
            FltError eError = ScFormatFilter::Get().ScImportExcel( rMedium, &aDocument, eFormat );
            aDocument.UpdateFontCharSet();
            if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
                aDocument.UpdateChartListenerCollection(); 

            
            aDocument.EnsureGraphicNames();

            if (eError == SCWARN_IMPORT_RANGE_OVERFLOW)
            {
                if (!GetError())
                    SetError(eError, OUString( OSL_LOG_PREFIX ));
                bRet = sal_True;
            }
            else if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OUString( OSL_LOG_PREFIX ));
            }
            else
                bRet = true;
        }
        else if (aFltName.equalsAscii(pFilterAscii))
        {
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            ScAsciiOptions aOptions;
            sal_Bool bOptInit = false;

            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                aOptions.ReadFromString( ((const SfxStringItem*)pItem)->GetValue() );
                bOptInit = sal_True;
            }

            if ( !bOptInit )
            {
                
                

                aOptions.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aOptions.SetFieldSeps( OUString(',') );
                aOptions.SetTextSep( '"' );
            }

            FltError eError = eERR_OK;
            bool bOverflowRow, bOverflowCol, bOverflowCell;
            bOverflowRow = bOverflowCol = bOverflowCell = false;

            if( ! rMedium.IsStorage() )
            {
                ScImportExport  aImpEx( &aDocument );
                aImpEx.SetExtOptions( aOptions );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->SetStreamCharSet( aOptions.GetCharSet() );
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL() );
                    eError = bRet ? eERR_OK : SCERR_IMPORT_CONNECT;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                    bOverflowRow = aImpEx.IsOverflowRow();
                    bOverflowCol = aImpEx.IsOverflowCol();
                    bOverflowCell = aImpEx.IsOverflowCell();
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OUString( OSL_LOG_PREFIX ));
            }
            else if (!GetError() && (bOverflowRow || bOverflowCol || bOverflowCell))
            {
                
                FltError nWarn = (bOverflowRow ? SCWARN_IMPORT_ROW_OVERFLOW :
                        (bOverflowCol ? SCWARN_IMPORT_COLUMN_OVERFLOW :
                         SCWARN_IMPORT_CELL_OVERFLOW));
                SetError( nWarn, OUString( OSL_LOG_PREFIX ));
            }
            bSetColWidths = sal_True;
            bSetSimpleTextColWidths = sal_True;
        }
        else if (aFltName.equalsAscii(pFilterDBase))
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                
                

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
            }

            ScDocRowHeightUpdater::TabRanges aRecalcRanges(0);
            sal_uLong eError = DBaseImport( rMedium.GetPhysicalName(),
                    ScGlobal::GetCharsetValue(sItStr), aColWidthParam, *aRecalcRanges.mpRanges );
            aRecalcRowRangesArray.push_back(aRecalcRanges);

            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OUString( OSL_LOG_PREFIX ));
                bRet = ( eError == SCWARN_IMPORT_RANGE_OVERFLOW );
            }
            else
                bRet = sal_True;

            aColWidthRange.aStart.SetRow( 1 );  
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
        }
        else if (aFltName.equalsAscii(pFilterDif))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError;
                OUString sItStr;
                SfxItemSet*  pSet = rMedium.GetItemSet();
                const SfxPoolItem* pItem;
                if ( pSet && SFX_ITEM_SET ==
                     pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
                {
                    sItStr = ((const SfxStringItem*)pItem)->GetValue();
                }

                if (sItStr.isEmpty())
                {
                    
                    

                    sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
                }

                eError = ScFormatFilter::Get().ScImportDif( *pStream, &aDocument, ScAddress(0,0,0),
                                    ScGlobal::GetCharsetValue(sItStr));
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError, OUString( OSL_LOG_PREFIX ));

                    if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                        bRet = sal_True;
                }
                else
                    bRet = sal_True;
            }
            bSetColWidths = sal_True;
            bSetSimpleTextColWidths = sal_True;
            bSetRowHeights = sal_True;
        }
        else if (aFltName.equalsAscii(pFilterSylk))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                ScImportExport aImpEx( &aDocument );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL(), SOT_FORMATSTR_ID_SYLK );
                    eError = bRet ? eERR_OK : SCERR_IMPORT_UNKNOWN;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OUString( OSL_LOG_PREFIX ));
            bSetColWidths = sal_True;
            bSetSimpleTextColWidths = sal_True;
            bSetRowHeights = sal_True;
        }
        else if (aFltName.equalsAscii(pFilterQPro6))
        {
            FltError eError = ScFormatFilter::Get().ScImportQuattroPro( rMedium, &aDocument);
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError( eError, OUString( OSL_LOG_PREFIX ) );
                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = sal_True;
            }
            else
                bRet = sal_True;
            
            
            
            
            bSetRowHeights = sal_True;
        }
        else if (aFltName.equalsAscii(pFilterRtf))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    ScRange aRange;
                    eError = ScFormatFilter::Get().ScImportRTF( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError, OUString( OSL_LOG_PREFIX ));

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = sal_True;
                    }
                    else
                        bRet = sal_True;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                    bSetColWidths = sal_True;
                    bSetRowHeights = sal_True;
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OUString( OSL_LOG_PREFIX ));
        }
        else if (aFltName.equalsAscii(pFilterHtml) || aFltName.equalsAscii(pFilterHtmlWebQ))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            sal_Bool bWebQuery = aFltName.equalsAscii(pFilterHtmlWebQ);
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    LanguageType eLang = LANGUAGE_SYSTEM;
                    bool bDateConvert = false;
                    SfxItemSet*  pSet = rMedium.GetItemSet();
                    const SfxPoolItem* pItem;
                    if ( pSet && SFX_ITEM_SET ==
                         pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
                    {
                        OUString aFilterOption = (static_cast<const SfxStringItem*>(pItem))->GetValue();
                        lcl_parseHtmlFilterOption(aFilterOption, eLang, bDateConvert);
                    }

                    pInStream->Seek( 0 );
                    ScRange aRange;
                    
                    CalcOutputFactor();
                    SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eLang);
                    eError = ScFormatFilter::Get().ScImportHTML( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange,
                                            GetOutputFactor(), !bWebQuery, &aNumFormatter, bDateConvert );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError, OUString( OSL_LOG_PREFIX ));

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = sal_True;
                    }
                    else
                        bRet = sal_True;
                    aDocument.StartAllListeners();
                    aDocument.SetDirty();
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OUString( OSL_LOG_PREFIX ));
        }
        else
        {
            if (!GetError())
                SetError(SCERR_IMPORT_NI, OUString( OSL_LOG_PREFIX ));
        }

        if (!bCalc3)
            aDocument.SetInsertingFromOtherDoc( false );
    }
    else
    {
        OSL_FAIL("No Filter in ConvertFrom");
    }

    InitItems();
    CalcOutputFactor();
    if ( bRet && (bSetColWidths || bSetRowHeights) )
    {   
        Fraction aZoom( 1, 1 );
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom / GetOutputFactor(); 
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        VirtualDevice aVirtDev;
        
        SCTAB nTabCount = aDocument.GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        {
            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( nTab, nEndCol, nEndRow );
            aColWidthRange.aEnd.SetCol( nEndCol );
            aColWidthRange.aEnd.SetRow( nEndRow );
            ScMarkData aMark;
            aMark.SetMarkArea( aColWidthRange );
            aMark.MarkToMulti();

            
            if ( bSetColWidths )
            {
                for ( SCCOL nCol=0; nCol <= nEndCol; nCol++ )
                {
                    if (!bSetSimpleTextColWidths)
                        aColWidthParam[nCol].mbSimpleText = false;

                    sal_uInt16 nWidth = aDocument.GetOptimalColWidth(
                        nCol, nTab, &aVirtDev, nPPTX, nPPTY, aZoom, aZoom, false, &aMark,
                        &aColWidthParam[nCol] );
                    aDocument.SetColWidth( nCol, nTab,
                        nWidth + (sal_uInt16)ScGlobal::nLastColWidthExtra );
                }
            }
        }

        if (bSetRowHeights)
        {
            
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(aDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), NULL);
            aUpdater.update();
        }
        else if (!aRecalcRowRangesArray.empty())
        {
            
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(aDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), &aRecalcRowRangesArray);
            aUpdater.update();
        }
    }
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );


    
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = false;

    return bRet;
}

bool ScDocShell::LoadExternal( SfxMedium& rMed )
{
    const SfxFilter* pFilter = rMed.GetFilter();
    if (!pFilter)
        return false;

    if (pFilter->GetProviderName() == "orcus")
    {
        ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
        if (!pOrcus)
            return false;

        const OUString& rFilterName = pFilter->GetName();
        if (rFilterName == "gnumeric")
        {
            if (!pOrcus->importGnumeric(aDocument, rMed))
                return false;
        }
        else if (rFilterName == "csv")
        {
            if (!pOrcus->importCSV(aDocument, rMed))
                return false;
        }
        else if (rFilterName == "xlsx")
        {
            if (!pOrcus->importXLSX(aDocument, rMed))
                return false;
        }
        else if (rFilterName == "ods")
        {
            if (!pOrcus->importODS(aDocument, rMed))
                return false;
        }

        FinishedLoading(SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES);
        return true;
    }

    return false;
}


ScDocShell::PrepareSaveGuard::PrepareSaveGuard( ScDocShell& rDocShell )
    : mrDocShell( rDocShell)
{
    

    ScChartListenerCollection* pCharts = mrDocShell.aDocument.GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                           
    mrDocShell.aDocument.StopTemporaryChartLock();
    if (mrDocShell.pAutoStyleList)
        mrDocShell.pAutoStyleList->ExecuteAllNow();             
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            pRefMgr->setAllCacheTableReferencedStati( false);
            mrDocShell.aDocument.MarkUsedExternalReferences();  
        }
    }
    if (mrDocShell.GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        mrDocShell.SfxObjectShell::SetVisArea( Rectangle() );   
}

ScDocShell::PrepareSaveGuard::~PrepareSaveGuard()
{
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            
            pRefMgr->setAllCacheTableReferencedStati( true);
        }
    }
}


sal_Bool ScDocShell::Save()
{
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            Window* pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                pSysWin->SetAccessibleName(OUString());
            }
        }
    }
    
    sal_Bool bRet = SfxObjectShell::Save();
    if( bRet )
        bRet = SaveXML( GetMedium(), NULL );
    return bRet;
}

namespace {

/**
 * Remove the file name from the full path, to keep only the directory path.
 */
void popFileName(OUString& rPath)
{
    if (!rPath.isEmpty())
    {
        INetURLObject aURLObj(rPath);
        aURLObj.removeSegment();
        rPath = aURLObj.GetMainURL(INetURLObject::NO_DECODE);
    }
}

}

sal_Bool ScDocShell::SaveAs( SfxMedium& rMedium )
{
    OUString aCurPath; 
    const SfxMedium* pCurMedium = GetMedium();
    if (pCurMedium)
    {
        aCurPath = pCurMedium->GetName();
        popFileName(aCurPath);
    }

    if (!aCurPath.isEmpty())
    {
        
        OUString aNewPath = rMedium.GetName();
        popFileName(aNewPath);
        OUString aRel = URIHelper::simpleNormalizedMakeRelative(aCurPath, aNewPath);
        if (!aRel.isEmpty())
        {
            
            aDocument.InvalidateStreamOnSave();
        }
    }

    ScTabViewShell* pViewShell = GetBestViewShell();
    bool bNeedsRehash = ScPassHashHelper::needsPassHashRegen(aDocument, PASSHASH_SHA1);
    if (bNeedsRehash)
        
        bNeedsRehash = ScPassHashHelper::needsPassHashRegen(aDocument, PASSHASH_XL, PASSHASH_SHA1);

    if (pViewShell && bNeedsRehash)
    {
        if (!pViewShell->ExecuteRetypePassDlg(PASSHASH_SHA1))
            
            return false;
    }


    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    OUString aFltName = rMedium.GetFilter()->GetFilterName();
    bool bChartExport = aFltName.indexOf("chart8") != -1;

    
    sal_Bool bRet = false;
    if(!bChartExport)
    {
        bRet = SfxObjectShell::SaveAs( rMedium );
        bRet = SaveXML( &rMedium, NULL );
    }
    else
    {
        bRet = SaveCurrentChart( rMedium );
    }

    return bRet;
}


sal_Bool ScDocShell::IsInformationLost()
{
    
    return SfxObjectShell::IsInformationLost();
}

namespace {


sal_Int32 lcl_ScDocShell_GetColWidthInChars( sal_uInt16 nWidth )
{
    double f = nWidth;
    f *= 1328.0 / 25.0;
    f += 90.0;
    f *= 1.0 / 23.0;
    f /= 256.0;

    return sal_Int32( f );
}


void lcl_ScDocShell_GetFixedWidthString( OUString& rStr, const ScDocument& rDoc,
        SCTAB nTab, SCCOL nCol, sal_Bool bValue, SvxCellHorJustify eHorJust )
{
    OUString aString = rStr;
    sal_Int32 nLen = lcl_ScDocShell_GetColWidthInChars(
            rDoc.GetColWidth( nCol, nTab ) );
    
    if ( nLen < aString.getLength() )
    {
        OUStringBuffer aReplacement;
        if (bValue)
            aReplacement.append("###");
        else
            aReplacement.append(aString);
        
        
        aString = comphelper::string::truncateToLength(aReplacement, nLen).makeStringAndClear();
    }
    if ( nLen > aString.getLength() )
    {
        if ( bValue && eHorJust == SVX_HOR_JUSTIFY_STANDARD )
            eHorJust = SVX_HOR_JUSTIFY_RIGHT;
        sal_Int32 nBlanks = nLen - aString.getLength();
        switch ( eHorJust )
        {
            case SVX_HOR_JUSTIFY_RIGHT:
            {
                OUStringBuffer aTmp;
                aTmp = comphelper::string::padToLength( aTmp, nBlanks, ' ' );
                aString = aTmp.append(aString).makeStringAndClear();
            }
            break;
            case SVX_HOR_JUSTIFY_CENTER:
            {
                sal_Int32 nLeftPad = nBlanks / 2;
                OUStringBuffer aTmp;
                comphelper::string::padToLength( aTmp, nLeftPad, ' ' );
                aTmp.append(aString);
                comphelper::string::padToLength( aTmp, nLen, ' ' );
                aString = aTmp.makeStringAndClear();
            }
            break;
            default:
            {
                OUStringBuffer aTmp(aString);
                comphelper::string::padToLength( aTmp, nLen, ' ' );
                aString = aTmp.makeStringAndClear();
            }
        }
    }
    rStr = aString;
}


void lcl_ScDocShell_WriteEmptyFixedWidthString( SvStream& rStream,
        const ScDocument& rDoc, SCTAB nTab, SCCOL nCol )
{
    OUString aString;
    lcl_ScDocShell_GetFixedWidthString( aString, rDoc, nTab, nCol, false,
            SVX_HOR_JUSTIFY_STANDARD );
    rStream.WriteUnicodeOrByteText( aString );
}

template<typename StrT, typename SepCharT>
sal_Int32 getTextSepPos(
    const StrT& rStr, const ScImportOptions& rAsciiOpt, const SepCharT& rTextSep, const SepCharT& rFieldSep, bool& rNeedQuotes)
{
    
    
    sal_Int32 nPos = rStr.indexOf(rTextSep);
    rNeedQuotes = rAsciiOpt.bQuoteAllText || (nPos >= 0) ||
        (rStr.indexOf(rFieldSep) >= 0) ||
        (rStr.indexOf('\n') >= 0);
    return nPos;
}

template<typename StrT, typename StrBufT>
void escapeTextSep(sal_Int32 nPos, const StrT& rStrDelim, StrT& rStr)
{
    while (nPos >= 0)
    {
        StrBufT aBuf(rStr);
        aBuf.insert(nPos, rStrDelim);
        rStr = aBuf.makeStringAndClear();
        nPos = rStr.indexOf(rStrDelim, nPos+1+rStrDelim.getLength());
    }
}

}

void ScDocShell::AsciiSave( SvStream& rStream, const ScImportOptions& rAsciiOpt )
{
    sal_Unicode cDelim    = rAsciiOpt.nFieldSepCode;
    sal_Unicode cStrDelim = rAsciiOpt.nTextSepCode;
    rtl_TextEncoding eCharSet      = rAsciiOpt.eCharSet;
    bool bFixedWidth      = rAsciiOpt.bFixedWidth;
    bool bSaveAsShown     = rAsciiOpt.bSaveAsShown;
    bool bShowFormulas    = rAsciiOpt.bSaveFormulas;

    rtl_TextEncoding eOldCharSet = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eCharSet );
    sal_uInt16 nOldNumberFormatInt = rStream.GetNumberFormatInt();
    OString aStrDelimEncoded;    
    OUString aStrDelimDecoded;     
    OString aDelimEncoded;
    OUString aDelimDecoded;
    bool bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rStream.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = false;
    }
    else
    {
        aStrDelimEncoded = OString(&cStrDelim, 1, eCharSet);
        aDelimEncoded = OString(&cDelim, 1, eCharSet);
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
            {
                aStrDelimDecoded = OStringToOUString(aStrDelimEncoded, eCharSet);
                aDelimDecoded = OStringToOUString(aDelimEncoded, eCharSet);
            }
        }
        else
            bContextOrNotAsciiEncoding = false;
    }

    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCTAB nTab = GetSaveTab();
    SCCOL nEndCol;
    SCROW nEndRow;
    aDocument.GetCellArea( nTab, nEndCol, nEndRow );

    ScProgress aProgress( this, ScGlobal::GetRscString( STR_SAVE_DOC ), nEndRow );

    OUString aString;

    bool bTabProtect = aDocument.IsTabProtected( nTab );

    SCCOL nCol;
    SCROW nRow;
    SCCOL nNextCol = nStartCol;
    SCROW nNextRow = nStartRow;
    SCCOL nEmptyCol;
    SCROW nEmptyRow;
    SvNumberFormatter& rFormatter = *aDocument.GetFormatTable();

    ScHorizontalCellIterator aIter( &aDocument, nTab, nStartCol, nStartRow,
        nEndCol, nEndRow );
    ScRefCellValue* pCell;
    while ( ( pCell = aIter.GetNext( nCol, nRow ) ) != NULL )
    {
        bool bProgress = false;     
        if ( nNextRow < nRow )
        {   
            bProgress = true;
            for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
            {   
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            endlub( rStream );
            nNextRow++;
            for ( nEmptyRow = nNextRow; nEmptyRow < nRow; nEmptyRow++ )
            {   
                for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
                {
                    if ( bFixedWidth )
                        lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                                aDocument, nTab, nEmptyCol );
                    else if ( cDelim != 0 )
                        rStream.WriteUniOrByteChar( cDelim );
                }
                endlub( rStream );
            }
            for ( nEmptyCol = nStartCol; nEmptyCol < nCol; nEmptyCol++ )
            {   
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            nNextRow = nRow;
        }
        else if ( nNextCol < nCol )
        {   
            for ( nEmptyCol = nNextCol; nEmptyCol < nCol; nEmptyCol++ )
            {   
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
        }
        if ( nCol == nEndCol )
        {
            bProgress = true;
            nNextCol = nStartCol;
            nNextRow = nRow + 1;
        }
        else
            nNextCol = nCol + 1;

        CellType eType = pCell->meType;
        ScAddress aPos(nCol, nRow, nTab);
        if ( bTabProtect )
        {
            const ScProtectionAttr* pProtAttr =
                (const ScProtectionAttr*) aDocument.GetAttr(
                                                            nCol, nRow, nTab, ATTR_PROTECTION );
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;  
        }
        bool bString;
        switch ( eType )
        {
            case CELLTYPE_NONE:
                aString = "";
                bString = false;
                break;
            case CELLTYPE_FORMULA :
                {
                    sal_uInt16 nErrCode;
                    if ( bShowFormulas )
                    {
                        pCell->mpFormula->GetFormula(aString);
                        bString = true;
                    }
                    else if ((nErrCode = pCell->mpFormula->GetErrCode()) != 0)
                    {
                        aString = ScGlobal::GetErrorString( nErrCode );
                        bString = true;
                    }
                    else if (pCell->mpFormula->IsValue())
                    {
                        sal_uInt32 nFormat = aDocument.GetNumberFormat(aPos);
                        if ( bFixedWidth || bSaveAsShown )
                        {
                            Color* pDummy;
                            aString = ScCellFormat::GetString(aDocument, aPos, nFormat, &pDummy, rFormatter);
                            bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                        }
                        else
                        {
                            ScCellFormat::GetInputString(*pCell, nFormat, aString, rFormatter, &aDocument);
                            bString = false;
                        }
                    }
                    else
                    {
                        if ( bSaveAsShown )
                        {
                            sal_uInt32 nFormat = aDocument.GetNumberFormat(aPos);
                            Color* pDummy;
                            aString = ScCellFormat::GetString(aDocument, aPos, nFormat, &pDummy, rFormatter);
                        }
                        else
                            aString = pCell->mpFormula->GetString().getString();
                        bString = true;
                    }
                }
                break;
            case CELLTYPE_STRING :
                if ( bSaveAsShown )
                {
                    sal_uInt32 nFormat = aDocument.GetNumberFormat(aPos);
                    Color* pDummy;
                    aString = ScCellFormat::GetString(aDocument, aPos, nFormat, &pDummy, rFormatter);
                }
                else
                    aString = pCell->mpString->getString();
                bString = true;
                break;
            case CELLTYPE_EDIT :
                {
                    const EditTextObject* pObj = pCell->mpEditText;
                    EditEngine& rEngine = aDocument.GetEditEngine();
                    rEngine.SetText( *pObj);
                    aString = rEngine.GetText();  
                    bString = true;
                }
                break;
            case CELLTYPE_VALUE :
                {
                    sal_uInt32 nFormat;
                    aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                    if ( bFixedWidth || bSaveAsShown )
                    {
                        Color* pDummy;
                        aString = ScCellFormat::GetString(aDocument, aPos, nFormat, &pDummy, rFormatter);
                        bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                    }
                    else
                    {
                        ScCellFormat::GetInputString(*pCell, nFormat, aString, rFormatter, &aDocument);
                        bString = false;
                    }
                }
                break;
            default:
                OSL_FAIL( "ScDocShell::AsciiSave: unknown CellType" );
                aString = "";
                bString = false;
        }

        if ( bFixedWidth )
        {
            SvxCellHorJustify eHorJust = (SvxCellHorJustify)
                ((const SvxHorJustifyItem*) aDocument.GetAttr( nCol, nRow,
                nTab, ATTR_HOR_JUSTIFY ))->GetValue();
            lcl_ScDocShell_GetFixedWidthString( aString, aDocument, nTab, nCol,
                    !bString, eHorJust );
            rStream.WriteUnicodeOrByteText( aString );
        }
        else
        {
            OUString aUniString = aString;
            if (!bString && cStrDelim != 0 && !aUniString.isEmpty())
            {
                sal_Unicode c = aUniString[0];
                bString = (c == cStrDelim || c == ' ' ||
                        aUniString.endsWith(" ") ||
                        aUniString.indexOf(cStrDelim) >= 0);
                if (!bString && cDelim != 0)
                    bString = (aUniString.indexOf(cDelim) >= 0);
            }
            if ( bString )
            {
                if ( cStrDelim != 0 ) 
                {
                    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                    {
                        bool bNeedQuotes = false;
                        sal_Int32 nPos = getTextSepPos(
                            aUniString, rAsciiOpt, cStrDelim, cDelim, bNeedQuotes);

                        escapeTextSep<OUString, OUStringBuffer>(
                            nPos, OUString(cStrDelim), aUniString);

                        if ( bNeedQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        write_uInt16s_FromOUString(rStream, aUniString);
                        if ( bNeedQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                    }
                    else
                    {
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        

                        if ( bContextOrNotAsciiEncoding )
                        {
                            
                            OString aStrEnc = OUStringToOString(aUniString, eCharSet);
                            
                            OUString aStrDec = OStringToOUString(aStrEnc, eCharSet);

                            
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrDec, rAsciiOpt, aStrDelimDecoded, aDelimDecoded, bNeedQuotes);

                            escapeTextSep<OUString, OUStringBuffer>(
                                nPos, aStrDelimDecoded, aStrDec);

                            
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                            rStream.WriteUnicodeOrByteText( aStrDec, eCharSet );
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        }
                        else
                        {
                            OString aStrEnc = OUStringToOString(aUniString, eCharSet);

                            
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrEnc, rAsciiOpt, aStrDelimEncoded, aDelimEncoded, bNeedQuotes);

                            escapeTextSep<OString, OStringBuffer>(
                                nPos, aStrDelimEncoded, aStrEnc);

                            
                            if ( bNeedQuotes )
                                rStream.Write(
                                    aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                            rStream.Write(aStrEnc.getStr(), aStrEnc.getLength());
                            if ( bNeedQuotes )
                                rStream.Write(
                                    aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                        }
                    }
                }
                else
                    rStream.WriteUnicodeOrByteText( aUniString );
            }
            else
                rStream.WriteUnicodeOrByteText( aUniString );
        }

        if( nCol < nEndCol )
        {
            if(cDelim!=0) 
                rStream.WriteUniOrByteChar( cDelim );
        }
        else
            endlub( rStream );

        if ( bProgress )
            aProgress.SetStateOnPercent( nRow );
    }

    
    if ( nNextRow <= nEndRow )
    {
        for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {   
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
        endlub( rStream );
        nNextRow++;
    }
    for ( nEmptyRow = nNextRow; nEmptyRow <= nEndRow; nEmptyRow++ )
    {   
        for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
        endlub( rStream );
    }

    rStream.SetStreamCharSet( eOldCharSet );
    rStream.SetNumberFormatInt( nOldNumberFormatInt );
}

sal_Bool ScDocShell::ConvertTo( SfxMedium &rMed )
{
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    
    

    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow(); 
    if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SfxObjectShell::SetVisArea( Rectangle() ); 

    OSL_ENSURE( rMed.GetFilter(), "Filter == 0" );

    sal_Bool bRet = false;
    OUString aFltName = rMed.GetFilter()->GetFilterName();

    if (aFltName.equalsAscii(pFilterXML))
    {
        
        OSL_FAIL("XML filter in ConvertFrom?!");
        bRet = SaveXML( &rMed, NULL );
    }
    else if (aFltName.equalsAscii(pFilterExcel5) || aFltName.equalsAscii(pFilterExcel95) ||
             aFltName.equalsAscii(pFilterExcel97) || aFltName.equalsAscii(pFilterEx5Temp) ||
             aFltName.equalsAscii(pFilterEx95Temp) || aFltName.equalsAscii(pFilterEx97Temp))
    {
        WaitObject aWait( GetActiveDialogParent() );

        bool bDoSave = true;
        if( ScTabViewShell* pViewShell = GetBestViewShell() )
        {
            ScExtDocOptions* pExtDocOpt = aDocument.GetExtDocOptions();
            if( !pExtDocOpt )
                aDocument.SetExtDocOptions( pExtDocOpt = new ScExtDocOptions );
            pViewShell->GetViewData()->WriteExtOptions( *pExtDocOpt );

            /*  #i104990# If the imported document contains a medium
                password, determine if we can save it, otherwise ask the users
                whether they want to save without it. */
            if( (rMed.GetFilter()->GetFilterFlags() & SFX_FILTER_ENCRYPTION) == 0 )
            {
                SfxItemSet* pItemSet = rMed.GetItemSet();
                const SfxPoolItem* pItem = 0;
                if( pItemSet && pItemSet->GetItemState( SID_PASSWORD, true, &pItem ) == SFX_ITEM_SET )
                {
                    bDoSave = ScWarnPassword::WarningOnPassword( rMed );
                    
                    if( bDoSave )
                        pItemSet->ClearItem( SID_PASSWORD );
                }
            }

            if( bDoSave )
            {
                bool bNeedRetypePassDlg = ScPassHashHelper::needsPassHashRegen( aDocument, PASSHASH_XL );
                bDoSave = !bNeedRetypePassDlg || pViewShell->ExecuteRetypePassDlg( PASSHASH_XL );
            }
        }

        if( bDoSave )
        {
            ExportFormatExcel eFormat = ExpBiff5;
            if( aFltName.equalsAscii( pFilterExcel97 ) || aFltName.equalsAscii( pFilterEx97Temp ) )
                eFormat = ExpBiff8;
            FltError eError = ScFormatFilter::Get().ScExportExcel5( rMed, &aDocument, eFormat, RTL_TEXTENCODING_MS_1252 );

            if( eError && !GetError() )
                SetError( eError, OUString( OSL_LOG_PREFIX ) );

            
            bRet = ((eError & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK) || (eError == eERR_OK);
        }
        else
        {
            
            SetError( ERRCODE_ABORT, OUString( OSL_LOG_PREFIX ) );
        }
    }
    else if (aFltName.equalsAscii(pFilterAscii))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if ( sItStr.isEmpty() )
            {
                
                

                ScImportOptions aDefOptions( ',', '"', RTL_TEXTENCODING_MS_1252 );
                sItStr = aDefOptions.BuildString();
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScImportOptions aOptions( sItStr );
            AsciiSave( *pStream, aOptions );
            bRet = sal_True;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, OUString( OSL_LOG_PREFIX ));
        }
    }
    else if (aFltName.equalsAscii(pFilterDBase))
    {
        OUString sCharSet;
        SfxItemSet* pSet = rMed.GetItemSet();
        const SfxPoolItem* pItem;
        if ( pSet && SFX_ITEM_SET ==
             pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
        {
            sCharSet = ((const SfxStringItem*)pItem)->GetValue();
        }

        if (sCharSet.isEmpty())
        {
            
            

            sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
        }

        WaitObject aWait( GetActiveDialogParent() );
        
        rMed.CloseOutStream();
        bool bHasMemo = false;

        sal_uLong eError = DBaseExport(
            rMed.GetPhysicalName(), ScGlobal::GetCharsetValue(sCharSet), bHasMemo);

        if ( eError != eERR_OK && (eError & ERRCODE_WARNING_MASK) )
        {
            eError = eERR_OK;
        }

        INetURLObject aTmpFile( rMed.GetPhysicalName(), INET_PROT_FILE );
        if ( bHasMemo )
            aTmpFile.setExtension(OUString("dbt"));
        if ( eError != eERR_OK )
        {
            if (!GetError())
                SetError(eError, OUString( OSL_LOG_PREFIX ));
            if ( bHasMemo && IsDocument( aTmpFile ) )
                KillFile( aTmpFile );
        }
        else
        {
            bRet = sal_True;
            if ( bHasMemo )
            {
                SfxStringItem* pNameItem =
                    (SfxStringItem*) rMed.GetItemSet()->GetItem( SID_FILE_NAME );
                INetURLObject aDbtFile( pNameItem->GetValue(), INET_PROT_FILE );
                aDbtFile.setExtension(OUString("dbt"));
                if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
                    bRet = false;
                if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
                    bRet = false;
                if ( !bRet )
                {
                    KillFile( aTmpFile );
                    if ( !GetError() )
                        SetError( SCERR_EXPORT_DATA, OUString( OSL_LOG_PREFIX ) );
                }
            }
        }
    }
    else if (aFltName.equalsAscii(pFilterDif))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                
                

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScFormatFilter::Get().ScExportDif( *pStream, &aDocument, ScAddress(0,0,0),
                ScGlobal::GetCharsetValue(sItStr) );
            bRet = sal_True;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, OUString( OSL_LOG_PREFIX ));
        }
    }
    else if (aFltName.equalsAscii(pFilterSylk))
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            WaitObject aWait( GetActiveDialogParent() );

            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( 0, nEndCol, nEndRow );
            ScRange aRange( 0,0,0, nEndCol,nEndRow,0 );

            ScImportExport aImExport( &aDocument, aRange );
            aImExport.SetFormulas( true );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_SYLK );
        }
    }
    else if (aFltName.equalsAscii(pFilterHtml))
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            WaitObject aWait( GetActiveDialogParent() );
            ScImportExport aImExport( &aDocument );
            aImExport.SetStreamPath( rMed.GetName() );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_HTML );
            if ( bRet && !aImExport.GetNonConvertibleChars().isEmpty() )
                SetError( *new StringErrorInfo(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_INFO ), OUString( OSL_LOG_PREFIX ) );
        }
    }
    else
    {
        if (GetError())
            SetError(SCERR_IMPORT_NI, OUString( OSL_LOG_PREFIX ) );
    }
    return bRet;
}


sal_Bool ScDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor )
{
    return SfxObjectShell::SaveCompleted( xStor );
}


sal_Bool ScDocShell::DoSaveCompleted( SfxMedium * pNewStor )
{
    sal_Bool bRet = SfxObjectShell::DoSaveCompleted( pNewStor );

    
    Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
    return bRet;
}


sal_Bool ScDocShell::QuerySlotExecutable( sal_uInt16 nSlotId )
{
    

    using namespace ::com::sun::star::script::vba;

    sal_Int32 nVbaEventId = VBAEventId::NO_EVENT;
    uno::Sequence< uno::Any > aArgs;
    switch( nSlotId )
    {
        case SID_SAVEDOC:
        case SID_SAVEASDOC:
            nVbaEventId = VBAEventId::WORKBOOK_BEFORESAVE;
            aArgs.realloc( 1 );
            aArgs[ 0 ] <<= (nSlotId == SID_SAVEASDOC);
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            nVbaEventId = VBAEventId::WORKBOOK_BEFOREPRINT;
        break;
    }

    sal_Bool bSlotExecutable = sal_True;
    if( nVbaEventId != VBAEventId::NO_EVENT ) try
    {
        uno::Reference< XVBAEventProcessor > xEventProcessor( aDocument.GetVbaEventProcessor(), uno::UNO_QUERY_THROW );
        xEventProcessor->processVbaEvent( nVbaEventId, aArgs );
    }
    catch( util::VetoException& )
    {
        bSlotExecutable = false;
    }
    catch( uno::Exception& )
    {
    }
    return bSlotExecutable;
}


bool ScDocShell::PrepareClose( sal_Bool bUI )
{
    if(SC_MOD()->GetCurRefDlgId()>0)
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
            if(pViewSh!=NULL)
            {
                Window *pWin=pViewSh->GetWindow();
                if(pWin!=NULL) pWin->GrabFocus();
            }
        }

        return false;
    }
    if ( aDocument.IsInLinkUpdate() || aDocument.IsInInterpreter() )
    {
        ErrorMessage(STR_CLOSE_ERROR_LINK);
        return false;
    }

    DoEnterHandler();

    
    if( !IsInPrepareClose() )
    {
        try
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( aDocument.GetVbaEventProcessor(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_BEFORECLOSE, aArgs );
        }
        catch( util::VetoException& )
        {
            
            return false;
        }
        catch( uno::Exception& )
        {
        }
    }
    

    bool nRet = SfxObjectShell::PrepareClose( bUI );
    if (nRet) 
        aDocument.EnableIdle(false); 

    return nRet;
}

void ScDocShell::PrepareReload()
{
    SfxObjectShell::PrepareReload(); 

    
    
    
    
    //
    
    aDocument.GetDocLinkManager().disconnectDdeLinks();
}


OUString ScDocShell::GetOwnFilterName()
{
    return OUString(pFilterSc50);
}

OUString ScDocShell::GetHtmlFilterName()
{
    return OUString(pFilterHtml);
}

OUString ScDocShell::GetWebQueryFilterName()
{
    return OUString(pFilterHtmlWebQ);
}

OUString ScDocShell::GetAsciiFilterName()
{
    return OUString(pFilterAscii);
}

OUString ScDocShell::GetLotusFilterName()
{
    return OUString(pFilterLotus);
}

OUString ScDocShell::GetDBaseFilterName()
{
    return OUString(pFilterDBase);
}

OUString ScDocShell::GetDifFilterName()
{
    return OUString(pFilterDif);
}

bool ScDocShell::HasAutomaticTableName( const OUString& rFilter )
{
    
    

    return rFilter.equalsAscii( pFilterAscii )
        || rFilter.equalsAscii( pFilterLotus )
        || rFilter.equalsAscii( pFilterExcel4 )
        || rFilter.equalsAscii( pFilterEx4Temp )
        || rFilter.equalsAscii( pFilterDBase )
        || rFilter.equalsAscii( pFilterDif )
        || rFilter.equalsAscii( pFilterSylk )
        || rFilter.equalsAscii( pFilterHtml )
        || rFilter.equalsAscii( pFilterRtf );
}

#if ! ENABLE_TELEPATHY
ScDocFunc *ScDocShell::CreateDocFunc()
{
    return new ScDocFuncDirect( *this );
}
#else
ScCollaboration* ScDocShell::GetCollaboration()
{
    return mpCollaboration;
}
#endif

ScDocShell::ScDocShell( const ScDocShell& rShell ) :
    SvRefBase(),
    SotObject(),
    SfxObjectShell( rShell.GetCreateMode() ),
    SfxListener(),
    aDocument       ( SCDOCMODE_DOCUMENT, this ),
    aDdeTextFmt(OUString("TEXT")),
    nPrtToScreenFactor( 1.0 ),
    pImpl           ( new DocShell_Impl ),
    bHeaderOn       ( true ),
    bFooterOn       ( true ),
    bIsEmpty        ( true ),
    bIsInUndo       ( false ),
    bDocumentModifiedPending( false ),
    bUpdateEnabled  ( true ),
    nDocumentLock   ( 0 ),
    nCanUpdate (com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG),
    pOldAutoDBRange ( NULL ),
    pDocHelper      ( NULL ),
    pAutoStyleList  ( NULL ),
    pPaintLockData  ( NULL ),
    pSolverSaveData ( NULL ),
    pSheetSaveData  ( NULL ),
    pModificator    ( NULL )
#if ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = rShell.bIsInplace;

    pDocFunc = CreateDocFunc();

    
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);

    GetPageOnFromPageStyleSet( NULL, 0, bHeaderOn, bFooterOn );
    SetHelpId( HID_SCSHELL_DOCSH );

    
}



ScDocShell::ScDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell( i_nSfxCreationFlags ),
    aDocument       ( SCDOCMODE_DOCUMENT, this ),
    aDdeTextFmt(OUString("TEXT")),
    nPrtToScreenFactor( 1.0 ),
    pImpl           ( new DocShell_Impl ),
    bHeaderOn       ( true ),
    bFooterOn       ( true ),
    bIsEmpty        ( true ),
    bIsInUndo       ( false ),
    bDocumentModifiedPending( false ),
    bUpdateEnabled  ( true ),
    nDocumentLock   ( 0 ),
    nCanUpdate (com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG),
    pOldAutoDBRange ( NULL ),
    pDocHelper      ( NULL ),
    pAutoStyleList  ( NULL ),
    pPaintLockData  ( NULL ),
    pSolverSaveData ( NULL ),
    pSheetSaveData  ( NULL ),
    pModificator    ( NULL )
#if ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
    

    pDocFunc = CreateDocFunc();

    
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);
    SetHelpId( HID_SCSHELL_DOCSH );

    aDocument.GetDBCollection()->SetRefreshHandler(
        LINK( this, ScDocShell, RefreshDBDataHdl ) );

    
}



ScDocShell::~ScDocShell()
{
    ResetDrawObjectShell(); 

    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        EndListening(*pStlPool);
    EndListening(*this);

    delete pAutoStyleList;

    SfxApplication *pSfxApp = SFX_APP();
    if ( pSfxApp->GetDdeService() ) 
        pSfxApp->RemoveDdeTopic( this );

    delete pDocFunc;
    delete aDocument.mpUndoManager;
    aDocument.mpUndoManager = 0;
    delete pImpl;

    delete pPaintLockData;

    delete pSolverSaveData;
    delete pSheetSaveData;
    delete pOldAutoDBRange;

    if (pModificator)
    {
        OSL_FAIL("The Modificator should not exist");
        delete pModificator;
    }
#if ENABLE_TELEPATHY
    delete mpCollaboration;
#endif
}



::svl::IUndoManager* ScDocShell::GetUndoManager()
{
    return aDocument.GetUndoManager();
}

void ScDocShell::SetModified( sal_Bool bModified )
{
    if ( SfxObjectShell::IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
    }
}


void ScDocShell::SetDocumentModified( bool bIsModified /* = true */ )
{
    
    
    

    if ( pPaintLockData && bIsModified )
    {
        
        
        aDocument.Broadcast(ScHint(SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS));
        aDocument.InvalidateTableArea();    
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        pPaintLockData->SetModified(); 
        return;
    }

    SetDrawModified( bIsModified );

    if ( bIsModified )
    {
        if ( aDocument.IsAutoCalcShellDisabled() )
            SetDocumentModifiedPending( true );
        else
        {
            SetDocumentModifiedPending( false );
            aDocument.InvalidateStyleSheetUsage();
            aDocument.InvalidateTableArea();
            aDocument.InvalidateLastTableOpParams();
            aDocument.Broadcast(ScHint(SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS));
            if ( aDocument.IsForcedFormulaPending() && aDocument.GetAutoCalc() )
                aDocument.CalcFormulaTree( true );
            PostDataChanged();

            
            
            
            

            ScDetOpList* pList = aDocument.GetDetOpList();
            if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
                 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
            {
                GetDocFunc().DetectiveRefresh(true);    
            }
            aDocument.SetDetectiveDirty(false);         
        }

        
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
    }
}

/**
 * SetDrawModified - without Formula update
 *
 * Drawing also needs to be updated for the normal SetDocumentModified
 * e.g.: when deleting tables etc.
 */
void ScDocShell::SetDrawModified( bool bIsModified /* = true */ )
{
    sal_Bool bUpdate = ( (bIsModified ? 1 : 0) != IsModified() );

    SetModified( bIsModified );

    SfxBindings* pBindings = GetViewBindings();
    if (bUpdate)
    {
        if (pBindings)
        {
            pBindings->Invalidate( SID_SAVEDOC );
            pBindings->Invalidate( SID_DOC_MODIFIED );
        }
    }

    if (bIsModified)
    {
        if (pBindings)
        {
            
            
            
            pBindings->Invalidate( SID_UNDO );
            pBindings->Invalidate( SID_REDO );
            pBindings->Invalidate( SID_REPEAT );
        }

        if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
        {
            aDocument.UpdateChartListenerCollection();
            SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));    
        }
        SC_MOD()->AnythingChanged();
    }
}

void ScDocShell::SetInUndo(bool bSet)
{
    bIsInUndo = bSet;
}


void ScDocShell::GetDocStat( ScDocStat& rDocStat )
{
    SfxPrinter* pPrinter = GetPrinter();

    aDocument.GetDocStat( rDocStat );
    rDocStat.nPageCount = 0;

    if ( pPrinter )
        for ( SCTAB i=0; i<rDocStat.nTableCount; i++ )
            rDocStat.nPageCount = sal::static_int_cast<sal_uInt16>( rDocStat.nPageCount +
                (sal_uInt16) ScPrintFunc( this, pPrinter, i ).GetTotalPages() );
}


SfxDocumentInfoDialog* ScDocShell::CreateDocumentInfoDialog(
                                         Window *pParent, const SfxItemSet &rSet )
{
    SfxDocumentInfoDialog* pDlg   = new SfxDocumentInfoDialog( pParent, rSet );
    ScDocShell*            pDocSh = PTR_CAST(ScDocShell,SfxObjectShell::Current());

    
    if( pDocSh == this )
    {
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
        ::CreateTabPage ScDocStatPageCreate =   pFact->GetTabPageCreatorFunc( RID_SCPAGE_STAT );
        OSL_ENSURE(ScDocStatPageCreate, "Tabpage create fail!");
        pDlg->AddFontTabPage();
        pDlg->AddTabPage( 42,
            ScGlobal::GetRscString( STR_DOC_STAT ),
            ScDocStatPageCreate,
            NULL);
    }
    return pDlg;
}

Window* ScDocShell::GetActiveDialogParent()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        return pViewSh->GetDialogParent();
    else
        return Application::GetDefDialogParent();
}

void ScDocShell::SetSolverSaveData( const ScOptSolverSave& rData )
{
    delete pSolverSaveData;
    pSolverSaveData = new ScOptSolverSave( rData );
}

ScSheetSaveData* ScDocShell::GetSheetSaveData()
{
    if (!pSheetSaveData)
        pSheetSaveData = new ScSheetSaveData;

    return pSheetSaveData;
}

namespace {

void removeKeysIfExists(Reference<ui::XAcceleratorConfiguration>& xScAccel, const vector<const awt::KeyEvent*>& rKeys)
{
    vector<const awt::KeyEvent*>::const_iterator itr = rKeys.begin(), itrEnd = rKeys.end();
    for (; itr != itrEnd; ++itr)
    {
        const awt::KeyEvent* p = *itr;
        if (!p)
            continue;

        try
        {
            xScAccel->removeKeyEvent(*p);
        }
        catch (const container::NoSuchElementException&) {}
    }
}

}

void ScDocShell::ResetKeyBindings( ScOptionsUtil::KeyBindingType eType )
{
    using namespace ::com::sun::star::ui;

    Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    if (!xContext.is())
        return;

    Reference<XModuleUIConfigurationManagerSupplier> xModuleCfgSupplier(
        theModuleUIConfigurationManagerSupplier::get(xContext) );

    
    Reference<XUIConfigurationManager> xConfigMgr =
        xModuleCfgSupplier->getUIConfigurationManager(
            OUString("com.sun.star.sheet.SpreadsheetDocument"));

    if (!xConfigMgr.is())
        return;

    
    Reference<XAcceleratorConfiguration> xScAccel = xConfigMgr->getShortCutManager();

    if (!xScAccel.is())
        return;

    vector<const awt::KeyEvent*> aKeys;
    aKeys.reserve(4);

    
    awt::KeyEvent aBackspace;
    aBackspace.KeyCode = awt::Key::BACKSPACE;
    aBackspace.Modifiers = 0;
    aKeys.push_back(&aBackspace);

    
    awt::KeyEvent aDelete;
    aDelete.KeyCode = awt::Key::DELETE;
    aDelete.Modifiers = 0;
    aKeys.push_back(&aDelete);

    
    awt::KeyEvent aCtrlD;
    aCtrlD.KeyCode = awt::Key::D;
    aCtrlD.Modifiers = awt::KeyModifier::MOD1;
    aKeys.push_back(&aCtrlD);

    
    awt::KeyEvent aAltDown;
    aAltDown.KeyCode = awt::Key::DOWN;
    aAltDown.Modifiers = awt::KeyModifier::MOD2;
    aKeys.push_back(&aAltDown);

    
    
    removeKeysIfExists(xScAccel, aKeys);
    xScAccel->store();

    switch (eType)
    {
        case ScOptionsUtil::KEY_DEFAULT:
            xScAccel->setKeyEvent(aDelete, OUString(".uno:ClearContents"));
            xScAccel->setKeyEvent(aBackspace, OUString(".uno:Delete"));
            xScAccel->setKeyEvent(aCtrlD, OUString(".uno:FillDown"));
            xScAccel->setKeyEvent(aAltDown, OUString(".uno:DataSelect"));
        break;
        case ScOptionsUtil::KEY_OOO_LEGACY:
            xScAccel->setKeyEvent(aDelete, OUString(".uno:Delete"));
            xScAccel->setKeyEvent(aBackspace, OUString(".uno:ClearContents"));
            xScAccel->setKeyEvent(aCtrlD, OUString(".uno:DataSelect"));
        break;
        default:
            ;
    }

    xScAccel->store();
}

void ScDocShell::UseSheetSaveEntries()
{
    if (pSheetSaveData)
    {
        pSheetSaveData->UseSaveEntries();   

        bool bHasEntries = false;
        SCTAB nTabCount = aDocument.GetTableCount();
        SCTAB nTab;
        for (nTab = 0; nTab < nTabCount; ++nTab)
            if (pSheetSaveData->HasStreamPos(nTab))
                bHasEntries = true;

        if (!bHasEntries)
        {
            
            

            for (nTab = 0; nTab < nTabCount; ++nTab)
                if (aDocument.IsStreamValid(nTab))
                    aDocument.SetStreamValid(nTab, false);
        }
    }
}



ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
        :
        rDocShell( rDS ),
        mpProtector(new ScRefreshTimerProtector(rDS.GetDocument()->GetRefreshTimerControlAddress()))
{
    ScDocument* pDoc = rDocShell.GetDocument();
    bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
    bIdleEnabled = pDoc->IsIdleEnabled();
    pDoc->SetAutoCalcShellDisabled( true );
    pDoc->EnableIdle(false);
}


ScDocShellModificator::~ScDocShellModificator()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
    if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
        rDocShell.SetDocumentModified();    
    pDoc->EnableIdle(bIdleEnabled);
}


void ScDocShellModificator::SetDocumentModified()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    pDoc->ClearFormulaContext();
    if ( !pDoc->IsImportingXML() )
    {
        
        bool bDisabled = pDoc->IsAutoCalcShellDisabled();
        pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
        rDocShell.SetDocumentModified();
        pDoc->SetAutoCalcShellDisabled( bDisabled );
    }
    else
    {
        
        
        pDoc->BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
    }
}

bool ScDocShell::IsChangeRecording() const
{
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    return pChangeTrack != NULL;
}


bool ScDocShell::HasChangeRecordProtection() const
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack)
        bRes = pChangeTrack->IsProtected();
    return bRes;
}


void ScDocShell::SetChangeRecording( bool bActivate )
{
    bool bOldChangeRecording = IsChangeRecording();

    if (bActivate)
    {
        aDocument.StartChangeTracking();
        ScChangeViewSettings aChangeViewSet;
        aChangeViewSet.SetShowChanges(true);
        aDocument.SetChangeViewSettings(aChangeViewSet);
    }
    else
    {
        aDocument.EndChangeTracking();
        PostPaintGridAll();
    }

    if (bOldChangeRecording != IsChangeRecording())
    {
        UpdateAcceptChangesDialog();
        
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->InvalidateAll(false);
    }
}


bool ScDocShell::SetProtectionPassword( const OUString &rNewPassword )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack)
    {
        bool bProtected = pChangeTrack->IsProtected();

        if (!rNewPassword.isEmpty())
        {
            
            SetChangeRecording( true );

            ::com::sun::star::uno::Sequence< sal_Int8 > aProtectionHash;
            SvPasswordHelper::GetHashPassword( aProtectionHash, rNewPassword );
            pChangeTrack->SetProtection( aProtectionHash );
        }
        else
        {
            pChangeTrack->SetProtection( ::com::sun::star::uno::Sequence< sal_Int8 >() );
        }
        bRes = true;

        if ( bProtected != pChangeTrack->IsProtected() )
        {
            UpdateAcceptChangesDialog();
            SetDocumentModified();
        }
    }

    return bRes;
}


bool ScDocShell::GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack && pChangeTrack->IsProtected())
    {
        rPasswordHash = pChangeTrack->GetProtection();
        bRes = true;
    }
    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
