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

#include <docsh.hxx>

#include <config_features.h>
#include <scitems.hxx>
#include <sc.hrc>
#include <vcl/errinf.hxx>
#include <editeng/justifyitem.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/classids.hxx>
#include <comphelper/propertyvalue.hxx>
#include <formula/errorcodes.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/syswin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <svl/PasswordHelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/event.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/fstathelper.hxx>
#include <svl/sharecontrolfile.hxx>
#include <svl/urihelper.hxx>
#include <osl/file.hxx>
#include <chgtrack.hxx>
#include <chgviset.hxx>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/VBAScriptEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAScriptListener.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <config_folders.h>

#include <scabstdlg.hxx>
#include <sot/formats.hxx>
#include <svx/compatflags.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/theme/Theme.hxx>

#include <formulacell.hxx>
#include <global.hxx>
#include <filter.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <docfunc.hxx>
#include <imoptdlg.hxx>
#include <impex.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <globstr.hrc>
#include <scerrors.hxx>
#include <brdcst.hxx>
#include <stlpool.hxx>
#include <autostyl.hxx>
#include <attrib.hxx>
#include <asciiopt.hxx>
#include <progress.hxx>
#include <pntlock.hxx>
#include <docuno.hxx>
#include <appoptio.hxx>
#include <formulaopt.hxx>
#include <scdll.hxx>
#include <detdata.hxx>
#include <printfun.hxx>
#include <dociter.hxx>
#include <cellform.hxx>
#include <chartlis.hxx>
#include <hints.hxx>
#include <xmlwrap.hxx>
#include <drwlayer.hxx>
#include <dbdata.hxx>
#include <scextopt.hxx>
#include <compiler.hxx>
#include <warnpassword.hxx>
#include <sheetdata.hxx>
#include <table.hxx>
#include <tabprotection.hxx>
#include <docparam.hxx>
#include "docshimp.hxx"
#include <sizedev.hxx>
#include <undomanager.hxx>
#include <refreshtimerprotector.hxx>

#include <officecfg/Office/Calc.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <uiitems.hxx>
#include <dpobject.hxx>
#include <markdata.hxx>
#include <docoptio.hxx>
#include <orcusfilters.hxx>
#include <datastream.hxx>
#include <documentlinkmgr.hxx>
#include <refupdatecontext.hxx>
#include <DocumentModelAccessor.hxx>

#include <memory>
#include <vector>

#include <comphelper/lok.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/pathoptions.hxx>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using std::shared_ptr;
using ::std::vector;

//  Filter names (like in sclib.cxx)

constexpr OUStringLiteral pFilterSc50 = u"StarCalc 5.0";
const char pFilterXML[]      = "StarOffice XML (Calc)";
constexpr OUString pFilterLotus = u"Lotus"_ustr;
const char pFilterQPro6[]    = "Quattro Pro 6.0";
const char16_t pFilterExcel4[] = u"MS Excel 4.0";
const char16_t pFilterEx4Temp[] = u"MS Excel 4.0 Vorlage/Template";
const char pFilterExcel5[]   = "MS Excel 5.0/95";
const char pFilterEx5Temp[]  = "MS Excel 5.0/95 Vorlage/Template";
const char pFilterExcel95[]  = "MS Excel 95";
const char pFilterEx95Temp[] = "MS Excel 95 Vorlage/Template";
const char pFilterExcel97[]  = "MS Excel 97";
const char pFilterEx97Temp[] = "MS Excel 97 Vorlage/Template";
constexpr OUString pFilterDBase = u"dBase"_ustr;
constexpr OUString pFilterDif = u"DIF"_ustr;
const char16_t pFilterSylk[] = u"SYLK";
constexpr OUString pFilterHtml = u"HTML (StarCalc)"_ustr;
constexpr OUString pFilterHtmlWebQ = u"calc_HTML_WebQuery"_ustr;
const char16_t pFilterRtf[]  = u"Rich Text Format (StarCalc)";

#define ShellClass_ScDocShell
#include <scslots.hxx>

SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell)

void ScDocShell::InitInterface_Impl()
{
}

//  GlobalName of the current version:
SFX_IMPL_OBJECTFACTORY( ScDocShell, SvGlobalName(SO3_SC_CLASSID), "scalc" )


void ScDocShell::FillClass( SvGlobalName* pClassName,
                                        SotClipboardFormatId* pFormat,
                                        OUString* pFullTypeName,
                                        sal_Int32 nFileFormat,
                                        bool bTemplate /* = false */) const
{
    if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SotClipboardFormatId::STARCALC_60;
        *pFullTypeName  = ScResId( SCSTR_LONG_SCDOC_NAME_60 );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = bTemplate ? SotClipboardFormatId::STARCALC_8_TEMPLATE : SotClipboardFormatId::STARCALC_8;
        *pFullTypeName  = ScResId( SCSTR_LONG_SCDOC_NAME_80 );
    }
    else
    {
        OSL_FAIL("Which version?");
    }
}

std::set<Color> ScDocShell::GetDocColors()
{
    return m_pDocument->GetDocColors();
}

std::shared_ptr<sfx::IDocumentModelAccessor> ScDocShell::GetDocumentModelAccessor() const
{
    std::shared_ptr<sfx::IDocumentModelAccessor> pReturn;
    pReturn.reset(new sc::DocumentModelAccessor(m_pDocument));
    return pReturn;
}

std::shared_ptr<model::ColorSet> ScDocShell::GetThemeColors()
{
    ScTabViewShell* pShell = GetBestViewShell();
    if (!pShell)
        return {};

    SdrModel* pSdrModel = GetDocument().GetDrawLayer();
    if (!pSdrModel)
        return {};

    auto const& pTheme = pSdrModel->getTheme();
    if (!pTheme)
        return {};

    return pTheme->getColorSet();
}

void ScDocShell::DoEnterHandler()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh && pViewSh->GetViewData().GetDocShell() == this)
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
        if ( m_pDocument->GetChangeTrack() && m_pDocument->GetChangeTrack()->GetFirst() )
          nState |= HiddenInformation::RECORDEDCHANGES;
    }
    if ( nStates & HiddenInformation::NOTES )
    {
        SCTAB nTableCount = m_pDocument->GetTableCount();
        bool bFound = false;
        for (SCTAB nTab = 0; nTab < nTableCount && !bFound; ++nTab)
        {
            if (m_pDocument->HasTabNotes(nTab)) //TODO:
                bFound = true;
        }

        if (bFound)
            nState |= HiddenInformation::NOTES;
    }

    return nState;
}

void ScDocShell::BeforeXMLLoading()
{
    m_pDocument->EnableIdle(false);

    // prevent unnecessary broadcasts and updates
    OSL_ENSURE(m_pModificator == nullptr, "The Modificator should not exist");
    m_pModificator.reset( new ScDocShellModificator( *this ) );

    m_pDocument->SetImportingXML( true );
    m_pDocument->EnableExecuteLink( false );   // #i101304# to be safe, prevent nested loading from external references
    m_pDocument->EnableUndo( false );
    // prevent unnecessary broadcasts and "half way listeners"
    m_pDocument->SetInsertingFromOtherDoc( true );
}

void ScDocShell::AfterXMLLoading(bool bRet)
{
    if (GetCreateMode() != SfxObjectCreateMode::ORGANIZER)
    {
        UpdateLinks();
        // don't prevent establishing of listeners anymore
        m_pDocument->SetInsertingFromOtherDoc( false );
        if ( bRet )
        {
            ScChartListenerCollection* pChartListener = m_pDocument->GetChartListenerCollection();
            if (pChartListener)
                pChartListener->UpdateDirtyCharts();

            // #95582#; set the table names of linked tables to the new path
            SCTAB nTabCount = m_pDocument->GetTableCount();
            for (SCTAB i = 0; i < nTabCount; ++i)
            {
                if (m_pDocument->IsLinked( i ))
                {
                    OUString aName;
                    m_pDocument->GetName(i, aName);
                    OUString aLinkTabName = m_pDocument->GetLinkTab(i);
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
                                else if( *pNameBuffer != '\\' || *(pNameBuffer+1) != '\'' )
                                    aDocURLBuffer.append(*pNameBuffer); // If escaped quote: only quote in the name
                                ++pNameBuffer;
                            }

                            if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  // after the last quote of the docname should be the # char
                            {
                                sal_Int32 nIndex = nNameLength - nLinkTabNameLength;
                                INetURLObject aINetURLObject(aDocURLBuffer);
                                if(aName.match( aLinkTabName, nIndex) &&
                                    (aName[nIndex - 1] == '#') && // before the table name should be the # char
                                    !aINetURLObject.HasError()) // the docname should be a valid URL
                                {
                                    aName = ScGlobal::GetDocTabName( m_pDocument->GetLinkDoc( i ), m_pDocument->GetLinkTab( i ) );
                                    m_pDocument->RenameTab(i, aName, true/*bExternalDocument*/);
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
            ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
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
        m_pDocument->SetInsertingFromOtherDoc( false );

    m_pDocument->SetImportingXML( false );
    m_pDocument->EnableExecuteLink( true );
    m_pDocument->EnableUndo( true );
    m_bIsEmpty = false;

    if (m_pModificator)
    {
        ScDocument::HardRecalcState eRecalcState = m_pDocument->GetHardRecalcState();
        // Temporarily set hard-recalc to prevent calling
        // ScFormulaCell::Notify() during destruction of the Modificator which
        // will set the cells dirty.
        if (eRecalcState == ScDocument::HardRecalcState::OFF)
            m_pDocument->SetHardRecalcState(ScDocument::HardRecalcState::TEMPORARY);
        m_pModificator.reset();
        m_pDocument->SetHardRecalcState(eRecalcState);
    }
    else
    {
        OSL_FAIL("The Modificator should exist");
    }

    m_pDocument->EnableIdle(true);
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
    if (r.maRange.aEnd.Row() == rShell.GetDocument().MaxRow())
        // Unlimited range.
        nLimit = 0;

    sc::DataStream::MoveType eMove =
        r.meInsertPos == sc::ImportPostProcessData::DataStream::InsertTop ?
        sc::DataStream::MOVE_DOWN : sc::DataStream::RANGE_DOWN;

    sc::DataStream* pStrm = new sc::DataStream(&rShell, r.maURL, aTopRange, nLimit, eMove);
    pStrm->SetRefreshOnEmptyLine(r.mbRefreshOnEmpty);
    sc::DocumentLinkManager& rMgr = rShell.GetDocument().GetDocLinkManager();
    rMgr.setDataStream(pStrm);
}

class MessageWithCheck : public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::CheckButton> m_xWarningOnBox;
public:
    MessageWithCheck(weld::Window *pParent, const OUString& rUIFile, const OUString& rDialogId)
        : MessageDialogController(pParent, rUIFile, rDialogId, "ask")
        , m_xWarningOnBox(m_xBuilder->weld_check_button("ask"))
    {
    }
    bool get_active() const { return m_xWarningOnBox->get_active(); }
    void hide_ask() const { m_xWarningOnBox->set_visible(false); };
};

#if HAVE_FEATURE_SCRIPTING
class VBAScriptListener : public ::cppu::WeakImplHelper< css::script::vba::XVBAScriptListener >
{
private:
    ScDocShell* m_pDocSh;
public:
    VBAScriptListener(ScDocShell* pDocSh) : m_pDocSh(pDocSh)
    {
    }

    // XVBAScriptListener
    virtual void SAL_CALL notifyVBAScriptEvent( const ::css::script::vba::VBAScriptEvent& aEvent ) override
    {
        if (aEvent.Identifier == script::vba::VBAScriptEventId::SCRIPT_STOPPED &&
            m_pDocSh->GetClipData().is())
        {
            m_pDocSh->SetClipData(uno::Reference<datatransfer::XTransferable2>());
        }
    }

    // XEventListener
    virtual void SAL_CALL disposing( const ::css::lang::EventObject& /*Source*/ ) override
    {
    }
};
#endif

}

bool ScDocShell::LoadXML( SfxMedium* pLoadMedium, const css::uno::Reference< css::embed::XStorage >& xStor )
{
    LoadMediumGuard aLoadGuard(m_pDocument.get());

    //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now

    // no Seek(0) here - always loading from storage, GetInStream must not be called

    BeforeXMLLoading();

    ScXMLImportWrapper aImport(*this, pLoadMedium, xStor);

    bool bRet = false;
    ErrCodeMsg nError = ERRCODE_NONE;
    m_pDocument->LockAdjustHeight();
    if (GetCreateMode() == SfxObjectCreateMode::ORGANIZER)
        bRet = aImport.Import(ImportFlags::Styles, nError);
    else
        bRet = aImport.Import(ImportFlags::All, nError);

    if ( nError )
        pLoadMedium->SetError(nError);

    processDataStream(*this, aImport.GetImportPostProcessData());

    //if the document was not generated by LibreOffice, do hard recalc in case some other document
    //generator saved cached formula results that differ from LibreOffice's calculated results or
    //did not use cached formula results.
    uno::Reference<document::XDocumentProperties> xDocProps = GetModel()->getDocumentProperties();

    ScRecalcOptions nRecalcMode =
        static_cast<ScRecalcOptions>(officecfg::Office::Calc::Formula::Load::ODFRecalcMode::get());

    bool bHardRecalc = false;
    if (nRecalcMode == RECALC_ASK)
    {
        OUString sProductName(utl::ConfigManager::getProductName());
        if (m_pDocument->IsUserInteractionEnabled() && xDocProps->getGenerator().indexOf(sProductName) == -1)
        {
            // Generator is not LibreOffice.  Ask if the user wants to perform
            // full re-calculation.
            MessageWithCheck aQueryBox(GetActiveDialogParent(),
                    "modules/scalc/ui/recalcquerydialog.ui", "RecalcQueryDialog");
            aQueryBox.set_primary_text(ScResId(STR_QUERY_FORMULA_RECALC_ONLOAD_ODS));
            aQueryBox.set_default_response(RET_YES);

            if ( officecfg::Office::Calc::Formula::Load::OOXMLRecalcMode::isReadOnly() )
                aQueryBox.hide_ask();

            bHardRecalc = aQueryBox.run() == RET_YES;

            if (aQueryBox.get_active())
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
        DoHardRecalc();
    else
    {
        // still need to recalc volatile formula cells.
        m_pDocument->Broadcast(ScHint(SfxHintId::ScDataChanged, BCA_BRDCST_ALWAYS));
    }

    AfterXMLLoading(bRet);

    m_pDocument->UnlockAdjustHeight();
    return bRet;
}

bool ScDocShell::SaveXML( SfxMedium* pSaveMedium, const css::uno::Reference< css::embed::XStorage >& xStor )
{
    m_pDocument->EnableIdle(false);

    ScXMLImportWrapper aImport(*this, pSaveMedium, xStor);
    bool bRet(false);
    if (GetCreateMode() != SfxObjectCreateMode::ORGANIZER)
        bRet = aImport.Export(false);
    else
        bRet = aImport.Export(true);

    m_pDocument->EnableIdle(true);

    return bRet;
}

bool ScDocShell::Load( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(m_pDocument.get());
    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    //  only the latin script language is loaded
    //  -> initialize the others from options (before loading)
    InitOptions(true);

    // If this is an ODF file being loaded, then by default, use legacy processing
    // (if required, it will be overridden in *::ReadUserDataSequence())
    if (IsOwnStorageFormat(rMedium))
    {
        if (ScDrawLayer* pDrawLayer = m_pDocument->GetDrawLayer())
        {
            pDrawLayer->SetCompatibilityFlag(SdrCompatibilityFlag::AnchoredTextOverflowLegacy,
                                             true); // for tdf#99729
            pDrawLayer->SetCompatibilityFlag(SdrCompatibilityFlag::LegacyFontwork,
                                             true); // for tdf#148000
        }
    }

    GetUndoManager()->Clear();

    bool bRet = SfxObjectShell::Load(rMedium);
    if (bRet)
    {
        SetInitialLinkUpdate(&rMedium);

        {
            //  prepare a valid document for XML filter
            //  (for ConvertFrom, InitNew is called before)
            m_pDocument->MakeTable(0);
            m_pDocument->GetStyleSheetPool()->CreateStandardStyles();
            m_pDocument->UpdStlShtPtrsFrmNms();

            /* Create styles that are imported through Orcus */

            OUString aURL("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/calc/styles.xml");
            rtl::Bootstrap::expandMacros(aURL);

            OUString aPath;
            osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

            ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();

            if (pOrcus)
            {
                pOrcus->importODS_Styles(*m_pDocument, aPath);
                m_pDocument->GetStyleSheetPool()->setAllParaStandard();
            }

            bRet = LoadXML( &rMedium, nullptr );
        }
    }

    if (!bRet && !rMedium.GetErrorIgnoreWarning())
        rMedium.SetError(SVSTREAM_FILEFORMAT_ERROR);

    if (rMedium.GetErrorIgnoreWarning())
        SetError(rMedium.GetErrorIgnoreWarning());

    InitItems();
    CalcOutputFactor();

    // invalidate eventually temporary table areas
    if ( bRet )
        m_pDocument->InvalidateTableArea();

    m_bIsEmpty = false;
    FinishedLoading();
    return bRet;
}

void ScDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const ScTablesHint* pScHint = dynamic_cast< const ScTablesHint* >( &rHint );
    if (pScHint)
    {
        if (pScHint->GetTablesHintId() == SC_TAB_INSERTED)
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = m_pDocument->GetVbaEventProcessor();
            if ( xVbaEvents.is() ) try
            {
                uno::Sequence< uno::Any > aArgs{ uno::Any(pScHint->GetTab1()) };
                xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_NEWSHEET, aArgs );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    if ( auto pStyleSheetHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint) ) // Template changed
        NotifyStyle( *pStyleSheetHint );
    else if ( auto pStlHint = dynamic_cast<const ScAutoStyleHint*>(&rHint) )
    {
        //! direct call for AutoStyles

        //  this is called synchronously from ScInterpreter::ScStyle,
        //  modifying the document must be asynchronous
        //  (handled by AddInitial)

        const ScRange& aRange = pStlHint->GetRange();
        const OUString& aName1 = pStlHint->GetStyle1();
        const OUString& aName2 = pStlHint->GetStyle2();
        sal_uInt32 nTimeout = pStlHint->GetTimeout();

        if (!m_pAutoStyleList)
            m_pAutoStyleList.reset( new ScAutoStyleList(this) );
        m_pAutoStyleList->AddInitial( aRange, aName1, nTimeout, aName2 );
    }
    else if (rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
    {
        switch (static_cast<const SfxEventHint&>(rHint).GetEventId())
        {
            case SfxEventHintId::LoadFinished:
                {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    // the readonly documents should not be opened in shared mode
                    if ( HasSharedXMLFlagSet() && !SC_MOD()->IsInSharedDocLoading() && !IsReadOnly() )
                    {
                        if ( SwitchToShared( true, false ) )
                        {
                            ScViewData* pViewData = GetViewData();
                            ScTabView* pTabView = ( pViewData ? pViewData->GetView() : nullptr );
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
            case SfxEventHintId::ViewCreated:
                {
 #if HAVE_FEATURE_SCRIPTING
                    uno::Reference<script::vba::XVBACompatibility> xVBACompat(GetBasicContainer(), uno::UNO_QUERY);
                    if ( !m_xVBAListener.is() && xVBACompat.is() )
                    {
                        m_xVBAListener.set(new VBAScriptListener(this));
                        xVBACompat->addVBAScriptListener(m_xVBAListener);
                    }
#endif

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocLoading()
                         && !comphelper::LibreOfficeKit::isActive() )
                    {
                        ScAppOptions aAppOptions = SC_MOD()->GetAppOptions();
                        if ( aAppOptions.GetShowSharedDocumentWarning() )
                        {
                            MessageWithCheck aWarningBox(ScDocShell::GetActiveDialogParent(),
                                    "modules/scalc/ui/sharedwarningdialog.ui", "SharedWarningDialog");
                            aWarningBox.run();

                            bool bChecked = aWarningBox.get_active();
                            if (bChecked)
                            {
                                aAppOptions.SetShowSharedDocumentWarning(false);
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
                                    SfxViewFrame* pViewFrame = ( pViewShell ? &pViewShell->GetViewFrame() : nullptr );
                                    SfxFrame* pFrame = ( pViewFrame ? &pViewFrame->GetFrame() : nullptr );
                                    uno::Reference< frame::XController > xController = ( pFrame ? pFrame->GetController() : nullptr );
                                    uno::Reference< sheet::XSpreadsheetView > xSpreadsheetView( xController, uno::UNO_QUERY_THROW );
                                    uno::Sequence< beans::NamedValue > aArgsForJob { { "SpreadsheetView", uno::Any( xSpreadsheetView ) } };
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
            case SfxEventHintId::SaveDoc:
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
                                xModel.set( LoadSharedDocument(), uno::UNO_SET_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = comphelper::getFromUnoTunnel<ScModelObj>( xModel );
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

                                        OUString aUserName( ScResId( STR_UNKNOWN_USER ) );
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
                                            OUString aMessage( ScResId( STR_FILE_LOCKED_SAVE_LATER ) );
                                            aMessage = aMessage.replaceFirst( "%1", aUserName );

                                            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                                                       VclMessageType::Warning, VclButtonsType::NONE,
                                                                                       aMessage));
                                            xWarn->add_button(GetStandardText(StandardButtonType::Retry), RET_RETRY);
                                            xWarn->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                                            xWarn->set_default_response(RET_RETRY);
                                            if (xWarn->run() == RET_RETRY)
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
                                            ScChangeViewSettings* pChangeViewSet = m_pDocument->GetChangeViewSettings();
                                            if ( pChangeViewSet && pChangeViewSet->ShowChanges() )
                                            {
                                                pChangeViewSet->SetShowChanges( false );
                                                pChangeViewSet->SetShowAccepted( false );
                                                m_pDocument->SetChangeViewSettings( *pChangeViewSet );
                                                bChangedViewSettings = true;
                                            }

                                            // TODO/LATER: More entries from the MediaDescriptor might be interesting for the merge
                                            uno::Sequence< beans::PropertyValue > aValues{
                                                comphelper::makePropertyValue(
                                                    "FilterName",
                                                    GetMedium()->GetFilter()->GetFilterName())
                                            };

                                            const SfxStringItem* pPasswordItem = GetMedium()->GetItemSet().GetItem(SID_PASSWORD, false);
                                            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
                                            {
                                                aValues.realloc( 2 );
                                                auto pValues = aValues.getArray();
                                                pValues[1].Name = "Password";
                                                pValues[1].Value <<= pPasswordItem->GetValue();
                                            }
                                            const SfxUnoAnyItem* pEncryptionItem = GetMedium()->GetItemSet().GetItem(SID_ENCRYPTIONDATA, false);
                                            if (pEncryptionItem)
                                            {
                                                aValues.realloc(aValues.getLength() + 1);
                                                auto pValues = aValues.getArray();
                                                pValues[aValues.getLength() - 1].Name = "EncryptionData";
                                                pValues[aValues.getLength() - 1].Value = pEncryptionItem->GetValue();
                                            }

                                            SC_MOD()->SetInSharedDocSaving( true );
                                            GetModel()->storeToURL( GetSharedFileURL(), aValues );
                                            SC_MOD()->SetInSharedDocSaving( false );

                                            if ( bChangedViewSettings )
                                            {
                                                pChangeViewSet->SetShowChanges( true );
                                                pChangeViewSet->SetShowAccepted( true );
                                                m_pDocument->SetChangeViewSettings( *pChangeViewSet );
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
                                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                                   ScResId(STR_DOC_NOLONGERSHARED)));
                                        xWarn->run();

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
                                TOOLS_WARN_EXCEPTION( "sc", "SfxEventHintId::SaveDoc" );
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
                            SetError(ERRCODE_IO_ABORT); // this error code will produce no error message, but will break the further saving process
                    }
#endif

                    if (m_pSheetSaveData)
                        m_pSheetSaveData->SetInSupportedSave(true);
                }
                break;
            case SfxEventHintId::SaveAsDoc:
                {
                    if ( GetDocument().GetExternalRefManager()->containsUnsavedReferences() )
                    {
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                                   ScResId(STR_UNSAVED_EXT_REF)));
                        if (RET_NO == xWarn->run())
                        {
                            SetError(ERRCODE_IO_ABORT); // this error code will produce no error message, but will break the further saving process
                        }
                    }
                    [[fallthrough]];
                }
            case SfxEventHintId::SaveToDoc:
                // #i108978# If no event is sent before saving, there will also be no "...DONE" event,
                // and SAVE/SAVEAS can't be distinguished from SAVETO. So stream copying is only enabled
                // if there is a SAVE/SAVEAS/SAVETO event first.
                if (m_pSheetSaveData)
                    m_pSheetSaveData->SetInSupportedSave(true);
                break;
            case SfxEventHintId::SaveDocDone:
            case SfxEventHintId::SaveAsDocDone:
                {
                    // new positions are used after "save" and "save as", but not "save to"
                    UseSheetSaveEntries();      // use positions from saved file for next saving
                    [[fallthrough]];
                }
            case SfxEventHintId::SaveToDocDone:
                // only reset the flag, don't use the new positions
                if (m_pSheetSaveData)
                    m_pSheetSaveData->SetInSupportedSave(false);
                break;
            default:
                {
                }
                break;
        }
    }
    else if (rHint.GetId() == SfxHintId::TitleChanged) // Without parameter
    {
        m_pDocument->SetName( SfxShell::GetName() );
        //  RegisterNewTargetNames doesn't exist any longer
        SfxGetpApp()->Broadcast(SfxHint( SfxHintId::ScDocNameChanged )); // Navigator
    }
    else if (rHint.GetId() == SfxHintId::Deinitializing)
    {

#if HAVE_FEATURE_SCRIPTING
        uno::Reference<script::vba::XVBACompatibility> xVBACompat(GetBasicContainer(), uno::UNO_QUERY);
        if (m_xVBAListener.is() && xVBACompat.is())
        {
            xVBACompat->removeVBAScriptListener(m_xVBAListener);
        }
#endif

        if (m_pDocument->IsClipboardSource())
        {
            // Notes copied to the clipboard have a raw SdrCaptionObj pointer
            // copied from this document, forget it as it references this
            // document's drawing layer pages and what not, which otherwise when
            // pasting to another document after this document was destructed would
            // attempt to access non-existing data. Preserve the text data though.
            ScDocument* pClipDoc = ScModule::GetClipDoc();
            if (pClipDoc)
                pClipDoc->ClosingClipboardSource();
        }
    }

    if (rHint.GetId() != SfxHintId::ThisIsAnSfxEventHint)
        return;

    switch(static_cast<const SfxEventHint&>(rHint).GetEventId())
    {
       case SfxEventHintId::CreateDoc:
            {
                uno::Any aWorkbook;
                aWorkbook <<= mxAutomationWorkbookObject;
                uno::Sequence< uno::Any > aArgs{ aWorkbook };
                SC_MOD()->CallAutomationApplicationEventSinks( "NewWorkbook", aArgs );
            }
            break;
        case SfxEventHintId::OpenDoc:
            {
                uno::Any aWorkbook;
                aWorkbook <<= mxAutomationWorkbookObject;
                uno::Sequence< uno::Any > aArgs{ aWorkbook };
                SC_MOD()->CallAutomationApplicationEventSinks( "WorkbookOpen", aArgs );
            }
            break;
        default:
            break;
    }
}

// Load contents for organizer
bool ScDocShell::LoadFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(m_pDocument.get());
    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    weld::WaitObject aWait( GetActiveDialogParent() );

    bool bRet = false;

    SetInitialLinkUpdate(&rMedium);

    //  until loading/saving only the styles in XML is implemented,
    //  load the whole file
    bRet = LoadXML( &rMedium, nullptr );
    InitItems();

    SfxObjectShell::LoadFrom( rMedium );

    return bRet;
}

static void lcl_parseHtmlFilterOption(const OUString& rOption, LanguageType& rLang, bool& rDateConvert, bool& rScientificConvert)
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

    if (!aTokens.empty())
        rLang = static_cast<LanguageType>(aTokens[0].toInt32());
    if (aTokens.size() > 1)
        rDateConvert = static_cast<bool>(aTokens[1].toInt32());
    if (aTokens.size() > 2)
        rScientificConvert = static_cast<bool>(aTokens[2].toInt32());
}

bool ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
    LoadMediumGuard aLoadGuard(m_pDocument.get());

    bool bRet = false; // sal_False means user quit!
                           // On error: Set error at stream

    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    GetUndoManager()->Clear();

    // Set optimal col width after import?
    bool bSetColWidths = false;
    bool bSetSimpleTextColWidths = false;
    std::map<SCCOL, ScColWidthParam> aColWidthParam;
    ScRange aColWidthRange;
    // Set optimal row height after import?
    bool bSetRowHeights = false;

    vector<ScDocRowHeightUpdater::TabRanges> aRecalcRowRangesArray;

    //  All filters need the complete file in one piece (not asynchronously)
    //  So make sure that we transfer the whole file with CreateFileStream
    rMedium.GetPhysicalName();  //! Call CreateFileStream directly, if available

    SetInitialLinkUpdate(&rMedium);

    std::shared_ptr<const SfxFilter> pFilter = rMedium.GetFilter();
    if (pFilter)
    {
        OUString aFltName = pFilter->GetFilterName();

        bool bCalc3 = aFltName == "StarCalc 3.0";
        bool bCalc4 = aFltName == "StarCalc 4.0";
        if (!bCalc3 && !bCalc4)
            m_pDocument->SetInsertingFromOtherDoc( true );

        if (aFltName == pFilterXML)
            bRet = LoadXML( &rMedium, nullptr );
        else if (aFltName == pFilterLotus)
        {
            OUString sItStr;
            if ( const SfxStringItem* pOptionsItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS, true ) )
            {
                sItStr = pOptionsItem->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for lotus import (from API without options):
                //  IBM_437 encoding
                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
            }

            ErrCode eError = ScFormatFilter::Get().ScImportLotus123( rMedium, *m_pDocument,
                                                ScGlobal::GetCharsetValue(sItStr));
            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);

                if( eError.IsWarning() )
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
            ErrCode eError = ScFormatFilter::Get().ScImportExcel( rMedium, m_pDocument.get(), eFormat );
            m_pDocument->UpdateFontCharSet();
            if ( m_pDocument->IsChartListenerCollectionNeedsUpdate() )
                m_pDocument->UpdateChartListenerCollection(); //! For all imports?

            // all graphics objects must have names
            m_pDocument->EnsureGraphicNames();

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
            else
                bRet = true;
        }
        else if (aFltName == SC_TEXT_CSV_FILTER_NAME)
        {
            ScAsciiOptions aOptions;
            bool bOptInit = false;

            if ( const SfxStringItem* pOptionsItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
            {
                aOptions.ReadFromString( pOptionsItem->GetValue() );
                bOptInit = true;
            }

            if ( !bOptInit )
            {
                //  default for ascii import (from API without options):
                //  UTF-8 encoding, comma, double quotes

                aOptions.SetCharSet(RTL_TEXTENCODING_UTF8);
                aOptions.SetFieldSeps( OUString(',') );
                aOptions.SetTextSep( '"' );
            }

            ErrCode eError = ERRCODE_NONE;
            bool bOverflowRow, bOverflowCol, bOverflowCell;
            bOverflowRow = bOverflowCol = bOverflowCell = false;

            if( ! rMedium.IsStorage() )
            {
                ScImportExport  aImpEx( *m_pDocument );
                aImpEx.SetExtOptions( aOptions );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->SetStreamCharSet( aOptions.GetCharSet() );
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL(), SotClipboardFormatId::STRING );
                    eError = bRet ? ERRCODE_NONE : SCERR_IMPORT_CONNECT;
                    m_pDocument->StartAllListeners();
                    sc::SetFormulaDirtyContext aCxt;
                    m_pDocument->SetAllFormulasDirty(aCxt);

                    // tdf#82254 - check whether to include a byte-order-mark in the output
                    if (const bool bIncludeBOM = aImpEx.GetIncludeBOM())
                    {
                        aOptions.SetIncludeBOM(bIncludeBOM);
                        rMedium.GetItemSet().Put(
                            SfxStringItem(SID_FILE_FILTEROPTIONS, aOptions.WriteToString()));
                    }

                    // for mobile case, we use a copy of the original document and give it a temporary name before editing
                    // Therefore, the sheet name becomes ugly, long and nonsensical.
#if !(defined ANDROID)
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
                    m_pDocument->RenameTab( 0, INetURLObject( rMedium.GetName()).GetBase(), true/*bExternalDocument*/);
#endif
                    bOverflowRow = aImpEx.IsOverflowRow();
                    bOverflowCol = aImpEx.IsOverflowCol();
                    bOverflowCell = aImpEx.IsOverflowCell();
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
            else if (!GetErrorIgnoreWarning() && (bOverflowRow || bOverflowCol || bOverflowCell))
            {
                // precedence: row, column, cell
                ErrCode nWarn = (bOverflowRow ? SCWARN_IMPORT_ROW_OVERFLOW :
                        (bOverflowCol ? SCWARN_IMPORT_COLUMN_OVERFLOW :
                         SCWARN_IMPORT_CELL_OVERFLOW));
                SetError(nWarn);
            }
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
        }
        else if (aFltName == pFilterDBase)
        {
            OUString sItStr;
            if ( const SfxStringItem* pOptionsItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
            {
                sItStr = pOptionsItem->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for dBase import (from API without options):
                //  IBM_850 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
            }

            ScDocRowHeightUpdater::TabRanges aRecalcRanges(0, m_pDocument->MaxRow());
            ErrCode eError = DBaseImport( rMedium.GetPhysicalName(),
                    ScGlobal::GetCharsetValue(sItStr), aColWidthParam, aRecalcRanges.maRanges );
            aRecalcRowRangesArray.push_back(aRecalcRanges);

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
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
                ErrCode eError;
                OUString sItStr;
                if ( const SfxStringItem* pOptionsItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
                {
                    sItStr = pOptionsItem->GetValue();
                }

                if (sItStr.isEmpty())
                {
                    //  default for DIF import (from API without options):
                    //  ISO8859-1/MS_1252 encoding

                    sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
                }

                eError = ScFormatFilter::Get().ScImportDif( *pStream, m_pDocument.get(), ScAddress(0,0,0),
                                    ScGlobal::GetCharsetValue(sItStr));
                if (eError != ERRCODE_NONE)
                {
                    if (!GetErrorIgnoreWarning())
                        SetError(eError);

                    if( eError.IsWarning() )
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
            ErrCode eError = SCERR_IMPORT_UNKNOWN;
            bool bOverflowRow, bOverflowCol, bOverflowCell;
            bOverflowRow = bOverflowCol = bOverflowCell = false;
            if( !rMedium.IsStorage() )
            {
                ScImportExport aImpEx( *m_pDocument );

                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL(), SotClipboardFormatId::SYLK );
                    eError = bRet ? ERRCODE_NONE : SCERR_IMPORT_UNKNOWN;
                    m_pDocument->StartAllListeners();
                    sc::SetFormulaDirtyContext aCxt;
                    m_pDocument->SetAllFormulasDirty(aCxt);

                    bOverflowRow = aImpEx.IsOverflowRow();
                    bOverflowCol = aImpEx.IsOverflowCol();
                    bOverflowCell = aImpEx.IsOverflowCell();
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
            else if (!GetErrorIgnoreWarning() && (bOverflowRow || bOverflowCol || bOverflowCell))
            {
                // precedence: row, column, cell
                ErrCode nWarn = (bOverflowRow ? SCWARN_IMPORT_ROW_OVERFLOW :
                        (bOverflowCol ? SCWARN_IMPORT_COLUMN_OVERFLOW :
                         SCWARN_IMPORT_CELL_OVERFLOW));
                SetError(nWarn);
            }
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
            bSetRowHeights = true;
        }
        else if (aFltName == pFilterQPro6)
        {
            ErrCode eError = ScFormatFilter::Get().ScImportQuattroPro(rMedium.GetInStream(), *m_pDocument);
            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
            else
                bRet = true;
            // TODO: Filter should set column widths. Not doing it here, it may
            // result in very narrow or wide columns, depending on content.
            // Setting row heights makes cells with font size attribution or
            // wrapping enabled look nicer...
            bSetRowHeights = true;
        }
        else if (aFltName == pFilterRtf)
        {
            ErrCode eError = SCERR_IMPORT_UNKNOWN;
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    pInStream->Seek( 0 );
                    ScRange aRange;
                    eError = ScFormatFilter::Get().ScImportRTF( *pInStream, rMedium.GetBaseURL(), m_pDocument.get(), aRange );
                    if (eError != ERRCODE_NONE)
                    {
                        if (!GetErrorIgnoreWarning())
                            SetError(eError);

                        if( eError.IsWarning() )
                            bRet = true;
                    }
                    else
                        bRet = true;
                    m_pDocument->StartAllListeners();
                    sc::SetFormulaDirtyContext aCxt;
                    m_pDocument->SetAllFormulasDirty(aCxt);
                    bSetColWidths = true;
                    bSetRowHeights = true;
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
        }
        else if (aFltName == pFilterHtml || aFltName == pFilterHtmlWebQ)
        {
            ErrCode eError = SCERR_IMPORT_UNKNOWN;
            bool bWebQuery = aFltName == pFilterHtmlWebQ;
            if( !rMedium.IsStorage() )
            {
                SvStream* pInStream = rMedium.GetInStream();
                if (pInStream)
                {
                    LanguageType eLang = LANGUAGE_SYSTEM;
                    bool bDateConvert = false;
                    bool bScientificConvert = true;
                    if ( const SfxStringItem* pOptionsItem = rMedium.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
                    {
                        OUString aFilterOption = pOptionsItem->GetValue();
                        lcl_parseHtmlFilterOption(aFilterOption, eLang, bDateConvert, bScientificConvert);
                    }

                    pInStream->Seek( 0 );
                    ScRange aRange;
                    // HTML does its own ColWidth/RowHeight
                    CalcOutputFactor();
                    SvNumberFormatter aNumFormatter( comphelper::getProcessComponentContext(), eLang);
                    eError = ScFormatFilter::Get().ScImportHTML( *pInStream, rMedium.GetBaseURL(), m_pDocument.get(), aRange,
                                            GetOutputFactor(), !bWebQuery, &aNumFormatter, bDateConvert, bScientificConvert );
                    if (eError != ERRCODE_NONE)
                    {
                        if (!GetErrorIgnoreWarning())
                            SetError(eError);

                        if( eError.IsWarning() )
                            bRet = true;
                    }
                    else
                        bRet = true;
                    m_pDocument->StartAllListeners();

                    sc::SetFormulaDirtyContext aCxt;
                    m_pDocument->SetAllFormulasDirty(aCxt);
                }
                else
                {
                    OSL_FAIL( "No Stream" );
                }
            }

            if (eError != ERRCODE_NONE)
            {
                if (!GetErrorIgnoreWarning())
                    SetError(eError);
                if( eError.IsWarning() )
                    bRet = true;
            }
        }
        else
        {
            ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
            if (!pOrcus)
                return false;

            switch (pOrcus->importByName(*m_pDocument, rMedium, aFltName))
            {
                case ScOrcusFilters::ImportResult::Success:
                    bRet = true;
                    break;
                case ScOrcusFilters::ImportResult::Failure:
                    bRet = false;
                    break;
                case ScOrcusFilters::ImportResult::NotSupported:
                {
                    if (!GetErrorIgnoreWarning())
                    {
                        SAL_WARN("sc.filter", "No match for filter '" << aFltName << "' in ConvertFrom");
                        SetError(SCERR_IMPORT_NI);
                    }
                    break;
                }
            }
        }

        if (!bCalc3)
            m_pDocument->SetInsertingFromOtherDoc( false );
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
        double nPPTX = ScGlobal::nScreenPPTX * static_cast<double>(aZoom) / GetOutputFactor(); // Factor is printer display ratio
        double nPPTY = ScGlobal::nScreenPPTY * static_cast<double>(aZoom);
        ScopedVclPtrInstance< VirtualDevice > pVirtDev;
        //  all sheets (for Excel import)
        SCTAB nTabCount = m_pDocument->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        {
            SCCOL nEndCol;
            SCROW nEndRow;
            m_pDocument->GetCellArea( nTab, nEndCol, nEndRow );
            aColWidthRange.aEnd.SetCol( nEndCol );
            aColWidthRange.aEnd.SetRow( nEndRow );
            ScMarkData aMark(m_pDocument->GetSheetLimits());
            aMark.SetMarkArea( aColWidthRange );
            aMark.MarkToMulti();

            // Order is important: First width, then height
            if ( bSetColWidths )
            {
                for ( SCCOL nCol=0; nCol <= nEndCol; nCol++ )
                {
                    if (!bSetSimpleTextColWidths)
                        aColWidthParam[nCol].mbSimpleText = false;

                    sal_uInt16 nWidth = m_pDocument->GetOptimalColWidth(
                        nCol, nTab, pVirtDev, nPPTX, nPPTY, aZoom, aZoom, false, &aMark,
                        &aColWidthParam[nCol] );
                    m_pDocument->SetColWidth( nCol, nTab,
                        nWidth + static_cast<sal_uInt16>(ScGlobal::nLastColWidthExtra) );
                }
            }
        }

        if (bSetRowHeights)
        {
            // Update all rows in all tables.
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(*m_pDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), nullptr);
            aUpdater.update();
        }
        else if (!aRecalcRowRangesArray.empty())
        {
            // Update only specified row ranges for better performance.
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(*m_pDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), &aRecalcRowRangesArray);
            aUpdater.update();
        }
    }
    FinishedLoading();

    // invalidate eventually temporary table areas
    if ( bRet )
        m_pDocument->InvalidateTableArea();

    m_bIsEmpty = false;

    return bRet;
}

bool ScDocShell::LoadExternal( SfxMedium& rMed )
{
    std::shared_ptr<const SfxFilter> pFilter = rMed.GetFilter();
    if (!pFilter)
        return false;

    if (pFilter->GetProviderName() == "orcus")
    {
        ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
        if (!pOrcus)
            return false;

        auto res = pOrcus->importByName(*m_pDocument, rMed, pFilter->GetName());
        if (res != ScOrcusFilters::ImportResult::Success)
            return false;

        FinishedLoading();
        return true;
    }

    return false;
}

ScDocShell::PrepareSaveGuard::PrepareSaveGuard( ScDocShell& rDocShell )
    : mrDocShell( rDocShell)
{
    // DoEnterHandler not here (because of AutoSave), is in ExecuteSave.

    ScChartListenerCollection* pCharts = mrDocShell.m_pDocument->GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                           // Charts to be updated.
    mrDocShell.m_pDocument->StopTemporaryChartLock();
    if (mrDocShell.m_pAutoStyleList)
        mrDocShell.m_pAutoStyleList->ExecuteAllNow();             // Execute template timeouts now.
    if (mrDocShell.m_pDocument->HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.m_pDocument->GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            pRefMgr->setAllCacheTableReferencedStati( false);
            mrDocShell.m_pDocument->MarkUsedExternalReferences();  // Mark tables of external references to be written.
        }
    }
    if (mrDocShell.GetCreateMode()== SfxObjectCreateMode::STANDARD)
        mrDocShell.SfxObjectShell::SetVisArea( tools::Rectangle() );   // "Normally" worked on => no VisArea.
}

ScDocShell::PrepareSaveGuard::~PrepareSaveGuard() COVERITY_NOEXCEPT_FALSE
{
    if (mrDocShell.m_pDocument->HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.m_pDocument->GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            // Prevent accidental data loss due to lack of knowledge.
            pRefMgr->setAllCacheTableReferencedStati( true);
        }
    }
}

bool ScDocShell::Save()
{
    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    if (const auto pFrame1 = SfxViewFrame::GetFirst(this))
    {
        if (auto pSysWin = pFrame1->GetWindow().GetSystemWindow())
        {
            pSysWin->SetAccessibleName(OUString());
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
        rPath = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    }
}

}

void ScDocShell::TerminateEditing()
{
    // Commit any cell changes before saving.
    SC_MOD()->InputEnterHandler();
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
            m_pDocument->InvalidateStreamOnSave();
        }
    }

    ScTabViewShell* pViewShell = GetBestViewShell();
    bool bNeedsRehash = ScPassHashHelper::needsPassHashRegen(*m_pDocument, PASSHASH_SHA1);
    if (bNeedsRehash)
        // legacy xls hash double-hashed by SHA1 is also supported.
        bNeedsRehash = ScPassHashHelper::needsPassHashRegen(*m_pDocument, PASSHASH_XL, PASSHASH_SHA1);
    if (bNeedsRehash)
    {   // SHA256 explicitly supported in ODF 1.2, implicitly in ODF 1.1
        bNeedsRehash = ScPassHashHelper::needsPassHashRegen(*m_pDocument, PASSHASH_SHA256);
    }

    if (pViewShell && bNeedsRehash)
    {
        bool bAutoSaveEvent = false;
        utl::MediaDescriptor lArgs(rMedium.GetArgs());
        lArgs[utl::MediaDescriptor::PROP_AUTOSAVEEVENT] >>= bAutoSaveEvent;
        if (bAutoSaveEvent)
        {
            // skip saving recovery file instead of showing re-type password dialog window
            SAL_WARN("sc.filter",
                     "Should re-type password for own format, won't export recovery file");
            rMedium.SetError(ERRCODE_SFX_WRONGPASSWORD);
            return false;
        }

        if (!pViewShell->ExecuteRetypePassDlg(PASSHASH_SHA1))
            // password re-type cancelled.  Don't save the document.
            return false;
    }

    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    //  wait cursor is handled with progress bar
    bool bRet = SfxObjectShell::SaveAs( rMedium );
    if (bRet)
        bRet = SaveXML( &rMedium, nullptr );

    return bRet;
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
        if ( bValue && eHorJust == SvxCellHorJustify::Standard )
            eHorJust = SvxCellHorJustify::Right;
        OUStringBuffer aTmp(nLen);
        switch ( eHorJust )
        {
            case SvxCellHorJustify::Right:
                comphelper::string::padToLength( aTmp, nLen - aString.getLength(), ' ' );
                aString = aTmp.append(aString);
                break;
            case SvxCellHorJustify::Center:
                comphelper::string::padToLength( aTmp, (nLen - aString.getLength()) / 2, ' ' );
                [[fallthrough]];
            default:
                aTmp.append(aString);
                comphelper::string::padToLength( aTmp, nLen, ' ' );
        }
        aString = aTmp.makeStringAndClear();
    }
    rStr = aString;
}

void lcl_ScDocShell_WriteEmptyFixedWidthString( SvStream& rStream,
        const ScDocument& rDoc, SCTAB nTab, SCCOL nCol )
{
    OUString aString;
    lcl_ScDocShell_GetFixedWidthString( aString, rDoc, nTab, nCol, false,
            SvxCellHorJustify::Standard );
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

}

void ScDocShell::AsciiSave( SvStream& rStream, const ScImportOptions& rAsciiOpt, SCTAB nTab )
{
    sal_Unicode cDelim    = rAsciiOpt.nFieldSepCode;
    sal_Unicode cStrDelim = rAsciiOpt.nTextSepCode;
    rtl_TextEncoding eCharSet      = rAsciiOpt.eCharSet;
    bool bFixedWidth      = rAsciiOpt.bFixedWidth;
    bool bSaveNumberAsSuch = rAsciiOpt.bSaveNumberAsSuch;
    bool bSaveAsShown     = rAsciiOpt.bSaveAsShown;
    bool bShowFormulas    = rAsciiOpt.bSaveFormulas;
    bool bIncludeBOM      = rAsciiOpt.bIncludeBOM;

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
        // tdf#82254 - check whether to include a byte-order-mark in the output
        if (bIncludeBOM && eCharSet == RTL_TEXTENCODING_UTF8)
            rStream.WriteUChar(0xEF).WriteUChar(0xBB).WriteUChar(0xBF);
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
    SCCOL nEndCol;
    SCROW nEndRow;
    m_pDocument->GetCellArea( nTab, nEndCol, nEndRow );

    ScProgress aProgress( this, ScResId( STR_SAVE_DOC ), nEndRow, true );

    OUString aString;

    bool bTabProtect = m_pDocument->IsTabProtected( nTab );

    SCCOL nCol;
    SCROW nRow;

    // Treat the top left cell separator "sep=" special.
    // Here nStartRow == 0 && nStartCol == 0
    if (!bFixedWidth && cDelim != 0)
    {
        // First row iterator.
        ScHorizontalCellIterator aIter( *m_pDocument, nTab, nStartCol, nStartRow, nEndCol, nStartRow);
        ScRefCellValue* pCell;
        // Must be first column and all following cells on this row must be
        // empty to fiddle with "sep=".
        if ((pCell = aIter.GetNext( nCol, nRow)) != nullptr && nCol == nStartCol && !aIter.GetNext( nCol, nRow))
        {
            if (pCell->getType() == CELLTYPE_STRING)
            {
                aString = pCell->getSharedString()->getString();
                if (aString.getLength() <= 5 && aString.startsWithIgnoreAsciiCase("sep="))
                {
                    // Cell content is /^sep=.?$/ so write current separator.
                    // Force the quote character to '"' regardless what is set
                    // for export because that is the only one recognized on
                    // import.
                    aString = "sep=" + OUStringChar(cDelim);
                    if (cStrDelim != 0)
                        rStream.WriteUniOrByteChar( '"', eCharSet);
                    if (eCharSet == RTL_TEXTENCODING_UNICODE)
                    {
                        write_uInt16s_FromOUString( rStream, aString);
                    }
                    else
                    {
                        OString aStrEnc = OUStringToOString( aString, eCharSet);
                        // write byte encoded
                        rStream.WriteBytes( aStrEnc.getStr(), aStrEnc.getLength());
                    }
                    if (cStrDelim != 0)
                        rStream.WriteUniOrByteChar( '"', eCharSet);
                    endlub( rStream );
                    ++nStartRow;
                }
            }
        }
    }

    SCCOL nNextCol = nStartCol;
    SCROW nNextRow = nStartRow;
    SCCOL nEmptyCol;
    SCROW nEmptyRow;
    SvNumberFormatter& rFormatter = *m_pDocument->GetFormatTable();

    ScHorizontalCellIterator aIter( *m_pDocument, nTab, nStartCol, nStartRow,
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
                            *m_pDocument, nTab, nEmptyCol );
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
                                *m_pDocument, nTab, nEmptyCol );
                    else if ( cDelim != 0 )
                        rStream.WriteUniOrByteChar( cDelim );
                }
                endlub( rStream );
            }
            for ( nEmptyCol = nStartCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // empty columns at beginning of row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            *m_pDocument, nTab, nEmptyCol );
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
                            *m_pDocument, nTab, nEmptyCol );
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

        CellType eType = pCell->getType();
        ScAddress aPos(nCol, nRow, nTab);
        if ( bTabProtect )
        {
            const ScProtectionAttr* pProtAttr =
                m_pDocument->GetAttr( nCol, nRow, nTab, ATTR_PROTECTION );
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;  // hide
        }
        bool bForceQuotes = false;
        bool bString;
        switch ( eType )
        {
            case CELLTYPE_NONE:
                aString.clear();
                bString = false;
                break;
            case CELLTYPE_FORMULA :
                {
                    FormulaError nErrCode;
                    if ( bShowFormulas )
                    {
                        aString = pCell->getFormula()->GetFormula();
                        bString = true;
                    }
                    else if ((nErrCode = pCell->getFormula()->GetErrCode()) != FormulaError::NONE)
                    {
                        aString = ScGlobal::GetErrorString( nErrCode );
                        bString = true;
                    }
                    else if (pCell->getFormula()->IsValue())
                    {
                        sal_uInt32 nFormat = m_pDocument->GetNumberFormat(aPos);
                        if ( bFixedWidth || bSaveAsShown )
                        {
                            const Color* pDummy;
                            aString = ScCellFormat::GetString(*pCell, nFormat, &pDummy, rFormatter, *m_pDocument);
                            bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                        }
                        else
                        {
                            aString = ScCellFormat::GetInputString(*pCell, nFormat, rFormatter, *m_pDocument);
                            bString = bForceQuotes = !bSaveNumberAsSuch;
                        }
                    }
                    else
                    {
                        if ( bSaveAsShown )
                        {
                            sal_uInt32 nFormat = m_pDocument->GetNumberFormat(aPos);
                            const Color* pDummy;
                            aString = ScCellFormat::GetString(*pCell, nFormat, &pDummy, rFormatter, *m_pDocument);
                        }
                        else
                            aString = pCell->getFormula()->GetString().getString();
                        bString = true;
                    }
                }
                break;
            case CELLTYPE_STRING :
                if ( bSaveAsShown )
                {
                    sal_uInt32 nFormat = m_pDocument->GetNumberFormat(aPos);
                    const Color* pDummy;
                    aString = ScCellFormat::GetString(*pCell, nFormat, &pDummy, rFormatter, *m_pDocument);
                }
                else
                    aString = pCell->getSharedString()->getString();
                bString = true;
                break;
            case CELLTYPE_EDIT :
                {
                    const EditTextObject* pObj = pCell->getEditText();
                    EditEngine& rEngine = m_pDocument->GetEditEngine();
                    rEngine.SetText( *pObj);
                    aString = rEngine.GetText();  // including LF
                    bString = true;
                }
                break;
            case CELLTYPE_VALUE :
                {
                    sal_uInt32 nFormat = m_pDocument->GetNumberFormat( nCol, nRow, nTab );
                    if ( bFixedWidth || bSaveAsShown )
                    {
                        const Color* pDummy;
                        aString = ScCellFormat::GetString(*pCell, nFormat, &pDummy, rFormatter, *m_pDocument);
                        bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                    }
                    else
                    {
                        aString = ScCellFormat::GetInputString(*pCell, nFormat, rFormatter, *m_pDocument);
                        bString = bForceQuotes = !bSaveNumberAsSuch;
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
            SvxCellHorJustify eHorJust =
                m_pDocument->GetAttr( nCol, nRow, nTab, ATTR_HOR_JUSTIFY )->GetValue();
            lcl_ScDocShell_GetFixedWidthString( aString, *m_pDocument, nTab, nCol,
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
                        sal_Int32 nPos = getTextSepPos(aUniString, rAsciiOpt, cStrDelim, cDelim, bNeedQuotes);
                        if (nPos >= 0)
                        {
                            OUString strFrom(cStrDelim);
                            OUString strTo = strFrom + strFrom;
                            aUniString = aUniString.replaceAll(strFrom, strTo);
                        }

                        if ( bNeedQuotes || bForceQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        write_uInt16s_FromOUString(rStream, aUniString);
                        if ( bNeedQuotes || bForceQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                    }
                    else
                    {
                        // This is nasty. The Unicode to byte encoding
                        // may convert typographical quotation marks to ASCII
                        // quotation marks, which may interfere with the delimiter,
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
                            sal_Int32 nPos = getTextSepPos(aStrDec, rAsciiOpt, aStrDelimDecoded, aDelimDecoded, bNeedQuotes);
                            if (nPos >= 0)
                            {
                                OUString strTo = aStrDelimDecoded + aStrDelimDecoded;
                                aStrDec = aStrDec.replaceAll(aStrDelimDecoded, strTo);
                            }

                            // write byte re-encoded
                            if ( bNeedQuotes || bForceQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                            rStream.WriteUnicodeOrByteText( aStrDec, eCharSet );
                            if ( bNeedQuotes || bForceQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        }
                        else
                        {
                            OString aStrEnc = OUStringToOString(aUniString, eCharSet);

                            // search on encoded string
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(aStrEnc, rAsciiOpt, aStrDelimEncoded, aDelimEncoded, bNeedQuotes);
                            if (nPos >= 0)
                            {
                                OString strTo = aStrDelimEncoded + aStrDelimEncoded;
                                aStrEnc = aStrEnc.replaceAll(aStrDelimEncoded, strTo);
                            }

                            // write byte encoded
                            if ( bNeedQuotes || bForceQuotes )
                                rStream.WriteBytes(
                                    aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                            rStream.WriteBytes(aStrEnc.getStr(), aStrEnc.getLength());
                            if ( bNeedQuotes || bForceQuotes )
                                rStream.WriteBytes(
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
                        *m_pDocument, nTab, nEmptyCol );
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
                        *m_pDocument, nTab, nEmptyCol );
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
    ScRefreshTimerProtector aProt( m_pDocument->GetRefreshTimerControlAddress() );

    //  #i6500# don't call DoEnterHandler here (doesn't work with AutoSave),
    //  it's already in ExecuteSave (as for Save and SaveAs)

    if (m_pAutoStyleList)
        m_pAutoStyleList->ExecuteAllNow(); // Execute template timeouts now
    if (GetCreateMode()== SfxObjectCreateMode::STANDARD)
        SfxObjectShell::SetVisArea( tools::Rectangle() ); // Edited normally -> no VisArea

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
        weld::WaitObject aWait( GetActiveDialogParent() );

        bool bDoSave = true;
        if( ScTabViewShell* pViewShell = GetBestViewShell() )
        {
            ScExtDocOptions* pExtDocOpt = m_pDocument->GetExtDocOptions();
            if( !pExtDocOpt )
            {
                m_pDocument->SetExtDocOptions( std::make_unique<ScExtDocOptions>() );
                pExtDocOpt = m_pDocument->GetExtDocOptions();
            }
            pViewShell->GetViewData().WriteExtOptions( *pExtDocOpt );

            /*  #i104990# If the imported document contains a medium
                password, determine if we can save it, otherwise ask the users
                whether they want to save without it. */
            if( (rMed.GetFilter()->GetFilterFlags() & SfxFilterFlags::ENCRYPTION) == SfxFilterFlags::NONE )
            {
                SfxItemSet& rItemSet = rMed.GetItemSet();
                if( rItemSet.GetItemState( SID_PASSWORD ) == SfxItemState::SET )
                {
                    bDoSave = ScWarnPassword::WarningOnPassword( rMed );
                    // #i42858# remove password from medium (warn only one time)
                    if( bDoSave )
                        rItemSet.ClearItem( SID_PASSWORD );
                }
            }

            if( bDoSave )
            {
                bool bNeedRetypePassDlg = ScPassHashHelper::needsPassHashRegen( *m_pDocument, PASSHASH_XL );
                bDoSave = !bNeedRetypePassDlg || pViewShell->ExecuteRetypePassDlg( PASSHASH_XL );
            }
        }

        if( bDoSave )
        {
            ExportFormatExcel eFormat = ExpBiff5;
            if( aFltName == pFilterExcel97 || aFltName == pFilterEx97Temp )
                eFormat = ExpBiff8;
            ErrCode eError = ScFormatFilter::Get().ScExportExcel5( rMed, m_pDocument.get(), eFormat, RTL_TEXTENCODING_MS_1252 );

            if( eError && !GetErrorIgnoreWarning() )
                SetError(eError);

            // don't return false for warnings
            bRet = eError.IsWarning() || (eError == ERRCODE_NONE);
        }
        else
        {
            // export aborted, i.e. "Save without password" warning
            SetError(ERRCODE_ABORT);
        }
    }
    else if (aFltName == SC_TEXT_CSV_FILTER_NAME)
    {
        OUString sItStr;
        if ( const SfxStringItem* pOptionsItem = rMed.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
        {
            sItStr = pOptionsItem->GetValue();
        }

        if ( sItStr.isEmpty() )
        {
            //  default for ascii export (from API without options):
            //  UTF-8 encoding, comma, double quotes

            ScImportOptions aDefOptions(',', '"', RTL_TEXTENCODING_UTF8);
            sItStr = aDefOptions.BuildString();
        }

        weld::WaitObject aWait( GetActiveDialogParent() );
        ScImportOptions aOptions( sItStr );

        if (aOptions.nSheetToExport)
        {
            // Only from command line --convert-to
            bRet = true;

            // Verbose only from command line, not UI (in case we actually
            // implement that) nor macro filter options.
            bool bVerbose = false;
            const css::uno::Sequence<css::beans::PropertyValue> & rArgs = rMed.GetArgs();
            const auto pProp = std::find_if( rArgs.begin(), rArgs.end(),
                    [](const css::beans::PropertyValue& rProp) { return rProp.Name == "ConversionRequestOrigin"; });
            if (pProp != rArgs.end())
            {
                OUString aOrigin;
                pProp->Value >>= aOrigin;
                bVerbose = (aOrigin == "CommandLine");
            }

            SCTAB nStartTab;
            SCTAB nCount = m_pDocument->GetTableCount();
            if (aOptions.nSheetToExport == -1)
            {
                // All sheets.
                nStartTab = 0;
            }
            else if (0 < aOptions.nSheetToExport && aOptions.nSheetToExport <= nCount)
            {
                // One sheet, 1-based.
                nCount = aOptions.nSheetToExport;
                nStartTab = nCount - 1;
            }
            else
            {
                // Usage error, no export but log.
                if (bVerbose)
                {
                    if (aOptions.nSheetToExport < 0)
                        std::cout << "Bad sheet number string given." << std::endl;
                    else
                        std::cout << "No sheet number " << aOptions.nSheetToExport
                            << ", number of sheets is " << nCount << std::endl;
                }
                nStartTab = 0;
                nCount = 0;
                SetError(SCERR_EXPORT_DATA);
                bRet = false;
            }

            INetURLObject aURLObject(rMed.GetURLObject());
            OUString sExt = aURLObject.CutExtension();
            OUString sBaseName = aURLObject.GetLastName();
            aURLObject.CutLastName();

            for (SCTAB i = nStartTab; i < nCount; ++i)
            {
                OUString sTabName;
                if (!m_pDocument->GetName(i, sTabName))
                    sTabName = OUString::number(i);
                INetURLObject aSheetURLObject(aURLObject);
                OUString sFileName = sBaseName + "-" + sTabName;
                if (!sExt.isEmpty())
                    sFileName = sFileName + "." + sExt;
                aSheetURLObject.Append(sFileName);

                // log similar to DispatchWatcher::executeDispatchRequests
                OUString aOutFile = aSheetURLObject.GetMainURL(INetURLObject::DecodeMechanism::NONE);
                if (bVerbose)
                {
                    OUString aDisplayedName;
                    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(aOutFile, aDisplayedName))
                        aDisplayedName = aOutFile;
                    std::cout << "Writing sheet " << OUStringToOString(sTabName, osl_getThreadTextEncoding()) << " -> "
                                                  << OUStringToOString(aDisplayedName, osl_getThreadTextEncoding())
                                                  << std::endl;

                    if (FStatHelper::IsDocument(aOutFile))
                        std::cout << "Overwriting: " << OUStringToOString(aDisplayedName, osl_getThreadTextEncoding())
                                                     << std::endl ;
                }

                std::unique_ptr<SvStream> xStm = ::utl::UcbStreamHelper::CreateStream(aOutFile, StreamMode::TRUNC | StreamMode::WRITE);
                if (!xStm)
                {
                    SetError(ERRCODE_IO_CANTCREATE);
                    bRet = false;
                    break;
                }
                AsciiSave(*xStm, aOptions, i);
            }
        }
        else
        {
            SvStream* pStream = rMed.GetOutStream();
            if (pStream)
            {
                AsciiSave(*pStream, aOptions, GetSaveTab());
                bRet = true;

                if (m_pDocument->GetTableCount() > 1)
                    if (!rMed.GetErrorIgnoreWarning())
                        rMed.SetError(SCWARN_EXPORT_ASCII);
            }
        }
    }
    else if (aFltName == pFilterDBase)
    {
        OUString sCharSet;
        if ( const SfxStringItem* pOptionsItem = rMed.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
        {
            sCharSet = pOptionsItem->GetValue();
        }

        if (sCharSet.isEmpty())
        {
            //  default for dBase export (from API without options):
            //  IBM_850 encoding

            sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
        }

        weld::WaitObject aWait( GetActiveDialogParent() );
        // Hack so that Sba can overwrite the opened TempFile.
        rMed.CloseOutStream();
        bool bHasMemo = false;

        ErrCodeMsg eError = DBaseExport(
            rMed.GetPhysicalName(), ScGlobal::GetCharsetValue(sCharSet), bHasMemo);

        INetURLObject aTmpFile( rMed.GetPhysicalName(), INetProtocol::File );
        if ( bHasMemo )
            aTmpFile.setExtension(u"dbt");
        if ( eError != ERRCODE_NONE && !eError.IsWarning() )
        {
            if (!GetErrorIgnoreWarning())
                SetError(eError);
            if ( bHasMemo && IsDocument( aTmpFile ) )
                KillFile( aTmpFile );
        }
        else
        {
            bRet = true;
            if ( bHasMemo )
            {
                const SfxStringItem* pNameItem = rMed.GetItemSet().GetItem<SfxStringItem>( SID_FILE_NAME );
                assert(pNameItem && "SID_FILE_NAME is required");
                INetURLObject aDbtFile( pNameItem->GetValue(), INetProtocol::File );
                aDbtFile.setExtension(u"dbt");

                // tdf#40713: don't lose dbt file
                // if aDbtFile corresponds exactly to aTmpFile, we just have to return
                if (aDbtFile.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) ==
                    aTmpFile.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ))
                {
                    if (eError != ERRCODE_NONE && !GetErrorIgnoreWarning())
                        SetError(eError);
                    return bRet;
                }

                if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
                    bRet = false;
                if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
                    bRet = false;
                if ( !bRet )
                {
                    KillFile( aTmpFile );
                    if (eError == ERRCODE_NONE || eError.IsWarning())
                        eError = SCERR_EXPORT_DATA;
                }
            }
            if (eError != ERRCODE_NONE && !GetErrorIgnoreWarning())
                SetError(eError);
        }
    }
    else if (aFltName == pFilterDif)
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            OUString sItStr;
            if ( const SfxStringItem* pOptionsItem = rMed.GetItemSet().GetItemIfSet( SID_FILE_FILTEROPTIONS ) )
            {
                sItStr = pOptionsItem->GetValue();
            }

            if (sItStr.isEmpty())
            {
                //  default for DIF export (from API without options):
                //  ISO8859-1/MS_1252 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
            }

            weld::WaitObject aWait( GetActiveDialogParent() );
            ScFormatFilter::Get().ScExportDif( *pStream, m_pDocument.get(), ScAddress(0,0,0),
                ScGlobal::GetCharsetValue(sItStr) );
            bRet = true;

            if (m_pDocument->GetTableCount() > 1)
                if (!rMed.GetErrorIgnoreWarning())
                    rMed.SetError(SCWARN_EXPORT_ASCII);
        }
    }
    else if (aFltName == pFilterSylk)
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            weld::WaitObject aWait( GetActiveDialogParent() );

            SCCOL nEndCol;
            SCROW nEndRow;
            m_pDocument->GetCellArea( 0, nEndCol, nEndRow );
            ScRange aRange( 0,0,0, nEndCol,nEndRow,0 );

            ScImportExport aImExport( *m_pDocument, aRange );
            aImExport.SetFormulas( true );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SotClipboardFormatId::SYLK );
        }
    }
    else if (aFltName == pFilterHtml)
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            OUString sFilterOptions;

            if (const SfxStringItem* pOptionsItem = rMed.GetItemSet().GetItemIfSet(SID_FILE_FILTEROPTIONS))
                sFilterOptions = pOptionsItem->GetValue();

            weld::WaitObject aWait(GetActiveDialogParent());
            ScImportExport aImExport(*m_pDocument);
            aImExport.SetStreamPath(rMed.GetName());
            aImExport.SetFilterOptions(sFilterOptions);
            bRet = aImExport.ExportStream(*pStream, rMed.GetBaseURL(true), SotClipboardFormatId::HTML);
            if (bRet && !aImExport.GetNonConvertibleChars().isEmpty())
            {
                SetError(ErrCodeMsg(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    DialogMask::ButtonsOk | DialogMask::MessageInfo));
            }
        }
    }
    else
    {
        if (GetErrorIgnoreWarning())
            SetError(SCERR_IMPORT_NI);
    }
    return bRet;
}

bool ScDocShell::DoSaveCompleted( SfxMedium * pNewStor, bool bRegisterRecent )
{
    bool bRet = SfxObjectShell::DoSaveCompleted( pNewStor, bRegisterRecent );

    //  SfxHintId::ScDocSaved for change ReadOnly -> Read/Write
    Broadcast( SfxHint( SfxHintId::ScDocSaved ) );
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
            aArgs = { uno::Any(nSlotId == SID_SAVEASDOC) };
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            nVbaEventId = VBAEventId::WORKBOOK_BEFOREPRINT;
        break;
    }

    bool bSlotExecutable = true;
    if( nVbaEventId != VBAEventId::NO_EVENT ) try
    {
        uno::Reference< XVBAEventProcessor > xEventProcessor( m_pDocument->GetVbaEventProcessor(), uno::UNO_SET_THROW );
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
    if ( m_pDocument->IsInLinkUpdate() || m_pDocument->IsInInterpreter() )
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
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( m_pDocument->GetVbaEventProcessor(), uno::UNO_SET_THROW );
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
        m_pDocument->EnableIdle(false); // Do not mess around with it anymore!

    return bRet;
}

OUString ScDocShell::GetOwnFilterName()
{
    return pFilterSc50;
}

OUString ScDocShell::GetHtmlFilterName()
{
    return pFilterHtml;
}

OUString ScDocShell::GetWebQueryFilterName()
{
    return pFilterHtmlWebQ;
}

OUString ScDocShell::GetAsciiFilterName()
{
    return SC_TEXT_CSV_FILTER_NAME;
}

OUString ScDocShell::GetLotusFilterName()
{
    return pFilterLotus;
}

OUString ScDocShell::GetDBaseFilterName()
{
    return pFilterDBase;
}

OUString ScDocShell::GetDifFilterName()
{
    return pFilterDif;
}

bool ScDocShell::HasAutomaticTableName( std::u16string_view rFilter )
{
    //  sal_True for those filters that keep the default table name
    //  (which is language specific)

    return rFilter == SC_TEXT_CSV_FILTER_NAME
        || rFilter == pFilterLotus
        || rFilter == pFilterExcel4
        || rFilter == pFilterEx4Temp
        || rFilter == pFilterDBase
        || rFilter == pFilterDif
        || rFilter == pFilterSylk
        || rFilter == pFilterHtml
        || rFilter == pFilterRtf;
}

std::unique_ptr<ScDocFunc> ScDocShell::CreateDocFunc()
{
    return std::make_unique<ScDocFuncDirect>( *this );
}

ScDocShell::ScDocShell( const SfxModelFlags i_nSfxCreationFlags, const std::shared_ptr<ScDocument>& pDoc ) :
    SfxObjectShell( i_nSfxCreationFlags ),
    m_pDocument       ( pDoc ? pDoc : std::make_shared<ScDocument>( SCDOCMODE_DOCUMENT, this )),
    m_aDdeTextFmt(OUString("TEXT")),
    m_nPrtToScreenFactor( 1.0 ),
    m_pImpl           ( new DocShell_Impl ),
    m_bHeaderOn       ( true ),
    m_bFooterOn       ( true ),
    m_bIsEmpty        ( true ),
    m_bIsInUndo       ( false ),
    m_bDocumentModifiedPending( false ),
    m_bUpdateEnabled  ( true ),
    m_bAreasChangedNeedBroadcast( false ),
    m_nDocumentLock   ( 0 ),
    m_nCanUpdate (css::document::UpdateDocMode::ACCORDING_TO_CONFIG)
{
    SetPool( &SC_MOD()->GetPool() );

    m_bIsInplace = (GetCreateMode() == SfxObjectCreateMode::EMBEDDED);
    //  Will be reset if not in place

    m_pDocFunc = CreateDocFunc();

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = m_pDocument->GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);

    m_pDocument->GetDBCollection()->SetRefreshHandler(
        LINK( this, ScDocShell, RefreshDBDataHdl ) );

    // InitItems and CalcOutputFactor are called now in Load/ConvertFrom/InitNew
}

ScDocShell::~ScDocShell()
{
    ResetDrawObjectShell(); // If the Drawing Layer still tries to access it, access it

    SfxStyleSheetPool* pStlPool = m_pDocument->GetStyleSheetPool();
    if (pStlPool)
        EndListening(*pStlPool);
    EndListening(*this);

    m_pAutoStyleList.reset();

    SfxApplication *pSfxApp = SfxGetpApp();
    if ( pSfxApp->GetDdeService() ) // Delete DDE for Document
        pSfxApp->RemoveDdeTopic( this );

    m_pDocFunc.reset();
    delete m_pDocument->mpUndoManager;
    m_pDocument->mpUndoManager = nullptr;
    m_pImpl.reset();

    m_pPaintLockData.reset();

    m_pSheetSaveData.reset();
    m_pFormatSaveData.reset();
    m_pOldAutoDBRange.reset();

    if (m_pModificator)
    {
        OSL_FAIL("The Modificator should not exist");
        m_pModificator.reset();
    }
}

SfxUndoManager* ScDocShell::GetUndoManager()
{
    return m_pDocument->GetUndoManager();
}

void ScDocShell::SetModified( bool bModified )
{
    if ( SfxObjectShell::IsEnableSetModified() )
    {
        SfxObjectShell::SetModified( bModified );
        Broadcast( SfxHint( SfxHintId::DocChanged ) );
    }
}

void ScDocShell::SetDocumentModified()
{
    //  BroadcastUno must also happen right away with pPaintLockData
    //  FIXME: Also for SetDrawModified, if Drawing is connected
    //  FIXME: Then own Hint?

    if ( m_pPaintLockData )
    {
        // #i115009# broadcast BCA_BRDCST_ALWAYS, so a component can read recalculated results
        // of RecalcModeAlways formulas (like OFFSET) after modifying cells
        m_pDocument->Broadcast(ScHint(SfxHintId::ScDataChanged, BCA_BRDCST_ALWAYS));
        m_pDocument->InvalidateTableArea();    // #i105279# needed here
        m_pDocument->BroadcastUno( SfxHint( SfxHintId::DataChanged ) );

        m_pPaintLockData->SetModified(); // Later on ...
        return;
    }

    SetDrawModified();

    if ( m_pDocument->IsAutoCalcShellDisabled() )
        SetDocumentModifiedPending( true );
    else
    {
        SetDocumentModifiedPending( false );
        m_pDocument->InvalidateStyleSheetUsage();
        m_pDocument->InvalidateTableArea();
        m_pDocument->InvalidateLastTableOpParams();
        m_pDocument->Broadcast(ScHint(SfxHintId::ScDataChanged, BCA_BRDCST_ALWAYS));
        if ( m_pDocument->IsForcedFormulaPending() && m_pDocument->GetAutoCalc() )
            m_pDocument->CalcFormulaTree( true );
        m_pDocument->RefreshDirtyTableColumnNames();
        PostDataChanged();

        //  Detective AutoUpdate:
        //  Update if formulas were modified (DetectiveDirty) or the list contains
        //  "Trace Error" entries (Trace Error can look completely different
        //  after changes to non-formula cells).

        ScDetOpList* pList = m_pDocument->GetDetOpList();
        if ( pList && ( m_pDocument->IsDetectiveDirty() || pList->HasAddError() ) &&
             pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
        {
            GetDocFunc().DetectiveRefresh(true);    // sal_True = caused by automatic update
        }
        m_pDocument->SetDetectiveDirty(false);         // always reset, also if not refreshed
    }

    if (m_bAreasChangedNeedBroadcast)
    {
        m_bAreasChangedNeedBroadcast = false;
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreasChanged));
    }

    // notify UNO objects after BCA_BRDCST_ALWAYS etc.
    m_pDocument->BroadcastUno( SfxHint( SfxHintId::DataChanged ) );
}

/**
 * SetDrawModified - without Formula update
 *
 * Drawing also needs to be updated for the normal SetDocumentModified
 * e.g.: when deleting tables etc.
 */
void ScDocShell::SetDrawModified()
{
    bool bUpdate = !IsModified();

    SetModified();

    SfxBindings* pBindings = GetViewBindings();
    if (bUpdate && pBindings)
    {
        pBindings->Invalidate( SID_SAVEDOC );
        pBindings->Invalidate( SID_DOC_MODIFIED );
    }

    if (pBindings)
    {
        // #i105960# Undo etc used to be volatile.
        // They always have to be invalidated, including drawing layer or row height changes
        // (but not while pPaintLockData is set).
        pBindings->Invalidate( SID_UNDO );
        pBindings->Invalidate( SID_REDO );
        pBindings->Invalidate( SID_REPEAT );
    }

    if ( m_pDocument->IsChartListenerCollectionNeedsUpdate() )
    {
        m_pDocument->UpdateChartListenerCollection();
        SfxGetpApp()->Broadcast(SfxHint( SfxHintId::ScDrawChanged ));    // Navigator
    }
    SC_MOD()->AnythingChanged();
}

void ScDocShell::SetInUndo(bool bSet)
{
    m_bIsInUndo = bSet;
}

void ScDocShell::GetDocStat( ScDocStat& rDocStat )
{
    SfxPrinter* pPrinter = GetPrinter();

    m_pDocument->GetDocStat( rDocStat );
    rDocStat.nPageCount = 0;

    if ( pPrinter )
        for ( SCTAB i=0; i<rDocStat.nTableCount; i++ )
            rDocStat.nPageCount = sal::static_int_cast<sal_uInt16>( rDocStat.nPageCount +
                static_cast<sal_uInt16>(ScPrintFunc( this, pPrinter, i ).GetTotalPages()) );
}

std::shared_ptr<SfxDocumentInfoDialog> ScDocShell::CreateDocumentInfoDialog(weld::Window* pParent, const SfxItemSet &rSet)
{
    std::shared_ptr<SfxDocumentInfoDialog> xDlg = std::make_shared<SfxDocumentInfoDialog>(pParent, rSet);
    ScDocShell*            pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );

    // Only for statistics, if this Doc is shown; not from the Doc Manager
    if( pDocSh == this )
    {
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        ::CreateTabPage ScDocStatPageCreate = pFact->GetTabPageCreatorFunc(SID_SC_TP_STAT);
        OSL_ENSURE(ScDocStatPageCreate, "Tabpage create fail!");
        xDlg->AddFontTabPage();
        xDlg->AddTabPage("calcstats", ScResId(STR_DOC_STAT), ScDocStatPageCreate);
    }
    return xDlg;
}

weld::Window* ScDocShell::GetActiveDialogParent()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh )
        return pViewSh->GetDialogParent();
    return Application::GetDefDialogParent();
}

ScSheetSaveData* ScDocShell::GetSheetSaveData()
{
    if (!m_pSheetSaveData)
        m_pSheetSaveData.reset( new ScSheetSaveData );

    return m_pSheetSaveData.get();
}

ScFormatSaveData* ScDocShell::GetFormatSaveData()
{
    if (!m_pFormatSaveData)
        m_pFormatSaveData.reset( new ScFormatSaveData );

    return m_pFormatSaveData.get();
}

namespace {

void removeKeysIfExists(const Reference<ui::XAcceleratorConfiguration>& xScAccel, const vector<const awt::KeyEvent*>& rKeys)
{
    for (const awt::KeyEvent* p : rKeys)
    {
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
    aKeys.reserve(9);

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

    // Ctrl-Space
    awt::KeyEvent aCtrlSpace;
    aCtrlSpace.KeyCode = awt::Key::SPACE;
    aCtrlSpace.Modifiers = awt::KeyModifier::MOD1;
    aKeys.push_back(&aCtrlSpace);

    // Ctrl-Shift-Space
    awt::KeyEvent aCtrlShiftSpace;
    aCtrlShiftSpace.KeyCode = awt::Key::SPACE;
    aCtrlShiftSpace.Modifiers = awt::KeyModifier::MOD1 | awt::KeyModifier::SHIFT;
    aKeys.push_back(&aCtrlShiftSpace);

    // F4
    awt::KeyEvent aF4;
    aF4.KeyCode = awt::Key::F4;
    aF4.Modifiers = 0;
    aKeys.push_back(&aF4);

    // CTRL+SHIFT+F4
    awt::KeyEvent aCtrlShiftF4;
    aCtrlShiftF4.KeyCode = awt::Key::F4;
    aCtrlShiftF4.Modifiers = awt::KeyModifier::MOD1 | awt::KeyModifier::SHIFT;
    aKeys.push_back(&aCtrlShiftF4);

    // SHIFT+F4
    awt::KeyEvent aShiftF4;
    aShiftF4.KeyCode = awt::Key::F4;
    aShiftF4.Modifiers = awt::KeyModifier::SHIFT;
    aKeys.push_back(&aShiftF4);

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
            xScAccel->setKeyEvent(aCtrlSpace, ".uno:SelectColumn");
            xScAccel->setKeyEvent(aCtrlShiftSpace, ".uno:SelectAll");
            xScAccel->setKeyEvent(aF4, ".uno:ToggleRelative");
            xScAccel->setKeyEvent(aCtrlShiftF4, ".uno:ViewDataSourceBrowser");
        break;
        case ScOptionsUtil::KEY_OOO_LEGACY:
            xScAccel->setKeyEvent(aDelete, ".uno:Delete");
            xScAccel->setKeyEvent(aBackspace, ".uno:ClearContents");
            xScAccel->setKeyEvent(aCtrlD, ".uno:DataSelect");
            xScAccel->setKeyEvent(aCtrlShiftSpace, ".uno:SelectColumn");
            xScAccel->setKeyEvent(aF4, ".uno:ViewDataSourceBrowser");
            xScAccel->setKeyEvent(aShiftF4, ".uno:ToggleRelative");
        break;
        default:
            ;
    }

    xScAccel->store();
}

void ScDocShell::UseSheetSaveEntries()
{
    if (!m_pSheetSaveData)
        return;

    m_pSheetSaveData->UseSaveEntries();   // use positions from saved file for next saving

    bool bHasEntries = false;
    SCTAB nTabCount = m_pDocument->GetTableCount();
    SCTAB nTab;
    for (nTab = 0; nTab < nTabCount; ++nTab)
        if (m_pSheetSaveData->HasStreamPos(nTab))
            bHasEntries = true;

    if (!bHasEntries)
    {
        // if no positions were set (for example, export to other format),
        // reset all "valid" flags
        for (nTab = 0; nTab < nTabCount; ++nTab)
            m_pDocument->SetStreamValid(nTab, false);
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

ScDocShellModificator::~ScDocShellModificator() COVERITY_NOEXCEPT_FALSE
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
    rDoc.PrepareFormulaCalc();
    if ( !rDoc.IsImportingXML() )
    {
        // temporarily restore AutoCalcShellDisabled
        bool bDisabled = rDoc.IsAutoCalcShellDisabled();
        rDoc.SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
        rDocShell.SetDocumentModified();
        rDoc.SetAutoCalcShellDisabled( bDisabled );
    }
    else
    {
        // uno broadcast is necessary for api to work
        // -> must also be done during xml import
        rDoc.BroadcastUno( SfxHint( SfxHintId::DataChanged ) );
    }
}

bool ScDocShell::IsChangeRecording() const
{
    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
    return pChangeTrack != nullptr;
}

bool ScDocShell::HasChangeRecordProtection() const
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
    if (pChangeTrack)
        bRes = pChangeTrack->IsProtected();
    return bRes;
}

void ScDocShell::SetChangeRecording( bool bActivate, bool /*bLockAllViews*/ )
{
    bool bOldChangeRecording = IsChangeRecording();

    if (bActivate)
    {
        m_pDocument->StartChangeTracking();
        ScChangeViewSettings aChangeViewSet;
        aChangeViewSet.SetShowChanges(true);
        m_pDocument->SetChangeViewSettings(aChangeViewSet);
    }
    else
    {
        m_pDocument->EndChangeTracking();
        PostPaintGridAll();
    }

    if (bOldChangeRecording != IsChangeRecording())
    {
        UpdateAcceptChangesDialog();
        // invalidate slots
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->InvalidateAll(false);
    }
}

void ScDocShell::SetProtectionPassword( const OUString &rNewPassword )
{
    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
    if (!pChangeTrack)
        return;

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

    if ( bProtected != pChangeTrack->IsProtected() )
    {
        UpdateAcceptChangesDialog();
        SetDocumentModified();
    }
}

bool ScDocShell::GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
    if (pChangeTrack && pChangeTrack->IsProtected())
    {
        rPasswordHash = pChangeTrack->GetProtection();
        bRes = true;
    }
    return bRes;
}

void ScDocShell::RegisterAutomationWorkbookObject(css::uno::Reference< ooo::vba::excel::XWorkbook > const& xWorkbook)
{
    mxAutomationWorkbookObject = xWorkbook;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportSLK(SvStream &rStream)
{
    ScDLL::Init();
    ScDocument aDocument;
    ScDocOptions aDocOpt = aDocument.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    aDocument.SetDocOptions(aDocOpt);
    aDocument.MakeTable(0);
    aDocument.EnableExecuteLink(false);
    aDocument.SetInsertingFromOtherDoc(true);
    aDocument.SetImportingXML(true);

    ScImportExport aImpEx(aDocument);
    return aImpEx.ImportStream(rStream, OUString(), SotClipboardFormatId::SYLK);
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportDBF(SvStream &rStream)
{
    ScDLL::Init();

    // we need a real file for this filter

    // put it in an empty dir
    utl::TempFileNamed aTmpDir(nullptr, true);
    aTmpDir.EnableKillingFile();
    OUString sTmpDir = aTmpDir.GetURL();

    utl::TempFileNamed aTempInput(u"", true, u".dbf", &sTmpDir);
    aTempInput.EnableKillingFile();

    SvStream* pInputStream = aTempInput.GetStream(StreamMode::WRITE);
    sal_uInt8 aBuffer[8192];
    while (auto nRead = rStream.ReadBytes(aBuffer, SAL_N_ELEMENTS(aBuffer)))
        pInputStream->WriteBytes(aBuffer, nRead);
    aTempInput.CloseStream();

    SfxMedium aMedium(aTempInput.GetURL(), StreamMode::STD_READWRITE);

    ScDocShellRef xDocShell = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT |
                                             SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
                                             SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    xDocShell->DoInitNew();

    ScDocument& rDoc = xDocShell->GetDocument();

    ScDocOptions aDocOpt = rDoc.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    rDoc.SetDocOptions(aDocOpt);
    rDoc.MakeTable(0);
    rDoc.EnableExecuteLink(false);
    rDoc.SetInsertingFromOtherDoc(true);

    ScDocRowHeightUpdater::TabRanges aRecalcRanges(0, rDoc.MaxRow());
    std::map<SCCOL, ScColWidthParam> aColWidthParam;
    ErrCode eError = xDocShell->DBaseImport(aMedium.GetPhysicalName(), RTL_TEXTENCODING_IBM_850, aColWidthParam, aRecalcRanges.maRanges);

    xDocShell->DoClose();
    xDocShell.clear();

    return eError == ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
