/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documen2.cxx,v $
 *
 *  $Revision: 1.70 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 13:23:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <svx/eeitem.hxx>

#include <svx/editeng.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/linkmgr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

#include "document.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "globstr.hrc"
#include "chartarr.hxx"
#include "chartlock.hxx"
#include "rechead.hxx"
#include "global.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "adiasync.hxx"
#include "addinlis.hxx"
#include "chartlis.hxx"
#include "markdata.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "progress.hxx"
#include "detdata.hxx"
#include "sc.hrc"               // FID_DATACHANGED
#include "ddelink.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include "editutil.hxx"
#include "hints.hxx"
#include "dpobject.hxx"
#include "indexmap.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"
#include "unoreflist.hxx"
#include "listenercalls.hxx"
#include "recursionhelper.hxx"
#include "lookupcache.hxx"

// pImpl because including lookupcache.hxx in document.hxx isn't wanted, and
// dtor is convenient.
struct ScLookupCacheMapImpl
{
    ScLookupCacheMap aCacheMap;
    ~ScLookupCacheMapImpl()
    {
        for (ScLookupCacheMap::iterator it( aCacheMap.begin()); it != aCacheMap.end(); ++it)
            delete (*it).second;
    }
};

// STATIC DATA -----------------------------------------------------------

ScDocument::ScDocument( ScDocumentMode  eMode,
                        SfxObjectShell* pDocShell ) :
        xServiceManager( ::comphelper::getProcessServiceFactory() ),
        pEditEngine( NULL ),
        pNoteEngine( NULL ),
        pNoteItemPool( NULL ),
        pShell( pDocShell ),
        pPrinter( NULL ),
        pVirtualDevice_100th_mm( NULL ),
        pDrawLayer( NULL ),
        pColorTable( NULL ),
        pCondFormList( NULL ),
        pValidationList( NULL ),
        pFormatExchangeList( NULL ),
        pDPCollection( NULL ),
        pLinkManager( NULL ),
        pFormulaTree( NULL ),
        pEOFormulaTree( NULL ),
        pFormulaTrack( NULL ),
        pEOFormulaTrack( NULL ),
        pOtherObjects( NULL ),
        pClipData( NULL ),
        pDetOpList(NULL),
        pChangeTrack( NULL ),
        pUnoBroadcaster( NULL ),
        pUnoListenerCalls( NULL ),
        pUnoRefUndoList( NULL ),
        pChangeViewSettings( NULL ),
        pScriptTypeData( NULL ),
        pCacheFieldEditEngine( NULL ),
        pViewOptions( NULL ),
        pDocOptions( NULL ),
        pExtDocOptions( NULL ),
        pConsolidateDlgData( NULL ),
        pLoadedSymbolStringCellList( NULL ),
        pRecursionHelper( NULL ),
        pAutoNameCache( NULL ),
        pLookupCacheMapImpl( NULL ),
        nUnoObjectId( 0 ),
        nRangeOverflowType( 0 ),
        aCurTextWidthCalcPos(MAXCOL,0,0),
        nFormulaCodeInTree(0),
        nXMLImportedFormulaCount( 0 ),
        nInterpretLevel(0),
        nMacroInterpretLevel(0),
        nInterpreterTableOpLevel(0),
        nMaxTableNumber( 0 ),
        nSrcVer( SC_CURRENT_VERSION ),
        nSrcMaxRow( MAXROW ),
        nFormulaTrackCount(0),
        nHardRecalcState(0),
        nVisibleTab( 0 ),
        eLinkMode(LM_UNKNOWN),
        bProtected( FALSE ),
        bAutoCalc( eMode == SCDOCMODE_DOCUMENT ),
        bAutoCalcShellDisabled( FALSE ),
        bForcedFormulaPending( FALSE ),
        bCalculatingFormulaTree( FALSE ),
        bIsClip( eMode == SCDOCMODE_CLIP ),
        bCutMode( FALSE ),
        bIsUndo( eMode == SCDOCMODE_UNDO ),
        bIsVisible( FALSE ),
        bIsEmbedded( FALSE ),
//      bNoSetDirty( TRUE ),
        bNoSetDirty( FALSE ),
        bInsertingFromOtherDoc( FALSE ),
        bImportingXML( FALSE ),
        bXMLFromWrapper( FALSE ),
        bCalcingAfterLoad( FALSE ),
        bNoListening( FALSE ),
        bLoadingDone( TRUE ),
        bIdleDisabled( FALSE ),
        bInLinkUpdate( FALSE ),
        bChartListenerCollectionNeedsUpdate( FALSE ),
        bHasForcedFormulas( FALSE ),
        bLostData(FALSE),
        bInDtorClear( FALSE ),
        bExpandRefs( FALSE ),
        bDetectiveDirty( FALSE ),
        nMacroCallMode( SC_MACROCALL_ALLOWED ),
        bHasMacroFunc( FALSE ),
        nVisSpellState( 0 ),
        nAsianCompression(SC_ASIANCOMPRESSION_INVALID),
        nAsianKerning(SC_ASIANKERNING_INVALID),
        bSetDrawDefaults( FALSE ),
        bPastingDrawFromOtherDoc( FALSE ),
        nInDdeLinkUpdate( 0 ),
        bInUnoBroadcast( FALSE ),
        bInUnoListenerCall( FALSE ),
        eAddrConv( ScAddress::CONV_OOO ),
        bStyleSheetUsageInvalid( TRUE ),
        bUndoEnabled( TRUE ),
        mbAdjustHeightEnabled( true ),
        mbExecuteLinkEnabled( true ),
        mbChangeReadOnlyEnabled( false ),
        mnNamedRangesLockCount( 0 )
{
    eSrcSet = gsl_getSystemTextEncoding();

    if ( eMode == SCDOCMODE_DOCUMENT )
    {
        if ( pDocShell )
            pLinkManager = new SvxLinkManager( pDocShell );

        xPoolHelper = new ScPoolHelper( this );

        pTab[0]  = NULL;
        pBASM = new ScBroadcastAreaSlotMachine( this );
        pChartListenerCollection = new ScChartListenerCollection( this );
        pRefreshTimerControl = new ScRefreshTimerControl;
    }
    else
    {
        pTab[0]     = NULL;
        pBASM       = NULL;
        pChartListenerCollection = NULL;
        pRefreshTimerControl = NULL;
    }

    for (SCTAB i=1; i<=MAXTAB; i++)
        pTab[i] = NULL;

    pRangeName = new ScRangeName( 4, 4, FALSE, this );
    pDBCollection = new ScDBCollection( 4, 4, FALSE, this );
    pPivotCollection = new ScPivotCollection(4, 4, this );
    pSelectionAttr = NULL;
    pChartCollection = new ScChartCollection;
    apTemporaryChartLock = std::auto_ptr< ScTemporaryChartLock >( new ScTemporaryChartLock(this) );
    xColNameRanges = new ScRangePairList;
    xRowNameRanges = new ScRangePairList;
    ImplCreateOptions();
    // languages for a visible document are set by docshell later (from options)
    SetLanguage( ScGlobal::eLnge, ScGlobal::eLnge, ScGlobal::eLnge );

    aTrackTimer.SetTimeoutHdl( LINK( this, ScDocument, TrackTimeHdl ) );
    aTrackTimer.SetTimeout( 100 );
}


void ScDocument::SetDocVisible( BOOL bSet )
{
    //  called from view ctor - only for a visible document,
    //  each new sheet's RTL flag is initialized from the locale
    bIsVisible = bSet;
}


sal_uInt32 ScDocument::GetDocumentID() const
{
    const ScDocument* pThis = this;
    sal_uInt32 nCrc = rtl_crc32( 0, &pThis, sizeof(ScDocument*) );
    // the this pointer only might not be sufficient
    nCrc = rtl_crc32( nCrc, &pShell, sizeof(SfxObjectShell*) );
    return nCrc;
}


void ScDocument::StartChangeTracking()
{
    if (!pChangeTrack)
        pChangeTrack = new ScChangeTrack( this );
}

void ScDocument::EndChangeTracking()
{
    delete pChangeTrack;
    pChangeTrack = NULL;
}

void ScDocument::SetChangeTrack( ScChangeTrack* pTrack )
{
    DBG_ASSERT( pTrack->GetDocument() == this, "SetChangeTrack: different documents" );
    if ( !pTrack || pTrack == pChangeTrack || pTrack->GetDocument() != this )
        return ;
    EndChangeTracking();
    pChangeTrack = pTrack;
}


IMPL_LINK( ScDocument, TrackTimeHdl, Timer*, EMPTYARG )
{
    if ( ScDdeLink::IsInUpdate() )      // nicht verschachteln
    {
        aTrackTimer.Start();            // spaeter nochmal versuchen
    }
    else if (pShell)                    // ausfuehren
    {
        TrackFormulas();
        pShell->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );

            //  modified...

        if (!pShell->IsModified())
        {
            pShell->SetModified( TRUE );
            SfxBindings* pBindings = GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
    }

    return 0;
}

void ScDocument::StartTrackTimer()
{
    if (!aTrackTimer.IsActive())        // nicht ewig aufschieben
        aTrackTimer.Start();
}

ScDocument::~ScDocument()
{
    DBG_ASSERT( !bInLinkUpdate, "bInLinkUpdate in dtor" );

    bInDtorClear = TRUE;

    // first of all disable all refresh timers by deleting the control
    if ( pRefreshTimerControl )
    {   // To be sure there isn't anything running do it with a protector,
        // this ensures also that nothing needs the control anymore.
        ScRefreshTimerProtector aProt( GetRefreshTimerControlAddress() );
        delete pRefreshTimerControl, pRefreshTimerControl = NULL;
    }

    // Links aufrauemen

    if ( pLinkManager )
    {
        // BaseLinks freigeben
        for ( USHORT n = pLinkManager->GetServers().Count(); n; )
            pLinkManager->GetServers()[ --n ]->Closed();

        if ( pLinkManager->GetLinks().Count() )
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
    }

    ScAddInAsync::RemoveDocument( this );
    ScAddInListener::RemoveDocument( this );
    delete pChartListenerCollection;    // vor pBASM wg. evtl. Listener!
    pChartListenerCollection = NULL;
    DELETEZ( pLookupCacheMapImpl);  // before pBASM because of listeners
    // BroadcastAreas vor allen Zellen zerstoeren um unnoetige
    // Einzel-EndListenings der Formelzellen zu vermeiden
    delete pBASM;       // BroadcastAreaSlotMachine
    pBASM = NULL;

    if (pUnoBroadcaster)
    {
        delete pUnoBroadcaster;     // broadcasted nochmal SFX_HINT_DYING
        pUnoBroadcaster = NULL;
    }

    delete pUnoRefUndoList;
    delete pUnoListenerCalls;

    Clear( sal_True );              // TRUE = from destructor (needed for SdrModel::ClearModel)

    if (pCondFormList)
    {
        pCondFormList->DeleteAndDestroy( 0, pCondFormList->Count() );
        DELETEZ(pCondFormList);
    }
    if (pValidationList)
    {
        pValidationList->DeleteAndDestroy( 0, pValidationList->Count() );
        DELETEZ(pValidationList);
    }
    delete pRangeName;
    delete pDBCollection;
    delete pPivotCollection;
    delete pSelectionAttr;
    apTemporaryChartLock.reset();
    delete pChartCollection;
    DeleteDrawLayer();
    delete pFormatExchangeList;
    delete pPrinter;
    ImplDeleteOptions();
    delete pConsolidateDlgData;
    delete pLinkManager;
    delete pClipData;
    delete pDetOpList;                  // loescht auch die Eintraege
    delete pChangeTrack;
    delete pEditEngine;
    delete pNoteEngine;
    delete pNoteItemPool;
    delete pChangeViewSettings;         // und weg damit
    delete pVirtualDevice_100th_mm;

    delete pDPCollection;

    // delete the EditEngine before destroying the xPoolHelper
    delete pCacheFieldEditEngine;

    if ( xPoolHelper.isValid() && !bIsClip )
        xPoolHelper->SourceDocumentGone();
    xPoolHelper.unbind();

    DeleteColorTable();
    delete pScriptTypeData;
    delete pOtherObjects;
    delete pRecursionHelper;

    DBG_ASSERT( !pAutoNameCache, "AutoNameCache still set in dtor" );
}

void ScDocument::InitClipPtrs( ScDocument* pSourceDoc )
{
    DBG_ASSERT(bIsClip, "InitClipPtrs und nicht bIsClip");

    if (pCondFormList)
    {
        pCondFormList->DeleteAndDestroy( 0, pCondFormList->Count() );
        DELETEZ(pCondFormList);
    }
    if (pValidationList)
    {
        pValidationList->DeleteAndDestroy( 0, pValidationList->Count() );
        DELETEZ(pValidationList);
    }

    Clear();

    xPoolHelper = pSourceDoc->xPoolHelper;

    //  bedingte Formate / Gueltigkeiten
    //! Vorlagen kopieren?
    const ScConditionalFormatList* pSourceCond = pSourceDoc->pCondFormList;
    if ( pSourceCond )
        pCondFormList = new ScConditionalFormatList(this, *pSourceCond);
    const ScValidationDataList* pSourceValid = pSourceDoc->pValidationList;
    if ( pSourceValid )
        pValidationList = new ScValidationDataList(this, *pSourceValid);

                        // Links in Stream speichern
    delete pClipData;
    if (pSourceDoc->HasDdeLinks())
    {
        pClipData = new SvMemoryStream;
        pSourceDoc->SaveDdeLinks(*pClipData);
    }
    else
        pClipData = NULL;

    // Options pointers exist (ImplCreateOptions) for any document.
    // Must be copied for correct results in OLE objects (#i42666#).
    SetDocOptions( pSourceDoc->GetDocOptions() );
    SetViewOptions( pSourceDoc->GetViewOptions() );
}

SvNumberFormatter* ScDocument::GetFormatTable() const
{
    return xPoolHelper->GetFormTable();
}

SfxItemPool* ScDocument::GetEditPool() const
{
    return xPoolHelper->GetEditPool();
}

SfxItemPool* ScDocument::GetEnginePool() const
{
    return xPoolHelper->GetEnginePool();
}

ScFieldEditEngine& ScDocument::GetEditEngine()
{
    if ( !pEditEngine )
    {
        pEditEngine = new ScFieldEditEngine( GetEnginePool(), GetEditPool() );
        pEditEngine->SetUpdateMode( FALSE );
        pEditEngine->EnableUndo( FALSE );
        pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pEditEngine->SetForbiddenCharsTable( xForbiddenCharacters );
    }
    return *pEditEngine;
}

ScNoteEditEngine& ScDocument::GetNoteEngine()
{
    if ( !pNoteEngine )
    {
        pNoteEngine = new ScNoteEditEngine( GetEnginePool(), GetEditPool() );
        pNoteEngine->SetUpdateMode( FALSE );
        pNoteEngine->EnableUndo( FALSE );
        pNoteEngine->SetRefMapMode( MAP_100TH_MM );
        pNoteEngine->SetForbiddenCharsTable( xForbiddenCharacters );
                const SfxItemSet& rItemSet = GetDefPattern()->GetItemSet();
                SfxItemSet* pEEItemSet = new SfxItemSet( pNoteEngine->GetEmptyItemSet() );
                ScPatternAttr::FillToEditItemSet( *pEEItemSet, rItemSet );
                pNoteEngine->SetDefaults( pEEItemSet );      // edit engine takes ownership
    }
    return *pNoteEngine;
}

SfxItemPool& ScDocument::GetNoteItemPool()
{
    if ( !pNoteItemPool )
        pNoteItemPool = new SfxItemPool(SdrObject::GetGlobalDrawObjectItemPool());
    return *pNoteItemPool;
}

void ScDocument::ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        for (SCTAB i = 0; i <= MAXTAB; i++)
            if (pSourceDoc->pTab[i])
                if (!pMarks || pMarks->GetTableSelect(i))
                {
                    String aString;
                    pSourceDoc->pTab[i]->GetName(aString);
                    pTab[i] = new ScTable(this, i, aString);
                    pTab[i]->SetLayoutRTL( pSourceDoc->pTab[i]->IsLayoutRTL() );
                    nMaxTableNumber = i+1;
                }
    }
    else
    {
        DBG_ERROR("ResetClip");
    }
}

void ScDocument::ResetClip( ScDocument* pSourceDoc, SCTAB nTab )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("baeh")));
        if (pSourceDoc->pTab[nTab])
            pTab[nTab]->SetLayoutRTL( pSourceDoc->pTab[nTab]->IsLayoutRTL() );
        nMaxTableNumber = nTab+1;
    }
    else
    {
        DBG_ERROR("ResetClip");
    }
}

void lcl_RefreshPivotData( ScPivotCollection* pColl )
{
    USHORT nCount = pColl->GetCount();
    for (USHORT i=0; i<nCount; i++)
    {
        ScPivot* pPivot = (*pColl)[i];
        if (pPivot->CreateData(TRUE))
            pPivot->ReleaseData();
    }
}


BOOL ScDocument::SymbolStringCellsPending() const
{
    return pLoadedSymbolStringCellList && pLoadedSymbolStringCellList->Count();
}


List& ScDocument::GetLoadedSymbolStringCellsList()
{
    if ( !pLoadedSymbolStringCellList )
        pLoadedSymbolStringCellList = new List;
    return *pLoadedSymbolStringCellList;
}


BOOL ScDocument::Load( SvStream& rStream, ScProgress* pProgress )
{
    bLoadingDone = FALSE;

    //----------------------------------------------------

    Clear();
    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );

        //  Progress-Bar

//  ULONG nCurPos = rStream.Tell();
//  ULONG nEndPos = rStream.Seek( STREAM_SEEK_TO_END );
//  rStream.Seek( nCurPos );
//  ScProgress aProgress( NULL, ScGlobal::GetRscString(STR_LOAD_DOC), nEndPos - nCurPos );

    BOOL bError = FALSE;
    USHORT nVersion = 0;
    SCROW nVerMaxRow = MAXROW_30;       // 8191, wenn in der Datei nichts steht
    SCTAB nTab = 0;
    USHORT nEnumDummy;
    String aEmptyName;
    String aPageStyle;
    CharSet eOldSet = rStream.GetStreamCharSet();

    USHORT nID;
    rStream >> nID;
    if (nID == SCID_DOCUMENT || nID == SCID_NEWDOCUMENT )
    {
        ScReadHeader aHdr( rStream );
        while (aHdr.BytesLeft() && !bError )
        {
            USHORT nSubID;
            rStream >> nSubID;
            switch (nSubID)
            {
                case SCID_DOCFLAGS:
                    {
                        ScReadHeader aFlagsHdr( rStream );

                        rStream >> nVersion;                // 312 abwaerts
                        rStream.ReadByteString( aPageStyle, rStream.GetStreamCharSet() );
                        rStream >> bProtected;              // Dokument geschuetzt
                        String aPass;
                        rStream.ReadByteString( aPass, rStream.GetStreamCharSet() );
                        if (aPass.Len())
                            SvPasswordHelper::GetHashPassword(aProtectPass, aPass);
                        if ( aFlagsHdr.BytesLeft() )
                        {
                            rStream >> nEnumDummy;
                            eLanguage = LanguageType( nEnumDummy );
                        }
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> bAutoCalc;
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVisibleTab;
#endif
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVersion;            // echte Version
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVerMaxRow;          // sonst auf 8191 lassen
#endif

                        nSrcVer     = nVersion;             // Member
                        nSrcMaxRow  = nVerMaxRow;           // Member

                        // Fuer Debugging bis hin zur SC 3.0a:
                        if( nVersion > 0x0002 && nVersion < SC_NUMFMT )
                        {
                            bError = TRUE;
                            rStream.SetError( SVSTREAM_WRONGVERSION );
                        }

                        // Das obere Byte muss kleiner oder gleich sein
                        //  (3.1 Dateien mit 8192 Zeilen koennen noch gelesen werden)

                        if( ( nSrcVer & 0xFF00 ) > ( SC_CURRENT_VERSION & 0xFF00 ) )
                        {
                            bError = TRUE;
                            rStream.SetError( SVSTREAM_WRONGVERSION );
                        }
                    }
                    break;
                case SCID_CHARSET:
                    {
                        ScReadHeader aSetHdr( rStream );
                        BYTE cSet, cGUI;    // cGUI is dummy, old GUIType
                        rStream >> cGUI >> cSet;
                        eSrcSet = (CharSet) cSet;
                        rStream.SetStreamCharSet( ::GetSOLoadTextEncoding(
                            eSrcSet, (USHORT)rStream.GetVersion() ) );
                    }
                    break;
                case SCID_LINKUPMODE: //    Link Update Mode
                    {
                        ScReadHeader aSetHdr( rStream );
                        BYTE cSet;
                        rStream >> cSet;
                        eLinkMode=(ScLkUpdMode) cSet;
                    }
                    break;
                case SCID_TABLE:
                    pTab[nTab] = new ScTable(this, nTab, aEmptyName);
                    pTab[nTab]->SetPageStyle( aPageStyle );
                    pTab[nTab]->Load(rStream,nVersion,pProgress);
                    ++nTab;
                    break;
                case SCID_DRAWING:
                    DBG_ERROR("ScDocument::LoadDrawLayer() no longer supported, binary loading removed (!)");
                    break;
                case SCID_DDELINKS:
                    LoadDdeLinks(rStream);
                    break;
                case SCID_AREALINKS:
                    LoadAreaLinks(rStream);
                    break;
                case SCID_RANGENAME:
                    pRangeName->Load(rStream, nVersion );
                    break;
                case SCID_DBAREAS:
                    pDBCollection->Load( rStream );
                    break;
                case SCID_DATAPILOT:
                    GetDPCollection()->LoadNew( rStream );
                    break;
                case SCID_PIVOT:
                    pPivotCollection->Load( rStream );
                    break;
                case SCID_CHARTS:
                    pChartCollection->Load( this, rStream );
                    break;
                case SCID_COLNAMERANGES:
                    xColNameRanges->Load( rStream, nVersion );
                    break;
                case SCID_ROWNAMERANGES:
                    xRowNameRanges->Load( rStream, nVersion );
                    break;
                case SCID_CONDFORMATS:
                    if (!pCondFormList)
                        pCondFormList = new ScConditionalFormatList;
                    pCondFormList->Load( rStream, this );
                    break;
                case SCID_VALIDATION:
                    if (!pValidationList)
                        pValidationList = new ScValidationDataList;
                    pValidationList->Load( rStream, this );
                    break;
                case SCID_DETOPLIST:
                    if (!pDetOpList)
                        pDetOpList = new ScDetOpList;
                    pDetOpList->Load( rStream );
                    break;
                case SCID_NUMFORMAT:
                    {
                        ScReadHeader aNumHeader(rStream);
                        xPoolHelper->GetFormTable()->Load(rStream);
                    }
                    break;
                case SCID_DOCOPTIONS:
                    ImplLoadDocOptions(rStream);
                    break;
                case SCID_VIEWOPTIONS:
                    ImplLoadViewOptions(rStream);
                    break;
                case SCID_PRINTSETUP:
                    {
                        ScReadHeader aJobHeader(rStream);
                        SfxItemSet* pSet = new SfxItemSet( *xPoolHelper->GetDocPool(),
                                SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                                SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
                                SID_SCPRINTOPTIONS,        SID_SCPRINTOPTIONS,
                                NULL );
                        SetPrinter( SfxPrinter::Create( rStream, pSet ) );
                    }
                    break;
                case SCID_CONSOLIDATA:
                    if (!pConsolidateDlgData)
                        pConsolidateDlgData = new ScConsolidateParam;
                    pConsolidateDlgData->Load( rStream );
                    break;
                case SCID_CHANGETRACK:
                    if ( pChangeTrack )
                        pChangeTrack->Clear();  // es kann nur einen geben
                    else
                        StartChangeTracking();
                    pChangeTrack->Load( rStream, nVersion );
                    break;
                case SCID_CHGVIEWSET:
                    if (!pChangeViewSettings)
                        pChangeViewSettings = new ScChangeViewSettings;
                    pChangeViewSettings->Load( rStream, nVersion );
                    break;
                default:
                    {
                        DBG_ERROR("unbekannter Sub-Record in ScDocument::Load");
                        ScReadHeader aDummyHdr( rStream );
                    }
            }

            if (rStream.GetError() != SVSTREAM_OK)
                bError = TRUE;
        }
    }
    else
    {
        //  Assertion nur, wenn kein Passwort gesetzt ist
        DBG_ASSERT( rStream.GetKey().Len(), "Load: SCID_DOCUMENT nicht gefunden" );
        bError = TRUE;
    }

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );

    if (!bError)                                    // Neuberechnungen
    {
        xPoolHelper->GetStylePool()->UpdateStdNames();  // falls mit Version in anderer Sprache gespeichert

        //  Zahlformat-Sprache
        //  (kann nicht in LoadPool passieren, weil der Numberformatter geladen sein muss)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        if ( pPool->GetLoadingVersion() == 0 )              // 0 = Pool-Version bis 3.1
        {
            //  in 3.1-Dokumenten gibt es ATTR_LANGUAGE_FORMAT noch nicht
            //  darum bei Bedarf zu ATTR_VALUE_FORMAT noch die Sprache dazutun
            //  (Bug #37441#)

            //  harte Attribute:

            SvNumberFormatter* pFormatter = xPoolHelper->GetFormTable();
            USHORT nCount = pPool->GetItemCount(ATTR_PATTERN);
            ScPatternAttr* pPattern;
            for (USHORT i=0; i<nCount; i++)
            {
                pPattern = (ScPatternAttr*)pPool->GetItem(ATTR_PATTERN, i);
                if (pPattern)
                    ScGlobal::AddLanguage( pPattern->GetItemSet(), *pFormatter );
            }

            //  Vorlagen:

            SfxStyleSheetIterator aIter( xPoolHelper->GetStylePool(), SFX_STYLE_FAMILY_PARA );
            for ( SfxStyleSheetBase* pStyle = aIter.First(); pStyle; pStyle = aIter.Next() )
                ScGlobal::AddLanguage( pStyle->GetItemSet(), *pFormatter );
        }

        // change FontItems in styles
        xPoolHelper->GetStylePool()->ConvertFontsAfterLoad();

        //  Druckbereiche etc.

        SfxStyleSheetIterator   aIter( xPoolHelper->GetStylePool(), SFX_STYLE_FAMILY_PAGE );
        ScStyleSheet*           pStyleSheet = NULL;

        nMaxTableNumber = 0;
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i])
            {
                // MaxTableNumber ermitteln

                nMaxTableNumber = i+1;

                // Druckbereiche aus <= 3.00.2 Dokumenten
                // aus den PageStyles holen und jetzt an
                // der Tabelle speichern.

                pStyleSheet = (ScStyleSheet*)aIter.Find( pTab[i]->GetPageStyle() );

                if ( pStyleSheet )
                {
                    SfxItemSet&         rSet            = pStyleSheet->GetItemSet();
                    const ScRangeItem*  pPrintAreaItem  = NULL;
                    const ScRangeItem*  pRepeatColItem  = NULL;
                    const ScRangeItem*  pRepeatRowItem  = NULL;

                    rSet.GetItemState( ATTR_PAGE_PRINTAREA, TRUE,
                                       (const SfxPoolItem**)&pPrintAreaItem );
                    rSet.GetItemState( ATTR_PAGE_REPEATCOL, TRUE,
                                       (const SfxPoolItem**)&pRepeatColItem );
                    rSet.GetItemState( ATTR_PAGE_REPEATROW, TRUE,
                                       (const SfxPoolItem**)&pRepeatRowItem );

                    if ( pPrintAreaItem ) // Druckbereiche
                    {
                        if ( !pPrintAreaItem->GetFlags() )
                            SetPrintRange( i, pPrintAreaItem->GetRange() );
                        rSet.ClearItem( ATTR_PAGE_PRINTAREA );
                    }

                    if ( pRepeatColItem ) // Wiederholungsspalte
                    {
                        SetRepeatColRange( i, !pRepeatColItem->GetFlags()
                                            ? &pRepeatColItem->GetRange()
                                            : (const ScRange *)NULL );
                        rSet.ClearItem( ATTR_PAGE_REPEATCOL );
                    }

                    if ( pRepeatRowItem ) // Wiederholungszeile
                    {
                        SetRepeatRowRange( i, !pRepeatRowItem->GetFlags()
                                            ? &pRepeatRowItem->GetRange()
                                            : (const ScRange *)NULL );
                        rSet.ClearItem( ATTR_PAGE_REPEATROW );
                    }
                }
            }


        if ( pDPCollection && pDPCollection->GetCount() )
            pPivotCollection->FreeAll();
        else
        {
            lcl_RefreshPivotData( pPivotCollection );
            GetDPCollection()->ConvertOldTables( *pPivotCollection );
        }
        if ( pDPCollection )
            pDPCollection->EnsureNames();   // make sure every table has a name

        SetAutoFilterFlags();
        if (pDrawLayer)
            UpdateAllCharts();
#ifndef PRODUCT
//2do: wg. #62107
// ChartListenerCollection speichern/laden, damit nach dem Laden das Update
// hier einmal eingespart werden kann und somit nicht mehr alle Charts
// angefasst werden muessen. Die ChartListenerCollection muss dann zum Master
// der Referenzen werden.
//      static BOOL bShown = 0;
//      if ( !bShown && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
//      {
//          bShown = 1;
//          DBG_ERRORFILE( "bei inkompatiblem FileFormat ChartListenerCollection speichern!" );
//      }
#endif
        UpdateChartListenerCollection();
        if (pDrawLayer)
            RefreshNoteFlags();
        CalcAfterLoad();
    }

    if ( pLoadedSymbolStringCellList )
    {   // we had symbol string cells, list was cleared by columns, delete it
        delete pLoadedSymbolStringCellList;
        pLoadedSymbolStringCellList = NULL;
    }

    //----------------------------------------------------

    bLoadingDone = TRUE;

    return !bError;
}

BOOL ScDocument::Save( SvStream& rStream, ScProgress* pProgress ) const
{
    ((ScDocument*)this)->bLoadingDone = FALSE;      // nicht zwischendrin reinpfuschen lassen

    ((ScDocument*)this)->bLostData = FALSE;         // wird beim Speichern gesetzt

    ((ScDocument*)this)->nSrcVer = SC_CURRENT_VERSION;
    ((ScDocument*)this)->nSrcMaxRow = MAXROW;
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_31 )
    {
        //  3.1 Export -> nur 8192 Zeilen schreiben, und kompatible Versionsnummer

        ((ScDocument*)this)->nSrcVer = SC_31_EXPORT_VER;
        ((ScDocument*)this)->nSrcMaxRow = MAXROW_30;
    }
    else if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
    {   //  4.0 Export -> kompatible Versionsnummer
        ((ScDocument*)this)->nSrcVer = SC_40_EXPORT_VER;
    }

    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );

    CharSet eOldSet = rStream.GetStreamCharSet();
    CharSet eStoreCharSet = ::GetSOStoreTextEncoding(
        gsl_getSystemTextEncoding(), (USHORT)rStream.GetVersion() );
    rStream.SetStreamCharSet( eStoreCharSet );


        //  Progress-Bar

    long nSavedDocCells = 0;
//  ScProgress aProgress( NULL, ScGlobal::GetRscString( STR_SAVE_DOC ), GetWeightedCount() + 1 );

    {
        rStream << (USHORT) SCID_NEWDOCUMENT;
        ScWriteHeader aHdr( rStream );

        //  Flags

        {
            rStream << (USHORT) SCID_DOCFLAGS;
            ScWriteHeader aFlagsHdr( rStream, 18 );         //! ausprobieren

            // wg. Bug in 312 ScToken::RelToRelAbs mit DoubleRefs bekommt
            // die 312er immer vorgegaukelt, dass es keine RelRefs gaebe,
            // was auch ok ist, da immer absolut gespeichert wird und
            // SR_RELATIVE nie zur Verwendung kam und nicht kommen darf.
            if ( nSrcVer & 0xFF00 )
                rStream << (USHORT) nSrcVer;
                // hoehere Major-Version darf von 312 nicht geladen werden
            else
                rStream << (USHORT) (SC_RELATIVE_REFS - 1);

            // dummy page style (for compatibility)
            rStream.WriteByteString(
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD)),
                        rStream.GetStreamCharSet() );
            rStream << bProtected;                  // Dokument geschuetzt
            String aPass;
            //rStream.WriteByteString( aProtectPass, rStream.GetStreamCharSet() );
            rStream.WriteByteString( aPass, rStream.GetStreamCharSet() );
            rStream << (USHORT) eLanguage;
            rStream << bAutoCalc;

#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
            rStream << nVisibleTab;
#endif

            // und hier jetzt die echte Versionsnummer
            rStream << (USHORT) nSrcVer;

#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
            rStream << nSrcMaxRow;                  // Zeilenanzahl
#endif
        }

        //  Zeichensatz

        {
            rStream << (USHORT) SCID_CHARSET;
            ScWriteHeader aSetHdr( rStream, 2 );
            rStream << (BYTE) 0     // dummy, old System::GetGUIType()
                    << (BYTE) eStoreCharSet;
        }

        //  Link Update Mode

        if(eLinkMode!=LM_UNKNOWN)
        {
            rStream << (USHORT) SCID_LINKUPMODE;
            ScWriteHeader aSetHdr( rStream, 1 );
            rStream << (BYTE) eLinkMode;
        }

        rStream << (USHORT) SCID_RANGENAME;
        pRangeName->Store( rStream );

        rStream << (USHORT) SCID_DBAREAS;
        pDBCollection->Store( rStream );

        rStream << (USHORT) SCID_DDELINKS;
        SaveDdeLinks( rStream );

        rStream << (USHORT) SCID_AREALINKS;
        SaveAreaLinks( rStream );

        {
            rStream << (USHORT) SCID_NUMFORMAT;
            ScWriteHeader aNumHeader(rStream);
            xPoolHelper->GetFormTable()->Save(rStream);
        }

        if ( xColNameRanges->Count() )
        {
            rStream << (USHORT) SCID_COLNAMERANGES;
            xColNameRanges->Store( rStream );
        }
        if ( xRowNameRanges->Count() )
        {
            rStream << (USHORT) SCID_ROWNAMERANGES;
            xRowNameRanges->Store( rStream );
        }

        if (pCondFormList)
            pCondFormList->ResetUsed();     // wird beim Speichern der Tabellen gesetzt
        if (pValidationList)
            pValidationList->ResetUsed();   // wird beim Speichern der Tabellen gesetzt

        //  Tabellen (Daten)

        for (SCTAB i=0; i<=MAXTAB; i++)
        {
            if (pTab[i])
            {
                rStream << (USHORT) SCID_TABLE;
                pTab[i]->Save(rStream, nSavedDocCells, pProgress);
            }
        }

        //  bedingte Formate / Gueltigkeit
        //  beim Speichern der Tabellen ist eingetragen worden,
        //  welche Eintraege benutzt werden

        if (pCondFormList)
        {
            rStream << (USHORT) SCID_CONDFORMATS;
            pCondFormList->Store(rStream);
        }
        if (pValidationList)
        {
            rStream << (USHORT) SCID_VALIDATION;
            pValidationList->Store(rStream);
        }

        //  Liste der Detektiv-Operationen (zum Aktualisieren)
        if (pDetOpList)
        {
            rStream << (USHORT) SCID_DETOPLIST;
            pDetOpList->Store(rStream);
        }

        //  Drawing
        if (pDrawLayer)
        {
            DBG_ERROR("ScDocument::StoreDrawLayer() no longer supported, binary saving removed (!)");
        }

        //  Collections

        //  (new) DataPilot collection must be saved before old Pivot collection
        //  so old data can be skipped by new office
        //  not in 3.0 or 4.0 export to avoid warning messages

        if ( nSrcVer > SC_40_EXPORT_VER && pDPCollection && pDPCollection->GetCount() )
        {
            rStream << (USHORT) SCID_DATAPILOT;             // new data
            pDPCollection->StoreNew( rStream );
        }

        rStream << (USHORT) SCID_PIVOT;                     // old data
        if ( pDPCollection && pDPCollection->GetCount() )
            pDPCollection->StoreOld( rStream );
        else
            pPivotCollection->Store( rStream );             // not converted or all empty

                //  Charts werden hier nicht mehr gespeichert, weil
                //  jedes Chart seine Daten selber speichert

        DBG_ASSERT(!pChartCollection || !pChartCollection->GetCount(),
                        "wer hat da ein Chart eingetragen?");

        rStream << (USHORT) SCID_DOCOPTIONS;
        ImplSaveDocOptions(rStream);

        rStream << (USHORT) SCID_VIEWOPTIONS;
        ImplSaveViewOptions(rStream);

        //  Job-Setup vom Printer

        if (pPrinter)
        {
            rStream << (USHORT) SCID_PRINTSETUP;
            ScWriteHeader aJobHeader(rStream);

            ((ScDocument*)this)->GetPrinter()->Store( rStream );
        }

        if ( nSrcVer > SC_40_EXPORT_VER )   //  Das folgende nicht bei 3.0 oder 4.0 Export...
        {
            if (pConsolidateDlgData)        //  Einstellungen fuer den Konsolidieren-Dialog
            {
                rStream << (USHORT) SCID_CONSOLIDATA;
                pConsolidateDlgData->Store( rStream );
            }
            if ( pChangeTrack )
            {
                rStream << (USHORT) SCID_CHANGETRACK;
                pChangeTrack->Store( rStream );
            }
            if ( pChangeViewSettings )
            {
                rStream << (USHORT) SCID_CHGVIEWSET;
                pChangeViewSettings->Store( rStream );
            }
        }
    }

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );

    ((ScDocument*)this)->bLoadingDone = TRUE;

    return ( rStream.GetError() == SVSTREAM_OK );
}

void ScDocument::SetLostData()
{
    bLostData = TRUE;
}

void ScDocument::DeleteNumberFormat( const sal_uInt32* /* pDelKeys */, sal_uInt32 /* nCount */ )
{
/*
    for (ULONG i = 0; i < nCount; i++)
        xPoolHelper->GetFormTable()->DeleteEntry(pDelKeys[i]);
*/
}

void ScDocument::PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                          ScBaseCell* pCell, ULONG nFormatIndex, BOOL bForceTab )
{
    if (VALIDTAB(nTab))
    {
        if ( bForceTab && !pTab[nTab] )
        {
            BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

            pTab[nTab] = new ScTable(this, nTab,
                                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                                bExtras, bExtras);
        }

        if (pTab[nTab])
            pTab[nTab]->PutCell( nCol, nRow, nFormatIndex, pCell );
    }
}

void ScDocument::PutCell( const ScAddress& rPos, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab )
{
    SCTAB nTab = rPos.Tab();
    if ( bForceTab && !pTab[nTab] )
    {
        BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

        pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                            bExtras, bExtras);
    }

    if (pTab[nTab])
        pTab[nTab]->PutCell( rPos, nFormatIndex, pCell );
}

BOOL ScDocument::GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintArea( rEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, TRUE ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}

BOOL ScDocument::GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintAreaHor( nStartRow, nEndRow, rEndCol, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,nStartRow,nTab, MAXCOL,nEndRow,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, FALSE ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    return FALSE;
}

BOOL ScDocument::GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintAreaVer( nStartCol, nEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(nStartCol,0,nTab, nEndCol,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, FALSE, TRUE ))
            {
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndRow = 0;
    return FALSE;
}

BOOL ScDocument::GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetDataStart( rStartCol, rStartRow );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, TRUE ))
            {
                if (aDrawRange.aStart.Col()<rStartCol) rStartCol=aDrawRange.aStart.Col();
                if (aDrawRange.aStart.Row()<rStartRow) rStartRow=aDrawRange.aStart.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rStartCol = 0;
    rStartRow = 0;
    return FALSE;
}

BOOL ScDocument::MoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    if (nOldPos == nNewPos) return FALSE;
    BOOL bValid = FALSE;
    if (VALIDTAB(nOldPos))
    {
        if (pTab[nOldPos])
        {
            SCTAB nTabCount = GetTableCount();
            if (nTabCount > 1)
            {
                BOOL bOldAutoCalc = GetAutoCalc();
                SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
                SetNoListening( TRUE );
                ScProgress* pProgress = new ScProgress( GetDocumentShell(),
                    ScGlobal::GetRscString(STR_UNDO_MOVE_TAB), GetCodeCount() );
                if (nNewPos == SC_TAB_APPEND)
                    nNewPos = nTabCount-1;

                //  Referenz-Updaterei
                //! mit UpdateReference zusammenfassen!

                SCsTAB nDz = ((SCsTAB)nNewPos) - (SCsTAB)nOldPos;
                ScRange aSourceRange( 0,0,nOldPos, MAXCOL,MAXROW,nOldPos );
                pRangeName->UpdateTabRef(nOldPos, 3, nNewPos);
                pDBCollection->UpdateMoveTab( nOldPos, nNewPos );
                xColNameRanges->UpdateReference( URM_REORDER, this, aSourceRange, 0,0,nDz );
                xRowNameRanges->UpdateReference( URM_REORDER, this, aSourceRange, 0,0,nDz );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference( URM_REORDER,
                                    0,0,nOldPos, MAXCOL,MAXROW,nOldPos, 0,0,nDz );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_REORDER, aSourceRange, 0,0,nDz );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_REORDER, aSourceRange, 0,0,nDz );
                UpdateChartRef( URM_REORDER,
                                    0,0,nOldPos, MAXCOL,MAXROW,nOldPos, 0,0,nDz );
                UpdateRefAreaLinks( URM_REORDER, aSourceRange, 0,0,nDz );
                if ( pCondFormList )
                    pCondFormList->UpdateMoveTab( nOldPos, nNewPos );
                if ( pValidationList )
                    pValidationList->UpdateMoveTab( nOldPos, nNewPos );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_REORDER,
                                    aSourceRange, 0,0,nDz ) );

                ScTable* pSaveTab = pTab[nOldPos];
                SCTAB i;
                for (i = nOldPos + 1; i < nTabCount; i++)
                    pTab[i - 1] = pTab[i];
                pTab[i-1] = NULL;
                for (i = nTabCount - 1; i > nNewPos; i--)
                    pTab[i] = pTab[i - 1];
                pTab[nNewPos] = pSaveTab;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateMoveTab( nOldPos, nNewPos, i, *pProgress );
                delete pProgress;   // freimachen fuer evtl. andere
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                SetNoListening( FALSE );
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->StartAllListeners();
                // #81844# sheet names of references may not be valid until sheet is moved
                pChartListenerCollection->UpdateScheduledSeriesRanges();
                SetDirty();
                SetAutoCalc( bOldAutoCalc );

                if (pDrawLayer)
                    DrawMovePage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

                bValid = TRUE;
            }
        }
    }
    return bValid;
}

BOOL ScDocument::CopyTab( SCTAB nOldPos, SCTAB nNewPos, const ScMarkData* pOnlyMarked )
{
    if (SC_TAB_APPEND == nNewPos ) nNewPos = nMaxTableNumber;
    String aName;
    GetName(nOldPos, aName);

    //  vorneweg testen, ob der Prefix als gueltig erkannt wird
    //  wenn nicht, nur doppelte vermeiden
    BOOL bPrefix = ValidTabName( aName );
    DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
    SCTAB nDummy;

    CreateValidTabName(aName);

    BOOL bValid;
    if (bPrefix)
        bValid = ( ValidNewTabName(aName) && (nMaxTableNumber <= MAXTAB) );
    else
        bValid = ( !GetTable( aName, nDummy ) && (nMaxTableNumber <= MAXTAB) );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    if (bValid)
    {
        if (nNewPos == nMaxTableNumber)
        {
            pTab[nMaxTableNumber] = new ScTable(this, nMaxTableNumber, aName);
            ++nMaxTableNumber;
        }
        else
        {
            if (VALIDTAB(nNewPos) && (nNewPos < nMaxTableNumber))
            {
                SetNoListening( TRUE );

                ScRange aRange( 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                pRangeName->UpdateTabRef(nNewPos, 1);
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,1 );
                UpdateChartRef( URM_INSDEL, 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,1 ) );

                SCTAB i;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos)
                        pTab[i]->UpdateInsertTab(nNewPos);
                for (i = nMaxTableNumber; i > nNewPos; i--)
                    pTab[i] = pTab[i - 1];
                if (nNewPos <= nOldPos)
                    nOldPos++;
                pTab[nNewPos] = new ScTable(this, nNewPos, aName);
                ++nMaxTableNumber;
                bValid = TRUE;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos && i != nNewPos)
                        pTab[i]->UpdateCompile();
                SetNoListening( FALSE );
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos && i != nNewPos)
                        pTab[i]->StartAllListeners();

                //  update conditional formats after table is inserted
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if ( pValidationList )
                    pValidationList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                // #81844# sheet names of references may not be valid until sheet is copied
                pChartListenerCollection->UpdateScheduledSeriesRanges();
            }
            else
                bValid = FALSE;
        }
    }
    if (bValid)
    {
        SetNoListening( TRUE );     // noch nicht bei CopyToTable/Insert
        pTab[nOldPos]->CopyToTable(0, 0, MAXCOL, MAXROW, IDF_ALL, (pOnlyMarked != NULL),
                                        pTab[nNewPos], pOnlyMarked );
        SCsTAB nDz;
/*      if (nNewPos < nOldPos)
            nDz = ((short)nNewPos) - (short)nOldPos + 1;
        else
*/          nDz = ((short)nNewPos) - (short)nOldPos;
        pTab[nNewPos]->UpdateReference(URM_COPY, 0, 0, nNewPos , MAXCOL, MAXROW,
                                        nNewPos, 0, 0, nDz, NULL);

        pTab[nNewPos]->UpdateInsertTabAbs(nNewPos); // alle abs. um eins hoch!!
        pTab[nOldPos]->UpdateInsertTab(nNewPos);

        pTab[nOldPos]->UpdateCompile();
        pTab[nNewPos]->UpdateCompile( TRUE );   // #67996# maybe already compiled in Clone, but used names need recompilation
        SetNoListening( FALSE );
        pTab[nOldPos]->StartAllListeners();
        pTab[nNewPos]->StartAllListeners();
        SetDirty();
        SetAutoCalc( bOldAutoCalc );

        if (pDrawLayer)
            DrawCopyPage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

        pTab[nNewPos]->SetPageStyle( pTab[nOldPos]->GetPageStyle() );
    }
    else
        SetAutoCalc( bOldAutoCalc );
    return bValid;
}

ULONG ScDocument::TransferTab( ScDocument* pSrcDoc, SCTAB nSrcPos,
                                SCTAB nDestPos, BOOL bInsertNew,
                                BOOL bResultsOnly )
{
    ULONG nRetVal = 1;                      // 0 => Fehler 1 = ok
                                            // 2 => RefBox, 3 => NameBox
                                            // 4 => beides
    BOOL bValid = TRUE;
    if (bInsertNew)             // neu einfuegen
    {
        String aName;
        pSrcDoc->GetName(nSrcPos, aName);
        CreateValidTabName(aName);
        bValid = InsertTab(nDestPos, aName);
    }
    else                        // bestehende Tabelle ersetzen
    {
        if (VALIDTAB(nDestPos) && pTab[nDestPos])
        {
            pTab[nDestPos]->DeleteArea( 0,0, MAXCOL,MAXROW, IDF_ALL );
//          ClearDrawPage(nDestPos);
        }
        else
            bValid = FALSE;
    }

    if (bValid)
    {
        BOOL bOldAutoCalcSrc = FALSE;
        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
        SetNoListening( TRUE );
        if ( bResultsOnly )
        {
            bOldAutoCalcSrc = pSrcDoc->GetAutoCalc();
            pSrcDoc->SetAutoCalc( TRUE );   // falls was berechnet werden muss
        }
        SvNumberFormatter* pThisFormatter = xPoolHelper->GetFormTable();
        SvNumberFormatter* pOtherFormatter = pSrcDoc->xPoolHelper->GetFormTable();
        if (pOtherFormatter && pOtherFormatter != pThisFormatter)
        {
            SvNumberFormatterIndexTable* pExchangeList =
                     pThisFormatter->MergeFormatter(*(pOtherFormatter));
            if (pExchangeList->Count() > 0)
                pFormatExchangeList = pExchangeList;
        }
        nDestPos = Min(nDestPos, (SCTAB)(GetTableCount() - 1));
        {   // scope for bulk broadcast
            ScBulkBroadcast aBulkBroadcast( pBASM);
            pSrcDoc->pTab[nSrcPos]->CopyToTable(0, 0, MAXCOL, MAXROW,
                    ( bResultsOnly ? IDF_ALL & ~IDF_FORMULA : IDF_ALL),
                    FALSE, pTab[nDestPos] );
        }
        pFormatExchangeList = NULL;
        pTab[nDestPos]->SetTabNo(nDestPos);

        if ( !bResultsOnly )
        {
            BOOL bNamesLost = FALSE;
            USHORT nSrcRangeNames = pSrcDoc->pRangeName->GetCount();
            // array containing range names which might need update of indices
            ScRangeData** pSrcRangeNames = nSrcRangeNames ? new ScRangeData* [nSrcRangeNames] : NULL;
            // the index mapping thereof
            ScIndexMap aSrcRangeMap( nSrcRangeNames );
            BOOL bRangeNameReplace = FALSE;

            // find named ranges that are used in the source sheet
            std::set<USHORT> aUsedNames;
            pSrcDoc->pTab[nSrcPos]->FindRangeNamesInUse( 0, 0, MAXCOL, MAXROW, aUsedNames );

            for (USHORT i = 0; i < nSrcRangeNames; i++)     //! DB-Bereiche Pivot-Bereiche auch !!!
            {
                ScRangeData* pSrcData = (*pSrcDoc->pRangeName)[i];
                USHORT nOldIndex = pSrcData->GetIndex();
                bool bInUse = ( aUsedNames.find(nOldIndex) != aUsedNames.end() );
                if (bInUse)
                {
                    USHORT nExisting = 0;
                    if ( pRangeName->SearchName( pSrcData->GetName(), nExisting ) )
                    {
                        // the name exists already in the destination document
                        // -> use the existing name, but show a warning
                        // (when refreshing links, the existing name is used and the warning not shown)

                        ScRangeData* pExistingData = (*pRangeName)[nExisting];
                        USHORT nExistingIndex = pExistingData->GetIndex();

                        pSrcRangeNames[i] = NULL;       // don't modify the named range
                        aSrcRangeMap.SetPair( i, nOldIndex, nExistingIndex );
                        bRangeNameReplace = TRUE;
                        bNamesLost = TRUE;
                    }
                    else
                    {
                        ScRangeData* pData = new ScRangeData( *pSrcData );
                        pData->SetDocument(this);
                        if ( pRangeName->FindIndex( pData->GetIndex() ) )
                            pData->SetIndex(0);     // need new index, done in Insert
                        if (!pRangeName->Insert(pData))
                        {
                            DBG_ERROR("can't insert name");     // shouldn't happen
                            delete pData;
                        }
                        else
                        {
                            pData->TransferTabRef( nSrcPos, nDestPos );
                            pSrcRangeNames[i] = pData;
                            USHORT nNewIndex = pData->GetIndex();
                            aSrcRangeMap.SetPair( i, nOldIndex, nNewIndex );
                            if ( !bRangeNameReplace )
                                bRangeNameReplace = ( nOldIndex != nNewIndex );
                        }
                    }
                }
                else
                {
                    pSrcRangeNames[i] = NULL;
                    //aSrcRangeMap.SetPair( i, 0, 0 );      // not needed, defaulted
                }
            }
            if ( bRangeNameReplace )
            {
                // first update all inserted named formulas if they contain other
                // range names and used indices changed
                for (USHORT i = 0; i < nSrcRangeNames; i++)     //! DB-Bereiche Pivot-Bereiche auch
                {
                    if ( pSrcRangeNames[i] )
                        pSrcRangeNames[i]->ReplaceRangeNamesInUse( aSrcRangeMap );
                }
                // then update the formulas, they might need the just updated range names
                pTab[nDestPos]->ReplaceRangeNamesInUse( 0, 0, MAXCOL, MAXROW, aSrcRangeMap );
            }
            if ( pSrcRangeNames )
                delete [] pSrcRangeNames;

            SCsTAB nDz = ((SCsTAB)nDestPos) - (SCsTAB)nSrcPos;
            pTab[nDestPos]->UpdateReference(URM_COPY, 0, 0, nDestPos,
                                                     MAXCOL, MAXROW, nDestPos,
                                                     0, 0, nDz, NULL);
            // Test for outside absolute references for info box
            BOOL bIsAbsRef = pSrcDoc->pTab[nSrcPos]->TestTabRefAbs(nSrcPos);
            // Readjust self-contained absolute references to this sheet
            pTab[nDestPos]->TestTabRefAbs(nSrcPos);
            if (bIsAbsRef)
            {
                nRetVal += 1;
                    // InfoBox AbsoluteRefs sind moeglicherweise nicht mehr korrekt!!
            }
            if (bNamesLost)
            {
                nRetVal += 2;
                // message: duplicate names
            }
            pTab[nDestPos]->CompileAll();
        }

        SetNoListening( FALSE );
        if ( !bResultsOnly )
            pTab[nDestPos]->StartAllListeners();
        SetDirty( ScRange( 0, 0, nDestPos, MAXCOL, MAXROW, nDestPos));

        if ( bResultsOnly )
            pSrcDoc->SetAutoCalc( bOldAutoCalcSrc );
        SetAutoCalc( bOldAutoCalc );

        //  Drawing kopieren

        if (bInsertNew)
            TransferDrawPage( pSrcDoc, nSrcPos, nDestPos );
    }
    if (!bValid)
        nRetVal = 0;
    return nRetVal;
}

//  ----------------------------------------------------------------------------

void ScDocument::SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const USHORT nError)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetError( nCol, nRow, nError );
}

void ScDocument::EraseNonUsedSharedNames(USHORT nLevel)
{
    for (USHORT i = 0; i < pRangeName->GetCount(); i++)
    {
        ScRangeData* pRangeData = (*pRangeName)[i];
        if (pRangeData && pRangeData->HasType(RT_SHARED))
        {
            String aName;
            pRangeData->GetName(aName);
            aName.Erase(0, 6);                      // !!! vgl. Table4, FillFormula !!
            USHORT nInd = (USHORT) aName.ToInt32();
            if (nInd <= nLevel)
            {
                USHORT nIndex = pRangeData->GetIndex();
                BOOL bInUse = FALSE;
                for (SCTAB j = 0; !bInUse && (j <= MAXTAB); j++)
                {
                    if (pTab[j])
                        bInUse = pTab[j]->IsRangeNameInUse(0, 0, MAXCOL-1, MAXROW-1,
                                                           nIndex);
                }
                if (!bInUse)
                    pRangeName->AtFree(i);
            }
        }
    }
}

//  ----------------------------------------------------------------------------

void ScDocument::SetConsolidateDlgData( const ScConsolidateParam* pData )
{
    delete pConsolidateDlgData;

    if ( pData )
        pConsolidateDlgData = new ScConsolidateParam( *pData );
    else
        pConsolidateDlgData = NULL;
}

void ScDocument::SetChangeViewSettings(const ScChangeViewSettings& rNew)
{
    if (pChangeViewSettings==NULL)
        pChangeViewSettings = new ScChangeViewSettings;

    DBG_ASSERT( pChangeViewSettings, "Oops. No ChangeViewSettings :-( by!" );

    *pChangeViewSettings=rNew;
}

//  ----------------------------------------------------------------------------

ScFieldEditEngine* ScDocument::CreateFieldEditEngine()
{
    ScFieldEditEngine* pNewEditEngine = NULL;
    if (!pCacheFieldEditEngine)
    {
        pNewEditEngine = new ScFieldEditEngine( GetEnginePool(),
            GetEditPool(), FALSE );
    }
    else
    {
        if ( !bImportingXML )
        {
            // #i66209# previous use might not have restored update mode,
            // ensure same state as for a new EditEngine (UpdateMode = TRUE)
            if ( !pCacheFieldEditEngine->GetUpdateMode() )
                pCacheFieldEditEngine->SetUpdateMode(TRUE);
        }

        pNewEditEngine = pCacheFieldEditEngine;
        pCacheFieldEditEngine = NULL;
    }
    return pNewEditEngine;
}

void ScDocument::DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine)
{
    if (!pCacheFieldEditEngine && rpEditEngine)
    {
        pCacheFieldEditEngine = rpEditEngine;
        pCacheFieldEditEngine->Clear();
    }
    else
        delete rpEditEngine;
    rpEditEngine = NULL;
}

//  ----------------------------------------------------------------------------

// static
ScRecursionHelper* ScDocument::CreateRecursionHelperInstance()
{
    return new ScRecursionHelper;
}

//  ----------------------------------------------------------------------------

ScLookupCache & ScDocument::GetLookupCache( const ScRange & rRange )
{
    ScLookupCache* pCache = 0;
    if (!pLookupCacheMapImpl)
        pLookupCacheMapImpl = new ScLookupCacheMapImpl;
    ScLookupCacheMap::iterator it( pLookupCacheMapImpl->aCacheMap.find( rRange));
    if (it == pLookupCacheMapImpl->aCacheMap.end())
    {
        pCache = new ScLookupCache( this, rRange);
        AddLookupCache( *pCache);
    }
    else
        pCache = (*it).second;
    return *pCache;
}

void ScDocument::AddLookupCache( ScLookupCache & rCache )
{
    if (!pLookupCacheMapImpl->aCacheMap.insert( ::std::pair< const ScRange,
                ScLookupCache*>( rCache.getRange(), &rCache)).second)
    {
        DBG_ERRORFILE( "ScDocument::AddLookupCache: couldn't add to hash map");
    }
    else
        StartListeningArea( rCache.getRange(), &rCache);
}

void ScDocument::RemoveLookupCache( ScLookupCache & rCache )
{
    ScLookupCacheMap::iterator it( pLookupCacheMapImpl->aCacheMap.find(
                rCache.getRange()));
    if (it == pLookupCacheMapImpl->aCacheMap.end())
    {
        DBG_ERRORFILE( "ScDocument::RemoveLookupCache: range not found in hash map");
    }
    else
    {
        ScLookupCache* pCache = (*it).second;
        pLookupCacheMapImpl->aCacheMap.erase( it);
        EndListeningArea( pCache->getRange(), &rCache);
    }
}
