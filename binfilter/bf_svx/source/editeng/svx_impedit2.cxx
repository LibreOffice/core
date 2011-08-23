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

#include "eeitem.hxx"
#include "eeitemid.hxx"

#include <lspcitem.hxx>



#include "itemdata.hxx"

#include <tools/date.hxx>

#include <tools/time.hxx>




#include <vcl/window.hxx>

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <eerdll2.hxx>
#include <eerdll.hxx>
#include <txtrange.hxx>
#include <bf_svtools/colorcfg.hxx>
#include <bf_svtools/ctloptions.hxx>

#include <lrspitem.hxx>
#include <ulspitem.hxx>
#include <adjitem.hxx>
#include <scripttypeitem.hxx>
#include <frmdiritem.hxx>




#include <vcl/cmdevt.h>

#ifndef SVX_LIGHT
#endif



#include <com/sun/star/i18n/ScriptType.hpp>

#include <com/sun/star/text/CharacterCompressionType.hpp>



#include <unicode/ubidi.h>
namespace binfilter {

#if defined(MACOSX) && ( __GNUC__ < 3 )
// moved from editundo.hxx
const EPaM& EditUndoRemoveChars::GetEPaM() { return aEPaM; }
String& EditUndoRemoveChars::GetStr() { return aText; }
#endif

using namespace ::com::sun::star;

/*N*/ USHORT lcl_CalcExtraSpace( ParaPortion* pPortion, const SvxLineSpacingItem& rLSItem )
/*N*/ {
/*N*/ 	USHORT nExtra = 0;
    /* if ( ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            && ( rLSItem.GetPropLineSpace() != 100 ) )
    {
        // ULONG nH = pPortion->GetNode()->GetCharAttribs().GetDefFont().GetSize().Height();
        ULONG nH = pPortion->GetLines().GetObject( 0 )->GetHeight();
        long n = nH * rLSItem.GetPropLineSpace();
        n /= 100;
        n -= nH;	// nur den Abstand
        if ( n > 0 )
            nExtra = (USHORT)n;
    }
    else */
/*N*/ 	if ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
/*N*/ 	{
/*?*/ 		nExtra = rLSItem.GetInterLineSpace();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nExtra;
/*N*/ }

// ----------------------------------------------------------------------
//	class ImpEditEngine
//	----------------------------------------------------------------------

/*N*/ ImpEditEngine::ImpEditEngine( EditEngine* pEE, SfxItemPool* pItemPool ) :
/*N*/ 	aEditDoc( pItemPool ),
/*N*/ 	aPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
/*N*/ 	aMaxAutoPaperSize( 0x7FFFFFFF, 0x7FFFFFFF ),
/*N*/ 	aMinAutoPaperSize( 0x0, 0x0 ),
/*N*/ 	aGroupChars( RTL_CONSTASCII_USTRINGPARAM( "{}()[]" ) ),
/*N*/ 	aWordDelimiters( RTL_CONSTASCII_USTRINGPARAM( "  .,;:-'`'?!_=\"{}()[]\0xFF" ) )
/*N*/ {
/*N*/ 	pEditEngine 		= pEE;
/*N*/ 	pRefDev 			= NULL;
/*N*/ 	pVirtDev 			= NULL;
/*N*/ 	pEmptyItemSet 		= NULL;
/*N*/ 	pActiveView 		= NULL;
/*N*/ 	pTextObjectPool 	= NULL;
/*N*/ 	mpIMEInfos			= NULL;
/*N*/ 	pStylePool 			= NULL;
/*N*/ 	pUndoManager 		= NULL;
/*N*/ 	pUndoMarkSelection	= NULL;
/*N*/ 	pTextRanger			= NULL;
/*N*/     pColorConfig        = NULL;
/*N*/     pCTLOptions         = NULL;
/*N*/ 
/*N*/ 	nCurTextHeight 		= 0;
/*N*/     nBlockNotifications = 0;
/*N*/ 	nBigTextObjectStart	= 20;
/*N*/ 
/*N*/ 	nStretchX			= 100;
/*N*/ 	nStretchY			= 100;
/*N*/ 
/*N*/ 	bInSelection 		= FALSE;
/*N*/ 	bOwnerOfRefDev 		= FALSE;
/*N*/ 	bDowning 			= FALSE;
/*N*/ 	bIsInUndo 			= FALSE;
/*N*/ 	bIsFormatting 		= FALSE;
/*N*/ 	bFormatted			= FALSE;
/*N*/ 	bUpdate 			= TRUE;
/*N*/     bUseAutoColor       = TRUE;
/*N*/     bForceAutoColor     = FALSE;
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	bUndoEnabled 		= TRUE;
/*N*/ #else
/*N*/ 	bUndoEnabled 		= FALSE;
/*N*/ #endif
/*N*/ 	bCallParaInsertedOrDeleted = FALSE;
/*N*/ 
/*N*/ 	eDefLanguage		= LANGUAGE_DONTKNOW;
/*N*/ 	maBackgroundColor	= COL_AUTO;
/*N*/ 
/*N*/     nAsianCompressionMode = text::CharacterCompressionType::NONE;
/*N*/ 	bKernAsianPunctuation = FALSE;
/*N*/ 
/*N*/     eDefaultHorizontalTextDirection = EE_HTEXTDIR_DEFAULT;
/*N*/ 
/*N*/ 
/*N*/ 	aStatus.GetControlWord() =	EE_CNTRL_USECHARATTRIBS | EE_CNTRL_DOIDLEFORMAT |
/*N*/ 								EE_CNTRL_PASTESPECIAL | EE_CNTRL_UNDOATTRIBS |
/*N*/ 								EE_CNTRL_ALLOWBIGOBJS | EE_CNTRL_RTFSTYLESHEETS |
/*N*/ 								EE_CNTRL_FORMAT100;
/*N*/ 
/*N*/ 	aSelEngine.SetFunctionSet( &aSelFuncSet );
/*N*/ 
/*N*/ 	pRefDev 			= EE_DLL()->GetGlobalData()->GetStdRefDevice();
/*N*/ 
/*N*/ 	// Ab hier wird schon auf Daten zugegriffen!
/*N*/ 	SetRefDevice( pRefDev );
/*N*/ 	InitDoc( FALSE );
/*N*/ 
/*N*/ 	bCallParaInsertedOrDeleted = TRUE;
/*N*/ 
/*N*/ }

/*N*/ ImpEditEngine::~ImpEditEngine()
/*N*/ {
/*N*/ 	// das Zerstoeren von Vorlagen kann sonst unnoetiges Formatieren ausloesen,
/*N*/ 	// wenn eine Parent-Vorlage zerstoert wird.
/*N*/ 	// Und das nach dem Zerstoeren der Daten!
/*N*/ 	bDowning = TRUE;
/*N*/ 	SetUpdateMode( FALSE );
/*N*/ 
/*N*/ 	delete pVirtDev;
/*N*/ 	delete pEmptyItemSet;
/*N*/ 	delete pUndoManager;
/*N*/ 	delete pTextRanger;
/*N*/ 	delete mpIMEInfos;
/*N*/     delete pColorConfig;
/*N*/     delete pCTLOptions;
/*N*/ 	if ( bOwnerOfRefDev )
/*N*/ 		delete pRefDev;
/*N*/ }

/*N*/ void ImpEditEngine::SetRefDevice( OutputDevice* pRef )
/*N*/ {
/*N*/ 	if ( bOwnerOfRefDev )
/*N*/ 		delete pRefDev;
/*N*/ 
/*N*/ 	pRefDev = pRef;
/*N*/ 	bOwnerOfRefDev = FALSE;
/*N*/ 
/*N*/ 	if ( !pRef )
/*N*/ 		pRefDev = EE_DLL()->GetGlobalData()->GetStdRefDevice();
/*N*/ 
/*N*/ 	nOnePixelInRef = (USHORT)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();
/*N*/ 
/*N*/ 	if ( IsFormatted() )
/*N*/ 	{
/*N*/ 		FormatFullDoc();
/*N*/ 		UpdateViews( (EditView*) 0);
/*N*/ 	}
/*N*/ }

/*N*/ void ImpEditEngine::SetRefMapMode( const MapMode& rMapMode )
/*N*/ {
/*N*/ 	if ( GetRefDevice()->GetMapMode() == rMapMode )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Wenn RefDev == GlobalRefDev => eigenes anlegen!
/*N*/ 	if ( !bOwnerOfRefDev && ( pRefDev == EE_DLL()->GetGlobalData()->GetStdRefDevice() ) )
/*N*/ 	{
/*N*/ 		pRefDev = new VirtualDevice;
/*N*/ 		pRefDev->SetMapMode( MAP_TWIP );
/*N*/ 		SetRefDevice( pRefDev );
/*N*/ 		bOwnerOfRefDev = TRUE;
/*N*/ 	}
/*N*/ 	pRefDev->SetMapMode( rMapMode );
/*N*/ 	nOnePixelInRef = (USHORT)pRefDev->PixelToLogic( Size( 1, 0 ) ).Width();
/*N*/ 	if ( IsFormatted() )
/*N*/ 	{
/*N*/ 		FormatFullDoc();
/*N*/ 		UpdateViews( (EditView*) 0);
/*N*/ 	}
/*N*/ }

/*N*/ void ImpEditEngine::InitDoc( BOOL bKeepParaAttribs )
/*N*/ {
/*N*/ 	USHORT nParas = aEditDoc.Count();
/*N*/ 	for ( USHORT n = bKeepParaAttribs ? 1 : 0; n < nParas; n++ )
/*N*/ 	{
/*N*/ 		if ( aEditDoc[n]->GetStyleSheet() )
/*N*/ 			EndListening( *aEditDoc[n]->GetStyleSheet(), FALSE );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bKeepParaAttribs )
/*N*/ 		aEditDoc.RemoveText();
/*N*/ 	else
/*N*/ 		aEditDoc.Clear();
/*N*/ 
/*N*/ 	GetParaPortions().Reset();
/*N*/ 
/*N*/ 	ParaPortion* pIniPortion = new ParaPortion( aEditDoc[0] );
/*N*/ 	GetParaPortions().Insert( pIniPortion, 0 );
/*N*/ 
/*N*/ 	bFormatted = FALSE;
/*N*/ 
/*N*/ 	if ( IsCallParaInsertedOrDeleted() )
/*N*/ 	{
/*N*/ 		GetEditEnginePtr()->ParagraphDeleted( EE_PARA_ALL );
/*N*/ 		GetEditEnginePtr()->ParagraphInserted( 0 );
/*N*/ 	}
/*N*/ }


/*N*/ XubString ImpEditEngine::GetSelected( const EditSelection& rSel, const LineEnd eEnd  ) const
/*N*/ {
/*N*/ 	XubString aText;
/*N*/ 	if ( !rSel.HasRange() )
/*N*/ 		return aText;
/*N*/ 
/*N*/ 	String aSep = EditDoc::GetSepStr( eEnd );
/*N*/ 
/*N*/ 	EditSelection aSel( rSel );
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 
/*N*/ 	ContentNode* pStartNode = aSel.Min().GetNode();
/*N*/ 	ContentNode* pEndNode = aSel.Max().GetNode();
/*N*/ 	USHORT nStartNode = aEditDoc.GetPos( pStartNode );
/*N*/ 	USHORT nEndNode = aEditDoc.GetPos( pEndNode );
/*N*/ 
/*N*/ 	DBG_ASSERT( nStartNode <= nEndNode, "Selektion nicht sortiert ?" );
/*N*/ 
/*N*/ 	// ueber die Absaetze iterieren...
/*N*/ 	for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++	)
/*N*/ 	{
/*N*/ 		DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: GetSelected" );
/*N*/ 		ContentNode* pNode = aEditDoc.GetObject( nNode );
/*N*/ 
/*N*/ 		xub_StrLen nStartPos = 0;
/*N*/ 		xub_StrLen nEndPos = pNode->Len();
/*N*/ 		if ( nNode == nStartNode )
/*N*/ 			nStartPos = aSel.Min().GetIndex();
/*N*/ 		if ( nNode == nEndNode ) // kann auch == nStart sein!
/*N*/ 			nEndPos = aSel.Max().GetIndex();
/*N*/ 
/*N*/ 		aText += aEditDoc.GetParaAsString( pNode, nStartPos, nEndPos );
/*N*/ 		if ( nNode < nEndNode )
/*N*/ 			aText += aSep;
/*N*/ 	}
/*N*/ 	return aText;
/*N*/ }






/*N*/ EditPaM ImpEditEngine::Clear()
/*N*/ {
/*N*/ 	InitDoc( FALSE );
/*N*/ 
/*N*/ 	EditPaM aPaM = aEditDoc.GetStartPaM();
/*N*/ 	EditSelection aSel( aPaM );
/*N*/ 
/*N*/ 	nCurTextHeight = 0;
/*N*/ 
/*N*/ 	ResetUndoManager();
/*N*/ 
/*N*/ 	for ( USHORT nView = aEditViews.Count(); nView; )
/*N*/ 	{
/*?*/ 		EditView* pView = aEditViews[--nView];
/*?*/ 		DBG_CHKOBJ( pView, EditView, 0 );
/*?*/ 		pView->pImpEditView->SetEditSelection( aSel );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::RemoveText()
/*N*/ {
/*N*/ 	InitDoc( TRUE );
/*N*/ 
/*N*/ 	EditPaM aStartPaM = aEditDoc.GetStartPaM();
/*N*/ 	EditSelection aEmptySel( aStartPaM, aStartPaM );
/*N*/ 	for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
/*N*/ 	{
/*N*/ 		EditView* pView = aEditViews.GetObject(nView);
/*N*/ 		DBG_CHKOBJ( pView, EditView, 0 );
/*N*/ 		pView->pImpEditView->SetEditSelection( aEmptySel );
/*N*/ 	}
/*N*/ 	ResetUndoManager();
/*N*/ 	return aEditDoc.GetStartPaM();
/*N*/ }


/*N*/ void ImpEditEngine::SetText( const XubString& rText )
/*N*/ {
/*N*/ 	// RemoveText loescht die Undo-Liste!
/*N*/ 	EditPaM aStartPaM = RemoveText();
/*N*/ 	BOOL bUndoCurrentlyEnabled = IsUndoEnabled();
/*N*/ 	// Der von Hand reingesteckte Text kann nicht vom Anwender rueckgaengig gemacht werden.
/*N*/ 	EnableUndo( FALSE );
/*N*/ 
/*N*/ 	EditSelection aEmptySel( aStartPaM, aStartPaM );
/*N*/ 	EditPaM aPaM = aStartPaM;
/*N*/ 	if ( rText.Len() )
/*N*/ 		aPaM = ImpInsertText( aEmptySel, rText );
/*N*/ 
/*N*/ 	for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
/*N*/ 	{
/*N*/ 		EditView* pView = aEditViews[nView];
/*N*/ 		DBG_CHKOBJ( pView, EditView, 0 );
/*N*/ 		pView->pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
/*N*/ 		// Wenn kein Text, dann auch Kein Format&Update
/*N*/ 		// => Der Text bleibt stehen.
/*N*/ 		if ( !rText.Len() && GetUpdateMode() )
/*N*/ 		{
/*?*/ 			Rectangle aTmpRec( pView->GetOutputArea().TopLeft(),
/*?*/ 								Size( aPaperSize.Width(), nCurTextHeight ) );
/*?*/ 			aTmpRec.Intersection( pView->GetOutputArea() );
/*?*/ 			pView->GetWindow()->Invalidate( aTmpRec );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !rText.Len() )	// sonst muss spaeter noch invalidiert werden, !bFormatted reicht.
/*N*/ 		nCurTextHeight = 0;
/*N*/ 	EnableUndo( bUndoCurrentlyEnabled );
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Undo nach SetText?" );
/*N*/ #endif
/*N*/ }


/*N*/ const SfxItemSet& ImpEditEngine::GetEmptyItemSet()
/*N*/ {
/*N*/ 	if ( !pEmptyItemSet )
/*N*/ 	{
/*N*/ 		pEmptyItemSet = new SfxItemSet( aEditDoc.GetItemPool(), EE_ITEMS_START, EE_ITEMS_END );
/*N*/ 		for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
/*N*/ 		{
/*N*/ 			pEmptyItemSet->ClearItem( nWhich );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return *pEmptyItemSet;
/*N*/ }

//	----------------------------------------------------------------------
//	MISC
//	----------------------------------------------------------------------
/*N*/ void ImpEditEngine::CursorMoved( ContentNode* pPrevNode )
/*N*/ {
/*N*/ 	// Leere Attribute loeschen, aber nur, wenn Absatz nicht leer!
/*N*/ 	if ( pPrevNode->GetCharAttribs().HasEmptyAttribs() && pPrevNode->Len() )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pPrevNode->GetCharAttribs().DeleteEmptyAttribs( aEditDoc.GetItemPool() );
/*N*/ }

/*N*/ void ImpEditEngine::TextModified()
/*N*/ {
/*N*/ 	bFormatted = FALSE;
/*N*/ 
/*N*/     if ( GetNotifyHdl().IsSet() )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EENotify aNotify( EE_NOTIFY_TEXTMODIFIED );
/*N*/     }
/*N*/ }


/*N*/ void ImpEditEngine::ParaAttribsChanged( ContentNode* pNode )
/*N*/ {
/*N*/ 	DBG_ASSERT( pNode, "ParaAttribsChanged: Welcher?" );
/*N*/ 
/*N*/ 	aEditDoc.SetModified( TRUE );
/*N*/ 	bFormatted = FALSE;
/*N*/ 
/*N*/ 	ParaPortion* pPortion = FindParaPortion( pNode );
/*N*/ 	DBG_ASSERT( pPortion, "ParaAttribsChanged: Portion?" );
/*N*/ 	pPortion->MarkSelectionInvalid( 0, pNode->Len() );
/*N*/ 
/*N*/ 	USHORT nPara = aEditDoc.GetPos( pNode );
/*N*/ 	pEditEngine->ParaAttribsChanged( nPara );
/*N*/ 
/*N*/ 	ParaPortion* pNextPortion = GetParaPortions().SaveGetObject( nPara+1 );
/*N*/ 	// => wird sowieso noch formatiert, wenn Invalid.
/*N*/ 	if ( pNextPortion && !pNextPortion->IsInvalid() )
/*?*/ 		CalcHeight( pNextPortion );
/*N*/ }

//	----------------------------------------------------------------------
//	Cursorbewegungen
//	----------------------------------------------------------------------


















/*N*/ EditPaM ImpEditEngine::WordRight( const EditPaM& rPaM, sal_Int16 nWordType )
/*N*/ {
/*N*/ 	xub_StrLen nMax = rPaM.GetNode()->Len();
/*N*/ 	EditPaM aNewPaM( rPaM );
/*N*/ 	if ( aNewPaM.GetIndex() < nMax )
/*N*/ 	{
/*N*/ 		uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
/*N*/ 		i18n::Boundary aBoundary = xBI->nextWord( *aNewPaM.GetNode(), aNewPaM.GetIndex(), GetLocale( aNewPaM ), nWordType );
/*N*/ 		aNewPaM.SetIndex( (USHORT)aBoundary.startPos );
/*N*/ 	}
/*N*/ 	// not 'else', maybe the index reached nMax now...
/*N*/ 	if ( aNewPaM.GetIndex() >= nMax )
/*N*/ 	{
/*?*/ 		// Naechster Absatz...
/*?*/ 		USHORT nCurPara = aEditDoc.GetPos( aNewPaM.GetNode() );
/*?*/ 		ContentNode* pNextNode = aEditDoc.SaveGetObject( ++nCurPara );
/*?*/ 		if ( pNextNode )
/*?*/ 		{
/*?*/ 			aNewPaM.SetNode( pNextNode );
/*?*/ 			aNewPaM.SetIndex( 0 );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return aNewPaM;
/*N*/ }



/*N*/ EditSelection ImpEditEngine::SelectWord( const EditSelection& rCurSel, sal_Int16 nWordType, BOOL bAcceptStartOfWord )
/*N*/ {
/*N*/ 	EditSelection aNewSel( rCurSel );
/*N*/ 	EditPaM aPaM( rCurSel.Max() );
/*N*/ 	uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
/*N*/ 	sal_Int16 nType = xBI->getWordType( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ) );
/*N*/ 	if ( nType == i18n::WordType::ANY_WORD )
/*N*/ 	{
/*N*/ 		i18n::Boundary aBoundary = xBI->getWordBoundary( *aPaM.GetNode(), aPaM.GetIndex(), GetLocale( aPaM ), nWordType, sal_True );
/*N*/ 		// don't select when curser at end of word
/*N*/ 		if ( ( aBoundary.endPos > aPaM.GetIndex() ) &&
/*N*/ 			 ( bAcceptStartOfWord || ( aBoundary.startPos < aPaM.GetIndex() ) ) )
/*N*/ 		{
/*N*/ 			aNewSel.Min().SetIndex( (USHORT)aBoundary.startPos );
/*N*/ 			aNewSel.Max().SetIndex( (USHORT)aBoundary.endPos );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return aNewSel;
/*N*/ }

/*N*/ void ImpEditEngine::InitScriptTypes( USHORT nPara )
/*N*/ {
/*N*/ 	ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 	ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 	rTypes.Remove( 0, rTypes.Count() );
/*N*/ 
/*N*/ 
/*N*/ //	pParaPortion->aExtraCharInfos.Remove( 0, pParaPortion->aExtraCharInfos.Count() );
/*N*/ 
/*N*/ 	ContentNode* pNode = pParaPortion->GetNode();
/*N*/ 	if ( pNode->Len() )
/*N*/ 	{
/*N*/ 		uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
/*N*/ 
/*N*/ 		String aText( *pNode );
/*N*/ 
/*N*/ 		// To handle fields put the character from the field in the string,
/*N*/ 		// because endOfScript( ... ) will skip the CH_FEATURE, because this is WEAK
/*N*/ 		EditCharAttrib* pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, 0 );
/*N*/ 		while ( pField )
/*N*/ 		{
/*N*/ 			::rtl::OUString aFldText( ((EditCharAttribField*)pField)->GetFieldValue() );
/*N*/ 			if ( aFldText.getLength() )
/*N*/ 			{
/*N*/ 				aText.SetChar( pField->GetStart(), aFldText.getStr()[0] );
/*N*/ 				short nFldScriptType = xBI->getScriptType( aFldText, 0 );
/*N*/ 
/*N*/ 				for ( USHORT nCharInField = 1; nCharInField < aFldText.getLength(); nCharInField++ )
/*N*/ 				{
/*N*/ 					short nTmpType = xBI->getScriptType( aFldText, nCharInField );
/*N*/ 
/*N*/ 					// First char from field wins...
/*N*/ 					if ( nFldScriptType == i18n::ScriptType::WEAK )
/*N*/ 					{
/*?*/ 						nFldScriptType = nTmpType;
/*?*/ 						aText.SetChar( pField->GetStart(), aFldText.getStr()[nCharInField] );
/*N*/ 					}
/*N*/ 
/*N*/ 					// ...  but if the first one is LATIN, and there are CJK or CTL chars too,
/*N*/ 					// we prefer that ScripType because we need an other font.
/*N*/ 					if ( ( nTmpType == i18n::ScriptType::ASIAN ) || ( nTmpType == i18n::ScriptType::COMPLEX ) )
/*N*/ 					{
/*?*/ 						aText.SetChar( pField->GetStart(), aFldText.getStr()[nCharInField] );
/*?*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pField = pNode->GetCharAttribs().FindNextAttrib( EE_FEATURE_FIELD, pField->GetEnd() );
/*N*/ 		}
/*N*/ 
/*N*/ 		::rtl::OUString aOUText( aText );
/*N*/ 		USHORT nTextLen = (USHORT)aOUText.getLength();
/*N*/ 
/*N*/ 		long nPos = 0;
/*N*/ 		short nScriptType = xBI->getScriptType( aOUText, nPos );
/*N*/ 		rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
/*N*/ 		nPos = xBI->endOfScript( aOUText, nPos, nScriptType );
/*N*/ 		while ( ( nPos != (-1) ) && ( nPos < nTextLen ) )
/*N*/ 		{
/*N*/ 			rTypes[rTypes.Count()-1].nEndPos = (USHORT)nPos;
/*N*/ 
/*N*/             nScriptType = xBI->getScriptType( aOUText, nPos );
/*N*/ 			long nEndPos = xBI->endOfScript( aOUText, nPos, nScriptType );
/*N*/ 
/*N*/             // #96850# Handle blanks as weak, remove if BreakIterator returns WEAK for spaces.
/*N*/ 			if ( ( nScriptType == i18n::ScriptType::LATIN ) && ( aOUText.getStr()[ nPos ] == 0x20 ) )
/*N*/             {
/*?*/                 BOOL bOnlySpaces = TRUE;
/*?*/                 for ( USHORT n = nPos+1; ( n < nEndPos ) && bOnlySpaces; n++ )
/*?*/                 {
/*?*/                     if ( aOUText.getStr()[ n ] != 0x20 )
/*?*/                         bOnlySpaces = FALSE;
/*?*/                 }
/*?*/                 if ( bOnlySpaces )
/*?*/                     nScriptType = i18n::ScriptType::WEAK;
/*N*/             }
/*N*/ 
/*N*/             if ( ( nScriptType == i18n::ScriptType::WEAK ) || ( nScriptType == rTypes[rTypes.Count()-1].nScriptType ) )
/*N*/             {
/*?*/                 // Expand last ScriptTypePosInfo, don't create weak or unecessary portions
/*?*/                 rTypes[rTypes.Count()-1].nEndPos = (USHORT)nEndPos;
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/ 			    rTypes.Insert( ScriptTypePosInfo( nScriptType, (USHORT)nPos, nTextLen ), rTypes.Count() );
/*N*/             }
/*N*/ 
/*N*/ 			nPos = nEndPos;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( rTypes[0].nScriptType == i18n::ScriptType::WEAK )
/*N*/ 			rTypes[0].nScriptType = ( rTypes.Count() > 1 ) ? rTypes[1].nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
/*N*/ 	}
/*N*/ }

/*N*/ USHORT ImpEditEngine::GetScriptType( const EditPaM& rPaM, USHORT* pEndPos ) const
/*N*/ {
/*N*/ 	USHORT nScriptType = 0;
/*N*/ 
/*N*/ 	if ( pEndPos )
/*?*/ 		*pEndPos = rPaM.GetNode()->Len();
/*N*/ 
/*N*/ 	if ( rPaM.GetNode()->Len() )
/*N*/ 	{
/*N*/  		USHORT nPara = GetEditDoc().GetPos( rPaM.GetNode() );
/*N*/ 		ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 		if ( !pParaPortion->aScriptInfos.Count() )
/*?*/ 			((ImpEditEngine*)this)->InitScriptTypes( nPara );
/*N*/ 
/*N*/ 		ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 		USHORT nPos = rPaM.GetIndex();
/*N*/ 		for ( USHORT n = 0; n < rTypes.Count(); n++ )
/*N*/ 		{
/*N*/ 			if ( ( rTypes[n].nStartPos <= nPos ) && ( rTypes[n].nEndPos >= nPos ) )
/*N*/ 	   		{
/*N*/ 				nScriptType = rTypes[n].nScriptType;
/*N*/ 				if( pEndPos )
/*?*/ 					*pEndPos = rTypes[n].nEndPos;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nScriptType ? nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
/*N*/ }

/*N*/ USHORT ImpEditEngine::GetScriptType( const EditSelection& rSel ) const
/*N*/ {
/*N*/ 	EditSelection aSel( rSel );
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 
/*N*/ 	short nScriptType = 0;
/*N*/ 
/*N*/  	USHORT nStartPara = GetEditDoc().GetPos( aSel.Min().GetNode() );
/*N*/  	USHORT nEndPara = GetEditDoc().GetPos( aSel.Max().GetNode() );
/*N*/ 
/*N*/ 	for ( USHORT nPara = nStartPara; nPara <= nEndPara; nPara++ )
/*N*/ 	{
/*N*/ 		ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 		if ( !pParaPortion->aScriptInfos.Count() )
/*N*/ 			((ImpEditEngine*)this)->InitScriptTypes( nPara );
/*N*/ 
/*N*/ 		ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 
/*N*/ 		USHORT nS = ( nPara == nStartPara ) ? aSel.Min().GetIndex() : 0;
/*N*/ 		USHORT nE = ( nPara == nEndPara ) ? aSel.Max().GetIndex() : pParaPortion->GetNode()->Len();
/*N*/ 		for ( USHORT n = 0; n < rTypes.Count(); n++ )
/*N*/ 		{
/*N*/ 			if ( ( rTypes[n].nStartPos <= nE ) && ( rTypes[n].nEndPos >= nS ) )
/*N*/ 		   	{
/*N*/ 				if ( rTypes[n].nScriptType != i18n::ScriptType::WEAK )
/*N*/ 				{
/*N*/                     nScriptType |= GetItemScriptType ( rTypes[n].nScriptType );
/*N*/                 }
/*N*/                 else
/*N*/ 				{
/*?*/                     if ( !nScriptType && n )
/*?*/                     {
/*?*/                         // #93548# When starting with WEAK, use prev ScriptType...
/*?*/                         nScriptType = rTypes[n-1].nScriptType;
/*N*/                     }
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nScriptType ? nScriptType : GetI18NScriptTypeOfLanguage( GetDefaultLanguage() );
/*N*/ }

/*N*/ BOOL ImpEditEngine::IsScriptChange( const EditPaM& rPaM ) const
/*N*/ {
/*N*/ 	BOOL bScriptChange = FALSE;
/*N*/ 
/*N*/ 	if ( rPaM.GetNode()->Len() )
/*N*/ 	{
/*N*/ 		USHORT nPara = GetEditDoc().GetPos( rPaM.GetNode() );
/*N*/ 		ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 		if ( !pParaPortion->aScriptInfos.Count() )
/*?*/ 			((ImpEditEngine*)this)->InitScriptTypes( nPara );
/*N*/ 
/*N*/ 		ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 		USHORT nPos = rPaM.GetIndex();
/*N*/ 		for ( USHORT n = 0; n < rTypes.Count(); n++ )
/*N*/ 		{
/*N*/ 			if ( rTypes[n].nStartPos == nPos )
/*N*/ 	   		{
/*N*/ 				bScriptChange = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bScriptChange;
/*N*/ }

/*N*/ BOOL ImpEditEngine::HasScriptType( USHORT nPara, USHORT nType ) const
/*N*/ {
/*N*/     BOOL bTypeFound = FALSE;
/*N*/ 
/*N*/ 	ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 	if ( !pParaPortion->aScriptInfos.Count() )
/*N*/ 		((ImpEditEngine*)this)->InitScriptTypes( nPara );
/*N*/ 
/*N*/     ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 	for ( USHORT n = rTypes.Count(); n && !bTypeFound; )
/*N*/ 	{
/*N*/ 	    if ( rTypes[--n].nScriptType == nType )
/*N*/                 bTypeFound = TRUE;
/*N*/     }
/*N*/     return bTypeFound;
/*N*/ }

/*N*/ void ImpEditEngine::InitWritingDirections( USHORT nPara )
/*N*/ {
/*N*/ 	ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 	WritingDirectionInfos& rInfos = pParaPortion->aWritingDirectionInfos;
/*N*/ 	rInfos.Remove( 0, rInfos.Count() );
/*N*/ 
/*N*/     BOOL bCTL = FALSE;
/*N*/ 	ScriptTypePosInfos& rTypes = pParaPortion->aScriptInfos;
/*N*/ 	for ( USHORT n = 0; n < rTypes.Count(); n++ )
/*N*/ 	{
/*N*/ 		if ( rTypes[n].nScriptType == i18n::ScriptType::COMPLEX )
/*N*/ 	   	{
/*N*/ 			bCTL = TRUE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     const BYTE nDefaultDir = IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;
/*N*/ 	if ( ( bCTL || ( nDefaultDir == UBIDI_RTL ) ) && pParaPortion->GetNode()->Len() )
/*N*/ 	{
/*?*/ 
/*?*/         String aText( *pParaPortion->GetNode() );
/*?*/ 
/*?*/         //
/*?*/         // Bidi functions from icu 2.0
/*?*/         //
/*?*/         UErrorCode nError = U_ZERO_ERROR;
/*?*/         UBiDi* pBidi = ubidi_openSized( aText.Len(), 0, &nError );
/*?*/         nError = U_ZERO_ERROR;
/*?*/ 
/*?*/         ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aText.GetBuffer()), aText.Len(), nDefaultDir, NULL, &nError );	// UChar != sal_Unicode in MinGW
/*?*/         nError = U_ZERO_ERROR;
/*?*/ 
/*?*/         long nCount = ubidi_countRuns( pBidi, &nError );
/*?*/ 
/*?*/         int32_t nStart = 0;
/*?*/         int32_t nEnd;
/*?*/         UBiDiLevel nCurrDir;
/*?*/ 
/*?*/         for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
/*?*/         {
/*?*/             ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
/*?*/             rInfos.Insert( WritingDirectionInfo( nCurrDir, (USHORT)nStart, (USHORT)nEnd ), rInfos.Count() );
/*?*/             nStart = nEnd;
/*?*/         }
/*?*/ 
/*?*/         ubidi_close( pBidi );
/*N*/ 	}
/*N*/ 
/*N*/     // No infos mean no CTL and default dir is L2R...
/*N*/     if ( !rInfos.Count() )
/*N*/         rInfos.Insert( WritingDirectionInfo( 0, 0, (USHORT)pParaPortion->GetNode()->Len() ), rInfos.Count() );
/*N*/ 
/*N*/ }

/*N*/ BOOL ImpEditEngine::IsRightToLeft( USHORT nPara ) const
/*N*/ {
/*N*/     BOOL bR2L = FALSE;
/*N*/     const SvxFrameDirectionItem* pFrameDirItem = NULL;
/*N*/ 
/*N*/     if ( !IsVertical() )
/*N*/     {
/*N*/         bR2L = GetDefaultHorizontalTextDirection() == EE_HTEXTDIR_R2L;
/*N*/         pFrameDirItem = &(const SvxFrameDirectionItem&)GetParaAttrib( nPara, EE_PARA_WRITINGDIR );
/*N*/         if ( pFrameDirItem->GetValue() == FRMDIR_ENVIRONMENT )
/*N*/         {
/*N*/             // #103045# if DefaultHorizontalTextDirection is set, use that value, otherwise pool default.
/*N*/             if ( GetDefaultHorizontalTextDirection() != EE_HTEXTDIR_DEFAULT )
/*N*/             {
/*?*/                 pFrameDirItem = NULL; // bR2L allready set to default horizontal text direction
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // Use pool default
/*N*/                 pFrameDirItem = &(const SvxFrameDirectionItem&)((ImpEditEngine*)this)->GetEmptyItemSet().Get( EE_PARA_WRITINGDIR );
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     if ( pFrameDirItem )
/*N*/         bR2L = pFrameDirItem->GetValue() == FRMDIR_HORI_RIGHT_TOP;
/*N*/ 
/*N*/     return bR2L;
/*N*/ }



/*N*/ BYTE ImpEditEngine::GetRightToLeft( USHORT nPara, USHORT nPos, USHORT* pStart, USHORT* pEnd )
/*N*/ {
/*N*/ //    BYTE nRightToLeft = IsRightToLeft( nPara ) ? 1 : 0;
/*N*/     BYTE nRightToLeft = 0;
/*N*/ 
/*N*/     ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/     if ( pNode && pNode->Len() )
/*N*/     {
/*N*/ 		ParaPortion* pParaPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 		if ( !pParaPortion->aWritingDirectionInfos.Count() )
/*N*/ 			InitWritingDirections( nPara );
/*N*/ 
/*N*/         BYTE nType = 0;
/*N*/ 		WritingDirectionInfos& rDirInfos = pParaPortion->aWritingDirectionInfos;
/*N*/ 		for ( USHORT n = 0; n < rDirInfos.Count(); n++ )
/*N*/ 		{
/*N*/ 			if ( ( rDirInfos[n].nStartPos <= nPos ) && ( rDirInfos[n].nEndPos >= nPos ) )
/*N*/ 	   		{
/*N*/ 				nRightToLeft = rDirInfos[n].nType;
/*N*/                 if ( pStart )
/*?*/                     *pStart = rDirInfos[n].nStartPos;
/*N*/                 if ( pEnd )
/*?*/                     *pEnd = rDirInfos[n].nEndPos;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/     }
/*N*/     return nRightToLeft;
/*N*/ }

/*N*/ SvxAdjust ImpEditEngine::GetJustification( USHORT nPara ) const
/*N*/ {
/*N*/ 	SvxAdjust eJustification = SVX_ADJUST_LEFT;
/*N*/ 
/*N*/     if ( !aStatus.IsOutliner() )
/*N*/     {
/*N*/ 		eJustification = ((const SvxAdjustItem&) GetParaAttrib( nPara, EE_PARA_JUST )).GetAdjust();
/*N*/ 
/*N*/         if ( IsRightToLeft( nPara ) )
/*N*/         {
/*?*/             if ( eJustification == SVX_ADJUST_LEFT )
/*?*/                 eJustification = SVX_ADJUST_RIGHT;
/*?*/             else if ( eJustification == SVX_ADJUST_RIGHT )
/*?*/                 eJustification = SVX_ADJUST_LEFT;
/*N*/         }
/*N*/     }
/*N*/     return eJustification;
/*N*/ }


//	----------------------------------------------------------------------
//	Textaenderung
//	----------------------------------------------------------------------

/*N*/ void ImpEditEngine::ImpRemoveChars( const EditPaM& rPaM, USHORT nChars, EditUndoRemoveChars* pCurUndo )
/*N*/ {
/*N*/ 	aEditDoc.RemoveChars( rPaM, nChars );
/*N*/ 	TextModified();
/*N*/ }



/*N*/ EditPaM ImpEditEngine::ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, BOOL bBackward )
/*N*/ {
/*N*/ 	DBG_ASSERT( pLeft != pRight, "Den gleichen Absatz zusammenfuegen ?" );
/*N*/ 	DBG_ASSERT( aEditDoc.GetPos( pLeft ) != USHRT_MAX, "Einzufuegenden Node nicht gefunden(1)" );
/*N*/ 	DBG_ASSERT( aEditDoc.GetPos( pRight ) != USHRT_MAX, "Einzufuegenden Node nicht gefunden(2)" );
/*N*/ 
/*N*/ 	USHORT nParagraphTobeDeleted = aEditDoc.GetPos( pRight );
/*N*/ 	DeletedNodeInfo* pInf = new DeletedNodeInfo( (ULONG)pRight, nParagraphTobeDeleted );
/*N*/ 	aDeletedNodes.Insert( pInf, aDeletedNodes.Count() );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( IsUndoEnabled() && !IsInUndo() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 InsertUndo( new EditUndoConnectParas( this,
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if ( bBackward )
/*N*/ 	{
/*?*/ 		pLeft->SetStyleSheet( pRight->GetStyleSheet(), TRUE );
/*?*/ 		pLeft->GetContentAttribs().GetItems().Set( pRight->GetContentAttribs().GetItems() );
/*?*/ 		pLeft->GetCharAttribs().GetDefFont() = pRight->GetCharAttribs().GetDefFont();
/*N*/ 	}
/*N*/ 
/*N*/ 	ParaAttribsChanged( pLeft );
/*N*/ 
/*N*/ 	// Erstmal Portions suchen, da pRight nach ConnectParagraphs weg.
/*N*/ 	ParaPortion* pLeftPortion = FindParaPortion( pLeft );
/*N*/ 	ParaPortion* pRightPortion = FindParaPortion( pRight );
/*N*/ 	DBG_ASSERT( pLeftPortion, "Blinde Portion in ImpConnectParagraphs(1)" );
/*N*/ 	DBG_ASSERT( pRightPortion, "Blinde Portion in ImpConnectParagraphs(2)" );
/*N*/ 	DBG_ASSERT( nParagraphTobeDeleted == GetParaPortions().GetPos( pRightPortion ), "NodePos != PortionPos?" );
/*N*/ 
/*N*/ 	if ( IsCallParaInsertedOrDeleted() )
/*N*/ 		GetEditEnginePtr()->ParagraphDeleted( nParagraphTobeDeleted );
/*N*/ 
/*N*/ 	EditPaM aPaM = aEditDoc.ConnectParagraphs( pLeft, pRight );
/*N*/ 	GetParaPortions().Remove( nParagraphTobeDeleted );
/*N*/ 	delete pRightPortion;
/*N*/ 
/*N*/ 	pLeftPortion->MarkSelectionInvalid( aPaM.GetIndex(), pLeft->Len() );
/*N*/ 
/*N*/ 	// der rechte Node wird von EditDoc::ConnectParagraphs() geloescht.
/*N*/ 
/*N*/ 	if ( GetTextRanger() )
/*N*/ 	{
/*?*/ 		// Durch das zusammenfuegen wird der linke zwar neu formatiert, aber
/*?*/ 		// wenn sich dessen Hoehe nicht aendert bekommt die Formatierung die
/*?*/ 		// Aenderung der Gesaamthoehe des Textes zu spaet mit...
/*?*/ 		for ( USHORT n = nParagraphTobeDeleted; n < GetParaPortions().Count(); n++ )
/*?*/ 		{
/*?*/ 			ParaPortion* pPP = GetParaPortions().GetObject( n );
/*?*/ 			pPP->MarkSelectionInvalid( 0, pPP->GetNode()->Len() );
/*?*/ 			pPP->GetLines().Reset();
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	TextModified();
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }


/*N*/ EditPaM ImpEditEngine::ImpDeleteSelection( EditSelection aSel )
/*N*/ {
/*N*/ 	if ( !aSel.HasRange() )
/*?*/ 		return aSel.Min();
/*N*/ 
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 	EditPaM aStartPaM( aSel.Min() );
/*N*/ 	EditPaM aEndPaM( aSel.Max() );
/*N*/ 
/*N*/ 	CursorMoved( aStartPaM.GetNode() ); // nur damit neu eingestellte Attribute verschwinden...
/*N*/ 	CursorMoved( aEndPaM.GetNode() );	// nur damit neu eingestellte Attribute verschwinden...
/*N*/ 
/*N*/ 	DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index im Wald in ImpDeleteSelection" );
/*N*/ 	DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index im Wald in ImpDeleteSelection" );
/*N*/ 
/*N*/ 	USHORT nStartNode = aEditDoc.GetPos( aStartPaM.GetNode() );
/*N*/ 	USHORT nEndNode = aEditDoc.GetPos( aEndPaM.GetNode() );
/*N*/ 
/*N*/ 	DBG_ASSERT( nEndNode != USHRT_MAX, "Start > End ?!" );
/*N*/ 	DBG_ASSERT( nStartNode <= nEndNode, "Start > End ?!" );
/*N*/ 
/*N*/ 	// Alle Nodes dazwischen entfernen....
/*N*/ 	for ( ULONG z = nStartNode+1; z < nEndNode; z++ )
/*N*/ 	{
/*?*/ 		// Immer nStartNode+1, wegen Remove()!
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ImpRemoveParagraph( nStartNode+1 );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
/*N*/ 	{
/*N*/ 		// Den Rest des StartNodes...
/*N*/ 		USHORT nChars;
/*N*/ 		nChars = aStartPaM.GetNode()->Len() - aStartPaM.GetIndex();
/*N*/ 		ImpRemoveChars( aStartPaM, nChars );
/*N*/ 		ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
/*N*/ 		DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(3)" );
/*N*/ 		pPortion->MarkSelectionInvalid( aStartPaM.GetIndex(), aStartPaM.GetNode()->Len() );
/*N*/ 
/*N*/ 		// Den Anfang des EndNodes....
/*N*/ 		nChars = aEndPaM.GetIndex();
/*N*/ 		aEndPaM.SetIndex( 0 );
/*N*/ 		ImpRemoveChars( aEndPaM, nChars );
/*N*/ 		pPortion = FindParaPortion( aEndPaM.GetNode() );
/*N*/ 		DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(4)" );
/*N*/ 		pPortion->MarkSelectionInvalid( 0, aEndPaM.GetNode()->Len() );
/*N*/ 		// Zusammenfuegen....
/*N*/ 		aStartPaM = ImpConnectParagraphs( aStartPaM.GetNode(), aEndPaM.GetNode() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nChars;
/*N*/ 		nChars = aEndPaM.GetIndex() - aStartPaM.GetIndex();
/*N*/ 		ImpRemoveChars( aStartPaM, nChars );
/*N*/ 		ParaPortion* pPortion = FindParaPortion( aStartPaM.GetNode() );
/*N*/ 		DBG_ASSERT( pPortion, "Blinde Portion in ImpDeleteSelection(5)" );
/*N*/ 		pPortion->MarkInvalid( aEndPaM.GetIndex(), aStartPaM.GetIndex() - aEndPaM.GetIndex() );
/*N*/ 	}
/*N*/ 
/*N*/ 	UpdateSelections();
/*N*/ 	TextModified();
/*N*/ 	return aStartPaM;
/*N*/ }


/*N*/ EditPaM ImpEditEngine::ImpInsertText( EditSelection aCurSel, const XubString& rStr )
/*N*/ {
/*N*/ 	EditPaM aPaM;
/*N*/ 	if ( aCurSel.HasRange() )
/*N*/ 		aPaM = ImpDeleteSelection( aCurSel );
/*N*/ 	else
/*N*/ 		aPaM = aCurSel.Max();
/*N*/ 
/*N*/ 	EditPaM aCurPaM( aPaM );	// fuers Invalidieren
/*N*/ 
/*N*/ 	XubString aText( rStr );
/*N*/ 	aText.ConvertLineEnd( LINEEND_LF );
/*N*/ 	SfxVoidItem aTabItem( EE_FEATURE_TAB );
/*N*/ 
/*N*/ 	// Konvertiert nach LineSep = \n
/*N*/ 	// Token mit LINE_SEP abfragen,
/*N*/ 	// da der MAC-Compiler aus \n etwas anderes macht!
/*N*/ 
/*N*/ 	USHORT nStart = 0;
/*N*/ 	while ( nStart < aText.Len() )
/*N*/ 	{
/*N*/ 		USHORT nEnd = aText.Search( LINE_SEP, nStart );
/*N*/ 		if ( nEnd == STRING_NOTFOUND )
/*N*/ 			nEnd = aText.Len();	// nicht dereferenzieren!
/*N*/ 
/*N*/ 		// Start == End => Leerzeile
/*N*/ 		if ( nEnd > nStart )
/*N*/ 		{
/*N*/ 			XubString aLine( aText, nStart, nEnd-nStart );
/*N*/ 			xub_StrLen nChars = aPaM.GetNode()->Len() + aLine.Len();
/*N*/ 			if ( nChars > MAXCHARSINPARA )
/*N*/ 			{
/*?*/                 USHORT nMaxNewChars = MAXCHARSINPARA-aPaM.GetNode()->Len();
/*?*/ 				nEnd -= ( aLine.Len() - nMaxNewChars );	// Dann landen die Zeichen im naechsten Absatz.
/*?*/ 				aLine.Erase( nMaxNewChars );            // Del Rest...
/*N*/ 			}
/*N*/ #ifndef SVX_LIGHT
/*N*/ 			if ( IsUndoEnabled() && !IsInUndo() )
/*N*/ 				InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), aLine ) );
/*N*/ #endif
/*N*/ 			// Tabs ?
/*N*/ 			if ( aLine.Search( '\t' ) == STRING_NOTFOUND )
/*N*/ 				aPaM = aEditDoc.InsertText( aPaM, aLine );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				USHORT nStart2 = 0;
/*N*/ 				while ( nStart2 < aLine.Len() )
/*N*/ 				{
/*N*/ 					USHORT nEnd2 = aLine.Search( '\t', nStart2 );
/*N*/ 					if ( nEnd2 == STRING_NOTFOUND )
/*?*/ 						nEnd2 = aLine.Len();	// nicht dereferenzieren!
/*N*/ 
/*N*/ 					if ( nEnd2 > nStart2 )
/*?*/ 						aPaM = aEditDoc.InsertText( aPaM, XubString( aLine, nStart2, nEnd2-nStart2 ) );
/*N*/ 					if ( nEnd2 < aLine.Len() )
/*N*/ 					{
/*N*/ 						// aPaM = ImpInsertFeature( EditSelection( aPaM, aPaM ),  );
/*N*/ 						aPaM = aEditDoc.InsertFeature( aPaM, aTabItem );
/*N*/ 					}
/*N*/ 					nStart2 = nEnd2+1;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
/*N*/ 			DBG_ASSERT( pPortion, "Blinde Portion in InsertText" );
/*N*/ 			pPortion->MarkInvalid( aCurPaM.GetIndex(), aLine.Len() );
/*N*/ 		}
/*N*/ 		if ( nEnd < aText.Len() )
/*N*/ 			aPaM = ImpInsertParaBreak( aPaM );
/*N*/ 
/*N*/ 		nStart = nEnd+1;
/*N*/ 	}
/*N*/ 
/*N*/ 	TextModified();
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::ImpFastInsertText( EditPaM aPaM, const XubString& rStr )
/*N*/ {
/*N*/ 	DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "FastInsertText: Zeilentrenner nicht erlaubt!" );
/*N*/ 	DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "FastInsertText: Zeilentrenner nicht erlaubt!" );
/*N*/ 	DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "FastInsertText: Features nicht erlaubt!" );
/*N*/ 
/*N*/ 	if ( ( aPaM.GetNode()->Len() + rStr.Len() ) < MAXCHARSINPARA )
/*N*/ 	{
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		if ( IsUndoEnabled() && !IsInUndo() )
/*?*/ 			InsertUndo( new EditUndoInsertChars( this, CreateEPaM( aPaM ), rStr ) );
/*N*/ #endif
/*N*/ 
/*N*/ 		aPaM = aEditDoc.InsertText( aPaM, rStr );
/*N*/ 		TextModified();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		aPaM = ImpInsertText( aPaM, rStr );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::ImpInsertFeature( EditSelection aCurSel, const SfxPoolItem& rItem )
/*N*/ {
/*N*/ 	EditPaM aPaM;
/*N*/ 	if ( aCurSel.HasRange() )
/*?*/ 		aPaM = ImpDeleteSelection( aCurSel );
/*N*/ 	else
/*N*/ 		aPaM = aCurSel.Max();
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( IsUndoEnabled() && !IsInUndo() )
/*N*/ 		InsertUndo( new EditUndoInsertFeature( this, CreateEPaM( aPaM ), rItem ) );
/*N*/ #endif
/*N*/ 	aPaM = aEditDoc.InsertFeature( aPaM, rItem );
/*N*/ 
/*N*/ 	ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
/*N*/ 	DBG_ASSERT( pPortion, "Blinde Portion in InsertFeature" );
/*N*/ 	pPortion->MarkInvalid( aPaM.GetIndex()-1, 1 );
/*N*/ 
/*N*/ 	TextModified();
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::ImpInsertParaBreak( const EditPaM& rPaM, BOOL bKeepEndingAttribs )
/*N*/ {
/*N*/ 	EditPaM aPaM( aEditDoc.InsertParaBreak( rPaM, bKeepEndingAttribs ) );
/*N*/ 
/*N*/ 	ParaPortion* pPortion = FindParaPortion( rPaM.GetNode() );
/*N*/ 	DBG_ASSERT( pPortion, "Blinde Portion in ImpInsertParaBreak" );
/*N*/ 	pPortion->MarkInvalid( rPaM.GetIndex(), 0 );
/*N*/ 
/*N*/ 	// Optimieren: Nicht unnoetig viele GetPos auf die Listen ansetzen!
/*N*/ 	// Hier z.B. bei Undo, aber auch in allen anderen Methoden.
/*N*/ 	USHORT nPos = GetParaPortions().GetPos( pPortion );
/*N*/ 	ParaPortion* pNewPortion = new ParaPortion( aPaM.GetNode() );
/*N*/ 	GetParaPortions().Insert( pNewPortion, nPos + 1 );
/*N*/ 	ParaAttribsChanged( pNewPortion->GetNode() );
/*N*/ 	if ( IsCallParaInsertedOrDeleted() )
/*N*/ 		GetEditEnginePtr()->ParagraphInserted( nPos+1 );
/*N*/ 
/*N*/ 	CursorMoved( rPaM.GetNode() );	// falls leeres Attribut entstanden.
/*N*/ 	TextModified();
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::ImpFastInsertParagraph( USHORT nPara )
/*N*/ {
/*N*/ 	ContentNode* pNode = new ContentNode( aEditDoc.GetItemPool() );
/*N*/ 	// Falls FlatMode, wird spaeter kein Font eingestellt:
/*N*/ 	pNode->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();
/*N*/ 
/*N*/ 	aEditDoc.Insert( pNode, nPara );
/*N*/ 
/*N*/ 	ParaPortion* pNewPortion = new ParaPortion( pNode );
/*N*/ 	GetParaPortions().Insert( pNewPortion, nPara );
/*N*/ 	if ( IsCallParaInsertedOrDeleted() )
/*N*/ 		GetEditEnginePtr()->ParagraphInserted( nPara );
/*N*/ 
/*N*/ 	return EditPaM( pNode, 0 );
/*N*/ }

/*N*/ BOOL ImpEditEngine::UpdateFields()
/*N*/ {
/*N*/ 	BOOL bChanges = FALSE;
/*N*/ 	USHORT nParas = GetEditDoc().Count();
/*N*/ 	for ( USHORT nPara = 0; nPara < nParas; nPara++ )
/*N*/ 	{
/*N*/ 		BOOL bChangesInPara = FALSE;
/*N*/ 		ContentNode* pNode = GetEditDoc().GetObject( nPara );
/*N*/ 		DBG_ASSERT( pNode, "NULL-Pointer im Doc" );
/*N*/ 		CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
/*N*/ 		USHORT nAttrs = rAttribs.Count();
/*N*/ 		for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
/*N*/ 		{
/*N*/ 			EditCharAttrib* pAttr = rAttribs[nAttr];
/*N*/ 			if ( pAttr->Which() == EE_FEATURE_FIELD )
/*N*/ 			{
/*N*/ 				EditCharAttribField* pField = (EditCharAttribField*)pAttr;
/*N*/ 				EditCharAttribField* pCurrent = new EditCharAttribField( *pField );
/*N*/ 				pField->Reset();
/*N*/ 
/*N*/                 if ( aStatus.MarkFields() )
/*N*/                     pField->GetFldColor() = new Color( GetColorConfig().GetColorValue( WRITERFIELDSHADINGS ).nColor );
/*N*/ 
/*N*/ 				XubString aFldValue = GetEditEnginePtr()->CalcFieldValue(
/*N*/ 										(const SvxFieldItem&)*pField->GetItem(),
/*N*/ 										nPara, pField->GetStart(),
/*N*/ 										pField->GetTxtColor(), pField->GetFldColor() );
/*N*/ 				pField->GetFieldValue() = aFldValue;
/*N*/ 				if ( *pField != *pCurrent )
/*N*/ 				{
/*N*/ 					bChanges = TRUE;
/*N*/ 					bChangesInPara = TRUE;
/*N*/ 				}
/*N*/ 				delete pCurrent;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bChangesInPara )
/*N*/ 		{
/*N*/ 			// ggf. etwas genauer invalidieren.
/*N*/ 			ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
/*N*/ 			DBG_ASSERT( pPortion, "NULL-Pointer im Doc" );
/*N*/ 			pPortion->MarkSelectionInvalid( 0, pNode->Len() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bChanges;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::InsertLineBreak( EditSelection aCurSel )
/*N*/ {
/*N*/ 	EditPaM aPaM( ImpInsertFeature( aCurSel, SfxVoidItem( EE_FEATURE_LINEBR ) ) );
/*N*/ 	return aPaM;
/*N*/ }

//	----------------------------------------------------------------------
//	Hilfsfunktionen
//	----------------------------------------------------------------------

/*N*/ EditPaM ImpEditEngine::GetPaM( Point aDocPos, BOOL bSmart )
/*N*/ {
/*N*/ 	DBG_ASSERT( GetUpdateMode(), "Darf bei Update=FALSE nicht erreicht werden: GetPaM" );
/*N*/ 
/*N*/ 	long nY = 0;
/*N*/ 	long nTmpHeight;
/*N*/ 	EditPaM aPaM;
/*N*/ 	USHORT nPortion;
/*N*/ 	for ( nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
/*N*/ 	{
/*N*/ 		ParaPortion* pPortion = GetParaPortions().GetObject(nPortion);
/*N*/ 		nTmpHeight = pPortion->GetHeight(); 	// sollte auch bei !bVisible richtig sein!
/*N*/ 		nY += nTmpHeight;
/*N*/ 		if ( nY > aDocPos.Y() )
/*N*/ 		{
/*N*/ 			nY -= nTmpHeight;
/*N*/ 			aDocPos.Y() -= nY;
/*N*/ 			// unsichtbare Portions ueberspringen:
/*N*/ 			while ( pPortion && !pPortion->IsVisible() )
/*N*/ 			{
/*?*/ 				nPortion++;
/*?*/ 				pPortion = GetParaPortions().SaveGetObject( nPortion );
/*N*/ 			}
/*N*/ 			DBG_ASSERT( pPortion, "Keinen sichtbaren Absatz gefunden: GetPaM" );
/*N*/ 			aPaM = GetPaM( pPortion, aDocPos, bSmart );
/*N*/ 			return aPaM;
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*?*/ 	// Dann den letzten sichtbaren Suchen:
/*?*/ 	nPortion = GetParaPortions().Count()-1;
/*?*/ 	while ( nPortion && !GetParaPortions()[nPortion]->IsVisible() )
/*?*/ 		nPortion--;
/*?*/ 
/*?*/ 	DBG_ASSERT( GetParaPortions()[nPortion]->IsVisible(), "Keinen sichtbaren Absatz gefunden: GetPaM" );
/*?*/ 	aPaM.SetNode( GetParaPortions()[nPortion]->GetNode() );
/*?*/ 	aPaM.SetIndex( GetParaPortions()[nPortion]->GetNode()->Len() );
/*?*/ 	return aPaM;
/*N*/ }

/*N*/ sal_uInt32 ImpEditEngine::GetTextHeight() const
/*N*/ {
/*N*/ 	DBG_ASSERT( GetUpdateMode(), "Sollte bei Update=FALSE nicht verwendet werden: GetTextHeight" );
/*N*/ 	DBG_ASSERT( IsFormatted() || IsFormatting(), "GetTextHeight: Nicht formatiert" );
/*N*/ 	return nCurTextHeight;
/*N*/ }

/*N*/ sal_uInt32 ImpEditEngine::CalcTextWidth( BOOL bIgnoreExtraSpace )
/*N*/ {
/*N*/ 	// Wenn noch nicht formatiert und nicht gerade dabei.
/*N*/ 	// Wird in der Formatierung bei AutoPageSize gerufen.
/*N*/ 	if ( !IsFormatted() && !IsFormatting() )
/*?*/ 		FormatDoc();
/*N*/ 
/*N*/ 	EditLine* pLine;
/*N*/ 
/*N*/ 	long nMaxWidth = 0;
/*N*/ 	long nCurWidth = 0;
/*N*/ 
/*N*/ 	// --------------------------------------------------
/*N*/ 	// Ueber alle Absaetze...
/*N*/ 	// --------------------------------------------------
/*N*/ 	USHORT nParas = GetParaPortions().Count();
/*N*/ 	USHORT nBiggestPara = 0;
/*N*/ 	USHORT nBiggestLine = 0;
/*N*/ 	for ( USHORT nPara = 0; nPara < nParas; nPara++ )
/*N*/ 	{
/*N*/ 		ParaPortion* pPortion = GetParaPortions().GetObject( nPara );
/*N*/ 		const SvxLRSpaceItem& rLRItem = GetLRSpaceItem( pPortion->GetNode() );
/*N*/ 
/*N*/ 		if ( pPortion->IsVisible() )
/*N*/ 		{
/*N*/ 			// --------------------------------------------------
/*N*/ 			// Ueber die Zeilen des Absatzes...
/*N*/ 			// --------------------------------------------------
/*N*/ 			ULONG nLines = pPortion->GetLines().Count();
/*N*/ 			for ( USHORT nLine = 0; nLine < nLines; nLine++ )
/*N*/ 			{
/*N*/ 				pLine = pPortion->GetLines().GetObject( nLine );
/*N*/ 				DBG_ASSERT( pLine, "NULL-Pointer im Zeileniterator in CalcWidth" );
/*N*/ 				// nCurWidth = pLine->GetStartPosX();
/*N*/ 				// Bei Center oder Right haengt die breite von der
/*N*/ 				// Papierbreite ab, hier nicht erwuenscht.
/*N*/ 				// Am besten generell nicht auf StartPosX verlassen,
/*N*/ 				// es muss auch die rechte Einrueckung beruecksichtigt werden!
/*N*/ 				nCurWidth = GetXValue( rLRItem.GetTxtLeft() );
/*N*/ 				if ( nLine == 0 )
/*N*/ 				{
/*N*/ 					long nFI = GetXValue( rLRItem.GetTxtFirstLineOfst() );
/*N*/ 					nCurWidth += nFI;
/*N*/ 					if ( pPortion->GetBulletX() > nCurWidth )
/*N*/ 					{
/*?*/ 						nCurWidth -= nFI;	// LI?
/*?*/ 						if ( pPortion->GetBulletX() > nCurWidth )
/*?*/ 							nCurWidth = pPortion->GetBulletX();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				nCurWidth += GetXValue( rLRItem.GetRight() );
/*N*/ 				nCurWidth += CalcLineWidth( pPortion, pLine, bIgnoreExtraSpace );
/*N*/ 				if ( nCurWidth > nMaxWidth )
/*N*/ 				{
/*N*/ 					nMaxWidth = nCurWidth;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( nMaxWidth < 0 )
/*?*/ 		nMaxWidth = 0;
/*N*/ 
/*N*/ 	nMaxWidth++; // Ein breiter, da in CreateLines bei >= umgebrochen wird.
/*N*/ 	return (sal_uInt32)nMaxWidth;
/*N*/ }

/*N*/ sal_uInt32 ImpEditEngine::CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, BOOL bIgnoreExtraSpace )
/*N*/ {
/*N*/ 	USHORT nPara = GetEditDoc().GetPos( pPortion->GetNode() );
/*N*/     ULONG nOldLayoutMode = GetRefDevice()->GetLayoutMode();
/*N*/ 
/*N*/     ImplInitLayoutMode( GetRefDevice(), nPara, 0xFFFF );
/*N*/ 
/*N*/     SvxAdjust eJustification = GetJustification( nPara );
/*N*/ 
/*N*/     // Berechnung der Breite ohne die Indents...
/*N*/ 	sal_uInt32 nWidth = 0;
/*N*/     USHORT nPos = pLine->GetStart();
/*N*/ 	for ( USHORT nTP = pLine->GetStartPortion(); nTP <= pLine->GetEndPortion(); nTP++ )
/*N*/ 	{
/*N*/ 		TextPortion* pTextPortion = pPortion->GetTextPortions().GetObject( nTP );
/*N*/ 		switch ( pTextPortion->GetKind() )
/*N*/ 		{
/*N*/ 			case PORTIONKIND_FIELD:
/*N*/ 			case PORTIONKIND_HYPHENATOR:
/*N*/ 			case PORTIONKIND_TAB:
/*N*/ 			{
/*N*/ 				nWidth += pTextPortion->GetSize().Width();
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case PORTIONKIND_TEXT:
/*N*/ 			{
/*N*/                 if ( ( eJustification != SVX_ADJUST_BLOCK ) || ( !bIgnoreExtraSpace ) )
/*N*/                 {
/*N*/ 				    nWidth += pTextPortion->GetSize().Width();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*?*/ 	                SvxFont aTmpFont( pPortion->GetNode()->GetCharAttribs().GetDefFont() );
/*?*/ 				    SeekCursor( pPortion->GetNode(), nPos+1, aTmpFont );
/*?*/ 				    aTmpFont.SetPhysFont( GetRefDevice() );
/*?*/ 					nWidth += aTmpFont.QuickGetTextSize( GetRefDevice(), *pPortion->GetNode(), nPos, pTextPortion->GetLen(), NULL ).Width();
/*N*/                 }
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/         nPos += pTextPortion->GetLen();
/*N*/ 	}
/*N*/ 
/*N*/     GetRefDevice()->SetLayoutMode( nOldLayoutMode );
/*N*/ 
/*N*/     return nWidth;
/*N*/ }

/*N*/ sal_uInt32 ImpEditEngine::CalcTextHeight()
/*N*/ {
/*N*/ 	DBG_ASSERT( GetUpdateMode(), "Sollte bei Update=FALSE nicht verwendet werden: CalcTextHeight" );
/*N*/ 	sal_uInt32 nY = 0;
/*N*/ 	for ( USHORT nPortion = 0; nPortion < GetParaPortions().Count(); nPortion++ )
/*N*/ 		nY += GetParaPortions()[nPortion]->GetHeight();
/*N*/ 	return nY;
/*N*/ }

/*N*/ USHORT ImpEditEngine::GetLineCount( USHORT nParagraph ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( nParagraph < GetParaPortions().Count(), "GetLineCount: Out of range" );
/*N*/ 	ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nParagraph );
/*N*/ 	DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetLineCount" );
/*N*/ 	if ( pPPortion )
/*N*/ 		return pPPortion->GetLines().Count();
/*N*/ 
/*N*/ 	return 0xFFFF;
/*N*/ }




/*N*/ void ImpEditEngine::UpdateSelections()
/*N*/ {
/*N*/ 	USHORT nInvNodes = aDeletedNodes.Count();
/*N*/ 
/*N*/ 	// Pruefen, ob eine der Selektionen auf einem geloeschten Node steht...
/*N*/ 	// Wenn der Node gueltig ist, muss noch der Index geprueft werden!
/*N*/ 	for ( USHORT nView = 0; nView < aEditViews.Count(); nView++ )
/*N*/ 	{
/*N*/ 		EditView* pView = aEditViews.GetObject(nView);
/*N*/ 		DBG_CHKOBJ( pView, EditView, 0 );
/*N*/ 		EditSelection aCurSel( pView->pImpEditView->GetEditSelection() );
/*N*/ 		BOOL bChanged = FALSE;
/*N*/ 		for ( USHORT n = 0; n < nInvNodes; n++ )
/*N*/ 		{
/*?*/ 			DeletedNodeInfo* pInf = aDeletedNodes.GetObject( n );
/*?*/ 			if ( ( ( ULONG )(aCurSel.Min().GetNode()) == pInf->GetInvalidAdress() ) ||
/*?*/ 				 ( ( ULONG )(aCurSel.Max().GetNode()) == pInf->GetInvalidAdress() ) )
/*?*/ 			{
/*?*/ 				// ParaPortions verwenden, da jetzt auch versteckte
/*?*/ 				// Absaetze beruecksichtigt werden muessen!
/*?*/ 				USHORT nPara = pInf->GetPosition();
/*?*/ 				ParaPortion* pPPortion = GetParaPortions().SaveGetObject( nPara );
/*?*/ 				if ( !pPPortion ) // letzter Absatz
/*?*/ 				{
/*?*/ 					nPara = GetParaPortions().Count()-1;
/*?*/ 					pPPortion = GetParaPortions().GetObject( nPara );
/*?*/ 				}
/*?*/ 				DBG_ASSERT( pPPortion, "Leeres Document in UpdateSelections ?" );
/*?*/ 				// Nicht aus einem verstecktem Absatz landen:
/*?*/ 				USHORT nCurPara = nPara;
/*?*/ 				USHORT nLastPara = GetParaPortions().Count()-1;
/*?*/ 				while ( nPara <= nLastPara && !GetParaPortions()[nPara]->IsVisible() )
/*?*/ 					nPara++;
/*?*/ 				if ( nPara > nLastPara ) // dann eben rueckwaerts...
/*?*/ 				{
/*?*/ 					nPara = nCurPara;
/*?*/ 					while ( nPara && !GetParaPortions()[nPara]->IsVisible() )
/*?*/ 						nPara--;
/*?*/ 				}
/*?*/ 				DBG_ASSERT( GetParaPortions()[nPara]->IsVisible(), "Keinen sichtbaren Absatz gefunden: UpdateSelections" );
/*?*/ 
/*?*/ 				ParaPortion* pParaPortion = GetParaPortions()[nPara];
/*?*/ 				EditSelection aTmpSelection( EditPaM( pParaPortion->GetNode(), 0 ) );
/*?*/ 				pView->pImpEditView->SetEditSelection( aTmpSelection );
/*?*/ 				bChanged=TRUE;
/*?*/ 				break;	// for-Schleife
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bChanged )
/*N*/ 		{
/*N*/ 			// Index prueffen, falls Node geschrumpft.
/*N*/ 			if ( aCurSel.Min().GetIndex() > aCurSel.Min().GetNode()->Len() )
/*N*/ 			{
/*?*/ 				aCurSel.Min().GetIndex() = aCurSel.Min().GetNode()->Len();
/*?*/ 				pView->pImpEditView->SetEditSelection( aCurSel );
/*N*/ 			}
/*N*/ 			if ( aCurSel.Max().GetIndex() > aCurSel.Max().GetNode()->Len() )
/*N*/ 			{
/*?*/ 				aCurSel.Max().GetIndex() = aCurSel.Max().GetNode()->Len();
/*?*/ 				pView->pImpEditView->SetEditSelection( aCurSel );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Loeschen...
/*N*/ 	for ( USHORT n = 0; n < nInvNodes; n++ )
/*N*/ 	{
/*N*/ 		DeletedNodeInfo* pInf = aDeletedNodes.GetObject( n );
/*N*/ 		delete pInf;
/*N*/ 	}
/*N*/ 	aDeletedNodes.Remove( 0, aDeletedNodes.Count() );
/*N*/ }

/*N*/ EditSelection ImpEditEngine::ConvertSelection( USHORT nStartPara, USHORT nStartPos,
/*N*/ 							 USHORT nEndPara, USHORT nEndPos ) const
/*N*/ {
/*N*/ 	EditSelection aNewSelection;
/*N*/ 
/*N*/ 	// Start...
/*N*/ 	ContentNode* pNode = aEditDoc.SaveGetObject( nStartPara );
/*N*/ 	USHORT nIndex = nStartPos;
/*N*/ 	if ( !pNode )
/*N*/ 	{
/*?*/ 		pNode = aEditDoc[ aEditDoc.Count()-1 ];
/*?*/ 		nIndex = pNode->Len();
/*N*/ 	}
/*N*/ 	else if ( nIndex > pNode->Len() )
/*?*/ 		nIndex = pNode->Len();
/*N*/ 
/*N*/ 	aNewSelection.Min().SetNode( pNode );
/*N*/ 	aNewSelection.Min().SetIndex( nIndex );
/*N*/ 
/*N*/ 	// End...
/*N*/ 	pNode = aEditDoc.SaveGetObject( nEndPara );
/*N*/ 	nIndex = nEndPos;
/*N*/ 	if ( !pNode )
/*N*/ 	{
/*?*/ 		pNode = aEditDoc[ aEditDoc.Count()-1 ];
/*?*/ 		nIndex = pNode->Len();
/*N*/ 	}
/*N*/ 	else if ( nIndex > pNode->Len() )
/*?*/ 		nIndex = pNode->Len();
/*N*/ 
/*N*/ 	aNewSelection.Max().SetNode( pNode );
/*N*/ 	aNewSelection.Max().SetIndex( nIndex );
/*N*/ 
/*N*/ 	return aNewSelection;
/*N*/ }



/*N*/ void ImpEditEngine::SetActiveView( EditView* pView )
/*N*/ {
/*N*/ 	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*N*/ 	// Eigentlich waere jetzt ein bHasVisSel und HideSelection notwendig !!!
/*N*/ 
/*N*/ 	if ( pView == pActiveView )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( pActiveView && pActiveView->HasSelection() )
/*?*/ 		pActiveView->pImpEditView->DrawSelection();	// Wegzeichnen...
/*N*/ 
/*N*/ 	pActiveView = pView;
/*N*/ 
/*N*/ 	if ( pActiveView && pActiveView->HasSelection() )
/*?*/ 		pActiveView->pImpEditView->DrawSelection();	// Wegzeichnen...
/*N*/ 
/*N*/ 	//	NN: Quick fix for #78668#:
/*N*/ 	//	When editing of a cell in Calc is ended, the edit engine is not deleted,
/*N*/ 	//	only the edit views are removed. If mpIMEInfos is still set in that case,
/*N*/ 	//	mpIMEInfos->aPos points to an invalid selection.
/*N*/ 	//	-> reset mpIMEInfos now
/*N*/ 	//	(probably something like this is necessary whenever the content is modified
/*N*/ 	//	from the outside)
/*N*/ 
/*N*/ 	if ( !pView && mpIMEInfos )
/*N*/ 	{
/*?*/ 		delete mpIMEInfos;
/*?*/ 		mpIMEInfos = NULL;
/*N*/ 	}
/*N*/ }



/*N*/ Range ImpEditEngine::GetInvalidYOffsets( ParaPortion* pPortion )
/*N*/ {
/*N*/ 	Range aRange( 0, 0 );
/*N*/ 
/*N*/ 	if ( pPortion->IsVisible() )
/*N*/ 	{
/*N*/ 		const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
/*N*/ 		const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
/*N*/ 		USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
/*N*/ 							? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
/*N*/ 
/*N*/ 		// erst von vorne...
/*N*/ 		USHORT nFirstInvalid = 0xFFFF;
/*N*/ 		USHORT nLine;
/*N*/ 		for ( nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
/*N*/ 		{
/*N*/ 			EditLine* pL = pPortion->GetLines().GetObject( nLine );
/*N*/ 			if ( pL->IsInvalid() )
/*N*/ 			{
/*N*/ 				nFirstInvalid = nLine;
/*N*/ 				break;
/*N*/ 			}
/*?*/ 			if ( nLine && !aStatus.IsOutliner() ) 	// nicht die erste Zeile
/*?*/ 				aRange.Min() += nSBL;
/*?*/ 			aRange.Min() += pL->GetHeight();
/*N*/ 		}
/*N*/ 		DBG_ASSERT( nFirstInvalid != 0xFFFF, "Keine ungueltige Zeile gefunden in GetInvalidYOffset(1)" );
/*N*/ 
/*N*/ 
/*N*/ 		// Abgleichen und weiter...
/*N*/ 		aRange.Max() = aRange.Min();
/*N*/ 		aRange.Max() += pPortion->GetFirstLineOffset();
/*N*/ 		if ( nFirstInvalid != 0 )	// Nur wenn nicht die erste Zeile ungueltig
/*?*/ 			aRange.Min() = aRange.Max();
/*N*/ 
/*N*/ 		USHORT nLastInvalid = pPortion->GetLines().Count()-1;
/*N*/ 		for ( nLine = nFirstInvalid; nLine < pPortion->GetLines().Count(); nLine++ )
/*N*/ 		{
/*N*/ 			EditLine* pL = pPortion->GetLines().GetObject( nLine );
/*N*/ 			if ( pL->IsValid() )
/*N*/ 			{
/*?*/ 				nLastInvalid = nLine;
/*?*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( nLine && !aStatus.IsOutliner() )
/*N*/ 				aRange.Max() += nSBL;
/*N*/ 			aRange.Max() += pL->GetHeight();
/*N*/ 		}
/*N*/ 
/*N*/ 		// MT 07/00 SBL kann jetzt kleiner 100% sein => ggf. die Zeile davor neu ausgeben.
/*N*/ 		if( ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP ) && rLSItem.GetPropLineSpace() &&
/*N*/ 			( rLSItem.GetPropLineSpace() < 100 ) )
/*N*/ 		{
/*?*/ 			EditLine* pL = pPortion->GetLines().GetObject( nFirstInvalid );
/*?*/ 			long n = pL->GetTxtHeight() * ( 100 - rLSItem.GetPropLineSpace() );
/*?*/ 			n /= 100;
/*?*/ 			aRange.Min() -= n;
/*?*/ 			aRange.Max() += n;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( ( nLastInvalid == pPortion->GetLines().Count()-1 ) && ( !aStatus.IsOutliner() ) )
/*N*/ 			aRange.Max() += GetYValue( rULSpace.GetLower() );
/*N*/ 	}
/*N*/ 	return aRange;
/*N*/ }

/*N*/ EditPaM ImpEditEngine::GetPaM( ParaPortion* pPortion, Point aDocPos, BOOL bSmart )
/*N*/ {
/*N*/ 	DBG_ASSERT( pPortion->IsVisible(), "Wozu GetPaM() bei einem unsichtbaren Absatz?" );
/*N*/ 	DBG_ASSERT( IsFormatted(), "GetPaM: Nicht formatiert" );
/*N*/ 
/*N*/ 	USHORT nCurIndex = 0;
/*N*/ 	EditPaM aPaM;
/*N*/ 	aPaM.SetNode( pPortion->GetNode() );
/*N*/ 
/*N*/ 	const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
/*N*/ 	USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX )
/*N*/ 						? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
/*N*/ 
/*N*/ 	long nY = pPortion->GetFirstLineOffset();
/*N*/ 
/*N*/ 	DBG_ASSERT( pPortion->GetLines().Count(), "Leere ParaPortion in GetPaM!" );
/*N*/ 
/*N*/ 	EditLine* pLine = 0;
/*N*/ 	for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
/*N*/ 	{
/*N*/ 		EditLine* pTmpLine = pPortion->GetLines().GetObject( nLine );
/*N*/ 		nY += pTmpLine->GetHeight();
/*N*/ 		if ( !aStatus.IsOutliner() )
/*N*/ 			nY += nSBL;
/*N*/ 		if ( nY > aDocPos.Y() ) 	// das war 'se
/*N*/ 		{
/*N*/ 			pLine = pTmpLine;
/*N*/ 			break;					// richtige Y-Position intressiert nicht
/*N*/ 		}
/*N*/ 
/*?*/ 		nCurIndex += pTmpLine->GetLen();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !pLine ) // darf nur im Bereich von SA passieren!
/*N*/ 	{
/*?*/ 		#ifdef DBG_UTIL
/*?*/ 		 const SvxULSpaceItem& rULSpace =(const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
/*?*/ 		 DBG_ASSERT( nY+GetYValue( rULSpace.GetLower() ) >= aDocPos.Y() , "Index in keiner Zeile, GetPaM ?" );
/*?*/ 		#endif
/*?*/ 		aPaM.SetIndex( pPortion->GetNode()->Len() );
/*?*/ 		return aPaM;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wenn Zeile gefunden, nur noch X-Position => Index
/*N*/ 	nCurIndex = GetChar( pPortion, pLine, aDocPos.X(), bSmart );
/*N*/ 	aPaM.SetIndex( nCurIndex );
/*N*/ 
/*N*/ 	if ( nCurIndex && ( nCurIndex == pLine->GetEnd() ) &&
/*N*/ 		 ( pLine != pPortion->GetLines().GetObject( pPortion->GetLines().Count()-1) ) )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aPaM = CursorLeft( aPaM, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
/*N*/     }
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ USHORT ImpEditEngine::GetChar( ParaPortion* pParaPortion, EditLine* pLine, long nXPos, BOOL bSmart )
/*N*/ {
/*N*/ 	DBG_ASSERT( pLine, "Keine Zeile erhalten: GetChar" );
/*N*/ 
/*N*/     USHORT nChar = 0xFFFF;
/*N*/     USHORT nCurIndex = pLine->GetStart();
/*N*/ 
/*N*/ 
/*N*/     // Search best matching portion with GetPortionXOffset()
/*N*/     for ( USHORT i = pLine->GetStartPortion(); i <= pLine->GetEndPortion(); i++ )
/*N*/ 	{
/*N*/ 		TextPortion* pPortion = pParaPortion->GetTextPortions().GetObject( i );
/*N*/         long nXLeft = GetPortionXOffset( pParaPortion, pLine, i );
/*N*/         long nXRight = nXLeft + pPortion->GetSize().Width();
/*N*/         if ( ( nXLeft <= nXPos ) && ( nXRight >= nXPos ) )
/*N*/         {
/*N*/              nChar = nCurIndex;
/*N*/ 
/*N*/             // Search within Portion...
/*N*/ 
/*N*/             // Don't search within special portions...
/*N*/ 			if ( pPortion->GetKind() != PORTIONKIND_TEXT )
/*N*/ 			{
/*?*/                 // ...but check on which side
/*?*/                 if ( bSmart )
/*?*/                 {
/*?*/ 				    long nLeftDiff = nXPos-nXLeft;
/*?*/ 				    long nRightDiff = nXRight-nXPos;
/*?*/ 				    if ( nRightDiff < nLeftDiff )
/*?*/ 					    nChar++;
/*?*/                 }
/*N*/ 			}
/*N*/             else
/*N*/             {
/*N*/ 			    USHORT nMax = pPortion->GetLen();
/*N*/ 			    USHORT nOffset = 0xFFFF;
/*N*/ 			    USHORT nTmpCurIndex = nChar - pLine->GetStart();
/*N*/ 
/*N*/                 long nXInPortion = nXPos - nXLeft;
/*N*/                 if ( pPortion->IsRightToLeft() )
/*?*/                     nXInPortion = nXRight - nXPos;
/*N*/ 
/*N*/                 // Search in Array...
/*N*/ 			    for ( USHORT x = 0; x < nMax; x++ )
/*N*/ 			    {
/*N*/ 				    long nTmpPosMax = pLine->GetCharPosArray().GetObject( nTmpCurIndex+x );
/*N*/ 				    if ( nTmpPosMax > nXInPortion )
/*N*/ 				    {
/*N*/ 					    // pruefen, ob dieser oder der davor...
/*N*/                         long nTmpPosMin = x ? pLine->GetCharPosArray().GetObject( nTmpCurIndex+x-1 ) : 0;
/*N*/ 					    long nDiffLeft = nXInPortion - nTmpPosMin;
/*N*/ 					    long nDiffRight = nTmpPosMax - nXInPortion;
/*N*/ 					    DBG_ASSERT( nDiffLeft >= 0, "DiffLeft negativ" );
/*N*/ 					    DBG_ASSERT( nDiffRight >= 0, "DiffRight negativ" );
/*N*/ 					    nOffset = ( bSmart && ( nDiffRight < nDiffLeft ) ) ? x+1 : x;
/*N*/ 					    // I18N: If there are character position with the length of 0,
/*N*/                         // they belong to the same character, we can not use this position as an index.
/*N*/ 					    // Skip all 0-positions, cheaper than using XBreakIterator:
/*N*/ 					    if ( nOffset < nMax )
/*N*/ 					    {
/*N*/ 						    const long nX = pLine->GetCharPosArray().GetObject(nOffset);
/*N*/ 						    while ( ( (nOffset+1) < nMax ) && ( pLine->GetCharPosArray().GetObject(nOffset+1) == nX ) )
/*N*/ 							    nOffset++;
/*N*/ 					    }
/*N*/ 					    break;
/*N*/ 				    }
/*N*/ 			    }
/*N*/ 
/*N*/ 			    // Bei Verwendung des CharPosArray duerfte es keine Ungenauigkeiten geben!
/*N*/ 			    // Vielleicht bei Kerning ?
/*N*/ 			    // 0xFFF passiert z.B. bei Outline-Font, wenn ganz hinten.
/*N*/ 			    if ( nOffset == 0xFFFF )
/*N*/ 				    nOffset = nMax;
/*N*/ 
/*N*/ 			    DBG_ASSERT( nOffset <= nMax, "nOffset > nMax" );
/*N*/ 
/*N*/                 nChar += nOffset;
/*N*/ 
/*N*/                 // Check if index is within a cell:
/*N*/                 if ( nChar && ( nChar < pParaPortion->GetNode()->Len() ) )
/*N*/                 {
/*N*/                     EditPaM aPaM( pParaPortion->GetNode(), nChar+1 );
/*N*/                     USHORT nScriptType = GetScriptType( aPaM );
/*N*/                     if ( nScriptType == i18n::ScriptType::COMPLEX )
/*N*/                     {
/*?*/ 		                uno::Reference < i18n::XBreakIterator > xBI = ImplGetBreakIterator();
/*?*/ 		                sal_Int32 nCount = 1;
/*?*/                         lang::Locale aLocale = GetLocale( aPaM );
/*?*/                         USHORT nRight = (USHORT)xBI->nextCharacters( *pParaPortion->GetNode(), nChar, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
/*?*/                         USHORT nLeft = (USHORT)xBI->previousCharacters( *pParaPortion->GetNode(), nRight, aLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, nCount, nCount );
/*?*/                         if ( ( nLeft != nChar ) && ( nRight != nChar ) )
/*?*/                         {
/*?*/                             nChar = ( Abs( nRight - nChar ) < Abs( nLeft - nChar ) ) ? nRight : nLeft;
/*?*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         nCurIndex += pPortion->GetLen();
/*N*/     }
/*N*/ 
/*N*/     if ( nChar == 0xFFFF )
/*N*/     {
/*?*/         nChar = ( nXPos <= pLine->GetStartPosX() ) ? pLine->GetStart() : pLine->GetEnd();
/*N*/     }
/*N*/ 
/*N*/     return nChar;
/*N*/ }

/*N*/ Range ImpEditEngine::GetLineXPosStartEnd( ParaPortion* pParaPortion, EditLine* pLine )
/*N*/ {
/*N*/     Range aLineXPosStartEnd;
/*N*/ 
/*N*/     USHORT nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
/*N*/     if ( !IsRightToLeft( nPara ) )
/*N*/     {
/*N*/         aLineXPosStartEnd.Min() = pLine->GetStartPosX();
/*N*/         aLineXPosStartEnd.Max() = pLine->GetStartPosX() + pLine->GetTextWidth();
/*N*/     }
/*N*/     else
/*N*/     {
/*?*/         aLineXPosStartEnd.Min() = GetPaperSize().Width() - ( pLine->GetStartPosX() + pLine->GetTextWidth() );
/*?*/         aLineXPosStartEnd.Max() = GetPaperSize().Width() - pLine->GetStartPosX();
/*N*/     }
/*N*/ 
/*N*/ 
/*N*/     return aLineXPosStartEnd;
/*N*/ }

/*N*/ long ImpEditEngine::GetPortionXOffset( ParaPortion* pParaPortion, EditLine* pLine, USHORT nTextPortion )
/*N*/ {
/*N*/ 	long nX = pLine->GetStartPosX();
/*N*/ 
/*N*/     for ( USHORT i = pLine->GetStartPortion(); i < nTextPortion; i++ )
/*N*/ 	{
/*N*/ 		TextPortion* pPortion = pParaPortion->GetTextPortions().GetObject( i );
/*N*/ 		switch ( pPortion->GetKind() )
/*N*/ 		{
/*N*/ 			case PORTIONKIND_FIELD:
/*N*/ 			case PORTIONKIND_TEXT:
/*N*/ 			case PORTIONKIND_HYPHENATOR:
/*N*/ 			case PORTIONKIND_TAB:
/*N*/ //	        case PORTIONKIND_EXTRASPACE:
/*N*/ 			{
/*N*/ 				nX += pPortion->GetSize().Width();
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/     }
/*N*/ 
/*N*/     USHORT nPara = GetEditDoc().GetPos( pParaPortion->GetNode() );
/*N*/     BOOL bR2LPara = IsRightToLeft( nPara );
/*N*/ 
/*N*/     TextPortion* pDestPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
/*N*/     if ( pDestPortion->GetKind() != PORTIONKIND_TAB )
/*N*/     {
/*N*/         if ( !bR2LPara && pDestPortion->GetRightToLeft() )
/*N*/         {
/*?*/             // Portions behind must be added, visual before this portion
/*?*/             sal_uInt16 nTmpPortion = nTextPortion+1;
/*?*/             while ( nTmpPortion <= pLine->GetEndPortion() )
/*?*/             {
/*?*/ 		        TextPortion* pNextTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
/*?*/                 if ( pNextTextPortion->GetRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
/*?*/                     nX += pNextTextPortion->GetSize().Width();
/*?*/                 else
/*?*/                     break;
/*?*/                 nTmpPortion++;
/*?*/             }
/*?*/             // Portions before must be removed, visual behind this portion
/*?*/             nTmpPortion = nTextPortion;
/*?*/             while ( nTmpPortion > pLine->GetStartPortion() )
/*?*/             {
/*?*/                 --nTmpPortion;
/*?*/ 		        TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
/*?*/                 if ( pPrevTextPortion->GetRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
/*?*/                     nX -= pPrevTextPortion->GetSize().Width();
/*?*/                 else
/*?*/                     break;
/*?*/             }
/*N*/         }
/*N*/         else if ( bR2LPara && !pDestPortion->IsRightToLeft() )
/*N*/         {
/*?*/             // Portions behind must be ermoved, visual behind this portion
/*?*/             sal_uInt16 nTmpPortion = nTextPortion+1;
/*?*/             while ( nTmpPortion <= pLine->GetEndPortion() )
/*?*/             {
/*?*/ 		        TextPortion* pNextTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
/*?*/                 if ( !pNextTextPortion->IsRightToLeft() && ( pNextTextPortion->GetKind() != PORTIONKIND_TAB ) )
/*?*/                     nX += pNextTextPortion->GetSize().Width();
/*?*/                 else
/*?*/                     break;
/*?*/                 nTmpPortion++;
/*?*/             }
/*?*/             // Portions before must be added, visual before this portion
/*?*/             nTmpPortion = nTextPortion;
/*?*/             while ( nTmpPortion > pLine->GetStartPortion() )
/*?*/             {
/*?*/                 --nTmpPortion;
/*?*/ 		        TextPortion* pPrevTextPortion = pParaPortion->GetTextPortions().GetObject( nTmpPortion );
/*?*/                 if ( !pPrevTextPortion->IsRightToLeft() && ( pPrevTextPortion->GetKind() != PORTIONKIND_TAB ) )
/*?*/                     nX -= pPrevTextPortion->GetSize().Width();
/*?*/                 else
/*?*/                     break;
/*?*/             }
/*N*/         }
/*N*/     }
/*N*/     if ( bR2LPara )
/*N*/     {
/*?*/         // Switch X postions...
/*?*/         DBG_ASSERT( GetTextRanger() || GetPaperSize().Width(), "GetPortionXOffset - paper size?!" );
/*?*/         DBG_ASSERT( GetTextRanger() || (nX <= GetPaperSize().Width()), "GetPortionXOffset - position out of paper size!" );
/*?*/         nX = GetPaperSize().Width() - nX;
/*?*/         nX -= pDestPortion->GetSize().Width();
/*N*/     }
/*N*/ 
/*N*/     return nX;
/*N*/ }


/*N*/ void ImpEditEngine::CalcHeight( ParaPortion* pPortion )
/*N*/ {
/*N*/ 	pPortion->nHeight = 0;
/*N*/ 	pPortion->nFirstLineOffset = 0;
/*N*/ 
/*N*/ 	if ( pPortion->IsVisible() )
/*N*/ 	{
/*N*/ 		DBG_ASSERT( pPortion->GetLines().Count(), "Absatz ohne Zeilen in ParaPortion::CalcHeight" );
/*N*/ 		for ( USHORT nLine = 0; nLine < pPortion->GetLines().Count(); nLine++ )
/*N*/ 			pPortion->nHeight += pPortion->GetLines().GetObject( nLine )->GetHeight();
/*N*/ 
/*N*/ 		if ( !aStatus.IsOutliner() )
/*N*/ 		{
/*N*/ 			const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
/*N*/ 			const SvxLineSpacingItem& rLSItem = (const SvxLineSpacingItem&)pPortion->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
/*N*/ 			USHORT nSBL = ( rLSItem.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX ) ? GetYValue( rLSItem.GetInterLineSpace() ) : 0;
/*N*/ 
/*N*/ 			if ( nSBL )
/*N*/ 			{
/*?*/ 				if ( pPortion->GetLines().Count() > 1 )
/*?*/ 					pPortion->nHeight += ( pPortion->GetLines().Count() - 1 ) * nSBL;
/*?*/ 				if ( aStatus.ULSpaceSummation() )
/*?*/ 					pPortion->nHeight += nSBL;
/*N*/ 			}
/*N*/ 
/*N*/ 			USHORT nPortion = GetParaPortions().GetPos( pPortion );
/*N*/ 			if ( nPortion || aStatus.ULSpaceFirstParagraph() )
/*N*/ 			{
/*N*/ 				USHORT nUpper = GetYValue( rULItem.GetUpper() );
/*N*/ 				pPortion->nHeight += nUpper;
/*N*/ 				pPortion->nFirstLineOffset = nUpper;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( ( nPortion != (GetParaPortions().Count()-1) ) )
/*N*/ 			{
/*N*/ 				pPortion->nHeight += GetYValue( rULItem.GetLower() );	// nicht in letzter
/*N*/ 			}
/*N*/ 
/*N*/ 
/*N*/ 			if ( nPortion && !aStatus.ULSpaceSummation() )
/*N*/ 			{
/*N*/ 				ParaPortion* pPrev = GetParaPortions().SaveGetObject( nPortion-1 );
/*N*/ 				const SvxULSpaceItem& rPrevULItem = (const SvxULSpaceItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_ULSPACE );
/*N*/ 				const SvxLineSpacingItem& rPrevLSItem = (const SvxLineSpacingItem&)pPrev->GetNode()->GetContentAttribs().GetItem( EE_PARA_SBL );
/*N*/ 
/*N*/ 				// Verhalten WinWord6/Writer3:
/*N*/ 				// Bei einem proportionalen Zeilenabstand wird auch der Absatzabstand
/*N*/ 				// manipuliert.
/*N*/ 				// Nur Writer3: Nicht aufaddieren, sondern Mindestabstand.
/*N*/ 
/*N*/ 				// Pruefen, ob Abstand durch LineSpacing > Upper:
/*N*/ 				USHORT nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPortion, rLSItem ) );
/*N*/ 				if ( nExtraSpace > pPortion->nFirstLineOffset )
/*N*/ 				{
/*?*/ 					// Absatz wird 'groesser':
/*?*/ 					pPortion->nHeight += ( nExtraSpace - pPortion->nFirstLineOffset );
/*?*/ 					pPortion->nFirstLineOffset = nExtraSpace;
/*N*/ 				}
/*N*/ 
/*N*/ 				// nFirstLineOffset jetzt f(pNode) => jetzt f(pNode, pPrev) ermitteln:
/*N*/ 				USHORT nPrevLower = GetYValue( rPrevULItem.GetLower() );
/*N*/ 
/*N*/ 				// Dieser PrevLower steckt noch in der Hoehe der PrevPortion...
/*N*/ 				if ( nPrevLower > pPortion->nFirstLineOffset )
/*N*/ 				{
/*N*/ 					// Absatz wird 'kleiner':
/*N*/ 					pPortion->nHeight -= pPortion->nFirstLineOffset;
/*N*/ 					pPortion->nFirstLineOffset = 0;
/*N*/ 				}
/*N*/ 				else if ( nPrevLower )
/*N*/ 				{
/*?*/ 					// Absatz wird 'etwas kleiner':
/*?*/ 					pPortion->nHeight -= nPrevLower;
/*?*/ 					pPortion->nFirstLineOffset -= nPrevLower;
/*N*/ 				}
/*N*/ 
/*N*/ 				// Finde ich zwar nicht so gut, aber Writer3-Feature:
/*N*/ 				// Pruefen, ob Abstand durch LineSpacing > Lower:
/*N*/ 				// Dieser Wert steckt nicht in der Hoehe der PrevPortion.
/*N*/ 				if ( !pPrev->IsInvalid() )
/*N*/ 				{
/*N*/ 					nExtraSpace = GetYValue( lcl_CalcExtraSpace( pPrev, rPrevLSItem ) );
/*N*/ 					if ( nExtraSpace > nPrevLower )
/*N*/ 					{
/*?*/ 						USHORT nMoreLower = nExtraSpace - nPrevLower;
/*?*/ 						// Absatz wird 'groesser', 'waechst' nach unten:
/*?*/ 						if ( nMoreLower > pPortion->nFirstLineOffset )
/*?*/ 						{
/*?*/ 							pPortion->nHeight += ( nMoreLower - pPortion->nFirstLineOffset );
/*?*/ 							pPortion->nFirstLineOffset = nMoreLower;
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ImpEditEngine::SetValidPaperSize( const Size& rNewSz )
/*N*/ {
/*N*/ 	aPaperSize = rNewSz;
/*N*/ 
/*N*/ 	long nMinWidth = aStatus.AutoPageWidth() ? aMinAutoPaperSize.Width() : 0;
/*N*/ 	long nMaxWidth = aStatus.AutoPageWidth() ? aMaxAutoPaperSize.Width() : 0x7FFFFFFF;
/*N*/ 	long nMinHeight = aStatus.AutoPageHeight() ? aMinAutoPaperSize.Height() : 0;
/*N*/ 	long nMaxHeight = aStatus.AutoPageHeight() ? aMaxAutoPaperSize.Height() : 0x7FFFFFFF;
/*N*/ 
/*N*/ 	// Minimale/Maximale Breite:
/*N*/ 	if ( aPaperSize.Width() < nMinWidth )
/*N*/ 		aPaperSize.Width() = nMinWidth;
/*N*/ 	else if ( aPaperSize.Width() > nMaxWidth )
/*?*/ 		aPaperSize.Width() = nMaxWidth;
/*N*/ 
/*N*/ 	// Minimale/Maximale Hoehe:
/*N*/ 	if ( aPaperSize.Height() < nMinHeight )
/*?*/ 		aPaperSize.Height() = nMinHeight;
/*N*/ 	else if ( aPaperSize.Height() > nMaxHeight )
/*?*/ 		aPaperSize.Height() = nMaxHeight;
/*N*/ }


/*N*/ rtl::Reference<SvxForbiddenCharactersTable> ImpEditEngine::GetForbiddenCharsTable( BOOL bGetInternal ) const
/*N*/ {
/*N*/ 	rtl::Reference<SvxForbiddenCharactersTable> xF = xForbiddenCharsTable;
/*N*/ 	if ( !xF.is() && bGetInternal )
/*N*/ 		xF = EE_DLL()->GetGlobalData()->GetForbiddenCharsTable();
/*N*/ 	return xF;
/*N*/ }

/*N*/ void ImpEditEngine::SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars )
/*N*/ {
/*N*/ 	EE_DLL()->GetGlobalData()->SetForbiddenCharsTable( xForbiddenChars );
/*N*/ }

        ColorConfig& ImpEditEngine::GetColorConfig()
/*N*/ {
/*N*/     if ( !pColorConfig )
/*N*/         pColorConfig = new ColorConfig;
/*N*/ 
/*N*/     return *pColorConfig;
/*N*/ }





/*N*/ void ImpEditEngine::EnterBlockNotifications()
/*N*/ {
/*N*/     if( !nBlockNotifications )
/*N*/     {
/*N*/         // #109864# Send out START notification immediately, to allow
/*N*/         // external, non-queued events to be captured as well from
/*N*/         // client side
/*N*/         EENotify aNotify( EE_NOTIFY_BLOCKNOTIFICATION_START );
/*N*/         aNotify.pEditEngine = GetEditEnginePtr();
/*N*/         GetNotifyHdl().Call( &aNotify );
/*N*/     }
/*N*/ 
/*N*/     nBlockNotifications++;
/*N*/ }

/*N*/ void ImpEditEngine::LeaveBlockNotifications()
/*N*/ {
/*N*/     DBG_ASSERT( nBlockNotifications, "LeaveBlockNotifications - Why?" );
/*N*/ 
/*N*/     nBlockNotifications--;
/*N*/     if ( !nBlockNotifications )
/*N*/     {
/*N*/         // Call blocked notify events...
/*N*/         while ( aNotifyCache.Count() )
/*N*/         {
/*N*/             EENotify* pNotify = aNotifyCache[0];
/*N*/             // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
/*N*/             aNotifyCache.Remove( 0 );
/*N*/             GetNotifyHdl().Call( pNotify );
/*N*/             delete pNotify;
/*N*/         }
/*N*/ 
/*N*/         EENotify aNotify( EE_NOTIFY_BLOCKNOTIFICATION_END );
/*N*/         aNotify.pEditEngine = GetEditEnginePtr();
/*N*/         GetNotifyHdl().Call( &aNotify );
/*N*/     }
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
