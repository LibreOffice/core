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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdio.h>

#include <hintids.hxx>

#if !defined _SVSTDARR_USHORTS_DECL || !defined _SVSTDARR_STRINGSDTOR_DECL
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#endif
#include <bf_svx/fontitem.hxx>
#include <bf_svtools/style.hxx>
#include <bf_svx/editeng.hxx>
#include <bf_svx/svdpool.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svtools/htmlkywd.hxx>
#include <bf_sfx2/docinf.hxx>
#include <tools/tenccvt.hxx>

#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <fmtfld.hxx>
#include <charfmt.hxx>
#include <hints.hxx>
#include <doctxm.hxx>
#include <swmodule.hxx>
#include <dcontact.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <dflyobj.hxx>
#include <viewopt.hxx>
#include <sw3io.hxx>
#include <sw3imp.hxx>
#include <sw3marks.hxx>
#include <shellio.hxx>
#include <pagedesc.hxx>
#include <mdiexp.hxx>
#include <poolfmt.hxx>
#include <crypter.hxx>
#include <dbfld.hxx>
#include <expfld.hxx>
#include <ndole.hxx>
#include <drawdoc.hxx>

#include <docsh.hxx>
#include <swfltopt.hxx>

#include <swerror.h>
#include <statstr.hrc>
#include <poolfmt.hrc>
#include <fmtinfmt.hxx>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



static Sw3IoImp* pCurIosys;					// fuer non-poolable Attrs

// Zugriff auf die INI-Datei, um interne Flags zu setzen

#define FLTF_NO_DRAWINGS	0x00000001L

#define MAX_SMALL_RECSIZE	0xfffffeUL
#define LONG_RECSIZE 		0xffffffUL

static sal_uInt32 nFltFlags = 0;


sal_Char __FAR_DATA SW3HEADER[] = "SW3HDR";		// 3.0/3.1 Header
sal_Char __FAR_DATA SW4HEADER[] = "SW4HDR";		// 4.0 Header
sal_Char __FAR_DATA SW5HEADER[] = "SW5HDR";		// 5.0 Header

// TODOs fuer 6.0-FF
// - Record-Laengen > 16MB richtig handeln.
// - while( BytesLeft() )-Schleifen elemenieren

/*N*/ SV_IMPL_PTRARR(Sw3Strings, String*)
/*N*/ SV_IMPL_VARARR(Sw3ULongs, sal_uInt32)
/*N*/ SV_IMPL_VARARR(Sw3Bytes, sal_uInt8)

class Sw3RecordSizeTable: private Table
{
public:
    Table::Count;
    Table::SearchKey;
    Table::GetObjectKey;

    Sw3RecordSizeTable() :
        Table()
    {}

    sal_Bool Insert( ULONG nKey, sal_uInt32 p )
    {
        return Table::Insert( nKey, (void*)p );
    }

    sal_uInt32 GetObject( ULONG nPos ) const
    {
        return (sal_uIntPtr)Table::GetObject( nPos );
    }
};


//////////////////////////////////////////////////////////////////////////////

/*  */

/*N*/ Sw3ExportInfo::~Sw3ExportInfo()
/*N*/ {
/*N*/ 	delete pTblLineBoxFmtNames40;
/*N*/ }

/*  */


/*N*/ Sw3IoImp::Sw3IoImp( Sw3Io& r )
/*N*/ 		: rIo( r ), pDoc( NULL ),
/*N*/ 		aDefWordDelim( SW_MOD()->GetDocStatWordDelim() ),
/*N*/ 		N_DOC( String::CreateFromAscii( "StarWriterDocument" ) ),
/*N*/ 		N_PAGESTYLES( String::CreateFromAscii( "SwPageStyleSheets" ) ),
/*N*/ 		N_NUMRULES( String::CreateFromAscii( "SwNumRules" ) ),
/*N*/ 		N_DRAWING( String::CreateFromAscii( DRAWING_STREAM_NAME ) ),
/*N*/ 		N_PICTURES( String::CreateFromAscii( "EmbeddedPictures" ) ),
/*N*/ 		N_BLOCKDIR( String::CreateFromAscii( "DocumentList" ) ),
/*N*/ 		sStarSymbol( "StarSymbol", sizeof("StarSymbol")-1, RTL_TEXTENCODING_ASCII_US ),
/*N*/ 		sOpenSymbol( "OpenSymbol", sizeof("OpenSymbol")-1, RTL_TEXTENCODING_ASCII_US ),
/*N*/ 		sStarBats( "StarBats", sizeof("StarBats")-1, RTL_TEXTENCODING_ASCII_US ),
/*N*/ 		sStarMath( "StarMath", sizeof("StarMath")-1, RTL_TEXTENCODING_ASCII_US )
/*N*/ {
/*N*/ 	bNormal		= sal_True;
/*N*/ 	bTxtColls 	=
/*N*/ 	bCharFmts	=
/*N*/ 	bFrmFmts	=
/*N*/ 	bPageDescs	=
/*N*/ 	bNumRules	=
/*N*/ 	bAdditive	=
/*N*/ 	bBlock	    =
/*N*/ 	bSw31Export	=
/*N*/ 	bOrganizer	=
/*N*/ 	bInsIntoHdrFtr = sal_False;
/*N*/ 	pFlyFrms    = NULL;
/*N*/ 	pExportInfo = NULL;
/*N*/ 	pCrypter    = NULL;
/*N*/ 	pTOXs       = NULL;
/*N*/ 	pRedlines	= NULL;
/*N*/ 	pRedlineMarks = NULL;
/*N*/ 	pMarks	    = NULL;
/*N*/ 	pBookmarks  = NULL;
/*N*/ 	p30OLENodes = NULL;
/*N*/ 	pTblLineBoxFmts = NULL;
/*N*/ 	pSectionDepths = 0;
/*N*/ 	pRecSizes = NULL;
/*N*/ 	pAuthorityMap = 0;
/*N*/ 	pConvToSymbolFmts = 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	pRefSdrObjects = 0;
/*N*/ #endif
/*N*/ 	hBatsFontConv = 0;
        hMathFontConv = 0;
/*N*/ 	Reset();
/*N*/ 
/*N*/ 	{
/*N*/ 		const sal_Char* aNames[1] = { "Writer/SW3Imp" };
/*N*/ 		sal_uInt32 aVal[1];
/*N*/ 		SwFilterOptions aOpt( 1, aNames, aVal );
/*N*/ 		nFltFlags = aVal[ 0 ];
/*N*/ 	}
/*N*/ 	nCntntBkmkStart = 0;
/*N*/ 	nCntntRedlineStart = 0;
/*N*/ 	pHiddenDrawObjs = 0;
/*N*/ }

/*N*/ Sw3IoImp::~Sw3IoImp()
/*N*/ {
/*N*/ 	if( pDoc && !pDoc->RemoveLink() )
/*N*/ 		delete pDoc;
/*N*/ }

/*N*/ Sw3IoImp* Sw3IoImp::GetCurrentIo()
/*N*/ {
/*N*/ 	return pCurIosys;
/*N*/ }

// Allgemeine Initialisierungen

/*N*/ void Sw3IoImp::Reset()
/*N*/ {
/*N*/ 	Date aDate;
/*N*/ 	Time aTime;
/*N*/ 	// die folgenden beiden Felder werden auch zum Passwort-Check benutzt
/*N*/ 	nDate		= aDate.GetDate();
/*N*/ 	nTime		= aTime.GetTime();
/*N*/ 	bInsert		=
/*N*/ 	bNoDrawings = sal_False;
/*N*/ 	bSaveAll    = sal_True;
/*N*/ 	bSpellAllAgain = bSpellWrongAgain = sal_True;
/*N*/ 	nCurBlk		= 0;
/*N*/ 	nRes 		= 0;
/*N*/ 	nWarn		= 0;
/*N*/ 	nCurPercent =
/*N*/ 	nEndPercent = 0;
/*N*/ 	nZOrderOff  = 0;
/*N*/ 	nHiddenDrawObjs = ULONG_MAX;
/*N*/ 	pBlkList	= NULL;
/*N*/ 	bInsIntoHdrFtr = sal_False;
/*N*/ 	bSw31Export = sal_False;
/*N*/ 	FreeFlyFrms();
/*N*/ 	aBlkName.Erase();
/*N*/ 	pStrm = 0;	// wg. Assert in IsSw.*Export
/*N*/ 	Reset2();
/*N*/ 	nGblFlags	= SW3F_NODRAWING;
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	delete pRefSdrObjects;
/*N*/ 	pRefSdrObjects = 0;
/*N*/ #endif
/*N*/ 	delete pConvToSymbolFmts;
/*N*/ 	pConvToSymbolFmts = 0;
/*N*/ }

// Initialisierung aller Daten vor dem Lesen/Schreiben eines Docs

/*N*/ void Sw3IoImp::Reset2()
/*N*/ {
/*N*/ 	bOut		  =
/*N*/ 	bConvertNoNum = sal_False;
/*N*/ 	nCurMark	=
/*N*/ 	nFlyLevel	=
/*N*/ 	nFileFlags  = 0;
/*N*/ 	nSizeDivFac	= 1;
/*N*/ 	nFlagRecEnd = 0;
/*N*/ 	pCurTbl		= NULL;
/*N*/ 	pCurNumRule = NULL;
/*N*/ 	pCurPaM		=
/*N*/ 	pCurNumRange= NULL;
/*N*/ 	eSrcSet		= gsl_getSystemTextEncoding();
/*N*/ 	if( pStrm )
/*N*/ 		eSrcSet = GetSOStoreTextEncoding( eSrcSet, pStrm->GetVersion() );
/*N*/ 	else if( pRoot.Is() )
/*N*/ 		eSrcSet = GetSOStoreTextEncoding( eSrcSet, pRoot->GetVersion() );
/*N*/ 	nVersion	= SWG_VERSION;
/*N*/ 
/*N*/ 	aRecTypes.Remove( 0, aRecTypes.Count() );
/*N*/ 	aRecSizes.Remove( 0, aRecSizes.Count() );
/*N*/ 	aValPositions.Remove( 0, aValPositions.Count() );
/*N*/ 	eStartNodeType = SwNormalStartNode;
/*N*/ 
/*N*/ 	aINetFldText.Erase();
/*N*/ 	pFmtINetFmt = 0;
/*N*/ 	bDrawFmtSkipped = sal_False;
/*N*/ 
/*N*/ 	memset( cPasswd, 0, 16 );
/*N*/ 	aStringPool.Clear();
/*N*/ 	if( pExportInfo && pExportInfo->pTblLineBoxFmtNames40 )
/*N*/ 	{
/*N*/ 		delete pExportInfo->pTblLineBoxFmtNames40;
/*N*/ 		pExportInfo->pTblLineBoxFmtNames40 = 0;
/*N*/ 	}
/*N*/ 	// Hier ggf das Passwort neu setzen
/*N*/ 	SetPasswd();
/*N*/ 
/*N*/ 	delete pRecSizes;
/*N*/ 	pRecSizes = 0;
/*N*/ 
/*N*/ 	delete pAuthorityMap;
/*N*/ 	pAuthorityMap = 0;
/*N*/ 
/*N*/ 	delete pSectionDepths;
/*N*/ 	pSectionDepths = 0;
/*N*/ 
/*N*/ 	if( hBatsFontConv )
/*N*/ 	{
/*N*/ 		DestroyFontToSubsFontConverter( hBatsFontConv );
/*N*/ 		hBatsFontConv = 0;
/*N*/ 	}
/*N*/ 	if( hMathFontConv )
/*N*/ 	{
/*N*/ 		DestroyFontToSubsFontConverter( hMathFontConv );
/*N*/ 		hMathFontConv = 0;
/*N*/ 	}
/*N*/ }

#ifdef DBG_UTIL
/*N*/ sal_Bool Sw3IoImp::IsSw40Export() const
/*N*/ {
/*N*/ 	ASSERT( !pStrm || pStrm->GetVersion()==pRoot->GetVersion(),
/*N*/ 			"Fileformat-Version am Stream stimmt nicht." );
/*N*/ 	return pRoot->GetVersion() == SOFFICE_FILEFORMAT_40;
/*N*/ }

/*N*/ sal_Bool Sw3IoImp::IsSw31Or40Export() const
/*N*/ {
/*N*/ 	ASSERT( !pStrm || pStrm->GetVersion()==pRoot->GetVersion(),
/*N*/ 			"Fileformat-Version am Stream stimmt nicht." );
/*N*/ 	return pRoot->GetVersion() <= SOFFICE_FILEFORMAT_40;
/*N*/ }
#endif

/*N*/ void Sw3IoImp::Error( ULONG nCode )
/*N*/ {
/*N*/ 	nRes = nCode ? nCode : ERR_SWG_FILE_FORMAT_ERROR;
/*N*/ }
 
/*N*/ void Sw3IoImp::Warning( ULONG nCode )
/*N*/ {
/*N*/ 	nWarn = nCode ? nCode : WARN_SWG_FEATURES_LOST;
/*N*/ }

/*************************************************************************
*
*		Einlese-Optionen
*
*************************************************************************/

/*N*/ void Sw3IoImp::SetReadOptions( const SwgReaderOption& rOpt, sal_Bool bOverwrite )
/*N*/ {
/*N*/ 	bTxtColls 	=
/*N*/ 	bCharFmts	=
/*N*/ 	bFrmFmts	=
/*N*/ 	bPageDescs	=
/*N*/ 	bNumRules 	= sal_False;	// fix #27376#: sonst sind alle sal_True
/*N*/ 	if( rOpt.IsFrmFmts() )
/*N*/ 		bFrmFmts = sal_True;
/*N*/ 	if( rOpt.IsTxtFmts() )
/*N*/ 		bCharFmts = bTxtColls = sal_True;
/*N*/ 	if( rOpt.IsPageDescs() )
/*N*/ 		bPageDescs = sal_True;
/*N*/ 	if( rOpt.IsNumRules() )
/*N*/ 		bNumRules = sal_True;
/*N*/ 	bNormal = sal_Bool( !( bTxtColls | bCharFmts | bFrmFmts | bPageDescs | bNumRules ) );
/*N*/ 	bAdditive = bNormal ? !bOverwrite : rOpt.IsMerge();
/*N*/ }

/*N*/ void Sw3IoImp::InsertHiddenDrawObjs()
/*N*/ {
/*N*/ 	if( !pDoc->GetDrawModel() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SwSpzFrmFmts* pFlys = pDoc->GetSpzFrmFmts();
/*N*/ 	sal_uInt16 nArrLen = pFlys->Count();
/*N*/ 
/*N*/ 	SdrPage *pPage = pDoc->GetDrawModel()->GetPage( 0 );
/*N*/ 	ASSERT( !pHiddenDrawObjs, "hidden obj list exists" );
/*N*/ 	delete pHiddenDrawObjs;
/*N*/ 	pHiddenDrawObjs = 0;
/*N*/ 
/*N*/ 	sal_uInt16 nCount = pPage->GetObjCount();
/*N*/ 	nHiddenDrawObjs = nCount;
/*N*/ 	for( sal_uInt16 i=0; i<nArrLen; i++ )
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFmt = (*pFlys)[ i ];
/*N*/ 		if( RES_DRAWFRMFMT != pFmt->Which() )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		SwClientIter aIter( *(SwFrmFmt*)pFmt );
/*N*/ 		if( aIter.First( TYPE(SwDrawContact) ) )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = ((SwDrawContact*)aIter())->GetMaster();
/*N*/ 
/*N*/ 			// Nowadays, all objects have to be in a page, while in earlier
/*N*/ 			// version, hidden objects were not in the page.
/*N*/ 			ASSERT( pObj->GetPage(), "draw objects have to be in a page" );
/*N*/ 			if( !pObj->GetPage() )
/*N*/ 				pPage->InsertObject( pObj );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nHiddenDrawObjs == pPage->GetObjCount() )
/*N*/ 		nHiddenDrawObjs = ULONG_MAX;
/*N*/ 
/*N*/ 	for( sal_uInt16 j=0; j<nCount; j++ )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = pPage->GetObj( j );
/*N*/ 
/*N*/ 		SdrLayerID nLayerId = pObj->GetLayer();
/*N*/ 		if( !pDoc->IsVisibleLayerId( nLayerId ) )
/*N*/ 		{
/*N*/ 			nLayerId = pDoc->GetVisibleLayerIdByInvisibleOne( nLayerId );
/*N*/ 			if( !pHiddenDrawObjs )
/*N*/ 				pHiddenDrawObjs = new SwHiddenDrawObjList_Impl;
/*N*/ 			pHiddenDrawObjs->push_back( pObj );
/*N*/ 			pObj->NbcSetLayer( nLayerId );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::RemoveHiddenDrawObjs()
/*N*/ {
/*N*/ 	if( pHiddenDrawObjs  )
/*N*/ 	{
/*N*/ 		if( pDoc->GetDrawModel() )
/*N*/ 		{
/*N*/ 			SdrPage *pPage = pDoc->GetDrawModel()->GetPage( 0 );
/*N*/ 			SwHiddenDrawObjList_Impl::const_iterator aIter = pHiddenDrawObjs->begin();
/*N*/ 			while( aIter != pHiddenDrawObjs->end() )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = *aIter;
/*N*/ 				SdrLayerID nLayerId = pObj->GetLayer();
/*N*/ 				nLayerId = pDoc->GetInvisibleLayerIdByVisibleOne( nLayerId );
/*N*/ 				pObj->NbcSetLayer( nLayerId );
/*N*/ 				++aIter;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		delete pHiddenDrawObjs;
/*N*/ 		pHiddenDrawObjs = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( ULONG_MAX != nHiddenDrawObjs )
/*N*/ 	{
/*N*/ 		if( pDoc->GetDrawModel() )
/*N*/ 		{
/*N*/ 			SdrPage *pPage = pDoc->GetDrawModel()->GetPage( 0 );
/*N*/ 			sal_uInt32 nPos = pPage->GetObjCount();
/*N*/ 			while( nPos > nHiddenDrawObjs )
/*N*/ 				pPage->RemoveObject( --nPos );
/*N*/ 		}
/*N*/ 
/*N*/ 		nHiddenDrawObjs = ULONG_MAX;
/*N*/ 	}
/*N*/ }

/*************************************************************************
*
*		Stream-Verwaltung
*
*************************************************************************/

// Does the drawing layer contain some objects or forms?

/*N*/ sal_Bool lcl_sw3io_doesDrawingLayerNeedSaving( SwDoc& rDoc )
/*N*/ {
/*N*/ 	if( !rDoc.GetDrawModel() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	const SdrPage *pPage = rDoc.GetDrawModel()->GetPage( 0 );
/*N*/ 	for ( sal_uInt32 i = 0; i < pPage->GetObjCount(); i++ )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = pPage->GetObj( i );
/*N*/ 		if ( !pObj->IsWriterFlyFrame() &&
/*N*/ 			 !pObj->ISA(SwFlyDrawObj) )
/*N*/ 			return sal_True;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDocShell *pDocSh = rDoc.GetDocShell();
/*N*/ 	if( !pDocSh )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	Reference< drawing::XDrawPageSupplier > xDPSupp( pDocSh->GetBaseModel(),
/*N*/ 													 UNO_QUERY );
/*N*/ 	ASSERT( xDPSupp.is(), "XDrawPageSupplier missing" );
/*N*/ 
/*N*/ 	Reference< drawing::XDrawPage >  xDrawPage = xDPSupp->getDrawPage();
/*N*/ 	ASSERT( xDrawPage.is(), "XDrawPage missing" );
/*N*/ 	if( !xDrawPage.is() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, UNO_QUERY );
/*N*/ 	ASSERT( xFormsSupplier.is(), "XFormsSupplier missing" );
/*N*/ 
/*N*/ 	Reference< container::XNameContainer > xForms = xFormsSupplier->getForms();
/*N*/ 	ASSERT( xForms.is(), "XForms missing" );
/*N*/ 
/*N*/ 	return xForms->hasElements();
/*N*/ }

// Alle Streams neu oeffnen. Der Root-Storage ist bereits gesetzt.
// Der Storage fuer die Grafiken wird in In/OutGrfNode() geoeffnet.
// Das Element StarWriterDocument kann entweder ein Storage oder
// ein Stream sein.
// Der Drawing Layer-Stream wird nur dann geoeffnet, wenn der Stream
// da ist (beim Lesen) oder wenn der Layer Zeichenobjekte enthaelt
// (Schreiben). Beim Schreiben wird er, falls vorhanden und nicht
// verwendet, weggeputzt!

/*N*/ sal_Bool Sw3IoImp::OpenStreams( sal_Bool bRdWr, sal_Bool bUseDrawStream )
/*N*/ {
/*N*/ 	sal_uInt16 nMode = STREAM_READ | STREAM_SHARE_DENYWRITE;
/*N*/ 	if( bRdWr )
/*N*/ 		nMode |= STREAM_WRITE;
/*N*/ 	else
/*N*/ 		nMode |= STREAM_NOCREATE;
/*N*/ 	Reset();
/*N*/ 
/*N*/ 	pCurIosys = this;
/*N*/ 
/*N*/ 	if( !pRoot.Is() || pRoot->GetError() != SVSTREAM_OK )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// Der Doc-Teil kann Storage oder Stream sein!
/*N*/ 	// Der DocStream fehlt natuerlich auch bei Textbausteinen
/*N*/ 	if( !bBlock )
/*N*/ 	{
/*N*/ 		ASSERT( !pRoot->IsContained( N_DOC ) || pRoot->IsStream( N_DOC ), "Beta1-Docs werden nicht unterstuetzt" );
/*N*/ 		if( pRoot->IsStorage( N_DOC ) )
/*N*/ 		{
/*N*/ 			nRes = ERR_SWG_READ_ERROR;
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	long nFFVersion = pRoot->GetVersion();
/*N*/ 	ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31 ||
/*N*/ 			nFFVersion == SOFFICE_FILEFORMAT_40 ||
/*N*/ 			nFFVersion == SOFFICE_FILEFORMAT_50,
/*N*/ 			"Am Root-Storage ist keine FF-Version gesetzt!" );
/*N*/ 
/*N*/ 	// Solange wie noch ein Flag fuer den 3.1-Export haben, muessen wir
/*N*/ 	// es auch noch setzen. Der Sw3Writer macht das nicht mehr fuer uns.
/*N*/ 	if( bRdWr && nFFVersion == SOFFICE_FILEFORMAT_31 )
/*N*/ 		SetSw31Export( sal_True );
/*N*/ 
/*N*/ 	// Wenn eine 3.1-Clipboard-ID gesetzt ist, die Fileformat-Version
/*N*/ 	// auf 3.1 setzten.
/*N*/ 	if( SOT_FORMATSTR_ID_STARWRITER_30 == pRoot->GetFormat() &&
/*N*/ 		nFFVersion != SOFFICE_FILEFORMAT_31 )
/*N*/ 	{
/*?*/ 		ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31,
/*?*/ 				"Fileformat-Version auf 3.1 umgesetzt" );
/*?*/ 		pRoot->SetVersion( SOFFICE_FILEFORMAT_31 );
/*?*/ 		nFFVersion = SOFFICE_FILEFORMAT_31;
/*N*/ 	}
/*N*/ 	else if( ( SOT_FORMATSTR_ID_STARWRITER_40 == pRoot->GetFormat() ||
/*N*/ 			   SOT_FORMATSTR_ID_STARWRITERGLOB_40 == pRoot->GetFormat() ||
/*N*/ 			   SOT_FORMATSTR_ID_STARWRITERWEB_40 == pRoot->GetFormat() ) &&
/*N*/ 			 nFFVersion != SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*?*/ 		ASSERT( nFFVersion == SOFFICE_FILEFORMAT_40,
/*?*/ 				"Fileformat-Version auf 4.0 umgesetzt" );
/*?*/ 		pRoot->SetVersion( SOFFICE_FILEFORMAT_40 );
/*?*/ 		nFFVersion = SOFFICE_FILEFORMAT_40;
/*N*/ 	}
/*N*/ 	else if( ( SOT_FORMATSTR_ID_STARWRITER_50 == pRoot->GetFormat() ||
/*N*/ 			   SOT_FORMATSTR_ID_STARWRITERGLOB_50 == pRoot->GetFormat() ||
/*N*/ 			   SOT_FORMATSTR_ID_STARWRITERWEB_50 == pRoot->GetFormat() ) &&
/*N*/ 			 nFFVersion != SOFFICE_FILEFORMAT_50 )
/*N*/ 	{
/*?*/ 		ASSERT( nFFVersion == SOFFICE_FILEFORMAT_50,
/*?*/ 				"Fileformat-Version auf 5.0 umgesetzt" );
/*?*/ 		pRoot->SetVersion( SOFFICE_FILEFORMAT_50 );
/*?*/ 		nFFVersion = SOFFICE_FILEFORMAT_50;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	// The drawing layer stream will be opened for reading if it exists only
/*N*/ 	// and for writing if it contains some (non virtual) objects or forms.
/*N*/ 	sal_Bool bDraw = bRdWr && bUseDrawStream && pDoc->GetDrawModel() &&
/*N*/ 				 lcl_sw3io_doesDrawingLayerNeedSaving( *pDoc );
/*N*/ 
/*N*/ 	sal_Bool bPresent = pRoot->IsStream( N_DRAWING );
/*N*/ 	if( bDraw || ( !bRdWr && bPresent ) )
/*N*/ 	{
/*N*/ 		if( bRdWr || !( nFltFlags & FLTF_NO_DRAWINGS ) )
/*N*/ 			pDrawing = pRoot->OpenStream( N_DRAWING, nMode );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( bPresent && bUseDrawStream )
/*?*/ 			pRoot->Remove( N_DRAWING );
/*N*/ 		pDrawing.Clear();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Absatzvorlagen
/*N*/ 	if( bRdWr || pRoot->IsStream( SfxStyleSheetBasePool::GetStreamName() ) )
/*N*/ 		pStyles = pRoot->OpenStream
/*N*/ 						( SfxStyleSheetBasePool::GetStreamName(), nMode );
/*N*/ 	else
/*?*/ 		pStyles.Clear();
/*N*/ 	// Numerierungs-Regeln
/*N*/ 	if( nFFVersion > SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		if( bRdWr || pRoot->IsStream( N_NUMRULES ) )
/*N*/ 			pNumRules = pRoot->OpenStream( N_NUMRULES, nMode );
/*N*/ 		else
/*?*/ 			pNumRules.Clear();
/*N*/ 	}
/*N*/ 	// Seitenvorlagen
/*N*/ 	if( bRdWr || pRoot->IsStream( N_PAGESTYLES ) )
/*N*/ 		pPageStyles = pRoot->OpenStream( N_PAGESTYLES, nMode );
/*N*/ 	else
/*?*/ 		pPageStyles.Clear();
/*N*/ 	// Inhalt
/*N*/ 	if( bRdWr || pRoot->IsStream( N_DOC ) )
/*N*/ 		pContents = pRoot->OpenStream( N_DOC, nMode );
/*N*/ 	else
/*?*/ 		pContents.Clear();
/*N*/ 	if( CheckStreams() )
/*N*/ 	{
/*N*/ 		// Versions-Nummer vom Storage an die Streams uebernehmen
/*N*/ 		// Setting the char set is in fact only required for
/*N*/ 		// saving files.
/*N*/ 		if( pDrawing.Is() )
/*N*/ 			pDrawing->SetVersion( nFFVersion );
/*N*/ 
/*N*/ 		pStyles->SetVersion( nFFVersion );
/*N*/ 		if( pNumRules.Is() )
/*N*/ 		{
/*N*/ 			pNumRules->SetVersion( nFFVersion );
/*N*/ 			pNumRules->SetStreamCharSet( eSrcSet );
/*N*/ 		}
/*N*/ 
/*N*/ 		pPageStyles->SetVersion( nFFVersion );
/*N*/ 		pPageStyles->SetStreamCharSet( eSrcSet );
/*N*/ 
/*N*/ 		pContents->SetVersion( nFFVersion );
/*N*/ 		pContents->SetStreamCharSet( eSrcSet );
/*N*/ 
/*N*/ 		if( bRdWr )
/*N*/ 		{
/*N*/ 			sal_uInt16 nComprMode = COMPRESSMODE_NONE;
/*N*/ 			if( SOFFICE_FILEFORMAT_40 <= pRoot->GetVersion() &&
/*N*/ 				pDoc->GetInfo()->IsSaveGraphicsCompressed() )
/*N*/ 				nComprMode |= COMPRESSMODE_ZBITMAP;
/*N*/ 			if( SOFFICE_FILEFORMAT_40 < pRoot->GetVersion() &&
/*N*/ 				pDoc->GetInfo()->IsSaveOriginalGraphics() )
/*N*/ 				nComprMode |= COMPRESSMODE_NATIVE;
/*N*/ 			if( COMPRESSMODE_NONE != nComprMode )
/*N*/ 			{
/*N*/ 				if( pDrawing.Is() )
/*N*/ 					pDrawing->SetCompressMode( nComprMode );
/*N*/ 				pStyles->SetCompressMode( nComprMode );
/*N*/ 				if( pNumRules.Is() )
/*N*/ 					pNumRules->SetCompressMode( nComprMode );
/*N*/ 				pPageStyles->SetCompressMode( nComprMode );
/*N*/ 				pContents->SetCompressMode( nComprMode );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Exort-Information anlegen
/*N*/ 		delete pExportInfo;
/*N*/ 		if( bRdWr && nFFVersion <= SOFFICE_FILEFORMAT_40 )
/*N*/ 			pExportInfo = new Sw3ExportInfo;
/*N*/ 		// Falls noch keine Marks gesammelt wurden, ist jetzt die Zeit!
/*N*/ 		// MIB 11.12.96: Es werden jetzt text::Bookmarks gesammelt, wenn sie
/*N*/ 		// benoetigt werden und auch nur noch die, die benoetigt werden.
/*N*/ 		// Das ist sauberer!
/*N*/ //		if( !pMarks && bRdWr )
/*N*/ //			CollectMarks( NULL, sal_False );
/*N*/ 		bOut = bRdWr;
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		CloseStreams();
/*?*/ 		return sal_False;
/*N*/ 	}
/*N*/ }

// Alle offenen Streams freigeben. Die Root bleibt so, wie sie war.

/*N*/ void Sw3IoImp::CloseStreams()
/*N*/ {
/*N*/ 	pCurIosys = NULL;
/*N*/ 	pStrm 	  = NULL;
/*N*/ 	pContents.Clear();
/*N*/ 	pDrawing.Clear();
/*N*/ 	pStyles.Clear();
/*N*/ 	pPageStyles.Clear();
/*N*/ 	pNumRules.Clear();
/*N*/ 	// Sicherheitshalber
/*N*/ 	pBlkDir.Clear();
/*N*/ 	// Nach dem Einlesen alle nicht benutzten Marks loeschen
/*N*/ 	// (kann auch nach dem Schreiben passieren, ist egal)
/*N*/ 	CleanupMarks();
/*N*/ 
/*N*/ 	// Brauchen wir nicht mehr
/*N*/ 	delete pExportInfo; pExportInfo = NULL;
/*N*/ 	delete pTOXs; pTOXs = NULL;
/*N*/ 	delete pRedlines; pRedlines = NULL;
/*N*/ 	delete pRedlineMarks; pRedlineMarks = NULL;
/*N*/ 	delete pMarks; pMarks = NULL;
/*N*/ 	delete pBookmarks; pBookmarks = NULL;
/*N*/ 	delete p30OLENodes; p30OLENodes = NULL;
/*N*/ 	delete pTblLineBoxFmts; pTblLineBoxFmts = NULL;
/*N*/ 
/*N*/ 	// der Returncode sollte so bleiben, wie er ist
/*N*/ 	sal_uInt32 n = nRes;
/*N*/ 	Reset();
/*N*/ 	nRes = n;
/*N*/ }

// Sind alle Streams auch da?

/*N*/ sal_Bool Sw3IoImp::CheckStreams()
/*N*/ {
/*N*/ 	sal_uInt16 n = pRoot.Is() && ( pRoot->GetError() == SVSTREAM_OK )
/*N*/ 		   && pPageStyles.Is() && ( pPageStyles->GetError() == SVSTREAM_OK )
/*N*/ 		   && pStyles.Is() && ( pStyles->GetError() == SVSTREAM_OK )
/*N*/ 		   && pContents.Is() && ( pContents->GetError() == SVSTREAM_OK );
/*N*/ 	if( n && pRoot->GetVersion() > SOFFICE_FILEFORMAT_40 )
/*N*/ 		n = n && pNumRules.Is() && ( pNumRules->GetError() == SVSTREAM_OK );
/*N*/ 	return sal_Bool( n );
/*N*/ }

// Record oeffnen

/*N*/ sal_Bool Sw3IoImp::OpenRec( sal_uInt8 cType )
/*N*/ {
/*N*/ 	sal_Bool bRes = sal_True;
/*N*/ 	sal_uInt16 nLvl = aRecTypes.Count();
/*N*/ 	ASSERT( nLvl == aRecSizes.Count(), "OpenRec: falscher Level" );
/*N*/ 	sal_uInt32 nPos = pStrm->Tell();
/*N*/ 	if( bOut )
/*N*/ 	{
/*N*/ 		if( nLvl >= 31 && (!pRoot.Is() || IsSw31Or40Export())  )
/*N*/ 			Error( ERR_SWG_LARGE_DOC_ERROR );
/*N*/ 		aRecTypes.Insert( cType, nLvl );
/*N*/ 		aRecSizes.Insert( nPos, nLvl );
/*N*/ 		*pStrm << (sal_uInt32) 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		sal_uInt32 nVal;
/*N*/ 		*pStrm >> nVal;
/*N*/ 		sal_uInt8 cRecTyp = (sal_uInt8)nVal;
/*N*/ 		aRecTypes.Insert( cRecTyp, nLvl );
/*N*/ 		sal_uInt32 nSize = nVal >> 8;
/*N*/ 		if( LONG_RECSIZE == nSize && IsVersion( SWG_LONGRECS ) )
/*N*/ 		{
                sal_uInt32 nTmp = GetRecordSize( nPos );
/*?*/           if( nTmp != SAL_MAX_UINT32 )
/*?*/               nSize = nTmp;
/*N*/ 		}
/*N*/ 
/*N*/ 		aRecSizes.Insert( nPos + nSize, nLvl );
/*N*/ 		if( !nVal || cRecTyp != cType )
/*N*/ 		{
/*N*/ 			// Bei Fehlern wird ein EOF-Record erzeugt
/*N*/ 			// fix #24089#: Ein 0-Record-Header ist in jedem Fall
/*N*/ 			// ein Formatfehler und sollte nicht zu einer Endlos-Schleife
/*N*/ 			// fuehren
/*?*/ 			ASSERT( nVal, "OpenRec: Record-Header ist 0" );
/*?*/ 			ASSERT( cRecTyp == cType,
/*?*/ 					"OpenRec: Falscher Block-Header" );
/*?*/ 		 	Error();
/*?*/ 			aRecTypes[nLvl] = SWG_EOF;
/*?*/ 			aRecSizes[nLvl] = pStrm->Tell();
/*?*/ 			bRes = sal_False;
/*N*/ 		}
/*N*/ 		if( pStrm->GetErrorCode() != SVSTREAM_OK || pStrm->IsEof() )
/*N*/ 		{
/*?*/ 			if( pStrm->GetErrorCode() != SVSTREAM_OK &&
/*?*/ 				IsWarning(pStrm->GetErrorCode()) )
/*?*/ 			{
/*?*/ 				Warning( pStrm->GetErrorCode() );
/*?*/ 				pStrm->ResetError();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				// Bei Fehlern wird ein EOF-Record erzeugt
/*?*/ 			 	Error( ERR_SWG_READ_ERROR );
/*?*/ 				aRecTypes[nLvl] = SWG_EOF;
/*?*/ 				aRecSizes[nLvl] = pStrm->Tell();
/*?*/ 				bRes = sal_False;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRes;
/*N*/ }

// Record schliessen

/*N*/ void Sw3IoImp::CloseRec( sal_uInt8 cType )
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aRecTypes.Count();
/*N*/ 	ASSERT( nLvl == aRecSizes.Count(), "CloseRec: falscher Level" );
/*N*/ 	ASSERT( nLvl, "CloseRec: keine Schachtelung" );
/*N*/ 	if( nLvl )
/*N*/ 	{
/*N*/ 		nLvl--;
/*N*/ 		ASSERT( cType == aRecTypes[nLvl],
/*N*/ 				"CloseRec: Falscher Block-Header" );
/*N*/ 		sal_uInt32 nPos = pStrm->Tell();
/*N*/ 		if( bOut )
/*N*/ 		{
/*N*/ 			sal_uInt32 nBgn = aRecSizes[nLvl];
/*N*/ 			pStrm->Seek( nBgn );
/*N*/ 			sal_uInt32 nSize = nPos - nBgn;
/*N*/ 			if( nSize > MAX_SMALL_RECSIZE )
/*N*/ 			{
/*?*/ 				if( !pRoot.Is() || IsSw31Or40Export() )
/*?*/ 				{
/*?*/ 				 	Error( ERR_SWG_LARGE_DOC_ERROR );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
                        InsertRecordSize( nBgn, nSize );
/*?*/                  nSize = LONG_RECSIZE;
/*?*/ 				}
/*N*/ 			}
/*N*/ 			sal_uInt32 nVal = ( nSize << 8 ) | aRecTypes[nLvl];
/*N*/ 			*pStrm << nVal;
/*N*/ 			pStrm->Seek( nPos );
/*N*/ 			if( pStrm->GetError() != SVSTREAM_OK )
/*?*/ 			 	Error( ERR_SWG_WRITE_ERROR );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			sal_uInt32 n = aRecSizes[nLvl];
/*N*/ 			ASSERT( n >= nPos, "CloseRec: zu viele Daten gelesen" );
/*N*/ 			if( n != nPos )
/*N*/ 			{
/*?*/ 				pStrm->Seek( n );
/*?*/ 				if( n < nPos )
/*?*/ 				 	Error();
/*?*/ 				else
/*?*/ 					Warning();
/*N*/ 			}
/*N*/ 			if( pStrm->GetErrorCode() != SVSTREAM_OK )
/*N*/ 			{
/*?*/ 				if( IsWarning(pStrm->GetErrorCode()) )
/*?*/ 				{
/*?*/ 					Warning( pStrm->GetErrorCode() );
/*?*/ 					pStrm->ResetError();
/*?*/ 				}
/*?*/ 				else
/*?*/ 				 	Error( ERR_SWG_READ_ERROR );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		aRecTypes.Remove( nLvl, 1 );
/*N*/ 		aRecSizes.Remove( nLvl, 1 );
/*N*/ 	}
/*N*/ }

void Sw3IoImp::InsertRecordSize( sal_uInt32 nPos, sal_uInt32 nSize )
{
    if( !pRecSizes )
        pRecSizes = new Sw3RecordSizeTable;

    pRecSizes->Insert( nPos, nSize );
}

sal_uInt32 Sw3IoImp::GetRecordSize( sal_uInt32 nPos )
{
    sal_uInt32 nRet = SAL_MAX_UINT32;
    ULONG nTablePos;

    if( pRecSizes &&
        TABLE_ENTRY_NOTFOUND != pRecSizes->SearchKey( nPos, &nTablePos ) )
    {
        nRet = pRecSizes->GetObject( nTablePos );
    }
    ASSERT( nRet != SAL_MAX_UINT32, "Record-Size nicht gefunden" );

    return nRet;
}

void Sw3IoImp::FlushRecSizes()
{
    delete pRecSizes;
    pRecSizes = 0;
}

// Zurueckliefern, wie viele Bytes ein Record noch enthaelt

/*N*/ ULONG Sw3IoImp::BytesLeft()
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aRecSizes.Count();
/*N*/ 	ULONG n = 0;
/*N*/ 	if( nLvl && !nRes )
/*N*/ 	{
/*N*/ 		sal_uInt32 nEndPos = aRecSizes[ nLvl-1 ];
/*N*/ 		ULONG nPos = pStrm->Tell();
/*N*/ 		if( nEndPos > nPos )
/*N*/ 			n = nEndPos - nPos;
/*N*/ 	}
/*N*/ 
/*N*/ 	return n;
/*N*/ }

// was gibt es als naechstes?

/*N*/ sal_uInt8 Sw3IoImp::Peek()
/*N*/ {
/*N*/ 	sal_uInt8 c = SWG_EOF;
/*N*/ 	if( !nRes )
/*N*/ 	{
/*N*/ 		sal_uInt32 nPos = pStrm->Tell();
/*N*/ 		*pStrm >> c;
/*N*/ 		pStrm->Seek( nPos );
/*N*/ 		if( pStrm->GetErrorCode() != SVSTREAM_OK )
/*N*/ 		{
/*?*/ 			if( IsWarning(pStrm->GetErrorCode()) )
/*?*/ 			{
/*?*/ 				Warning( pStrm->GetErrorCode() );
/*?*/ 				pStrm->ResetError();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 			 	Error( ERR_SWG_READ_ERROR );
/*?*/ 				c = SWG_EOF;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return c;
/*N*/ }

// Den aktuellen Record ueberspringen

/*N*/ void Sw3IoImp::SkipRec()
/*N*/ {
/*N*/ 	sal_uInt8 c = Peek();
/*N*/ 	OpenRec( c );
/*N*/ 	pStrm->Seek( aRecSizes[aRecSizes.Count()-1] );
/*N*/ 	CloseRec( c );
/*N*/ }

void Sw3IoImp::InRecSizes( ULONG nRecPos )
{
    ASSERT( !pRecSizes, "RecSize-Tabelle existiert noch" );
    ASSERT( nRecPos, "Keine Position der RecSize-Tabelle" );

    if( nRecPos )
    {
        pRecSizes = new Sw3RecordSizeTable;

        // Wenn der Stream vor dem Record steht, wird der Record
        // normal gelesen. Sonst wird erstmal zu ihm geseekt und
        // nach dem Lesen wieder am die aktuelle Stelle zurueck.
        ULONG nOldPos = pStrm->Tell();
        if( nOldPos != nRecPos )
            pStrm->Seek( nRecPos );

        sal_uInt32 nCount, nPos, nSize;
        OpenRec( SWG_RECSIZES );
        OpenFlagRec();
        *pStrm >> nCount;
        CloseFlagRec();

        for( sal_uInt32 i=0; i < nCount; i++ )
        {
            *pStrm >> nPos >> nSize;
            pRecSizes->Insert( nPos, nSize );
        }

        CloseRec( SWG_RECSIZES );

        if( nOldPos != nRecPos )
            pStrm->Seek( nOldPos );
    }
}

ULONG Sw3IoImp::OutRecSizes()
{
    ULONG nRecPos = 0;
    if( pRecSizes )
    {
        sal_uInt32 nCount = pRecSizes->Count();
        ASSERT( nCount, "RecSize-Tabelle ist leer" );

        if( (nCount*8 + 4) > MAX_SMALL_RECSIZE )
        {
            // Der Record darf kein langer sein, sonst haben wir ein
            // Problem ...
            Error( ERR_SWG_LARGE_DOC_ERROR );
        }
        else
        {
            nRecPos = pStrm->Tell();
            OpenRec( SWG_RECSIZES );

            *pStrm << (sal_uInt8)0x04 << (sal_uInt32)nCount;

            for( sal_uInt32 i=0; i < nCount; i++ )
            {
                *pStrm  << (sal_uInt32)pRecSizes->GetObjectKey( i )
                        << (sal_uInt32)pRecSizes->GetObject( i );
            }

            CloseRec( SWG_RECSIZES );
        }
    }

    return nRecPos;
}

// Ein Flag-Byte enthaelt immer die Anzahl Daten im unteren Nybble.
// Hier wird das Flagbyte mit der aktuellen Position ausgewertet.

/*N*/ sal_uInt8 Sw3IoImp::OpenFlagRec()
/*N*/ {
/*N*/ 	sal_uInt8 cFlags;
/*N*/ 	*pStrm >> cFlags;
/*N*/ 	nFlagRecEnd = pStrm->Tell() + ( cFlags & 0x0F );
/*N*/ 	return cFlags;
/*N*/ }

// Die folgende Routine fuehrt einen Skip durch, falls noch nicht
// alle Daten gelesen wurden.

/*N*/ void Sw3IoImp::CloseFlagRec()
/*N*/ {
/*N*/ 	ASSERT( pStrm->Tell() <= nFlagRecEnd, "Zu viele Binaerdaten gelesen" );
/*N*/ 	if( pStrm->Tell() != nFlagRecEnd )
/*N*/ 		Warning(),
/*?*/ 		pStrm->Seek( nFlagRecEnd );
/*N*/ }

// Merken der Position fuer einen 16-Bit-Wert, der spaeter dort eingefuegt wird
// Der Parameter ist nur Dokumentation

/*N*/ void Sw3IoImp::OpenValuePos16( sal_uInt16 )
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aValPositions.Count();
/*N*/ 	if( nLvl >= 31 && IsSw31Or40Export() )
/*N*/ 	 	Error( ERR_SWG_LARGE_DOC_ERROR );
/*N*/ 	aValPositions.Insert( pStrm->Tell(), nLvl  );
/*N*/ 	*pStrm << (sal_uInt16) 0;
/*N*/ }

// Einfuegen eines 16-Bit-Werts an der obersten Stack-Position

/*N*/ void Sw3IoImp::CloseValuePos16( sal_uInt16 nVal )
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aValPositions.Count();
/*N*/ 	ASSERT( nLvl, "Unterlauf ValuePos-Stack" );
/*N*/ 	if( nLvl )
/*N*/ 	{
/*N*/ 		nLvl--;
/*N*/ 		sal_uInt32 nPos = pStrm->Tell();
/*N*/ 		pStrm->Seek( aValPositions[nLvl] );
/*N*/ 		aValPositions.Remove( nLvl, 1 );
/*N*/ 		*pStrm << nVal;
/*N*/ 		pStrm->Seek( nPos );
/*N*/ 	}
/*N*/ }

// Merken der Position fuer einen 32-Bit-Wert, der spaeter dort eingefuegt wird
// Der Parameter ist nur Dokumentation

/*N*/ void Sw3IoImp::OpenValuePos32( sal_uInt32 )
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aValPositions.Count();
/*N*/ 	if( nLvl >= 31 && IsSw31Or40Export() )
/*N*/ 	 	Error( ERR_SWG_LARGE_DOC_ERROR );
/*N*/ 	aValPositions.Insert( pStrm->Tell(), nLvl );
/*N*/ 	*pStrm << (sal_uInt32) 0;
/*N*/ }

// Einfuegen eines 32-Bit-Werts an der obersten Stack-Position

/*N*/ void Sw3IoImp::CloseValuePos32( sal_uInt32 nVal )
/*N*/ {
/*N*/ 	sal_uInt16 nLvl = aValPositions.Count();
/*N*/ 	ASSERT( nLvl, "Unterlauf ValuePos-Stack" );
/*N*/ 	if( nLvl )
/*N*/ 	{
/*N*/ 		nLvl--;
/*N*/ 		sal_uInt32 nPos = pStrm->Tell();
/*N*/ 		pStrm->Seek( aValPositions[nLvl] );
/*N*/ 		aValPositions.Remove( nLvl, 1 );
/*N*/ 		*pStrm << nVal;
/*N*/ 		pStrm->Seek( nPos );
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::CheckIoError( SvStream* p )
/*N*/ {
/*N*/ 	const sal_uInt32 nErr = p->GetErrorCode();
/*N*/ 	if( nErr != SVSTREAM_OK )
/*N*/ 	{
/*?*/ 		if( IsWarning(nErr) )
/*?*/ 		{
/*?*/ 			nWarn = p->GetErrorCode();
/*?*/ 			p->ResetError();
/*?*/ 		}
/*?*/ 		else if( nErr == SVSTREAM_DISK_FULL )
/*?*/ 			nRes = ERR_W4W_WRITE_FULL;
/*?*/ 		else
/*?*/ 			nRes = bOut ? ERR_SWG_WRITE_ERROR : ERR_SWG_READ_ERROR;
/*N*/ 	}
/*N*/ }

// Komprimiertes Speichern von sal_uInt32. Das Speichern erfolgt aehnlich wie
// im UTF-Zeichensatz:
//
// von			bis         bytes	start byte (binaer)
// ----------------------------------------------------
// 0x0000 0000	0x0000 007f 1		0x0xxx xxxx
// 0x0000 0080	0x0000 3fff 2		0x10xx xxxx
// 0x0000 4000	0x001f ffff 3		0x110x xxxx
// 0x0020 0000	0x0fff ffff	4		0x1110 xxxx
// 0x1000 0000	0xffff ffff	5		0x1111 0??? xxxx xxxx
//
// Sofern zwei oder view Bytes gleichzeitig geschrieben werden koennten,
// werden sie als sal_uInt16 oder sal_uInt32 am Stueck geschrieben.
// Bei 5 Bytes bleiben die ersten 3 Bits unbenutzt. Sie werden beim
// Einlesen ignoriert und koennen deshalb spaeter mal fuer etwas anderes
// benutzt werden.

/*N*/ sal_uInt32 Sw3IoImp::InULong( SvStream& rStrm )
/*N*/ {
/*N*/ 	sal_uInt32 n32;
/*N*/ 	sal_uInt16 n16;
/*N*/ 	sal_uInt8 n8;
/*N*/ 
/*N*/ 	rStrm >> n8;
/*N*/ 	if( (n8 & 0x80) == 0x00 )
/*N*/ 	{
/*N*/ 		// 0x0www wwww
/*N*/ 		// --> 0x0000 0000  0000 0000  0000 0000  0www wwww
/*N*/ 
/*N*/ 		n32 = (sal_uInt32)(n8 & 0x7F);
/*N*/ 	}
/*N*/ 	else if( (n8 & 0xC0) == 0x80 )
/*N*/ 	{
/*N*/ 		// 0x10xx xxxx  wwww wwww
/*N*/ 		// --> 0x0000 0000  0000 0000  00xx xxxx  wwww wwww
/*N*/ 
/*?*/ 		n32 = (sal_uInt32)(n8 & 0x3F);
/*?*/ 		n32 <<= 8;
/*?*/ 
/*?*/ 		rStrm >> n8;
/*?*/ 		n32 += (sal_uInt32)n8;
/*?*/ 	}
/*?*/ 	else if( (n8 & 0xE0) == 0xC0 )
/*?*/ 	{
/*?*/ 		// 0x110y yyyy  wwww wwww  xxxx xxxx
/*?*/ 		// --> 0x0000 0000  000y yyyy  xxxx xxxx  wwww wwww
/*?*/ 
/*?*/ 		n32 = (sal_uInt32)(n8 & 0x1F);
/*?*/ 		n32 <<= 16;
/*?*/ 
/*?*/ 		rStrm >> n16;
/*?*/ 		n32 += (sal_uInt32)n16;
/*?*/ 	}
/*?*/ 	else if( (n8 & 0xF0) == 0xE0 )
/*?*/ 	{
/*?*/ 		// 0x1110 zzzz  yyyy yyyy  wwww wwww  xxxx xxxx
/*?*/ 		// --> 0x0000 zzzz  yyyy yyyy  xxxx xxxx  wwww wwww
/*?*/ 
/*?*/ 		n32 = (sal_uInt32)(n8 & 0x0F);
/*?*/ 		n32 <<= 8;
/*?*/ 
/*?*/ 		rStrm >> n8;
/*?*/ 		n32 += (sal_uInt32)n8;
/*?*/ 		n32 <<= 16;
/*?*/ 
/*?*/ 		rStrm >> n16;
/*?*/ 		n32 += (sal_uInt32)n16;
/*?*/ 	}
/*?*/ 	else if( (n8 & 0xF8) == 0xF0 )
/*?*/ 	{
/*?*/ 		// 0x1111 0???  wwww wwww  xxxx xxxx  yyyy yyyy  zzzz zzzz
/*?*/ 		// --> zzzz zzzz  yyyy yyyy  xxxx xxxx  wwww wwww
/*?*/ 		rStrm >> n32;
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ //	 	Error( ERR_SWG_READ_ERROR );
/*?*/ 		n32 = 0xABADCAFE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return (sal_uInt32)n32;
/*N*/ }

/*N*/ void Sw3IoImp::OutULong( SvStream& rStrm, sal_uInt32 n32 )
/*N*/ {
/*N*/ 	if( n32 < 0x00000080 )
/*N*/ 	{
/*N*/ 		// 0w0000 0000  0000 0000  0000 0000  0www wwww
/*N*/ 		// -> 0w0www wwww
/*N*/ 
/*N*/ 		rStrm << (sal_uInt8)n32;
/*N*/ 	}
/*N*/ 	else if( n32 < 0x00004000 )
/*N*/ 	{
/*N*/ 		// 0x0000 0000  0000 0000  00xx xxxx  wwww wwww
/*N*/ 		// --> 0x10xx xxxx  wwww wwww
/*N*/ 
/*?*/ 		n32 |= 0x00008000;
/*?*/ 		rStrm << (sal_uInt8)(n32 >> 8);
/*?*/ 		rStrm << (sal_uInt8)n32;
/*?*/ 	}
/*?*/ 	else if( n32 < 0x00200000 )
/*?*/ 	{
/*?*/ 		// 0x110y yyyy  xxxx xxxx  wwww wwww
/*?*/ 		// --> 0x0000 0000  000y yyyy  wwww wwww  xxxx xxxx
/*?*/ 
/*?*/ 		n32 |= 0x00C00000;
/*?*/ 		rStrm << (sal_uInt8)(n32 >> 16);
/*?*/ 		rStrm << (sal_uInt16)n32;
/*?*/ 	}
/*?*/ 	else if( n32 < 0x10000000 )
/*?*/ 	{
/*?*/ 		// 0x1110 zzzz  yyyy yyyy  xxxx xxxx  wwww wwww
/*?*/ 		// --> 0x0000 zzzz  yyyy yyyy  wwww wwww  xxxx xxxx
/*?*/ 
/*?*/ 		n32 |= 0xE0000000;
/*?*/ 		rStrm << (sal_uInt8)(n32 >> 24);
/*?*/ 		rStrm << (sal_uInt8)(n32 >> 16);
/*?*/ 		rStrm << (sal_uInt16)n32;
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		// zzzz zzzz  yyyy yyyy  xxxx xxxx  wwww wwww
/*?*/ 		// --> 0x1111 00000  wwww wwww  xxxx xxxx  yyyy yyyy  zzzz zzzz
/*?*/ 
/*?*/ 		rStrm << (sal_uInt8)0xF0;
/*?*/ 		rStrm << (sal_uInt32)n32;
/*N*/ 	}
/*N*/ }

/*************************************************************************
*
*		Prozentanzeige
*
*************************************************************************/

/*N*/ void Sw3IoImp::OpenPercentBar( sal_uInt32 n1, sal_uInt32 n2 )
/*N*/ {
/*N*/ 	if( !bBlock )
/*N*/ 	{
/*N*/ 		nCurPercent = n1;
/*N*/ 		nEndPercent = n2;
/*N*/ 		::binfilter::StartProgress( bOut ? STR_STATSTR_SWGWRITE : STR_STATSTR_SWGREAD,
/*N*/ 						 n1, n2, pDoc->GetDocShell() );
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void Sw3IoImp::ClosePercentBar()
/*N*/ {
/*N*/ 	if( !bBlock )
/*N*/ 		::binfilter::EndProgress( pDoc->GetDocShell() );
/*N*/ }

/*N*/ void Sw3IoImp::SetPercentBar( sal_uInt32 n )
/*N*/ {
/*N*/ 	//ASSERT( pStrm!=pContents || (n-nCurPercent)<1024, "SetPercentBar zu lange nicht mehr aufgerufen!" );
/*N*/ 	if( !bBlock && (pStrm == &pContents ) && 	// nicht fuer SwPageStyles
/*N*/ 		( n > nCurPercent ) && ( n <= nEndPercent ) )
/*N*/ 		::binfilter::SetProgressState( nCurPercent = n, pDoc->GetDocShell() );
/*N*/ }

/*************************************************************************
*
*		Drawing Layer
*
*************************************************************************/

#define SIGN_FRMIDS 0x444D

/*N*/ void Sw3IoImp::LoadDrawingLayer()
/*N*/ {
/*N*/ 	if( !pDrawing.Is() || ( nFltFlags & FLTF_NO_DRAWINGS )
/*N*/ #ifdef DBG_UTIL
/*N*/ 			|| SwViewOption::IsTest9()
/*N*/ #endif
/*N*/ 			)
/*N*/ 	{
/*N*/ 		// gar kein Drawing-Stream da!
/*N*/ //		nGblFlags |= SW3F_NODRAWING;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wenn der erste Stream gelesen wird, kann es noch keine RecSizes geben
/*N*/ 	ASSERT( !HasRecSizes(), "Hier darf es noch keine RecSizes geben" );
/*N*/ 	if( HasRecSizes() )
            FlushRecSizes();
/*N*/ 
/*N*/ 	pDrawing->Seek( 0L );
/*N*/ 	pDrawing->SetBufferSize( SW3_BSR_DRAWING );
/*N*/ 	SdrModel* pModel = NULL;
/*N*/ 	if( bInsert )
/*N*/   {
/*N*/       //!!Diesen Code parallel zum Code fuer die Pools im docnew.cxx
/*N*/       //pflegen.
/*?*/       SfxItemPool *pAttrPool = new SfxItemPool(
/*?*/                                       String::CreateFromAscii("SWG"),
/*?*/                                       POOLATTR_BEGIN, POOLATTR_END-1,
/*?*/                                       aSlotTab, aAttrTab );
/*?*/       SfxItemPool *pSdrPool = new SdrItemPool( pAttrPool );
/*?*/       SfxItemPool *pEEgPool = EditEngine::CreatePool();
/*?*/       pSdrPool->SetSecondaryPool( pEEgPool );
/*?*/       pAttrPool->FreezeIdRanges();
/*?*/       SdrModel* pInsModel = new SwDrawDocument( pAttrPool, pDoc->GetDocShell() );
/*?*/       pSdrPool->Load( *pDrawing );
/*?*/       if( pDrawing->GetError() == SVSTREAM_OK )
/*?*/           *pDrawing >> *pInsModel;
/*?*/       if( pDrawing->GetError() == SVSTREAM_OK )
/*?*/       {
/*?*/           pModel = pDoc->MakeDrawModel();
/*?*/           // Objekte werden vor die aktuellen Objekte eingefuegt
/*?*/           SdrPage* pInsPage = pInsModel->GetPage( 0 );
/*?*/           SdrPage* pPage = pModel->GetPage( 0 );
/*?*/           pPage->RecalcObjOrdNums();
/*?*/           nZOrderOff = pPage->GetObjCount();
/*?*/ 
/*?*/           for( sal_uInt32 n = pInsPage->GetObjCount(); n--; )
/*?*/           {
/*?*/               SdrObject* pObj = pInsPage->RemoveObject( 0 );
/*?*/               if( pObj )
/*?*/                   pPage->InsertObject( pObj );
/*?*/               else
/*?*/                   ASSERT( !this, "wo ist das DrawObject?" );
/*?*/           }
/*?*/       }
/*?*/       delete pInsModel;
/*?*/       pSdrPool->Delete();
/*?*/       pAttrPool->SetSecondaryPool(0);
/*?*/       pSdrPool->SetSecondaryPool(0);
/*?*/       delete pAttrPool;
/*?*/       delete pSdrPool;
/*?*/       delete pEEgPool;
/*N*/   }
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pModel = pDoc->MakeDrawModel();
/*N*/ 		SfxItemPool *pDrawPool = pModel->GetItemPool().GetSecondaryPool();
/*N*/ 		ASSERT( pDrawPool, "DrawPool not found" );
/*N*/ 		pDrawPool->Load( *pDrawing );
/*N*/ 		if( pDrawing->GetError() == SVSTREAM_OK )
/*N*/ 		{
/*N*/ 			*pDrawing >> *pModel;
/*N*/ 			// for Bug 73110 - decrement all item refcounts
/*N*/ 			pDrawPool->LoadCompleted();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pDrawing->GetError() != SVSTREAM_OK )
/*N*/ 	{
/*N*/ 		// Wenn der doofe Drawing Layer mal wieder Probleme gemacht hat,
/*N*/ 		// einfach keine Zeichenobjekte laden!
/*?*/ 		pDoc->InitDrawModel();
/*?*/ 		// Keinen Fehler setzen, sondern Dokument weiter bearbeiten,
/*?*/ 		// um so viel wie moeglich vom Dok zu retten.
/*?*/ 		bNoDrawings = sal_True;
/*?*/ 		pDrawing->SetBufferSize( 0 );
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		// Der Drawing Layer ist jetzt verfuegbar
/*N*/ 		nGblFlags &= ~SW3F_NODRAWING;
/*N*/ 	// Zu guter Letzt muss noch fuer jeden frame::Frame ein FlyDraw-Objekt
/*N*/ 	// angelegt werden, damit die Z-Order der Zeichenobjekte wieder
/*N*/ 	// stimmt.
/*N*/ 	sal_uInt16 nSign = 0, nFrmIds = 0;
/*N*/ 	*pDrawing >> nSign >> nFrmIds;
/*N*/ 	if( nSign == SIGN_FRMIDS )
/*N*/ 	{
/*N*/ 		pModel = pDoc->MakeDrawModel();
/*N*/ 		SdrPage* pPage = pModel->GetPage( 0 );
/*N*/ 		for( sal_uInt16 i = 0; i < nFrmIds; i++ )
/*N*/ 		{
/*N*/ 			sal_uInt32 nFrmPos;
/*N*/ 			*pDrawing >> nFrmPos;
/*N*/ 			if( pDrawing->GetError() != SVSTREAM_OK || pDrawing->IsEof() )
/*N*/ 			{
/*N*/ 			 	Error( ERR_SWG_READ_ERROR );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			pPage->InsertObject( new SwFlyDrawObj, nFrmPos + nZOrderOff );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nHiddenDrawObjs = ULONG_MAX;
/*N*/ 	if( !nRes && SOFFICE_FILEFORMAT_50 <= pDrawing->GetVersion() )
/*N*/ 	{
/*N*/ 		sal_uInt32 n;
/*N*/ 		*pDrawing >> n;
// Nowadays all objects remain in the page, but might be in different layers
//		if( !pDrawing->IsEof() && pModel )
//		{
//			sal_uInt32 nCount = pModel->GetPage( 0 )->GetObjCount();
//			if( n <= nCount )
//				nHiddenDrawObjs = nCount - n;
//		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CheckIoError( pDrawing );
/*N*/ 	pDrawing->SetBufferSize( 0 );
/*N*/ }

/*N*/ void Sw3IoImp::SaveDrawingLayer()
/*N*/ {
/*N*/ 	// Wenn kein Stream da ist, gibt es im Layer nix zu saven.
/*N*/ 	if( !pDrawing.Is() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if( HasRecSizes() )	// sicher ist sicher
/*?*/       FlushRecSizes();
/*N*/ 
/*N*/ 	pDrawing->SetSize( 0L );
/*N*/ 	// Stream weg, wenn wir kein OLE-Objekt sind
/*N*/ 	//if( pRoot->IsRoot() )
/*N*/ 	//	pRoot->Commit();
/*N*/ 	pDrawing->SetBufferSize( SW3_BSW_DRAWING );
/*N*/ 
/*N*/ 	// Den Pool nicht vergessen...
/*N*/ 	SdrModel* pModel = pDoc->GetDrawModel();
/*N*/ 	ASSERT( pModel, "SaveDrawingLayer without DrawModel" );
/*N*/ 	//Auf die Korrektheit der OrdNums sind wir schon angewiesen.
/*N*/ 	pModel->GetPage( 0 )->RecalcObjOrdNums();
/*N*/ 
/*N*/ //-/	pModel->PrepareStore();
/*N*/ 	pModel->PreSave();
/*N*/ 
/*N*/ 	SfxItemPool *pDrawIPool = pModel->GetItemPool().GetSecondaryPool();
/*N*/ 	ASSERT( pDrawIPool, "DrawPool not found" );
/*N*/ 
/*N*/ 	// Die FF-Version muss an einem Master-Pool stehen
/*N*/ 	long nFFVersion = pDrawing->GetVersion();
/*N*/ 	long nOldFFVersion = pModel->GetItemPool().GetFileFormatVersion();
/*N*/ 	ASSERT( IsSw31Export() ? nFFVersion==SOFFICE_FILEFORMAT_31
/*N*/ 						   : (nFFVersion==SOFFICE_FILEFORMAT_40 ||
/*N*/ 							  nFFVersion==SOFFICE_FILEFORMAT_50),
/*N*/ 			"FF-Version am Drawing-Layer-Stream stimmt nicht" );
/*N*/ 	pModel->GetItemPool().SetFileFormatVersion( (sal_uInt16)nFFVersion );
/*N*/ 
/*N*/ 	pDrawIPool->Store( *pDrawing );
/*N*/ 	*pDrawing << *pModel;
/*N*/ 
/*N*/ 	pModel->GetItemPool().SetFileFormatVersion( nOldFFVersion );
/*N*/ 
/*N*/ 
/*N*/ 	// Nun noch die Liste der frame::Frame-Z-IDs
/*N*/ 	sal_uInt16 nFrmIds = 0;
/*N*/ 	sal_uInt32 nIdPos = 0;
/*N*/ 	SdrPage* pPage = pModel->GetPage( 0 );
/*N*/ 	for ( sal_uInt32 i = 0; i < pPage->GetObjCount(); i++ )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = pPage->GetObj( i );
/*N*/ 		// fix #45256#: Auch die Indizes von Master-Objekten muessen
/*N*/ 		// exportiert werden, weil sonst die Indizes der anderen Objekte
/*N*/ 		// beim Laden nicht mehr stimmen.
/*N*/ 		if ( pObj->IsWriterFlyFrame() || pObj->ISA(SwFlyDrawObj) )
/*N*/ 		{
/*N*/ 			if( !nFrmIds )
/*N*/ 			{
/*N*/ 				*pDrawing << (sal_uInt16) SIGN_FRMIDS;
/*N*/ 				nIdPos = pDrawing->Tell();
/*N*/ 				*pDrawing << (sal_uInt16) nFrmIds;
/*N*/ 			}
/*N*/ 			*pDrawing << (sal_uInt32) i;
/*N*/ 			nFrmIds++;
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef DBG_UTIL
/*N*/ 	{
/*N*/ 		size_t nArrSz = pPage->GetObjCount() / 8;
/*N*/ 		if( (pPage->GetObjCount() % 8) > 0 )
/*N*/ 			nArrSz++;
/*N*/ 		pRefSdrObjects = new sal_uInt8[nArrSz];
/*N*/ 		while( nArrSz )
/*N*/ 			pRefSdrObjects[--nArrSz] = 0;
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if( nFrmIds )
/*N*/ 	{
/*N*/ 		sal_uInt32 nPos = pDrawing->Tell();
/*N*/ 		pDrawing->Seek( nIdPos );
/*N*/ 		*pDrawing << (sal_uInt16) nFrmIds;
/*N*/ 		pDrawing->Seek( nPos );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		*pDrawing << (sal_uInt16)0 << (sal_uInt16)0;
/*N*/ 	}
/*N*/ 	if( !IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		*pDrawing << 0; // was nHiddenDrawObjs!!!
/*N*/ 	}
/*N*/ 
/*N*/ 	ASSERT( !pModel->GetStyleSheetPool(), "SdrModel hat StyleSheet-Pool" );
/*N*/ 	pDrawing->Commit();
/*N*/ 	CheckIoError( pDrawing );
/*N*/ 	pDrawing->SetBufferSize( 0 );
/*N*/ 
/*N*/ 	pModel->PostSave();
/*N*/ }

/*************************************************************************
*
*		Seitenvorlagen
*
*************************************************************************/

/*N*/ void Sw3IoImp::LoadPageStyles()
/*N*/ {
/*N*/ 	pPageStyles->Seek( 0L );
/*N*/ 	pPageStyles->SetBufferSize( SW3_BSR_PAGESTYLES );
/*N*/ 	SvStream* pOld = pStrm;
/*N*/ 	pStrm = pPageStyles;
/*N*/ 	if( !nRes )
/*N*/ 		InPageDescs();
/*N*/ 	pStrm = pOld;
/*N*/ 	CheckIoError( pPageStyles );
/*N*/ 	pPageStyles->SetBufferSize( 0 );
/*N*/ }

/*N*/ void Sw3IoImp::SavePageStyles( sal_Bool bUsed )
/*N*/ {
/*N*/ 	// Falls der Stream existiert, muessen die dazugehoerigen Grafiken
/*N*/ 	// geloescht werden.
/*N*/ 	SvStream* pOld = pStrm;
/*N*/ 	pStrm = pPageStyles;
/*N*/ 	pPageStyles->SetSize( 0L );
/*N*/ 	// Stream weg, wenn wir kein OLE-Objekt sind
/*N*/ 	//if( pRoot->IsRoot() )
/*N*/ 	//	pRoot->Commit();
/*N*/ 	pPageStyles->SetBufferSize( SW3_BSW_PAGESTYLES );
/*N*/ 	OutPageDescs( bUsed );
/*N*/ 
/*N*/ 	sal_uInt32 nRecSzPos = 0;
/*N*/ 	if( !nRes && HasRecSizes() && !IsSw31Or40Export() )
/*?*/       nRecSzPos = OutRecSizes();
/*N*/ 
/*N*/ 	OpenRec( SWG_EOF );
/*N*/ 	CloseRec( SWG_EOF );
/*N*/ 
/*N*/ 	if( nRecSzPos )
/*?*/       OutRecordSizesPos( nRecSzPos );
/*N*/ 
/*N*/ 	pStrm = pOld;
/*N*/ 	pPageStyles->Commit();
/*N*/ 	CheckIoError( pPageStyles );
/*N*/ 	pPageStyles->SetBufferSize( 0 );
/*N*/ }

/*************************************************************************
*
*		Numerierungs-Regeln
*
*************************************************************************/

/*N*/ void Sw3IoImp::LoadNumRules()
/*N*/ {
/*N*/ 	if( !pNumRules.Is() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	pNumRules->Seek( 0L );
/*N*/ 	pNumRules->SetBufferSize( SW3_BSR_NUMRULES );
/*N*/ 	SvStream* pOld = pStrm;
/*N*/ 	pStrm = pNumRules;
/*N*/ 	if( !nRes )
/*N*/ 		InNumRules();
/*N*/ 	pStrm = pOld;
/*N*/ 	CheckIoError( pNumRules );
/*N*/ 	pNumRules->SetBufferSize( 0 );
/*N*/ }

/*N*/ void Sw3IoImp::SaveNumRules( sal_Bool bUsed )
/*N*/ {
/*N*/ 	if( !pNumRules.Is() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SvStream* pOld = pStrm;
/*N*/ 	pStrm = pNumRules;
/*N*/ 	pNumRules->SetSize( 0L );
/*N*/ 	pNumRules->SetBufferSize( SW3_BSW_NUMRULES );
/*N*/ 	OutNumRules( bUsed );
/*N*/ 
/*N*/ 	sal_uInt32 nRecSzPos = 0;
/*N*/ 	if( !nRes && HasRecSizes() && !IsSw31Or40Export() )
/*?*/       nRecSzPos = OutRecSizes();
/*N*/ 
/*N*/ 	OpenRec( SWG_EOF );
/*N*/ 	CloseRec( SWG_EOF );
/*N*/ 
/*N*/ 	if( nRecSzPos )
/*?*/       OutRecordSizesPos( nRecSzPos );
/*N*/ 
/*N*/ 	pStrm = pOld;
/*N*/ 	pNumRules->Commit();
/*N*/ 	CheckIoError( pNumRules );
/*N*/ 	pNumRules->SetBufferSize( 0 );
/*N*/ }

/*************************************************************************
*
*		Inhaltlicher Teil
*
*************************************************************************/

/*N*/ void Sw3IoImp::LoadContents( SwPaM* pPaM )
/*N*/ {
/*N*/ 	pContents->Seek( 0L );
/*N*/ 	pContents->SetBufferSize( SW3_BSR_CONTENTS );
/*N*/ 	pStrm = pContents;
/*N*/ 	LoadDocContents( pPaM );
/*N*/ 	pStrm = NULL;
/*N*/ 	CheckIoError( pContents );
/*N*/ 	pContents->SetBufferSize( 0 );
/*N*/ }

/*N*/ void Sw3IoImp::SaveContents( SwPaM& rPaM, const String* pBlkName )
/*N*/ {
/*N*/ 	pStrm = pContents;
/*N*/ 	pContents->SetSize( 0L );
/*N*/ 	// Stream weg, wenn wir kein OLE-Objekt sind
/*N*/ 	//if( pRoot->IsRoot() )
/*N*/ 	//	pRoot->Commit();
/*N*/ 	pContents->SetBufferSize( SW3_BSW_CONTENTS );
/*N*/ 	SaveDocContents( rPaM, pBlkName );
/*N*/ 	pStrm = NULL;
/*N*/ 	pContents->Commit();
/*N*/ 	CheckIoError( pContents );
/*N*/ 	pContents->SetBufferSize( 0 );
/*N*/ }

/*************************************************************************
*
*		String-Pool
*
*************************************************************************/

#define EXT_SEPARATOR ((sal_Unicode) 0x0A)

/*N*/ Sw3StringPool::Sw3StringPool()
/*N*/ {
/*N*/ 	bFixed = sal_False;
/*N*/ }

const int RES_POOLCOLL_HTML_LISTING_40 = 0x3002;
const int RES_POOLCOLL_HTML_XMP_40 = 0x3003;
const int RES_POOLCOLL_HTML_HR_40 = 0x3004;
const int RES_POOLCOLL_HTML_H6_40 = 0x3005;
const int RES_POOLCOLL_HTML_DD_40 = 0x3006;
const int RES_POOLCOLL_HTML_DT_40 = 0x3007;

/*N*/ sal_uInt16 Sw3StringPool::ConvertFromOldPoolId( sal_uInt16 nId, sal_uInt16 nVersion )
/*N*/ {
/*N*/ 	if( nVersion < SWG_HTMLCOLLCHG )
/*N*/ 	{
/*N*/ 		// - H6 wurde entfernt und muss durch Ueberschrift 6 ersetzt werden.
/*N*/ 		// - LISTING unx XMP wurden ebenfalls entfernt. Fuer sie muessen
/*N*/ 		//	 Benutzer-Vorlagen angelegt werden
/*N*/ 		// - HR, DT und DD sind nach vorne gerueckt.
/*N*/ 		switch( nId )
/*N*/ 		{
/*N*/ 		case RES_POOLCOLL_HTML_LISTING_40:
/*?*/ 		case RES_POOLCOLL_HTML_XMP_40:
/*?*/ 			nId |= USER_FMT;
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_POOLCOLL_HTML_HR_40:	nId = RES_POOLCOLL_HTML_HR; break;
/*?*/ 		case RES_POOLCOLL_HTML_H6_40:	nId = RES_POOLCOLL_HEADLINE6; break;
/*?*/ 		case RES_POOLCOLL_HTML_DD_40:	nId = RES_POOLCOLL_HTML_DD; break;
/*?*/ 		case RES_POOLCOLL_HTML_DT_40:	nId = RES_POOLCOLL_HTML_DT; break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nId;
/*N*/ }

/*N*/ sal_uInt16 Sw3StringPool::ConvertToOldPoolId( sal_uInt16 nId, sal_uInt32 nFFVersion )
/*N*/ {
/*N*/ 	if( nFFVersion <= SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		// H6, LISTUNG undx XMP wurden entfernt.
/*N*/ 		// HR, DT und DD sind nach vorne gerueckt.
/*N*/ 		switch( nId )
/*N*/ 		{
/*?*/ 		case RES_POOLCOLL_HTML_HR:	nId = RES_POOLCOLL_HTML_HR_40; break;
/*?*/ 		case RES_POOLCOLL_HEADLINE6:nId = RES_POOLCOLL_HTML_H6_40; break;
/*?*/ 		case RES_POOLCOLL_HTML_DD:	nId = RES_POOLCOLL_HTML_DD_40; break;
/*?*/ 		case RES_POOLCOLL_HTML_DT:	nId = RES_POOLCOLL_HTML_DT_40; break;
/*N*/ 
/*N*/ 		//JP 05.05.98: im 3.1/4.0 war ein Bug im sw3style bei der Abfrage
/*N*/ 		//				ob die PoolID einer Zeichenvorlage eine Benutzer-
/*N*/ 		//				definierte ID ist. Alles zwischen
/*N*/ 		// 				RES_POOLCHR_NORMAL_END und RES_POOLCHR_HTML_BEGIN
/*N*/ 		//				werden als gueltig erkannt, was falsch ist.
/*N*/ 		default:
/*N*/ 			if( ( RES_POOLCHR_TOXJUMP < nId && nId < RES_POOLCHR_NORMAL_END )
/*N*/ 			  ||( RES_POOLFRM_WATERSIGN < nId && nId < RES_POOLFRM_END )
/*N*/ 			  ||( RES_POOLPAGE_HTML < nId && nId < RES_POOLPAGE_END ) )
/*N*/ 			  nId = USHRT_MAX;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nId;
/*N*/ }


// Sammeln der Strings am Doc. Es werden alle Strings aus benannten
// Formaten, Collections und PageDescs gesammelt. Weiter wird allen
// Auto-Frameformaten eine Extension im Namen verpaát, weil die Namen
// nicht eindeutig sind.

/*N*/ void Sw3StringPool::Setup( SwDoc& rDoc, long nFFVersion,
/*N*/ 						   Sw3ExportInfo *pExportInfo )
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	nExpFFVersion = nFFVersion;
/*N*/ 
/*N*/ 	// Das Written-Flag wird gleich mit geknackt!
/*N*/ 	// Das ist wichtig, da Formate (theoretisch) in verschiedenen
/*N*/ 	// Streams landen koennen.
/*N*/ 	SwMsgPoolItem aMsgHint( RES_RESET_FMTWRITTEN );
/*N*/ 	rDoc.GetDfltFrmFmt()->Modify( &aMsgHint, &aMsgHint );
/*N*/ 
/*N*/ 	bFixed = sal_False;
/*N*/ 	// Defaultformate:
/*N*/ 	Setup( rDoc, *rDoc.GetDfltFrmFmt() );
/*N*/ 	Setup( rDoc, *rDoc.GetDfltCharFmt() );
/*N*/ 	// Text-Collections
/*N*/ 	sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->Count();
        sal_uInt16 n=0;
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 		Setup( rDoc, *(*rDoc.GetTxtFmtColls())[ n ] );
/*N*/ 	// benannte Zeichen- und Rahmenvorlagen
/*N*/ 	nArrLen = rDoc.GetCharFmts()->Count();
/*N*/ 	// Element 0 ist das Default-Format!
/*N*/ 	for( n = 1; n < nArrLen; n++ )
/*N*/ 	Setup( rDoc, *(*rDoc.GetCharFmts())[ n ] );
/*N*/ 	nArrLen = rDoc.GetFrmFmts()->Count();
/*N*/ 	// Element 0 ist das Default-Format!
/*N*/ 	for( n = 1; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		const SwFmt& rFmt = *(*rDoc.GetFrmFmts())[ n ];
/*N*/ 		// Autoformate in dieser Liste muessen mit einer
/*N*/ 		// Extension versehen werden!
/*N*/ 		sal_uInt16 nFmtId =  0;
/*N*/ 		const String& rName = rFmt.GetName();
/*N*/ 		// TODO: unicode: correct?
/*N*/ 		if( rFmt.IsAuto() ) 		// Autoformat
/*N*/ 			nFmtId = Count()+1; //++nId;
/*N*/ 		Setup( rDoc, rFmt, nFmtId );
/*N*/ 	}
/*N*/ 	nArrLen = rDoc.GetSpzFrmFmts()->Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 		Setup( rDoc, *(*rDoc.GetSpzFrmFmts())[ n ], Count()+1 /*++nId*/ );
/*N*/ 	// Auch hier muss eine Extension mit ran
/*N*/ 	nArrLen = rDoc.GetTblFrmFmts()->Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 		Setup( rDoc, *(*rDoc.GetTblFrmFmts())[ n ], Count()+1 /*++nId*/ );
/*N*/ 	nArrLen = rDoc.GetSections().Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 		Setup( rDoc, *rDoc.GetSections()[ n ], Count()+1 /*++nId*/ );
/*N*/ 	// Seitenvorlagen
/*N*/ 	nArrLen = rDoc.GetPageDescCnt();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		const SwPageDesc& rPg = rDoc.GetPageDesc( n );
/*N*/ 		Add( rPg.GetName(), rPg.GetPoolFmtId() );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nExpFFVersion <= SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		Add( String::CreateFromAscii(sSW3IO_FixedField), USHRT_MAX );
/*N*/ 		Add( String::CreateFromAscii(sSW3IO_AuthorityField), USHRT_MAX );
/*N*/ 	}
/*N*/     /* #108791# */
/*N*/     Add( String::CreateFromAscii(sSW3IO_DropDownField), USHRT_MAX );
/*N*/ 
/*N*/ 	// fix #26064#: Namen der 3 Label Numernkreise: Hier wird zusaetzlich
/*N*/ 	// noch die Pool-Id des dazugehoerigen Formats gespeichert
/*N*/ 	{
/*N*/ 		String aAbb( SW_RES(STR_POOLCOLL_LABEL_ABB) );
/*N*/ 		String aTable( SW_RES(STR_POOLCOLL_LABEL_TABLE) );
/*N*/ 		String aFrame( SW_RES(STR_POOLCOLL_LABEL_FRAME) );
/*N*/ 		String aDrawing( SW_RES(STR_POOLCOLL_LABEL_DRAWING) );
/*N*/ 
/*N*/ 		for( n = 0; n < INIT_FLDTYPES; n++ )
/*N*/ 		{
/*N*/ 			SwFieldType *pFldType = (*rDoc.GetFldTypes())[n];
/*N*/ 			switch (pFldType->Which())
/*N*/ 			{
/*N*/ 				case RES_SETEXPFLD:
/*N*/ 				{
/*N*/ 					if( GSE_SEQ & ((const SwSetExpFieldType *)pFldType)->GetType() )
/*N*/ 					{
/*N*/ 						sal_uInt16 nPoolId = USHRT_MAX;
/*N*/ 						const String& rName = pFldType->GetName();
/*N*/ 						if( aAbb == rName )
/*N*/ 							nPoolId = RES_POOLCOLL_LABEL_ABB;
/*N*/ 						else if( aTable == rName )
/*N*/ 							nPoolId = RES_POOLCOLL_LABEL_TABLE;
/*N*/ 						else if( aFrame == rName )
/*N*/ 							nPoolId = RES_POOLCOLL_LABEL_FRAME;
/*N*/ 						else if( aDrawing == rName )
/*N*/ 							nPoolId = RES_POOLCOLL_LABEL_DRAWING;
/*N*/ 
/*N*/ 						Add( rName, nPoolId );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 				case RES_DBSETNUMBERFLD:
/*N*/ 				case RES_DBNUMSETFLD:
/*N*/ 				case RES_DBNEXTSETFLD:
/*N*/ 				case RES_DBNAMEFLD:
/*N*/ 				if( nExpFFVersion > SOFFICE_FILEFORMAT_31 )
/*N*/ 				{
/*N*/ 					SwClientIter aIter( *pFldType );
/*N*/ 
/*N*/ 					for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
/*N*/ 						pFmtFld; pFmtFld = (SwFmtFld*)aIter.Next() )
/*N*/ 					{
/*N*/ 						if( pFmtFld->GetTxtFld() )
/*N*/ 						{
/*N*/ 							SwDBNameInfField* pFld = (SwDBNameInfField *)pFmtFld->GetFld();
/*N*/ 							SwDBData aData(pFld->GetRealDBData());
/*N*/ 							String sDBName(aData.sDataSource);
/*N*/ 							sDBName += DB_DELIM;
/*N*/ 							sDBName += (String)aData.sCommand;
/*N*/ 							Add( sDBName, IDX_NOCONV_FF );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Namen benutzerdefinierter Felder
/*N*/ 	nArrLen = rDoc.GetFldTypes()->Count();
/*N*/ 	if( nArrLen > INIT_FLDTYPES )
/*N*/ 	{
/*N*/ 		for( n = INIT_FLDTYPES; n < nArrLen; n++ )
/*N*/ 		{
/*N*/ 			SwFieldType *pFldType = (*rDoc.GetFldTypes())[n];
/*N*/ 
/*N*/ 			switch (pFldType->Which())
/*N*/ 			{
/*N*/ 				case RES_DBFLD:
/*N*/ 					Add( ((SwDBFieldType*)pFldType)->GetColumnName(), USHRT_MAX );
/*N*/ 					if( nExpFFVersion > SOFFICE_FILEFORMAT_31 )
/*N*/ 					{
/*N*/ 						SwDBData aData(((SwDBFieldType*)pFldType)->GetDBData());
/*N*/ 						String sDBName(aData.sDataSource);
/*N*/ 						sDBName += DB_DELIM;
/*N*/ 						sDBName += (String)aData.sCommand;
/*N*/ 						Add( sDBName, IDX_NOCONV_FF );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 
/*N*/ 				default:
/*N*/ 					Add( pFldType->GetName(), USHRT_MAX );
/*N*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ab dem 5.0-FF stehen auch die Benutzer-definierten Verzeichnisnamen
/*N*/ 	// im String-Pool
/*N*/ 	if( nExpFFVersion > SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		const SwSectionFmts& rSectFmts = rDoc.GetSections();
/*N*/ 		for( sal_uInt16 n = rSectFmts.Count(); n; )
/*N*/ 		{
/*N*/ 			const SwTOXType* pType;
/*N*/ 			const SwTOXBaseSection *pTOXBaseSect;
/*N*/ 			const SwSectionFmt* pSectFmt = rSectFmts[ --n ];
/*N*/ 			const SwSection* pSect = pSectFmt->GetSection();
/*N*/ 			if( pSect && TOX_CONTENT_SECTION == pSect->GetType() &&
/*N*/ 				pSect->GetFmt() && pSectFmt->GetSectionNode() &&
/*N*/ 				0 != (pTOXBaseSect = PTR_CAST( SwTOXBaseSection, pSect )) &&
/*N*/ 				0 != (pType = pTOXBaseSect->GetTOXType()) )
/*N*/ 			{
/*N*/ 				// All types above TOX_USER are new in version 5.2. That for,
/*N*/ 				// their names must be added to the string pool.
/*N*/ 				if( pType->GetType() >= TOX_ILLUSTRATIONS ||
/*N*/ 					pType->GetTypeName() !=
/*N*/ 								SwTOXBase::GetTOXName(pType->GetType()) )
/*N*/ 				{
/*?*/ 					Add( pType->GetTypeName(), USHRT_MAX );
/*N*/ 				}
/*N*/ 				// Add template names
/*N*/ 				const SwForm& rForm = pTOXBaseSect->GetTOXForm();
                    sal_uInt16 i=0;
/*N*/ 				for( i = 0; i < rForm.GetFormMax(); i++ )
/*N*/ 				{
/*N*/ 					const String& rCollNm = rForm.GetTemplate( i );
/*N*/ 					if( rCollNm.Len() )
/*N*/ 						SetupTxtCollByName( rDoc, rCollNm );
/*N*/ 				}
/*N*/ 				// Add text style names used in patterns
/*N*/ 				for( i = 0; i < rForm.GetFormMax(); i++ )
/*N*/ 				{
/*N*/ 					const String& rPattern = rForm.GetPattern(i);
/*N*/ 					if( rPattern.Len() )
/*N*/ 					{
/*N*/ 						SwFormTokenEnumerator aEnum( rPattern );
/*N*/ 						while( aEnum.HasNextToken() )
/*N*/ 						{
/*N*/ 							SwFormToken aToken( aEnum.GetNextToken() );
/*N*/ 							if( aToken.sCharStyleName.Len() )
/*?*/ 								Add( aToken.sCharStyleName, aToken.nPoolId );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				// Add style names
/*N*/ 				for( i=0; i<MAXLEVEL; i++ )
/*N*/ 				{
/*N*/ 					const String& rStyleNames = pTOXBaseSect->GetStyleNames(i);
/*N*/ 					if( rStyleNames.Len() )
/*N*/ 					{
/*?*/ 						xub_StrLen nStrPos = 0;
/*?*/ 						while( nStrPos != STRING_NOTFOUND )
/*?*/ 						{
/*?*/ 							String aName = rStyleNames.GetToken( 0,
/*?*/ 												TOX_STYLE_DELIMITER, nStrPos );
/*?*/ 							ASSERT( aName.Len(), "empty style name" );
/*?*/ 							if( aName.Len() )
/*?*/ 								SetupTxtCollByName( rDoc, aName );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				// Add text coll names used if new indices are converted to
/*N*/ 				// user indices
/*N*/ 				sal_uInt16 nPoolId = USHRT_MAX;
/*N*/ 				switch( pType->GetType() )
/*N*/ 				{
/*?*/ 				case TOX_ILLUSTRATIONS:
/*?*/ 					nPoolId = RES_POOLCOLL_LABEL_ABB;
/*?*/ 					break;
/*?*/ 				case TOX_OBJECTS:
/*?*/ 					nPoolId = RES_POOLCOLL_LABEL_FRAME;
/*?*/ 					break;
/*?*/ 				case TOX_TABLES:
/*?*/ 					nPoolId = RES_POOLCOLL_LABEL_TABLE;
/*?*/ 					break;
/*N*/ 				}
/*N*/ 				if( USHRT_MAX != nPoolId )
/*N*/ 				{
/*?*/ 					String aName;
/*?*/ 					SwStyleNameMapper::FillUIName( nPoolId, aName );
/*?*/ 					Add( aName, nPoolId );
/*N*/ 				}
/*N*/ 				const String& rMainStyle =
/*N*/ 					pTOXBaseSect->GetMainEntryCharStyle();
/*N*/ 				if( rMainStyle.Len() )
/*N*/ 				{
/*?*/ 					sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rMainStyle,
/*?*/ 													 GET_POOLID_CHRFMT );
/*N*/ 					Add( rMainStyle, nPoolId );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwTOXMark* pItem;
/*N*/ 	nArrLen = nExpFFVersion > SOFFICE_FILEFORMAT_40
/*N*/ 			? rDoc.GetAttrPool().GetItemCount( RES_TXTATR_TOXMARK )
/*N*/ 			: 0;
/*N*/ 	for( n = 0; n < nArrLen; ++n )
/*N*/ 	{
/*N*/ 		pItem = (const SwTOXMark*)rDoc.GetAttrPool().GetItem( RES_TXTATR_TOXMARK, n );
/*N*/ 		if( pItem )
/*N*/ 		{
/*N*/ 			const SwTOXType* pType = pItem->GetTOXType();
/*N*/ 			if( pType && pType->GetTypeName() !=
/*N*/ 								SwTOXBase::GetTOXName(pType->GetType()) )
/*N*/ 			{
/*?*/ 				Add( pType->GetTypeName(), USHRT_MAX );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwFmtINetFmt* pINetFmt;
/*N*/ 	nArrLen = nExpFFVersion > SOFFICE_FILEFORMAT_31
/*N*/ 			? rDoc.GetAttrPool().GetItemCount( RES_TXTATR_INETFMT )
/*N*/ 			: 0;
/*N*/ 	for( n = 0; n < nArrLen; ++n )
/*N*/ 	{
/*N*/ 		pINetFmt = (const SwFmtINetFmt*)rDoc.GetAttrPool().GetItem( RES_TXTATR_INETFMT, n );
/*N*/ 		if( pINetFmt )
/*N*/ 		{
/*N*/ 			if( pINetFmt->GetINetFmt().Len() )
/*?*/ 				Add( pINetFmt->GetINetFmt(), pINetFmt->GetINetFmtId() );
/*N*/ 			if( pINetFmt->GetVisitedFmt().Len() )
/*?*/ 				Add( pINetFmt->GetVisitedFmt(), pINetFmt->GetVisitedFmtId() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Seit der 5.0 gibt es Redlines. Die Autoren stehen dann im String-Pool
/*N*/ 	if( nExpFFVersion > SOFFICE_FILEFORMAT_40 )
/*N*/ 		SetupRedlines( rDoc );
/*N*/ 
/*N*/ 	// Beim 3.1/4.0-Export standen die Tabellen-Line/Box-Formate noch
/*N*/ 	// im FrmFmt-Array und wurden ueber den String-Pool dort eingetragen.
/*N*/ 	// Deshalb muessen sie jetzt auch dort eingetragen werden. Die
/*N*/ 	// Umbennenerei erfolgt allerdings nicht mehr im Format. Stattdessen
/*N*/ 	// wird der (erweiterte) Name in einem Array gemerkt und ueber dieses
/*N*/ 	// ermittelt.
/*N*/ 	if( nExpFFVersion <= SOFFICE_FILEFORMAT_40 && pExportInfo &&
/*N*/ 		pExportInfo->pTblLineBoxFmts40 )
/*N*/ 	{
/*N*/ 		if( !pExportInfo->pTblLineBoxFmtNames40 )
/*N*/ 			pExportInfo->pTblLineBoxFmtNames40 = new SvStringsDtor;
/*N*/ 
/*N*/ 		SvStringsDtor *pNames = pExportInfo->pTblLineBoxFmtNames40;
/*N*/ 		if( pNames->Count() )
/*?*/ 			pNames->DeleteAndDestroy( 0, pNames->Count() );
/*N*/ 
/*N*/ 		Sw3FrmFmts *pFmts = pExportInfo->pTblLineBoxFmts40;
/*N*/ 		sal_uInt16 nArrLen = pFmts->Count();
/*N*/ 		for( n=0; n<nArrLen; n++ )
/*N*/ 		{
/*N*/ 			// TODO: unicode: currect?
/*N*/ 			String * pName = new String( (*pFmts)[n]->GetName() );
/*N*/ 			(*pName += EXT_SEPARATOR) += String::CreateFromInt32( Count()+1 );
/*N*/ 			Add( *pName, (*pFmts)[n]->GetPoolFmtId() );
/*N*/ 			pNames->Insert( pName, pNames->Count() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Und danach darf nix mehr in den Pool hinein!
/*N*/ 	bFixed = sal_True;
/*N*/ }

/*N*/ void Sw3StringPool::SetupForNumRules( SwDoc& rDoc, long nFFVersion )
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	nExpFFVersion = nFFVersion;
/*N*/ 	bFixed = sal_False;
/*N*/ 
/*N*/ 	// Die Namen von Zeichenvorlagen werden benoetigt, weil in den
/*N*/ 	// Numerierungen auuf sie verwiesen werden kann.
/*N*/ 	sal_uInt16 nArrLen = rDoc.GetCharFmts()->Count();
/*N*/ 	// Element 0 ist das Default-Format!
        sal_uInt16 n;
/*N*/ 	for( n = 1; n < nArrLen; n++ )
/*N*/ 		Setup( rDoc, *(*rDoc.GetCharFmts())[ n ] );
/*N*/ 
/*N*/ 	// Die Namen der Numerierungen werden sowieso benoetigt.
/*N*/ 	nArrLen = rDoc.GetNumRuleTbl().Count();
/*N*/ 	for( n=0; n<nArrLen; n++ )
/*N*/ 	{
/*N*/ 		const SwNumRule *pNumRule = rDoc.GetNumRuleTbl()[n];
/*N*/ 		Add( pNumRule->GetName(), pNumRule->GetPoolFmtId() );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Und die Outline-Numerierung hat auch einen Namen.
/*N*/ 	const SwNumRule *pOutline = rDoc.GetOutlineNumRule();
/*N*/ 	if( pOutline )
/*N*/ 		Add( pOutline->GetName(), pOutline->GetPoolFmtId() );
/*N*/ 
/*N*/ 	// Und danach darf nix mehr in den Pool hinein!
/*N*/ 	bFixed = sal_True;
/*N*/ }

// Entfernen der Autoformat-Extension nach dem Laden/Speichern
// Dies ist ein Hash und eine Zahl, mit der Autoformate im
// FrmFmt-Array des Docs erweitert wurden.

/*N*/ void Sw3StringPool::RemoveExtensions( SwDoc& rDoc )
/*N*/ {
/*N*/ 	sal_uInt16 nArrLen = rDoc.GetFrmFmts()->Count();
/*N*/ 	// Element 0 ist das Default-Format!
        sal_uInt16 n;
/*N*/ 	for( n = 1; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		SwFmt& rFmt = (SwFmt&) *(*rDoc.GetFrmFmts())[ n ];
/*N*/ 		RemoveExtension( rFmt );
/*N*/ 	}
/*N*/ 	nArrLen = rDoc.GetSpzFrmFmts()->Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		SwFmt& rFmt = (SwFmt&) *(*rDoc.GetSpzFrmFmts())[ n ];
/*N*/ 		RemoveExtension( rFmt );
/*N*/ 	}
/*N*/ 	nArrLen = rDoc.GetTblFrmFmts()->Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		SwFmt& rFmt = (SwFmt&) *(*rDoc.GetTblFrmFmts())[ n ];
/*N*/ 		RemoveExtension( rFmt );
/*N*/ 	}
/*N*/ 	nArrLen = rDoc.GetSections().Count();
/*N*/ 	for( n = 0; n < nArrLen; n++ )
/*N*/ 	{
/*N*/ 		SwFmt& rFmt = (SwFmt&) *rDoc.GetSections()[ n ];
/*N*/ 		RemoveExtension( rFmt );
/*N*/ 	}
/*N*/ }

// Entfernen der Autoformat-Extension nach dem Laden/Speichern
// Dies ist ein Hash und eine Zahl, mit der Autoformate im
// FrmFmt-Array des Docs erweitert wurden.

/*N*/ void Sw3StringPool::RemoveExtension( SwFmt& rFmt )
/*N*/ {
/*N*/ 	String aName( rFmt.GetName() );
/*N*/ 	RemoveExtension( aName );
/*N*/ 	// Kein SetName() wg. Assert
/*N*/ 	(String&)rFmt.GetName() = aName;
/*N*/ }

/*N*/ void Sw3StringPool::RemoveExtension( String& rName )
/*N*/ {
/*N*/ 	xub_StrLen nOff = rName.Search( EXT_SEPARATOR );
/*N*/ 	if( nOff != STRING_NOTFOUND )
/*N*/ 		rName.Erase( nOff );
/*N*/ }

// Eintragen der Strings eines Formats
// Falls nId angegeben ist, wird diese Zahl mit einem Hash an den Namen
// angefuegt, um eine Eindeutigkeit zu gewaehrleisten.

/*N*/ void Sw3StringPool::Setup( SwDoc& rDoc, const SwFmt& rFmt, sal_uInt16 nId )
/*N*/ {
/*N*/ 	// Das Written-Flag wird gleich mit geknackt!
/*N*/ 	// Das ist wichtig, da Formate (theoretisch) in verschiedenen
/*N*/ 	// Streams landen koennen.
/*N*/ 	ASSERT( !rFmt.IsWritten(), "Written-Flag am Format ist gesetzt" );
/*N*/ //	((SwFmt&)rFmt).ResetWritten();
/*N*/ 
/*N*/ 	String aName( rFmt.GetName() );
/*N*/ 	if( nId )
/*N*/ 	{
/*N*/ 		ASSERT( nId==Count()+1, "id ungleich String-Pool-Pos" );
/*N*/ 		aName += EXT_SEPARATOR, aName += String::CreateFromInt32(nId);
/*N*/ 		// Kein SetName() wg. Assert
/*N*/ 		(String&)((SwFmt&)rFmt).GetName() = aName;
/*N*/ 	}
/*N*/ 	Add( aName, rFmt.GetPoolFmtId(), nId!=0 );
/*N*/ 	if( rFmt.GetPoolHlpFileId() != UCHAR_MAX )
/*N*/ 	{
/*?*/ 		String* pId = rDoc.GetDocPattern( rFmt.GetPoolHlpFileId() );
/*?*/ 		if( pId )
/*?*/ 			Add( *pId, 0 );
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3StringPool::SetupTxtCollByName( SwDoc& rDoc, const String& rName )
/*N*/ {
/*N*/ 	if( !rName.Len() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SwTxtFmtColl *pColl = rDoc.FindTxtFmtCollByName( rName );
/*N*/ 	sal_uInt16 nPoolId = pColl ? pColl->GetPoolFmtId()
/*N*/ 						   : SwStyleNameMapper::GetPoolIdFromUIName( rName, GET_POOLID_TXTCOLL );
/*N*/ 	Add( rName, nPoolId );
/*N*/ }

/*N*/ sal_uInt16 Sw3StringPool::Add( const String& r, sal_uInt16 n, sal_Bool bDontSearch )
/*N*/ {
/*N*/ 	sal_uInt16 i = bDontSearch ? aPool.Count() : 0;
/*N*/ 
/*N*/ 	ASSERT( nExpFFVersion, "String-Pool: FF-Version nicht gesetzt" );
/*N*/ 	if( nExpFFVersion <= SOFFICE_FILEFORMAT_40 && n && n<IDX_SPEC_VALUE )
/*N*/ 		n = ConvertToOldPoolId( n, nExpFFVersion );
/*N*/ 
/*N*/ 	for( ; i < aPool.Count(); i++ )
/*N*/ 	{
/*N*/ 		const Sw3String* p3Str = aPool.GetObject( i );
/*N*/ 		if( n == p3Str->GetPoolId() && r == *p3Str )
/*N*/ 			return i;
/*N*/ 	}
/*N*/ 	if( !bFixed )
/*N*/ 	{
/*N*/ 		Sw3String* p = new Sw3String( r, n );
/*N*/ 		aPool.Insert( p, i );
/*N*/ 		return i;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ASSERT( !bFixed, "String nicht im Pool eingetragen" );
/*?*/ 		return IDX_NO_VALUE;
/*N*/ 	}
/*N*/ }


/*N*/ sal_uInt16 Sw3StringPool::Find( const String& r, sal_uInt16 nPoolId )
/*N*/ {
/*N*/ 	ASSERT( nExpFFVersion, "String-Pool: FF-Version nicht gesetzt" );
/*N*/ 	if( nExpFFVersion  <= SOFFICE_FILEFORMAT_40 && nPoolId && nPoolId<IDX_SPEC_VALUE )
/*N*/ 		nPoolId = ConvertToOldPoolId( nPoolId, nExpFFVersion );
/*N*/ 
/*N*/ 	xub_StrLen nOff = r.Search( EXT_SEPARATOR );
/*N*/ 	if( nOff != STRING_NOTFOUND )
/*N*/ 	{
/*N*/ 		xub_StrLen nPos = (xub_StrLen)r.Copy(nOff+1).ToInt32() - 1;
/*N*/ 		if( nPos<aPool.Count() )
/*N*/ 		{
/*N*/ 			const Sw3String* p3Str = aPool.GetObject( nPos );
/*N*/ 			if( nPoolId == p3Str->GetPoolId() && r==*p3Str )
/*N*/ 				return nPos;
/*N*/ 		}
/*N*/ 		ASSERT( !this, "String-Pool-Position ungueltig" );
/*N*/ 	}
/*N*/ 
/*N*/ 	xub_StrLen nLen = r.Len();
/*N*/ 	for( sal_uInt16 i = 0; i < aPool.Count(); i++ )
/*N*/ 	{
/*N*/ 		const Sw3String* p3Str = aPool.GetObject( i );
/*N*/ 		if( nPoolId == p3Str->GetPoolId() && nLen==p3Str->Len() )
/*N*/ 		{
/*N*/ 			xub_StrLen n = nLen;
/*N*/ 			sal_Bool bFound = sal_True;
/*N*/ 			while( bFound && n )
/*N*/ 			{
/*N*/ 				n--;
/*N*/ 				if( r.GetChar(n) != p3Str->GetChar(n) )
/*N*/ 				{
/*N*/ 					bFound = sal_False;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( bFound )
/*N*/ 				return i;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return IDX_NO_VALUE;
/*N*/ }

/*N*/ const String& Sw3StringPool::Find( sal_uInt16 i )
/*N*/ {
/*N*/ 	if( i < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		if( i < aPool.Count() )
/*N*/ 			return *aPool.GetObject( i );
/*N*/ 		ASSERT( !this, "String-Index nicht im Stringpool" );
/*N*/ 	}
/*N*/ 	return aEmpty;
/*N*/ }

/*N*/ sal_uInt16 Sw3StringPool::FindPoolId( sal_uInt16 i )
/*N*/ {
/*N*/ 	if( i < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		if( i < aPool.Count() )
/*N*/ 			return aPool.GetObject( i )->GetPoolId();
/*N*/ 		ASSERT( !this, "String-Index nicht im Stringpool" );
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

// OPT: Cache fuer Formate im StringPool

/*N*/ void Sw3StringPool::SetCachedFmt( sal_uInt16 i, SwFmt *pFmt )
/*N*/ {
/*N*/ 	ASSERT( i < IDX_SPEC_VALUE && i < aPool.Count(),
/*N*/ 			"SetCachedFmt fuer ungueltigen Index" );
/*N*/ 	if( i < IDX_SPEC_VALUE && i < aPool.Count() )
/*N*/ 		aPool.GetObject( i )->SetCachedFmt( pFmt );
/*N*/ }



/*N*/ SwFmt *Sw3StringPool::FindCachedFmt( sal_uInt16 i ) const
/*N*/ {
/*N*/ 	if( i < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		if( i < aPool.Count() )
/*N*/ 			return aPool.GetObject( i )->GetCachedFmt();
/*N*/ 		ASSERT( !this, "String-Index nicht im Stringpool" );
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

// /OPT: Cache fuer Formate im StringPool


// Laderoutine fuer Dokumente bis 134g

void Sw3StringPool::LoadOld( SvStream& r )
{
    aPool.DeleteAndDestroy( 0, aPool.Count() );
    sal_uInt16 n;
    String s;
    r >> n;
    // Dieser schlimme Hack ist notwendig, da ich vergessen habe,
    // den Zeichensatz im Record zu speichern. Und kein Flag-Byte da!
    // Dieser Hack arbeitet gut, wenn das LSByte der Anzahl Strings != 0
    // ist, was bis heute (10.04.95) der Fall ist.
    rtl_TextEncoding eSrcEnc = r.GetStreamCharSet();
    if( n >= 256 )
    {
        sal_uInt8 cCharSet, cDummyFlags;
        r.SeekRel( -2L );
        r >> cDummyFlags >> cCharSet >> n;
        eSrcEnc = (rtl_TextEncoding)cCharSet;
    }
    while( n-- )
    {
        r.ReadByteString( s, eSrcEnc );
        Sw3String* p = new Sw3String( s, 0 );
        aPool.Insert( p, aPool.Count() );
    }
    bFixed = sal_True;
}

/*N*/ void Sw3StringPool::Load( SvStream& r, sal_uInt16 nSVersion )
/*N*/ {
/*N*/ 	aPool.DeleteAndDestroy( 0, aPool.Count() );
/*N*/ 	sal_uInt8 cSet;
/*N*/ 	sal_uInt16 n;
/*N*/ 	String s;
/*N*/ 	r >> cSet >> n;
/*N*/ 
/*N*/ 	// MIB 8.4.97: Datenbank-Felder enthalten ein 0xff, das auf keinen
/*N*/ 	// Fall mit konvertiert werden darf. Wir erkennen sie am Wert
/*N*/ 	// IDX_NOCONV_FF.
/*N*/  	rtl_TextEncoding eSrcEnc =  GetSOLoadTextEncoding( (rtl_TextEncoding)cSet,
/*N*/ 													   r.GetVersion() );
/*N*/ 	while( n-- )
/*N*/ 	{
/*N*/ 		sal_uInt16 nId;
/*N*/ 		ByteString s8;
/*N*/ 		r >> nId;
/*N*/ 		r.ReadByteString( s8 );
/*N*/ 		if( nId == IDX_NOCONV_FF )
/*N*/ 			s = Sw3IoImp::ConvertStringNoDbDelim( s8, eSrcEnc );
/*N*/ 		else
/*N*/ 			s = String( s8, eSrcEnc );
/*N*/ 
/*N*/ 		// Ggf. alte Pool-Ids auf neue mappen.
/*N*/ 		if( nSVersion < SWG_HTMLCOLLCHG && nId && nId < IDX_SPEC_VALUE )
/*N*/ 		{
/*N*/ 			// Die ehemaligen Listing und XMP-Vorlagen werden umbenannt,
/*N*/ 			// damit sie beim Export wiedergefunden werden.
/*N*/ 			switch( nId )
/*N*/ 			{
/*?*/ 			case RES_POOLCOLL_HTML_LISTING_40:
/*?*/ 				s.AssignAscii( sHTML_listing );
/*?*/ 				break;
/*?*/ 			case RES_POOLCOLL_HTML_XMP_40:
/*?*/ 				s.AssignAscii( sHTML_xmp );
/*?*/ 				break;
/*N*/ 			}
/*N*/ 			nId = ConvertFromOldPoolId( nId, nSVersion );
/*N*/ 		}
/*N*/ 
/*N*/ 		// Id und ist PoolFormat ?
/*N*/ 		if( nId && !(USER_FMT & nId))
/*N*/ 			SwStyleNameMapper::FillUIName( nId, s );     // dann String vom Pool erfragen
/*N*/ 
/*N*/ 		Sw3String* p = new Sw3String( s, nId );
/*N*/ 		aPool.Insert( p, aPool.Count() );
/*N*/ 	}
/*N*/ 	bFixed = sal_True;
/*N*/ }

/*N*/ void Sw3StringPool::Store( SvStream& r )
/*N*/ {
/*N*/ 	ASSERT( nExpFFVersion==r.GetVersion(),
/*N*/ 			"Sw3StringPool::Store: FF-Version am Stream stimmt nicht" );
/*N*/ 	rtl_TextEncoding eSrcEnc = r.GetStreamCharSet();
/*N*/ 	sal_uInt16 n = aPool.Count();
/*N*/ 	r << (sal_uInt8) eSrcEnc
/*N*/ 	  << (sal_uInt16) n;
/*N*/ 	for( sal_uInt16 i = 0; i < n; i++ )
/*N*/ 	{
/*N*/ 		Sw3String* p = aPool.GetObject( i );
/*N*/ 		sal_uInt16 nId = (sal_uInt16) p->GetPoolId();
/*N*/ 		r << nId;
/*N*/ 		if( nId == IDX_NOCONV_FF )
/*N*/ 			r.WriteByteString( Sw3IoImp::ConvertStringNoDbDelim(
/*N*/ 										(const String&)*p, eSrcEnc ) );
/*N*/ 		else
/*N*/ 			r.WriteByteString( (const String&)*p, eSrcEnc );
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3StringPool::Clear()
/*N*/ {
/*N*/ 	aPool.DeleteAndDestroy( 0, aPool.Count() );
/*N*/ 	nExpFFVersion = 0;	// um ungesetzte Version zu erkennen!
/*N*/ }

/*N*/ ByteString Sw3IoImp::ConvertStringNoDbDelim( const String& rStr,
/*N*/ 											 rtl_TextEncoding eSource )
/*N*/ {
/*N*/ 	return ConvertStringNoDelim( rStr, DB_DELIM, '\xff', eSource );
/*N*/ }
/*N*/ 
/*N*/ String Sw3IoImp::ConvertStringNoDbDelim( const ByteString& rStr,
/*N*/ 										 rtl_TextEncoding eSource )
/*N*/ {
/*N*/ 	return ConvertStringNoDelim( rStr, '\xff', DB_DELIM, eSource );
/*N*/ }

/*N*/ ByteString Sw3IoImp::ConvertStringNoDelim( const String& rStr,
/*N*/ 										   sal_Unicode cSrcDelim,
/*N*/ 										   sal_Char cDelim,
/*N*/ 										   rtl_TextEncoding eSource )
/*N*/ {
/*N*/ 	ByteString sDest;
/*N*/ 	xub_StrLen nStart = 0;
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		nPos = rStr.Search( cSrcDelim, nStart );
/*N*/ 		if( STRING_NOTFOUND == nPos )
/*N*/ 			nPos = rStr.Len();
/*N*/ 
/*N*/ 		if( nStart > 0 )
/*N*/ 			sDest += cDelim;
/*N*/ 
/*N*/ 		if( nPos > nStart )
/*N*/ 			sDest += ByteString( rStr.Copy( nStart, nPos-nStart ), eSource );
/*N*/ 		nStart = nPos+1;
/*N*/ 	}
/*N*/ 	while( nPos < rStr.Len() );
/*N*/ 
/*N*/ 	return sDest;
/*N*/ }

/*N*/ String Sw3IoImp::ConvertStringNoDelim( const ByteString& rStr,
/*N*/ 									   sal_Char cSrcDelim,
/*N*/ 									   sal_Unicode cDelim,
/*N*/ 									   rtl_TextEncoding eSource )
/*N*/ {
/*N*/ 	String sDest;
/*N*/ 	xub_StrLen nStart = 0;
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		nPos = rStr.Search( cSrcDelim, nStart );
/*N*/ 		if( STRING_NOTFOUND == nPos )
/*N*/ 			nPos = rStr.Len();
/*N*/ 
/*N*/ 		if( nStart > 0 )
/*N*/ 			sDest += cDelim;
/*N*/ 
/*N*/ 		if( nPos > nStart )
/*N*/ 			sDest += String( rStr.Copy( nStart, nPos-nStart ), eSource );
/*N*/ 		nStart = nPos+1;
/*N*/ 	}
/*N*/ 	while( nPos < rStr.Len() );
/*N*/ 
/*N*/ 	return sDest;
/*N*/ }

/*N*/ void Sw3IoImp::ChangeFontItemCharSet()
/*N*/ {
/*N*/ 	rtl_TextEncoding eSysEnc = gsl_getSystemTextEncoding();
/*N*/ 	sal_uInt16 nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_CHRATR_FONT );
/*N*/ 	SvxFontItem *pItem;
/*N*/ 	for( sal_uInt16 i=0; i<nMaxItems; i++ )
/*N*/ 	{
/*N*/ 		pItem = (SvxFontItem *)pDoc->GetAttrPool().GetItem( RES_CHRATR_FONT, i );
/*N*/ 		if( pItem && pItem->GetCharSet() == eSrcSet )
/*N*/ 			pItem->GetCharSet() = eSysEnc;
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::ConvertFmtsToStarSymbol()
/*N*/ {
/*N*/ 	if( pConvToSymbolFmts && pConvToSymbolFmts->Count() )
/*N*/ 	{
/*N*/ 		const Font& rSymbolFont = SwNumRule::GetDefBulletFont();
/*N*/ 		SvxFontItem aFontItem( rSymbolFont.GetFamily(), rSymbolFont.GetName(),
/*N*/ 						  	   rSymbolFont.GetStyleName(),
/*N*/ 							   rSymbolFont.GetPitch(),
/*N*/ 							   rSymbolFont.GetCharSet() );
/*N*/ 		for( USHORT i=0; i < pConvToSymbolFmts->Count(); i++ )
/*N*/ 		{
/*N*/ 			if( (SW3IO_CONV_TO_SYMBOL & pConvToSymbolFmts->GetFlags(i)) != 0 )
/*N*/ 				pConvToSymbolFmts->GetFmt(i)->SetAttr( aFontItem );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
*
*		Cleanup nach dem Laden
*
*************************************************************************/

/*N*/ void Sw3IoImp::Cleanup( sal_Bool bConnectPageDescs )
/*N*/ {
/*N*/ 	// Expr-Felder in Shared-Formaten?
/*N*/ //	if( nGblFlags & SW3F_UPDEXPR )
/*N*/ //		pDoc->SetUpdateExpFldStat();
/*N*/ 	// Offene Seitenbeschreibungen in Formaten aufloesen
/*N*/ 	// (falls noch nicht geschehen)
/*N*/ 	if( bConnectPageDescs )
/*?*/ 		ConnectPageDescAttrs();
/*N*/ 	// Temporaere Namenserweiterungen entfernen
/*N*/ 	aStringPool.RemoveExtensions( *pDoc );
/*N*/ 
/*N*/ 	rtl_TextEncoding eSysEnc = gsl_getSystemTextEncoding();
/*N*/ 	if( eSysEnc != eSrcSet )
/*N*/ 	{
/*N*/ 		// Bug 9714: Der CharSet an den Fonts muss geaendert werden, wenn
/*N*/ 		// es der globale CharSet ist
/*N*/ 		ChangeFontItemCharSet();
/*N*/ 
/*N*/ 		SvxFontItem *pItem = (SvxFontItem*) pDoc->GetAttrPool().GetPoolDefaultItem(RES_CHRATR_FONT);
/*N*/ 		if( pItem && pItem->GetCharSet() == eSrcSet )
/*N*/ 			pItem->GetCharSet() = eSysEnc;
/*N*/ 	}
/*N*/ 	ConvertFmtsToStarSymbol();
/*N*/ 
/*N*/ 	// Unbenutzte SwDrawFlyObjs aus dem Drawing Layer entfernen
/*N*/ 	// Dies sind vor allem jene, die vor dem Speichern in den
/*N*/ 	// Seitenkoepfen auftraten
/*N*/ 	if ( pDoc->GetDrawModel() )
/*N*/ 	{
/*N*/ //		if( nHiddenDrawObjs != ULONG_MAX )
/*N*/ //			RemoveHiddenDrawObjs();
/*N*/ 		SdrPage* pPage = pDoc->GetDrawModel()->GetPage( 0 );
/*N*/ 		sal_uInt32 i = 0;
/*N*/ 		while( i < pPage->GetObjCount() )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = pPage->GetObj( i );
/*N*/ 			if ( !pObj->GetUserCall() )
/*N*/ 			{
/*?*/ 				pPage->RemoveObject( i );
/*?*/ 				delete pObj;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				i++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//if( nVersion < SWG_DBTABLE )
/*N*/ 	if( !IsVersion( SWG_DBTABLE, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 		pDoc->SetInitDBFields(sal_True);
/*N*/ 
/*N*/ 	// Unbenutzte DB-Feldtypen loeschen
/*N*/ 	const SwFldTypes *pFldTypes = pDoc->GetFldTypes();
/*N*/ 	sal_uInt16 i = INIT_FLDTYPES;
/*N*/ 
/*N*/ 	SwFieldType* pFnd;
/*N*/ 
/*N*/ 	while (i < pFldTypes->Count())
/*N*/ 	{
/*N*/ 		if (RES_DBFLD == (pFnd = pFldTypes->GetObject(i))->Which() &&
/*N*/ 			((SwDBFieldType*)pFnd)->GetRefCount() <= 0)
/*N*/ 		{
/*?*/ 			pDoc->RemoveFldType(i);
/*N*/ 		}
/*N*/ 		else
/*N*/ 			i++;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Unbenutzte automatische NumRules loeschen.
/*N*/ 	// Kann hier geschehen, denn der NumRules-Stream wird nur bei normalen
/*N*/ 	// Dokumenten und beim Laden von NumRules und beim Laden von
/*N*/ 	// Seiten-Vorlagen gelesen. Beim normalen Laden und Laden von
/*N*/ 	// Seiten-Vorlagen wird Cleanup gerufen. Beim Laden von NumRules zwar
/*N*/ 	// nicht, da werden aber auch keine automatischen NumRules eingelesen.
/*N*/ 	// Wenn Vorlagen ueber den Organizer geladen werden, duerfen unbenutzte
/*N*/ 	// Vorlagen allerdings nicht geloescht werden, denn sie koennten aus
/*N*/ 	// dem Content-Stream kommen und der wird dann weder geladen noch
/*N*/ 	// gespeichert.
/*N*/ 	while( aNumRuleInfos.Count() )
/*N*/ 	{
/*N*/ 		Sw3NumRuleInfo *pInfo = aNumRuleInfos[0];
/*N*/ 		if( !bOrganizer && !pInfo->IsUsed() )
/*N*/ 		{
/*N*/ #ifdef DBG_UTIL
/*N*/ 			sal_uInt16 nCount = pDoc->GetNumRuleTbl().Count();
/*N*/ #endif
/*N*/ 			pDoc->DelNumRule( pInfo->GetNewName() );
/*N*/ #ifdef DBG_UTIL
/*N*/ 			ASSERT( pDoc->GetNumRuleTbl().Count() == nCount-1,
/*N*/ 					"unbenutzte NumRule wurde nicht geloescht" );
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		aNumRuleInfos.Remove( (sal_uInt16)0, 1 );
/*N*/ 		delete pInfo;
/*N*/ 	}
/*N*/ 
/*N*/ 	CleanupRedlines();
/*N*/ }

/*************************************************************************
*
*		Passwortroutinen
*
*************************************************************************/

// Testen des an der Root gesetzten Passwortes
// Der erforderliche Header ist bereits eingelesen

/*N*/ sal_Bool Sw3IoImp::CheckPasswd()
/*N*/ {
/*N*/ 	if( pCrypter )
/*N*/ 	{
/*?*/       sal_Char buf[ 17 ];
/*?*/       snprintf( buf, sizeof(buf), "%08lx%08lx", nDate, nTime );
/*?*/       ByteString aTest( buf );
/*?*/       pCrypter->Encrypt( aTest );
/*?*/       return sal_Bool( !memcmp( cPasswd, aTest.GetBuffer(), PASSWDLEN ) );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return sal_Bool( ( nFileFlags & SWGF_HAS_PASSWD ) == 0 );
/*N*/ }

// Setzen des Passwortes in der Root.

/*N*/ void Sw3IoImp::SetPasswd()
/*N*/ {
/*N*/ 	delete pCrypter; pCrypter = NULL;
/*N*/ 	if( pRoot && pRoot->GetKey().Len() )
/*N*/ 	{
/*?*/       pCrypter = new Crypter( pRoot->GetKey() );
/*?*/       sal_Char buf[ 17 ];
/*?*/       snprintf( buf, sizeof(buf), "%08lx%08lx", nDate, nTime );
/*?*/       ByteString aTest( buf );
/*?*/       pCrypter->Encrypt( aTest );
/*?*/       memcpy( cPasswd, aTest.GetBuffer(), aTest.Len() );
/*?*/       nFileFlags|= SWGF_HAS_PASSWD;
/*N*/ 	}
/*N*/ }

/*************************************************************************
*
*		unbenutzte Objekte aus 3.0-Storages entfernen
*
*************************************************************************/

/*N*/ void Sw3IoImp::RemoveUnusedObjects()
/*N*/ {
/*N*/ 	ASSERT( nVersion<=SWG_SHORTFIELDS && bNormal && !bInsert,
/*N*/ 			"ungueltiger Aufruf von RomoveUnusedObjects" );
/*N*/ 
/*N*/ 	if( pDoc->GetPersist()->GetObjectList() )
/*N*/ 	{
/*N*/ 		SvPersistRef rpDoc( pDoc->GetPersist() );
/*N*/ 		SvInfoObject* pIO = NULL;
/*N*/ 		String aObjName;
/*N*/ 
/*N*/ 		// alle Objekte in SvPersist betrachten
/*N*/ 		for( sal_uInt32 i=0; i < rpDoc->GetObjectList()->Count(); i++ )
/*N*/ 		{
/*?*/ 			pIO = rpDoc->GetObjectList()->GetObject(i);
/*?*/ 			aObjName = pIO->GetObjName();
/*?*/ 			if( aObjName.CompareToAscii("Obj",3)==COMPARE_EQUAL ||
/*?*/ 				aObjName.CompareToAscii("StarObj",7)==COMPARE_EQUAL )
/*?*/ 			{
/*?*/ 				// wenn kein OLE-Node dazu existiert: Objekt loeschen
/*?*/ 				if( p30OLENodes )
/*?*/ 					for( sal_uInt16 j=0; j<p30OLENodes->Count(); j++ )
/*?*/ 					{
/*?*/ 						if( aObjName==((*p30OLENodes)[j])->GetOLEObj().GetName() )
/*?*/ 						{
/*?*/ 							pIO = NULL;
/*?*/ 							p30OLENodes->Remove(j);
/*?*/ 							break;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				if( pIO )
/*?*/ 				{
/*?*/ 					rpDoc->Remove(aObjName);
/*?*/ 					i--; // das aktuelle Object fehlt ja jetzt
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

#ifdef TEST_HUGE_DOCS

#endif


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
