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

#ifndef INCLUDED_STORE_SOURCE_STORBASE_HXX
#define INCLUDED_STORE_SOURCE_STORBASE_HXX

#include <sal/config.h>
#include <salhelper/simplereferenceobject.hxx>

#include <sal/types.h>

#include <rtl/crc.h>
#include <rtl/ref.hxx>

#include <osl/diagnose.h>
#include <osl/endian.h>

#include <store/types.h>

#include <memory>
#include <utility>

/** @file store common internals.
*/

namespace store
{

#ifdef htons
#undef htons
#endif
#ifdef ntohs
#undef ntohs
#endif

#ifdef htonl
#undef htonl
#endif
#ifdef ntohl
#undef ntohl
#endif

#ifdef OSL_BIGENDIAN
inline sal_uInt16 htons (sal_uInt16 h) { return OSL_SWAPWORD(h); }
inline sal_uInt16 ntohs (sal_uInt16 n) { return OSL_SWAPWORD(n); }

inline sal_uInt32 htonl (sal_uInt32 h) { return OSL_SWAPDWORD(h); }
inline sal_uInt32 ntohl (sal_uInt32 n) { return OSL_SWAPDWORD(n); }
#else
inline sal_uInt16 htons (sal_uInt16 h) { return h; }
inline sal_uInt16 ntohs (sal_uInt16 n) { return n; }

inline sal_uInt32 htonl (sal_uInt32 h) { return h; }
inline sal_uInt32 ntohl (sal_uInt32 n) { return n; }
#endif /* OSL_BIGENDIAN */

struct OStorePageGuard
{
    /** Representation.
     */
    sal_uInt32 m_nMagic;
    sal_uInt32 m_nCRC32;

    /** Construction.
     */
    explicit OStorePageGuard (sal_uInt32 nMagic = 0)
        : m_nMagic (store::htonl(nMagic)),
          m_nCRC32 (store::htonl(0))
    {}

    void swap (OStorePageGuard & rhs)
    {
        std::swap(m_nMagic, rhs.m_nMagic);
        std::swap(m_nCRC32, rhs.m_nCRC32);
    }

    OStorePageGuard (OStorePageGuard const & rhs)
        : m_nMagic (rhs.m_nMagic),
          m_nCRC32 (rhs.m_nCRC32)
    {}

    OStorePageGuard& operator= (const OStorePageGuard& rhs)
    {
        m_nMagic = rhs.m_nMagic;
        m_nCRC32 = rhs.m_nCRC32;
        return *this;
    }

    /** Comparison.
     */
    bool operator== (const OStorePageGuard& rhs) const
    {
        return ((m_nMagic == rhs.m_nMagic) &&
                (m_nCRC32 == rhs.m_nCRC32)    );
    }
};

#define STORE_PAGE_NULL (sal_uInt32(~0))

struct OStorePageDescriptor
{
    /** Representation.
     */
    sal_uInt32 m_nAddr;
    sal_uInt16 m_nSize;
    sal_uInt16 m_nUsed;

    /** Construction.
     */
    explicit OStorePageDescriptor (
        sal_uInt32 nAddr,
        sal_uInt16 nSize,
        sal_uInt16 nUsed)
        : m_nAddr (store::htonl(nAddr)),
          m_nSize (store::htons(nSize)),
          m_nUsed (store::htons(nUsed))
    {}

    void swap (OStorePageDescriptor & rhs)
    {
        std::swap(m_nAddr, rhs.m_nAddr);
        std::swap(m_nSize, rhs.m_nSize);
        std::swap(m_nUsed, rhs.m_nUsed);
    }

    OStorePageDescriptor (const OStorePageDescriptor & rhs)
        : m_nAddr (rhs.m_nAddr),
          m_nSize (rhs.m_nSize),
          m_nUsed (rhs.m_nUsed)
    {}

    OStorePageDescriptor & operator= (const OStorePageDescriptor & rhs)
    {
        m_nAddr = rhs.m_nAddr;
        m_nSize = rhs.m_nSize;
        m_nUsed = rhs.m_nUsed;
        return *this;
    }

    /** Comparison.
     */
    bool operator== (const OStorePageDescriptor & rhs) const
    {
        return ((m_nAddr == rhs.m_nAddr) &&
                (m_nSize == rhs.m_nSize)    );
    }
};

struct OStorePageKey
{
    /** Representation.
     */
    sal_uInt32 m_nLow;
    sal_uInt32 m_nHigh;

    /** Construction.
     */
    explicit OStorePageKey (sal_uInt32 nLow = 0, sal_uInt32 nHigh = 0)
        : m_nLow  (store::htonl(nLow)),
          m_nHigh (store::htonl(nHigh))
    {}

    /** Comparison.
     */
    bool operator== (const OStorePageKey & rhs) const
    {
        return ((m_nLow  == rhs.m_nLow ) &&
                (m_nHigh == rhs.m_nHigh)    );
    }

    bool operator< (const OStorePageKey & rhs) const
    {
        if (m_nHigh == rhs.m_nHigh)
            return (store::ntohl(m_nLow) < store::ntohl(rhs.m_nLow));
        else
            return (store::ntohl(m_nHigh) < store::ntohl(rhs.m_nHigh));
    }
};

struct OStorePageLink
{
    /** Representation.
     */
    sal_uInt32 m_nAddr;

    /** Construction.
     */
    explicit OStorePageLink (sal_uInt32 nAddr = STORE_PAGE_NULL)
        : m_nAddr (store::htonl(nAddr))
    {}

    void swap (OStorePageLink & rhs)
    {
        std::swap(m_nAddr, rhs.m_nAddr);
    }

    OStorePageLink & operator= (sal_uInt32 nAddr)
    {
        m_nAddr = store::htonl(nAddr);
        return *this;
    }

    /** Comparison.
     */
    bool operator== (const OStorePageLink & rhs) const
    {
        return (m_nAddr == rhs.m_nAddr);
    }

    /** Operation.
     */
    sal_uInt32 location() const
    {
        return store::ntohl(m_nAddr);
    }

};

struct PageData
{
    typedef OStorePageGuard      G;
    typedef OStorePageDescriptor D;
    typedef OStorePageLink       L;

    /** Representation.
     */
    G m_aGuard;
    D m_aDescr;
    L m_aMarked;
    L m_aUnused;

    /** theSize.
     */
    static const size_t     theSize     = sizeof(G) + sizeof(D) + 2 * sizeof(L);
    static const sal_uInt16 thePageSize = theSize;
    static_assert(STORE_MINIMUM_PAGESIZE >= thePageSize, "must be at least thePageSize");

    /** location.
     */
    sal_uInt32 location() const
    {
        return store::ntohl(m_aDescr.m_nAddr);
    }
    void location (sal_uInt32 nAddr)
    {
        m_aDescr.m_nAddr = store::htonl(nAddr);
    }

    /** size.
     */
    sal_uInt16 size() const
    {
        return store::ntohs(m_aDescr.m_nSize);
    }

    /** type.
     */
    sal_uInt32 type() const
    {
        return store::ntohl(m_aGuard.m_nMagic);
    }

    /** Allocation.
     */
    class Allocator_Impl;
    class Allocator : public virtual salhelper::SimpleReferenceObject
    {
    public:
        template< class T > T * construct()
        {
            void * page = nullptr; sal_uInt16 size = 0;
            if (allocate (&page, &size))
            {
                return new(page) T(size);
            }
            return 0;
        }

        bool allocate (void ** ppPage, sal_uInt16 * pnSize)
        {
            allocate_Impl (ppPage, pnSize);
            return ((*ppPage != nullptr) && (*pnSize != 0));
        }

        void deallocate (void * pPage)
        {
            if (pPage != nullptr)
                deallocate_Impl (pPage);
        }

        static storeError createInstance (
            rtl::Reference< PageData::Allocator > & rxAllocator, sal_uInt16 nPageSize);

    protected:
        virtual ~Allocator() override {}

    private:
        /** Implementation (abstract).
         */
        virtual void allocate_Impl (void ** ppPage, sal_uInt16 * pnSize) = 0;
        virtual void deallocate_Impl (void * pPage) = 0;
    };

    class Deallocate {
    public:
        explicit Deallocate(rtl::Reference<Allocator> const & allocator):
            allocator_(allocator) {};

        void operator ()(void * page) const { allocator_->deallocate(page); }

    private:
        rtl::Reference<Allocator> allocator_;
    };

    static void* operator new (size_t, void * p) { return p; }
    static void  operator delete (void * , void *) {}

    /** Construction.
     */
    explicit PageData (sal_uInt16 nPageSize = thePageSize)
        : m_aGuard(),
          m_aDescr(STORE_PAGE_NULL, nPageSize, thePageSize),
          m_aMarked(),
          m_aUnused()
    {}

    void swap (PageData & rhs) // nothrow
    {
        m_aGuard.swap(rhs.m_aGuard);
        m_aDescr.swap(rhs.m_aDescr);
        m_aMarked.swap(rhs.m_aMarked);
        m_aUnused.swap(rhs.m_aUnused);
    }

    PageData (PageData const & rhs) // nothrow
        : m_aGuard (rhs.m_aGuard),
          m_aDescr (rhs.m_aDescr),
          m_aMarked(rhs.m_aMarked),
          m_aUnused(rhs.m_aUnused)
    {}

    PageData & operator= (PageData const & rhs) // nothrow
    {
        PageData tmp (rhs);
        swap (tmp);
        return *this;
    }

    /** guard (external representation).
     */
    void guard (sal_uInt32 nAddr)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        m_aDescr.m_nAddr = store::htonl(nAddr);
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, static_cast<sal_uInt32>(theSize - sizeof(G)));
        m_aGuard.m_nCRC32 = store::htonl(nCRC32);
    }

    /** verify (external representation).
     */
    storeError verify (sal_uInt32 nAddr) const
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, static_cast<sal_uInt32>(theSize - sizeof(G)));
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        if (m_aDescr.m_nAddr != store::htonl(nAddr))
            return store_E_InvalidAccess;
        return store_E_None;
    }

};

template< class T >
class PageHolderObject
{
    /** Representation.
     */
    std::shared_ptr<PageData> m_xPage;

    /** Checked cast.
     */
    template< class U >
    static bool isA (PageData const * p)
    {
        return ((p != nullptr) && (p->type() == U::theTypeId));
    }

    template< class U >
    static U * dynamic_page_cast (PageData * p)
    {
        return isA<U>(p) ? static_cast<U*>(p) : 0;
    }

    template< class U >
    static U const * dynamic_page_cast (PageData const * p)
    {
        return isA<U>(p) ? static_cast<U const *>(p) : 0;
    }

public:
    bool construct (rtl::Reference< PageData::Allocator > const & rxAllocator)
    {
        if ((m_xPage.get() == 0) && rxAllocator.is())
        {
            std::shared_ptr<PageData> tmp (rxAllocator->construct<T>(), PageData::Deallocate(rxAllocator));
            m_xPage.swap (tmp);
        }
        return (m_xPage.get() != 0);
    }

    explicit PageHolderObject (std::shared_ptr<PageData> const & rxPage = std::shared_ptr<PageData>())
        : m_xPage (rxPage)
    {}

    void swap (PageHolderObject<T> & rhs)
    {
        m_xPage.swap (rhs.m_xPage);
    }

    PageHolderObject (PageHolderObject<T> const & rhs)
        : m_xPage (rhs.m_xPage)
    {}

    PageHolderObject<T> & operator= (PageHolderObject<T> const & rhs)
    {
        PageHolderObject<T> tmp (rhs);
        this->swap (tmp);
        return *this;
    }

    bool is() const
    {
        return (m_xPage.get() != 0);
    }

    std::shared_ptr<PageData> & get() { return m_xPage; }
    std::shared_ptr<PageData> const & get() const { return m_xPage; }

    T * operator->()
    {
        T * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != nullptr, "store::PageHolder<T>::operator*(): Null pointer");
        return pImpl;
    }

    T const * operator->() const
    {
        T const * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != nullptr, "store::PageHolder<T>::operator*(): Null pointer");
        return pImpl;
    }

    T & operator*()
    {
        T * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != nullptr, "store::PageHolder<T>::operator*(): Null pointer");
        return (*pImpl);
    }

    T const & operator*() const
    {
        T const * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != nullptr, "store::PageHolder<T>::operator*(): Null pointer");
        return (*pImpl);
    }

    static storeError guard (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nAddr)
    {
        PageData * pHead = rxPage.get();
        if (!pHead)
            return store_E_InvalidAccess;
        pHead->guard(nAddr);

        T * pImpl = dynamic_page_cast<T>(pHead);
        OSL_PRECOND(pImpl != nullptr, "store::PageHolder<T>::guard(): Null pointer");
        pImpl->guard();

        return store_E_None;
    }

    static storeError verify (std::shared_ptr<PageData> const & rxPage, sal_uInt32 nAddr)
    {
        PageData const * pHead = rxPage.get();
        if (!pHead)
            return store_E_InvalidAccess;

        storeError eErrCode = pHead->verify(nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        T const * pImpl = dynamic_page_cast<T>(pHead);
        if (!pImpl)
            return store_E_WrongVersion;

        return pImpl->verify();
    }
};

class OStorePageObject
{
    typedef PageData       page;

public:
    /** Allocation.
     */
    static void * operator new (size_t n)
    {
        return std::malloc(sal_uInt32(n));
    }
    static void operator delete (void * p)
    {
        std::free (p);
    }

    /** State.
     */
    inline bool dirty() const;
    inline void clean();
    inline void touch();

    /** Location.
     */
    inline sal_uInt32 location() const;

protected:
    /** Representation.
     */
    std::shared_ptr<PageData> m_xPage;
    bool       m_bDirty;

    /** Construction.
     */
    explicit OStorePageObject (std::shared_ptr<PageData> const & rxPage)
        : m_xPage (rxPage), m_bDirty (false)
    {}

    /** Destruction.
     */
    virtual ~OStorePageObject();

public:
    template< class U >
    PageHolderObject<U> makeHolder() const
    {
        return PageHolderObject<U>(m_xPage);
    }

    template< class U >
    storeError construct (rtl::Reference< PageData::Allocator > const & rxAllocator)
    {
        if (!rxAllocator.is())
            return store_E_InvalidAccess;

        std::shared_ptr<PageData> tmp (rxAllocator->construct<U>(), PageData::Deallocate(rxAllocator));
        if (!tmp.get())
            return store_E_OutOfMemory;

        m_xPage.swap (tmp);
        return store_E_None;
    }

    std::shared_ptr<PageData> & get() { return m_xPage; }

    virtual storeError guard  (sal_uInt32 nAddr) = 0;
    virtual storeError verify (sal_uInt32 nAddr) const = 0;
};

inline bool OStorePageObject::dirty() const
{
    return m_bDirty;
}

inline void OStorePageObject::clean()
{
    m_bDirty = false;
}

inline void OStorePageObject::touch()
{
    m_bDirty = true;
}

inline sal_uInt32 OStorePageObject::location() const
{
    return m_xPage->location();
}

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
