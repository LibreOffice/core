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

#include <tools/resid.hxx>
#include <txtatr.hxx>
#include <ndtxt.hxx>
#include <tox.hxx>
#include <poolfmt.hrc>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <bf_svx/tstpitem.hxx>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

/*N*/ const sal_Char* SwForm::aFormEntry 		= "<E>";
/*N*/ const sal_Char* SwForm::aFormTab 		= "<T>";
/*N*/ const sal_Char* SwForm::aFormPageNums 	= "<#>";
/*N*/ const sal_Char* SwForm::aFormLinkStt 	= "<LS>";
/*N*/ const sal_Char* SwForm::aFormLinkEnd 	= "<LE>";
/*N*/ const sal_Char*	SwForm::aFormEntryNum	= "<E#>";
/*N*/ const sal_Char*	SwForm::aFormEntryTxt	= "<ET>";
/*N*/ const sal_Char*	SwForm::aFormChapterMark= "<C>";
/*N*/ const sal_Char*	SwForm::aFormText		= "<X>";
/*N*/ const sal_Char*	SwForm::aFormAuth		= "<A>";
/*N*/ BYTE SwForm::nFormTabLen			= 3;
/*N*/ BYTE SwForm::nFormEntryLen			= 3;
/*N*/ BYTE SwForm::nFormPageNumsLen		= 3;
/*N*/ BYTE SwForm::nFormLinkSttLen		= 4;
/*N*/ BYTE SwForm::nFormLinkEndLen		= 4;
/*N*/ BYTE SwForm::nFormEntryNumLen		= 4;
/*N*/ BYTE SwForm::nFormEntryTxtLen		= 4;
/*N*/ BYTE SwForm::nFormChapterMarkLen	= 3;
/*N*/ BYTE SwForm::nFormTextLen			= 3;
/*N*/ BYTE SwForm::nFormAuthLen			= 5;



/*N*/ TYPEINIT2( SwTOXMark, SfxPoolItem, SwClient );    // fuers rtti

/* -----------------23.09.99 14:09-------------------
    includes plain text at a given position into
     the appropriate token
 --------------------------------------------------*/
USHORT lcl_ConvertTextIntoPattern( String& rPattern,
                                    xub_StrLen nStart, xub_StrLen nEnd )
{
    String sTmp; sTmp.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<X ,,"));
    sTmp += TOX_STYLE_DELIMITER;
    sTmp += rPattern.Copy( nStart, nEnd - nStart);
    rPattern.Erase( nStart, nEnd - nStart);
    sTmp += TOX_STYLE_DELIMITER;
    sTmp += '>';
    rPattern.Insert(sTmp, nStart);
    // return the offset of the new end position
    return 8;
}

/* -----------------23.09.99 13:59-------------------

 --------------------------------------------------*/
struct PatternIni
{
    USHORT n1;
    USHORT n2;
    USHORT n3;
    USHORT n4;
    USHORT n5;
};
const PatternIni aPatternIni[] =
{
    {USHRT_MAX},    //Header - no pattern
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},   //AUTH_TYPE_ARTICLE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOKLET,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CONFERENCE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INBOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INCOLLECTION,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INPROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_JOURNAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MANUAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MASTERSTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MISC,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PHDTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_TECHREPORT,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_UNPUBLISHED,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_EMAIL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, AUTH_FIELD_URL, USHRT_MAX},//AUTH_TYPE_WWW,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM5,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_YEAR,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_URL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM5,
    {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX}
};
/* -----------------23.09.99 13:58-------------------

 --------------------------------------------------*/
String lcl_GetAuthPattern(USHORT nTypeId)
{
    String sAuth; sAuth.AssignAscii( SwForm::aFormAuth );
    sAuth.Insert(' ', 2);
    //
    PatternIni aIni = aPatternIni[nTypeId];

    //TODO: create pattern for authorities
    // create a level for each type of authorities
    String sRet = sAuth;
    String sTmp( String::CreateFromInt32( AUTH_FIELD_IDENTIFIER ));
    if(sTmp.Len() < 2)
        sTmp.Insert('0', 0);
    sRet.Insert(sTmp, 2);
    sRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ));
    lcl_ConvertTextIntoPattern(sRet, sRet.Len() - 2, sRet.Len());
    USHORT nVals[5];
    nVals[0] = aIni.n1;
    nVals[1] = aIni.n2;
    nVals[2] = aIni.n3;
    nVals[3] = aIni.n4;
    nVals[4] = aIni.n5;

    for(USHORT i = 0; i < 5 ; i++)
    {
        if(nVals[i] == USHRT_MAX)
            break;
        if(i > 0)
        {
            sRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ));
            lcl_ConvertTextIntoPattern(sRet, sRet.Len() - 2, sRet.Len());
        }
        sRet += sAuth;
        sTmp = String::CreateFromInt32( nVals[ i ] );
        if(sTmp.Len() < 2)
            sTmp.Insert('0', 0);
        sRet.Insert(sTmp, sRet.Len() - 2);
    }
    return sRet;
}
/*--------------------------------------------------------------------
     Beschreibung:	Verzeichnis-Markierungen D/Ctor
 --------------------------------------------------------------------*/


    // Konstruktor fuers Default vom Attribut-Pool
/*N*/ SwTOXMark::SwTOXMark()
/*N*/ 	: SfxPoolItem( RES_TXTATR_TOXMARK ),
/*N*/ 	SwClient( 0 ),
/*N*/ 	pTxtAttr( 0 ),
/*N*/ 	bAutoGenerated(FALSE),
/*N*/ 	bMainEntry(FALSE)
/*N*/ {
/*N*/ }


/*N*/ SwTOXMark::SwTOXMark( const SwTOXType* pTyp )
/*N*/ 	: SfxPoolItem( RES_TXTATR_TOXMARK ),
/*N*/ 	SwClient( (SwModify*)pTyp ),
/*N*/ 	pTxtAttr( 0 ), nLevel( 0 ),
/*N*/ 	bAutoGenerated(FALSE),
/*N*/ 	bMainEntry(FALSE)
/*N*/ {
/*N*/ }


/*N*/ SwTOXMark::SwTOXMark( const SwTOXMark& rCopy )
/*N*/ 	: SfxPoolItem( RES_TXTATR_TOXMARK ),
/*N*/ 	SwClient((SwModify*)rCopy.GetRegisteredIn()),
/*N*/ 	pTxtAttr( 0 ), nLevel( rCopy.nLevel ),
/*N*/ 	aPrimaryKey( rCopy.aPrimaryKey ), aSecondaryKey( rCopy.aSecondaryKey ),
/*N*/     aTextReading( rCopy.aTextReading ),
/*N*/     aPrimaryKeyReading( rCopy.aPrimaryKeyReading ),
/*N*/     aSecondaryKeyReading( rCopy.aSecondaryKeyReading ),
/*N*/ 	bAutoGenerated(	rCopy.bAutoGenerated),
/*N*/ 	bMainEntry(rCopy.bMainEntry)
/*N*/ {
/*N*/ 	// AlternativString kopieren
/*N*/ 	aAltText = rCopy.aAltText;
/*N*/ }


/*N*/ SwTOXMark::~SwTOXMark()
/*N*/ {
/*N*/ }


/*N*/ int SwTOXMark::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
/*N*/ 	return GetRegisteredIn() == ((SwTOXMark&)rAttr).GetRegisteredIn();
/*N*/ }


/*N*/ SfxPoolItem* SwTOXMark::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwTOXMark( *this );
/*N*/ }



/*--------------------------------------------------------------------
     Beschreibung: Typen von Verzeichnissen verwalten
 --------------------------------------------------------------------*/

/*N*/ SwTOXType::SwTOXType( TOXTypes eTyp, const String& rName )
/*N*/ 	: SwModify(0),
/*N*/ 	aName(rName),
/*N*/ 	eType(eTyp)
/*N*/ {
/*N*/ }


/*N*/ SwTOXType::SwTOXType(const SwTOXType& rCopy)
/*N*/ 	: SwModify( (SwModify*)rCopy.GetRegisteredIn() ),
/*N*/ 	aName(rCopy.aName),
/*N*/ 	eType(rCopy.eType)
/*N*/ {
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Formen bearbeiten
  --------------------------------------------------------------------*/


/*N*/ SwForm::SwForm( USHORT nTyp )
/*N*/ 	: nType( nTyp ), nFormMaxLevel( SwForm::GetFormMaxLevel( nTyp )),
/*N*/ //	nFirstTabPos( lNumIndent ),
/*N*/ 	bCommaSeparated(FALSE)
/*N*/ {
/*N*/ 	//bHasFirstTabPos =
/*N*/ 	bGenerateTabPos = FALSE;
/*N*/ 	bIsRelTabPos = TRUE;
/*N*/ 
/*N*/ 	// Inhaltsverzeichnis hat entsprechend Anzahl Headlines + Ueberschrift
/*N*/ 	// Benutzer hat 10 Ebenen + Ueberschrift
/*N*/ 	// Stichwort hat 3 Ebenen + Ueberschrift + Trenner
/*N*/ 	// indexes of tables, objects illustrations and authorities consist of a heading and one level
/*N*/ 
/*N*/ 	USHORT nPoolId;
/*N*/ 	switch( nType )
/*N*/ 	{
/*N*/ 	case TOX_INDEX:			nPoolId = STR_POOLCOLL_TOX_IDXH;	break;
/*N*/ 	case TOX_USER:			nPoolId = STR_POOLCOLL_TOX_USERH;   break;
/*N*/ 	case TOX_CONTENT:		nPoolId = STR_POOLCOLL_TOX_CNTNTH;  break;
/*?*/ 	case TOX_ILLUSTRATIONS:	nPoolId = STR_POOLCOLL_TOX_ILLUSH;	break;
/*?*/ 	case TOX_OBJECTS      : nPoolId = STR_POOLCOLL_TOX_OBJECTH;	break;
/*?*/ 	case TOX_TABLES       : nPoolId = STR_POOLCOLL_TOX_TABLESH;	break;
/*?*/ 	case TOX_AUTHORITIES  : nPoolId = STR_POOLCOLL_TOX_AUTHORITIESH;	break;
/*?*/ 	default:
/*?*/ 		ASSERT( !this, "ungueltiger TOXTyp");
/*?*/ 		return ;
/*N*/ 	}
/*N*/ 
/*N*/ 	String sStr;
/*N*/ 	{
/*N*/ 		ByteString sBStr;
/*N*/ 		if( TOX_CONTENT == nType )
/*N*/ 			( sBStr = SwForm::aFormEntryNum ) += SwForm::aFormEntryTxt;
/*N*/ 		else
/*N*/ 			sBStr = SwForm::aFormEntry;
/*N*/ 		if( TOX_AUTHORITIES != nType)
/*N*/ 		{
/*N*/ 			sBStr += SwForm::aFormTab;
/*N*/ 			ByteString sTmp( RTL_CONSTASCII_STRINGPARAM( " ,65535,0," ));
/*N*/ 			if(TOX_CONTENT == nType)
/*N*/ 			{
/*N*/ 				//the most right tab stop is "most_right_aligned"
/*N*/ 				sTmp += ByteString::CreateFromInt32( SVX_TAB_ADJUST_END );
/*N*/ 				//and has a dot as FillChar
/*N*/ 				sTmp.Append( RTL_CONSTASCII_STRINGPARAM( ",." ));
/*N*/ 				sBStr.Insert(sTmp, sBStr.Len() - 1);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				sTmp += ByteString::CreateFromInt32( SVX_TAB_ADJUST_LEFT );
/*N*/ 				//and has a space as FillChar
/*N*/ 				sTmp.Append( RTL_CONSTASCII_STRINGPARAM( ", " ));
/*N*/ 			}
/*N*/ 			sBStr.Insert( sTmp, sBStr.Len() - 1);
/*N*/ 			sBStr += SwForm::aFormPageNums;
/*N*/ 		}
/*N*/ 		sStr.AppendAscii( sBStr.GetBuffer(), sBStr.Len() );
/*N*/ 	}
/*N*/ 
/*N*/ 	SetTemplate( 0, SW_RESSTR( nPoolId++ ));
/*N*/ 
/*N*/ 	if(TOX_INDEX == nType)
/*N*/ 	{
/*N*/ 		for( USHORT i = 1; i < 5; ++i  )
/*N*/ 		{
/*N*/ 			if(1 == i)
/*N*/ 			{
/*N*/ 				String sTmp; sTmp.AssignAscii( SwForm::aFormEntry );
/*N*/ 				sTmp.Insert(' ', SwForm::nFormEntryLen - 1);
/*N*/ 				SetPattern( i, sTmp );
/*N*/ 				SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDXBREAK    ));
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SetPattern( i, sStr );
/*N*/ 				SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDX1 + i - 2 ));
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		for( USHORT i = 1; i < GetFormMax(); ++i, ++nPoolId )    // Nr 0 ist der Titel
/*N*/ 		{
/*N*/ 			if(TOX_AUTHORITIES == nType)
/*?*/           SetPattern(i, lcl_GetAuthPattern(i));
/*N*/ 			else
/*N*/ 				SetPattern( i, sStr );
/*N*/ 
/*N*/ 			if( TOX_CONTENT == nType && 6 == i )
/*N*/ 				nPoolId = STR_POOLCOLL_TOX_CNTNT6;
/*N*/ 			else if( TOX_USER == nType && 6 == i )
/*N*/ 				nPoolId = STR_POOLCOLL_TOX_USER6;
/*N*/ 			else if( TOX_AUTHORITIES == nType )
/*?*/ 				nPoolId = STR_POOLCOLL_TOX_AUTHORITIES1;
/*N*/ 			SetTemplate( i, SW_RESSTR( nPoolId ) );
/*N*/ 		}
/*N*/ }


/*N*/ SwForm::SwForm(const SwForm& rForm)
/*N*/ 	: nType( rForm.nType )
/*N*/ {
/*N*/ 	*this = rForm;
/*N*/ }


/*N*/ SwForm& SwForm::operator=(const SwForm& rForm)
/*N*/ {
/*N*/ 	nType = rForm.nType;
/*N*/ 	nFormMaxLevel = rForm.nFormMaxLevel;
/*N*/ //	nFirstTabPos = rForm.nFirstTabPos;
/*N*/ //	bHasFirstTabPos = rForm.bHasFirstTabPos;
/*N*/ 	bGenerateTabPos = rForm.bGenerateTabPos;
/*N*/ 	bIsRelTabPos = rForm.bIsRelTabPos;
/*N*/ 	bCommaSeparated = rForm.bCommaSeparated;
/*N*/ 	for(USHORT i=0; i < nFormMaxLevel; ++i)
/*N*/ 	{
/*N*/ 		aPattern[i] = rForm.aPattern[i];
/*N*/ 		aTemplate[i] = rForm.aTemplate[i];
/*N*/ 	}
/*N*/ 	return *this;
/*N*/ }

/*N*/ USHORT SwForm::GetFormMaxLevel( USHORT nType )
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	switch( nType )
/*N*/ 	{
/*N*/ 		case TOX_INDEX:			nRet = 5;					break;
/*N*/ 		case TOX_USER:			nRet = MAXLEVEL+1;   		break;
/*N*/ 		case TOX_CONTENT:		nRet = MAXLEVEL+1;          break;
/*N*/ 		case TOX_ILLUSTRATIONS:
/*N*/ 		case TOX_OBJECTS      :
/*?*/ 		case TOX_TABLES       : nRet = 2; break;
/*?*/ 		case TOX_AUTHORITIES  :	nRet = AUTH_TYPE_END + 1;		break;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }
/* -----------------15.06.99 13:39-------------------
    ::com::patibilty methods: Version 5.0 and 5.1 need
    a value for the first tab stop
 --------------------------------------------------*/
/*N*/ USHORT lcl_GetPatternCount( const String& rPattern, const sal_Char* pToken )
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	String aSearch; aSearch.AssignAscii( pToken );
/*N*/ 	aSearch.Erase( aSearch.Len() - 1, 1 );
/*N*/ 	xub_StrLen nFound = rPattern.Search( aSearch );
/*N*/ 	while( STRING_NOTFOUND != nFound )
/*N*/ 		if( STRING_NOTFOUND != ( nFound = rPattern.Search( '>', nFound ) ))
/*N*/ 		{
/*N*/ 			++nRet;
/*N*/ 			nFound = rPattern.Search( aSearch, nFound );
/*N*/ 		}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ String lcl_GetPattern( const String& rPattern, const sal_Char* pToken )
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 	String aSearch; aSearch.AssignAscii( pToken );
/*N*/ 	aSearch.Erase( aSearch.Len() - 1, 1 );
/*N*/ 
/*N*/ 	xub_StrLen nEnd, nFound = rPattern.Search( aSearch );
/*N*/ 	if( STRING_NOTFOUND != nFound &&
/*N*/ 		STRING_NOTFOUND != ( nEnd = rPattern.Search( '>', nFound )) )
/*N*/ 		sRet = rPattern.Copy( nFound, nEnd - nFound + 1 );
/*N*/ 	return sRet;
/*N*/ }

/*N*/ USHORT SwForm::GetFirstTabPos() const  	//{ return nFirstTabPos; }
/*N*/ {
/*N*/ 	DBG_WARNING("compatibility");
/*N*/ 	String sFirstLevelPattern = aPattern[ 1 ];
/*N*/ 	USHORT nRet = 0;
/*N*/ 	if( 2 <= ::binfilter::lcl_GetPatternCount( sFirstLevelPattern, SwForm::aFormTab ))
/*N*/ 	{
/*N*/ 		//sTab is in the Form "<T ,,value>" where value is the tab position an may be empty
/*N*/ 		String sTab = lcl_GetPattern( sFirstLevelPattern, SwForm::aFormTab );
/*N*/ 		if( 3 <= sTab.GetTokenCount(',') )
/*N*/ 		{
/*N*/ 			sTab = sTab.GetToken( 2, ',');
/*N*/ 			sTab.Erase( sTab.Len() - 1, 1 );
/*N*/ 			nRet = sTab.ToInt32();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }
void SwForm::SetFirstTabPos( USHORT n )     //{ nFirstTabPos = n; }
{
    // the tab stop token looks like: <T ,,1234,0,.> <T> <T ,,1234>
    //for loading only: all levels get a first tab stop at the given position
    String sVal( String::CreateFromInt32(  n ));
    String sTmp; sTmp.AssignAscii( SwForm::aFormTab );
    sTmp.Insert(sVal, 2);

    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        //if two tabstops then exchange
        String& rPattern = aPattern[ i + 1];
        if( 2 <= lcl_GetPatternCount( rPattern, SwForm::aFormTab ))
        {
            //change existing tab
            xub_StrLen nStart = rPattern.SearchAscii( "<T" );
            xub_StrLen nEnd = rPattern.Search( '>', nStart );
            String sTmp( rPattern.Copy( nStart, nEnd - nStart + 1 ));
            rPattern.Erase( nStart, nEnd - nStart + 1 );

            // if TabAlign is set
            String sTabAlign;
            if(sTmp.GetTokenCount(',') >= 4)
            {
                sTabAlign = sTmp.GetToken(3, ',');
                sTabAlign.Erase(sTabAlign.Len() - 1, 1);
            }
            String sTabFillChar;
            if(sTmp.GetTokenCount(',') >= 5)
            {
                sTabFillChar = sTmp.GetToken(4, ',');
                sTabFillChar.Erase(sTabAlign.Len() - 1, 1);
            }
            sTmp.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<T ,," ));
            sTmp += sVal;
            if( sTabAlign.Len() )
            {
                sTmp += ',';
                sTmp += sTabAlign;
            }
            if(sTabFillChar.Len())
            {
                sTmp += ',';
                sTmp += sTabFillChar;
            }
            sTmp += '>';
            rPattern.Insert( sTmp, nStart );
        }
        else
        {
            //insert new tab after the first token
            xub_StrLen nIndex = rPattern.Search('>');
            String sTmp;
            sTmp.AssignAscii( SwForm::aFormTab );
            sTmp.InsertAscii( " ,,", nFormTabLen - 1);
            sTmp.Insert( sVal, nFormTabLen + 2 );
            rPattern.Insert( sTmp, nIndex + 1 );
        }
    }
}
/* -----------------------------28.02.00 09:48--------------------------------
    if the templates contain settings of the tab positions (<5.1) then
    they must be imported into the pattern
 ---------------------------------------------------------------------------*/
/*N*/ BOOL lcl_FindTabToken( const String& rPattern, xub_StrLen nSearchFrom,
/*N*/ 					xub_StrLen& rFoundStart, xub_StrLen& rFoundEnd)
/*N*/ {
/*N*/ 	// search for <T ...>
/*N*/ 	// special handling of <TX ...>
/*N*/ 
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	String sToFind; sToFind.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<T" ));
/*N*/ 	xub_StrLen nFoundPos = rPattern.Search( sToFind, nSearchFrom );
/*N*/ 	while( STRING_NOTFOUND != nFoundPos )
/*N*/ 	{
/*N*/ 		if( rPattern.GetChar(nFoundPos +1) == 'X')
/*N*/ 		{
/*?*/ 			// handling of text tokens
/*?*/ 			xub_StrLen nTempDelim = rPattern.Search(TOX_STYLE_DELIMITER, nFoundPos);
/*?*/ 			xub_StrLen nTempEndSymbol = rPattern.Search(TOX_STYLE_DELIMITER, nFoundPos);
/*?*/ 			if( nTempEndSymbol < nTempDelim && STRING_NOTFOUND != nTempDelim )
/*?*/ 			{
/*?*/ 				//text delimiter must! be in pairs
/*?*/ 				nTempDelim = rPattern.Search(TOX_STYLE_DELIMITER, nTempDelim + 1);
/*?*/ 				nTempEndSymbol = rPattern.Search(TOX_STYLE_DELIMITER, nTempDelim + 1);
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				nFoundPos = nTempDelim;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//tab stop token found - find end position
/*N*/ 			rFoundStart = nFoundPos;
/*N*/ 			rFoundEnd = rPattern.Search('>', nFoundPos +1);
/*N*/ 			bRet = TRUE;
/*N*/ 			break;
/*N*/ 		}
/*?*/ 		nFoundPos = rPattern.Search( sToFind, nSearchFrom );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }
//-----------------------------------------------------------------------------
/*N*/ void SwForm::AdjustTabStops(SwDoc& rDoc)
/*N*/ {
/*N*/ 	for(USHORT nLevel = 1; nLevel < GetFormMax(); nLevel++)
/*N*/ 	{
/*N*/ 		const String& sTemplateName = GetTemplate(nLevel);
/*N*/ 
/*N*/ 		SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( sTemplateName );
/*N*/ 		if( !pColl )
/*N*/ 		{
/*N*/ 			USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( sTemplateName, GET_POOLID_TXTCOLL );
/*N*/ 			if( USHRT_MAX != nId )
/*N*/ 				pColl = rDoc.GetTxtCollFromPool( nId );
/*N*/ 		}
/*N*/ 		if( pColl )
/*N*/ 		{
/*N*/ 			const SvxTabStopItem& rTabStops = pColl->GetTabStops( FALSE );
/*N*/ 			USHORT nTabCount = rTabStops.Count();
/*N*/ 			String sCurrentPattern = GetPattern(nLevel);
/*N*/ 			xub_StrLen nLastTabFoundEndPos = 0;
/*N*/ 			BOOL bChanged = FALSE;
/*N*/ 			for(USHORT nTab = 0; nTab < nTabCount; nTab++)
/*N*/ 			{
/*N*/ 				bChanged = TRUE;
/*N*/ 				const SvxTabStop& rTab = rTabStops[nTab];
/*N*/ 				xub_StrLen nStart, nEnd;
/*N*/ 				if( ::binfilter::lcl_FindTabToken( sCurrentPattern, nLastTabFoundEndPos,
/*N*/ 										nStart, nEnd ))
/*N*/ 				{
/*N*/ 					sCurrentPattern.Erase( nStart, nEnd - nStart + 1 );
/*N*/ 					// should look like: T ,,12345,2,_
/*N*/ 					long nPosition = rTab.GetTabPos();
/*N*/ 					USHORT eAlign =  rTab.GetAdjustment();
/*N*/ 					sal_Unicode cFillChar = rTab.GetFill();
/*N*/ 					//create new token
/*N*/ 					String sNewToken;
/*N*/ 					sNewToken.AssignAscii( RTL_CONSTASCII_STRINGPARAM("<T ,,"));
/*N*/ 					sNewToken += String::CreateFromInt32( nPosition );
/*N*/ 					sNewToken += ',';
/*N*/ 					//the last tab stop is converted into a 'most right' tab stop
/*N*/ 					//this will work most of the time - not if a right tab stop
/*N*/ 					//is define before the line end - but it cannot be detected
/*N*/ 					if( nTab == nTabCount - 1 && eAlign == SVX_TAB_ADJUST_RIGHT)
/*N*/ 						eAlign = SVX_TAB_ADJUST_END;
/*N*/ 					sNewToken += String::CreateFromInt32( eAlign );
/*N*/ 					sNewToken += ',';
/*N*/ 					sNewToken += cFillChar;
/*N*/ 					sNewToken += '>';
/*N*/ 					sCurrentPattern.Insert(sNewToken, nStart);
/*N*/ 					//set next search pos behind the changed token
/*N*/ 					nLastTabFoundEndPos = nStart + sNewToken.Len();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					break;
/*N*/ 			}
/*N*/ 
/*N*/ 			if(bChanged)
/*N*/ 				SetPattern(nLevel, sCurrentPattern);
/*N*/ 		}
/*N*/ 	}
/*N*/ }
//-----------------------------------------------------------------------------

/*N*/ BOOL SwForm::IsFirstTabPosFlag() const 		//{ return bHasFirstTabPos; }
/*N*/ {
/*N*/ 	//rturn true if the first level contains two ore more tabstops
/*N*/ 	return 2 <= lcl_GetPatternCount(aPattern[ 1 ], SwForm::aFormTab);
/*N*/ }
/* -----------------29.07.99 14:37-------------------

 --------------------------------------------------*/
/*N*/ String	SwForm::ConvertPatternTo51(const String& rSource)
/*N*/ {
/*N*/ 	SwFormTokenEnumerator aEnum(rSource);
/*N*/ 	String sRet;
/*N*/ 	while(aEnum.HasNextToken())
/*N*/ 	{
/*N*/ 		SwFormToken  aToken(aEnum.GetNextToken());
/*N*/ 		switch(aToken.eTokenType)
/*N*/ 		{
/*N*/ 			case TOKEN_ENTRY_NO     :
/*N*/ 				sRet.AppendAscii( SwForm::aFormEntryNum );
/*N*/ 			break;
/*N*/ 			case TOKEN_ENTRY_TEXT   :
/*N*/ 				sRet.AppendAscii( SwForm::aFormEntryTxt );
/*N*/ 			break;
/*N*/ 			case TOKEN_ENTRY        :
/*N*/ 				sRet.AppendAscii( SwForm::aFormEntry );
/*N*/ 			break;
/*N*/ 			case TOKEN_TAB_STOP     :
/*N*/ 				sRet.AppendAscii( SwForm::aFormTab );
/*N*/ 			break;
/*N*/ 			case TOKEN_TEXT         :
/*?*/ 				sRet += aToken.sText;
/*?*/ 			break;
/*N*/ 			case TOKEN_PAGE_NUMS    :
/*N*/ 				sRet.AppendAscii( SwForm::aFormPageNums );
/*N*/ 			break;
/*N*/ 			case TOKEN_CHAPTER_INFO :
/*N*/ 				// not available in 5.1
/*N*/ 			break;
/*N*/ 			case TOKEN_LINK_START   :
/*N*/ 				sRet.AppendAscii( SwForm::aFormLinkStt );
/*N*/ 			break;
/*N*/ 			case TOKEN_LINK_END     :
/*N*/ 				sRet.AppendAscii( SwForm::aFormLinkEnd );
/*N*/ 			break;
/*N*/ 			case TOKEN_AUTHORITY :
/*N*/ 				// no conversion available
/*?*/ 				sRet.AppendAscii( SwForm::aFormEntry );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sRet;
/*N*/ }
/* -----------------29.07.99 14:37-------------------

 --------------------------------------------------*/
/*N*/ String	SwForm::ConvertPatternFrom51(const String& rSource, TOXTypes eType)
/*N*/ {
/*N*/ 	String sRet( rSource );
/*N*/ 	DBG_ASSERT(rSource.GetTokenCount('>') == rSource.GetTokenCount('<'),
/*N*/ 				"count of '<' and '>' not identical");
/*N*/ //	simple task - convert tokens
/*N*/ //	<E> -> <E >
/*N*/ //	<E#> -> <E# >
/*N*/ //	<ET> -> <ET >
/*N*/ //	<#> -> <# >
/*N*/ //	<T> -> <T >
/*N*/ //	<LS> -> <LS >
/*N*/ //	<LE> -> <LE >
/*N*/ 	for( xub_StrLen nFound = 0;
/*N*/ 		STRING_NOTFOUND != ( nFound = sRet.Search( '>', nFound )); )
/*N*/ 	{
/*N*/ 		sRet.Insert( ' ', nFound );
/*N*/ 		nFound += 2;	                // skip over the blank and '>'
/*N*/ 	}
/*N*/ 
/*N*/ 	//more complicated: convert text between >TEXT< to <X \0xffTEXT\0xff>
/*N*/ 	xub_StrLen nOpenStart = 0;
/*N*/ 	xub_StrLen nCloseStart = 0;
/*N*/ 	xub_StrLen nClosePos = sRet.Search('>', nCloseStart);
/*N*/ 	xub_StrLen nOpenPos = sRet.Search('<', nOpenStart);
/*N*/ 	if(nOpenPos != STRING_NOTFOUND && nOpenPos > 0)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nOffset = lcl_ConvertTextIntoPattern( sRet, 0, nOpenPos);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nOpenStart = nClosePos;
/*N*/ 	nOpenPos = sRet.Search('<', ++nOpenStart);
/*N*/ 	while(nOpenPos != STRING_NOTFOUND)
/*N*/ 	{
/*N*/ 		if(nClosePos < nOpenPos - 1)
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nOffset = lcl_ConvertTextIntoPattern(sRet, nClosePos + 1, nOpenPos);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nCloseStart = nClosePos;
/*N*/ 			nOpenStart = nOpenPos;
/*N*/ 		}
/*N*/ 		nClosePos = sRet.Search('>', ++nCloseStart);
/*N*/ 		nOpenPos = sRet.Search('<', ++nOpenStart);
/*N*/ 	}
/*N*/ 	//is there any text at the end?
/*N*/ 	if(nClosePos != STRING_NOTFOUND && nClosePos < sRet.Len() - 1)
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	lcl_ConvertTextIntoPattern(sRet, nClosePos + 1, sRet.Len());
/*N*/ 	if(eType != TOX_INDEX)
/*N*/ 	{
/*N*/ 		// set most left tab stop to right alignment and FillChar == '.'
/*N*/ 		String sTabSearch;
/*N*/ 		sTabSearch.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<T "));
/*N*/ 		xub_StrLen nOldTabPos = sRet.Search(sTabSearch);
/*N*/ 		if(STRING_NOTFOUND != nOldTabPos)
/*N*/ 		{
/*N*/ 			xub_StrLen nNewTabPos = nOldTabPos;
/*N*/ 			do {
/*N*/ 				nOldTabPos = nNewTabPos;
/*N*/ 				nNewTabPos = sRet.Search(sTabSearch, nOldTabPos + 1);
/*N*/ 			} while(STRING_NOTFOUND != nNewTabPos);
/*N*/ 			sRet.InsertAscii( ",65535,0,5,.", nOldTabPos + 3 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sRet;
/*N*/ }

/*--------------------------------------------------------------------
     Beschreibung: Ctor TOXBase
 --------------------------------------------------------------------*/


/*N*/ SwTOXBase::SwTOXBase(const SwTOXType* pTyp,	const SwForm& rForm,
/*N*/ 					 USHORT nCreaType, const String& rTitle )
/*N*/ 	: SwClient((SwModify*)pTyp),
/*N*/ 	nCreateType(nCreaType),
/*N*/ 	aTitle(rTitle),
/*N*/ 	aForm(rForm),
/*N*/ 	eCaptionDisplay(CAPTION_COMPLETE),
/*N*/     eLanguage((LanguageType)::binfilter::GetAppLanguage()),
/*N*/ 	bProtected( TRUE ),
/*N*/ 	bFromChapter(FALSE),
/*N*/ 	bFromObjectNames(FALSE),
/*N*/ 	bLevelFromChapter(FALSE),
/*N*/ 	nOLEOptions(0)
/*N*/ {
/*N*/ 	aData.nOptions = 0;
/*N*/ }


/*N*/ SwTOXBase::SwTOXBase( const SwTOXBase& rSource, SwDoc* pDoc )
/*N*/ 	: SwClient( rSource.pRegisteredIn )
/*N*/ {
/*N*/ 	CopyTOXBase( pDoc, rSource );
/*N*/ }

/*N*/ SwTOXBase& SwTOXBase::CopyTOXBase( SwDoc* pDoc, const SwTOXBase& rSource )
/*N*/ {
/*N*/ 	SwTOXType* pType = (SwTOXType*)rSource.GetTOXType();
/*N*/ 	if( pDoc && USHRT_MAX == pDoc->GetTOXTypes().GetPos( pType ))
/*N*/ 	{
/*?*/ 		// type not in pDoc, so create it now
/*?*/ 		const SwTOXTypes& rTypes = pDoc->GetTOXTypes();
/*?*/ 		BOOL bFound = FALSE;
/*?*/ 		for( USHORT n = rTypes.Count(); n; )
/*?*/ 		{
/*?*/ 			const SwTOXType* pCmp = rTypes[ --n ];
/*?*/ 			if( pCmp->GetType() == pType->GetType() &&
/*?*/ 				pCmp->GetTypeName() == pType->GetTypeName() )
/*?*/ 			{
/*?*/ 				pType = (SwTOXType*)pCmp;
/*?*/ 				bFound = TRUE;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		if( !bFound )
/*?*/ 			pType = (SwTOXType*)pDoc->InsertTOXType( *pType );
/*N*/ 	}
/*N*/ 	pType->Add( this );
/*N*/ 
/*N*/ 	nCreateType = rSource.nCreateType;
/*N*/ 	aTitle 		= rSource.aTitle;
/*N*/ 	aForm 		= rSource.aForm;
/*N*/ 	bProtected	= rSource.bProtected;
/*N*/ 	bFromChapter = rSource.bFromChapter;
/*N*/ 	bFromObjectNames = rSource.bFromObjectNames;
/*N*/ 	sMainEntryCharStyle = rSource.sMainEntryCharStyle;
/*N*/ 	sSequenceName =	rSource.sSequenceName;
/*N*/ 	eCaptionDisplay = rSource.eCaptionDisplay;
/*N*/ 	nOLEOptions = rSource.nOLEOptions;
/*N*/     eLanguage = rSource.eLanguage;
/*N*/     sSortAlgorithm = rSource.sSortAlgorithm;
/*N*/ 
/*N*/ 	for( USHORT i = 0; i < MAXLEVEL; ++i )
/*N*/ 		aStyleNames[i] = rSource.aStyleNames[i];
/*N*/ 
/*N*/ 	// its the same data type!
/*N*/ 	aData.nOptions =  rSource.aData.nOptions;
/*N*/ 
/*N*/ 	if( !pDoc || pDoc->IsCopyIsMove() )
/*N*/ 		aName = rSource.GetTOXName();
/*N*/ 	else
/*?*/ 		aName = pDoc->GetUniqueTOXBaseName( *pType, &rSource.GetTOXName() );
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

/* -----------------30.06.99 14:46-------------------
    Check if any style names are set in the array
 --------------------------------------------------*/

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisspezifische Funktionen
 --------------------------------------------------------------------*/

/*N*/ SwTOXBase::~SwTOXBase()
/*N*/ {
/*N*/ //    if( GetTOXType()->GetType() == TOX_USER  )
/*N*/ //        delete aData.pTemplateName;
/*N*/ }

/* -----------------16.07.99 16:02-------------------

 --------------------------------------------------*/
/*N*/ SwFormTokenEnumerator::SwFormTokenEnumerator( const String& rPattern )
/*N*/ 	: sPattern( rPattern ), nCurPatternPos( 0  ), nCurPatternLen( 0 )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	DBG_ASSERT( !sPattern.Len() ||
/*N*/ 				(sPattern.GetChar(0) == '<' &&
/*N*/ 				sPattern.GetChar(sPattern.Len() - 1) == '>'),
/*N*/ 				"Pattern incorrect!" );
/*N*/ 
/*N*/ 
/*N*/ 	// strip all characters included in TOX_STYLE_DELIMITER
/*N*/ 	String sTmp( sPattern );
/*N*/ 	xub_StrLen nFoundStt = sTmp.Search(TOX_STYLE_DELIMITER);
/*N*/ 	while(nFoundStt != STRING_NOTFOUND)
/*N*/ 	{
/*?*/ 		xub_StrLen nFoundEnd = sTmp.Search(TOX_STYLE_DELIMITER, nFoundStt + 1);
/*?*/ 		DBG_ASSERT(STRING_NOTFOUND != nFoundEnd, "Pattern incorrect");
/*?*/ 		sTmp.Erase(nFoundStt, nFoundEnd - nFoundStt + 1);
/*?*/ 		nFoundStt = sTmp.Search(TOX_STYLE_DELIMITER);
/*N*/ 	}
/*N*/ 	DBG_ASSERT(	sTmp.GetTokenCount('<') == sTmp.GetTokenCount('>'),
/*N*/ 							"Pattern incorrect!");
/*N*/ #endif
/*N*/ }

/* -----------------29.06.99 11:55-------------------

 --------------------------------------------------*/
/*N*/ SwFormToken SwFormTokenEnumerator::GetNextToken()
/*N*/ {
/*N*/ 	xub_StrLen nTokenLen, nEnd;
/*N*/ 	nCurPatternPos += nCurPatternLen;
/*N*/ 	FormTokenType eTokenType = _SearchNextToken( nCurPatternPos, nEnd,
/*N*/ 												&nTokenLen );
/*N*/ 	nCurPatternLen = nEnd - nCurPatternPos;
/*N*/ 	return BuildToken( eTokenType, nTokenLen );
/*N*/ }


/*N*/ SwFormToken SwFormTokenEnumerator::BuildToken( FormTokenType eTokenType,
/*N*/ 			  									xub_StrLen nTokenLen ) const
/*N*/ {
/*N*/ 	String sToken( sPattern.Copy( nCurPatternPos, nCurPatternLen ));
/*N*/ 
/*N*/ 	// at this point sPattern contains the
/*N*/ 	// character style name, the PoolId, tab stop position, tab stop alignment, chapter info format
/*N*/ 	// the form is: CharStyleName, PoolId[, TabStopPosition|ChapterInfoFormat[, TabStopAlignment[, TabFillChar]]]
/*N*/ 	// in text tokens the form differs from the others: CharStyleName, PoolId[,\0xffinserted text\0xff]
/*N*/ 	SwFormToken eRet( eTokenType );
/*N*/ 	String sAuthFieldEnum = sToken.Copy( 2, 2 );
/*N*/ 	sToken = sToken.Copy( nTokenLen, sToken.Len() - nTokenLen - 1);
/*N*/ 
/*N*/ 	eRet.sCharStyleName = sToken.GetToken( 0, ',');
/*N*/ 	String sTmp( sToken.GetToken( 1, ',' ));
/*N*/ 	if( sTmp.Len() )
/*N*/ 		eRet.nPoolId = sTmp.ToInt32();
/*N*/ 
/*N*/ 	switch( eTokenType )
/*N*/ 	{
/*N*/ 	case TOKEN_TEXT:
/*?*/ 		{
/*?*/ 			xub_StrLen nStartText = sToken.Search( TOX_STYLE_DELIMITER );
/*?*/ 			if( STRING_NOTFOUND != nStartText )
/*?*/ 			{
/*?*/ 				xub_StrLen nEndText = sToken.Search( TOX_STYLE_DELIMITER,
/*?*/ 												nStartText + 1);
/*?*/ 				if( STRING_NOTFOUND != nEndText )
/*?*/ 				{
/*?*/ 					eRet.sText = sToken.Copy( nStartText + 1,
/*?*/ 												nEndText - nStartText - 1);
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		break;
/*N*/ 
/*N*/ 	case TOKEN_TAB_STOP:
/*N*/ 		if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
/*N*/ 			eRet.nTabStopPosition = sTmp.ToInt32();
/*N*/ 
/*N*/ 		if( (sTmp = sToken.GetToken( 3, ',' ) ).Len() )
/*N*/ 			eRet.eTabAlign = sTmp.ToInt32();
/*N*/ 
/*N*/ 		if( (sTmp = sToken.GetToken( 4, ',' ) ).Len() )
/*N*/ 			eRet.cTabFillChar = sTmp.GetChar(0);
/*N*/ 		break;
/*N*/ 
/*N*/ 	case TOKEN_CHAPTER_INFO:
/*?*/ 		if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
/*?*/ 			eRet.nChapterFormat = sTmp.ToInt32(); //SwChapterFormat;
/*?*/ 		break;
/*N*/ 
/*N*/ 	case TOKEN_AUTHORITY:
/*?*/ 		eRet.nAuthorityField = sAuthFieldEnum.ToInt32();
/*?*/ 		break;
/*N*/ 	}
/*N*/ 	return eRet;
/*N*/ }

/*N*/ FormTokenType SwFormTokenEnumerator::_SearchNextToken( xub_StrLen nStt,
/*N*/ 							xub_StrLen& rEnd, xub_StrLen* pTokenLen ) const
/*N*/ {
/*N*/ 	//it's not so easy - it doesn't work if the text part contains a '>'
/*N*/ 	//USHORT nTokenEnd = sPattern.Search('>');
/*N*/ 	rEnd = sPattern.Search( '>', nStt );
/*N*/ 	if( STRING_NOTFOUND == rEnd )
/*N*/ 	{
/*?*/ 		rEnd = sPattern.Len();
/*?*/ 		return TOKEN_END;
/*N*/ 	}
/*N*/ 
/*N*/ 	xub_StrLen nTextSeparatorFirst = sPattern.Search( TOX_STYLE_DELIMITER, nStt );
/*N*/ 	if( STRING_NOTFOUND != nTextSeparatorFirst )
/*N*/ 	{
/*?*/ 		xub_StrLen nTextSeparatorSecond = sPattern.Search( TOX_STYLE_DELIMITER,
/*?*/ 												nTextSeparatorFirst + 1 );
/*?*/ 		if( STRING_NOTFOUND != nTextSeparatorSecond &&
/*?*/ 			rEnd > nTextSeparatorFirst )
/*?*/ 			rEnd = sPattern.Search( '>', nTextSeparatorSecond );
/*N*/ 	}
/*N*/ 
/*N*/ 	++rEnd;
/*N*/ 	String sToken( sPattern.Copy( nStt, rEnd - nStt ) );
/*N*/ 
/*N*/ 	static struct
/*N*/ 	{
/*N*/ 		const sal_Char* pNm;
/*N*/ 		USHORT nLen;
/*N*/ 		USHORT nOffset;
/*N*/ 		FormTokenType eToken;
/*N*/ 	} __READONLY_DATA aTokenArr[] = {
/*N*/ 		SwForm::aFormTab, 		SwForm::nFormEntryLen,		1, TOKEN_TAB_STOP,
/*N*/ 		SwForm::aFormPageNums,	SwForm::nFormPageNumsLen,	1, TOKEN_PAGE_NUMS,
/*N*/ 		SwForm::aFormLinkStt,	SwForm::nFormLinkSttLen,	1, TOKEN_LINK_START,
/*N*/ 		SwForm::aFormLinkEnd,	SwForm::nFormLinkEndLen,	1, TOKEN_LINK_END,
/*N*/ 		SwForm::aFormEntryNum,	SwForm::nFormEntryNumLen,	1, TOKEN_ENTRY_NO,
/*N*/ 		SwForm::aFormEntryTxt,	SwForm::nFormEntryTxtLen,	1, TOKEN_ENTRY_TEXT,
/*N*/ 		SwForm::aFormChapterMark,SwForm::nFormChapterMarkLen,1,TOKEN_CHAPTER_INFO,
/*N*/ 		SwForm::aFormText,		SwForm::nFormTextLen,		1, TOKEN_TEXT,
/*N*/ 		SwForm::aFormEntry,		SwForm::nFormEntryLen,		1, TOKEN_ENTRY,
/*N*/ 		SwForm::aFormAuth,		SwForm::nFormAuthLen,		3, TOKEN_AUTHORITY,
/*N*/ 		0,						0,							0, TOKEN_END
/*N*/ 	};
/*N*/ 
/*N*/ 	FormTokenType eTokenType = TOKEN_TEXT;
/*N*/ 	xub_StrLen nTokenLen = 0;
/*N*/ 	const sal_Char* pNm;
/*N*/ 	for( int i = 0; 0 != ( pNm = aTokenArr[ i ].pNm ); ++i )
/*N*/ 		if( COMPARE_EQUAL == sToken.CompareToAscii( pNm,
/*N*/ 							aTokenArr[ i ].nLen - aTokenArr[ i ].nOffset ))
/*N*/ 		{
/*N*/ 			eTokenType = aTokenArr[ i ].eToken;
/*N*/ 			nTokenLen = aTokenArr[ i ].nLen;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 	ASSERT( pNm, "wrong token" );
/*N*/ 	if( pTokenLen )
/*N*/ 		*pTokenLen = nTokenLen;
/*N*/ 	return eTokenType;
/*N*/ }






/*N*/ String SwFormToken::GetString() const
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 
/*N*/ 	BOOL bAppend = TRUE;
/*N*/ 	switch( eTokenType )
/*N*/ 	{
/*N*/ 		case TOKEN_ENTRY_NO:
/*N*/ 			sRet.AssignAscii( SwForm::aFormEntryNum );
/*N*/ 		break;
/*N*/ 		case TOKEN_ENTRY_TEXT:
/*N*/ 			sRet.AssignAscii( SwForm::aFormEntryTxt );
/*N*/ 		break;
/*N*/ 		case TOKEN_ENTRY:
/*N*/ 			sRet.AssignAscii( SwForm::aFormEntry );
/*N*/ 		break;
/*N*/ 		case TOKEN_TAB_STOP:
/*N*/ 			sRet.AssignAscii( SwForm::aFormTab );
/*N*/ 		break;
/*N*/ 		case TOKEN_TEXT:
/*?*/ 			sRet.AssignAscii( SwForm::aFormText );
/*?*/ 		break;
/*N*/ 		case TOKEN_PAGE_NUMS:
/*N*/ 			sRet.AssignAscii( SwForm::aFormPageNums );
/*N*/ 		break;
/*N*/ 		case TOKEN_CHAPTER_INFO:
/*?*/ 			sRet.AssignAscii( SwForm::aFormChapterMark );
/*?*/ 		break;
/*N*/ 		case TOKEN_LINK_START:
/*N*/ 			sRet.AssignAscii( SwForm::aFormLinkStt );
/*N*/ 		break;
/*N*/ 		case TOKEN_LINK_END:
/*N*/ 			sRet.AssignAscii( SwForm::aFormLinkEnd );
/*N*/ 		break;
/*N*/ 		case TOKEN_AUTHORITY:
/*N*/ 		{
/*?*/ 			sRet.AssignAscii( SwForm::aFormAuth );
/*?*/ 			String sTmp( String::CreateFromInt32( nAuthorityField ));
/*?*/ 			if( sTmp.Len() < 2 )
/*?*/ 				sTmp.Insert('0', 0);
/*?*/ 			sRet.Insert( sTmp, 2 );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	sRet.Erase( sRet.Len() - 1 );
/*N*/ 	sRet += ' ';
/*N*/ 	sRet += sCharStyleName;
/*N*/ 	sRet += ',';
/*N*/ 	sRet += String::CreateFromInt32( nPoolId );
/*N*/ 	sRet += ',';
/*N*/ 
/*N*/ 	// TabStopPosition and TabAlign or ChapterInfoFormat
/*N*/ 	if(TOKEN_TAB_STOP == eTokenType)
/*N*/ 	{
/*N*/ 		sRet += String::CreateFromInt32( nTabStopPosition );
/*N*/ 		sRet += ',';
/*N*/ 		sRet += String::CreateFromInt32( eTabAlign );
/*N*/ 		sRet += ',';
/*N*/ 		sRet += cTabFillChar;
/*N*/ 	}
/*N*/ 	else if(TOKEN_CHAPTER_INFO == eTokenType)
/*N*/ 	{
/*?*/ 		sRet += String::CreateFromInt32( nChapterFormat );
/*N*/ 	}
/*N*/ 	else if(TOKEN_TEXT == eTokenType)
/*N*/ 	{
/*?*/ 		//append Text if Len() > 0 only!
/*?*/ 		if( sText.Len() )
/*?*/ 		{
/*?*/ 			sRet += TOX_STYLE_DELIMITER;
/*?*/ 			String sTmp( sText );
/*?*/ 			sTmp.EraseAllChars( TOX_STYLE_DELIMITER );
/*?*/ 			sRet += sTmp;
/*?*/ 			sRet += TOX_STYLE_DELIMITER;
/*?*/ 		}
/*?*/ 		else
/*?*/ 			bAppend = FALSE;
/*N*/ 	}
/*N*/ 	if(bAppend)
/*N*/ 	{
/*N*/ 		sRet += '>';
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// don't append empty text tokens
/*?*/ 		sRet.Erase();
/*N*/ 	}
/*N*/ 
/*N*/ 	return sRet;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
