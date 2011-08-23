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

// include ----------------------------------------------------------------

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_ESCAPEMENT	0

#define _SVX_SVXFONT_CXX

#include "svxfont.hxx"
namespace binfilter {

// Minimum: Prozentwert fuers kernen
#define MINKERNPERCENT 5

// prop. Groesse der Kleinbuchstaben bei Kapitaelchen
#define KAPITAELCHENPROP 66

#ifndef REDUCEDSVXFONT
    const sal_Unicode CH_BLANK = sal_Unicode(' ');  	// ' ' Leerzeichen
    static sal_Char __READONLY_DATA sDoubleSpace[] = "  ";
#endif

/*************************************************************************
 *						class SvxFont
 *************************************************************************/

/*N*/ SvxFont::SvxFont()
/*N*/ {
/*N*/ 	nKern = nEsc = 0;
/*N*/ 	nPropr = 100;
/*N*/ 	eCaseMap = SVX_CASEMAP_NOT_MAPPED;
/*N*/ 	eLang = LANGUAGE_SYSTEM;
/*N*/ }

/*N*/ SvxFont::SvxFont( const Font &rFont )
/*N*/ 	: Font( rFont )
/*N*/ {
/*N*/ 	nKern = nEsc = 0;
/*N*/ 	nPropr = 100;
/*N*/ 	eCaseMap = SVX_CASEMAP_NOT_MAPPED;
/*N*/ 	eLang = LANGUAGE_SYSTEM;
/*N*/ }

/*************************************************************************
 *						class SvxFont: Copy-Ctor
 *************************************************************************/

/*N*/ SvxFont::SvxFont( const SvxFont &rFont )
/*N*/ 	: Font( rFont )
/*N*/ {
/*N*/ 	nKern = rFont.GetFixKerning();
/*N*/ 	nEsc  = rFont.GetEscapement();
/*N*/ 	nPropr = rFont.GetPropr();
/*N*/ 	eCaseMap = rFont.GetCaseMap();
/*N*/ 	eLang = rFont.GetLanguage();
/*N*/ }

/*************************************************************************
 *				 static	SvxFont::DrawArrow
 *************************************************************************/


/*************************************************************************
 *                      SvxFont::CalcCaseMap
 *************************************************************************/

/*N*/ XubString SvxFont::CalcCaseMap( const XubString &rTxt ) const
/*N*/ {
/*N*/ 	if( !IsCaseMap() || !rTxt.Len() ) return rTxt;
/*N*/ 	XubString aTxt( rTxt );
/*N*/ 	// Ich muss mir noch die Sprache besorgen
/*N*/ 	const LanguageType eLng = LANGUAGE_DONTKNOW == eLang
/*N*/ 							? LANGUAGE_SYSTEM : eLang;
/*N*/ 
/*N*/ 	CharClass aCharClass( SvxCreateLocale( eLng ) );
/*N*/ 
/*N*/ 	switch( eCaseMap )
/*N*/ 	{
/*N*/ 		case SVX_CASEMAP_KAPITAELCHEN:
/*N*/ 		case SVX_CASEMAP_VERSALIEN:
/*N*/ 		{
/*N*/ 			aCharClass.toUpper( aTxt );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		case SVX_CASEMAP_GEMEINE:
/*N*/ 		{
/*?*/ 			aCharClass.toLower( aTxt );
/*?*/ 			break;
/*?*/ 		}
/*?*/ 		case SVX_CASEMAP_TITEL:
/*?*/ 		{
/*?*/ 			// Jeder Wortbeginn wird gross geschrieben,
/*?*/ 			// der Rest des Wortes wird unbesehen uebernommen.
/*?*/ 			// Bug: wenn das Attribut mitten im Wort beginnt.
/*?*/ 			BOOL bBlank = TRUE;
/*?*/ 
/*?*/ 			for( UINT32 i = 0; i < aTxt.Len(); ++i )
/*?*/ 			{
/*?*/ 				if( sal_Unicode(' ') == aTxt.GetChar(i) || sal_Unicode('\t') == aTxt.GetChar(i) )
/*?*/ 					bBlank = TRUE;
/*?*/ 				else
/*?*/ 				{
/*?*/ 					if( bBlank )
/*?*/ 					{
/*?*/ 						String aTemp( aTxt.GetChar( i ) );
/*?*/ 						aCharClass.toUpper( aTemp );
/*?*/ 						aTxt.Replace( i, 1, aTemp );
/*?*/ 					}
/*?*/ 					bBlank = FALSE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		}
/*?*/ 		default:
/*?*/ 		{
/*?*/ 			DBG_ASSERT(!this, "SvxFont::CaseMapTxt: unknown casemap");
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aTxt;
/*N*/ }

/*************************************************************************
 * Hier beginnen die Methoden, die im Writer nicht benutzt werden koennen,
 * deshalb kann man diesen Bereich durch setzen von REDUCEDSVXFONT ausklammern.
 *************************************************************************/
#ifndef REDUCEDSVXFONT

/*************************************************************************
 *						class SvxDoCapitals
 * die virtuelle Methode Do wird von SvxFont::DoOnCapitals abwechselnd mit
 * den "Gross-" und "Kleinbuchstaben"-Teilen aufgerufen.
 * Die Ableitungen von SvxDoCapitals erfuellen diese Methode mit Leben.
 *************************************************************************/

class SvxDoCapitals
{
protected:
    OutputDevice *pOut;
    const XubString &rTxt;
    const xub_StrLen nIdx;
    const xub_StrLen nLen;

public:
    SvxDoCapitals( OutputDevice *pOut, const XubString &rTxt,
                   const xub_StrLen nIdx, const xub_StrLen nLen )
        : pOut(pOut), rTxt(rTxt), nIdx(nIdx), nLen(nLen)
        { }


    inline OutputDevice *GetOut() { return pOut; }
    inline const XubString &GetTxt() const { return rTxt; }
    xub_StrLen GetIdx() const { return nIdx; }
    xub_StrLen GetLen() const { return nLen; }
};




/*************************************************************************
 *					SvxFont::DoOnCapitals() const
 * zerlegt den String in Gross- und Kleinbuchstaben und ruft jeweils die
 * Methode SvxDoCapitals::Do( ) auf.
 *************************************************************************/


/**************************************************************************
 *					  SvxFont::SetPhysFont()
 *************************************************************************/

/*N*/ void SvxFont::SetPhysFont( OutputDevice *pOut ) const
/*N*/ {
/*N*/ 	const Font& rCurrentFont = pOut->GetFont();
/*N*/ 	if ( nPropr == 100 )
/*N*/ 	{
/*N*/ 		if ( !rCurrentFont.IsSameInstance( *this ) )
/*N*/ 			pOut->SetFont( *this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Font aNewFont( *this );
/*N*/ 		Size aSize( aNewFont.GetSize() );
/*N*/ 		aNewFont.SetSize( Size(	aSize.Width() * nPropr / 100L,
/*N*/ 									aSize.Height() * nPropr / 100L ) );
/*N*/ 		if ( !rCurrentFont.IsSameInstance( aNewFont ) )
/*N*/ 			pOut->SetFont( aNewFont );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					  SvxFont::ChgPhysFont()
 *************************************************************************/


/*************************************************************************
 *					  SvxFont::GetPhysTxtSize()
 *************************************************************************/


/*N*/ Size SvxFont::GetPhysTxtSize( const OutputDevice *pOut, const XubString &rTxt )
/*N*/ {
/*N*/ 	if ( !IsCaseMap() && !IsKern() )
/*N*/ 		return Size( pOut->GetTextWidth( rTxt ), pOut->GetTextHeight() );
/*N*/ 
/*?*/ 	Size aTxtSize;
/*?*/ 	aTxtSize.setHeight( pOut->GetTextHeight() );
/*?*/ 	if ( !IsCaseMap() )
/*?*/ 		aTxtSize.setWidth( pOut->GetTextWidth( rTxt ) );
/*?*/ 	else
/*?*/ 		aTxtSize.setWidth( pOut->GetTextWidth( CalcCaseMap( rTxt ) ) );
/*?*/ 
/*?*/ 	if( IsKern() && ( rTxt.Len() > 1 ) )
/*?*/ 		aTxtSize.Width() += ( ( rTxt.Len()-1 ) * long( nKern ) );
/*?*/ 
/*?*/ 	return aTxtSize;
/*N*/ }

/*N*/ Size SvxFont::QuickGetTextSize( const OutputDevice *pOut, const XubString &rTxt,
/*N*/ 						 const USHORT nIdx, const USHORT nLen, sal_Int32* pDXArray ) const
/*N*/ {
/*N*/ 	if ( !IsCaseMap() && !IsKern() )
/*N*/ 		return Size( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ),
/*N*/ 					 pOut->GetTextHeight() );
/*N*/ 
/*N*/ 	Size aTxtSize;
/*N*/ 	aTxtSize.setHeight( pOut->GetTextHeight() );
/*N*/ 	if ( !IsCaseMap() )
/*N*/ 		aTxtSize.setWidth( pOut->GetTextArray( rTxt, pDXArray, nIdx, nLen ) );
/*N*/ 	else
/*?*/ 		aTxtSize.setWidth( pOut->GetTextArray( CalcCaseMap( rTxt ),
/*?*/ 						   pDXArray, nIdx, nLen ) );
/*N*/ 
/*N*/ 	if( IsKern() && ( nLen > 1 ) )
/*N*/ 	{
/*N*/ 		aTxtSize.Width() += ( ( nLen-1 ) * long( nKern ) );
/*N*/ 
/*N*/ 		if ( pDXArray )
/*N*/ 		{
/*N*/ 			for ( xub_StrLen i = 0; i < nLen; i++ )
/*N*/ 				pDXArray[i] += ( (i+1) * long( nKern ) );
/*N*/ 			// Der letzte ist um ein nKern zu gross:
/*N*/ 			pDXArray[nLen-1] -= nKern;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aTxtSize;
/*N*/ }

/*************************************************************************
 *					  SvxFont::GetTxtSize()
 *************************************************************************/


/*************************************************************************
 *					  SvxFont::DrawText()
 *************************************************************************/


/*N*/ void SvxFont::QuickDrawText( OutputDevice *pOut,
/*N*/ 	const Point &rPos, const XubString &rTxt,
/*N*/ 	const xub_StrLen nIdx, const xub_StrLen nLen, const sal_Int32* pDXArray ) const
/*N*/ {
/*N*/ 	// Font muss ins OutputDevice selektiert sein...
/*N*/ 	if ( !IsCaseMap() && !IsCapital() && !IsKern() && !IsEsc() )
/*N*/ 	{
/*N*/ 		pOut->DrawTextArray( rPos, rTxt, pDXArray, nIdx, nLen );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*?*/ 	Point aPos( rPos );
/*?*/ 
/*?*/     if ( nEsc )
/*?*/     {
/*?*/         long nDiff = GetSize().Height();
/*?*/         nDiff *= nEsc;
/*?*/         nDiff /= 100;
/*?*/ 
/*?*/         if ( !IsVertical() )
/*?*/ 		    aPos.Y() -= nDiff;
/*?*/         else
/*?*/             aPos.X() += nDiff;
/*?*/     }
/*?*/ 
/*?*/ 	if( IsCapital() )
/*?*/ 	{
/*?*/		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		DBG_ASSERT( !pDXArray, "DrawCapital nicht fuer TextArray!" );
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		if ( IsKern() && !pDXArray )
/*?*/ 		{
/*?*/			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			Size aSize = GetPhysTxtSize( pOut, rTxt, nIdx, nLen );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			if ( !IsCaseMap() )
/*?*/ 				pOut->DrawTextArray( aPos, rTxt, pDXArray, nIdx, nLen );
/*?*/ 			else
/*?*/ 				pOut->DrawTextArray( aPos, CalcCaseMap( rTxt ), pDXArray, nIdx, nLen );
/*?*/ 		}
/*?*/ 	}
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SvxFont& SvxFont::operator=( const Font& rFont )
/*N*/ {
/*N*/ 	Font::operator=( rFont );
/*N*/ 	return *this;
/*N*/ }

/*N*/ SvxFont& SvxFont::operator=( const SvxFont& rFont )
/*N*/ {
/*N*/ 	Font::operator=( rFont );
/*N*/ 	eLang = rFont.eLang;
/*N*/ 	eCaseMap = rFont.eCaseMap;
/*N*/ 	nEsc = rFont.nEsc;
/*N*/ 	nPropr = rFont.nPropr;
/*N*/ 	nKern = rFont.nKern;
/*N*/ 	return *this;
/*N*/ }


/*************************************************************************
 *					  class SvxDoGetCapitalSize
 * wird von SvxFont::GetCapitalSize() zur Berechnung der TxtSize bei
 * eingestellten Kapitaelchen benutzt.
 *************************************************************************/

class SvxDoGetCapitalSize : public SvxDoCapitals
{
protected:
    SvxFont*	pFont;
    Size 		aTxtSize;
    short    	nKern;
public:
      SvxDoGetCapitalSize( SvxFont *pFnt, const OutputDevice *pOut,
                           const XubString &rTxt, const xub_StrLen nIdx,
                           const xub_StrLen nLen, const short nKrn )
            : SvxDoCapitals( (OutputDevice*)pOut, rTxt, nIdx, nLen ),
              pFont( pFnt ),
              nKern( nKrn )
            { }


    inline const Size &GetSize() const { return aTxtSize; };
};


/*************************************************************************
 *					  SvxFont::GetCapitalSize()
 * berechnet TxtSize, wenn Kapitaelchen eingestellt sind.
 *************************************************************************/


/*************************************************************************
 *					   class SvxDoDrawCapital
 * wird von SvxFont::DrawCapital zur Ausgabe von Kapitaelchen benutzt.
 *************************************************************************/

class SvxDoDrawCapital : public SvxDoCapitals
{
protected:
    SvxFont *pFont;
    Point aPos;
    Point aSpacePos;
    short nKern;
public:
    SvxDoDrawCapital( SvxFont *pFnt, OutputDevice *pOut, const XubString &rTxt,
                      const xub_StrLen nIdx, const xub_StrLen nLen,
                      const Point &rPos, const short nKrn )
        : SvxDoCapitals( pOut, rTxt, nIdx, nLen ),
          pFont( pFnt ),
          aPos( rPos ),
          aSpacePos( rPos ),
          nKern( nKrn )
        { }
};




/*************************************************************************
 * SvxFont::DrawCapital() gibt Kapitaelchen aus.
 *************************************************************************/


#endif // !REDUCEDSVXFONT


}
