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

// Diese Implementation geht davon aus, dass der Pool nur temporaer
// verwendet wird, d.h. der ctor macht einen Snapshot der Vorlagen am Doc.


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "swerror.h"

#include <horiornt.hxx>

#include "doc.hxx"
#include "hintids.hxx"			//Damit "unsere" Attribute angezogen werden.

#include <bf_svx/fontitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svtools/style.hxx>
#include <bf_svtools/htmlkywd.hxx>


#include <errhdl.hxx>

#include <fmtcol.hxx>

#include <frmfmt.hxx>
#include <charfmt.hxx>
#include "sw3imp.hxx"
#include "poolfmt.hxx"
#include "hints.hxx"
#include "paratr.hxx"
#include "frmatr.hxx"
#include "numrule.hxx"
#include <SwStyleNameMapper.hxx>
namespace binfilter {

#define SWG_CONDSTYLES_BUG	1		//Versionsnummern fuer die StyleSheets
#define SWG_CONDSTYLES		2		//Versionsnummern fuer die StyleSheets
#define SWG_CHARFMTOFFSET 	3		//Versionsnummern fuer die StyleSheets
#define SWG_AUTOUPDATE_FLAG 4		// zusaetzliches Flag-Byte
#define SWG_TEN_HEADLINES	5		// HTML-H6-Vorlage mappen
#define SWG_NUM_RELSPACE	6		// Numerierung mit rel. Abstaenden

#ifdef NUM_RELSPACE
#define SWG_STYLE_VERSION (SWG_NUM_RELSPACE)
#else
#define SWG_STYLE_VERSION (SWG_TEN_HEADLINES)
#endif

///////////////////////////////////////////////////////////////////////////

//FEATURE::CONDCOLL
struct SwStyleCondColl
{
    UINT32 nCondition, nSubCondition;
    String sColl;

    SwStyleCondColl()
        : nCondition( 0 ), nSubCondition( 0 )
    {}
    SwStyleCondColl( const String& rColl, UINT32 nCond, UINT32 nSubCond )
        : nCondition( nCond ),
          nSubCondition( nSubCond ),
          sColl( rColl )
    {}
    SwStyleCondColl( const SwStyleCondColl& rCopy )
        : nCondition( rCopy.nCondition ),
          nSubCondition( rCopy.nSubCondition ),
          sColl( rCopy.sColl )
    {}
};
typedef SwStyleCondColl* SwStyleCondCollPtr;
SV_DECL_PTRARR_DEL( SwStyleCondColls, SwStyleCondCollPtr, 0, 5 )//STRIP008 ;
//FEATURE::CONDCOLL

class SwStyleSheetPool;

// Unsere Ableitungen der Klassen SfxStyleSheetBase und SfxStyleSheetBasePool

class SwStyleSheet : public SfxStyleSheetBase {
    friend class SwStyleSheetPool;
    SfxItemSet aSet;				// Set fuer Load/Store
    SwDoc& 	   rDoc;				// Dokument
    SwFmt*	   pFmt;				// zugehoeriges Format
    long   	   nVersion;			// Export: FF-Version
                                    // Import: Vorlagen-Version
    USHORT 	   nId;					// PoolId
    BYTE 	   nLevel;				// Outline-Level
    BYTE	   cFlags;				// diverse Flags (0x01 == AutoUpdate)
    BOOL	   bNew : 1;	   		// TRUE: neu eingelesen/erzeugt
    SwStyleCondColls* pCondColls;	// != 0 -> CondColl - Tabelle der Conditions
    SvxLRSpaceItem *pNumLRSpace;	// Original-LRSpace-Item bei Kapitel-Num.
private:
    SwStyleSheet( const String&, SwStyleSheetPool&, SfxStyleFamily eFam,
                    USHORT nMask );
    virtual SfxItemSet& GetItemSet();
public:
    virtual ~SwStyleSheet();
    SwDoc& GetDoc() { return rDoc; }
    SwFmt* GetFmt() { return pFmt; }
    SwTxtFmtColl* GetColl();
    BYTE GetNumLevel() const { return nLevel; }
    void ConnectParent( const String& );
    void ConnectFollow( const String& );
    virtual void Load( SvStream&, USHORT );
    virtual void Store( SvStream& );
    virtual USHORT GetVersion() const;

    // fuers Rename - Vorlagen muessen noch nicht im Pool umbenannt sein,
    // 				  die normale Set-Methoden suchen aber danach und setzen
    //				  nur um, wenn sie die Vorlage gefunden haben.
    void SetNewName( const String& rName )		{ aName = rName; }
    void SetNewParent( const String& rName )	{ aParent = rName; }
    void SetNewFollow( const String& rName )	{ aFollow = rName; }
};

class SwStyleSheetPool : public SfxStyleSheetBasePool {
    friend class SwStyleSheet;
    SwDoc&  	 rDoc;			// Dokument
    Sw3Fmts* 	 pConvToSymbolFmts;
    long		 nExpFFVersion;	// FF-Version fuer Export
    virtual SfxStyleSheetBase* Create( const String&, SfxStyleFamily, USHORT nMask);
    void Add( const SwFmt& rFmt, SfxStyleFamily eFam );
    void CopyFromDoc( BOOL bUsed );
    void CopyToDoc( BOOL bOverwrite, USHORT eMask );
    void Rename( const String&, const String&, SfxStyleFamily );
    SwStyleSheet *FindByPoolId( USHORT nPoolId );
public:
    SwStyleSheetPool( SwDoc&, SfxItemPool&, long nFFVersion, Sw3Fmts *pConvFmts );
    virtual ~SwStyleSheetPool();
    BOOL Load( SvStream&, BOOL, USHORT eMask );
    BOOL Store( SvStream&, BOOL );
};




/*N*/ SV_IMPL_PTRARR( SwStyleCondColls, SwStyleCondCollPtr )

///////////////////////////////////////////////////////////////////////////
//							lokale Hilfsroutinen						 //
///////////////////////////////////////////////////////////////////////////

// Suche eines Formats anhand der Family-Bits

/*N*/ SwFmt* lcl_sw3io__FindFmt( SwDoc& rDoc, const String& rName, USHORT nFamily )
/*N*/ {
/*N*/ 	if( nFamily & SFX_STYLE_FAMILY_PARA )
/*N*/ 		return rDoc.FindTxtFmtCollByName( rName );
/*N*/ 	else if( nFamily & SFX_STYLE_FAMILY_FRAME )
/*N*/ 		return rDoc.FindFrmFmtByName( rName );
/*N*/ 	else if( nFamily & SFX_STYLE_FAMILY_CHAR )
/*N*/ 		return rDoc.FindCharFmtByName( rName );
/*N*/
/*N*/ 	DBG_ASSERT( nFamily || !nFamily, "Unbekannte Style-Familie" );
/*N*/ 	return NULL;
/*N*/ }

////////////////////////////////////////////////////////////////////////////

/*N*/ SwStyleSheet::SwStyleSheet( const String& rName, SwStyleSheetPool& rPool,
/*N*/ 							SfxStyleFamily eFam, USHORT nMask )
/*N*/ 	: SfxStyleSheetBase( rName, rPool, eFam, nMask ),
/*N*/ 	  rDoc( rPool.rDoc ), aSet( rPool.rPool,
/*N*/ 			RES_CHRATR_BEGIN,		RES_CHRATR_END - 1,
/*N*/ 			RES_PARATR_BEGIN, 		RES_PARATR_END - 1,
/*N*/ 			RES_FRMATR_BEGIN, 		RES_FRMATR_END - 1,
/*N*/ 			0 ),
/*N*/ 	  cFlags( 0 ),
/*N*/ 	  pCondColls( 0 ),
/*N*/ 	  pNumLRSpace( 0 )
/*N*/ {
/*N*/ 	nId = 0xFFFF;
/*N*/ 	nHelpId = UCHAR_MAX;
/*N*/ 	nLevel = NO_NUMBERING;
/*N*/ 	pFmt = NULL;
/*N*/ 	bNew = FALSE;
/*N*/ 	nVersion = 0;
/*N*/ }

/*N*/ __EXPORT SwStyleSheet::~SwStyleSheet()
/*N*/ {
/*N*/ 	// wird von SFX-DLL gerufen!
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	delete pCondColls;
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	delete pNumLRSpace;
/*N*/ }

/*N*/ SwTxtFmtColl* SwStyleSheet::GetColl()
/*N*/ {
/*N*/ 	return ( nFamily & SFX_STYLE_FAMILY_PARA )
/*N*/ 		   ? (SwTxtFmtColl*) pFmt : NULL;
/*N*/ }


/*N*/ void __EXPORT SwStyleSheet::Load( SvStream& r, USHORT nVer )
/*N*/ {
/*N*/ 	nVersion = nVer;	// Version wird noch gebraucht
/*N*/
/*N*/ 	r >> nId >> nLevel;
/*N*/
/*N*/ 	// Numerierungs-Ebende weglassen, wenn sie nicht unterstuetzt wird
/*N*/ 	if( nLevel != NO_NUMBERING && GetRealLevel(nLevel) >= MAXLEVEL )
/*N*/ 		nLevel = NO_NUMBERING;
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	if( SWG_CONDSTYLES_BUG <= nVer )
/*N*/ 	{
/*N*/ 		//JP 31.10.96: Bug vom TemplateDialog beheben. Es wurde eine falsche
/*N*/ 		//				PoolId (1) fuer die Textkoerpervorlage verwendet
/*N*/ 		if( SWG_CONDSTYLES_BUG == nVer && SFX_STYLE_FAMILY_PARA == nFamily
/*N*/ 			&& 1 == nId )
/*N*/ 		{
/*N*/ 			nId = RES_POOLCOLL_TEXT;
/*N*/ 		}
/*N*/
/*N*/ 		// ConditionStyleSheets ??
/*N*/ 		UINT16 nTmp;
/*N*/ 		r >> nTmp;
/*N*/ 		if( 1 == nTmp )
/*N*/ 		{
/*N*/ 			r >> nTmp;	// Anzahl:
/*N*/ 			pCondColls = new SwStyleCondColls( nTmp < 255 ? nTmp : 255, 5 );
/*N*/ 			for( USHORT n = 0; n < nTmp; ++n )
/*N*/ 			{
/*?*/ 				SwStyleCondColl* pNew = new SwStyleCondColl;
/*?*/ 				r.ReadByteString( pNew->sColl, r.GetStreamCharSet() );
/*?*/ 				r >> pNew->nCondition;
/*?*/
/*?*/ 				if( USRFLD_EXPRESSION & pNew->nCondition )
/*?*/ 				{
/*?*/ 					ASSERT( !this, "noch nicht implementiert" );
/*?*/ 					String s;
/*?*/ 					r.ReadByteString( s, r.GetStreamCharSet() );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					r >> pNew->nSubCondition;
/*?*/ 				pCondColls->Insert( pNew, n );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 	// Zusaetzliches Flag-Byte laden
/*N*/ 	if( SWG_AUTOUPDATE_FLAG <= nVer )
/*N*/ 	{
/*N*/ 		r >> cFlags;
/*N*/ 	}
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 	if( SWG_NUM_RELSPACE >= nVer && (cFlags & 0x02) != 0 )
/*N*/ 	{
/*N*/ 		UINT32 nLen;
/*N*/ 		r >> nLen;
/*N*/ 		if( nLen > 0 )
/*N*/ 		{
/*?*/ 			ULONG nPos = r.Tell();
/*?*/ 			UINT16 nIVer;
/*?*/ 			r >> nIVer;
/*?*/ 			const SfxPoolItem& rDflt =
/*?*/ 				GetItemSet().GetPool()->GetDefaultItem( RES_LR_SPACE );
/*?*/ 			SfxPoolItem *pItem = rDflt.Create( r, nIVer );
/*?*/ 			GetItemSet().Put( *pItem );
/*?*/ 			delete pItem;
/*?*/ 			nPos += nLen;
/*?*/ 			if( r.Tell() != nPos )
/*?*/ 				r.Seek( nPos );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			GetItemSet().ClearItem( RES_LR_SPACE );
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/
/*N*/ 	// JP 11.12.96: die Zeichenvorlagen wurden um die HTML-Vorlagen erweitert,
/*N*/ 	// aber ohne ein wenig Platz zu lassen. Das wurde spaeter nachgepflegt.
/*N*/ 	if( SWG_CONDSTYLES == nVer && SFX_STYLE_FAMILY_CHAR == nFamily &&
/*N*/ 		RES_POOLCHR_JUMPEDIT < nId )
/*N*/ 	{
/*N*/ 		// dann den akt. Offset drauf addieren
/*N*/ 		nId = RES_POOLCHR_HTML_BEGIN + ( nId - RES_POOLCHR_JUMPEDIT ) - 1;
/*N*/ 	}
/*N*/ 	// In der 5.0 gibt es 10 Ueberschrift-Vorlagen. Die alte HTML-H6-Vorlage
/*N*/ 	// wird deshalb auf die neue Ueberschrift 6 gemappt.
/*N*/ 	if( SWG_TEN_HEADLINES > nVer )
/*N*/ 		nId = Sw3StringPool::ConvertFromOldPoolId( nId, SWG_LONGIDX );
/*N*/ }

/*N*/ void __EXPORT SwStyleSheet::Store( SvStream& r )
/*N*/ {
/*N*/ 	ASSERT( nVersion == r.GetVersion(),
/*N*/ 			"SwStyleSheet::Store: FF-Version != Stream-FF-Version" );
/*N*/
/*N*/ 	r << nId;
/*N*/ 	if( r.GetVersion() <= SOFFICE_FILEFORMAT_40 &&
/*N*/ 		nLevel != NO_NUMBERING && nLevel >= OLD_MAXLEVEL )
/*N*/ 	{
/*N*/ 		nLevel = NO_NUMBERING;
/*N*/ 	}
/*N*/ 	r << nLevel;
/*N*/
/*N*/ 	if( nVersion > SOFFICE_FILEFORMAT_31 )
/*N*/ 	{
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		UINT16 nType = RES_CONDTXTFMTCOLL == pFmt->Which() ? 1 : 0;
/*N*/ 		r << nType;
/*N*/ 		if( nType )
/*N*/ 		{
/*N*/ 			// Tabelle der ConditionTypes und der Vorlagen ausgeben:
/*N*/ 			const SwFmtCollConditions& rCColls = ((SwConditionTxtFmtColl*)pFmt)->
/*N*/ 													GetCondColls();
/*N*/ 			r << (UINT16)rCColls.Count();
/*N*/ 			for( USHORT n = 0; n < rCColls.Count(); ++n )
/*N*/ 			{
/*?*/ 				const SwCollCondition& rCColl = *rCColls[ n ];
/*?*/ 				r.WriteByteString( rCColl.GetTxtFmtColl()->GetName(),
/*?*/ 								   r.GetStreamCharSet() );
/*?*/ 				r << (UINT32) rCColl.GetCondition();
/*?*/
/*?*/ 				if( USRFLD_EXPRESSION & rCColl.GetCondition() )
/*?*/ 				{
/*?*/ 					String s( *rCColl.GetFldExpression() );
/*?*/ 					r.WriteByteString( s, r.GetStreamCharSet() );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					r << (UINT32) rCColl.GetSubCondition();
/*N*/ 			}
/*N*/ 		}
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 		// zusaetzliches Flag-Byte speichern
/*N*/ 		r << cFlags;
/*N*/
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 		if( nVersion > SOFFICE_FILEFORMAT_40 && (cFlags & 0x02) != 0 )
/*N*/ 		{
/*N*/ 			r << (UINT32)0;
/*N*/ 			if( pNumLRSpace )
/*N*/ 			{
/*?*/ 				USHORT nIVer = pNumLRSpace->GetVersion( (USHORT)nVersion );
/*?*/ 				if( nIVer != USHRT_MAX )
/*?*/ 				{
/*?*/ 					ULONG nPos = r.Tell();
/*?*/ 					r << (UINT16)nIVer;
/*?*/ 					pNumLRSpace->Store( r, nIVer );
/*?*/
/*?*/ 					ULONG nNewPos = r.Tell();
/*?*/ 					r.Seek( nPos-4UL );
/*?*/ 					r << (UINT32)(nNewPos - nPos);
/*?*/ 					r.Seek( nNewPos );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/ 	}
/*N*/ }

//FEATURE::CONDCOLL
/*N*/ USHORT __EXPORT SwStyleSheet::GetVersion() const
/*N*/ {
/*N*/ 	ASSERT( nVersion,
/*N*/ 			"SwStyleSheet::GetVersion: Fileformat-Version nicht gesetzt" );
/*N*/ 	switch( nVersion )
/*N*/ 	{
/*N*/ 	case SOFFICE_FILEFORMAT_31:
/*N*/ 		return 0;
/*N*/ 	case SOFFICE_FILEFORMAT_40:
/*N*/ 		return SWG_AUTOUPDATE_FLAG;
/*N*/ 	}
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 	return SWG_NUM_RELSPACE;
/*N*/ #else
/*N*/ 	return SWG_TEN_HEADLINES;
/*N*/ #endif
/*N*/ }
//FEATURE::CONDCOLL

/*N*/ SfxItemSet& __EXPORT SwStyleSheet::GetItemSet()
/*N*/ {
/*N*/ 	return aSet;
/*N*/ }

// Setzen des Parents: Wenn kein Parent angegeben ist, wird der Default
// gesetzt

/*N*/ void SwStyleSheet::ConnectParent( const String& rName )
/*N*/ {
/*N*/ 	SwFmt* pParent = NULL;
/*N*/ 	if( rName.Len() )
/*N*/ 		pParent = lcl_sw3io__FindFmt( rDoc, rName, nFamily );
/*N*/ 	else switch( nFamily )
/*N*/ 	{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR:
/*N*/ 			pParent = (*rDoc.GetCharFmts())[ 0 ]; break;
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*N*/ 			pParent = (*rDoc.GetFrmFmts())[ 0 ]; break;
/*N*/ 		case SFX_STYLE_FAMILY_PARA:
/*N*/ 			pParent = (*rDoc.GetTxtFmtColls())[ 0 ]; break;
/*N*/ 	}
/*N*/ 	if( pParent )
/*N*/ 		pFmt->SetDerivedFrom( pParent );
/*N*/ 	else
/*N*/ 		ASSERT( !this, "Parent nicht gefunden" );
/*N*/ }

// Setzen des Follows

/*N*/ void SwStyleSheet::ConnectFollow( const String& rName )
/*N*/ {
/*N*/ 	if( nFamily == SFX_STYLE_FAMILY_PARA )
/*N*/ 	{
/*N*/ 		SwTxtFmtColl* pFollow;
/*N*/ 		if( rName.Len() )
/*N*/ 			pFollow = rDoc.FindTxtFmtCollByName( rName );
/*N*/ 		else
/*?*/ 			pFollow = rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
/*N*/ 		if( pFollow )
/*N*/ 			GetColl()->SetNextTxtFmtColl( *pFollow );
/*N*/ 		else
/*N*/ 			ASSERT( !this, "Follow nicht gefunden" );
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////

// der nachfolgende ctor ist potentiell crashgefaehrlich, da die
// Sfx-Implementation den Pool als Referenz haelt.

/*N*/ SwStyleSheetPool::SwStyleSheetPool( SwDoc& r, SfxItemPool& rp, long nFFVersion,
/*N*/ 								 	Sw3Fmts *pConvFmts )
/*N*/ 				: SfxStyleSheetBasePool( rp ), rDoc( r ),
/*N*/ 				  nExpFFVersion( nFFVersion ),
/*N*/ 				  pConvToSymbolFmts( pConvFmts )
/*N*/ {}

/*N*/ __EXPORT SwStyleSheetPool::~SwStyleSheetPool()
/*N*/ {
/*N*/ 	// wird von SFX-DLL gerufen!
/*N*/ }

// Laden und Speichern eines Pools

/*N*/ BOOL SwStyleSheetPool::Load( SvStream& s, BOOL bOverWrite, USHORT eMask )
/*N*/ {
/*N*/ 	// Der Stream koennte auch leer sein, wenn eine neue Textbausteindatei
/*N*/ 	// eingelesen wird.
/*N*/ 	BOOL bRes = TRUE;
/*N*/ 	USHORT n;
/*N*/ 	s.Seek( 0L );
/*N*/ 	s >> n;
/*N*/ 	if( s.GetError() != SVSTREAM_OK )
/*N*/ 		bRes = FALSE;
/*N*/ 	if( !s.IsEof() )
/*N*/ 	{
/*N*/ 		s.Seek( 0L );
/*N*/ 		rPool.Load( s );
/*N*/ 		if( s.GetError() != SVSTREAM_OK )
/*N*/ 			bRes = FALSE;
/*N*/ 		bRes = SfxStyleSheetBasePool::Load( s );
/*N*/ 		SetSearchMask( SFX_STYLE_FAMILY_ALL );
/*N*/ 		if( bRes )
/*N*/ 			CopyToDoc( bOverWrite, eMask );
/*N*/ 	}
/*N*/ 	return bRes;
/*N*/ }

const int RES_POOLCOLL_HTML_LISTING_40_USER = 0x3002 | USER_FMT;
const int RES_POOLCOLL_HTML_XMP_40_USER = 0x3003 | USER_FMT;

/*N*/ BOOL SwStyleSheetPool::Store( SvStream& s, BOOL bUsed )
/*N*/ {
/*N*/ 	ASSERT( nExpFFVersion == s.GetVersion(),
/*N*/ 			"SwStyleSheetPool::Store: FF-Version != Stream-FF-Version" );
/*N*/
/*N*/ 	CopyFromDoc( bUsed );
/*N*/ 	SetSearchMask( SFX_STYLE_FAMILY_ALL );
/*N*/
/*N*/ 	rPool.SetFileFormatVersion( (USHORT)nExpFFVersion );
/*N*/
/*N*/ 	//JP 11.06.97: laut ChangesMail muss das vorm Speichern gesetzt werden.
/*N*/ 	if( SOFFICE_FILEFORMAT_31 == nExpFFVersion )
/*N*/ 		rPool.SetStoringRange( 1, 60 );
/*N*/
/*N*/ 	rPool.Store( s );
/*N*/
/*N*/ 	return SfxStyleSheetBasePool::Store( s, bUsed );
/*N*/ }

// Auffuellen eines Pools mit allen am Doc definierten Vorlagen

/*N*/ void SwStyleSheetPool::CopyFromDoc( BOOL bUsed )
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	// Uebernahme der benannten Formate
/*N*/ 	// Die Defaultvorlagen werden nicht uebernommen
/*N*/ 	// Zeichenvorlagen: alle uebernehmen
/*N*/ 	// Rahmenvorlagen: nur die Nicht-Auto-Vorlagen uebernehmen
/*N*/ 	const SwFmt* pFmt;
/*N*/ 	USHORT nArrLen = rDoc.GetCharFmts()->Count();
        USHORT i=0;
/*N*/ 	for( i = 0; i < nArrLen; i++ )
/*N*/ 	{
/*N*/ 		pFmt = (*rDoc.GetCharFmts())[ i ];
/*N*/ 		if( ( !bUsed || rDoc.IsUsed( *pFmt ) ) && !pFmt->IsDefault() )
/*N*/ 			Add( *pFmt, SFX_STYLE_FAMILY_CHAR );
/*N*/ 	}
/*N*/ 	nArrLen = rDoc.GetFrmFmts()->Count();
/*N*/ 	for( i = 0; i < nArrLen; i++ )
/*N*/ 	{
/*N*/ 		pFmt = (*rDoc.GetFrmFmts())[ i ];
/*N*/ 		if( ( !bUsed || rDoc.IsUsed( *pFmt ) )
/*N*/ 		  && !pFmt->IsDefault() && !pFmt->IsAuto() )
/*N*/ 			Add( *pFmt, SFX_STYLE_FAMILY_FRAME );
/*N*/ 	}
/*N*/ 	// Uebernahme der Absatzvorlagen
/*N*/ 	nArrLen = rDoc.GetTxtFmtColls()->Count();
/*N*/ 	for( i = 0; i < nArrLen; i++ )
/*N*/ 	{
/*N*/ 		const SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];
/*N*/ 		if( ( !bUsed || rDoc.IsUsed( *pColl ) ) && !pColl->IsDefault() )
/*N*/ 			Add( *pColl, SFX_STYLE_FAMILY_PARA );
/*N*/ 	}
/*N*/ 	if( bUsed )
/*N*/ 	{
/*N*/ 		//JP 30.03.99: falls die Follows nicht angewendet werden, so muessen
/*N*/ 		//				sie doch kopiert werden!
/*?*/ 		for( i = 0; i < nArrLen; i++ )
/*?*/ 		{
/*?*/ 			const SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];
/*?*/ 			if( pColl != &pColl->GetNextTxtFmtColl() &&
/*?*/ 				!Find( pColl->GetNextTxtFmtColl().GetName(),
/*?*/ 					   SFX_STYLE_FAMILY_PARA ))
/*?*/ 				Add( pColl->GetNextTxtFmtColl(), SFX_STYLE_FAMILY_PARA );
/*?*/ 		}
/*N*/ 	}
/*N*/ }

// Hinzufuegen eines neuen StyleSheets.
// Es muss mind. ein Bit in der Maske gesetzt werden, da die Suchroutinen
// der Basisklasse ein AND auf die Maske machen!
// Der Set wird auch temporaer kopiert.

#define DUMMYBITS	0x0001

/*N*/ void SwStyleSheetPool::Add( const SwFmt& rFmt, SfxStyleFamily eFam )
/*N*/ {
/*N*/ 	SwStyleSheet& r = (SwStyleSheet&) Make( rFmt.GetName(), eFam, DUMMYBITS );
/*N*/
/*N*/ 	// SW31-Export oder nicht
/*N*/ 	r.nVersion = nExpFFVersion;
/*N*/
/*N*/ 	// ItemSet
/*N*/ 	r.GetItemSet().Put( rFmt.GetAttrSet() );
/*N*/
/*N*/ 	// es muss natuerlich der Pointer vom kopierten geschrieben werden !!!
/*N*/ 	r.pSet = &r.GetItemSet();
/*N*/ 	r.bMySet = FALSE;
/*N*/
/*N*/ 	// Members setzen
/*N*/ 	ASSERT( nExpFFVersion, "SwStylePool::Add: FF-Version ist nicht gesetzt" );
/*N*/ 	if( nExpFFVersion <= SOFFICE_FILEFORMAT_40 )
/*N*/ 		r.nId = Sw3StringPool::ConvertToOldPoolId( rFmt.GetPoolFmtId(),
/*N*/ 												   nExpFFVersion );
/*N*/ 	else
/*N*/ 		r.nId  = rFmt.GetPoolFmtId();
/*N*/ 	r.pFmt = (SwFmt*) &rFmt;
/*N*/ 	if( r.nId & USER_FMT )
/*N*/ 		r.nMask |= SFXSTYLEBIT_USERDEF;
/*N*/ 	if( rDoc.IsUsed( rFmt ) )
/*N*/ 		r.nMask |= SFXSTYLEBIT_USED;
/*N*/ 	if( rFmt.GetPoolHlpFileId() != UCHAR_MAX )
/*?*/ 		r.aHelpFile = *rDoc.GetDocPattern( rFmt.GetPoolHlpFileId() );
/*N*/ 	r.nHelpId = rFmt.GetPoolHelpId();
/*N*/ 	if( rFmt.IsAutoUpdateFmt() )
/*N*/ 		r.cFlags |= 0x01;
/*N*/
/*N*/ 	// Parent und Follow
/*N*/ 	SwFmt* pFmt = rFmt.DerivedFrom();
/*N*/ 	// Parent-Namen nur uebernehmen, wenn kein Default
/*N*/ 	if( pFmt && !pFmt->IsDefault() )
/*N*/ 		r.aParent = rFmt.DerivedFrom()->GetName();
/*N*/ 	if( r.nFamily & SFX_STYLE_FAMILY_PARA )
/*N*/ 	{
/*N*/ 		r.nLevel = r.GetColl()->GetOutlineLevel();
/*N*/ #ifdef NUM_RELSPACE
/*N*/ 		if( NO_NUMBERING != r.nLevel )
/*N*/ 		{
/*N*/ 			// Beim Export in das 3.1- oder 4.0-Format wird die Vorlage noch
/*N*/ 			// an die Outline Numerierung angepasst. Der linke Einzug wird
/*N*/ 			// aber nur veraendert, wenn er nicht relativ ist.
/*N*/ 			// Beim Schreiben des 5.0/5.1-Formats werden die Einzuege immer
/*N*/ 			// veraendert und das Original-Item gemerkt.
/*N*/ 			// Beim 3.1- und 4.0-Export werden auch Vorlagen angepasst, deren
/*N*/ 			// Outline-Level >= 5 ist.
/*N*/ 			const SwNumRule *pOutline = rDoc.GetOutlineNumRule();
/*N*/ 			BYTE nLevel = GetRealLevel( r.nLevel );
/*N*/ 			if( pOutline )
/*N*/ 			{
/*N*/ 				const SwNumFmt& rNumFmt = pOutline->Get( nLevel );
/*N*/ 				const SvxLRSpaceItem& rLRSpace = rFmt.GetLRSpace();
/*N*/ 				USHORT nOldLSpace = rLRSpace.GetTxtLeft();
/*N*/ 				USHORT nLSpace = rLRSpace.GetTxtLeft();
/*N*/ 				BOOL bNonProp = FALSE;
/*N*/ 				if( rLRSpace.GetPropLeft() == 100U ||
/*N*/ 					nExpFFVersion > SOFFICE_FILEFORMAT_40 )
/*N*/ 				{
/*N*/ 					if( pOutline->IsAbsSpaces() )
/*?*/ 						nLSpace = rNumFmt.GetAbsLSpace();
/*N*/ 					else
/*N*/ 						nLSpace += rNumFmt.GetAbsLSpace();
/*N*/ 					bNonProp = TRUE;
/*N*/ 				}
/*N*/ 				if( nLSpace != rLRSpace.GetTxtLeft() ||
/*N*/ 					rNumFmt.GetFirstLineOffset() !=
/*N*/ 								rLRSpace.GetTxtFirstLineOfst() )
/*N*/ 				{
/*N*/ 					if( nExpFFVersion > SOFFICE_FILEFORMAT_40 )
/*N*/ 					{
/*N*/ 						r.cFlags |= 0x02;
/*N*/ 						if( SFX_ITEM_SET == r.GetItemSet().
/*N*/ 									GetItemState( RES_LR_SPACE, FALSE ) )
/*N*/ 						{
/*?*/ 							r.pNumLRSpace = new SvxLRSpaceItem( rLRSpace );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					SvxLRSpaceItem aLRSpace( rLRSpace );
/*N*/ 					aLRSpace.SetTxtFirstLineOfst(
/*N*/ 							rNumFmt.GetFirstLineOffset() );
/*N*/ 					if( bNonProp )
/*N*/ 						aLRSpace.SetTxtLeft( nLSpace );
/*N*/ 					r.GetItemSet().Put( aLRSpace );
/*N*/
/*N*/ 					if( nExpFFVersion <= SOFFICE_FILEFORMAT_40 &&
/*N*/ 						nLSpace != nOldLSpace )
/*N*/ 					{
/*N*/ 						const SfxPoolItem* pItem;
/*N*/ 						if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
/*N*/ 										RES_PARATR_TABSTOP, TRUE, &pItem ))
/*N*/ 						{
/*N*/ 							SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
/*N*/ 							lcl_sw3io__ConvertNumTabStop( aTStop,
/*N*/ 											(long)nOldLSpace - (long)nLSpace );
/*N*/ 							r.GetItemSet().Put( aTStop );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/
/*N*/ 		const SwTxtFmtColl& rFollow = r.GetColl()->GetNextTxtFmtColl();
/*N*/ 		if( !rFollow.IsDefault() )
/*N*/ 			r.aFollow = rFollow.GetName();
/*N*/ 	}
/*N*/
/*N*/ }

// Kopieren aller StyleSheets in das Doc
// Ggf. werden die StyleSheets neu erzeugt

sal_Bool lcl_sw3io_isStarSymbolFontItem( const SvxFontItem& rFontItem );
extern sal_Char const SW_CONSTASCII_DECL( sHTML_xmp, "XMP" );
extern sal_Char const SW_CONSTASCII_DECL( sHTML_listing, "LISTING" );

sal_Char const SW_CONSTASCII_DEF( sHTML_xmp, "XMP" );
sal_Char const SW_CONSTASCII_DEF( sHTML_listing, "LISTING" );

/*N*/ void SwStyleSheetPool::CopyToDoc( BOOL bOverwrite, USHORT eMask )
/*N*/ {
/*N*/ 	SwFmt* pFmt;
        SwStyleSheet* p;
/*N*/ 	for( p = (SwStyleSheet*) aStyles.First(); p;
/*N*/ 		 p = (SwStyleSheet*) aStyles.Next() )
/*N*/ 	{
/*N*/ 		if( !p->pFmt &&	(eMask & p->nFamily) )
/*N*/ 		{
/*N*/ 			BOOL bNewFmt = FALSE;
/*N*/
/*N*/ 			if( p->nVersion < SWG_TEN_HEADLINES &&
/*N*/ 				p->nFamily & SFX_STYLE_FAMILY_PARA &&
/*N*/ 				(RES_POOLCOLL_HTML_LISTING_40_USER == p->nId ||
/*N*/ 				 RES_POOLCOLL_HTML_XMP_40_USER == p->nId) )
/*N*/ 			{
/*?*/ 				// Listing und XMP werden umbenannt, aber nur, wenn
/*?*/ 				// sie nicht schon im Pool vorhanden sind
/*?*/ 				String aNewName( String::CreateFromAscii(
/*?*/ 						RES_POOLCOLL_HTML_XMP_40_USER == p->nId
/*?*/ 							? sHTML_xmp
/*?*/ 							: sHTML_listing ) );
/*?*/ 				BOOL bPresent = Find( aNewName, p->nFamily ) != 0;
/*?*/
/*?*/ 				// Umbennnen. Das muss auch sein, wenn eine Vorlage
/*?*/ 				// mit dem neuen Namen existiert, damit die abgeleiteten
/*?*/ 				// und Folgevorlagen umgehaengt wenrden.
/*?*/ 				Rename( p->GetName(), aNewName, p->nFamily );
/*?*/ 				aStyles.First();
/*?*/
/*?*/ 				if( bPresent )
/*?*/ 				{
/*?*/ 					// Vorlage von nun an ignorieren
/*?*/ 					p->nFamily = (SfxStyleFamily)0;
/*?*/ 					continue;
/*?*/ 				}
/*?*/
/*?*/ 				// Benutzter-Vorlage ohne ID daraus machen.
/*?*/ 				p->nId |= (USHRT_MAX &
/*?*/ 								~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID));
/*?*/ 				p->nMask |= SFXSTYLEBIT_USERDEF;
/*N*/ 			}
/*N*/
/*N*/ 			pFmt = lcl_sw3io__FindFmt( rDoc, p->GetName(), p->nFamily );
/*N*/ 			if( !pFmt )
/*N*/ 			{
/*N*/ 				bNewFmt = TRUE;
/*N*/ 				BOOL bIsUserDefined = p->IsUserDefined();
/*N*/ 				if( !bIsUserDefined )
/*N*/ 				{
/*N*/ 					//JP 31.01.96:
/*N*/ 					// 		dann teste mal ob die Ids im aktuellen PoolId-
/*N*/ 					//		Bereich sind. Wenn nicht, dann muss sie als
/*N*/ 					//		BenutzerVorlage angelegt werden!!!!!!!
/*N*/ 					USHORT nId = p->nId;
/*N*/ 					if( p->nFamily & SFX_STYLE_FAMILY_CHAR )
/*N*/ 						bIsUserDefined = !(
/*N*/ 								(RES_POOLCHR_NORMAL_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCHR_NORMAL_END) ||
/*N*/ 								(RES_POOLCHR_HTML_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCHR_HTML_END));
/*N*/ 					else if( p->nFamily & SFX_STYLE_FAMILY_FRAME )
/*N*/ 						bIsUserDefined = RES_POOLFRM_BEGIN > nId ||
/*N*/ 											nId >= RES_POOLFRM_END;
/*N*/ 					else
/*N*/ 						bIsUserDefined = !(
/*N*/ 								(RES_POOLCOLL_TEXT_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_TEXT_END) ||
/*N*/ 								(RES_POOLCOLL_LISTS_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_LISTS_END) ||
/*N*/ 								(RES_POOLCOLL_EXTRA_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_EXTRA_END) ||
/*N*/ 								(RES_POOLCOLL_REGISTER_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_REGISTER_END) ||
/*N*/ 								(RES_POOLCOLL_DOC_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_DOC_END) ||
/*N*/ 								(RES_POOLCOLL_HTML_BEGIN <= nId &&
/*N*/ 										nId < RES_POOLCOLL_HTML_END));
/*N*/
/*N*/ 					// es war eine Poolvorlage die wir aber noch nicht kennen
/*N*/ 					if( bIsUserDefined )
/*N*/ 					{
/*N*/ 						p->nId |= (USHRT_MAX &
/*N*/ 									~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID));
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Die Vorlage ist Benutzer-definiert, aber
/*N*/ 					// vielleicht gibt es ja mitlerweile eine
/*N*/ 					// Pool-Vorlage mit dem gleichen Namen.
/*N*/ 					// Dann nehmen wir die Pool-Vorlage
/*N*/ 					// statt der Benutzer-Vorlage, aber auch nur
/*N*/ 					// dann, wenn die Pool-Vorlage nicht auch noch
/*N*/ 					// eingelesen wird. Ist das der Fall, wird
/*N*/ 					// die Benutzer-Vorlage ignoriert.
/*N*/ 					SwGetPoolIdFromName eNameType;
/*N*/ 					if( p->nFamily & SFX_STYLE_FAMILY_CHAR )
/*N*/ 						eNameType = GET_POOLID_CHRFMT;
/*N*/ 					else if( p->nFamily & SFX_STYLE_FAMILY_FRAME )
/*N*/ 						eNameType = GET_POOLID_FRMFMT;
/*N*/ 					else
/*N*/ 						eNameType = GET_POOLID_TXTCOLL;
/*N*/ 					USHORT nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( p->GetName(),
/*N*/ 													  eNameType );
/*N*/ 					if( nPoolId != USHRT_MAX )
/*N*/ 					{
/*N*/ 						if( FindByPoolId( nPoolId ) )
/*N*/ 						{
/*N*/ 							// Vorlage von nun an ignorieren
/*?*/ 							p->nFamily = (SfxStyleFamily)0;
/*?*/ 							continue;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							bIsUserDefined = FALSE;
/*N*/ 							p->nId = nPoolId;
/*N*/ 						}
/*N*/ 					}
/*N*/
/*N*/ 				}
/*N*/
/*N*/ 				if( bIsUserDefined )
/*N*/ 				{
/*N*/ 					// Benutzerdefinierte Formate werden am Doc neu eingerichtet.
/*N*/ 					// PageDescs werden z.Zt. noch nicht hier gepflegt!
/*N*/ 					if( p->nFamily & SFX_STYLE_FAMILY_CHAR )
/*N*/ 					{
/*N*/ 						pFmt = rDoc.MakeCharFmt( p->GetName(), NULL );
/*N*/ 						pFmt->SetAuto( FALSE );
/*N*/ 					}
/*N*/ 					else if( p->nFamily & SFX_STYLE_FAMILY_FRAME )
/*N*/ 					{
/*N*/ 						pFmt = rDoc.MakeFrmFmt( p->GetName(), NULL );
/*N*/ 						pFmt->SetAuto( FALSE );
/*N*/ 					}
/*N*/ 					else if( p->pCondColls )
/*N*/ 					{
/*NBFF*/ 						SwTxtFmtColl* pDer = (*rDoc.GetTxtFmtColls())[ 0 ];
/*NBFF*/ 						pFmt = rDoc.MakeCondTxtFmtColl( p->GetName(), pDer );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						SwTxtFmtColl* pDer = (*rDoc.GetTxtFmtColls())[ 0 ];
/*N*/ 						pFmt = rDoc.MakeTxtFmtColl( p->GetName(), pDer );
/*N*/ 					}
/*N*/
/*N*/ 					// Bei benutzerdefinierten Vorlagen werden die Help-Ids
/*N*/ 					// im Doc gesetzt.
/*N*/ 					pFmt->SetPoolFmtId( p->nId );
/*N*/ 					if( p->aHelpFile.Len() )
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 /*?*/ 						pFmt->SetPoolHlpFileId
/*N*/ 					pFmt->SetPoolHelpId( (USHORT)p->nHelpId );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Poolformate werden am Doc angefordert.
/*N*/ 					// PageDescs werden z.Zt. noch nicht hier gepflegt!
/*N*/ 					if( p->nFamily & SFX_STYLE_FAMILY_CHAR )
/*N*/ 					{
/*N*/ 						USHORT nStyleCnt = rDoc.GetCharFmts()->Count();
/*N*/ 						pFmt = rDoc.GetCharFmtFromPool( p->nId );
/*N*/ 						bNewFmt = nStyleCnt != rDoc.GetCharFmts()->Count();
/*N*/ 					}
/*N*/ 					else if( p->nFamily & SFX_STYLE_FAMILY_FRAME )
/*N*/ 					{
/*N*/ 						USHORT nStyleCnt = rDoc.GetFrmFmts()->Count();
/*N*/ 						pFmt = rDoc.GetFrmFmtFromPool( p->nId );
/*N*/ 						bNewFmt = nStyleCnt != rDoc.GetFrmFmts()->Count();
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						USHORT nStyleCnt = rDoc.GetTxtFmtColls()->Count();
/*N*/ 						pFmt = rDoc.GetTxtCollFromPool( p->nId );
/*N*/ 						bNewFmt = nStyleCnt != rDoc.GetTxtFmtColls()->Count();
/*N*/ 					}
/*N*/ 					// Hier werden die Help-Ids aus der Vorlage uebernommen!
/*N*/ 					if( pFmt->GetPoolHlpFileId() != UCHAR_MAX )
/*?*/ 						p->aHelpFile = *rDoc.GetDocPattern( pFmt->GetPoolHlpFileId() );
/*N*/ 					p->nHelpId = pFmt->GetPoolHelpId();
/*N*/
/*N*/ 					// MIB 27.11.96: "kleine" Optimierung: Nichts umbenennen,
/*N*/ 					// wo nichts umzubenennen ist.
/*N*/ 					if( p->GetName() != pFmt->GetName() )
/*N*/ 					{
/*N*/ 						Rename( p->GetName(), pFmt->GetName(), p->GetFamily() );
/*N*/ 						// Da Rename() selbst eine Schleife hat, muss von
/*N*/ 						// vorne gearbeitet werden.
/*N*/ 						aStyles.First();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if( p->IsUserDefined() )
/*N*/ 			{
/*N*/ 				// Wenn aus dem selben Dokument eine Pool-Vorlage
/*N*/ 				// mit gleichem Namen gelesen wurde, dann wird die
/*N*/ 				// Benutzer-Vorlage ignoriert.
/*?*/ 				USHORT nPoolId = pFmt->GetPoolFmtId();
/*?*/ 				SwStyleSheet *pTmp;
/*?*/ 				if( nPoolId != p->nId && (nPoolId & USER_FMT) == 0 &&
/*?*/ 					(pTmp = FindByPoolId( nPoolId )) != 0 &&
/*?*/ 					pTmp->pFmt == pFmt )
/*?*/ 				{
/*?*/ 					// Vorlage von nun an ignorieren
/*?*/ 					p->nFamily = (SfxStyleFamily)0;
/*?*/ 					continue;
/*?*/ 				}
/*N*/ 			}
/*N*/ 			p->pFmt = pFmt;
/*N*/ 			/* Attribute nur uebernehmen, wenn bOverwrite TRUE ist */
/*N*/ 			if( bOverwrite || bNewFmt )
/*N*/ 			{
/*N*/ 				p->bNew = TRUE;
/*N*/ 				pFmt->ResetAllAttr();	// die neue Vorlage kann auch weniger
/*N*/ 										// Attribute enthalten
/*N*/ 				((SwAttrSet&) pFmt->GetAttrSet()).Put( p->GetItemSet() );
/*N*/ 				SwTxtFmtColl* pColl = p->GetColl();
/*N*/ 				if( pColl )
/*N*/ 				{
/*N*/ 					((SwAttrSet&) pColl->GetAttrSet()).SetModifyAtAttr( pColl );
/*N*/
/*N*/ 					// Bei Einfuegen von Vorlagen muss ggf. der Outline-Level
/*N*/ 					// geloescht werden, wenn bereits eine Vorlage mit diesem
/*N*/ 					// Level existiert.
/*N*/
/*N*/ 					BYTE nLevel = p->GetNumLevel();
/*N*/ 					if( !bOverwrite && nLevel != NO_NUMBERING )
/*N*/ 					{
/*?*/ 						BYTE nRealLevel = GetRealLevel(nLevel);
/*?*/ 						USHORT nArrLen = rDoc.GetTxtFmtColls()->Count();
/*?*/ 						for( USHORT i=0; i<nArrLen; i++ )
/*?*/ 						{
/*?*/ 							SwTxtFmtColl* pCur =
/*?*/ 								(*rDoc.GetTxtFmtColls())[i];
/*?*/ 							BYTE nCurLevel = pCur->GetOutlineLevel();
/*?*/ 							if( nCurLevel != NO_NUMBERING &&
/*?*/ 								GetRealLevel(nCurLevel) == nRealLevel )
/*?*/ 							{
/*?*/ 								if( pColl != pCur )
/*?*/ 									nLevel = NO_NUMBERING;
/*?*/ 								break;
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 					pColl->SetOutlineLevel( nLevel );
/*N*/ 				}
/*N*/ 				pFmt->SetAutoUpdateFmt( (p->cFlags & 0x01) != 0 );
/*N*/ 			}
/*N*/ 			const SfxPoolItem *pItem;
/*N*/ 			if( SFX_ITEM_SET == p->GetItemSet().GetItemState(
/*N*/ 						RES_CHRATR_FONT, sal_False, &pItem ) &&
/*N*/ 				RTL_TEXTENCODING_SYMBOL ==
/*N*/ 						((const SvxFontItem *)pItem)->GetCharSet() )
/*N*/ 			{
/*N*/ 				BOOL bBats = ((const SvxFontItem *)pItem)->GetFamilyName().EqualsIgnoreCaseAscii( "StarBats", 0, sizeof("StarBats")-1 );
/*N*/ 				BOOL bMath = ((const SvxFontItem *)pItem)->GetFamilyName().EqualsIgnoreCaseAscii( "StarMath", 0, sizeof("StarMath")-1 );
/*N*/
/*N*/ 				if( bBats || bMath )
/*N*/ 				{
/*N*/ 					if( p->bNew )
/*N*/ 					{
/*N*/ 						// Allways convert font to StarSymbol
/*N*/ 						pConvToSymbolFmts ->Insert( pFmt, SW3IO_CONV_TO_SYMBOL );
/*N*/ 					}
/*?*/ 					else if( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(
/*?*/ 							 RES_CHRATR_FONT, sal_True, &pItem ) &&
/*?*/ 							 lcl_sw3io_isStarSymbolFontItem( *(const SvxFontItem *)pItem ) )
/*?*/ 					{
/*?*/ 						pConvToSymbolFmts ->Insert( pFmt, bBats ? SW3IO_CONV_FROM_BATS : SW3IO_CONV_FROM_MATH  );
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Nun sind alle Vorlagen drin, sie koennen also verbunden werden
/*N*/ 	for( p = (SwStyleSheet*) aStyles.First(); p;
/*N*/ 		 p = (SwStyleSheet*) aStyles.Next() )
/*N*/ 	{
/*N*/ 		if( p->bNew )
/*N*/ 		{
/*N*/ 			p->ConnectParent( p->GetParent() );
/*N*/ 			p->ConnectFollow( p->GetFollow() );
/*N*/ 			// das Format hat sich geaendert!
/*N*/ 			SwFmtChg aHint( p->pFmt );
/*N*/ 			p->pFmt->Modify( &aHint, &aHint );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	// dann koennen auch alle bedingten Vorlagen verbunden werden
/*N*/ 	for( p = (SwStyleSheet*) aStyles.First(); p;
/*N*/ 		 p = (SwStyleSheet*) aStyles.Next() )
/*N*/ 	{
/*N*/ 		if( p->bNew && p->pCondColls )
/*N*/ 			for( USHORT n = 0; n < p->pCondColls->Count(); ++n )
/*N*/ 			{
/*?*/ 				SwStyleCondColl* pCColl = (*p->pCondColls)[n];
/*?*/ 				SwTxtFmtColl* pDColl = rDoc.FindTxtFmtCollByName( pCColl->sColl );
/*?*/ 				if( pDColl )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ((SwConditionTxtFmtColl*)p->pFmt)->InsertCondition(
/*?*/ 				}
/*?*/ 				else
/*?*/ 					ASSERT( !this, "Collection nicht gefunden" );
/*N*/ 			}
/*N*/ 	}
/*N*/ //FEATURE::CONDCOLL
/*N*/ }

////////////////////////////////////////////////////////////////////////////

// Faktorei

/*N*/ SfxStyleSheetBase* __EXPORT SwStyleSheetPool::Create( const String& rName,
/*N*/ 											 SfxStyleFamily eFam, USHORT nMask)
/*N*/ {
/*N*/ 	return new SwStyleSheet( rName, *this, eFam, nMask);
/*N*/ }


// Umbenennen eines bestimmten Strings innerhalb einer Family

/*N*/ void SwStyleSheetPool::Rename
/*N*/ 	( const String& rOld, const String& rNew, SfxStyleFamily eFam )
/*N*/ {
/*N*/ 	// da mit Referenzen gearbeitet wird, muss der Name zwischen-
/*N*/ 	// gelagert werden!
/*N*/ 	String aOld( rOld );
/*N*/ 	for( SwStyleSheet* p = (SwStyleSheet*) aStyles.First(); p;
/*N*/ 		 p = (SwStyleSheet*) aStyles.Next() )
/*N*/ 	{
/*N*/ 		if( p->GetFamily() == eFam )
/*N*/ 		{
/*N*/ 			if( p->GetName() == aOld )
/*N*/ 				p->SetNewName( rNew );
/*N*/ 			if( p->GetParent() == aOld )
/*N*/ 				p->SetNewParent( rNew );
/*N*/ 			if( p->GetFollow() == aOld )
/*N*/ 				p->SetNewFollow( rNew );
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 			if( p->pCondColls )
/*N*/ 			{
/*?*/ 				SwStyleCondColl* pCColl;
/*?*/ 				for( USHORT n = 0; n < p->pCondColls->Count(); ++n )
/*?*/ 					if( ( pCColl = (*p->pCondColls)[n] )->sColl == aOld )
/*?*/ 						pCColl->sColl = rNew;
/*N*/ 			}
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SwStyleSheet *SwStyleSheetPool::FindByPoolId( USHORT nPoolId )
/*N*/ {
/*N*/ 	SwStyleSheet *pS = 0;
/*N*/ 	ULONG nCount = aStyles.Count();
/*N*/ 	for( ULONG i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		SwStyleSheet *p = (SwStyleSheet *)aStyles.GetObject( i );
/*N*/ 		if( p->nId == nPoolId )
/*N*/ 		{
/*?*/ 			pS = p;
/*?*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return pS;
/*N*/ }

///////////////////////////////////////////////////////////////////////////

// Laden und speichern aller StyleSheets
// Der Code liegt hier, um ein HXX fuer den lokalen Pool einzusparen

/*N*/ void Sw3IoImp::LoadStyleSheets( BOOL bNew )
/*N*/ {
/*N*/ 	ASSERT( !HasRecSizes(), "Hier darf es noch keine RecSizes geben" );
/*N*/ 	// Bisher wurde allenfalls der Drawing-Layer gelesen. Deshalb
/*N*/ 	// kann es hier noch gar keine RecSizes geben. Besser ist aber besser ...
/*N*/ 	if( HasRecSizes() )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		FlushRecSizes();
/*N*/
/*N*/ 	SfxItemPool *pTmp = pDoc->GetAttrPool().GetSecondaryPool();
/*N*/ 	pDoc->GetAttrPool().SetSecondaryPool( 0 );
/*N*/ 	SfxItemPool* pPool = pDoc->GetAttrPool().Clone();
/*N*/ 	pDoc->GetAttrPool().SetSecondaryPool( pTmp );
/*N*/
/*N*/ 	ASSERT( !pConvToSymbolFmts, "ConvToSymbol array exists" );
/*N*/ 	pConvToSymbolFmts = new Sw3Fmts;
/*N*/ 	SwStyleSheetPool* p = new SwStyleSheetPool( *pDoc, *pPool, 0, pConvToSymbolFmts );
/*N*/ 	pStyles->SetBufferSize( SW3_BSR_STYLES );
/*N*/ 	pStrm = pStyles;
/*N*/ 	USHORT eMask = bNormal ? SFX_STYLE_FAMILY_ALL : 0;
/*N*/ 	if( !bNormal )
/*N*/ 	{
/*?*/ 		if( bFrmFmts )
/*?*/ 			eMask |= SFX_STYLE_FAMILY_FRAME;
/*?*/ 		if( bTxtColls )
/*?*/ 			eMask |= SFX_STYLE_FAMILY_PARA;
/*?*/ 		if( bCharFmts )
/*?*/ 			eMask |= SFX_STYLE_FAMILY_CHAR;
/*N*/ 	}
/*N*/
/*N*/ 	nRes = p->Load( *pStyles, bNew, eMask ) ? 0 : ERR_SWG_READ_ERROR;
/*N*/ 	pStyles->SetBufferSize( 0 );
/*N*/
/*N*/ 	// JP 09.06.95: die Pool-Defaults muessen beim normalen Einlesen
/*N*/ 	//				aus temp Pool mitgenommen werden !!
/*N*/ 	if( !nRes && !bInsert )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pCpyItem;
/*N*/ 		for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
/*N*/ 			if( 0 != ( pCpyItem = pPool->GetPoolDefaultItem( nWh ) ) )
/*N*/ 				pDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );
/*N*/ 	}
/*N*/
/*N*/ 	// alle Attribute, die auf irgendwelche Vorlagen verweisen hier verbinden
/*N*/ 	// (Attribute: DropCaps)
/*N*/ 	{
/*N*/ 		BOOL bReadStrPool = FALSE;
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		USHORT nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_PARATR_DROP );
/*N*/
/*N*/ 		for( USHORT n = 0; n < nMaxItems; ++n )
/*N*/ 			if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_PARATR_DROP, n ) )
/*N*/ 				&& USHRT_MAX != ((SwFmtDrop*)pItem)->GetReadFmt() )
/*N*/ 			{
/*?*/ 				if( !bReadStrPool )
/*?*/ 				{
/*?*/ 					bReadStrPool = TRUE;
/*?*/ 					if( pPageStyles.Is() )
/*?*/ 					{
/*?*/ 						pPageStyles->Seek( 0L );
/*?*/ 						pPageStyles->SetBufferSize( SW3_BSR_PAGESTYLES );
/*?*/ 						SvStream* pOld = pStrm;
/*?*/ 						pStrm = pPageStyles;
/*?*/ 						if( !nRes )
/*?*/ 						{
/*?*/ 							if( Peek() != SWG_STRINGPOOL )
/*?*/ 								InHeader( TRUE );
/*?*/ 							if( Good() && Peek() == SWG_STRINGPOOL )
/*?*/ 								InStringPool( SWG_STRINGPOOL, aStringPool );
/*?*/ 						}
/*?*/ 						pStrm = pOld;
/*?*/ 						CheckIoError( pPageStyles );
/*?*/ 						pPageStyles->SetBufferSize( 0 );
/*?*/ 					}
/*?*/ 				}
/*?*/ 				((SwFmtDrop*)pItem)->SetCharFmt( (SwCharFmt*) FindFmt(
/*?*/ 						((SwFmtDrop*)pItem)->GetReadFmt(), SWG_CHARFMT ) );
/*N*/ 			}
/*N*/
/*N*/ 		// fix #41308#: Wenn nur Vorlagen geladen werden, muessen die
/*N*/ 		// Font-Items noch den richten Charset verpasst bekommen. Es sei
/*N*/ 		// denn, es werden auch Seitenvorlagen geladen. Dann passiert das
/*N*/ 		// automatisch im Cleanup
/*N*/ 		if( !bNormal && !bPageDescs )
/*N*/ 		{
/*?*/ 			BOOL bSrcSetValid = bReadStrPool;
/*?*/ 			if( !bSrcSetValid && pPageStyles.Is() )
/*?*/ 			{
/*?*/ 				pPageStyles->Seek( 0L );
/*?*/ 				pPageStyles->SetBufferSize( SW3_BSR_PAGESTYLES );
/*?*/ 				SvStream* pOld = pStrm;
/*?*/ 				pStrm = pPageStyles;
/*?*/ 				if( !nRes && Peek() != SWG_STRINGPOOL )
/*?*/ 				{
/*?*/ 					InHeader(); // RecSizes brauchen hier nicht gelesen zu
/*?*/ 								// werden, weil keine Records gelesen werden.
/*?*/ 					bSrcSetValid = Good();
/*?*/ 				}
/*?*/ 				pStrm = pOld;
/*?*/ 				CheckIoError( pPageStyles );
/*?*/ 				pPageStyles->SetBufferSize( 0 );
/*?*/ 			}
/*?*/
/*?*/ 			if( bSrcSetValid )
/*?*/ 				ChangeFontItemCharSet();
/*?*/ 			ConvertFmtsToStarSymbol();
/*N*/ 		}
/*N*/
/*N*/ 		if( bReadStrPool )
/*?*/ 			aStringPool.Clear();
/*N*/ 	}
/*N*/
/*N*/ #ifndef NUM_RELSPACE
/*?*/ 	// JP 14.09.98: Bug 55957 - die Vorlagen muessen dann aber
/*?*/ 	// 				auch die OutlineRule aktualisieren
/*?*/ 	if( !bNormal && bTxtColls )
/*?*/ 	{
/*?*/ 		const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
/*?*/ 		const SwTxtFmtColl* pColl;
/*?*/ 		SwNumRule* pOutlRule = (SwNumRule*)pDoc->GetOutlineNumRule();
/*?*/ 		BYTE nLevel;
/*?*/ 		for( USHORT n = rColls.Count(); n; )
/*?*/ 			if( MAXLEVEL > ( nLevel = GetRealLevel(
/*?*/ 				( pColl = rColls[ --n ])->GetOutlineLevel() )) )
/*?*/ 			{
/*?*/ 				const SvxLRSpaceItem& rLR = pColl->GetLRSpace();
/*?*/ 				const SwNumFmt& rNFmt = pOutlRule->Get( nLevel );
/*?*/ 				SwNumFmt aTmp( rNFmt );
/*?*/ 
/*?*/ 				// ohne Nummer immer ohne FirstLineOffset!!!!
/*?*/ 				if( pColl->GetOutlineLevel() & NO_NUMLEVEL )
/*?*/ 					aTmp.SetFirstLineOffset( 0 );
/*?*/ 				else
/*?*/ 					aTmp.SetFirstLineOffset( rLR.GetTxtFirstLineOfst() );
/*?*/ 
/*?*/ 				aTmp.SetAbsLSpace( rLR.GetTxtLeft() );
/*?*/ 				if( aTmp != rNFmt )
/*?*/ 					pOutlRule->Set( nLevel, aTmp );
/*?*/ 			}
/*?*/ 	}
/*?*/ 
/*?*/ 	//JP 21.07.98: Bug 53390
/*?*/ 	if( !bNormal && !bAdditive && bTxtColls )
/*?*/ 	{
/*?*/ 		pDoc->SetOutlineNumRule( *pDoc->GetOutlineNumRule() );
/*?*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	delete p;
/*N*/ 	delete pPool;
/*N*/ 	if( !pConvToSymbolFmts->Count() )
/*N*/ 	{
/*N*/ 		delete pConvToSymbolFmts;
/*N*/ 		pConvToSymbolFmts = 0;
/*N*/ 	}
/*N*/ }

/*N*/ void Sw3IoImp::SaveStyleSheets( BOOL bUsed )
/*N*/ {
/*N*/ 	ASSERT( !HasRecSizes(), "Hier darf es noch keine RecSizes geben" );
/*N*/ 	// Bisher wurde allenfalls der Drawing-Layer gespeichert. Deshalb
/*N*/ 	// kann es hier noch gar keine RecSizes geben. Besser ist aber besser ...
/*N*/ 	if( HasRecSizes() )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		FlushRecSizes();
/*N*/ 
/*N*/ 	SfxItemPool *pTmp = pDoc->GetAttrPool().GetSecondaryPool();
/*N*/ 	pDoc->GetAttrPool().SetSecondaryPool( 0 );
/*N*/ 	SfxItemPool* pPool = pDoc->GetAttrPool().Clone();
/*N*/ 	pDoc->GetAttrPool().SetSecondaryPool( pTmp );
/*N*/ 
/*N*/ 	pStyles->Seek( 0L );
/*N*/ 	pStyles->SetSize( 0L );
/*N*/ 	pStyles->SetBufferSize( SW3_BSW_STYLES );
/*N*/ 	pStrm = pStyles;
/*N*/ 	SwStyleSheetPool* p = new SwStyleSheetPool( *pDoc, *pPool,
/*N*/ 												pStrm->GetVersion(), 0 );
/*N*/ 	nRes = p->Store( *pStyles, bUsed ) ? 0 : ERR_SWG_WRITE_ERROR;
/*N*/ 	pStyles->SetBufferSize( 0 );
/*N*/ 	pStyles->Commit();
/*N*/ 	delete p;
/*N*/ 	delete pPool;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
