/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

#define LEVDISBIG   (nLimit + 1)    
#define LEVDISDOUBLEBUF 2048        





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


int WLevDistance::WLD( const sal_Unicode* cString, sal_Int32 nStringLen )
{
    int nSPMin = 0;     
    int nRepS = 0;      

    
    int nLenDiff = nPatternLen - nStars - nStringLen;
    
    if ( (nLenDiff * nInsQ0 > nLimit)
            || ((nStars == 0) && (nLenDiff * nDelR0 < -nLimit)) )
        return(LEVDISBIG);

    
    
    if ( nStringLen >= nArrayLen )
    {
        
        
        if ( nStringLen < LEVDISDOUBLEBUF )
            nArrayLen = 2 * nStringLen;
        else
            nArrayLen = nStringLen + 1;
        npDistance = aDisMem.NewMem( nArrayLen );
    }

    
    
    
    if ( nPatternLen == 0 )
    {
        
        for ( sal_Int32 i=0; i <= nStringLen; i++ )
            npDistance[i] = i * nDelR0;
    }
    else if ( cpPattern[0] == '*' && bpPatIsWild[0] )
    {
        
        for ( sal_Int32 i=0; i <= nStringLen; i++ )
            npDistance[i] = 0;
    }
    else
    {
        sal_Unicode c;
        int nP;
        c = cpPattern[0];
        if ( c == '?' && bpPatIsWild[0] )
            nP = 0;     
        else
            
            nP = Min3( nRepP0, nRepP0, nDelR0 + nInsQ0 );
        npDistance[0] = nInsQ0;     
        npDistance[1] = nInsQ0;
        npDistance[2] = nInsQ0;
        int nReplacePos = -1;       
        int nDelCnt = 0;
        for ( sal_Int32 i=1; i <= nStringLen; i++, nDelCnt += nDelR0 )
        {
            if ( cString[i-1] == c )
                nP = 0;     
            
            npDistance[i] = nDelCnt + nP;
            if ( bSplitCount )
            {
                if ( nReplacePos < 0 && nP )
                {   
                    nRepS++;
                    nReplacePos = i;
                }
                else if ( nReplacePos > 0 && !nP )
                {
                    int nBalance = 0;   
                    LEVDISBALANCE( 0, i-1 );
                    if ( !nBalance )
                    {   
                        nRepS--;
                        nReplacePos = 0;
                    }
                }
            }
        }
        nSPMin = Min3( npDistance[0], npDistance[1], npDistance[2] );
    }

    
    sal_Int32 j = 0;        
    while ( (j < nPatternLen-1)
            && nSPMin <= (bSplitCount ? 2 * nLimit : nLimit) )
    {
        sal_Unicode c;
        int nP, nQ, nR, nPij, d1, d2;

        j++;
        c = cpPattern[j];
        if ( bpPatIsWild[j] )   
            nP = 0;     
        else
            nP = nRepP0;
        if ( c == '*' && bpPatIsWild[j] )
        {
            nQ = 0;     
            nR = 0;
        }
        else
        {
            nQ = nInsQ0;    
            nR = nDelR0;
        }
        d2 = npDistance[0];
        
        npDistance[0] = npDistance[0] + nQ;
        nSPMin = npDistance[0];
        int nReplacePos = -1;       
        
        for ( sal_Int32 i=1; i <= nStringLen; i++ )
        {
            d1 = d2;                
            d2 = npDistance[i];     
            if ( cString[i-1] == c )
            {
                nPij = 0;           
                if ( nReplacePos < 0 )
                {
                    int nBalance = 0;   
                    LEVDISBALANCE( j, i-1 );
                    if ( !nBalance )
                        nReplacePos = 0;    
                }
            }
            else
                nPij = nP;
            
            
            
            npDistance[i] = Min3( d1 + nPij, d2 + nQ, npDistance[i-1] + nR );
            if ( npDistance[i] < nSPMin )
                nSPMin = npDistance[i];
            if ( bSplitCount )
            {
                if ( nReplacePos < 0 && nPij && npDistance[i] == d1 + nPij )
                {   
                    nRepS++;
                    nReplacePos = i;
                }
                else if ( nReplacePos > 0 && !nPij )
                {   
                    
                    
                    
                    
                    
                    
                    int nBalance = 0;   
                    LEVDISBALANCE( j, i-1 );
                    if ( !nBalance )
                    {   
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
                nRepS -= nLenDiff;      
            if ( (nSPMin <= 2 * nLimit)
                    && (npDistance[nStringLen] <= 2 * nLimit)
                    && (nRepS * nRepP0 <= nLimit) )
                return( -npDistance[nStringLen] );
            return(LEVDISBIG);
        }
        return(LEVDISBIG);
    }
}



int WLevDistance::CalcLPQR( int nX, int nY, int nZ, bool bRelaxed )
{
    if ( nX < 0 ) nX = 0;       
    if ( nY < 0 ) nY = 0;
    if ( nZ < 0 ) nZ = 0;
    if (0 == Min3( nX, nY, nZ ))     
    {
        int nMid, nMax;
        nMax = Max3( nX, nY, nZ );      
        if ( 0 == (nMid = Mid3( nX, nY, nZ )) )     
            nLimit = nMax;  
        else        
            nLimit = KGV( nMid, nMax );
    }
    else        
        nLimit = KGV( KGV( nX, nY ), nZ );
    nRepP0 = ( nX ? nLimit / nX : nLimit + 1 );
    nInsQ0 = ( nY ? nLimit / nY : nLimit + 1 );
    nDelR0 = ( nZ ? nLimit / nZ : nLimit + 1 );
    bSplitCount = bRelaxed;
    return( nLimit );
}



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


int WLevDistance::KGV( int a, int b )
{
    if ( a > b )    
        return( (a / GGT(a,b)) * b );
    else
        return( (b / GGT(a,b)) * a );
}


inline int WLevDistance::Min3( int x, int y, int z )
{
    if ( x < y )
        return( x < z ? x : z );
    else
        return( y < z ? y : z );
}


int WLevDistance::Mid3( int x, int y, int z )
{
    int min = Min3(x,y,z);
    if ( x == min )
        return( y < z ? y : z);
    else if ( y == min )
        return( x < z ? x : z);
    else        
        return( x < y ? x : y);
}


int WLevDistance::Max3( int x, int y, int z )
{
    if ( x > y )
        return( x > z ? x : z );
    else
        return( y > z ? y : z );
}


void WLevDistance::InitData( const sal_Unicode* cPattern )
{
    cpPattern = aPatMem.GetcPtr();
    bpPatIsWild = aPatMem.GetbPtr();
    npDistance = aDisMem.GetPtr();
    nStars = 0;
    const sal_Unicode* cp1 = cPattern;
    sal_Unicode* cp2 = cpPattern;
    bool* bp = bpPatIsWild;
    
    while ( *cp1 )
    {
        if ( *cp1 == '\\' )     
        {
            if ( *(cp1+1) == '*' || *(cp1+1) == '?' )   
            {
                cp1++;          
                nPatternLen--;
            }
            *bp++ = false;
        }
        else if ( *cp1 == '*' || *cp1 == '?' )      
        {
            if ( *cp1 == '*' )
                nStars++;       
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


WLevDistance::~WLevDistance()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
