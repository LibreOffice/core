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
#ifndef _SWCACHE_HXX
#define _SWCACHE_HXX




/*
 * Es werden Pointer auf Objekte verwaltet. Diese werden in einem einfachen
 * PtrArray verwaltet.
 * Angelegt (new) werden die Objekte von Cache-Zugriffsklassen, zuerstoert
 * werden die Objekte vom Cache.
 *
 * Auf die Objekte kann wahlweise per Index in das Array oder per Suche
 * zugegriffen werden. Soll per Index zugegriffen werden, so obliegt die
 * Verwaltung des Index dem Anwender des Cache.
 *
 * Fuer die verwalteten Cache-Objekte gibt es eine Basisklasse, von dieser
 * sind spezifische Klassen abzuleiten.
 * In der Basisklasse werden die Cache-Objekte eines Cache doppelt verkettet,
 * das ermoeglich die Implementierung eines LRU-Algorithmus.
 *
 * Der LRU kann in der Cache-Basisklasse manipuliert werden, indem ein
 * virtueller First-Pointer gesetzt wird. Dieser kann auf den echten ersten
 * plus einem Ofst gesetzt werden. Dadurch kann man den Anfangsbereich des
 * Cache sichern und so dafuer sorgen, dass man waehrend bestimmter
 * Operationen nicht den Cache versaut. Beispiel: Der Idle-Handler sollte nicht
 * den Cache fuer den sichtbaren Bereich vernichten.
 *
 * Der Cache kann in der Groesse erweitert und wieder verkleinert werden.
 * Beispiel: Fuer jede neue Shell wird der Cache fuer FormatInfo vergrossert
 * und beim Destruieren der Shell wieder verkleinert.
 *
 */

#ifdef DBG_UTIL
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {
class SwCacheObj;

SV_DECL_PTRARR_DEL(SwCacheObjArr,SwCacheObj*,1,1)

class SwCache : public SwCacheObjArr
{
    SvUShorts aFreePositions;		//Freie Positionen fuer das Insert wenn
                                    //die Maximalgrenze nicht erreicht ist.
                                    //Immer wenn ein Objekt ausgetragen wird,
                                    //so wird seine Position hier eingetragen.

    SwCacheObj *pRealFirst;			//_immer_ der echte LRU-erste
    SwCacheObj *pFirst;				//der virtuelle erste.
    SwCacheObj *pLast;

    const USHORT nMax;				//Mehr sollen nicht aufgenommen werden,
                                    //der Cache kann aber dynamisch um jeweils
                                    //nMax vergroessert werden.
          USHORT nCurMax;			//Mehr werden nicht aufgenommen.


    void DeleteObj( SwCacheObj *pObj );

#ifdef DBG_UTIL
    ByteString aName;
    long nAppend;			//Anzahl der Eintragungen durch Erweiterung.
    long nInsertFree;		//Anzahl der Eintragungen auf freie Plaetze.
    long nReplace;			//Anzahl der Ersetzungen durch ein neues Objekt
    long nGetSuccess;		//Anzahl der Erfolgreichen Get's
    long nGetFail;			//Anzahl der nicht Erfolgreichen Get's
    long nToTop;			//Anzahl der Umsortierungen (LRU)
    long nDelete;			//Anzahl der Loeschungen (von Aussen)
    long nGetSeek;			//Anzahl der Get's ohne Index
    long nAverageSeekCnt;	//Anzahl der Seek's fuer alle Get's ohne Index
    long nFlushCnt;			//Anzahl von Flush-Aufrufen.
    long nFlushedObjects;	//Anzahl der wg. Flush vernichteten Objekte
    long nIncreaseMax;		//Anzahl Cache-Erweiterungen
    long nDecreaseMax;		//Anzahl Cache-Verkleinerungen

    void Check();			//Wird bei swcache.cxx mit DEBUG aktiv!
#endif

public:

    //nur BYTE hineinstecken!!!
#ifdef DBG_UTIL
    SwCache( const USHORT nInitSize, const USHORT nGrowSize,
            const ByteString &rNm );
    ~SwCache();
#else
    SwCache( const USHORT nInitSize, const USHORT nGrowSize );
#endif

    void Flush( const BYTE nPercent = 100 );

    //bToTop == FALSE -> Keine LRU-Umsortierung!
    SwCacheObj *Get( const void *pOwner, const BOOL bToTop = TRUE );
    SwCacheObj *Get( const void *pOwner, const USHORT nIndex,
                     const BOOL bToTop = TRUE );
    void ToTop( SwCacheObj *pObj );

    BOOL Insert( SwCacheObj *pNew );
    void Delete( const void *pOwner );
//	void Delete( const void *pOwner, const USHORT nIndex );

    void SetLRUOfst( const USHORT nOfst );		//nOfst sagt wieviele unangetastet
                                                //bleiben sollen.
    void ResetLRUOfst() { pFirst = pRealFirst; }

    inline void IncreaseMax( const USHORT nAdd );
    inline void DecreaseMax( const USHORT nSub );
    USHORT GetCurMax() const { return nCurMax; }
    inline SwCacheObj *First() { return pRealFirst; }
    inline SwCacheObj *Last()  { return pLast; }
    inline SwCacheObj *Next( SwCacheObj *pCacheObj);
};

//Cache-Manipulation auf die sichere Art.
class SwSaveSetLRUOfst
{
    SwCache &rCache;
public:
    SwSaveSetLRUOfst( SwCache &rC, const USHORT nOfst )
        : rCache( rC ) 			{ rCache.SetLRUOfst( nOfst );  }

    ~SwSaveSetLRUOfst()			{ rCache.ResetLRUOfst(); }
};

//Das allgemeine CacheObjekt. Anwender des Cache muessen eine Klasse vom
//CacheObjekt ableiten und dort die Nutzdaten unterbringen.

class SwCacheObj
{
    friend class SwCache;	//Der darf alles

    SwCacheObj *pNext;		//Fuer die LRU-Verkettung.
    SwCacheObj *pPrev;

    USHORT nCachePos;		//Position im Cache-Array.

    BYTE		nLock;

    inline SwCacheObj *GetNext() { return pNext; }
    inline SwCacheObj *GetPrev() { return pPrev; }
    inline void SetNext( SwCacheObj *pNew )	 { pNext = pNew; }
    inline void SetPrev( SwCacheObj *pNew )  { pPrev = pNew; }

    inline void   SetCachePos( const USHORT nNew ) { nCachePos = nNew; }

protected:
    const void *pOwner;
    inline void SetOwner( const void *pNew ) { pOwner = pNew; }

public:

    SwCacheObj( const void *pOwner );
    virtual ~SwCacheObj();

    inline const void *GetOwner() const { return pOwner; }
    inline BOOL IsOwner( const void *pNew ) const;

    inline USHORT GetCachePos() const { return nCachePos; }
    inline void Invalidate()		  { pOwner = 0; }

    inline BOOL IsLocked() const { return 0 != nLock; }

#ifndef DBG_UTIL
    inline void Lock() { ++nLock; }
    inline void Unlock() { --nLock; }
#else
    void Lock();
    void Unlock();
#endif

    SwCacheObj *Next() { return pNext; }
    SwCacheObj *Prev() { return pPrev; }

};

//Zugriffsklasse fuer den Cache. Im CTor wird das CacheObjekt erzeugt.
//Wenn der Cache keines herausrueckt wird der Member zunaechst auf 0 gesetzt.
//Beim Get wird dann eines erzeugt und, falls moeglich, in den Cache
//eingetragen.
//Anwender der des Cache muessen eine Klasse vom Access ableiten um
//fuer Typsicherheit zu sorgen, die Basisklasse sollte fuer das Get aber immer
//gerufen werden, ein Abgeleitetes Get sollte nur der Typsicherheit dienen.
//Cache-Objekte werden stets gelockt solange die Instanz lebt.

class SwCacheAccess
{
    SwCache &rCache;

    void _Get();

protected:
    SwCacheObj *pObj;
    const void *pOwner;		//Kann ggf. in NewObj benutzt werden.

    virtual SwCacheObj *NewObj() = 0;

    inline SwCacheObj *Get();

    inline SwCacheAccess( SwCache &rCache, const void *pOwner, BOOL bSeek = TRUE );
    inline SwCacheAccess( SwCache &rCache, const void *pOwner, const USHORT nIndex );

public:
    ~SwCacheAccess() { if ( pObj ) pObj->Unlock(); }


    //Abkuerzung fuer diejenigen, die wissen, das die Ableitung das IsAvailable
    //nicht ueberladen haben.
    BOOL IsAvail() const { return pObj != 0; }
};

inline void SwCache::IncreaseMax( const USHORT nAdd )
{
    nCurMax += nAdd;
#ifdef DBG_UTIL
    ++nIncreaseMax;
#endif
}
inline void SwCache::DecreaseMax( const USHORT nSub )
{
    if ( nCurMax > nSub )
        nCurMax -= nSub;
#ifdef DBG_UTIL
    ++nDecreaseMax;
#endif
}

inline BOOL SwCacheObj::IsOwner( const void *pNew ) const
{
    return pOwner && pOwner == pNew;
}

inline SwCacheObj *SwCache::Next( SwCacheObj *pCacheObj)
{
    if ( pCacheObj )
        return pCacheObj->GetNext();
    else
        return NULL;
}

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void *pOwn, BOOL bSeek ) :
    rCache( rC ),
    pOwner( pOwn ),
    pObj( 0 )
{
    if ( bSeek && pOwner && 0 != (pObj = rCache.Get( pOwner )) )
        pObj->Lock();
}

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void *pOwn,
                              const USHORT nIndex ) :
    rCache( rC ),
    pOwner( pOwn ),
    pObj( 0 )
{
    if ( pOwner && 0 != (pObj = rCache.Get( pOwner, nIndex )) )
        pObj->Lock();
}

inline SwCacheObj *SwCacheAccess::Get()
{
    if ( !pObj )
        _Get();
    return pObj;
}


} //namespace binfilter
#endif
