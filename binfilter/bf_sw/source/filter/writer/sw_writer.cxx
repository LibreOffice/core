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

#include <hintids.hxx>

#define _SVSTDARR_STRINGSSORTDTOR

 #ifndef _STREAM_HXX //autogen
 #include <tools/stream.hxx>
 #endif
#include <bf_svx/fontitem.hxx>
#include <bf_svx/eeitem.hxx>

#include <shellio.hxx>

#include <horiornt.hxx>
#include <errhdl.hxx>

#include <pam.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <bookmrk.hxx>          // fuer SwBookmark ...
#include <numrule.hxx>

#include <swerror.h>
namespace binfilter {

// Stringbuffer fuer die umgewandelten Zahlen
static sal_Char aNToABuf[] = "0000000000000000000000000";
#define NTOABUFLEN (sizeof(aNToABuf))

/*N*/ DECLARE_TABLE( SwBookmarkNodeTable, SvPtrarr* )

/*N*/ struct Writer_Impl
/*N*/ {
/*N*/ 	SvStringsSortDtor *pSrcArr, *pDestArr;
/*N*/ 	SvPtrarr* pFontRemoveLst, *pBkmkArr;
/*N*/ 	SwBookmarkNodeTable* pBkmkNodePos;
/*N*/
/*N*/ 	Writer_Impl( const SwDoc& rDoc );
/*N*/ 	~Writer_Impl();
/*N*/
/*N*/ 	void RemoveFontList( SwDoc& rDoc );
/*N*/ };

/*N*/ Writer_Impl::Writer_Impl( const SwDoc& rDoc )
/*N*/ 	: pSrcArr( 0 ), pDestArr( 0 ), pFontRemoveLst( 0 ), pBkmkNodePos( 0 )
/*N*/ {
/*N*/ }

/*N*/ Writer_Impl::~Writer_Impl()
/*N*/ {
/*N*/ 	delete pSrcArr;
/*N*/ 	delete pDestArr;
/*N*/ 	delete pFontRemoveLst;
/*N*/
/*N*/ 	if( pBkmkNodePos )
/*N*/ 	{
/*?*/ 		for( SvPtrarr* p = pBkmkNodePos->First(); p; p = pBkmkNodePos->Next() )
/*?*/ 			delete p;
/*?*/ 		delete pBkmkNodePos;
/*N*/ 	}
/*N*/ }

/*N*/ void Writer_Impl::RemoveFontList( SwDoc& rDoc )
/*N*/ {
/*N*/ 	ASSERT( pFontRemoveLst, "wo ist die FontListe?" );
/*N*/ 	for( USHORT i = pFontRemoveLst->Count(); i; )
/*N*/ 	{
/*N*/ 		SvxFontItem* pItem = (SvxFontItem*)(*pFontRemoveLst)[ --i ];
/*N*/ 		rDoc.GetAttrPool().Remove( *pItem );
/*N*/ 	}
/*N*/ }


/*
 * Dieses Modul ist die Zentrale-Sammelstelle fuer alle Write-Filter
 * und ist eine DLL !
 *
 * Damit der Writer mit den unterschiedlichen Writern arbeiten kann,
 * muessen fuer diese die Ausgabe-Funktionen der Inhalts tragenden
 * Objecte auf die verschiedenen Ausgabe-Funktionen gemappt werden.
 *
 * Dazu kann fuer jedes Object ueber den Which-Wert in einen Tabelle ge-
 * griffen werden, um seine Ausgabe-Funktion zu erfragen.
 * Diese Funktionen stehen in den entsprechenden Writer-DLL's.
 */

/*N*/ Writer::Writer()
/*N*/ 	: pImpl( 0 ), pStrm( 0 ), pOrigPam( 0 ), pOrigFileName( 0 ),
/*N*/ 	pCurPam(0), pDoc( 0 )
/*N*/ {
/*N*/ 	bWriteAll = bShowProgress = bUCS2_WithStartChar = TRUE;
/*N*/ 	bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
/*N*/ 		bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
/*N*/ 		bOrganizerMode = FALSE;
/*N*/ }

/*N*/ Writer::~Writer()
/*N*/ {
/*N*/ }

/*N*/ void Writer::ResetWriter()
/*N*/ {
/*N*/ 	if( pImpl && pImpl->pFontRemoveLst )
/*N*/ 		pImpl->RemoveFontList( *pDoc );
/*N*/ 	delete pImpl, pImpl = 0;
/*N*/
/*N*/ 	if( pCurPam )
/*N*/ 	{
/*N*/ 		while( pCurPam->GetNext() != pCurPam )
/*?*/ 			delete pCurPam->GetNext();
/*N*/ 		delete pCurPam;
/*N*/ 	}
/*N*/ 	pCurPam = 0;
/*N*/ 	pOrigFileName = 0;
/*N*/ 	pDoc = 0;
/*N*/ 	pStrm = 0;
/*N*/
/*N*/ 	bShowProgress = bUCS2_WithStartChar = TRUE;
/*N*/ 	bASCII_NoLastLineEnd = bASCII_ParaAsBlanc = bASCII_ParaAsCR =
/*N*/ 		bWriteClipboardDoc = bWriteOnlyFirstTable = bBlock =
/*N*/ 		bOrganizerMode = FALSE;
/*N*/ }

/*N*/ BOOL Writer::CopyNextPam( SwPaM ** ppPam )
/*N*/ {
/*N*/ 	if( (*ppPam)->GetNext() == pOrigPam )
/*N*/ 	{
/*N*/ 		*ppPam = pOrigPam;			// wieder auf den Anfangs-Pam setzen
/*N*/ 		return FALSE;				// Ende vom Ring
/*N*/ 	}
/*N*/
/*N*/ 	// ansonsten kopiere den die Werte aus dem naechsten Pam
/*?*/ 	*ppPam = ((SwPaM*)(*ppPam)->GetNext() );
/*?*/
/*?*/ 	*pCurPam->GetPoint() = *(*ppPam)->Start();
/*?*/ 	*pCurPam->GetMark() = *(*ppPam)->End();
/*?*/
/*?*/ 	return TRUE;
/*N*/ }

// suche die naechste Bookmark-Position aus der Bookmark-Tabelle

 SwPaM* Writer::NewSwPaM( SwDoc & rDoc, ULONG nStartIdx, ULONG nEndIdx ) const
 {
    SwNodes* pNds = &rDoc.GetNodes();

    SwNodeIndex aStt( *pNds, nStartIdx );
    SwCntntNode* pCNode = aStt.GetNode().GetCntntNode();
    if( !pCNode && 0 == ( pCNode = pNds->GoNext( &aStt )) )
        ASSERT( !this, "An StartPos kein ContentNode mehr" );

    SwPaM* pNew = new SwPaM( aStt );
    pNew->SetMark();
    aStt = nEndIdx;
    if( 0 == (pCNode = aStt.GetNode().GetCntntNode()) &&
        0 == (pCNode = pNds->GoPrevious( &aStt )) )
        ASSERT( !this, "An StartPos kein ContentNode mehr" );
    pCNode->MakeEndIndex( &pNew->GetPoint()->nContent );
    pNew->GetPoint()->nNode = aStt;
    return pNew;
 }

/////////////////////////////////////////////////////////////////////////////

// Stream-spezifisches
/*N*/ #ifdef DBG_UTIL
/*N*/ SvStream& Writer::Strm()
/*N*/ {
/*N*/ 	ASSERT( pStrm, "Oh-oh. Dies ist ein Storage-Writer. Gleich knallts!" );
/*N*/ 	return *pStrm;
/*N*/ }
/*N*/ #endif


SvStream& Writer::OutHex( SvStream& rStrm, ULONG nHex, BYTE nLen )
{                                                  // in einen Stream aus
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);
    for( BYTE n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStrm << pStr;
}

SvStream& Writer::OutLong( SvStream& rStrm, long nVal )
{
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);

    int bNeg = nVal < 0;
    if( bNeg )
        nVal = -nVal;

    do {
        *(--pStr) = (sal_Char)(nVal % 10 ) + 48;
        nVal /= 10;
    } while( nVal );

    // Ist Zahl negativ, dann noch -
    if( bNeg )
        *(--pStr) = '-';

    return rStrm << pStr;
}

SvStream& Writer::OutULong( SvStream& rStrm, ULONG nVal )
{
    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (NTOABUFLEN-1);

    do {
        *(--pStr) = (sal_Char)(nVal % 10 ) + 48;
        nVal /= 10;
    } while ( nVal );
    return rStrm << pStr;
}


/*N*/ ULONG Writer::Write( SwPaM& rPaM, SvStream& rStrm, const String* pFName )
/*N*/ {
/*N*/ 	pStrm = &rStrm;
/*N*/ 	pDoc = rPaM.GetDoc();
/*N*/ 	pOrigFileName = pFName;
/*N*/ 	pImpl = new Writer_Impl( *pDoc );
/*N*/
/*N*/ 	// PaM kopieren, damit er veraendert werden kann
/*N*/ 	pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
/*N*/ 	// zum Vergleich auf den akt. Pam sichern
/*N*/ 	pOrigPam = &rPaM;
/*N*/
/*N*/ 	ULONG nRet = WriteStream();
/*N*/
/*N*/ 	ResetWriter();
/*N*/
/*N*/ 	return nRet;
/*N*/ }


/*N*/ ULONG Writer::Write( SwPaM& rPam, SvStorage&, const String* )
/*N*/ {
/*N*/ 	ASSERT( !this, "Schreiben in Storages auf einem Stream?" );
/*N*/ 	return ERR_SWG_WRITE_ERROR;
/*N*/ }



/*N*/ void Writer::PutNumFmtFontsInAttrPool()
/*N*/ {
/*N*/ 	if( !pImpl )
/*?*/ 		pImpl = new Writer_Impl( *pDoc );
/*N*/
/*N*/ 	// dann gibt es noch in den NumRules ein paar Fonts
/*N*/ 	// Diese in den Pool putten. Haben sie danach einen RefCount > 1
/*N*/ 	// kann es wieder entfernt werden - ist schon im Pool
/*N*/ 	SfxItemPool& rPool = pDoc->GetAttrPool();
/*N*/ 	const SwNumRuleTbl& rListTbl = pDoc->GetNumRuleTbl();
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	const SwNumFmt* pFmt;
/*N*/ 	const Font *pFont, *pDefFont = &SwNumRule::GetDefBulletFont();
/*N*/ 	BOOL bCheck = FALSE;
/*N*/
/*N*/ 	for( USHORT nGet = rListTbl.Count(); nGet; )
/*N*/ 		if( pDoc->IsUsed( *(pRule = rListTbl[ --nGet ] )))
/*?*/ 			for( BYTE nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
/*?*/ 				if( SVX_NUM_CHAR_SPECIAL == (pFmt = &pRule->Get( nLvl ))->GetNumberingType() ||
/*?*/ 					SVX_NUM_BITMAP == pFmt->GetNumberingType() )
/*?*/ 				{
/*?*/ 					if( 0 == ( pFont = pFmt->GetBulletFont() ) )
/*?*/ 						pFont = pDefFont;
/*?*/
/*?*/ 					if( bCheck )
/*?*/ 					{
/*?*/ 						if( *pFont == *pDefFont )
/*?*/ 							continue;
/*?*/ 					}
/*?*/ 					else if( *pFont == *pDefFont )
/*?*/ 						bCheck = TRUE;
/*?*/
/*?*/ 					_AddFontItem( rPool, SvxFontItem( pFont->GetFamily(),
/*?*/ 								pFont->GetName(), pFont->GetStyleName(),
/*?*/ 								pFont->GetPitch(), pFont->GetCharSet() ));
/*N*/ 				}
/*N*/ }

/*N*/ void Writer::PutEditEngFontsInAttrPool( BOOL bIncl_CJK_CTL )
/*N*/ {
/*N*/ 	if( !pImpl )
/*?*/ 		pImpl = new Writer_Impl( *pDoc );
/*N*/
/*N*/ 	SfxItemPool& rPool = pDoc->GetAttrPool();
/*N*/ 	if( rPool.GetSecondaryPool() )
/*N*/ 	{
/*N*/ 		_AddFontItems( rPool, EE_CHAR_FONTINFO );
/*N*/ 		if( bIncl_CJK_CTL )
/*N*/ 		{
/*N*/ 			_AddFontItems( rPool, EE_CHAR_FONTINFO_CJK );
/*N*/ 			_AddFontItems( rPool, EE_CHAR_FONTINFO_CTL );
/*N*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ void Writer::_AddFontItems( SfxItemPool& rPool, USHORT nW )
/*N*/ {
/*N*/ 	const SvxFontItem* pFont = (const SvxFontItem*)&rPool.GetDefaultItem( nW );
/*N*/ 	_AddFontItem( rPool, *pFont );
/*N*/
/*N*/ 	if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem( nW )) )
/*N*/ 		_AddFontItem( rPool, *pFont );
/*N*/
/*N*/ 	USHORT nMaxItem = rPool.GetItemCount( nW );
/*N*/ 	for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
/*N*/ 		if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem( nW, nGet )) )
/*N*/ 			_AddFontItem( rPool, *pFont );
/*N*/ }

/*N*/ void Writer::_AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont )
/*N*/ {
/*N*/ 	const SvxFontItem* pItem;
/*N*/ 	if( RES_CHRATR_FONT != rFont.Which() )
/*N*/ 	{
/*N*/ 		SvxFontItem aFont( rFont );
/*N*/ 		aFont.SetWhich( RES_CHRATR_FONT );
/*N*/ 		pItem = (SvxFontItem*)&rPool.Put( aFont );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pItem = (SvxFontItem*)&rPool.Put( rFont );
/*N*/
/*N*/ 	if( 1 < pItem->GetRefCount() )
/*N*/ 		rPool.Remove( *pItem );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( !pImpl->pFontRemoveLst )
/*N*/ 			pImpl->pFontRemoveLst = new SvPtrarr( 0, 10 );
/*N*/
/*N*/ 		void* p = (void*)pItem;
/*N*/ 		pImpl->pFontRemoveLst->Insert( p, pImpl->pFontRemoveLst->Count() );
/*N*/ 	}
/*N*/ }

// build a bookmark table, which is sort by the node position. The
// OtherPos of the bookmarks also inserted.


// search alle Bookmarks in the range and return it in the Array

////////////////////////////////////////////////////////////////////////////

// Storage-spezifisches


/*N*/ ULONG StgWriter::Write( SwPaM& rPaM, SvStorage& rStg, const String* pFName )
/*N*/ {
/*N*/ 	pStrm = 0;
/*N*/ 	pStg = &rStg;
/*N*/ 	pDoc = rPaM.GetDoc();
/*N*/ 	pOrigFileName = pFName;
/*N*/ 	pImpl = new Writer_Impl( *pDoc );
/*N*/
/*N*/ 	// PaM kopieren, damit er veraendert werden kann
/*N*/ 	pCurPam = new SwPaM( *rPaM.End(), *rPaM.Start() );
/*N*/ 	// zum Vergleich auf den akt. Pam sichern
/*N*/ 	pOrigPam = &rPaM;
/*N*/
/*N*/ 	ULONG nRet = WriteStorage();
/*N*/
/*N*/ 	pStg = NULL;
/*N*/ 	ResetWriter();
/*N*/
/*N*/ 	return nRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
