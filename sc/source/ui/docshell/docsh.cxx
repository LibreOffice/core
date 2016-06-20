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
#include <svl/urihelper.hxx>
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
#include <refupdatecontext.hxx>

#include <config_telepathy.h>

#if ENABLE_TELEPATHY
#include "sccollaboration.hxx"
#endif

#include <memory>
#include <vector>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::XMultiServiceFactory;
using std::shared_ptr;
using ::std::vector;

//  Filter names (like in sclib.cxx)

static const sal_Char pFilterSc50[]     = "StarCalc 5.0";
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

SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell)

void ScDocShell::InitInterface_Impl()
{
}

//  GlobalName of the current version:
SFX_IMPL_OBJECTFACTORY( ScDocShell, SvGlobalName(SO3_SC_CLASSID), SfxObjectShellFlags::STD_NORMAL, "scalc" )


void ScDocShell::FillClass( SvGlobalName* pClassName,
                                        SotClipboardFormatId* pFormat,
                                        OUString* /* pAppName */,
                                        OUString* pFullTypeName,
                                        OUString* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        bool bTemplate /* = false */) const
{
    if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SotClipboardFormatId::STARCALC_60;
        *pFullTypeName  = OUString( ScResId( SCSTR_LONG_SCDOC_NAME ) );
        *pShortTypeName = OUString( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = bTemplate ? SotClipboardFormatId::STARCALC_8_TEMPLATE : SotClipboardFormatId::STARCALC_8;
        *pFullTypeName  = "calc8";
        *pShortTypeName = ScResId(SCSTR_SHORT_SCDOC_NAME).toString();
    }
    else
    {
        OSL_FAIL("Which version?");
    }
}

std::set<Color> ScDocShell::GetDocColors()
{
    return aDocument.GetDocColors();
}

void ScDocShell::DoEnterHandler()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        if (pViewSh->GetViewData().GetDocShell() == this)
            SC_MOD()->InputEnterHandler();
}

SCTAB ScDocShell::GetSaveTab()
{
    SCTAB nTab = 0;
    ScTabViewShell* pSh = GetBestViewShell();
    if (pSh)
    {
        const ScMarkData& rMark = pSh->GetViewData().GetMarkData();
        nTab = rMark.GetFirstSelected();
    }
    return nTab;
}

HiddenInformation ScDocShell::GetHiddenInformationState( HiddenInformation nStates )
{
    // get global state like HiddenInformation::DOCUMENTVERSIONS
    HiddenInformation nState = SfxObjectShell::GetHiddenInformationState( nStates );

    if ( nStates & HiddenInformation::RECORDEDCHANGES )
    {
        if ( aDocument.GetChangeTrack() && aDocument.GetChangeTrack()->GetFirst() )
          nState |= HiddenInformation::RECORDEDCHANGES;
    }
    if ( nStates & HiddenInformation::NOTES )
    {
        SCTAB nTableCount = aDocument.GetTableCount();
        bool bFound = false;
        for (SCTAB nTab = 0; nTab < nTableCount && !bFound; ++nTab)
        {
            if (aDocument.HasTabNotes(nTab)) //TODO:
                bFound = true;
        }

        if (bFound)
            nState |= HiddenInformation::NOTES;
    }

    return nState;
}

void ScDocShell::BeforeXMLLoading()
{
    aDocument.EnableIdle(false);

    // prevent unnecessary broadcasts and updates
    OSL_ENSURE(pModificator == nullptr, "The Modificator should not exist");
    pModificator = new ScDocShellModificator( *this );

    aDocument.SetImportingXML( true );
    aDocument.EnableExecuteLink( false );   // #i101304# to be safe, prevent nested loading from external references
    aDocument.EnableUndo( false );
    // prevent unnecessary broadcasts and "half way listeners"
    aDocument.SetInsertingFromOtherDoc( true );
}

void ScDocShell::AfterXMLLoading(bool bRet)
{
    if (GetCreateMode() != SfxObjectCreateMode::ORGANIZER)
    {
        UpdateLinks();
        // don't prevent establishing of listeners anymore
        aDocument.SetInsertingFromOtherDoc( false );
        if ( bRet )
        {
            ScChartListenerCollection* pChartListener = aDocument.GetChartListenerCollection();
            if (pChartListener)
                pChartListener->UpdateDirtyCharts();

            // #95582#; set the table names of linked tables to the new path
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

                        // remove the quotes on begin and end of the docname and restore the escaped quotes
                        const sal_Unicode* pNameBuffer = aName.getStr();
                        if ( *pNameBuffer == '\'' && // all docnames have to have a ' character on the first pos
                            ScGlobal::UnicodeStrChr( pNameBuffer, SC_COMPILER_FILE_TAB_SEP ) )
                        {
                            OUStringBuffer aDocURLBuffer;
                            bool bQuote = true; // Document name is always quoted
                            ++pNameBuffer;
                            while ( bQuote && *pNameBuffer )
                            {
                                if ( *pNameBuffer == '\'' && *(pNameBuffer-1) != '\\' )
                                    bQuote = false;
                                else if( !(*pNameBuffer == '\\' && *(pNameBuffer+1) == '\'') )
                                    aDocURLBuffer.append(*pNameBuffer); // If escaped quote: only quote in the name
                                ++pNameBuffer;
                            }

                            if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  // after the last quote of the docname should be the # char
                            {
                                sal_Int32 nIndex = nNameLength - nLinkTabNameLength;
                                INetURLObject aINetURLObject(aDocURLBuffer.makeStringAndClear());
                                if(aName.match( aLinkTabName, nIndex) &&
                                    (aName[nIndex - 1] == '#') && // before the table name should be the # char
                                    !aINetURLObject.HasError()) // the docname should be a valid URL
                                {
                                    aName = ScGlobal::GetDocTabName( aDocument.GetLinkDoc( i ), aDocument.GetLinkTab( i ) );
                                    aDocument.RenameTab(i, aName, true, true);
                                }
                                // else;  nothing has to happen, because it is a user given name
                            }
                            // else;  nothing has to happen, because it is a user given name
                        }
                        // else;  nothing has to happen, because it is a user given name
                    }
                    // else;  nothing has to happen, because it is a user given name
                }
            }

            // #i94570# DataPilot table names have to be unique, or the tables can't be accessed by API.
            // If no name (or an invalid name, skipped in ScXMLDataPilotTableContext::EndElement) was set, create a new name.
            ScDPCollection* pDPCollection = aDocument.GetDPCollection();
            if ( pDPCollection )
            {
                size_t nDPCount = pDPCollection->GetCount();
                for (size_t nDP=0; nDP<nDPCount; ++nDP)
                {
                    ScDPObject& rDPObj = (*pDPCollection)[nDP];
                    if (rDPObj.GetName().isEmpty())
                        rDPObj.SetName( pDPCollection->CreateNewName() );
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
        ScDocument::HardRecalcState eRecalcState = aDocument.GetHardRecalcState();
        // Temporarily set hard-recalc to prevent calling
        // ScFormulaCell::Notify() during destruction of the Modificator which
        // will set the cells dirty.
        if (eRecalcState == ScDocument::HARDRECALCSTATE_OFF)
            aDocument.SetHardRecalcState(ScDocument::HARDRECALCSTATE_TEMPORARY);
        delete pModificator;
        pModificator = nullptr;
        aDocument.SetHardRecalcState(eRecalcState);
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

    // Break the streamed range into the top range and the height limit.  A
    // height limit of 0 means unlimited i.e. the streamed data will go all
    // the way to the last row.

    ScRange aTopRange = r.maRange;
    aTopRange.aEnd.SetRow(aTopRange.aStart.Row());
    sal_Int32 nLimit = r.maRange.aEnd.Row() - r.maRange.aStart.Row() + 1;
    if (r.maRange.aEnd.Row() == MAXROW)
        // Unlimited range.
        nLimit = 0;

    sc::DataStream::MoveType eMove =
        r.meInsertPos == sc::ImportPostProcessData::DataStream::InsertTop ?
        sc::DataStream::MOVE_DOWN : sc::DataStream::RANGE_DOWN;

    sc::DataStream* pStrm = new sc::DataStream(&rShell, r.maURL, aTopRange, nLimit, eMove, 0);
    pStrm->SetRefreshOnEmptyLine(r.mbRefreshOnEmpty);
    sc::DocumentLinkManager& rMgr = rShell.GetDocument().GetDocLinkManager();
    rMgr.setDataStream(pStrm);
}

}

bool ScDocShell::LoadXML( SfxMedium* pLoadMedium, const css::uno::Reference< css::embed::XStorage >& xStor )
{
    LoadMediumGuard aLoadGuard(&aDocument);

    //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now

    // no Seek(0) here - always loading from storage, GetInStream must not be called

    BeforeXMLLoading();

    ScXMLImportWrapper aImport(*this, pLoadMedium, xStor);

    bool bRet = false;
    ErrCode nError = ERRCODE_NONE;
    aDocument.EnableAdjustHeight(false);
    if (GetCreateMode() == SfxObjectCreateMode::ORGANIZER)
        bRet = aImport.Import(ScXMLImportWrapper::STYLES, nError);
    else
        bRet = aImport.Import(ScXMLImportWrapper::ALL, nError);

    if ( nError )
        pLoadMedium->SetError( nError, OSL_LOG_PREFIX );

    processDataStream(*this, aImport.GetImportPostProcessData());

    //if the document was not generated by LibreOffice, do hard recalc in case some other document
    //generator saved cached formula results that differ from LibreOffice's calculated results or
    //did not use cached formula results.
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
            // Generator is not LibreOffice.  Ask if the user wants to perform
            // full re-calculation.
            ScopedVclPtrInstance<QueryBox> aBox(
                GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                ScGlobal::GetRscString(STR_QUERY_FORMULA_RECALC_ONLOAD_ODS));
            aBox->SetCheckBoxText(ScGlobal::GetRscString(STR_ALWAYS_PERFORM_SELECTED));

            bHardRecalc = aBox->Execute() == RET_YES;

            if (aBox->GetCheckBoxState())
            {
                // Always perform selected action in the future.
                std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
                officecfg::Office::Calc::Formula::Load::ODFRecalcMode::set(sal_Int32(0), batch);
                ScFormulaOptions aOpt = SC_MOD()->GetFormulaOptions();
                aOpt.SetODFRecalcOptions(bHardRecalc ? RECALC_ALWAYS : RECALC_NEVER);
                /* XXX  is this really supposed to set the ScModule options?
                 *      Not the ScDocShell options? */
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
        // still need to recalc volatile formula cells.
        aDocument.Broadcast(ScHint(SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS));
    }

    AfterXMLLoading(bRet);

    aDocument.EnableAdjustHeight(true);
    return bRet;
}

bool ScDocShell::SaveXML( SfxMedium* pSaveMedium, const css::uno::Reference< css::embed::XStorage >& xStor )
{
    aDocument.EnableIdle(false);

    ScXMLImportWrapper aImport(*this, pSaveMedium, xStor);
    bool bRet(false);
    if (GetCreateMode() != SfxObjectCreateMode::ORGANIZER)
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

bool ScDocShell::Load( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    //  only the latin script language is loaded
    //  -> initialize the others from options (before loading)
    InitOptions(true);

    GetUndoManager()->Clear();

    bool bRet = SfxObjectShell::Load( rMedium );
    if( bRet )
    {
        if (GetMedium())
        {
            const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(rMedium.GetItemSet(), SID_UPDATEDOCMODE, false);
            nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : css::document::UpdateDocMode::NO_UPDATE;
        }

        {
            //  prepare a valid document for XML filter
            //  (for ConvertFrom, InitNew is called before)
            aDocument.MakeTable(0);
            aDocument.GetStyleSheetPool()->CreateStandardStyles();
            aDocument.UpdStlShtPtrsFrmNms();

            bRet = LoadXML( &rMedium, nullptr );
        }
    }

    if (!bRet && !rMedium.GetError())
        rMedium.SetError( SVSTREAM_FILEFORMAT_ERROR, OSL_LOG_PREFIX );

    if (rMedium.GetError())
        SetError( rMedium.GetError(), OSL_LOG_PREFIX );

    InitItems();
    CalcOutputFactor();

    // invalidate eventually temporary table areas
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = false;
    FinishedLoading();
    return bRet;
}

void ScDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const ScTablesHint* pScHint = dynamic_cast< const ScTablesHint* >( &rHint );
    if (pScHint)
    {
        if (pScHint->GetId() == SC_TAB_INSERTED)
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = aDocument.GetVbaEventProcessor();
            if ( xVbaEvents.is() ) try
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= pScHint->GetTab1();
                xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_NEWSHEET, aArgs );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    if ( dynamic_cast<const SfxSimpleHint*>(&rHint) ) // Without parameter
    {
        switch ( static_cast<const SfxSimpleHint&>(rHint).GetId() )
        {
            case SFX_HINT_TITLECHANGED:
                aDocument.SetName( SfxShell::GetName() );
                //  RegisterNewTargetNames gibts nicht mehr
                SfxGetpApp()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED )); // Navigator
                break;
        }
    }
    else if ( dynamic_cast<const SfxStyleSheetHint*>(&rHint) ) // Template changed
        NotifyStyle( static_cast<const SfxStyleSheetHint&>(rHint) );
    else if ( dynamic_cast<const ScAutoStyleHint*>(&rHint) )
    {
        //! direct call for AutoStyles

        //  this is called synchronously from ScInterpreter::ScStyle,
        //  modifying the document must be asynchronous
        //  (handled by AddInitial)

        const ScAutoStyleHint& rStlHint = static_cast<const ScAutoStyleHint&>(rHint);
        ScRange aRange = rStlHint.GetRange();
        OUString aName1 = rStlHint.GetStyle1();
        OUString aName2 = rStlHint.GetStyle2();
        sal_uInt32 nTimeout = rStlHint.GetTimeout();

        if (!pAutoStyleList)
            pAutoStyleList = new ScAutoStyleList(this);
        pAutoStyleList->AddInitial( aRange, aName1, nTimeout, aName2 );
    }
    else if ( dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        sal_uLong nEventId = static_cast<const SfxEventHint*>(&rHint)->GetEventId();
        switch ( nEventId )
        {
            case SFX_EVENT_LOADFINISHED:
                {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    // the readonly documents should not be opened in shared mode
                    if ( HasSharedXMLFlagSet() && !SC_MOD()->IsInSharedDocLoading() && !IsReadOnly() )
                    {
                        if ( SwitchToShared( true, false ) )
                        {
                            ScViewData* pViewData = GetViewData();
                            ScTabView* pTabView = ( pViewData ? dynamic_cast< ScTabView* >( pViewData->GetView() ) : nullptr );
                            if ( pTabView )
                            {
                                pTabView->UpdateLayerLocks();
                            }
                        }
                        else
                        {
                            // switching to shared mode has failed, the document should be opened readonly
                            // TODO/LATER: And error message should be shown here probably
                            SetReadOnlyUI();
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
                            ScopedVclPtrInstance<WarningBox> aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                ScGlobal::GetRscString( STR_SHARED_DOC_WARNING ) );
                            aBox->SetDefaultCheckBoxText();
                            aBox->Execute();
                            bool bChecked = aBox->GetCheckBoxState();
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
                            "com.sun.star.sheet.SpreadsheetDocumentJob" );
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
                                    ScViewData* pViewData = GetViewData();
                                    SfxViewShell* pViewShell = ( pViewData ? pViewData->GetViewShell() : nullptr );
                                    SfxViewFrame* pViewFrame = ( pViewShell ? pViewShell->GetViewFrame() : nullptr );
                                    SfxFrame* pFrame = ( pViewFrame ? &pViewFrame->GetFrame() : nullptr );
                                    uno::Reference< frame::XController > xController = ( pFrame ? pFrame->GetController() : nullptr );
                                    uno::Reference< sheet::XSpreadsheetView > xSpreadsheetView( xController, uno::UNO_QUERY_THROW );
                                    uno::Sequence< beans::NamedValue > aArgsForJob { { "SpreadsheetView", uno::makeAny( xSpreadsheetView ) } };
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
                                // load shared file
                                xModel.set( LoadSharedDocument(), uno::UNO_QUERY_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
                                ScDocShell* pSharedDocShell = ( pDocObj ? dynamic_cast< ScDocShell* >( pDocObj->GetObjectShell() ) : nullptr );
                                if ( pSharedDocShell )
                                {
                                    bShared = pSharedDocShell->HasSharedXMLFlagSet();
                                }

                                // #i87870# check if shared status was disabled and enabled again
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
                                        xCloseable->close( true );

                                        OUString aUserName( ScGlobal::GetRscString( STR_UNKNOWN_USER ) );
                                        bool bNoLockAccess = false;
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            LockFileEntry aData = aLockFile.GetLockData();
                                            if ( !aData[LockFileComponent::OOOUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::OOOUSERNAME];
                                            }
                                            else if ( !aData[LockFileComponent::SYSUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::SYSUSERNAME];
                                            }
                                        }
                                        catch ( uno::Exception& )
                                        {
                                            bNoLockAccess = true;
                                        }

                                        if ( bNoLockAccess )
                                        {
                                            // TODO/LATER: in future an error regarding impossibility to open file for writing could be shown
                                            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                        }
                                        else
                                        {
                                            OUString aMessage( ScGlobal::GetRscString( STR_FILE_LOCKED_SAVE_LATER ) );
                                            aMessage = aMessage.replaceFirst( "%1", aUserName );

                                            ScopedVclPtrInstance< WarningBox > aBox( GetActiveDialogParent(), WinBits( WB_RETRY_CANCEL | WB_DEF_RETRY ), aMessage );
                                            if ( aBox->Execute() == RET_RETRY )
                                            {
                                                bRetry = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // merge changes from shared file into temp file
                                        bool bSaveToShared = false;
                                        if ( pSharedDocShell )
                                        {
                                            bSaveToShared = MergeSharedDocument( pSharedDocShell );
                                        }

                                        // close shared file
                                        xCloseable->close( true );

                                        // TODO: keep file lock on shared file

                                        // store to shared file
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
                                            // TODO/LATER: More entries from the MediaDescriptor might be interesting for the merge
                                            uno::Sequence< beans::PropertyValue > aValues(1);
                                            aValues[0].Name = "FilterName";
                                            aValues[0].Value <<= OUString( GetMedium()->GetFilter()->GetFilterName() );

                                            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_PASSWORD, false);
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
                                    xCloseable->close( true );

                                    if ( bEntriesNotAccessible )
                                    {
                                        // TODO/LATER: in future an error regarding impossibility to write to share control file could be shown
                                        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                    }
                                    else
                                    {
                                        ScopedVclPtrInstance<WarningBox> aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                            ScGlobal::GetRscString( STR_DOC_NOLONGERSHARED ) );
                                        aBox->Execute();

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
                                    xClose->close( true );
                                }
                                catch ( uno::Exception& )
                                {
                                }
                            }
                        }

                        if ( !bSuccess )
                            SetError( ERRCODE_IO_ABORT, OSL_LOG_PREFIX ); // this error code will produce no error message, but will break the further saving process
                    }
#endif

                    if (pSheetSaveData)
                        pSheetSaveData->SetInSupportedSave(true);
                }
                break;
            case SFX_EVENT_SAVEASDOC:
                {
                    if ( GetDocument().GetExternalRefManager()->containsUnsavedReferences() )
                    {
                        ScopedVclPtrInstance<WarningBox> aBox( GetActiveDialogParent(), WinBits( WB_YES_NO ),
                                ScGlobal::GetRscString( STR_UNSAVED_EXT_REF ) );

                        if( RET_NO == aBox->Execute())
                        {
                            SetError( ERRCODE_IO_ABORT, OSL_LOG_PREFIX ); // this error code will produce no error message, but will break the further saving process
                        }
                    }
                    SAL_FALLTHROUGH;
                }
            case SFX_EVENT_SAVETODOC:
                // #i108978# If no event is sent before saving, there will also be no "...DONE" event,
                // and SAVE/SAVEAS can't be distinguished from SAVETO. So stream copying is only enabled
                // if there is a SAVE/SAVEAS/SAVETO event first.
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(true);
                break;
            case SFX_EVENT_SAVEDOCDONE:
            case SFX_EVENT_SAVEASDOCDONE:
                {
                    // new positions are used after "save" and "save as", but not "save to"
                    UseSheetSaveEntries();      // use positions from saved file for next saving
                    SAL_FALLTHROUGH;
                }
            case SFX_EVENT_SAVETODOCDONE:
                // only reset the flag, don't use the new positions
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

// Load contents for organizer
bool ScDocShell::LoadFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    WaitObject aWait( GetActiveDialogParent() );

    bool bRet = false;

    if (GetMedium())
    {
        const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(rMedium.GetItemSet(), SID_UPDATEDOCMODE, false);
        nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : css::document::UpdateDocMode::NO_UPDATE;
    }

    //  until loading/saving only the styles in XML is implemented,
    //  load the whole file
    bRet = LoadXML( &rMedium, nullptr );
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

bool ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(&aDocument);

    bool bRet = false; // sal_False means user quit!
                           // On error: Set error at stream

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    GetUndoManager()->Clear();

    // Set optimal col width after import?
    bool bSetColWidths = false;
    bool bSetSimpleTextColWidths = false;
    ScColWidthParam aColWidthParam[MAXCOLCOUNT];
    ScRange aColWidthRange;
    // Set optimal row height after import?
    bool bSetRowHeights = false;

    vector<ScDocRowHeightUpdater::TabRanges> aRecalcRowRangesArray;

    //  All filters need the complete file in one piece (not asynchronously)
    //  So make sure that we transfer the whole file with CreateFileStream
    rMedium.GetPhysicalName();  //! Call CreateFileStream directly, if available

    const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(rMedium.GetItemSet(), SID_UPDATEDOCMODE, false);
    nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : css::document::UpdateDocMode::NO_UPDATE;

    const SfxFilter* pFilter = rMedium.GetFilter();
    if (pFilter)
    {
        OUString aFltName = pFilter->GetFilterName();

        bool bCalc3 = aFltName == "StarCalc 3.0";
        bool bCalc4 = aFltName == "StarCalc 4.0";
        if (!bCalc3 && !bCalc4)
            aDocument.SetInsertingFromOtherDoc( true );

        if (aFltName == pFilterXML)
            bRet = LoadXML( &rMedium, nullptr );
        else if (aFltName == "StarCalc 1.0")
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError = ScFormatFilter::Get().ScImportStarCalc10( *pStream, &aDocument );
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError, OSL_LOG_PREFIX);
                }
                else
                    bRet = true;
            }
        }
        else if (aFltName == pFilterLotus)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for lotus import (from API without options):
                //  IBM_437 encoding
                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
            }

            FltError eError = ScFormatFilter::Get().ScImportLotus123( rMedium, &aDocument,
                                                ScGlobal::GetCharsetValue(sItStr));
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OSL_LOG_PREFIX);

                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = true;
            }
            else
                bRet = true;
            bSetColWidths = true;
            bSetRowHeights = true;
        }
        else if ( aFltName == pFilterExcel4 || aFltName == pFilterExcel5 ||
                   aFltName == pFilterExcel95 || aFltName == pFilterExcel97 ||
                   aFltName == pFilterEx4Temp || aFltName == pFilterEx5Temp ||
                   aFltName == pFilterEx95Temp || aFltName == pFilterEx97Temp )
        {
            EXCIMPFORMAT eFormat = EIF_AUTO;
            if ( aFltName == pFilterExcel4 || aFltName == pFilterEx4Temp )
                eFormat = EIF_BIFF_LE4;
            else if ( aFltName == pFilterExcel5 || aFltName == pFilterExcel95 ||
                      aFltName == pFilterEx5Temp || aFltName == pFilterEx95Temp )
                eFormat = EIF_BIFF5;
            else if ( aFltName == pFilterExcel97 || aFltName == pFilterEx97Temp )
                eFormat = EIF_BIFF8;

            MakeDrawLayer(); //! In the filter
            CalcOutputFactor(); // prepare update of row height
            FltError eError = ScFormatFilter::Get().ScImportExcel( rMedium, &aDocument, eFormat );
            aDocument.UpdateFontCharSet();
            if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
                aDocument.UpdateChartListenerCollection(); //! For all imports?

            // all graphics objects must have names
            aDocument.EnsureGraphicNames();

            if (eError == SCWARN_IMPORT_RANGE_OVERFLOW)
            {
                if (!GetError())
                    SetError(eError, OSL_LOG_PREFIX);
                bRet = true;
            }
            else if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OSL_LOG_PREFIX);
            }
            else
                bRet = true;
        }
        else if (aFltName == "Gnumeric Spreadsheet")
        {
            ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
            if (!pOrcus)
                return false;

            bRet = pOrcus->importGnumeric(aDocument, rMedium);
        }
        else if (aFltName == pFilterAscii)
        {
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            ScAsciiOptions aOptions;
            bool bOptInit = false;

            if ( pSet && SfxItemState::SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                aOptions.ReadFromString( static_cast<const SfxStringItem*>(pItem)->GetValue() );
                bOptInit = true;
            }

            if ( !bOptInit )
            {
                //  default for ascii import (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

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
                    sc::SetFormulaDirtyContext aCxt;
                    aDocument.SetAllFormulasDirty(aCxt);

                    // The same resulting name has to be handled in
                    // ScExternalRefCache::initializeDoc() and related, hence
                    // pass 'true' for RenameTab()'s bExternalDocument for a
                    // composed name so ValidTabName() will not be checked,
                    // which could veto the rename in case it contained
                    // characters that Excel does not handle. If we wanted to
                    // change that then it needed to be handled in all
                    // corresponding places of the external references
                    // manager/cache. Likely then we'd also need a method to
                    // compose a name excluding such characters.
                    aDocument.RenameTab( 0, INetURLObject( rMedium.GetName()).GetBase(), true, true);

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
                    SetError(eError, OSL_LOG_PREFIX);
            }
            else if (!GetError() && (bOverflowRow || bOverflowCol || bOverflowCell))
            {
                // precedence: row, column, cell
                FltError nWarn = (bOverflowRow ? SCWARN_IMPORT_ROW_OVERFLOW :
                        (bOverflowCol ? SCWARN_IMPORT_COLUMN_OVERFLOW :
                         SCWARN_IMPORT_CELL_OVERFLOW));
                SetError( nWarn, OSL_LOG_PREFIX);
            }
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
        }
        else if (aFltName == pFilterDBase)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for dBase import (from API without options):
                //  IBM_850 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
            }

            ScDocRowHeightUpdater::TabRanges aRecalcRanges(0);
            sal_uLong eError = DBaseImport( rMedium.GetPhysicalName(),
                    ScGlobal::GetCharsetValue(sItStr), aColWidthParam, *aRecalcRanges.mpRanges );
            aRecalcRowRangesArray.push_back(aRecalcRanges);

            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, OSL_LOG_PREFIX);
                bRet = ( eError == SCWARN_IMPORT_RANGE_OVERFLOW );
            }
            else
                bRet = true;

            aColWidthRange.aStart.SetRow( 1 );  // Except for the column header
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
        }
        else if (aFltName == pFilterDif)
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError;
                OUString sItStr;
                SfxItemSet*  pSet = rMedium.GetItemSet();
                const SfxPoolItem* pItem;
                if ( pSet && SfxItemState::SET ==
                     pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
                {
                    sItStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                }

                if (sItStr.isEmpty())
                {
                    //  default for DIF import (from API without options):
                    //  ISO8859-1/MS_1252 encoding

                    sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
                }

                eError = ScFormatFilter::Get().ScImportDif( *pStream, &aDocument, ScAddress(0,0,0),
                                    ScGlobal::GetCharsetValue(sItStr));
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError, OSL_LOG_PREFIX);

                    if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                        bRet = true;
                }
                else
                    bRet = true;
            }
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
            bSetRowHeights = true;
        }
        else if (aFltName == pFilterSylk)
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                ScImportExport aImpEx( &aDocument );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL(), SotClipboardFormatId::SYLK );
                    eError = bRet ? eERR_OK : SCERR_IMPORT_UNKNOWN;
                    aDocument.StartAllListeners();
                    sc::SetFormulaDirtyContext aCxt;
                    aDocument.SetAllFormulasDirty(aCxt);
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OSL_LOG_PREFIX);
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
            bSetRowHeights = true;
        }
        else if (aFltName == pFilterQPro6)
        {
            FltError eError = ScFormatFilter::Get().ScImportQuattroPro( rMedium, &aDocument);
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError( eError, OSL_LOG_PREFIX );
                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = true;
            }
            else
                bRet = true;
            // TODO: Filter should set column widths. Not doing it here, it may
            // result in very narrow or wide columns, depending on content.
            // Setting row heights makes cells with font size attribution or
            // wrapping enabled look nicer..
            bSetRowHeights = true;
        }
        else if (aFltName == pFilterRtf)
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
                            SetError(eError, OSL_LOG_PREFIX);

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = true;
                    }
                    else
                        bRet = true;
                    aDocument.StartAllListeners();
                    sc::SetFormulaDirtyContext aCxt;
                    aDocument.SetAllFormulasDirty(aCxt);
                    bSetColWidths = true;
                    bSetRowHeights = true;
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OSL_LOG_PREFIX);
        }
        else if (aFltName == pFilterHtml || aFltName == pFilterHtmlWebQ)
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            bool bWebQuery = aFltName == pFilterHtmlWebQ;
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    LanguageType eLang = LANGUAGE_SYSTEM;
                    bool bDateConvert = false;
                    SfxItemSet*  pSet = rMedium.GetItemSet();
                    const SfxPoolItem* pItem;
                    if ( pSet && SfxItemState::SET ==
                         pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
                    {
                        OUString aFilterOption = (static_cast<const SfxStringItem*>(pItem))->GetValue();
                        lcl_parseHtmlFilterOption(aFilterOption, eLang, bDateConvert);
                    }

                    pInStream->Seek( 0 );
                    ScRange aRange;
                    // HTML does its own ColWidth/RowHeight
                    CalcOutputFactor();
                    SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eLang);
                    eError = ScFormatFilter::Get().ScImportHTML( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange,
                                            GetOutputFactor(), !bWebQuery, &aNumFormatter, bDateConvert );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError, OSL_LOG_PREFIX);

                        if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                            bRet = true;
                    }
                    else
                        bRet = true;
                    aDocument.StartAllListeners();

                    sc::SetFormulaDirtyContext aCxt;
                    aDocument.SetAllFormulasDirty(aCxt);
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if ( eError != eERR_OK && !GetError() )
                SetError(eError, OSL_LOG_PREFIX);
        }
        else
        {
            if (!GetError())
                SetError(SCERR_IMPORT_NI, OSL_LOG_PREFIX);
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
    {   // Adjust column width/row height; base 100% zoom
        Fraction aZoom( 1, 1 );
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom / GetOutputFactor(); // Factor is printer display ratio
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        ScopedVclPtrInstance< VirtualDevice > pVirtDev;
        //  all sheets (for Excel import)
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

            // Order is important: First width, then height
            if ( bSetColWidths )
            {
                for ( SCCOL nCol=0; nCol <= nEndCol; nCol++ )
                {
                    if (!bSetSimpleTextColWidths)
                        aColWidthParam[nCol].mbSimpleText = false;

                    sal_uInt16 nWidth = aDocument.GetOptimalColWidth(
                        nCol, nTab, pVirtDev, nPPTX, nPPTY, aZoom, aZoom, false, &aMark,
                        &aColWidthParam[nCol] );
                    aDocument.SetColWidth( nCol, nTab,
                        nWidth + (sal_uInt16)ScGlobal::nLastColWidthExtra );
                }
            }
        }

        if (bSetRowHeights)
        {
            // Update all rows in all tables.
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(aDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), nullptr);
            aUpdater.update();
        }
        else if (!aRecalcRowRangesArray.empty())
        {
            // Update only specified row ranges for better performance.
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(aDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), &aRecalcRowRangesArray);
            aUpdater.update();
        }
    }
    FinishedLoading();

    // invalidate eventually temporary table areas
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

        FinishedLoading();
        return true;
    }

    return false;
}

ScDocShell::PrepareSaveGuard::PrepareSaveGuard( ScDocShell& rDocShell )
    : mrDocShell( rDocShell)
{
    // DoEnterHandler not here (because of AutoSave), is in ExecuteSave.

    ScChartListenerCollection* pCharts = mrDocShell.aDocument.GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                           // Charts to be updated.
    mrDocShell.aDocument.StopTemporaryChartLock();
    if (mrDocShell.pAutoStyleList)
        mrDocShell.pAutoStyleList->ExecuteAllNow();             // Execute template timeouts now.
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            pRefMgr->setAllCacheTableReferencedStati( false);
            mrDocShell.aDocument.MarkUsedExternalReferences();  // Mark tables of external references to be written.
        }
    }
    if (mrDocShell.GetCreateMode()== SfxObjectCreateMode::STANDARD)
        mrDocShell.SfxObjectShell::SetVisArea( Rectangle() );   // "Normally" worked on => no VisArea.
}

ScDocShell::PrepareSaveGuard::~PrepareSaveGuard()
{
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            // Prevent accidental data loss due to lack of knowledge.
            pRefMgr->setAllCacheTableReferencedStati( true);
        }
    }
}

bool ScDocShell::Save()
{
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        vcl::Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            vcl::Window* pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                pSysWin->SetAccessibleName(OUString());
            }
        }
    }
    //  wait cursor is handled with progress bar
    bool bRet = SfxObjectShell::Save();
    if( bRet )
        bRet = SaveXML( GetMedium(), nullptr );
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

bool ScDocShell::SaveAs( SfxMedium& rMedium )
{
    OUString aCurPath; // empty for new document that hasn't been saved.
    const SfxMedium* pCurMedium = GetMedium();
    if (pCurMedium)
    {
        aCurPath = pCurMedium->GetName();
        popFileName(aCurPath);
    }

    if (!aCurPath.isEmpty())
    {
        // current document has a path -> not a brand-new document.
        OUString aNewPath = rMedium.GetName();
        popFileName(aNewPath);
        OUString aRel = URIHelper::simpleNormalizedMakeRelative(aCurPath, aNewPath);
        if (!aRel.isEmpty())
        {
            // Directory path will change before and after the save.
            aDocument.InvalidateStreamOnSave();
        }
    }

    ScTabViewShell* pViewShell = GetBestViewShell();
    bool bNeedsRehash = ScPassHashHelper::needsPassHashRegen(aDocument, PASSHASH_SHA1);
    if (bNeedsRehash)
        // legacy xls hash double-hashed by SHA1 is also supported.
        bNeedsRehash = ScPassHashHelper::needsPassHashRegen(aDocument, PASSHASH_XL, PASSHASH_SHA1);

    if (pViewShell && bNeedsRehash)
    {
        if (!pViewShell->ExecuteRetypePassDlg(PASSHASH_SHA1))
            // password re-type cancelled.  Don't save the document.
            return false;
    }

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    OUString aFltName = rMedium.GetFilter()->GetFilterName();
    bool bChartExport = aFltName.indexOf("chart8") != -1;

    //  wait cursor is handled with progress bar
    bool bRet = false;
    if(!bChartExport)
    {
        bRet = SfxObjectShell::SaveAs( rMedium );
        if (bRet)
            bRet = SaveXML( &rMedium, nullptr );
    }
    else
    {
        bRet = SaveCurrentChart( rMedium );
    }

    return bRet;
}

bool ScDocShell::IsInformationLost()
{
    //FIXME: If we have time build a correct own way how to handle this
    return SfxObjectShell::IsInformationLost();
}

namespace {

// Xcl-like column width measured in characters of standard font.
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
        SCTAB nTab, SCCOL nCol, bool bValue, SvxCellHorJustify eHorJust )
{
    OUString aString = rStr;
    sal_Int32 nLen = lcl_ScDocShell_GetColWidthInChars(
            rDoc.GetColWidth( nCol, nTab ) );
    //If the text won't fit in the column
    if ( nLen < aString.getLength() )
    {
        OUStringBuffer aReplacement;
        if (bValue)
            aReplacement.append("###");
        else
            aReplacement.append(aString);
        //truncate to the number of characters that should fit, even in the
        //bValue case nLen might be < len ###
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
    // #i116636# quotes are needed if text delimiter (quote), field delimiter,
    // or LF or CR is in the cell text.
    sal_Int32 nPos = rStr.indexOf(rTextSep);
    rNeedQuotes = rAsciiOpt.bQuoteAllText || (nPos >= 0) ||
        (rStr.indexOf(rFieldSep) >= 0) ||
        (rStr.indexOf('\n') >= 0) ||
        (rStr.indexOf('\r') >= 0);
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
    SvStreamEndian nOldNumberFormatInt = rStream.GetEndian();
    OString aStrDelimEncoded;    // only used if not Unicode
    OUString aStrDelimDecoded;     // only used if context encoding
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
    while ( ( pCell = aIter.GetNext( nCol, nRow ) ) != nullptr )
    {
        bool bProgress = false;     // only upon line change
        if ( nNextRow < nRow )
        {   // empty rows or/and empty columns up to end of row
            bProgress = true;
            for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
            {   // remaining columns of last row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            endlub( rStream );
            nNextRow++;
            for ( nEmptyRow = nNextRow; nEmptyRow < nRow; nEmptyRow++ )
            {   // completely empty rows
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
            {   // empty columns at beginning of row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            nNextRow = nRow;
        }
        else if ( nNextCol < nCol )
        {   // empty columns in same row
            for ( nEmptyCol = nNextCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // columns in between
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
                static_cast<const ScProtectionAttr*>( aDocument.GetAttr(
                                                            nCol, nRow, nTab, ATTR_PROTECTION ));
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;  // hide
        }
        bool bString;
        switch ( eType )
        {
            case CELLTYPE_NONE:
                aString.clear();
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
                            ScCellFormat::GetString(*pCell, nFormat, aString, &pDummy, rFormatter, &aDocument);
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
                            ScCellFormat::GetString(*pCell, nFormat, aString, &pDummy, rFormatter, &aDocument);
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
                    ScCellFormat::GetString(*pCell, nFormat, aString, &pDummy, rFormatter, &aDocument);
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
                    aString = rEngine.GetText();  // including LF
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
                        ScCellFormat::GetString(*pCell, nFormat, aString, &pDummy, rFormatter, &aDocument);
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
                aString.clear();
                bString = false;
        }

        if ( bFixedWidth )
        {
            SvxCellHorJustify eHorJust = (SvxCellHorJustify)
                static_cast<const SvxHorJustifyItem*>( aDocument.GetAttr( nCol, nRow,
                nTab, ATTR_HOR_JUSTIFY ))->GetValue();
            lcl_ScDocShell_GetFixedWidthString( aString, aDocument, nTab, nCol,
                    !bString, eHorJust );
            rStream.WriteUnicodeOrByteText( aString );
        }
        else
        {
            OUString aUniString = aString;// TODO: remove that later
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
                if ( cStrDelim != 0 ) //@ BugId 55355
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
                        // This is nasty. The Unicode to byte encoding
                        // may convert typographical quotation marks to ASCII
                        // quotation marks, which may interfer with the delimiter,
                        // so we have to escape delimiters after the string has
                        // been encoded. Since this may happen also with UTF-8
                        // encoded typographical quotation marks if such was
                        // specified as a delimiter we have to check for the full
                        // encoded delimiter string, not just one character.
                        // Now for RTL_TEXTENCODING_ISO_2022_... and similar brain
                        // dead encodings where one code point (and especially a
                        // low ASCII value) may represent different characters, we
                        // have to convert forth and back and forth again. Same for
                        // UTF-7 since it is a context sensitive encoding too.

                        if ( bContextOrNotAsciiEncoding )
                        {
                            // to byte encoding
                            OString aStrEnc = OUStringToOString(aUniString, eCharSet);
                            // back to Unicode
                            OUString aStrDec = OStringToOUString(aStrEnc, eCharSet);

                            // search on re-decoded string
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrDec, rAsciiOpt, aStrDelimDecoded, aDelimDecoded, bNeedQuotes);

                            escapeTextSep<OUString, OUStringBuffer>(
                                nPos, aStrDelimDecoded, aStrDec);

                            // write byte re-encoded
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                            rStream.WriteUnicodeOrByteText( aStrDec, eCharSet );
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        }
                        else
                        {
                            OString aStrEnc = OUStringToOString(aUniString, eCharSet);

                            // search on encoded string
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrEnc, rAsciiOpt, aStrDelimEncoded, aDelimEncoded, bNeedQuotes);

                            escapeTextSep<OString, OStringBuffer>(
                                nPos, aStrDelimEncoded, aStrEnc);

                            // write byte encoded
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
            if(cDelim!=0) //@ BugId 55355
                rStream.WriteUniOrByteChar( cDelim );
        }
        else
            endlub( rStream );

        if ( bProgress )
            aProgress.SetStateOnPercent( nRow );
    }

    // write out empty if requested
    if ( nNextRow <= nEndRow )
    {
        for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {   // remaining empty columns of last row
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
    {   // entire empty rows
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
    rStream.SetEndian( nOldNumberFormatInt );
}

bool ScDocShell::ConvertTo( SfxMedium &rMed )
{
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    //  #i6500# don't call DoEnterHandler here (doesn't work with AutoSave),
    //  it's already in ExecuteSave (as for Save and SaveAs)

    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow(); // Execute template timeouts now
    if (GetCreateMode()== SfxObjectCreateMode::STANDARD)
        SfxObjectShell::SetVisArea( Rectangle() ); // Edited normally -> no VisArea

    OSL_ENSURE( rMed.GetFilter(), "Filter == 0" );

    bool bRet = false;
    OUString aFltName = rMed.GetFilter()->GetFilterName();

    if (aFltName == pFilterXML)
    {
        //TODO/LATER: this shouldn't happen!
        OSL_FAIL("XML filter in ConvertFrom?!");
        bRet = SaveXML( &rMed, nullptr );
    }
    else if (aFltName == pFilterExcel5 || aFltName == pFilterExcel95 ||
             aFltName == pFilterExcel97 || aFltName == pFilterEx5Temp ||
             aFltName == pFilterEx95Temp || aFltName == pFilterEx97Temp)
    {
        WaitObject aWait( GetActiveDialogParent() );

        bool bDoSave = true;
        if( ScTabViewShell* pViewShell = GetBestViewShell() )
        {
            ScExtDocOptions* pExtDocOpt = aDocument.GetExtDocOptions();
            if( !pExtDocOpt )
                aDocument.SetExtDocOptions( pExtDocOpt = new ScExtDocOptions );
            pViewShell->GetViewData().WriteExtOptions( *pExtDocOpt );

            /*  #i104990# If the imported document contains a medium
                password, determine if we can save it, otherwise ask the users
                whether they want to save without it. */
            if( (rMed.GetFilter()->GetFilterFlags() & SfxFilterFlags::ENCRYPTION) == SfxFilterFlags::NONE )
            {
                SfxItemSet* pItemSet = rMed.GetItemSet();
                const SfxPoolItem* pItem = nullptr;
                if( pItemSet && pItemSet->GetItemState( SID_PASSWORD, true, &pItem ) == SfxItemState::SET )
                {
                    bDoSave = ScWarnPassword::WarningOnPassword( rMed );
                    // #i42858# remove password from medium (warn only one time)
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
            if( aFltName == pFilterExcel97 || aFltName == pFilterEx97Temp )
                eFormat = ExpBiff8;
            FltError eError = ScFormatFilter::Get().ScExportExcel5( rMed, &aDocument, eFormat, RTL_TEXTENCODING_MS_1252 );

            if( eError && !GetError() )
                SetError( eError, OSL_LOG_PREFIX );

            // don't return false for warnings
            bRet = ((eError & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK) || (eError == eERR_OK);
        }
        else
        {
            // export aborted, i.e. "Save without password" warning
            SetError( ERRCODE_ABORT, OSL_LOG_PREFIX );
        }
    }
    else if (aFltName == pFilterAscii)
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
            }

            if ( sItStr.isEmpty() )
            {
                //  default for ascii export (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

                ScImportOptions aDefOptions( ',', '"', RTL_TEXTENCODING_MS_1252 );
                sItStr = aDefOptions.BuildString();
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScImportOptions aOptions( sItStr );
            AsciiSave( *pStream, aOptions );
            bRet = true;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, OSL_LOG_PREFIX);
        }
    }
    else if (aFltName == pFilterDBase)
    {
        OUString sCharSet;
        SfxItemSet* pSet = rMed.GetItemSet();
        const SfxPoolItem* pItem;
        if ( pSet && SfxItemState::SET ==
             pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
        {
            sCharSet = static_cast<const SfxStringItem*>(pItem)->GetValue();
        }

        if (sCharSet.isEmpty())
        {
            //  default for dBase export (from API without options):
            //  IBM_850 encoding

            sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
        }

        WaitObject aWait( GetActiveDialogParent() );
        // FIXME:  Hack so that the Sba opened TempFile can be overwritten
        rMed.CloseOutStream();
        bool bHasMemo = false;

        sal_uLong eError = DBaseExport(
            rMed.GetPhysicalName(), ScGlobal::GetCharsetValue(sCharSet), bHasMemo);

        if ( eError != eERR_OK && (eError & ERRCODE_WARNING_MASK) )
        {
            eError = eERR_OK;
        }

        INetURLObject aTmpFile( rMed.GetPhysicalName(), INetProtocol::File );
        if ( bHasMemo )
            aTmpFile.setExtension("dbt");
        if ( eError != eERR_OK )
        {
            if (!GetError())
                SetError(eError, OSL_LOG_PREFIX);
            if ( bHasMemo && IsDocument( aTmpFile ) )
                KillFile( aTmpFile );
        }
        else
        {
            bRet = true;
            if ( bHasMemo )
            {
                const SfxStringItem* pNameItem =
                    static_cast<const SfxStringItem*>( rMed.GetItemSet()->GetItem( SID_FILE_NAME ) );
                INetURLObject aDbtFile( pNameItem->GetValue(), INetProtocol::File );
                aDbtFile.setExtension("dbt");
                if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
                    bRet = false;
                if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
                    bRet = false;
                if ( !bRet )
                {
                    KillFile( aTmpFile );
                    if ( !GetError() )
                        SetError( SCERR_EXPORT_DATA, OSL_LOG_PREFIX );
                }
            }
        }
    }
    else if (aFltName == pFilterDif)
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            OUString sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
            {
                sItStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for DIF export (from API without options):
                //  ISO8859-1/MS_1252 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScFormatFilter::Get().ScExportDif( *pStream, &aDocument, ScAddress(0,0,0),
                ScGlobal::GetCharsetValue(sItStr) );
            bRet = true;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, OSL_LOG_PREFIX);
        }
    }
    else if (aFltName == pFilterSylk)
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
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SotClipboardFormatId::SYLK );
        }
    }
    else if (aFltName == pFilterHtml)
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            SfxItemSet* pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            OUString sFilterOptions;

            if (pSet->GetItemState(SID_FILE_FILTEROPTIONS, true, &pItem) == SfxItemState::SET)
                sFilterOptions = static_cast<const SfxStringItem*>(pItem)->GetValue();

            WaitObject aWait(GetActiveDialogParent());
            ScImportExport aImExport(&aDocument);
            aImExport.SetStreamPath(rMed.GetName());
            aImExport.SetFilterOptions(sFilterOptions);
            bRet = aImExport.ExportStream(*pStream, rMed.GetBaseURL(true), SotClipboardFormatId::HTML);
            if (bRet && !aImExport.GetNonConvertibleChars().isEmpty())
            {
                SetError(*new StringErrorInfo(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_INFO), OSL_LOG_PREFIX);
            }
        }
    }
    else
    {
        if (GetError())
            SetError(SCERR_IMPORT_NI, OSL_LOG_PREFIX);
    }
    return bRet;
}

bool ScDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor )
{
    return SfxObjectShell::SaveCompleted( xStor );
}

bool ScDocShell::DoSaveCompleted( SfxMedium * pNewStor )
{
    bool bRet = SfxObjectShell::DoSaveCompleted( pNewStor );

    //  SC_HINT_DOC_SAVED for change ReadOnly -> Read/Write
    Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
    return bRet;
}

bool ScDocShell::QuerySlotExecutable( sal_uInt16 nSlotId )
{
    // #i112634# ask VBA event handlers whether to save or print the document

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

    bool bSlotExecutable = true;
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

bool ScDocShell::PrepareClose( bool bUI )
{
    if(SC_MOD()->GetCurRefDlgId()>0)
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
            if(pViewSh!=nullptr)
            {
                vcl::Window *pWin=pViewSh->GetWindow();
                if(pWin!=nullptr) pWin->GrabFocus();
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

    // start 'Workbook_BeforeClose' VBA event handler for possible veto
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
            // if event processor throws VetoException, macro has vetoed close
            return false;
        }
        catch( uno::Exception& )
        {
        }
    }
    // end handler code

    bool bRet = SfxObjectShell::PrepareClose( bUI );
    if (bRet) // true == close
        aDocument.EnableIdle(false); // Do not mess around with it anymore!

    return bRet;
}

void ScDocShell::PrepareReload()
{
    SfxObjectShell::PrepareReload(); // FIXME: Doesn't do a thing?

    //  The Disconnect of DDE Links can trigger a Reschedule.
    //  If the DDE Links are not deleted before the Document dtor,
    //  the DDE Link Update for this Document can be triggered from this Reschedule on Reload.
    //  This causes a hang.
    //
    //  Thus: Disconnect the DDE Links of the old Document before Reload
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
    //  sal_True for those filters that keep the default table name
    //  (which is language specific)

    return rFilter == pFilterAscii
        || rFilter == pFilterLotus
        || rFilter == pFilterExcel4
        || rFilter == pFilterEx4Temp
        || rFilter == pFilterDBase
        || rFilter == pFilterDif
        || rFilter == pFilterSylk
        || rFilter == pFilterHtml
        || rFilter == pFilterRtf;
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
    nCanUpdate (css::document::UpdateDocMode::ACCORDING_TO_CONFIG),
    pOldAutoDBRange ( nullptr ),
    pAutoStyleList  ( nullptr ),
    pPaintLockData  ( nullptr ),
    pSolverSaveData ( nullptr ),
    pSheetSaveData  ( nullptr ),
    pModificator    ( nullptr )
#if ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = rShell.bIsInplace;

    pDocFunc = CreateDocFunc();

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);

    GetPageOnFromPageStyleSet( nullptr, 0, bHeaderOn, bFooterOn );
    SetHelpId( HID_SCSHELL_DOCSH );

    // InitItems and CalcOutputFactor are called now in Load/ConvertFrom/InitNew
}

ScDocShell::ScDocShell( const SfxModelFlags i_nSfxCreationFlags ) :
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
    nCanUpdate (css::document::UpdateDocMode::ACCORDING_TO_CONFIG),
    pOldAutoDBRange ( nullptr ),
    pAutoStyleList  ( nullptr ),
    pPaintLockData  ( nullptr ),
    pSolverSaveData ( nullptr ),
    pSheetSaveData  ( nullptr ),
    pModificator    ( nullptr )
#if ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = (GetCreateMode() == SfxObjectCreateMode::EMBEDDED);
    //  Will be reset if not in place

    pDocFunc = CreateDocFunc();

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);
    SetHelpId( HID_SCSHELL_DOCSH );

    aDocument.GetDBCollection()->SetRefreshHandler(
        LINK( this, ScDocShell, RefreshDBDataHdl ) );

    // InitItems and CalcOutputFactor are called now in Load/ConvertFrom/InitNew
}

ScDocShell::~ScDocShell()
{
    ResetDrawObjectShell(); // If the Drawing Layer still tries to access it, access it

    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        EndListening(*pStlPool);
    EndListening(*this);

    delete pAutoStyleList;

    SfxApplication *pSfxApp = SfxGetpApp();
    if ( pSfxApp->GetDdeService() ) // Delete DDE for Document
        pSfxApp->RemoveDdeTopic( this );

    delete pDocFunc;
    delete aDocument.mpUndoManager;
    aDocument.mpUndoManager = nullptr;
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

void ScDocShell::SetModified( bool bModified )
{
    if ( SfxObjectShell::IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
    }
}

void ScDocShell::SetDocumentModified( bool bIsModified /* = true */ )
{
    //  BroadcastUno must also happen right away with pPaintLockData
    //  FIXME: Also for SetDrawModified, if Drawing is connected
    //  FIXME: Then own Hint?

    if ( pPaintLockData && bIsModified )
    {
        // #i115009# broadcast BCA_BRDCST_ALWAYS, so a component can read recalculated results
        // of RecalcModeAlways formulas (like OFFSET) after modifying cells
        aDocument.Broadcast(ScHint(SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS));
        aDocument.InvalidateTableArea();    // #i105279# needed here
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        pPaintLockData->SetModified(); // Later on ...
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
            aDocument.RefreshDirtyTableColumnNames();
            PostDataChanged();

            //  Detective AutoUpdate:
            //  Update if formulas were modified (DetectiveDirty) or the list contains
            //  "Trace Error" entries (Trace Error can look completely different
            //  after changes to non-formula cells).

            ScDetOpList* pList = aDocument.GetDetOpList();
            if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
                 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
            {
                GetDocFunc().DetectiveRefresh(true);    // sal_True = caused by automatic update
            }
            aDocument.SetDetectiveDirty(false);         // always reset, also if not refreshed
        }

        // notify UNO objects after BCA_BRDCST_ALWAYS etc.
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
    bool bUpdate = bIsModified != IsModified();

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
            // #i105960# Undo etc used to be volatile.
            // They always have to be invalidated, including drawing layer or row height changes
            // (but not while pPaintLockData is set).
            pBindings->Invalidate( SID_UNDO );
            pBindings->Invalidate( SID_REDO );
            pBindings->Invalidate( SID_REPEAT );
        }

        if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
        {
            aDocument.UpdateChartListenerCollection();
            SfxGetpApp()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));    // Navigator
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

VclPtr<SfxDocumentInfoDialog> ScDocShell::CreateDocumentInfoDialog(
                                         vcl::Window *pParent, const SfxItemSet &rSet )
{
    VclPtr<SfxDocumentInfoDialog> pDlg   = VclPtr<SfxDocumentInfoDialog>::Create( pParent, rSet );
    ScDocShell*            pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );

    // Only for statistics, if this Doc is shown; not from the Doc Manager
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
            nullptr);
    }
    return pDlg;
}

vcl::Window* ScDocShell::GetActiveDialogParent()
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

    // Grab the Calc configuration.
    Reference<XUIConfigurationManager> xConfigMgr =
        xModuleCfgSupplier->getUIConfigurationManager(
            "com.sun.star.sheet.SpreadsheetDocument");

    if (!xConfigMgr.is())
        return;

    // shortcut manager
    Reference<XAcceleratorConfiguration> xScAccel = xConfigMgr->getShortCutManager();

    if (!xScAccel.is())
        return;

    vector<const awt::KeyEvent*> aKeys;
    aKeys.reserve(4);

    // Backspace key
    awt::KeyEvent aBackspace;
    aBackspace.KeyCode = awt::Key::BACKSPACE;
    aBackspace.Modifiers = 0;
    aKeys.push_back(&aBackspace);

    // Delete key
    awt::KeyEvent aDelete;
    aDelete.KeyCode = awt::Key::DELETE;
    aDelete.Modifiers = 0;
    aKeys.push_back(&aDelete);

    // Ctrl-D
    awt::KeyEvent aCtrlD;
    aCtrlD.KeyCode = awt::Key::D;
    aCtrlD.Modifiers = awt::KeyModifier::MOD1;
    aKeys.push_back(&aCtrlD);

    // Alt-Down
    awt::KeyEvent aAltDown;
    aAltDown.KeyCode = awt::Key::DOWN;
    aAltDown.Modifiers = awt::KeyModifier::MOD2;
    aKeys.push_back(&aAltDown);

    // Remove all involved keys first, because swapping commands don't work
    // well without doing this.
    removeKeysIfExists(xScAccel, aKeys);
    xScAccel->store();

    switch (eType)
    {
        case ScOptionsUtil::KEY_DEFAULT:
            xScAccel->setKeyEvent(aDelete, ".uno:ClearContents");
            xScAccel->setKeyEvent(aBackspace, ".uno:Delete");
            xScAccel->setKeyEvent(aCtrlD, ".uno:FillDown");
            xScAccel->setKeyEvent(aAltDown, ".uno:DataSelect");
        break;
        case ScOptionsUtil::KEY_OOO_LEGACY:
            xScAccel->setKeyEvent(aDelete, ".uno:Delete");
            xScAccel->setKeyEvent(aBackspace, ".uno:ClearContents");
            xScAccel->setKeyEvent(aCtrlD, ".uno:DataSelect");
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
        pSheetSaveData->UseSaveEntries();   // use positions from saved file for next saving

        bool bHasEntries = false;
        SCTAB nTabCount = aDocument.GetTableCount();
        SCTAB nTab;
        for (nTab = 0; nTab < nTabCount; ++nTab)
            if (pSheetSaveData->HasStreamPos(nTab))
                bHasEntries = true;

        if (!bHasEntries)
        {
            // if no positions were set (for example, export to other format),
            // reset all "valid" flags

            for (nTab = 0; nTab < nTabCount; ++nTab)
                if (aDocument.IsStreamValid(nTab))
                    aDocument.SetStreamValid(nTab, false);
        }
    }
}

// --- ScDocShellModificator ------------------------------------------

ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
        :
        rDocShell( rDS ),
        mpProtector(new ScRefreshTimerProtector(rDS.GetDocument().GetRefreshTimerControlAddress()))
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bAutoCalcShellDisabled = rDoc.IsAutoCalcShellDisabled();
    bIdleEnabled = rDoc.IsIdleEnabled();
    rDoc.SetAutoCalcShellDisabled( true );
    rDoc.EnableIdle(false);
}

ScDocShellModificator::~ScDocShellModificator()
{
    ScDocument& rDoc = rDocShell.GetDocument();
    rDoc.SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
    if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
        rDocShell.SetDocumentModified();    // last one shuts off the lights
    rDoc.EnableIdle(bIdleEnabled);
}

void ScDocShellModificator::SetDocumentModified()
{
    ScDocument& rDoc = rDocShell.GetDocument();
    rDoc.ClearFormulaContext();
    if ( !rDoc.IsImportingXML() )
    {
        // AutoCalcShellDisabled temporaer restaurieren
        bool bDisabled = rDoc.IsAutoCalcShellDisabled();
        rDoc.SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
        rDocShell.SetDocumentModified();
        rDoc.SetAutoCalcShellDisabled( bDisabled );
    }
    else
    {
        // uno broadcast is necessary for api to work
        // -> must also be done during xml import
        rDoc.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
    }
}

bool ScDocShell::IsChangeRecording() const
{
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    return pChangeTrack != nullptr;
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
        // Slots invalidieren
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
            // when password protection is applied change tracking must always be active
            SetChangeRecording( true );

            css::uno::Sequence< sal_Int8 > aProtectionHash;
            SvPasswordHelper::GetHashPassword( aProtectionHash, rNewPassword );
            pChangeTrack->SetProtection( aProtectionHash );
        }
        else
        {
            pChangeTrack->SetProtection( css::uno::Sequence< sal_Int8 >() );
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

bool ScDocShell::GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash )
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

void ScDocShell::libreOfficeKitCallback(int /*nType*/, const char* /*pPayload*/) const
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
