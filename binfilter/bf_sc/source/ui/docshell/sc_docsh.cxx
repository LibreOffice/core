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
// System - Includes -----------------------------------------------------

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "scitems.hxx"

#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_sfx2/sfx.hrc>
#include <bf_offmgr/app.hxx>
#include <comphelper/classids.hxx>
#include <bf_sfx2/request.hxx>

#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

// INCLUDE ---------------------------------------------------------------

#include "viewdata.hxx"
#include "filter.hxx"
#include "scmod.hxx"
#include "docfunc.hxx"
#include "impex.hxx"
#include "scresid.hxx"
#include "bf_sc.hrc"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "stlpool.hxx"
#include "autostyl.hxx"
#include "asciiopt.hxx"
#include "docpool.hxx"		// LoadCompleted
#include "progress.hxx"
#include "pntlock.hxx"
#include "docuno.hxx"
#include "appoptio.hxx"
#include "detdata.hxx"
#include "printfun.hxx"
#include "chartlis.hxx"
#include "hints.hxx"
#include "xmlwrap.hxx"
#include "drwlayer.hxx"
#include "dbcolect.hxx"

#include "docsh.hxx"
#include "ViewSettingsSequenceDefines.hxx"

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
namespace binfilter {
using namespace ::com::sun::star;

#ifndef SO2_DECL_SVSTORAGESTREAM_DEFINED
#define SO2_DECL_SVSTORAGESTREAM_DEFINED
SO2_DECL_REF(SvStorageStream)
#endif

// STATIC DATA -----------------------------------------------------------

//	Stream-Namen im Storage

const sal_Char __FAR_DATA ScDocShell::pStarCalcDoc[] = STRING_SCSTREAM;		// "StarCalcDocument"
const sal_Char __FAR_DATA ScDocShell::pStyleName[] = "SfxStyleSheets";

//	Filter-Namen (wie in sclib.cxx)

static const sal_Char __FAR_DATA pFilterSc50[]		= "StarCalc 5.0";
//static const sal_Char __FAR_DATA pFilterSc50Temp[]	= "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]		= "StarCalc 4.0";
//static const sal_Char __FAR_DATA pFilterSc40Temp[]	= "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]		= "StarCalc 3.0";
//static const sal_Char __FAR_DATA pFilterSc30Temp[]	= "StarCalc 3.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc10[]		= "StarCalc 1.0";
static const sal_Char __FAR_DATA pFilterXML[]		= "StarOffice XML (Calc)";
static const sal_Char __FAR_DATA pFilterAscii[]		= "Text - txt - csv (StarCalc)";
static const sal_Char __FAR_DATA pFilterLotus[]		= "Lotus";
static const sal_Char __FAR_DATA pFilterExcel4[]	= "MS Excel 4.0";
static const sal_Char __FAR_DATA pFilterEx4Temp[]	= "MS Excel 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel5[]	= "MS Excel 5.0/95";
static const sal_Char __FAR_DATA pFilterEx5Temp[]	= "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel95[]	= "MS Excel 95";
static const sal_Char __FAR_DATA pFilterEx95Temp[]	= "MS Excel 95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel97[]	= "MS Excel 97";
static const sal_Char __FAR_DATA pFilterEx97Temp[]	= "MS Excel 97 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterDBase[]		= "dBase";
static const sal_Char __FAR_DATA pFilterDif[]		= "DIF";
static const sal_Char __FAR_DATA pFilterSylk[]		= "SYLK";
static const sal_Char __FAR_DATA pFilterHtml[]		= "HTML (StarCalc)";
static const sal_Char __FAR_DATA pFilterHtmlWebQ[]	= "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]		= "Rich Text Format (StarCalc)";

//----------------------------------------------------------------------

//	GlobalName der aktuellen Version:
/*N*/ SFX_IMPL_OBJECTFACTORY_DLL(ScDocShell, SFXOBJECTSHELL_STD_NORMAL,
/*N*/ 							SvGlobalName(BF_SO3_SC_CLASSID), Sc)

/*N*/ TYPEINIT1( ScDocShell, SfxObjectShell );		// SfxInPlaceObject: kein Type-Info ?

//------------------------------------------------------------------

/*N*/ void __EXPORT ScDocShell::FillClass( SvGlobalName* pClassName,
/*N*/ 										ULONG* pFormat,
/*N*/ 										String* pAppName,
/*N*/ 										String* pFullTypeName,
/*N*/ 										String* pShortTypeName,
/*N*/ 										long nFileFormat ) const
/*N*/ {
/*N*/ 	SfxInPlaceObject::FillClass( pClassName, pFormat, pAppName,
/*N*/ 								pFullTypeName, pShortTypeName, nFileFormat );
/*N*/ 
/*N*/ 	if ( nFileFormat == SOFFICE_FILEFORMAT_31 )
/*N*/ 	{
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SC_CLASSID_30 );
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARCALC_30;
/*N*/ 		*pAppName		= String( ScResId( SCSTR_30_APPLICATION ) );
/*N*/ 		*pFullTypeName	= String( ScResId( SCSTR_30_LONG_DOCNAME ) );
/*N*/ 		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
/*N*/ 	}
/*N*/ 	else if ( nFileFormat == SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SC_CLASSID_40 );
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARCALC_40;
/*N*/ 		*pAppName		= String( ScResId( SCSTR_40_APPLICATION ) );
/*N*/ 		*pFullTypeName	= String( ScResId( SCSTR_40_LONG_DOCNAME ) );
/*N*/ 		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
/*N*/ 	}
/*N*/ 	else if ( nFileFormat == SOFFICE_FILEFORMAT_50 )
/*N*/ 	{
/*N*/ 		*pClassName		= SvGlobalName( BF_SO3_SC_CLASSID_50 );
/*N*/ 		*pFormat		= SOT_FORMATSTR_ID_STARCALC_50;
/*N*/ 		*pAppName		= String( ScResId( SCSTR_50_APPLICATION ) );
/*N*/ 		*pFullTypeName	= String( ScResId( SCSTR_50_LONG_DOCNAME ) );
/*N*/ 		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
/*N*/ 	}
/*N*/ 	else if ( nFileFormat == SOFFICE_FILEFORMAT_60 ||
                  nFileFormat == SOFFICE_FILEFORMAT_CURRENT )   // #i41083# also allow CURRENT
/*N*/ 	{
/*N*/ 		// for binfilter, we need the FormatIDs to be set. Not setting them
            // has always been an error (!)
/*N*/ 	 	*pClassName		= SvGlobalName( BF_SO3_SC_CLASSID_60 );
/*N*/ 		*pFormat        = SOT_FORMATSTR_ID_STARCALC_60;
/*N*/ 	 	*pAppName		= String( ScResId( SCSTR_APPLICATION ) );
/*N*/ 
/*N*/ 		*pFullTypeName	= String( ScResId( SCSTR_LONG_SCDOC_NAME ) );
/*N*/ 		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );

/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("wat fuer ne Version?");
/*N*/ }

//------------------------------------------------------------------


//------------------------------------------------------------------


//------------------------------------------------------------------

/*N*/ BOOL ScDocShell::LoadCalc( SvStorage* pStor )		// StarCalc 3, 4 or 5 file
/*N*/ {
/*N*/ 	//	MacroCallMode is no longer needed, state is kept in SfxObjectShell now
/*N*/ 
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 
/*N*/ 	SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName), STREAM_STD_READ );
/*N*/ 	SvStorageStreamRef aDocStm  = pStor->OpenStream( String::CreateFromAscii(pStarCalcDoc), STREAM_STD_READ );
/*N*/ 	ULONG nPoolErr = aPoolStm->GetError();
/*N*/ 	ULONG nDocErr  = aDocStm->GetError();
/*N*/ 
/*N*/ 	ScProgress* pProgress = NULL;
/*N*/ 	SfxObjectCreateMode eShellMode = GetCreateMode();
/*N*/ 	if ( eShellMode == SFX_CREATE_MODE_STANDARD && !nDocErr )
/*N*/ 	{
/*N*/ 		ULONG nCurPos = aDocStm->Tell();
/*N*/ 		ULONG nEndPos = aDocStm->Seek( STREAM_SEEK_TO_END );
/*N*/ 		aDocStm->Seek( nCurPos );
/*N*/ 		ULONG nRange = nEndPos - nCurPos;
/*N*/ 		pProgress = new ScProgress( this, ScGlobal::GetRscString(STR_LOAD_DOC), nRange );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nPoolErr == ERRCODE_IO_NOTEXISTS && nDocErr == ERRCODE_IO_NOTEXISTS )
/*N*/ 	{
/*?*/ 		//	leerer Storage = leeres Dokument -> wie InitNew
/*?*/ 
/*?*/ 		aDocument.MakeTable(0);
/*?*/ 		Size aSize( (long) ( STD_COL_WIDTH			 * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
/*?*/ 					(long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
/*?*/ 		// hier muss auch der Start angepasst werden
/*?*/ 		SetVisAreaOrSize( Rectangle( Point(), aSize ), TRUE );
/*?*/ 		aDocument.GetStyleSheetPool()->CreateStandardStyles();
/*?*/ 		aDocument.UpdStlShtPtrsFrmNms();
/*N*/ 	}
/*N*/ 	else if ( !nPoolErr && !nDocErr )
/*N*/ 	{
/*N*/ 		aPoolStm->SetVersion(pStor->GetVersion());
/*N*/ 		aDocStm->SetVersion(pStor->GetVersion());
/*N*/ 
/*N*/ 		aDocument.Clear();			// keine Referenzen auf Pool behalten!
/*N*/ 
/*N*/ 		RemoveItem( SID_ATTR_CHAR_FONTLIST );
/*N*/ 		RemoveItem( ITEMID_COLOR_TABLE );
/*N*/ 		RemoveItem( ITEMID_GRADIENT_LIST );
/*N*/ 		RemoveItem( ITEMID_HATCH_LIST );
/*N*/ 		RemoveItem( ITEMID_BITMAP_LIST );
/*N*/ 		RemoveItem( ITEMID_DASH_LIST );
/*N*/ 		RemoveItem( ITEMID_LINEEND_LIST );
/*N*/ 
/*N*/ 		aDocument.LoadPool( *aPoolStm, FALSE );		// FALSE: RefCounts nicht laden
/*N*/ 		bRet = (aPoolStm->GetError() == 0);
/*N*/ 
/*N*/ 		if ( !bRet )
/*N*/ 		{
/*N*/ 			pStor->SetError( aPoolStm->GetError() );
/*N*/ 			DBG_ERROR( "Fehler im Pool-Stream" );
/*N*/ 		}
/*N*/ 		else if (eShellMode != SFX_CREATE_MODE_ORGANIZER)
/*N*/ 		{
/*N*/ 			//	ViewOptions are not completely stored (to avoid warnings),
/*N*/ 			//	so must be initialized from global settings.
/*N*/ 			//!	This can be removed if a new file format loads all ViewOptions !!!
/*N*/ 
/*N*/ 			aDocument.SetViewOptions( SC_MOD()->GetViewOptions() );
/*N*/ 
/*N*/ 			ULONG nErrCode = aPoolStm->GetErrorCode();
/*N*/ 			const ByteString aStrKey = pStor->GetKey();
/*N*/ 			const BOOL	 bKeySet = (aStrKey.Len() > 0);
/*N*/ 
/*N*/ 			if ( bKeySet )
/*?*/ 				aDocStm->SetKey( aStrKey );			// Passwort setzen
/*N*/ 
/*N*/ 			bRet = aDocument.Load( *aDocStm, pProgress );
/*N*/ 
/*N*/ 			if ( !bRet && bKeySet ) // Passwort falsch
/*N*/ 			{
/*?*/ 				SetError( ERRCODE_SFX_WRONGPASSWORD );
/*?*/ 				pStor->SetError( ERRCODE_SFX_WRONGPASSWORD );
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( aDocStm->GetError() )
/*N*/ 			{
/*?*/ 				//	Zeile-8192-Meldung braucht nur zu kommen, wenn die Zeilen
/*?*/ 				//	beschraenkt sind, sonst ist "falsches Format" besser
/*?*/ #ifdef SC_LIMIT_ROWS
/*?*/ 				if ( aDocument.HasLostData() )					// zuviele Zeilen?
/*?*/ 					pStor->SetError( SCERR_IMPORT_8K_LIMIT );
/*?*/ 				else
/*?*/ #endif
/*?*/ 					pStor->SetError( aDocStm->GetError() );
/*?*/ 				bRet = FALSE;
/*?*/ 				DBG_ERROR( "Fehler im Document-Stream" );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if ( !(nErrCode & ERRCODE_WARNING_MASK) )
/*N*/ 					nErrCode = aDocStm->GetErrorCode();
/*N*/ 				if ( nErrCode & ERRCODE_WARNING_MASK )
/*?*/ 					SetError( nErrCode );
/*N*/ 			}
/*N*/ 
/*N*/ 			if (bRet)
/*N*/ 			{
/*N*/ 				aDocument.UpdateFontCharSet();
/*N*/ 
/*N*/ 				UpdateLinks();				// verknuepfte Tabellen in Link-Manager
/*N*/ 				RemoveUnknownObjects();		// unbekannte Ole-Objekte loeschen

                    // #116578# Manually load view state from sfx window data (no view is created).
                    // See SfxObjectShell::LoadWindows_Impl.
                    // Only the user data string is used, only from the first view.

                    SvStorageStreamRef aWinStm  = pStor->OpenStream(
                            String::CreateFromAscii("SfxWindows"), STREAM_STD_READ );
                    if ( aWinStm.Is() && aWinStm->GetError() == ERRCODE_NONE )
                    {
                        aWinStm->SetBufferSize(1024);
                        String aWinData;
                        aWinStm->ReadByteString( aWinData );
                        if ( aWinData.Len() )
                        {
                            char cToken =',';
                            BOOL bOldFormat = TRUE;
                            if ( aWinData.GetToken( 0, cToken ).EqualsAscii( "TASK" ) )
                            {
                                bOldFormat = FALSE;
                                aWinStm->ReadByteString( aWinData );    // read next string
                            }
                            USHORT nViewId = (USHORT) aWinData.GetToken( 0, cToken ).ToInt32();

                            // only ScTabViewShell is handled (ID as in ScDLL::Init)
                            if ( nViewId == 1 )
                            {
                                if ( bOldFormat )
                                {
                                    aUserData = aWinData.GetToken( 2, cToken );
                                }
                                else
                                {
                                    USHORT nPos=0;
                                    FASTBOOL bActive = aWinData.GetToken( 3, cToken, nPos ).ToInt32();
                                    aUserData = aWinData.Copy( nPos );
                                }
                                // aUserData is used in ScModelObj::getViewData
                            }
                        }
                    }
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else			// SFX_CREATE_MODE_ORGANIZER
/*N*/ 		{
/*?*/ 			//	UpdateStdNames is called from ScDocument::Load, but is also needed
/*?*/ 			//	if only the styles are loaded!
/*?*/ 			ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
/*?*/ 			if (pStylePool)
/*?*/ 				pStylePool->UpdateStdNames();	// correct style names for different languages
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bRet)
/*N*/ 			aDocument.GetPool()->MyLoadCompleted();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		DBG_ERROR( "Stream-Fehler");
/*?*/ 		bRet = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!aDocument.HasTable(0))
/*?*/ 		aDocument.MakeTable(0);
/*N*/ 
/*N*/ 	if (eShellMode == SFX_CREATE_MODE_EMBEDDED)
/*N*/ 	{
/*N*/ 		USHORT nVisTab = aDocument.GetVisibleTab();
/*N*/ 		BOOL bHasVis = aDocument.HasTable(nVisTab);
/*N*/ 		if ( SfxInPlaceObject::GetVisArea().IsEmpty() || !bHasVis )
/*N*/ 		{
/*?*/ 			if (!bHasVis)
/*?*/ 			{
/*?*/ 				nVisTab = 0;
/*?*/ 				aDocument.SetVisibleTab(nVisTab);
/*?*/ 			}
/*?*/ 			USHORT nStartCol,nStartRow;
/*?*/ 			aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
/*?*/ 			USHORT nEndCol,nEndRow;
/*?*/ 			aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
/*?*/ 			if (nStartCol>nEndCol)
/*?*/ 				nStartCol = nEndCol;
/*?*/ 			if (nStartRow>nEndRow)
/*?*/ 				nStartRow = nEndRow;
/*?*/ 			// hier muss auch der Start angepasst werden
/*?*/ 			SetVisAreaOrSize( aDocument.GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab ),
/*?*/ 								TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	delete pProgress;
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL ScDocShell::SaveCalc( SvStorage* pStor )			// Calc 3, 4 or 5 file
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 
/*N*/ 	ScProgress* pProgress = NULL;
/*N*/ 	SfxObjectCreateMode eShellMode = GetCreateMode();
/*N*/ 	if ( eShellMode == SFX_CREATE_MODE_STANDARD )
/*N*/ 	{
/*N*/ 		ULONG nRange = aDocument.GetWeightedCount() + 1;
/*N*/ 		pProgress = new ScProgress( this, ScGlobal::GetRscString(STR_SAVE_DOC), nRange );
/*N*/ 	}
/*N*/ 
/*N*/ 	SvStorageStreamRef aPoolStm = pStor->OpenStream( String::CreateFromAscii(pStyleName) );
/*N*/ 	if( !aPoolStm->GetError() )
/*N*/ 	{
/*N*/ 		aPoolStm->SetVersion(pStor->GetVersion());
/*N*/ 		aPoolStm->SetSize(0);
/*N*/ 		bRet = aDocument.SavePool( *aPoolStm );
/*N*/ 		if ( aPoolStm->GetErrorCode() && !pStor->GetErrorCode() )
/*?*/ 			pStor->SetError(aPoolStm->GetErrorCode());
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		DBG_ERROR( "Stream Error" );
/*?*/ 		bRet = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bRet && eShellMode != SFX_CREATE_MODE_ORGANIZER )
/*N*/ 	{
/*N*/ 		SvStorageStreamRef aDocStm  = pStor->OpenStream( String::CreateFromAscii(pStarCalcDoc) );
/*N*/ 		if( !aDocStm->GetError() )
/*N*/ 		{
/*N*/ 			aDocStm->SetVersion(pStor->GetVersion());
/*N*/ 			aDocStm->SetKey(pStor->GetKey());				// Passwort setzen
/*N*/ 			aDocStm->SetSize(0);
/*N*/ 			bRet = aDocument.Save( *aDocStm, pProgress );
/*N*/ 			DBG_ASSERT( bRet, "Error while saving" );
/*N*/ 
/*N*/ 			if ( aDocument.HasLostData() )
/*N*/ 			{
/*?*/ 				//	Warnung, dass nicht alles gespeichert wurde
/*?*/ 
/*?*/ 				if (!pStor->GetError())
/*?*/ 					pStor->SetError(SCWARN_EXPORT_MAXROW);
/*?*/ 			}
/*N*/ 			else if ( aDocStm->GetErrorCode() && !pStor->GetErrorCode() )
/*?*/ 				pStor->SetError(aDocStm->GetErrorCode());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			DBG_ERROR( "Stream Error" );
/*?*/ 			bRet = FALSE;
/*N*/ 		}

            // #116578# Manually create sfx window data from uno view data (no view is created).
            // See SfxObjectShell::SaveWindows_Impl.
            // Only data from the first view is used.

            uno::Reference<document::XViewDataSupplier> xSupplier( GetModel(), uno::UNO_QUERY );
            if ( xSupplier.is() && eShellMode == SFX_CREATE_MODE_STANDARD )
            {
                uno::Reference<container::XIndexAccess> xData = xSupplier->getViewData();
                if ( xData.is() && xData->getCount() >= 1  )
                {
                    uno::Sequence<beans::PropertyValue> aSeq;
                    uno::Any aAny = xData->getByIndex( 0 );
                    if ( aAny >>= aSeq )
                    {
                        sal_Int32 nViewId = 0;

                        sal_Int32 nCount = aSeq.getLength();
                        for (sal_Int32 i = 0; i < nCount; i++)
                            if ( aSeq[i].Name.compareToAscii(SC_VIEWID) == 0 )
                            {
                                ::rtl::OUString aId;
                                aSeq[i].Value >>= aId;
                                String aTmp( aId );
                                aTmp.Erase( 0, 4 );  // format is like in "view3"
                                nViewId = aTmp.ToInt32();
                            }

                        if ( nViewId == 1 )         // ScTabViewShell, ID from ScDLL::Init
                        {
                            String aUserStr;
                            ScViewData aLocalViewData( this );
                            aLocalViewData.ReadUserDataSequence( aSeq );
                            aLocalViewData.WriteUserData( aUserStr );

                            if ( aUserStr.Len() )
                            {
                                SvStorageStreamRef aWinStm  = pStor->OpenStream(
                                        String::CreateFromAscii("SfxWindows"), STREAM_TRUNC | STREAM_STD_READWRITE );
                                if ( aWinStm.Is() && aWinStm->GetError() == ERRCODE_NONE )
                                {
                                    aWinStm->SetBufferSize(1024);

                                    char cToken = ',';
                                    String aWinData = String::CreateFromInt32( nViewId );
                                    aWinData += cToken;
                                    aWinData += cToken;
                                    aWinData += aUserStr;
                                    aWinData += cToken;
                                    aWinData += '1';        // active

                                    aWinStm->WriteByteString( aWinData );
                                }
                            }
                        }
                    }
                }
            }
/*N*/ 	}
/*N*/ 
/*N*/ 	delete pProgress;
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void ScDocShell::BeforeXMLLoading()
/*N*/ {
/*N*/ 	// prevent unnecessary broadcasts and updates
/*N*/     DBG_ASSERT(pModificator == NULL, "The Modificator should not exist");
/*N*/ 	pModificator = new ScDocShellModificator( *this );
/*N*/ 	
/*N*/     aDocument.SetImportingXML( TRUE );
/*N*/ 	// prevent unnecessary broadcasts and "half way listeners"
/*N*/ 	aDocument.SetInsertingFromOtherDoc( TRUE );
/*N*/ 
/*N*/ 	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*N*/ 		ScColumn::bDoubleAlloc = sal_True;
/*N*/ }

/*N*/ void ScDocShell::AfterXMLLoading(sal_Bool bRet)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
            aDocument.SetInsertingFromOtherDoc( FALSE );
            aDocument.SetImportingXML( FALSE );

            if (pModificator)
            {
                delete pModificator;
                pModificator = NULL;
            }
            else
                DBG_ERROR("The Modificator should exist");
/*N*/ }

/*N*/ BOOL ScDocShell::LoadXML( SfxMedium* pMedium, SvStorage* pStor )
/*N*/ {
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::LoadXML" );
/*N*/ 
/*N*/ 	//	MacroCallMode is no longer needed, state is kept in SfxObjectShell now
/*N*/ 
/*N*/ 	// no Seek(0) here - always loading from storage, GetInStream must not be called
/*N*/ 
/*N*/     BeforeXMLLoading();
/*N*/ 
/*N*/ 	ScXMLImportWrapper aImport( aDocument, pMedium, pStor );
/*N*/ 
/*N*/     sal_Bool bRet(sal_False);
/*N*/ 	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*N*/ 		bRet = aImport.Import(sal_False);
/*N*/ 	else
/*N*/ 		bRet = aImport.Import(sal_True);
/*N*/ 
/*N*/     AfterXMLLoading(bRet);
/*N*/ 
/*N*/ 	//!	row heights...
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL ScDocShell::SaveXML( SfxMedium* pMedium, SvStorage* pStor )
/*N*/ {
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::SaveXML" );
/*N*/ 
/*N*/ 	ScXMLImportWrapper aImport( aDocument, pMedium, pStor );
/*N*/ 	sal_Bool bRet(sal_False);
/*N*/ 	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
/*N*/ 		bRet = aImport.Export(sal_False);
/*N*/ 	else
/*N*/ 		bRet = aImport.Export(sal_True);
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL __EXPORT ScDocShell::Load( SvStorage* pStor )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Load" );
/*N*/ 
/*N*/ 	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/ 
/*N*/ 	DBG_ASSERT( pStor, "Load without storage?" );
/*N*/ 	BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );
/*N*/ 
/*N*/ 	//	only the latin script language is loaded
/*N*/ 	//	-> initialize the others from options (before loading)
/*N*/ 	InitOptions();
/*N*/ 
/*N*/ 	BOOL bRet = SfxInPlaceObject::Load( pStor );
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/         if (GetMedium())
/*N*/         {
/*N*/             SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*N*/             nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*N*/         }
/*N*/ 
/*N*/         if (bXML)
/*N*/ 		{
/*N*/ 			//	prepare a valid document for XML filter
/*N*/ 			//	(for ConvertFrom, InitNew is called before)
/*N*/ 			aDocument.MakeTable(0);
/*N*/ 			aDocument.GetStyleSheetPool()->CreateStandardStyles();
/*N*/ 			aDocument.UpdStlShtPtrsFrmNms();
/*N*/ 
/*N*/ 			bRet = LoadXML( GetMedium(), pStor );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = LoadCalc( pStor );
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!bRet && !pStor->GetError())
/*?*/ 		pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );
/*N*/ 
/*N*/ 	if (pStor->GetError())
/*?*/ 		SetError( pStor->GetError() );
/*N*/ 
/*N*/ 	InitItems();
/*N*/ 	CalcOutputFactor();
/*N*/ 
/*N*/ 	// #73762# invalidate eventually temporary table areas
/*N*/ 	if ( bRet )
/*N*/ 		aDocument.InvalidateTableArea();
/*N*/ 
/*N*/ 	bIsEmpty = FALSE;
/*N*/ 	FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
/*N*/ 	return bRet;
/*N*/ }


/*N*/ void __EXPORT ScDocShell::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
/*N*/ 						 const SfxHint& rHint, const TypeId& rHintType )
/*N*/ {
/*N*/ 	if (rHint.ISA(SfxSimpleHint))								// ohne Parameter
/*N*/ 	{
/*N*/ 		ULONG nSlot = ((const SfxSimpleHint&)rHint).GetId();
/*N*/ 		switch ( nSlot )
/*N*/ 		{
/*N*/ 			case SFX_HINT_TITLECHANGED:
/*N*/ 				aDocument.SetName( SfxShell::GetName() );
/*N*/ 				//	RegisterNewTargetNames gibts nicht mehr
/*N*/ 				SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED ));	// Navigator
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (rHint.ISA(SfxStyleSheetHint))						// Vorlagen geaendert
/*N*/ 		NotifyStyle((const SfxStyleSheetHint&) rHint);
/*N*/ 	else if (rHint.ISA(ScAutoStyleHint))
/*N*/ 	{
/*?*/ 		//!	direct call for AutoStyles
/*?*/ 
/*?*/ 		//	this is called synchronously from ScInterpreter::ScStyle,
/*?*/ 		//	modifying the document must be asynchronous
/*?*/ 		//	(handled by AddInitial)
/*?*/ 
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScAutoStyleHint& rStlHint = (ScAutoStyleHint&)rHint;
/*N*/ 	}
/*N*/ }

    // Inhalte fuer Organizer laden




/*N*/ BOOL __EXPORT ScDocShell::ConvertFrom( SfxMedium& rMedium )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertFrom" );
/*N*/ 
/*N*/ 	BOOL bRet = FALSE;				// FALSE heisst Benutzerabbruch !!
/*N*/ 									// bei Fehler: Fehler am Stream setzen!!
/*N*/ 
/*N*/ 	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/ 
/*N*/ 	// ob nach dem Import optimale Spaltenbreiten gesetzt werden sollen
/*N*/ 	BOOL bSetColWidths = FALSE;
/*N*/ 	BOOL bSetSimpleTextColWidths = FALSE;
/*N*/ 	BOOL bSimpleColWidth[MAXCOL+1];
/*N*/ 	memset( bSimpleColWidth, 1, (MAXCOL+1) * sizeof(BOOL) );
/*N*/ 	ScRange aColWidthRange;
/*N*/ 	// ob nach dem Import optimale Zeilenhoehen gesetzt werden sollen
/*N*/ 	BOOL bSetRowHeights = FALSE;
/*N*/ 
/*N*/ 	aConvFilterName.Erase(); //@ #BugId 54198
/*N*/ 
/*N*/ 	//	Alle Filter brauchen die komplette Datei am Stueck (nicht asynchron),
/*N*/ 	//	darum vorher per CreateFileStream dafuer sorgen, dass die komplette
/*N*/ 	//	Datei uebertragen wird.
/*N*/ 	rMedium.GetPhysicalName();	//! CreateFileStream direkt rufen, wenn verfuegbar
/*N*/ 
/*N*/     SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
/*N*/     nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : ::com::sun::star::document::UpdateDocMode::NO_UPDATE;
/*N*/ 
/*N*/     const SfxFilter* pFilter = rMedium.GetFilter();
/*N*/ 	if (pFilter)
/*N*/ 	{
/*N*/ 		String aFltName = pFilter->GetFilterName();
/*N*/ 
/*N*/ 		aConvFilterName=aFltName; //@ #BugId 54198
/*N*/ 
/*N*/ 		BOOL bCalc3 = ( aFltName.EqualsAscii(pFilterSc30) );
/*N*/ 		BOOL bCalc4 = ( aFltName.EqualsAscii(pFilterSc40) );
/*N*/ 		if (!bCalc3 && !bCalc4)
/*N*/ 			aDocument.SetInsertingFromOtherDoc( TRUE );
/*N*/ 
/*N*/ 		if (bCalc3 || bCalc4)					// Calc3/4 - "Import"
/*N*/ 		{
/*?*/ 			//	wait cursor is handled with progress bar
/*?*/ 			SvStorage* pStor = rMedium.GetStorage();
/*?*/ 			if ( pStor )
/*?*/ 			{
/*?*/ 				bRet = SfxInPlaceObject::Load( pStor );
/*?*/ 				if( bRet )
/*?*/ 					bRet = LoadCalc( pStor );
/*?*/ 				if (!bRet && !pStor->GetError())
/*?*/ 					pStor->SetError( SVSTREAM_FILEFORMAT_ERROR );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				DBG_ERROR("Calc3/4: kein Storage");
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterXML))
/*?*/ 		{	DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 bRet = LoadXML( &rMedium, NULL );
/*N*/ 		else if (aFltName.EqualsAscii(pFilterSc10))
/*N*/ 		{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 		SvStream* pStream = rMedium.GetInStream();
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterLotus))
/*N*/ 		{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 /*N*/ //			SvStream* pStream = rMedium.GetInStream();
/*N*/ 		}
/*N*/ 		else if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterExcel5) ||
/*N*/ 				   aFltName.EqualsAscii(pFilterExcel95) || aFltName.EqualsAscii(pFilterExcel97) ||
/*N*/ 				   aFltName.EqualsAscii(pFilterEx4Temp) || aFltName.EqualsAscii(pFilterEx5Temp) ||
/*N*/ 				   aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp) )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EXCIMPFORMAT eFormat = EIF_AUTO;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterAscii))
/*N*/ 		{
/*N*/ 			SfxItemSet*	 pSet = rMedium.GetItemSet();
/*N*/ 			const SfxPoolItem* pItem;
/*N*/ 			ScAsciiOptions aOptions;
/*N*/ 			BOOL bOptInit = FALSE;
/*N*/ 
/*N*/ 			if ( pSet && SFX_ITEM_SET ==
/*N*/ 				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
/*N*/ 			{
/*N*/ 				aOptions.ReadFromString( ((const SfxStringItem*)pItem)->GetValue() );
/*N*/ 				bOptInit = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( !bOptInit )
/*N*/ 			{
/*?*/ 				//	default for ascii import (from API without options):
/*?*/ 				//	ISO8859-1/MS_1252 encoding, comma, double quotes
/*?*/ 
/*?*/ 				aOptions.SetCharSet( RTL_TEXTENCODING_MS_1252 );
/*?*/ 				aOptions.SetFieldSeps( (sal_Unicode) ',' );
/*?*/ 				aOptions.SetTextSep( (sal_Unicode) '"' );
/*N*/ 			}
/*N*/ 
/*N*/ 			FltError eError = eERR_OK;
/*N*/ 			BOOL bOverflow = FALSE;
/*N*/ 
/*N*/ 			if( ! rMedium.IsStorage() )
/*N*/ 			{
/*N*/ 				ScImportExport	aImpEx( &aDocument );
/*N*/ 				aImpEx.SetExtOptions( aOptions );
/*N*/ 
/*N*/ 				SvStream* pInStream = rMedium.GetInStream();
/*N*/ 				if (pInStream)
/*N*/ 				{
/*N*/ 					pInStream->SetStreamCharSet( aOptions.GetCharSet() );
/*N*/ 					pInStream->Seek( 0 );
/*N*/ 					bRet = aImpEx.ImportStream( *pInStream );
/*N*/ 					eError = bRet ? eERR_OK : SCERR_IMPORT_CONNECT;
/*N*/ 					aDocument.StartAllListeners();
/*N*/ 					aDocument.SetDirty();
/*N*/ 					bOverflow = aImpEx.IsOverflow();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					DBG_ERROR( "No Stream" );
/*N*/ 			}
/*N*/ 
/*N*/ 			if (eError != eERR_OK)
/*N*/ 			{
/*N*/ 				if (!GetError())
/*N*/ 					SetError(eError);
/*N*/ 			}
/*N*/ 			else if ( bOverflow )
/*N*/ 			{
/*N*/ 				if (!GetError())
/*N*/ 					SetError(SCWARN_IMPORT_RANGE_OVERFLOW);
/*N*/ 			}
/*N*/ 			bSetColWidths = TRUE;
/*N*/ 			bSetSimpleTextColWidths = TRUE;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterDBase))
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 String sItStr;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterDif))
/*N*/ 		{
/*?*/ 			SvStream* pStream = rMedium.GetInStream();
/*?*/ 			if (pStream)
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 FltError eError;
/*?*/ 			}
/*?*/ 			bSetColWidths = TRUE;
/*?*/ 			bSetSimpleTextColWidths = TRUE;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterSylk))
/*N*/ 		{
/*?*/ 			FltError eError = SCERR_IMPORT_UNKNOWN;
/*?*/ 			if( !rMedium.IsStorage() )
/*?*/ 			{
/*?*/ 				ScImportExport aImpEx( &aDocument );
/*?*/ 
/*?*/ 				SvStream* pInStream = rMedium.GetInStream();
/*?*/ 				if (pInStream)
/*?*/ 				{
/*?*/ 					pInStream->Seek( 0 );
/*?*/ 					bRet = aImpEx.ImportStream( *pInStream, SOT_FORMATSTR_ID_SYLK );
/*?*/ 					eError = bRet ? eERR_OK : SCERR_IMPORT_UNKNOWN;
/*?*/ 					aDocument.StartAllListeners();
/*?*/ 					aDocument.SetDirty();
/*?*/ 				}
/*?*/ 				else
/*?*/ 					DBG_ERROR( "No Stream" );
/*?*/ 			}
/*?*/ 
/*?*/ 			if ( eError != eERR_OK && !GetError() )
/*?*/ 				SetError(eError);
/*?*/ 			bSetColWidths = TRUE;
/*?*/ 			bSetSimpleTextColWidths = TRUE;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterRtf))
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 FltError eError = SCERR_IMPORT_UNKNOWN;
/*N*/ 		}
/*N*/ 		else if (aFltName.EqualsAscii(pFilterHtml) || aFltName.EqualsAscii(pFilterHtmlWebQ))
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 FltError eError = SCERR_IMPORT_UNKNOWN;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (!GetError())
/*N*/ 				SetError(SCERR_IMPORT_NI);
/*N*/ 		}
/*N*/ 
/*N*/ 		if (!bCalc3)
/*N*/ 			aDocument.SetInsertingFromOtherDoc( FALSE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Kein Filter bei ConvertFrom");
/*N*/ 
/*N*/ 	InitItems();
/*N*/ 	CalcOutputFactor();
/*N*/ 	if ( bRet && (bSetColWidths || bSetRowHeights) )
/*N*/ 	{	// Spaltenbreiten/Zeilenhoehen anpassen, Basis 100% Zoom
/*N*/ 		Fraction aZoom( 1, 1 );
/*N*/ 		double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
/*N*/ 			/ GetOutputFactor();	// Faktor ist Drucker zu Bildschirm
/*N*/ 		double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
/*N*/ 		VirtualDevice aVirtDev;
/*N*/ 		//	all sheets (for Excel import)
/*N*/ 		USHORT nTabCount = aDocument.GetTableCount();
/*N*/ 		for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 		{
/*N*/ 			USHORT nEndCol, nEndRow;
/*N*/ 			aDocument.GetCellArea( nTab, nEndCol, nEndRow );
/*N*/ 			aColWidthRange.aEnd.SetCol( nEndCol );
/*N*/ 			aColWidthRange.aEnd.SetRow( nEndRow );
/*N*/ 			ScMarkData aMark;
/*N*/ 			aMark.SetMarkArea( aColWidthRange );
/*N*/ 			aMark.MarkToMulti();
/*N*/ 			// Reihenfolge erst Breite dann Hoehe ist wichtig (vergl. hund.rtf)
/*N*/ 			if ( bSetColWidths )
/*N*/ 			{
/*N*/ 				for ( USHORT nCol=0; nCol <= nEndCol; nCol++ )
/*N*/ 				{
/*N*/ 					USHORT nWidth = aDocument.GetOptimalColWidth(
/*N*/ 						nCol, nTab, &aVirtDev, nPPTX, nPPTY, aZoom, aZoom, FALSE, &aMark,
/*N*/ 						(bSetSimpleTextColWidths && bSimpleColWidth[nCol]) );
/*N*/ 					aDocument.SetColWidth( nCol, nTab,
/*N*/ 						nWidth + (USHORT)ScGlobal::nLastColWidthExtra );
/*N*/ 				}
/*N*/ 			}
/*N*/ //			if ( bSetRowHeights )
/*N*/ //			{
/*N*/ //				//	nExtra must be 0
/*N*/ //				aDocument.SetOptimalHeight(	0, nEndRow, nTab, 0, &aVirtDev,
/*N*/ //					nPPTX, nPPTY, aZoom, aZoom, FALSE );
/*N*/ //			}
/*N*/ 		}
/*N*/ 		if ( bSetRowHeights )
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 UpdateAllRowHeights();		// with vdev or printer, depending on configuration
/*N*/ 	}
/*N*/ 	FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
/*N*/ 
/*N*/ 	// #73762# invalidate eventually temporary table areas
/*N*/ 	if ( bRet )
/*N*/ 		aDocument.InvalidateTableArea();
/*N*/ 
/*N*/ 	bIsEmpty = FALSE;
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }


/*N*/ void __EXPORT ScDocShell::HandsOff()
/*N*/ {
/*N*/ 	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 
/*N*/ 	SfxInPlaceObject::HandsOff();
/*N*/ 
/*N*/ 	if( pDrawLayer )
/*N*/ 		pDrawLayer->ReleasePictureStorage();
/*N*/ }


/*N*/ BOOL __EXPORT ScDocShell::Save()
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Save" );
/*N*/ 
/*N*/ 	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/ 
/*N*/ 	SvStorage* pStor = GetStorage();
/*N*/ 	DBG_ASSERT( pStor, "Save: no storage" );
/*N*/ 	BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );
/*N*/ 
/*N*/ 	//	DoEnterHandler hier nicht (wegen AutoSave), ist im ExecuteSave
/*N*/ 
/*N*/ 	ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
/*N*/ 	if (pCharts)
/*N*/ 		pCharts->UpdateDirtyCharts();					// Charts, die noch upgedated werden muessen
/*N*/ 	if (pAutoStyleList)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pAutoStyleList->ExecuteAllNow();				// Vorlagen-Timeouts jetzt ausfuehren
/*N*/ 	if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
/*N*/ 		SvInPlaceObject::SetVisArea( Rectangle() );		// normal bearbeitet -> keine VisArea
/*N*/ 
/*N*/ 	// #77577# save additionally XML in storage
/*N*/ 	if ( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED && !bXML )
/*N*/ 		AddXMLAsZipToTheStorage( *pStor );
/*N*/ 
/*N*/ 	//	wait cursor is handled with progress bar
/*N*/ 	BOOL bRet = SfxInPlaceObject::Save();
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		if (bXML)
/*N*/ 			bRet = SaveXML( NULL, pStor );
/*N*/ 		else
/*N*/ 			bRet = SaveCalc( pStor );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL __EXPORT ScDocShell::SaveAs( SvStorage* pStor )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::SaveAs" );
/*N*/ 
/*N*/ 	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );
/*N*/ 
/*N*/ 	DBG_ASSERT( pStor, "SaveAs without storage?" );
/*N*/ 	BOOL bXML = ( pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 );
/*N*/ 
/*N*/ 	//	DoEnterHandler hier nicht (wegen AutoSave), ist im ExecuteSave
/*N*/ 
/*N*/ 	ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
/*N*/ 	if (pCharts)
/*N*/ 		pCharts->UpdateDirtyCharts();					// Charts, die noch upgedated werden muessen
/*N*/ 	if (pAutoStyleList)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pAutoStyleList->ExecuteAllNow();				// Vorlagen-Timeouts jetzt ausfuehren
/*N*/ 	if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
/*N*/ 		SvInPlaceObject::SetVisArea( Rectangle() );		// normal bearbeitet -> keine VisArea
/*N*/ 
/*N*/ 	// #77577# save additionally XML in storage
/*N*/ 	if ( GetCreateMode() != SFX_CREATE_MODE_EMBEDDED && !bXML )
/*N*/ 		AddXMLAsZipToTheStorage( *pStor );
/*N*/ 
/*N*/ 	//	wait cursor is handled with progress bar
/*N*/ 	BOOL bRet = SfxInPlaceObject::SaveAs( pStor );
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		if (bXML)
/*N*/ 			bRet = SaveXML( NULL, pStor );
/*N*/ 		else
/*N*/ 			bRet = SaveCalc( pStor );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }




// Xcl-like column width measured in characters of standard font.










/*N*/ BOOL __EXPORT ScDocShell::SaveCompleted( SvStorage * pStor )
/*N*/ {
/*N*/ 	return SfxInPlaceObject::SaveCompleted( pStor );
/*N*/ }


/*N*/ BOOL __EXPORT ScDocShell::DoSaveCompleted( SfxMedium * pNewStor )
/*N*/ {
/*N*/ 	BOOL bRet = SfxObjectShell::DoSaveCompleted( pNewStor );
/*N*/ 
/*N*/ 	//	SC_HINT_DOC_SAVED fuer Wechsel ReadOnly -> Read/Write
/*N*/ 	Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
/*N*/ 	return bRet;
/*N*/ }





/*N*/ String ScDocShell::GetOwnFilterName()			// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterSc50);
/*N*/ }

/*N*/ String ScDocShell::GetWebQueryFilterName()		// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterHtmlWebQ);
/*N*/ }

/*N*/ String ScDocShell::GetAsciiFilterName()			// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterAscii);
/*N*/ }

/*N*/ String ScDocShell::GetLotusFilterName()			// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterLotus);
/*N*/ }

/*N*/ String ScDocShell::GetDBaseFilterName()			// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterDBase);
/*N*/ }

/*N*/ String ScDocShell::GetDifFilterName()			// static
/*N*/ {
/*N*/ 	return String::CreateFromAscii(pFilterDif);
/*N*/ }

/*N*/ BOOL ScDocShell::HasAutomaticTableName( const String& rFilter )		// static
/*N*/ {
/*N*/ 	//	TRUE for those filters that keep the default table name
/*N*/ 	//	(which is language specific)
/*N*/ 
/*N*/ 	return rFilter.EqualsAscii( pFilterAscii )
/*N*/ 		|| rFilter.EqualsAscii( pFilterLotus )
/*N*/ 		|| rFilter.EqualsAscii( pFilterExcel4 )
/*N*/ 		|| rFilter.EqualsAscii( pFilterEx4Temp )
/*N*/ 		|| rFilter.EqualsAscii( pFilterDBase )
/*N*/ 		|| rFilter.EqualsAscii( pFilterDif )
/*N*/ 		|| rFilter.EqualsAscii( pFilterSylk )
/*N*/ 		|| rFilter.EqualsAscii( pFilterHtml )
/*N*/ 		|| rFilter.EqualsAscii( pFilterRtf );
/*N*/ }

//==================================================================

#define __SCDOCSHELL_INIT \
        aDocument		( SCDOCMODE_DOCUMENT, this ), \
        pFontList		( NULL ), \
        bHeaderOn		( TRUE ), \
        bFooterOn		( TRUE ), \
        pDocHelper 		( NULL ), \
        pAutoStyleList	( NULL ), \
        pOldJobSetup	( NULL ), \
        pPaintLockData	( NULL ), \
        nPrtToScreenFactor( 1.0 ), \
        bIsEmpty		( TRUE ), \
        bIsInUndo		( FALSE ), \
        bDocumentModifiedPending( FALSE ), \
        nDocumentLock	( 0 ), \
        nCanUpdate (::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG), \
        bUpdateEnabled  ( TRUE ), \
        pVirtualDevice_100th_mm ( NULL ), \
        pModificator    ( NULL )

//------------------------------------------------------------------

/*N*/ ScDocShell::ScDocShell( const ScDocShell& rShell )
/*N*/ 	:	SfxObjectShell( rShell.GetCreateMode() ),
/*N*/ 		aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))),
/*N*/ 		bNoInformLost( TRUE ),
/*N*/ 		__SCDOCSHELL_INIT
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );
/*N*/ }

//------------------------------------------------------------------

/*N*/ ScDocShell::ScDocShell( SfxObjectCreateMode eMode )
/*N*/ 	:	SfxObjectShell( eMode ),
/*N*/ 		aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))),
/*N*/ 		bNoInformLost( TRUE ),
/*N*/ 		__SCDOCSHELL_INIT
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );
/*N*/ 
/*N*/ 	SetPool( &SC_MOD()->GetPool() );
/*N*/ 
/*N*/ 	SetShell(this);
/*N*/ 	bIsInplace = (eMode == SFX_CREATE_MODE_EMBEDDED);
/*N*/ 	//	wird zurueckgesetzt, wenn nicht inplace
/*N*/ 
/*N*/ 	pDocFunc = new ScDocFunc(*this);
/*N*/ 
/*N*/ 	//	SetBaseModel needs exception handling
/*N*/ 	ScModelObj::CreateAndSet( this );
/*N*/ 
/*N*/ 	StartListening(*this);
/*N*/ 	SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
/*N*/ 	if (pStlPool)
/*N*/ 		StartListening(*pStlPool);

/*N*/ 	aDocument.GetDBCollection()->SetRefreshHandler(
/*N*/ 		LINK( this, ScDocShell, RefreshDBDataHdl ) );
/*N*/ 
/*N*/ 	//	InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
/*N*/ }

//------------------------------------------------------------------

/*N*/ __EXPORT ScDocShell::~ScDocShell()
/*N*/ {
/*N*/ 	ResetDrawObjectShell();	// #55570# falls der Drawing-Layer noch versucht, darauf zuzugreifen
/*N*/ 
/*N*/ 	SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
/*N*/ 	if (pStlPool)
/*N*/ 		EndListening(*pStlPool);
/*N*/ 	EndListening(*this);
/*N*/ 
/*N*/ 	delete pAutoStyleList;
/*N*/ 
/*N*/ 	delete pDocFunc;
/*N*/ 	delete pFontList;
/*N*/ 
/*N*/ 	delete pPaintLockData;
/*N*/ 
/*N*/ 	delete pOldJobSetup;		// gesetzt nur bei Fehler in StartJob()
/*N*/ 
/*N*/ 	delete pVirtualDevice_100th_mm;
/*N*/ 
/*N*/     if (pModificator)
/*N*/     {
/*N*/         DBG_ERROR("The Modificator should not exist");
/*N*/         delete pModificator;
/*N*/     }
/*N*/ }

//------------------------------------------------------------------


/*N*/ void ScDocShell::SetModified( BOOL bModified )
/*N*/ {
/*N*/ 	SfxInPlaceObject::SetModified( bModified );
/*N*/ 	Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
/*N*/ }


/*N*/ void ScDocShell::SetDocumentModified( BOOL bIsModified /* = TRUE */ )
/*N*/ {
/*N*/ 	//	BroadcastUno muss auch mit pPaintLockData sofort passieren
/*N*/ 	//!	auch bei SetDrawModified, wenn Drawing angebunden ist
/*N*/ 	//!	dann eigener Hint???
/*N*/ 
/*N*/ 	if (bIsModified)
/*N*/ 		aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/ 
/*N*/ 	if ( pPaintLockData && bIsModified )
/*N*/ 	{
/*N*/ 		pPaintLockData->SetModified();			// spaeter...
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetDrawModified( bIsModified );
/*N*/ 
/*N*/ 	if ( bIsModified )
/*N*/ 	{
/*N*/ 		if ( aDocument.IsAutoCalcShellDisabled() )
/*?*/ 			SetDocumentModifiedPending( TRUE );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SetDocumentModifiedPending( FALSE );
/*N*/             aDocument.InvalidateStyleSheetUsage();
/*N*/ 			aDocument.InvalidateTableArea();
/*N*/             aDocument.InvalidateLastTableOpParams();
/*N*/ 			aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
/*N*/ 			if ( aDocument.IsForcedFormulaPending() && aDocument.GetAutoCalc() )
/*?*/ 			{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 aDocument.CalcFormulaTree( TRUE );
/*N*/ 			PostDataChanged();
/*N*/ 
/*N*/ 			//	Detective AutoUpdate:
/*N*/ 			//	Update if formulas were modified (DetectiveDirty) or the list contains
/*N*/ 			//	"Trace Error" entries (#75362# - Trace Error can look completely different
/*N*/ 			//	after changes to non-formula cells).
/*N*/ 
/*N*/ 			ScDetOpList* pList = aDocument.GetDetOpList();
/*N*/ 			if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
/*N*/ 				 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
/*N*/ 			{
/*N*/ 				GetDocFunc().DetectiveRefresh(TRUE);	// TRUE = caused by automatic update
/*N*/ 			}
/*N*/ 			aDocument.SetDetectiveDirty(FALSE);			// always reset, also if not refreshed
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//	SetDrawModified - ohne Formel-Update
//	(Drawing muss auch beim normalen SetDocumentModified upgedated werden,
//	 z.B. bei Tabelle loeschen etc.)

/*N*/ void ScDocShell::SetDrawModified( BOOL bIsModified /* = TRUE */ )
/*N*/ {
/*N*/ 	BOOL bUpdate = ( bIsModified != IsModified() );
/*N*/ 
/*N*/ 	SetModified( bIsModified );
/*N*/ 
/*N*/ 	if (bIsModified)
/*N*/ 	{
/*N*/ 		if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
/*N*/ 		{
/*N*/ 			aDocument.UpdateChartListenerCollection();
/*N*/ 			SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));	// Navigator
/*N*/ 		}
/*N*/ 		SC_MOD()->AnythingChanged();
/*N*/ 	}
/*N*/ }






/*N*/ Window* ScDocShell::GetDialogParent()
/*N*/ {
/*N*/ 		return Application::GetDefDialogParent();
/*N*/ }

/*N*/ VirtualDevice* ScDocShell::GetVirtualDevice_100th_mm()
/*N*/ {
/*N*/ 	if (!pVirtualDevice_100th_mm)
/*N*/ 	{
/*N*/ 		pVirtualDevice_100th_mm = new VirtualDevice;
/*N*/ 		pVirtualDevice_100th_mm->SetMapMode( MAP_100TH_MM );
/*N*/ 	}
/*N*/ 	return pVirtualDevice_100th_mm;
/*N*/ }

// --- ScDocShellModificator ------------------------------------------

/*N*/ ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
/*N*/ 		:
/*N*/ 		rDocShell( rDS ),
/*N*/ 		aProtector( rDS.GetDocument()->GetRefreshTimerControlAddress() )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
/*N*/ 	bIdleDisabled = pDoc->IsIdleDisabled();
/*N*/ 	pDoc->SetAutoCalcShellDisabled( TRUE );
/*N*/ 	pDoc->DisableIdle( TRUE );
/*N*/ }


/*N*/ ScDocShellModificator::~ScDocShellModificator()
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
/*N*/ 	if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
/*?*/ 		rDocShell.SetDocumentModified();	// last one shuts off the lights
/*N*/ 	pDoc->DisableIdle( bIdleDisabled );
/*N*/ }


/*N*/ void ScDocShellModificator::SetDocumentModified()
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	if ( !pDoc->IsImportingXML() )
/*N*/ 	{
/*N*/ 		// AutoCalcShellDisabled temporaer restaurieren
/*N*/ 		BOOL bDisabled = pDoc->IsAutoCalcShellDisabled();
/*N*/ 		pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
/*N*/ 		rDocShell.SetDocumentModified();
/*N*/ 		pDoc->SetAutoCalcShellDisabled( bDisabled );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		// uno broadcast is necessary for api to work
/*?*/ 		// -> must also be done during xml import
/*?*/ 		pDoc->BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/ 	}
/*N*/ }
}
