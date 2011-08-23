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


#define _ZFORLIST_DECLARE_TABLE

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <bf_sfx2/docinf.hxx>
#endif

#include <stdio.h>

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <bf_svtools/zforlist.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <bf_svx/svdmodel.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef SW_LINEINFO_HXX //autogen
#include <lineinfo.hxx>
#endif
#ifndef _PVPRTDAT_HXX
#include <pvprtdat.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SW3IMP_HXX
#include <sw3imp.hxx>
#endif
#ifndef _SW3MARKS_HXX
#include <sw3marks.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

#define URL_DECODE 	\
    , INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS

//#define TEST_FMTCHACHE

// local record of SWG_TOX
#define SWG_FORMPATTERN_LCL 'P'

// local record of SWG_FORMPATTERN_LCL
#define SWG_FORMTOKEN_LCL 'D'

class Sw3TOXBase : public SwTOXBase
{
    SwNodeIndex *pStartNodeIdx;
    SwNodeIndex *pEndNodeIdx;

    SwSectionFmt *pSectFmt;
    SwSectionFmt *pTitleSectFmt;

    sal_uInt32 nTitleLen;
    sal_uInt16 nStrIdx;

public:

    Sw3TOXBase( const SwTOXType* pTyp, const SwForm& rForm,
                sal_uInt16 nCreaType, const String& rTitle ) :
        SwTOXBase( pTyp, rForm, nCreaType, rTitle ),
        pStartNodeIdx( 0 ), pEndNodeIdx( 0 ),
        pSectFmt( 0 ), pTitleSectFmt( 0 ),
        nTitleLen( 0 ), nStrIdx( IDX_NO_VALUE )
    {}

    ~Sw3TOXBase();

    const SwNodeIndex *GetStartNodeIdx() const { return pStartNodeIdx; }
    const SwNodeIndex *GetEndNodeIdx() const { return pEndNodeIdx; }
    void SetNodeIdx( const SwNodeIndex& rNodeIdx );

    SwSectionFmt *GetSectFmt() const { return pSectFmt; }
    void SetSectFmt( SwSectionFmt *pFmt ) { pSectFmt = pFmt; }

    SwSectionFmt *GetTitleSectFmt() const { return pTitleSectFmt; }
    void SetTitleSectFmt( SwSectionFmt *pFmt ) { pTitleSectFmt = pFmt; }

    sal_uInt32 GetTitleLen() const { return nTitleLen; }
    void SetTitleLen( sal_uInt32 n ) { nTitleLen = n; }

    sal_uInt16 GetSectFmtStrIdx() const { return nStrIdx; }
    void SetSectFmtStrIdx( sal_uInt16 n ) { nStrIdx = n; }
};

/*N*/ SV_IMPL_PTRARR(Sw3TOXs,Sw3TOXBase*)

/*N*/ void Sw3TOXBase::SetNodeIdx( const SwNodeIndex& rNodeIdx )
/*N*/ {
/*N*/ 	SwNodeIndex *pNdIdx = new SwNodeIndex( rNodeIdx );
/*N*/ 	if( pStartNodeIdx )
/*N*/ 	{
/*N*/ 		ASSERT( !pEndNodeIdx, "more than two TOX indexes" );
/*N*/ 		if( !pEndNodeIdx )
/*N*/ 		{
/*N*/ 			if( rNodeIdx.GetIndex() < pStartNodeIdx->GetIndex() )
/*N*/ 			{
/*?*/ 				pEndNodeIdx = pStartNodeIdx;
/*?*/ 				pStartNodeIdx = pNdIdx;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pEndNodeIdx = pNdIdx;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pStartNodeIdx = pNdIdx;
/*N*/ 	}
/*N*/ }

/*N*/ Sw3TOXBase::~Sw3TOXBase()
/*N*/ {
/*N*/ 	delete pStartNodeIdx;
/*N*/ 	delete pEndNodeIdx;
/*N*/ 
/*N*/ 	// Section formats must not be deleted by delete.
/*N*/ 	SwDoc *pDoc = 0;
/*N*/ 	if( pTitleSectFmt )
/*N*/ 	{
/*?*/ 		pDoc = pTitleSectFmt->GetDoc();
/*?*/ 		pDoc->DelSectionFmt( pTitleSectFmt, sal_False );
/*N*/ 	}
/*N*/ 	if( pSectFmt )
/*N*/ 	{
/*?*/ 		if( !pDoc )
/*?*/ 			pDoc = pSectFmt->GetDoc();
/*?*/ 		pDoc->DelSectionFmt( pSectFmt, sal_False );
/*N*/ 	}
/*N*/ }

// Finden eines Formats nach Namen

/*N*/ SwFmt* Sw3IoImp::FindFmt( sal_uInt16 nIdx, sal_uInt8 cKind )
/*N*/ {
/*N*/ 	SwFmt* pFmt = NULL;
/*N*/ 	switch( nIdx )
/*N*/ 	{
/*N*/ 		case IDX_NO_VALUE:
/*N*/ 			return NULL;	// Direkter Abbruch, kein Assert
/*N*/ 		case IDX_DFLT_VALUE:
/*N*/ 			switch( cKind )
/*N*/ 			{
/*N*/ 				case SWG_FLYFMT:
/*N*/ 				case SWG_SDRFMT:
/*N*/ 				case SWG_FREEFMT:
/*N*/ 				case SWG_FRAMEFMT:
/*N*/ 					pFmt = pDoc->GetDfltFrmFmt(); break;
/*?*/ 				case SWG_CHARFMT:
/*?*/ 					pFmt = pDoc->GetDfltCharFmt(); break;
/*?*/ 				case SWG_GRFFMT:
/*?*/ 					pFmt = (SwFmt*) pDoc->GetDfltGrfFmtColl(); break;
/*N*/ 				case SWG_SECTFMT:
/*N*/ 				case 0:
/*N*/ 					return NULL;	// Direkter Abbruch, kein Assert
/*N*/ 			} break;
/*N*/ 		default:
/*N*/ 			// Holen des Namens und suchen im Doc
/*N*/ 
/*N*/ // OPT: Cache fuer Formate im StringPool
/*N*/ 			if( nIdx < IDX_SPEC_VALUE )
/*N*/ 			{
/*N*/ 				pFmt = aStringPool.FindCachedFmt( nIdx );
/*N*/ 				if( pFmt )
/*N*/ 				{
/*N*/ #ifdef TEST_FMTCHACHE
/*N*/ 					ASSERT( FindNamedFmt( nIdx, cKind ) == pFmt,
/*N*/ 							"Format-Cache liefert falsches Ergebnis" );
/*N*/ #endif
/*N*/ 					return pFmt;
/*N*/ 				}
/*N*/ 			}
/*N*/ // /OPT: Cache fuer Formate im StringPool
/*N*/ 
/*N*/ 			pFmt = FindNamedFmt( nIdx, cKind );
/*N*/ 	}
/*N*/ 	ASSERT( pFmt, "Format-ID unbekannt" );
/*N*/ 	return pFmt;
/*N*/ }

// Suchen eines (benannten) Formats am Dokument

/*N*/ SwFmt* Sw3IoImp::FindNamedFmt( sal_uInt16 nIdx, sal_uInt8 cKind )
/*N*/ {
/*N*/ 	if( nIdx < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		SwFmt* pFmt;
/*N*/ 		const String& rName = aStringPool.Find( nIdx );
/*N*/ 		sal_uInt16 n, nArrLen;
/*N*/ 		if( cKind == SWG_CHARFMT )
/*N*/ 		{
/*N*/ 			nArrLen = pDoc->GetCharFmts()->Count();
/*N*/ 			for( n = 0; n < nArrLen; n++ )
/*N*/ 			{
/*N*/ 				pFmt = (SwFmt*) (*pDoc->GetCharFmts())[ n ];
/*N*/ 				if( pFmt->GetName() == rName )
/*N*/ 				{
/*N*/ // OPT: Cache fuer Formate im StringPool
/*N*/ 					aStringPool.SetCachedFmt( nIdx, pFmt );
/*N*/ // /OPT: Cache fuer Formate im StringPool
/*N*/ 					return pFmt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( cKind == SWG_SECTFMT )
/*N*/ 		{
/*?*/ 			nArrLen = pDoc->GetSections().Count();
/*?*/ 			for( n = 0; n < nArrLen; n++ )
/*?*/ 			{
/*?*/ 				pFmt = pDoc->GetSections()[ n ];
/*?*/ 				if( pFmt->GetName() == rName )
/*?*/ 				{
/*?*/ // OPT: Cache fuer Formate im StringPool
/*?*/ 					aStringPool.SetCachedFmt( nIdx, pFmt );
/*?*/ // /OPT: Cache fuer Formate im StringPool
/*?*/ 					return pFmt;
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nArrLen = pDoc->GetFrmFmts()->Count();
/*N*/ 			const SwFrmFmts *pFrmFmts = pDoc->GetFrmFmts();
/*N*/ 			for( n = 0; n < nArrLen; n++ )
/*N*/ 			{
/*N*/ 				pFmt = (SwFmt*) (*pFrmFmts)[ n ];
/*N*/ 				if( pFmt->GetName() == rName )
/*N*/ 				{
/*N*/ // OPT: Cache fuer Formate im StringPool
/*N*/ 					aStringPool.SetCachedFmt( nIdx, pFmt );
/*N*/ // /OPT: Cache fuer Formate im StringPool
/*N*/ 					return pFmt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*?*/ 			if( 0 != ( pFmt = pDoc->FindSpzFrmFmtByName( rName )) ||
/*?*/ 				0 != ( pFmt = pDoc->FindTblFmtByName( rName )) )
/*?*/ 				return pFmt;
/*?*/ 		}
/*?*/ 		// Vielleicht koennen wir uns eines machen?
/*?*/ 		sal_uInt16 nPoolId = aStringPool.FindPoolId( nIdx );
/*?*/ 		if( !IsPoolUserFmt( nPoolId ) )  // fix 20976
/*?*/ 		{
/*?*/ 			pFmt = pDoc->GetFmtFromPool( nPoolId );
/*?*/ 			if( pFmt )
/*?*/ 				return pFmt;
/*?*/ 		}
/*?*/ 	}
/*?*/ 	ASSERT( !this, "Benanntes Format nicht gefunden" );
/*?*/ // Falls ueber den Organizer, die Styles veraendert wurden, kein Fehler
/*?*/ // melden, sonder auf Standard zurueck fallen
/*?*/ // Error();
/*?*/ 	Warning();
/*?*/ 	if( cKind == SWG_CHARFMT )
/*?*/ 		return pDoc->GetDfltCharFmt();
/*?*/ 	else if( cKind==SWG_FRAMEFMT || cKind==SWG_FLYFMT )
/*?*/ 		return pDoc->GetFmtFromPool( RES_POOLFRM_FRAME );
/*?*/ 	else if( cKind==SWG_SDRFMT )
/*?*/ 		return pDoc->GetDfltFrmFmt();
/*?*/ 
/*?*/  	Error();			// alle anderen erzeugen einen Fehler
/*?*/ 	return NULL;
/*N*/ }

// Suchen einer TextColl am Dokument
/*N*/ SwTxtFmtColl* Sw3IoImp::FindTxtColl( sal_uInt16 nIdx )
/*N*/ {
/*N*/ 	if( nIdx < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		const String& rName = aStringPool.Find( nIdx );
/*N*/ 		sal_uInt16 nArrLen = pDoc->GetTxtFmtColls()->Count();
/*N*/ 		for( sal_uInt16 n = 0; n < nArrLen; n++ )
/*N*/ 		{
/*N*/ 			SwTxtFmtColl* pColl = (SwTxtFmtColl*) (*pDoc->GetTxtFmtColls())[ n ];
/*N*/ 			if( pColl->GetName() == rName )
/*N*/ 				return pColl;
/*N*/ 		}
/*N*/ 		// Vielleicht koennen wir uns eines machen?
/*?*/ 		sal_uInt16 nPoolId = aStringPool.FindPoolId( nIdx );
/*?*/ 		if( nPoolId && !IsPoolUserFmt( nPoolId ) )
/*?*/ 		{
/*?*/ 			SwTxtFmtColl* pColl = pDoc->GetTxtCollFromPool( nPoolId );
/*?*/ 			if( pColl )
/*?*/ 				return pColl;
/*?*/ 		}
/*?*/ 		// Werden nur Seitenvorlagen eingelesen, wird still auf StdColl gemapt
/*?*/ 		// in diesem Fall lesen wir naemlich gerade einen Header/Footer ein,
/*?*/ 		// und der hat vielleicht eine unbekannte Absatzvorlage
/*?*/ 		if( bPageDescs && !bTxtColls )
/*?*/ 			return pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
/*?*/ 		ASSERT( !this, "TextColl nicht gefunden" );
/*?*/ 
/*?*/ // Falls ueber den Organizer, die Styles veraendert wurden, kein Fehler
/*?*/ // melden, sonder auf Standard zurueck fallen
/*?*/ //	 Error();
/*?*/ 		Warning();
/*?*/ 	}
/*?*/ 	return pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
/*N*/ }

// Suchen einer Seitenvorlage am Dokument

/*N*/ SwPageDesc* Sw3IoImp::FindPageDesc( sal_uInt16 nIdx )
/*N*/ {
/*N*/ 	if( nIdx < IDX_SPEC_VALUE )
/*N*/ 	{
/*N*/ 		const String& rName = aStringPool.Find( nIdx );
/*N*/ 		sal_uInt16 nArrLen = pDoc->GetPageDescCnt();
/*N*/ 		for( sal_uInt16 n = 0; n < nArrLen; n++ )
/*N*/ 		{
/*N*/ 			const SwPageDesc& rPg = pDoc->GetPageDesc( n );
/*N*/ 			if( rPg.GetName() == rName )
/*N*/ 				return (SwPageDesc*) &rPg;
/*N*/ 		}
/*N*/ 		// Vielleicht koennen wir uns eines machen?
/*?*/ 		sal_uInt16 nPoolId = aStringPool.FindPoolId( nIdx );
/*?*/ 		if( nPoolId )
/*?*/ 		{
/*?*/ 			SwPageDesc* p = pDoc->GetPageDescFromPool( nPoolId );
/*?*/ 			if( p )
/*?*/ 				return p;
/*?*/ 		}
/*?*/ 		ASSERT( !this, "PageDesc nicht gefunden" );
/*?*/ // Falls ueber den Organizer, die Styles veraendert wurden, kein Fehler
/*?*/ // melden, sonder auf Standard zurueck fallen
/*?*/ //	 Error();
/*?*/ 		Warning();
/*?*/ 	}
/*?*/ 	return (SwPageDesc*) &pDoc->GetPageDesc( 0 );
/*N*/ }

//////////////////////////////////////////////////////////////////////////

// Makros

void Sw3IoImp::InMacroTbl()
{
    OpenRec( SWG_MACROTBL );
    while( BytesLeft() )
    {
        OpenRec( SWG_MACRO );
        sal_uInt16 nKey, nScriptType = STARBASIC;
        String aLib, aMac;
        *pStrm >> nKey;
        InString( *pStrm, aLib );
        InString( *pStrm, aMac );

        if( SWG_SVXMACROS <= nVersion )
            *pStrm >> nScriptType;
        pDoc->SetGlobalMacro( nKey, SvxMacro( aMac, aLib,
                                        (ScriptType)nScriptType ) );
        CloseRec( SWG_MACRO );
    }
    CloseRec( SWG_MACROTBL );
}

/*N*/ void Sw3IoImp::OutMacroTbl()
/*N*/ {
/*N*/ 	const SvxMacroTableDtor& rTbl = pDoc->GetMacroTable();
/*N*/ 	SvxMacro* pMac = ((SvxMacroTableDtor&) rTbl).First();
/*N*/ 	if( !pMac )
/*N*/ 		return;
/*N*/ 
/*?*/ 	ASSERT( SOFFICE_FILEFORMAT_31 == pStrm->GetVersion() ||
/*?*/ 			SOFFICE_FILEFORMAT_40 == pStrm->GetVersion() ||
/*?*/ 			SOFFICE_FILEFORMAT_50 == pStrm->GetVersion(),
/*?*/ 			"Macro-Table: Gibt es ein neues Fileformat?" );
/*?*/ 	if( SOFFICE_FILEFORMAT_31 == pStrm->GetVersion() )
/*?*/ 	{
/*?*/ 		// suche das erste StarBasicMacro!
/*?*/ 		while( pMac && STARBASIC != pMac->GetScriptType() )
/*?*/ 			pMac = ((SvxMacroTableDtor&) rTbl).Next();
/*?*/ 		if( !pMac )
/*?*/ 			return ;
/*?*/ 	}
/*?*/ 
/*?*/ 	OpenRec( SWG_MACROTBL );
/*?*/ 	while( pMac && Good() )
/*?*/ 	{
/*?*/ 		OpenRec( SWG_MACRO );
/*?*/ 		*pStrm << (sal_uInt16) rTbl.GetCurKey();
/*?*/ 	  	OutString( *pStrm, pMac->GetLibName() );
/*?*/ 	 	OutString( *pStrm, pMac->GetMacName() );
/*?*/ 
/*?*/ 		if( SOFFICE_FILEFORMAT_31 == pStrm->GetVersion() )
/*?*/ 		{
/*?*/ 			do {
/*?*/ 				pMac = ((SvxMacroTableDtor&) rTbl).Next();
/*?*/ 			} while( pMac && STARBASIC != pMac->GetScriptType() );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			*pStrm << (sal_uInt16)pMac->GetScriptType();
/*?*/ 			pMac = ((SvxMacroTableDtor&) rTbl).Next();
/*?*/ 		}
/*?*/ 
/*?*/ 		CloseRec( SWG_MACRO );
/*?*/ 	}
/*?*/ 	CloseRec( SWG_MACROTBL );
/*N*/ }

/*************************************************************************
*
*		Job Setup
*
*************************************************************************/

/*N*/ void Sw3IoImp::InJobSetup()
/*N*/ {
/*N*/ 	OpenRec( SWG_JOBSETUP );
/*N*/ 	OpenFlagRec();
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	//JP 13.10.95: laut Changes-Mail von MI
/*N*/ 	static sal_uInt16 __READONLY_DATA nRange[] =
/*N*/ 					{
/*N*/ 						FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
/*N*/ 						SID_HTML_MODE,	SID_HTML_MODE,
/*N*/ 						SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
/*N*/ 						SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
/*N*/ 						0
/*N*/ 					};
/*N*/ 	SfxItemSet *pItemSet = new SfxItemSet( pDoc->GetAttrPool(), nRange );
/*N*/ 	SfxPrinter *pPrinter = SfxPrinter::Create( *pStrm, pItemSet );
/*N*/ 
/*N*/     if ( !IsVersion(SWG_VIRTUAL_DEVICE) )
/*N*/         pDoc->_SetUseVirtualDevice( sal_False );
/*N*/ 	pDoc->_SetPrt( pPrinter );
/*N*/ 
/*N*/ 	if ( !pPrinter->IsOriginal() )
/*N*/ 	{
/*N*/ 		pDoc->GetDocShell()->UpdateFontList();
/*N*/ 		if ( pDoc->pDrawModel )
/*N*/ 			pDoc->pDrawModel->SetRefDevice( pPrinter );
/*N*/ 
/*N*/         pDoc->SetOLEPrtNotifyPending( sal_True );
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_JOBSETUP );
/*N*/ }

// Neu fuer OS/2: Wenn das Job-Setup keine Daten enthaelt, dann wird das
// Job-Setup nicht rausgeschrieben.

/*N*/ void Sw3IoImp::OutJobSetup()
/*N*/ {
/*N*/ 	SfxPrinter*	pPrt = pDoc->GetPrt();
/*N*/ 	if( pPrt )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_JOBSETUP );
/*N*/ 		sal_uInt8 cFlags = 0x00;
/*N*/ 		*pStrm << cFlags;
/*N*/ 		pPrt->Store( *pStrm );
/*N*/ 		CloseRec( SWG_JOBSETUP );
/*N*/ 	}
/*N*/ }

/*************************************************************************
*
*		Stringpool (ab Version 2)
*
*************************************************************************/

/*N*/ void Sw3IoImp::InStringPool( sal_uInt8 cType, Sw3StringPool& rPool )
/*N*/ {
/*N*/ 	OpenRec( cType );
/*N*/ 	if( nVersion < SWG_POOLIDS )
            rPool.LoadOld( *pStrm );
/*N*/ 	else
/*N*/ 		rPool.Load( *pStrm, nVersion );
/*N*/ 	CloseRec( cType );
/*N*/ 	if( pStrm->GetError() != SVSTREAM_OK )
/*N*/ 	 	Error( ERR_SWG_READ_ERROR );
/*N*/ }

/*N*/ void Sw3IoImp::OutStringPool( sal_uInt8 cType, Sw3StringPool& rPool )
/*N*/ {
/*N*/ 	OpenRec( cType );
/*N*/ 	rPool.Store( *pStrm );
/*N*/ 	CloseRec( cType );
/*N*/ }

void Sw3IoImp::InPasswd()
{
    OpenRec( SWG_PASSWORD );
    if( nVersion >= SWG_CRYPT )
    {
        sal_uInt8 cType;
        ByteString aPasswd;
        *pStrm >> cType;
        // TODO: unicode: It seems that we had a bug here, because the
        // password was converted from the source to the system encoding
        // before it was decrypted. We now decrypt it first.
        pStrm->ReadByteString( aPasswd );
        /*
        // Datum und Uhrzeit als Passwort fuers Passwort nehmen
        sal_Char buf[ 17 ];
        snprintf( buf, sizeof(buf), "%08lx%08lx", nDate, nTime );
        Crypter( buf ).Decrypt( aPasswd );
        switch( cType )
        {
            case 1:
                {
                    ::com::sun::star::uno::Sequence <sal_Int8> aPWD;
                    SvPasswordHelper::GetHashPassword( aPWD,
                                        String( aPasswd, eSrcSet ));
                    pDoc->ChgSectionPasswd( aPWD );
                    break;
                }
        }*/
    }
    CloseRec( SWG_PASSWORD );
}

/*N*/ int sw3mark_compare( const Sw3Mark& r1, const Sw3Mark& r2 )
/*N*/ {
/*N*/ 	int nRet = 0;
/*N*/ 	if( r1.nNodePos < r2.nNodePos )
/*N*/ 		nRet = -1;
/*N*/ 	else if( r1.nNodePos > r2.nNodePos )
/*N*/ 		nRet = 1;
/*N*/ 	else if( r1.nNodeOff < r2.nNodeOff )
/*N*/ 		nRet = -1;
/*N*/ 	else if( r1.nNodeOff > r2.nNodeOff )
/*N*/ 		nRet = 1;
/*N*/ 	else if( r1.nId < r2.nId )
/*N*/ 		nRet = -1;
/*N*/ 	else if( r1.nId > r2.nId || r1.eType != r2.eType )
/*N*/ 		nRet = 1;
/*N*/ 
/*N*/ 	return nRet;
/*N*/ }

/*N*/ IMPL_CONTAINER_SORT( Sw3Marks, Sw3Mark, sw3mark_compare )

extern sal_Bool lcl_sw3io_isTOXHeaderSection( const SwStartNode& rSttNd );
/*N*/ void Sw3IoImp::CollectMarks( SwPaM* pPaM, sal_Bool bPageOnly )
/*N*/ {
/*N*/ 	sal_uInt32 nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
/*N*/ 
/*N*/ 	// Bereich bestimmen
/*N*/ 	sal_uInt32 nStart, nEnd;
/*N*/ 	if( !bSaveAll && pPaM )
/*?*/ 		nStart = pPaM->GetPoint()->nNode.GetIndex(),
/*?*/ 		nEnd   = pPaM->GetMark()->nNode.GetIndex();
/*N*/ 	else
/*N*/ 		nStart = 0,
/*N*/ 		nEnd   = pDoc->GetNodes().GetEndOfContent().GetIndex();
/*N*/ 	if( nStart > nEnd )
/*N*/ 	{
/*N*/ 		// Start muss kleiner als Ende sein
/*N*/ 		sal_uInt32 n = nStart; nStart = nEnd; nEnd = n;
/*N*/ 	}
/*N*/ 	// Array anlegen
/*N*/ 	delete pMarks;
/*N*/ 	pMarks = 0;
/*N*/ 
/*N*/ 	delete pBookmarks;
/*N*/ 	pBookmarks = new Sw3Bookmarks;
/*N*/ 	nCntntBkmkStart = 0;
/*N*/ 
/*N*/ 	Sw3Mark aMark;
/*N*/ 	// text::Bookmarks absammeln (nicht, wenn Konversion SW2-Textbausteine auf SW3)
/*N*/ 	if( !( nGblFlags & SW3F_CONVBLOCK ) )
/*N*/ 	{
/*N*/ 		//JP 23.09.95: wenn Selektion geschrieben wird, dann auch alle aus
/*N*/ 		//				den Sonderbereichen mit nehmen!!
/*N*/ 		const SwBookmarks& rMarks = pDoc->GetBookmarks();
/*N*/ 		sal_uInt16 nArrLen = rMarks.Count();
/*N*/ 
/*N*/ 		for( sal_uInt16 n = 0; n < nArrLen; ++n )
/*N*/ 		{
/*N*/ 			SwBookmark* pMark = rMarks.GetObject( n );
/*N*/ 			if( !pMark->IsBookMark() )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			const SwPosition& rPos1 = pMark->GetPos();
/*N*/ 			const SwPosition* pPos2 = pMark->GetOtherPos();
/*N*/ 			if( rPos1.nContent.GetIndex() > STRING_MAXLEN52 &&
/*N*/ 				( !pPos2 ||
/*N*/ 				  (rPos1.nNode.GetIndex() == pPos2->nNode.GetIndex() &&
/*N*/ 				   pPos2->nContent.GetIndex() > STRING_MAXLEN52) ) )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			if( pDoc->IsInHeaderFooter( rPos1.nNode ) )
/*N*/ 			{
/*?*/ 				pBookmarks->Insert( pMark, nCntntBkmkStart );
/*?*/ 				nCntntBkmkStart++;
/*N*/ 			}
/*N*/ 			else if( !bPageOnly )
/*N*/ 			{
/*N*/ 				pBookmarks->Insert( pMark, pBookmarks->Count() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		nArrLen = pBookmarks->Count();
/*N*/ 		if( nArrLen )
/*N*/ 		{
/*N*/ 			pMarks = new Sw3Marks( nArrLen + nArrLen / 4, nArrLen / 4 );
/*N*/ 
/*N*/ 			if( pPaM )
/*N*/ 			{
/*N*/ 				for( sal_uInt16 n = 0; n < nArrLen; ++n )
/*N*/ 				{
/*N*/ 					const SwBookmark* pMark = pBookmarks->GetObject( n );
/*N*/ 					ASSERT( pMark->IsBookMark(),
/*N*/ 							"Wo kommt da die Nicht text::Bookmark her?" );
/*N*/ 
/*N*/ 					if( !IsSw31Export() && n >= nCntntBkmkStart )
/*N*/ 						aMark.SetId( n - nCntntBkmkStart );
/*N*/ 					else
/*N*/ 						aMark.SetId( n );
/*N*/ 					const SwPosition* pPos1 = &pMark->GetPos();
/*N*/ 					const SwPosition* pPos2 = pMark->GetOtherPos();
/*N*/ 					ASSERT( pPos1->nNode.GetNode().IsTxtNode(),
/*N*/ 							 "Bookmark position outside text node" );
/*N*/ 					aMark.SetNodePos( pPos1->nNode.GetIndex() );
/*N*/ 					if( aMark.GetNodePos() < nEndOfIcons ||
/*N*/ 						( aMark.GetNodePos() >= nStart && aMark.GetNodePos() <= nEnd ))
/*N*/ 					{
/*N*/ 						aMark.SetNodeOff( pPos1->nContent.GetIndex() );
/*N*/ 						aMark.SetType( SW3_BOOK_POINT );
/*N*/ 						pMarks->Insert( new Sw3Mark(aMark) );
/*N*/ 					}
/*N*/ 					if( pPos2 )
/*N*/ 					{
/*?*/ 						ASSERT( pPos2->nNode.GetNode().IsTxtNode(),
/*?*/ 								"Other bookmark position outside text node" );
/*?*/ 						aMark.SetNodePos( pPos2->nNode.GetIndex() );
/*?*/ 						if( aMark.GetNodePos() < nEndOfIcons ||
/*?*/ 							( aMark.GetNodePos() >= nStart && aMark.GetNodePos() <= nEnd ))
/*?*/ 						{
/*?*/ 							aMark.SetNodeOff( pPos2->nContent.GetIndex() );
/*?*/ 							aMark.SetType( SW3_BOOK_MARK );
/*?*/ 							pMarks->Insert( new Sw3Mark(aMark) );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				for( sal_uInt16 n = 0; n < nArrLen; ++n )
/*?*/ 				{
/*?*/ 					const SwBookmark* pMark = pBookmarks->GetObject( n );
/*?*/ 					ASSERT( pMark->IsBookMark(),
/*?*/ 							"Wo kommt da die Nicht Bookmark her?" );
/*?*/ 
/*?*/ 					if( !IsSw31Export() && n >= nCntntBkmkStart )
/*?*/ 						aMark.SetId( n - nCntntBkmkStart );
/*?*/ 					else
/*?*/ 						aMark.SetId( n );
/*?*/ 
/*?*/ 					const SwPosition* pPos = &pMark->GetPos();
/*?*/ 					ASSERT( pPos->nNode.GetNode().IsTxtNode(),
/*?*/ 							 "Bookmark position outside text node" );
/*?*/ 
/*?*/ 					aMark.SetNodePos( pPos->nNode.GetIndex() );
/*?*/ 					aMark.SetNodeOff( pPos->nContent.GetIndex() );
/*?*/ 					aMark.SetType( SW3_BOOK_POINT );
/*?*/ 					pMarks->Insert( new Sw3Mark(aMark) );
/*?*/ 
/*?*/ 					pPos = pMark->GetOtherPos();
/*?*/ 					if( pPos )
/*?*/ 					{
/*?*/ 						ASSERT( pPos->nNode.GetNode().IsTxtNode(),
/*?*/ 								"Other bookmark position outside text node" );
/*?*/ 						aMark.SetNodePos( pPos->nNode.GetIndex() );
/*?*/ 						aMark.SetNodeOff( pPos->nContent.GetIndex() );
/*?*/ 						aMark.SetType( SW3_BOOK_MARK );
/*?*/ 						pMarks->Insert( new Sw3Mark(aMark) );
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Collect TOX sections. To be compatible with the 5.0 file format and
/*N*/ 	// earlier file format versions, TOX section are not exported directly.
/*N*/ 	// Instead of this, the start and end positions of this sections is
/*N*/ 	// exported.
/*N*/ 	// MIB 01.09.97: wenn Selektion geschrieben wird, dann auch alle aus
/*N*/ 	//				 den Sonderbereichen mit nehmen (##)
/*N*/ 
/*N*/ 	aMark.SetId( 0 );
/*N*/ 	const SwSectionFmts& rSectFmts = pDoc->GetSections();
/*N*/ 
/*N*/ 	for( sal_uInt16 i=0; i < rSectFmts.Count(); i++ )
/*N*/ 	{
/*N*/ 		const SwSectionFmt* pSectFmt = rSectFmts[i];
/*N*/ 
/*N*/ 		// Skip TOXs that are somehow incomplete.
/*N*/ 		const SwSection* pSect = pSectFmt->GetSection();
/*N*/ 		if( !pSect || TOX_CONTENT_SECTION != pSect->GetType() )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwSectionNode *pSectNd = pSectFmt->GetSectionNode();
/*N*/ 		if( !pSectNd )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwTOXBaseSection *pTOXBaseSect =
/*N*/ 			PTR_CAST( SwTOXBaseSection, pSect );
/*N*/ 		if( !pTOXBaseSect || !pTOXBaseSect->GetTOXType() )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		ULONG nStartIdx = pSectNd->GetIndex();
/*N*/ 		ULONG nEndIdx = pSectNd->EndOfSectionIndex();
/*N*/ 
/*N*/ 		// Skip TOXs that are not contained within the saved area completely.
/*N*/ 		if( nStart >= nEndOfIcons && (nStartIdx < nStart || nEndIdx > nEnd) )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		if( !pMarks )
/*N*/ 			pMarks = new Sw3Marks( 16, 4 );
/*N*/ 
/*N*/ 		// The start pos is the first section node. If this is the start node
/*N*/ 		// of a TOX header section that starts with a text node, the start pos
/*N*/ 		// is the second node. See Sw3IoImp::OutSection in sw3sect.cxx.
/*N*/ 		nStartIdx++;
/*N*/ 		const SwStartNode *pSttNd = pDoc->GetNodes()[nStartIdx]->GetStartNode();
/*N*/ 		if( pSttNd && lcl_sw3io_isTOXHeaderSection( *pSttNd ) &&
/*N*/ 			pDoc->GetNodes()[nStartIdx+1]->IsTxtNode() ) // could be a start nd
/*N*/ 			nStartIdx++;
/*N*/ 
/*N*/ 		aMark.SetNodePos( nStartIdx );
/*N*/ 		aMark.SetNodeOff( 0 );
/*N*/ 		aMark.SetType( SW3_TOX_POINT );
/*N*/ 		pMarks->Insert( new Sw3Mark(aMark) );
/*N*/ 
/*N*/ 		// The end pos is the last but one section node. If this is the end
/*N*/ 		// node of a TOX header section that ends with a text node, the end
/*N*/ 		// pos is the last but two node. See Sw3IoImp::OutSection in
/*N*/ 		// sw3sect.cxx.
/*N*/ 		nEndIdx--;
/*N*/ 		const SwEndNode *pEndNd = pDoc->GetNodes()[nEndIdx]->GetEndNode();
/*N*/ 		if( pEndNd &&
/*N*/ 			lcl_sw3io_isTOXHeaderSection( *pEndNd->StartOfSectionNode() ) &&
/*N*/ 			pDoc->GetNodes()[nEndIdx-1]->IsTxtNode() ) // could be an end node
/*N*/ 			nEndIdx--;
/*N*/ 
/*N*/ 		aMark.SetNodePos( nEndIdx );
/*N*/ 
/*N*/ 		const SwTxtNode *pTxtNd = pDoc->GetNodes()[nEndIdx]->GetTxtNode();
/*N*/ 		xub_StrLen nCntntIdx = pTxtNd ? pTxtNd->Len() : 0;
/*N*/ 		aMark.SetNodeOff( nCntntIdx );
/*N*/ 		aMark.SetType( SW3_TOX_MARK );
/*N*/ 		pMarks->Insert( new Sw3Mark(aMark) );
/*N*/ 
/*N*/ 		aMark.SetId( aMark.GetId() + 1 );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ist was da?
/*N*/ 	if( !pBookmarks->Count() )
/*N*/ 		delete pBookmarks, pBookmarks = NULL;
/*N*/ 	ASSERT( !pMarks || pMarks->Count(),
/*N*/ 			"Marks-Array haette nicht angelegt werden muessen" );
/*N*/ 	if( pMarks && !pMarks->Count() )
/*?*/ 		delete pMarks, pMarks = NULL;
/*N*/ }

// Eine Mark einlesen

/*N*/ void Sw3IoImp::InNodeMark( const SwNodeIndex& rPos, xub_StrLen nCntntOff )
/*N*/ {
/*N*/ 	OpenRec( SWG_MARK );
/*N*/ 	sal_uInt8 cType;
/*N*/ 	sal_uInt16 nId, nOff;
/*N*/ 	*pStrm >> cType >> nId >> nOff;
/*N*/ 	CloseRec( SWG_MARK );
/*N*/ 
/*N*/ 	SwIndex aOff( rPos.GetNode().GetCntntNode(), nCntntOff + nOff );
/*N*/ 	if( cType < SW3_BOOK_POINT )
/*N*/ 	{
/*N*/ 		ASSERT( pTOXs && nId < pTOXs->Count(), "invalid TOX id" );
/*N*/ 		if( pTOXs && nId < pTOXs->Count() )
/*N*/ 			pTOXs->GetObject( nId )->SetNodeIdx( rPos );
/*N*/ 	}
/*N*/ 	else if( cType < SW3_REDLINE_START)
/*N*/ 	{
/*N*/ 		SwBookmark* pBook = 0;
/*N*/ 		if( pBookmarks && nId < pBookmarks->Count() )
/*N*/ 			pBook = pBookmarks->GetObject( nId );
/*N*/ 		ASSERT( pBook, "Ungueltige text::Bookmark-ID" );
/*N*/ 
/*N*/ 		if( pBook )
/*N*/ 		{
/*N*/ 			switch( cType )
/*N*/ 			{
/*N*/ 				case SW3_BOOK_POINT:
/*N*/ 				{
/*N*/ 					delete pBook->pPos1;
/*N*/ 					pBook->pPos1 = new SwPosition( rPos, aOff );
/*N*/ 
/*N*/ 					// Da dies die Sort Order durcheinander bringt,
/*N*/ 					// die text::Bookmark neu einsortieren!
/*N*/ 					SwBookmarks& rMarks = (SwBookmarks&) pDoc->GetBookmarks();
/*N*/ 
/*N*/ 					// JP 19.07.95: das Suchen machen wir ueber den Pointer!!
/*N*/ 					// ansonsten muss es vorm setzen der neue
/*N*/ 					// Position erfolgen !! (SortArray!!!!!)
/*N*/ 					const SwBookmarkPtr* ppMarks = rMarks.GetData();
/*N*/ 					for( sal_uInt16 nCnt = rMarks.Count(); nCnt; --nCnt, ++ppMarks )
/*N*/ 						if( *ppMarks == pBook )
/*N*/ 						{
/*N*/ 							rMarks.Remove( rMarks.Count() - nCnt );
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 					rMarks.Insert( pBook );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				case SW3_BOOK_MARK:
/*?*/ 					delete pBook->pPos2;
/*?*/ 					pBook->pPos2 = new SwPosition( rPos, aOff );
/*?*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		Warning();
/*N*/ 	}
/*N*/ }

// Die Marks eines Nodes schreiben

/*N*/ void Sw3IoImp::OutNodeMarks( ULONG nIdx )
/*N*/ {
/*N*/ 	if(pMarks )
/*N*/ 	{
/*N*/ 		for( sal_uInt16 nPos = 0; nPos < pMarks->Count(); ++nPos )
/*N*/ 		{
/*N*/ 			Sw3Mark *pMark = (*pMarks)[ nPos ];
/*N*/ 			if( pMark->GetNodePos() == nIdx )
/*N*/ 			{
/*N*/ 				OpenRec( SWG_MARK );
/*N*/ 				xub_StrLen nOffs = pMark->GetNodeOff();
/*N*/ 				if( nOffs > STRING_MAXLEN52 )
/*N*/ 					nOffs = STRING_MAXLEN52;
/*N*/ 				*pStrm << (sal_uInt8) pMark->GetType()
/*N*/ 					   << (sal_uInt16) pMark->GetId()
/*N*/ 					   << (sal_uInt16) nOffs;
/*N*/ 				CloseRec( SWG_MARK );
/*N*/ 				pMarks->Remove( nPos-- );
/*N*/ 				delete pMark;
/*N*/ 				if( !pMarks->Count() )
/*N*/ 				{
/*N*/ 					delete pMarks;
/*N*/ 					pMarks = NULL;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if( pMark->GetNodePos() > nIdx )
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// text::Bookmark einlesen

/*N*/ void Sw3IoImp::InBookmarks()
/*N*/ {
/*N*/ 	ASSERT( !pBookmarks || IsVersion(SWG_SVXMACROS),
/*N*/ 			"Frueher standen text::Bookmarks doch nur in einem Stream???" )
/*N*/ 	if( pBookmarks )
/*N*/ 	{
/*?*/ 		delete pBookmarks;
/*?*/ 		pBookmarks = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	OpenRec( SWG_BOOKMARKS );
/*N*/ 	// Die Mark erst einmal an den Anfang setzen
/*N*/ 	SwPaM aPaM( pDoc->GetNodes().GetEndOfPostIts() );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		String aShortName, aName;
/*N*/ 		OpenRec( SWG_BOOKMARK );
/*N*/ 		InString( *pStrm, aShortName );
/*N*/ 		InString( *pStrm, aName );
/*N*/ 		OpenFlagRec();
/*N*/ 		sal_uInt16 nOffset, nKey, nMod;
/*N*/ 		*pStrm >> nOffset >> nKey >> nMod;
/*N*/ 		CloseFlagRec();
/*N*/ 		SwBookmarkPtr pMark = NULL;
/*N*/ 
/*N*/ 		// Gibt es die Marke bereits (tw. fix 23304) ?
/*N*/ 		if( bInsert && pDoc->FindBookmark( aName ) != USHRT_MAX )
/*?*/ 			pDoc->MakeUniqueBookmarkName( aName );
/*N*/ 
/*N*/ 		pMark = pDoc->MakeBookmark( aPaM, KeyCode( nKey, nMod ),
/*N*/ 									aName, aShortName );
/*N*/ 		if( pMark )
/*N*/ 		{
/*N*/ 			// JP 10.12.96: die Macros an den text::Bookmarks koennen von der
/*N*/ 			//				UI nirgends gesetzt/angefragt werden. Darum
/*N*/ 			//				werden sie auch nicht um den ScriptType erweitert!
/*N*/ 			String aMac, aLib;
/*N*/ 
/*N*/ 			InString( *pStrm, aMac );
/*N*/ 			InString( *pStrm, aLib );
/*N*/ 			SvxMacro aStart( aMac, aLib, STARBASIC );
/*N*/ 			pMark->SetStartMacro( aStart );
/*N*/ 
/*N*/ 			InString( *pStrm, aMac );
/*N*/ 			InString( *pStrm, aLib );
/*N*/ 			SvxMacro aEnd( aMac, aLib, STARBASIC );
/*N*/ 			pMark->SetEndMacro( aEnd );
/*N*/ 		}
/*N*/ 		CloseRec( SWG_BOOKMARK );
/*N*/ 		// Und fuer die Mark-Records merken. NULL bedeutet, dass
/*N*/ 		// die text::Bookmark eine Dublette war und fortfaellt.
/*N*/ 		if( !pBookmarks )
/*N*/ 			pBookmarks = new Sw3Bookmarks;
/*N*/ 		pBookmarks->Insert( pMark, pBookmarks->Count() );
/*N*/ 	}
/*N*/ 	CloseRec( SWG_BOOKMARKS );
/*N*/ }

// String		Kurzname
// String		voller Name
// sal_uInt8 		Flags
// sal_uInt16		Offset
// sal_uInt16		KeyCode
// sal_uInt16		Modifier
// String		Name des Startmakros
// String		Lib des Startmakros
// String		Name des Endmakros
// String		Lib des Endmakros

/*N*/ void Sw3IoImp::OutBookmarks( sal_Bool bPageStyles )
/*N*/ {
/*N*/ 	ASSERT( bPageStyles || !IsSw31Export(),
/*N*/ 			"Beim 3.1-Export kommen keine text::Bookmarks in den Contents-Stream" );
/*N*/ 
/*N*/ 	short nArrLen = pBookmarks ? pBookmarks->Count() : 0;
/*N*/ 	if( nArrLen && bPageStyles && !IsSw31Export() )
/*N*/ 	{
/*N*/ 		ASSERT( nCntntBkmkStart <= nArrLen,
/*N*/ 				"Mehr text::Bookmarks in Page-Styles als ueberhaupt vorhanden?" );
/*N*/ 		nArrLen = nCntntBkmkStart;
/*N*/ 		nCntntBkmkStart = 0;
/*N*/ 	}
/*N*/ 	ASSERT( IsSw31Export() || nCntntBkmkStart==0,
/*N*/ 			"Wieso sind da noch text::Bookmarks aus Seitenvorlagen?" );
/*N*/ 
/*N*/ 	if( nArrLen )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_BOOKMARKS );
/*N*/ 		for( int i = 0; i < nArrLen; i++ )
/*N*/ 		{
/*N*/ 			SwBookmark& rMark = (SwBookmark&) *(*pBookmarks)[i];
/*N*/ 			ASSERT( rMark.IsBookMark(),
/*N*/ 					"Wo kommt da die Nicht text::Bookmark her?" )
/*N*/ 
/*N*/ 			const SvxMacro& rStt = rMark.GetStartMacro();
/*N*/ 			const SvxMacro& rEnd = rMark.GetEndMacro();
/*N*/ 			OpenRec( SWG_BOOKMARK );
/*N*/ 			OutString( *pStrm, rMark.GetShortName() );
/*N*/ 			OutString( *pStrm, rMark.GetName() );
/*N*/ 			*pStrm << (BYTE) 0x06
/*N*/ 				   << (sal_uInt16) 0	// frueher: Position, jetzt frei
/*N*/ 				   << (sal_uInt16) rMark.GetKeyCode().GetCode()
/*N*/ 				   << (sal_uInt16) rMark.GetKeyCode().GetModifier();
/*N*/ 						// JP 10.12.96: die Macros an den text::Bookmarks koennen
/*N*/ 						//				von der UI nirgends gesetzt/abgefragt
/*N*/ 						//				werden. Darum werden sie auch nicht
/*N*/ 						//				um den ScriptType erweitert!
/*N*/ 			OutString( *pStrm, rStt.GetMacName() );
/*N*/ 			OutString( *pStrm, rStt.GetLibName() );
/*N*/ 			OutString( *pStrm, rEnd.GetMacName() );
/*N*/ 			OutString( *pStrm, rEnd.GetLibName() );
/*N*/ 			CloseRec( SWG_BOOKMARK );
/*N*/ 		}
/*N*/ 		CloseRec( SWG_BOOKMARKS );
/*N*/ 
/*N*/ 		// Wenn die text::Bookmarks fuer Page-Styles geschrienen sind, werden
/*N*/ 		// sie geloescht
/*N*/ 		if( bPageStyles && !IsSw31Export() )
/*?*/ 			pBookmarks->Remove( 0, nArrLen );
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::InTOXs51()
/*N*/ {
/*N*/ 	OpenRec( SWG_TOXDESCS51 );
/*N*/ 
/*N*/ 	// Die Mark erst einmal an den Anfang setzen
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_TOXDESC );
/*N*/ 		String aTypeName, aTitle;
/*N*/ 		sal_uInt16 nStrIdx = IDX_NO_VALUE;
/*N*/ 		if( IsVersion( SWG_LONGIDX ) )
/*N*/ 			*pStrm >> nStrIdx;
/*N*/ 		else
/*N*/ 			InString( *pStrm, aTypeName );
/*N*/ 
/*N*/ 		InString( *pStrm, aTitle );
/*N*/ 		sal_uInt8 cFlags = OpenFlagRec();
/*N*/ 		sal_Int16 nCreateType;
/*N*/ 		sal_uInt16 nFirstTabPos = 0;
/*N*/ 		BYTE  cType;
/*N*/ 		*pStrm >> nCreateType
/*N*/ 			   >> cType;
/*N*/ 
/*N*/ 		if( IsVersion(SWG_TOXTABS) && (cFlags & 0x10) != 0 )
/*?*/ 			*pStrm >> nFirstTabPos;
/*N*/ 		CloseFlagRec();
/*N*/ 
/*N*/ 		TOXTypes eTOXType = (TOXTypes) cType;
/*N*/ 
/*N*/ 		// Wenn keine Name da ist, dann handelt es sich um
/*N*/ 		// das Standard-Verzeichnis des entsprechenden Typs. Der
/*N*/ 		// (internationalisierte) Name wird dann gesetzt.
/*N*/ 		if( IDX_NO_VALUE != nStrIdx )
/*?*/ 			aTypeName = aStringPool.Find( nStrIdx );
/*N*/ 		else if( !aTypeName.Len() )
/*N*/ 			aTypeName = SwTOXBase::GetTOXName( eTOXType );
/*N*/ 
/*N*/ 		// nach dem TOXType suchen
/*N*/ 		sal_uInt16 nTOXType = pDoc->GetTOXTypeCount( eTOXType );
/*N*/ 		const SwTOXType* pTOXType = NULL;
/*N*/ 		for( sal_uInt16 n = 0; n < nTOXType; n++ )
/*N*/ 		{
/*N*/ 			const SwTOXType* p = pDoc->GetTOXType( eTOXType, n );
/*N*/ 			if( p->GetTypeName() == aTypeName )
/*N*/ 			{
/*N*/ 				pTOXType = p; break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Falls nicht vorhanden, am Dokument einfuegen
/*N*/ 		if( !pTOXType )
/*N*/ 		{
/*N*/ 			pDoc->InsertTOXType( SwTOXType ( eTOXType, aTypeName ) );
/*N*/ 			pTOXType = pDoc->GetTOXType( eTOXType, nTOXType );
/*N*/ 		}
/*N*/ 		// Die SwForm einlesen
/*N*/ 		SwForm aForm( cType );
/*N*/ 
/*N*/ 		sal_Bool bSetTabs = IsVersion(SWG_TOXTABS);
/*N*/ 		aForm.SetGenerateTabPos(  bSetTabs && (cFlags & 0x20) != 0 );
/*N*/ 		aForm.SetRelTabPos( bSetTabs && (cFlags & 0x40) != 0 );
/*N*/ 
/*N*/ 		sal_uInt8 nPat, nTmpl;
/*N*/ 		*pStrm >> nPat;
            sal_uInt16 i=0;
/*N*/ 		for( i = 0; Good() && i < nPat; i++ )
/*N*/ 		{
/*N*/ 			String aText;
/*N*/ 			InString( *pStrm, aText );
/*N*/ 			if( i < aForm.GetFormMax() )
/*N*/ 			{
/*N*/ 				if( aText.Len() )
/*N*/ 					aText = SwForm::ConvertPatternFrom51(aText, eTOXType);
/*N*/ 				aForm.SetPattern( i, aText );
/*N*/ 			}
/*N*/ 			else if( IsVersion(SWG_LONGIDX) ||
/*N*/ 					 TOX_INDEX != eTOXType ||
/*N*/ 					 i != OLD_MAXLEVEL )
/*N*/ 			{
/*N*/ 				// In aelteren Versionen wurden bei Stichwort-
/*N*/ 				// Verzeichnissen ein Eintrag zu viel geschrieben.
/*?*/ 				Warning();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		*pStrm >> nTmpl;
/*N*/ 		for( i = 0; Good() && i < nTmpl; i++ )
/*N*/ 		{
/*N*/ 			sal_uInt16 nStrIdx;
/*N*/ 			*pStrm >> nStrIdx;
/*N*/ 			if( i < aForm.GetFormMax() )
/*N*/ 			{
/*N*/ 				String sNm( aStringPool.Find( nStrIdx ) );
/*N*/ 				if( !sNm.Len() )
/*N*/ 				{
/*N*/ 					// Bug 37672: falls keiner gefunden wird, nehme
/*N*/ 					//			die defaults
/*N*/ 					sal_uInt16 nPoolIdOffset = 0;
/*N*/ 					switch( cType )
/*N*/ 					{
/*N*/ 						case TOX_INDEX:
/*N*/ 							nPoolIdOffset = RES_POOLCOLL_TOX_IDXH -
/*N*/ 											RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 							break;
/*N*/ 						case TOX_CONTENT:
/*N*/ 							if( 6 > i )
/*N*/ 								nPoolIdOffset = RES_POOLCOLL_TOX_CNTNTH -
/*N*/ 												RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 							else
/*N*/ 								nPoolIdOffset = RES_POOLCOLL_TOX_CNTNT6 - 6 -
/*N*/ 												RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 							break;
/*N*/ 						case TOX_USER:
/*N*/ 							nPoolIdOffset = RES_POOLCOLL_TOX_USERH -
/*N*/ 											RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 							break;
/*N*/ 					}
/*N*/ 
/*N*/ 					sNm = *SwStyleNameMapper::GetRegisterUINameArray()[ nPoolIdOffset + i ];
/*N*/ 				}
/*N*/ 				aForm.SetTemplate( i, sNm );
/*N*/ 			}
/*N*/ 			else if( IsVersion(SWG_LONGIDX) ||
/*N*/ 					 TOX_INDEX != eTOXType ||
/*N*/ 					 i != OLD_MAXLEVEL )
/*N*/ 			{
/*N*/ 				Warning();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		// SetFirstTabPos modifies the patterns
/*N*/ 		// TODO:.is code that OK?
/*N*/ 		if( bSetTabs && (cFlags & 0x10) != 0 )
/*?*/           aForm.SetFirstTabPos( nFirstTabPos );
/*N*/ 		else
/*N*/ 			//fill tab stop positions into the patterns
/*N*/ 			aForm.AdjustTabStops(*pDoc);
/*N*/ 
/*N*/ 		sal_uInt16 nInf;
/*N*/ 		*pStrm >> nInf;
/*N*/ 		Sw3TOXBase *pTOX = new Sw3TOXBase( pTOXType, aForm, nCreateType,
/*N*/ 										   aTitle );
/*N*/ 		switch( cType )
/*N*/ 		{
/*N*/ 			case TOX_INDEX:
/*N*/ 				pTOX->SetOptions( nInf );
/*N*/ 				break;
/*N*/ 			case TOX_CONTENT:
/*N*/ 				if( nInf > MAXLEVEL )
/*N*/ 					nInf = MAXLEVEL;
/*N*/ 				pTOX->SetLevel( nInf );
/*N*/ 				break;
/*N*/ 			case TOX_USER:
/*N*/ 				{
/*N*/ 					String sNm( aStringPool.Find( nInf ) );
/*N*/ 					if( !sNm.Len() )
/*N*/ 						// Bug 37672: falls keiner gefunden wird, nehme
/*N*/ 						//			irgendeinen, also Abbildung
/*N*/ 						sNm = *SwStyleNameMapper::GetExtraUINameArray()[ RES_POOLCOLL_LABEL_ABB
/*N*/ 											- RES_POOLCOLL_EXTRA_BEGIN ];
/*N*/ 					// TODO: Does SetTemplateName what it should do?
/*N*/ 					pTOX->SetTemplateName( sNm );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 		CloseRec( SWG_TOXDESC );
/*N*/ 		if( !Good() )
/*N*/ 			break;
/*N*/ 
/*N*/ 		if( !pTOXs )
/*N*/ 			pTOXs = new Sw3TOXs;
/*N*/ 		pTOXs->C40_INSERT( Sw3TOXBase, pTOX, pTOXs->Count() );
/*N*/ 	}
/*N*/ 	CloseRec( SWG_TOXDESCS51 );
/*N*/ }


/*N*/ void Sw3IoImp::OutTOXs51()
/*N*/ {
/*N*/ 	const SwSectionFmts& rSectFmts = pDoc->GetSections();
/*N*/ 
/*N*/ 	sal_Bool bTOXs = sal_False;
/*N*/ 	for( sal_uInt16 nFmt=0; nFmt<rSectFmts.Count(); nFmt++ )
/*N*/ 	{
/*N*/ 		const SwSectionFmt* pSectFmt = rSectFmts[nFmt];
/*N*/ 
/*N*/ 		// Skip TOXs that are somehow incomplete.
/*N*/ 		const SwSection* pSect = pSectFmt->GetSection();
/*N*/ 		if( !pSect || TOX_CONTENT_SECTION != pSect->GetType() )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwSectionNode *pSectNd = pSectFmt->GetSectionNode();
/*N*/ 		if( !pSectNd )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwTOXBaseSection *pTOXBaseSect =
/*N*/ 			PTR_CAST( SwTOXBaseSection, pSect );
/*N*/ 		if( !pTOXBaseSect )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwTOXType* pType = pTOXBaseSect->GetTOXType();
/*N*/ 		if( !pType )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		if( !bTOXs )
/*N*/ 		{
/*N*/ 			OpenRec( SWG_TOXDESCS51 );
/*N*/ 			bTOXs = sal_True;
/*N*/ 		}
/*N*/ 
/*N*/ 		OpenRec( SWG_TOXDESC );
/*N*/ 
/*N*/ 		// If it is one of the predefined indexes, its name isn't exported.
/*N*/ 		// This way, the index is found in foreign version, too.
/*N*/ 		TOXTypes eType = pType->GetType();
/*N*/ 		TOXTypes eOldType = eType >= TOX_ILLUSTRATIONS ? TOX_USER : eType;
/*N*/ 		const String& rTypeName = pType->GetTypeName();
/*N*/ 		if( IsSw31Or40Export() )
/*N*/ 		{
/*N*/ 			OutString( *pStrm, rTypeName );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// All types above TOX_USER are new in version 5.2. That for,
/*N*/ 			// their names must be written.
/*N*/ 			sal_uInt16 nStrIdx =
/*N*/ 				( eType >= TOX_ILLUSTRATIONS ||
/*N*/ 				  rTypeName != SwTOXBase::GetTOXName(eType) )
/*N*/ 				? aStringPool.Find( rTypeName, USHRT_MAX )
/*N*/ 				: IDX_NO_VALUE;
/*N*/ 			*pStrm << nStrIdx;
/*N*/ 		}
/*N*/ 
/*N*/ 		OutString( *pStrm, pTOXBaseSect->GetTitle() );
/*N*/ 
/*N*/ 		const SwForm& rFrm = pTOXBaseSect->GetTOXForm();
/*N*/ 
/*N*/ 		sal_uInt8 cFlags = 0x03; 	// Anzahl Datenbytes
/*N*/ 		if( !IsSw31Or40Export() )
/*N*/ 		{
/*N*/ 			if( rFrm.IsFirstTabPosFlag() )
/*N*/ 				cFlags += 0x12;
/*N*/ 			if( rFrm.IsGenerateTabPos() )
/*N*/ 				cFlags += 0x20;
/*N*/ 			if( rFrm.IsRelTabPos() )
/*N*/ 				cFlags += 0x40;
/*N*/ 		}
/*N*/ 
/*N*/ 		// TODO: Must some flags be masked out?
/*N*/ 		sal_uInt16 nCreateType = pTOXBaseSect->GetCreateType();
/*N*/ 		if( eType >= TOX_ILLUSTRATIONS )
/*N*/ 			nCreateType |= TOX_TEMPLATE;
/*N*/ 		*pStrm << (sal_uInt8)   cFlags
/*N*/ 			   << (sal_Int16)  nCreateType
/*N*/ 			   << (BYTE)   eOldType;
/*N*/ 		if( (cFlags & 0x10) != 0 )
/*N*/ 			*pStrm << (sal_uInt16)rFrm.GetFirstTabPos();
/*N*/ 
/*N*/ 		// Die SwForm ausgeben
/*N*/ 		// Zaehlen der Patterns und Templates
/*N*/ 		sal_uInt16 nPat = 0, nTmpl = 0;
/*N*/ 		sal_uInt16 nCount = rFrm.GetFormMax();
/*N*/ 		if( IsSw31Or40Export() &&  nCount > OLD_MAXLEVEL+1 )
/*N*/ 			nCount = OLD_MAXLEVEL+1;
/*N*/ 		else if( TOX_AUTHORITIES == eType && nCount > MAXLEVEL+1 )
/*N*/ 			nCount = MAXLEVEL+1;
/*N*/ 
            sal_uInt16 i=0;
/*N*/ 		for( i = nCount; i > 0; i-- )
/*N*/ 		{
/*N*/ 			if( rFrm.GetPattern( i - 1 ).Len() )
/*N*/ 			{
/*N*/ 				nPat = i; break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		for( i = nCount; i > 0; i-- )
/*N*/ 		{
/*N*/ 			if( rFrm.GetTemplate( i - 1 ).Len() )
/*N*/ 			{
/*N*/ 				nTmpl = i; break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// und die Strings der form ausgeben
/*N*/ 		*pStrm << (BYTE) nPat;
/*N*/ 		for( i = 0; i < nPat; i++ )
/*N*/ 		{
/*N*/ 			String aPattern( rFrm.GetPattern(i) );
/*N*/ 			if( aPattern.Len() )
/*N*/ 				aPattern = SwForm::ConvertPatternTo51( aPattern );
/*N*/ 			if( IsSw31Or40Export() && TOX_CONTENT == eType && aPattern.Len() )
/*N*/ 			{
/*N*/ 				xub_StrLen nENPos =
/*N*/ 					aPattern.SearchAscii( SwForm::aFormEntryNum );
/*N*/ 				xub_StrLen nETPos =
/*N*/ 					aPattern.SearchAscii( SwForm::aFormEntryTxt );
/*N*/ 				xub_StrLen nStart = 0, nLen = 0;
/*N*/ 
/*N*/ 				if( nENPos != STRING_NOTFOUND &&
/*N*/ 					(STRING_NOTFOUND == nETPos || nENPos < nETPos) )
/*N*/ 				{
/*N*/ 					nStart = nENPos;
/*N*/ 					nLen = STRING_NOTFOUND == nETPos
/*N*/ 								? SwForm::nFormEntryNumLen
/*N*/ 								: (nETPos - nENPos) + SwForm::nFormEntryTxtLen;
/*N*/ 				}
/*N*/ 				else if( nETPos != STRING_NOTFOUND &&
/*N*/ 						 (STRING_NOTFOUND == nENPos || nETPos < nENPos) )
/*N*/ 				{
/*N*/ 					nStart = nETPos;
/*N*/ 					nLen = STRING_NOTFOUND == nENPos
/*N*/ 								? SwForm::nFormEntryTxtLen
/*N*/ 								: (nENPos - nETPos) + SwForm::nFormEntryNumLen;
/*N*/ 				}
/*N*/ 
/*N*/ 				if( nLen > 0 )
/*N*/ 				{
/*N*/ 					aPattern.Erase( nStart, nLen );
/*N*/ 					aPattern.InsertAscii( SwForm::aFormEntry, nStart );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			OutString( *pStrm, aPattern );
/*N*/ 		}
/*N*/ 		*pStrm << (BYTE) nTmpl;
/*N*/ 		for( i = 0; i < nTmpl; i++ )
/*N*/ 		{
/*N*/ 			const String& rCollNm = rFrm.GetTemplate( i );
/*N*/ 			const SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName( rCollNm );
/*N*/ 			sal_uInt16 nPId = pColl ? pColl->GetPoolFmtId()
/*N*/ 								: SwStyleNameMapper::GetPoolIdFromUIName( rCollNm,GET_POOLID_TXTCOLL );
/*N*/ 			*pStrm << (sal_uInt16) aStringPool.Find( rCollNm, nPId );
/*N*/ 		}
/*N*/ 		// Zuletzt noch die Spezial-Variablen
/*N*/ 		sal_uInt16 nInf = 0;
/*N*/ 		switch( (int) pType->GetType() )
/*N*/ 		{
/*N*/ 			case TOX_INDEX:
/*N*/ 				// TODO: Must some flags be masked out?
/*N*/ 				nInf = pTOXBaseSect->GetOptions();
/*N*/ 				break;
/*N*/ 			case TOX_CONTENT:
/*N*/ 				nInf = pTOXBaseSect->GetLevel();
/*N*/ 				if( IsSw31Or40Export() && nInf > OLD_MAXLEVEL )
/*N*/ 					nInf = OLD_MAXLEVEL;
/*N*/ 				break;
/*N*/ 			case TOX_ILLUSTRATIONS:
/*N*/ 			case TOX_OBJECTS:
/*N*/ 			case TOX_TABLES:
/*N*/ 				{
/*N*/ 					sal_uInt16 nPoolId;
/*N*/ 					switch( pType->GetType() )
/*N*/ 					{
/*?*/ 					case TOX_ILLUSTRATIONS:
/*?*/ 						nPoolId = RES_POOLCOLL_LABEL_ABB;
/*?*/ 						break;
/*?*/ 					case TOX_OBJECTS:
/*?*/ 						nPoolId = RES_POOLCOLL_LABEL_FRAME;
/*?*/ 						break;
/*?*/ 					case TOX_TABLES:
/*?*/ 						nPoolId = RES_POOLCOLL_LABEL_TABLE;
/*?*/ 						break;
/*N*/ 					}
/*N*/ 					String aName;
/*N*/ 					SwStyleNameMapper::FillUIName( nPoolId, aName );
/*N*/ 					nInf = aStringPool.Find( aName, nPoolId );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case TOX_USER:
/*N*/ 			case TOX_AUTHORITIES:
/*N*/ 			default:
/*N*/ 				{
/*N*/ 					const String sCollNm = pTOXBaseSect->GetTemplateName();
/*N*/ 					if( sCollNm.Len() )
/*N*/ 					{
/*N*/ 						const SwTxtFmtColl* pColl =
/*N*/ 							pDoc->FindTxtFmtCollByName( sCollNm );
/*N*/ 						sal_uInt16 nPId = pColl ? pColl->GetPoolFmtId()
/*N*/ 											: SwStyleNameMapper::GetPoolIdFromUIName( sCollNm,
/*N*/ 														GET_POOLID_TXTCOLL );
/*N*/ 						nInf = aStringPool.Find( sCollNm, nPId );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						nInf = IDX_NO_VALUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 		}
/*N*/ 		*pStrm << (sal_uInt16) nInf;
/*N*/ 
/*N*/ 		CloseRec( SWG_TOXDESC );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bTOXs )
/*N*/ 		CloseRec( SWG_TOXDESCS51 );
/*N*/ }

extern void lcl_sw3io_FillSetExpFieldName( Sw3IoImp& rIo, sal_uInt16 nStrId,
                                           String& rName );

/*N*/ void Sw3IoImp::InTOXs()
/*N*/ {
/*N*/ 	OpenRec( SWG_TOXDESCS );
/*N*/ 	// Die Mark erst einmal an den Anfang setzen
/*N*/ 	SwPosition aPos( pDoc->GetNodes().GetEndOfPostIts() );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_TOXDESC );
/*N*/ 
/*N*/ 		sal_uInt8 cFlags = OpenFlagRec();
/*N*/ 
/*N*/ 		sal_uInt16 nType, nCreateType, nCaptionDisplay, nStrIdx, nSeqStrIdx, nData;
/*N*/ 		sal_uInt16 nOLEOptions = 0;
/*N*/ 		sal_uInt8 cFormFlags;
/*N*/ 
/*N*/ 		String aTitle, aName, aDummy;
/*N*/ 
/*N*/ 		*pStrm  >> nType
/*N*/ 				>> nCreateType
/*N*/ 				>> nCaptionDisplay
/*N*/ 				>> nStrIdx
/*N*/ 				>> nSeqStrIdx
/*N*/ 				>> nData
/*N*/ 				>> cFormFlags;
/*N*/ 		CloseFlagRec();
/*N*/ 
/*N*/ 		sal_uInt16 nMainStyleIdx = IDX_NO_VALUE;
/*N*/ 		InString( *pStrm, aName );
/*N*/ 		InString( *pStrm, aTitle );
/*N*/ 		if( nVersion < SWG_NEWTOX2 )
/*N*/ 			InString( *pStrm, aDummy );		// was AutoMarkURL
/*N*/ 		if( IsVersion(SWG_NEWTOX2) )
/*N*/ 		{
/*N*/ 			*pStrm	>> nOLEOptions
/*N*/ 					>> nMainStyleIdx;
/*N*/ 		}
/*N*/ 
/*N*/ 		TOXTypes eTOXType = (TOXTypes)nType;
/*N*/ 		String aTypeName;
/*N*/ 		if( IDX_NO_VALUE != nStrIdx )
/*?*/ 			aTypeName = aStringPool.Find( nStrIdx );
/*N*/ 		else
/*N*/ 			aTypeName = SwTOXBase::GetTOXName( eTOXType );
/*N*/ 
/*N*/ 		// Search TOX type
/*N*/ 		sal_uInt16 nTOXTypes = pDoc->GetTOXTypeCount( eTOXType );
/*N*/ 		const SwTOXType* pTOXType = NULL;
            sal_uInt16 i=0;
/*N*/ 		for( i=0; i<nTOXTypes; i++ )
/*N*/ 		{
/*N*/ 			const SwTOXType* pTmp = pDoc->GetTOXType( eTOXType, i );
/*N*/ 			if( pTmp->GetTypeName() == aTypeName )
/*N*/ 			{
/*N*/ 				pTOXType = pTmp;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// If the TOX type could not be found, it is inserted now.
/*N*/ 		if( !pTOXType )
/*N*/ 		{
/*?*/ 			pDoc->InsertTOXType( SwTOXType ( eTOXType, aTypeName ) );
/*?*/ 			pTOXType = pDoc->GetTOXType( eTOXType, nTOXTypes );
/*N*/ 		}
/*N*/ 
/*N*/ 		// form patterns
/*N*/ 		SwForm aForm( eTOXType );
/*N*/ 
/*N*/ 		aForm.SetGenerateTabPos(  (cFormFlags & 0x01) != 0 );
/*N*/ 		aForm.SetRelTabPos( (cFormFlags & 0x02) != 0 );
/*N*/ 		aForm.SetCommaSeparated( (cFormFlags & 0x04) != 0 );
/*N*/ 
/*N*/ 		sal_uInt8 nPatterns;
/*N*/ 		*pStrm >> nPatterns;
/*N*/ 		for( i = 0; Good() && i < nPatterns; i++ )
/*N*/ 		{
/*N*/ 			OpenRec( SWG_FORMPATTERN_LCL );
/*N*/ 
/*N*/ 			sal_uInt8 nLevel;
/*N*/ 			sal_uInt16 nTokens;
/*N*/ 
/*N*/ 			OpenFlagRec();
/*N*/ 			*pStrm	>> nLevel
/*N*/ 					>> nTokens;
/*N*/ 			CloseFlagRec();
/*N*/ 
/*N*/ 			if( nLevel < aForm.GetFormMax() )
/*N*/ 			{
/*N*/ 				String aPattern;
/*N*/ 				for( sal_uInt16 j=0; Good() && j < nTokens; j++  )
/*N*/ 				{
/*N*/ 					OpenRec( SWG_FORMTOKEN_LCL );
/*N*/ 
/*N*/ 					sal_uInt16 nType, nStrIdx;
/*N*/ 
/*N*/ 					OpenFlagRec();
/*N*/ 					*pStrm	>> nType
/*N*/ 							>> nStrIdx;
/*N*/ 					CloseFlagRec();
/*N*/ 
/*N*/ 					if( nType < TOKEN_END )
/*N*/ 					{
/*N*/                         //#92986# some older versions created a
/*N*/                         //wrong content index entry definition
/*N*/                         if(TOKEN_ENTRY == nType && eTOXType == TOX_CONTENT)
/*N*/                             nType = TOKEN_ENTRY_TEXT;
/*N*/                         SwFormToken aToken( (FormTokenType)nType );
/*N*/ 						if( IDX_NO_VALUE != nStrIdx )
/*N*/ 						{
/*?*/ 							aToken.sCharStyleName = aStringPool.Find( nStrIdx );
/*?*/ 							aToken.nPoolId = aStringPool.FindPoolId( nStrIdx );
/*N*/ 						}
/*N*/ 						switch( aToken.eTokenType )
/*N*/ 						{
/*N*/ 						case TOKEN_TAB_STOP:
/*N*/ 							{
/*N*/ 								sal_Int32 nTabStopPosition;
/*N*/ 								sal_uInt16 nTabAlign;
/*N*/ 								*pStrm	>> nTabStopPosition
/*N*/ 										>> nTabAlign;
/*N*/ 								aToken.nTabStopPosition = nTabStopPosition;
/*N*/ 								aToken.eTabAlign = nTabAlign;
/*N*/ 								if( IsVersion(SWG_TOXTABCHAR) )
/*N*/ 								{
/*N*/ 									sal_uInt8 cFillChar;
/*N*/ 									*pStrm	>> cFillChar;
/*N*/ 									aToken.cTabFillChar =
/*N*/ 										ByteString::ConvertToUnicode( cFillChar,
/*N*/ 																	  eSrcSet );
/*N*/ 								}
/*N*/ 							}
/*N*/ 							break;
/*?*/ 						case TOKEN_CHAPTER_INFO:
/*?*/ 							{
/*?*/ 								sal_uInt16 nChapterFormat;
/*?*/ 								*pStrm	>> nChapterFormat;
/*?*/ 								aToken.nChapterFormat = nChapterFormat;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case TOKEN_TEXT:
/*?*/ 							InString( *pStrm, aToken.sText );
/*?*/ 							break;
/*?*/ 						case TOKEN_AUTHORITY:
/*?*/ 							{
/*?*/ 								sal_uInt16 nAuthorityField;
/*?*/ 								*pStrm  >> nAuthorityField;
/*?*/ 								aToken.nAuthorityField = nAuthorityField;
/*?*/ 							}
/*?*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 						aPattern += aToken.GetString();
/*N*/ 					}
/*N*/ 					else
/*?*/ 						Warning();
/*N*/ 
/*N*/ 					CloseRec( SWG_FORMTOKEN_LCL );
/*N*/ 				}
/*N*/ 
/*N*/ 				aForm.SetPattern( nLevel, aPattern );
/*N*/ 			}
/*N*/ 			else
/*?*/ 				Warning();
/*N*/ 
/*N*/ 			CloseRec( SWG_FORMPATTERN_LCL );
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_uInt8 nTemplates;
/*N*/ 		*pStrm >> nTemplates;
/*N*/ 		for( i = 0; Good() && i < nTemplates; i++ )
/*N*/ 		{
/*N*/ 			sal_uInt16 nStrIdx;
/*N*/ 			*pStrm >> nStrIdx;
/*N*/ 			if( i < aForm.GetFormMax() )
/*N*/ 			{
/*N*/ 				String sNm( aStringPool.Find( nStrIdx ) );
/*N*/ 				ASSERT( sNm.Len(), "Template name not found" );
/*N*/ 				if( !sNm.Len() )
/*N*/ 				{
/*N*/ 					// #37672#: if the style has not been found, use the default
/*?*/ 					sal_uInt16 nPoolIdOffset = 0;
/*?*/ 					sal_uInt16 nOffs = i;
/*?*/ 					switch( eTOXType )
/*?*/ 					{
/*?*/ 					case TOX_INDEX:
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_IDXH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_CONTENT:
/*?*/ 						if( 6 > i )
/*?*/ 							nPoolIdOffset = RES_POOLCOLL_TOX_CNTNTH -
/*?*/ 											RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						else
/*?*/ 							nPoolIdOffset = RES_POOLCOLL_TOX_CNTNT6 - 6 -
/*?*/ 											RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_USER:
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_USERH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_ILLUSTRATIONS:
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_ILLUSH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_OBJECTS:
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_OBJECTH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_TABLES:
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_TABLESH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						break;
/*?*/ 					case TOX_AUTHORITIES:
/*?*/ 						// There is one text collection for all levels only!
/*?*/ 						nPoolIdOffset = RES_POOLCOLL_TOX_AUTHORITIESH -
/*?*/ 										RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 						if( nOffs > 1 )
/*?*/ 							nOffs = 1;
/*?*/ 						break;
/*?*/ 					}
/*?*/ 
/*?*/ 					sNm = *SwStyleNameMapper::GetRegisterUINameArray()[ nPoolIdOffset + nOffs ];
/*N*/ 				}
/*N*/ 				aForm.SetTemplate( i, sNm );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				Warning();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		Sw3TOXBase *pTOX = new Sw3TOXBase( pTOXType, aForm, nCreateType,
/*N*/ 										   aTitle );
/*N*/ 		pTOX->SetTOXName( aName );
/*N*/ 		pTOX->SetCaptionDisplay( (SwCaptionDisplay)nCaptionDisplay );
/*N*/ 		pTOX->SetOLEOptions( nOLEOptions );
/*N*/ 
/*N*/ 		pTOX->SetProtected( (cFlags & 0x10) != 0 );
/*N*/ 		pTOX->SetFromChapter( (cFlags & 0x20) != 0 );
/*N*/ 		pTOX->SetFromObjectNames( (cFlags & 0x40) != 0 );
/*N*/ 		pTOX->SetLevelFromChapter( (cFlags & 0x80) != 0 );
/*N*/ 		if( IDX_NO_VALUE != nMainStyleIdx )
/*?*/ 			pTOX->SetMainEntryCharStyle( aStringPool.Find( nMainStyleIdx ) );
/*N*/ 		if( IDX_NO_VALUE != nSeqStrIdx )
/*N*/ 		{
/*?*/ 			String aSequenceName;
/*?*/ 			lcl_sw3io_FillSetExpFieldName( *this, nSeqStrIdx, aSequenceName );
/*?*/ 			pTOX->SetSequenceName( aSequenceName );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( TOX_INDEX == eTOXType )
/*N*/ 			pTOX->SetOptions( nData );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nData > MAXLEVEL )
/*N*/ 				nData = MAXLEVEL;
/*N*/ 			pTOX->SetLevel( nData );
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_uInt8 nStyleNames;
/*N*/ 		*pStrm >> nStyleNames;
/*N*/ 		for( i=0; i < nStyleNames; i++ )
/*N*/ 		{
/*?*/ 			sal_uInt8 nLevel;
/*?*/ 			sal_uInt16 nCount;
/*?*/ 			*pStrm	>> nLevel
/*?*/ 					>> nCount;
/*?*/ 
/*?*/ 			String aStyleNames;
/*?*/ 			for( sal_uInt16 j=0; j<nCount; j++ )
/*?*/ 			{
/*?*/ 				String aName;
/*?*/ 				*pStrm >> nStrIdx;
/*?*/ 				if( IDX_NO_VALUE != nStrIdx )
/*?*/ 				{
/*?*/ 					aName = aStringPool.Find( nStrIdx );
/*?*/ 
/*?*/ 					if( aStyleNames.Len() )
/*?*/ 						aStyleNames += TOX_STYLE_DELIMITER;
/*?*/ 					aStyleNames += aName;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			if( nLevel < aForm.GetFormMax() )
/*?*/ 				pTOX->SetStyleNames( aStyleNames, nLevel );
/*?*/ 			else
/*?*/ 				Warning();
/*N*/ 		}
/*N*/ 
/*N*/ 		// #37672#: every user index must have a style name.
/*N*/ 		if( TOX_USER == eTOXType && !pTOX->GetStyleNames(0).Len() )
/*N*/ 		{
/*N*/ 			String aName = *SwStyleNameMapper::GetExtraUINameArray()[ RES_POOLCOLL_LABEL_ABB
/*N*/ 											- RES_POOLCOLL_EXTRA_BEGIN ];
/*?*/ 			pTOX->SetStyleNames( aName, 0 );
/*N*/ 		}
/*N*/ 
/*N*/ 		cFlags = OpenFlagRec();
/*N*/ 		sal_uInt16 nSectStrIdx;
/*N*/ 		*pStrm >> nSectStrIdx;
/*N*/ 		pTOX->SetSectFmtStrIdx( nSectStrIdx );
/*N*/ 		if( (cFlags & 0x10) != 0 )
/*N*/ 		{
/*N*/ 			sal_uInt32 nTitleLen;
/*N*/ 			*pStrm	>> nTitleLen;
/*N*/ 			pTOX->SetTitleLen( nTitleLen );
/*N*/ 		}
/*N*/ 		CloseFlagRec();
/*N*/ 
/*N*/ 		if( SWG_SECTFMT == Peek() )
/*N*/ 			pTOX->SetSectFmt( (SwSectionFmt*)InFormat( SWG_SECTFMT, NULL ) );
/*N*/ 
/*N*/ 		if( (cFlags & 0x10) != 0 && SWG_SECTFMT == Peek() )
/*N*/ 			pTOX->SetTitleSectFmt( (SwSectionFmt*)InFormat(SWG_SECTFMT,NULL) );
/*N*/ 
/*N*/ 		CloseRec( SWG_TOXDESC );
/*N*/ 
/*N*/ 		if( !Good() )
/*N*/ 			break;
/*N*/ 
/*N*/ 		if( !pTOXs )
/*N*/ 			pTOXs = new Sw3TOXs;
/*N*/ 		pTOXs->C40_INSERT( Sw3TOXBase, pTOX, pTOXs->Count() );
/*N*/ 	}
/*N*/ 	CloseRec( SWG_TOXDESCS );
/*N*/ }

extern sal_uInt16 lcl_sw3io_GetSetExpFieldPoolId( const String& rName );

/*N*/ void Sw3IoImp::OutTOXs()
/*N*/ {
/*N*/ 	const SwSectionFmts& rSectFmts = pDoc->GetSections();
/*N*/ 
/*N*/ 	sal_Bool bTOXs = sal_False;
/*N*/ 	for( sal_uInt16 nFmt=0; nFmt<rSectFmts.Count(); nFmt++ )
/*N*/ 	{
/*N*/ 		const SwSectionFmt* pSectFmt = rSectFmts[nFmt];
/*N*/ 
/*N*/ 		// Skip TOXs that are somehow incomplete.
/*N*/ 		const SwSection* pSect = pSectFmt->GetSection();
/*N*/ 		if( !pSect || TOX_CONTENT_SECTION != pSect->GetType() )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwSectionNode *pSectNd = pSectFmt->GetSectionNode();
/*N*/ 		if( !pSectNd )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwTOXBaseSection *pTOXBaseSect =
/*N*/ 			PTR_CAST( SwTOXBaseSection, pSect );
/*N*/ 		if( !pTOXBaseSect )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwTOXType* pType = pTOXBaseSect->GetTOXType();
/*N*/ 		if( !pType )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		if( !bTOXs )
/*N*/ 		{
/*N*/ 			OpenRec( SWG_TOXDESCS );
/*N*/ 			bTOXs = sal_True;
/*N*/ 		}
/*N*/ 
/*N*/ 		OpenRec( SWG_TOXDESC );
/*N*/ 
/*N*/ 
/*N*/ 		sal_uInt8 cFlags = 0x0d; 	// Anzahl Datenbytes
/*N*/ 		if( pTOXBaseSect->IsProtected() )
/*N*/ 			cFlags += 0x10;
/*N*/ 		if( pTOXBaseSect->IsFromChapter() )
/*N*/ 			cFlags += 0x20;
/*N*/ 		if( pTOXBaseSect->IsFromObjectNames() )
/*N*/ 			cFlags += 0x40;
/*N*/ 		if( pTOXBaseSect->IsLevelFromChapter() )
/*N*/ 			cFlags += 0x80;
/*N*/ 
/*N*/ 		TOXTypes eType = pType->GetType();
/*N*/ 
/*N*/ 		const SwForm& rForm = pTOXBaseSect->GetTOXForm();
/*N*/ 		sal_uInt8 cFormFlags = 0x00;
/*N*/ 		if( rForm.IsGenerateTabPos() )
/*N*/ 			cFormFlags += 0x01;
/*N*/ 		if( rForm.IsRelTabPos() )
/*N*/ 			cFormFlags += 0x02;
/*N*/ 		if( rForm.IsCommaSeparated() )
/*N*/ 			cFormFlags += 0x04;
/*N*/ 
/*N*/ 		sal_uInt16 nSeqStrIdx = IDX_NO_VALUE;
/*N*/ 		const String& rSequenceName = pTOXBaseSect->GetSequenceName();
/*N*/ 		if( rSequenceName.Len() )
/*N*/ 		{
/*?*/ 			SwFieldType *pFldType =
/*?*/ 				pDoc->GetFldType( RES_SETEXPFLD, rSequenceName );
/*?*/ 			ASSERT( pFldType, "sequence field type not found" );
/*?*/ 			if( pFldType )
/*?*/ 			{
/*?*/ 				sal_uInt16 nPoolId = lcl_sw3io_GetSetExpFieldPoolId(rSequenceName);
/*?*/ 				nSeqStrIdx = aStringPool.Find( rSequenceName, nPoolId );
/*?*/ 				ASSERT( IDX_NO_VALUE != nSeqStrIdx,
/*?*/ 						"no string pool entry found for sequence field" );
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// If it is one of the predefined indexes, its name isn't exported.
/*N*/ 		// This way, the index is found in foreign version, too.
/*N*/ 		const String& rTypeName = pType->GetTypeName();
/*N*/ 		sal_uInt16 nStrIdx = rTypeName != SwTOXBase::GetTOXName(eType)
/*N*/ 				? aStringPool.Find( rTypeName, USHRT_MAX )
/*N*/ 				: IDX_NO_VALUE;
/*N*/ 
/*N*/ 		// Options or Level;
/*N*/ 		sal_uInt16 nData = 0;
/*N*/ 		if( TOX_INDEX == eType )
/*N*/ 			nData = pTOXBaseSect->GetOptions();
/*N*/ 		else
/*N*/ 			nData = pTOXBaseSect->GetLevel();
/*N*/ 
/*N*/ 		*pStrm  << cFlags
/*N*/ 				<< (sal_uInt16)  eType
/*N*/ 				<< (sal_uInt16)  pTOXBaseSect->GetCreateType()
/*N*/ 				<< (sal_uInt16)  pTOXBaseSect->GetCaptionDisplay()
/*N*/ 				<< nStrIdx
/*N*/ 				<< nSeqStrIdx
/*N*/ 				<< nData
/*N*/ 				<< cFormFlags;
/*N*/ 
/*N*/ 		sal_uInt16 nMainStyleIdx = IDX_NO_VALUE;
/*N*/ 		const String& rMainStyle = pTOXBaseSect->GetMainEntryCharStyle();
/*N*/ 		if( rMainStyle.Len() )
/*N*/ 		{
/*?*/ 			sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( rMainStyle, GET_POOLID_CHRFMT );
/*?*/ 			nMainStyleIdx = aStringPool.Find( rMainStyle, nPoolId );
/*N*/ 		}
/*N*/ 
/*N*/ 		OutString( *pStrm, pTOXBaseSect->GetTOXName() );
/*N*/ 		OutString( *pStrm, pTOXBaseSect->GetTitle() );
/*N*/ 		*pStrm << (sal_uInt16)	pTOXBaseSect->GetOLEOptions()
/*N*/ 			   << (sal_uInt16) nMainStyleIdx;
/*N*/ 
/*N*/ 		// form patterns
/*N*/ 		sal_uInt8 nPatterns = 0, i;
/*N*/ 		for( i = (sal_uInt8)rForm.GetFormMax(); i > 0; i-- )
/*N*/ 		{
/*N*/ 			if( rForm.GetPattern( i-1 ).Len() )
/*N*/ 			{
/*N*/ 				nPatterns = i;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		*pStrm << (sal_uInt8)nPatterns;
/*N*/ 		for( i = 0; i < nPatterns; i++ )
/*N*/ 		{
/*N*/ 			OpenRec( SWG_FORMPATTERN_LCL );
/*N*/ 			*pStrm  << (sal_uInt8)0x03	// flag byte
/*N*/ 					<< (sal_uInt8)i;
/*N*/ 
/*N*/ 			sal_uInt16 nToken = 0;
/*N*/ 			const String& rPattern = rForm.GetPattern( i );
/*N*/ 			if( rPattern.Len() )
/*N*/ 			{
/*N*/ 				OpenValuePos16( nToken );
/*N*/ 
/*N*/ 				SwFormTokenEnumerator aEnum( rPattern );
/*N*/ 				while( aEnum.HasNextToken() )
/*N*/ 				{
/*N*/ 					SwFormToken aToken( aEnum.GetNextToken() );
/*N*/ 
/*N*/ 					OpenRec( SWG_FORMTOKEN_LCL );
/*N*/ 					sal_uInt16 nStrIdx = IDX_NO_VALUE;
/*N*/ 					if( aToken.sCharStyleName.Len() )
/*N*/ 						nStrIdx = aStringPool.Find( aToken.sCharStyleName,
/*N*/ 													aToken.nPoolId );
/*N*/ 					*pStrm	<< (sal_uInt8)0x04
/*N*/ 							<< (sal_uInt16)aToken.eTokenType
/*N*/ 							<< nStrIdx;
/*N*/ 					switch( aToken.eTokenType )
/*N*/ 					{
/*N*/ 					case TOKEN_TAB_STOP:
/*N*/ 						*pStrm	<< (sal_Int32)aToken.nTabStopPosition
/*N*/ 								<< (sal_uInt16)aToken.eTabAlign
/*N*/ 							 	<< (sal_uInt8)aToken.cTabFillChar;
/*N*/ 						break;
/*?*/ 					case TOKEN_CHAPTER_INFO:
/*?*/ 						*pStrm	<< (sal_uInt16)aToken.nChapterFormat;
/*?*/ 						break;
/*?*/ 					case TOKEN_TEXT:
/*?*/ 						OutString( *pStrm, aToken.sText );
/*?*/ 						break;
/*?*/ 					case TOKEN_AUTHORITY:
/*?*/ 						*pStrm  << (sal_uInt16)aToken.nAuthorityField;
/*?*/ 						break;
/*N*/ 					}
/*N*/ 
/*N*/ 					CloseRec( SWG_FORMTOKEN_LCL );
/*N*/ 
/*N*/ 					nToken++;
/*N*/ 				}
/*N*/ 
/*N*/ 				CloseValuePos16( nToken );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				*pStrm << (sal_uInt16)0;
/*N*/ 			}
/*N*/ 
/*N*/ 			CloseRec( SWG_FORMPATTERN_LCL );
/*N*/ 		}
/*N*/ 
/*N*/ 		// form templates
/*N*/ 		sal_uInt8 nTemplates = 0;
/*N*/ 		for( i = (sal_uInt8)rForm.GetFormMax(); i > 0; i-- )
/*N*/ 		{
/*N*/ 			if( rForm.GetTemplate( i-1 ).Len() )
/*N*/ 			{
/*N*/ 				nTemplates = i;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		*pStrm << (sal_uInt8) nTemplates;
/*N*/ 		for( i = 0; i < nTemplates; i++ )
/*N*/ 		{
/*N*/ 			const String& rCollNm = rForm.GetTemplate( i );
/*N*/ 			const SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName( rCollNm );
/*N*/ 			sal_uInt16 nPId = pColl ? pColl->GetPoolFmtId()
/*N*/ 								: SwStyleNameMapper::GetPoolIdFromUIName( rCollNm,GET_POOLID_TXTCOLL );
/*N*/ 			*pStrm << (sal_uInt16) aStringPool.Find( rCollNm, nPId );
/*N*/ 		}
/*N*/ 
/*N*/ 		// style names
/*N*/ 		sal_uInt8 nStyleNames = 0;
/*N*/ 		for( i=0; i<MAXLEVEL; i++ )
/*N*/ 		{
/*N*/ 			if( pTOXBaseSect->GetStyleNames(i).Len() )
/*N*/ 				nStyleNames++;
/*N*/ 		}
/*N*/ 
/*N*/ 		*pStrm << nStyleNames;
/*N*/ 
/*N*/ 		for( i=0; i<MAXLEVEL; i++ )
/*N*/ 		{
/*N*/ 			const String& rStyleNames = pTOXBaseSect->GetStyleNames(i);
/*N*/ 			if( rStyleNames.Len() )
/*N*/ 			{
/*?*/ 				*pStrm	<< (sal_uInt8)i
/*?*/ 						<< (sal_uInt16)rStyleNames.GetTokenCount(
/*?*/ 														TOX_STYLE_DELIMITER );
/*?*/ 
/*?*/ 				xub_StrLen nStrPos = 0;
/*?*/ 				while( nStrPos != STRING_NOTFOUND )
/*?*/ 				{
/*?*/ 					String aName =
/*?*/ 						rStyleNames.GetToken( 0, TOX_STYLE_DELIMITER, nStrPos );
/*?*/ 					ASSERT( aName.Len(), "empty style name" );
/*?*/ 					sal_uInt16 nStrIdx = IDX_NO_VALUE;
/*?*/ 					if( aName.Len() )
/*?*/ 					{
/*?*/ 						const SwTxtFmtColl *pColl =
/*?*/ 							pDoc->FindTxtFmtCollByName( aName );
/*?*/ 						sal_uInt16 nPoolId = pColl
/*?*/ 							? pColl->GetPoolFmtId()
/*?*/ 							: SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_TXTCOLL );
/*?*/ 						nStrIdx = aStringPool.Find( aName, nPoolId );
/*?*/ 					}
/*?*/ 
/*N*/ 					*pStrm	<< nStrIdx;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// the following stuff is required to export the TOX sections as
/*N*/ 		// TOX range.
/*N*/ 		cFlags = 0x02;
/*N*/ 		sal_uInt32 nTitleLen = 0UL;
/*N*/ 		sal_uInt16 nSectStrIdx = aStringPool.Find( pSectFmt->GetName(),
/*N*/ 											   pSectFmt->GetPoolFmtId() );
/*N*/ 		const SwSectionFmt *pTitleSectFmt = 0;
/*N*/ 
/*N*/ 		ULONG nNextNdIdx = pSectNd->GetIndex() + 1;
/*N*/ 		const SwSectionNode *pNextSectNd =
/*N*/ 			pDoc->GetNodes()[nNextNdIdx]->GetSectionNode();
/*N*/ 			if( pNextSectNd &&
/*N*/ 				TOX_HEADER_SECTION == pNextSectNd->GetSection().GetType() )
/*N*/ 		{
/*N*/ 			cFlags += 0x14;
/*N*/ 			nTitleLen = pNextSectNd->EndOfSectionIndex() - nNextNdIdx - 1;
/*N*/ 			pTitleSectFmt = pNextSectNd->GetSection().GetFmt();
/*N*/ 		}
/*N*/ 
/*N*/ 		*pStrm	<< cFlags
/*N*/ 				<< nSectStrIdx;
/*N*/ 		if( pTitleSectFmt )
/*N*/ 			*pStrm	<< nTitleLen;
/*N*/ 
/*N*/ 		OutFormat( SWG_SECTFMT, *pSectFmt );
/*N*/ 		if( pTitleSectFmt )
/*N*/ 			OutFormat( SWG_SECTFMT, *pTitleSectFmt );
/*N*/ 
/*N*/ 		CloseRec( SWG_TOXDESC );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bTOXs )
/*N*/ 		CloseRec( SWG_TOXDESCS );
/*N*/ }

/*N*/ void Sw3IoImp::ConnectTOXs()
/*N*/ {
/*N*/ 	if( pTOXs )
/*N*/ 	{
/*N*/ 		for( sal_uInt16 i=0; i < pTOXs->Count(); i++ )
/*N*/ 		{
/*N*/ 			// #67763# If, for some reason, a TOX's point or mark have not been
/*N*/ 			// read, the TOX has to be skipped.
/*N*/ 			Sw3TOXBase *pTOX = (*pTOXs)[i];
/*N*/ 			ASSERT( pTOX->GetStartNodeIdx() && pTOX->GetEndNodeIdx(),
/*N*/ 					"missing TOX index" );
/*N*/ 			if( !(pTOX->GetStartNodeIdx() && pTOX->GetEndNodeIdx()) )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			ASSERT( pTOX->GetStartNodeIdx()->GetIndex() <=
/*N*/ 					pTOX->GetEndNodeIdx()->GetIndex(),
/*N*/ 					"wrong TOX index order" );
/*N*/ 			if( pTOX->GetStartNodeIdx()->GetIndex() >
/*N*/ 										pTOX->GetEndNodeIdx()->GetIndex() )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			// Ensure that start and end indices are within the same section.
/*N*/ 			// The indices are at content nodes, because they are read at
/*N*/ 			// text nodes only.
/*N*/ 			if( pTOX->GetStartNodeIdx()->GetNode().FindStartNode() !=
/*N*/ 			  	pTOX->GetEndNodeIdx()->GetNode().FindStartNode() )
/*N*/ 			{
/*N*/ 				ASSERT( !this, "TOX indices are not within the same section" );
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 
/*N*/ 			// Make TOX name unique.
/*N*/ 			const String& rTOXName = pTOX->GetTOXName();
/*N*/ 			if( !rTOXName.Len() || bInsert )
/*N*/ 			{
/*N*/ 				const String *pOldTOXName = rTOXName.Len() ? &rTOXName : 0;
/*N*/ 				String aTOXName(
/*N*/ 					pDoc->GetUniqueTOXBaseName( *pTOX->GetTOXType(),
/*N*/ 												pOldTOXName ) );
/*N*/ 				pTOX->SetTOXName( aTOXName );
/*N*/ 			}
/*N*/ 
/*N*/ 			// Insert tox. This inserts a section that has the name of the
/*N*/ 			// tox type.
/*N*/ 			sal_uInt32 nStartNdIdx = pTOX->GetStartNodeIdx()->GetIndex();
/*N*/ 			sal_uInt32 nEndNdIdx = pTOX->GetEndNodeIdx()->GetIndex();
/*N*/ 			const SwTOXBaseSection *pSect =
/*N*/ 				pDoc->InsertTableOf( nStartNdIdx, nEndNdIdx, *pTOX );
/*N*/ 			ASSERT( pSect, "insertion of TOX failed" );
/*N*/ 			if( !pSect )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			ASSERT( pSect->GetName() == pTOX->GetTOXName(),
/*N*/ 					"tox has wrong name" );
/*N*/ 			ASSERT( pSect->GetFmt()->GetSectionNode()->GetIndex() ==
/*N*/ 					nStartNdIdx,
/*N*/ 					"unexpected tox section start position" );
/*N*/ 
/*N*/ 			// Copy section format attributes to the new section format.
/*N*/ 			// In addition, the section format has to be renamed, because
/*N*/ 			// loading the layout requires the frame have the original name.
/*N*/ 			SwSectionFmt *pTOXSectFmt = pTOX->GetSectFmt();
/*N*/ 			if( pTOXSectFmt )
/*N*/ 			{
/*N*/ 				SwSectionFmt *pSectFmt = pSect->GetFmt();
/*N*/ 				const String& rName = pTOXSectFmt->GetName();
/*N*/ 				pSectFmt->SetName( rName );
/*N*/ 				pSectFmt->SetAttr( pTOXSectFmt->GetAttrSet() );
/*N*/ 
/*N*/ 				// If the format has been cached already, the cache entry has
/*N*/ 				// to be replaced, too.
/*N*/ 				sal_uInt16 nStrIdx = pTOX->GetSectFmtStrIdx();
/*N*/ 				ASSERT( IDX_NO_VALUE != nStrIdx, "string pool index missing" );
/*N*/ 				if( IDX_NO_VALUE != nStrIdx )
/*N*/ 				{
/*N*/ #ifdef DBG_UTIL
/*N*/ 					SwFmt *pTmp = aStringPool.FindCachedFmt( nStrIdx );
/*N*/ 					ASSERT( !pTmp || pTmp == (SwFmt *)pTOXSectFmt,
/*N*/ 							"wrong format cached?" );
/*N*/ 
/*N*/ #endif
/*N*/ 					aStringPool.SetCachedFmt( nStrIdx, pSectFmt );
/*N*/ 				}
/*N*/ 
/*N*/ 				// The loaded format isn't required any longer.
/*N*/ 				pDoc->DelSectionFmt( pTOXSectFmt, sal_False );
/*N*/ 				pTOX->SetSectFmt( 0 );
/*N*/ 			}
/*N*/ 
/*N*/ 			sal_uInt32 nTitleLen = pTOX->GetTitleLen();
/*N*/ 			ASSERT( !nTitleLen || pTOX->GetTitleSectFmt(),
/*N*/ 					"missing tox title section format" );
/*N*/ 			if( nTitleLen > 0 && pTOX->GetTitleSectFmt() )
/*N*/ 			{
/*N*/ 				SwNodeIndex aEndNdIdx( *pTOX->GetStartNodeIdx(), nTitleLen-1 );
/*N*/ 
/*N*/ 				ASSERT( pTOX->GetStartNodeIdx()->GetNode().IsCntntNode(),
/*N*/ 						"tox title section start is not a content node" );
/*N*/ 				ASSERT( aEndNdIdx.GetNode().IsCntntNode(),
/*N*/ 						"tox title section end is not a content node" );
/*N*/ 				ASSERT( pTOX->GetStartNodeIdx()->GetIndex() <=
/*N*/ 						aEndNdIdx.GetIndex(),
/*N*/ 						"invalid TOX title section end" );
/*N*/ 
/*N*/ 				// insert title section
/*N*/ 				if( pTOX->GetStartNodeIdx()->GetNode().IsCntntNode() &&
/*N*/ 					aEndNdIdx.GetNode().IsCntntNode() &&
/*N*/ 					pTOX->GetStartNodeIdx()->GetIndex()<=aEndNdIdx.GetIndex() )
/*N*/ 				{
/*N*/ 					String aSectName( pTOX->GetTOXName() );
/*N*/ 					aSectName.AppendAscii( "_Head" );
/*N*/ 					SwSection aSection( TOX_HEADER_SECTION, aSectName );
/*N*/ 
/*N*/ 					pDoc->GetNodes().InsertSection( *pTOX->GetStartNodeIdx(),
/*N*/ 							*pTOX->GetTitleSectFmt(), aSection, &aEndNdIdx,
/*N*/ 							sal_False );
/*N*/ 
/*N*/ 					// The section format's owner now is the section
/*N*/ 					pTOX->SetTitleSectFmt( 0 );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		delete pTOXs;
/*N*/ 		pTOXs = 0;
/*N*/ 	}
/*N*/ }

// Nicht benutzte Markierungen loeschen

/*N*/ void Sw3IoImp::CleanupMarks()
/*N*/ {
/*N*/ 	sal_uInt16 i = 0;
/*N*/ 	const SwBookmarks& rMarks = pDoc->GetBookmarks();
/*N*/ 	while( i < rMarks.Count() )
/*N*/ 	{
/*N*/ 		const SwBookmark* pMark = rMarks.GetObject( i );
/*N*/ 		if( &pMark->pPos1->nNode.GetNode() == &pDoc->GetNodes().GetEndOfPostIts() )
/*?*/ 			pDoc->DelBookmark( i );
/*N*/ 		else
/*N*/ 			i++;
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::InDBName()
/*N*/ {
/*N*/ 	ByteString sStr8;
/*N*/ 	String	sStr, sSQL;
/*N*/ 	SwDBData aData;
/*N*/ 	OpenRec( SWG_DBNAME );
/*N*/ 
/*N*/ 	// MIB 9.4.97: Die Datenbanknamen koennen 0xff enthalten und muessen
/*N*/ 	// deshalb von Hand konvertiert werden.
/*N*/ 	pStrm->ReadByteString( sStr8 );
/*N*/ 	sStr = ConvertStringNoDbDelim( sStr8,  eSrcSet );
/*N*/ 	aData.sDataSource = sStr.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = sStr.GetToken(1, DB_DELIM);
/*N*/ 
/*N*/ 	if( IsVersion( SWG_NONUMLEVEL, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		InString( *pStrm, sSQL );
/*N*/ 	}
/*N*/ 	if( IsVersion( SWG_TARGETFRAME, SWG_EXPORT31 ) )
/*N*/ 	{
/*N*/ 		// Von der Version SWG_TARGETFRAME bis zur Version SWG_REGISTER
/*N*/ 		// stand hier der Default-Target-Frame.
/*N*/ 		// Seit der Version SWG_REGISTER steht hier der Tabellenname.
/*N*/ 		// Dazwischen wurde ein Leerstring geschrieben.
/*?*/ 		String aTableName;
/*?*/ 		InString( *pStrm, aTableName );
/*?*/ 
/*?*/ 		if( nVersion < SWG_REGISTER )
/*?*/ 		{
/*?*/ 			SfxDocumentInfo aInfo( *pDoc->GetInfo() );
/*?*/ 			aInfo.SetDefaultTarget( aTableName );
/*?*/ 			pDoc->SetInfo( aInfo );
/*?*/ 		}
/*?*/ 		else if( nVersion >= SWG_DBTABLE )
/*?*/ 		{
/*?*/ 			aData.sCommand = aTableName;
/*?*/ 		}
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	// Die folgende Tabelle der Datenbanknamen wird schon seit der
/*N*/ 	// Version SWG_USEDDB geschrieben, aber nur fuer Doks seit SWG_DBTABLE
/*N*/ 	// gelesen, da vorher kein Datenabnkname enthalten war. Auch der
/*N*/ 	// DB-Name des Doks ist erst seit dieser Version gueltig. Die
/*N*/ 	// Anzahl der Datenbanken lesen wir aber trotzdem, denn sie
/*N*/ 	// koennte 0 sein. Dann verliert man keine Informationen und muss
/*N*/ 	// auch keine Warnung hierzu bekommen.
/*N*/ 	if( IsVersion( SWG_USEDDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		if( sSQL.Len() && nVersion >= SWG_DBTABLE )
/*N*/ 		{
/*?*/ 			aData.sCommand = sSQL;
/*?*/ 			aData.nCommandType = sdb::CommandType::COMMAND;
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_uInt16 nCount;
/*N*/ 		*pStrm >> nCount;
/*N*/ 		if( nCount>0 && nVersion >= SWG_DBTABLE )
/*N*/ 		{
/*N*/ 			// MIB 9.4.97: Die Datenbanknamen koennen 0xff enthalten und
/*N*/ 			// muessen deshalb von Hand konvertiert werden.
/*N*/ 			String sDBName;
/*N*/ 			sal_Int32 nSelStart, nSelEnd;
/*N*/ 
/*N*/ 			SwNewDBMgr& rDBMgr = *pDoc->GetNewDBMgr();
/*N*/ 			for( sal_uInt16 i = 0; i < nCount; i++ )
/*N*/ 			{
/*N*/ 				pStrm->ReadByteString( sStr8 );
/*N*/ 				*pStrm >> nSelStart
/*N*/ 					   >> nSelEnd;
/*N*/ 				sDBName = ConvertStringNoDbDelim( sStr8, eSrcSet );
/*N*/ 
/*N*/                 SwDBData aData;
/*N*/                 aData.sDataSource = sDBName.GetToken(0, DB_DELIM);
/*N*/                 aData.sCommand = sDBName.GetToken(1, DB_DELIM);
/*N*/                 aData.nCommandType = -1;
/*N*/                 rDBMgr.AddDSData(aData, nSelStart, nSelEnd );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// sonst zumindest irgendeinen Namen setzen
/*N*/ 		aData = pDoc->GetNewDBMgr()->GetAddressDBName();
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_DBNAME );
/*N*/ 	pDoc->ChgDBData( aData );
/*N*/ }

/*N*/ void Sw3IoImp::OutDBName()
/*N*/ {
/*N*/ 	// Erstmal alle verwendeten Datenbanken holen
/*N*/ 	SvStringsDtor aDBNameList;
/*N*/ 	pDoc->GetAllUsedDB(aDBNameList);
/*N*/ 	sal_uInt16 nCount = aDBNameList.Count();
/*N*/ 
/*N*/ 	OpenRec( SWG_DBNAME );
/*N*/ 
/*N*/ 	if( IsSw31Export() )
/*N*/ 	{
/*N*/ 		// Datenbanknamen und das SQL-Statement holen
/*N*/ 		String sDBDesc, sDBName;
/*N*/ 		if( nCount==1 && pDoc->GetNewDBMgr() )
/*N*/ 		{
/*N*/ 			sDBDesc = *aDBNameList.GetObject(0);
/*N*/ 			sDBName = sDBDesc;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwDBData aData = pDoc->_GetDBDesc();
/*N*/ 			sDBName = aData.sDataSource;
/*N*/ 			sDBName += DB_DELIM;
/*N*/ 			sDBName += (String)aData.sCommand;
/*N*/ 		}
/*N*/ 
/*N*/ 		OutString( *pStrm, sDBName.GetToken( 0, DB_DELIM ) );
/*N*/ 
/*N*/ 		String aTmpStr;
/*N*/ 		xub_StrLen nPos = 0;
/*N*/ 		if( ( nPos = sDBDesc.Search(';') ) != STRING_NOTFOUND )
/*?*/ 			aTmpStr = sDBDesc.Copy( nPos + 1 );
/*N*/ 		OutString( *pStrm, aTmpStr );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Die Datenbank-Beschreibung koennen wir im 4.0 FF auf einen Schlag
/*N*/ 		// rausschreiben
/*N*/ 		SwDBData aData = pDoc->_GetDBDesc();
/*N*/ 		String sTmpDBName = aData.sDataSource;
/*N*/ 		sTmpDBName += DB_DELIM;
/*N*/ 		sTmpDBName += (String)aData.sCommand;
/*N*/ 		ByteString s8 = ConvertStringNoDbDelim( sTmpDBName, eSrcSet );
/*N*/ 		pStrm->WriteByteString( s8 );
/*N*/ 
/*N*/ 		// Datenbankname, SQL-Statement und aktuelle Selektion abspeichern
/*N*/ 		long nSelStart, nSelEnd;
/*N*/ 
/*N*/ 		*pStrm << nCount;
/*N*/ 
/*N*/ 		SwNewDBMgr& rDBMgr = *pDoc->GetNewDBMgr();
/*N*/ 		for (sal_uInt16 i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			String sDesc(*aDBNameList.GetObject(i));
/*N*/ 			ByteString s8 = ConvertStringNoDbDelim( sDesc, eSrcSet );
/*N*/ 			pStrm->WriteByteString( s8 );
/*N*/ 
/*N*/             SwDBData aData;
/*N*/             aData.sDataSource = sDesc.GetToken(0, DB_DELIM);
/*N*/             aData.sCommand = sDesc.GetToken(1, DB_DELIM);
/*N*/             aData.nCommandType = -1;
/*N*/             rDBMgr.GetDSSelection(aData, nSelStart, nSelEnd);
/*N*/ 
/*N*/ 			*pStrm << (sal_Int32)nSelStart
/*N*/ 				   << (sal_Int32)nSelEnd;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_DBNAME );
/*N*/ }

/*N*/ void Sw3IoImp::InDocStat()
/*N*/ {
/*N*/ 	OpenRec( SWG_DOCSTAT );
/*N*/ 	SwDocStat aDocStat;
/*N*/ 	BYTE c;
/*N*/ 	*pStrm  >> aDocStat.nTbl
/*N*/ 			>> aDocStat.nGrf
/*N*/ 			>> aDocStat.nOLE;
/*N*/ 
/*N*/ 	if( IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		sal_uInt32 nPage, nPara;
/*N*/ 		*pStrm >> nPage
/*N*/ 			   >> nPara;
/*N*/ 
/*N*/ 		aDocStat.nPage = nPage;
/*N*/ 		aDocStat.nPara = nPara;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		sal_uInt16 nPage, nPara;
/*N*/ 		*pStrm >> nPage
/*N*/ 			   >> nPara;
/*N*/ 
/*N*/ 		aDocStat.nPage = nPage;
/*N*/ 		aDocStat.nPara = nPara;
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_uInt32 w;
/*N*/ 	*pStrm	>> w;
/*N*/ 	aDocStat.nWord = w;
/*N*/ 	*pStrm	>> w;
/*N*/ 	aDocStat.nChar = w;
/*N*/ 	*pStrm >> c;
/*N*/ 
/*N*/ 	aDocStat.bModified = c;
/*N*/ 
/*N*/ 	CloseRec( SWG_DOCSTAT );
/*N*/ 	pDoc->SetDocStat( aDocStat );
/*N*/ }

/*N*/ void Sw3IoImp::OutDocStat( sal_Bool bFirst )
/*N*/ {
/*N*/ 	sal_uInt32 nAktPos;
/*N*/ 	if( bFirst )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_DOCSTAT );
/*N*/ 		nStatStart = pStrm->Tell();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nAktPos = pStrm->Tell();
/*N*/ 		pStrm->Seek( nStatStart );
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwDocStat& rDocStat = pDoc->GetDocStat();
/*N*/ 	*pStrm << (sal_uInt16)rDocStat.nTbl
/*N*/ 		   << (sal_uInt16)rDocStat.nGrf
/*N*/ 		   << (sal_uInt16)rDocStat.nOLE;
/*N*/ 
/*N*/ 	if( pStrm->GetVersion() <= SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		*pStrm << (sal_uInt16)rDocStat.nPage
/*N*/ 			   << (sal_uInt16)rDocStat.nPara;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Die Longs duerfen hier nicht komprimiert geschrieben werden,
/*N*/ 		// weil sich dann die Laenge der Dok-Info beim zweiten Schreiben
/*N*/ 		// gegenueber dem ersten Schreiben aendern kann.
/*N*/ 		*pStrm << (sal_uInt32)rDocStat.nPage
/*N*/ 			   << (sal_uInt32)rDocStat.nPara;
/*N*/ 	}
/*N*/ 
/*N*/ 	*pStrm << (sal_uInt32)rDocStat.nWord
/*N*/ 		   << (sal_uInt32)rDocStat.nChar
/*N*/ 		   << (BYTE)rDocStat.bModified;
/*N*/ 
/*N*/ 	if( bFirst )
/*N*/ 		CloseRec( SWG_DOCSTAT );
/*N*/ 	else
/*N*/ 		pStrm->Seek( nAktPos );
/*N*/ }

/*N*/ void Sw3IoImp::InDictionary()
/*N*/ {
/*N*/ 	Reference< XDictionaryList > xDicList( ::binfilter::GetDictionaryList() );
/*N*/ 	Sequence< Reference< XDictionary > > aDics;
/*N*/ 	if( xDicList.is() )
/*N*/ 		aDics = xDicList->getDictionaries();
/*N*/ 	const Reference< XDictionary > *pDic = aDics.getConstArray();
/*N*/ 
/*N*/ 	bSpellAllAgain = bSpellWrongAgain = sal_True;
/*N*/ 
/*N*/ 	sal_uInt16 i, nDicCount = (sal_uInt16)aDics.getLength();
/*N*/ 
/*N*/ 	sal_Bool *pChecked = nDicCount > 0 ? new sal_Bool[nDicCount] : 0;
/*N*/ 	for( i = 0; i < nDicCount; i++ )
/*N*/ 		pChecked[ i ] = !pDic[i]->isActive();
/*N*/ 
/*N*/ 	OpenRec( SWG_DICTIONARY );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		String aStr;
/*N*/ 		sal_uInt16 nLanguage;
/*N*/ 		sal_uInt16 nCount;
/*N*/ 		BYTE c;
/*N*/ 		sal_Bool bNeg;
/*N*/ 		InString( *pStrm, aStr );
/*N*/ 		*pStrm >> nLanguage
/*N*/ 			   >> nCount
/*N*/ 			   >> c;
/*N*/ 		bNeg = c;
/*N*/ 		sal_Bool bFound = sal_False;
/*N*/ 		for( i = 0; !bFound && ( i < nDicCount ); i++ )
/*N*/ 		{
/*N*/ 			Reference< XDictionary1 > xDic( pDic[i], UNO_QUERY );
/*N*/ 			if( xDic.is()  &&  xDic->isActive() )
/*N*/ 				bFound = ( aStr == String(xDic->getName()) &&
/*N*/ 						   nLanguage == xDic->getLanguage() &&
/*N*/ 						   bNeg == (xDic->getDictionaryType()
/*N*/ 						   			== DictionaryType_NEGATIVE) );
/*N*/ 		}
/*N*/ 		if( bFound )
/*N*/ 		{
/*N*/ 			ASSERT( pChecked, "Sw3IoImp::InDictonary: pChecked == 0" );
/*N*/ 			sal_uInt16 nCnt = pDic[--i]->getCount();
/*N*/ 			pChecked[ i ] = sal_True;
/*N*/ 			if( nCnt != nCount )
/*N*/ 			{
/*N*/ 				if( bNeg || nCnt < nCount )
/*N*/ 					bSpellAllAgain = sal_True;
/*N*/ 				else
/*N*/ 					bSpellWrongAgain = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( bNeg )
/*N*/ 			bSpellWrongAgain = sal_True;
/*N*/ 		else
/*N*/ 			bSpellAllAgain = sal_True;
/*N*/ 	}
/*N*/ 	CloseRec( SWG_DICTIONARY );
/*N*/ 
/*N*/ 	for( i = 0; i < nDicCount; i++ )
/*N*/ 	{
/*N*/ 		if( !pChecked[ i ] )
/*N*/ 		{
/*N*/ 			if( pDic[i]->getDictionaryType() == DictionaryType_NEGATIVE )
/*N*/ 				bSpellAllAgain = sal_True;
/*N*/ 			else
/*N*/ 				bSpellWrongAgain =sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete[] pChecked;
/*N*/ 
/*N*/ }

/*N*/ void Sw3IoImp::OutDictionary()
/*N*/ {
/*N*/ 	OpenRec( SWG_DICTIONARY );
/*N*/ 
/*N*/ 	Reference< XDictionaryList > xDicList( ::binfilter::GetDictionaryList() );
/*N*/ 	Sequence< Reference< XDictionary > > aDics;
/*N*/ 	if( xDicList.is() )
/*N*/ 		aDics = xDicList->getDictionaries();
/*N*/ 	const Reference< XDictionary > *pDic = aDics.getConstArray();
/*N*/ 
/*N*/ 	sal_uInt16 i, nDicCount = (sal_uInt16)aDics.getLength();
/*N*/ 	for( i = 0; i < nDicCount; i++ )
/*N*/ 	{
/*N*/ 		Reference< XDictionary1 > xDic( pDic[i], UNO_QUERY );
/*N*/ 		if( xDic.is()  &&  xDic->isActive() )
/*N*/ 		{
/*N*/ 			const String aStr( xDic->getName() );
/*N*/ 			sal_uInt16 nActLanguage = xDic->getLanguage();
/*N*/ 			sal_uInt16 nCount = xDic->getCount();
/*N*/ 			sal_Bool bNeg = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
/*N*/ 			OutString( *pStrm, aStr );
/*N*/ 			*pStrm << (sal_uInt16)nActLanguage
/*N*/ 				   << (sal_uInt16)nCount
/*N*/ 				   << (BYTE)bNeg;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( SWG_DICTIONARY );
/*N*/ }

/*N*/ void Sw3IoImp::OutNumberFormatter()
/*N*/ {
/*N*/ 	SvNumberFormatter* pN = pDoc->GetNumberFormatter( sal_False );
/*N*/ 	if( pN )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_NUMBERFORMATTER );
/*N*/ 		pN->Save( *pStrm );
/*N*/ 		CloseRec( SWG_NUMBERFORMATTER );
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::InNumberFormatter()
/*N*/ {
/*N*/ 	OpenRec( SWG_NUMBERFORMATTER );
/*N*/ 
/*N*/ 	if( bInsert || bOrganizer )
/*N*/ 	{
/*?*/ 		Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*?*/ 		SvNumberFormatter* pN = new SvNumberFormatter( xMSF, LANGUAGE_SYSTEM );
/*?*/ 		pN->Load( *pStrm );
/*?*/ 		pDoc->GetNumberFormatter( sal_True )->MergeFormatter( *pN );
/*?*/ 		delete pN;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pDoc->GetNumberFormatter( sal_True )->Load( *pStrm );
/*N*/ 
/*N*/ 	CloseRec( SWG_NUMBERFORMATTER );
/*N*/ }

/*N*/ void Sw3IoImp::InLineNumberInfo()
/*N*/ {
/*N*/ 	OpenRec( SWG_LINENUMBERINFO );
/*N*/ 
/*N*/ 	sal_uInt8 nType, nPos;
/*N*/ 	sal_uInt16 nChrIdx, nPosFromLeft, nCountBy, nDividerCountBy;
/*N*/ 	String sDivider;
/*N*/ 
/*N*/ 	sal_uInt8 cFlags = OpenFlagRec();
/*N*/ 	*pStrm  >>	nType
/*N*/ 			>>	nPos
/*N*/ 			>>	nChrIdx
/*N*/ 			>>	nPosFromLeft
/*N*/ 			>>	nCountBy
/*N*/ 			>>	nDividerCountBy;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	InString( *pStrm, sDivider );
/*N*/ 
/*N*/ 	SwLineNumberInfo aInfo;
/*N*/ 
/*N*/ 	aInfo.SetPaintLineNumbers( (cFlags & 0x10 ) != 0 );
/*N*/ 	aInfo.SetCountBlankLines( (cFlags & 0x20 ) != 0 );
/*N*/ 	aInfo.SetCountInFlys( (cFlags & 0x40 ) != 0 );
/*N*/ 	aInfo.SetRestartEachPage( (cFlags & 0x80 ) != 0 );
/*N*/ 	aInfo.SetPos(  (LineNumberPosition)nPos );
/*N*/ 
/*N*/ 	SvxNumberType aNumType;
/*N*/ 	aNumType.SetNumberingType((sal_Int16)nType);
/*N*/ 	aInfo.SetNumType( aNumType );
/*N*/ 
/*N*/ 	if( nChrIdx != IDX_NO_VALUE )
/*N*/ 	{
/*N*/ 		SwCharFmt *pChrFmt = (SwCharFmt *)FindFmt( nChrIdx, SWG_CHARFMT );
/*N*/ 		if( pChrFmt )
/*N*/ 			aInfo.SetCharFmt( pChrFmt );
/*N*/ 	}
/*N*/ 	aInfo.SetPosFromLeft( nPosFromLeft );
/*N*/ 	aInfo.SetCountBy( nCountBy );
/*N*/ 	aInfo.SetDividerCountBy( nDividerCountBy );
/*N*/ 	aInfo.SetDivider( sDivider );
/*N*/ 
/*N*/ 	pDoc->SetLineNumberInfo( aInfo );
/*N*/ 
/*N*/ 	CloseRec( SWG_LINENUMBERINFO );
/*N*/ }

/*N*/ void Sw3IoImp::OutLineNumberInfo()
/*N*/ {
/*N*/ 	OpenRec( SWG_LINENUMBERINFO );
/*N*/ 
/*N*/ 	const SwLineNumberInfo& rInfo = pDoc->GetLineNumberInfo();
/*N*/ 
/*N*/ 	sal_uInt8 cFlags = 0x0a;
/*N*/ 	if( rInfo.IsPaintLineNumbers() )
/*N*/ 		cFlags += 0x10;
/*N*/ 	if( rInfo.IsCountBlankLines() )
/*N*/ 		cFlags += 0x20;
/*N*/ 	if( rInfo.IsCountInFlys() )
/*N*/ 		cFlags += 0x40;
/*N*/ 	if( rInfo.IsRestartEachPage() )
/*N*/ 		cFlags += 0x80;
/*N*/ 
/*N*/ 	const SwCharFmt *pCharFmt = (const SwCharFmt *)rInfo.GetRegisteredIn();
/*N*/ 	sal_uInt16 nChrIdx = pCharFmt ? aStringPool.Find( pCharFmt->GetName(),
/*N*/ 												  pCharFmt->GetPoolFmtId() )
/*N*/ 							  : IDX_NO_VALUE;
/*N*/ 
/*N*/ 	*pStrm  << 			cFlags
/*N*/ 			<< (sal_uInt8)   rInfo.GetNumType().GetNumberingType()
/*N*/ 			<< (sal_uInt8)   rInfo.GetPos()
/*N*/ 			<< (sal_uInt16)	nChrIdx
/*N*/ 			<< (sal_uInt16) rInfo.GetPosFromLeft()
/*N*/ 			<< (sal_uInt16) rInfo.GetCountBy()
/*N*/ 			<< (sal_uInt16) rInfo.GetDividerCountBy();
/*N*/ 	OutString( *pStrm, rInfo.GetDivider() );
/*N*/ 
/*N*/ 	CloseRec( SWG_LINENUMBERINFO );
/*N*/ }


/*N*/ void Sw3IoImp::InDocDummies()
/*N*/ {
/*N*/ 	OpenRec( SWG_DOCDUMMIES );
/*N*/ 
/*N*/ 	sal_uInt32 n1, n2;
/*N*/ 	sal_uInt8 n3, n4;
/*N*/ 	String sAutoMarkURL, s2;
/*N*/ 
/*N*/ 	*pStrm >> n1 >> n2 >> n3 >> n4;
/*N*/ 	InString( *pStrm, sAutoMarkURL );
/*N*/ 	InString( *pStrm, s2 );
/*N*/ 
/*N*/ 	// Das 0. und 1. Bit ist der Link Update-Mode
/*N*/ 	sal_uInt16 nUpdMode = MANUAL;
/*N*/ 	switch( n3 & 0x03 )
/*N*/ 	{
/*N*/ 	case 1:	nUpdMode = NEVER;			break;
/*N*/ 	case 2:	nUpdMode = AUTOMATIC;		break;
/*N*/ 	case 3:	nUpdMode = GLOBALSETTING;	break;
/*N*/ 	}
/*N*/ 	pDoc->SetLinkUpdMode( nUpdMode );
/*N*/ 
/*N*/ 	// das 2. Bit verbleibt im Byte-Dummy1
/*N*/ 
/*N*/ 	// das 3. und 4. Bit ist der Field Update-Mode
/*N*/ 	nUpdMode = AUTOUPD_OFF;
/*N*/ 	switch( (n3 & 0x18) >> 3 )
/*N*/ 	{
/*N*/ 	case 0:	nUpdMode = AUTOUPD_OFF;					break;
/*N*/ 	case 1:	nUpdMode = AUTOUPD_FIELD_ONLY;			break;
/*N*/ 	case 2:	nUpdMode = AUTOUPD_FIELD_AND_CHARTS;	break;
/*N*/ 	case 3:	nUpdMode = AUTOUPD_GLOBALSETTING;		break;
/*N*/ 	}
/*N*/ 	pDoc->SetFldUpdateFlags( nUpdMode );
/*N*/ 
/*N*/ 	// Achtung: Das drittunterste Bit 0x04 wird jetzt benutzt
/*N*/ 	// als Flag, ob Absatzabstaende addiert oder maximiert werden
/*N*/ 	// 0x20 - makes the same at start of doc an at page starts behind page breaks
/*N*/ 	n3 &= 0xE4;
/*N*/ 
/*N*/ 	pDoc->SetULongDummy1( n1 );
/*N*/ 	pDoc->SetULongDummy2( n2 );
/*N*/ 	pDoc->SetByteDummy1( n3 );
/*N*/ 	pDoc->SetByteDummy2( n4 );
/*N*/ 	if( sAutoMarkURL.Len() )
/*?*/ 		sAutoMarkURL = ::binfilter::StaticBaseUrl::SmartRelToAbs( sAutoMarkURL );
/*N*/ 	pDoc->SetTOIAutoMarkURL( sAutoMarkURL );
/*N*/ 	pDoc->SetStringDummy2( s2 );
/*N*/ 
/*N*/ 	CloseRec( SWG_DOCDUMMIES );
/*N*/ }

/*N*/ void Sw3IoImp::OutDocDummies()
/*N*/ {
/*N*/ 	OpenRec( SWG_DOCDUMMIES );
/*N*/ 
/*N*/ 	sal_uInt8 n3 = (sal_uInt8)pDoc->GetByteDummy1();
/*N*/ 	// Achtung: Das drittunterste Bit 0x04 wird jetzt benutzt
/*N*/ 	// als Flag, ob Absatzabstaende addiert oder maximiert werden
/*N*/ 	// 0x20 - makes the same at start of doc an at page starts behind page breaks
/*N*/ 	ASSERT( (n3 & 0x03) == 0, "Sw3IoImp::OutDocDummies: byte1 falsch" );
/*N*/ 	n3 &= 0xfc;
/*N*/ 
/*N*/ 	// Der Link-Update-Mode muss noch etwas konvertiert werden, damit
/*N*/ 	// der Default-Wert in den Dummies (0) einem MANUAL (1) entspricht
/*N*/ 	switch( pDoc->_GetLinkUpdMode() )
/*N*/ 	{
/*N*/ 	case MANUAL:		break;
/*N*/ 	case NEVER:			n3 |= 0x01;	break;
/*N*/ 	case AUTOMATIC:		n3 |= 0x02;	break;
/*N*/ 	case GLOBALSETTING:	n3 |= 0x03;	break;
/*N*/ 	default:
/*N*/ 		ASSERT( !this, "Sw3IoImp::OutDocDummies: Wert von LinkUpdMode unbek." );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der Feld-Update-Mode muss noch etwas konvertiert werden, damit
/*N*/ 	// der Default-Wert in den Dummies (0) einem OFF entspricht
/*N*/ 	switch( pDoc->_GetFldUpdateFlags() )
/*N*/ 	{
/*N*/ 	case AUTOUPD_OFF:				break;
/*N*/ 	case AUTOUPD_FIELD_ONLY:		n3 |= ( 0x01 << 3);	break;
/*N*/ 	case AUTOUPD_FIELD_AND_CHARTS:	n3 |= ( 0x02 << 3);	break;
/*N*/ 	case AUTOUPD_GLOBALSETTING:		n3 |= ( 0x03 << 3);	break;
/*N*/ 	default:
/*N*/ 		ASSERT( !this, "Sw3IoImp::OutDocDummies: Wert von FieldUpdMode unbek." );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	String sAutoMarkURL( pDoc->GetTOIAutoMarkURL() );
/*N*/ 	if( sAutoMarkURL.Len() )
/*?*/ 		sAutoMarkURL = ::binfilter::StaticBaseUrl::AbsToRel( sAutoMarkURL URL_DECODE );
/*N*/ 	*pStrm	<< (sal_uInt32)pDoc->GetULongDummy1()
/*N*/ 			<< (sal_uInt32)pDoc->GetULongDummy2()
/*N*/ 			<< n3
/*N*/ 			<< (sal_uInt8)pDoc->GetByteDummy2();
/*N*/ 	OutString( *pStrm, sAutoMarkURL );
/*N*/ 	OutString( *pStrm, pDoc->GetStringDummy2() );
/*N*/ 
/*N*/ 	CloseRec( SWG_DOCDUMMIES );
/*N*/ }

void Sw3IoImp::InPagePreViewPrintData()
{
    OpenRec( SWG_PGPREVIEWPRTDATA );

    sal_uInt8 cFlags;
    sal_uInt8 nRow, nCol;
    sal_uInt32 nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;

    *pStrm  >> cFlags
            >> nRow
            >> nCol
            >> nLeftSpace
            >> nRightSpace
            >> nTopSpace
            >> nBottomSpace
            >> nHorzSpace
            >> nVertSpace
            ;
    CloseRec( SWG_PGPREVIEWPRTDATA );

    SwPagePreViewPrtData aData;
    aData.SetLeftSpace( nLeftSpace );
    aData.SetRightSpace( nRightSpace );
    aData.SetTopSpace( nTopSpace );
    aData.SetBottomSpace( nBottomSpace );
    aData.SetHorzSpace( nHorzSpace );
    aData.SetVertSpace( nVertSpace );
    aData.SetRow( nRow );
    aData.SetCol( nCol );
    aData.SetLandscape( 0 != ( cFlags & 0x01 ) );
    aData.SetStretch( 0 != ( cFlags & 0x02 ) );
    pDoc->SetPreViewPrtData( &aData );
}

/*N*/ void Sw3IoImp::OutPagePreViewPrintData()
/*N*/ {
/*N*/ 	// Wenn keine Daten gesetzt oder keine gueltige Zeile/Spalte
/*N*/ 	// gesetzt sind, so braucht es auch nicht geschrieben werden.
/*N*/ 	const SwPagePreViewPrtData* pPPVPD = pDoc->GetPreViewPrtData();
/*N*/ 	if( !pPPVPD || !pPPVPD->GetCol() || !pPPVPD->GetRow() )
/*N*/ 		return;
/*N*/ 
/*?*/ 	OpenRec( SWG_PGPREVIEWPRTDATA );
/*?*/ 
/*?*/ 	sal_uInt8 cFlags = 0;
/*?*/ 	if( pPPVPD->GetLandscape() )
/*?*/ 		cFlags += 0x01;
/*?*/ 	if( pPPVPD->GetStretch() )
/*?*/ 		cFlags += 0x02;
/*?*/ 
/*?*/ 	*pStrm	<< cFlags
/*?*/ 			<< pPPVPD->GetRow()
/*?*/ 			<< pPPVPD->GetCol()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetLeftSpace()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetRightSpace()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetTopSpace()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetBottomSpace()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetHorzSpace()
/*?*/ 			<< (sal_uInt32)pPPVPD->GetVertSpace()
/*?*/ 			;
/*?*/ 	CloseRec( SWG_PGPREVIEWPRTDATA );
/*N*/ }



}
