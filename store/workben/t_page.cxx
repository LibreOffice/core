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

#include "sal/config.h"

#include "boost/static_assert.hpp"
#include "osl/diagnose.h"
#include "rtl/alloc.h"
#include "rtl/ref.hxx"

#include "storbase.hxx"

#include "osl/file.h"
#include "rtl/ustring.hxx"

template< class T > void swap (T & lhs, T & rhs)
{
  T tmp = rhs; rhs = lhs; lhs = tmp;
}

class SharedCount
{
  long * m_pCount;

  class Allocator
  {
    rtl_cache_type * m_cache;

  public:
    static Allocator & get();

    long * alloc()
    {
      return static_cast<long*>(rtl_cache_alloc (m_cache));
    }
    void free (long * pCount)
    {
      rtl_cache_free (m_cache, pCount);
    }

  protected:
    Allocator();
    ~Allocator();
  };

public:
  SharedCount()
    : m_pCount(Allocator::get().alloc())
  {
    if (m_pCount != 0) (*m_pCount) = 1;
  }

  ~SharedCount()
  {
    if (m_pCount != 0)
    {
      long new_count = --(*m_pCount);
      if (new_count == 0)
    Allocator::get().free(m_pCount);
    }
  }

  bool operator== (long count) const
  {
    return (m_pCount != 0) ? *m_pCount == count : false;
  }

  friend void swap<> (SharedCount & lhs, SharedCount & rhs); // nothrow

  SharedCount (SharedCount const & rhs); // nothrow
  SharedCount & operator= (SharedCount const & rhs); // nothrow
};

template<>
inline void swap (SharedCount & lhs, SharedCount & rhs) // nothrow
{
    swap<long*>(lhs.m_pCount, rhs.m_pCount);
}

SharedCount::SharedCount (SharedCount const & rhs) // nothrow
    : m_pCount (rhs.m_pCount)
{
    if (m_pCount != 0) ++(*m_pCount);
}

SharedCount &
SharedCount::operator= (SharedCount const & rhs) // nothrow
{
    SharedCount tmp(rhs);
    swap<SharedCount>(tmp, *this);
    return *this;
}

SharedCount::Allocator &
SharedCount::Allocator::get()
{
    static Allocator g_aSharedCountAllocator;
    return g_aSharedCountAllocator;
}

SharedCount::Allocator::Allocator()
{
    m_cache = rtl_cache_create (
        "store_shared_count_cache",
        sizeof(long),
        0, // objalign
        0, // constructor
        0, // destructor
        0, // reclaim
        0, // userarg
        0, // default source
        0  // flags
        );
}

SharedCount::Allocator::~Allocator()
{
    rtl_cache_destroy (m_cache), m_cache = 0;
}

/*======================================================================*/

#if defined(OSL_BIGENDIAN)
#define STORE_DWORD(dword) OSL_SWAPDWORD((dword))
#else
#define STORE_DWORD(dword) (dword)
#endif

struct PageData
{
    typedef store::OStorePageGuard      G;
    typedef store::OStorePageDescriptor D;
    typedef store::OStorePageLink       L;

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
    BOOST_STATIC_ASSERT(STORE_MINIMUM_PAGESIZE >= thePageSize);

    /** type.
     */
    sal_uInt32 type() const { return m_aGuard.m_nMagic; /* @@@ */ }

    /** offset.
     */
    sal_uInt32 offset() const { return m_aDescr.m_nAddr; /* @@@ */ }
    void offset (sal_uInt32 nOffset) { m_aDescr.m_nAddr = nOffset; }

    /** size.
     */
    sal_uInt16 size() const { return m_aDescr.m_nSize; /* @@@ */ }

    /** Allocation.
     */
    class Allocator : public rtl::IReference
    {
    public:
        template< class T > T * construct()
        {
            void * page = 0; sal_uInt16 size = 0;
            if (allocate (&page, &size))
            {
                return new(page) T(size);
            }
            return 0;
        }

        virtual bool allocate (void ** ppPage, sal_uInt16 * pnSize) = 0;
        virtual void deallocate (void * pPage) = 0;
    };

    static void * operator new (size_t, void * p) { return p; }
    static void   operator delete (void *, void *) {}

    /** Construction.
     */
    explicit PageData (sal_uInt16 nPageSize = thePageSize)
        : m_aDescr (STORE_PAGE_NULL, nPageSize, thePageSize)
    {}

    /** ...
     */
    void guard()
    {}

    storeError verify() const
    {
        return store_E_None;
    }
};

class PageAllocator
{
    rtl_cache_type * m_cache;
    SharedCount      m_refcount;

public:
    PageAllocator()
        : m_cache(0), m_refcount()
    {}

    ~PageAllocator()
    {
        // NYI
        if (m_refcount == 1)
        {
        }
    }

    friend void swap<>(PageAllocator & lhs, PageAllocator & rhs);

    PageAllocator (PageAllocator const & rhs);
    PageAllocator & operator= (PageAllocator const & rhs);
};

template<>
inline void swap (PageAllocator & lhs, PageAllocator & rhs)
{
    swap<rtl_cache_type*>(lhs.m_cache, rhs.m_cache);
    swap<SharedCount>(lhs.m_refcount, rhs.m_refcount);
}

PageAllocator::PageAllocator (PageAllocator const & rhs)
    : m_cache (rhs.m_cache),
      m_refcount (rhs.m_refcount)
{
}

PageAllocator &
PageAllocator::operator= (PageAllocator const & rhs)
{
    PageAllocator tmp (rhs);
    swap<PageAllocator>(tmp, *this);
    return *this;
}

/*======================================================================*/

class PageHolder
{
    SharedCount m_refcount;
    PageData  * m_pagedata;

    typedef rtl::Reference< PageData::Allocator > allocator_type;
    allocator_type m_allocator;

public:
    explicit PageHolder (PageData * pagedata = 0, allocator_type const & allocator = allocator_type())
        : m_refcount (),
          m_pagedata (pagedata),
          m_allocator(allocator)
    {}

    ~PageHolder()
    {
        if ((m_refcount == 1) && (m_pagedata != 0) && m_allocator.is())
        {
            // free pagedata.
            m_allocator->deallocate (m_pagedata);
        }
    }

    PageData * get() { return m_pagedata; }
    PageData const * get() const { return m_pagedata; }

    PageData * operator->() { return m_pagedata; }
    PageData const * operator->() const { return m_pagedata; }

    friend void swap<> (PageHolder & lhs, PageHolder & rhs); // nothrow

    PageHolder (PageHolder const & rhs); // nothrow
    PageHolder & operator= (PageHolder const & rhs); // nothrow
};

template<>
inline void swap (PageHolder & lhs, PageHolder & rhs) // nothrow
{
    swap<SharedCount>(lhs.m_refcount, rhs.m_refcount);
    swap<PageData*>(lhs.m_pagedata, rhs.m_pagedata);
    swap<PageHolder::allocator_type>(lhs.m_allocator, rhs.m_allocator);
}

PageHolder::PageHolder (PageHolder const & rhs) // nothrow
    : m_refcount (rhs.m_refcount),
      m_pagedata (rhs.m_pagedata),
      m_allocator(rhs.m_allocator)
{}

PageHolder &
PageHolder::operator= (PageHolder const & rhs) // nothrow
{
    PageHolder tmp (rhs);
    swap<PageHolder>(tmp, *this);
    return *this;
}

/*======================================================================*/

template< class T >
class PageHolderObject
{
protected:
    /** Representation.
     */
    PageHolder m_xPage;

    /** Checked cast.
     */
    template< class U >
    static bool isA (PageData const * p)
    {
        return ((p != 0) && (p->type() == U::theTypeId));
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
    static PageHolderObject<T> construct (rtl::Reference< PageData::Allocator > const & rxAllocator)
    {
        PageHolderObject<T> tmp;
        if (rxAllocator.is())
        {
            PageHolder xPage (rxAllocator->construct<T>(), rxAllocator);
            store::swap<PageHolder>(tmp.m_xPage, xPage);
        }
        return tmp;
    }

    explicit PageHolderObject (PageHolder const & rxPage = PageHolder())
        : m_xPage (rxPage)
    {}

    void swap (PageHolderObject<T> & rhs)
    {
        store::swap<PageHolder>(m_xPage, rhs.m_xPage);
    }

    PageHolderObject (PageHolderObject<T> const & rhs)
        : m_xPage (rhs.m_xPage)
    {
    }

    PageHolderObject<T> & operator= (PageHolderObject<T> const & rhs)
    {
        PageHolderObject<T> tmp (rhs);
        this->swap(tmp);
        return *this;
    }

    T * operator->()
    {
        T * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "store::PageHolder<T>::operator->(): Null pointer");
        return pImpl;
    }
    T const * operator->() const
    {
        T const * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "store::PageHolder<T>::operator->(): Null pointer");
        return pImpl;
    }

    T & operator*()
    {
        T * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "store::PageHolder<T>::operator*(): Null pointer");
        return *pImpl;
    }
    T const & operator*() const
    {
        T const * pImpl = dynamic_page_cast<T>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "store::PageHolder<T>::operator*(): Null pointer");
        return *pImpl;
    }

    static storeError guard (PageHolder & rxPage)
    {
        T * pImpl = dynamic_page_cast<T>(rxPage.get());
        if (pImpl != 0)
        { pImpl->guard(); return store_E_None; }
        else if (rxPage.get() != 0)
            return store_E_WrongVersion;
        else
            return store_E_InvalidAccess;
    }
    static storeError verify (PageHolder const & rxPage)
    {
        T const * pImpl = dynamic_page_cast<T>(rxPage.get());
        if (pImpl != 0)
            return pImpl->verify();
        else if (rxPage.get() != 0)
            return store_E_WrongVersion;
        else
            return store_E_InvalidAccess;
    }
};

/*======================================================================*/

class PageObject
{
public:
    explicit PageObject (PageHolder const & rxPage = PageHolder())
        : m_xPage (rxPage)
    {}

    virtual ~PageObject();

    PageHolder & get() { return m_xPage; }
    PageHolder const & get() const { return m_xPage; }

    PageData * operator->()
    {
        PageData * pImpl = m_xPage.get();
        OSL_PRECOND(pImpl != 0, "store::PageObject::operator->(): Null pointer");
        return pImpl;
    }
    PageData & operator*()
    {
        PageData * pImpl = m_xPage.get();
        OSL_PRECOND(pImpl != 0, "store::PageObject::operator*(): Null pointer");
        return *pImpl;
    }

    virtual void guard();
    virtual storeError verify() const;

protected:
    PageHolder m_xPage;
};

PageObject::~PageObject()
{}
void PageObject::guard()
{
    PageData * p = m_xPage.get();
    p->guard();
}
storeError PageObject::verify() const
{
    PageData const * p = m_xPage.get();
    return p->verify();
}

/*======================================================================*/

template< class T >
T * dynamic_page_cast (PageData * pagedata)
{
    if ((pagedata != 0) && (pagedata->type() == T::theTypeId))
        return static_cast<T*>(pagedata);
    return 0;
}

template< class T >
T * dynamic_page_cast (PageData const * pagedata)
{
    if ((pagedata != 0) && (pagedata->type() == T::theTypeId))
        return static_cast<T*>(pagedata);
    return 0;
}

/*======================================================================*/

class TestBIOS
{
public:
    storeError loadPageAt (PageHolder & rPage, storeError (*pfnVerify)(PageHolder const &))
    {
        return (pfnVerify)(rPage);
    }

    storeError allocate (PageHolder & rxPage, ...)
    {
        // NYI: PageObject.save(nAddr, *this);
        (void)rxPage; // NYI
        return store_E_Unknown; // NYI
    }

    storeError loadAt (PageHolder & rPage, sal_uInt32 nOffset)
    {
        (void)rPage; // NYI
        (void)nOffset; // NYI
        return store_E_Unknown; // NYI
    }
    storeError saveAt (PageHolder const & rPage, sal_uInt32 nOffset)
    {
        (void)rPage; // NYI
        (void)nOffset; // NYI
        return store_E_Unknown; // NYI
    }

    template< class T >
    storeError save (PageHolder & rxPage, sal_uInt32 nOffset)
    {
        storeError result = PageHolderObject<T>::guard (rxPage);
        if (result != store_E_None)
            return result;
        return saveAt (rxPage, nOffset);
    }

    storeError lookupAt (PageHolder & rPage, sal_uInt32 nOffset)
    {
        (void)rPage; // NYI
        (void)nOffset; // NYI
        return store_E_NotExists;
    }
    storeError replaceAt (PageHolder const & rPage, sal_uInt32 nOffset)
    {
        (void)rPage; // NYI
        (void)nOffset; // NYI
        return store_E_None;
    }
};

struct TestDataV1 : public PageData
{
    static const sal_uInt32 theTypeId = 6 * 9;
};
struct TestData : public PageData
{
    typedef PageData base;
    typedef TestData self;

    static const sal_uInt32 theTypeId = 42;

    void guard()
    {
        base::guard();
        // self::m_aGuard = ...;
    }
    storeError verify() const
    {
        storeError result = base::verify();
        if (result != store_E_None)
            return result;
        if (!(base::type() == self::theTypeId))
            return store_E_WrongVersion;
        return store_E_None;
    }

    storeError dwim() const
    {
        return store_E_None;
    }
};
class TestObject : public PageObject
{
    typedef PageObject base;

public:

    void dwim()
    {
        PageHolderObject< TestData > xPage (m_xPage);
        xPage->guard();
    }

    virtual void guard()
    {
        TestData * pagedata = dynamic_page_cast< TestData >(m_xPage.get());
        if (pagedata != 0)
        {}
    }
    virtual storeError verify() const
    {
        storeError result = base::verify();
        if (result != store_E_None)
            return result;

        TestData const * pagedata = dynamic_page_cast< TestData const >(m_xPage.get());
        if (!pagedata)
            return store_E_WrongVersion;

        return pagedata->verify();
    }

    static storeError verify (PageHolder const & rPage)
    {
        return PageHolderObject< TestData >::verify (rPage);
    }

    storeError loadAt (sal_uInt32 nOffset, TestBIOS & rBIOS)
    {
        storeError result = rBIOS.lookupAt (m_xPage, nOffset); // cache lookup
        if (result == store_E_NotExists)
        {
            result = rBIOS.loadAt (m_xPage, nOffset);
            if (result != store_E_None)
                return result;

            result = PageHolderObject< TestData >::verify (m_xPage);
            if (result != store_E_None)
                return result;

            result = rBIOS.replaceAt (m_xPage, nOffset); // cache insert
        }
        return result;
    }
    storeError saveAt (sal_uInt32 nOffset, TestBIOS & rBIOS)
    {
        if (!m_xPage.get())
            return store_E_InvalidAccess;
        m_xPage->m_aDescr.m_nAddr = store::htonl(nOffset); // m_xPage->location (nOffset);

        storeError result = PageHolderObject< TestData >::guard (m_xPage);
        if (result != store_E_None)
            return result;

        result = rBIOS.saveAt (m_xPage, nOffset);
        if (result != store_E_None)
            return result;

        return rBIOS.replaceAt (m_xPage, nOffset); // cache update
    }
};

class TestObjectV2 : public PageHolderObject< TestData >
{
    typedef PageHolderObject< TestData > base;

public:
    storeError saveAt (sal_uInt32 nOffset, TestBIOS & rBIOS)
    {
        m_xPage->offset(nOffset);

        storeError result = PageHolderObject< TestData >::guard (m_xPage);
        if (result != store_E_None)
            return result;

        result = rBIOS.saveAt (m_xPage, nOffset);
        if (result != store_E_None)
            return result;

        return rBIOS.replaceAt (m_xPage, nOffset);
    }
#if 1
    storeError dwim() const
    {
        TestData const * pImpl1 = operator->();

        PageHolderObject< TestData > xImpl (m_xPage);

        TestData const * pImpl2 = &*xImpl;
        OSL_ASSERT(pImpl1 == pImpl2);

        return xImpl->dwim();
    }
#endif
};

class TestClient
{
public:
    void dwim(TestBIOS & rBIOS)
    {
        TestObject aObj;

        rBIOS.loadPageAt(aObj.get(), aObj.verify);
        rBIOS.loadPageAt(aObj.get(), TestObject::verify);
        rBIOS.loadPageAt(aObj.get(), PageHolderObject<TestData>::verify);

        aObj.loadAt (1024, rBIOS);

        TestObjectV2 aObj2;
        aObj2.dwim();
        aObj2->dwim();
    }
};

/*======================================================================*/

class IPageAccess
{
public:
    virtual storeError initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize) = 0;

public:
    storeError readPageAt (PageHolder & rPage, sal_uInt32 nOffset)
    {
        return readPageAt_Impl (rPage, nOffset);
    }
    storeError writePageAt (PageHolder const & rPage, sal_uInt32 nOffset)
    {
        // [SECURITY:ValInput]
        PageData const * pagedata = rPage.get();
        OSL_PRECOND(!(pagedata == 0), "invalid Page");
        if (pagedata == 0)
            return store_E_InvalidParameter;

        sal_uInt32 const offset = pagedata->offset();
        OSL_PRECOND(!(nOffset != offset), "inconsistent Offset");
        if (nOffset != offset)
            return store_E_InvalidParameter;

        OSL_PRECOND(!(nOffset == STORE_PAGE_NULL), "store::IPageAccess::writePageAt(): invalid Offset");
        if (nOffset == STORE_PAGE_NULL)
            return store_E_CantSeek;

        return writePageAt_Impl (rPage, nOffset);
    }

    storeError peekAt (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
    {
        // [SECURITY:ValInput]
        sal_uInt8 * dst_lo = static_cast<sal_uInt8*>(pBuffer);
        if (!(dst_lo != 0))
            return store_E_InvalidParameter;

        sal_uInt8 * dst_hi = dst_lo + nBytes;
        if (!(dst_lo < dst_hi))
            return (dst_lo > dst_hi) ? store_E_InvalidParameter : store_E_None;

        sal_uInt64 const dst_size = nOffset + nBytes;
        if (dst_size > SAL_MAX_UINT32)
            return store_E_CantSeek;

        return peekAt_Impl (nOffset, dst_lo, (dst_hi - dst_lo));
    }

    storeError pokeAt (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes)
    {
        // [SECURITY:ValInput]
        sal_uInt8 const * src_lo = static_cast<sal_uInt8 const*>(pBuffer);
        if (!(src_lo != 0))
            return store_E_InvalidParameter;

        sal_uInt8 const * src_hi = src_lo + nBytes;
        if (!(src_lo < src_hi))
            return (src_lo > src_hi) ? store_E_InvalidParameter : store_E_None;

        sal_uInt64 const dst_size = nOffset + nBytes;
        if (dst_size > SAL_MAX_UINT32)
            return store_E_CantSeek;

        return pokeAt_Impl (nOffset, src_lo, (src_hi - src_lo));
    }

    storeError getSize (sal_uInt32 & rnSize)
    {
        rnSize = 0;
        return getSize_Impl (rnSize);
    }

    storeError setSize (sal_uInt32 nSize)
    {
        return setSize_Impl (nSize);
    }

private:
    virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset) = 0;
    virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset) = 0;

    virtual storeError peekAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes) = 0;
    virtual storeError pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes) = 0;

    virtual storeError getSize_Impl (sal_uInt32 & rnSize) = 0;
    virtual storeError setSize_Impl (sal_uInt32 nSize) = 0;
};

/*======================================================================*/

template< class T > struct ResourceHolder
{
    typedef typename T::destructor_type destructor_type;

  T m_value;

  explicit ResourceHolder (T const & value = T()) : m_value (value) {}
  ~ResourceHolder() { reset(); }

  T & get() { return m_value; }
  T const & get() const { return m_value; }

  void set (T const & value) { m_value = value; }
  void reset (T const & value = T())
  {
    T tmp (m_value);
    if (tmp != value)
      destructor_type()(tmp);
    set (value);
  }
  T release()
  {
    T tmp (m_value);
    set (T());
    return tmp;
  }

  ResourceHolder (ResourceHolder & rhs)
  {
    set (rhs.release());
  }
  ResourceHolder & operator= (ResourceHolder & rhs)
  {
    reset (rhs.release());
    return *this;
  }
};

struct FileHandle
{
  oslFileHandle m_handle;

  FileHandle() : m_handle(0) {}

  operator oslFileHandle() { return m_handle; }

  bool operator != (FileHandle const & rhs)
  {
    return (m_handle != rhs.m_handle);
  }

  oslFileError initialize (rtl_uString * pFilename, sal_uInt32 nFlags)
  {
    // Verify arguments.
    if (!pFilename || !nFlags)
      return osl_File_E_INVAL;

    // Convert into FileUrl.
    OUString aFileUrl;
    if (osl_getFileURLFromSystemPath (pFilename, &(aFileUrl.pData)) != osl_File_E_None)
    {
      // Not system path. Maybe a file url, already.
      rtl_uString_assign (&(aFileUrl.pData), pFilename);
    }

    // Acquire handle.
    return osl_openFile (aFileUrl.pData, &m_handle, nFlags);
  }

  struct CloseFile
  {
    void operator()(FileHandle & rFile) const
    {
      if (rFile.m_handle != 0)
      {
    // Release handle.
    (void) osl_closeFile (rFile.m_handle);
    rFile.m_handle = 0;
      }
    }
  };
  typedef CloseFile destructor_type;
};

struct FileMapping
{
  void *     m_pAddr;
  sal_uInt64 m_uSize;

  FileMapping() : m_pAddr(0), m_uSize(0) {}

  bool operator != (FileMapping const & rhs) const
  {
    return ((m_pAddr != rhs.m_pAddr) || (m_uSize != rhs.m_uSize));
  }

  oslFileError initialize (oslFileHandle hFile)
  {
    // Determine mapping size.
    oslFileError result = osl_getFileSize (hFile, &m_uSize);
    if (result != osl_File_E_None)
      return result;
    if (m_uSize > SAL_MAX_UINT32)
      return osl_File_E_OVERFLOW;

    // Acquire mapping.
    return osl_mapFile (hFile, &m_pAddr, m_uSize, 0, 0);
  }

  struct UnmapFile
  {
    void operator ()(FileMapping & rMapping) const
    {
      if ((rMapping.m_pAddr != 0) && (rMapping.m_uSize != 0))
      {
    // Release mapping.
    (void) osl_unmapFile (rMapping.m_pAddr, rMapping.m_uSize);
    rMapping.m_pAddr = 0, rMapping.m_uSize = 0;
      }
    }
  };
  typedef UnmapFile destructor_type;
};

/*======================================================================*/

class FilePageAccess : public IPageAccess
{
  oslFileHandle m_hFile;

public:
  static storeError ERROR_FROM_NATIVE (oslFileError eErrno);
  static sal_uInt32 MODE_TO_NATIVE (storeAccessMode eMode);

public:
  explicit FilePageAccess (oslFileHandle hFile = 0) : m_hFile (hFile) {}
  virtual storeError initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize);

private:
  virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset);
  virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset);

  /* see @ OFileLockBytes */
  virtual storeError peekAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes);
  virtual storeError pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes);

  virtual storeError getSize_Impl (sal_uInt32 & rnSize);
  virtual storeError setSize_Impl (sal_uInt32 nSize);

protected:
  virtual ~FilePageAccess();

private:
  /** Not implemented.
   */
  FilePageAccess (FilePageAccess const &);
  FilePageAccess & operator= (FilePageAccess const &);
};

storeError FilePageAccess::initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize)
{
  (void) eAccessMode;     // UNUSED
  (void) nPageSize;       // UNUSED
  return store_E_Unknown; // NYI
}
FilePageAccess::~FilePageAccess()
{
  if (m_hFile != 0)
    (void) osl_closeFile (m_hFile);
}
storeError FilePageAccess::readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset)
{
  PageHolder page (0/*allocate()*/); /* @@@ construct w/ deallocator argument @@@ */
  if (!page.get())
    return store_E_OutOfMemory;

  swap<PageHolder>(page, rPage);
  return peekAt (nOffset, rPage.get(), 0/*size*/);
}
storeError FilePageAccess::writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset)
{
  return pokeAt (nOffset, rPage.get(), 0/*size*/);
}
storeError FilePageAccess::peekAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
  sal_uInt64 nDone = 0;
  oslFileError result = osl_readFileAt (m_hFile, nOffset, pBuffer, nBytes, &nDone);
  if (result != osl_File_E_None)
    return ERROR_FROM_NATIVE(result);
  if (nDone != nBytes)
    return (nDone != 0) ? store_E_CantRead : store_E_NotExists;
  return store_E_None;
}
storeError FilePageAccess::pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes)
{
  sal_uInt64 nDone = 0;
  oslFileError result = osl_writeFileAt (m_hFile, nOffset, pBuffer, nBytes, &nDone);
  if (result != osl_File_E_None)
    return ERROR_FROM_NATIVE(result);
  if (nDone != nBytes)
    return store_E_CantWrite;
  return store_E_None;
}
storeError FilePageAccess::getSize_Impl (sal_uInt32 & rnSize)
{
  sal_uInt64 uSize = 0;
  oslFileError result = osl_getFileSize (m_hFile, &uSize);
  if (result != osl_File_E_None)
    return ERROR_FROM_NATIVE(result);
  if (uSize > SAL_MAX_UINT32)
    return store_E_CantSeek;

  rnSize = sal::static_int_cast<sal_uInt32>(uSize);
  return store_E_None;
}
storeError FilePageAccess::setSize_Impl (sal_uInt32 nSize)
{
  oslFileError result = osl_setFileSize (m_hFile, nSize);
  if (result != osl_File_E_None)
    return ERROR_FROM_NATIVE(result);
  return store_E_None;
}
storeError FilePageAccess::ERROR_FROM_NATIVE (oslFileError eErrno)
{
  switch (eErrno)
  {
  case osl_File_E_None:
    return store_E_None;

  case osl_File_E_NOENT:
    return store_E_NotExists;

  case osl_File_E_ACCES:
  case osl_File_E_PERM:
    return store_E_AccessViolation;

  case osl_File_E_AGAIN:
  case osl_File_E_DEADLK:
    return store_E_LockingViolation;

  case osl_File_E_BADF:
    return store_E_InvalidHandle;

  case osl_File_E_INVAL:
    return store_E_InvalidParameter;

  case osl_File_E_NOSPC:
    return store_E_OutOfSpace;

  case osl_File_E_OVERFLOW:
    return store_E_CantSeek;

  default:
    return store_E_Unknown;
  }
}
sal_uInt32 FilePageAccess::MODE_TO_NATIVE(storeAccessMode eAccessMode)
{
  sal_uInt32 nMode = 0;
  switch (eAccessMode)
  {
  case store_AccessCreate:
  case store_AccessReadCreate:
    nMode |= osl_File_OpenFlag_Create;
    // fall through
  case store_AccessReadWrite:
    nMode |= osl_File_OpenFlag_Write;
    // fall through
  case store_AccessReadOnly:
    nMode |= osl_File_OpenFlag_Read;
    break;
  default:
    OSL_PRECOND(0, "store::FilePageAccess: unknown storeAccessMode");
  }
  return nMode;
}

/*===*/

class MemoryPageAccess : public IPageAccess
{
  /** Representation.
   */
  sal_uInt8 * m_pData;
  sal_uInt32  m_nSize;

  /** Callback function to release Representation.
   */
  typedef void (*destructor_type)(sal_uInt8 * pData, sal_uInt32 nSize);
  destructor_type m_destructor;

  /** Default destructor callback.
   */
  static void freeMemory (sal_uInt8 * pData, sal_uInt32 nSize);

public:
  MemoryPageAccess()
    : m_pData (0), m_nSize (0), m_destructor (MemoryPageAccess::freeMemory)
  {}
  MemoryPageAccess (sal_uInt8 * pData, sal_uInt32 nSize, destructor_type destructor = MemoryPageAccess::freeMemory)
    : m_pData (pData), m_nSize (nSize), m_destructor (destructor)
  {}

  virtual storeError initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize);

private:
  /** Page (size aligned) access.
   */
  virtual storeError readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset);
  virtual storeError writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset);

  /** Low level access.
   */
  virtual storeError peekAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes);
  virtual storeError pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes);

  virtual storeError getSize_Impl (sal_uInt32 & rnSize);
  virtual storeError setSize_Impl (sal_uInt32 nSize);

protected:
  virtual ~MemoryPageAccess();

private:
  /** Not implemented.
   */
  MemoryPageAccess (MemoryPageAccess const &);
  MemoryPageAccess & operator= (MemoryPageAccess const &);
};

storeError MemoryPageAccess::initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize)
{
  (void) eAccessMode;     // UNUSED
  (void) nPageSize;       // UNUSED
  return store_E_Unknown; // NYI
}
MemoryPageAccess::~MemoryPageAccess()
{
  if (m_destructor != 0)
  {
    // release resource.
    (*m_destructor)(m_pData, m_nSize);
  }
}
storeError MemoryPageAccess::readPageAt_Impl (PageHolder & rPage, sal_uInt32 nOffset)
{
    /* OSL_PRECOND(nOffset % size == 0, "Unaligned page read."); */
    PageHolder page (reinterpret_cast< PageData* >(m_pData + nOffset));
    swap<PageHolder>(page, rPage);
    return store_E_None;
}
storeError MemoryPageAccess::writePageAt_Impl (PageHolder const & rPage, sal_uInt32 nOffset)
{
    PageData const * pagedata = rPage.get();
    if (!(pagedata != 0))
        return store_E_InvalidParameter;

    return pokeAt (nOffset, pagedata, pagedata->size());
}
storeError MemoryPageAccess::peekAt_Impl (sal_uInt32 nOffset, void * pBuffer, sal_uInt32 nBytes)
{
  // [SECURITY:ValInput]
  sal_uInt8 * dst_lo = static_cast<sal_uInt8*>(pBuffer);
  if (!(dst_lo != 0))
    return store_E_InvalidParameter;

  sal_uInt8 * dst_hi = dst_lo + nBytes;
  if (!(dst_lo <= dst_hi))
    return store_E_InvalidParameter;

  // ...
  sal_uInt8 const * src_lo = m_pData + nOffset;
  if (!(src_lo <= m_pData + m_nSize))
    return store_E_CantSeek;

  sal_uInt8 const * src_hi = src_lo + nBytes;
  if (!(src_hi <= m_pData + m_nSize))
    return store_E_CantRead;

  // copy.
  memcpy (pBuffer, src_lo, (src_hi - src_lo));
  return store_E_None;
}
storeError MemoryPageAccess::pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes)
{
  // [SECURITY:ValInput]
  sal_uInt8 const * src_lo = static_cast<sal_uInt8 const*>(pBuffer);
  if (!(src_lo != 0))
    return store_E_InvalidParameter;

  sal_uInt8 const * src_hi = src_lo + nBytes;
  if (!(src_lo <= src_hi))
    return store_E_InvalidParameter;

  sal_uInt64 const uSize = nOffset + nBytes;
  if (uSize > SAL_MAX_UINT32)
    return store_E_CantSeek;

  // ...
  if (uSize > m_nSize)
  {
    // increase size.
    storeError eErrCode = setSize (sal::static_int_cast<sal_uInt32>(uSize));
    if (eErrCode != store_E_None)
      return eErrCode;
  }

  sal_uInt8 * dst_lo = m_pData + nOffset;
  if (!(dst_lo <= m_pData + m_nSize))
    return store_E_CantSeek;

  sal_uInt8 * dst_hi = dst_lo + nBytes;
  if (!(dst_hi <= m_pData + m_nSize))
    return store_E_CantWrite;

  // copy.
  memcpy (dst_lo, src_lo, (src_hi - src_lo));
  return store_E_None;
}
storeError MemoryPageAccess::getSize_Impl (sal_uInt32 & rnSize)
{
  rnSize = m_nSize;
  return store_E_None;
}
storeError MemoryPageAccess::setSize_Impl (sal_uInt32 nSize)
{
  if (nSize != m_nSize)
  {
    sal_uInt8 * pData = static_cast<sal_uInt8*>(rtl_reallocateMemory (m_pData, nSize));
    if (pData != 0)
    {
      if (nSize > m_nSize)
    memset (pData + m_nSize, 0, sal::static_int_cast< size_t >(nSize - m_nSize));
    }
    else
    {
      if (nSize != 0)
    return store_E_OutOfMemory;
    }
    m_pData = pData, m_nSize = nSize;
  }
  return store_E_None;
}
void MemoryPageAccess::freeMemory (sal_uInt8 * pData, sal_uInt32 /*nSize*/)
{
  rtl_freeMemory (pData);
}

/*===*/

class MappedPageAccess : public MemoryPageAccess
{
  /** @see MemoryPageAccess::destructor_type callback function.
   */
  static void unmapFile (sal_uInt8 * pData, sal_uInt32 nSize);

public:
  MappedPageAccess (sal_uInt8 * pData, sal_uInt32 nSize);

  virtual storeError initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize);

  virtual storeError writePageAt (PageHolder const & rPage, sal_uInt32 nOffset);

private:
  virtual storeError pokeAt_Impl (sal_uInt32 nOffset, void const * pBuffer, sal_uInt32 nBytes);
  virtual storeError setSize_Impl (sal_uInt32 nSize);

protected:
  virtual ~MappedPageAccess() {}
};

MappedPageAccess::MappedPageAccess (sal_uInt8 * pData, sal_uInt32 nSize)
  : MemoryPageAccess (pData, nSize, MappedPageAccess::unmapFile)
{
}
storeError MappedPageAccess::initialize (storeAccessMode eAccessMode, sal_uInt16 nPageSize)
{
  OSL_PRECOND(eAccessMode == store_AccessReadOnly, "store::MappedPageAccess: invalid AccessMode");
  return MemoryPageAccess::initialize (eAccessMode, nPageSize);
}
storeError MappedPageAccess::writePageAt (PageHolder const & /*rPage*/, sal_uInt32 /*nOffset*/)
{
  return store_E_AccessViolation;
}
storeError MappedPageAccess::pokeAt_Impl (sal_uInt32 /*nOffset*/, void const * /*pBuffer*/, sal_uInt32 /*nBytes*/)
{
  return store_E_AccessViolation;
}
storeError MappedPageAccess::setSize_Impl (sal_uInt32 /*nSize*/)
{
  return store_E_AccessViolation;
}
void MappedPageAccess::unmapFile (sal_uInt8 * pData, sal_uInt32 nSize)
{
  (void) osl_unmapFile (pData, nSize);
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/

#include <stdio.h>

int SAL_CALL main (int argc, char ** argv)
{
    OSL_PRECOND(argc >= 1, "t_page: error: insufficient number of arguments.");
    if (argc < 1)
        return 0;

    {
        void *a = (void*)1, *b = (void*)2;
        swap<void*>(a, b);
    }
    {
        PageObject a;
        PageObject b (a);
        PageObject c;

        c = b;
        a = a;
    }
    {
        TestBIOS aBIOS;
        TestClient aClient;
        aClient.dwim (aBIOS);
    }

    if (argc > 1)
    {
        rtl_uString * pFilename = 0;
        rtl_uString_newFromAscii (&pFilename, argv[1]);
        storeAccessMode eAccessMode = store_AccessReadOnly;

        // Acquire file handle.
        ResourceHolder<FileHandle> h1;
        oslFileError result = h1.get().initialize (pFilename, FilePageAccess::MODE_TO_NATIVE(eAccessMode));
        if (result == osl_File_E_None)
        {
            ResourceHolder<FileHandle> h2 (h1);
            h1 = h2;

            if (eAccessMode == store_AccessReadOnly)
            {
                ResourceHolder<FileMapping> m1;
                result = m1.get().initialize (h1.get());

                const sal_uInt32 nSize = sal::static_int_cast<sal_uInt32>(m1.get().m_uSize);
                (void) nSize; // UNUSED

                ResourceHolder<FileMapping> m2 (m1);
                m1 = m2;

                return result;
            }
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
