/*************************************************************************
 *
 *  $RCSfile: zformat.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: er $ $Date: 2000-11-23 13:00:06 $
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

#pragma hdrstop

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CALENDARFIELDINDEX_HPP_
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_CALENDARDISPLAYINDEX_HPP_
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_AMPMVALUE_HPP_
#include <com/sun/star/i18n/AmPmValue.hpp>
#endif

#define _ZFORMAT_CXX
#include "zformat.hxx"
#include "zforscan.hxx"

#include "zforfind.hxx"
#include "zforlist.hxx"
#include "numhead.hxx"


const double _D_MAX_U_LONG_ = (double) 0xffffffff;      // 4294967295.0
const double _D_MAX_LONG_   = (double) 0x7fffffff;      // 2147483647.0
const USHORT _MAX_FRACTION_PREC = 3;
const double D_EPS = 1.0E-2;

#ifdef S390
const double _D_MAX_D_BY_100  = 7.2E73;
const double _D_MIN_M_BY_1000 = 5.4E-76;
#else
const double _D_MAX_D_BY_100  = 1.7E306;
const double _D_MIN_M_BY_1000 = 2.3E-305;
#endif

static BYTE cCharWidths[ 128-32 ] = {
    1,1,1,2,2,3,2,1,1,1,1,2,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,
    3,2,2,2,2,2,2,3,2,1,2,2,2,3,3,3,
    2,3,2,2,2,2,2,3,2,2,2,1,1,1,2,2,
    1,2,2,2,2,2,1,2,2,1,1,2,1,3,2,2,
    2,2,1,2,1,2,2,2,2,2,2,1,1,1,2,1
};

static xub_StrLen InsertBlanks( String& r, xub_StrLen nPos, sal_Unicode c )
{
    if( c >= 32 )
    {
        USHORT n = 2;   // Default fuer Zeichen > 128 (HACK!)
        if( c <= 127 )
            n = cCharWidths[ c - 32 ];
        while( n-- )
            r.Insert( ' ', nPos++ );
    }
    return nPos;
}

static long GetPrecExp( double fAbsVal )
{
    DBG_ASSERT( fAbsVal > 0.0, "GetPrecExp: fAbsVal <= 0.0" );
    if ( fAbsVal < 1e-7 || fAbsVal > 1e7 )
    {   // die Schere, ob's schneller ist oder nicht, liegt zwischen 1e6 und 1e7
        return (long) floor( log10( fAbsVal ) ) + 1;
    }
    else
    {
        long nPrecExp = 1;
        while( fAbsVal < 1 )
        {
            fAbsVal *= 10;
            nPrecExp--;
        }
        while( fAbsVal >= 10 )
        {
            fAbsVal /= 10;
            nPrecExp++;
        }
        return nPrecExp;
    }
}

const USHORT nNewCurrencyVersionId = 0x434E;    // "NC"
const sal_Unicode cNewCurrencyMagic = 0x01;         // Magic fuer Format im Kommentar

/***********************Funktion SvNumberformatInfo******************************/

void ImpSvNumberformatInfo::Copy( const ImpSvNumberformatInfo& rNumFor, USHORT nAnz )
{
    for (USHORT i = 0; i < nAnz; i++)
    {
        sStrArray[i]  = rNumFor.sStrArray[i];
        nTypeArray[i] = rNumFor.nTypeArray[i];
    }
    eScannedType = rNumFor.eScannedType;
    bThousand    = rNumFor.bThousand;
    nThousand    = rNumFor.nThousand;
    nCntPre      = rNumFor.nCntPre;
    nCntPost     = rNumFor.nCntPost;
    nCntExp      = rNumFor.nCntExp;
}

void ImpSvNumberformatInfo::Save(SvStream& rStream, USHORT nAnz) const
{
    for (USHORT i = 0; i < nAnz; i++)
    {
        rStream.WriteByteString( sStrArray[i], rStream.GetStreamCharSet() );
        short nType = nTypeArray[i];
        switch ( nType )
        {   // der Krampf fuer Versionen vor SV_NUMBERFORMATTER_VERSION_NEW_CURR
            case SYMBOLTYPE_CURRENCY :
                rStream << short( SYMBOLTYPE_STRING );
            break;
            case SYMBOLTYPE_CURRDEL :
            case SYMBOLTYPE_CURREXT :
                rStream << short(0);        // werden ignoriert (hoffentlich..)
            break;
            default:
                if ( nType > NF_KEY_LASTKEYWORD_SO5 )
                    rStream << short( SYMBOLTYPE_STRING );  // all new keywords are string
                else
                    rStream << nType;
        }

    }
    rStream << eScannedType << bThousand << nThousand
            << nCntPre << nCntPost << nCntExp;
}

void ImpSvNumberformatInfo::Load(SvStream& rStream, USHORT nAnz)
{
    for (USHORT i = 0; i < nAnz; i++)
    {
        SvNumberformat::LoadString( rStream, sStrArray[i] );
        rStream >> nTypeArray[i];
    }
    rStream >> eScannedType >> bThousand >> nThousand
            >> nCntPre >> nCntPost >> nCntExp;
}

/***********************Funktionen SvNumFor******************************/

ImpSvNumFor::ImpSvNumFor()
{
    nAnzStrings = 0;
    aI.nTypeArray = NULL;
    aI.sStrArray = NULL;
    aI.eScannedType = NUMBERFORMAT_UNDEFINED;
    aI.bThousand = FALSE;
    aI.nThousand = 0;
    aI.nCntPre = 0;
    aI.nCntPost = 0;
    aI.nCntExp = 0;
    pColor = NULL;
}

ImpSvNumFor::~ImpSvNumFor()
{
    for (USHORT i = 0; i < nAnzStrings; i++)
        aI.sStrArray[i].Erase();
    __DELETE(nAnzStrings) aI.sStrArray;
    __DELETE(nAnzStrings) aI.nTypeArray;
}

void ImpSvNumFor::Enlarge(USHORT nAnz)
{
    if ( nAnzStrings != nAnz )
    {
        if ( aI.nTypeArray )
            delete [] aI.nTypeArray;
        if ( aI.sStrArray )
            delete [] aI.sStrArray;
        nAnzStrings = nAnz;
        if ( nAnz )
        {
            aI.nTypeArray = new short[nAnz];
            aI.sStrArray  = new String[nAnz];
        }
        else
        {
            aI.nTypeArray = NULL;
            aI.sStrArray  = NULL;
        }
    }
}

void ImpSvNumFor::Copy( const ImpSvNumFor& rNumFor )
{
    Enlarge( rNumFor.nAnzStrings );
    aI.Copy( rNumFor.aI, nAnzStrings );
    pColor = rNumFor.pColor;
    sColorName = rNumFor.sColorName;
}

void ImpSvNumFor::Save(SvStream& rStream) const
{
    rStream << nAnzStrings;
    aI.Save(rStream, nAnzStrings);
    rStream.WriteByteString( sColorName, rStream.GetStreamCharSet() );
}

void ImpSvNumFor::Load(SvStream& rStream, ImpSvNumberformatScan& rSc,
        String& rLoadedColorName )
{
    USHORT nAnz;
    rStream >> nAnz;        //! noch nicht direkt nAnzStrings wg. Enlarge
    Enlarge( nAnz );
    aI.Load( rStream, nAnz );
    rStream.ReadByteString( sColorName, rStream.GetStreamCharSet() );
    rLoadedColorName = sColorName;
    pColor = rSc.GetColor(sColorName);
}


BOOL ImpSvNumFor::HasNewCurrency() const
{
    for ( USHORT j=0; j<nAnzStrings; j++ )
    {
        if ( aI.nTypeArray[j] == SYMBOLTYPE_CURRENCY )
            return TRUE;
    }
    return FALSE;
}


BOOL ImpSvNumFor::GetNewCurrencySymbol( String& rSymbol,
            String& rExtension ) const
{
    for ( USHORT j=0; j<nAnzStrings; j++ )
    {
        if ( aI.nTypeArray[j] == SYMBOLTYPE_CURRENCY )
        {
            rSymbol = aI.sStrArray[j];
            if ( j < nAnzStrings-1 && aI.nTypeArray[j+1] == SYMBOLTYPE_CURREXT )
                rExtension = aI.sStrArray[j+1];
            else
                rExtension.Erase();
            return TRUE;
        }
    }
    //! kein Erase an rSymbol, rExtension
    return FALSE;
}


void ImpSvNumFor::SaveNewCurrencyMap( SvStream& rStream ) const
{
    USHORT j;
    USHORT nCnt = 0;
    for ( j=0; j<nAnzStrings; j++ )
    {
        switch ( aI.nTypeArray[j] )
        {
            case SYMBOLTYPE_CURRENCY :
            case SYMBOLTYPE_CURRDEL :
            case SYMBOLTYPE_CURREXT :
                nCnt++;
            break;
        }
    }
    rStream << nCnt;
    for ( j=0; j<nAnzStrings; j++ )
    {
        switch ( aI.nTypeArray[j] )
        {
            case SYMBOLTYPE_CURRENCY :
            case SYMBOLTYPE_CURRDEL :
            case SYMBOLTYPE_CURREXT :
                rStream << j << aI.nTypeArray[j];
            break;
        }
    }
}


void ImpSvNumFor::LoadNewCurrencyMap( SvStream& rStream )
{
    USHORT nCnt;
    rStream >> nCnt;
    for ( USHORT j=0; j<nCnt; j++ )
    {
        USHORT nPos;
        short nType;
        rStream >> nPos >> nType;
        if ( nPos < nAnzStrings )
            aI.nTypeArray[nPos] = nType;
    }
}


/***********************Funktionen SvNumberformat************************/

enum Sc_FormatSymbolType
{
    SYMBOLTYPE_FORMAT    = -1,              // TeilFormatstring
    SYMBOLTYPE_COLOR     = -2,              // Farbe
    SYMBOLTYPE_ERROR     = -3               // Fehler
};

SvNumberformat::SvNumberformat( ImpSvNumberformatScan& rSc, LanguageType eLge )
        :
        rScan(rSc),
        eLnge(eLge),
        bStarFlag( FALSE ),
        nNewStandardDefined(0)
{
}

void SvNumberformat::CopyNumberformat( const SvNumberformat& rFormat )
{
    sFormatstring = rFormat.sFormatstring;
    eType         = rFormat.eType;
    eLnge         = rFormat.eLnge;
    fLimit1       = rFormat.fLimit1;
    fLimit2       = rFormat.fLimit2;
    eOp1          = rFormat.eOp1;
    eOp2          = rFormat.eOp2;
    bStandard     = rFormat.bStandard;
    bIsUsed       = rFormat.bIsUsed;
    sComment      = rFormat.sComment;
    nNewStandardDefined = rFormat.nNewStandardDefined;
    for (USHORT i = 0; i < 4; i++)
        NumFor[i].Copy(rFormat.NumFor[i]);
}

SvNumberformat::SvNumberformat( SvNumberformat& rFormat )
    : rScan(rFormat.rScan), bStarFlag( rFormat.bStarFlag )
{
    CopyNumberformat( rFormat );
}

SvNumberformat::SvNumberformat( SvNumberformat& rFormat, ImpSvNumberformatScan& rSc )
    : rScan(rSc), bStarFlag( rFormat.bStarFlag )
{
    CopyNumberformat( rFormat );
}

SvNumberformat::SvNumberformat(String& rString,
                               ImpSvNumberformatScan* pSc,
                               ImpSvNumberInputScan* pISc,
                               xub_StrLen& nCheckPos,
                               LanguageType& eLan,
                               BOOL bStan)
        :
        rScan(*pSc),
        bStarFlag( FALSE ),
        nNewStandardDefined(0)
{
    // If the group (AKA thousand) separator is a Non-Breaking Space (French)
    // replace all occurences by a simple space.
    // The tokens will be changed to the LocaleData separator again later on.
    const sal_Unicode cNBSp = 0xA0;
    const String& rThSep = pSc->GetLoc().getNumThousandSep();
    if ( rThSep.GetChar(0) == cNBSp && rThSep.Len() == 1 )
    {
        xub_StrLen nIndex = 0;
        do
            nIndex = rString.SearchAndReplace( cNBSp, ' ', nIndex );
        while ( nIndex != STRING_NOTFOUND );
    }

    if (rScan.GetConvertMode())
    {
        eLnge = rScan.GetNewLnge();
        eLan = eLnge;                   // Wechsel auch zurueckgeben
    }
    else
        eLnge = eLan;
    bStandard = bStan;
    bIsUsed = FALSE;
    fLimit1 = 0.0;
    fLimit2 = 0.0;
    eOp1 = NUMBERFORMAT_OP_NO;
    eOp2 = NUMBERFORMAT_OP_NO;
    eType = NUMBERFORMAT_DEFINED;

    BOOL bCancel = FALSE;
    BOOL bCondition = FALSE;
    short eSymbolType;
    xub_StrLen nStrPos;
    xub_StrLen nPos = 0;
    xub_StrLen nPosOld;
    nCheckPos = 0;
    String aComment;
                                            // Zerlegung in 4 Teilstrings:
    for (USHORT nIndex = 0; nIndex < 4 && !bCancel; nIndex++)
    {
        if (rScan.GetConvertMode())
            (rScan.GetNumberformatter())->ChangeIntl(rScan.GetTmpLnge());
                                            // in hinteren Formaten muss
                                            // hier ggfs. wieder die
                                            // Ausgangssprache eingestellt
                                            // werden
        String sStr;
        nPosOld = nPos;                         // Position vor Teilstring
        eSymbolType = ImpNextSymbol(rString, nPos, sStr);
        if (eSymbolType > 0)                    // Bedingung
        {
            if (nIndex == 0)
            {
                bCondition = TRUE;
                eOp1 = (SvNumberformatLimitOps) eSymbolType;
            }
            else if (nIndex == 1 && bCondition)
                eOp2 = (SvNumberformatLimitOps) eSymbolType;
            else                                // Fehler
            {
                bCancel = TRUE;                 // Abbruch for
                nCheckPos = nPos;
            }
            if (!bCancel)
            {
                double fNumber;
                xub_StrLen nAnzChars = ImpGetNumber(rString, nPos, sStr);
                if (nAnzChars > 0)
                {
                    short F_Type;
                    if (!pISc->IsNumberFormat(sStr,F_Type,fNumber) ||
                        ( F_Type != NUMBERFORMAT_NUMBER &&
                          F_Type != NUMBERFORMAT_SCIENTIFIC) )
                    {
                        fNumber = 0.0;
                        nPos -= nAnzChars;
                        rString.Erase(nPos, nAnzChars);
                        rString.Insert('0',nPos);
                        nPos++;
                    }
                    nPos++;                         // Skip ']'
                }
                else
                {
                    fNumber = 0.0;
                    rString.Insert('0',nPos);
                }
                if (nIndex == 0)
                    fLimit1 = fNumber;
                else
                    fLimit2 = fNumber;
            }
            nPosOld = nPos;                         // Position vor String
            eSymbolType = ImpNextSymbol(rString, nPos, sStr);
        }
        if (eSymbolType == SYMBOLTYPE_COLOR)
        {
            NumFor[nIndex].SetColor(pSc->GetColor(sStr), sStr);
#ifndef DOS
            if (NumFor[nIndex].GetColor() == NULL)  // Fehler
            {
                bCancel = TRUE;                     // Abbruch for
                nCheckPos = nPos;
            }
            else
            {
                rString.Erase(nPosOld,nPos-nPosOld);
                rString.Insert(sStr,nPosOld);
                nPos = nPosOld + sStr.Len();
                rString.Insert(']', nPos);
                rString.Insert('[', nPosOld);
                nPos += 2;
                nPosOld = nPos;                     // Position vor String
                eSymbolType = ImpNextSymbol(rString, nPos, sStr);
            }
#else
                bCancel = TRUE;                     // Abbruch for
                nCheckPos = nPos;
#endif
        }
        if (eSymbolType == SYMBOLTYPE_FORMAT)
        {
            if (nIndex == 1 && eOp1 == NUMBERFORMAT_OP_NO)// Bdg. undefiniert
                eOp1 = NUMBERFORMAT_OP_GT;              // default: > 0
            else if (nIndex == 2 && eOp2 == NUMBERFORMAT_OP_NO)
                eOp2 = NUMBERFORMAT_OP_LT;              // default: < 0
            if (sStr.Len() == 0)                        // leeres Teilformat
            {                                           // Sonderfall 1
            }
/*                                          // Typ undefined
#ifndef ENABLEUNICODE
            else if (sStr.ICompare(pSc->GetStandardName()) == COMPARE_EQUAL)
#else
            else if (sStr.Compare(pSc->GetStandardName()) == COMPARE_EQUAL)
#endif
            {
                rString.Replace(pSc->GetStandardName(), rString.Search(sStr));
                NumFor[nIndex].Info().eScannedType = NUMBERFORMAT_NUMBER;
            }
*/
                                                        // -> Sonderfall 2
            else
            {
                nStrPos = pSc->ScanFormat( sStr, aComment );
                USHORT nAnz = pSc->GetAnzResStrings();
                if (nAnz == 0)              // Fehler
                    nStrPos = 1;
                if (nStrPos == 0)               // ok
                {
                    rString.Erase(nPosOld,nPos-nPosOld);
                    rString.Insert(sStr,nPosOld);
                    nPos = nPosOld + sStr.Len();
                    if (nPos < rString.Len())
                    {
                        rString.Insert(';',nPos);
                        nPos++;
                    }
                    NumFor[nIndex].Enlarge(nAnz);
                    pSc->CopyInfo(&(NumFor[nIndex].Info()), nAnz);
                    if (nIndex == 0)                        // Typcheck:
                        eType = (short) NumFor[nIndex].Info().eScannedType;
                    else if (nIndex == 3)
                    {   // #77026# Everything recognized IS text
                        NumFor[nIndex].Info().eScannedType = NUMBERFORMAT_TEXT;
                    }
                    else if ( (short) NumFor[nIndex].Info().eScannedType !=
                          eType)
                        eType = NUMBERFORMAT_DEFINED;
                }
                else                                // Fehler im String
                {
                    nCheckPos = nPosOld + nStrPos;
                    bCancel = TRUE;                 // Abbruch for
                }
            }                                       // of != Standard
        }
        else if (eSymbolType == SYMBOLTYPE_ERROR)   // Fehler
        {
            nCheckPos = nPos;
            bCancel = TRUE;
        }
        else if (eSymbolType > 0 ||                 // doppelt
                 eSymbolType == SYMBOLTYPE_COLOR    )
        {
            nCheckPos = nPosOld+1;
            bCancel = TRUE;
        }
        if (rString.Len() == nPos)
            bCancel = TRUE;
    }                                               // of for
    if (bCondition)
    {
        if (NumFor[1].GetnAnz() == 0)               // kein 2. Teilstring
        {
            if (NumFor[1].Info().eScannedType != NUMBERFORMAT_NUMBER)
                rString.InsertAscii(";Standard", rString.Len());
        }
        else if (NumFor[2].GetnAnz() == 0 && eOp2 != NUMBERFORMAT_OP_NO)
        {
            if (NumFor[2].Info().eScannedType != NUMBERFORMAT_NUMBER)
                rString.InsertAscii(";Standard", rString.Len());
        }
    }
    sFormatstring = rString;
    if ( aComment.Len() )
    {
        SetComment( aComment );     // setzt sComment und sFormatstring
        rString = sFormatstring;    // geaenderten sFormatstring uebernehmen
    }
    if (NumFor[2].GetnAnz() == 0 &&                 // kein 3. Teilstring
        eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_NO &&
        fLimit1 == 0.0 && fLimit2 == 0.0)
        eOp1 = NUMBERFORMAT_OP_GE;                  // 0 zum ersten Format dazu

}

SvNumberformat::~SvNumberformat()
{
}

//---------------------------------------------------------------------------
// Next_Symbol
//---------------------------------------------------------------------------
// Zerlegt die Eingabe in Symbole fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = SsStart
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// SsStart       | ;                 | Pos--                 | SsGetString
//               | [                 | Symbol += Zeichen     | SsGetColCon
//               | ]                 | Fehler                | SsStop
//               | BLANK             |                       |
//               | Sonst             | Symbol += Zeichen     | SsGetString
//---------------+-------------------+-----------------------+---------------
// SsGetString   | ;                 |                       | SsStop
//               | Sonst             | Symbol+=Zeichen       |
//---------------+-------------------+-----------------------+---------------
// SsGetColCon   | <, > =            | del [                 |
//               |                   | Symbol += Zeichen     | SsGetCon
//               | BLANK             |                       |
//               | h, H, m, M, s, S  | Symbol += Zeichen     | SsGetTime
//               | sonst             | del [                 |
//               |                   | Symbol += Zeichen     | SsGetCol
//---------------+-------------------+-----------------------+---------------
// SsGetTime     | ]                 | Symbol += Zeichen     | SsGetString
//               | h, H, m, M, s, S  | Symbol += Zeichen, *  | SsGetString
//               | sonst             | del [; Symbol+=Zeichen| SsGetCol
//---------------+-------------------+-----------------------+---------------
// SsGetCol      | ]                 |                       | SsStop
//               | sonst             | Symbol += Zeichen     |
//---------------+-------------------+-----------------------+---------------
// SsGetCon      | >, =              | Symbol+=Zeichen       |
//               | ]                 |                       | SsStop
//               | sonst             | Fehler                | SsStop
//---------------+-------------------+-----------------------+---------------
// * : Sonderbedingung

enum ScanState
{
    SsStop      = 0,
    SsStart     = 1,
    SsGetCon    = 2,                        // Bedingung
    SsGetString = 3,                        // Format
    SsGetCol    = 4,                        // Farbe
    SsGetTime   = 5,                        // [MM] fuer Zeiten
    SsGetColCon = 6                         // noch unentschieden
};

xub_StrLen SvNumberformat::ImpGetNumber(String& rString,
                                 xub_StrLen& nPos,
                                 String& sSymbol)
{
    xub_StrLen nStartPos = nPos;
    sal_Unicode cToken;
    xub_StrLen nLen = rString.Len();
    if (nPos < nLen)
        cToken = rString.GetChar(nPos);
    sSymbol.Erase();
    while (nPos < nLen-1 && cToken != ']')              // bis ']'
    {
        if (cToken == ' ')                              // Blanks loeschen
        {
            rString.Erase(nPos,1);
            nLen--;
        }
        else
        {
            nPos++;
            sSymbol += cToken;
        }
        cToken = rString.GetChar(nPos);
    }
    return nPos - nStartPos;
}


short SvNumberformat::ImpNextSymbol(String& rString,
                                 xub_StrLen& nPos,
                                 String& sSymbol)
{
    short eType = SYMBOLTYPE_FORMAT;
    sal_Unicode cToken;
    sal_Unicode cLetter;                               // Zwischenergebnis
    xub_StrLen nLen = rString.Len();
    ScanState eState = SsStart;
    sSymbol.Erase();
    const String* pKeywords = rScan.GetKeyword();
    while (nPos < nLen && eState != SsStop)
    {
        cToken = rString.GetChar(nPos);
        nPos++;
        switch (eState)
        {
            case SsStart:
            {
                if (cToken == '[')
                {
                    eState = SsGetColCon;
                    sSymbol += cToken;
                }
                else if (cToken == ';')
                {
                    eState = SsGetString;
                    nPos--;
                    eType = SYMBOLTYPE_FORMAT;
                }
                else if (cToken == ']')
                {
                    eState = SsStop;
                    eType = SYMBOLTYPE_ERROR;
                }
                else if (cToken == ' ')             // Skip Blanks
                {
                    rString.Erase(nPos-1,1);
                    nPos--;
                    nLen--;
                }
                else
                {
                    sSymbol += cToken;
                    eState = SsGetString;
                    eType = SYMBOLTYPE_FORMAT;
                }
            }
            break;
            case SsGetColCon:
            {
                switch (cToken)
                {
                    case '<':
                    case '>':
                    case '=':
                    {
                        sSymbol.EraseAllChars('[');
                        sSymbol += cToken;
                        cLetter = cToken;
                        eState = SsGetCon;
                        switch (cToken)
                        {
                            case '<': eType = NUMBERFORMAT_OP_LT; break;
                            case '>': eType = NUMBERFORMAT_OP_GT; break;
                            case '=': eType = NUMBERFORMAT_OP_EQ; break;
                            default: break;
                        }
                    }
                    break;
                    case ' ':
                    {
                        rString.Erase(nPos-1,1);
                        nPos--;
                        nLen--;
                    }
                    break;
                    case '$' :
                    {   // Waehrung ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
                        eType = SYMBOLTYPE_FORMAT;
                        sSymbol += cToken;
                        eState = SsGetString;
                    }
                    break;
                    default:
                    {
                        sal_Unicode cUpper = rChrCls().toUpper( rString, nPos-1, 1 ).GetChar(0);
                        if (cUpper == pKeywords[NF_KEY_H].GetChar(0)    ||  // H
                            cUpper == pKeywords[NF_KEY_MI].GetChar(0)   ||  // M
                            cUpper == pKeywords[NF_KEY_S].GetChar(0)    )   // S
                        {
                            sSymbol += cToken;
                            eState = SsGetTime;
                            cLetter = cToken;
                        }
                        else
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += cToken;
                            eType = SYMBOLTYPE_COLOR;
                            eState = SsGetCol;
                        }
                    }
                    break;
                }
            }
            break;
            case SsGetString:
            {
                if (cToken == ';')
                    eState = SsStop;
                else
                    sSymbol += cToken;
            }
            break;
            case SsGetTime:
            {
                if (cToken == ']')
                {
                    sSymbol += cToken;
                    eState = SsGetString;
                    eType = SYMBOLTYPE_FORMAT;
                }
                else
                {
                    sal_Unicode cUpper = rChrCls().toUpper( rString, nPos-1, 1 ).GetChar(0);
                    if (cUpper == pKeywords[NF_KEY_H].GetChar(0)    ||  // H
                        cUpper == pKeywords[NF_KEY_MI].GetChar(0)   ||  // M
                        cUpper == pKeywords[NF_KEY_S].GetChar(0)    )   // S
                    {
                        if (cLetter == cToken)
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                        }
                        else
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += cToken;
                            eState = SsGetCol;
                        }
                    }
                    else
                    {
                        sSymbol.EraseAllChars('[');
                        sSymbol += cToken;
                        eType = SYMBOLTYPE_COLOR;
                        eState = SsGetCol;
                    }
                }
            }
            break;
            case SsGetCon:
            {
                switch (cToken)
                {
                    case '<':
                    {
                        eState = SsStop;
                        eType = SYMBOLTYPE_ERROR;
                    }
                    break;
                    case '>':
                    {
                        if (cLetter == '<')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eState = SsStop;
                            eType = NUMBERFORMAT_OP_NE;
                        }
                        else
                        {
                            eState = SsStop;
                            eType = SYMBOLTYPE_ERROR;
                        }
                    }
                    break;
                    case '=':
                    {
                        if (cLetter == '<')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eType = NUMBERFORMAT_OP_LE;
                        }
                        else if (cLetter == '>')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eType = NUMBERFORMAT_OP_GE;
                        }
                        else
                        {
                            eState = SsStop;
                            eType = SYMBOLTYPE_ERROR;
                        }
                    }
                    break;
                    case ' ':
                    {
                        rString.Erase(nPos-1,1);
                        nPos--;
                        nLen--;
                    }
                    break;
                    default:
                    {
                        eState = SsStop;
                        nPos--;
                    }
                    break;
                }
            }
            break;
            case SsGetCol:
            {
                if (cToken == ']')
                    eState = SsStop;
                else
                    sSymbol += cToken;
            }
            break;
            default:
            break;
        }                                   // of switch
    }                                       // of while

    return eType;
}

NfHackConversion SvNumberformat::Load( SvStream& rStream,
        ImpSvNumMultipleReadHeader& rHdr, SvNumberFormatter* pHackConverter,
        ImpSvNumberInputScan& rISc )
{
    rHdr.StartEntry();
    USHORT nOp1, nOp2;
    SvNumberformat::LoadString( rStream, sFormatstring );
    rStream >> eType >> fLimit1 >> fLimit2
            >> nOp1 >> nOp2 >> bStandard >> bIsUsed;
    NfHackConversion eHackConversion = NF_CONVERT_NONE;
    BOOL bOldConvert;
    LanguageType eOldTmpLang, eOldNewLang;
    if ( pHackConverter )
    {   // werden nur hierbei gebraucht
        bOldConvert = rScan.GetConvertMode();
        eOldTmpLang = rScan.GetTmpLnge();
        eOldNewLang = rScan.GetNewLnge();
    }
    String aLoadedColorName;
    for (USHORT i = 0; i < 4; i++)
    {
        NumFor[i].Load( rStream, rScan, aLoadedColorName );
        if ( pHackConverter && eHackConversion == NF_CONVERT_NONE )
        {
            //! HACK! ER 29.07.97 13:52
            // leider wurde nicht gespeichert, was SYSTEM on Save wirklich war :-/
            // aber immerhin wird manchmal fuer einen Entry FARBE oder COLOR gespeichert..
            // System-German FARBE nach System-xxx COLOR umsetzen und vice versa,
            //! geht davon aus, dass onSave nur GERMAN und ENGLISH KeyWords in
            //! ImpSvNumberformatScan existierten
            if ( aLoadedColorName.Len() && !NumFor[i].GetColor()
                    && aLoadedColorName != rScan.GetColorString() )
            {
                if ( rScan.GetColorString().EqualsAscii( "FARBE" ) )
                {   // English -> German
                    eHackConversion = NF_CONVERT_ENGLISH_GERMAN;
                    rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_ENGLISH_US );
                    rScan.SetConvertMode( LANGUAGE_ENGLISH_US, LANGUAGE_GERMAN );
                }
                else
                {   // German -> English
                    eHackConversion = NF_CONVERT_GERMAN_ENGLISH;
                    rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_GERMAN );
                    rScan.SetConvertMode( LANGUAGE_GERMAN, LANGUAGE_ENGLISH_US );
                }
                String aColorName = NumFor[i].GetColorName();
                const Color* pColor = rScan.GetColor( aColorName );
                if ( !pColor && aLoadedColorName == aColorName )
                    eHackConversion = NF_CONVERT_NONE;
                rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_SYSTEM );
                rScan.SetConvertMode( eOldTmpLang, eOldNewLang );
                rScan.SetConvertMode( bOldConvert );
            }
        }
    }
    eOp1 = (SvNumberformatLimitOps) nOp1;
    eOp2 = (SvNumberformatLimitOps) nOp2;
    String aComment;        // wird nach dem NewCurrency-Geraffel richtig gesetzt
    if ( rHdr.BytesLeft() )
    {   // ab SV_NUMBERFORMATTER_VERSION_NEWSTANDARD
        SvNumberformat::LoadString( rStream, aComment );
        rStream >> nNewStandardDefined;
    }

    xub_StrLen nNewCurrencyEnd = STRING_NOTFOUND;
    BOOL bNewCurrencyComment = ( aComment.GetChar(0) == cNewCurrencyMagic &&
        (nNewCurrencyEnd = aComment.Search( cNewCurrencyMagic, 1 )) != STRING_NOTFOUND );
    BOOL bNewCurrencyLoaded = FALSE;
    BOOL bNewCurrency = FALSE;
    if ( rHdr.BytesLeft() )
    {   // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
        USHORT nId;
        rStream >> nId;
        DBG_ASSERT( nId == nNewCurrencyVersionId, "SvNumberformat::Load: unknown nId" );
        if ( nId == nNewCurrencyVersionId )
        {
            bNewCurrencyLoaded = TRUE;
            rStream >> bNewCurrency;
            if ( bNewCurrency )
            {
                for ( USHORT j=0; j<4; j++ )
                {
                    NumFor[j].LoadNewCurrencyMap( rStream );
                }
            }
        }
    }
    if ( bNewCurrencyLoaded )
    {
        if ( bNewCurrency && bNewCurrencyComment )
        {   // original Formatstring und Kommentar wiederherstellen
            sFormatstring = aComment.Copy( 1, nNewCurrencyEnd-1 );
            aComment.Erase( 0, nNewCurrencyEnd+1 );
        }
    }
    else if ( bNewCurrencyComment )
    {   // neu, aber mit Version vor SV_NUMBERFORMATTER_VERSION_NEW_CURR gespeichert
        // original Formatstring und Kommentar wiederherstellen
        sFormatstring = aComment.Copy( 1, nNewCurrencyEnd-1 );
        aComment.Erase( 0, nNewCurrencyEnd+1 );
        // Zustaende merken
        short nDefined = ( eType & NUMBERFORMAT_DEFINED );
        USHORT nNewStandard = nNewStandardDefined;
        // neu parsen etc.
        String aStr( sFormatstring );
        xub_StrLen nCheckPos;
        SvNumberformat* pFormat = new SvNumberformat( aStr, &rScan, &rISc,
            nCheckPos, eLnge, bStandard );
        DBG_ASSERT( !nCheckPos, "SvNumberformat::Load: NewCurrencyRescan nCheckPos" );
        CopyNumberformat( *pFormat );
        delete pFormat;
        // Zustaende wiederherstellen
        eType |= nDefined;
        if ( nNewStandard )
            SetNewStandardDefined( nNewStandard );
    }
    SetComment( aComment );

    rHdr.EndEntry();

    if ( eHackConversion != NF_CONVERT_NONE )
    {   //! und weiter mit dem HACK!
        switch ( eHackConversion )
        {
            case NF_CONVERT_ENGLISH_GERMAN :
                ConvertLanguage( *pHackConverter,
                    LANGUAGE_ENGLISH_US, LANGUAGE_GERMAN, TRUE );
            break;
            case NF_CONVERT_GERMAN_ENGLISH :
                ConvertLanguage( *pHackConverter,
                    LANGUAGE_GERMAN, LANGUAGE_ENGLISH_US, TRUE );
            break;
            default:
                DBG_ERRORFILE( "SvNumberformat::Load: eHackConversion unknown" );
        }
    }
    return eHackConversion;
}

void SvNumberformat::ConvertLanguage( SvNumberFormatter& rConverter,
        LanguageType eConvertFrom, LanguageType eConvertTo, BOOL bSystem )
{
    xub_StrLen nCheckPos;
    ULONG nKey;
    short nType = eType;
    String aFormatString( sFormatstring );
    rConverter.PutandConvertEntry( aFormatString, nCheckPos, nType, nKey,
        eConvertFrom, eConvertTo );
    const SvNumberformat* pFormat = rConverter.GetEntry( nKey );
    DBG_ASSERT( pFormat, "SvNumberformat::ConvertLanguage: Conversion ohne Format" );
    if ( pFormat )
    {
        CopyNumberformat( *pFormat );
        // aus Formatter/Scanner uebernommene Werte zuruecksetzen
        if ( bSystem )
            eLnge = LANGUAGE_SYSTEM;
        // pColor zeigt noch auf Tabelle in temporaerem Formatter/Scanner
        for ( USHORT i = 0; i < 4; i++ )
        {
            String aColorName = NumFor[i].GetColorName();
            Color* pColor = rScan.GetColor( aColorName );
            NumFor[i].SetColor( pColor, aColorName );
        }
    }
}


// static
void SvNumberformat::LoadString( SvStream& rStream, String& rStr )
{
    CharSet eStream = rStream.GetStreamCharSet();
    ByteString aStr;
    rStream.ReadByteString( aStr );
    sal_Char cStream = NfCurrencyEntry::GetEuroSymbol( eStream );
    if ( aStr.Search( cStream ) == STRING_NOTFOUND )
    {   // simple conversion to unicode
        rStr = UniString( aStr, eStream );
    }
    else
    {
        sal_Unicode cTarget = NfCurrencyEntry::GetEuroSymbol();
        register const sal_Char* p = aStr.GetBuffer();
        register const sal_Char* const pEnd = p + aStr.Len();
        register sal_Unicode* pUni = rStr.AllocBuffer( aStr.Len() );
        while ( p < pEnd )
        {
            if ( *p == cStream )
                *pUni = cTarget;
            else
                *pUni = ByteString::ConvertToUnicode( *p, eStream );
            p++;
            pUni++;
        }
        *pUni = 0;
    }
}


void SvNumberformat::Save( SvStream& rStream, ImpSvNumMultipleWriteHeader& rHdr ) const
{
    String aFormatstring( sFormatstring );
    String aComment( sComment );
#if NF_COMMENT_IN_FORMATSTRING
    // der Kommentar im Formatstring wird nicht gespeichert, um in alten Versionen
    // nicht ins schleudern zu kommen und spaeter getrennte Verarbeitung
    // (z.B. im Dialog) zu ermoeglichen
    SetComment( "", aFormatstring, aComment );
#endif

    BOOL bNewCurrency = HasNewCurrency();
    if ( bNewCurrency )
    {   // SV_NUMBERFORMATTER_VERSION_NEW_CURR im Kommentar speichern
        aComment.Insert( cNewCurrencyMagic, 0 );
        aComment.Insert( cNewCurrencyMagic, 0 );
        aComment.Insert( aFormatstring, 1 );
        Build50Formatstring( aFormatstring );       // alten Formatstring generieren
    }

    rHdr.StartEntry();
    rStream.WriteByteString( aFormatstring, rStream.GetStreamCharSet() );
    rStream << eType << fLimit1 << fLimit2 << (USHORT) eOp1 << (USHORT) eOp2
            << bStandard << bIsUsed;
    for (USHORT i = 0; i < 4; i++)
        NumFor[i].Save(rStream);
    // ab SV_NUMBERFORMATTER_VERSION_NEWSTANDARD
    rStream.WriteByteString( aComment, rStream.GetStreamCharSet() );
    rStream << nNewStandardDefined;
    // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
    rStream << nNewCurrencyVersionId;
    rStream << bNewCurrency;
    if ( bNewCurrency )
    {
        for ( USHORT j=0; j<4; j++ )
        {
            NumFor[j].SaveNewCurrencyMap( rStream );
        }
    }
    rHdr.EndEntry();
}


BOOL SvNumberformat::HasNewCurrency() const
{
    for ( USHORT j=0; j<4; j++ )
    {
        if ( NumFor[j].HasNewCurrency() )
            return TRUE;
    }
    return FALSE;
}


BOOL SvNumberformat::GetNewCurrencySymbol( String& rSymbol,
            String& rExtension ) const
{
    for ( USHORT j=0; j<4; j++ )
    {
        if ( NumFor[j].GetNewCurrencySymbol( rSymbol, rExtension ) )
            return TRUE;
    }
    rSymbol.Erase();
    rExtension.Erase();
    return FALSE;
}


void SvNumberformat::Build50Formatstring( String& rStr ) const
{
    rStr.Erase();
    xub_StrLen nStartPos, nPos, nLen;
    nLen = sFormatstring.Len();
    nStartPos = 0;
    while ( (nPos = sFormatstring.SearchAscii( "[$", nStartPos )) != STRING_NOTFOUND  )
    {
        xub_StrLen nEnd;
        if ( (nEnd = GetQuoteEnd( sFormatstring, nPos )) < nLen )
        {
            rStr += sFormatstring.Copy( nStartPos, ++nEnd - nStartPos );
            nStartPos = nEnd;
        }
        else
        {
            rStr += sFormatstring.Copy( nStartPos, nPos - nStartPos );
            nStartPos = nPos + 2;
            xub_StrLen nDash;
            nEnd = nStartPos - 1;
            do
            {
                nDash = sFormatstring.Search( '-', ++nEnd );
            } while ( (nEnd = GetQuoteEnd( sFormatstring, nDash )) < nLen );
            xub_StrLen nClose;
            nEnd = nStartPos - 1;
            do
            {
                nClose = sFormatstring.Search( ']', ++nEnd );
            } while ( (nEnd = GetQuoteEnd( sFormatstring, nClose )) < nLen );
            nPos = ( nDash < nClose ? nDash : nClose );
            if ( sFormatstring.GetChar( nStartPos ) == '"' )
                rStr += sFormatstring.Copy( nStartPos, nPos - nStartPos );
            else
            {
                rStr += '"';
                rStr += sFormatstring.Copy( nStartPos, nPos - nStartPos );
                rStr += '"';
            }
            nStartPos = nClose + 1;
        }
    }
    if ( nLen > nStartPos )
        rStr += sFormatstring.Copy( nStartPos, nLen - nStartPos );
}


void SvNumberformat::ImpGetOutputStandard(double& fNumber, String& OutString)
{
    OutString.Erase();
    USHORT nStandardPrec = rScan.GetStandardPrec();
    if ( fabs(fNumber) > 1.0E15 )       // #58531# war E16
        SolarMath::DoubleToString(OutString, fNumber, 'E', nStandardPrec /*2*/,
                       rLoc().getNumDecimalSep().GetChar(0));
    else
    {
        SolarMath::DoubleToString( OutString, fNumber, 'F', nStandardPrec /*2*/,
                       rLoc().getNumDecimalSep().GetChar(0), TRUE );
        if (OutString.GetChar(0) == '-' &&
            OutString.GetTokenCount('0') == OutString.Len())
            OutString.EraseLeadingChars('-');            // nicht -0
    }
    return;
}

void SvNumberformat::ImpGetOutputInputLine(double fNumber, String& OutString)
{
    BOOL bModified = FALSE;
    if ( (eType & NUMBERFORMAT_PERCENT) && (fabs(fNumber) < _D_MAX_D_BY_100))
    {
        if (fNumber == 0.0)
        {
            OutString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "0%" ) );
            return;
        }
        fNumber *= 100;
        bModified = TRUE;
    }

    if (fNumber == 0.0)
    {
        OutString = '0';
        return;
    }

    SolarMath::DoubleToString( OutString, fNumber, 'A', INT_MAX,
                    rLoc().getNumDecimalSep().GetChar(0), TRUE );

    if ( eType & NUMBERFORMAT_PERCENT && bModified)
        OutString += '%';
    return;
}

short SvNumberformat::ImpCheckCondition(double& fNumber,
                                     double& fLimit,
                                     SvNumberformatLimitOps eOp)
{
    switch(eOp)
    {
        case NUMBERFORMAT_OP_NO: return -1;
        case NUMBERFORMAT_OP_EQ: return (short) (fNumber == fLimit);
        case NUMBERFORMAT_OP_NE: return (short) (fNumber != fLimit);
        case NUMBERFORMAT_OP_LT: return (short) (fNumber <  fLimit);
        case NUMBERFORMAT_OP_LE: return (short) (fNumber <= fLimit);
        case NUMBERFORMAT_OP_GT: return (short) (fNumber >  fLimit);
        case NUMBERFORMAT_OP_GE: return (short) (fNumber >= fLimit);
        default: return -1;
    }
}

BOOL SvNumberformat::GetOutputString(String& sString,
                                     String& OutString,
                                     Color** ppColor)
{
    OutString.Erase();
    USHORT nIx;
    if (eType & NUMBERFORMAT_TEXT)
        nIx = 0;
    else if (NumFor[3].GetnAnz() > 0)
        nIx = 3;
    else
    {
        *ppColor = NULL;        // no change of color
        return FALSE;
    }
    *ppColor = NumFor[nIx].GetColor();
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == NUMBERFORMAT_TEXT)
    {
        BOOL bRes = FALSE;
        const USHORT nAnz = NumFor[nIx].GetnAnz();
        for (USHORT i = 0; i < nAnz; i++)
        {
            switch (rInfo.nTypeArray[i])
            {
                case SYMBOLTYPE_STAR:
                    if( bStarFlag )
                    {
                        OutString += (sal_Unicode) 0x1B;
                        OutString += rInfo.sStrArray[i].GetChar(1);
                        bRes = TRUE;
                    }
                break;
                case SYMBOLTYPE_BLANK:
                    InsertBlanks( OutString, OutString.Len(),
                        rInfo.sStrArray[i].GetChar(1) );
                break;
                case NF_KEY_GENERAL :   // #77026# "General" is the same as "@"
                case SYMBOLTYPE_DEL :
                    OutString += sString;
                break;
                default:
                    OutString += rInfo.sStrArray[i];
            }
        }
        return bRes;
    }
    return FALSE;
}
/*
void SvNumberformat::GetNextFareyNumber(ULONG nPrec, ULONG x0, ULONG x1,
                                        ULONG y0, ULONG y1,
                                        ULONG& x2,ULONG& y2)
{
    x2 = ((y0+nPrec)/y1)*x1 - x0;
    y2 = ((y0+nPrec)/y1)*y1 - y0;
}
*/
ULONG SvNumberformat::ImpGGT(ULONG x, ULONG y)
{
    if (y == 0)
        return x;
    else
    {
        ULONG z = x%y;
        while (z)
        {
            x = y;
            y = z;
            z = x%y;
        }
        return y;
    }
}

ULONG SvNumberformat::ImpGGTRound(ULONG x, ULONG y)
{
    if (y == 0)
        return x;
    else
    {
        ULONG z = x%y;
        while ((double)z/(double)y > D_EPS)
        {
            x = y;
            y = z;
            z = x%y;
        }
        return y;
    }
}

BOOL SvNumberformat::GetOutputString(double fNumber,
                                     String& OutString,
                                     Color** ppColor)
{
    BOOL bRes = FALSE;
    OutString.Erase();                          // alles loeschen
    *ppColor = NULL;                            // keine Farbaenderung
    if (eType & NUMBERFORMAT_LOGICAL)
    {
        if (fNumber)
            OutString = rScan.GetTrueString();
        else
            OutString = rScan.GetFalseString();
        return FALSE;
    }
    if (eType & NUMBERFORMAT_TEXT && bStandard)
    {
        ImpGetOutputStandard(fNumber, OutString);
        return FALSE;
    }
    BOOL bHadStandard = FALSE;
    if (bStandard)                              // einzelne Standardformate
    {
        if (rScan.GetStandardPrec() == 300)     // alle Zahlformate InputLine
        {
            ImpGetOutputInputLine(fNumber, OutString);
            return FALSE;
        }
        switch (eType)
        {
            case NUMBERFORMAT_NUMBER:                   // Standardzahlformat
                ImpGetOutputStandard(fNumber, OutString);
                bHadStandard = TRUE;
            break;
            case NUMBERFORMAT_DATE:
                bRes |= ImpGetDateOutput(fNumber, 0, OutString);
                bHadStandard = TRUE;
            break;
            case NUMBERFORMAT_TIME:
                bRes |= ImpGetTimeOutput(fNumber, 0, OutString);
                bHadStandard = TRUE;
            break;
            case NUMBERFORMAT_DATETIME:
                bRes |= ImpGetDateTimeOutput(fNumber, 0, OutString);
                bHadStandard = TRUE;
            break;
            case NUMBERFORMAT_PERCENT:
            {
                 if (fabs(fNumber) < _D_MAX_D_BY_100)
                 {
                    fNumber *= 100;
                    SolarMath::DoubleToString(OutString, fNumber, 'F', 2,
                                   rLoc().getNumDecimalSep().GetChar(0));
                    if (OutString.GetChar(0) == '-' &&
                        OutString.GetTokenCount('0') == OutString.Len())
                        OutString.EraseLeadingChars('-');            // nicht -0
                    OutString+= '%';
                 }
                 else
                    OutString = rScan.GetErrorString();
                bHadStandard = TRUE;
            }
            break;
            case NUMBERFORMAT_SCIENTIFIC:
            {
                SolarMath::DoubleToString(OutString, fNumber, 'E', 2,
                    rLoc().getNumDecimalSep().GetChar(0));
                return FALSE;
            }
            break;
            default:
            break;
        }
    }
    if ( !bHadStandard )
    {
        USHORT nIx;                             // Index des Teilformats
        short nCheck = ImpCheckCondition(fNumber, fLimit1, eOp1);
        if (nCheck == -1 || nCheck == 1)            // nur 1 String oder True
            nIx = 0;
        else
        {
            nCheck = ImpCheckCondition(fNumber, fLimit2, eOp2);
            if (nCheck == -1 || nCheck == 1)
                nIx = 1;
            else
                nIx = 2;
        }
        if (nIx == 1 && fNumber < 0.0 &&        // negatives Format
                IsNegativeRealNegative() )      // ohne Vorzeichen
            fNumber = -fNumber;                 // Vorzeichen eliminieren
        *ppColor = NumFor[nIx].GetColor();
        const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
        const USHORT nAnz = NumFor[nIx].GetnAnz();
        if (nAnz == 0 && rInfo.eScannedType == NUMBERFORMAT_UNDEFINED)
            return FALSE;                       // leer => nichts
        else if (nAnz == 0)                     // sonst Standard-Format
        {
            ImpGetOutputStandard(fNumber, OutString);
            return FALSE;
        }
        switch (rInfo.eScannedType)
        {
            case NUMBERFORMAT_TEXT:
            case NUMBERFORMAT_DEFINED:
            {
                for (USHORT i = 0; i < nAnz; i++)
                {
                    switch (rInfo.nTypeArray[i])
                    {
                        case SYMBOLTYPE_STAR:
                            if( bStarFlag )
                            {
                                OutString += (sal_Unicode) 0x1B;
                                OutString += rInfo.sStrArray[i].GetChar(1);
                                bRes = TRUE;
                            }
                            break;
                        case SYMBOLTYPE_BLANK:
                            InsertBlanks( OutString, OutString.Len(),
                                rInfo.sStrArray[i].GetChar(1) );
                            break;
                        case SYMBOLTYPE_STRING:
                        case SYMBOLTYPE_CURRENCY:
                            OutString += rInfo.sStrArray[i];
                            break;
                        case SYMBOLTYPE_THSEP:
                            if (rInfo.nThousand == 0)
                                OutString += rInfo.sStrArray[i];
                        break;
                        default:
                        break;
                    }
                }
            }
            break;
            case NUMBERFORMAT_DATE:
                bRes |= ImpGetDateOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_TIME:
                bRes |= ImpGetTimeOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_DATETIME:
                bRes |= ImpGetDateTimeOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_NUMBER:
            case NUMBERFORMAT_PERCENT:
            case NUMBERFORMAT_CURRENCY:
                bRes |= ImpGetNumberOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_FRACTION:
            {
                String sStr, sFrac, sDiv;               // Strings, Wert fuer
                ULONG nFrac, nDiv;                      // Vorkommaanteil
                                                        // Zaehler und Nenner
                BOOL bSign = FALSE;
                if (fNumber < 0)
                {
                    if (nIx == 0)                       // nicht in hinteren
                        bSign = TRUE;                   // Formaten
                    fNumber = -fNumber;
                }
                double fNum = floor(fNumber);           // Vorkommateil
                fNumber -= fNum;                        // Nachkommateil
                if (fNum > _D_MAX_U_LONG_ || rInfo.nCntExp > 9)
                                                        // zu gross
                {
                    OutString = rScan.GetErrorString();
                    return FALSE;
                }
                if (rInfo.nCntExp == 0)
                {
                    DBG_ERROR("SvNumberformat:: Bruch, nCntExp == 0");
                    return FALSE;
                }
                ULONG nBasis = ((ULONG)floor(           // 9, 99, 999 ,...
                                    pow(10.0,rInfo.nCntExp))) - 1;
                ULONG x0, y0, x1, y1, x2, y2;

                if (rInfo.nCntExp <= _MAX_FRACTION_PREC)
                {
                    BOOL bUpperHalf;
                    if (fNumber > 0.5)
                    {
                        bUpperHalf = TRUE;
                        fNumber -= (fNumber - 0.5) * 2.0;
                    }
                    else
                        bUpperHalf = FALSE;
                                                    // Einstieg in Farey-Serie
                                                    // finden:
                    x0 = (ULONG) floor(fNumber*nBasis); // z.B. 2/9 <= x < 3/9
                    if (x0 == 0)                        //      => x0 = 2
                    {
                        y0 = 1;
                        x1 = 1;
                        y1 = nBasis;
                    }
                    else if (x0 == (nBasis-1)/2)    // (b-1)/2, 1/2
                    {                               // geht (nBasis ungerade)
                        y0 = nBasis;
                        x1 = 1;
                        y1 = 2;
                    }
                    else if (x0 == 1)
                    {
                        y0 = nBasis;                    //  1/n; 1/(n-1)
                        x1 = 1;
                        y1 = nBasis - 1;
                    }
                    else
                    {
                      y0 = nBasis;                    // z.B. 2/9   2/8
                        x1 = x0;
                        y1 = nBasis - 1;
                        double fUg = (double) x0 / (double) y0;
                        double fOg = (double) x1 / (double) y1;
                        ULONG nGgt = ImpGGT(y0, x0);       // x0/y0 kuerzen
                        x0 /= nGgt;
                        y0 /= nGgt;                     // Einschachteln:
                        double fTest;
                        BOOL bStop = FALSE;
                        while (!bStop)
                        {
                            fTest = (double)x1/(double)y1;
                            while (!bStop)
                            {
                                while (fTest > fOg)
                                {
                                    x1--;
                                    fTest = (double)x1/(double)y1;
                                }
                                while (fTest < fUg && y1 > 1)
                               {
                                    y1--;
                                    fTest = (double)x1/(double)y1;
                                }
                                if (fTest <= fOg)
                                {
                                    fOg = fTest;
                                    bStop = TRUE;
                                }
                                else if (y1 == 1)
                                    bStop = TRUE;
                            }                               // of while
                            nGgt = ImpGGT(y1, x1);             // x1/y1 kuerzen
                            x2 = x1 / nGgt;
                            y2 = y1 / nGgt;
                            if (x2*y0 - x0*y2 == 1 || y1 <= 1)  // Test, ob x2/y2
                                bStop = TRUE;               // naechste Farey-Zahl
                            else
                            {
                                y1--;
                                bStop = FALSE;
                            }
                        }                                   // of while
                        x1 = x2;
                        y1 = y2;
                    }                                       // of else
                    double fup, flow;
                    flow = (double)x0/(double)y0;
                    fup  = (double)x1/(double)y1;
                    while (fNumber > fup)
                    {
                        x2 = ((y0+nBasis)/y1)*x1 - x0;      // naechste Farey-Zahl
                        y2 = ((y0+nBasis)/y1)*y1 - y0;
//                      GetNextFareyNumber(nBasis, x0, x1, y0, y1, x2, y2);
                        x0 = x1;
                        y0 = y1;
                        x1 = x2;
                        y1 = y2;
                        flow = fup;
                        fup  = (double)x1/(double)y1;
                    }
                    if (fNumber - flow < fup - fNumber)
                    {
                        nFrac = x0;
                        nDiv  = y0;
                    }
                    else
                    {
                        nFrac = x1;
                        nDiv  = y1;
                    }
                    if (bUpperHalf)                     // Original restaur.
                    {
                        if (nFrac == 0 && nDiv == 1)    // 1/1
                            fNum += 1.0;
                        else
                            nFrac = nDiv - nFrac;
                    }
                }
                else                                    // grosse Nenner
                {                                       // 0,1234->123/1000
                    ULONG nGgt;
/*
                    nDiv = nBasis+1;
                    nFrac = ((ULONG)floor(0.5 + fNumber *
                                    pow(10.0,rInfo.nCntExp)));
*/
                    nDiv = 10000000;
                    nFrac = ((ULONG)floor(0.5 + fNumber * 10000000.0));
                    nGgt = ImpGGT(nDiv, nFrac);
                    if (nGgt > 1)
                    {
                        nDiv  /= nGgt;
                        nFrac /= nGgt;
                    }
                    if (nDiv > nBasis)
                    {
                        nGgt = ImpGGTRound(nDiv, nFrac);
                        if (nGgt > 1)
                        {
                            nDiv  /= nGgt;
                            nFrac /= nGgt;
                        }
                    }
                    if (nDiv > nBasis)
                    {
                        nDiv = nBasis;
                        nFrac = ((ULONG)floor(0.5 + fNumber *
                                    pow(10.0,rInfo.nCntExp)));
                        nGgt = ImpGGTRound(nDiv, nFrac);
                        if (nGgt > 1)
                        {
                            nDiv  /= nGgt;
                            nFrac /= nGgt;
                        }
                    }
                }

                if (rInfo.nCntPre == 0)    // unechter Bruch
                {
                    double fNum1 = fNum * (double)nDiv + (double)nFrac;
                    if (fNum1 > _D_MAX_U_LONG_)
                    {
                        OutString = rScan.GetErrorString();
                        return FALSE;
                    }
                    nFrac = (ULONG) floor(fNum1);
                    sStr.Erase();
                }
                else if (fNum == 0.0 && nFrac != 0)
                    sStr.Erase();
                else
                {
                    char aBuf[100];
                    sprintf( aBuf, "%.f", fNum );   // simple rounded integer
                    sStr.AssignAscii( aBuf );
                }
                sFrac = String::CreateFromInt32(nFrac);
                sDiv = String::CreateFromInt32(nDiv);
                if (rInfo.nCntPre > 0 && nFrac == 0)
                {
                    sFrac.Erase();
                    sDiv.Erase();
                }

                USHORT j = nAnz-1;                  // letztes Symbol->rueckw.
                xub_StrLen k;                       // Nenner:
                bRes |= ImpNumberFill(sDiv, fNumber, k, j, nIx, SYMBOLTYPE_FRAC);
                BOOL bCont = TRUE;
                if (rInfo.nTypeArray[j] == SYMBOLTYPE_FRAC)
                {
                    if (rInfo.nCntPre > 0 && nFrac == 0)
                        sDiv.Insert(' ',0);
                    else
                        sDiv.Insert( rInfo.sStrArray[j].GetChar(0), 0 );
                    if ( j )
                        j--;
                    else
                        bCont = FALSE;
                }
                                                    // weiter Zaehler:
                if ( !bCont )
                    sFrac.Erase();
                else
                {
                    bRes |= ImpNumberFill(sFrac, fNumber, k, j, nIx, SYMBOLTYPE_FRACBLANK);
                    if (rInfo.nTypeArray[j] == SYMBOLTYPE_FRACBLANK)
                    {
                        sFrac.Insert(rInfo.sStrArray[j],0);
                        if ( j )
                            j--;
                        else
                            bCont = FALSE;
                    }
                }
                                                    // weiter Hauptzahl
                if ( !bCont )
                    sStr.Erase();
                else
                {
                    k = sStr.Len();                 // hinter letzter Ziffer
                    bRes |= ImpNumberFillWithThousands(sStr, fNumber, k, j, nIx,
                                            rInfo.nCntPre);
                }
                if (bSign && !(nFrac == 0 && fNum == 0.0))
                    OutString.Insert('-',0);        // nicht -0
                OutString += sStr;
                OutString += sFrac;
                OutString += sDiv;
            }
            break;
            case NUMBERFORMAT_SCIENTIFIC:
            {
                BOOL bSign = FALSE;
                if (fNumber < 0)
                {
                    if (nIx == 0)                       // nicht in hinteren
                        bSign = TRUE;                   // Formaten
                    fNumber = -fNumber;
                }
                String sStr;
                SolarMath::DoubleToString(sStr, fNumber, 'E',
                               rInfo.nCntPre+rInfo.nCntPost-1);

                String ExpStr;
                short nExpSign = 1;
                xub_StrLen nExPos = sStr.Search('E');
                if ( nExPos != STRING_NOTFOUND )
                {
                    ExpStr = sStr.Copy( nExPos );
                    sStr.Erase( nExPos );
                    ExpStr.Erase(0,2);              // get rid of "E+" or "E-"
                    sStr.EraseAllChars('.');        // Komma (Punkt) weg
                    if (rInfo.nCntPre != 1)         // Reskalierung Exp
                    {
                        sal_Int32 nExp = ExpStr.ToInt32();
                        nExp -= sal_Int32(rInfo.nCntPre)-1;
                        ExpStr = String::CreateFromInt32( nExp );
                    }
                    if (ExpStr.GetChar(0) == '-')
                    {
                        nExpSign = -1;
                        ExpStr.Erase(0,1);                      // Vorzeichen weg
                    }
                }
                USHORT j = nAnz-1;              // letztes Symbol
                xub_StrLen k;
                bRes |= ImpNumberFill(ExpStr, fNumber, k, j, nIx, SYMBOLTYPE_EXP);

                while (k > 0)                   // fuehrende Nullen loeschen
                {
                    k--;
                    if (ExpStr.GetChar(k) == '0')
                        ExpStr.Erase(0,1);
                }
                BOOL bCont = TRUE;
                if (rInfo.nTypeArray[j] == SYMBOLTYPE_EXP)
                {
                    const String& rStr = rInfo.sStrArray[j];
                    if (nExpSign == -1)
                        ExpStr.Insert('-',0);
                    else if (rStr.Len() > 1 && rStr.GetChar(1) == '+')
                        ExpStr.Insert('+',0);
                    ExpStr.Insert(rStr.GetChar(0),0);
                    if ( j )
                        j--;
                    else
                        bCont = FALSE;
                }
                                                    // weiter Hauptzahl:
                if ( !bCont )
                    sStr.Erase();
                else
                {
                    k = sStr.Len();                 // hinter letzter Ziffer
                    bRes |= ImpNumberFillWithThousands(sStr,fNumber, k,j,nIx,
                                            rInfo.nCntPre +
                                            rInfo.nCntPost);
                }
                if (bSign)
                    sStr.Insert('-',0);
                OutString = sStr;
                OutString += ExpStr;
                return bRes;
            }
            break;
        }
    }
    return bRes;
}

BOOL SvNumberformat::ImpGetTimeOutput(double fNumber,
                                   USHORT nIx,
                                   String& OutString)
{
    using namespace ::com::sun::star::i18n;
    BOOL bCalendarSet = FALSE;
    CalendarWrapper& rCal = GetCal();
    double fNumberOrig = fNumber;
    BOOL bRes = FALSE;
    BOOL bSign = FALSE;
    if (fNumber < 0.0)
    {
        fNumber = -fNumber;
        if (nIx == 0)
            bSign = TRUE;
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.bThousand)       // []-Format
    {
        if (fNumber > 1.0E10)               // zu gross
        {
            OutString = rScan.GetErrorString();
            return FALSE;
        }
    }
    else
        fNumber -= floor(fNumber);          // sonst Datum abtrennen
    BOOL bInputLine;
    xub_StrLen nCntPost;
    if ( rScan.GetStandardPrec() == 300 &&
            0 < rInfo.nCntPost && rInfo.nCntPost < 12 )
    {
        bInputLine = TRUE;
        nCntPost = 12;
    }
    else
    {
        bInputLine = FALSE;
        nCntPost = xub_StrLen(rInfo.nCntPost);
    }
    if (bSign && !rInfo.bThousand)     // kein []-Format
        fNumber = 1.0 - fNumber;        // "Kehrwert"
    double fTime = fNumber * 86400.0;
    double fFactor = pow( 10.0, double(nCntPost) );
    fTime = floor( 0.5 + fTime * fFactor ) / fFactor; // runden
    if (bSign && fTime == 0.0)
        bSign = FALSE;                      // nicht -00:00:00

    ULONG nSeconds = 0;
    ULONG nHours = 0;
    if( floor( fTime ) > _D_MAX_U_LONG_ )
    {
    if( (floor( fTime )/3600.0) > _D_MAX_U_LONG_ )
    {
        OutString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###" ) );
        return FALSE;
    }
    else
    {
        nSeconds = (ULONG)(((fTime/3600.0) - floor( fTime/3600.0 ))*3600.0);
        nHours = (ULONG)(fTime/3600.0);
    }
    }
    else
    nSeconds = (ULONG)floor( fTime );

    String sSecStr;
    SolarMath::DoubleToString(sSecStr, fTime-nSeconds, 'F', int(nCntPost));
    sSecStr.EraseLeadingChars('0');
    sSecStr.EraseLeadingChars('.');
    if ( bInputLine )
    {
        sSecStr.EraseTrailingChars('0');
        if ( sSecStr.Len() < xub_StrLen(rInfo.nCntPost) )
            sSecStr.Expand( xub_StrLen(rInfo.nCntPost), '0' );
        nCntPost = sSecStr.Len();
    }
    xub_StrLen nSecPos = 0;                 // Zum Ziffernweisen
                                            // abarbeiten
    ULONG nHour, nMin, nSec;
    if (!rInfo.bThousand)      // kein [] Format
    {
        nHour = (nSeconds/3600) % 24;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    else if (rInfo.nThousand == 3) // [ss]
    {
        nHour = 0;
        nMin = 0;
        nSec = nSeconds;
    }
    else if (rInfo.nThousand == 2) // [mm]:ss
    {
        nHour = 0;
        nMin = nSeconds / 60;
        nSec = nSeconds % 60;
    }
    else if (rInfo.nThousand == 1) // [hh]:mm:ss
    {
        nHour = nSeconds / 3600;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    sal_Unicode cAmPm = ' ';                   // a oder p
    if (rInfo.nCntExp)     // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
            cAmPm = 'a';
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
                nHour -= 12;
        }
    }
    const USHORT nAnz = NumFor[nIx].GetnAnz();
    for (USHORT i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = TRUE;
                }
                break;
            case SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
                break;
            case SYMBOLTYPE_STRING:
            case SYMBOLTYPE_CURRENCY:
                OutString += rInfo.sStrArray[i];
                break;
            case SYMBOLTYPE_DIGIT:
            {
                xub_StrLen nLen = ( bInputLine && i > 0 &&
                    rInfo.sStrArray[i-1] == rLoc().getNumDecimalSep() ?
                    nCntPost : rInfo.sStrArray[i].Len() );
                for (xub_StrLen j = 0; j < nLen && nSecPos < nCntPost; j++)
                {
                    OutString += sSecStr.GetChar(nSecPos);
                    nSecPos++;
                }
            }
            break;
            case NF_KEY_AMPM:               // AM/PM
            {
                if ( !bCalendarSet )
                {
                    DateTime aDateTime( *(rScan.GetNullDate()) );
                    aDateTime += fNumberOrig;
                    CalendarWrapper& rCal = GetCal();
                    rCal.setGregorianDateTime( aDateTime );
                    bCalendarSet = TRUE;
                }
                if (cAmPm == 'a')
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::AM, 0 );
                else
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::PM, 0 );
            }
            break;
            case NF_KEY_AP:                 // A/P
            {
                if (cAmPm == 'a')
                    OutString += 'a';
                else
                    OutString += 'p';
            }
            break;
            case NF_KEY_MI:                 // M
                OutString += String::CreateFromInt32(nMin);
            break;
            case NF_KEY_MMI:                // MM
            {
                if (nMin < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nMin);
            }
            break;
            case NF_KEY_H:                  // H
                OutString += String::CreateFromInt32(nHour);
            break;
            case NF_KEY_HH:                 // HH
            {
                if (nHour < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nHour);
            }
            break;
            case NF_KEY_S:                  // S
                OutString += String::CreateFromInt32(nSec);
            break;
            case NF_KEY_SS:                 // SS
            {
                if (nSec < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nSec);
            }
            break;
            default:
            break;
        }
    }
    if (bSign && rInfo.bThousand)
        OutString.Insert('-',0);
    return bRes;
}

BOOL SvNumberformat::ImpGetDateOutput(double fNumber,
                                   USHORT nIx,
                                   String& OutString)
{
    using namespace ::com::sun::star::i18n;
    BOOL bRes = FALSE;
    if (fabs(fNumber) > _D_MAX_LONG_)       // zu gross
    {
        OutString = rScan.GetErrorString();
        return FALSE;
    }
    long nNum = (long) floor(fNumber);
    Date aDate = *(rScan.GetNullDate()) + nNum;
    CalendarWrapper& rCal = GetCal();
    rCal.setGregorianDateTime( aDate );
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const USHORT nAnz = NumFor[nIx].GetnAnz();
    for (USHORT i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = TRUE;
                }
                break;
            case SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
                break;
            case SYMBOLTYPE_STRING:
            case SYMBOLTYPE_CURRENCY:
                OutString += rInfo.sStrArray[i];
                break;
            case NF_KEY_M:                  // M
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::MONTH ) + 1 );
            break;
            case NF_KEY_MM:                 // MM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH ) + 1;
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_MMM:                // MMM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::MONTH,
                    nVal, 0 );
            }
            break;
            case NF_KEY_MMMM:               // MMMM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::MONTH,
                    nVal, 1 );
            }
            break;
            case NF_KEY_Q:                  // Q
            case NF_KEY_QQ:                 // QQ
            {
                if (rInfo.nTypeArray[i] == NF_KEY_Q)
                    OutString += 'Q';
                USHORT nMonth = aDate.GetMonth();
                if (nMonth <= 3)
                    OutString += '1';
                else if (nMonth <=6)
                    OutString += '2';
                else if (nMonth <= 9)
                    OutString += '3';
                else
                    OutString += '4';
                if (rInfo.nTypeArray[i] == NF_KEY_QQ)
                {
                    OutString += '.';       // #40387# "1. Quartal" mit Leerzeichen
                    OutString += ' ';
                    OutString += rScan.GetQuarterString();
                }
            }
            break;
            case NF_KEY_T:                  // T
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::DAY_OF_MONTH ) );
            break;
            case NF_KEY_TT:                 // TT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_TTT:                // TTT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 0 );
            }
            break;
            case NF_KEY_TTTT:               // TTTT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
            }
            break;
            case NF_KEY_JJ:                 // JJ
            {
//! TODO: what about negative values? abs and append era?
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::YEAR );
                if ( 99 < nVal )
                    nVal %= 100;
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_JJJJ:               // JJJJ
//! TODO: what about negative values? abs and append era?
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::YEAR ) );
            break;
            case NF_KEY_EC:                 // E
            {
//! TODO: what about negative values? abs and append era?
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::YEAR );
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_EEC:                // EE
            {
//! TODO: what about negative values? abs and append era?
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::YEAR );
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_NN:                 // NN
            case NF_KEY_AAA:                // AAA
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 0 );
            }
            break;
            case NF_KEY_NNN:                // NNN
            case NF_KEY_AAAA:               // AAAA
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
            }
            break;
            case NF_KEY_NNNN:               // NNNN
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
                OutString += rLoc().getLongDateDayOfWeekSep();
            }
            break;
            case NF_KEY_WW :                // WW
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::WEEK_OF_YEAR ) );
            break;
            case NF_KEY_G:                  // G
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
//! TODO: is this right? what if the calendar is not Japanese?
                sal_Unicode cEra;
                switch ( nVal )
                {
                    case 0 :    cEra = 'M'; break;
                    case 1 :    cEra = 'T'; break;
                    case 2 :    cEra = 'S'; break;
                    case 3 :    cEra = 'H'; break;
                    default:
                        cEra = '?';
                        DBG_ERRORFILE( "SvNumberformat::ImpGetDateOutput: which era is it?" );
                }
                OutString += cEra;
            }
            break;
            case NF_KEY_GG:                 // GG
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::ERA,
                    nVal, 0 );
            }
            break;
            case NF_KEY_GGG:                // GGG
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::ERA,
                    nVal, 1 );
            }
            break;
            default:
            break;
        }
    }
    return bRes;
}

BOOL SvNumberformat::ImpGetDateTimeOutput(double fNumber,
                                       USHORT nIx,
                                       String& OutString)
{
    using namespace ::com::sun::star::i18n;
    BOOL bRes = FALSE;
    if (fabs(fNumber) > _D_MAX_LONG_)       // zu gross
    {
        OutString = rScan.GetErrorString();
        return FALSE;
    }
    double fNum1 = floor(fNumber);          // -> Datum
    double fNum2 = fNumber - fNum1;         // -> Zeit
    long nNum1 = (long) fNum1;

    DateTime aDateTime( *(rScan.GetNullDate()) );
    aDateTime += fNumber;
    CalendarWrapper& rCal = GetCal();
    rCal.setGregorianDateTime( aDateTime );

    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    double fTime = fNum2*86400.0;
    double fFactor = pow(10.0,rInfo.nCntPost);
    fTime = floor(0.5 + fTime * fFactor) / fFactor; // runden

    ULONG nSeconds = 0;
    ULONG nHours = 0;
    if( floor( fTime ) > _D_MAX_U_LONG_ )
    {
    if( (floor( fTime )/3600.0) > _D_MAX_U_LONG_ )
    {
        OutString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###" ) );
        return FALSE;
    }
    else
    {
        nSeconds = (ULONG)(((fTime/3600.0) - floor( fTime/3600.0 ))*3600.0);
        nHours = (ULONG)(fTime/3600.0);
    }
    }
    else
    nSeconds = (ULONG)floor( fTime );

    String sSecStr;
    SolarMath::DoubleToString(sSecStr, fTime-nSeconds, 'F', rInfo.nCntPost);
    sSecStr.EraseLeadingChars('0');
    sSecStr.EraseLeadingChars('.');
    xub_StrLen nSecPos = 0;                     // Zum Ziffernweisen
                                            // abarbeiten
    ULONG nHour, nMin, nSec;
    if (!rInfo.bThousand)      // [] Format
    {
        nHour = (nSeconds/3600) % 24;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    else if (rInfo.nThousand == 3) // [ss]
    {
        nHour = 0;
        nMin = 0;
        nSec = nSeconds;
    }
    else if (rInfo.nThousand == 2) // [mm]:ss
    {
        nHour = 0;
        nMin = nSeconds / 60;
        nSec = nSeconds % 60;
    }
    else if (rInfo.nThousand == 1) // [hh]:mm:ss
    {
        nHour = nSeconds / 3600;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    sal_Unicode cAmPm = ' ';                   // a oder p
    if (rInfo.nCntExp)     // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
            cAmPm = 'a';
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
                nHour -= 12;
        }
    }
    const USHORT nAnz = NumFor[nIx].GetnAnz();
    for (USHORT i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = TRUE;
                }
                break;
            case SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
                break;
            case SYMBOLTYPE_STRING:
            case SYMBOLTYPE_CURRENCY:
                OutString += rInfo.sStrArray[i];
                break;
            case SYMBOLTYPE_DIGIT:
            {
                const xub_StrLen nLen = rInfo.sStrArray[i].Len();
                const xub_StrLen nCntPost = xub_StrLen(rInfo.nCntPost);
                for (xub_StrLen j = 0; j < nLen && nSecPos < nCntPost; j++)
                {
                    OutString += sSecStr.GetChar(nSecPos);
                    nSecPos++;
                }
            }
            break;
            case NF_KEY_AMPM:               // AM/PM
            {
                if (cAmPm == 'a')
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::AM, 0 );
                else
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::PM, 0 );
            }
            break;
            case NF_KEY_AP:                 // A/P
            {
                if (cAmPm == 'a')
                    OutString += 'a';
                else
                    OutString += 'p';
            }
            break;
            case NF_KEY_MI:                 // M
                OutString += String::CreateFromInt32(nMin);
            break;
            case NF_KEY_MMI:                // MM
            {
                if (nMin < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nMin);
            }
            break;
            case NF_KEY_H:                  // H
                OutString += String::CreateFromInt32(nHour);
            break;
            case NF_KEY_HH:                 // HH
            {
                if (nHour < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nHour);
            }
            break;
            case NF_KEY_S:                  // S
                OutString += String::CreateFromInt32(nSec);
            break;
            case NF_KEY_SS:                 // SS
            {
                if (nSec < 10)
                    OutString += '0';
                OutString += String::CreateFromInt32(nSec);
            }
            break;
            case NF_KEY_M:                  // M
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::MONTH ) + 1 );
            break;
            case NF_KEY_MM:                 // MM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH ) + 1;
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_MMM:                // MMM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::MONTH,
                    nVal, 0 );
            }
            break;
            case NF_KEY_MMMM:               // MMMM
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::MONTH,
                    nVal, 1 );
            }
            break;
            case NF_KEY_Q:                  // Q
            case NF_KEY_QQ:                 // QQ
            {
                USHORT nMonth = aDateTime.GetMonth();
                if (nMonth <= 3)
                    OutString += '1';
                else if (nMonth <=6)
                    OutString += '2';
                else if (nMonth <= 9)
                    OutString += '3';
                else
                    OutString += '4';
                OutString += '.';       // #40387# "1. Quartal" mit Leerzeichen
                OutString += ' ';
                OutString += rScan.GetQuarterString();
                if (rInfo.nTypeArray[i] == NF_KEY_QQ)
                {
                    OutString += ' ';
                    OutString += String::CreateFromInt32(aDateTime.GetYear());
                }
            }
            break;
            case NF_KEY_T:                  // T
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::DAY_OF_MONTH ) );
            break;
            case NF_KEY_TT:                 // TT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_TTT:                // TTT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 0 );
            }
            break;
            case NF_KEY_TTTT:               // TTTT
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
            }
            break;
            case NF_KEY_JJ:                 // JJ
            {
                //! TODO: what about negative values? abs and era?
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::YEAR );
                if ( 99 < nVal )
                    nVal %= 100;
                if ( nVal < 10 )
                    OutString += '0';
                OutString += String::CreateFromInt32( nVal );
            }
            break;
            case NF_KEY_JJJJ:               // JJJJ
                //! TODO: what about negative values? abs and era?
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::YEAR ) );
            break;
            case NF_KEY_NN:                 // NN
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 0 );
            }
            break;
            case NF_KEY_NNN:                // NNN
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
            }
            break;
            case NF_KEY_NNNN:               // NNNN
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
                OutString += rCal.getDisplayName( CalendarDisplayIndex::DAY,
                    nVal, 1 );
                OutString += rLoc().getLongDateDayOfWeekSep();
            }
            break;
            case NF_KEY_WW :                // WW
                OutString += String::CreateFromInt32( rCal.getValue(
                    CalendarFieldIndex::WEEK_OF_YEAR ) );
            break;
            default:
            break;
        }
    }
    return bRes;
}

BOOL SvNumberformat::ImpGetNumberOutput(double fNumber,
                                     USHORT nIx,
                                     String& OutString)
{
    BOOL bRes = FALSE;
    BOOL bSign;
    if (fNumber < 0.0)
    {
        if (nIx == 0)                       // nicht in hinteren
            bSign = TRUE;                   // Formaten
        else
            bSign = FALSE;
        fNumber = -fNumber;
    }
    else
        bSign = FALSE;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == NUMBERFORMAT_PERCENT)
    {
        if (fNumber < _D_MAX_D_BY_100)
            fNumber *= 100.0;
        else
        {
            OutString = rScan.GetErrorString();
            return FALSE;
        }
    }
    USHORT i, j;
    xub_StrLen k;
    String sStr;
    long nPrecExp;
    BOOL bInteger = FALSE;
    if (rInfo.nThousand == FLAG_STANDARD_IN_FORMAT) // Hack: "Standard"
    {                                                            // im Format
        bSign = FALSE;                  // dann Vorzeichen per Format, nicht per Zahl
                                        // und keine Zahlformatiererei an dieser
    }                                   // Stelle!
    else
    {
        const USHORT nThousand = rInfo.nThousand;
        for (i = 0; i < nThousand; i++)
        {
           if (fNumber > _D_MIN_M_BY_1000)
               fNumber /= 1000.0;
           else
               fNumber = 0.0;
        }
        if (fNumber > 0.0)
            nPrecExp = GetPrecExp( fNumber );
        else
            nPrecExp = 0;
        if (rInfo.nCntPost)    // NachkommaStellen
        {
            if (rInfo.nCntPost + nPrecExp > 15 && nPrecExp < 15)
            {
                SolarMath::DoubleToString( sStr, fNumber, 'F', 15-nPrecExp );
                for (long l = 15-nPrecExp; l < (long) rInfo.nCntPost; l++)
                    sStr += '0';
            }
            else
                SolarMath::DoubleToString( sStr, fNumber, 'F', rInfo.nCntPost );
            sStr.EraseLeadingChars('0');        // fuehrende Nullen weg
        }
        else if (fNumber == 0.0)            // Null
        {
            // nothing to be done here, keep empty string sStr,
            // ImpNumberFillWithThousands does the rest
        }
        else                                // Integer
        {
            SolarMath::DoubleToString(sStr, fNumber, 'F', 0);
            sStr.EraseLeadingChars('0');        // fuehrende Nullen weg
        }
        xub_StrLen nPoint = sStr.Search( '.' );
        if ( nPoint != STRING_NOTFOUND )
        {
            register const sal_Unicode* p = sStr.GetBuffer() + nPoint;
            while ( *++p == '0' )
                ;
            if ( !*p )
                bInteger = TRUE;
            sStr.Erase( nPoint, 1 );            //  . herausnehmen
        }
        if (bSign &&
            (sStr.Len() == 0 || sStr.GetTokenCount('0') == sStr.Len()+1))   // nur 00000
            bSign = FALSE;              // nicht -0.00
    }                                   // End of != FLAG_STANDARD_IN_FORMAT

                                        // von hinten nach vorn
                                        // editieren:
    k = sStr.Len();                     // hinter letzter Ziffer
    j = NumFor[nIx].GetnAnz()-1;        // letztes Symbol
                                        // Nachkommastellen:
    if (rInfo.nCntPost > 0)
    {
        BOOL bTrailing = TRUE;          // ob Endnullen?
        BOOL bFilled = FALSE;           // ob aufgefuellt wurde ?
        short nType;
        while (j > 0 &&                 // rueckwaerts
           (nType = rInfo.nTypeArray[j]) != SYMBOLTYPE_DECSEP)
        {
            switch ( nType )
            {
                case SYMBOLTYPE_STAR:
                    if( bStarFlag )
                    {
                        sStr.Insert( (sal_Unicode) 0x1B, k /*++*/ );
                        sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                        bRes = TRUE;
                    }
                    break;
                case SYMBOLTYPE_BLANK:
                    /*k = */ InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                    break;
                case SYMBOLTYPE_STRING:
                case SYMBOLTYPE_CURRENCY:
                    sStr.Insert(rInfo.sStrArray[j],k);
                    break;
                case SYMBOLTYPE_THSEP:
                    if (rInfo.nThousand == 0)
                        sStr.Insert(rInfo.sStrArray[j],k);
                break;
                case SYMBOLTYPE_DIGIT:
                {
                    const String& rStr = rInfo.sStrArray[j];
                    const sal_Unicode* p1 = rStr.GetBuffer();
                    register const sal_Unicode* p = p1 + rStr.Len();
                    while ( p1 < p-- )
                    {
                        const sal_Unicode c = *p;
                        k--;
                        if ( sStr.GetChar(k) != '0' )
                            bTrailing = FALSE;
                        if (bTrailing)
                        {
                            if ( c == '0' )
                                bFilled = TRUE;
                            else if ( c == '-' )
                            {
                                if ( bInteger )
                                    sStr.SetChar( k, '-' );
                                bFilled = TRUE;
                            }
                            else if ( c == '?' )
                            {
                                sStr.SetChar( k, ' ' );
                                bFilled = TRUE;
                            }
                            else if ( !bFilled )    // #
                                sStr.Erase(k,1);
                        }
                    }                           // of for
                }                               // of case digi
                break;
                case NF_KEY_CCC:                // CCC-Waehrung
                    sStr.Insert(rLoc().getCurrBankSymbol(), k);
                break;
                case NF_KEY_GENERAL:            // Standard im String
                {
                    String sNum;
                    ImpGetOutputStandard(fNumber, sNum);
                    sNum.EraseLeadingChars('-');
                    sStr.Insert(sNum, k);
                }
                break;
                default:
                break;
            }                                   // of switch
            j--;
        }                                       // of while
    }                                           // of Nachkomma

    bRes |= ImpNumberFillWithThousands(sStr, fNumber, k, j, nIx, // ggfs Auffuellen mit .
                            rInfo.nCntPre);
    if ( rInfo.nCntPost > 0 )
    {
        const String& rDecSep = rLoc().getNumDecimalSep();
        xub_StrLen nLen = rDecSep.Len();
        if ( sStr.Len() > nLen && sStr.Equals( rDecSep, sStr.Len() - nLen, nLen ) )
            sStr.Erase( sStr.Len() - nLen );        // no decimals => strip DecSep
    }
    if (bSign)
        sStr.Insert('-',0);
    OutString = sStr;
    return bRes;
}

BOOL SvNumberformat::ImpNumberFillWithThousands(
                                String& sStr,             // Zahlstring
                                double& rNumber,
                                xub_StrLen k,               // Zeiger darin
                                USHORT j,                   // Symbolzeiger
                                USHORT nIx,                 // Teilformatstring
                                USHORT nDigCnt)             // Anz Dig. im For.
{
    BOOL bRes = FALSE;
    BOOL bLeading = FALSE;                              // fuehrende 0
    xub_StrLen nAnzLeadingChars = 0;        // Anzahl fuehrende Nullen oder Blanks
    USHORT nThousandCnt = 0;            // Anzahl Ziffern vor letztem .
    xub_StrLen nLeadingStringChars = 0;     // inserted StringChars vor Zahl
    USHORT nDigitCount = 0;             // Zaehlt Vorkommaziffern
    BOOL bStop = FALSE;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const String& rThousandSep = rLoc().getNumThousandSep();
    while (!bStop)                                      // rueckwaerts
    {
        if (j == 0)
            bStop = TRUE;
        switch (rInfo.nTypeArray[j])
        {
            case SYMBOLTYPE_DECSEP:
            case SYMBOLTYPE_STRING:
            case SYMBOLTYPE_CURRENCY:
                sStr.Insert(rInfo.sStrArray[j],k);
                if ( k == 0 )
                    nLeadingStringChars += rInfo.sStrArray[j].Len();
            break;
            case SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    sStr.Insert( (sal_Unicode) 0x1B, k/*++*/ );
                    sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                    bRes = TRUE;
                }
                break;
            case SYMBOLTYPE_BLANK:
                /*k = */ InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                break;
            case SYMBOLTYPE_THSEP:
            {
                if (k > 0 && rInfo.nThousand == 0)
                {
                    sStr.Insert(rInfo.sStrArray[j],k);
                    nThousandCnt = 0;
                }
            }
            break;
            case SYMBOLTYPE_DIGIT:
            {
                const String& rStr = rInfo.sStrArray[j];
                const sal_Unicode* p1 = rStr.GetBuffer();
                register const sal_Unicode* p = p1 + rStr.Len();
                while ( p1 < p-- )
                {
                    nDigitCount++;
                    const sal_Unicode c = *p;
//! TODO: what if rThousandSep is more than one charater? => change this damned backward loop
                    if ( c == rThousandSep.GetChar(0) && rThousandSep.Len() == 1 )
                    {
                        nDigitCount--;
                        if (k > 0)
                        {
                            sStr.Insert(c,k);
                            nThousandCnt = 0;
                        }
                    }
                    else if (k > 0)
                    {
                        k--;
                        nThousandCnt++;
                    }
                    else
                        bLeading = TRUE;
                    if (bLeading)
                    {
                        if (c == '?')
                        {
                            sStr.Insert(' ',0);
                            nAnzLeadingChars++;
                        }
                        else if (c == '0')
                        {
                            sStr.Insert('0',0);
                            nAnzLeadingChars++;
                        }
                    }
                    if (nDigitCount == nDigCnt && k > 0)
                    {   // mehr Zahlen als Stellen
                        ImpDigitFill(sStr, 0, k, nIx, nThousandCnt);
                    }
                }
            }
            break;
            case NF_KEY_CCC:                        // CCC-Waehrung
                sStr.Insert(rLoc().getCurrBankSymbol(), k);
            break;
            case NF_KEY_GENERAL:                    // Standard im String
            {
                String sNum;
                ImpGetOutputStandard(rNumber, sNum);
                sNum.EraseLeadingChars('-');
                sStr.Insert(sNum, k);
            }
            break;

            default:
            break;
        } // switch
        j--;        // naechster String
    } // while
    k += nLeadingStringChars + nAnzLeadingChars;
    if (k > nLeadingStringChars)
        ImpDigitFill(sStr, nLeadingStringChars, k, nIx, nThousandCnt);
    return bRes;
}

void SvNumberformat::ImpDigitFill(
        String& sStr,                   // Zahlstring
        xub_StrLen nStart,                  // Start der Ziffern
        xub_StrLen& k,                  // Zeiger darin
        USHORT nIx,                     // Index Teilstring
        USHORT nThousandCnt )           // Anzahl seit .
{
    if (NumFor[nIx].Info().bThousand)                       // noch Ziffern da
    {                                                       // Aufuellen mit .
        const String& rThousandSep = rLoc().getNumThousandSep();
        while (k > nStart)
        {
            if (nThousandCnt > 2)
            {                                       // hier muss . dazwischen
                sStr.Insert( rThousandSep, k );
                nThousandCnt = 1;
            }
            else
                nThousandCnt++;
            k--;
        }
    }
    else                                           // einfach ueberspringen
        k = nStart;
}

BOOL SvNumberformat::ImpNumberFill(String& sStr,        // Zahlstring
                                double& rNumber,            // Zahl fuer Standard
                                xub_StrLen& k,              // Zeigen darin
                                USHORT& j,                  // Symbolzeiger
                                USHORT nIx,                 // Teilformatstring
                                short eSymbolType )         // Abbruchtyp
{
    BOOL bRes = FALSE;
    k = sStr.Len();                         // hinter letzter Ziffer
    BOOL bLeading = FALSE;                  // fuehrende ? oder 0
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const String& rThousandSep = rLoc().getNumThousandSep();
    short nType;
    while (j > 0 && (nType = rInfo.nTypeArray[j]) != eSymbolType )
    {                                       // rueckwaerts:
        switch ( nType )
        {
            case SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    sStr.Insert( sal_Unicode(0x1B), k++ );
                    sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                    bRes = TRUE;
                }
                break;
            case SYMBOLTYPE_BLANK:
                k = InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                break;
            case SYMBOLTYPE_DIGIT:
            {
                const String& rStr = rInfo.sStrArray[j];
                const sal_Unicode* p1 = rStr.GetBuffer();
                register const sal_Unicode* p = p1 + rStr.Len();
                while ( p1 < p-- )
                {
                    const sal_Unicode c = *p;
//! TODO: what if rThousandSep is more than one charater? => change this damned backward loop
                    if ( c == rThousandSep.GetChar(0) && rThousandSep.Len() == 1 )
                    {
                        if (k > 0)
                            sStr.Insert(c,k);
                    }
                    else if (k > 0)
                        k--;
                    else
                        bLeading = TRUE;
                    if (bLeading)
                    {
                        if (c == '?')
                            sStr.Insert(' ',0);
                        else if (c == '0')
                            sStr.Insert('0',0);
                    }                           // of if
                }                               // of for
            }                                   // of case digi
            break;
            case NF_KEY_CCC:                // CCC-Waehrung
                sStr.Insert(rLoc().getCurrBankSymbol(), k);
            break;
            case NF_KEY_GENERAL:            // Standard im String
            {
                String sNum;
                ImpGetOutputStandard(rNumber, sNum);
                sNum.EraseLeadingChars('-');    // Vorzeichen weg!!
                sStr.Insert(sNum, k);
            }
            break;

            default:
                sStr.Insert(rInfo.sStrArray[j],k);
            break;
        }                                       // of switch
        j--;                                    // naechster String
    }                                           // of while
    return bRes;
}

void SvNumberformat::GetFormatSpecialInfo(BOOL& bThousand,
                                          BOOL& IsRed,
                                          USHORT& nPrecision,
                                          USHORT& nAnzLeading)
{
    const ImpSvNumberformatInfo& rInfo = NumFor[0].Info();
    bThousand = rInfo.bThousand;
    nPrecision = GetFormatPrecision();
    const Color* pColor = NumFor[1].GetColor();
    if (fLimit1 == 0.0 && fLimit2 == 0.0 && pColor
                       && (*pColor == rScan.GetRedColor()))
        IsRed = TRUE;
    else
        IsRed = FALSE;
    if (bStandard && rInfo.eScannedType == NUMBERFORMAT_NUMBER)
                                                        // StandardFormat
        nAnzLeading = 1;
    else
    {
        nAnzLeading = 0;
        BOOL bStop = FALSE;
        USHORT i = 0;
        const USHORT nAnz = NumFor[0].GetnAnz();
        while (!bStop && i < nAnz)
        {
            short nType = rInfo.nTypeArray[i];
            if ( nType == SYMBOLTYPE_DIGIT)
            {
                register const sal_Unicode* p = rInfo.sStrArray[i].GetBuffer();
                while ( *p == '#' )
                    p++;
                while ( *p++ == '0' )
                    nAnzLeading++;
            }
            else if (nType == SYMBOLTYPE_DECSEP)
                bStop = TRUE;
            i++;
        }
    }
}

const String* SvNumberformat::GetNumForString( USHORT nNumFor, USHORT nPos,
            BOOL bString /* = FALSE */ ) const
{
    if ( nNumFor > 3 )
        return NULL;
    USHORT nAnz = NumFor[nNumFor].GetnAnz();
    if ( !nAnz )
        return NULL;
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
        if ( bString )
        {   // rueckwaerts
            short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
            while ( nPos > 0 && (*pType != SYMBOLTYPE_STRING) &&
                    (*pType != SYMBOLTYPE_CURRENCY) )
            {
                pType--;
                nPos--;
            }
            if ( (*pType != SYMBOLTYPE_STRING) && (*pType != SYMBOLTYPE_CURRENCY) )
                return NULL;
        }
    }
    else if ( nPos > nAnz - 1 )
        return NULL;
    else if ( bString )
    {   // vorwaerts
        short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
        while ( nPos < nAnz && (*pType != SYMBOLTYPE_STRING) &&
                (*pType != SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( (*pType != SYMBOLTYPE_STRING) && (*pType != SYMBOLTYPE_CURRENCY) )
            return NULL;
    }
    return &NumFor[nNumFor].Info().sStrArray[nPos];
}


short SvNumberformat::GetNumForType( USHORT nNumFor, USHORT nPos,
            BOOL bString /* = FALSE */ ) const
{
    if ( nNumFor > 3 )
        return 0;
    USHORT nAnz = NumFor[nNumFor].GetnAnz();
    if ( !nAnz )
        return 0;
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
        if ( bString )
        {   // rueckwaerts
            short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
            while ( nPos > 0 && (*pType != SYMBOLTYPE_STRING) &&
                    (*pType != SYMBOLTYPE_CURRENCY) )
            {
                pType--;
                nPos--;
            }
            if ( (*pType != SYMBOLTYPE_STRING) && (*pType != SYMBOLTYPE_CURRENCY) )
                return 0;
        }
    }
    else if ( nPos > nAnz - 1 )
        return 0;
    else if ( bString )
    {   // vorwaerts
        short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
        while ( nPos < nAnz && (*pType != SYMBOLTYPE_STRING) &&
                (*pType != SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( (*pType != SYMBOLTYPE_STRING) && (*pType != SYMBOLTYPE_CURRENCY) )
            return 0;
    }
    return NumFor[nNumFor].Info().nTypeArray[nPos];
}


BOOL SvNumberformat::IsNegativeWithoutSign() const
{
    if ( IsNegativeRealNegative() )
    {
        const String* pStr = GetNumForString( 1, 0, TRUE );
        if ( pStr )
            return !HasStringNegativeSign( *pStr );
    }
    return FALSE;
}


DateFormat SvNumberformat::GetDateOrder() const
{
    if ( (eType & NUMBERFORMAT_DATE) == NUMBERFORMAT_DATE )
    {
        const short* pType = NumFor[0].Info().nTypeArray;
        USHORT nAnz = NumFor[0].GetnAnz();
        for ( USHORT j=0; j<nAnz; j++ )
        {
            switch ( pType[j] )
            {
                case NF_KEY_T :
                case NF_KEY_TT :
                    return DMY;
                break;
                case NF_KEY_M :
                case NF_KEY_MM :
                case NF_KEY_MMM :
                case NF_KEY_MMMM :
                    return MDY;
                break;
                case NF_KEY_JJ :
                case NF_KEY_JJJJ :
                    return YMD;
                break;
            }
        }
    }
    else
        DBG_ERROR( "SvNumberformat::GetDateOrder: kein Datum" );
    return rLoc().getDateFormat();
}


void SvNumberformat::GetConditions( SvNumberformatLimitOps& rOper1, double& rVal1,
                           SvNumberformatLimitOps& rOper2, double& rVal2 ) const
{
    rOper1 = eOp1;
    rOper2 = eOp2;
    rVal1  = fLimit1;
    rVal2  = fLimit2;
}


Color* SvNumberformat::GetColor( USHORT nNumFor ) const
{
    if ( nNumFor > 3 )
        return NULL;

    return NumFor[nNumFor].GetColor();
}


void lcl_SvNumberformat_AddLimitStringImpl( String& rStr,
            SvNumberformatLimitOps eOp, double fLimit, const String& rDecSep )
{
    if ( eOp != NUMBERFORMAT_OP_NO )
    {
        switch ( eOp )
        {
            case NUMBERFORMAT_OP_EQ :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[=" ) );
            break;
            case NUMBERFORMAT_OP_NE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<>" ) );
            break;
            case NUMBERFORMAT_OP_LT :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<" ) );
            break;
            case NUMBERFORMAT_OP_LE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<=" ) );
            break;
            case NUMBERFORMAT_OP_GT :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[>" ) );
            break;
            case NUMBERFORMAT_OP_GE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[>=" ) );
            break;
        }
        SolarMath::DoubleToString( rStr, fLimit, 'A', INT_MAX, rDecSep.GetChar(0), TRUE );
        rStr += ']';
    }
}


String SvNumberformat::GetMappedFormatstring(
        const NfKeywordTable& rKeywords, const LocaleDataWrapper& rLoc,
        BOOL bDontQuote ) const
{
    String aStr;
    // 1 subformat matches all if no condition specified
    BOOL bDefault1 = ( NumFor[1].GetnAnz() == 0 && eOp1 == NUMBERFORMAT_OP_NO );
    // with 2 subformats [>=0];[<0] is implied if no condition specified
    BOOL bDefault2 = ( !bDefault1 && NumFor[2].GetnAnz() == 0 &&
        eOp1 == NUMBERFORMAT_OP_GE && fLimit1 == 0.0 &&
        eOp2 == NUMBERFORMAT_OP_NO && fLimit2 == 0.0 );
    // with 3 subformats [>0];[<0];[=0] is implied if no condition specified
    BOOL bDefault3 = ( !bDefault2 && NumFor[2].GetnAnz() > 0 &&
        eOp1 == NUMBERFORMAT_OP_GT && fLimit1 == 0.0 &&
        eOp2 == NUMBERFORMAT_OP_LT && fLimit2 == 0.0 );
    BOOL bDefault = bDefault1 || bDefault2 || bDefault3;
    int nSem = 0;       // needed ';' delimiters
    for ( int n=0; n<4; n++ )
    {
        if ( n > 0 )
            nSem++;

        String aPrefix;

        if ( !bDefault )
        {
            switch ( n )
            {
                case 0 :
                    lcl_SvNumberformat_AddLimitStringImpl( aPrefix, eOp1,
                        fLimit1, rLoc.getNumDecimalSep() );
                break;
                case 1 :
                    lcl_SvNumberformat_AddLimitStringImpl( aPrefix, eOp2,
                        fLimit2, rLoc.getNumDecimalSep() );
                break;
            }
        }

        const String& rColorName = NumFor[n].GetColorName();
        if ( rColorName.Len() )
        {
            const String* pKey = rScan.GetKeyword() + NF_KEY_FIRSTCOLOR;
            for ( int j=NF_KEY_FIRSTCOLOR; j<=NF_KEY_LASTCOLOR; j++, pKey++ )
            {
                if ( *pKey == rColorName )
                {
                    aPrefix += '[';
                    aPrefix += rKeywords[j];
                    aPrefix += ']';
                    break;  // for
                }
            }
        }

        USHORT nAnz = NumFor[n].GetnAnz();
        if ( nAnz || aPrefix.Len() )
        {
            while ( nSem )
            {
                aStr += ';';
                nSem--;
            }
        }

        if ( aPrefix.Len() )
            aStr += aPrefix;

        if ( nAnz )
        {
            const short* pType = NumFor[n].Info().nTypeArray;
            const String* pStr = NumFor[n].Info().sStrArray;
            for ( USHORT j=0; j<nAnz; j++ )
            {
                if ( 0 <= pType[j] && pType[j] < NF_KEYWORD_ENTRIES_COUNT )
                {
                    aStr += rKeywords[pType[j]];
                    if( NF_KEY_NNNN == pType[j] )
                        aStr += rLoc.getLongDateDayOfWeekSep();
                }
                else
                {
                    switch ( pType[j] )
                    {
                        case SYMBOLTYPE_DECSEP :
                            aStr += rLoc.getNumDecimalSep();
                        break;
                        case SYMBOLTYPE_THSEP :
                            aStr += rLoc.getNumThousandSep();
                        break;
                        case SYMBOLTYPE_STRING :
                            if( bDontQuote )
                                aStr += pStr[j];
                            else if ( pStr[j].Len() == 1 )
                            {
                                sal_Unicode cx = pStr[j].GetChar(0);
                                switch ( cx )
                                {
                                    case '+' :
                                    case '-' :
                                    case ' ' :
                                        aStr += cx;     // don't escape simple forms
                                    break;
                                    default:
                                        aStr += '\\';
                                        aStr += cx;
                                }
                            }
                            else
                            {
                                aStr += '"';
                                aStr += pStr[j];
                                aStr += '"';
                            }
                            break;
                        default:
                            aStr += pStr[j];
                    }

                }
            }
        }
    }
    return aStr;
}


// static
BOOL SvNumberformat::HasStringNegativeSign( const String& rStr )
{
    // fuer Sign muss '-' am Anfang oder am Ende des TeilStrings sein (Blanks ignored)
    xub_StrLen nLen = rStr.Len();
    if ( !nLen )
        return FALSE;
    const sal_Unicode* const pBeg = rStr.GetBuffer();
    const sal_Unicode* const pEnd = pBeg + nLen;
    register const sal_Unicode* p = pBeg;
    do
    {   // Anfang
        if ( *p == '-' )
            return TRUE;
    } while ( *p == ' ' && ++p < pEnd );
    p = pEnd - 1;
    do
    {   // Ende
        if ( *p == '-' )
            return TRUE;
    } while ( *p == ' ' && pBeg < --p );
    return FALSE;
}


// static
void SvNumberformat::SetComment( const String& rStr, String& rFormat,
        String& rComment )
{
    if ( rComment.Len() )
    {   // alten Kommentar aus Formatstring loeschen
        //! nicht per EraseComment, der Kommentar muss matchen
        String aTmp( '{' );
        aTmp += ' ';
        aTmp += rComment;
        aTmp += ' ';
        aTmp += '}';
        xub_StrLen nCom = 0;
        do
        {
            nCom = rFormat.Search( aTmp, nCom );
        } while ( (nCom != STRING_NOTFOUND) && (nCom + aTmp.Len() != rFormat.Len()) );
        if ( nCom != STRING_NOTFOUND )
            rFormat.Erase( nCom );
    }
    if ( rStr.Len() )
    {   // neuen Kommentar setzen
        rFormat += '{';
        rFormat += ' ';
        rFormat += rStr;
        rFormat += ' ';
        rFormat += '}';
        rComment = rStr;
    }
}


// static
void SvNumberformat::EraseCommentBraces( String& rStr )
{
    xub_StrLen nLen = rStr.Len();
    if ( nLen && rStr.GetChar(0) == '{' )
    {
        rStr.Erase( 0, 1 );
        --nLen;
    }
    if ( nLen && rStr.GetChar(0) == ' ' )
    {
        rStr.Erase( 0, 1 );
        --nLen;
    }
    if ( nLen && rStr.GetChar( nLen-1 ) == '}' )
        rStr.Erase( --nLen, 1 );
    if ( nLen && rStr.GetChar( nLen-1 ) == ' ' )
        rStr.Erase( --nLen, 1 );
}


// static
void SvNumberformat::EraseComment( String& rStr )
{
    register const sal_Unicode* p = rStr.GetBuffer();
    BOOL bInString = FALSE;
    BOOL bEscaped = FALSE;
    BOOL bFound = FALSE;
    xub_StrLen nPos;
    while ( !bFound && *p )
    {
        switch ( *p )
        {
            case '\\' :
                bEscaped = !bEscaped;
            break;
            case '\"' :
                if ( !bEscaped )
                    bInString = !bInString;
            break;
            case '{' :
                if ( !bEscaped && !bInString )
                {
                    bFound = TRUE;
                    nPos = p - rStr.GetBuffer();
                }
            break;
        }
        if ( bEscaped && *p != '\\' )
            bEscaped = FALSE;
        ++p;
    }
    if ( bFound )
        rStr.Erase( nPos );
}


// static
BOOL SvNumberformat::IsInQuote( const String& rStr, xub_StrLen nPos,
            sal_Unicode cQuote, sal_Unicode cEscIn, sal_Unicode cEscOut )
{
    xub_StrLen nLen = rStr.Len();
    if ( nPos >= nLen )
        return FALSE;
    register const sal_Unicode* p0 = rStr.GetBuffer();
    register const sal_Unicode* p = p0;
    register const sal_Unicode* p1 = p0 + nPos;
    BOOL bQuoted = FALSE;
    while ( p <= p1 )
    {
        if ( *p == cQuote )
        {
            if ( p == p0 )
                bQuoted = TRUE;
            else if ( bQuoted )
            {
                if ( *(p-1) != cEscIn )
                    bQuoted = FALSE;
            }
            else
            {
                if ( *(p-1) != cEscOut )
                    bQuoted = TRUE;
            }
        }
        p++;
    }
    return bQuoted;
}


// static
xub_StrLen SvNumberformat::GetQuoteEnd( const String& rStr, xub_StrLen nPos,
            sal_Unicode cQuote, sal_Unicode cEscIn, sal_Unicode cEscOut )
{
    xub_StrLen nLen = rStr.Len();
    if ( nPos >= nLen )
        return STRING_NOTFOUND;
    if ( !IsInQuote( rStr, nPos, cQuote, cEscIn, cEscOut ) )
    {
        if ( rStr.GetChar( nPos ) == cQuote )
            return nPos;        // schliessendes cQuote
        return STRING_NOTFOUND;
    }
    register const sal_Unicode* p0 = rStr.GetBuffer();
    register const sal_Unicode* p = p0 + nPos;
    register const sal_Unicode* p1 = p0 + nLen;
    while ( p < p1 )
    {
        if ( *p == cQuote && p > p0 && *(p-1) != cEscIn )
            return p - p0;
        p++;
    }
    return nLen;        // String Ende
}



