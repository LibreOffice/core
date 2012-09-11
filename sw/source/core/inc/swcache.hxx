/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vector>

#include <rtl/ustring.hxx>

class SwCacheObj;

typedef std::vector<SwCacheObj*> SwCacheObjArr;
class SwCache
{
    SwCacheObjArr m_aCacheObjects;
    std::vector<sal_uInt16> aFreePositions; //Freie Positionen fuer das Insert wenn
                                    //die Maximalgrenze nicht erreicht ist.
                                    //Immer wenn ein Objekt ausgetragen wird,
                                    //so wird seine Position hier eingetragen.

    SwCacheObj *pRealFirst;         //_immer_ der echte LRU-erste
    SwCacheObj *pFirst;             //der virtuelle erste.
    SwCacheObj *pLast;

          sal_uInt16 nCurMax;           //Mehr werden nicht aufgenommen.


    void DeleteObj( SwCacheObj *pObj );

#ifdef DBG_UTIL
    rtl::OString m_aName;
    long m_nAppend;           /// number of entries appended
    long m_nInsertFree;       /// number of entries inserted on freed position
    long m_nReplace;          /// number of LRU replacements
    long m_nGetSuccess;
    long m_nGetFail;
    long m_nToTop;            /// number of reordering (LRU)
    long m_nDelete;           /// number of explicit deletes
    long m_nGetSeek;          /// number of gets without index
    long m_nAverageSeekCnt;   /// number of seeks for all gets without index
    long m_nFlushCnt;         /// number of flush calls
    long m_nFlushedObjects;
    long m_nIncreaseMax;      /// number of cache size increases
    long m_nDecreaseMax;      /// number of cache size decreases

    void Check();
#endif

public:

    //nur sal_uInt8 hineinstecken!!!
#ifdef DBG_UTIL
    SwCache( const sal_uInt16 nInitSize, const rtl::OString &rNm );
#else
    SwCache( const sal_uInt16 nInitSize );
#endif
    // the destructor will free all objects still in the vector
    ~SwCache();

    void Flush( const sal_uInt8 nPercent = 100 );

    //bToTop == sal_False -> Keine LRU-Umsortierung!
    SwCacheObj *Get( const void *pOwner, const sal_Bool bToTop = sal_True );
    SwCacheObj *Get( const void *pOwner, const sal_uInt16 nIndex,
                     const sal_Bool bToTop = sal_True );
    void ToTop( SwCacheObj *pObj );

    sal_Bool Insert( SwCacheObj *pNew );
    void Delete( const void *pOwner );
//  void Delete( const void *pOwner, const sal_uInt16 nIndex );

    void SetLRUOfst( const sal_uInt16 nOfst );      //nOfst sagt wieviele unangetastet
                                                //bleiben sollen.
    void ResetLRUOfst() { pFirst = pRealFirst; }

    inline void IncreaseMax( const sal_uInt16 nAdd );
    inline void DecreaseMax( const sal_uInt16 nSub );
    sal_uInt16 GetCurMax() const { return nCurMax; }
    inline SwCacheObj *First() { return pRealFirst; }
    inline SwCacheObj *Last()  { return pLast; }
    inline SwCacheObj *Next( SwCacheObj *pCacheObj);
    inline SwCacheObj* operator[](sal_uInt16 nIndex) { return m_aCacheObjects[nIndex]; }
    inline sal_uInt16 size() { return m_aCacheObjects.size(); }
};

//Cache-Manipulation auf die sichere Art.
class SwSaveSetLRUOfst
{
    SwCache &rCache;
public:
    SwSaveSetLRUOfst( SwCache &rC, const sal_uInt16 nOfst )
        : rCache( rC )          { rCache.SetLRUOfst( nOfst );  }

    ~SwSaveSetLRUOfst()         { rCache.ResetLRUOfst(); }
};

//Das allgemeine CacheObjekt. Anwender des Cache muessen eine Klasse vom
//CacheObjekt ableiten und dort die Nutzdaten unterbringen.

class SwCacheObj
{
    friend class SwCache;   //Der darf alles

    SwCacheObj *pNext;      //Fuer die LRU-Verkettung.
    SwCacheObj *pPrev;

    sal_uInt16 nCachePos;       //Position im Cache-Array.

    sal_uInt8       nLock;

    inline SwCacheObj *GetNext() { return pNext; }
    inline SwCacheObj *GetPrev() { return pPrev; }
    inline void SetNext( SwCacheObj *pNew )  { pNext = pNew; }
    inline void SetPrev( SwCacheObj *pNew )  { pPrev = pNew; }

    inline void   SetCachePos( const sal_uInt16 nNew ) { nCachePos = nNew; }

protected:
    const void *pOwner;
    inline void SetOwner( const void *pNew ) { pOwner = pNew; }

public:

    SwCacheObj( const void *pOwner );
    virtual ~SwCacheObj();

    inline const void *GetOwner() const { return pOwner; }
    inline sal_Bool IsOwner( const void *pNew ) const;

    inline sal_uInt16 GetCachePos() const { return nCachePos; }
    inline void Invalidate()          { pOwner = 0; }

    inline sal_Bool IsLocked() const { return 0 != nLock; }

#ifdef DBG_UTIL
    void Lock();
    void Unlock();
#else
    inline void Lock() { ++nLock; }
    inline void Unlock() { --nLock; }
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
    const void *pOwner;     //Kann ggf. in NewObj benutzt werden.

    virtual SwCacheObj *NewObj() = 0;

    inline SwCacheObj *Get();

    inline SwCacheAccess( SwCache &rCache, const void *pOwner, sal_Bool bSeek = sal_True );
    inline SwCacheAccess( SwCache &rCache, const void *pOwner, const sal_uInt16 nIndex );

public:
    virtual ~SwCacheAccess();

    virtual sal_Bool IsAvailable() const;

    //Abkuerzung fuer diejenigen, die wissen, das die Ableitung das IsAvailable
    //nicht ueberladen haben.
    sal_Bool IsAvail() const { return pObj != 0; }
};

inline void SwCache::IncreaseMax( const sal_uInt16 nAdd )
{
    nCurMax = nCurMax + sal::static_int_cast< sal_uInt16 >(nAdd);
#ifdef DBG_UTIL
    ++m_nIncreaseMax;
#endif
}
inline void SwCache::DecreaseMax( const sal_uInt16 nSub )
{
    if ( nCurMax > nSub )
        nCurMax = nCurMax - sal::static_int_cast< sal_uInt16 >(nSub);
#ifdef DBG_UTIL
    ++m_nDecreaseMax;
#endif
}

inline sal_Bool SwCacheObj::IsOwner( const void *pNew ) const
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

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void *pOwn, sal_Bool bSeek ) :
    rCache( rC ),
    pObj( 0 ),
    pOwner( pOwn )
{
    if ( bSeek && pOwner && 0 != (pObj = rCache.Get( pOwner )) )
        pObj->Lock();
}

inline SwCacheAccess::SwCacheAccess( SwCache &rC, const void *pOwn,
                              const sal_uInt16 nIndex ) :
    rCache( rC ),
    pObj( 0 ),
    pOwner( pOwn )
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


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
