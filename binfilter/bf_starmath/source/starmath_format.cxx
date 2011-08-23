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

#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <bf_svx/scripttypeitem.hxx>

#include "format.hxx"
namespace binfilter {

/////////////////////////////////////////////////////////////////

// Latin default-fonts
/*N*/ static const USHORT aLatinDefFnts[FNT_END] =
/*N*/ {
/*N*/     DEFAULTFONT_SERIF,  // FNT_VARIABLE
/*N*/     DEFAULTFONT_SERIF,  // FNT_FUNCTION
/*N*/     DEFAULTFONT_SERIF,  // FNT_NUMBER
/*N*/     DEFAULTFONT_SERIF,  // FNT_TEXT
/*N*/     DEFAULTFONT_SERIF,  // FNT_SERIF
/*N*/     DEFAULTFONT_SANS,   // FNT_SANS
/*N*/     DEFAULTFONT_FIXED   // FNT_FIXED
/*N*/     //StarSymbol,    // FNT_MATH
/*N*/ };

// CJK default-fonts
//! we use non-asian fonts for variables, functions and numbers since they
//! look better and even in asia only latin letters will be used for those.
//! At least that's what I was told...
/*N*/ static const USHORT aCJKDefFnts[FNT_END] =
/*N*/ {
/*N*/     DEFAULTFONT_SERIF,          // FNT_VARIABLE
/*N*/     DEFAULTFONT_SERIF,          // FNT_FUNCTION
/*N*/     DEFAULTFONT_SERIF,          // FNT_NUMBER
/*N*/     DEFAULTFONT_CJK_TEXT,       // FNT_TEXT
/*N*/     DEFAULTFONT_CJK_TEXT,       // FNT_SERIF
/*N*/     DEFAULTFONT_CJK_DISPLAY,    // FNT_SANS
/*N*/     DEFAULTFONT_CJK_TEXT        // FNT_FIXED
/*N*/     //StarSymbol,    // FNT_MATH
/*N*/ };

// CTL default-fonts
/*N*/ static const USHORT aCTLDefFnts[FNT_END] =
/*N*/ {
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_VARIABLE
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_FUNCTION
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_NUMBER
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_TEXT
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_SERIF
/*N*/     DEFAULTFONT_CTL_TEXT,    // FNT_SANS
/*N*/     DEFAULTFONT_CTL_TEXT     // FNT_FIXED
/*N*/     //StarSymbol,    // FNT_MATH
/*N*/ };


/*N*/ String GetDefaultFontName( LanguageType nLang, USHORT nIdent )
/*N*/ {
/*N*/     DBG_ASSERT( FNT_BEGIN <= nIdent  &&  nIdent <= FNT_END,
/*N*/             "index out opd range" );
/*N*/
/*N*/     if (FNT_MATH == nIdent)
/*?*/         return String::CreateFromAscii( FNTNAME_MATH );
/*N*/     else
/*N*/     {
/*N*/         const USHORT *pTable;
/*N*/         switch ( SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ) )
/*N*/         {
/*N*/             case SCRIPTTYPE_LATIN :     pTable = aLatinDefFnts; break;
/*?*/             case SCRIPTTYPE_ASIAN :     pTable = aCJKDefFnts; break;
/*?*/             case SCRIPTTYPE_COMPLEX :   pTable = aCTLDefFnts; break;
/*?*/             default :
/*?*/                 pTable = aLatinDefFnts;
/*?*/                 DBG_ERROR( "unknown script-type" );
/*N*/         }
/*N*/
/*N*/         return Application::GetDefaultDevice()->GetDefaultFont(
/*N*/                         pTable[ nIdent ], nLang,
/*N*/                         DEFAULTFONT_FLAGS_ONLYONE ).GetName();
/*N*/     }
/*N*/ }

/////////////////////////////////////////////////////////////////

/*N*/ SmFormat::SmFormat()
/*N*/ :	aBaseSize(0, SmPtsTo100th_mm(12))
/*N*/ {
/*N*/ 	nVersion 	= SM_FMT_VERSION_NOW;
/*N*/
/*N*/ 	eHorAlign   = AlignCenter;
/*N*/ 	bIsTextmode = bScaleNormalBrackets = FALSE;
/*N*/
/*N*/ 	vSize[SIZ_TEXT] 	= 100;
/*N*/ 	vSize[SIZ_INDEX]	= 60;
/*N*/ 	vSize[SIZ_FUNCTION] =
/*N*/ 	vSize[SIZ_OPERATOR] = 100;
/*N*/ 	vSize[SIZ_LIMITS]	= 60;
/*N*/
/*N*/ 	vDist[DIS_HORIZONTAL]	 		= 10;
/*N*/ 	vDist[DIS_VERTICAL] 	 		= 5;
/*N*/ 	vDist[DIS_ROOT] 		 		= 0;
/*N*/ 	vDist[DIS_SUPERSCRIPT]	 		=
/*N*/ 	vDist[DIS_SUBSCRIPT]	 		= 20;
/*N*/ 	vDist[DIS_NUMERATOR]	 		=
/*N*/ 	vDist[DIS_DENOMINATOR]	 		= 0;
/*N*/ 	vDist[DIS_FRACTION] 	 		= 10;
/*N*/ 	vDist[DIS_STROKEWIDTH]	 		= 5;
/*N*/ 	vDist[DIS_UPPERLIMIT]	 		=
/*N*/ 	vDist[DIS_LOWERLIMIT]    		= 0;
/*N*/ 	vDist[DIS_BRACKETSIZE]	 		=
/*N*/ 	vDist[DIS_BRACKETSPACE]  		= 5;
/*N*/ 	vDist[DIS_MATRIXROW]	 		= 3;
/*N*/ 	vDist[DIS_MATRIXCOL]	 		= 30;
/*N*/ 	vDist[DIS_ORNAMENTSIZE]  		=
/*N*/ 	vDist[DIS_ORNAMENTSPACE] 		= 0;
/*N*/ 	vDist[DIS_OPERATORSIZE]  		= 50;
/*N*/ 	vDist[DIS_OPERATORSPACE] 		= 20;
/*N*/ 	vDist[DIS_LEFTSPACE]	 		=
/*N*/ 	vDist[DIS_RIGHTSPACE]	 		= 100;
/*N*/ 	vDist[DIS_TOPSPACE]		 		=
/*N*/ 	vDist[DIS_BOTTOMSPACE]	 		=
/*N*/ 	vDist[DIS_NORMALBRACKETSIZE]	= 0;
/*N*/
/*N*/ 	vFont[FNT_VARIABLE]	=
/*N*/ 	vFont[FNT_FUNCTION]	=
/*N*/ 	vFont[FNT_NUMBER]	=
/*N*/ 	vFont[FNT_TEXT]		=
/*N*/ 	vFont[FNT_SERIF]	= SmFace(C2S(FNTNAME_TIMES), aBaseSize);
/*N*/ 	vFont[FNT_SANS]		= SmFace(C2S(FNTNAME_HELV),  aBaseSize);
/*N*/ 	vFont[FNT_FIXED]	= SmFace(C2S(FNTNAME_COUR),  aBaseSize);
/*N*/ 	vFont[FNT_MATH]		= SmFace(C2S(FNTNAME_MATH),  aBaseSize);
/*N*/
/*N*/     vFont[FNT_MATH].SetCharSet( RTL_TEXTENCODING_UNICODE );
/*N*/
/*N*/ 	vFont[FNT_VARIABLE].SetItalic(ITALIC_NORMAL);
/*N*/ 	vFont[FNT_FUNCTION].SetItalic(ITALIC_NONE);
/*N*/ 	vFont[FNT_TEXT].SetItalic(ITALIC_NONE);
/*N*/
/*N*/ 	for ( USHORT i = FNT_BEGIN;  i <= FNT_END;  i++ )
/*N*/ 	{
/*N*/         SmFace &rFace = vFont[i];
/*N*/         rFace.SetTransparent( TRUE );
/*N*/         rFace.SetAlign( ALIGN_BASELINE );
/*N*/         rFace.SetColor( COL_AUTO );
/*N*/         bDefaultFont[i] = FALSE;
/*N*/ 	}
/*N*/ }


/*N*/ void SmFormat::SetFont(USHORT nIdent, const SmFace &rFont, BOOL bDefault )
/*N*/ {
/*N*/     vFont[nIdent] = rFont;
/*N*/     vFont[nIdent].SetTransparent( TRUE );
/*N*/     vFont[nIdent].SetAlign( ALIGN_BASELINE );
/*N*/
/*N*/     bDefaultFont[nIdent] = bDefault;
/*N*/ }

/*N*/ SmFormat & SmFormat::operator = (const SmFormat &rFormat)
/*N*/ {
/*N*/ 	SetBaseSize(rFormat.GetBaseSize());
/*N*/ 	SetVersion (rFormat.GetVersion());
/*N*/ 	SetHorAlign(rFormat.GetHorAlign());
/*N*/ 	SetTextmode(rFormat.IsTextmode());
/*N*/ 	SetScaleNormalBrackets(rFormat.IsScaleNormalBrackets());
/*N*/
/*N*/ 	USHORT  i;
/*N*/ 	for (i = FNT_BEGIN;  i <= FNT_END;  i++)
/*N*/     {
/*N*/ 		SetFont(i, rFormat.GetFont(i));
/*N*/         SetDefaultFont(i, rFormat.IsDefaultFont(i));
/*N*/     }
/*N*/ 	for (i = SIZ_BEGIN;  i <= SIZ_END;  i++)
/*N*/ 		SetRelSize(i, rFormat.GetRelSize(i));
/*N*/ 	for (i = DIS_BEGIN;  i <= DIS_END;  i++)
/*N*/ 		SetDistance(i, rFormat.GetDistance(i));
/*N*/
/*N*/ 	return *this;
/*N*/ }


/*N*/ BOOL SmFormat::operator == (const SmFormat &rFormat) const
/*N*/ {
/*N*/     BOOL bRes = aBaseSize == rFormat.aBaseSize  &&
/*N*/                 eHorAlign == rFormat.eHorAlign  &&
/*N*/                 bIsTextmode == rFormat.bIsTextmode  &&
/*N*/                 bScaleNormalBrackets  == rFormat.bScaleNormalBrackets;
/*N*/
/*N*/     USHORT i;
/*N*/     for (i = 0;  i <= SIZ_END && bRes;  ++i)
/*N*/     {
/*N*/         if (vSize[i] != rFormat.vSize[i])
/*N*/             bRes = FALSE;
/*N*/     }
/*N*/     for (i = 0;  i <= DIS_END && bRes;  ++i)
/*N*/     {
/*N*/         if (vDist[i] != rFormat.vDist[i])
/*N*/             bRes = FALSE;
/*N*/     }
/*N*/     for (i = 0;  i <= FNT_END && bRes;  ++i)
/*N*/     {
/*N*/         if (vFont[i] != rFormat.vFont[i]  ||
/*N*/             bDefaultFont[i] != rFormat.bDefaultFont[i])
/*N*/             bRes = FALSE;
/*N*/     }
/*N*/
/*N*/     return bRes;
/*N*/ }


/*N*/ SvStream & operator << (SvStream &rStream, const SmFormat &rFormat)
/*N*/ {
/*N*/   //Da hier keinerlei Kompatibilitaet vorgesehen ist muessen wir leider
/*N*/ 	//heftig tricksen. Gluecklicherweise sind offenbar einige Informationen
/*N*/ 	//ueberfluessig geworden. In diese quetschen wir jetzt vier neue
/*N*/ 	//Einstellungen fuer die Rander.
/*N*/ 	//Bei Gelegenheit wird hier ein Im- Und Export gebraucht. Dann muessen
/*N*/ 	//die Stream-Operatoren dieser Klassen dringend mit Versionsverwaltung
/*N*/ 	//versehen werden!
/*N*/
/*N*/ 	UINT16	n;
/*N*/
/*N*/ 	// convert the heigth (in 100th of mm) to Pt and round the result to the
/*N*/ 	// nearest integer
/*N*/ 	n = (UINT16) SmRoundFraction(Sm100th_mmToPts(rFormat.aBaseSize.Height()));
/*N*/ 	DBG_ASSERT((n & 0xFF00) == 0, "Sm : higher Byte nicht leer");
/*N*/
/*N*/ 	// to be compatible with the old format (size and order) we put the info
/*N*/ 	// about textmode in the higher byte (height is already restricted to a
/*N*/ 	// maximum of 127!)
/*N*/ 	n |=   (rFormat.bIsTextmode != 0)          << 8
/*N*/ 		 | (rFormat.bScaleNormalBrackets != 0) << 9;
/*N*/ 	rStream << n;
/*N*/
/*N*/ 	rStream << rFormat.vDist[DIS_LEFTSPACE];	//Wir nutzen den Platz
/*N*/ 	rStream << rFormat.vDist[DIS_RIGHTSPACE];	//Wir nutzen den Platz
/*N*/
/*N*/ 	for ( n = SIZ_BEGIN; n <= SIZ_END; ++n )
/*N*/ 		rStream << rFormat.vSize[n];
/*N*/
/*N*/ 	rStream << rFormat.vDist[DIS_TOPSPACE];		//Wir nutzen den Platz
/*N*/
/*N*/ 	for ( n = 0; n <= FNT_FIXED; ++n )
/*N*/ 		rStream << rFormat.vFont[n];
/*N*/
/*N*/ 	// Den zweiten Wert noch im HigherByte unterbringen
/*N*/ 	USHORT uTmp =   rFormat.vDist[DIS_BRACKETSIZE]
/*N*/ 				  | rFormat.vDist[DIS_NORMALBRACKETSIZE] << 8;
/*N*/ 	// und dann dieses rausstreamen
/*N*/ 	for ( n = 0; n <= DIS_OPERATORSPACE; ++n )
/*N*/ 		rStream << (USHORT)(n != DIS_BRACKETSIZE ? rFormat.vDist[(USHORT) n] : uTmp);
/*N*/
/*N*/ 	// higher byte is version number, lower byte is horizontal alignment
/*N*/ 	n = rFormat.eHorAlign | SM_FMT_VERSION_NOW << 8;
/*N*/ 	rStream << n;
/*N*/
/*N*/ 	rStream << rFormat.vDist[DIS_BOTTOMSPACE];	//Wir nutzen den Platz
/*N*/
/*N*/ 	return rStream;
/*N*/ }


/*N*/ SvStream & operator >> (SvStream &rStream, SmFormat &rFormat)
/*N*/ {
/*N*/ 	UINT16	n;
/*N*/
/*N*/ 	rStream >> n;
/*N*/ 	long nBaseHeight    = n & 0x00FF;
/*N*/ 	rFormat.bIsTextmode 		 = ((n >> 8) & 0x01) != 0;
/*N*/ 	rFormat.bScaleNormalBrackets = ((n >> 9) & 0x01) != 0;
/*N*/ 	rFormat.aBaseSize   = Size(0, SmPtsTo100th_mm(nBaseHeight));
/*N*/
/*N*/ 	rStream >> rFormat.vDist[DIS_LEFTSPACE];	//Wir nutzen den Platz
/*N*/ 	rStream >> rFormat.vDist[DIS_RIGHTSPACE];	//Wir nutzen den Platz
/*N*/
/*N*/ 	for ( n = SIZ_BEGIN; n <= SIZ_END; ++n )
/*N*/ 		rStream >> rFormat.vSize[n];
/*N*/
/*N*/ 	rStream >> rFormat.vDist[DIS_TOPSPACE];		//Wir nutzen den Platz
/*N*/
/*N*/ 	for ( n = 0; n <= FNT_FIXED; ++n )
/*N*/ 		rStream >> rFormat.vFont[n];
/*N*/
/*N*/ 	for ( n = 0; n <= DIS_OPERATORSPACE; ++n )
/*N*/ 		rStream >> rFormat.vDist[n];
/*N*/ 	// den zweiten Wert aus dem HigherByte holen
/*N*/ 	rFormat.vDist[DIS_NORMALBRACKETSIZE] = rFormat.vDist[DIS_BRACKETSIZE] >> 8;
/*N*/ 	// und dieses dann ausblenden
/*N*/ 	rFormat.vDist[DIS_BRACKETSIZE] &= 0x00FF;
/*N*/
/*N*/ 	// higher byte is version number, lower byte is horizontal alignment
/*N*/ 	rStream >> n;
/*N*/ 	rFormat.nVersion  = n >> 8;
/*N*/ 	rFormat.eHorAlign = (SmHorAlign) (n & 0x00FF);
/*N*/
/*N*/ 	rStream >> rFormat.vDist[DIS_BOTTOMSPACE];	//Wir nutzen den Platz
/*N*/
/*N*/ 	const Size aTmp( rFormat.GetBaseSize() );
/*N*/ 	for ( USHORT i = 0; i <= FNT_FIXED; ++i )
/*N*/ 	{
/*N*/ 		rFormat.vFont[i].SetSize(aTmp);
/*N*/ 		rFormat.vFont[i].SetTransparent(TRUE);
/*N*/ 		rFormat.vFont[i].SetAlign(ALIGN_BASELINE);
/*N*/ 	}
/*N*/ 	rFormat.vFont[FNT_MATH].SetSize(aTmp);
/*N*/
/*N*/   // Fuer Version 4.0 (und aelter) sollen auch die normalen Klammern skalierbar
/*N*/ 	// sein und wachsen (so wie es der Fall war), in der 5.0 Version jedoch nicht.
/*N*/   // In spaeteren Versionen (>= 5.1) ist das Verhalten nun durch den Anwender
/*N*/ 	// festzulegen (bleibt also wie aus dem Stream gelesen).
/*N*/ 	if (rFormat.nVersion < SM_FMT_VERSION_51)
/*N*/ 	{
/*?*/ 		BOOL    bIs50Stream = rStream.GetVersion() == SOFFICE_FILEFORMAT_50;
/*?*/ 		BOOL	bVal        = bIs50Stream ? FALSE : TRUE;
/*?*/ 		USHORT  nExcHeight  = bIs50Stream ? 0 : rFormat.vDist[DIS_BRACKETSIZE];
/*?*/
/*?*/ 		rFormat.SetScaleNormalBrackets(bVal);
/*?*/ 		rFormat.SetDistance(DIS_NORMALBRACKETSIZE, nExcHeight);
/*N*/ 	}
/*N*/
/*N*/ 	return rStream;
/*N*/ }

/*?*/ void SmFormat::ReadSM20Format(SvStream &rStream)
/*?*/ {
/*?*/ 	UINT16	n;
/*?*/ 	USHORT  i;
/*?*/
/*?*/ 	rStream >> n;
/*?*/ 	SetBaseSize( Size(0, SmPtsTo100th_mm(n)) );
/*?*/
/*?*/ 	rStream >> n >> n;
/*?*/
/*?*/ 	for (i = SIZ_BEGIN;  i <= SIZ_LIMITS;  i++)
/*?*/ 	{	rStream >> n;
/*?*/ 		SetRelSize(i, n);
/*?*/ 	}
/*?*/
/*?*/ 	rStream >> n;
/*?*/
/*?*/ 	for (i = FNT_BEGIN;  i <= FNT_FIXED;  i++)
/*?*/         ReadSM20Font(rStream, vFont[i]);
/*?*/
/*?*/ 	for (i = DIS_BEGIN;  i <= DIS_OPERATORSPACE;  i++)
/*?*/ 	{	rStream >> n;
/*?*/ 		SetDistance(i, n);
/*?*/ 	}
/*?*/
/*?*/ 	rStream >> n;
/*?*/ 	SetHorAlign((SmHorAlign) n);
/*?*/ 	rStream >> n;
/*?*/
/*?*/ 	const Size  aTmp (GetBaseSize());
/*?*/ 	for (i = FNT_BEGIN;  i <= FNT_FIXED;  i++)
/*?*/ 	{
/*?*/         SmFace &rFace = vFont[i];
/*?*/         rFace.SetSize(aTmp);
/*?*/         rFace.SetTransparent(TRUE);
/*?*/         rFace.SetAlign(ALIGN_BASELINE);
/*?*/ 	}
/*?*/     vFont[FNT_MATH].SetSize(aTmp);
/*?*/ }


/*N*/ void SmFormat::From300To304a()
/*N*/ {
/*N*/ 	long nBaseSize = SmRoundFraction(Sm100th_mmToPts(aBaseSize.Height()))
/*N*/ 					 * 2540l / 72l;
/*N*/ 	for (USHORT i = DIS_BEGIN;  i < DIS_OPERATORSPACE;  i++)
/*N*/ 		SetDistance(i, USHORT(GetDistance(i) * 254000L / 72L / nBaseSize));
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
