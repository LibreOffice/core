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

#include <cassert>

#include <svl/hint.hxx>
#include <svl/broadcast.hxx>
#include "swdllapi.h"
#include "ring.hxx"
#include <type_traits>
#include <vector>
#include <memory>

class SwModify;
class SfxPoolItem;

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
    The SwIterator class allows to iterate over the SwClient objects registered at an
    SwModify. For historical reasons its ability to use TypeInfo to restrict this iteration
    to objects of a particular type created a lot of code that misuses SwClient-SwModify
    relationships that basically should be used only for Modify/Notify callbacks.
    This is still subject to refactoring.
 */

namespace sw
{
    class ClientIteratorBase;
    struct SW_DLLPUBLIC LegacyModifyHint final: SfxHint
    {
        LegacyModifyHint(const SfxPoolItem* pOld, const SfxPoolItem* pNew) : m_pOld(pOld), m_pNew(pNew) {};
        virtual ~LegacyModifyHint() override;
        const SfxPoolItem* m_pOld;
        const SfxPoolItem* m_pNew;
    };
    struct SW_DLLPUBLIC ModifyChangedHint final: SfxHint
    {
        ModifyChangedHint(const SwModify* pNew) : m_pNew(pNew) {};
        virtual ~ModifyChangedHint() override;
        const SwModify* m_pNew;
    };
    // Observer pattern using svl implementation
    // use this instead of SwClient/SwModify wherever possible
    // In writer layout, this might not always be possible,
    // but for listeners outside of it (e.g. unocore) this should be used.
    // The only "magic" signal this class issues is a ModifyChangedHint
    // proclaiming its death. It does NOT however provide a new SwModify for
    // listeners to switch to like the old SwModify/SwClient did, as that leads
    // to madness.
    class SW_DLLPUBLIC BroadcasterMixin {
        SvtBroadcaster m_aNotifier;
        public:
            BroadcasterMixin() = default;
            BroadcasterMixin(BroadcasterMixin const &) = default;
            BroadcasterMixin& operator=(const BroadcasterMixin&)
            {
                return *this; // Listeners are never copied or moved.
            }
            SvtBroadcaster& GetNotifier() { return m_aNotifier; }
    };
    /// refactoring out the same of the more sane SwClient functionality
    class SW_DLLPUBLIC WriterListener
    {
        friend class ::SwModify;
        friend class ::sw::ClientIteratorBase;
        private:
            WriterListener* m_pLeft;
            WriterListener* m_pRight; ///< double-linked list of other clients

            WriterListener(WriterListener const&) = delete;
            WriterListener& operator=(WriterListener const&) = delete;

        protected:
            WriterListener()
                : m_pLeft(nullptr), m_pRight(nullptr)
            {}
            virtual ~WriterListener() COVERITY_NOEXCEPT_FALSE {}
            virtual void SwClientNotify( const SwModify&, const SfxHint& rHint) =0;
        public:
            bool IsLast() const { return !m_pLeft && !m_pRight; }
    };
    enum class IteratorMode { Exact, UnwrapMulti };
}

// SwClient
class SW_DLLPUBLIC SwClient : public ::sw::WriterListener
{
    // avoids making the details of the linked list and the callback method public
    friend class SwModify;
    friend class sw::ClientIteratorBase;
    template<typename E, typename S, sw::IteratorMode> friend class SwIterator;

    SwModify *m_pRegisteredIn;        ///< event source

protected:
    // single argument ctors shall be explicit.
    inline explicit SwClient( SwModify* pToRegisterIn );

    // write access to pRegisteredIn shall be granted only to the object itself (protected access)
    SwModify* GetRegisteredInNonConst() const { return m_pRegisteredIn; }

public:
    SwClient() : m_pRegisteredIn(nullptr) {}
    SwClient(SwClient&&) noexcept;
    virtual ~SwClient() override;
    // callbacks received from SwModify (friend class - so these methods can be private)
    // should be called only from SwModify the client is registered in
    // mba: IMHO this method should be pure virtual
    // DO NOT USE IN NEW CODE! use SwClientNotify instead.
    virtual void Modify(const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue);
    // when overriding this, you MUST call SwClient::SwClientModify() in the override!
    virtual void SwClientNotify(const SwModify&, const SfxHint& rHint) override;

    // in case an SwModify object is destroyed that itself is registered in another SwModify,
    // its SwClient objects can decide to get registered to the latter instead by calling this method
    std::unique_ptr<sw::ModifyChangedHint> CheckRegistration( const SfxPoolItem* pOldValue );

    // controlled access to Modify method
    // mba: this is still considered a hack and it should be fixed; the name makes grep-ing easier
    virtual void ModifyNotification( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue ) { Modify ( pOldValue, pNewValue ); }
    void SwClientNotifyCall( const SwModify& rModify, const SfxHint& rHint ) { SwClientNotify( rModify, rHint ); }

    const SwModify* GetRegisteredIn() const { return m_pRegisteredIn; }
    SwModify* GetRegisteredIn() { return m_pRegisteredIn; }
    void EndListeningAll();
    void StartListeningToSameModifyAs(const SwClient&);


    // get information about attribute
    virtual bool GetInfo( SfxPoolItem& ) const { return true; }
};


// SwModify

// class has a doubly linked list for dependencies
class SW_DLLPUBLIC SwModify: public SwClient
{
    friend class sw::ClientIteratorBase;
    template<typename E, typename S, sw::IteratorMode> friend class SwIterator;
    sw::WriterListener* m_pWriterListeners;                // the start of the linked list of clients
    bool m_bModifyLocked : 1;         // don't broadcast changes now
    bool m_bLockClientList : 1;       // may be set when this instance notifies its clients
    bool m_bInCache   : 1;
    bool m_bInSwFntCache : 1;

    // mba: IMHO this method should be pure virtual
    // DO NOT USE IN NEW CODE! use CallSwClientNotify instead.
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override
        { NotifyClients( pOld, pNew ); };

    SwModify(SwModify const &) = delete;
    SwModify &operator =(const SwModify&) = delete;
public:
    SwModify()
        : SwClient(nullptr), m_pWriterListeners(nullptr), m_bModifyLocked(false), m_bLockClientList(false), m_bInCache(false), m_bInSwFntCache(false)
    {}
    explicit SwModify( SwModify* pToRegisterIn )
        : SwClient(pToRegisterIn), m_pWriterListeners(nullptr), m_bModifyLocked(false), m_bLockClientList(false), m_bInCache(false), m_bInSwFntCache(false)
    {}

    // broadcasting: send notifications to all clients
    // DO NOT USE IN NEW CODE! use CallSwClientNotify instead.
    void NotifyClients( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue );
    // the same, but without setting m_bModifyLocked or checking for any of the flags
    // DO NOT USE IN NEW CODE! use CallSwClientNotify instead.
    void ModifyBroadcast( const SfxPoolItem *pOldValue, const SfxPoolItem *pNewValue)
        { CallSwClientNotify( sw::LegacyModifyHint{ pOldValue, pNewValue } ); };

    // a more universal broadcasting mechanism
    virtual void CallSwClientNotify( const SfxHint& rHint ) const;

    virtual ~SwModify() override;

    void Add(SwClient *pDepend);
    SwClient* Remove(SwClient *pDepend);
    bool HasWriterListeners() const { return m_pWriterListeners; }

    // get information about attribute
    virtual bool GetInfo( SfxPoolItem& ) const override;

    void LockModify()                   { m_bModifyLocked = true;  }
    void UnlockModify()                 { m_bModifyLocked = false; }
    void SetInCache( bool bNew )        { m_bInCache = bNew;       }
    void SetInSwFntCache( bool bNew )   { m_bInSwFntCache = bNew;  }
    void SetInDocDTOR();
    bool IsModifyLocked() const     { return m_bModifyLocked;  }
    bool IsInCache()      const     { return m_bInCache;       }
    bool IsInSwFntCache() const     { return m_bInSwFntCache;  }

    void CheckCaching( const sal_uInt16 nWhich );
    bool HasOnlyOneListener() { return m_pWriterListeners && m_pWriterListeners->IsLast(); }
};

template<typename TElementType, typename TSource, sw::IteratorMode eMode> class SwIterator;

namespace sw
{
    // this class is part of the migration: it still forwards the "old"
    // SwModify events and announces them both to the old SwClients still
    // registered and also to the new SvtListeners.
    // Still: in the long run the SwClient/SwModify interface should not be
    // used anymore, in which case a BroadcasterMixin should be enough instead
    // then.
    class SW_DLLPUBLIC BroadcastingModify : public SwModify, public BroadcasterMixin {
        public:
            virtual void CallSwClientNotify(const SfxHint& rHint) const override;
    };
    // this should be hidden but sadly SwIterator template needs it...
    class ListenerEntry final : public SwClient
    {
    private:
        template<typename E, typename S, sw::IteratorMode> friend class ::SwIterator;
        SwClient *m_pToTell;

    public:
        ListenerEntry(SwClient *const pTellHim, SwModify *const pDepend)
            : SwClient(pDepend), m_pToTell(pTellHim)
        {}
        ListenerEntry(ListenerEntry const &) = delete;
        ListenerEntry& operator=(ListenerEntry const&) = delete;
        ListenerEntry(ListenerEntry&& other) noexcept
            : SwClient(std::move(other))
            , m_pToTell(other.m_pToTell)
        { }
        ListenerEntry& operator=(ListenerEntry&& other) noexcept
        {
            m_pToTell = other.m_pToTell;
            other.GetRegisteredIn()->Add(this);
            other.EndListeningAll();
            return *this;
        }

        /** get Client information */
        virtual bool GetInfo( SfxPoolItem& rInfo) const override;
    private:
        virtual void Modify(const SfxPoolItem* pOldValue, const SfxPoolItem *pNewValue) override;
        virtual void SwClientNotify(const SwModify& rModify, const SfxHint& rHint) override;
    };

    class SW_DLLPUBLIC WriterMultiListener final
    {
        SwClient& m_rToTell;
        std::vector<ListenerEntry> m_vDepends;
        public:
            WriterMultiListener(SwClient& rToTell);
            WriterMultiListener& operator=(WriterMultiListener const&) = delete; // MSVC2015 workaround
            WriterMultiListener(WriterMultiListener const&) = delete; // MSVC2015 workaround
            ~WriterMultiListener();
            void StartListening(SwModify* pDepend);
            void EndListening(SwModify* pDepend);
            bool IsListeningTo(const SwModify* const pDepend) const;
            void EndListeningAll();
    };
    class ClientIteratorBase : public sw::Ring< ::sw::ClientIteratorBase >
    {
            friend SwClient* SwModify::Remove(SwClient*);
            friend void SwModify::Add(SwClient*);
        protected:
            const SwModify& m_rRoot;
            // the current object in an iteration
            WriterListener* m_pCurrent;
            // in case the current object is already removed, the next object in the list
            // is marked down to become the current object in the next step
            // this is necessary because iteration requires access to members of the current object
            WriterListener* m_pPosition;
            static SW_DLLPUBLIC ClientIteratorBase* s_pClientIters;

            ClientIteratorBase( const SwModify& rModify )
                : m_rRoot(rModify)
            {
                MoveTo(s_pClientIters);
                s_pClientIters = this;
                m_pCurrent = m_pPosition = m_rRoot.m_pWriterListeners;
            }
            WriterListener* GetLeftOfPos() { return m_pPosition->m_pLeft; }
            WriterListener* GetRightOfPos() { return m_pPosition->m_pRight; }
            WriterListener* GoStart()
            {
                m_pPosition = m_rRoot.m_pWriterListeners;
                if(m_pPosition)
                    while( m_pPosition->m_pLeft )
                        m_pPosition = m_pPosition->m_pLeft;
                return m_pCurrent = m_pPosition;
            }
            ~ClientIteratorBase() override
            {
                assert(s_pClientIters);
                if(s_pClientIters == this)
                    s_pClientIters = unique() ? nullptr : GetNextInRing();
                MoveTo(nullptr);
            }
            // return "true" if an object was removed from a client chain in iteration
            // adding objects to a client chain in iteration is forbidden
            // SwModify::Add() asserts this
            bool IsChanged() const { return m_pPosition != m_pCurrent; }
            // ensures the iterator to point at a current client
            WriterListener* Sync() { return m_pCurrent = m_pPosition; }
    };
}

template<typename TElementType, typename TSource,
        sw::IteratorMode eMode = sw::IteratorMode::Exact> class SwIterator final
    : private sw::ClientIteratorBase
{
    //static_assert(!std::is_base_of<SwPageDesc,TSource>::value, "SwPageDesc as TSource is deprecated.");
    static_assert(std::is_base_of<SwClient,TElementType>::value, "TElementType needs to be derived from SwClient.");
    static_assert(std::is_base_of<SwModify,TSource>::value, "TSource needs to be derived from SwModify.");
public:
    SwIterator( const TSource& rSrc ) : sw::ClientIteratorBase(rSrc) {}
    TElementType* First()
    {
        GoStart();
        if(!m_pPosition)
            return nullptr;
        m_pCurrent = nullptr;
        return Next();
    }
    TElementType* Last()
    {
        if(!m_pPosition)
            m_pPosition = m_rRoot.m_pWriterListeners;
        if(!m_pPosition)
            return static_cast<TElementType*>(Sync());
        while(GetRightOfPos())
            m_pPosition = GetRightOfPos();
        sw::WriterListener * pCurrent(m_pPosition);
        if (eMode == sw::IteratorMode::UnwrapMulti)
        {
            if (auto const pLE = dynamic_cast<sw::ListenerEntry const*>(pCurrent))
            {
                pCurrent = pLE->m_pToTell;
            }
        }
        if (dynamic_cast<const TElementType *>(pCurrent) != nullptr)
        {
            Sync();
            return static_cast<TElementType*>(pCurrent);
        }
        return Previous();
    }
    TElementType* Next()
    {
        if(!IsChanged())
            m_pPosition = GetRightOfPos();
        sw::WriterListener *pCurrent(m_pPosition);
        while (m_pPosition)
        {
            if (eMode == sw::IteratorMode::UnwrapMulti)
            {
                if (auto const pLE = dynamic_cast<sw::ListenerEntry const*>(m_pPosition))
                {
                    pCurrent = pLE->m_pToTell;
                }
            }
            if (dynamic_cast<const TElementType *>(pCurrent) == nullptr)
            {
                m_pPosition = GetRightOfPos();
                pCurrent = m_pPosition;
            }
            else
                break;
        }
        Sync();
        return static_cast<TElementType*>(pCurrent);
    }
    TElementType* Previous()
    {
        m_pPosition = GetLeftOfPos();
        sw::WriterListener *pCurrent(m_pPosition);
        while (m_pPosition)
        {
            if (eMode == sw::IteratorMode::UnwrapMulti)
            {
                if (auto const pLE = dynamic_cast<sw::ListenerEntry const*>(m_pPosition))
                {
                    pCurrent = pLE->m_pToTell;
                }
            }
            if (dynamic_cast<const TElementType *>(pCurrent) == nullptr)
            {
                m_pPosition = GetLeftOfPos();
                pCurrent = m_pPosition;
            }
            else
                break;
        }
        Sync();
        return static_cast<TElementType*>(pCurrent);
    }
    using sw::ClientIteratorBase::IsChanged;
};

template< typename TSource > class SwIterator<SwClient, TSource> final : private sw::ClientIteratorBase
{
    static_assert(std::is_base_of<SwModify,TSource>::value, "TSource needs to be derived from SwModify");
public:
    SwIterator( const TSource& rSrc ) : sw::ClientIteratorBase(rSrc) {}
    SwClient* First()
        { return static_cast<SwClient*>(GoStart()); }
    SwClient* Next()
    {
        if(!IsChanged())
            m_pPosition = GetRightOfPos();
        return static_cast<SwClient*>(Sync());
    }
    using sw::ClientIteratorBase::IsChanged;
};

SwClient::SwClient( SwModify* pToRegisterIn )
    : m_pRegisteredIn( nullptr )
{
    if(pToRegisterIn)
        pToRegisterIn->Add(this);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
