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

/*
 maximal X Ersetzungen  (User: gefundenes darf X Zeichen anders sein)
 maximal Y Einfuegungen (User: gefundenes darf Y Zeichen kuerzer sein)
 maximal Z Loeschungen  (User: gefundenes darf Z Zeichen laenger sein)
 mathematische WLD oder SplitCount  (User: strikt oder relaxed)

 Joker ('?' und '*') fallen natuerlich nicht unter diese Bedingungen.
 Bei einem '?' wird eine Ersetzung nicht gezahlt, bei einem '*' darf
 der gefundene String an dieser Stelle beliebig viele Zeichen laenger sein.

 Strikt: entweder maximal X anders oder Y kuerzer oder Z laenger
 Relaxed: maximal X anders und/oder Y kuerzer und/oder Z laenger

 Wertebereich fuer X,Y,Z ist 0..33 um mit Limit sicher unter
 16-bit-signed-int-max zu bleiben, 31*32*33 gibt das Maximum
 KGV(31,32,33) == 32736
 */

#define LEVDISDEFAULT_XOTHER    2
#define LEVDISDEFAULT_YSHORTER  1
#define LEVDISDEFAULT_ZLONGER   3
#define LEVDISDEFAULT_RELAXED   TRUE

#define LEVDISDEFAULTLIMIT  6       // default nLimit, passt zu x=2, y=1, z=3,
                                    // p=3, q=6, r=2
#define LEVDISDEFAULT_P0    3       // default nRepP0, Gewichtung Ersetzen
#define LEVDISDEFAULT_Q0    6       // default nInsQ0, Gewichtung Einfuegen
#define LEVDISDEFAULT_R0    2       // default nDelR0, Gewichtung Loeschen
/*
 Berechnung von angegebenen Userwerten max Ersetzen, Kuerzer, Laenger mittels
 CalcLPQR(). Unschoen: wenn in WLD z.B. nLimit durch nDelR0 erreicht ist
 (String ist nZ laenger als Pattern), kann kein Zeichen mehr ersetzt werden.
 Dies kann durch Erhoehung von nX oder/und nZ vermieden werden, natuerlich
 mit den entsprechenden Seiteneffekten.. oder aber mit SplitCount (s.u.), was
 der Default bei CalcLPQR() ist.

 Achtung: Kuerzer = WLD.Insert, Laenger = WLD.Delete

 Gezaehlt wird von String nach Pattern (eine Loeschung bedeutet, dass aus
 String etwas geloescht werden muss, um auf Pattern zu kommen)

 Loeschungen zaehlen in diesem Beispiel am wenigsten, da meistens weniger
 bekannt ist, als gesucht wird. Ersetzungen erhalten mittlere Gewichtung
 wegen z.B. falscher Schreibweisen. Einfuegungen werden teuer.

 Oder z.B.: P0 = 1, Q0 = 4, R0 = 4, Limit = 3
 Erlaubt sind maximal 4 Ersetzungen, keine Einfuegung, keine Loeschung
 Entspricht den Userangaben X = 3, Y = 0, Z = 0

 bSplitCount: wenn TRUE, werden Rep/Ins/Del anders gezaehlt.  Der
 Rueckgabewert von WLD ist dann nicht mehr unbedingt die Levenshtein-Distanz,
 sondern kann negativ (-WLD) sein, wenn die WLD groesser als nLimit ist, aber
 die Einzelwerte jeweils innerhalb der Grenzen liegen.
 Bei den Default-Werten hiesse das z.B.: auch wenn der gefundene String 2
 Zeichen laenger ist (nLongerZ), koennen noch 3 Ersetzungen (nOtherX)
 erfolgen.  Zusatz-Gimmick:  Buchstabendreher zaehlen als eine Ersetzung.
 Mathematisch voellig unkorrekt, aber gut fuer den User ;-)

 Zur Erlaeuterung: bei der echten WLD schoepfen alle Aktionen aus einem
 gemeinsamen 100%-Pool, wenn einer alles hat, ist fuer die anderen nichts
 mehr da. Mit bSplitCount hat Replace sein eigenes Wildwasser..
 */

class WLevDisPatternMem     // "sichere" Speicheranforderung im cTor
{
    sal_Unicode     *cp;
    bool            *bp;
public:
    WLevDisPatternMem( sal_Int32 s )    { cp = new sal_Unicode[ s ];
                                          bp = new bool[ s ];
                                        }
    ~WLevDisPatternMem()                { if (cp) delete [] cp;
                                          if (bp) delete [] bp;
                                        }
    sal_Unicode* GetcPtr() const        { return cp; }
    bool* GetbPtr() const               { return bp; }
};

class WLevDisDistanceMem
{
    int*    p;
public:
    WLevDisDistanceMem( size_t s )  { p = 0; NewMem(s); }
    ~WLevDisDistanceMem()           { if (p) delete [] p; }
    int* GetPtr() const             { return p; }
    int* NewMem( size_t s )         {   if (p) delete [] p;
                                        return (p = new int[ s<3 ? 3 : s ]);
                                    }
};

class WLevDistance
{
    sal_Int32       nPatternLen;    // Laenge des Pattern
    WLevDisPatternMem   aPatMem;    // Verwaltung des Pattern Arrays
    sal_Unicode*    cpPattern;      // Pointer auf Pattern Array
    bool*           bpPatIsWild;    // Pointer auf bool Array, ob Pattern Wildcard ist
    sal_Int32       nArrayLen;      // Laenge des Distanz Arrays
    WLevDisDistanceMem  aDisMem;    // Verwaltung des Distanz Arrays
    int*            npDistance;     // Pointer auf Distanz Array
    int             nLimit;         // WLD Limit Ersetzungen/Einfuegungen/Loeschungen
    int             nRepP0;         // Ersetzen Gewichtung
    int             nInsQ0;         // Einfuegen Gewichtung
    int             nDelR0;         // Loeschen Gewichtung
    int             nStars;         // Anzahl '*' Joker im Pattern
    bool            bSplitCount;    // wenn TRUE, werden Rep/Ins/Del getrennt gezaehlt

    void InitData( const sal_Unicode* cPattern );       // fuer die CToren
    inline int Min3( int x, int y, int z );     // inline wegen Schleife
    int Mid3( int x, int y, int z );
    int Max3( int x, int y, int z );
    int GGT( int a, int b );    // Groesster Gemeinsamer Teiler
    int KGV( int a, int b );    // Kleinstes Gemeinsames Vielfaches

public:
    // CToren mit Userangaben, danach mit GetLimit() Limit holen
    // interner Aufruf von CalcLPQR()
    // die mathematisch unkorrekte Berechnung wird als Default genommen!
    WLevDistance( const sal_Unicode* cPattern, int nOtherX, int nShorterY,
                    int nLongerZ, bool bRelaxed = true );

    WLevDistance( const WLevDistance& rWLD );
    ~WLevDistance();

    // Berechnung der Levenshtein-Distanz von String zu Pattern
    int WLD( const sal_Unicode* cString, sal_Int32 nStringLen );

    // Berechnung der Gewichtung aus Userangaben, return nLimit
    int CalcLPQR( int nOtherX, int nShorterY, int nLongerZ,
                    bool bRelaxed = true );

    inline int GetLimit() const     { return( nLimit ); }   // Limit holen
    inline int GetReplaceP0() const { return( nRepP0 ); }   // Gewichtungen holen
    inline int GetInsertQ0() const  { return( nInsQ0 ); }
    inline int GetDeleteR0() const  { return( nDelR0 ); }
    inline bool GetSplit() const    { return( bSplitCount ); }
    inline int SetLimit( int nNewLimit );       // Limit setzen,
    inline int SetReplaceP0( int nNewP0 );      // Gewichtungen setzen,
    inline int SetInsertQ0( int nNewQ0 );       // returnen bisherigen Wert
    inline int SetDeleteR0( int nNewR0 );
    inline bool SetSplit( bool bNewSplit );
        // SetSplit( TRUE ) macht nur mit Werten nach CalcLPQR() Sinn!

    inline bool IsNormal( sal_Int32 nPos ) const { return( !bpPatIsWild[nPos] ); }

    // Balance, aus Geschwindigkeitsgruenden ist dieses keine Funktion
    // c == cpPattern[jj] == cString[ii]
    // erst wird bis Fundstelle gesucht, wenn dort die Balance gleich ist, wird
    // auch nach der Fundstelle verglichen
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

inline int WLevDistance::SetLimit( int nNewLimit )
{
    int nTmp = nLimit;
    nLimit = nNewLimit;
    return( nTmp );
}

inline int WLevDistance::SetReplaceP0( int nNewP0 )
{
    int nTmp = nRepP0;
    nRepP0 = nNewP0;
    return( nTmp );
}

inline int WLevDistance::SetInsertQ0( int nNewQ0 )
{
    int nTmp = nInsQ0;
    nInsQ0 = nNewQ0;
    return( nTmp );
}

inline int WLevDistance::SetDeleteR0( int nNewR0 )
{
    int nTmp = nDelR0;
    nDelR0 = nNewR0;
    return( nTmp );
}

inline bool WLevDistance::SetSplit( bool bNewSplit )
{
    bool bTmp = bSplitCount;
    bSplitCount = bNewSplit;
    return( bTmp );
}

#endif      // _LEVDIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
