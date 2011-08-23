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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svtools/colorcfg.hxx>
#include <bf_svx/editstat.hxx>


#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _SVX_ITEMDATA_HXX
#include <bf_svx/itemdata.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#include <bf_svx/flditem.hxx>
//#include <vcl/system.hxx>
#include <bf_svtools/inethist.hxx>
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <bf_svtools/syslocale.hxx>
#endif
#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#endif

#include "editutil.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

//	Delimiters zusaetzlich zu EditEngine-Default:

const sal_Char __FAR_DATA ScEditUtil::pCalcDelimiters[] = "=();+-*/^&<>";


//------------------------------------------------------------------------

/*N*/ String ScEditUtil::ModifyDelimiters( const String& rOld )
/*N*/ {
/*N*/ 	String aRet = rOld;
/*N*/ 	aRet.EraseAllChars( '_' );	// underscore is used in function argument names
/*N*/ 	aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( pCalcDelimiters ) );
/*N*/ 	return aRet;
/*N*/ }

/*N*/ String ScEditUtil::GetSpaceDelimitedString( const EditEngine& rEngine )
/*N*/ {
/*N*/ 	String aRet;
/*N*/ 	USHORT nParCount = rEngine.GetParagraphCount();
/*N*/ 	for (USHORT nPar=0; nPar<nParCount; nPar++)
/*N*/ 	{
/*N*/ 		if (nPar > 0)
/*N*/ 			aRet += ' ';
/*N*/ 		aRet += rEngine.GetText( nPar );
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ ScEditAttrTester::ScEditAttrTester( EditEngine* pEng ) :
/*N*/ 	pEngine( pEng ),
/*N*/ 	pEditAttrs( NULL ),
/*N*/ 	bNeedsObject( FALSE ),
/*N*/ 	bNeedsCellAttr( FALSE )
/*N*/ {
/*N*/ 	if ( pEngine->GetParagraphCount() > 1 )
/*N*/ 	{
/*N*/ 		bNeedsObject = TRUE;			//!	Zellatribute finden ?
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem = NULL;
/*N*/ 		pEditAttrs = new SfxItemSet( pEngine->GetAttribs(
/*N*/ 										ESelection(0,0,0,pEngine->GetTextLen(0)) ) );
/*N*/ 		const SfxItemPool* pEditPool = pEditAttrs->GetPool();
/*N*/ 
/*N*/ 		for (USHORT nId = EE_CHAR_START; nId <= EE_CHAR_END && !bNeedsObject; nId++)
/*N*/ 		{
/*N*/ 			SfxItemState eState = pEditAttrs->GetItemState( nId, FALSE, &pItem );
/*N*/ 			if (eState == SFX_ITEM_DONTCARE)
/*N*/ 				bNeedsObject = TRUE;
/*N*/ 			else if (eState == SFX_ITEM_SET)
/*N*/ 			{
/*N*/ 				if ( nId == EE_CHAR_ESCAPEMENT || nId == EE_CHAR_PAIRKERNING ||
/*N*/ 						nId == EE_CHAR_KERNING || nId == EE_CHAR_XMLATTRIBS )
/*N*/ 				{
/*N*/ 					//	Escapement and kerning are kept in EditEngine because there are no
/*N*/ 					//	corresponding cell format items. User defined attributes are kept in
/*N*/ 					//	EditEngine because "user attributes applied to all the text" is different
/*N*/ 					//	from "user attributes applied to the cell".
/*N*/ 
/*N*/ 					if ( *pItem != pEditPool->GetDefaultItem(nId) )
/*N*/ 						bNeedsObject = TRUE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					if (!bNeedsCellAttr)
/*N*/ 						if ( *pItem != pEditPool->GetDefaultItem(nId) )
/*N*/ 							bNeedsCellAttr = TRUE;
/*N*/ 				//	SetDefaults an der EditEngine setzt Pool-Defaults
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//	Feldbefehle enthalten?
/*N*/ 
/*N*/ 		SfxItemState eFieldState = pEditAttrs->GetItemState( EE_FEATURE_FIELD, FALSE );
/*N*/ 		if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
/*N*/ 			bNeedsObject = TRUE;
/*N*/ 
/*N*/ 		//	not converted characters?
/*N*/ 
/*N*/ 		SfxItemState eConvState = pEditAttrs->GetItemState( EE_FEATURE_NOTCONV, FALSE );
/*N*/ 		if ( eConvState == SFX_ITEM_DONTCARE || eConvState == SFX_ITEM_SET )
/*N*/ 			bNeedsObject = TRUE;
/*N*/ 	}
/*N*/ }

/*N*/ ScEditAttrTester::~ScEditAttrTester()
/*N*/ {
/*N*/ 	delete pEditAttrs;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ ScEnginePoolHelper::ScEnginePoolHelper( SfxItemPool* pEnginePoolP,
/*N*/ 				BOOL bDeleteEnginePoolP )
/*N*/ 			:
/*N*/ 			pEnginePool( pEnginePoolP ),
/*N*/ 			bDeleteEnginePool( bDeleteEnginePoolP ),
/*N*/ 			pDefaults( NULL ),
/*N*/ 			bDeleteDefaults( FALSE )
/*N*/ {
/*N*/ }


/*N*/ ScEnginePoolHelper::ScEnginePoolHelper( const ScEnginePoolHelper& rOrg )
/*N*/ 			:
/*N*/ 			pEnginePool( rOrg.bDeleteEnginePool ? rOrg.pEnginePool->Clone() : rOrg.pEnginePool ),
/*N*/ 			bDeleteEnginePool( rOrg.bDeleteEnginePool ),
/*N*/ 			pDefaults( NULL ),
/*N*/ 			bDeleteDefaults( FALSE )
/*N*/ {
/*N*/ }


/*N*/ ScEnginePoolHelper::~ScEnginePoolHelper()
/*N*/ {
/*N*/ 	if ( bDeleteDefaults )
/*N*/ 		delete pDefaults;
/*N*/ 	if ( bDeleteEnginePool )
/*N*/ 		delete pEnginePool;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ ScEditEngineDefaulter::ScEditEngineDefaulter( SfxItemPool* pEnginePoolP,
/*N*/ 				BOOL bDeleteEnginePoolP )
/*N*/ 			:
/*N*/ 			ScEnginePoolHelper( pEnginePoolP, bDeleteEnginePoolP ),
/*N*/ 			EditEngine( pEnginePoolP )
/*N*/ {
/*N*/ 	//	All EditEngines use ScGlobal::GetEditDefaultLanguage as DefaultLanguage.
/*N*/ 	//	DefaultLanguage for InputHandler's EditEngine is updated later.
/*N*/ 
/*N*/ 	SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );
/*N*/ }


/*N*/ ScEditEngineDefaulter::ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg )
/*N*/ 			:
/*N*/ 			ScEnginePoolHelper( rOrg ),
/*N*/ 			EditEngine( pEnginePool )
/*N*/ {
/*N*/ 	SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );
/*N*/ }


/*N*/ ScEditEngineDefaulter::~ScEditEngineDefaulter()
/*N*/ {
/*N*/ }


/*N*/ void ScEditEngineDefaulter::SetDefaults( const SfxItemSet& rSet, BOOL bRememberCopy )
/*N*/ {
/*N*/ 	if ( bRememberCopy )
/*N*/ 	{
/*N*/ 		if ( bDeleteDefaults )
/*N*/ 			delete pDefaults;
/*N*/ 		pDefaults = new SfxItemSet( rSet );
/*N*/ 		bDeleteDefaults = TRUE;
/*N*/ 	}
/*N*/ 	const SfxItemSet& rNewSet = bRememberCopy ? *pDefaults : rSet;
/*N*/ 	BOOL bUndo = IsUndoEnabled();
/*N*/ 	EnableUndo( FALSE );
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( FALSE );
/*N*/ 	USHORT nPara = GetParagraphCount();
/*N*/ 	for ( USHORT j=0; j<nPara; j++ )
/*N*/ 	{
/*N*/ 		SetParaAttribs( j, rNewSet );
/*N*/ 	}
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( TRUE );
/*N*/ 	if ( bUndo )
/*N*/ 		EnableUndo( TRUE );
/*N*/ }


/*N*/ void ScEditEngineDefaulter::SetDefaults( SfxItemSet* pSet, BOOL bTakeOwnership )
/*N*/ {
/*N*/ 	if ( bDeleteDefaults )
/*N*/ 		delete pDefaults;
/*N*/ 	pDefaults = pSet;
/*N*/ 	bDeleteDefaults = bTakeOwnership;
/*N*/ 	if ( pDefaults )
/*N*/ 		SetDefaults( *pDefaults, FALSE );
/*N*/ }




/*N*/ void ScEditEngineDefaulter::SetText( const EditTextObject& rTextObject )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rTextObject );
/*N*/ 	if ( pDefaults )
/*N*/ 		SetDefaults( *pDefaults, FALSE );
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( TRUE );
/*N*/ }

/*N*/ void ScEditEngineDefaulter::SetTextNewDefaults( const EditTextObject& rTextObject,
/*N*/ 			const SfxItemSet& rSet, BOOL bRememberCopy )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rTextObject );
/*N*/ 	SetDefaults( rSet, bRememberCopy );
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( TRUE );
/*N*/ }

/*N*/ void ScEditEngineDefaulter::SetTextNewDefaults( const EditTextObject& rTextObject,
/*N*/ 			SfxItemSet* pSet, BOOL bTakeOwnership )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*?*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rTextObject );
/*N*/ 	SetDefaults( pSet, bTakeOwnership );
/*N*/ 	if ( bUpdateMode )
/*?*/ 		SetUpdateMode( TRUE );
/*N*/ }


/*N*/ void ScEditEngineDefaulter::SetText( const String& rText )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rText );
/*N*/ 	if ( pDefaults )
/*?*/ 		SetDefaults( *pDefaults, FALSE );
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( TRUE );
/*N*/ }

/*N*/ void ScEditEngineDefaulter::SetTextNewDefaults( const String& rText,
/*N*/ 			const SfxItemSet& rSet, BOOL bRememberCopy )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rText );
/*N*/ 	SetDefaults( rSet, bRememberCopy );
/*N*/ 	if ( bUpdateMode )
/*N*/ 		SetUpdateMode( TRUE );
/*N*/ }

/*N*/ void ScEditEngineDefaulter::SetTextNewDefaults( const String& rText,
/*N*/ 			SfxItemSet* pSet, BOOL bTakeOwnership )
/*N*/ {
/*N*/ 	BOOL bUpdateMode = GetUpdateMode();
/*N*/ 	if ( bUpdateMode )
/*?*/ 		SetUpdateMode( FALSE );
/*N*/ 	EditEngine::SetText( rText );
/*N*/ 	SetDefaults( pSet, bTakeOwnership );
/*N*/ 	if ( bUpdateMode )
/*?*/ 		SetUpdateMode( TRUE );
/*N*/ }



//------------------------------------------------------------------------




//------------------------------------------------------------------------
//		Feldbefehle fuer Kopf- und Fusszeilen
//------------------------------------------------------------------------

//
//		Zahlen aus \sw\source\core\doc\numbers.cxx
//



/*N*/ String lcl_GetNumStr( USHORT nNo, SvxNumType eType )
/*N*/ {
/*N*/ 	String aTmpStr( '0' );
/*N*/ 	if( nNo )
/*N*/ 	{
/*N*/ 		switch( eType )
/*N*/ 		{
/*?*/ 		case SVX_CHARS_UPPER_LETTER:
/*?*/ 		case SVX_CHARS_LOWER_LETTER:
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aTmpStr = lcl_GetCharStr( nNo );
/*?*/ 			break;
/*?*/ 
/*?*/ 		case SVX_ROMAN_UPPER:
/*?*/ 		case SVX_ROMAN_LOWER:
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aTmpStr = lcl_GetRomanStr( nNo );
/*?*/ 			break;
/*?*/ 
/*?*/ 		case SVX_NUMBER_NONE:
/*?*/ 			aTmpStr.Erase();
/*?*/ 			break;
/*?*/ 
/*?*/ //		CHAR_SPECIAL:
/*?*/ //			????
/*?*/ 
/*?*/ //		case ARABIC:	ist jetzt default
/*N*/ 		default:
/*N*/ 			aTmpStr = String::CreateFromInt32( nNo );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( SVX_CHARS_UPPER_LETTER == eType || SVX_ROMAN_UPPER == eType )
/*N*/ 			aTmpStr.ToUpperAscii();
/*N*/ 	}
/*N*/ 	return aTmpStr;
/*N*/ }

/*N*/ ScHeaderFieldData::ScHeaderFieldData()
/*N*/ {
/*N*/ 	nPageNo = nTotalPages = 0;
/*N*/ 	eNumType = SVX_ARABIC;
/*N*/ }

/*N*/ ScHeaderEditEngine::ScHeaderEditEngine( SfxItemPool* pEnginePool, BOOL bDeleteEnginePool )
/*N*/ 		: ScEditEngineDefaulter( pEnginePool, bDeleteEnginePool )
/*N*/ {
/*N*/ }

/*N*/ String __EXPORT ScHeaderEditEngine::CalcFieldValue( const SvxFieldItem& rField,
/*N*/ 									USHORT nPara, USHORT nPos,
/*N*/ 									Color*& rTxtColor, Color*& rFldColor )
/*N*/ {
/*N*/ 	String aRet;
/*N*/ 	const SvxFieldData*	pFieldData = rField.GetField();
/*N*/ 	if ( pFieldData )
/*N*/ 	{
/*N*/ 		TypeId aType = pFieldData->Type();
/*N*/ 		if (aType == TYPE(SvxPageField))
/*N*/ 			aRet = lcl_GetNumStr( (USHORT)aData.nPageNo,aData.eNumType );
/*N*/ 		else if (aType == TYPE(SvxPagesField))
/*?*/ 			aRet = lcl_GetNumStr( (USHORT)aData.nTotalPages,aData.eNumType );
/*N*/ 		else if (aType == TYPE(SvxTimeField))
/*N*/             aRet = ScGlobal::pLocaleData->getTime(aData.aTime);
/*N*/ 		else if (aType == TYPE(SvxFileField))
/*N*/ 			aRet = aData.aTitle;
/*N*/ 		else if (aType == TYPE(SvxExtFileField))
/*N*/ 		{
/*?*/ 			switch ( ((const SvxExtFileField*)pFieldData)->GetFormat() )
/*?*/ 			{
/*?*/ 				case SVXFILEFORMAT_FULLPATH :
/*?*/ 					aRet = aData.aLongDocName;
/*?*/ 				break;
/*?*/ 				default:
/*?*/ 					aRet = aData.aShortDocName;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if (aType == TYPE(SvxTableField))
/*N*/ 			aRet = aData.aTabName;
/*N*/ 		else if (aType == TYPE(SvxDateField))
/*N*/             aRet = ScGlobal::pLocaleData->getDate(aData.aDate);
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//DBG_ERROR("unbekannter Feldbefehl");
/*N*/ 			aRet = '?';
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
            // #i75599# no assertion - can happen with old files
/*N*/       //DBG_ERROR("FieldData ist 0");
/*N*/ 		aRet = '?';
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }

//------------------------------------------------------------------------
//
//							Feld-Daten
//
//------------------------------------------------------------------------

/*N*/ ScFieldEditEngine::ScFieldEditEngine( SfxItemPool* pEnginePool,
/*N*/ 			SfxItemPool* pTextObjectPool, BOOL bDeleteEnginePool )
/*N*/ 		:
/*N*/ 		ScEditEngineDefaulter( pEnginePool, bDeleteEnginePool ),
/*N*/ 		bExecuteURL( TRUE )
/*N*/ {
/*N*/ 	if ( pTextObjectPool )
/*N*/ 		SetEditTextObjectPool( pTextObjectPool );
/*N*/ 	//	EE_CNTRL_URLSFXEXECUTE nicht, weil die Edit-Engine den ViewFrame nicht kennt
/*N*/ 	// wir haben keine StyleSheets fuer Text
/*N*/ 	SetControlWord( (GetControlWord() | EE_CNTRL_MARKFIELDS) & ~EE_CNTRL_RTFSTYLESHEETS );
/*N*/ }

/*N*/ String __EXPORT ScFieldEditEngine::CalcFieldValue( const SvxFieldItem& rField,
/*N*/ 									USHORT nPara, USHORT nPos,
/*N*/ 									Color*& rTxtColor, Color*& rFldColor )
/*N*/ {
/*N*/ 	String aRet;
/*N*/ 	const SvxFieldData*	pFieldData = rField.GetField();
/*N*/ 
/*N*/ 	if ( pFieldData )
/*N*/ 	{
/*N*/ 		TypeId aType = pFieldData->Type();
/*N*/ 
/*N*/ 		if (aType == TYPE(SvxURLField))
/*N*/ 		{
/*N*/ 			String aURL = ((const SvxURLField*)pFieldData)->GetURL();
/*N*/ 
/*N*/ 			switch ( ((const SvxURLField*)pFieldData)->GetFormat() )
/*N*/ 			{
/*N*/ 				case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
/*N*/ 				case SVXURLFORMAT_REPR:
/*N*/ 					aRet = ((const SvxURLField*)pFieldData)->GetRepresentation();
/*N*/ 					break;
/*N*/ 
/*N*/ 				case SVXURLFORMAT_URL:
/*N*/ 					aRet = aURL;
/*N*/ 					break;
/*N*/ 			}
/*N*/ 
                    ColorConfigEntry eEntry = INetURLHistory::GetOrCreate()->QueryUrl( aURL ) ? LINKSVISITED : LINKS;
/*N*/ 			rTxtColor = new Color( SC_MOD()->GetColorConfig().GetColorValue(eEntry).nColor );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//DBG_ERROR("unbekannter Feldbefehl");
/*N*/ 			aRet = '?';
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!aRet.Len()) 		// leer ist baeh
/*N*/ 		aRet = ' ';			// Space ist Default der Editengine
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }


}
