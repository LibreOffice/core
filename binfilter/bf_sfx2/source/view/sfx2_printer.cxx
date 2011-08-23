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

#include <bf_svtools/itemset.hxx>
#include <bf_svtools/svarray.hxx>

#include "bf_sfx2/printer.hxx"
namespace binfilter {

//--------------------------------------------------------------------

/*N*/ SV_DECL_PTRARR_DEL(SfxFontArr_Impl,SfxFont*,10,5)

// struct SfxPrinter_Impl ------------------------------------------------

/*N*/ struct SfxPrinter_Impl
/*N*/ {
/*N*/ 	SfxFontArr_Impl*	mpFonts;
/*N*/ 	BOOL				mbAll;
/*N*/ 	BOOL				mbSelection;
/*N*/ 	BOOL				mbFromTo;
/*N*/ 	BOOL				mbRange;
/*N*/
/*N*/ 	SfxPrinter_Impl() :
/*N*/ 		mpFonts		( NULL ),
/*N*/ 		mbAll		( TRUE ),
/*N*/ 		mbSelection ( TRUE ),
/*N*/ 		mbFromTo	( TRUE ),
/*N*/ 		mbRange 	( TRUE ) {}
/*N*/ 	~SfxPrinter_Impl() { delete mpFonts; }
/*N*/ };

/*N*/ #define FONTS()	pImpl->mpFonts

//--------------------------------------------------------------------

/*N*/ SfxFont::SfxFont( const FontFamily eFontFamily, const String& aFontName,
/*N*/ 				  const FontPitch eFontPitch, const CharSet eFontCharSet ):
/*N*/ 	aName( aFontName ),
/*N*/ 	eFamily( eFontFamily ),
/*N*/ 		ePitch( eFontPitch ),
/*N*/ 	eCharSet( eFontCharSet )
/*N*/ {
/*N*/ }

// class SfxPrinter ------------------------------------------------------

/*N*/ SfxPrinter* SfxPrinter::Create( SvStream& rStream, SfxItemSet* pOptions )

/* 	[Beschreibung]

    Erzeugt einen <SfxPrinter> aus dem Stream. Geladen wird genaugenommen
    nur ein JobSetup. Falls ein solcher Drucker auf dem System nicht
    verf"augbar ist, wird das Original als Orig-JobSetup gemerkt und
    ein "anhlicher exisitierender Drucker genommen.

    Die 'pOptions' werden in den erzeugten SfxPrinter "ubernommen,
    der Returnwert geh"ort dem Caller.
*/

/*N*/ {
/*N*/ 	// JobSetup laden
/*N*/ 	JobSetup aFileJobSetup;
/*N*/ 	rStream >> aFileJobSetup;
/*N*/
/*N*/ 	// Drucker erzeugen
/*N*/ 	SfxPrinter *pPrinter = new SfxPrinter( pOptions, aFileJobSetup );
/*N*/ 	return pPrinter;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SvStream& SfxPrinter::Store( SvStream& rStream ) const

/*	[Beschreibung]

    Speichert das verwendete JobSetup des <SfxPrinter>s.
*/

/*N*/ {
/*N*/ 	return ( rStream << GetJobSetup() );
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions ) :

/*  [Beschreibung]

    Dieser Ctor erzeugt einen Standard-Drucker.
*/

/*N*/ 	pOptions( pTheOptions ),
/*N*/ 	bKnown(sal_True)

/*N*/ {
/*N*/ 	pImpl = new SfxPrinter_Impl;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
/*N*/ 						const JobSetup& rTheOrigJobSetup ) :

/*N*/ 	Printer			( rTheOrigJobSetup.GetPrinterName() ),
/*N*/ 	pOptions		( pTheOptions )

/*N*/ {
/*N*/ 	pImpl = new SfxPrinter_Impl;
/*N*/ 	bKnown = GetName() == rTheOrigJobSetup.GetPrinterName();
/*N*/
/*N*/   if ( bKnown )
        {
/*N*/ 		SetJobSetup( rTheOrigJobSetup );
        }
        
        // --> FME 2006-09-19 #b6449032# Use old XPrinter emulation. rTheOrigJobSetup
        // already has this setting (see SfxPrinter::Create()).
        JobSetup aJobSetup( GetJobSetup() );
        aJobSetup.SetValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StrictSO52Compatibility" ) ),
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) ) );
        SetJobSetup( aJobSetup );
        // <--
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
/*N*/ 						const String& rPrinterName ) :
/*N*/
/*N*/ 	Printer			( rPrinterName ),
/*N*/ 	pOptions		( pTheOptions ),
/*N*/ 	bKnown			( GetName() == rPrinterName )
/*N*/
/*N*/ {
/*N*/ 	pImpl = new SfxPrinter_Impl;
/*N*/ }

//--------------------------------------------------------------------


//--------------------------------------------------------------------


//--------------------------------------------------------------------

/*N*/ SfxPrinter::~SfxPrinter()
/*N*/ {
/*N*/ 	delete pOptions;
/*N*/ 	delete pImpl;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
/*N*/ {
/*N*/ 	pOptions->Set(rNewOptions);
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SV_IMPL_PTRARR(SfxFontArr_Impl,SfxFont*)

//--------------------------------------------------------------------

/*?*/ const SfxFont* SfxPrinter::GetFontByName( const String &rFontName )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001
/*?*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
