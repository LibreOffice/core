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


/*************************************************************
#* Service-Klassen
 *************************************************************/

/*
#* Aendert sich ein Attribut in einem Format, so muss diese
#* Aenderung an alle abhaengigen Formate und ueber sie an
#* alle betroffenen Nodes propagiert werden. Dabei muss
#* festgestellt werden, ob die Aenderung einen Effekt haben
#* kann, oder ob das geaenderte Attribut von dem abhaengigen
#* Format ueberdefiniert wird (so dass ohnehin der
#* Attributwert des abhaengigen Formates den geaenderten
#* Wert verdeckt). Weiterhin kann der betroffene Node
#* feststellen, ob er von dem geaenderten Attribut Gebrauch
#* macht (Beispiel: Linienabstand fuer Unterstreichung wurde
#* geaendert, das Attribut Unterstreichung wurde aber nicht
#* verwendet). So wird bei Aenderungen der minimale Aufwand
#* zum Reformatieren erkannt.
 */
#ifndef _CALBCK_HXX
#define _CALBCK_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
class SvStream; 

namespace binfilter {
class SfxPoolItem;
class SwModify;
class SwClientIter;

// ----------
// SwClient
// ----------

class SwClient
{
    friend class SwModify;
    friend class SwClientIter;

    SwClient *pLeft, *pRight;			// fuer die AVL-Sortierung
    BOOL bModifyLocked : 1;				// wird in SwModify::Modify benutzt,
                                        // eigentlich ein Member des SwModify
                                        // aber aus Platzgruenden hier.
    BOOL bInModify 	: 1;				// ist in einem Modify. (Debug!!!)
    BOOL bInDocDTOR	: 1;				// Doc wird zerstoert, nicht "abmelden"
    BOOL bInCache	: 1;				// Ist im BorderAttrCache des Layout,
                                        // Traegt sich dann im Modify aus!
    BOOL bInSwFntCache : 1; 			// Ist im SwFont-Cache der Formatierung

protected:
    SwModify *pRegisteredIn;
    SwClient(SwModify *pToRegisterIn);

public:
    inline SwClient();
    virtual ~SwClient();
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    const SwModify* GetRegisteredIn() const { return pRegisteredIn; }

    //rtti, abgeleitete moegens gleichtun oder nicht. Wenn sie es gleichtun
    //kann ueber die Abhaengigkeitsliste eines Modify typsicher gecastet
    //werden.
    TYPEINFO();

    void LockModify()					{ bModifyLocked = TRUE;  }
    void UnlockModify()					{ bModifyLocked = FALSE; }
    void SetInCache( BOOL bNew )		{ bInCache = bNew;		 }
    void SetInSwFntCache( BOOL bNew ) 	{ bInSwFntCache = bNew;	 }
    int  IsModifyLocked() const			{ return bModifyLocked;  }
    int  IsInDocDTOR()	  const 		{ return bInDocDTOR;	 }
    int  IsInCache()	  const 		{ return bInCache;		 }
    int  IsInSwFntCache()  const 		{ return bInSwFntCache;	 }

        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

private:
    SwClient( const SwClient& );
    SwClient &operator=( const SwClient& );
};

inline SwClient::SwClient() :
    pLeft(0), pRight(0), pRegisteredIn(0)
{ bModifyLocked = bInModify = bInDocDTOR = bInCache = bInSwFntCache = FALSE; }


// ----------
// SwModify
// ----------

// Klasse hat eine doppelt Verkette Liste fuer die Abhaengigen.

class SwModify: public SwClient
{
    friend SvStream& operator<<( SvStream& aS, SwModify & );

    friend class SwClientIter;
    SwClient* pRoot;

    SwClient *_Remove(SwClient *pDepend);

public:
    SwModify(SwModify *pToRegisterIn );
    virtual ~SwModify();
    virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );
    void Add(SwClient *pDepend);
    SwClient *Remove(SwClient *pDepend)
        {	return bInDocDTOR ?  0 : _Remove( pDepend ); }

    const SwClient* GetDepends() const	{ return pRoot; }

        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    void SetInDocDTOR() { bInDocDTOR = TRUE; }

    void CheckCaching( const USHORT nWhich );

    BOOL IsLastDepend() const
        { return pRoot && !pRoot->pLeft && !pRoot->pRight; }
};

// ----------
// SwDepend
// ----------

/*
 * Sehr sinnvolle Klasse, wenn ein Objekt von mehreren Objekten
 * abhaengig ist. Diese sollte fuer jede Abhaengigkeit ein Objekt
 * der Klasse SwDepend als Member haben.
 */
class SwDepend: public SwClient
{
private:
    SwClient *pToTell;
public:
    SwClient* GetToTell() { return pToTell; }
    virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );
    SwDepend(SwClient *pTellHim, SwModify *pDepend);

        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem & ) const;
};


class SwClientIter
{
    friend SwClient* SwModify::_Remove(SwClient *);	// fuer Ptr-Korrektur
    friend void SwModify::Add(SwClient *);			// nur fuer ASSERT !

    SwModify& rRoot;
    SwClient *pAkt, *pDelNext;
    // fuers Updaten der aller Iteratoren beim Einfuegen/Loeschen von
    // Clients, wenn der Iterator gerade draufsteht.
    SwClientIter *pNxtIter;
    TypeId aSrchId;				// fuer First/Next - suche diesen Type

public:
    SwClientIter( SwModify& );
    ~SwClientIter();

    const SwModify& GetModify() const		{ return rRoot; }
          SwModify& GetModify() 			{ return rRoot; }

#ifndef CFRONT
    SwClient* operator++(int);	// zum Naechsten
    SwClient* operator--(int);	// zum Vorherigen
#endif
    SwClient* operator++();		// zum Naechsten
    SwClient* operator--();		// zum Vorherigen

    SwClient* GoStart(); 		// zum Anfang
    SwClient* GoEnd();			// zum Ende

    inline SwClient* GoRoot(); 		// wieder ab Root (==Start) anfangen

    SwClient* operator()() const
        { return pDelNext == pAkt ? pAkt : pDelNext; }

    int IsChanged() const { return pDelNext != pAkt; }

    SwClient* First( TypeId nType );
    SwClient* Next();
};

inline SwClient* SwClientIter::GoRoot()		// wieder ab Root anfangen
{
    pAkt = rRoot.pRoot;
    return (pDelNext = pAkt);
}



} //namespace binfilter
#endif
