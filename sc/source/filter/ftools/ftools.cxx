/*************************************************************************
 *
 *  $RCSfile: ftools.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:18:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#include <math.h>

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif


#include "excdefs.hxx"


// ============================================================================
// ScFilterTools::ReadLongDouble()

#ifdef _MSC_VER
#if _MSC_VER <= 800
#undef __SIMPLE_FUNC
#define __SIMPLE_FUNC
#endif
#endif

double ScfTools::ReadLongDouble( SvStream& rStrm )

#ifdef __SIMPLE_FUNC                // for <=VC 1.5
{
    long double fRet;
    rStrm.Read( &fRet, 10 );
    return static_cast< double >( fRet );
}
#undef __SIMPLE_FUNC

#else                               // detailed for all others
{

/*
" M a p p i n g - G u i d e " 10-Byte Intel

77777777 77666666 66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000   x10
98765432 10987654 32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit-# total
9      9 8      8 7      7 6      6 5      5 4      4 3      3 2      2 1      1 0      0   Byte-#
76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210   Bit-# in Byte
SEEEEEEE EEEEEEEE IMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM   Group
01111110 00000000 06665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000       x10
14321098 76543210 02109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210   Bit in Group
*/

    register long double lfDouble = 0.0;
    register long double lfFakt = 256.0;
    sal_uInt8 pDouble10[ 10 ];

    rStrm.Read( pDouble10, 10 );            // Intel-10 in pDouble10

    lfDouble  = static_cast< long double >( pDouble10[ 7 ] );   // Byte 7
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 6 ] );   // Byte 6
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 5 ] );   // Byte 5
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 4 ] );   // Byte 4
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 3 ] );   // Byte 3
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 2 ] );   // Byte 2
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 1 ] );   // Byte 1
    lfDouble *= lfFakt;
    lfDouble += static_cast< long double >( pDouble10[ 0 ] );   // Byte 0

    //  For value 0.0 all bits are zero; pow(2.0,-16446) does not work with CSet compilers
    if( lfDouble != 0.0 )
    {
        // exponent
        register sal_Int32 nExp;
        nExp = pDouble10[ 9 ] & 0x7F;
        nExp <<= 8;
        nExp += pDouble10[ 8 ];
        nExp -= 16446;

        lfDouble *= pow( 2.0, static_cast< double >( nExp ) );
    }

    // sign
    if( pDouble10[ 9 ] & 0x80 )
        lfDouble *= static_cast< long double >( -1.0 );

    return static_cast< double >( lfDouble );
}
#endif


// *** common methods *** -----------------------------------------------------

CharSet ScfTools::GetSystemCharSet()
{
    return gsl_getSystemTextEncoding();
}

String ScfTools::GetHexStr( sal_uInt16 nValue )
{
    const sal_Char pHex[] = "0123456789ABCDEF";
    String aStr;

    aStr += pHex[ nValue >> 12 ];
    aStr += pHex[ (nValue >> 8) & 0x000F ];
    aStr += pHex[ (nValue >> 4) & 0x000F ];
    aStr += pHex[ nValue & 0x000F ];
    return aStr;
}

sal_uInt8 ScfTools::GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt16 nTrans )
{
    sal_Int32 nTemp = ((static_cast< sal_Int32 >( nBack ) - nFore) * nTrans) / 0x8000 + nFore;
    return static_cast< sal_uInt8 >( nTemp );
}

Color ScfTools::GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt16 nTrans )
{
    return Color(
        GetMixedColorComp( rFore.GetRed(), rBack.GetRed(), nTrans ),
        GetMixedColorComp( rFore.GetGreen(), rBack.GetGreen(), nTrans ),
        GetMixedColorComp( rFore.GetBlue(), rBack.GetBlue(), nTrans ) );
}


// *** conversion of names *** ------------------------------------------------

void ScfTools::ConvertToScSheetName( String& rName )
{
    for( xub_StrLen nPos = 0, nLen = rName.Len(); nPos < nLen; ++nPos )
    {
        sal_Unicode cChar = rName.GetChar( nPos );
        bool bSpace = (cChar == ' ');
        if( (!ScGlobal::pCharClass->isLetterNumeric( rName, nPos ) && !bSpace) || (!nPos && bSpace) )
            rName.SetChar( nPos, '_' );
    }
}

void ScfTools::ConvertToScDefinedName( String& rName )
{
    xub_StrLen nLen = rName.Len();
    if( nLen && !ScCompiler::IsCharWordChar( rName.GetChar( 0 ) ) )
        rName.SetChar( 0, '_' );
    for( xub_StrLen nPos = 1; nPos < nLen; ++nPos )
        if( !ScCompiler::IsWordChar( rName.GetChar( nPos ) ) )
            rName.SetChar( nPos, '_' );
}


// *** streams and storages *** -----------------------------------------------

const SvStorageStreamRef ScfTools::OpenStorageStreamRead( SvStorage* pStorage, const String& rStrmName )
{
    SvStorageStreamRef xStrm;
    if( pStorage && pStorage->IsContained( rStrmName ) && pStorage->IsStream( rStrmName ) )
        xStrm = pStorage->OpenStream( rStrmName, STREAM_READ | STREAM_SHARE_DENYALL );
    return xStrm;
}

const SvStorageStreamRef ScfTools::OpenStorageStreamWrite( SvStorage* pStorage, const String& rStrmName )
{
    SvStorageStreamRef xStrm;
    if( pStorage )
        xStrm = pStorage->OpenStream( rStrmName/*, STREAM_READWRITE | STREAM_TRUNC*/ );
    return xStrm;
}



// *** item handling *** ------------------------------------------------------

bool ScfTools::CheckItem( const SfxItemSet& rItemSet, sal_uInt16 nWhichId, bool bDeep )
{
    return rItemSet.GetItemState( nWhichId, bDeep ) == SFX_ITEM_SET;
}


// *** style sheet handling *** -----------------------------------------------

ScStyleSheet& ScfTools::MakeCellStyleSheet( ScStyleSheetPool& rPool, const String& rStyleName, bool bForceName )
{
    // find an unused name
    String aNewName( rStyleName );
    sal_Int32 nIndex = 0;
    SfxStyleSheetBase* pOldStyleSheet = NULL;
    while( SfxStyleSheetBase* pStyleSheet = rPool.Find( aNewName, SFX_STYLE_FAMILY_PARA ) )
    {
        if( !pOldStyleSheet )
            pOldStyleSheet = pStyleSheet;
        aNewName.Assign( rStyleName ).Append( ' ' ).Append( String::CreateFromInt32( ++nIndex ) );
    }

    // rename existing style
    if( pOldStyleSheet && bForceName )
    {
        pOldStyleSheet->SetName( aNewName );
        aNewName = rStyleName;
    }

    // create new style sheet
    return static_cast< ScStyleSheet& >( rPool.Make( aNewName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_USERDEF ) );
}


// *** byte string import operations *** --------------------------------------

ByteString ScfTools::ReadCString( SvStream& rStrm )
{
    ByteString aRet;
    sal_Char cChar;

    rStrm >> cChar;
    while( cChar )
    {
        aRet += cChar;
        rStrm >> cChar;
    }
    return aRet;
}

ByteString ScfTools::ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft )
{
    ByteString aRet;
    sal_Char cChar;

    rStrm >> cChar;
    rnBytesLeft--;
    while( cChar )
    {
        aRet += cChar;
        rStrm >> cChar;
        rnBytesLeft--;
    }

    return aRet;
}

void ScfTools::AppendCString( SvStream& rStrm, ByteString& rString )
{
    sal_Char cChar;

    rStrm >> cChar;
    while( cChar )
    {
        rString += cChar;
        rStrm >> cChar;
    }
}

void ScfTools::AppendCString( SvStream& rStrm, String& rString, CharSet eSrc )
{
    ByteString aByteString;
    AppendCString( rStrm, aByteString );
    rString += String( aByteString, eSrc );
}


// *** HTML table names <-> named range names *** -----------------------------

const String& ScfTools::GetHTMLDocName()
{
    static const String saHTMLDoc( RTL_CONSTASCII_USTRINGPARAM( "HTML_all" ) );
    return saHTMLDoc;
}

const String& ScfTools::GetHTMLTablesName()
{
    static const String saHTMLTables( RTL_CONSTASCII_USTRINGPARAM( "HTML_tables" ) );
    return saHTMLTables;
}

const String& ScfTools::GetHTMLIndexPrefix()
{
    static const String saHTMLIndexPrefix( RTL_CONSTASCII_USTRINGPARAM( "HTML_" ) );
    return saHTMLIndexPrefix;

}

const String& ScfTools::GetHTMLNamePrefix()
{
    static const String saHTMLNamePrefix( RTL_CONSTASCII_USTRINGPARAM( "HTML__" ) );
    return saHTMLNamePrefix;
}

String ScfTools::GetNameFromHTMLIndex( sal_uInt32 nIndex )
{
    String aName( GetHTMLIndexPrefix() );
    aName += String::CreateFromInt32( static_cast< sal_Int32 >( nIndex ) );
    return aName;
}

String ScfTools::GetNameFromHTMLName( const String& rTabName )
{
    String aName( GetHTMLNamePrefix() );
    aName += rTabName;
    return aName;
}

bool ScfTools::IsHTMLDocName( const String& rSource )
{
    return rSource.EqualsIgnoreCaseAscii( GetHTMLDocName() ) == TRUE;
}

bool ScfTools::IsHTMLTablesName( const String& rSource )
{
    return rSource.EqualsIgnoreCaseAscii( GetHTMLTablesName() ) == TRUE;
}

bool ScfTools::GetHTMLNameFromName( const String& rSource, String& rName )
{
    rName.Erase();
    if( rSource.EqualsIgnoreCaseAscii( GetHTMLNamePrefix(), 0, GetHTMLNamePrefix().Len() ) )
    {
        rName = rSource.Copy( GetHTMLNamePrefix().Len() );
        ScGlobal::AddQuotes( rName );
    }
    else if( rSource.EqualsIgnoreCaseAscii( GetHTMLIndexPrefix(), 0, GetHTMLIndexPrefix().Len() ) )
    {
        String aIndex( rSource.Copy( GetHTMLIndexPrefix().Len() ) );
        if( CharClass::isAsciiNumeric( aIndex ) && (aIndex.ToInt32() > 0) )
            rName = aIndex;
    }
    return rName.Len() > 0;
}


// ============================================================================

