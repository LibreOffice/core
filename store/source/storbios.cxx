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

#include <sal/config.h>

#include "storbios.hxx"

#include <sal/types.h>
#include <sal/log.hxx>

#include <rtl/alloc.h>
#include <rtl/ref.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <store/types.h>
#include "lockbyte.hxx"
#include "storcach.hxx"

using namespace store;

/*========================================================================
 *
 * OStoreSuperBlock.
 *
 *======================================================================*/
#define STORE_MAGIC_SUPERBLOCK sal_uInt32(0x484D5343)

struct OStoreSuperBlock
{
    typedef OStorePageGuard      G;
    typedef OStorePageDescriptor D;
    typedef OStorePageLink       L;

    /** Representation.
     */
    G          m_aGuard;
    D          m_aDescr;
    sal_uInt32 m_nMarked;
    L          m_aMarked;
    sal_uInt32 m_nUnused;
    L          m_aUnused;

    /** theSize.
     */
    static const size_t theSize = sizeof(G) + sizeof(D) + 2 * (sizeof(L) + sizeof(sal_uInt32));

    /** Construction.
     */
    explicit OStoreSuperBlock (sal_uInt16 nPageSize)
        : m_aGuard  (STORE_MAGIC_SUPERBLOCK),
          m_aDescr  (nPageSize, nPageSize, STORE_MINIMUM_PAGESIZE),
          m_nMarked (store::htonl(0)),
          m_aMarked (0),
          m_nUnused (store::htonl(0)),
          m_aUnused (0)
    {}

    /** Comparison.
     */
    bool operator== (const OStoreSuperBlock & rhs) const
    {
        return ((m_aGuard  == rhs.m_aGuard ) &&
                (m_aDescr  == rhs.m_aDescr ) &&
                (m_nMarked == rhs.m_nMarked) &&
                (m_aMarked == rhs.m_aMarked) &&
                (m_nUnused == rhs.m_nUnused) &&
                (m_aUnused == rhs.m_aUnused)    );
    }

    /** unused(Count|Head|Insert|Remove|Reset).
     */
    sal_uInt32 unusedCount() const
    {
        return store::ntohl(m_nUnused);
    }
    const L& unusedHead() const
    {
        return m_aUnused;
    }
    void unusedInsert (const L& rLink)
    {
        sal_uInt32 nUnused = unusedCount();
        m_nUnused = store::htonl(nUnused + 1);
        m_aUnused = rLink;
    }
    void unusedRemove (const L& rLink)
    {
        sal_uInt32 nUnused = unusedCount();
        m_nUnused = store::htonl(nUnused - 1);
        m_aUnused = rLink;
    }
    void unusedReset()
    {
        m_nUnused = store::htonl(0);
        m_aUnused = L(0);
    }

    /** guard (external representation).
     */
    void guard()
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, static_cast<sal_uInt32>(theSize - sizeof(G)));
        m_aGuard.m_nCRC32 = store::htonl(nCRC32);
    }

    /** verify (external representation).
     */
    storeError verify() const
    {
        sal_uInt32 nMagic = store::ntohl(m_aGuard.m_nMagic);
        if (nMagic != STORE_MAGIC_SUPERBLOCK)
            return store_E_WrongFormat;

        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, static_cast<sal_uInt32>(theSize - sizeof(G)));
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * SuperBlockPage interface.
 *
 *======================================================================*/
namespace store
{

struct SuperBlockPage
{
    typedef OStoreSuperBlock SuperBlock;

    /** Representation.
     */
    SuperBlock m_aSuperOne;
    SuperBlock m_aSuperTwo;

    /** theSize.
     */
    static const size_t     theSize     = 2 * SuperBlock::theSize;
    static const sal_uInt16 thePageSize = theSize;
    static_assert(STORE_MINIMUM_PAGESIZE >= thePageSize, "must be at least thePageSize");

    /** Allocation.
     */
    static void * operator new (size_t n)
    {
        return std::malloc(sal::static_int_cast<sal_Size>(n));
    }
    static void operator delete (void * p)
    {
        std::free (p);
    }

    static void * operator new (SAL_UNUSED_PARAMETER size_t, sal_uInt16 nPageSize)
    {
        return rtl_allocateZeroMemory (sal::static_int_cast<sal_Size>(nPageSize));
    }
    static void operator delete (void * p, SAL_UNUSED_PARAMETER sal_uInt16)
    {
        std::free (p);
    }

    /** Construction.
     */
    explicit SuperBlockPage (sal_uInt16 nPageSize = thePageSize)
        : m_aSuperOne(nPageSize),
          m_aSuperTwo(nPageSize)
    {}

    /** save.
     */
    storeError save (OStorePageBIOS const & rBIOS, sal_uInt32 nSize = theSize)
    {
        m_aSuperOne.guard();
        m_aSuperTwo = m_aSuperOne;
        return rBIOS.write (0, this, nSize);
    }

    /** Page allocation.
     */
    storeError unusedHead (
        OStorePageBIOS const & rBIOS,
        PageData &       rPageHead);

    storeError unusedPop (
        OStorePageBIOS const & rBIOS,
        PageData const & rPageHead);

    storeError unusedPush (
        OStorePageBIOS const & rBIOS,
        sal_uInt32       nAddr);

    /** verify (with repair).
     */
    storeError verify (OStorePageBIOS const & rBIOS);
};

} // namespace store

/*========================================================================
 *
 * SuperBlockPage implementation.
 *
 *======================================================================*/
/*
 * unusedHead(): get freelist head (alloc page, step 1).
 */
storeError SuperBlockPage::unusedHead (OStorePageBIOS const & rBIOS, PageData & rPageHead)
{
    storeError eErrCode = verify (rBIOS);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check freelist head.
    OStorePageLink const aListHead (m_aSuperOne.unusedHead());
    if (aListHead.location() == 0)
    {
        // Freelist empty, see SuperBlock::ctor().
        rPageHead.location (STORE_PAGE_NULL);
        return store_E_None;
    }

    // Load PageHead.
    eErrCode = rBIOS.read (aListHead.location(), &rPageHead, PageData::theSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = rPageHead.verify (aListHead.location());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Verify page is unused.
    sal_uInt32 const nAddr = rPageHead.m_aUnused.location();
    if (nAddr == STORE_PAGE_NULL)
    {
        SAL_WARN("store", "store::SuperBlock::unusedHead(): page not free");

        // Page in use.
        rPageHead.location (STORE_PAGE_NULL);

        // Recovery: Reset freelist to empty.
        m_aSuperOne.unusedReset();
        eErrCode = save (rBIOS);
    }
    return eErrCode;
}

/*
 * unusedPop(): pop freelist head (alloc page, step 2).
 */
storeError SuperBlockPage::unusedPop (OStorePageBIOS const & rBIOS, PageData const & rPageHead)
{
    sal_uInt32 const nAddr = rPageHead.m_aUnused.location();
    OSL_PRECOND(nAddr != STORE_PAGE_NULL, "store::SuperBlock::unusedPop(): page not free");
    if (nAddr == STORE_PAGE_NULL)
        return store_E_CantSeek;

    // Pop from FreeList.
    OStorePageLink const aListHead (nAddr);
    m_aSuperOne.unusedRemove (aListHead);
    return save (rBIOS);
}

/*
 * unusedPush(): push new freelist head.
 */
storeError SuperBlockPage::unusedPush (OStorePageBIOS const & rBIOS, sal_uInt32 nAddr)
{
    storeError eErrCode = verify (rBIOS);
    if (eErrCode != store_E_None)
        return eErrCode;

    PageData aPageHead;
    eErrCode = rBIOS.read (nAddr, &aPageHead, PageData::theSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = aPageHead.verify (nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    aPageHead.m_aUnused = m_aSuperOne.unusedHead();
    aPageHead.guard (nAddr);

    eErrCode = rBIOS.write (nAddr, &aPageHead, PageData::theSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    OStorePageLink const aListHead (nAddr);
    m_aSuperOne.unusedInsert(aListHead);
    return save (rBIOS);
}

/*
 * verify (with repair).
 */
storeError SuperBlockPage::verify (OStorePageBIOS const & rBIOS)
{
    // Verify 1st copy.
    storeError eErrCode = m_aSuperOne.verify();
    if (eErrCode == store_E_None)
    {
        // Ok. Verify 2nd copy.
        eErrCode = m_aSuperTwo.verify();
        if (eErrCode == store_E_None)
        {
            // Ok. Ensure identical copies (1st copy wins).
            if (!(m_aSuperOne == m_aSuperTwo))
            {
                // Different. Replace 2nd copy with 1st copy.
                m_aSuperTwo = m_aSuperOne;

                // Write back.
                if (rBIOS.isWriteable())
                    eErrCode = rBIOS.write (0, this, theSize);
                else
                    eErrCode = store_E_None;
            }
        }
        else
        {
            // Failure. Replace 2nd copy with 1st copy.
            m_aSuperTwo = m_aSuperOne;

            // Write back.
            if (rBIOS.isWriteable())
                eErrCode = rBIOS.write (0, this, theSize);
            else
                eErrCode = store_E_None;
        }
    }
    else
    {
        // Failure. Verify 2nd copy.
        eErrCode = m_aSuperTwo.verify();
        if (eErrCode == store_E_None)
        {
            // Ok. Replace 1st copy with 2nd copy.
            m_aSuperOne = m_aSuperTwo;

            // Write back.
            if (rBIOS.isWriteable())
                eErrCode = rBIOS.write (0, this, theSize);
            else
                eErrCode = store_E_None;
        }
        else
        {
            // Double Failure.
            SAL_WARN("store", "OStoreSuperBlockPage::verify(): double failure.");
        }
    }

    // Done.
    return eErrCode;
}

/*========================================================================
 *
 * OStorePageBIOS::Ace implementation.
 *
 *======================================================================*/
OStorePageBIOS::Ace::Ace()
  : m_next (this), m_prev (this), m_addr (STORE_PAGE_NULL), m_used (0)
{}

OStorePageBIOS::Ace::~Ace()
{
  m_next->m_prev = m_prev;
  m_prev->m_next = m_next;
}

int
SAL_CALL OStorePageBIOS::Ace::constructor (
    void * obj, SAL_UNUSED_PARAMETER void * /* arg */)
{
  Ace * ace = static_cast<Ace*>(obj);
  ace->m_next = ace->m_prev = ace;
  return 1;
}

OStorePageBIOS::Ace *
OStorePageBIOS::Ace::find (OStorePageBIOS::Ace * head, sal_uInt32 addr)
{
  OStorePageBIOS::Ace * entry;
  for (entry = head->m_next; entry != head; entry = entry->m_next)
  {
    if (entry->m_addr >= addr)
      return entry;
  }
  return head;
}

void
OStorePageBIOS::Ace::insert (OStorePageBIOS::Ace * head, OStorePageBIOS::Ace * entry)
{
  // insert entry at queue tail (before head).
  entry->m_next = head;
  entry->m_prev = head->m_prev;
  head->m_prev = entry;
  entry->m_prev->m_next = entry;
}

/*========================================================================
 *
 * OStorePageBIOS::AceCache interface.
 *
 *======================================================================*/
namespace store
{

class OStorePageBIOS::AceCache
{
  rtl_cache_type * m_ace_cache;

public:
  static AceCache & get();

  OStorePageBIOS::Ace *
  create (sal_uInt32 addr);

  void
  destroy (OStorePageBIOS::Ace * ace);

protected:
  AceCache();
  ~AceCache();
};

} // namespace store

/*========================================================================
 *
 * OStorePageBIOS::AceCache implementation.
 *
 *======================================================================*/

OStorePageBIOS::AceCache &
OStorePageBIOS::AceCache::get()
{
  static AceCache g_ace_cache;
  return g_ace_cache;
}

OStorePageBIOS::AceCache::AceCache()
{
  m_ace_cache = rtl_cache_create (
    "store_ace_cache",
    sizeof (OStorePageBIOS::Ace),
    0, // objalign
    OStorePageBIOS::Ace::constructor,
    nullptr, // destructor,
    nullptr, // reclaim,
    nullptr, // userarg,
    nullptr, // default source,
    0  // flags
    );
}

OStorePageBIOS::AceCache::~AceCache()
{
  rtl_cache_destroy (m_ace_cache);
  m_ace_cache = nullptr;
}

OStorePageBIOS::Ace *
OStorePageBIOS::AceCache::create (sal_uInt32 addr)
{
  Ace * ace = static_cast<Ace*>(rtl_cache_alloc (m_ace_cache));
  if (ace != nullptr)
  {
    // verify invariant state.
    OSL_ASSERT((ace->m_next == ace) && (ace->m_prev == ace));

    // initialize.
    ace->m_addr = addr;
    ace->m_used = 1;
  }
  return ace;
}

void
OStorePageBIOS::AceCache::destroy (OStorePageBIOS::Ace * ace)
{
  if (ace != nullptr)
  {
    // remove from queue (if any).
    ace->m_next->m_prev = ace->m_prev;
    ace->m_prev->m_next = ace->m_next;

    // restore invariant state.
    ace->m_next = ace->m_prev = ace;

    // return to cache.
    rtl_cache_free (m_ace_cache, ace);
  }
}

/*========================================================================
 *
 * OStorePageBIOS implementation.
 *
 *======================================================================*/
/*
 * OStorePageBIOS.
 */
OStorePageBIOS::OStorePageBIOS()
    : m_bWriteable (false)
{
}

/*
 * ~OStorePageBIOS.
 */
OStorePageBIOS::~OStorePageBIOS()
{
    cleanup_Impl();
}

/*
 * initialize.
 * Precond: none.
 */
storeError OStorePageBIOS::initialize (
    ILockBytes *    pLockBytes,
    storeAccessMode eAccessMode,
    sal_uInt16 &    rnPageSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Initialize.
    storeError eErrCode = initialize_Impl (pLockBytes, eAccessMode, rnPageSize);
    if (eErrCode != store_E_None)
    {
        // Cleanup.
        cleanup_Impl();
    }
    return eErrCode;
}

/*
 * initialize_Impl.
 * Internal: Precond: exclusive access.
 */
storeError OStorePageBIOS::initialize_Impl (
    ILockBytes *    pLockBytes,
    storeAccessMode eAccessMode,
    sal_uInt16 &    rnPageSize)
{
    // Cleanup.
    cleanup_Impl();

    // Initialize.
    m_xLockBytes = pLockBytes;
    if (!m_xLockBytes.is())
        return store_E_InvalidParameter;
    m_bWriteable = (eAccessMode != storeAccessMode::ReadOnly);

    // Check access mode.
    storeError eErrCode = store_E_None;
    if (eAccessMode != storeAccessMode::Create)
    {
        // Load SuperBlock page.
        m_pSuper.reset(new SuperBlockPage());

        eErrCode = read (0, m_pSuper.get(), SuperBlockPage::theSize);
        if (eErrCode == store_E_None)
        {
            // Verify SuperBlock page (with repair).
            eErrCode = m_pSuper->verify (*this);
        }
    }
    else
    {
        // Truncate to zero length.
        eErrCode = m_xLockBytes->setSize(0);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Mark as not existing.
        eErrCode = store_E_NotExists;
    }

    if (eErrCode != store_E_None)
    {
        // Check reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        // Check mode.
        if (eAccessMode == storeAccessMode::ReadOnly)
            return store_E_NotExists;
        if (eAccessMode == storeAccessMode::ReadWrite)
            return store_E_NotExists;

        // Check PageSize.
        if ((STORE_MINIMUM_PAGESIZE > rnPageSize) || (rnPageSize > STORE_MAXIMUM_PAGESIZE))
            return store_E_InvalidParameter;
        rnPageSize = ((rnPageSize + STORE_MINIMUM_PAGESIZE - 1) & ~(STORE_MINIMUM_PAGESIZE - 1));

        // Create initial page (w/ SuperBlock).
        m_pSuper.reset(new(rnPageSize) SuperBlockPage(rnPageSize));
        eErrCode = m_pSuper->save (*this, rnPageSize);
    }
    if (eErrCode == store_E_None)
    {
        // Obtain page size.
        rnPageSize = store::ntohs(m_pSuper->m_aSuperOne.m_aDescr.m_nSize);

        // Create page allocator.
        eErrCode = m_xLockBytes->initialize (m_xAllocator, rnPageSize);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Create page cache.
        eErrCode = PageCache_createInstance (m_xCache, rnPageSize);
    }
    return eErrCode;
}

/*
 * cleanup_Impl.
 * Internal: Precond: exclusive access.
 */
void OStorePageBIOS::cleanup_Impl()
{
    // Check referer count.
    if (m_ace_head.m_used > 0)
    {
        // Report remaining referer count.
        SAL_INFO("store", "referer count: " << m_ace_head.m_used);
        for (Ace * ace = m_ace_head.m_next; ace != &m_ace_head; ace = m_ace_head.m_next)
        {
            m_ace_head.m_used -= ace->m_used;
            AceCache::get().destroy (ace);
        }
        OSL_ENSURE(m_ace_head.m_used == 0, "store::PageBIOS::cleanup_Impl(): logic error");
    }

    // Release SuperBlock page.
    m_pSuper.reset();

    // Release PageCache.
    m_xCache.clear();

    // Release PageAllocator.
    m_xAllocator.clear();

    // Release LockBytes.
    m_xLockBytes.clear();
}

/*
 * read.
 * Low Level: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::read (
    sal_uInt32 nAddr, void *pData, sal_uInt32 nSize) const
{
    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Read Data.
    return m_xLockBytes->readAt (nAddr, pData, nSize);
}

/*
 * write.
 * Low Level: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageBIOS::write (
    sal_uInt32 nAddr, const void *pData, sal_uInt32 nSize) const
{
    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Write Data.
    return m_xLockBytes->writeAt (nAddr, pData, nSize);
}

/*
 * acquirePage.
 * Precond: initialized.
 */
storeError OStorePageBIOS::acquirePage (
    const OStorePageDescriptor& rDescr, storeAccessMode eMode)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Check access mode.
    if (!(m_bWriteable || (eMode == storeAccessMode::ReadOnly)))
        return store_E_AccessViolation;

    // Find access control list entry.
    Ace * ace = Ace::find (&m_ace_head, rDescr.m_nAddr);
    if (ace->m_addr == rDescr.m_nAddr)
    {
      // Acquire existing entry (with ShareDenyWrite).
      if (eMode == storeAccessMode::ReadOnly)
        ace->m_used += 1;
      else
        return store_E_AccessViolation;
    }
    else
    {
      // Insert new entry.
      Ace * entry = AceCache::get().create (rDescr.m_nAddr);
      if (!entry)
        return store_E_OutOfMemory;
      Ace::insert (ace, entry);
    }

    // Increment total referer count and finish.
    m_ace_head.m_used += 1;
    return store_E_None;
}

/*
 * releasePage.
 * Precond: initialized.
 */
storeError OStorePageBIOS::releasePage (const OStorePageDescriptor& rDescr)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Find access control list entry.
    Ace * ace = Ace::find (&m_ace_head, rDescr.m_nAddr);
    if (ace->m_addr != rDescr.m_nAddr)
      return store_E_NotExists;

    // Release existing entry.
    if (ace->m_used > 1)
      ace->m_used -= 1;
    else
      AceCache::get().destroy (ace);

    // Decrement total referer count and finish.
    m_ace_head.m_used -= 1;
    return store_E_None;
}

/*
 * allocate.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::allocate (
    OStorePageObject& rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Check allocation type.
    storeError eErrCode = store_E_None;
    // Try freelist head.
    PageData aPageHead;
    eErrCode = m_pSuper->unusedHead (*this, aPageHead);
    if (eErrCode != store_E_None)
        return eErrCode;

    sal_uInt32 const nAddr = aPageHead.location();
    if (nAddr != STORE_PAGE_NULL)
    {
        // Save page.
        eErrCode = saveObjectAt_Impl (rPage, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Pop freelist head and finish.
        return m_pSuper->unusedPop (*this, aPageHead);
    }

    // Allocate from EOF. Determine current size.
    sal_uInt32 nSize = STORE_PAGE_NULL;
    eErrCode = m_xLockBytes->getSize (nSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Save page at current EOF.
    return saveObjectAt_Impl (rPage, nSize);
}

/*
 * free.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::free (sal_uInt32 nAddr)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Invalidate cache.
    (void) m_xCache->removePageAt (nAddr);

    // Push onto freelist.
    return m_pSuper->unusedPush (*this, nAddr);
}

/*
 * loadObjectAt.
 * Precond: initialized, readable.
 */
storeError OStorePageBIOS::loadObjectAt (OStorePageObject & rPage, sal_uInt32 nAddr)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    return loadObjectAt_Impl (rPage, nAddr);
}

/*
 * loadObjectAt_Impl.
 * Internal: Precond: initialized, readable, exclusive access.
 */
storeError OStorePageBIOS::loadObjectAt_Impl (OStorePageObject & rPage, sal_uInt32 nAddr) const
{
    storeError eErrCode = m_xCache->lookupPageAt (rPage.get(), nAddr);
    if (eErrCode != store_E_NotExists)
        return eErrCode;

    // Read page.
    eErrCode = m_xLockBytes->readPageAt (rPage.get(), nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Verify page.
    eErrCode = rPage.verify (nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Mark page as clean.
    rPage.clean();

    // Cache page.
    return m_xCache->insertPageAt (rPage.get(), nAddr);
}

/*
 * saveObjectAt.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::saveObjectAt (OStorePageObject & rPage, sal_uInt32 nAddr)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Save Page.
    return saveObjectAt_Impl (rPage, nAddr);
}

/*
 * saveObjectAt_Impl.
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageBIOS::saveObjectAt_Impl (OStorePageObject & rPage, sal_uInt32 nAddr) const
{
    // Guard page (incl. set location).
    storeError eErrCode = rPage.guard (nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Write page.
    eErrCode = m_xLockBytes->writePageAt(rPage.get(), nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Mark page as clean.
    rPage.clean();

    // Cache page.
    return m_xCache->updatePageAt (rPage.get(), nAddr);
}

/*
 * close.
 * Precond: none.
 */
storeError OStorePageBIOS::close()
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Cleanup.
    cleanup_Impl();

    // Done.
    return store_E_None;
}

/*
 * flush.
 * Precond: initialized.
 */
storeError OStorePageBIOS::flush()
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Flush LockBytes and finish.
    return m_xLockBytes->flush();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
