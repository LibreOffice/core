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

#ifndef INCLUDED_SW_INC_CALBCK_HXX
#define INCLUDED_SW_INC_CALBCK_HXX

#include <tools/rtti.hxx>
#include "swdllapi.h"
#include <boost/noncopyable.hpp>
#include <ring.hxx>
#include <hintids.hxx>
#include <hints.hxx>


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

class SwModify;
class SwClient;
class SwClientIter;
namespace sw
{
    /// refactoring out the some of the more sane SwClient functionality
    class SW_DLLPUBLIC WriterListener : ::boost::noncopyable
    {
        friend class ::SwModify;
        friend class ::SwClient;
        friend class ::SwClientIter;
        private:
            WriterListener* m_pLeft;
            WriterListener* m_pRight; ///< double-linked list of other clients
        protected:
            WriterListener()
                : m_pLeft(nullptr), m_pRight(nullptr)
            {}
            virtual ~WriterListener() {};
            // callbacks received from SwModify (friend class - so these methods can be private)
            // should be called only from SwModify the client is registered in
            // mba: IMHO these methods should be pure virtual
            virtual void Modify(const SfxPoolItem*, const SfxPoolItem*) {};
            virtual void SwClientNotify( const SwModify&, const SfxHint&) {};
        public:
            bool IsLast() const { return !m_pLeft && !m_pRight; }
   };
}
// SwClient
class SW_DLLPUBLIC SwClient : ::sw::WriterListener
{
    // avoids making the details of the linked list and the callback method public
    friend class SwModify;
    friend class SwClientIter;

    SwModify *pRegisteredIn;        ///< event source

protected:
    // single argument ctors shall be explicit.
    inline explicit SwClient( SwModify* pToRegisterIn );

    // write access to pRegisteredIn shall be granted only to the object itself (protected access)
    SwModify* GetRegisteredInNonConst() const { return pRegisteredIn; }

public:

    SwClient() : pRegisteredIn(nullptr) {}
    virtual ~SwClient() SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue ) SAL_OVERRIDE
        { CheckRegistration( pOldValue, pNewValue ); }

    // in case an SwModify object is destroyed that itself is registered in another SwModify,
    // its SwClient objects can decide to get registered to the latter instead by calling this method
    void CheckRegistration( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue );

    // controlled access to Modify method
    // mba: this is still considered a hack and it should be fixed; the name makes grep-ing easier
    void ModifyNotification( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue ) { this->Modify ( pOldValue, pNewValue ); }
    void SwClientNotifyCall( const SwModify& rModify, const SfxHint& rHint ) { SwClientNotify( rModify, rHint ); }

    const SwModify* GetRegisteredIn() const { return pRegisteredIn; }
    SwModify* GetRegisteredIn() { return pRegisteredIn; }

    // needed for class SwClientIter
    TYPEINFO();

    // get information about attribute
    virtual bool GetInfo( SfxPoolItem& ) const { return true; }
};


// SwModify

// class has a doubly linked list for dependencies
class SW_DLLPUBLIC SwModify: public SwClient
{
    sw::WriterListener* pRoot;                // the start of the linked list of clients
    bool bModifyLocked : 1;         // don't broadcast changes now
    bool bLockClientList : 1;       // may be set when this instance notifies its clients
    bool bInDocDTOR : 1;            // workaround for problems when a lot of objects are destroyed
    bool bInCache   : 1;
    bool bInSwFntCache : 1;

    // mba: IMHO this method should be pure virtual
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE
        { NotifyClients( pOld, pNew ); };

public:
    SwModify()
        : SwClient(nullptr), pRoot(nullptr), bModifyLocked(false), bLockClientList(false), bInDocDTOR(false), bInCache(false), bInSwFntCache(false)
    {}

    // broadcasting: send notifications to all clients
    void NotifyClients( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue );

    // the same, but without setting bModifyLocked or checking for any of the flags
    // mba: it would be interesting to know why this is necessary
    // also allows to limit callback to certain type (HACK)
    inline void ModifyBroadcast( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue, TypeId nType = TYPE(SwClient) );

    // a more universal broadcasting mechanism
    inline void CallSwClientNotify( const SfxHint& rHint ) const;

    // single argument ctors shall be explicit.
    explicit SwModify( SwModify* pToRegisterIn )
        : SwClient(pToRegisterIn), pRoot(nullptr), bModifyLocked(false), bLockClientList(false), bInDocDTOR(false), bInCache(false), bInSwFntCache(false)
    {}

    virtual ~SwModify();

    void Add(SwClient *pDepend);
    SwClient* Remove(SwClient *pDepend);
    const SwClient* GetDepends() const  { return static_cast<SwClient*>(pRoot); }

    // get information about attribute
    virtual bool GetInfo( SfxPoolItem& ) const SAL_OVERRIDE;

    void LockModify()                   { bModifyLocked = true;  }
    void UnlockModify()                 { bModifyLocked = false; }
    void SetInCache( bool bNew )        { bInCache = bNew;       }
    void SetInSwFntCache( bool bNew )   { bInSwFntCache = bNew;  }
    void SetInDocDTOR()                 { bInDocDTOR = true; }
    bool IsModifyLocked() const     { return bModifyLocked;  }
    bool IsInDocDTOR()    const     { return bInDocDTOR;     }
    bool IsInCache()      const     { return bInCache;       }
    bool IsInSwFntCache() const     { return bInSwFntCache;  }

    void CheckCaching( const sal_uInt16 nWhich );
    bool IsLastDepend() { return pRoot && pRoot->IsLast(); }
};

// SwDepend

/*
 * Helper class for objects that need to depend on more than one SwClient
 */
class SW_DLLPUBLIC SwDepend SAL_FINAL : public SwClient
{
    SwClient *m_pToTell;

public:
    SwDepend() : m_pToTell(nullptr) {}
    SwDepend(SwClient *pTellHim, SwModify *pDepend) : SwClient(pDepend), m_pToTell(pTellHim) {}

    SwClient* GetToTell() { return m_pToTell; }

    /** get Client information */
    virtual bool GetInfo( SfxPoolItem& rInfo) const SAL_OVERRIDE
        { return m_pToTell ? m_pToTell->GetInfo( rInfo ) : true; }
protected:
    virtual void Modify( const SfxPoolItem* pOldValue, const SfxPoolItem *pNewValue ) SAL_OVERRIDE
    {
        if( pNewValue && pNewValue->Which() == RES_OBJECTDYING )
            CheckRegistration(pOldValue,pNewValue);
        else if( m_pToTell )
            m_pToTell->ModifyNotification(pOldValue, pNewValue);
    }
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint ) SAL_OVERRIDE
        { if(m_pToTell) m_pToTell->SwClientNotifyCall(rModify, rHint); }
};

class SwClientIter SAL_FINAL : public sw::Ring<SwClientIter>
{
    friend SwClient* SwModify::Remove(SwClient *); ///< for pointer adjustments
    friend void SwModify::Add(SwClient *pDepend);   ///< for pointer adjustments

    const SwModify& m_rRoot;

    // the current object in an iteration
    SwClient* m_pCurrent;

    // in case the current object is already removed, the next object in the list
    // is marked down to become the current object in the next step
    // this is necessary because iteration requires access to members of the current object
    SwClient* m_pPosition;

    // iterator can be limited to return only SwClient objects of a certain type
    TypeId m_aSearchType;

    static SW_DLLPUBLIC SwClientIter* our_pClientIters;

public:
    SwClientIter( const SwModify& rModify )
        : m_rRoot(rModify)
        , m_aSearchType(nullptr)
    {
        MoveTo(our_pClientIters);
        our_pClientIters = this;
        m_pCurrent = m_pPosition = const_cast<SwClient*>(m_rRoot.GetDepends());
    }
    ~SwClientIter() SAL_OVERRIDE
    {
        assert(our_pClientIters);
        if(our_pClientIters == this)
            our_pClientIters = unique() ? nullptr : GetNextInRing();
        MoveTo(nullptr);
    }

    const SwModify& GetModify() const { return m_rRoot; }

    SwClient* operator++()
    {
        if( m_pPosition == m_pCurrent )
            m_pPosition = static_cast<SwClient*>(m_pPosition->m_pRight);
        return m_pCurrent = m_pPosition;
    }
    SwClient* GoStart()
    {
        if((m_pPosition = const_cast<SwClient*>(m_rRoot.GetDepends())))
            while( m_pPosition->m_pLeft )
                m_pPosition = static_cast<SwClient*>(m_pPosition->m_pLeft);
        return m_pCurrent = m_pPosition;
    }
    SwClient* GoEnd()
    {
        if(!m_pPosition)
            m_pPosition = const_cast<SwClient*>(m_rRoot.GetDepends());
        if(m_pPosition)
            while( m_pPosition->m_pRight )
                m_pPosition = static_cast<SwClient*>(m_pPosition->m_pRight);
        return m_pCurrent = m_pPosition;
    }

    // returns the current SwClient object, if its still a listener
    // otherwise it returns the next SwClient that still is
    SwClient* operator()() const
        { return m_pPosition; }
    // returns the current SwClient object, wether it is still a client or not
    SwClient& operator*() const
        { return *m_pCurrent; }
    // returns the current SwClient object, wether it is still a client or not
    SwClient* operator->() const
        { return m_pCurrent; }
    explicit operator bool() const
        { return m_pCurrent!=nullptr; }

    // return "true" if an object was removed from a client chain in iteration
    // adding objects to a client chain in iteration is forbidden
    // SwModify::Add() asserts this
    bool IsChanged() const { return m_pPosition != m_pCurrent; }

    SwClient* First( TypeId nType )
    {
        m_aSearchType = nType;
        GoStart();
        if(!m_pPosition)
            return nullptr;
        m_pCurrent = nullptr;
        return Next();
    }
    SwClient* Last( TypeId nType )
    {
        m_aSearchType = nType;
        GoEnd();
        if(!m_pPosition)
            return nullptr;
        if( m_pPosition->IsA( m_aSearchType ) )
            return m_pPosition;
        return Previous();
    }
    SwClient* Next()
    {
        if( m_pPosition == m_pCurrent )
            m_pPosition = static_cast<SwClient*>(m_pPosition->m_pRight);
        while(m_pPosition && !m_pPosition->IsA( m_aSearchType ) )
            m_pPosition = static_cast<SwClient*>(m_pPosition->m_pRight);
        return m_pCurrent = m_pPosition;
    }

    SwClient* Previous()
    {
        m_pPosition = static_cast<SwClient*>(m_pPosition->m_pLeft);
        while(m_pPosition && !m_pPosition->IsA( m_aSearchType ) )
            m_pPosition = static_cast<SwClient*>(m_pPosition->m_pLeft);
        return m_pCurrent = m_pPosition;
    }
};

SwClient::SwClient( SwModify* pToRegisterIn )
    : pRegisteredIn( nullptr )
{
    if(pToRegisterIn)
        pToRegisterIn->Add(this);
}

void SwModify::ModifyBroadcast( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue, TypeId nType)
{
    SwClientIter aIter(*this);
    for(aIter.First(nType); aIter; aIter.Next())
        aIter->Modify( pOldValue, pNewValue );
}

void SwModify::CallSwClientNotify( const SfxHint& rHint ) const
{
    for(SwClientIter aIter(*this); aIter; ++aIter)
        aIter->SwClientNotify( *this, rHint );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
