/*************************************************************************
 *
 *  $RCSfile: stordata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STORE_STORDATA_HXX_
#define _STORE_STORDATA_HXX_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif
#ifndef _STORE_MACROS_HXX_
#include <store/macros.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace store {
#endif

/*========================================================================
 *
 * OStoreDataPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_DATAPAGE 0x94190310UL

struct OStoreDataPageData :
    public NAMESPACE_STORE(OStorePageData)
{
    typedef OStorePageData       base;
    typedef OStoreDataPageData   self;

    typedef OStorePageDescriptor D;

    /** Representation.
    */
    sal_uInt8 m_pData[1];

    /** size.
    */
    static sal_uInt16 size (void)
    {
        return 0;
    }

    /** capacity.
    */
    static sal_uInt16 capacity (const D& rDescr)
    {
        return (rDescr.m_nSize - (base::size() + self::size()));
    }

    sal_uInt16 capacity (void) const
    {
        return self::capacity (base::m_aDescr);
    }

    /** usage.
    */
    static sal_uInt16 usage (const D& rDescr)
    {
        return (rDescr.m_nUsed - (base::size() + self::size()));
    }

    sal_uInt16 usage (void) const
    {
        return self::usage (base::m_aDescr);
    }

    /** initialize.
    */
    void initialize (void)
    {
        base::m_aGuard.m_nMagic = STORE_MAGIC_DATAPAGE;
        base::m_aDescr.m_nUsed += self::size();
        rtl_zeroMemory (m_pData, capacity());
    }

    /** Construction.
    */
    OStoreDataPageData (sal_uInt16 nPageSize)
        : base (nPageSize)
    {
        initialize();
    }
};

/*========================================================================
 *
 * OStoreDataPageObject.
 *
 *======================================================================*/
class OStoreDataPageObject :
    public NAMESPACE_STORE(OStorePageObject)
{
    typedef OStorePageObject   base;
    typedef OStoreDataPageData page;

public:
    /** Construction.
    */
    inline OStoreDataPageObject (page& rPage);

private:
    /** Representation.
    */
    page& m_rPage;
};

inline OStoreDataPageObject::OStoreDataPageObject (page& rPage)
    : OStorePageObject (rPage), m_rPage (rPage)
{
}

/*========================================================================
 *
 * OStoreIndirectionPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_INDIRECTPAGE 0x89191107UL

struct OStoreIndirectionPageData :
    public NAMESPACE_STORE(OStorePageData)
{
    typedef OStorePageData            base;
    typedef OStoreIndirectionPageData self;

    typedef OStorePageGuard           G;
    typedef OStorePageDescriptor      D;

    /** Representation.
    */
    G          m_aGuard;
    sal_uInt32 m_pData[1];

    /** size.
    */
    static sal_uInt16 size (void)
    {
        return (sizeof(G));
    }

    /** capacity.
    */
    static sal_uInt16 capacity (const D& rDescr)
    {
        return (rDescr.m_nSize - (base::size() + self::size()));
    }
    sal_uInt16 capacity (void) const
    {
        return self::capacity (base::m_aDescr);
    }

    /** capacityCount.
    */
    static sal_uInt16 capacityCount (const D& rDescr)
    {
        return (capacity(rDescr) / sizeof(sal_uInt32));
    }
    sal_uInt16 capacityCount (void) const
    {
        return (capacity() / sizeof(sal_uInt32));
    }

    /** Construction.
    */
    OStoreIndirectionPageData (sal_uInt16 nPageSize);
    void initialize (void);

    /** Comparison.
    */
    sal_Bool operator== (const OStoreIndirectionPageData& rOther) const
    {
        return (base::operator==(rOther) && (m_aGuard == rOther.m_aGuard));
    }

    /** swap (internal and external representation).
    */
    void swap (const D& rDescr);

    /** guard (external representation).
    */
    void guard (const D& rDescr)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, m_pData, capacity(rDescr));
#ifdef OSL_BIGENDIAN
        nCRC32 = VOS_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        m_aGuard.m_nCRC32 = nCRC32;
    }

    /** verify (external representation).
    */
    storeError verify (const D& rDescr)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, m_pData, capacity(rDescr));
#ifdef OSL_BIGENDIAN
        nCRC32 = VOS_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        if (m_aGuard.m_nCRC32 != nCRC32)
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * OStoreIndirectionPageObject.
 *
 *======================================================================*/
class OStoreIndirectionPageObject :
    public NAMESPACE_STORE(OStorePageObject)
{
    typedef OStorePageObject          base;
    typedef OStoreIndirectionPageData page;

    typedef OStorePageDescriptor      D;

public:
    /** Construction.
    */
    inline OStoreIndirectionPageObject (page& rPage);

    /** External representation.
    */
    virtual void       swap   (const D& rDescr);
    virtual void       guard  (const D& rDescr);
    virtual storeError verify (const D& rDescr);

    /** get (indirect data page).
    */
    storeError get (
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError get (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError get (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpDouble,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    /** put (indirect data page).
    */
    storeError put (
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError put (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError put (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpDouble,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    /** truncate (indirect data page).
    */
    storeError truncate (
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError truncate (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    storeError truncate (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        page                 *&rpDouble,
        page                 *&rpSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

private:
    /** Representation.
    */
    page& m_rPage;
};

inline OStoreIndirectionPageObject::OStoreIndirectionPageObject (page& rPage)
    : OStorePageObject (rPage), m_rPage (rPage)
{
}

/*========================================================================
 *
 * OStoreDirectoryDataBlock.
 *
 *======================================================================*/
#define STORE_LIMIT_DATAPAGE_DIRECT 16
#define STORE_LIMIT_DATAPAGE_SINGLE  8
#define STORE_LIMIT_DATAPAGE_DOUBLE  1
#define STORE_LIMIT_DATAPAGE_TRIPLE  1

struct OStoreDirectoryDataBlock
{
    typedef OStorePageGuard G;

    /** LinkDescriptor.
    */
    struct LinkDescriptor
    {
        /** Representation.
        */
        sal_uInt16 m_nIndex0;
        sal_uInt16 m_nIndex1;
        sal_uInt16 m_nIndex2;
        sal_uInt16 m_nIndex3;

        /** Construction.
        */
        LinkDescriptor (void)
            : m_nIndex0 ((sal_uInt16)(~0)),
              m_nIndex1 ((sal_uInt16)(~0)),
              m_nIndex2 ((sal_uInt16)(~0)),
              m_nIndex3 ((sal_uInt16)(~0))
        {}
    };

    /** LinkTable.
    */
    struct LinkTable
    {
        /** Representation.
        */
        sal_uInt32 m_pDirect[STORE_LIMIT_DATAPAGE_DIRECT];
        sal_uInt32 m_pSingle[STORE_LIMIT_DATAPAGE_SINGLE];
        sal_uInt32 m_pDouble[STORE_LIMIT_DATAPAGE_DOUBLE];
        sal_uInt32 m_pTriple[STORE_LIMIT_DATAPAGE_TRIPLE];

        /** Construction.
        */
        LinkTable (void);
        void initialize (void);

        /** swap (internal and external representation).
        */
        void swap (void);
    };

    /** Representation.
    */
    G          m_aGuard;
    LinkTable  m_aTable;
    sal_uInt32 m_nDataLen;

    /** size.
    */
    static sal_uInt16 size (void)
    {
        return (sizeof(G) + sizeof(LinkTable) + sizeof(sal_uInt32));
    }

    /** initialize.
    */
    void initialize (void)
    {
        m_aGuard = G();
        m_aTable.initialize();
        m_nDataLen = 0;
    }

    /** Construction.
    */
    OStoreDirectoryDataBlock (void)
        : m_nDataLen (0)
    {}

    /** Comparison.
    */
    sal_Bool operator== (const OStoreDirectoryDataBlock& rOther) const
    {
        return (m_aGuard == rOther.m_aGuard);
    }

    /** swap (internal and external representation).
    */
    void swap (void)
    {
#ifdef OSL_BIGENDIAN
        m_aGuard.swap();
        m_aTable.swap();
        m_nDataLen = VOS_SWAPDWORD(m_nDataLen);
#endif /* OSL_BIGENDIAN */
    }

    /** guard (external representation).
    */
    void guard (void)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, &m_aTable, size() - sizeof(G));
#ifdef OSL_BIGENDIAN
        nCRC32 = VOS_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        m_aGuard.m_nCRC32 = nCRC32;
    }

    /** verify (external representation).
    */
    storeError verify (void)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, &m_aTable, size() - sizeof(G));
#ifdef OSL_BIGENDIAN
        nCRC32 = VOS_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        if (m_aGuard.m_nCRC32 != nCRC32)
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }

    /** direct.
    */
    static sal_uInt16 directCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_DIRECT));
    }
    sal_uInt32 directLink (sal_uInt16 nIndex) const
    {
        if (nIndex < directCount())
            return m_aTable.m_pDirect[nIndex];
        else
            return STORE_PAGE_NULL;
    }
    void directLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < directCount())
            m_aTable.m_pDirect[nIndex] = nAddr;
    }

    /** single.
    */
    static sal_uInt16 singleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_SINGLE));
    }
    sal_uInt32 singleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < singleCount())
            return m_aTable.m_pSingle[nIndex];
        else
            return STORE_PAGE_NULL;
    }
    void singleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < singleCount())
            m_aTable.m_pSingle[nIndex] = nAddr;
    }

    /** double.
    */
    static sal_uInt16 doubleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_DOUBLE));
    }
    sal_uInt32 doubleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < doubleCount())
            return m_aTable.m_pDouble[nIndex];
        else
            return STORE_PAGE_NULL;
    }
    void doubleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < doubleCount())
            m_aTable.m_pDouble[nIndex] = nAddr;
    }

    /** triple.
    */
    static sal_uInt16 tripleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_TRIPLE));
    }
    sal_uInt32 tripleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < tripleCount())
            return m_aTable.m_pTriple[nIndex];
        else
            return STORE_PAGE_NULL;
    }
    void tripleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < tripleCount())
            m_aTable.m_pTriple[nIndex] = nAddr;
    }
};

/*========================================================================
 *
 * OStoreDirectoryPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_DIRECTORYPAGE 0x62190120UL

struct OStoreDirectoryPageData :
    public NAMESPACE_STORE(OStorePageData)
{
    typedef OStorePageData           base;
    typedef OStoreDirectoryPageData  self;

    typedef OStorePageDescriptor     D;
    typedef OStorePageNameBlock      NameBlock;
    typedef OStoreDirectoryDataBlock DataBlock;

    /** Representation.
    */
    NameBlock m_aNameBlock;
    DataBlock m_aDataBlock;
    sal_uInt8 m_pData[1];

    /** size.
    */
    static sal_uInt16 size (void)
    {
        return (NameBlock::size() + DataBlock::size());
    }

    /** capacity.
    */
    static sal_uInt16 capacity (const D& rDescr)
    {
        return (rDescr.m_nSize - (base::size() + self::size()));
    }
    sal_uInt16 capacity (void) const
    {
        return self::capacity (base::m_aDescr);
    }

    /** usage.
    */
    static sal_uInt16 usage (const D& rDescr)
    {
        return (rDescr.m_nUsed - (base::size() + self::size()));
    }
    sal_uInt16 usage (void) const
    {
        return self::usage (base::m_aDescr);
    }

    /** initialize.
    */
    void initialize (void)
    {
        base::m_aGuard.m_nMagic = STORE_MAGIC_DIRECTORYPAGE;
        base::m_aDescr.m_nUsed  = base::size() + self::size();

        m_aNameBlock.initialize();
        m_aDataBlock.initialize();

        rtl_zeroMemory (m_pData, capacity());
    }

    /** Construction.
    */
    OStoreDirectoryPageData (sal_uInt16 nPageSize)
        : base (nPageSize)
    {
        base::m_aGuard.m_nMagic = STORE_MAGIC_DIRECTORYPAGE;
        base::m_aDescr.m_nUsed += self::size();
        rtl_zeroMemory (m_pData, capacity());
    }

    /** Comparsion.
    */
    sal_Bool operator== (const OStoreDirectoryPageData& rOther) const
    {
        return ((base::operator==(rOther)           ) &&
                (m_aNameBlock == rOther.m_aNameBlock) &&
                (m_aDataBlock == rOther.m_aDataBlock)    );
    }

    /** swap (internal and external representation).
    */
    void swap (const D& rDescr)
    {
#ifdef OSL_BIGENDIAN
        m_aNameBlock.swap();
        m_aDataBlock.swap();
#endif /* OSL_BIGENDIAN */
    }

    /** guard (external representation).
    */
    void guard (const D& rDescr)
    {
        m_aNameBlock.guard();
        m_aDataBlock.guard();
    }

    /** verify (external representation).
    */
    storeError verify (const D& rDescr)
    {
        storeError eErrCode = m_aNameBlock.verify();
        if (eErrCode == store_E_None)
            eErrCode = m_aDataBlock.verify();
        return eErrCode;
    }

    /** ChunkDescriptor.
    */
    struct ChunkDescriptor
    {
        /** Representation.
        */
        sal_uInt32 m_nPage;
        sal_uInt16 m_nOffset;
        sal_uInt16 m_nLength;

        /** Construction.
        */
        ChunkDescriptor (sal_uInt32 nPosition, sal_uInt16 nCapacity)
        {
            m_nPage   = nPosition / nCapacity;
            m_nOffset = (sal_uInt16)((nPosition % nCapacity) & 0xffff);
            m_nLength = nCapacity - m_nOffset;
        }
    };

    /** ChunkScope.
    */
    enum ChunkScope
    {
        SCOPE_INTERNAL,
        SCOPE_EXTERNAL,
        SCOPE_DIRECT,
        SCOPE_SINGLE,
        SCOPE_DOUBLE,
        SCOPE_TRIPLE,
        SCOPE_UNREACHABLE,
        SCOPE_UNKNOWN
    };

    /** scope (internal).
    */
    ChunkScope scope (sal_uInt32 nPosition) const
    {
        sal_uInt32 nCapacity = capacity();
        if (nPosition < nCapacity)
            return SCOPE_INTERNAL;
        else
            return SCOPE_EXTERNAL;
    }
};

/*========================================================================
 *
 * OStoreDirectoryPageObject.
 *
 *======================================================================*/
class OStoreDirectoryPageObject :
    public NAMESPACE_STORE(OStorePageObject)
{
    typedef OStorePageObject          base;
    typedef OStoreDirectoryPageData   page;
    typedef OStoreIndirectionPageData indirect;

    typedef OStorePageDescriptor      D;

public:
    /** Construction.
    */
    inline OStoreDirectoryPageObject (page& rPage);

    /** External representation.
    */
    virtual void       swap   (const D& rDescr);
    virtual void       guard  (const D& rDescr);
    virtual storeError verify (const D& rDescr);

    /** attrib.
    */
    sal_uInt32 attrib (void) const
    {
        return m_rPage.m_aNameBlock.m_nAttrib;
    }
    void attrib (sal_uInt32 nAttrib)
    {
        m_rPage.m_aNameBlock.m_nAttrib = nAttrib;
        touch();
    }

    /** dataLength.
    */
    sal_uInt32 dataLength (void) const
    {
        return m_rPage.m_aDataBlock.m_nDataLen;
    }
    void dataLength (sal_uInt32 nLength)
    {
        m_rPage.m_aDataBlock.m_nDataLen = nLength;
        touch();
    }

    /** direct.
    */
    sal_uInt32 directLink (sal_uInt16 nIndex) const
    {
        return m_rPage.m_aDataBlock.directLink (nIndex);
    }
    void directLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        m_rPage.m_aDataBlock.directLink (nIndex, nAddr);
        touch();
    }

    /** single indirect.
    */
    sal_uInt32 singleLink (sal_uInt16 nIndex) const
    {
        return m_rPage.m_aDataBlock.singleLink (nIndex);
    }
    void singleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        m_rPage.m_aDataBlock.singleLink (nIndex, nAddr);
        touch();
    }

    /** double indirect.
    */
    sal_uInt32 doubleLink (sal_uInt16 nIndex) const
    {
        return m_rPage.m_aDataBlock.doubleLink (nIndex);
    }
    void doubleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        m_rPage.m_aDataBlock.doubleLink (nIndex, nAddr);
        touch();
    }

    /** triple indirect.
    */
    sal_uInt32 tripleLink (sal_uInt16 nIndex) const
    {
        return m_rPage.m_aDataBlock.tripleLink (nIndex);
    }
    void tripleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        m_rPage.m_aDataBlock.tripleLink (nIndex, nAddr);
        touch();
    }

    /** scope (external data page).
    */
    page::ChunkScope scope (
        sal_uInt32                       nPage,
        page::DataBlock::LinkDescriptor &rDescr) const;

    /** get (external data page).
    */
    storeError get (
        sal_uInt32             nPage,
        indirect             *&rpSingle,
        indirect             *&rpDouble,
        indirect             *&rpTriple,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    /** put (external data page).
    */
    storeError put (
        sal_uInt32             nPage,
        indirect             *&rpSingle,
        indirect             *&rpDouble,
        indirect             *&rpTriple,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

    /** truncate (external data page).
    */
    storeError truncate (
        sal_uInt32             nPage,
        indirect             *&rpSingle,
        indirect             *&rpDouble,
        indirect             *&rpTriple,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);

private:
    /** Representation.
    */
    page& m_rPage;

    /** truncate (external data page scope; private).
    */
    storeError truncate (
        page::ChunkScope       eScope,
        sal_uInt16             nRemain,
        indirect             *&rpSingle,
        indirect             *&rpDouble,
        indirect             *&rpTriple,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS,
        NAMESPACE_VOS(IMutex) *pMutex = NULL);
};

inline OStoreDirectoryPageObject::OStoreDirectoryPageObject (page& rPage)
    : OStorePageObject (rPage), m_rPage (rPage)
{
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/
#ifdef _USE_NAMESPACE
}
#endif

#endif /* !_STORE_STORDATA_HXX_ */

