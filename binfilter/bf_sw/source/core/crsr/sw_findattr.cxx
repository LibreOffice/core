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



#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;


    // Sonderbehandlung fuer SvxFontItem, nur den Namen vergleichen:


/*N*/ const SwTxtAttr* GetFrwrdTxtHint( const SwpHints& rHtsArr, USHORT& rPos,
/*N*/ 									xub_StrLen nCntntPos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return 0;			 		// kein gueltiges TextAttribut
/*N*/ }


/*N*/ const SwTxtAttr* GetBkwrdTxtHint( const SwpHints& rHtsArr, USHORT& rPos,
/*N*/ 								  xub_StrLen nCntntPos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return 0; 					// kein gueltiges TextAttribut
/*N*/ }



//------------------ Suche nach einem Text Attribut -----------------------

// diese Funktion sucht in einem TextNode nach dem vorgegebenen Attribut.
// Wird es gefunden, dann hat der SwPaM den Bereich der das Attribut
// umspannt, unter Beachtung des Suchbereiches




//------------------ Suche nach mehren Text Attributen -------------------

































//------------------ Methoden vom SwCursor ---------------------------

// Parameter fuer das Suchen vom Attributen
struct SwFindParaAttr : public SwFindParas
{
    BOOL bValue;
    const SfxItemSet *pSet, *pReplSet;
    const SearchOptions *pSearchOpt;
    SwCursor& rCursor;
    ::utl::TextSearch* pSTxt;

    SwFindParaAttr( const SfxItemSet& rSet, BOOL bNoCollection,
                    const SearchOptions* pOpt, const SfxItemSet* pRSet,
                    SwCursor& rCrsr )
        : pSet( &rSet ), pReplSet( pRSet ), rCursor( rCrsr ),
            bValue( bNoCollection ), pSearchOpt( pOpt ), pSTxt( 0 )
        {}
    ~SwFindParaAttr()	{ delete pSTxt; }

    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, FASTBOOL bInReadOnly );
     virtual int IsReplaceMode() const;
};


/*N*/int SwFindParaAttr::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
/*N*/							FASTBOOL bInReadOnly )
/*N*/{
DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001  	// String ersetzen ?? (nur wenn Text angegeben oder nicht attributiert
/*N*/ }


/*N*/int SwFindParaAttr::IsReplaceMode() const
/*N*/{
DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	return ( pSearchOpt && pSearchOpt->replaceString.getLength() ) ||
/*N*/ }

// Suchen nach Attributen


/*M*/ ULONG SwCursor::Find( const SfxItemSet& rSet, FASTBOOL bNoCollections,
/*M*/ 					SwDocPositions nStart, SwDocPositions nEnde, BOOL& bCancel,
/*M*/ 					FindRanges eFndRngs,
/*M*/ 					const SearchOptions* pSearchOpt, const SfxItemSet* pReplSet )
/*M*/ {
/*M*/ 	// OLE-Benachrichtigung abschalten !!
/*M*/ 	SwDoc* pDoc = GetDoc();
/*M*/ 	Link aLnk( pDoc->GetOle2Link() );
/*M*/ 	pDoc->SetOle2Link( Link() );
/*M*/
/*M*/ 	BOOL bReplace = ( pSearchOpt && ( pSearchOpt->replaceString.getLength() ||
/*M*/ 									!rSet.Count() ) ) ||
/*M*/ 					(pReplSet && pReplSet->Count());
/*M*/
/*M*/ 	SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pSearchOpt,
/*M*/ 									pReplSet, *this );
/*M*/
/*M*/     ULONG nRet = FindAll(aSwFindParaAttr, nStart, nEnde, eFndRngs, bCancel );
/*M*/ 	pDoc->SetOle2Link( aLnk );
/*M*/ 	if( nRet && bReplace )
/*M*/ 		pDoc->SetModified();
/*M*/
/*M*/ 	return nRet;
/*M*/ }



}
