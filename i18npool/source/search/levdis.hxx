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

#ifndef INCLUDED_I18NPOOL_SOURCE_SEARCH_LEVDIS_HXX
#define INCLUDED_I18NPOOL_SOURCE_SEARCH_LEVDIS_HXX

#include <rtl/ustring.hxx>

// Sensible default values for a user interface could be:
//  LEVDISDEFAULT_XOTHER    2
//      Maximum X replacements to match query, found data may be different by X
//      characters from query.
//  LEVDISDEFAULT_YSHORTER  1
//      Maximum Y insertions to match query, found data may be Y characters
//      shorter than query.
//  LEVDISDEFAULT_ZLONGER   3
//      Maximum Z deletions to match query, found data may be Z characters
//      longer than query.
//  LEVDISDEFAULT_RELAXED   TRUE
//      Use relaxed SplitCount instead of mathematical WLD.
//
// Joker/wildcards ('?' and '*') of course do not count as
// replacement/insertion/deletion. At a '?' a replacement is not counted, for a
// '*' the found data may be any number of characters longer than the query.
//
// Strict mathematical WLD: EITHER maximum X replacements OR Y characters
// shorter OR Z characters longer.
// Relaxed SplitCount: maximum X replacements AND/OR Y character shorter AND/OR
// Z characters longer. Any combination of actions is valid.
//
// The value range for X,Y,Z is 0..33 to keep the limit within a 16 bit signed
// integer, 31*32*33 is the maximum limit, LCM(31,32,33) == 32736.
//
// The corresponding internal default weigh values for these user interface
// values would be:
//  LEVDISDEFAULTLIMIT  6
//      Default nLimit, matches x=2, y=1, z=3, p=3, q=6, r=2
//  LEVDISDEFAULT_P0    3
//      Default nRepP0, weight of replacements.
//  LEVDISDEFAULT_Q0    6
//      Default nInsQ0, weight of insertions.
//  LEVDISDEFAULT_R0    2
//      Default nDelR0, weight of deletions.

// The transformation of user input values to weighs is done using CalcLPQR().
// One caveat, if the WLD reaches nLimit due to nDelR0 (i.e. data string is nZ
// characters longer than pattern) then no character can be replaced any more.
// This can be circumvented by increasing nX or/and nZ, but of course with the
// side effect of being less strict then.. or the other solution is to use
// relaxed SplitCount (see below), which is the default when using CalcLPQR().
//
// Attention: shorter = WLD.Insert, longer = WLD.Delete
//
// View and counting is always from data string to pattern, a deletion means
// that something is deleted from data to match pattern.
//
// Deletions weigh less in this example because usually less is known than is
// searched for. Replacements get middle weight, for example because of
// misspellings. Insertions are expensive.
//
// Another example: P0 = 1, Q0 = 4, R0 = 4, Limit = 3
// Allowed are maximum 4 replacements, no insertion, no deletion.
// Matches the user interface values X = 3, Y = 0, Z = 0
//
// bSplitCount: if TRUE, Rep/Ins/Del are counted differently. The return value
// of WLD() then isn't necessarily the Levenshtein-Distance, but can be
// negative (-WLD) if the WLD is greater than nLimit but single values are
// within the limits.
// For the above default values that could mean: even if the found string is
// already 2 characters longer (nLongerZ), 3 replacements (nOtherX) can be made
// to reach pattern. Additionally, character swaps count as one replacement.
// Mathematically completely incorrect, but meets user expectations ;-)
//
// Explanation: in the real WLD all actions are withdrawn from a common 100%
// pool, if one gets all there's nothing left for others. With bSplitCount
// replacements have their own pool.


/** "Safe" memory allocation in ctor */
class WLevDisPatternMem
{
    sal_Unicode     *cp;
    bool            *bp;
public:
    explicit WLevDisPatternMem( sal_Int32 s )
        : cp(new sal_Unicode[s])
        , bp(new bool[s])
    {
    }

    ~WLevDisPatternMem()
    {
        delete [] cp;
        delete [] bp;
    }
    sal_Unicode* GetcPtr() const        { return cp; }
    bool* GetbPtr() const               { return bp; }
};

class WLevDisDistanceMem
{
    int*    p;
public:
    explicit WLevDisDistanceMem( size_t s )
        : p(nullptr)
    {
        NewMem(s);
    }
    ~WLevDisDistanceMem()           { delete [] p; }
    int* GetPtr() const             { return p; }
    int* NewMem( size_t s )
    {
        delete [] p;
        return (p = new int[ s<3 ? 3 : s ]);
    }
};

/** Weighted Levenshtein Distance (WLD)

    For a more detailed explanation see documentation in
    i18npool/source/search/levdis.hxx
 */
class WLevDistance
{
    sal_Int32       nPatternLen;    ///< length of pattern
    WLevDisPatternMem   aPatMem;    ///< manage allocation of pattern array
    sal_Unicode*    cpPattern;      ///< pointer to pattern array
    bool*           bpPatIsWild;    ///< pointer to bool array whether pattern is wildcard
    sal_Int32       nArrayLen;      ///< length of distance array
    WLevDisDistanceMem  aDisMem;    ///< manage allocation of distance array
    int*            npDistance;     ///< pointer to distance array
    int             nLimit;         ///< WLD limit replacements/insertions/deletions
    int             nRepP0;         ///< replacement weigh
    int             nInsQ0;         ///< insertion weigh
    int             nDelR0;         ///< deletion weigh
    int             nStars;         ///< count of '*' wildcards in pattern
    bool            bSplitCount;    ///< if TRUE, Rep/Ins/Del are counted separately

    void InitData( const sal_Unicode* cPattern );
    static inline int Min3( int x, int y, int z ); ///< minimum value of 3 values
    static int Mid3( int x, int y, int z );        ///< middle value of 3 values
    static int Max3( int x, int y, int z );        ///< maximum value of 3 values
    static int GCD( int a, int b );                ///< Greatest Common Divisor
    static int LCM( int a, int b );                ///< Least Common Multiple

public:

    /** CTor with user input. Internally calls CalcLPQR().

        After this, obtain the resulting limit using GetLimit().

        @param  bRelaxed    the mathematically incorrect method is default (TRUE)
     */
    WLevDistance( const sal_Unicode* cPattern, int nOtherX, int nShorterY,
                    int nLongerZ, bool bRelaxed = true );

    WLevDistance( const WLevDistance& rWLD );
    ~WLevDistance();

    /** Calculate the Weighted Levenshtein Distance from string to pattern. */
    int WLD( const sal_Unicode* cString, sal_Int32 nStringLen );

    /** Calculate the internal weighs corresponding to the user input values.
        @returns nLimit for later comparison with WLD()
     */
    int CalcLPQR( int nOtherX, int nShorterY, int nLongerZ,
                    bool bRelaxed = true );

    inline int GetLimit() const     { return nLimit; }

    // Calculate current balance, keep this inline for performance reasons!
    // c == cpPattern[jj] == cString[ii]
    // First seek up to found place, if the balance is still equal there then
    // also compare after the found place.
    int levdisbalance(sal_Int32 jj, sal_Int32 ii, sal_Unicode c, const sal_Unicode* cString, sal_Int32 nStringLen)
    {
        int nBalance = 0;

        if ( jj != ii )
        {
            sal_Int32 k;
            if ( jj > 0 )
                for ( k=0; k < jj; k++ )
                    if ( cpPattern[k] == c )
                        nBalance++;
            if ( ii > 0 )
                for ( k=0; k < ii; k++ )
                    if ( cString[k] == c )
                        nBalance--;
            if ( !nBalance )
            {
                for ( k=jj+1; k < nPatternLen; k++ )
                    if ( cpPattern[k] == c )
                        nBalance++;
                for ( k=ii+1; k < nStringLen; k++ )
                    if ( cString[k] == c )
                        nBalance--;
            }
        }

        return nBalance;
    }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
