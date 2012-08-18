/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <limits.h>
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>

// Beschreibung: Berechnet den groessten gemeinsamen Teiler von
//               nVal1 und nVal2
// Parameter     long nVal1, long nVal2

// Die Funktion GetGGT berechnet den groessten gemeinsamen Teiler der
// beiden als Parameter uebergebenen Werte nVal1 und nVal2 nach dem
// Algorithmus von Euklid. Hat einer der beiden Parameter den Wert 0 oder
// 1, so wird als Ergebnis der Wert 1 zur�ckgegeben. Da der Algorithmus
// nur mit positiven Zahlen arbeitet, werden die beiden Parameter
// entsprechend umgewandelt.
// Zum Algorithmus: die beiden Parameter werden solange ducheinander
//              geteilt, bis sie beide gleich sind oder bis bei der Division
//              kein Rest bleibt. Der kleinere der beiden Werte ist dann der
//              GGT.
static long GetGGT( long nVal1, long nVal2 )
{
    nVal1 = Abs( nVal1 );
    nVal2 = Abs( nVal2 );

    if ( nVal1 <= 1 || nVal2 <= 1 )
        return 1;

    while ( nVal1 != nVal2 )
    {
        if ( nVal1 > nVal2 )
        {
            nVal1 %= nVal2;
            if ( nVal1 == 0 )
                return nVal2;
        }
        else
        {
            nVal2 %= nVal1;
            if ( nVal2 == 0 )
                return nVal1;
        }
    }
    return nVal1;
}

static void Reduce( BigInt &rVal1, BigInt &rVal2 )
{
    BigInt nA( rVal1 );
    BigInt nB( rVal2 );
    nA.Abs();
    nB.Abs();

    if ( nA.IsOne() || nB.IsOne() || nA.IsZero() || nB.IsZero() )
        return;

    while ( nA != nB )
    {
        if ( nA > nB )
        {
            nA %= nB;
            if ( nA.IsZero() )
            {
                rVal1 /= nB;
                rVal2 /= nB;
                return;
            }
        }
        else
        {
            nB %= nA;
            if ( nB.IsZero() )
            {
                rVal1 /= nA;
                rVal2 /= nA;
                return;
            }
        }
    }

    rVal1 /= nA;
    rVal2 /= nB;
}

// Zur Initialisierung eines Bruches wird nNum dem Zaehler und nDen dem
// Nenner zugewiesen. Da negative Werte des Nenners einen Bruch als
// ungueltig kennzeichnen, wird bei der Eingabe eines negativen Nenners
// sowohl das Vorzeichen des Nenners und des Zaehlers invertiert um wieder
// einen gueltigen Wert fuer den Bruch zu erhalten.
Fraction::Fraction( long nNum, long nDen )
{
    nNumerator = nNum;
    nDenominator = nDen;
    if ( nDenominator < 0 )
    {
        nDenominator = -nDenominator;
        nNumerator   = -nNumerator;
    }

    // Kuerzen ueber Groesste Gemeinsame Teiler
    long n = GetGGT( nNumerator, nDenominator );
    nNumerator   /= n;
    nDenominator /= n;
}

// Wenn der Wert von dVal groesser ist als LONG_MAX, dann wird der Bruch
// auf den Wert ungueltig gesetzt, ansonsten werden dVal und der Nenner
// solange mit 10 multipliziert, bis entweder der Zaehler oder der Nenner
// groesser als LONG_MAX / 10 ist. Zum Schluss wird der so entstandene Bruch
// gekuerzt.
Fraction::Fraction( double dVal )
{
    long nDen = 1;
    long nMAX = LONG_MAX / 10;

    if ( dVal > LONG_MAX || dVal < LONG_MIN )
    {
        nNumerator   = 0;
        nDenominator = -1;
        return;
    }

    while ( Abs( (long)dVal ) < nMAX && nDen < nMAX )
    {
        dVal *= 10;
        nDen *= 10;
    }
    nNumerator   = (long)dVal;
    nDenominator = nDen;

    // Kuerzen ueber Groesste Gemeinsame Teiler
    long n = GetGGT( nNumerator, nDenominator );
    nNumerator   /= n;
    nDenominator /= n;
}

Fraction::operator double() const
{
    if ( nDenominator > 0 )
        return (double)nNumerator / (double)nDenominator;
    else
        return (double)0;
}

// Zunaechst werden die beiden Parameter auf ihre Gueltigkeit ueberprueft.
// Ist einer der Parameter ungueltig, dann ist auch des Ergebnis
// ungueltig. Zur Addition werden die beiden Brueche erst durch
// Erweiterung mit den Nenner des jeweils anderen Bruches auf einen
// gemeinsamen Nenner gebracht. Anschliessend werden die beiden Zaehler
// addiert und das Ergebnis gekuerzt (durch Division von Zaehler und
// Nenner mit nGGT). Innerhalb der Funktion wird mit dem Datentyp SLong
// gerechnet, um einen Moeglichen Ueberlauf erkennen zu koennen. Bei
// einem Ueberlauf wird das Ergebnis auf den Wert ungueltig gesetzt.
Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    // (a/b) + (c/d) = ( (a*d) + (c*b) ) / (b*d)
    BigInt nN( nNumerator );
    nN *= BigInt( rVal.nDenominator );
    BigInt nW1Temp( nDenominator );
    nW1Temp *= BigInt( rVal.nNumerator );
    nN += nW1Temp;

    BigInt nD( nDenominator );
    nD *= BigInt( rVal.nDenominator );

    Reduce( nN, nD );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}

// Zunaechst werden die beiden Parameter auf ihre Gueltigkeit ueberprueft.
// Ist einer der Parameter ungueltig, dann ist auch des Ergebnis
// ungueltig. Zur Subtraktion werden die beiden Brueche erst durch
// Erweiterung mit den Nenner des jeweils anderen Bruches auf einen
// gemeinsamen Nenner gebracht. Anschliessend werden die beiden Zaehler
// subtrahiert und das Ergebnis gekuerzt (durch Division von Zaehler und
// Nenner mit nGGT). Innerhalb der Funktion wird mit dem Datentyp BigInt
// gerechnet, um einen Moeglichen Ueberlauf erkennen zu koennen. Bei
// einem Ueberlauf wird das Ergebnis auf den Wert ungueltig gesetzt.
Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    // (a/b) - (c/d) = ( (a*d) - (c*b) ) / (b*d)
    BigInt nN( nNumerator );
    nN *= BigInt( rVal.nDenominator );
    BigInt nW1Temp( nDenominator );
    nW1Temp *= BigInt( rVal.nNumerator );
    nN -= nW1Temp;

    BigInt nD( nDenominator );
    nD *= BigInt( rVal.nDenominator );

    Reduce( nN, nD );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}

// Zunaechst werden die beiden Parameter auf ihre Gueltigkeit ueberprueft.
// Ist einer der Parameter ungueltig, dann ist auch des Ergebnis
// ungueltig. Zur Multiplikation werden jeweils die beiden Zaehler und
// Nenner miteinander multipliziert. Um Ueberlaufe zu vermeiden, werden
// vorher jeweils der GGT zwischen dem Zaehler des einen und dem Nenner
// des anderen Bruches bestimmt und bei der Multiplikation Zaehler und
// Nenner durch die entsprechenden Werte geteilt.
// Innerhalb der Funktion wird mit dem Datentyp BigInt gerechnet, um
// einen Moeglichen Ueberlauf erkennen zu koennen. Bei einem Ueberlauf
// wird das Ergebnis auf den Wert ungueltig gesetzt.
Fraction& Fraction::operator *= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    long nGGT1 = GetGGT( nNumerator, rVal.nDenominator );
    long nGGT2 = GetGGT( rVal.nNumerator, nDenominator );
    BigInt nN( nNumerator / nGGT1 );
    nN *= BigInt( rVal.nNumerator / nGGT2 );
    BigInt nD( nDenominator / nGGT2 );
    nD *= BigInt( rVal.nDenominator / nGGT1 );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}

// Zunaechst werden die beiden Parameter auf ihre Gueltigkeit ueberprueft.
// Ist einer der Parameter ungueltig, dann ist auch des Ergebnis
// ungueltig.
// Um den Bruch a durch b zu teilen, wird a mit dem Kehrwert von b
// multipliziert. Analog zu Multiplikation wird jezt jeweils der Zaehler
// des einen Bruches mit dem Nenner des anderen multipliziert.
// Um Ueberlaufe zu vermeiden, werden vorher jeweils der GGT zwischen den
// beiden Zaehlern und den beiden Nennern bestimmt und bei der
// Multiplikation Zaehler und Nenner durch die entsprechenden Werte
// geteilt.
// Innerhalb der Funktion wird mit dem Datentyp BigInt gerechnet, um
// einen Moeglichen Ueberlauf erkennen zu koennen. Bei einem Ueberlauf
// wird das Ergebnis auf den Wert ungueltig gesetzt.
Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    long nGGT1 = GetGGT( nNumerator, rVal.nNumerator );
    long nGGT2 = GetGGT( rVal.nDenominator, nDenominator );
    BigInt nN( nNumerator / nGGT1 );
    nN *= BigInt( rVal.nDenominator / nGGT2 );
    BigInt nD( nDenominator / nGGT2 );
    nD *= BigInt( rVal.nNumerator / nGGT1 );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
        if ( nDenominator < 0 )
        {
            nDenominator = -nDenominator;
            nNumerator   = -nNumerator;
        }
    }

    return *this;
}

// Similar to clz_table that can be googled
const char nbits_table[32] =
{
    32,  1, 23,  2, 29, 24, 14,  3,
    30, 27, 25, 18, 20, 15, 10,  4,
    31, 22, 28, 13, 26, 17, 19,  9,
    21, 12, 16,  8, 11,  7,  6,  5
};

static int impl_NumberOfBits( unsigned long nNum )
{
    // http://en.wikipedia.org/wiki/De_Bruijn_sequence
    // background paper: Using de Bruijn Sequences to Index a 1 in a
    // Computer Word (1998) Charles E. Leiserson,
    // Harald Prokop, Keith H. Randall
    // (e.g. http://citeseer.ist.psu.edu/leiserson98using.html)
    const sal_uInt32 nDeBruijn = 0x7DCD629;

    if ( nNum == 0 )
        return 0;

    // Get it to form like 0000001111111111b
    nNum |= ( nNum >>  1 );
    nNum |= ( nNum >>  2 );
    nNum |= ( nNum >>  4 );
    nNum |= ( nNum >>  8 );
    nNum |= ( nNum >> 16 );

    sal_uInt32 nNumber;
    int nBonus = 0;

#if SAL_TYPES_SIZEOFLONG == 4
    nNumber = nNum;
#elif SAL_TYPES_SIZEOFLONG == 8
    nNum |= ( nNum >> 32 );

    if ( nNum & 0x80000000 )
    {
        nNumber = sal_uInt32( nNum >> 32 );
        nBonus = 32;

        if ( nNumber == 0 )
            return 32;
    }
    else
        nNumber = sal_uInt32( nNum & 0xFFFFFFFF );
#else
#error "Unknown size of long!"
#endif

    // De facto shift left of nDeBruijn using multiplication (nNumber
    // is all ones from topmost bit, thus nDeBruijn + (nDeBruijn *
    // nNumber) => nDeBruijn * (nNumber+1) clears all those bits to
    // zero, sets the next bit to one, and thus effectively shift-left
    // nDeBruijn by lg2(nNumber+1). This generates a distinct 5bit
    // sequence in the msb for each distinct position of the last
    // leading 0 bit - that's the property of a de Bruijn number.
    nNumber = nDeBruijn + ( nDeBruijn * nNumber );

    // 5-bit window indexes the result
    return ( nbits_table[nNumber >> 27] ) + nBonus;
}

/** Inaccurate cancellation for a fraction.

    Clip both nominator and denominator to said number of bits. If
    either of those already have equal or less number of bits used,
    this method does nothing.

    @param nSignificantBits denotes, how many significant binary
    digits to maintain, in both nominator and denominator.

    @example ReduceInaccurate(8) has an error <1% [1/2^(8-1)] - the
    largest error occurs with the following pair of values:

    binary    1000000011111111111111111111111b/1000000000000000000000000000000b
    =         1082130431/1073741824
    = approx. 1.007812499

    A ReduceInaccurate(8) yields 1/1.
*/
void Fraction::ReduceInaccurate( unsigned nSignificantBits )
{
    if ( !nNumerator || !nDenominator )
        return;

    // Count with unsigned longs only
    const bool bNeg = ( nNumerator < 0 );
    unsigned long nMul = (unsigned long)( bNeg? -nNumerator: nNumerator );
    unsigned long nDiv = (unsigned long)( nDenominator );

    DBG_ASSERT(nSignificantBits<65, "More than 64 bit of significance is overkill!");

    // How much bits can we lose?
    const int nMulBitsToLose = Max( ( impl_NumberOfBits( nMul ) - int( nSignificantBits ) ), 0 );
    const int nDivBitsToLose = Max( ( impl_NumberOfBits( nDiv ) - int( nSignificantBits ) ), 0 );

    const int nToLose = Min( nMulBitsToLose, nDivBitsToLose );

    // Remove the bits
    nMul >>= nToLose;
    nDiv >>= nToLose;

    if ( !nMul || !nDiv )
    {
        // Return without reduction
        OSL_FAIL( "Oops, we reduced too much..." );
        return;
    }

    // Reduce
    long n1 = GetGGT( nMul, nDiv );
    if ( n1 != 1 )
    {
        nMul /= n1;
        nDiv /= n1;
    }

    nNumerator = bNeg? -long( nMul ): long( nMul );
    nDenominator = nDiv;
}

bool operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    return rVal1.nNumerator == rVal2.nNumerator
           && rVal1.nDenominator == rVal2.nDenominator;
}

// Beide Operanden werden zunaechst auf ihre Gueltigkeit ueberprueft und
// anschliessend zur Sicherheit noch einmal gekuerzt. Um die Brueche
// (a/b) und (c/d) zu vergleichen, werden sie zunaechst auf einen
// gemeinsamen Nenner gebracht (b*d), um dann die beiden Zaehler (a*d)
// und (c*b) zu vergleichen. Das Ergebnis dieses Vergleichs wird
// zurueckgegeben.
bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator );
    nD *= BigInt( rVal2.nNumerator );

    return nN < nD;
}

// Beide Operanden werden zunaechst auf ihre Gueltigkeit ueberprueft und
// anschliessend zur Sicherheit noch einmal gekuerzt. Um die Brueche
// (a/b) und (c/d) zu vergleichen, werden sie zunaechst auf einen
// gemeinsamen Nenner gebracht (b*d), um dann die beiden Zaehler (a*d)
// und (c*b) zu vergleichen. Das Ergebnis dieses Vergleichs wird
// zurueckgegeben.
bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator);
    nD *= BigInt( rVal2.nNumerator );

    return nN > nD;
}

SvStream& operator >> ( SvStream& rIStream, Fraction& rFract )
{
    //fdo#39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmp(0);
    rIStream >> nTmp;
    rFract.nNumerator = nTmp;
    rIStream >> nTmp;
    rFract.nDenominator = nTmp;
    return rIStream;
}

SvStream& operator << ( SvStream& rOStream, const Fraction& rFract )
{
    //fdo#39428 SvStream no longer supports operator<<(long)
    rOStream << sal::static_int_cast<sal_Int32>(rFract.nNumerator);
    rOStream << sal::static_int_cast<sal_Int32>(rFract.nDenominator);
    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
