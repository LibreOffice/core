/*************************************************************************
 *
 *  $RCSfile: levdis.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:00:29 $
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
/*************************************************************************

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

*************************************************************************/


#include <string.h>     // strlen()

#if defined( _MSC_VER )
#pragma warning(once: 4068)
#endif

#include "levdis.hxx"


#ifdef erTEST
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#endif

#ifdef SOLARIS
#undef min
#endif

#pragma hdrstop

#define LEVDISBIG   (nLimit + 1)    // Returnwert wenn Distanz > nLimit
#define LEVDISDOUBLEBUF 2048        // dadrueber wird nicht mehr gedoppelt

// Balance, aus Geschwindigkeitsgruenden ist dieses keine Funktion
// c == cpPattern[jj] == cString[ii]
// erst wird bis Fundstelle gesucht, wenn dort die Balance gleich ist, wird
// auch nach der Fundstelle verglichen
#define LEVDISBALANCE(jj,ii)                        \
{                                                   \
    if ( jj != ii )                                 \
    {                                               \
        register sal_Int32 k;                       \
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

sal_Int32 Impl_WLD_StringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        pTempStr++;
    return (sal_Int32)(pTempStr-pStr);
}

#ifdef erTESTMAT
#define erTESTMATMAX 180
static int far npMatrix[erTESTMATMAX][erTESTMATMAX];        // nearly 64K
#endif

// Distanz von String zu Pattern
int WLevDistance::WLD( const sal_Unicode* cString, sal_Int32 nStringLen )
{
    int nSPMin = 0;     // StrafPunkteMinimum
    int nRepS = 0;      // fuer SplitCount

#ifdef erTESTMAT
{
    for ( sal_Int32 r=0; r<=nStringLen && r < erTESTMATMAX; r++ )
        for ( sal_Int32 c=0; c<=nPatternLen && c < erTESTMATMAX; c++ )
            npMatrix[r][c] = 99;    // Matrix initialisieren, nur visuell
}
#endif

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
#ifdef erTEST
        if ( !npDistance )
        {
            cerr << "DOOM! (Damned, Out Of Memory)" << endl;
            exit(1);
        }
#endif
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
        int nReplacePos = -1;       // tristate Flag
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
#ifdef erTESTMAT
                    npMatrix[i][1] = -npDistance[i];
#endif
                }
                else if ( nReplacePos > 0 && !nP )
                {
                    int nBalance = 0;   // gleiche Anzahl c
                    LEVDISBALANCE( 0, i-1 );
                    if ( !nBalance )
                    {   // einer wurde ersetzt, der ein Insert war
                        nRepS--;
#ifdef erTESTMAT
                        npMatrix[nReplacePos][1] = npDistance[nReplacePos];
#endif
                        nReplacePos = 0;
                    }
                }
            }
        }
        nSPMin = Min3( npDistance[0], npDistance[1], npDistance[2] );
    }
#ifdef erTESTMAT
{
    for ( sal_Int32 r=0; r<=nStringLen && r < erTESTMATMAX; r++ )
    {
        npMatrix[r][0] = r * nInsQ0;
        if ( npMatrix[r][1] >= 0)
            npMatrix[r][1] = npDistance[r];
    }
}
#endif

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
        for ( register sal_Int32 i=1; i <= nStringLen; i++ )
        {
            d1 = d2;                // WLD( X(i-1), Y(j-1) )
            d2 = npDistance[i];     // WLD( X(i)  , Y(j-1) )
            if ( cString[i-1] == c )
            {
                nPij = 0;           // p(i,j)
                if ( nReplacePos < 0 )
                {
                    int nBalance = 0;   // gleiche Anzahl c
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
                {   // diese Stelle wird ersetzt
                    nRepS++;
                    nReplacePos = i;
#ifdef erTESTMAT
                    npMatrix[i][j+1] = -npDistance[i];
#endif
                }
                else if ( nReplacePos > 0 && !nPij )
                {   // Zeichen in String und Pattern gleich.
                    // wenn ab hier die gleiche Anzahl dieses Zeichens
                    // sowohl in Pattern als auch in String ist, und vor
                    // dieser Stelle das Zeichen gleich oft vorkommt, war das
                    // Replace keins. Buchstabendreher werden hier erfasst
                    // und der ReplaceS zurueckgenommen, wodurch das doppelte
                    // Limit zum Tragen kommt.
                    int nBalance = 0;   // gleiche Anzahl c
                    LEVDISBALANCE( j, i-1 );
                    if ( !nBalance )
                    {   // einer wurde ersetzt, der ein Insert war
                        nRepS--;
#ifdef erTESTMAT
                        npMatrix[nReplacePos][j+1] = npDistance[nReplacePos];
#endif
                        nReplacePos = 0;
                    }
                }
            }
        }
#ifdef erTESTMAT
{
        for ( sal_Int32 r=0; r<=nStringLen && r < erTESTMATMAX; r++ )
            if ( npMatrix[r][j+1] >= 0)
                npMatrix[r][j+1] = npDistance[r];
}
#endif
    }
#ifdef erTESTSPLIT
    printf(" nRepS: %d ", nRepS );
#endif
    if ( (nSPMin <= nLimit) && (npDistance[nStringLen] <= nLimit) )
        return(npDistance[nStringLen]);
    else
    {
        if ( bSplitCount )
        {
            if ( nRepS && nLenDiff > 0 )
                nRepS -= nLenDiff;      // Inserts wurden mitgezaehlt
#ifdef erTESTSPLIT
            printf(" nRepSdiff: %d ", nRepS );
#endif
            if ( (nSPMin <= 2 * nLimit)
                    && (npDistance[nStringLen] <= 2 * nLimit)
                    && (nRepS * nRepP0 <= nLimit) )
                return( -npDistance[nStringLen] );
            return(LEVDISBIG);
        }
        return(LEVDISBIG);
    }
}


int WLevDistance::WLD( const ::rtl::OUString& rString )
{
    return( WLD( rString.getStr(), rString.getLength() ));
}



int WLevDistance::WLD( const sal_Unicode* cString )
{
    return( WLD( cString, Impl_WLD_StringLen(cString) ));
}



// Berechnung von nLimit,   nReplP0,    nInsQ0,     nDelR0,     bSplitCount
// aus Userwerten           nOtherX,    nShorterY,  nLongerZ,   bRelaxed
int WLevDistance::CalcLPQR( int nX, int nY, int nZ, bool bRelaxed )
{
    int nMin, nMid, nMax;
    if ( nX < 0 ) nX = 0;       // nur positive Werte
    if ( nY < 0 ) nY = 0;
    if ( nZ < 0 ) nZ = 0;
    if ( 0 == (nMin = Min3( nX, nY, nZ )) )     // mindestens einer 0
    {
        nMax = Max3( nX, nY, nZ );      // entweder 0 bei drei 0 oder Max
        if ( 0 == (nMid = Mid3( nX, nY, nZ )) )     // sogar zwei 0
            nLimit = nMax;  // entweder 0 oder einziger >0
        else        // einer 0
            nLimit = KGV( nMid, nMax );
    }
    else        // alle drei nicht 0
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


// Minimum von drei Werten
inline int WLevDistance::Min3( int x, int y, int z )
{
    if ( x < y )
        return( x < z ? x : z );
    else
        return( y < z ? y : z );
}



// mittlerer von drei Werten
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



// Maximum von drei Werten
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
    // Pattern kopieren, Sternchen zaehlen, escaped Jokers
    while ( *cp1 )
    {
        if ( *cp1 == '\\' )     // maybe escaped
        {
            if ( *(cp1+1) == '*' || *(cp1+1) == '?' )   // naechstes Joker?
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


// CTor

WLevDistance::WLevDistance( const sal_Unicode* cPattern ) :
    nPatternLen( Impl_WLD_StringLen(cPattern) ),
    aPatMem( nPatternLen + 1 ),
    nArrayLen( nPatternLen + 1 ),
    aDisMem( nArrayLen ),
    nLimit( LEVDISDEFAULTLIMIT ),
    nRepP0( LEVDISDEFAULT_P0 ),
    nInsQ0( LEVDISDEFAULT_Q0 ),
    nDelR0( LEVDISDEFAULT_R0 ),
    bSplitCount( false )
{
    InitData( cPattern );
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


WLevDistance::WLevDistance( const ::rtl::OUString& rPattern ) :
    nPatternLen( rPattern.getLength() ),
    aPatMem( nPatternLen + 1 ),
    nArrayLen( nPatternLen + 1 ),
    aDisMem( nArrayLen ),
    nLimit( LEVDISDEFAULTLIMIT ),
    nRepP0( LEVDISDEFAULT_P0 ),
    nInsQ0( LEVDISDEFAULT_Q0 ),
    nDelR0( LEVDISDEFAULT_R0 ),
    bSplitCount( false )
{
    InitData( rPattern.getStr() );
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
    bSplitCount( rWLD.bSplitCount ),
    nStars( rWLD.nStars )
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

/*************************************************************************
 * Test
 *************************************************************************/

#ifdef erTEST

#define LINESIZE 1000
typedef char MAXSTRING [LINESIZE+1];

#ifdef erTESTMAT

void WLevDistance::ShowMatrix( const char* cString )
{
    sal_Int32 r, c, l = strlen(cString);
    printf("   |   ");
    for ( c=0; c<nPatternLen; c++ )
        printf( " %c ", cpPattern[c] );
    printf("\n---+---");
    for ( c=0; c<nPatternLen; c++ )
        printf( "---" );
    for ( r=0; r<=l && r < erTESTMATMAX; r++ )
    {
        printf( "\n %c |", ( r==0 ? ' ' : cString[r-1] ) );
        for ( c=0; c<=nPatternLen && c < erTESTMATMAX; c++ )
            printf( "%2d ", npMatrix[r][c] );
    }
    printf("\n\n");
}

#endif  // erTESTMAT

// Delimiter fuer Token, \t Tab bleibt fuer immer an der ersten Stelle
MAXSTRING cDelim = "\t, ;(){}[]<>&=+-/%!|.\\'\"~";

void WLevDistance::ShowTest()
{
    printf("  \n");
    printf(" a *cpPattern . . . . : %s\n", cpPattern);
    printf(" b *bpPatIsWild . . . : ");
    for ( sal_Int32 i=0; i<nPatternLen; i++ )
        printf("%d", bpPatIsWild[i]);
    printf("\n");
    printf(" c nPatternLen  . . . : %d\n", nPatternLen);
    printf(" d nStars . . . . . . : %d\n", nStars);
    printf(" e nLimit . . . . . . : %d\n", nLimit);
    printf(" f nRepP0 (Ersetzen)  : %d\n", nRepP0);
    printf(" g nInsQ0 (Einfuegen) : %d\n", nInsQ0);
    printf(" h nDelR0 (Loeschen)  : %d\n", nDelR0);
    printf(" i bSplitCount  . . . : %d\n", bSplitCount);
    printf(" j cDelim . . . . . . : '%s'\n", cDelim);
    printf(" ~\n");
}

inline bool IsDelim( char c )
{
    char* cp = cDelim;
    while ( *cp )
        if ( *cp++ == c )
            return( true );
    return( false );
}

MAXSTRING cString, cLine, cIgString;

main( int argc, char **argv )
{
    int nLim, nP0, nQ0, nR0, nX, nY, nZ;
    int args = 0;
    bool IgnoreCase = false, Direct = false, bStrict = false;
    WLevDistance* pTest;
    if ( argc < 2 )
    {
        printf("%s  Syntax:\n"
            " ... [-i] cPattern [nOtherX, nShorterY, nLongerZ [bStrict [cDelim]]] [<stdin] [>stdout]\n"
            " ...  -d  cPattern [nLimit [nRepP0 nInsQ0 nDelR0 [cDelim]]] [<stdin] [>stdout]\n"
            , argv[0]);
        exit(1);
    }
    if ( *argv[1] == '-' )
    {
        args++;
        argc--;
        switch ( *(argv[1]+1) )
        {
            case 'i':
            {
                IgnoreCase = true;
                char* cp = argv[args+1];
                while ( *cp = tolower( *cp ) )
                    cp++;
        break;
            }
            case 'd':
                Direct = true;
        break;
        }
    }
    if ( Direct )
    {
        if ( argc > 2 )
            nLim = atoi(argv[args+2]);
        else
            nLim = LEVDISDEFAULTLIMIT;
        if ( argc > 3 )
        {
            nP0 = atoi(argv[args+3]);
            nQ0 = atoi(argv[args+4]);
            nR0 = atoi(argv[args+5]);
        }
        else
        {
            nP0 = LEVDISDEFAULT_P0;
            nQ0 = LEVDISDEFAULT_Q0;
            nR0 = LEVDISDEFAULT_R0;
        }
        if ( argc > 6 )
        {
            strncpy( cDelim+1, argv[args+6], LINESIZE );    // \t Tab always remains
            cDelim[LINESIZE-1] = 0;
        }
    }
    else
    {
        if ( argc > 2 )
        {
            nX = atoi(argv[args+2]);
            nY = atoi(argv[args+3]);
            nZ = atoi(argv[args+4]);
        }
        else
        {
            nX = LEVDISDEFAULT_XOTHER;
            nY = LEVDISDEFAULT_YSHORTER;
            nZ = LEVDISDEFAULT_ZLONGER;
        }
        if ( argc > 5 )
            bStrict = atoi(argv[args+5]);
        if ( argc > 6 )
        {
            strncpy( cDelim+1, argv[args+6], LINESIZE );    // \t Tab always remains
            cDelim[LINESIZE-1] = 0;
        }
    }
    if ( Direct )
    {
        pTest = new WLevDistance( argv[args+1] );
#ifdef erTESTDEFAULT
        pTest->ShowTest();
#endif
        pTest->SetLimit( nLim );
        pTest->SetReplaceP0( nP0 );
        pTest->SetInsertQ0( nQ0 );
        pTest->SetDeleteR0( nR0 );
    }
    else
    {
        pTest = new WLevDistance( argv[args+1], nX, nY, nZ, !bStrict );
#ifdef erTESTCCTOR
        WLevDistance aTmp( *pTest );
        aTmp.ShowTest();
#endif
        nLim = pTest->GetLimit();
    }
    pTest->ShowTest();
    do
    {
        char* cp1, *cp2;
        static ULONG nLine = 0;
        cp1 = cLine;
        cin.getline( cLine, LINESIZE ) ;
        nLine++;
        while ( *cp1 )
        {
            while ( *cp1 && IsDelim(*cp1) )
                cp1++;
            cp2 = cString;
            while ( *cp1 && !IsDelim(*cp1) )
                *cp2++ = *cp1++;
            *cp2 = '\0';
            while ( *cp1 && IsDelim(*cp1) )
                cp1++;
            if ( *cString )
            {
                int ret;
                if ( IgnoreCase )
                {
                    char* cp1 = cString;
                    char* cp2 = cIgString;
                    while ( *cp1 )
                        *cp2++ = tolower( *cp1++ );
                    *cp2 = '\0';
                    ret = pTest->WLD( cIgString );
                }
                else
                    ret = pTest->WLD( cString );
#ifdef erTESTMAT
                printf("\n# %3d : %s\n", ret, cString);
                pTest->ShowMatrix( cString );
#else
                if ( ret <= nLim )
                    printf("# %3d : %s\t(line %ld)\t%s\n", ret, cString, nLine, cLine);
#endif
            }
        }
    } while ( !cin.eof() );
    return 0;
}

#endif  // erTEST

