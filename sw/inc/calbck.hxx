/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: calbck.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:56:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SwModify;
class SwClientIter;
class SfxPoolItem;
class SvStream;

// ----------
// SwClient
// ----------

class SW_DLLPUBLIC SwClient
{
    friend class SwModify;
    friend class SwClientIter;

    SwClient *pLeft, *pRight;           // fuer die AVL-Sortierung
    BOOL bModifyLocked : 1;             // wird in SwModify::Modify benutzt,
                                        // eigentlich ein Member des SwModify
                                        // aber aus Platzgruenden hier.
    BOOL bInModify  : 1;                // ist in einem Modify. (Debug!!!)
    BOOL bInDocDTOR : 1;                // Doc wird zerstoert, nicht "abmelden"
    BOOL bInCache   : 1;                // Ist im BorderAttrCache des Layout,
                                        // Traegt sich dann im Modify aus!
    BOOL bInSwFntCache : 1;             // Ist im SwFont-Cache der Formatierung

protected:
    SwModify *pRegisteredIn;

    // single argument ctors shall be explicit.
    explicit SwClient(SwModify *pToRegisterIn);

public:
    inline SwClient();
    virtual ~SwClient();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    const SwModify* GetRegisteredIn() const { return pRegisteredIn; }

    //rtti, abgeleitete moegens gleichtun oder nicht. Wenn sie es gleichtun
    //kann ueber die Abhaengigkeitsliste eines Modify typsicher gecastet
    //werden.
    TYPEINFO();

    void LockModify()                   { bModifyLocked = TRUE;  }
    void UnlockModify()                 { bModifyLocked = FALSE; }
    void SetInCache( BOOL bNew )        { bInCache = bNew;       }
    void SetInSwFntCache( BOOL bNew )   { bInSwFntCache = bNew;  }
    BOOL IsModifyLocked() const         { return bModifyLocked;  }
    BOOL IsInDocDTOR()    const         { return bInDocDTOR;     }
    BOOL IsInCache()      const         { return bInCache;       }
    BOOL IsInSwFntCache()  const        { return bInSwFntCache;  }

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

class SW_DLLPUBLIC SwModify: public SwClient
{
    friend SvStream& operator<<( SvStream& aS, SwModify & );

    friend class SwClientIter;
    SwClient* pRoot;

    SW_DLLPRIVATE SwClient *_Remove(SwClient *pDepend);

public:
    SwModify() : pRoot(0) {}

    // single argument ctors shall be explicit.
    explicit SwModify(SwModify *pToRegisterIn );
    virtual ~SwModify();

    virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );
    void Add(SwClient *pDepend);
    SwClient *Remove(SwClient *pDepend)
        {   return bInDocDTOR ?  0 : _Remove( pDepend ); }

    const SwClient* GetDepends() const  { return pRoot; }

        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    void SetInDocDTOR() { bInDocDTOR = TRUE; }

    void CheckCaching( const USHORT nWhich );

    BOOL IsLastDepend() const
        { return pRoot && !pRoot->pLeft && !pRoot->pRight; }

private:
    // forbidden and not implemented (see @ SwClient).
    SwModify & operator= (const SwModify &);

protected:
    // forbidden and not implemented (see @ SwClient),
    //   but GCC >= 3.4 needs an accessible "T (const T&)"
    //   to pass a "T" as a "const T&" argument
    SwModify (const SwModify &);
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
    SwClient *pToTell;

public:
    SwDepend() : pToTell(0) {}
    SwDepend(SwClient *pTellHim, SwModify *pDepend);

    SwClient* GetToTell() { return pToTell; }
    virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );

        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem & ) const;

private:
    // forbidden and not implemented (see @ SwClient).
    SwDepend (const SwDepend &);
    SwDepend & operator= (const SwDepend &);
};


class SW_DLLPUBLIC SwClientIter
{
    friend SwClient* SwModify::_Remove(SwClient *); // fuer Ptr-Korrektur
    friend void SwModify::Add(SwClient *);          // nur fuer ASSERT !

    SwModify& rRoot;
    SwClient *pAkt, *pDelNext;
    // fuers Updaten der aller Iteratoren beim Einfuegen/Loeschen von
    // Clients, wenn der Iterator gerade draufsteht.
    SwClientIter *pNxtIter;

    SwClient* mpWatchClient;    // if set, SwModify::_Remove checks if this client is removed

    TypeId aSrchId;             // fuer First/Next - suche diesen Type

public:
    SwClientIter( SwModify& );
    ~SwClientIter();

    const SwModify& GetModify() const       { return rRoot; }
          SwModify& GetModify()             { return rRoot; }

#ifndef CFRONT
    SwClient* operator++(int);  // zum Naechsten
    SwClient* operator--(int);  // zum Vorherigen
#endif
    SwClient* operator++();     // zum Naechsten
    SwClient* operator--();     // zum Vorherigen

    SwClient* GoStart();        // zum Anfang
    SwClient* GoEnd();          // zum Ende

    inline SwClient* GoRoot();      // wieder ab Root (==Start) anfangen

    SwClient* operator()() const
        { return pDelNext == pAkt ? pAkt : pDelNext; }

    int IsChanged() const { return pDelNext != pAkt; }

    SwClient* First( TypeId nType );
    SwClient* Next();

    const SwClient* GetWatchClient() const { return mpWatchClient; }
    void SetWatchClient( SwClient* pWatch ) { mpWatchClient = pWatch; }
};

inline SwClient* SwClientIter::GoRoot()     // wieder ab Root anfangen
{
    pAkt = rRoot.pRoot;
    return (pDelNext = pAkt);
}



#endif
