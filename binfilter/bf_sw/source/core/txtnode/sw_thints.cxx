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

#ifndef _SFX_WHITER_HXX //autogen
#include <bf_svtools/whiter.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <bf_svtools/itemiter.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <bf_svx/emphitem.hxx>
#endif

#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
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
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>			// fuer SwFmtChg
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
// OD 26.06.2003 #108784#
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif

#include <algorithm>
namespace binfilter {

#ifdef DBG_UTIL
#define CHECK    Check();
#else
#define CHECK
#endif

using namespace ::com::sun::star::i18n;

/*************************************************************************
 *						SwTxtNode::MakeTxtAttr()
 *************************************************************************/

    // lege ein neues TextAttribut an und fuege es SwpHints-Array ein
/*N*/ SwTxtAttr* SwTxtNode::MakeTxtAttr( const SfxPoolItem& rAttr,
/*N*/ 								   xub_StrLen nStt, xub_StrLen nEnd, BOOL bPool )
/*N*/ {
/*N*/ 	// das neue Attribut im Pool anlegen
/*N*/ 	const SfxPoolItem& rNew = bPool ? GetDoc()->GetAttrPool().Put( rAttr ) :
/*N*/ 							  rAttr;
/*N*/
/*N*/ 	SwTxtAttr* pNew = 0;
/*N*/ 	switch( rNew.Which() )
/*N*/ 	{
/*N*/ 	case RES_CHRATR_CASEMAP:
/*N*/ 	case RES_CHRATR_COLOR:
/*N*/ 	case RES_CHRATR_CHARSETCOLOR:
/*N*/ 	case RES_CHRATR_CONTOUR:
/*N*/ 	case RES_CHRATR_CROSSEDOUT:
/*N*/ 	case RES_CHRATR_ESCAPEMENT:
/*N*/ 	case RES_CHRATR_KERNING:
/*N*/ 	case RES_CHRATR_SHADOWED:
/*N*/ 	case RES_CHRATR_AUTOKERN:
/*N*/ 	case RES_CHRATR_WORDLINEMODE:
/*N*/ 	case RES_CHRATR_UNDERLINE:
/*N*/     case RES_CHRATR_FONT:
/*N*/     case RES_CHRATR_CTL_FONT:
/*N*/     case RES_CHRATR_CJK_FONT:
/*N*/ 	case RES_CHRATR_FONTSIZE:
/*N*/     case RES_CHRATR_CTL_FONTSIZE:
/*N*/     case RES_CHRATR_CJK_FONTSIZE:
/*N*/ 	case RES_CHRATR_LANGUAGE:
/*N*/     case RES_CHRATR_CTL_LANGUAGE:
/*N*/     case RES_CHRATR_CJK_LANGUAGE:
/*N*/ 	case RES_CHRATR_POSTURE:
/*N*/     case RES_CHRATR_CTL_POSTURE:
/*N*/     case RES_CHRATR_CJK_POSTURE:
/*N*/ 	case RES_CHRATR_WEIGHT:
/*N*/     case RES_CHRATR_CTL_WEIGHT:
/*N*/     case RES_CHRATR_CJK_WEIGHT:
/*N*/ 	case RES_CHRATR_EMPHASIS_MARK:
/*N*/ 	case RES_CHRATR_NOHYPHEN:
/*N*/ 	case RES_CHRATR_BLINK:
/*N*/ 	case RES_CHRATR_BACKGROUND:
/*N*/ 	case RES_CHRATR_ROTATE:
/*N*/ 	case RES_CHRATR_SCALEW:
/*N*/ 	case RES_CHRATR_RELIEF:
/*N*/         pNew = new SwTxtAttrEnd( rNew, nStt, nEnd );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_CHARFMT:
/*N*/ 		{
/*N*/ 			SwFmtCharFmt &rFmtCharFmt = (SwFmtCharFmt&) rNew;
/*N*/ 			if( !rFmtCharFmt.GetCharFmt() )
/*?*/ 				rFmtCharFmt.SetCharFmt( GetDoc()->GetDfltCharFmt() );
/*N*/
/*N*/ 			pNew = new SwTxtCharFmt( rFmtCharFmt, nStt, nEnd );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_INETFMT:
/*N*/ 		pNew = new SwTxtINetFmt( (SwFmtINetFmt&)rNew, nStt, nEnd );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_FIELD:
/*N*/ 		pNew = new SwTxtFld( (SwFmtFld&)rNew, nStt );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_FLYCNT:
/*N*/ 		{
/*N*/ 			// erst hier wird das Frame-Format kopiert (mit Inhalt) !!
/*N*/ 			pNew = new SwTxtFlyCnt( (SwFmtFlyCnt&)rNew, nStt );
/*N*/ 			// Kopie von einem Text-Attribut
/*N*/ 			if( ((SwFmtFlyCnt&)rAttr).GetTxtFlyCnt() )
/*N*/ 				// dann muss das Format Kopiert werden
/*N*/ 				((SwTxtFlyCnt*)pNew)->CopyFlyFmt( GetDoc() );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_FTN:
/*N*/ 		pNew = new SwTxtFtn( (SwFmtFtn&)rNew, nStt );
/*N*/ 		// ggfs. SeqNo kopieren
/*N*/ 		if( ((SwFmtFtn&)rAttr).GetTxtFtn() )
/*?*/ 			((SwTxtFtn*)pNew)->SetSeqNo( ((SwFmtFtn&)rAttr).GetTxtFtn()->GetSeqRefNo() );
/*N*/ 		break;
/*?*/ 	case RES_TXTATR_HARDBLANK:
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		pNew = new SwTxtHardBlank( (SwFmtHardBlank&)rNew, nStt );
/*?*/ 		break;
/*?*/     case RES_CHRATR_TWO_LINES:
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		pNew = new SwTxt2Lines( (SvxTwoLinesItem&)rNew, nStt, nEnd );
/*?*/ 		break;
/*N*/ 	case RES_TXTATR_REFMARK:
/*N*/ 		pNew = nStt == nEnd
/*N*/ 				? new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt )
/*N*/ 				: new SwTxtRefMark( (SwFmtRefMark&)rNew, nStt, &nEnd );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_TOXMARK:
/*N*/ 		pNew = new SwTxtTOXMark( (SwTOXMark&)rNew, nStt, &nEnd );
/*N*/ 		break;
/*N*/ 	case RES_UNKNOWNATR_CONTAINER:
/*?*/ 	case RES_TXTATR_UNKNOWN_CONTAINER:
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		pNew = new SwTxtXMLAttrContainer( (SvXMLAttrContainerItem&)rNew,
/*?*/ 		break;
/*N*/ 	case RES_TXTATR_CJK_RUBY:
/*N*/ 		pNew = new SwTxtRuby( (SwFmtRuby&)rNew, nStt, nEnd );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	ASSERT( pNew, "was fuer ein TextAttribut soll hier angelegt werden?" );
/*N*/ 	return pNew;
/*N*/ }

// loesche das Text-Attribut (muss beim Pool abgemeldet werden!)
/*N*/ void SwTxtNode::DestroyAttr( SwTxtAttr* pAttr )
/*N*/ {
/*N*/ 	if( pAttr )
/*N*/ 	{
/*N*/ 		// einige Sachen muessen vorm Loeschen der "Format-Attribute" erfolgen
/*N*/ 		SwDoc* pDoc = GetDoc();
/*N*/ 		USHORT nDelMsg = 0;
/*N*/ 		switch( pAttr->Which() )
/*N*/ 		{
/*N*/ 		case RES_TXTATR_FLYCNT:
/*N*/ 			{
/*N*/ 				// siehe auch die Anmerkung "Loeschen von Formaten
/*N*/ 				// zeichengebundener Frames" in fesh.cxx, SwFEShell::DelFmt()
/*N*/ 				SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
/*N*/ 				if( pFmt )		// vom Undo auf 0 gesetzt ??
/*N*/ 					pDoc->DelLayoutFmt( (SwFlyFrmFmt*)pFmt );
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_FTN:
/*N*/ 			((SwTxtFtn*)pAttr)->SetStartNode( 0 );
/*N*/ 			nDelMsg = RES_FOOTNOTE_DELETED;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_FIELD:
/*N*/ 			if( !pDoc->IsInDtor() )
/*N*/ 			{
/*N*/ 				// Wenn wir ein HiddenParaField sind, dann muessen wir
/*N*/ 				// ggf. fuer eine Neuberechnung des Visible-Flags sorgen.
/*N*/ 				const SwField* pFld = pAttr->GetFld().GetFld();
/*N*/
/*N*/ 				//JP 06-08-95: DDE-Felder bilden eine Ausnahme
/*N*/ 				ASSERT( RES_DDEFLD == pFld->GetTyp()->Which() ||
/*N*/ 						this == ((SwTxtFld*)pAttr)->GetpTxtNode(),
/*N*/ 						"Wo steht denn dieses Feld?" )
/*N*/
/*N*/ 				// bestimmte Felder mussen am Doc das Calculations-Flag updaten
/*N*/ 				switch( pFld->GetTyp()->Which() )
/*N*/ 				{
/*?*/ 				case RES_HIDDENPARAFLD:
/*?*/ 					SetCalcVisible();
/*?*/ 					// kein break !
/*?*/ 				case RES_DBSETNUMBERFLD:
/*?*/ 				case RES_GETEXPFLD:
/*?*/ 				case RES_DBFLD:
/*?*/ 				case RES_SETEXPFLD:
/*?*/ 				case RES_HIDDENTXTFLD:
/*?*/ 				case RES_DBNUMSETFLD:
/*?*/ 				case RES_DBNEXTSETFLD:
/*?*/ 					if( !pDoc->IsNewFldLst() && GetNodes().IsDocNodes() )
/*?*/ 						pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pAttr );
/*?*/ 					break;
/*?*/ 				case RES_DDEFLD:
/*?*/ 					if( GetNodes().IsDocNodes() &&
/*?*/ 						((SwTxtFld*)pAttr)->GetpTxtNode() )
/*?*/ 						((SwDDEFieldType*)pFld->GetTyp())->DecRefCnt();
/*?*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nDelMsg = RES_FIELD_DELETED;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_TOXMARK:
/*N*/ 			nDelMsg = RES_TOXMARK_DELETED;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_REFMARK:
/*N*/ 			nDelMsg = RES_REFMARK_DELETED;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		if( nDelMsg && !pDoc->IsInDtor() && GetNodes().IsDocNodes() )
/*N*/ 		{
/*N*/ 			SwPtrMsgPoolItem aMsgHint( nDelMsg, (void*)&pAttr->GetAttr() );
/*N*/ 			pDoc->GetUnoCallBack()->Modify( &aMsgHint, &aMsgHint );
/*N*/ 		}
/*N*/
/*N*/ 		pAttr->RemoveFromPool( pDoc->GetAttrPool() );
/*N*/ 		delete pAttr;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtNode::Insert()
 *************************************************************************/

// lege ein neues TextAttribut an und fuege es ins SwpHints-Array ein
/*N*/ SwTxtAttr* SwTxtNode::Insert( const SfxPoolItem& rAttr,
/*N*/ 							  xub_StrLen nStt, xub_StrLen nEnd, USHORT nMode )
/*N*/ {
/*N*/ 	SwTxtAttr* pNew = MakeTxtAttr( rAttr, nStt, nEnd );
/*N*/ 	return (pNew && Insert( pNew, nMode )) ? pNew : 0;
/*N*/ }


// uebernehme den Pointer auf das Text-Attribut

/*N*/ BOOL SwTxtNode::Insert( SwTxtAttr *pAttr, USHORT nMode )
/*N*/ {
/*N*/ 	BOOL bHiddenPara = FALSE;
/*N*/
/*N*/ 	ASSERT( *pAttr->GetStart() <= Len(), "StartIdx hinter Len!" );
/*N*/
/*N*/ 	if( !pAttr->GetEnd() )
/*N*/ 	{
/*N*/ 		USHORT nInsMode = nMode;
/*N*/ 		switch( pAttr->Which() )
/*N*/ 		{
/*N*/ 			case RES_TXTATR_FLYCNT:
/*N*/ 			{
/*N*/ 				SwTxtFlyCnt *pFly = (SwTxtFlyCnt *)pAttr;
/*N*/ 				SwFrmFmt* pFmt = pAttr->GetFlyCnt().GetFrmFmt();
/*N*/ 				if( !(SETATTR_NOTXTATRCHR & nInsMode) )
/*N*/ 				{
/*N*/ 					// Wir muessen zuerst einfuegen, da in SetAnchor()
/*N*/ 					// dem FlyFrm GetStart() uebermittelt wird.
/*N*/ 					//JP 11.05.98: falls das Anker-Attribut schon richtig
/*N*/ 					// gesetzt ist, dann korrigiere dieses nach dem Einfuegen
/*N*/ 					// des Zeichens. Sonst muesste das immer  ausserhalb
/*N*/ 					// erfolgen (Fehleranfaellig !)
/*N*/ 					const SwFmtAnchor* pAnchor = 0;
/*N*/ 					pFmt->GetItemState( RES_ANCHOR, FALSE,
/*N*/ 											(const SfxPoolItem**)&pAnchor );
/*N*/
/*N*/ 					SwIndex aIdx( this, *pAttr->GetStart() );
/*N*/ 					Insert( GetCharOfTxtAttr(*pAttr), aIdx );
/*N*/ 					nInsMode |= SETATTR_NOTXTATRCHR;
/*N*/
/*N*/ 					if( pAnchor && FLY_IN_CNTNT == pAnchor->GetAnchorId() &&
/*N*/ 						pAnchor->GetCntntAnchor() &&
/*N*/ 						pAnchor->GetCntntAnchor()->nNode == *this &&
/*N*/ 						pAnchor->GetCntntAnchor()->nContent == aIdx )
/*N*/ 						((SwIndex&)pAnchor->GetCntntAnchor()->nContent)--;
/*N*/ 				}
/*N*/ 				pFly->SetAnchor( this );
/*N*/
/*N*/ 				// Format-Pointer kann sich im SetAnchor geaendert haben!
/*N*/ 				// (Kopieren in andere Docs!)
/*N*/ 				pFmt = pAttr->GetFlyCnt().GetFrmFmt();
/*N*/ 				SwDoc *pDoc = pFmt->GetDoc();
/*N*/
/*N*/                 // OD 26.06.2003 #108784# - allow drawing objects in header/footer.
/*N*/                 // But don't allow control objects in header/footer
/*N*/                 if( RES_DRAWFRMFMT == pFmt->Which() &&
/*N*/                     pDoc->IsInHeaderFooter( pFmt->GetAnchor().GetCntntAnchor()->nNode ) )
/*N*/ 				{
/*N*/                     SwDrawContact* pDrawContact =
/*N*/                         static_cast<SwDrawContact*>(pFmt->FindContactObj());
/*N*/                     if ( pDrawContact &&
/*N*/                          pDrawContact->GetMaster() &&
/*N*/                          ::binfilter::CheckControlLayer( pDrawContact->GetMaster() ) )
/*N*/ 					{
/*N*/ 					// das soll nicht meoglich sein; hier verhindern
/*N*/ 					// Der Dtor des TxtHints loescht nicht das Zeichen.
/*N*/ 					// Wenn ein CH_TXTATR_.. vorliegt, dann muss man
/*N*/ 					// dieses explizit loeschen
/*?*/ 						if( SETATTR_NOTXTATRCHR & nInsMode )
/*?*/ 						{
/*?*/ 						// loesche das Zeichen aus dem String !
/*?*/ 						ASSERT( ( CH_TXTATR_BREAKWORD ==
/*?*/ 										aText.GetChar(*pAttr->GetStart() ) ||
/*?*/ 								  CH_TXTATR_INWORD ==
/*?*/ 								  		aText.GetChar(*pAttr->GetStart())),
/*?*/ 								"where is my attribu character" );
/*?*/ 						aText.Erase( *pAttr->GetStart(), 1 );
/*?*/ 						// Indizies Updaten
/*?*/ 						SwIndex aTmpIdx( this, *pAttr->GetStart() );
/*?*/ 						Update( aTmpIdx, 1, TRUE );
/*?*/ 						}
/*?*/ 					DestroyAttr( pAttr );
/*?*/ 					return FALSE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case RES_TXTATR_FTN :
/*N*/ 			{
/*N*/ 				// Fussnoten, man kommt an alles irgendwie heran.
/*N*/ 				// CntntNode erzeugen und in die Inserts-Section stellen
/*N*/ 				SwDoc *pDoc = GetDoc();
/*N*/ 				SwNodes &rNodes = pDoc->GetNodes();
/*N*/
/*N*/ 				// FussNote in nicht Content-/Redline-Bereich einfuegen ??
/*N*/ 				if( StartOfSectionIndex() < rNodes.GetEndOfAutotext().GetIndex() )
/*N*/ 				{
/*N*/ 					// das soll nicht meoglich sein; hier verhindern
/*N*/ 					// Der Dtor des TxtHints loescht nicht das Zeichen.
/*N*/ 					// Wenn ein CH_TXTATR_.. vorliegt, dann muss man
/*N*/ 					// dieses explizit loeschen
/*?*/ 					if( SETATTR_NOTXTATRCHR & nInsMode )
/*?*/ 					{
/*?*/ 						// loesche das Zeichen aus dem String !
/*?*/ 						ASSERT( ( CH_TXTATR_BREAKWORD ==
/*?*/ 										aText.GetChar(*pAttr->GetStart() ) ||
/*?*/ 								  CH_TXTATR_INWORD ==
/*?*/ 								  		aText.GetChar(*pAttr->GetStart())),
/*?*/ 								"where is my attribu character" );
/*?*/ 						aText.Erase( *pAttr->GetStart(), 1 );
/*?*/ 						// Indizies Updaten
/*?*/ 						SwIndex aTmpIdx( this, *pAttr->GetStart() );
/*?*/ 						Update( aTmpIdx, 1, TRUE );
/*?*/ 					}
/*?*/ 					DestroyAttr( pAttr );
/*?*/ 					return FALSE;
/*N*/ 				}
/*N*/
/*N*/ 				// wird eine neue Fussnote eingefuegt ??
/*N*/ 				BOOL bNewFtn = 0 == ((SwTxtFtn*)pAttr)->GetStartNode();
/*N*/ 				if( bNewFtn )
/*N*/ 					((SwTxtFtn*)pAttr)->MakeNewTextSection( GetNodes() );
/*N*/ 				else if ( !GetpSwpHints() || !GetpSwpHints()->IsInSplitNode() )
/*N*/ 				{
/*N*/ 					// loesche alle Frames der Section, auf die der StartNode zeigt
/*N*/ 					ULONG nSttIdx =
/*N*/ 						((SwTxtFtn*)pAttr)->GetStartNode()->GetIndex();
/*N*/ 					ULONG nEndIdx = rNodes[ nSttIdx++ ]->EndOfSectionIndex();
/*N*/ 					SwCntntNode* pCNd;
/*N*/ 					for( ; nSttIdx < nEndIdx; ++nSttIdx )
/*N*/ 						if( 0 != ( pCNd = rNodes[ nSttIdx ]->GetCntntNode() ))
/*N*/ 							pCNd->DelFrms();
/*N*/ 				}
/*N*/
/*N*/ 				if( !(SETATTR_NOTXTATRCHR & nInsMode) )
/*N*/ 				{
/*N*/ 					// Wir muessen zuerst einfuegen, da sonst gleiche Indizes
/*N*/ 					// entstehen koennen und das Attribut im _SortArr_ am
/*N*/ 					// Dokument nicht eingetrage wird.
/*N*/ 					SwIndex aNdIdx( this, *pAttr->GetStart() );
/*N*/ 					Insert( GetCharOfTxtAttr(*pAttr), aNdIdx );
/*N*/ 					nInsMode |= SETATTR_NOTXTATRCHR;
/*N*/ 				}
/*N*/
/*N*/ 				// Wir tragen uns am FtnIdx-Array des Docs ein ...
/*N*/ 				SwTxtFtn* pTxtFtn = 0;
/*N*/ 				if( !bNewFtn )
/*N*/ 				{
/*N*/ 					// eine alte Ftn wird umgehaengt (z.B. SplitNode)
/*N*/ 					for( USHORT n = 0; n < pDoc->GetFtnIdxs().Count(); ++n )
/*?*/ 						if( pAttr == pDoc->GetFtnIdxs()[n] )
/*?*/ 						{
/*?*/ 							// neuen Index zuweisen, dafuer aus dem SortArray
/*?*/ 							// loeschen und neu eintragen
/*?*/ 							pTxtFtn = pDoc->GetFtnIdxs()[n];
/*?*/ 							pDoc->GetFtnIdxs().Remove( n );
/*?*/ 							break;
/*?*/ 						}
/*N*/ 					// wenn ueber Undo der StartNode gesetzt wurde, kann
/*N*/ 					// der Index noch gar nicht in der Verwaltung stehen !!
/*N*/ 				}
/*N*/ 				if( !pTxtFtn )
/*N*/ 					pTxtFtn = (SwTxtFtn*)pAttr;
/*N*/
/*N*/ 				// fuers Update der Nummern und zum Sortieren
/*N*/ 				// muss der Node gesetzt sein.
/*N*/ 				((SwTxtFtn*)pAttr)->ChgTxtNode( this );
/*N*/
/*N*/ 				// FussNote im Redline-Bereich NICHT ins FtnArray einfuegen!
/*N*/ 				if( StartOfSectionIndex() > rNodes.GetEndOfRedlines().GetIndex() )
/*N*/ 				{
/*N*/ #ifdef DBG_UTIL
/*N*/ 					const BOOL bSuccess =
/*N*/ #endif
/*N*/ 						pDoc->GetFtnIdxs().Insert( pTxtFtn );
/*N*/ #ifdef DBG_UTIL
/*N*/ 					ASSERT( bSuccess, "FtnIdx nicht eingetragen." );
/*N*/ #endif
/*N*/ 				}
/*N*/ 				SwNodeIndex aTmpIndex( *this );
/*N*/ 				pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
/*N*/ 				((SwTxtFtn*)pAttr)->SetSeqRefNo();
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 			case RES_TXTATR_FIELD:
/*N*/ 				{
/*N*/ 					// fuer HiddenParaFields Benachrichtigungsmechanismus
/*N*/ 					// anwerfen
/*N*/ 					if( RES_HIDDENPARAFLD ==
/*N*/ 						pAttr->GetFld().GetFld()->GetTyp()->Which() )
/*N*/ 					bHiddenPara = TRUE;
/*N*/ 				}
/*N*/ 				break;
/*N*/
/*N*/ 		}
/*N*/ 		// Fuer SwTxtHints ohne Endindex werden CH_TXTATR_..
/*N*/ 		// eingefuegt, aStart muss danach um einen zurueckgesetzt werden.
/*N*/ 		// Wenn wir im SwTxtNode::Copy stehen, so wurde das Zeichen bereits
/*N*/ 		// mitkopiert. In solchem Fall ist SETATTR_NOTXTATRCHR angegeben worden.
/*N*/ 		if( !(SETATTR_NOTXTATRCHR & nInsMode) )
/*N*/ 		{
/*N*/ 			SwIndex aIdx( this, *pAttr->GetStart() );
/*N*/ 			Insert( GetCharOfTxtAttr(*pAttr), aIdx );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		ASSERT( *pAttr->GetEnd() <= Len(), "EndIdx hinter Len!" );
/*N*/
/*N*/ 	if ( !pSwpHints )
/*N*/ 		pSwpHints = new SwpHints();
/*N*/
/*N*/ 	// 4263: AttrInsert durch TextInsert => kein Adjust
/*N*/ 	pSwpHints->Insert( pAttr, *this, nMode );
/*N*/
/*N*/ 	// 47375: In pSwpHints->Insert wird u.a. Merge gerufen und das Hints-Array
/*N*/ 	// von ueberfluessigen Hints befreit, dies kann u.U. sogar der frisch
/*N*/ 	// eingefuegte Hint pAttr sein, der dann zerstoert wird!!
/*N*/ 	if( USHRT_MAX == pSwpHints->GetPos( pAttr ) )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	if(bHiddenPara)
/*N*/ 		SetCalcVisible();
/*N*/ 	return TRUE;
/*N*/ }


/*************************************************************************
 *						SwTxtNode::Delete()
 *************************************************************************/

/*N*/ void SwTxtNode::Delete( SwTxtAttr *pAttr, BOOL bThisOnly )
/*N*/ {
/*N*/ 	if ( !pSwpHints )
/*N*/ 		return;
/*N*/ 	if( bThisOnly )
/*N*/ 	{
/*?*/ 		xub_StrLen* pEndIdx = pAttr->GetEnd();
/*?*/ 		if( !pEndIdx )
/*?*/ 		{
/*?*/ 			// hat es kein Ende kann es nur das sein, was hier steht!
/*?*/ 			// Unbedingt Copy-konstruieren!
/*?*/ 			const SwIndex aIdx( this, *pAttr->GetStart() );
/*?*/ 			Erase( aIdx, 1 );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			// den MsgHint jetzt fuettern, weil gleich sind
/*?*/ 			// Start und End weg.
/*?*/ 			SwUpdateAttr aHint( *pAttr->GetStart(), *pEndIdx, pAttr->Which() );
/*?*/ 			pSwpHints->Delete( pAttr );
/*?*/ 			pAttr->RemoveFromPool( GetDoc()->GetAttrPool() );
/*?*/ 			delete pAttr;
/*?*/ 			SwModify::Modify( 0, &aHint );	   // die Frames benachrichtigen
/*?*/
/*?*/ 			if( pSwpHints && pSwpHints->CanBeDeleted() )
/*?*/ 				DELETEZ( pSwpHints );
/*?*/ 		}
/*?*/
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	Delete( pAttr->Which(), *pAttr->GetStart(), *pAttr->GetAnyEnd() );
/*N*/ }

/*************************************************************************
 *						SwTxtNode::Delete()
 *************************************************************************/

/*N*/ void SwTxtNode::Delete( USHORT nTxtWhich, xub_StrLen nStart, xub_StrLen nEnd )
/*N*/ {
/*N*/ 	if ( !pSwpHints )
/*N*/ 		return;
/*N*/
/*N*/ 	const xub_StrLen *pEndIdx;
/*N*/ 	const xub_StrLen *pSttIdx;
/*N*/ 	SwTxtAttr* pTxtHt;
/*N*/
/*N*/ 	for( USHORT nPos = 0; pSwpHints && nPos < pSwpHints->Count(); nPos++ )
/*N*/ 	{
/*N*/ 		pTxtHt = pSwpHints->GetHt( nPos );
/*N*/ 		const USHORT nWhich = pTxtHt->Which();
/*N*/ 		if( nWhich == nTxtWhich &&
/*N*/ 			*( pSttIdx = pTxtHt->GetStart()) == nStart )
/*N*/ 		{
/*N*/ 			pEndIdx = pTxtHt->GetEnd();
/*N*/
/*N*/ 			// Text-Attribute sind voellig dynamisch, so dass diese nur
/*N*/ 			// mit ihrer Start-Position verglichen werden.
/*N*/ 			if( !pEndIdx )
/*N*/ 			{
/*N*/ 				// Unbedingt Copy-konstruieren!
/*?*/ 				const SwIndex aIdx( this, *pSttIdx );
/*?*/ 				Erase( aIdx, 1 );
/*?*/ 				break;
/*N*/ 			}
/*N*/ 			else if( *pEndIdx == nEnd )
/*N*/ 			{
/*N*/ 				// den MsgHint jetzt fuettern, weil gleich sind
/*N*/ 				// Start und End weg.
/*N*/ 				// Das CalcVisibleFlag bei HiddenParaFields entfaellt,
/*N*/ 				// da dies das Feld im Dtor selbst erledigt.
/*N*/ 				SwUpdateAttr aHint( *pSttIdx, *pEndIdx, nTxtWhich );
/*N*/ 				pSwpHints->DeleteAtPos( nPos );    // gefunden, loeschen,
/*N*/ 				pTxtHt->RemoveFromPool( GetDoc()->GetAttrPool() );
/*N*/ 				delete pTxtHt;
/*N*/ 				SwModify::Modify( 0, &aHint );	   // die Frames benachrichtigen
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pSwpHints && pSwpHints->CanBeDeleted() )
/*N*/ 		DELETEZ( pSwpHints );
/*N*/ }

/*************************************************************************
 *						SwTxtNode::DelSoftHyph()
 *************************************************************************/


// setze diese Attribute am TextNode. Wird der gesamte Bereich umspannt,
// dann setze sie nur im AutoAttrSet (SwCntntNode:: SetAttr)
/*N*/ BOOL SwTxtNode::SetAttr( const SfxItemSet& rSet, xub_StrLen nStt,
/*N*/ 						 xub_StrLen nEnd, USHORT nMode )
/*N*/ {
/*N*/ 	if( !rSet.Count() )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	// teil die Sets auf (fuer Selektion in Nodes)
/*N*/ 	const SfxItemSet* pSet = &rSet;
/*N*/ 	SfxItemSet aTxtSet( *rSet.GetPool(), RES_TXTATR_BEGIN, RES_TXTATR_END-1 );
/*N*/
/*N*/ 	// gesamter Bereich
/*N*/ 	if( !nStt && nEnd == aText.Len() && !(nMode & SETATTR_NOFORMATATTR ) )
/*N*/ 	{
/*N*/ 		// sind am Node schon Zeichenvorlagen gesetzt, muss man diese Attribute
/*N*/ 		// (rSet) immer als TextAttribute setzen, damit sie angezeigt werden.
/*N*/ 		int bHasCharFmts = FALSE;
/*N*/ 		if( pSwpHints )
/*N*/ 			for( USHORT n = 0; n < pSwpHints->Count(); ++n )
/*N*/ 				if( (*pSwpHints)[ n ]->IsCharFmtAttr() )
/*N*/ 				{
/*N*/ 					bHasCharFmts = TRUE;
/*N*/ 					break;
/*N*/ 				}
/*N*/
/*N*/ 		if( !bHasCharFmts )
/*N*/ 		{
/*N*/ 			aTxtSet.Put( rSet );
/*N*/ 			if( aTxtSet.Count() != rSet.Count() )
/*N*/ 			{
/*N*/ 				BOOL bRet = SwCntntNode::SetAttr( rSet );
/*N*/ 				if( !aTxtSet.Count() )
/*N*/ 					return bRet;
/*N*/ 			}
/*N*/ 			pSet = &aTxtSet;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if ( !pSwpHints )
/*N*/ 		pSwpHints = new SwpHints();
/*N*/
/*N*/ 	USHORT nWhich, nCount = 0;
/*N*/ 	SwTxtAttr* pNew;
/*N*/ 	SfxItemIter aIter( *pSet );
/*N*/ 	const SfxPoolItem* pItem = aIter.GetCurItem();
/*N*/ 	do {
/*N*/ 		if( pItem && (SfxPoolItem*)-1 != pItem &&
/*N*/ 			(( RES_CHRATR_BEGIN <= ( nWhich = pItem->Which()) &&
/*N*/ 			  RES_CHRATR_END > nWhich ) ||
/*N*/ 			( RES_TXTATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich ) ||
/*N*/ 			( RES_UNKNOWNATR_BEGIN <= nWhich && RES_UNKNOWNATR_END > nWhich )) )
/*N*/ 		{
/*N*/ 			if( RES_TXTATR_CHARFMT == pItem->Which() &&
/*N*/ 				GetDoc()->GetDfltCharFmt()==((SwFmtCharFmt*)pItem)->GetCharFmt())
/*N*/ 			{
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				SwIndex aIndex( this, nStt );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pNew = MakeTxtAttr( *pItem, nStt, nEnd );
/*N*/ 				if( pNew )
/*N*/ 				{
/*N*/ 					// Attribut ohne Ende, aber Bereich markiert ?
/*N*/ 					if( nEnd != nStt && !pNew->GetEnd() )
/*N*/ 					{
/*?*/ 						ASSERT( !this, "Attribut ohne Ende aber Bereich vorgegeben" );
/*?*/ 						DestroyAttr( pNew );		// nicht einfuegen
/*N*/ 					}
/*N*/ 					else if( Insert( pNew, nMode ))
/*N*/ 						++nCount;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( aIter.IsAtEnd() )
/*N*/ 			break;
/*?*/ 		pItem = aIter.NextItem();
/*N*/ 	} while( TRUE );
/*N*/
/*N*/ 	if( pSwpHints && pSwpHints->CanBeDeleted() )
/*?*/ 		DELETEZ( pSwpHints );
/*N*/
/*N*/ 	return nCount ? TRUE : FALSE;
/*N*/ }

/*N*/ BOOL lcl_Included( const USHORT nWhich, const SwTxtAttr *pAttr )
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	static long nTest = 0;
/*N*/ 	++nTest;
/*N*/ #endif
/*N*/ 	BOOL bRet;
/*N*/ 	SwCharFmt* pFmt = RES_TXTATR_INETFMT == pAttr->Which() ?
/*N*/ 						((SwTxtINetFmt*)pAttr)->GetCharFmt() :
/*N*/ 						pAttr->GetCharFmt().GetCharFmt();
/*N*/ 	if( pFmt )
/*N*/ 		bRet = SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState( nWhich, TRUE );
/*N*/ 	else
/*N*/ 		bRet = FALSE;
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void lcl_MergeAttr( SfxItemSet& rSet, const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	rSet.Put( rAttr );
/*N*/ }

/*N*/ void lcl_MergeAttr_ExpandChrFmt( SfxItemSet& rSet, const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	if( RES_TXTATR_CHARFMT == rAttr.Which() ||
/*N*/ 		RES_TXTATR_INETFMT == rAttr.Which() )
/*N*/ 	{
/*N*/ 		// aus der Vorlage die Attribute holen:
/*N*/ 		SwCharFmt* pFmt = RES_TXTATR_INETFMT == rAttr.Which() ?
/*N*/ 						((SwFmtINetFmt&)rAttr).GetTxtINetFmt()->GetCharFmt() :
/*N*/ 						((SwFmtCharFmt&)rAttr).GetCharFmt();
/*N*/ 		if( pFmt )
/*N*/ 		{
/*N*/ 			const SfxItemSet& rCFSet = pFmt->GetAttrSet();
/*N*/ 			SfxWhichIter aIter( rCFSet );
/*N*/ 			register USHORT nWhich = aIter.FirstWhich();
/*N*/ 			while( nWhich )
/*N*/ 			{
/*N*/ 				if( ( nWhich < RES_CHRATR_END ) &&
/*N*/ 					( SFX_ITEM_SET == rCFSet.GetItemState( nWhich, TRUE ) ) )
/*N*/ 					rSet.Put( rCFSet.Get( nWhich ) );
/*N*/ 				nWhich = aIter.NextWhich();
/*N*/ 			}
/*N*/ #if 0
/*N*/ 			SfxItemSet aTmpSet( *rSet.GetPool(), rSet.GetRanges() );
/*N*/ 			aTmpSet.Set( pFmt->GetAttrSet(), TRUE );
 /*
 ????? JP 31.01.95 ????	wie jetzt ???
             rSet.MergeValues( aTmpSet );

             // jetzt alle zusammen "mergen"
             rSet.Differentiate( aTmpSet );
 */
/*N*/ 			rSet.Put( aTmpSet );
/*N*/ #endif
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// aufnehmen als MergeWert (falls noch nicht gesetzt neu setzen!)
/*N*/ #if 0
 /* wenn mehrere Attribute ueberlappen werden diese gemergt !!
  z.B
             1234567890123456789
               |------------| 		Font1
                  |------|			Font2
                     ^  ^
                     |--|		Abfragebereich: -> uneindeutig
 */
/*N*/ 	else if( SFX_ITEM_DEFAULT == rSet.GetItemState( rAttr.Which(), FALSE ))
/*N*/ 		rSet.Put( rAttr );
/*N*/ 	else
/*N*/ 		rSet.MergeValue( rAttr );
/*N*/ #else
 /* wenn mehrere Attribute ueberlappen gewinnt der letze !!
  z.B
             1234567890123456789
               |------------| 		Font1
                  |------|			Font2
                     ^  ^
                     |--|		Abfragebereich: -> Gueltig ist Font2
 */
/*N*/ 		rSet.Put( rAttr );
/*N*/ #endif
/*N*/ }

// erfrage die Attribute vom TextNode ueber den Bereich
/*N*/ BOOL SwTxtNode::GetAttr( SfxItemSet& rSet, xub_StrLen nStart, xub_StrLen nEnd,
/*N*/ 							BOOL bOnlyTxtAttr, BOOL bGetFromChrFmt ) const
/*N*/ {
/*N*/ 	if( pSwpHints )
/*N*/ 	{
         /* stelle erstmal fest, welche Text-Attribut in dem Bereich gueltig
          * sind. Dabei gibt es folgende Faelle:
          * 	UnEindeutig wenn: (wenn != Format-Attribut)
          * 		- das Attribut liegt vollstaendig im Bereich
          * 		- das Attributende liegt im Bereich
          * 		- der Attributanfang liegt im Bereich:
          * Eindeutig (im Set mergen):
          *		- das Attrib umfasst den Bereich
          * nichts tun:
          * 		das Attribut liegt ausserhalb des Bereiches
          */
/*N*/
/*N*/ 		void (*fnMergeAttr)( SfxItemSet&, const SfxPoolItem& )
/*N*/ 			= bGetFromChrFmt ? &lcl_MergeAttr_ExpandChrFmt
/*N*/ 							 : &lcl_MergeAttr;
/*N*/
/*N*/ 		// dann besorge mal die Auto-(Fmt)Attribute
/*N*/ 		SfxItemSet aFmtSet( *rSet.GetPool(), rSet.GetRanges() );
/*N*/ 		if( !bOnlyTxtAttr )
/*N*/ 			SwCntntNode::GetAttr( aFmtSet );
/*N*/
/*N*/ 		const USHORT nSize = pSwpHints->Count();
/*N*/ 		register USHORT n;
/*N*/ 	    register xub_StrLen nAttrStart;
/*N*/ 		register const xub_StrLen* pAttrEnd;
/*N*/
/*N*/ 		if( nStart == nEnd )				// kein Bereich:
/*N*/ 		{
/*N*/ 			for( n = 0; n < nSize; ++n )		//
/*N*/ 			{
/*N*/ 				const SwTxtAttr* pHt = (*pSwpHints)[n];
/*N*/ 				nAttrStart = *pHt->GetStart();
/*N*/ 				if( nAttrStart > nEnd ) 		// ueber den Bereich hinaus
/*N*/ 					break;
/*N*/
/*N*/ 				if( 0 == ( pAttrEnd = pHt->GetEnd() ))		// nie Attribute ohne Ende
/*N*/ 					continue;
/*N*/
/*N*/ 				if( ( nAttrStart < nStart &&
/*N*/ 						( pHt->DontExpand() ? nStart < *pAttrEnd
/*N*/ 											: nStart <= *pAttrEnd )) ||
/*N*/ 					( nStart == nAttrStart &&
/*N*/ 						( nAttrStart == *pAttrEnd || !nStart )))
/*N*/ 					(*fnMergeAttr)( rSet, pHt->GetAttr() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else							// es ist ein Bereich definiert
/*N*/ 		{
/*N*/ 			SwTxtAttr** aAttrArr = 0;
/*N*/ 			const USHORT coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN +
/*N*/ 									( RES_UNKNOWNATR_END -
/*N*/ 													RES_UNKNOWNATR_BEGIN );
/*N*/
/*N*/ 			for( n = 0; n < nSize; ++n )
/*N*/ 			{
/*N*/ 				const SwTxtAttr* pHt = (*pSwpHints)[n];
/*N*/ 				nAttrStart = *pHt->GetStart();
/*N*/ 				if( nAttrStart > nEnd ) 		// ueber den Bereich hinaus
/*N*/ 					break;
/*N*/
/*N*/ 				if( 0 == ( pAttrEnd = pHt->GetEnd() ))		// nie Attribute ohne Ende
/*N*/ 					continue;
/*N*/
/*N*/ 				BOOL bChkInvalid = FALSE;
/*N*/ 				if( nAttrStart <= nStart )		// vor oder genau Start
/*N*/ 				{
/*N*/ 					if( *pAttrEnd <= nStart )	// liegt davor
/*N*/ 						continue;
/*N*/
/*N*/ 					if( nEnd <= *pAttrEnd )		// hinter oder genau Ende
/*N*/ 						(*fnMergeAttr)( aFmtSet, pHt->GetAttr() );
/*N*/ 					else
/*N*/ //					else if( pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
/*N*/ 						// uneindeutig
/*N*/ 						bChkInvalid = TRUE;
/*N*/ 				}
/*N*/ 				else if( nAttrStart < nEnd 		// reicht in den Bereich
/*N*/ )//						 && pHt->GetAttr() != aFmtSet.Get( pHt->Which() ) )
/*N*/ 					bChkInvalid = TRUE;
/*N*/
/*N*/ 				if( bChkInvalid )
/*N*/ 				{
/*?*/ 					// uneindeutig ?
/*?*/ 					if( !aAttrArr )
/*?*/ 					{
/*?*/ 						aAttrArr = new SwTxtAttr* [ coArrSz ];
/*?*/ 						memset( aAttrArr, 0, sizeof( SwTxtAttr* ) * coArrSz );
/*?*/ 					}
/*?*/
/*?*/ 					const SwTxtAttr** ppPrev;
/*?*/ 					if( RES_CHRATR_BEGIN <= pHt->Which() &&
/*?*/ 						pHt->Which() < RES_TXTATR_WITHEND_END )
/*?*/ 					 	ppPrev = (const SwTxtAttr**)&aAttrArr[
/*?*/ 										pHt->Which() - RES_CHRATR_BEGIN ];
/*?*/ 					else if( RES_UNKNOWNATR_BEGIN <= pHt->Which() &&
/*?*/ 						pHt->Which() < RES_UNKNOWNATR_END )
/*?*/ 					 	ppPrev = (const SwTxtAttr**)&aAttrArr[
/*?*/ 										pHt->Which() - RES_UNKNOWNATR_BEGIN
/*?*/ 										+ ( RES_TXTATR_WITHEND_END -
/*?*/ 											RES_CHRATR_BEGIN ) ];
/*?*/ 					else
/*?*/ 						ppPrev = 0;
/*?*/
/*?*/ 					if( !*ppPrev )
/*?*/ 					{
/*?*/ 						if( nAttrStart > nStart )
/*?*/ 						{
/*?*/ 							rSet.InvalidateItem( pHt->Which() );
/*?*/ 							*ppPrev = (SwTxtAttr*)-1;
/*?*/ 						}
/*?*/ 						else
/*?*/ 							*ppPrev = pHt;
/*?*/ 					}
/*?*/ 					else if( (SwTxtAttr*)-1 != *ppPrev )
/*?*/ 					{
/*?*/ 						if( *(*ppPrev)->GetEnd() == nAttrStart &&
/*?*/ 							(*ppPrev)->GetAttr() == pHt->GetAttr() )
/*?*/ 							*ppPrev = pHt;
/*?*/ 						else
/*?*/ 						{
/*?*/ 							rSet.InvalidateItem( pHt->Which() );
/*?*/ 							*ppPrev = (SwTxtAttr*)-1;
/*?*/ 						}
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( aAttrArr )
/*N*/ 			{
/*?*/ 				const SwTxtAttr* pAttr;
/*?*/ 				for( n = 0; n < coArrSz; ++n )
/*?*/ 					if( 0 != ( pAttr = aAttrArr[ n ] ) &&
/*?*/ 						(SwTxtAttr*)-1 != pAttr )
/*?*/ 					{
/*?*/ 						USHORT nWh;
/*?*/ 						if( n < (RES_TXTATR_WITHEND_END -
/*?*/ 											RES_CHRATR_BEGIN ))
/*?*/ 							nWh = n + RES_CHRATR_BEGIN;
/*?*/ 						else
/*?*/ 							nWh = n - ( RES_TXTATR_WITHEND_END -
/*?*/ 												  RES_CHRATR_BEGIN ) +
/*?*/ 												RES_UNKNOWNATR_BEGIN;
/*?*/
/*?*/ 						if( nEnd <= *pAttr->GetEnd() )	// hinter oder genau Ende
/*?*/ 						{
/*?*/ 							if( pAttr->GetAttr() != aFmtSet.Get( nWh ) )
/*?*/ 								(*fnMergeAttr)( rSet, pAttr->GetAttr() );
/*?*/ 						}
/*?*/ 						else
/*?*/ 							// uneindeutig
/*?*/ 							rSet.InvalidateItem( nWh );
/*?*/ 					}
/*?*/ 				delete [] aAttrArr;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( aFmtSet.Count() )
/*N*/ 		{
/*N*/ 			// aus dem Format-Set alle entfernen, die im TextSet auch gesetzt sind
/*N*/ 			aFmtSet.Differentiate( rSet );
/*N*/ 			// jetzt alle zusammen "mergen"
/*N*/ 			rSet.Put( aFmtSet );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( !bOnlyTxtAttr )
/*N*/ 		// dann besorge mal die Auto-(Fmt)Attribute
/*N*/ 		SwCntntNode::GetAttr( rSet );
/*N*/
/*N*/ 	return rSet.Count() ? TRUE : FALSE;
/*N*/ }


/*N*/ void SwTxtNode::FmtToTxtAttr( SwTxtNode* pNd )
/*N*/ {
/*N*/ 	SfxItemSet aThisSet( GetDoc()->GetAttrPool(), aCharFmtSetRange );
/*N*/ 	if( GetpSwAttrSet() && GetpSwAttrSet()->Count() )
/*?*/ 		aThisSet.Put( *GetpSwAttrSet() );
/*N*/
/*N*/ 	if ( !pSwpHints )
/*N*/ 		pSwpHints = new SwpHints();
/*N*/
/*N*/ 	if( pNd == this )
/*N*/ 	{
/*?*/ 		if( aThisSet.Count() )
/*?*/ 		{
/*?*/ 			SfxItemIter aIter( aThisSet );
/*?*/ 			const SfxPoolItem* pItem = aIter.GetCurItem();
/*?*/ 			while( TRUE )
/*?*/ 			{
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				if( lcl_IsNewAttrInSet( *pSwpHints, *pItem, GetTxt().Len() ) )
/*?*/ 			}
/*?*/ 		}
/*N*/
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SfxItemSet aNdSet( pNd->GetDoc()->GetAttrPool(), aCharFmtSetRange );
/*N*/ 		if( pNd->GetpSwAttrSet() && pNd->GetpSwAttrSet()->Count() )
/*?*/ 			aNdSet.Put( *pNd->GetpSwAttrSet() );
/*N*/
/*N*/ 		if ( !pNd->pSwpHints )
/*N*/ 			pNd->pSwpHints = new SwpHints();
/*N*/
/*N*/ 		if( aThisSet.Count() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SfxItemIter aIter( aThisSet );
/*N*/ 		}
/*N*/
/*N*/ 		if( aNdSet.Count() )
/*N*/ 		{
/*?*/ 			SfxItemIter aIter( aNdSet );
/*?*/ 			const SfxPoolItem* pItem = aIter.GetCurItem();
/*?*/ 			while( TRUE )
/*?*/ 			{
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				if( lcl_IsNewAttrInSet( *pNd->pSwpHints, *pItem, pNd->GetTxt().Len() ) )
/*?*/ 			}
/*?*/
/*?*/ 			SwFmtChg aTmp1( pNd->GetFmtColl() );
/*?*/ 			pNd->SwModify::Modify( &aTmp1, &aTmp1 );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if( pNd->pSwpHints->CanBeDeleted() )
/*N*/ 		DELETEZ( pNd->pSwpHints );
/*N*/ }

/*************************************************************************
 *						SwpHints::CalcFlags()
 *************************************************************************/

/*N*/ void SwpHints::CalcFlags()
/*N*/ {
/*N*/ 	bDDEFlds = bFtn = FALSE;
/*N*/ 	const USHORT nSize = Count();
/*N*/ 	const SwTxtAttr* pAttr;
/*N*/ 	for( USHORT nPos = 0; nPos < nSize; ++nPos )
/*N*/ 		switch( ( pAttr = (*this)[ nPos ])->Which() )
/*N*/ 		{
/*N*/ 		case RES_TXTATR_FTN:
/*N*/ 			bFtn = TRUE;
/*N*/ 			if( bDDEFlds )
/*N*/ 				return;
/*N*/ 			break;
/*N*/ 		case RES_TXTATR_FIELD:
/*N*/ 			{
/*N*/ 				const SwField* pFld = pAttr->GetFld().GetFld();
/*N*/ 				if( RES_DDEFLD == pFld->GetTyp()->Which() )
/*N*/ 				{
/*N*/ 					bDDEFlds = TRUE;
/*N*/ 					if( bFtn )
/*N*/ 						return;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ }

/*************************************************************************
 *						SwpHints::CalcVisibleFlag()
 *************************************************************************/

/*N*/ BOOL SwpHints::CalcVisibleFlag()
/*N*/ {
/*N*/ 	BOOL bOldVis = bVis;
/*N*/ 	bCalcVis = FALSE;
/*N*/ 	BOOL   			bNewVis  = TRUE;
/*N*/ 	const USHORT	nSize = Count();
/*N*/ 	const SwTxtAttr *pTxtHt;
/*N*/
/*N*/ 	for( USHORT nPos = 0; nPos < nSize; ++nPos )
/*N*/ 	{
/*N*/ 		pTxtHt = (*this)[ nPos ];
/*N*/ 		const USHORT nWhich = pTxtHt->Which();
/*N*/
/*N*/ 		if( RES_TXTATR_FIELD == nWhich )
/*N*/ 		{
/*N*/ 			const SwFmtFld& rFld = pTxtHt->GetFld();
/*N*/ 			if( RES_HIDDENPARAFLD == rFld.GetFld()->GetTyp()->Which())
/*N*/ 			{
/*N*/ 				if( !((SwHiddenParaField*)rFld.GetFld())->IsHidden() )
/*N*/ 				{
/*N*/ 					SetVisible(TRUE);
/*N*/ 					return !bOldVis;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bNewVis = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetVisible( bNewVis );
/*N*/ 	return bOldVis != bNewVis;
/*N*/ }


/*************************************************************************
 *						SwpHints::Resort()
 *************************************************************************/

// Ein Hint hat seinen Anfangswert geaendert.
// Hat sich dadurch die Sortierreihenfolge
// geaendert, so wird solange umsortiert, bis
// sie wieder stimmt.

/*N*/ BOOL SwpHints::Resort( const USHORT nPos )
/*N*/ {
/*N*/ 	const SwTxtAttr *pTmp;
/*N*/
/*N*/ 	if ( ( nPos+1 < Count() &&
/*N*/ 		   *(*this)[nPos]->GetStart() > *(*this)[nPos+1]->GetStart() ) ||
/*N*/ 		 ( nPos > 0 &&
/*N*/ 		   *(*this)[nPos]->GetStart() < *(*this)[nPos-1]->GetStart() ) )
/*N*/ 	{
/*?*/ 		pTmp = (*this)[nPos];
/*?*/ 		DeleteAtPos( nPos );
/*?*/ 		SwpHintsArr::Insert( pTmp );
/*?*/ 		// Wenn tatsaechlich umsortiert wurde, muss die
/*?*/ 		// Position i nochmal bearbeitet werden.
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*************************************************************************
 *						SwpHints::ClearDummies()
 *************************************************************************/

/*
 * ClearDummies: Hints, die genau den gleichen Bereich umfassen wie
 * ein nachfolgender mit gleichem Attribut oder eine nachfolgende Zeichen-
 * vorlage, muessen entfernt werden.
 * Solche Hints entstehen, wenn sie urspruenglich einen groesseren, den
 * Nachfolger umfassenden Bereich hatten, die Aussenanteile aber durch
 * SwTxtNode::RstAttr oder SwTxtNode::Update geloescht wurden.
 */

/*N*/ void SwpHints::ClearDummies( SwTxtNode &rNode )
/*N*/ {
/*N*/ 	USHORT i = 0;
/*N*/ 	while ( i < Count() )
/*N*/ 	{
/*N*/ 		SwTxtAttr *pHt = GetHt( i++ );
/*N*/ 		const USHORT nWhich = pHt->Which();
/*N*/ 		const xub_StrLen *pEnd = pHt->GetEnd();
/*N*/ 		if ( pEnd && !pHt->IsOverlapAllowedAttr() && !pHt->IsCharFmtAttr() )
/*N*/ 			for( USHORT j = i; j < Count(); ++j  )
/*N*/ 			{
/*N*/ 				SwTxtAttr *pOther = GetHt(j);
/*N*/ 				if ( *pOther->GetStart() > *pHt->GetStart() )
/*N*/ 					break;
/*N*/
/*N*/ 				if( pOther->Which() == nWhich || pOther->IsCharFmtAttr() )
/*N*/ 				{
/*N*/ 					//JP 03.10.95: nicht zusammenfassen, zu kompliziert
/*N*/ 					//			fuer WIN95-Compiler!!
/*?*/ 					if( *pEnd == *pOther->GetEnd() &&
/*?*/ 						( pOther->Which() == nWhich ||
/*?*/ 						  lcl_Included( nWhich, pOther ) ) )
/*?*/ 					{
/*?*/ 						rNode.DestroyAttr( Cut( --i ) );
/*?*/ 						break;
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwpHints::Merge( )
 *************************************************************************/

/*
 * Merge: Gleichartigen, gleichwertige Hints, die aneinandergrenzen,
 * koennen verschmolzen werden, wenn an ihrer gemeinsamen Kante nicht ein
 * gleichartiges Attribut endet oder beginnt, welches einen von der beiden
 * Kandidaten umfasst, auch Zeichenvorlage duerfen nicht ueberlappt werden.
 */
/*-----------------5.9.2001 09:26-------------------
 * And here a smarter version of good old Merge(..)
 * If a SwTxtAttr is given, only the merging between this attribute and the
 * neighbours are checked. This saves time, if no attribute is given (in more
 * complex situations), the whole attribute array is checked as before.
 * --------------------------------------------------*/

/*N*/ BOOL SwpHints::Merge( SwTxtNode &rNode, SwTxtAttr* pAttr )
/*N*/ {
/*N*/     USHORT i;
/*N*/ 	BOOL bMerged = FALSE;
/*N*/     if( pAttr && USHRT_MAX != ( i = GetPos( pAttr ) ) )
/*N*/     {
/*N*/         const xub_StrLen *pEnd = pAttr->GetEnd();
/*N*/         if ( pEnd && !pAttr->IsDontMergeAttr() )
/*N*/ 		{
/*N*/             USHORT j = i;
/*N*/             const USHORT nWhich = pAttr->Which();
/*N*/             const USHORT nStart = *pAttr->GetStart();
/*N*/             const USHORT nEnd = *pEnd;
/*N*/             SwTxtAttr *pPrev;
/*N*/             SwTxtAttr *pNext;
/*N*/             USHORT nEndIdx = aHtEnd.C40_GETPOS( SwTxtAttr, pAttr );
/*N*/             ASSERT( USHRT_MAX != nEndIdx, "Missing end index" );
/*N*/             if( nEndIdx )
/*N*/             {
/*N*/                 // If there's a attribute with same start and which-id,
/*N*/                 // it's not possible to merge with a previous attribute.
/*N*/                 pPrev = pAttr;
/*N*/                 while( j )
/*N*/                 {
/*N*/                     SwTxtAttr *pOther = GetHt(--j);
/*N*/                     if ( *pOther->GetStart() < nStart )
/*N*/                         break;
/*N*/                     if( pOther->Which() == nWhich || pOther->IsCharFmtAttr()
/*N*/                         || pAttr->IsCharFmtAttr() )
/*N*/                     {
/*N*/                         pPrev = NULL;
/*N*/                         break;
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/                 pPrev = NULL;
/*N*/             if( i + 1 < Count() )
/*N*/             {
/*N*/                 // If there's a attribute with same end and which-id,
/*N*/                 // it's not possible to merge with a following attribute.
/*N*/                 j = nEndIdx + 1;
/*N*/                 pNext = pAttr;
/*N*/                 while( j < Count() )
/*N*/                 {
/*N*/                     SwTxtAttr *pOther = GetEnd( j++ );
/*N*/                     if( !pOther->GetEnd() )
/*N*/                         continue;
/*N*/                     if( *pOther->GetEnd() > nEnd )
/*N*/                         break;
/*N*/                     if( pOther->Which() == nWhich || pOther->IsCharFmtAttr()
/*N*/                         || pAttr->IsCharFmtAttr() )
/*N*/                     {
/*N*/                         pNext = NULL;
/*N*/                         break;
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/                 pNext = NULL;
/*N*/             if( pPrev )
/*N*/             {
/*N*/                 pPrev = NULL;
/*N*/                 j = nEndIdx;
/*N*/                 do
/*N*/                 {
/*N*/                     // Looking for a previous attribute with the same which-id
/*N*/                     // which ends exact at the start of the given attribute
/*N*/                     SwTxtAttr *pOther = GetEnd( --j );
/*N*/                     pEnd = pOther->GetEnd();
/*N*/                     if( !pEnd || *pEnd > nStart )
/*N*/                         continue;
/*N*/                     if( *pEnd < nStart )
/*N*/                         break;
/*N*/                     if( pOther->Which() == nWhich )
/*N*/                     {
/*N*/                         if( pOther->GetAttr() == pAttr->GetAttr() )
/*N*/                         {
/*N*/                             bMerged = TRUE;
/*N*/                             pPrev = pOther;
/*N*/                         }
/*N*/                         else
/*N*/                             pPrev = NULL;
/*N*/                         break;
/*N*/                     }
/*N*/                     if( pOther->IsCharFmtAttr() || pAttr->IsCharFmtAttr() )
/*N*/                     {
/*N*/                         pPrev = NULL;
/*N*/                         break;
/*N*/                     }
/*N*/                 } while ( j );
/*N*/             }
/*N*/             if( pNext )
/*N*/             {
/*N*/                 j = i + 1;
/*N*/                 pNext = NULL;
/*N*/                 while( j < Count() )
/*N*/                 {
/*N*/                     // Looking for a following attribute with the same which-id
/*N*/                     // which starts at the end of the given attribute
/*N*/                     SwTxtAttr *pOther = GetStart( j++ );
/*N*/                     pEnd = pOther->GetEnd();
/*N*/                     if( !pEnd || *pOther->GetStart() < nEnd )
/*N*/                         continue;
/*N*/                     if( *pOther->GetStart() > nEnd )
/*N*/                         break;
/*N*/                     if( pOther->Which() == nWhich )
/*N*/                     {
/*N*/                         if( pOther->GetAttr() == pAttr->GetAttr() )
/*N*/                         {
/*N*/                             bMerged = TRUE;
/*N*/                             pNext = pOther;
/*N*/                         }
/*N*/                         else
/*N*/                             pNext = NULL;
/*N*/                         break;
/*N*/                     }
/*?*/                     if( pOther->IsCharFmtAttr() || pAttr->IsCharFmtAttr() )
/*?*/                     {
/*?*/                         pNext = NULL;
/*?*/                         break;
/*?*/                     }
/*N*/                 }
/*N*/             }
/*N*/             if( bMerged )
/*N*/             {
/*N*/                 if( pNext )
/*N*/                 {
/*N*/                     if ( pPrev == pNext )
/*N*/                         pPrev = NULL;
/*N*/
/*?*/                     *pAttr->GetEnd() = *pNext->GetEnd();
/*?*/                     pAttr->SetDontExpand( FALSE );
/*?*/                     rNode.DestroyAttr( Cut( j - 1 ) );
/*N*/                 }
/*N*/                 if( pPrev )
/*N*/                 {
/*N*/                     *pPrev->GetEnd() = *pAttr->GetEnd();
/*N*/                     pPrev->SetDontExpand( FALSE );
/*N*/                     rNode.DestroyAttr( Cut( i ) );
/*N*/                 }
/*N*/                 return TRUE;
/*N*/             }
/*N*/             return FALSE;
/*N*/         }
/*N*/     }
/*N*/     i = 0;
/*N*/ 	while ( i < Count() )
/*N*/ 	{
/*N*/ 		SwTxtAttr *pHt = GetHt( i++ );
/*N*/ 		const xub_StrLen *pEnd = pHt->GetEnd();
/*N*/ 		if ( pEnd && !pHt->IsDontMergeAttr() )
/*N*/ 		{
/*N*/ 			const USHORT nWhich = pHt->Which();
/*N*/ 			for ( USHORT j = i; j < Count(); j++  )
/*N*/ 			{
/*N*/ 				SwTxtAttr *pOther = GetHt(j);
/*N*/ 				if ( *pOther->GetStart() > *pEnd )
/*N*/ 					break;	 // keine beruehrenden Attribute mehr vorhanden
/*N*/
/*N*/ 				if( *pOther->GetStart() == *pEnd &&
/*N*/ 					 ( pOther->Which() == nWhich ||
/*N*/ 					   pOther->IsCharFmtAttr() ||
/*N*/ 					   ( pHt->IsCharFmtAttr() && !pHt->IsDontMergeAttr() )))
/*N*/ 				{
/*?*/ 					// Beruehrendes Attribut gefunden mit gleichem Typ bzw.
/*?*/ 					// Zeichenvorlage.
/*?*/ 					// Bei Attribut mit anderem Wert bzw. Zeichenvorlage
/*?*/ 					// ist keine Verschmelzung mehr moeglich
/*?*/ 					if( pOther->Which() == nWhich &&
/*?*/ 						 pOther->GetAttr() == pHt->GetAttr() )
/*?*/ 					{
/*?*/ 						// Unser Partner pOther erfuellt alle Voraussetzungen,
/*?*/ 						// jetzt muessen wir uns selbst noch ueberpruefen, ob wir
/*?*/ 						// nicht von einem gleichartigen Attribut oder einer
/*?*/ 						// Zeichenvorlage umfasst werden, die das gleiche Ende
/*?*/ 						// wie wir selbst haben.
/*?*/ 						BOOL bMerge = TRUE;
/*?*/ 						for ( USHORT k = 0; k+1 < i; k++ )
/*?*/ 						{
/*?*/ 							SwTxtAttr *pAnOther = GetHt(k);
/*?*/ 							if( ( pAnOther->Which() == nWhich ||
/*?*/ 								  pAnOther->IsCharFmtAttr() ||
/*?*/ 								  pHt->IsCharFmtAttr() )
/*?*/ 								 && pAnOther->GetEnd()
/*?*/ 								 && *pAnOther->GetEnd() == *pEnd )
/*?*/ 							{
/*?*/ 								bMerge = FALSE; // kein Verschmelzen von i+j
/*?*/ 								break;
/*?*/ 							}
/*?*/ 						}
/*?*/ 						if ( bMerge )
/*?*/ 						{
/*?*/ 							*pHt->GetEnd() = *pOther->GetEnd();
/*?*/ 							pHt->SetDontExpand( FALSE );
/*?*/ 							rNode.DestroyAttr( Cut( j ) );
/*?*/ 							--i;
/*?*/ 							bMerged = TRUE;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bMerged )
/*?*/ 		ClearDummies( rNode );
/*N*/ 	return bMerged;
/*N*/ }

/*************************************************************************
 *						SwpHints::Forget( ... )
 *************************************************************************/

/*
 * Forget: Hints, die genau den gleichen Bereich umfassen wie
 * ein nachfolgender mit gleichem Attribut oder eine nachfolgende Zeichen-
 * vorlage, duerfen nicht eingefuegt werden.
 * Solche Hints koennen entstehen, wenn durch SwTxtNode::RstAttr
 * ein Attribut in zwei Teile zerlegt wird und der zweite Teil einen
 * identischen Bereich mit einem inneren Attribut bekaeme.
 */


/*************************************************************************
 *						SwpHints::Insert()
 *************************************************************************/

/*
 * Insert: Der neue Hint wird immer eingefuegt. Wenn dabei ein
 * ueberlappender oder gleicher Hintbereich mit gleichem Attribut
 * und Wert gefunden, wird der neue Hint entsprechend veraendert
 * und der alte herausgenommen (und zerstoert:
 * SwpHints::Destroy()).
 */

/*N*/ void SwpHints::Insert( SwTxtAttr *pHint, SwTxtNode &rNode, USHORT nMode )
/*N*/ {
/*N*/ 	// Irgendwann ist immer Schluss
/*N*/     if( USHRT_MAX == Count() )
/*N*/ 		return;
/*N*/
/*N*/ 	// Felder bilden eine Ausnahme:
/*N*/ 	// 1) Sie koennen nie ueberlappen
/*N*/ 	// 2) Wenn zwei Felder genau aneinander liegen,
/*N*/ 	//	  sollen sie nicht zu einem verschmolzen werden.
/*N*/ 	// Wir koennen also auf die while-Schleife verzichten
/*N*/
/*N*/ 	xub_StrLen *pHtEnd = pHint->GetEnd();
/*N*/ 	USHORT nWhich = pHint->Which();
/*N*/
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 	case RES_TXTATR_CHARFMT:
/*N*/ 		((SwTxtCharFmt*)pHint)->ChgTxtNode( &rNode );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_INETFMT:
/*N*/ 		{
/*N*/ 			((SwTxtINetFmt*)pHint)->ChgTxtNode( &rNode );
/*N*/ 			SwCharFmt* pFmt = rNode.GetDoc()->GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
/*N*/ 			pFmt->Add( (SwTxtINetFmt*)pHint );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_FIELD:
/*N*/ 		{
/*N*/ 			BOOL bDelFirst = 0 != ((SwTxtFld*)pHint)->GetpTxtNode();
/*N*/ 			((SwTxtFld*)pHint)->ChgTxtNode( &rNode );
/*N*/ 			SwDoc* pDoc = rNode.GetDoc();
/*N*/ 			const SwField* pFld = ((SwTxtFld*)pHint)->GetFld().GetFld();
/*N*/
/*N*/ 			if( !pDoc->IsNewFldLst() )
/*N*/ 			{
/*N*/ 				// was fuer ein Feld ist es denn ??
/*N*/ 				// bestimmte Felder mussen am Doc das Calculations-Flag updaten
/*N*/ 				switch( pFld->GetTyp()->Which() )
/*N*/ 				{
/*N*/ 				case RES_DBFLD:
/*N*/ 				case RES_SETEXPFLD:
/*N*/ 				case RES_HIDDENPARAFLD:
/*N*/ 				case RES_HIDDENTXTFLD:
/*N*/ 				case RES_DBNUMSETFLD:
/*N*/ 				case RES_DBNEXTSETFLD:
/*N*/ 					{
/*N*/ 						if( bDelFirst )
/*?*/ 							pDoc->InsDelFldInFldLst( FALSE, *(SwTxtFld*)pHint );
/*N*/ 						if( rNode.GetNodes().IsDocNodes() )
/*N*/ 							pDoc->InsDelFldInFldLst( TRUE, *(SwTxtFld*)pHint );
/*N*/ 					}
/*N*/ 					break;
/*?*/ 				case RES_DDEFLD:
/*?*/ 					if( rNode.GetNodes().IsDocNodes() )
/*?*/ 						((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
/*?*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			// gehts ins normale Nodes-Array?
/*N*/ 			if( rNode.GetNodes().IsDocNodes() )
/*N*/ 			{
/*N*/ 				BOOL bInsFldType = FALSE;
/*N*/ 				switch( pFld->GetTyp()->Which() )
/*N*/ 				{
/*N*/ 				case RES_SETEXPFLD:
/*N*/ 					bInsFldType = ((SwSetExpFieldType*)pFld->GetTyp())->IsDeleted();
/*N*/ 					if( GSE_SEQ & ((SwSetExpFieldType*)pFld->GetTyp())->GetType() )
/*N*/ 					{
/*N*/ 						// bevor die ReferenzNummer gesetzt wird, sollte
/*N*/ 						// das Feld am richtigen FeldTypen haengen!
/*N*/ 						SwSetExpFieldType* pFldType = (SwSetExpFieldType*)
/*N*/ 									pDoc->InsertFldType( *pFld->GetTyp() );
/*N*/ 						if( pFldType != pFld->GetTyp() )
/*N*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFmtFld* pFmtFld = (SwFmtFld*)&((SwTxtFld*)pHint)
/*N*/ 						}
/*N*/ 						pFldType->SetSeqRefNo( *(SwSetExpField*)pFld );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case RES_USERFLD:
/*N*/ 					bInsFldType = ((SwUserFieldType*)pFld->GetTyp())->IsDeleted();
/*N*/ 					break;
/*N*/
/*N*/ 				case RES_DDEFLD:
/*N*/ 					if( pDoc->IsNewFldLst() )
/*N*/ 						((SwDDEFieldType*)pFld->GetTyp())->IncRefCnt();
/*N*/ 					bInsFldType = ((SwDDEFieldType*)pFld->GetTyp())->IsDeleted();
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				if( bInsFldType )
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pDoc->InsDeletedFldType( *pFld->GetTyp() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_FTN :
/*N*/ 		((SwTxtFtn*)pHint)->ChgTxtNode( &rNode );
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_REFMARK:
/*N*/ 		((SwTxtRefMark*)pHint)->ChgTxtNode( &rNode );
/*N*/ 		if( rNode.GetNodes().IsDocNodes() )
/*N*/ 		{
/*N*/ 			//search for a refernce with the same name
/*N*/ 			SwTxtAttr* pHt;
/*N*/ 			xub_StrLen *pHtEnd, *pHintEnd;
/*N*/ 			for( USHORT n = 0, nEnd = Count(); n < nEnd; ++n )
/*N*/ 				if( RES_TXTATR_REFMARK == (pHt = GetHt( n ))->Which() &&
/*N*/ 					pHint->GetAttr() == pHt->GetAttr() &&
/*N*/ 					0 != ( pHtEnd = pHt->GetEnd() ) &&
/*N*/ 					0 != ( pHintEnd = pHint->GetEnd() ) )
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwComparePosition eCmp = ::ComparePosition(
/*N*/ 				}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_TXTATR_TOXMARK:
/*N*/ 		((SwTxtTOXMark*)pHint)->ChgTxtNode( &rNode );
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_TXTATR_CJK_RUBY:
/*N*/ 		{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			((SwTxtRuby*)pHint)->ChgTxtNode( &rNode );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/
/*N*/ 	if( SETATTR_DONTEXPAND & nMode )
/*N*/ 		pHint->SetDontExpand( TRUE );
/*N*/
/*N*/ 	// SwTxtAttrs ohne Ende werden sonderbehandelt:
/*N*/ 	// Sie werden natuerlich in das Array insertet, aber sie werden nicht
/*N*/ 	// in die pPrev/Next/On/Off-Verkettung aufgenommen.
/*N*/ 	// Der Formatierer erkennt diese TxtHints an dem CH_TXTATR_.. im Text !
/*N*/ 	xub_StrLen nHtStart = *pHint->GetStart();
/*N*/ 	if( !pHtEnd )
/*N*/ 	{
/*N*/ 		SwpHintsArr::Insert( pHint );
/*N*/ 		CalcFlags();
/*N*/ #ifdef DBG_UTIL
/*N*/         if( !rNode.GetDoc()->IsInReading() )
/*N*/             CHECK;
/*N*/ #endif
/*N*/ 		// ... und die Abhaengigen benachrichtigen
/*N*/ 		if ( rNode.GetDepends() )
/*N*/ 		{
/*N*/ 			SwUpdateAttr aHint( nHtStart, nHtStart, nWhich );
/*N*/ 			rNode.Modify( 0, &aHint );
/*N*/ 		}
/*N*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	// ----------------------------------------------------------------
/*N*/ 	// Ab hier gibt es nur noch pHint mit einem EndIdx !!!
/*N*/
/*N*/     SwTxtAttr *pMerge = pHint; // For a smarter Merge-function
/*N*/
/*N*/     BOOL bResort = FALSE;
/*N*/
/*N*/ 	if( *pHtEnd < nHtStart )
/*N*/ 	{
/*?*/ 		ASSERT( *pHtEnd >= nHtStart,
/*?*/ 					"+SwpHints::Insert: invalid hint, end < start" );
/*?*/
/*?*/ 		// Wir drehen den Quatsch einfach um:
/*?*/ 		*pHint->GetStart() = *pHtEnd;
/*?*/ 		*pHtEnd = nHtStart;
/*?*/ 		nHtStart = *pHint->GetStart();
/*N*/ 	}
/*N*/
/*N*/ 	// AMA: Damit wir endlich mit ueberlappenden Hints fertig werden ...
/*N*/ 	// 		das neue Verfahren:
/*N*/
/*N*/ 	if( !(SETATTR_NOHINTADJUST & nMode) && !pHint->IsOverlapAllowedAttr() )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pParaItem;
/*N*/ 		xub_StrLen nMaxEnd = *pHtEnd;
/*N*/ 		xub_StrLen nHtEnd = *pHtEnd;
/*N*/ 		BOOL bParaAttr = rNode.GetpSwAttrSet() &&
/*N*/ 			( SFX_ITEM_SET == rNode.GetpSwAttrSet()->GetItemState( nWhich,
/*N*/ 				FALSE, &pParaItem ) ) && ( pParaItem == &pHint->GetAttr() );
/*N*/ 		BOOL bReplace = !( SETATTR_DONTREPLACE & nMode );
/*N*/ 		::binfilter::SwpHtStart_SAR *pTmpHints = 0;
/*N*/
/*N*/ 		USHORT i;
/*N*/ 		// Wir wollen zwar von nHtStart bis nMaxEnd, muessen aber ggf.
/*N*/ 		// stueckeln (Attribute duerfen keine Zeichenvorlagen ueberlappen).
/*N*/ 		// Das erste Stueck wird also von nHtStart bis zum ersten Start/Ende
/*N*/ 		// einer Zeichenvorlage gehen usw. bis nHtEnd = nMaxEnd erreicht ist.
/*N*/ 		do {
/*N*/ 			BOOL bINet = nWhich == RES_TXTATR_INETFMT;
/*N*/ 			BOOL bForgetAttr = bParaAttr;
/*N*/ 			// Muessen wir uns aufspalten?
/*N*/ 			if ( !bINet && !pHint->IsDontMergeAttr() )
/*N*/ 			{
/*N*/ 				// Ab der zweiten Runde muessen wir zunaechst einen neuen
/*N*/ 				// Hint erzeugen.
/*N*/ 				if ( nHtEnd != nMaxEnd )
/*N*/ 				{
/*?*/                     pMerge = NULL; // No way to use the smarter Merge()
/*?*/                     pHint = rNode.MakeTxtAttr( bParaAttr ? *pParaItem :
/*?*/ 															pHint->GetAttr(),
/*?*/ 												nHtStart, nMaxEnd );
/*?*/ 					nHtEnd = *pHint->GetEnd();
/*N*/ 				}
/*N*/
/*N*/ 				for ( i = 0; i < Count(); i++)
/*N*/ 				{
/*N*/ 					SwTxtAttr *pOther = GetHt(i);
/*N*/ 					// Wir suchen nach Zeichenvorlagen, die uns schneiden
/*N*/ 					// oder in uns liegen
/*N*/ 					BOOL bOtherFmt = pOther->IsCharFmtAttr();
/*N*/ 					if( bOtherFmt ||
/*N*/ 						( RES_TXTATR_CHARFMT == nWhich && pOther->GetEnd() ) )
/*N*/ 					{
/*N*/ 						if( bForgetAttr && bOtherFmt &&
/*N*/ 							*pOther->GetStart() <= nHtStart &&
/*N*/ 							*pOther->GetEnd() >= nHtStart )
/*N*/ 							bForgetAttr = FALSE;
                        /* Die Flags bCheckInclude und bOtherFmt sollen die
                         * Anzahl der Aufrufe von lcl_Include minimieren, da
                         * dieses wg. IsVisitedURL() teuer ist. */
/*N*/ 						BOOL bCheckInclude = FALSE;
/*N*/ 						if( *pOther->GetStart() > nHtStart
/*N*/ 							&& *pOther->GetStart() < nHtEnd
/*N*/ 							&& ( bReplace || *pOther->GetEnd() > nHtEnd ) )
/*N*/ 						{
/*?*/ 							if( !bOtherFmt )
/*?*/ 							{
                                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 								bOtherFmt = !lcl_Included( pOther->Which(),
/*?*/ 							}
/*?*/ 							if( bOtherFmt )
/*?*/ 								nHtEnd = *pOther->GetStart();
/*N*/ 						}
/*N*/ 						if( *pOther->GetEnd() > nHtStart
/*N*/ 							&& *pOther->GetEnd() < nHtEnd
/*N*/ 							&& ( bReplace || *pOther->GetStart() < nHtStart ) )
/*N*/ 						{
/*?*/ 							if( bOtherFmt || ( !bCheckInclude &&
/*?*/ 									!lcl_Included( pOther->Which(), pHint ) ) )
/*?*/ 								nHtEnd = *pOther->GetEnd();
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				*pHint->GetEnd() = nHtEnd;
/*N*/ 			}
/*N*/
/*N*/ 			i = 0;
/*N*/ 			while(i < Count())
/*N*/ 			{
/*N*/ 				SwTxtAttr *pOther = GetHt(i);
/*N*/ 				const USHORT nOtherWhich = pOther->Which();
/*N*/ 				BOOL bCheckInclude = pHint->IsCharFmtAttr() &&
/*N*/ 									 pOther->IsCharFmtAttr() &&
/*N*/ 									 nWhich != nOtherWhich;
/*N*/
/*N*/ 				BOOL bOtherCharFmt = RES_TXTATR_CHARFMT == nOtherWhich;
/*N*/ 				if( nOtherWhich == nWhich || bCheckInclude )
/*N*/ 				{
/*N*/ 					if(0 == pOther->GetEnd())
/*N*/ 					{
/*?*/ 						if( *pOther->GetStart() == nHtStart &&
/*?*/ 							nOtherWhich == nWhich &&
/*?*/ 							pOther->GetAttr() == pHint->GetAttr() )
/*?*/ 						{
/*?*/ 							// Gibts schon, alten raus.
/*?*/ 							rNode.DestroyAttr( Cut(i) );
/*?*/ 						}
/*?*/ 						else
/*?*/ 							++i;
/*?*/ 						continue;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						// Attribute mit Anfang und Ende.
/*N*/ 						const Range aHintRg( nHtStart, nHtEnd );
/*N*/ 						const Range aOtherRg( *pOther->GetStart(),
/*N*/ 												*pOther->GetEnd() );
/*N*/
/*N*/ 						if( aOtherRg.IsInside( aHintRg.Min() ) ||
/*N*/ 							aHintRg.IsInside( aOtherRg.Min() ) )
/*N*/ 						{
/*N*/ 							// aBig umspannt beide Ranges
/*N*/ 							const Range aBig(
/*N*/ 									Min( aHintRg.Min(), aOtherRg.Min()),
/*N*/ 									Max( aHintRg.Max(), aOtherRg.Max()));
/*N*/
/*N*/ 							// Gleiches Attribut
/*N*/ 							// oder Zeichenvorlage:
/*N*/ 							//		Bereiche duerfen nicht ueberlappen.
/*N*/
/*N*/ 							// Zuerst wurde geprueft, ob sich der neue mit dem
/*N*/ 							// alten ueberschneidet, danach gibt es nur noch
/*N*/ 							// drei Faelle zu beachten:
/*N*/ 							// 1) der neue umschliesst den alten
/*N*/ 							// 2) der neue ueberlappt den alten hinten
/*N*/ 							// 3) der neue ueberlappt den alten vorne
/*N*/
/*N*/ 							BOOL bNoINet = RES_TXTATR_INETFMT != nOtherWhich ||
/*N*/ 										   nWhich == nOtherWhich;
/*N*/
/*N*/ 							// 1) der neue umschliesst den alten
/*N*/ 							if( aBig == aHintRg )
/*N*/ 							{
/*N*/ 								BOOL bTmpReplace = bReplace ||
/*N*/ 									( aHintRg == aOtherRg &&
/*N*/ 									  nWhich == nOtherWhich &&
/*N*/ 									  !pHint->IsCharFmtAttr() );
/*N*/ 								if( bNoINet && bTmpReplace &&
/*N*/ 									( !bOtherCharFmt || nWhich == nOtherWhich ) )
/*N*/ 								{
/*N*/ 									if( !bCheckInclude ||
/*N*/ 										lcl_Included( nOtherWhich, pHint ) )
/*N*/ 									{
/*N*/ 										rNode.DestroyAttr( Cut(i) );
/*N*/ 									}
/*N*/ 									else
/*N*/ 										++i;
/*N*/ 									continue;
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else if( !bReplace && aBig == aOtherRg )
/*?*/ 								bForgetAttr = FALSE;
/*N*/ 							// 2) der neue ueberlappt hinten
/*N*/ 							else if( aBig.Max() == aHintRg.Max() )
/*N*/ 							{
/*N*/ 								if( bNoINet &&
/*N*/ 									( !bOtherCharFmt || RES_TXTATR_CHARFMT != nWhich ) &&
/*N*/ 									( bReplace || aHintRg.Max() != aOtherRg.Max() ) )
/*N*/ 								{
/*N*/ 									if( ( bCheckInclude &&
/*N*/ 										  lcl_Included( nOtherWhich, pHint ) ) ||
/*N*/ 										  ( !bCheckInclude && !bOtherCharFmt ) )
/*N*/ 									{
/*N*/ 										if( nMaxEnd == nHtStart )
/*N*/ 											bForgetAttr = FALSE;
/*N*/ 										*pOther->GetEnd() = nHtStart;
/*N*/                                         bResort = TRUE;
/*N*/ 										// ChainDelEnd( pOther );
/*N*/ 										// ChainEnds( pOther );
/*N*/ 									}
/*N*/ 									else if( bINet &&
/*N*/ 											 aHintRg.Max() != aOtherRg.Max() &&
/*N*/ 											 aHintRg.Min() < aOtherRg.Max() )
/*N*/ 									{
/*N*/ 									// Wenn ein INetFmt eingefuegt wird, muss
/*N*/ 									// sich ein anderes Attribut ggf. aufspal-
/*N*/ 									// ten. Es wird beiseite gestellt und in
/*N*/ 									// einem spaeteren Durchgang eingefuegt.
/*N*/ 									// Beim Einfuegen spaltet es sich selbst.
/*?*/ 										if( !pTmpHints )
/*?*/ 											pTmpHints = new ::binfilter::SwpHtStart_SAR();
/*?*/ 										pTmpHints->C40_INSERT( SwTxtAttr, pOther,
/*?*/ 											pTmpHints->Count() );
/*?*/ 										Cut( i );
/*?*/ 										continue;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							// 3) der neue ueberlappt vorne
/*N*/ 							else if( aBig.Min() == aHintRg.Min() )
/*N*/ 							{
/*N*/ 								if( bNoINet &&
/*N*/ 									( RES_TXTATR_CHARFMT != nOtherWhich ||
/*N*/ 									  RES_TXTATR_CHARFMT != nWhich ) &&
/*N*/ 									( bReplace || aHintRg.Min() != aOtherRg.Min() ) )
/*N*/ 								{
/*N*/ 									if( ( bCheckInclude &&
/*N*/ 										  lcl_Included( nOtherWhich, pHint ) ) ||
/*N*/ 										( !bCheckInclude && !bOtherCharFmt ) )
/*N*/ 									{
/*N*/
/*N*/ 										*pOther->GetStart() = nHtEnd;
/*N*/
/*N*/ 										// ChainDelStart( pOther );
/*N*/ 										// ChainStarts( pOther );
/*N*/
/*N*/ 										const BOOL bOk = Resort(i);
/*N*/ 										if( bOk )
/*N*/ 											continue;
/*N*/ 									}
/*N*/ 									else if( bINet &&
/*N*/ 											 aHintRg.Min() != aOtherRg.Min() &&
/*N*/ 											 aHintRg.Max() > aOtherRg.Min() )
/*N*/ 									{
/*?*/ 									// Wenn ein INetFmt eingefuegt wird, muss
/*?*/ 									// sich ein anderes Attribut ggf. aufspal-
/*?*/ 									// ten. Es wird beiseite gestellt und in
/*?*/ 									// einem spaeteren Durchgang eingefuegt.
/*?*/ 									// Beim Einfuegen spaltet es sich selbst.
/*?*/ 										if( !pTmpHints )
/*?*/ 											pTmpHints = new ::binfilter::SwpHtStart_SAR();
/*?*/ 										pTmpHints->C40_INSERT( SwTxtAttr, pOther,
/*?*/ 											pTmpHints->Count() );
/*?*/ 										Cut( i );
/*?*/ 										continue;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else
/*N*/ 								bForgetAttr = FALSE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				++i;
/*N*/ 			}
/*N*/
/*N*/ 			ClearDummies( rNode );
/*N*/ 			// Nur wenn wir nicht sowieso schon durch die Absatzattribute
/*N*/ 			// gueltig sind, werden wir eingefuegt ...
/*N*/ 			if( bForgetAttr )
/*N*/             {
/*?*/ 				rNode.DestroyAttr( pHint );
/*?*/                 pMerge = NULL; // No smart Merge()
/*N*/             }
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwpHintsArr::Insert( pHint );
/*N*/                 bResort = FALSE;
/*N*/ 			}
/*N*/ 			// InsertChain( pHint );
/*N*/
/*N*/ 			// ... und die Abhaengigen benachrichtigen
/*N*/ 			if ( rNode.GetDepends() )
/*N*/ 			{
/*N*/ 				SwUpdateAttr aHint( nHtStart, nHtEnd, nWhich );
/*N*/ 				rNode.Modify( 0, &aHint );
/*N*/ 			}
/*N*/ 			// Falls es noch 'ne Runde gibt:
/*N*/ 			nHtStart = nHtEnd;
/*N*/ 			if( nMaxEnd > nHtEnd )
/*N*/ 				continue;
/*N*/ 			if( !pTmpHints )
/*N*/ 				break;
/*?*/             pMerge = NULL; // No smart Merge()
/*?*/ 			pHint = pTmpHints->GetObject(0);
/*?*/ 			nWhich = pHint->Which();
/*?*/ 			nHtStart = *pHint->GetStart();
/*?*/ 			nHtEnd = *pHint->GetEnd();
/*?*/ 			nMaxEnd = nHtEnd;
/*?*/ 			bParaAttr = FALSE;
/*?*/ 			pTmpHints->Remove(0);
/*?*/ 			if( !pTmpHints->Count() )
/*?*/ 			{
/*?*/ 				delete pTmpHints;
/*?*/ 				pTmpHints = NULL;
/*?*/ 			}
/*N*/ 		} while ( TRUE );
/*N*/
/*N*/         if( bResort )
/*?*/             SwpHintsArr::Resort();
/*N*/
/*N*/ 		// Jetzt wollen wir mal gucken, ob wir das SwpHintsArray nicht
/*N*/ 		// etwas vereinfachen koennen ...
/*N*/         Merge( rNode, pMerge );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwpHintsArr::Insert( pHint );
/*N*/ 		// InsertChain( pHint );
/*N*/
/*N*/ 		// ... und die Abhaengigen benachrichtigen
/*N*/ 		if ( rNode.GetDepends() )
/*N*/ 		{
/*N*/ 			SwUpdateAttr aHint( nHtStart,
/*N*/ 				nHtStart == *pHtEnd ? *pHtEnd + 1 : *pHtEnd, nWhich );
/*N*/ 			rNode.Modify( 0, &aHint );
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef DBG_UTIL
/*N*/     if( !rNode.GetDoc()->IsInReading() )
/*N*/         CHECK;
/*N*/ #endif
/*N*/ }

/*************************************************************************
 *						SwpHints::DeleteAtPos()
 *************************************************************************/

/*N*/ void SwpHints::DeleteAtPos( const USHORT nPos )
/*N*/ {
/*N*/ 	SwTxtAttr *pHint = GetHt(nPos);
/*N*/ 	// ChainDelete( pHint );
/*N*/ 	SwpHintsArr::DeleteAtPos( nPos );
/*N*/
/*N*/ 	if( RES_TXTATR_FIELD == pHint->Which() )
/*N*/ 	{
/*N*/ 		SwFieldType* pFldTyp = ((SwTxtFld*)pHint)->GetFld().GetFld()->GetTyp();
/*N*/ 		if( RES_DDEFLD == pFldTyp->Which() )
/*N*/ 		{
/*?*/ 			const SwTxtNode* pNd = ((SwTxtFld*)pHint)->GetpTxtNode();
/*?*/ 			if( pNd && pNd->GetNodes().IsDocNodes() )
/*?*/ 				((SwDDEFieldType*)pFldTyp)->DecRefCnt();
/*?*/ 			((SwTxtFld*)pHint)->ChgTxtNode( 0 );
/*N*/ 		}
/*N*/ 		else if( !bVis && RES_HIDDENPARAFLD == pFldTyp->Which() )
/*N*/ 			bCalcVis = TRUE;
/*N*/ 	}
/*N*/ 	CalcFlags();
/*N*/ 	CHECK;
/*N*/ }

// Ist der Hint schon bekannt, dann suche die Position und loesche ihn.
// Ist er nicht im Array, so gibt es ein ASSERT !!

/*N*/ void SwpHints::Delete( SwTxtAttr* pTxtHt )
/*N*/ {
/*N*/ 	// Attr 2.0: SwpHintsArr::Delete( pTxtHt );
/*N*/ 	const USHORT nPos = GetStartOf( pTxtHt );
/*N*/ 	ASSERT( USHRT_MAX != nPos, "Attribut nicht im Attribut-Array!" );
/*N*/ 	if( USHRT_MAX != nPos )
/*N*/ 		DeleteAtPos( nPos );
/*N*/ }



/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/ USHORT SwTxtNode::GetLang( const xub_StrLen nBegin, const xub_StrLen nLen,
/*N*/                            USHORT nScript ) const
/*N*/ #else
/*N*/ USHORT SwTxtNode::GetLang( const xub_StrLen nBegin, const xub_StrLen nLen) const
/*N*/ #endif
/*N*/ {
/*N*/ 	USHORT nWhichId = RES_CHRATR_LANGUAGE;
/*N*/ 	USHORT nRet = LANGUAGE_DONTKNOW;
/*N*/ 	if( pSwpHints )
/*N*/ 	{
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/         if ( ! nScript )
/*N*/             nScript = pBreakIt->GetRealScriptOfText( aText, nBegin );
/*N*/
/*N*/         nWhichId = GetWhichOfScript( nWhichId, nScript );
/*N*/ #else
/*N*/ 		nWhichId = GetWhichOfScript( nWhichId,
/*N*/ 						pBreakIt->GetRealScriptOfText( aText, nBegin ));
/*N*/ #endif
/*N*/
/*N*/ 		xub_StrLen nEnd = nBegin + nLen;
/*N*/ 		for( USHORT i = 0, nSize = pSwpHints->Count(); i < nSize; ++i )
/*N*/ 		{
/*N*/ 			// ist der Attribut-Anfang schon groesser als der Idx ?
/*N*/ 			const SwTxtAttr *pHt = pSwpHints->operator[](i);
/*N*/ 			xub_StrLen nAttrStart = *pHt->GetStart();
/*N*/ 			if( nEnd < nAttrStart )
/*N*/ 				break;
/*N*/
/*N*/ 			const USHORT nWhich = pHt->Which();
/*N*/
/*N*/ 			if( ( pHt->IsCharFmtAttr() && lcl_Included( nWhichId, pHt ) )
/*N*/ 				|| nWhichId == nWhich )
/*N*/ 			{
/*N*/ 				const xub_StrLen *pEndIdx = pHt->GetEnd();
/*N*/ 				// Ueberlappt das Attribut den Bereich?
/*N*/
/*N*/ 				if( pEndIdx &&
/*N*/ 					nLen ? ( nAttrStart < nEnd && nBegin < *pEndIdx )
/*N*/ 					 	: (( nAttrStart < nBegin &&
/*N*/ 								( pHt->DontExpand() ? nBegin < *pEndIdx
/*N*/ 													: nBegin <= *pEndIdx )) ||
/*N*/ 							( nBegin == nAttrStart &&
/*N*/ 								( nAttrStart == *pEndIdx || !nBegin ))) )
/*N*/ 				{
/*N*/ 					const SfxPoolItem* pItem;
/*N*/ 					if( RES_TXTATR_CHARFMT == nWhich )
/*N*/ 						pItem = &pHt->GetCharFmt().GetCharFmt()->GetAttr( nWhichId );
/*N*/ 					else if( RES_TXTATR_INETFMT == nWhich )
/*?*/ 						pItem = &((SwTxtINetFmt*)pHt)->GetCharFmt()->GetAttr( nWhichId );
/*N*/ 					else
/*N*/ 						pItem = &pHt->GetAttr();
/*N*/
/*N*/ 					USHORT nLng = ((SvxLanguageItem*)pItem)->GetLanguage();
/*N*/
/*N*/ 					// Umfasst das Attribut den Bereich komplett?
/*N*/ 					if( nAttrStart <= nBegin && nEnd <= *pEndIdx )
/*N*/ 						nRet = nLng;
/*N*/ 					else if( LANGUAGE_DONTKNOW == nRet )
/*N*/ 						nRet = nLng; // partielle Ueberlappung, der 1. gewinnt
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( LANGUAGE_DONTKNOW == nRet )
/*N*/ 	{
/*N*/ 		if( !pSwpHints )
/*N*/ 			nWhichId = GetWhichOfScript( RES_CHRATR_LANGUAGE,
/*N*/                         pBreakIt->GetRealScriptOfText( aText, nBegin ));
/*N*/
/*N*/ 		nRet = ((SvxLanguageItem&)GetSwAttrSet().Get( nWhichId )).GetLanguage();
/*N*/ 		if( LANGUAGE_DONTKNOW == nRet )
/*N*/ 			nRet = GetAppLanguage();
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }


/*N*/ sal_Unicode GetCharOfTxtAttr( const SwTxtAttr& rAttr )
/*N*/ {
/*N*/ 	sal_Unicode cRet = CH_TXTATR_BREAKWORD;
/*N*/ 	switch ( rAttr.Which() )
/*N*/ 	{
/*N*/ 	case RES_TXTATR_REFMARK:
/*N*/ 	case RES_TXTATR_TOXMARK:
/*N*/
/*N*/ //	case RES_TXTATR_FIELD: 			??????
/*N*/ //	case RES_TXTATR_FLYCNT,								// 29
/*N*/
/*N*/ 	case RES_TXTATR_FTN:
/*N*/ 		cRet = CH_TXTATR_INWORD;
/*N*/ 		break;
/*N*/
/*N*/ 		// depends on the character ??
/*N*/ //	case RES_TXTATR_HARDBLANK:
/*N*/ //		cRet = CH_TXTATR_INWORD;
/*N*/ //		break;
/*N*/ 	}
/*N*/ 	return cRet;
/*N*/ }


}
