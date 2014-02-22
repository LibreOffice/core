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

/*

    Weighted Levenshtein Distance
    including wildcards
    '*' for any number (0 or more) of arbitrary characters
    '?' for exactly one arbitrary character
    escapeable with  backslash, "\*" or "\?"

    Return:
        WLD if WLD <= nLimit, else nLimit+1

    or, if bSplitCount:
        WLD if WLD <= nLimit
        -WLD if Replace and Insert and Delete <= nLimit
        else nLimit+1

    Recursive definition of WLD:

    WLD( X(i), Y(j) ) = min( WLD( X(i-1), Y(j-1) ) + p(i,j) ,
                             WLD( X(i)  , Y(j-1) ) + q      ,
                             WLD( X(i-1), Y(j)   ) + r      )

    X(i)   := the first i characters of the word X
    Y(j)   := the first j characters of the word Y
    p(i,j) := 0 if i-th character of X == j-th character of Y,
              p else

    Boundary conditions:
    WLD( X(0), Y(j) ) := j*q  (Y created by j inserts)
    WLD( X(i), Y(0) ) := i*r  (Y created by i deletes)
    WLD( X(0), Y(0) ) := 0

    Instead of recursions a dynamic algorithm is used.

    See also: German computer magazine
    c't 07/89 pages 192-208 and c't 03/94 pages 230-239
*/

#include <string.h>

#if defined( _MSC_VER )
#pragma warning(once: 4068)
#endif

#include "levdis.hxx"

#ifdef SOLARIS
#undef min
#endif

#define LEVDISBIG   (nLimit + 1)    // Return value if distance > nLimit
#define LEVDISDOUBLEBUF 2048        // dadrueber wird nicht mehr gedoppelt

// Balance, aus Geschwindigkeitsgruenden ist dieses keine Funktion
// c == cpPattern[jj] == cString[ii]
// erst wird bis Fundstelle gesucht, wenn dort die Balance gleich ist, wird
// auch nach der Fundstelle verglichen
#define LEVDISBALANCE(jj,ii)                        \
{                                                   \
    if ( jj != ii )                                 \
    {                                               \
        sal_Int32 k;                                \
        if ( jj > 0 )                               \
            for ( k=0; k < jj; k++ )                \
                if ( cpPattern[k] == c )            \
                    nBalance++;                     \
        if ( ii > 0 )                               \
            for ( k=0; k < ii; k++ )                \
                if ( cString[k] == c )              \
                    nBalance--;                     \
        if ( !nBalance )                            \
        {                                           \
            for ( k=jj+1; k < nPatternLen; k++ )    \
                if ( cpPattern[k] == c )            \
                    nBalance++;                     \
            for ( k=ii+1; k < nStringLen; k++ )     \
                if ( cString[k] == c )              \
                    nBalance--;                     \
        }                                           \
    }                                               \
}

static sal_Int32 Impl_WLD_StringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        pTempStr++;
    return (sal_Int32)(pTempStr-pStr);
}

// Distance from string to pattern
int WLevDistance::WLD( const sal_Unicode* cString, sal_Int32 nStringLen )
{
    int nSPMin = 0;     // penalty point Minimum
    int nRepS = 0;      // for SplitCount

    // Laengendifferenz von Pattern und String
    int nLenDiff = nPatternLen - nStars - nStringLen;
    // mehr Einfuegungen oder Loeschungen noetig als Limit? => raus hier
    if ( (nLenDiff * nInsQ0 > nLimit)
            || ((nStars == 0) && (nLenDiff * nDelR0 < -nLimit)) )
        return(LEVDISBIG);

    // wenn der zu vergleichende String groesser ist als das bisherige Array
    // muss dieses angepasst werden
    if ( nStringLen >= nArrayLen )
    {
        // gib ihm moeglichst mehr, damit nicht gleich naechstesmal
        // wieder realloziert werden muss
        if ( nStringLen < LEVDISDOUBLEBUF )
            nArrayLen = 2 * nStringLen;
        else
            nArrayLen = nStringLen + 1;
        npDistance = aDisMem.NewMem( nArrayLen );
    }

    // Anfangswerte der zweiten Spalte (erstes Pattern-Zeichen) berechnen
    // die erste Spalte (0-Len Pattern) ist immer 0 .. nStringLen * nInsQ0,
    // deren Minimum also 0
    if ( nPatternLen == 0 )
    {
        // Anzahl der Loeschungen, um auf Pattern zu kommen
        for ( sal_Int32 i=0; i <= nStringLen; i++ )
            npDistance[i] = i * nDelR0;
    }
    else if ( cpPattern[0] == '*' && bpPatIsWild[0] )
    {
        // statt einem '*' ist alles einsetzbar
        for ( sal_Int32 i=0; i <= nStringLen; i++ )
            npDistance[i] = 0;
    }
    else
    {
        sal_Unicode c;
        int nP;
        c = cpPattern[0];
        if ( c == '?' && bpPatIsWild[0] )
            nP = 0;     // ein '?' kann jedes Zeichen sein
        else
            // Minimum von Ersetzen und Loeschen+Einfuegen Gewichtung
            nP = Min3( nRepP0, nRepP0, nDelR0 + nInsQ0 );
        npDistance[0] = nInsQ0;     // mit einfachem Einfuegen geht's los
        npDistance[1] = nInsQ0;
        npDistance[2] = nInsQ0;
        int nReplacePos = -1;       // tristate flag
        int nDelCnt = 0;
        for ( sal_Int32 i=1; i <= nStringLen; i++, nDelCnt += nDelR0 )
        {
            if ( cString[i-1] == c )
                nP = 0;     // Replace ab dieser Stelle ist 0
            // Loeschungen um auf Pattern zu kommen + Replace
            npDistance[i] = nDelCnt + nP;
            if ( bSplitCount )
            {
                if ( nReplacePos < 0 && nP )
                {   // diese Stelle wird ersetzt
                    nRepS++;
                    nReplacePos = i;
                }
                else if ( nReplacePos > 0 && !nP )
                {
                    int nBalance = 0;   // gleiche Anzahl c
                    LEVDISBALANCE( 0, i-1 );
                    if ( !nBalance )
                    {   // einer wurde ersetzt, der ein Insert war
                        nRepS--;
                        nReplacePos = 0;
                    }
                }
            }
        }
        nSPMin = Min3( npDistance[0], npDistance[1], npDistance[2] );
    }

    // Distanzmatrix berechnen
    sal_Int32 j = 0;        // fuer alle Spalten des Pattern, solange nicht Limit
    while ( (j < nPatternLen-1)
            && nSPMin <= (bSplitCount ? 2 * nLimit : nLimit) )
    {
        sal_Unicode c;
        int nP, nQ, nR, nPij, d1, d2;

        j++;
        c = cpPattern[j];
        if ( bpPatIsWild[j] )   // '*' oder '?' nicht escaped
            nP = 0;     // kann ohne Strafpunkte ersetzt werden
        else
            nP = nRepP0;
        if ( c == '*' && bpPatIsWild[j] )
        {
            nQ = 0;     // Einfuegen und Loeschen ohne Strafpunkte
            nR = 0;
        }
        else
        {
            nQ = nInsQ0;    // normale Gewichtung
            nR = nDelR0;
        }
        d2 = npDistance[0];
        // Anzahl Einfuegungen um von Null-String auf Pattern zu kommen erhoehen
        npDistance[0] = npDistance[0] + nQ;
        nSPMin = npDistance[0];
        int nReplacePos = -1;       // tristate Flag
        // fuer jede Patternspalte den String durchgehen
        for ( sal_Int32 i=1; i <= nStringLen; i++ )
        {
            d1 = d2;                // WLD( X(i-1), Y(j-1) )
            d2 = npDistance[i];     // WLD( X(i)  , Y(j-1) )
            if ( cString[i-1] == c )
            {
                nPij = 0;           // p(i,j)
                if ( nReplacePos < 0 )
                {
                    int nBalance = 0;   // same quantity c
                    LEVDISBALANCE( j, i-1 );
                    if ( !nBalance )
                        nReplacePos = 0;    // keine Ersetzung mehr
                }
            }
            else
                nPij = nP;
            // WLD( X(i), Y(j) ) = min( WLD( X(i-1), Y(j-1) ) + p(i,j) ,
            //                          WLD( X(i)  , Y(j-1) ) + q      ,
            //                          WLD( X(i-1), Y(j)   ) + r      )
            npDistance[i] = Min3( d1 + nPij, d2 + nQ, npDistance[i-1] + nR );
            if ( npDistance[i] < nSPMin )
                nSPMin = npDistance[i];
            if ( bSplitCount )
            {
                if ( nReplacePos < 0 && nPij && npDistance[i] == d1 + nPij )
                {   // this poition will be replaced
                    nRepS++;
                    nReplacePos = i;
                }
                else if ( nReplacePos > 0 && !nPij )
                {   // Zeichen in String und Pattern gleich.
                    // wenn ab hier die gleiche Anzahl dieses Zeichens
                    // sowohl in Pattern als auch in String ist, und vor
                    // dieser Stelle das Zeichen gleich oft vorkommt, war das
                    // Replace keins. Buchstabendreher werden hier erfasst
                    // und der ReplaceS zurueckgenommen, wodurch das doppelte
                    // Limit zum Tragen kommt.
                    int nBalance = 0;   // same quantity c
                    LEVDISBALANCE( j, i-1 );
                    if ( !nBalance )
                    {   // einer wurde ersetzt, der ein Insert war
                        nRepS--;
                        nReplacePos = 0;
                    }
                }
            }
        }
    }
    if ( (nSPMin <= nLimit) && (npDistance[nStringLen] <= nLimit) )
        return(npDistance[nStringLen]);
    else
    {
        if ( bSplitCount )
        {
            if ( nRepS && nLenDiff > 0 )
                nRepS -= nLenDiff;      // Inserts were counted
            if ( (nSPMin <= 2 * nLimit)
                    && (npDistance[nStringLen] <= 2 * nLimit)
                    && (nRepS * nRepP0 <= nLimit) )
                return( -npDistance[nStringLen] );
            return(LEVDISBIG);
        }
        return(LEVDISBIG);
    }
}

// Calculating nLimit,   nReplP0,    nInsQ0,     nDelR0,     bSplitCount
// from user values           nOtherX,    nShorterY,  nLongerZ,   bRelaxed
int WLevDistance::CalcLPQR( int nX, int nY, int nZ, bool bRelaxed )
{
    if ( nX < 0 ) nX = 0;       // only positive values
    if ( nY < 0 ) nY = 0;
    if ( nZ < 0 ) nZ = 0;
    if (0 == Min3( nX, nY, nZ ))     // at least one 0
    {
        int nMid, nMax;
        nMax = Max3( nX, nY, nZ );      // either 0 for three 0s or Max
        if ( 0 == (nMid = Mid3( nX, nY, nZ )) )     // even two 0
            nLimit = nMax;  // either 0 or the only one >0
        else        // one is 0
            nLimit = KGV( nMid, nMax );
    }
    else        // all three of them are not 0
        nLimit = KGV( KGV( nX, nY ), nZ );
    nRepP0 = ( nX ? nLimit / nX : nLimit + 1 );
    nInsQ0 = ( nY ? nLimit / nY : nLimit + 1 );
    nDelR0 = ( nZ ? nLimit / nZ : nLimit + 1 );
    bSplitCount = bRelaxed;
    return( nLimit );
}

// Groesster Gemeinsamer Teiler nach Euklid (Kettendivision)
// Sonderfall: 0 und irgendwas geben 1
int WLevDistance::GGT( int a, int b )
{
    if ( !a || !b )
        return 1;
    if ( a < 0 ) a = -a;
    if ( b < 0 ) b = -b;
    do
    {
        if ( a > b )
            a -= int(a / b) * b;
        else
            b -= int(b / a) * a;
    } while ( a && b );
    return( a ? a : b);
}

// Kleinstes Gemeinsames Vielfaches: a * b / GGT(a,b)
int WLevDistance::KGV( int a, int b )
{
    if ( a > b )    // Ueberlauf unwahrscheinlicher machen
        return( (a / GGT(a,b)) * b );
    else
        return( (b / GGT(a,b)) * a );
}

// Minimum of three values
inline int WLevDistance::Min3( int x, int y, int z )
{
    if ( x < y )
        return( x < z ? x : z );
    else
        return( y < z ? y : z );
}

// The value in the middle
int WLevDistance::Mid3( int x, int y, int z )
{
    int min = Min3(x,y,z);
    if ( x == min )
        return( y < z ? y : z);
    else if ( y == min )
        return( x < z ? x : z);
    else        // z == min
        return( x < y ? x : y);
}

// Maximum of three values
int WLevDistance::Max3( int x, int y, int z )
{
    if ( x > y )
        return( x > z ? x : z );
    else
        return( y > z ? y : z );
}

// Daten aus CTor initialisieren
void WLevDistance::InitData( const sal_Unicode* cPattern )
{
    cpPattern = aPatMem.GetcPtr();
    bpPatIsWild = aPatMem.GetbPtr();
    npDistance = aDisMem.GetPtr();
    nStars = 0;
    const sal_Unicode* cp1 = cPattern;
    sal_Unicode* cp2 = cpPattern;
    bool* bp = bpPatIsWild;
    // copy pattern, count asterisks, escaped Jokers
    while ( *cp1 )
    {
        if ( *cp1 == '\\' )     // maybe escaped
        {
            if ( *(cp1+1) == '*' || *(cp1+1) == '?' )   // next Joker?
            {
                cp1++;          // skip '\\'
                nPatternLen--;
            }
            *bp++ = false;
        }
        else if ( *cp1 == '*' || *cp1 == '?' )      // Joker
        {
            if ( *cp1 == '*' )
                nStars++;       // Sternchenzaehler erhoehen
            *bp++ = true;
        }
        else
            *bp++ = false;
        *cp2++ = *cp1++;
    }
    *cp2 = '\0';
}

WLevDistance::WLevDistance( const sal_Unicode* cPattern,
                            int nOtherX, int nShorterY, int nLongerZ,
                            bool bRelaxed ) :
    nPatternLen( Impl_WLD_StringLen(cPattern) ),
    aPatMem( nPatternLen + 1 ),
    nArrayLen( nPatternLen + 1 ),
    aDisMem( nArrayLen )
{
    InitData( cPattern );
    CalcLPQR( nOtherX, nShorterY, nLongerZ, bRelaxed );
}

// CopyCTor
WLevDistance::WLevDistance( const WLevDistance& rWLD ) :
    nPatternLen( rWLD.nPatternLen ),
    aPatMem( nPatternLen + 1 ),
    nArrayLen( nPatternLen + 1 ),
    aDisMem( nArrayLen ),
    nLimit( rWLD.nLimit ),
    nRepP0( rWLD.nRepP0 ),
    nInsQ0( rWLD.nInsQ0 ),
    nDelR0( rWLD.nDelR0 ),
    nStars( rWLD.nStars ),
    bSplitCount( rWLD.bSplitCount )
{
    cpPattern = aPatMem.GetcPtr();
    bpPatIsWild = aPatMem.GetbPtr();
    npDistance = aDisMem.GetPtr();
    sal_Int32 i;
    for ( i=0; i<nPatternLen; i++ )
    {
        cpPattern[i] = rWLD.cpPattern[i];
        bpPatIsWild[i] = rWLD.bpPatIsWild[i];
    }
    cpPattern[i] = '\0';
}

// DTor
WLevDistance::~WLevDistance()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
