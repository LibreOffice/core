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


#ifndef _COM_SUN_STAR_I18N_CHARTYPE_HDL
#include <com/sun/star/i18n/CharType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star::i18n;


#define KAPITAELCHENPROP 66

/*************************************************************************
 *						class SwCapitalInfo
 *
 * The information encapsulated in SwCapitalInfo is required
 * by the ::Do functions. They contain the information about
 * the original string, whereas rDo.GetInf() contains information
 * about the display string.
 *************************************************************************/

class SwCapitalInfo
{
public:
    explicit SwCapitalInfo( const XubString& rOrigText ) :
        rString( rOrigText ), nIdx( 0 ), nLen( 0 ) {};
    const XubString& rString;
    xub_StrLen nIdx;
    xub_StrLen nLen;
};

/*************************************************************************
 *						xub_StrLen lcl_CalcCaseMap()
 *
 * rFnt: required for CalcCaseMap
 * rOrigString: The original string
 * nOfst: Position of the substring in rOrigString
 * nLen: Length if the substring in rOrigString
 * nIdx: Referes to a position in the display string and should be mapped
 *       to a position in rOrigString
 *************************************************************************/

xub_StrLen lcl_CalcCaseMap( const SwFont& rFnt,
                            const XubString& rOrigString,
                            xub_StrLen nOfst,
                            xub_StrLen nLen,
                            xub_StrLen nIdx )
{
    int j = 0;
    const xub_StrLen nEnd = nOfst + nLen;
    ASSERT( nEnd <= rOrigString.Len(), "lcl_CalcCaseMap: Wrong parameters" )

    // special case for title case:
    const bool bTitle = SVX_CASEMAP_TITEL == rFnt.GetCaseMap() &&
                        pBreakIt->xBreak.is();
    for ( int i = nOfst; i < nEnd; ++i )
    {
        XubString aTmp( rOrigString, i, 1 );

        if ( !bTitle ||
             pBreakIt->xBreak->isBeginWord(
                 rOrigString, i,
                 pBreakIt->GetLocale( rFnt.GetLanguage() ), 
                 WordType::ANYWORD_IGNOREWHITESPACES ) )
            aTmp = rFnt.GetActualFont().CalcCaseMap( aTmp );
    
        j += aTmp.Len();

        if ( j > nIdx )
            return i;
    }

    return nOfst + nLen;
}

/*************************************************************************
 *						class SwDoCapitals
 *************************************************************************/

class SwDoCapitals
{
protected:
    SwDrawTextInfo &rInf;
    SwCapitalInfo* pCapInf; // referes to additional information
                           // required by the ::Do function
public:
    SwDoCapitals ( SwDrawTextInfo &rInfo ) : rInf( rInfo ), pCapInf( 0 ) { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont ) = 0;
    virtual void Do() = 0;
    inline OutputDevice *GetOut() { return rInf.GetpOut(); }
    inline SwDrawTextInfo& GetInf() { return rInf; }
    inline SwCapitalInfo* GetCapInf() const { return pCapInf; }
    inline void SetCapInf( SwCapitalInfo& rNew ) { pCapInf = &rNew; }
};

/*************************************************************************
 *					  class SwDoGetCapitalSize
 *************************************************************************/

class SwDoGetCapitalSize : public SwDoCapitals
{
protected:
    Size aTxtSize;
public:
    SwDoGetCapitalSize( SwDrawTextInfo &rInfo ) : SwDoCapitals ( rInfo ) { }
    virtual void Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont );
    virtual void Do();
    const Size &GetSize() const { return aTxtSize; }
};

/*N*/ void SwDoGetCapitalSize::Init( SwFntObj *pUpperFont, SwFntObj *pLowerFont )
/*N*/ {
/*N*/ 	aTxtSize.Height() = 0;
/*N*/ 	aTxtSize.Width() = 0;
/*N*/ }

/*N*/ void SwDoGetCapitalSize::Do()
/*N*/ {
/*N*/ 	aTxtSize.Width() += rInf.GetSize().Width();
/*N*/ 	if( rInf.GetUpper() )
/*N*/ 		aTxtSize.Height() = rInf.GetSize().Height();
/*N*/ }

/*************************************************************************
 *					  SwSubFont::GetCapitalSize()
 *************************************************************************/

/*N*/ Size SwSubFont::GetCapitalSize( SwDrawTextInfo& rInf )
/*N*/ {
/*N*/ 	// Start:
/*N*/ 	short nOldKern = rInf.GetKern();
/*N*/ 	rInf.SetKern( CheckKerning() );
/*N*/ 	Point aPos;
/*N*/ 	rInf.SetPos( aPos );
/*N*/ 	rInf.SetSpace( 0 );
/*N*/ 	rInf.SetDrawSpace( FALSE );
/*N*/ 	SwDoGetCapitalSize aDo( rInf );
/*N*/ 	DoOnCapitals( aDo );
/*N*/ 	Size aTxtSize( aDo.GetSize() );
/*N*/ 
/*N*/ 	// End:
/*N*/ 	if( !aTxtSize.Height() )
/*N*/ 	{
/*N*/ 		SV_STAT( nGetTextSize );
/*N*/ 		aTxtSize.Height() = short ( rInf.GetpOut()->GetTextHeight() );
/*N*/ 	}
/*N*/ 	rInf.SetKern( nOldKern );
/*N*/ 	return aTxtSize;
/*N*/ }

/*************************************************************************
 *					  class SwDoGetCapitalBreak
 *************************************************************************/

class SwDoGetCapitalBreak : public SwDoCapitals
{
protected:
    xub_StrLen *pExtraPos;
    long nTxtWidth;
    xub_StrLen nBreak;
public:
    SwDoGetCapitalBreak( SwDrawTextInfo &rInfo, long nWidth, xub_StrLen *pExtra)
        :	SwDoCapitals ( rInfo ), nTxtWidth( nWidth ),
            nBreak( STRING_LEN ), pExtraPos( pExtra )
        { }
    xub_StrLen GetBreak() const { return nBreak; }
};



/*************************************************************************
 *					  SwFont::GetCapitalBreak()
 *************************************************************************/


/*************************************************************************
 *					   class SwDoDrawCapital
 *************************************************************************/

class SwDoDrawCapital : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
public:
    SwDoDrawCapital( SwDrawTextInfo &rInfo ) :
        SwDoCapitals( rInfo )
        { }
};



/*************************************************************************
 *					  SwDoDrawCapital::DrawSpace()
 *************************************************************************/


/*************************************************************************
 *					  SwSubFont::DrawCapital()
 *************************************************************************/


/*************************************************************************
 *					   class SwDoDrawCapital
 *************************************************************************/

class SwDoCapitalCrsrOfst : public SwDoCapitals
{
protected:
    SwFntObj *pUpperFnt;
    SwFntObj *pLowerFnt;
    xub_StrLen nCrsr;
    USHORT nOfst;
public:
    SwDoCapitalCrsrOfst( SwDrawTextInfo &rInfo, const USHORT nOfs ) :
        SwDoCapitals( rInfo ), nOfst( nOfs ), nCrsr( 0 )
        { }

    void DrawSpace( const Point &rPos );
    inline xub_StrLen GetCrsr(){ return nCrsr; }
};



/*************************************************************************
 *					  SwSubFont::GetCapitalCrsrOfst()
 *************************************************************************/


/*************************************************************************
 *					  class SwDoDrawStretchCapital
 *************************************************************************/

class SwDoDrawStretchCapital : public SwDoDrawCapital
{
    const xub_StrLen nStrLen;
    const USHORT nCapWidth;
    const USHORT nOrgWidth;
public:

    SwDoDrawStretchCapital( SwDrawTextInfo &rInfo, const USHORT nCapWidth )
            : SwDoDrawCapital( rInfo ),
              nCapWidth( nCapWidth ),
              nOrgWidth( rInfo.GetWidth() ),
              nStrLen( rInfo.GetLen() )
        { }
};

/*************************************************************************
 *					  SwDoDrawStretchCapital
 *************************************************************************/


/*************************************************************************
 *					  SwSubFont::DrawStretchCapital()
 *************************************************************************/


/*************************************************************************
 *					SwSubFont::DoOnCapitals() const
 *************************************************************************/

// JP 22.8.2001 - global optimization off - Bug 91245 / 91223
#ifdef _MSC_VER
#pragma optimize("g",off)
#endif

/*N*/ void SwSubFont::DoOnCapitals( SwDoCapitals &rDo )
/*N*/ {
/*N*/ 	ASSERT( pLastFont, "SwFont::DoOnCapitals: No LastFont?!" );
/*N*/ 
/*N*/ 	Size aPartSize;
/*N*/ 	long nKana = 0;
/*N*/ 	const XubString aTxt( CalcCaseMap( rDo.GetInf().GetText() ) );
/*N*/ 	xub_StrLen nMaxPos = Min( USHORT(rDo.GetInf().GetText().Len()
/*N*/ 							- rDo.GetInf().GetIdx()), rDo.GetInf().GetLen() );
/*N*/ 	rDo.GetInf().SetLen( nMaxPos );
/*N*/ 
/*N*/ 	const XubString& rOldText = rDo.GetInf().GetText();
/*N*/ 	rDo.GetInf().SetText( aTxt );
/*N*/ 	rDo.GetInf().SetSize( aPartSize );
/*N*/ 	xub_StrLen nPos = rDo.GetInf().GetIdx();
/*N*/ 	xub_StrLen nOldPos = nPos;
/*N*/ 	nMaxPos += nPos;
/*N*/ 
/*N*/ 	// #107816#
/*N*/ 	// Look if the length of the original text and the ToUpper-converted
/*N*/ 	// text is different. If yes, do special handling.
/*N*/ 	XubString aNewText;
/*N*/     SwCapitalInfo aCapInf( rOldText );
/*N*/     sal_Bool bCaseMapLengthDiffers( aTxt.Len() != rOldText.Len() );
/*N*/     if ( bCaseMapLengthDiffers )
/*N*/         rDo.SetCapInf( aCapInf );
/*N*/ 
/*N*/ 	SwFntObj *pOldLast = pLastFont;
/*N*/ 	SwFntAccess *pBigFontAccess = NULL;
/*N*/ 	SwFntObj *pBigFont;
/*N*/ 	SwFntAccess *pSpaceFontAccess = NULL;
/*N*/ 	SwFntObj *pSpaceFont = NULL;
/*N*/ 
/*N*/ 	const void *pMagic2 = NULL;
/*N*/ 	USHORT nIndex2 = 0;
/*N*/ 	SwSubFont aFont( *this );
/*N*/ 	Point aStartPos( rDo.GetInf().GetPos() );
/*N*/ 
/*N*/ 	const BOOL bUnderStriked = aFont.GetUnderline() != UNDERLINE_NONE
/*N*/ 							|| aFont.GetStrikeout() != STRIKEOUT_NONE;
/*N*/ 	const BOOL bWordWise = bUnderStriked && aFont.IsWordLineMode() &&
/*N*/ 						   rDo.GetInf().GetDrawSpace();
/*N*/ 	const short nKern = rDo.GetInf().GetKern();
/*N*/ 
/*N*/ 	if ( bUnderStriked )
/*N*/ 	{
/*N*/ 		if ( bWordWise )
/*N*/ 		{
/*?*/ 			aFont.SetWordLineMode( FALSE );
/*?*/ 			pSpaceFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
/*?*/ 												rDo.GetInf().GetShell() );
/*?*/ 			pSpaceFont = pSpaceFontAccess->Get();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pSpaceFont = pLastFont;
/*N*/ 
/*N*/ 		// Wir basteln uns einen Font fuer die Grossbuchstaben:
/*N*/ 		aFont.SetUnderline( UNDERLINE_NONE );
/*N*/ 		aFont.SetStrikeout( STRIKEOUT_NONE );
/*N*/ 		pMagic2 = NULL;
/*N*/ 		nIndex2 = 0;
/*N*/ 		pBigFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
/*N*/ 										  rDo.GetInf().GetShell() );
/*N*/ 		pBigFont = pBigFontAccess->Get();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pBigFont = pLastFont;
/*N*/ 
/*N*/ 	// Hier entsteht der Kleinbuchstabenfont:
/*N*/ 	aFont.SetProportion( BYTE( (aFont.GetPropr()*KAPITAELCHENPROP) / 100L) );
/*N*/ 	pMagic2 = NULL;
/*N*/ 	nIndex2 = 0;
/*N*/ 	SwFntAccess *pSmallFontAccess = new SwFntAccess( pMagic2, nIndex2, &aFont,
/*N*/ 													 rDo.GetInf().GetShell() );
/*N*/ 	SwFntObj *pSmallFont = pSmallFontAccess->Get();
/*N*/ 
/*N*/ 	rDo.Init( pBigFont, pSmallFont );
/*N*/ 	OutputDevice* pOutSize = pSmallFont->GetPrt();
/*N*/ 	if( !pOutSize )
/*N*/ 		pOutSize = rDo.GetOut();
/*N*/ 	OutputDevice* pOldOut = rDo.GetOut();
/*N*/ 
/*N*/ 	const LanguageType eLng = LANGUAGE_DONTKNOW == GetLanguage()
/*N*/ 							? LANGUAGE_SYSTEM : GetLanguage();
/*N*/ 
/*N*/ 	if( nPos < nMaxPos )
/*N*/ 	{
/*N*/ 		nPos = (xub_StrLen)pBreakIt->xBreak->endOfCharBlock( rOldText, nPos,
/*N*/ 			pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
/*N*/ 		if( nPos == STRING_LEN )
/*N*/ 			nPos = nOldPos;
/*N*/ 		else if( nPos > nMaxPos )
/*N*/ 			nPos = nMaxPos;
/*N*/ 	}
/*N*/ 
/*N*/ 	while( nOldPos < nMaxPos )
/*N*/ 	{
/*N*/ 
/*N*/ 		//  The lower ones...
/*N*/ 		if( nOldPos != nPos )
/*N*/ 		{
/*N*/ 			SV_STAT( nGetTextSize );
/*N*/ 			pLastFont = pSmallFont;
/*N*/ 			pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
/*N*/ 
/*N*/ 			// #107816#, #i14820#
/*N*/             if( bCaseMapLengthDiffers )
/*N*/             {
/*N*/                 // Build an own 'changed' string for the given part of the
/*N*/                 // source string and use it. That new string may differ in length
/*N*/                 // from the source string.
/*N*/                 const XubString aSnippet( rOldText, nOldPos, nPos - nOldPos);
/*N*/                 aNewText = CalcCaseMap( aSnippet );
/*N*/                 aCapInf.nIdx = nOldPos;
/*N*/                 aCapInf.nLen = nPos - nOldPos;
/*N*/                 rDo.GetInf().SetIdx( 0 );
/*N*/                 rDo.GetInf().SetLen( aNewText.Len() );
/*N*/                 rDo.GetInf().SetText( aNewText );
/*N*/             }
/*N*/ 			else
/*N*/ 			{
/*N*/ 				rDo.GetInf().SetIdx( nOldPos );
/*N*/ 				rDo.GetInf().SetLen( nPos - nOldPos );
/*N*/ 			}
/*N*/ 
/*N*/ 			rDo.GetInf().SetUpper( FALSE );
/*N*/ 			rDo.GetInf().SetOut( *pOutSize );
/*N*/ 			aPartSize = pSmallFont->GetTextSize( rDo.GetInf() );
/*N*/ 			nKana += rDo.GetInf().GetKanaDiff();
/*N*/ 			rDo.GetInf().SetOut( *pOldOut );
/*N*/ 			if( nKern && nPos < nMaxPos )
/*?*/ 				aPartSize.Width() += nKern;
/*N*/ 			rDo.Do();
/*N*/ 			nOldPos = nPos;
/*N*/ 		}
/*N*/ 		nPos = (xub_StrLen)pBreakIt->xBreak->nextCharBlock( rOldText, nPos,
/*N*/ 			   pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
/*N*/ 		if( nPos == STRING_LEN || nPos > nMaxPos )
/*N*/ 			nPos = nMaxPos;
/*N*/ 		ASSERT( nPos, "nextCharBlock not implemented?" );
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if( !nPos )
/*N*/ 			nPos = nMaxPos;
/*N*/ #endif
/*N*/ 		// The upper ones...
/*N*/ 		if( nOldPos != nPos )
/*N*/ 		{
/*N*/ 			do
/*N*/ 			{
/*N*/ 				rDo.GetInf().SetUpper( TRUE );
/*N*/ 				pLastFont = pBigFont;
/*N*/ 				pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
/*N*/ 				xub_StrLen nTmp;
/*N*/ 				if( bWordWise )
/*N*/ 				{
/*?*/ 					nTmp = nOldPos;
/*?*/ 					while( nTmp < nPos && CH_BLANK == rOldText.GetChar( nTmp ) )
/*?*/ 						++nTmp;
/*?*/ 					if( nOldPos < nTmp )
/*?*/ 					{
                            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 						pLastFont = pSpaceFont;
/*?*/ 					}
/*?*/ 					while( nTmp < nPos && CH_BLANK != rOldText.GetChar( nTmp ) )
/*?*/ 						++nTmp;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nTmp = nPos;
/*N*/ 				if( nTmp > nOldPos )
/*N*/ 				{
/*N*/           			// #107816#, #i14820#
/*N*/                     if( bCaseMapLengthDiffers )
/*N*/                     {
/*N*/                         // Build an own 'changed' string for the given part of the
/*N*/                         // source string and use it. That new string may differ in length
/*N*/                         // from the source string.
/*N*/                         const XubString aSnippet( rOldText, nOldPos, nTmp - nOldPos);
/*N*/                         aNewText = CalcCaseMap( aSnippet );
/*N*/                         aCapInf.nIdx = nOldPos;
/*N*/                         aCapInf.nLen = nTmp - nOldPos;
/*N*/                         rDo.GetInf().SetIdx( 0 );
/*N*/                         rDo.GetInf().SetLen( aNewText.Len() );
/*N*/                         rDo.GetInf().SetText( aNewText );
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         rDo.GetInf().SetIdx( nOldPos );
/*N*/                         rDo.GetInf().SetLen( nPos - nOldPos );
/*N*/                     }
/*N*/ 
/*N*/ 					rDo.GetInf().SetOut( *pOutSize );
/*N*/ 					aPartSize = pBigFont->GetTextSize( rDo.GetInf() );
/*N*/ 					nKana += rDo.GetInf().GetKanaDiff();
/*N*/ 					rDo.GetInf().SetOut( *pOldOut );
/*N*/ 					if( !bWordWise && rDo.GetInf().GetSpace() )
/*?*/ 						for( xub_StrLen nI = nOldPos; nI < nPos; ++nI )
/*?*/ 							if( CH_BLANK == rOldText.GetChar( nI ) )
/*?*/ 							aPartSize.Width() += rDo.GetInf().GetSpace();
/*N*/ 					if( nKern && nPos < nMaxPos )
/*?*/ 						aPartSize.Width() += nKern;
/*N*/ 					rDo.Do();
/*N*/ 					nOldPos = nTmp;
/*N*/ 				}
/*N*/ 			} while( nOldPos != nPos );
/*N*/ 		}
/*N*/ 		nPos = (xub_StrLen)pBreakIt->xBreak->endOfCharBlock( rOldText, nPos,
/*N*/ 			   pBreakIt->GetLocale( eLng ), CharType::LOWERCASE_LETTER);
/*N*/ 		if( nPos == STRING_LEN || nPos > nMaxPos )
/*N*/ 			nPos = nMaxPos;
/*N*/ 		ASSERT( nPos, "endOfCharBlock not implemented?" );
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if( !nPos )
/*N*/ 			nPos = nMaxPos;
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	// Aufraeumen:
/*N*/ 	if( pBigFont != pOldLast )
/*N*/ 		delete pBigFontAccess;
/*N*/ 
/*N*/ 	if( bUnderStriked )
/*N*/ 	{
/*N*/ 		if( rDo.GetInf().GetDrawSpace() )
/*N*/ 		{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			pLastFont = pSpaceFont;
/*N*/ 		}
/*N*/ 		if ( bWordWise )
/*?*/ 			delete pSpaceFontAccess;
/*N*/ 	}
/*N*/ 	pLastFont =	pOldLast;
/*N*/ 	pLastFont->SetDevFont( rDo.GetInf().GetShell(), rDo.GetOut() );
/*N*/ 
/*N*/ 	delete pSmallFontAccess;
/*N*/ 	rDo.GetInf().SetText( rOldText );
/*N*/ 	rDo.GetInf().SetKanaDiff( nKana );
/*N*/ }

// JP 22.8.2001 - global optimization off - Bug 91245 / 91223
#ifdef _MSC_VER
#pragma optimize("g",on)
#endif


}
