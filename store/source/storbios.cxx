/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_store.hxx"

#include "storbios.hxx"

#include "sal/types.h"
#include "sal/macros.h"

#include "rtl/alloc.h"
#include "rtl/ref.hxx"

#include "osl/diagnose.h"
#include "osl/mutex.hxx"

#include "store/types.h"
#include "object.hxx"
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

    OStoreSuperBlock (const OStoreSuperBlock & rhs)
        : m_aGuard  (rhs.m_aGuard),
          m_aDescr  (rhs.m_aDescr),
          m_nMarked (rhs.m_nMarked),
          m_aMarked (rhs.m_aMarked),
          m_nUnused (rhs.m_nUnused),
          m_aUnused (rhs.m_aUnused)
    {}

    OStoreSuperBlock& operator= (const OStoreSuperBlock & rhs)
    {
        m_aGuard  = rhs.m_aGuard;
        m_aDescr  = rhs.m_aDescr;
        m_nMarked = rhs.m_nMarked;
        m_aMarked = rhs.m_aMarked;
        m_nUnused = rhs.m_nUnused;
        m_aUnused = rhs.m_aUnused;
        return *this;
    }

    /** Comparison.
     */
    sal_Bool operator== (const OStoreSuperBlock & rhs) const
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
    sal_uInt32 unusedCount (void) const
    {
        return store::ntohl(m_nUnused);
    }
    const L& unusedHead (void) const
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
    void unusedReset (void)
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
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, theSize - sizeof(G));
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
        nCRC32 = rtl_crc32 (nCRC32, &m_aDescr, theSize - sizeof(G));
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * OStoreSuperBlockPage interface.
 *
 *======================================================================*/
namespace store
{

struct OStoreSuperBlockPage
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
    STORE_STATIC_ASSERT(STORE_MINIMUM_PAGESIZE >= thePageSize);

    /** Allocation.
     */
    static void * operator new (size_t n) SAL_THROW(())
    {
        return rtl_allocateMemory (sal::static_int_cast<sal_Size>(n));
    }
    static void operator delete (void * p, size_t) SAL_THROW(())
    {
        rtl_freeMemory (p);
    }

    static void * operator new (size_t, sal_uInt16 nPageSize) SAL_THROW(())
    {
        return rtl_allocateZeroMemory (sal::static_int_cast<sal_Size>(nPageSize));
    }
    static void operator delete (void * p, sal_uInt16) SAL_THROW(())
    {
        rtl_freeMemory (p);
    }

    /** Construction.
     */
    explicit OStoreSuperBlockPage (sal_uInt16 nPageSize = thePageSize)
        : m_aSuperOne(nPageSize),
          m_aSuperTwo(nPageSize)
    {}

    /** guard (external representation).
     */
    void guard()
    {
        m_aSuperOne.guard();
        m_aSuperTwo.guard();
    }

    /** save.
     */
    storeError save (OStorePageBIOS & rBIOS)
    {
        m_aSuperOne.guard();
        m_aSuperTwo = m_aSuperOne;
        return rBIOS.write (0, this, theSize);
    }

    /** verify (with repair).
     */
    storeError verify (OStorePageBIOS & rBIOS);
};

} // namespace store

/*========================================================================
 *
 * OStoreSuperBlockPage implementation.
 *
 *======================================================================*/
#if 0  /* NEW */
/**
 * alloc page, step 1: get freelist head.
 */
storeError SuperBlockPage::unusedHead(OStorePageBIOS & rBIOS, PageData & rPageHead)
{
    // Check FreeList.
    OStorePageLink const aListHead (m_aSuperOne.unusedHead());
    if (aListHead.location() == 0) // @see SuperBlock::ctor()
    {
        rPageHead.location (STORE_PAGE_NULL);
        return store_E_None;
    }

    // Load PageHead.
    eErrCode = rBIOS.read (aListHead.location(), &rPageHead, PageData::theSize);

    eErrCode = rPageHead.verify (aListHead.location());

    // Verify page is unused.
    sal_uInt32 const nAddr = rPageHead.m_aUnused.location();
    OSL_POSTCOND(nAddr != STORE_PAGE_NULL, "store::SuperBlock::unusedHead(): page not free");
    if (nAddr == STORE_PAGE_NULL)
    {
        // Page in use.
        rPageHead.location (STORE_PAGE_NULL);

        // Recovery: Reset FreeList.
        m_aSuperOne.unusedReset();
        return save (rBIOS);
    }
    return store_E_None;
}

/**
 * alloc page, step 2: pop freelist head.
 */
SuperBlockPage::unusedPop (OStorePageBIOS & rBIOS, PageData const & rPageHead)
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

storeError OStoreSuperBlockPage::unusedPush (OStorePageBIOS & rBIOS, sal_uInt32 nAddr)
{
    PageData aPageHead (PageData::theSize);
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
#endif /* NEW */

/*
 * verify (with repair).
 */
storeError OStoreSuperBlockPage::verify (OStorePageBIOS &rBIOS)
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
            OSL_TRACE("OStoreSuperBlockPage::verify(): double failure.\n");
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
  m_next->m_prev = m_prev, m_prev->m_next = m_next;
}

int
SAL_CALL OStorePageBIOS::Ace::constructor (void * obj, void * /* arg */)
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
  create (sal_uInt32 addr, sal_uInt32 used = 1);

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
extern "C"  typedef  int (SAL_CALL * ace_constructor_type)(void*,void*);

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
   reinterpret_cast<ace_constructor_type>( OStorePageBIOS::Ace::constructor),
    0, // destructor,
    0, // reclaim,
    0, // userarg,
    0, // default source,
    0  // flags
    );
}

OStorePageBIOS::AceCache::~AceCache()
{
  rtl_cache_destroy (m_ace_cache), m_ace_cache = 0;
}

OStorePageBIOS::Ace *
OStorePageBIOS::AceCache::create (sal_uInt32 addr, sal_uInt32 used)
{
  Ace * ace = static_cast<Ace*>(rtl_cache_alloc (m_ace_cache));
  if (ace != 0)
  {
    // verify invariant state.
    OSL_ASSERT((ace->m_next == ace) && (ace->m_prev == ace));

    // initialize.
    ace->m_addr = addr;
    ace->m_used = used;
  }
  return ace;
}

void
OStorePageBIOS::AceCache::destroy (OStorePageBIOS::Ace * ace)
{
  if (ace != 0)
  {
    // remove from queue (if any).
    ace->m_next->m_prev = ace->m_prev, ace->m_prev->m_next = ace->m_next;

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
OStorePageBIOS::OStorePageBIOS (void)
    : m_xLockBytes (NULL),
      m_pSuper     (NULL),
      m_bWriteable (false)
{
}

/*
 * ~OStorePageBIOS.
 */
OStorePageBIOS::~OStorePageBIOS (void)
{
    OStorePageBIOS::close();
}

/*
 * verify (SuperBlock with repair).
 * Internal: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::verify (SuperPage *&rpSuper)
{
    // Check SuperBlock page allocation.
    if (rpSuper == 0)
    {
        // Allocate.
        if ((rpSuper = new SuperPage()) == 0)
            return store_E_OutOfMemory;

        // Load (w/o verification).
        storeError eErrCode = read (0, rpSuper, SuperPage::theSize);
        if (eErrCode != store_E_None)
        {
            // Cleanup and fail.
            delete rpSuper, rpSuper = 0;
            return eErrCode;
        }
    }

    // Verify SuperBlock page (with repair).
    return rpSuper->verify (*this);
}

/*
 * create (SuperBlock).
 * Internal: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::create (sal_uInt16 nPageSize)
{
    // Check (internal) precond.
    OSL_PRECOND(m_xLockBytes.is(), "store::PageBIOS::create(): contract violation");

    // Check PageSize.
    if ((STORE_MINIMUM_PAGESIZE > nPageSize) || (nPageSize > STORE_MAXIMUM_PAGESIZE))
        return store_E_InvalidParameter;
    nPageSize = ((nPageSize + STORE_MINIMUM_PAGESIZE - 1) & ~(STORE_MINIMUM_PAGESIZE - 1));

    // Allocate SuperBlock page.
    delete m_pSuper, m_pSuper = 0;
    if ((m_pSuper = new(nPageSize) SuperPage(nPageSize)) == 0)
        return store_E_OutOfMemory;
    m_pSuper->guard();

    // Create initial page (w/ SuperBlock).
    return write (0, m_pSuper, nPageSize);
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

    // Check arguments.
    storeError eErrCode = store_E_InvalidParameter;
    if (!pLockBytes)
        return eErrCode;

    // Cleanup.
#if 0  /* OLD */
    __STORE_DELETEZ (m_pAcl); /* @@@ */
#endif /* OLD */
    delete m_pSuper, m_pSuper = 0;

    // Initialize.
    m_xLockBytes = pLockBytes;
    m_bWriteable = (!(eAccessMode == store_AccessReadOnly));

    // Check access mode.
    if (eAccessMode != store_AccessCreate)
    {
        // Verify (w/ repair) SuperBlock page.
        eErrCode = verify (m_pSuper);
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
        if (eAccessMode == store_AccessReadOnly)
            return store_E_NotExists;
        if (eAccessMode == store_AccessReadWrite)
            return store_E_NotExists;

        // Create SuperBlock page.
        eErrCode = create (rnPageSize);
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
 * read.
 * Low Level: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::read (
    sal_uInt32 nAddr, void *pData, sal_uInt32 nSize)
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
    sal_uInt32 nAddr, const void *pData, sal_uInt32 nSize)
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
    if (!(m_bWriteable || (eMode == store_AccessReadOnly)))
        return store_E_AccessViolation;

    // Find access control list entry.
    Ace * ace = Ace::find (&m_ace_head, rDescr.m_nAddr);
    if (ace->m_addr == rDescr.m_nAddr)
    {
      // Acquire existing entry (with ShareDenyWrite).
      if (eMode == store_AccessReadOnly)
        ace->m_used += 1;
      else
        return store_E_AccessViolation;
    }
    else
    {
      // Insert new entry.
      Ace * entry = AceCache::get().create (rDescr.m_nAddr, 1);
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
storeError OStorePageBIOS::releasePage (
    const OStorePageDescriptor& rDescr, storeAccessMode /* eMode */)
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
 * getRefererCount.
 * Precond: none.
 */
sal_uInt32 OStorePageBIOS::getRefererCount (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Obtain total referer count.
    return m_ace_head.m_used;
}

/*
 * allocate.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::allocate (
    OStorePageObject& rPage, Allocation eAlloc)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Load SuperBlock and require good health.
    storeError eErrCode = verify (m_pSuper);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check allocation.
    if (eAlloc != ALLOCATE_EOF)
    {
        // Check FreeList.
        OStorePageLink aListHead (m_pSuper->m_aSuperOne.unusedHead());
        if (aListHead.location())
        {
            // Allocate from FreeList.
            OStorePageData aPageHead (OStorePageData::theSize);

            // Load PageHead.
            eErrCode = peek (aPageHead, aListHead.location());
            if (eErrCode != store_E_None)
                return eErrCode;

            // Verify FreeList head.
            OSL_PRECOND(
                aPageHead.m_aUnused.m_nAddr != STORE_PAGE_NULL,
                "OStorePageBIOS::allocate(): page not free");
            if (aPageHead.m_aUnused.location() == STORE_PAGE_NULL)
            {
                // Recovery: Reset FreeList.
                m_pSuper->m_aSuperOne.unusedReset();

                // Save SuperBlock page.
                eErrCode = m_pSuper->save (*this);

                // Recovery: Allocate from EOF.
                if (eErrCode == store_E_None)
                    return allocate (rPage, ALLOCATE_EOF);
                else
                    return store_E_Unknown;
            }

            // Pop from FreeList.
            aListHead = aPageHead.m_aUnused.location();
            rPage.get()->m_aUnused = STORE_PAGE_NULL;

            // Save page at PageHead location.
            eErrCode = saveObjectAt_Impl (rPage, aPageHead.location());
            if (eErrCode != store_E_None)
                return eErrCode;

            // Save SuperBlock page and finish.
            m_pSuper->m_aSuperOne.unusedRemove (aListHead);

            eErrCode = m_pSuper->save (*this);
            OSL_POSTCOND(
                eErrCode == store_E_None,
                "OStorePageBIOS::allocate(): SuperBlock save failed");
            return eErrCode;
        }
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
storeError OStorePageBIOS::free (OStorePageData & /* rData */, sal_uInt32 nAddr)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Load SuperBlock and require good health.
    storeError eErrCode = verify (m_pSuper);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Load PageHead.
    OStorePageData aPageHead(OStorePageData::theSize);
    eErrCode = peek (aPageHead, nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Invalidate cache.
    (void) m_xCache->removePageAt (nAddr);

    // Push onto FreeList.
    // return m_pSuper->unusedPush (*this, nAddr); // @@@ NEW @@@

    OStorePageLink aListHead (m_pSuper->m_aSuperOne.unusedHead());

    aPageHead.m_aUnused.m_nAddr = aListHead.m_nAddr;
    aListHead.m_nAddr = aPageHead.m_aDescr.m_nAddr;

    // Save PageHead.
    eErrCode = poke (aPageHead, nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Save SuperBlock page and finish.
    m_pSuper->m_aSuperOne.unusedInsert (aListHead);

    eErrCode = m_pSuper->save (*this);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::free(): SuperBlock save failed");
    return eErrCode;
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
storeError OStorePageBIOS::loadObjectAt_Impl (OStorePageObject & rPage, sal_uInt32 nAddr)
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
storeError OStorePageBIOS::saveObjectAt_Impl (OStorePageObject & rPage, sal_uInt32 nAddr)
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
storeError OStorePageBIOS::close (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check referer count.
    if (m_ace_head.m_used > 0)
    {
        // Report remaining referer count.
        OSL_TRACE("store::PageBIOS::close(): referer count: %d\n", m_ace_head.m_used);
#if 1  /* NEW */
        for (Ace * ace = m_ace_head.m_next; ace != &m_ace_head; ace = m_ace_head.m_next)
        {
            m_ace_head.m_used -= ace->m_used;
            AceCache::get().destroy (ace);
        }
        OSL_ENSURE(m_ace_head.m_used == 0, "store::PageBIOS::close(): logic error");
#endif /* NEW */
    }

    // Release SuperBlock page.
    delete m_pSuper, m_pSuper = 0;

    // Release PageCache.
    m_xCache.clear();

    // Release LockBytes.
    m_xLockBytes.clear();

    // Done.
    return store_E_None;
}

/*
 * flush.
 * Precond: initialized.
 */
storeError OStorePageBIOS::flush (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Flush LockBytes and finish.
    return m_xLockBytes->flush();
}

/*
 * size.
 * Precond: initialized.
 */
storeError OStorePageBIOS::size (sal_uInt32 &rnSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Initialize [out] param.
    rnSize = 0;

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Obtain LockBytes size.
    return m_xLockBytes->getSize (rnSize);
}

/*
 * scanBegin.
 * Precond: initialized.
 */
storeError OStorePageBIOS::scanBegin (
    ScanContext &rCtx, sal_uInt32 nMagic)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Initialize [out] param.
    rCtx.m_aDescr = OStorePageDescriptor(0, 0, 0);
    rCtx.m_nSize  = 0;
    rCtx.m_nMagic = nMagic;

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Check SuperBlock page.
    storeError eErrCode = verify (m_pSuper);
    if (eErrCode != store_E_None)
    {
        // Damaged. Determine page size (NYI).
        OSL_TRACE ("OStorePageBIOS::scanBegin(): damaged.\n");
        return eErrCode;
    }

    // Setup Context descriptor.
    rCtx.m_aDescr = m_pSuper->m_aSuperOne.m_aDescr;
    rCtx.m_aDescr.m_nSize = store::ntohs(rCtx.m_aDescr.m_nSize);
    rCtx.m_aDescr.m_nAddr = rCtx.m_aDescr.m_nSize;

    // Setup Context size.
    eErrCode = size (rCtx.m_nSize);
    if (eErrCode != store_E_None)
        rCtx.m_nSize = ((sal_uInt32)(~0));

    // Done.
    return store_E_None;
}

/*
 * scanNext.
 * Precond: initialized.
 */
storeError OStorePageBIOS::scanNext (
    ScanContext &rCtx, OStorePageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Setup PageHead.
    OStorePageData aPageHead (OStorePageData::theSize);

    // Check context.
    while (rCtx.isValid())
    {
        // Assign next location.
        sal_uInt32 nAddr = rCtx.m_aDescr.m_nAddr;
        rCtx.m_aDescr.m_nAddr += rCtx.m_aDescr.m_nSize;

        // Load PageHead.
        storeError eErrCode = peek (aPageHead, nAddr);
        if (eErrCode != store_E_None)
            continue;

        // Check PageHead Magic number.
        if (aPageHead.m_aGuard.m_nMagic != rCtx.m_nMagic)
            continue;

        // Check PageHead Unused link.
        if (aPageHead.m_aUnused.m_nAddr != STORE_PAGE_NULL)
            continue;

        // Load page.
        eErrCode = loadObjectAt_Impl (rPage, nAddr);
        if (eErrCode != store_E_None)
            continue;

        // Deliver page.
        return store_E_None;
    }

    // Done.
    return store_E_CantSeek;
}

/*
 * peek (PageHead).
 * Internal: Precond: initialized, readable, exclusive access.
 */
storeError OStorePageBIOS::peek (OStorePageData &rData, sal_uInt32 nAddr)
{
    // Read PageHead.
    storeError eErrCode = read (nAddr, &rData, OStorePageData::theSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Verify PageHead.
    return rData.verify (nAddr);
}

/*
 * poke (PageHead).
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageBIOS::poke (OStorePageData &rData, sal_uInt32 nAddr)
{
    // Guard PageHead.
    rData.guard (nAddr);

    // Write PageHead.
    return write (nAddr, &rData, OStorePageData::theSize);
}
