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

#include "scitems.hxx"
#include <editeng/justifyitem.hxx>
#include <sot/clsids.hxx>
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
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
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
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include "scabstdlg.hxx"
#include <sot/formats.hxx>

#include "cell.hxx"
#include "column.hxx"
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
#include "docpool.hxx"      // LoadCompleted
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

#include "docsh.hxx"
#include "docshimp.hxx"
#include "sizedev.hxx"
#include <rtl/logfile.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include "uiitems.hxx"
#include "cellsuno.hxx"
#include "dpobject.hxx"
#include "markdata.hxx"
#include "orcusfilters.hxx"

#ifdef ENABLE_TELEPATHY
#include "sccollaboration.hxx"
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

#define SC_LIBO_PROD_NAME "LibreOffice"

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::boost::shared_ptr;
using ::std::vector;

// STATIC DATA -----------------------------------------------------------

//  Stream-Namen im Storage

const sal_Char ScDocShell::pStarCalcDoc[] = STRING_SCSTREAM;        // "StarCalcDocument"
const sal_Char ScDocShell::pStyleName[] = "SfxStyleSheets";

//  Filter-Namen (wie in sclib.cxx)

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

//----------------------------------------------------------------------

#define ScDocShell
#include "scslots.hxx"


SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell, ScResId(SCSTR_DOCSHELL))
{
}

//  GlobalName der aktuellen Version:
SFX_IMPL_OBJECTFACTORY( ScDocShell, SvGlobalName(SO3_SC_CLASSID), SFXOBJECTSHELL_STD_NORMAL, "scalc" )

TYPEINIT1( ScDocShell, SfxObjectShell );        // SfxInPlaceObject: kein Type-Info ?

//------------------------------------------------------------------

void ScDocShell::FillClass( SvGlobalName* pClassName,
                                        sal_uInt32* pFormat,
                                        String* /* pAppName */,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
    if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = SOT_FORMATSTR_ID_STARCALC_60;
        *pFullTypeName  = String( ScResId( SCSTR_LONG_SCDOC_NAME ) );
        *pShortTypeName = String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
    }
    else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
    {
        *pClassName     = SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat        = bTemplate ? SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE : SOT_FORMATSTR_ID_STARCALC_8;
        *pFullTypeName  = rtl::OUString("calc8");
        *pShortTypeName = ScResId(SCSTR_SHORT_SCDOC_NAME).toString();
    }
    else
    {
        OSL_FAIL("wat fuer ne Version?");
    }
}

//------------------------------------------------------------------

void ScDocShell::DoEnterHandler()
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        if (pViewSh->GetViewData()->GetDocShell() == this)
            SC_MOD()->InputEnterHandler();
}

//------------------------------------------------------------------

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
    // get global state like HIDDENINFORMATION_DOCUMENTVERSIONS
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
            if (!aDocument.GetNotes(nTab)->empty())
                bFound = true;
        }

        if (bFound)
            nState |= HIDDENINFORMATION_NOTES;
    }

    return nState;
}

void ScDocShell::BeforeXMLLoading()
{
    aDocument.DisableIdle( sal_True );

    // prevent unnecessary broadcasts and updates
    OSL_ENSURE(pModificator == NULL, "The Modificator should not exist");
    pModificator = new ScDocShellModificator( *this );

    aDocument.SetImportingXML( sal_True );
    aDocument.EnableExecuteLink( false );   // #i101304# to be safe, prevent nested loading from external references
    aDocument.EnableUndo( false );
    // prevent unnecessary broadcasts and "half way listeners"
    aDocument.SetInsertingFromOtherDoc( sal_True );

    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        ScColumn::bDoubleAlloc = sal_True;
}

void ScDocShell::AfterXMLLoading(sal_Bool bRet)
{
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
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
                    rtl::OUString aName;
                    aDocument.GetName(i, aName);
                    rtl::OUString aLinkTabName = aDocument.GetLinkTab(i);
                    sal_Int32 nLinkTabNameLength = aLinkTabName.getLength();
                    sal_Int32 nNameLength = aName.getLength();
                    if (nLinkTabNameLength < nNameLength)
                    {

                        // remove the quottes on begin and end of the docname and restore the escaped quotes
                        const sal_Unicode* pNameBuffer = aName.getStr();
                        if ( *pNameBuffer == '\'' && // all docnames have to have a ' character on the first pos
                            ScGlobal::UnicodeStrChr( pNameBuffer, SC_COMPILER_FILE_TAB_SEP ) )
                        {
                            rtl::OUStringBuffer aDocURLBuffer;
                            sal_Bool bQuote = sal_True;         // Dokumentenname ist immer quoted
                            ++pNameBuffer;
                            while ( bQuote && *pNameBuffer )
                            {
                                if ( *pNameBuffer == '\'' && *(pNameBuffer-1) != '\\' )
                                    bQuote = false;
                                else if( !(*pNameBuffer == '\\' && *(pNameBuffer+1) == '\'') )
                                    aDocURLBuffer.append(*pNameBuffer);     // falls escaped Quote: nur Quote in den Namen
                                ++pNameBuffer;
                            }


                            if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  // after the last quote of the docname should be the # char
                            {
                                xub_StrLen nIndex = nNameLength - nLinkTabNameLength;
                                INetURLObject aINetURLObject(aDocURLBuffer.makeStringAndClear());
                                if( String(aName).Equals(String(aLinkTabName), nIndex, nLinkTabNameLength) &&
                                    (aName[nIndex - 1] == '#') && // before the table name should be the # char
                                    !aINetURLObject.HasError()) // the docname should be a valid URL
                                {
                                    aName = ScGlobal::GetDocTabName( aDocument.GetLinkDoc( i ), aDocument.GetLinkTab( i ) );
                                    aDocument.RenameTab(i, aName, sal_True, sal_True);
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
                    ScDPObject* pDPObj = (*pDPCollection)[nDP];
                    if (pDPObj->GetName().isEmpty())
                        pDPObj->SetName( pDPCollection->CreateNewName() );
                }
            }
        }
        ScColumn::bDoubleAlloc = false;
    }
    else
        aDocument.SetInsertingFromOtherDoc( false );

    aDocument.SetImportingXML( false );
    aDocument.EnableExecuteLink( true );
    aDocument.EnableUndo( sal_True );
    bIsEmpty = false;

    if (pModificator)
    {
        bool bRecalcState = aDocument.GetHardRecalcState();
        //temporarily set hard-recalc to prevent calling ScFormulaCell::Notify()
        //which will set the cells dirty.
        aDocument.SetHardRecalcState(true);
        delete pModificator;
        aDocument.SetHardRecalcState(bRecalcState);
        pModificator = NULL;
    }
    else
    {
        OSL_FAIL("The Modificator should exist");
    }

    aDocument.DisableIdle( false );
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

}

sal_Bool ScDocShell::LoadXML( SfxMedium* pLoadMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    LoadMediumGuard aLoadGuard(&aDocument);

    //  MacroCallMode is no longer needed, state is kept in SfxObjectShell now

    // no Seek(0) here - always loading from storage, GetInStream must not be called

    BeforeXMLLoading();

    // #i62677# BeforeXMLLoading is also called from ScXMLImport::startDocument when invoked
    // from an external component. The XMLFromWrapper flag is only set here, when called
    // through ScDocShell.
    aDocument.SetXMLFromWrapper( sal_True );

    ScXMLImportWrapper aImport( aDocument, pLoadMedium, xStor );

    sal_Bool bRet(false);
    ErrCode nError = ERRCODE_NONE;
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Import(false, nError);
    else
        bRet = aImport.Import(sal_True, nError);

    if ( nError )
        pLoadMedium->SetError( nError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    //if the document was not generated by LibreOffice, do hard recalc in case some other document
    //generator saved cached formula results that differ from LibreOffice's calculated results or
    //did not use cached formula results.
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();
    rtl::OUString sGenerator(xDocProps->getGenerator());
    if(sGenerator.indexOf(SC_LIBO_PROD_NAME) == -1)
        DoHardRecalc(false);
    else //still need to recalc volatile formula cells
        aDocument.CalcFormulaTree(false, false, false);

    aDocument.EnableAdjustHeight(false);

    aDocument.SetXMLFromWrapper( false );
    AfterXMLLoading(bRet);

    return bRet;
}

sal_Bool ScDocShell::SaveXML( SfxMedium* pSaveMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::SaveXML" );

    aDocument.DisableIdle( sal_True );

    ScXMLImportWrapper aImport( aDocument, pSaveMedium, xStor );
    sal_Bool bRet(false);
    if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Export(false);
    else
        bRet = aImport.Export(sal_True);

    aDocument.DisableIdle( false );

    return bRet;
}

sal_Bool ScDocShell::Load( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Load" );
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    //  only the latin script language is loaded
    //  -> initialize the others from options (before loading)
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
            //  prepare a valid document for XML filter
            //  (for ConvertFrom, InitNew is called before)
            aDocument.MakeTable(0);
            aDocument.GetStyleSheetPool()->CreateStandardStyles();
            aDocument.UpdStlShtPtrsFrmNms();

            bRet = LoadXML( &rMedium, NULL );
        }
    }

    if (!bRet && !rMedium.GetError())
        rMedium.SetError( SVSTREAM_FILEFORMAT_ERROR, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    if (rMedium.GetError())
        SetError( rMedium.GetError(), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    InitItems();
    CalcOutputFactor();

    // invalidate eventually temporary table areas
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = false;
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
    return bRet;
}

void ScDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = aDocument.GetVbaEventProcessor();
    if ( xVbaEvents.is() ) try
    {
        using namespace ::com::sun::star::script::vba::VBAEventId;
        if (rHint.ISA(ScTablesHint) )
        {
            const ScTablesHint& rScHint = static_cast< const ScTablesHint& >( rHint );
            if (rScHint.GetId() == SC_TAB_INSERTED)
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= rScHint.GetTab1();
                xVbaEvents->processVbaEvent( WORKBOOK_NEWSHEET, aArgs );
            }
            else if (rScHint.GetId() == SC_TABS_INSERTED)
            {
                for (SCTAB i = 0; i < rScHint.GetTab2(); ++i)
                {
                    uno::Sequence< uno::Any > aArgs( 1 );
                    aArgs[0] <<= static_cast<sal_Int16>(rScHint.GetTab1() + i);
                    xVbaEvents->processVbaEvent( WORKBOOK_NEWSHEET, aArgs );
                }
            }
        }
        else if ( rHint.ISA( SfxEventHint ) )
        {
            sal_uLong nEventId = static_cast< const SfxEventHint& >( rHint ).GetEventId();
            switch ( nEventId )
            {
                case SFX_EVENT_ACTIVATEDOC:
                {
                    uno::Sequence< uno::Any > aArgs;
                    xVbaEvents->processVbaEvent( WORKBOOK_ACTIVATE, aArgs );
                }
                break;
                case SFX_EVENT_DEACTIVATEDOC:
                {
                    uno::Sequence< uno::Any > aArgs;
                    xVbaEvents->processVbaEvent( WORKBOOK_DEACTIVATE, aArgs );
                }
                break;
                case SFX_EVENT_OPENDOC:
                {
                    uno::Sequence< uno::Any > aArgs;
                    xVbaEvents->processVbaEvent( WORKBOOK_OPEN, aArgs );
                }
                break;
                case SFX_EVENT_SAVEDOCDONE:
                case SFX_EVENT_SAVEASDOCDONE:
                case SFX_EVENT_SAVETODOCDONE:
                {
                    uno::Sequence< uno::Any > aArgs( 1 );
                    aArgs[ 0 ] <<= true;
                    xVbaEvents->processVbaEvent( WORKBOOK_AFTERSAVE, aArgs );
                }
                break;
                case SFX_EVENT_SAVEASDOCFAILED:
                case SFX_EVENT_SAVEDOCFAILED:
                case SFX_EVENT_SAVETODOCFAILED:
                {
                    uno::Sequence< uno::Any > aArgs( 1 );
                    aArgs[ 0 ] <<= false;
                    xVbaEvents->processVbaEvent( WORKBOOK_AFTERSAVE, aArgs );
                }
                break;
                case SFX_EVENT_CLOSEDOC:
                {
                    // #163655# prevent event processing after model is disposed
                    aDocument.SetVbaEventProcessor( uno::Reference< script::vba::XVBAEventProcessor >() );
                    uno::Reference< lang::XEventListener >( xVbaEvents, uno::UNO_QUERY_THROW )->disposing( lang::EventObject() );
                }
                break;
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    if (rHint.ISA(SfxSimpleHint))                               // ohne Parameter
    {
        sal_uLong nSlot = ((const SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case SFX_HINT_TITLECHANGED:
                aDocument.SetName( SfxShell::GetName() );
                //  RegisterNewTargetNames gibts nicht mehr
                SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED )); // Navigator
                break;
        }
    }
    else if (rHint.ISA(SfxStyleSheetHint))                      // Vorlagen geaendert
        NotifyStyle((const SfxStyleSheetHint&) rHint);
    else if (rHint.ISA(ScAutoStyleHint))
    {
        //! direct call for AutoStyles

        //  this is called synchronously from ScInterpreter::ScStyle,
        //  modifying the document must be asynchronous
        //  (handled by AddInitial)

        ScAutoStyleHint& rStlHint = (ScAutoStyleHint&)rHint;
        ScRange aRange = rStlHint.GetRange();
        String aName1 = rStlHint.GetStyle1();
        String aName2 = rStlHint.GetStyle2();
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
                    // the readonly documents should not be opened in shared mode
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
                            // switching to shared mode has failed, the document should be opened readonly
                            // TODO/LATER: And error message should be shown here probably
                            SetReadOnlyUI( sal_True );
                        }
                    }
                }
                break;
            case SFX_EVENT_VIEWCREATED:
                {
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

                    try
                    {
                        uno::Reference< uno::XComponentContext > xContext(
                            comphelper::getProcessComponentContext() );
                        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
                            xContext->getServiceManager(),
                            uno::UNO_QUERY_THROW );
                        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xServiceManager, uno::UNO_QUERY_THROW );
                        uno::Reference< container::XEnumeration> xEnum = xEnumAccess->createContentEnumeration(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocumentJob" ) ) );
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
                                    aArgsForJob[0] = beans::NamedValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "SpreadsheetView" )),
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
                                ScDocShell* pSharedDocShell = ( pDocObj ? dynamic_cast< ScDocShell* >( pDocObj->GetObjectShell() ) : NULL );
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
                                        xCloseable->close( sal_True );

                                        String aUserName( ScGlobal::GetRscString( STR_UNKNOWN_USER ) );
                                        bool bNoLockAccess = false;
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            uno::Sequence< ::rtl::OUString > aData = aLockFile.GetLockData();
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
                                            // TODO/LATER: in future an error regarding impossibility to open file for writing could be shown
                                            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                        }
                                        else
                                        {
                                            String aMessage( ScGlobal::GetRscString( STR_FILE_LOCKED_SAVE_LATER ) );
                                            aMessage.SearchAndReplaceAscii( "%1", aUserName );

                                            WarningBox aBox( GetActiveDialogParent(), WinBits( WB_RETRY_CANCEL | WB_DEF_RETRY ), aMessage );
                                            if ( aBox.Execute() == RET_RETRY )
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
                                        xCloseable->close( sal_True );

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
                                            aValues[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName"));
                                            aValues[0].Value <<= ::rtl::OUString( GetMedium()->GetFilter()->GetFilterName() );

                                            SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, false);
                                            if ( pPasswordItem && pPasswordItem->GetValue().Len() )
                                            {
                                                aValues.realloc( 2 );
                                                aValues[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Password") );
                                                aValues[1].Value <<= ::rtl::OUString( pPasswordItem->GetValue() );
                                            }

                                            SC_MOD()->SetInSharedDocSaving( true );
                                            xStor->storeToURL( GetSharedFileURL(), aValues );
                                            SC_MOD()->SetInSharedDocSaving( false );

                                            if ( bChangedViewSettings )
                                            {
                                                pChangeViewSet->SetShowChanges( sal_True );
                                                pChangeViewSet->SetShowAccepted( sal_True );
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
                                        // TODO/LATER: in future an error regarding impossibility to write to share control file could be shown
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
                            SetError( ERRCODE_IO_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) ); // this error code will produce no error message, but will break the further saving process
                    }


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
                            SetError( ERRCODE_IO_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) ); // this error code will produce no error message, but will break the further saving process
                        }
                    }
                } // fall through
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
                } // fall through
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

    // Inhalte fuer Organizer laden


sal_Bool ScDocShell::LoadFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::LoadFrom" );
    LoadMediumGuard aLoadGuard(&aDocument);
    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    WaitObject aWait( GetActiveDialogParent() );

    sal_Bool bRet = false;

    if (GetMedium())
    {
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
        nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
    }

    //  until loading/saving only the styles in XML is implemented,
    //  load the whole file
    bRet = LoadXML( &rMedium, NULL );
    InitItems();

    SfxObjectShell::LoadFrom( rMedium );

    return bRet;
}

static void lcl_parseHtmlFilterOption(const OUString& rOption, LanguageType& rLang, bool& rDateConvert)
{
    OUStringBuffer aBuf;
    OUString aTokens[2];
    sal_Int32 n = rOption.getLength();
    const sal_Unicode* p = rOption.getStr();
    sal_Int32 nTokenId = 0;
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const sal_Unicode c = p[i];
        if (c == sal_Unicode(' '))
        {
            if (aBuf.getLength())
                aTokens[nTokenId++] = aBuf.makeStringAndClear();
        }
        else
            aBuf.append(c);

        if (nTokenId >= 2)
            break;
    }

    if (aBuf.getLength())
        aTokens[nTokenId] = aBuf.makeStringAndClear();

    rLang = static_cast<LanguageType>(aTokens[0].toInt32());
    rDateConvert = static_cast<bool>(aTokens[1].toInt32());
}

sal_Bool ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertFrom" );

    LoadMediumGuard aLoadGuard(&aDocument);

    sal_Bool bRet = false;              // sal_False heisst Benutzerabbruch !!
                                    // bei Fehler: Fehler am Stream setzen!!

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    GetUndoManager()->Clear();

    // ob nach dem Import optimale Spaltenbreiten gesetzt werden sollen
    sal_Bool bSetColWidths = false;
    sal_Bool bSetSimpleTextColWidths = false;
    ScColWidthParam aColWidthParam[MAXCOLCOUNT];
    ScRange aColWidthRange;
    // ob nach dem Import optimale Zeilenhoehen gesetzt werden sollen
    sal_Bool bSetRowHeights = false;

    vector<ScDocRowHeightUpdater::TabRanges> aRecalcRowRangesArray;

    //  Alle Filter brauchen die komplette Datei am Stueck (nicht asynchron),
    //  darum vorher per CreateFileStream dafuer sorgen, dass die komplette
    //  Datei uebertragen wird.
    rMedium.GetPhysicalName();  //! CreateFileStream direkt rufen, wenn verfuegbar

    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, false);
    nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;

    const SfxFilter* pFilter = rMedium.GetFilter();
    if (pFilter)
    {
        String aFltName = pFilter->GetFilterName();

        sal_Bool bCalc3 = ( aFltName.EqualsAscii(pFilterSc30) );
        sal_Bool bCalc4 = ( aFltName.EqualsAscii(pFilterSc40) );
        if (!bCalc3 && !bCalc4)
            aDocument.SetInsertingFromOtherDoc( sal_True );

        if (aFltName.EqualsAscii(pFilterXML))
            bRet = LoadXML( &rMedium, NULL );
        else if (aFltName.EqualsAscii(pFilterSc10))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError = ScFormatFilter::Get().ScImportStarCalc10( *pStream, &aDocument );
                if (eError != eERR_OK)
                {
                    if (!GetError())
                        SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
                }
                else
                    bRet = sal_True;
            }
        }
        else if (aFltName.EqualsAscii(pFilterLotus))
        {
            String sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.Len() == 0)
            {
                //  default for lotus import (from API without options):
                //  IBM_437 encoding
                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
            }

            ScColumn::bDoubleAlloc = sal_True;
            FltError eError = ScFormatFilter::Get().ScImportLotus123( rMedium, &aDocument,
                                                ScGlobal::GetCharsetValue(sItStr));
            ScColumn::bDoubleAlloc = false;
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = sal_True;
            }
            else
                bRet = sal_True;
            bSetColWidths = sal_True;
            bSetRowHeights = sal_True;
        }
        else if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterExcel5) ||
                   aFltName.EqualsAscii(pFilterExcel95) || aFltName.EqualsAscii(pFilterExcel97) ||
                   aFltName.EqualsAscii(pFilterEx4Temp) || aFltName.EqualsAscii(pFilterEx5Temp) ||
                   aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp) )
        {
            EXCIMPFORMAT eFormat = EIF_AUTO;
            if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterEx4Temp) )
                eFormat = EIF_BIFF_LE4;
            else if ( aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
                      aFltName.EqualsAscii(pFilterEx5Temp) || aFltName.EqualsAscii(pFilterEx95Temp) )
                eFormat = EIF_BIFF5;
            else if ( aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx97Temp) )
                eFormat = EIF_BIFF8;

            MakeDrawLayer();                //! im Filter
            CalcOutputFactor();             // prepare update of row height
            ScColumn::bDoubleAlloc = true;
            FltError eError = ScFormatFilter::Get().ScImportExcel( rMedium, &aDocument, eFormat );
            ScColumn::bDoubleAlloc = false;
            aDocument.UpdateFontCharSet();
            if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
                aDocument.UpdateChartListenerCollection();              //! fuer alle Importe?

            // all graphics objects must have names
            aDocument.EnsureGraphicNames();

            if (eError == SCWARN_IMPORT_RANGE_OVERFLOW)
            {
                if (!GetError())
                    SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
                bRet = sal_True;
            }
            else if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
            }
            else
                bRet = true;
        }
        else if (aFltName.EqualsAscii(pFilterAscii))
        {
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            ScAsciiOptions aOptions;
            sal_Bool bOptInit = false;

            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            {
                aOptions.ReadFromString( ((const SfxStringItem*)pItem)->GetValue() );
                bOptInit = sal_True;
            }

            if ( !bOptInit )
            {
                //  default for ascii import (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

                aOptions.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                aOptions.SetFieldSeps( rtl::OUString(',') );
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
                    SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
            }
            else if (!GetError() && (bOverflowRow || bOverflowCol || bOverflowCell))
            {
                // precedence: row, column, cell
                FltError nWarn = (bOverflowRow ? SCWARN_IMPORT_ROW_OVERFLOW :
                        (bOverflowCol ? SCWARN_IMPORT_COLUMN_OVERFLOW :
                         SCWARN_IMPORT_CELL_OVERFLOW));
                SetError( nWarn, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
            }
            bSetColWidths = sal_True;
            bSetSimpleTextColWidths = sal_True;
        }
        else if (aFltName.EqualsAscii(pFilterDBase))
        {
            String sItStr;
            SfxItemSet*  pSet = rMedium.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.Len() == 0)
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
                    SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
                bRet = ( eError == SCWARN_IMPORT_RANGE_OVERFLOW );
            }
            else
                bRet = sal_True;

            aColWidthRange.aStart.SetRow( 1 );  // Spaltenheader nicht
            bSetColWidths = true;
            bSetSimpleTextColWidths = true;
        }
        else if (aFltName.EqualsAscii(pFilterDif))
        {
            SvStream* pStream = rMedium.GetInStream();
            if (pStream)
            {
                FltError eError;
                String sItStr;
                SfxItemSet*  pSet = rMedium.GetItemSet();
                const SfxPoolItem* pItem;
                if ( pSet && SFX_ITEM_SET ==
                     pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
                {
                    sItStr = ((const SfxStringItem*)pItem)->GetValue();
                }

                if (sItStr.Len() == 0)
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
                        SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

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
        else if (aFltName.EqualsAscii(pFilterSylk))
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
                SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
            bSetColWidths = sal_True;
            bSetSimpleTextColWidths = sal_True;
            bSetRowHeights = sal_True;
        }
        else if (aFltName.EqualsAscii(pFilterQPro6))
        {
            ScColumn::bDoubleAlloc = sal_True;
            FltError eError = ScFormatFilter::Get().ScImportQuattroPro( rMedium, &aDocument);
            ScColumn::bDoubleAlloc = false;
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError( eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = sal_True;
            }
            else
                bRet = sal_True;
            // TODO: Filter should set column widths. Not doing it here, it may
            // result in very narrow or wide columns, depending on content.
            // Setting row heights makes cells with font size attribution or
            // wrapping enabled look nicer..
            bSetRowHeights = sal_True;
        }
        else if (aFltName.EqualsAscii(pFilterRtf))
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
                            SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

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
                SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
        }
        else if (aFltName.EqualsAscii(pFilterHtml) || aFltName.EqualsAscii(pFilterHtmlWebQ))
        {
            FltError eError = SCERR_IMPORT_UNKNOWN;
            sal_Bool bWebQuery = aFltName.EqualsAscii(pFilterHtmlWebQ);
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
                         pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
                    {
                        String aFilterOption = (static_cast<const SfxStringItem*>(pItem))->GetValue();
                        lcl_parseHtmlFilterOption(aFilterOption, eLang, bDateConvert);
                    }

                    pInStream->Seek( 0 );
                    ScRange aRange;
                    // HTML macht eigenes ColWidth/RowHeight
                    CalcOutputFactor();
                    SvNumberFormatter aNumFormatter(aDocument.GetServiceManager(), eLang);
                    eError = ScFormatFilter::Get().ScImportHTML( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange,
                                            GetOutputFactor(), !bWebQuery, &aNumFormatter, bDateConvert );
                    if (eError != eERR_OK)
                    {
                        if (!GetError())
                            SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

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
                SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
        }
        else
        {
            if (!GetError())
                SetError(SCERR_IMPORT_NI, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
        }

        if (!bCalc3)
            aDocument.SetInsertingFromOtherDoc( false );
    }
    else
    {
        OSL_FAIL("Kein Filter bei ConvertFrom");
    }

    InitItems();
    CalcOutputFactor();
    if ( bRet && (bSetColWidths || bSetRowHeights) )
    {   // Spaltenbreiten/Zeilenhoehen anpassen, Basis 100% Zoom
        Fraction aZoom( 1, 1 );
        double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
            / GetOutputFactor();    // Faktor ist Drucker zu Bildschirm
        double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
        VirtualDevice aVirtDev;
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
            // Reihenfolge erst Breite dann Hoehe ist wichtig (vergl. hund.rtf)
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
            // Update all rows in all tables.
            ScSizeDeviceProvider aProv(this);
            ScDocRowHeightUpdater aUpdater(aDocument, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), NULL);
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
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );


    // invalidate eventually temporary table areas
    if ( bRet )
        aDocument.InvalidateTableArea();

    bIsEmpty = false;

    return bRet;
}

bool ScDocShell::LoadExternal(SfxMedium& rMed, const OUString& rProvider)
{
    if (rProvider == "orcus")
    {
        ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
        if (!pOrcus)
            return false;

        if (!pOrcus->importCSV(aDocument, rMed.GetName()))
            return false;

        FinishedLoading(SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES);
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
    if (mrDocShell.GetCreateMode()== SFX_CREATE_MODE_STANDARD)
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


sal_Bool ScDocShell::Save()
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Save" );

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

    //  wait cursor is handled with progress bar
    sal_Bool bRet = SfxObjectShell::Save();
    if( bRet )
        bRet = SaveXML( GetMedium(), NULL );
    return bRet;
}


sal_Bool ScDocShell::SaveAs( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::SaveAs" );

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

    //  wait cursor is handled with progress bar
    sal_Bool bRet = SfxObjectShell::SaveAs( rMedium );
    if( bRet )
        bRet = SaveXML( &rMedium, NULL );

    return bRet;
}


sal_Bool ScDocShell::IsInformationLost()
{
    //!!! bei Gelegenheit ein korrekte eigene Behandlung einbauen

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


void lcl_ScDocShell_GetFixedWidthString( rtl::OUString& rStr, const ScDocument& rDoc,
        SCTAB nTab, SCCOL nCol, sal_Bool bValue, SvxCellHorJustify eHorJust )
{
    rtl::OUString aString = rStr;
    sal_Int32 nLen = lcl_ScDocShell_GetColWidthInChars(
            rDoc.GetColWidth( nCol, nTab ) );
    //If the text won't fit in the column
    if ( nLen < aString.getLength() )
    {
        rtl::OUStringBuffer aReplacement;
        if (bValue)
            aReplacement.appendAscii(RTL_CONSTASCII_STRINGPARAM("###"));
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
                rtl::OUStringBuffer aTmp;
                aTmp = comphelper::string::padToLength( aTmp, nBlanks, ' ' );
                aString = aTmp.append(aString).makeStringAndClear();
            }
            break;
            case SVX_HOR_JUSTIFY_CENTER:
            {
                sal_Int32 nLeftPad = nBlanks / 2;
                rtl::OUStringBuffer aTmp;
                comphelper::string::padToLength( aTmp, nLeftPad, ' ' );
                aTmp.append(aString);
                comphelper::string::padToLength( aTmp, nLen, ' ' );
                aString = aTmp.makeStringAndClear();
            }
            break;
            default:
            {
                rtl::OUStringBuffer aTmp(aString);
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
    rtl::OUString aString;
    lcl_ScDocShell_GetFixedWidthString( aString, rDoc, nTab, nCol, false,
            SVX_HOR_JUSTIFY_STANDARD );
    rStream.WriteUnicodeOrByteText( aString );
}

template<typename StrT, typename SepCharT>
sal_Int32 getTextSepPos(
    const StrT& rStr, const ScImportOptions& rAsciiOpt, const SepCharT& rTextSep, const SepCharT& rFieldSep, bool& rNeedQuotes)
{
    // #i116636# quotes are needed if text delimiter (quote), field delimiter,
    // or LF is in the cell text.
    sal_Int32 nPos = rStr.indexOf(rTextSep);
    rNeedQuotes = rAsciiOpt.bQuoteAllText || (nPos >= 0) ||
        (rStr.indexOf(rFieldSep) >= 0) ||
        (rStr.indexOf(sal_Unicode(_LF)) >= 0);
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
    CharSet eCharSet      = rAsciiOpt.eCharSet;
    bool bFixedWidth      = rAsciiOpt.bFixedWidth;
    bool bSaveAsShown     = rAsciiOpt.bSaveAsShown;
    bool bShowFormulas    = rAsciiOpt.bSaveFormulas;

    CharSet eOldCharSet = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eCharSet );
    sal_uInt16 nOldNumberFormatInt = rStream.GetNumberFormatInt();
    rtl::OString aStrDelimEncoded;    // only used if not Unicode
    rtl::OUString aStrDelimDecoded;     // only used if context encoding
    rtl::OString aDelimEncoded;
    rtl::OUString aDelimDecoded;
    bool bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rStream.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = false;
    }
    else
    {
        aStrDelimEncoded = rtl::OString(&cStrDelim, 1, eCharSet);
        aDelimEncoded = rtl::OString(&cDelim, 1, eCharSet);
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
            {
                aStrDelimDecoded = rtl::OStringToOUString(aStrDelimEncoded, eCharSet);
                aDelimDecoded = rtl::OStringToOUString(aDelimEncoded, eCharSet);
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

    rtl::OUString aString;

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
    ScBaseCell* pCell;
    while ( ( pCell = aIter.GetNext( nCol, nRow ) ) != NULL )
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

        CellType eType = pCell->GetCellType();
        if ( bTabProtect )
        {
            const ScProtectionAttr* pProtAttr =
                (const ScProtectionAttr*) aDocument.GetAttr(
                                                            nCol, nRow, nTab, ATTR_PROTECTION );
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;  // hide
        }
        bool bString;
        switch ( eType )
        {
            case CELLTYPE_NOTE:
            case CELLTYPE_NONE:
                aString = rtl::OUString();
                bString = false;
                break;
            case CELLTYPE_FORMULA :
                {
                    sal_uInt16 nErrCode;
                    if ( bShowFormulas )
                    {
                        ((ScFormulaCell*)pCell)->GetFormula( aString );
                        bString = true;
                    }
                    else if ( ( nErrCode = ((ScFormulaCell*)pCell)->GetErrCode() ) != 0 )
                    {
                        aString = ScGlobal::GetErrorString( nErrCode );
                        bString = true;
                    }
                    else if ( ((ScFormulaCell*)pCell)->IsValue() )
                    {
                        sal_uInt32 nFormat;
                        aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                        if ( bFixedWidth || bSaveAsShown )
                        {
                            Color* pDummy;
                            ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                            bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                        }
                        else
                        {
                            ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                            bString = false;
                        }
                    }
                    else
                    {
                        if ( bSaveAsShown )
                        {
                            sal_uInt32 nFormat;
                            aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                            Color* pDummy;
                            ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                        }
                        else
                            aString = ((ScFormulaCell*)pCell)->GetString();
                        bString = true;
                    }
                }
                break;
            case CELLTYPE_STRING :
                if ( bSaveAsShown )
                {
                    sal_uInt32 nFormat;
                    aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                    Color* pDummy;
                    ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                }
                else
                    aString = ((ScStringCell*)pCell)->GetString();
                bString = true;
                break;
            case CELLTYPE_EDIT :
                {
                    const EditTextObject* pObj;
                    static_cast<const ScEditCell*>(pCell)->GetData( pObj);
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
                        ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                        bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                    }
                    else
                    {
                        ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                        bString = false;
                    }
                }
                break;
            default:
                OSL_FAIL( "ScDocShell::AsciiSave: unknown CellType" );
                aString = rtl::OUString();
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
            rtl::OUString aUniString = aString;//remove that later
            if (!bString && cStrDelim != 0 && !aUniString.isEmpty())
            {
                sal_Unicode c = aUniString[0];
                bString = (c == cStrDelim || c == ' ' ||
                        aUniString[aUniString.getLength()-1] == ' ' ||
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

                        escapeTextSep<rtl::OUString, rtl::OUStringBuffer>(
                            nPos, rtl::OUString(cStrDelim), aUniString);

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
                            rtl::OString aStrEnc = rtl::OUStringToOString(aUniString, eCharSet);
                            // back to Unicode
                            rtl::OUString aStrDec = rtl::OStringToOUString(aStrEnc, eCharSet);

                            // search on re-decoded string
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrDec, rAsciiOpt, aStrDelimDecoded, aDelimDecoded, bNeedQuotes);

                            escapeTextSep<rtl::OUString, rtl::OUStringBuffer>(
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
                            rtl::OString aStrEnc = rtl::OUStringToOString(aUniString, eCharSet);

                            // search on encoded string
                            bool bNeedQuotes = false;
                            sal_Int32 nPos = getTextSepPos(
                                aStrEnc, rAsciiOpt, aStrDelimEncoded, aDelimEncoded, bNeedQuotes);

                            escapeTextSep<rtl::OString, rtl::OStringBuffer>(
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
    rStream.SetNumberFormatInt( nOldNumberFormatInt );
}

sal_Bool ScDocShell::ConvertTo( SfxMedium &rMed )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertTo" );

    ScRefreshTimerProtector aProt( aDocument.GetRefreshTimerControlAddress() );

    //  #i6500# don't call DoEnterHandler here (doesn't work with AutoSave),
    //  it's already in ExecuteSave (as for Save and SaveAs)

    if (pAutoStyleList)
        pAutoStyleList->ExecuteAllNow();                // Vorlagen-Timeouts jetzt ausfuehren
    if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SfxObjectShell::SetVisArea( Rectangle() );     // normal bearbeitet -> keine VisArea

    OSL_ENSURE( rMed.GetFilter(), "Filter == 0" );

    sal_Bool bRet = false;
    String aFltName = rMed.GetFilter()->GetFilterName();

    if (aFltName.EqualsAscii(pFilterXML))
    {
        //TODO/LATER: this shouldn't happen!
        OSL_FAIL("XML filter in ConvertFrom?!");
        bRet = SaveXML( &rMed, NULL );
    }
    else if (aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
             aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx5Temp) ||
             aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp))
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
                if( pItemSet && pItemSet->GetItemState( SID_PASSWORD, sal_True, &pItem ) == SFX_ITEM_SET )
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
            if( aFltName.EqualsAscii( pFilterExcel97 ) || aFltName.EqualsAscii( pFilterEx97Temp ) )
                eFormat = ExpBiff8;
            FltError eError = ScFormatFilter::Get().ScExportExcel5( rMed, &aDocument, eFormat, RTL_TEXTENCODING_MS_1252 );

            if( eError && !GetError() )
                SetError( eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

            // don't return false for warnings
            bRet = ((eError & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK) || (eError == eERR_OK);
        }
        else
        {
            // export aborted, i.e. "Save without password" warning
            SetError( ERRCODE_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        }
    }
    else if (aFltName.EqualsAscii(pFilterAscii))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            String sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if ( sItStr.Len() == 0 )
            {
                //  default for ascii export (from API without options):
                //  ISO8859-1/MS_1252 encoding, comma, double quotes

                ScImportOptions aDefOptions( ',', '"', RTL_TEXTENCODING_MS_1252 );
                sItStr = aDefOptions.BuildString();
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScImportOptions aOptions( sItStr );
            AsciiSave( *pStream, aOptions );
            bRet = sal_True;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
        }
    }
    else if (aFltName.EqualsAscii(pFilterDBase))
    {
        String sCharSet;
        SfxItemSet* pSet = rMed.GetItemSet();
        const SfxPoolItem* pItem;
        if ( pSet && SFX_ITEM_SET ==
             pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
        {
            sCharSet = ((const SfxStringItem*)pItem)->GetValue();
        }

        if (sCharSet.Len() == 0)
        {
            //  default for dBase export (from API without options):
            //  IBM_850 encoding

            sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
        }

        WaitObject aWait( GetActiveDialogParent() );
// HACK damit Sba geoffnetes TempFile ueberschreiben kann
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
            aTmpFile.setExtension(rtl::OUString("dbt"));
        if ( eError != eERR_OK )
        {
            if (!GetError())
                SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
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
                aDbtFile.setExtension(rtl::OUString("dbt"));
                if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
                    bRet = false;
                if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
                    bRet = false;
                if ( !bRet )
                {
                    KillFile( aTmpFile );
                    if ( !GetError() )
                        SetError( SCERR_EXPORT_DATA, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                }
            }
        }
    }
    else if (aFltName.EqualsAscii(pFilterDif))
    {
        SvStream* pStream = rMed.GetOutStream();
        if (pStream)
        {
            String sItStr;
            SfxItemSet*  pSet = rMed.GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET ==
                 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
            {
                sItStr = ((const SfxStringItem*)pItem)->GetValue();
            }

            if (sItStr.Len() == 0)
            {
                //  default for DIF export (from API without options):
                //  ISO8859-1/MS_1252 encoding

                sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
            }

            WaitObject aWait( GetActiveDialogParent() );
            ScFormatFilter::Get().ScExportDif( *pStream, &aDocument, ScAddress(0,0,0),
                ScGlobal::GetCharsetValue(sItStr) );
            bRet = sal_True;

            if (aDocument.GetTableCount() > 1)
                if (!rMed.GetError())
                    rMed.SetError(SCWARN_EXPORT_ASCII, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
        }
    }
    else if (aFltName.EqualsAscii(pFilterSylk))
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
            aImExport.SetFormulas( sal_True );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_SYLK );
        }
    }
    else if (aFltName.EqualsAscii(pFilterHtml))
    {
        SvStream* pStream = rMed.GetOutStream();
        if ( pStream )
        {
            WaitObject aWait( GetActiveDialogParent() );
            ScImportExport aImExport( &aDocument );
            aImExport.SetStreamPath( rMed.GetName() );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_HTML );
            if ( bRet && aImExport.GetNonConvertibleChars().Len() )
                SetError( *new StringErrorInfo(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_INFO ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        }
    }
    else
    {
        if (GetError())
            SetError(SCERR_IMPORT_NI, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
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

    //  SC_HINT_DOC_SAVED fuer Wechsel ReadOnly -> Read/Write
    Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
    return bRet;
}


sal_Bool ScDocShell::QuerySlotExecutable( sal_uInt16 nSlotId )
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


sal_uInt16 ScDocShell::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
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

    sal_uInt16 nRet = SfxObjectShell::PrepareClose( bUI, bForBrowsing );
    if (nRet == sal_True)                       // sal_True = schliessen
        aDocument.DisableIdle(sal_True);        // nicht mehr drin rumpfuschen !!!

    return nRet;
}

void ScDocShell::PrepareReload()
{
    SfxObjectShell::PrepareReload();    // tut nichts?

    //  Das Disconnect von DDE-Links kann Reschedule ausloesen.
    //  Wenn die DDE-Links erst im Dokument-dtor geloescht werden, kann beim Reload
    //  aus diesem Reschedule das DDE-Link-Update fuer das neue Dokument ausgeloest
    //  werden. Dabei verklemmt sicht dann irgendwas.
    //  -> Beim Reload die DDE-Links des alten Dokuments vorher disconnecten

    aDocument.DisconnectDdeLinks();
}


String ScDocShell::GetOwnFilterName()
{
    return rtl::OUString(pFilterSc50);
}

String ScDocShell::GetHtmlFilterName()
{
    return rtl::OUString(pFilterHtml);
}

String ScDocShell::GetWebQueryFilterName()
{
    return rtl::OUString(pFilterHtmlWebQ);
}

String ScDocShell::GetAsciiFilterName()
{
    return rtl::OUString(pFilterAscii);
}

String ScDocShell::GetLotusFilterName()
{
    return rtl::OUString(pFilterLotus);
}

String ScDocShell::GetDBaseFilterName()
{
    return rtl::OUString(pFilterDBase);
}

String ScDocShell::GetDifFilterName()
{
    return rtl::OUString(pFilterDif);
}

sal_Bool ScDocShell::HasAutomaticTableName( const String& rFilter )
{
    //  sal_True for those filters that keep the default table name
    //  (which is language specific)

    return rFilter.EqualsAscii( pFilterAscii )
        || rFilter.EqualsAscii( pFilterLotus )
        || rFilter.EqualsAscii( pFilterExcel4 )
        || rFilter.EqualsAscii( pFilterEx4Temp )
        || rFilter.EqualsAscii( pFilterDBase )
        || rFilter.EqualsAscii( pFilterDif )
        || rFilter.EqualsAscii( pFilterSylk )
        || rFilter.EqualsAscii( pFilterHtml )
        || rFilter.EqualsAscii( pFilterRtf );
}

#ifndef ENABLE_TELEPATHY
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
    aDdeTextFmt(rtl::OUString("TEXT")),
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
#ifdef ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = rShell.bIsInplace;

    pDocFunc = CreateDocFunc();

    //  SetBaseModel needs exception handling
    ScModelObj::CreateAndSet( this );

    StartListening(*this);
    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        StartListening(*pStlPool);

    GetPageOnFromPageStyleSet( NULL, 0, bHeaderOn, bFooterOn );
    SetHelpId( HID_SCSHELL_DOCSH );

    //  InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

ScDocShell::ScDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell( i_nSfxCreationFlags ),
    aDocument       ( SCDOCMODE_DOCUMENT, this ),
    aDdeTextFmt(rtl::OUString("TEXT")),
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
#ifdef ENABLE_TELEPATHY
    , mpCollaboration( new ScCollaboration( this ) )
#endif
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

    SetPool( &SC_MOD()->GetPool() );

    bIsInplace = (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
    //  wird zurueckgesetzt, wenn nicht inplace

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

    //  InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

ScDocShell::~ScDocShell()
{
    ResetDrawObjectShell(); // falls der Drawing-Layer noch versucht, darauf zuzugreifen

    SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
    if (pStlPool)
        EndListening(*pStlPool);
    EndListening(*this);

    delete pAutoStyleList;

    SfxApplication *pSfxApp = SFX_APP();
    if ( pSfxApp->GetDdeService() )             // DDE vor Dokument loeschen
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
#ifdef ENABLE_TELEPATHY
    delete mpCollaboration;
#endif
}

//------------------------------------------------------------------

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


void ScDocShell::SetDocumentModified( sal_Bool bIsModified /* = sal_True */ )
{
    //  BroadcastUno muss auch mit pPaintLockData sofort passieren
    //! auch bei SetDrawModified, wenn Drawing angebunden ist
    //! dann eigener Hint???

    if ( pPaintLockData && bIsModified )
    {
        // #i115009# broadcast BCA_BRDCST_ALWAYS, so a component can read recalculated results
        // of RecalcModeAlways formulas (like OFFSET) after modifying cells
        aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
        aDocument.InvalidateTableArea();    // #i105279# needed here
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        pPaintLockData->SetModified();          // spaeter...
        return;
    }

    SetDrawModified( bIsModified );

    if ( bIsModified )
    {
        if ( aDocument.IsAutoCalcShellDisabled() )
            SetDocumentModifiedPending( sal_True );
        else
        {
            SetDocumentModifiedPending( false );
            aDocument.InvalidateStyleSheetUsage();
            aDocument.InvalidateTableArea();
            aDocument.InvalidateLastTableOpParams();
            aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
            if ( aDocument.IsForcedFormulaPending() && aDocument.GetAutoCalc() )
                aDocument.CalcFormulaTree( sal_True );
            PostDataChanged();

            //  Detective AutoUpdate:
            //  Update if formulas were modified (DetectiveDirty) or the list contains
            //  "Trace Error" entries (Trace Error can look completely different
            //  after changes to non-formula cells).

            ScDetOpList* pList = aDocument.GetDetOpList();
            if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
                 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
            {
                GetDocFunc().DetectiveRefresh(sal_True);    // sal_True = caused by automatic update
            }
            aDocument.SetDetectiveDirty(false);         // always reset, also if not refreshed
        }

        // notify UNO objects after BCA_BRDCST_ALWAYS etc.
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
    }
}

//  SetDrawModified - ohne Formel-Update
//  (Drawing muss auch beim normalen SetDocumentModified upgedated werden,
//   z.B. bei Tabelle loeschen etc.)

void ScDocShell::SetDrawModified( sal_Bool bIsModified /* = sal_True */ )
{
    sal_Bool bUpdate = ( bIsModified != IsModified() );

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
            SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));    // Navigator
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

    //nur mit Statistik, wenn dieses Doc auch angezeigt wird, nicht
    //aus dem Doc-Manager

    if( pDocSh == this )
    {
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
        ::CreateTabPage ScDocStatPageCreate =   pFact->GetTabPageCreatorFunc( RID_SCPAGE_STAT );
        OSL_ENSURE(ScDocStatPageCreate, "Tabpage create fail!");
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
        ModuleUIConfigurationManagerSupplier::create(xContext) );

    // Grab the Calc configuration.
    Reference<XUIConfigurationManager> xConfigMgr =
        xModuleCfgSupplier->getUIConfigurationManager(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument")));

    if (!xConfigMgr.is())
        return;

    // shortcut manager
    Reference<XAcceleratorConfiguration> xScAccel(
        xConfigMgr->getShortCutManager(), UNO_QUERY);

    if (!xScAccel.is())
        return;

    vector<const awt::KeyEvent*> aKeys;
    aKeys.reserve(4);

    // Backsapce key
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
            xScAccel->setKeyEvent(aDelete, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:ClearContents")));
            xScAccel->setKeyEvent(aBackspace, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Delete")));
            xScAccel->setKeyEvent(aCtrlD, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillDown")));
            xScAccel->setKeyEvent(aAltDown, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DataSelect")));
        break;
        case ScOptionsUtil::KEY_OOO_LEGACY:
            xScAccel->setKeyEvent(aDelete, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Delete")));
            xScAccel->setKeyEvent(aBackspace, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:ClearContents")));
            xScAccel->setKeyEvent(aCtrlD, OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DataSelect")));
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
        aProtector( rDS.GetDocument()->GetRefreshTimerControlAddress() )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
    bIdleDisabled = pDoc->IsIdleDisabled();
    pDoc->SetAutoCalcShellDisabled( sal_True );
    pDoc->DisableIdle( sal_True );
}


ScDocShellModificator::~ScDocShellModificator()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
    if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
        rDocShell.SetDocumentModified();    // last one shuts off the lights
    pDoc->DisableIdle( bIdleDisabled );
}


void ScDocShellModificator::SetDocumentModified()
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( !pDoc->IsImportingXML() )
    {
        // AutoCalcShellDisabled temporaer restaurieren
        sal_Bool bDisabled = pDoc->IsAutoCalcShellDisabled();
        pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
        rDocShell.SetDocumentModified();
        pDoc->SetAutoCalcShellDisabled( bDisabled );
    }
    else
    {
        // uno broadcast is necessary for api to work
        // -> must also be done during xml import
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
        aChangeViewSet.SetShowChanges(sal_True);
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


bool ScDocShell::SetProtectionPassword( const String &rNewPassword )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack)
    {
        bool bProtected = pChangeTrack->IsProtected();

        if (rNewPassword.Len())
        {
            // when password protection is applied change tracking must always be active
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
