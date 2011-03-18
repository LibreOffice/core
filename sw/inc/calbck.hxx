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

/*************************************************************
#* service classes
 *************************************************************/

/*
 * When an attribute in a format is changed, this change has to be propagated
 * to all dependent formats and over them to all concerned nodes.
 * In doing so it has to be decided whether the change can have an effect or
 * whether the dependent format redefines the changed attribute (such that the
 * attribute value of the dependent format occludes the changed value).
 * Furthermore, the concerned node can decide whether it makes use of the
 * changed attribute (for example: line distance for underlining was changed,
 * but the underlining attribute was not used). This way, the minimal effort
 * for reformatting is identified.
 */
#ifndef _CALBCK_HXX
#define _CALBCK_HXX

#include <tools/rtti.hxx>
#include "swdllapi.h"

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

    SwClient *pLeft, *pRight;           // for AVL sorting
    sal_Bool bModifyLocked : 1;         // used in SwModify::Modify,
                                        // is really a member of SwModify
                                        // but here for lack of space

    sal_Bool bInModify  : 1;            // is in a modify. (Debug!!!)
    sal_Bool bInDocDTOR : 1;            // Doc gets destroyed,
                                        // do not "unsubscribe"
    sal_Bool bInCache   : 1;            // is in BorderAttrCache of the layout,
                                        // unsubscribes itself then in Modify!
    sal_Bool bInSwFntCache : 1;         // is in SwFont cache of the formatting

protected:
    SwModify *pRegisteredIn;

    // single argument ctors shall be explicit.
    explicit SwClient(SwModify *pToRegisterIn);

public:
    inline SwClient();
    virtual ~SwClient();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    const SwModify* GetRegisteredIn() const { return pRegisteredIn; }

    //rtti, derived classes may do likewise or not. When they do, it can be
    //casted typesafely via the dependency list of a Modify
    TYPEINFO();

    void LockModify()                   { bModifyLocked = sal_True;  }
    void UnlockModify()                 { bModifyLocked = sal_False; }
    void SetInCache( sal_Bool bNew )        { bInCache = bNew;       }
    void SetInSwFntCache( sal_Bool bNew )   { bInSwFntCache = bNew;  }
    sal_Bool IsModifyLocked() const         { return bModifyLocked;  }
    sal_Bool IsInDocDTOR()    const         { return bInDocDTOR;     }
    sal_Bool IsInCache()      const         { return bInCache;       }
    sal_Bool IsInSwFntCache()  const        { return bInSwFntCache;  }

    // get client Information
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

private:
    SwClient( const SwClient& );
    SwClient &operator=( const SwClient& );
};

inline SwClient::SwClient() :
    pLeft(0), pRight(0), pRegisteredIn(0)
{ bModifyLocked = bInModify = bInDocDTOR = bInCache = bInSwFntCache = sal_False; }


// ----------
// SwModify
// ----------

// class has a doubly linked list for dependencies

class SW_DLLPUBLIC SwModify: public SwClient
{
    friend SvStream& operator<<( SvStream& aS, SwModify & );

    friend class SwClientIter;
    SwClient* pRoot;

    SwClient *_Remove(SwClient *pDepend);

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

    // get client information
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    void SetInDocDTOR() { bInDocDTOR = sal_True; }

    void CheckCaching( const sal_uInt16 nWhich );

    sal_Bool IsLastDepend() const
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
 * Very useful class when an object depends on multiple objects.
 * This should have an object of the class SwDepend as member for each
 * dependency.
 */
class SW_DLLPUBLIC SwDepend: public SwClient
{
    SwClient *pToTell;

public:
    SwDepend() : pToTell(0) {}
    SwDepend(SwClient *pTellHim, SwModify *pDepend);

    SwClient* GetToTell() { return pToTell; }
    virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );

    // get Client information
    virtual sal_Bool GetInfo( SfxPoolItem & ) const;

private:
    // forbidden and not implemented (see @ SwClient).
    SwDepend (const SwDepend &);
    SwDepend & operator= (const SwDepend &);
};


class SwClientIter
{
    friend SwClient* SwModify::_Remove(SwClient *); // for ptr correction
    friend void SwModify::Add(SwClient *);          // only for OSL_ENSURE !

    SwModify const& rRoot;
    SwClient *pAkt, *pDelNext;
    // for updating of all iterators when inserting/deleting clients, while the
    // iterator points on it
    SwClientIter *pNxtIter;

    SwClient* mpWatchClient;    // if set, SwModify::_Remove checks if this client is removed

    TypeId aSrchId;             // for First/Next - look for this type

public:
    SW_DLLPUBLIC SwClientIter( SwModify const& );
    SW_DLLPUBLIC ~SwClientIter();

    const SwModify& GetModify() const       { return rRoot; }

    SwClient* operator++(int);
    SwClient* operator--(int);
    SwClient* operator++();
    SwClient* operator--();

    SwClient* GoStart();
    SwClient* GoEnd();

    inline SwClient* GoRoot();      // restart from root

    SwClient* operator()() const
        { return pDelNext == pAkt ? pAkt : pDelNext; }

    int IsChanged() const { return pDelNext != pAkt; }

    SW_DLLPUBLIC SwClient* First( TypeId nType );
    SW_DLLPUBLIC SwClient* Next();

    const SwClient* GetWatchClient() const { return mpWatchClient; }
    void SetWatchClient( SwClient* pWatch ) { mpWatchClient = pWatch; }
};

inline SwClient* SwClientIter::GoRoot()
{
    pAkt = rRoot.pRoot;
    return (pDelNext = pAkt);
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
