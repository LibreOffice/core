/*************************************************************************
 *
 *  $RCSfile: documen2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-30 11:33:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
//#define _STACK_HXX
//#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
//#define _SHL_HXX ***
//#define _LINK_HXX ***
//#define _ERRCODE_HXX ***
//#define _GEN_HXX ***
//#define _FRACT_HXX ***
//#define _STRING_HXX ***
//#define _MTF_HXX ***
//#define _CONTNR_HXX ***
//#define _LIST_HXX ***
//#define _TABLE_HXX ***
#define _DYNARY_HXX
//#define _UNQIDX_HXX ***
//#define _SVMEMPOOL_HXX ***
//#define _UNQID_HXX ***
//#define _DEBUG_HXX ***
//#define _DATE_HXX ***
//#define _TIME_HXX ***
//#define _DATETIME_HXX ***
//#define _INTN_HXX ***
//#define _WLDCRD_HXX ***
//#define _FSYS_HXX ***
//#define _STREAM_HXX ***
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX



#define _SV_NOXSOUND

#define _BASDLG_HXX
#define _CACHESTR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
//#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
#define _STATUS_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SELENG_HXX
#define _SETBRW_HXX
#define _SOUND_HXX
#define _STATUS_HXX
#define _STDMENU_HXX


//#define _PRNDLG_HXX ***
//#define _POLY_HXX **
// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
//#define _STACK_HXX
//#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX

//SV
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX

#ifndef OS2
#define _MENUBTN_HXX
#endif

//#define _SOBASE_HXX
//#define _SOSTOR_HXX
//#define _SOCORE_HXX
//#define _SOINPL_HXX

#define _SFX_DOCFILE_HXX
#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
#define _SFX_INTERNO_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
#define _SFXDISPATCH_HXX
#define _SFXDOCFILE_HXX
#define _SFXDOCMAN_HXX
#define _SFXDOCMGR_HXX
#define _SFXDOCTDLG_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

//#define _SI_HXX
//#define SI_NODRW
#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW         //
#define _SI_NOCONTROL
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
#define _VCATTR_HXX


#define _SVBOXITM_HXX
#define _SVCONTNR_HXX
#define _SVDIALDLL_HXX
//#define _SVDATTR_HXX
#define _SVDRAG_HXX
#define _SVDXOUT_HXX
#define _SVDEC_HXX
#define _SVDIO_HXX
//#define _SVDLAYER_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX


// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"

#include <svx/editeng.hxx>
#include <svx/langitem.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <vcl/system.hxx>
#include <comphelper/processfactory.hxx>

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

// STATIC DATA -----------------------------------------------------------

ScDocument::ScDocument( ScDocumentMode  eMode,
                        SfxObjectShell* pDocShell ) :
        xServiceManager( ::comphelper::getProcessServiceFactory() ),
        pDrawLayer( NULL ),
        pColorTable( NULL ),
        bOwner( FALSE ),
        pDocPool( NULL ),
        pStylePool( NULL ),
        pFormTable( NULL ),
        pShell( pDocShell ),
        pPrinter( NULL ),
        bAutoCalc( eMode == SCDOCMODE_DOCUMENT ),
        bAutoCalcShellDisabled( FALSE ),
        bForcedFormulaPending( FALSE ),
        bIsUndo( eMode == SCDOCMODE_UNDO ),
        bIsClip( eMode == SCDOCMODE_CLIP ),
        bCutMode( FALSE ),
        nMaxTableNumber( 0 ),
        pCondFormList( NULL ),
        pValidationList( NULL ),
        pFormatExchangeList( NULL ),
        bIsEmbedded( FALSE ),
        bProtected( FALSE ),
        pLinkManager( NULL ),
        pDocOptions( NULL ),
        pViewOptions( NULL ),
        pExtDocOptions( NULL ),
        pConsolidateDlgData( NULL ),
        pFormulaTree( NULL ),
        pEOFormulaTree( NULL ),
        aCurTextWidthCalcPos(MAXCOL,0,0),
//      bNoSetDirty( TRUE ),
        bNoSetDirty( FALSE ),
        pFormulaTrack( NULL ),
        pEOFormulaTrack( NULL ),
        nFormulaTrackCount(0),
        bInsertingFromOtherDoc( FALSE ),
        bImportingXML( FALSE ),
        nHardRecalcState(0),
        bCalcingAfterLoad( FALSE ),
        bNoListening( FALSE ),
        bLoadingDone( TRUE ),
        nVisibleTab( 0 ),
        bIdleDisabled( FALSE ),
        bInLinkUpdate( FALSE ),
        bDetectiveDirty( FALSE ),
        nMacroCallMode( SC_MACROCALL_ALLOWED ),
        bHasMacroFunc( FALSE ),
        bChartListenerCollectionNeedsUpdate( FALSE ),
        bHasForcedFormulas( FALSE ),
        nVisSpellState( 0 ),
        pClipData( NULL ),
        nFormulaCodeInTree(0),
        nInterpretLevel(0),
        nMacroInterpretLevel(0),
        bLostData(FALSE),
        pDetOpList(NULL),
        bInDtorClear( FALSE ),
        bExpandRefs( FALSE ),
        pUnoBroadcaster( NULL ),
        pChangeTrack( NULL ),
        pChangeViewSettings( NULL ),
        pEditPool( NULL ),
        pEnginePool( NULL ),
        pEditEngine( NULL ),
        eLinkMode(LM_UNKNOWN),
        pDPCollection( NULL )
{
    eSrcSet = gsl_getSystemTextEncoding();
    nSrcVer = SC_CURRENT_VERSION;
    nSrcMaxRow = MAXROW;

    if ( eMode == SCDOCMODE_DOCUMENT )
    {
        if ( pDocShell )
            pLinkManager = new SvxLinkManager( pDocShell );

        bOwner = TRUE;
        pDocPool = new ScDocumentPool;
        pDocPool->FreezeIdRanges();
        pStylePool = new ScStyleSheetPool( *pDocPool, this );
        pFormTable = new SvNumberFormatter( GetServiceManager(), ScGlobal::eLnge );
        aColorLink = LINK(this, ScDocument, GetUserDefinedColor);
        pFormTable->SetColorLink(&aColorLink);
        pFormTable->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
        pTab[0]  = NULL;
        pBASM = new ScBroadcastAreaSlotMachine( this );
        pChartListenerCollection = new ScChartListenerCollection( this );
        pEditPool = EditEngine::CreatePool();
        pEditPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
        pEditPool->FreezeIdRanges();
        pEnginePool = EditEngine::CreatePool();
        pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
        pEnginePool->FreezeIdRanges();
    }
    else
    {
        pDrawLayer  = NULL;
        bOwner      = FALSE;
        pDocPool    = NULL;
        pStylePool  = NULL;
        pFormTable  = NULL;
        pTab[0]     = NULL;
        pBASM       = NULL;
        pChartListenerCollection = NULL;
        pEditPool   = NULL;
        pEnginePool = NULL;
    }

    for (USHORT i=1; i<=MAXTAB; i++)
        pTab[i] = NULL;

    pRangeName = new ScRangeName( 4, 4, FALSE, this );
    pDBCollection = new ScDBCollection( 4, 4, FALSE, this );
    pPivotCollection = new ScPivotCollection(4, 4, this );
    pSelectionAttr = NULL;
    pChartCollection = new ScChartCollection;
    xColNameRanges = new ScRangePairList;
    xRowNameRanges = new ScRangePairList;
    ImplCreateOptions();
    SetLanguage( ScGlobal::eLnge );

    aTrackTimer.SetTimeoutHdl( LINK( this, ScDocument, TrackTimeHdl ) );
    aTrackTimer.SetTimeout( 100 );
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

IMPL_LINK( ScDocument, TrackTimeHdl, Timer*, pTimer )
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

    // Links aufrauemen

    if ( pLinkManager )
    {
        // BaseLinks freigeben
        for ( USHORT n = pLinkManager->GetServers().Count(); n; )
            ( (SvPseudoObject*) pLinkManager->GetServers()[ --n ])->Closed();

        if ( pLinkManager->GetLinks().Count() )
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
    }

    ScAddInAsync::RemoveDocument( this );
    ScAddInListener::RemoveDocument( this );
    delete pChartListenerCollection;    // vor pBASM wg. evtl. Listener!
    pChartListenerCollection = NULL;
    // BroadcastAreas vor allen Zellen zerstoeren um unnoetige
    // Einzel-EndListenings der Formelzellen zu vermeiden
    delete pBASM;       // BroadcastAreaSlotMachine
    pBASM = NULL;

    if (pUnoBroadcaster)
    {
        delete pUnoBroadcaster;     // broadcasted nochmal SFX_HINT_DYING
        pUnoBroadcaster = NULL;
    }

    Clear();

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
    delete pChartCollection;
//  delete pDrawLayer;
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
    delete pChangeViewSettings;         // und weg damit

    delete pDPCollection;

    if (bOwner)
    {
        BOOL bDoDelete = FALSE;
        if (bIsClip)
            bDoDelete = TRUE;
        else
        {
            ScDocument* pClipDoc = NULL;
            if (ScGlobal::HasClipDoc())                 // GetClipDoc legt das ClipDoc an
                pClipDoc = ScGlobal::GetClipDoc();
            if (pClipDoc && pClipDoc->pDocPool == pDocPool)
            {
                pClipDoc->bOwner = TRUE;
                pStylePool->SetDocument(pClipDoc);      // #67178# don't keep old document pointer
            }
            else
                bDoDelete = TRUE;
        }

        if (bDoDelete)
        {
            delete pStylePool;
            delete pDocPool;
            delete pFormTable;
            delete pEditPool;
            delete pEnginePool;
        }
    }

//  delete pColorTable;
    DeleteColorTable();
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
    if (bOwner)
    {
        delete pStylePool;
        delete pDocPool;
        delete pFormTable;
        delete pEditPool;
        delete pEnginePool;
    }
    bOwner = FALSE;
    pDocPool = pSourceDoc->pDocPool;
    pStylePool = pSourceDoc->pStylePool;
    pFormTable = pSourceDoc->pFormTable;
    pEditPool = pSourceDoc->pEditPool;
    pEnginePool = pSourceDoc->pEnginePool;

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
}

ScFieldEditEngine& ScDocument::GetEditEngine()
{
    if ( !pEditEngine )
    {
        pEditEngine = new ScFieldEditEngine( GetEditPool() );
        pEditEngine->SetUpdateMode( FALSE );
        pEditEngine->EnableUndo( FALSE );
        pEditEngine->SetRefMapMode( MAP_100TH_MM );
    }
    return *pEditEngine;
}

void ScDocument::ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        for (USHORT i = 0; i <= MAXTAB; i++)
            if (pSourceDoc->pTab[i])
                if (!pMarks || pMarks->GetTableSelect(i))
                {
                    String aString;
                    pSourceDoc->pTab[i]->GetName(aString);
                    pTab[i] = new ScTable(this, i, aString);
                    nMaxTableNumber = i+1;
                }
    }
    else
        DBG_ERROR("ResetClip");
}

void ScDocument::ResetClip( ScDocument* pSourceDoc, USHORT nTab )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("baeh")));
        nMaxTableNumber = nTab+1;
    }
    else
        DBG_ERROR("ResetClip");
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

void lcl_AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter )
{
    //  Sprache dann dazutueten, wenn ein Zahlformat mit einer anderen Sprache
    //  als im Parent (incl. Default) hart eingestellt ist

    DBG_ASSERT(rSet.GetItemState(ATTR_LANGUAGE_FORMAT,FALSE)==SFX_ITEM_DEFAULT,
                "AddLanguage: Sprache ist schon da ?!??!");

    const SfxPoolItem* pHardItem;
    if ( rSet.GetItemState( ATTR_VALUE_FORMAT, FALSE, &pHardItem ) == SFX_ITEM_SET )
    {
        const SvNumberformat* pHardFormat = rFormatter.GetEntry(
                            ((const SfxUInt32Item*)pHardItem)->GetValue() );

        ULONG nParentFmt = 0;                           // 0 ist Pool-Default
        const SfxItemSet* pParent = rSet.GetParent();
        if (pParent)
            nParentFmt = ((const SfxUInt32Item&)pParent->Get(ATTR_VALUE_FORMAT)).GetValue();
        const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );

        if ( pHardFormat && pParFormat &&
                pHardFormat->GetLanguage() != pParFormat->GetLanguage() )
            rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
    }
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
    USHORT nVerMaxRow = MAXROW_30;      // 8191, wenn in der Datei nichts steht
    USHORT nTab = 0;
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
                        rStream.ReadByteString( aProtectPass, rStream.GetStreamCharSet() );
                        if ( aFlagsHdr.BytesLeft() )
                        {
                            rStream >> nEnumDummy;
                            eLanguage = LanguageType( nEnumDummy );
                        }
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> bAutoCalc;
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVisibleTab;
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVersion;            // echte Version
                        if ( aFlagsHdr.BytesLeft() )
                            rStream >> nVerMaxRow;          // sonst auf 8191 lassen

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
                        rStream.SetStreamCharSet( eSrcSet );
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
                    LoadDrawLayer(rStream);
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
                        pFormTable->Load(rStream);
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
                        SfxItemSet* pSet = new SfxItemSet( *pDocPool,
                                SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                                SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
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
        DBG_ASSERT( pStylePool, "Oops. No StyleSheetPool :-(" );

        pStylePool->UpdateStdNames();   // falls mit Version in anderer Sprache gespeichert

        //  Zahlformat-Sprache
        //  (kann nicht in LoadPool passieren, weil der Numberformatter geladen sein muss)

        if ( pDocPool->GetLoadingVersion() == 0 )       // 0 = Pool-Version bis 3.1
        {
            //  in 3.1-Dokumenten gibt es ATTR_LANGUAGE_FORMAT noch nicht
            //  darum bei Bedarf zu ATTR_VALUE_FORMAT noch die Sprache dazutun
            //  (Bug #37441#)

            //  harte Attribute:

            USHORT nCount = pDocPool->GetItemCount(ATTR_PATTERN);
            ScPatternAttr* pPattern;
            for (USHORT i=0; i<nCount; i++)
            {
                pPattern = (ScPatternAttr*)pDocPool->GetItem(ATTR_PATTERN, i);
                if (pPattern)
                    lcl_AddLanguage( pPattern->GetItemSet(), *pFormTable );
            }

            //  Vorlagen:

            SfxStyleSheetIterator aIter( pStylePool, SFX_STYLE_FAMILY_PARA );
            for ( SfxStyleSheetBase* pStyle = aIter.First(); pStyle; pStyle = aIter.Next() )
                lcl_AddLanguage( pStyle->GetItemSet(), *pFormTable );
        }

        //  Druckbereiche etc.

        SfxStyleSheetIterator   aIter( pStylePool, SFX_STYLE_FAMILY_PAGE );
        ScStyleSheet*           pStyleSheet = NULL;

        nMaxTableNumber = 0;
        for (USHORT i=0; i<=MAXTAB; i++)
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
                        {
                            SetPrintRangeCount( i, 1 );
                            SetPrintRange( i, 0, pPrintAreaItem->GetRange() );
                        }
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
        static BOOL bShown = 0;
        if ( !bShown && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
        {
            bShown = 1;
            DBG_ERRORFILE( "bei inkompatiblem FileFormat ChartListenerCollection speichern!" );
        }
#endif
        UpdateChartListenerCollection();
        if (pDrawLayer)
            RefreshNoteFlags();
        CalcAfterLoad();
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

    USHORT i;
    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );

    CharSet eOldSet = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( gsl_getSystemTextEncoding() );

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
            rStream.WriteByteString( aProtectPass, rStream.GetStreamCharSet() );
            rStream << (USHORT) eLanguage;
            rStream << bAutoCalc;

            rStream << nVisibleTab;

            // und hier jetzt die echte Versionsnummer
            rStream << (USHORT) nSrcVer;

            rStream << nSrcMaxRow;                  // Zeilenanzahl
        }

        //  Zeichensatz

        {
            rStream << (USHORT) SCID_CHARSET;
            ScWriteHeader aSetHdr( rStream, 2 );
            rStream << (BYTE) 0     // dummy, old System::GetGUIType()
                    << (BYTE) ::GetStoreCharSet( gsl_getSystemTextEncoding() );
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
            pFormTable->Save(rStream);
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

        for (i=0; i<=MAXTAB; i++)
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

//      if (pDrawLayer && pDrawLayer->HasObjects())
        //  auch ohne Objekte - wegen ColorTable etc.

        if (pDrawLayer)
        {
            rStream << (USHORT) SCID_DRAWING;
            StoreDrawLayer(rStream);
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

void ScDocument::DeleteNumberFormat( const ULONG* pDelKeys, ULONG nCount )
{
/*
    for (ULONG i = 0; i < nCount; i++)
        pFormTable->DeleteEntry(pDelKeys[i]);
*/
}

void ScDocument::PutCell( USHORT nCol, USHORT nRow, USHORT nTab,
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
    USHORT nTab = rPos.Tab();
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

BOOL ScDocument::GetPrintArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow,
                                BOOL bNotes ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
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

BOOL ScDocument::GetPrintAreaHor( USHORT nTab, USHORT nStartRow, USHORT nEndRow,
                                        USHORT& rEndCol, BOOL bNotes ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
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

BOOL ScDocument::GetPrintAreaVer( USHORT nTab, USHORT nStartCol, USHORT nEndCol,
                                        USHORT& rEndRow, BOOL bNotes ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
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

BOOL ScDocument::GetDataStart( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
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

BOOL ScDocument::MoveTab( USHORT nOldPos, USHORT nNewPos )
{
    if (nOldPos == nNewPos) return FALSE;
    BOOL bValid = FALSE;
    if (VALIDTAB(nOldPos))
    {
        if (pTab[nOldPos])
        {
            USHORT nTabCount = GetTableCount();
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

                short nDz = ((short)nNewPos) - (short)nOldPos;
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
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_REORDER,
                                    aSourceRange, 0,0,nDz ) );

                ScTable* pSaveTab = pTab[nOldPos];
                USHORT i;
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
                SetDirty();
                SetAutoCalc( bOldAutoCalc );

                if (pDrawLayer)
                    DrawMovePage( nOldPos, nNewPos );

                bValid = TRUE;
            }
        }
    }
    return bValid;
}

BOOL ScDocument::CopyTab( USHORT nOldPos, USHORT nNewPos, const ScMarkData* pOnlyMarked )
{
    if (SC_TAB_APPEND == nNewPos ) nNewPos = nMaxTableNumber;
    String aName;
    GetName(nOldPos, aName);

    //  vorneweg testen, ob der Prefix als gueltig erkannt wird
    //  wenn nicht, nur doppelte vermeiden
    BOOL bPrefix = ValidTabName( aName );
    DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
    USHORT nDummy;

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

                USHORT i;
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
        short nDz;
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
            DrawCopyPage( nOldPos, nNewPos );

        pTab[nNewPos]->SetPageStyle( pTab[nOldPos]->GetPageStyle() );
    }
    else
        SetAutoCalc( bOldAutoCalc );
    return bValid;
}

ULONG ScDocument::TransferTab( ScDocument* pSrcDoc, USHORT nSrcPos,
                                USHORT nDestPos, BOOL bInsertNew,
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
        BOOL bOldAutoCalcSrc;
        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
        SetNoListening( TRUE );
        if ( bResultsOnly )
        {
            bOldAutoCalcSrc = pSrcDoc->GetAutoCalc();
            pSrcDoc->SetAutoCalc( TRUE );   // falls was berechnet werden muss
        }
        if (pSrcDoc->pFormTable && pSrcDoc->pFormTable != pFormTable)
        {
            SvULONGTable* pExchangeList =
                     pFormTable->MergeFormatter(*(pSrcDoc->pFormTable));
            if (pExchangeList->Count() > 0)
                pFormatExchangeList = pExchangeList;
        }
        nDestPos = Min(nDestPos, (USHORT)(GetTableCount() - 1));
        pSrcDoc->pTab[nSrcPos]->CopyToTable(0, 0, MAXCOL, MAXROW,
            ( bResultsOnly ? IDF_ALL & ~IDF_FORMULA : IDF_ALL),
            FALSE, pTab[nDestPos] );
        pFormatExchangeList = NULL;
        pTab[nDestPos]->SetTabNo(nDestPos);

        if ( !bResultsOnly )
        {
            USHORT nSrcRangeNames = pSrcDoc->pRangeName->GetCount();
            // array containing range names which might need update of indices
            ScRangeData** pSrcRangeNames = nSrcRangeNames ? new ScRangeData* [nSrcRangeNames] : NULL;
            // the index mapping thereof
            ScIndexMap aSrcRangeMap( nSrcRangeNames );
            BOOL bRangeNameReplace = FALSE;

            for (USHORT i = 0; i < nSrcRangeNames; i++)     //! DB-Bereiche Pivot-Bereiche auch !!!
            {
                ScRangeData* pSrcData = (*pSrcDoc->pRangeName)[i];
                USHORT nOldIndex = pSrcData->GetIndex();
                BOOL bInUse = FALSE;
                for (USHORT j = 0; !bInUse && (j <= MAXTAB); j++)
                {
                    if (pSrcDoc->pTab[j])
                        bInUse = pSrcDoc->pTab[j]->IsRangeNameInUse(0, 0, MAXCOL, MAXROW,
                                                           nOldIndex);
                }
                if (bInUse)
                {
                    ScRangeData* pData = new ScRangeData( *pSrcData );
                    pData->SetDocument(this);
                    if ( pRangeName->FindIndex( pData->GetIndex() ) )
                        pData->SetIndex(0);     // need new index, done in Insert
                    if (!pRangeName->Insert(pData))
                    {
                        delete pData;
                        nRetVal += 2;
                        // InfoBox :: Name doppelt
                        pSrcRangeNames[i] = NULL;
                        aSrcRangeMap.SetPair( i, nOldIndex, 0 );
                        bRangeNameReplace = TRUE;
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

            short nDz = ((short)nDestPos) - (short)nSrcPos;
            pTab[nDestPos]->UpdateReference(URM_COPY, 0, 0, nDestPos,
                                                     MAXCOL, MAXROW, nDestPos,
                                                     0, 0, nDz, NULL);
            BOOL bIsAbsRef = pTab[nDestPos]->TestTabRefAbs(nSrcPos); // eigene Ref retten
            if (bIsAbsRef)
            {
                nRetVal += 1;
                    // InfoBox AbsoluteRefs sind möglicherweise nicht mehr korrekt!!
            }
            pTab[nDestPos]->CompileAll();
        }

        SetNoListening( FALSE );
        if ( !bResultsOnly )
            pTab[nDestPos]->StartAllListeners();
        SetDirty();     // ist das wirklich dokumentweit noetig?!?
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

void ScDocument::SetError( USHORT nCol, USHORT nRow, USHORT nTab, const USHORT nError)
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
                for (USHORT j = 0; !bInUse && (j <= MAXTAB); j++)
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





