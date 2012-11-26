/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CALBCK_HXX
#define _CALBCK_HXX

#include <tools/solar.h>
#include "swdllapi.h"
#include <typeinfo>
#include <boost/utility.hpp>

class SwModify;
class SwClientIter;
class SfxPoolItem;
class SfxHint;

/*
    SwModify and SwClient cooperate in propagating attribute changes.
    If an attribute changes, the change is notified to all dependent
    formats and other interested objects, e.g. Nodes. The clients will detect
    if the change affects them. It could be that the changed attribute is
    overruled in the receiving object so that its change does not become
    effective or that the receiver is not interested in the particular attribute
    in general (though probably in other attributes of the SwModify object they
    are registered in).
    As SwModify objects are derived from SwClient, they can create a chain of SwClient
    objects where changes can get propagated through.
    Each SwClient can be registered at only one SwModify object, while each SwModify
    object is connected to a list of SwClient objects. If an object derived from SwClient
    wants to get notifications from more than one SwModify object, it must create additional
    SwClient objects. The SwDepend class allows to handle their notifications in the same
    notification callback as it forwards the Modify() calls it receives to a "master"
    SwClient implementation.
    The SwClientIter class allows to iterate over the SwClient objects registered at an
    SwModify. For historical reasons its ability to use TypeInfo to restrict this iteration
    to objects of a particular type created a lot of code that misuses SwClient-SwModify
    relationships that basically should be used only for Modify() callbacks.
    This is still subject to refactoring.
    Until this gets resolved, new SwClientIter base code should be reduced to the absolute
    minimum and it also should be wrapped by SwIterator templates that prevent that the
    code gets polluted by pointer casts (see switerator.hxx).
 */

// ----------
// SwClient
// ----------

class SW_DLLPUBLIC SwClient : ::boost::noncopyable
{
    // avoids making the details of the linked list and the callback method public
    friend class SwModify;
    friend class SwClientIter;

    SwClient *pLeft, *pRight;       // double-linked list of other clients
    SwModify *pRegisteredIn;        // event source

    // in general clients should not be removed when their SwModify sends out Modify()
    // notifications; in some rare cases this is necessary, but only the concrete SwClient
    // sub class will know that; this flag allows to make that known
    bool mbIsAllowedToBeRemovedInModifyCall;

    // callbacks received from SwModify (friend class - so these methods can be private)
    // should be called only from SwModify the client is registered in
    // mba: IMHO these methods should be pure virtual
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint );

protected:
    // single argument ctors shall be explicit.
    explicit SwClient(SwModify *pToRegisterIn);

    // write access to pRegisteredIn shall be granted only to the object itself (protected access)
    SwModify* GetRegisteredInNonConst() const { return pRegisteredIn; }
    void SetIsAllowedToBeRemovedInModifyCall( bool bSet ) { mbIsAllowedToBeRemovedInModifyCall = bSet; }

public:

    inline SwClient();
    virtual ~SwClient();

    // in case an SwModify object is destroyed that itself is registered in another SwModify,
    // its SwClient objects can decide to get registered to the latter instead by calling this method
    void CheckRegistration( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue );

    // controlled access to Modify method
    // mba: this is still considered a hack and it should be fixed; the name makes grep-ing easier
    void ModifyNotification( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue ) { Modify ( pOldValue, pNewValue ); }
   void SwClientNotifyCall( const SwModify& rModify, const SfxHint& rHint ) { SwClientNotify( rModify, rHint ); }

    const SwModify* GetRegisteredIn() const { return pRegisteredIn; }
    bool IsLast() const { return !pLeft && !pRight; }

    virtual sal_Bool GetInfo( SfxPoolItem& ) const;
};

inline SwClient::SwClient() :
    pLeft(0), pRight(0), pRegisteredIn(0), mbIsAllowedToBeRemovedInModifyCall(false)
{}

// ----------
// SwModify
// ----------

class SW_DLLPUBLIC SwModify: public SwClient
{
//  friend class SwClientIter;

    SwClient* pRoot;                // the start of the linked list of clients
    sal_Bool bModifyLocked : 1;         // don't broadcast changes now
    sal_Bool bLockClientList : 1;       // may be set when this instance notifies its clients
    sal_Bool bInDocDTOR : 1;            // workaround for problems when a lot of objects are destroyed
    sal_Bool bInCache   : 1;
    sal_Bool bInSwFntCache : 1;

    // mba: IMHO this method should be pure virtual
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwModify();

    // broadcasting: send notifications to all clients
    void NotifyClients( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue );

    // the same, but without setting bModifyLocked or checking for any of the flags
    // mba: it would be interesting to know why this is necessary
    // also allows to limit callback to certain type (HACK)
    void ModifyBroadcast(
        const SfxPoolItem *pOldValue,
        const SfxPoolItem *pNewValue,
        bool (*pIsValidSwClient)(const SwClient&) = 0);

    // a more universal broadcasting mechanism
    void CallSwClientNotify( const SfxHint& rHint ) const;

    // single argument ctors shall be explicit.
    explicit SwModify( SwModify *pToRegisterIn );
    virtual ~SwModify();

    void Add(SwClient *pDepend);
    SwClient* Remove(SwClient *pDepend);
    const SwClient* GetDepends() const  { return pRoot; }

    // get information about attribute
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    void LockModify()                   { bModifyLocked = sal_True;  }
    void UnlockModify()                 { bModifyLocked = sal_False; }
    void SetInCache( sal_Bool bNew )        { bInCache = bNew;       }
    void SetInSwFntCache( sal_Bool bNew )   { bInSwFntCache = bNew;  }
    void SetInDocDTOR()                 { bInDocDTOR = sal_True; }
    sal_Bool IsModifyLocked() const     { return bModifyLocked;  }
    sal_Bool IsInDocDTOR()    const     { return bInDocDTOR;     }
    sal_Bool IsInCache()      const     { return bInCache;       }
    sal_Bool IsInSwFntCache() const     { return bInSwFntCache;  }

    void CheckCaching( const sal_uInt16 nWhich );
    bool IsLastDepend() { return pRoot && pRoot->IsLast(); }
    int GetClientCount() const;
};

// ----------
// SwDepend
// ----------

/*
 * Helper class for objects that need to depend on more than one SwClient
 */
class SW_DLLPUBLIC SwDepend: public SwClient
{
    SwClient *pToTell;

public:
    SwDepend() : pToTell(0) {}
    SwDepend(SwClient *pTellHim, SwModify *pDepend);

    SwClient* GetToTell() { return pToTell; }

    virtual sal_Bool GetInfo( SfxPoolItem & ) const;

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNewValue );
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint );
};


class SwClientIter
{
private:
    friend SwClient* SwModify::Remove(SwClient *); // for pointer adjustments
    friend void SwModify::Add(SwClient *pDepend);   // for pointer adjustments

    const SwModify& rRoot;

    // the current object in an iteration
    SwClient* pAct;

    // in case the current object is already removed, the next object in the list
    // is marked down to become the current object in the next step
    // this is necessary because iteration requires access to members of the current object
    SwClient* pDelNext;

    // SwClientIter objects are tracked in linked list so that they can react
    // when the current (pAct) or marked down (pDelNext) SwClient is removed
    // from its SwModify
    SwClientIter *pNxtIter;

public:
    SW_DLLPUBLIC SwClientIter( const SwModify& );
    SW_DLLPUBLIC ~SwClientIter();

    // return "true" if an object was removed from a client chain in iteration
    // adding objects to a client chain in iteration is forbidden
    // SwModify::Add() asserts this
    bool IsChanged() const { return pDelNext != pAct; }

    SW_DLLPUBLIC SwClient* SwClientIter_First();
    SW_DLLPUBLIC SwClient* SwClientIter_Next();
};

#endif
