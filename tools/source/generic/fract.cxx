/*************************************************************************
 *
 *  $RCSfile: fract.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
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

#ifndef _LIMITS_H
#include <limits.h>
#endif

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif

#ifndef _FRACT_HXX
#include <fract.hxx>
#endif

#define private public
#define bIsLong bIsBig
#include <bigint.hxx>
#include <stream.hxx>

/*************************************************************************
|*
|*    GetGGT()
|*
|*    Beschreibung      Berechnet den groessten gemeinsamen Teiler von
|*                      nVal1 und nVal2
|*    Parameter         long nVal1, long nVal2
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

// Die Funktion GetGGT berechnet den groessten gemeinsamen Teiler der
// beiden als Parameter uebergebenen Werte nVal1 und nVal2 nach dem
// Algorithmus von Euklid. Hat einer der beiden Parameter den Wert 0 oder
// 1, so wird als Ergebnis der Wert 1 zurückgegeben. Da der Algorithmus
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

/*************************************************************************
|*
|*    Fraction::Fraction()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    WP 07.03.97
|*    Letzte Aenderung
|*
*************************************************************************/

Fraction::Fraction( long nN1, long nN2, long nD1, long nD2 )
{
    long n;
    int  i = 1;

    if( nN1 < 0 ) { i = -i; nN1 = -nN1; }
    if( nN2 < 0 ) { i = -i; nN2 = -nN2; }
    if( nD1 < 0 ) { i = -i; nD1 = -nD1; }
    if( nD2 < 0 ) { i = -i; nD2 = -nD2; }

    n = GetGGT( nN1, nD1 ); if( n > 1 ) { nN1 /= n; nD1 /= n; }
    n = GetGGT( nN1, nD2 ); if( n > 1 ) { nN1 /= n; nD2 /= n; }
    n = GetGGT( nN2, nD1 ); if( n > 1 ) { nN2 /= n; nD1 /= n; }
    n = GetGGT( nN2, nD2 ); if( n > 1 ) { nN2 /= n; nD2 /= n; }

    BigInt nN( nN1 );
    nN *= BigInt( nN2 );

    BigInt nD( nD1 );
    nD *= BigInt( nD2 );

    while ( nN.bIsLong || nD.bIsLong )
    {
        BigInt n1 = 1;
        BigInt n2 = 2;

        nN += n1;
        nN /= n2;
        nD += n1;
        nD /= n2;

        // Kuerzen ueber Groesste Gemeinsame Teiler
        Reduce( nN, nD );
    }

    nNumerator   = i * (long)nN;
    nDenominator = (long)nD;
}

/*************************************************************************
|*
|*    Fraction::Fraction()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Fraction::Fraction()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    Fraction::operator double()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 14.05.91
|*
*************************************************************************/

Fraction::operator double() const
{
    if ( nDenominator > 0 )
        return (double)nNumerator / (double)nDenominator;
    else
        return (double)0;
}

/*************************************************************************
|*
|*    Fraction::operator+=()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

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

    if ( nN.bIsLong || nD.bIsLong )
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

/*************************************************************************
|*
|*    Fraction::operator-=()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

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

    if ( nN.bIsLong || nD.bIsLong )
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

/*************************************************************************
|*
|*    Fraction::operator*=()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  TH 19.08.92
|*
*************************************************************************/

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

    if ( nN.bIsLong || nD.bIsLong )
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

/*************************************************************************
|*
|*    Fraction::operator/=()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

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

    if ( nN.bIsLong || nD.bIsLong )
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

/*************************************************************************
|*
|*    Fraction::ReduceInaccurate()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    JOE 17.09.95
|*    Letzte Aenderung  JOE 17.09.95
|*
*************************************************************************/

// Funktioniert z.Zt. nur fuer 32-Bit Werte !!!
// Fehlerbehaftetes Kuerzen einer Fraction.
// nSignificantBits gibt an, wieviele signifikante Binaerstellen
// in Zaehler/Nenner mindestens erhalten bleiben sollen.
// Beispiel: ReduceInaccurate(8) hat einen Fehler <1% [1/2^(8-1)]
// dabei tritt der groesste Fehler bei folgendem Wertepaar auf:
// Binaer 1000000011111111111111111111111b/1000000000000000000000000000000b
// =      1082130431/1073741824
// = ca.  1.007812499
// Nach ReduceInaccurate( 8 ) wird daraus 1/1.

void Fraction::ReduceInaccurate( unsigned nSignificantBits )
{
    if ( !nNumerator || !nDenominator )
        return;

    // Zaehler und Nenner auf den Stack fuer schnelleren Zugriff
    UINT32 nMul;
    UINT32 nDiv;
    BOOL   bNeg;
    if ( nNumerator >= 0 )
    {
        nMul = (UINT32)nNumerator;
        bNeg = FALSE;
    }
    else
    {
        nMul = (UINT32)(-nNumerator);
        bNeg = TRUE;
    }
    nDiv=(UINT32)nDenominator;

    UINT32 a=nMul; unsigned nMulZ=0; // Fuehrende Nullen zaehlen
    while (a<0x00800000) { nMulZ+=8; a<<=8; }
    while (a<0x80000000) { nMulZ++; a<<=1; }
    a=nDiv; unsigned nDivZ=0; // Fuehrende Nullen zaehlen
    while (a<0x00800000) { nDivZ+=8; a<<=8; }
    while (a<0x80000000) { nDivZ++; a<<=1; }
    // Anzahl der verwendeten Digits bestimmen
    // Auch hier gehe ich davon aus, dass es sich um 32Bit-Werte handelt
    int nMulDigits=32-nMulZ;
    int nDivDigits=32-nDivZ;
    // Nun bestimmen, wieviele Stellen hinten weg koennen
    // Hier koennte man das Ergebnis noch etwas optimieren...
    int nMulWeg=nMulDigits-nSignificantBits; if (nMulWeg<0) nMulWeg=0;
    int nDivWeg=nDivDigits-nSignificantBits; if (nDivWeg<0) nDivWeg=0;
    int nWeg=Min(nMulWeg,nDivWeg);
    nMul>>=nWeg;
    nDiv>>=nWeg;
    if ( !nMul || !nDiv )
    {
        DBG_ERROR( "Oups, beim kuerzen einer Fraction hat sich Joe verrechnet." );
        return;
    }

    // Nun noch kuerzen ueber GGT
    long n1=GetGGT( nMul, nDiv );
    if ( n1!=1 )
    {
        nMul/=n1;
        nDiv/=n1;
    }
    if ( !bNeg )
        nNumerator = (long)nMul;
    else
        nNumerator = -(long)nMul;
    nDenominator = nDiv;
}

/*************************************************************************
|*
|*    Fraction::operator ==()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  TH 19.08.92
|*
*************************************************************************/

BOOL operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return FALSE;

    return rVal1.nNumerator == rVal2.nNumerator
           && rVal1.nDenominator == rVal2.nDenominator;
}

/*************************************************************************
|*
|*    Fraction::operator <()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  DV 21.12.92
|*
*************************************************************************/

// Beide Operanden werden zunaechst auf ihre Gueltigkeit ueberprueft und
// anschliessend zur Sicherheit noch einmal gekuerzt. Um die Brueche
// (a/b) und (c/d) zu vergleichen, werden sie zunaechst auf einen
// gemeinsamen Nenner gebracht (b*d), um dann die beiden Zaehler (a*d)
// und (c*b) zu vergleichen. Das Ergebnis dieses Vergleichs wird
// zurueckgegeben.

BOOL operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return FALSE;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator );
    nD *= BigInt( rVal2.nNumerator );

    return nN < nD;
}

/*************************************************************************
|*
|*    Fraction::operator >()
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    DV 20.09.90
|*    Letzte Aenderung  TH 19.08.92
|*
*************************************************************************/

// Beide Operanden werden zunaechst auf ihre Gueltigkeit ueberprueft und
// anschliessend zur Sicherheit noch einmal gekuerzt. Um die Brueche
// (a/b) und (c/d) zu vergleichen, werden sie zunaechst auf einen
// gemeinsamen Nenner gebracht (b*d), um dann die beiden Zaehler (a*d)
// und (c*b) zu vergleichen. Das Ergebnis dieses Vergleichs wird
// zurueckgegeben.

BOOL operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return FALSE;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator);
    nD *= BigInt( rVal2.nNumerator );

    return nN > nD;
}

/*************************************************************************
|*
|*    SvStream& operator>>( SvStream& rIStream, Fraction& rFract )
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    MM 08.01.96
|*    Letzte Aenderung  MM 08.01.96
|*
*************************************************************************/
SvStream& operator >> ( SvStream& rIStream, Fraction& rFract )
{
    rIStream >> rFract.nNumerator;
    rIStream >> rFract.nDenominator;
    return rIStream;
}

/*************************************************************************
|*
|*    SvStream& operator<<( SvStream& rIStream, Fraction& rFract )
|*
|*    Beschreibung      FRACT.SDW
|*    Ersterstellung    MM 08.01.96
|*    Letzte Aenderung  MM 08.01.96
|*
*************************************************************************/
SvStream& operator << ( SvStream& rOStream, const Fraction& rFract )
{
    rOStream << rFract.nNumerator;
    rOStream << rFract.nDenominator;
    return rOStream;
}
