/*************************************************************************
 *
 *  $RCSfile: storbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 20:54:25 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _STORE_STORBASE_CXX_ "$Revision: 1.2 $"

#ifndef __ALGORITHM__
#include <algorithm>
#endif
#ifndef __UTILITY__
#include <utility>
#endif
#ifndef __HASH_MAP__
#include <hash_map>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif
#ifndef _STORE_OBJECT_HXX_
#include <store/object.hxx>
#endif
#ifndef _STORE_LOCKBYTE_HXX_
#include <store/lockbyte.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif

using namespace store;

/*========================================================================
 *
 * OStorePageGuard.
 *
 *======================================================================*/
/*
 * CRC polynomial 0xEDB88320.
 */
const sal_uInt32 store::OStorePageGuard::m_pTable[] =
{
    /* 0 */
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,

    /* 1 */
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,

    /* 2 */
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,

    /* 3 */
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

    /* 4 */
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,

    /* 5 */
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,

    /* 6 */
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,

    /* 7 */
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

    /* 8 */
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,

    /* 9 */
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,

    /* A */
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,

    /* B */
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

    /* C */
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,

    /* D */
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,

    /* E */
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,

    /* F */
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*
 * crc32.
 */
sal_uInt32 OStorePageGuard::crc32 (
    sal_uInt32 nCRC32, const void *pData, sal_uInt32 nSize)
{
    if (pData)
    {
        register const sal_uInt8 *p = (const sal_uInt8*)pData;
        register const sal_uInt8 *q = p + nSize;

        nCRC32 = ~nCRC32;
        while (p < q)
            nCRC32 = updcrc32 (nCRC32, *(p++));
        nCRC32 = ~nCRC32;
    }
    return nCRC32;
}

/*========================================================================
 *
 * OStorePageObject.
 *
 *======================================================================*/
/*
 * ~OStorePageObject.
 */
OStorePageObject::~OStorePageObject (void)
{
}

/*
 * swap.
 */
void OStorePageObject::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    m_rPage.swap (rDescr);
#endif /* OSL_BIGENDIAN */
}

/*
 * guard.
 */
void OStorePageObject::guard (const D& rDescr)
{
    m_rPage.guard (rDescr);
}

/*
 * verify.
 */
storeError OStorePageObject::verify (const D& rDescr)
{
    return m_rPage.verify (rDescr);
}

/*========================================================================
 *
 * OStoreSuperBlock.
 *
 *======================================================================*/
#define STORE_MAGIC_SUPERBLOCK 0x484D5343UL

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

    /** size.
     */
    static sal_uInt16 size (void)
    {
        return (sizeof(G) + sizeof(D) + 2 * (sizeof(L) + sizeof(sal_uInt32)));
    }

    /** Construction.
     */
    OStoreSuperBlock (void)
        : m_aGuard  (STORE_MAGIC_SUPERBLOCK),
          m_nMarked (0),
          m_aMarked (0),
          m_nUnused (0),
          m_aUnused (0)
    {}

    OStoreSuperBlock (const OStoreSuperBlock& rOther)
        : m_aGuard  (rOther.m_aGuard),
          m_aDescr  (rOther.m_aDescr),
          m_nMarked (rOther.m_nMarked),
          m_aMarked (rOther.m_aMarked),
          m_nUnused (rOther.m_nUnused),
          m_aUnused (rOther.m_aUnused)
    {}

    OStoreSuperBlock& operator= (const OStoreSuperBlock& rOther)
    {
        m_aGuard  = rOther.m_aGuard;
        m_aDescr  = rOther.m_aDescr;
        m_nMarked = rOther.m_nMarked;
        m_aMarked = rOther.m_aMarked;
        m_nUnused = rOther.m_nUnused;
        m_aUnused = rOther.m_aUnused;
        return *this;
    }

    /** Comparison.
     */
    sal_Bool operator== (const OStoreSuperBlock& rOther) const
    {
        return ((m_aGuard  == rOther.m_aGuard ) &&
                (m_aDescr  == rOther.m_aDescr ) &&
                (m_nMarked == rOther.m_nMarked) &&
                (m_aMarked == rOther.m_aMarked) &&
                (m_nUnused == rOther.m_nUnused) &&
                (m_aUnused == rOther.m_aUnused)    );
    }

    /** create.
     */
    void create (const D& rDescr)
    {
        m_aGuard  = G(STORE_MAGIC_SUPERBLOCK);
        m_aDescr  = rDescr;

        m_nMarked = 0;
        m_aMarked = L(0);

        m_nUnused = 0;
        m_aUnused = L(0);
    }

    /** unused(Count|Head|Insert|Remove|Reset).
     */
    sal_uInt32 unusedCount (void) const
    {
        return m_nUnused;
    }
    const L& unusedHead (void) const
    {
        return m_aUnused;
    }
    void unusedInsert (const L& rLink)
    {
        m_nUnused++;
        m_aUnused = rLink;
    }
    void unusedRemove (const L& rLink)
    {
        m_nUnused--;
        m_aUnused = rLink;
    }
    void unusedReset (void)
    {
        m_nUnused = 0;
        m_aUnused = L(0);
    }

    /** swap (internal and external representation).
     */
    void swap (void)
    {
#ifdef OSL_BIGENDIAN
        m_aGuard.swap();
        m_aDescr.swap();

        m_nMarked = OSL_SWAPDWORD(m_nMarked);
        m_aMarked.swap();

        m_nUnused = OSL_SWAPDWORD(m_nUnused);
        m_aUnused.swap();
#endif /* OSL_BIGENDIAN */
    }

    /** guard (external representation).
     */
    void guard (void)
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, &m_aDescr, size() - sizeof(G));
#ifdef OSL_BIGENDIAN
        nCRC32 = OSL_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        m_aGuard.m_nCRC32 = nCRC32;
    }

    /** verify (external representation).
     */
    storeError verify (void)
    {
        sal_uInt32 nMagic = m_aGuard.m_nMagic;
#ifdef OSL_BIGENDIAN
        nMagic = OSL_SWAPDWORD(nMagic);
#endif /* OSL_BIGENDIAN */
        if (nMagic != STORE_MAGIC_SUPERBLOCK)
            store_E_WrongFormat;

        sal_uInt32 nCRC32 = 0;
        nCRC32 = G::crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = G::crc32 (nCRC32, &m_aDescr, size() - sizeof(G));
#ifdef OSL_BIGENDIAN
        nCRC32 = OSL_SWAPDWORD(nCRC32);
#endif /* OSL_BIGENDIAN */
        if (m_aGuard.m_nCRC32 != nCRC32)
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * OStoreStateBlock.
 *
 *======================================================================*/
struct OStoreStateBlock
{
    enum StateBits
    {
        STATE_CLEAN      = 0,
        STATE_CLOSE_WAIT = 1,
        STATE_FLUSH_WAIT = 2
    };

    /** Representation.
     */
    sal_uInt32 m_nState;

    /** size.
     */
    static sal_uInt16 size (void)
    {
        return sizeof(sal_uInt32);
    }

    /** Construction.
     */
    OStoreStateBlock (void)
        : m_nState (STATE_CLEAN)
    {}

    /** swap (internal and external representation).
     */
    void swap (void)
    {
#ifdef OSL_BIGENDIAN
        m_nState = OSL_SWAPDWORD(m_nState);
#endif /* OSL_BIGENDIAN */
    }

    /** Operation.
     */
    sal_Bool closePending (void) const
    {
        return ((m_nState & STATE_CLOSE_WAIT) ? 1 : 0);
    }
    void closed (void)
    {
        m_nState &= ~STATE_CLOSE_WAIT;
    }

    sal_Bool flushPending (void) const
    {
        return ((m_nState & STATE_FLUSH_WAIT) ? 1 : 0);
    }
    void flushed (void)
    {
        m_nState &= ~STATE_FLUSH_WAIT;
    }

    void modified (void)
    {
        m_nState |= (STATE_CLOSE_WAIT | STATE_FLUSH_WAIT);
    }
    void clean (void)
    {
        m_nState &= ~(STATE_CLOSE_WAIT | STATE_FLUSH_WAIT);
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
    typedef OStoreStateBlock StateBlock;

    /** Representation.
     */
    SuperBlock m_aSuperOne;
    SuperBlock m_aSuperTwo;
    StateBlock m_aState;

    /** size.
     */
    static sal_uInt16 size (void)
    {
        return (2 * SuperBlock::size() + StateBlock::size());
    }

    /** Construction.
     */
    OStoreSuperBlockPage (void)
    {}

    /** swap (internal and external representation).
     */
    void swap (void)
    {
#ifdef OSL_BIGENDIAN
        m_aSuperOne.swap();
        m_aSuperTwo.swap();
        m_aState.swap();
#endif /* OSL_BIGENDIAN */
    }

    /** load (w/o verification).
     */
    storeError load (OStorePageBIOS &rBIOS)
    {
        // Read.
        storeError eErrCode = rBIOS.read (0, this, size());

#ifdef OSL_BIGENDIAN
        // Swap to internal representation.
        swap();
#endif /* OSL_BIGENDIAN */

        // Done.
        return eErrCode;
    }

    /** save.
     */
    storeError save (OStorePageBIOS &rBIOS)
    {
#ifdef OSL_BIGENDIAN
        // Swap to external representation.
        swap();
#endif /* OSL_BIGENDIAN */

        // Guard.
        m_aSuperOne.guard();
        m_aSuperTwo.guard();

        // Write.
        storeError eErrCode = rBIOS.write (0, this, size());

#ifdef OSL_BIGENDIAN
        // Swap back to internal representation.
        swap();
#endif /* OSL_BIGENDIAN */

        // Done.
        return eErrCode;
    }

    /** create.
     */
    storeError create (
        OStorePageBIOS             &rBIOS,
        const OStorePageDescriptor &rDescr);

    /** close.
     */
    storeError close (
        OStorePageBIOS &rBIOS);

    /** flush.
     */
    storeError flush (
        OStorePageBIOS &rBIOS);

    /** modified.
     */
    storeError modified (
        OStorePageBIOS &rBIOS);

    /** verify (with repair).
     */
    storeError verify (
        OStorePageBIOS &rBIOS);
};

} // namespace store

/*========================================================================
 *
 * OStoreSuperBlockPage implementation.
 *
 *======================================================================*/
/*
 * create.
 */
storeError OStoreSuperBlockPage::create (
    OStorePageBIOS             &rBIOS,
    const OStorePageDescriptor &rDescr)
{
    // Setup initial Page.
    void *p = rtl_allocateMemory (rDescr.m_nSize);
    rtl_zeroMemory (p, rDescr.m_nSize);

    // Mark as modified.
    m_aState.modified();

    // Write initial Page.
    storeError eErrCode = rBIOS.write (0, p, rDescr.m_nSize);
    if (eErrCode == store_E_None)
    {
        // Setup 1st and 2nd SuperBlock copy.
        m_aSuperOne.create (rDescr);
        m_aSuperTwo = m_aSuperOne;

        // Mark as modified.
        m_aState.modified();

        // Save.
        eErrCode = save (rBIOS);
    }

    // Cleanup and finish.
    rtl_freeMemory (p);
    return eErrCode;
}

/*
 * close.
 */
storeError OStoreSuperBlockPage::close (OStorePageBIOS &rBIOS)
{
    storeError eErrCode = store_E_None;
    if (m_aState.closePending())
    {
        // Mark as modified.
        m_aState.modified();

        // Check access mode.
        if (rBIOS.isWriteable())
        {
            // Save StateBlock.
            StateBlock aState (m_aState);

            // Mark as clean.
            aState.clean();

#ifdef OSL_BIGENDIAN
            // Swap to external representation.
            aState.swap();
#endif /* OSL_BIGENDIAN */

            // Write behind SuperBlock.
            sal_uInt32 nAddr = 2 * SuperBlock::size();
            eErrCode = rBIOS.write (nAddr, &aState, StateBlock::size());
        }

        // Mark as clean.
        m_aState.clean();
    }
    return eErrCode;
}

/*
 * flush.
 */
storeError OStoreSuperBlockPage::flush (OStorePageBIOS &rBIOS)
{
    storeError eErrCode = store_E_None;
    if (m_aState.flushPending())
    {
        // Check access mode.
        if (rBIOS.isWriteable())
        {
            // Save StateBlock.
            StateBlock aState (m_aState);

            // Mark as flushed.
            aState.flushed();

#ifdef OSL_BIGENDIAN
            // Swap to external representation.
            aState.swap();
#endif /* OSL_BIGENDIAN */

            // Write behind SuperBlock.
            sal_uInt32 nAddr = 2 * SuperBlock::size();
            eErrCode = rBIOS.write (nAddr, &aState, StateBlock::size());
        }

        // Mark as flushed.
        m_aState.flushed();
    }
    return eErrCode;
}

/*
 * modified.
 */
storeError OStoreSuperBlockPage::modified (OStorePageBIOS &rBIOS)
{
    storeError eErrCode = store_E_None;
    if (!m_aState.flushPending())
    {
        // Mark as modified.
        m_aState.modified();

        // Check access mode.
        if (rBIOS.isWriteable())
        {
            // Save StateBlock.
            StateBlock aState (m_aState);

#ifdef OSL_BIGENDIAN
            // Swap to external representation.
            aState.swap();
#endif /* OSL_BIGENDIAN */

            // Write behind SuperBlock.
            sal_uInt32 nAddr = 2 * SuperBlock::size();
            eErrCode = rBIOS.write (nAddr, &aState, StateBlock::size());
        }
    }
    return eErrCode;
}

/*
 * verify (with repair).
 */
storeError OStoreSuperBlockPage::verify (OStorePageBIOS &rBIOS)
{
#ifdef OSL_BIGENDIAN
    // Swap to external representation.
    swap();
#endif /* OSL_BIGENDIAN */

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
                    eErrCode = rBIOS.write (0, this, size());
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
                eErrCode = rBIOS.write (0, this, size());
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
                eErrCode = rBIOS.write (0, this, size());
            else
                eErrCode = store_E_None;
        }
        else
        {
            // Double Failure.
            OSL_TRACE("OStoreSuperBlockPage::verify(): double failure.\n");
        }
    }

#ifdef OSL_BIGENDIAN
    // Swap back to internal representation.
    swap();
#endif /* OSL_BIGENDIAN */

    // Done.
    return eErrCode;
}

/*========================================================================
 *
 * OStorePageACL.
 *
 *======================================================================*/
typedef sal_uInt32              key_type;
typedef sal_uInt32              val_type;

typedef std::hash<key_type>     key_hash;
typedef std::equal_to<key_type> key_cmp;

typedef std::hash_map<key_type, val_type, key_hash, key_cmp> map_type;

namespace store
{

struct OStorePageACL : public map_type
{
    /** Representation.
     */
    sal_uInt32 m_nRefCount;

    /** Construction.
     */
    OStorePageACL (void)
        : m_nRefCount (0)
    {}
};

} // namespace store

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
      m_pAcl       (NULL),
      m_pSuper     (NULL),
      m_bModified  (sal_False),
      m_bWriteable (sal_False)
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
    if (rpSuper == NULL)
    {
        // Allocate.
        rpSuper = new SuperPage();

        // Load (w/o verification).
        storeError eErrCode = rpSuper->load (*this);
        if (eErrCode != store_E_None)
        {
            // Cleanup and fail.
            __STORE_DELETEZ (rpSuper);
            return eErrCode;
        }

        // Check SuperBlock state.
        if (rpSuper->m_aState.closePending())
            OSL_TRACE("OStorePageBIOS::verify(): close pending.\n");

        if (rpSuper->m_aState.flushPending())
            OSL_TRACE("OStorePageBIOS::verify(): flush pending.\n");
    }

    // Verify SuperBlock page (with repair).
    return rpSuper->verify (*this);
}

/*
 * repair (SuperBlock).
 * Internal: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::repair (SuperPage *&rpSuper)
{
    // Acquire Lock.
    storeError eErrCode = acquireLock (0, SuperPage::size());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Verify SuperBlock page (with repair).
    eErrCode = verify (rpSuper);
    if (eErrCode != store_E_None)
    {
        // Failure.
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // ReleaseLock.
    return releaseLock (0, SuperPage::size());
}

/*
 * initialize.
 * Precond: none.
 */
storeError OStorePageBIOS::initialize (
    ILockBytes      *pLockBytes,
    storeAccessMode  eAccessMode)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check arguments.
    storeError eErrCode = store_E_InvalidParameter;
    if (pLockBytes)
    {
        // Cleanup.
        __STORE_DELETEZ (m_pAcl);
        __STORE_DELETEZ (m_pSuper);

        // Initialize.
        m_xLockBytes = pLockBytes;
        m_bModified  = sal_False;
        m_bWriteable = (!(eAccessMode == store_AccessReadOnly));

        // Check access mode.
        if (!(eAccessMode == store_AccessCreate))
        {
            // Verify (repair) SuperBlock page.
            if (!(eAccessMode == store_AccessReadOnly))
                eErrCode = repair (m_pSuper);
            else
                eErrCode = verify (m_pSuper);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Obtain modified state.
            m_bModified = m_pSuper->m_aState.flushPending();
        }
        else
        {
            // Truncate to zero length.
            eErrCode = m_xLockBytes->setSize(0);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Commit.
            eErrCode = m_xLockBytes->flush();
            if (eErrCode != store_E_None)
                return eErrCode;

            // Mark not existent.
            eErrCode = store_E_NotExists;
        }
    }
    return eErrCode;
}

/*
 * create (SuperBlock).
 * Precond: initialized, writeable (store_AccessCreate).
 */
storeError OStorePageBIOS::create (sal_uInt16 nPageSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Check PageSize.
    sal_uInt16 nMinSize = SuperPage::size();
    nMinSize  = std::max (nMinSize, STORE_MINIMUM_PAGESIZE);

    nPageSize = std::max (nPageSize, nMinSize);
    nPageSize = std::min (nPageSize, STORE_MAXIMUM_PAGESIZE);

    sal_uInt16 nRemSize = nPageSize % nMinSize;
    if (nRemSize)
        nPageSize += (nMinSize - nRemSize);

    // Acquire Lock.
    storeError eErrCode = acquireLock (0, SuperPage::size());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check SuperBlock page allocation.
    if (m_pSuper == NULL)
        m_pSuper = new SuperPage();

    // Create SuperBlock page.
    eErrCode = m_pSuper->create (
        *this, OStorePageDescriptor (nPageSize, nPageSize, nMinSize));
    if (eErrCode != store_E_None)
    {
        // Cleanup and fail.
        __STORE_DELETEZ (m_pSuper);
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Flush SuperBlock page.
    eErrCode = m_pSuper->flush (*this);
    if (eErrCode != store_E_None)
    {
        // Cleanup and fail.
        __STORE_DELETEZ (m_pSuper);
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Commit.
    eErrCode = m_xLockBytes->flush();
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::create(): flush failed");
    if (eErrCode == store_E_None)
    {
        // Mark not modified.
        m_bModified = sal_False;
    }

    // Release Lock and finish.
    return releaseLock (0, SuperPage::size());
}

/*
 * getPageSize.
 * Precond: initialized.
 */
storeError OStorePageBIOS::getPageSize (sal_uInt16 &rnPageSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Initialize [out] param.
    rnPageSize = 0;

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Load SuperBlock and require good health.
    storeError eErrCode = verify (m_pSuper);
    if (eErrCode == store_E_None)
    {
        // Obtain PageSize.
        rnPageSize = m_pSuper->m_aSuperOne.m_aDescr.m_nSize;
    }
    return eErrCode;
}

/*
 * acquireLock.
 * Low Level: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::acquireLock (
    sal_uInt32 nAddr, sal_uInt32 nSize)
{
    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Check Address.
    OSL_PRECOND(
        nAddr != STORE_PAGE_NULL,
        "OStorePageBIOS::acquireLock(): invalid Address");
    if (nAddr == STORE_PAGE_NULL)
        return store_E_CantSeek;

    // Acquire Lock.
#ifdef STORE_FEATURE_LOCKING
    return m_xLockBytes->lockRegion (nAddr, nSize);
#else
    return store_E_None;
#endif /* STORE_FEATURE_LOCKING */
}

/*
 * releaseLock.
 * Low Level: Precond: initialized, exclusive access.
 */
storeError OStorePageBIOS::releaseLock (
    sal_uInt32 nAddr, sal_uInt32 nSize)
{
    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Check Address.
    OSL_PRECOND(
        nAddr != STORE_PAGE_NULL,
        "OStorePageBIOS::releaseLock(): invalid Address");
    if (nAddr == STORE_PAGE_NULL)
        return store_E_CantSeek;

    // Release Lock.
#ifdef STORE_FEATURE_LOCKING
    return m_xLockBytes->unlockRegion (nAddr, nSize);
#else
    return store_E_None;
#endif /* STORE_FEATURE_LOCKING */
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

    // Check Address.
    OSL_PRECOND(
        nAddr != STORE_PAGE_NULL,
        "OStorePageBIOS::read(): invalid Address");
    if (nAddr == STORE_PAGE_NULL)
        return store_E_CantSeek;

    // Check Data.
    OSL_PRECOND(pData, "OStorePageBIOS::read(): no Data");
    if (pData == NULL)
        return store_E_InvalidParameter;

    // Read Page.
    sal_uInt32 nDone = 0;
    storeError eErrCode = m_xLockBytes->readAt (nAddr, pData, nSize, nDone);
    if ((eErrCode == store_E_None) && (nDone != nSize))
    {
        // Page too short.
        if (nDone == 0)
            eErrCode = store_E_NotExists;
        else
            eErrCode = store_E_CantRead;
    }
    return eErrCode;
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

    // Check Address.
    OSL_PRECOND(
        nAddr != STORE_PAGE_NULL,
        "OStorePageBIOS::write(): invalid Address");
    if (nAddr == STORE_PAGE_NULL)
        return store_E_CantSeek;

    // Check Data.
    OSL_PRECOND(pData, "OStorePageBIOS::write(): no Data");
    if (pData == NULL)
        return store_E_InvalidParameter;

    // Check modified state.
    storeError eErrCode = store_E_None;
    if (!m_bModified)
    {
        // Mark as modified.
        m_bModified = sal_True;

        // Mark SuperBlock modified.
        eErrCode = m_pSuper->modified (*this);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Write Data.
    sal_uInt32 nDone = 0;
    eErrCode = m_xLockBytes->writeAt (nAddr, pData, nSize, nDone);
    if ((eErrCode == store_E_None) && (nDone != nSize))
    {
        // Page too short.
        eErrCode = store_E_CantWrite;
    }
    return eErrCode;
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

    // Check access control list.
    if (!m_pAcl)
        m_pAcl = new OStorePageACL();

    // Find access control list entry.
    map_type::iterator it = m_pAcl->find (rDescr.m_nAddr);
    if (it != m_pAcl->end())
    {
        // Acquire existing entry (with ShareDenyWrite).
        if (eMode == store_AccessReadOnly)
            (*it).second += 1;
        else
            return store_E_AccessViolation;
    }
    else
    {
        // Insert new entry.
        typedef std::pair<const sal_uInt32, sal_uInt32> map_entry;
        m_pAcl->insert (map_entry (rDescr.m_nAddr, 1));
    }

    // Increment total referer count and finish.
    m_pAcl->m_nRefCount += 1;
    return store_E_None;
}

/*
 * releasePage.
 * Precond: initialized.
 */
storeError OStorePageBIOS::releasePage (
    const OStorePageDescriptor& rDescr, storeAccessMode eMode)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Check access control list.
    if (!m_pAcl)
        return store_E_NotExists;

    // Find access control list entry.
    map_type::iterator it = m_pAcl->find (rDescr.m_nAddr);
    if (it == m_pAcl->end())
        return store_E_NotExists;

    // Release existing entry.
    if ((*it).second > 1)
        (*it).second -= 1;
    else
        m_pAcl->erase(it);

    // Decrement total referer count and finish.
    m_pAcl->m_nRefCount -= 1;
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
    if (m_pAcl)
        return m_pAcl->m_nRefCount;
    else
        return 0;
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

    // Acquire SuperBlock Lock.
    storeError eErrCode = acquireLock (0, SuperPage::size());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Load SuperBlock and require good health.
    eErrCode = verify (m_pSuper);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Check allocation.
    if (eAlloc != ALLOCATE_EOF)
    {
        // Check FreeList.
        OStorePageLink aListHead (m_pSuper->m_aSuperTwo.unusedHead());
        if (aListHead.m_nAddr)
        {
            // Allocate from FreeList.
            OStorePageData aPageHead (OStorePageData::size());
            aPageHead.location (aListHead.m_nAddr);

            // Load PageHead.
            eErrCode = peek (aPageHead);
            if (eErrCode != store_E_None)
            {
                releaseLock (0, SuperPage::size());
                return eErrCode;
            }

            // Verify FreeList head.
            OSL_PRECOND(
                aPageHead.m_aUnused.m_nAddr != STORE_PAGE_NULL,
                "OStorePageBIOS::allocate(): page not free");
            if (aPageHead.m_aUnused.m_nAddr == STORE_PAGE_NULL)
            {
                // Recovery: Reset FreeList.
                m_pSuper->m_aSuperTwo.unusedReset();
                m_pSuper->m_aSuperOne = m_pSuper->m_aSuperTwo;

                // Save SuperBlock page.
                eErrCode = m_pSuper->save (*this);

                // Release SuperBlock Lock.
                releaseLock (0, SuperPage::size());

                // Recovery: Allocate from EOF.
                if (eErrCode == store_E_None)
                    return allocate (rPage, ALLOCATE_EOF);
                else
                    return store_E_Unknown;
            }

            // Assign location.
            OStorePageData &rData = rPage.getData();
            rData.location (aPageHead.m_aDescr.m_nAddr);

            // Pop from FreeList.
            aListHead.m_nAddr = aPageHead.m_aUnused.m_nAddr;
            rData.m_aUnused.m_nAddr = STORE_PAGE_NULL;

            // Save page.
            eErrCode = poke (rPage);
            if (eErrCode != store_E_None)
            {
                releaseLock (0, SuperPage::size());
                return eErrCode;
            }

            // Save SuperBlock page.
            m_pSuper->m_aSuperTwo.unusedRemove (aListHead);
            m_pSuper->m_aSuperOne = m_pSuper->m_aSuperTwo;

            eErrCode = m_pSuper->save (*this);
            OSL_POSTCOND(
                eErrCode == store_E_None,
                "OStorePageBIOS::allocate(): SuperBlock save failed");

            // Release SuperBlock Lock and finish.
            return releaseLock (0, SuperPage::size());
        }
    }

    // Allocate from logical EOF. Determine physical EOF.
    sal_uInt32 nAddr = STORE_PAGE_NULL;
    eErrCode = m_xLockBytes->stat (nAddr);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Obtain logical EOF.
    OStorePageDescriptor aDescr (m_pSuper->m_aSuperTwo.m_aDescr);
    if (aDescr.m_nAddr == 0)
        aDescr.m_nAddr = nAddr; /* backward compatibility */

    if (!(aDescr.m_nAddr < nAddr))
    {
        // Check modified state.
        if (!m_bModified)
        {
            // Mark modified.
            m_bModified = sal_True;

            // Mark SuperBlock modified.
            eErrCode = m_pSuper->modified (*this);
            if (eErrCode != store_E_None)
            {
                releaseLock (0, SuperPage::size());
                return eErrCode;
            }
        }

        // Resize.
        sal_uInt32 nAlign = STORE_MAXIMUM_PAGESIZE;

        nAlign = std::min (nAddr, nAlign);
        nAddr = ((nAddr + nAlign) / nAlign) * nAlign;

        eErrCode = m_xLockBytes->setSize (nAddr);
        if (eErrCode != store_E_None)
        {
            releaseLock (0, SuperPage::size());
            return eErrCode;
        }
    }

    // Save page.
    rPage.location (aDescr.m_nAddr);
    eErrCode = poke (rPage);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Save SuperBlock page.
    aDescr.m_nAddr += aDescr.m_nSize;

    m_pSuper->m_aSuperTwo.m_aDescr = aDescr;
    m_pSuper->m_aSuperOne = m_pSuper->m_aSuperTwo;

    eErrCode = m_pSuper->save (*this);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::allocate(): SuperBlock save failed");

    // Release SuperBlock Lock and finish.
    return releaseLock (0, SuperPage::size());
}

/*
 * free.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::free (OStorePageObject& rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Acquire SuperBlock Lock.
    storeError eErrCode = acquireLock (0, SuperPage::size());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Load SuperBlock and require good health.
    eErrCode = verify (m_pSuper);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Load PageHead.
    OStorePageData &rData = rPage.getData();

    eErrCode = peek (rData);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Push onto FreeList.
    OStorePageLink aListHead (m_pSuper->m_aSuperTwo.unusedHead());

    rData.m_aUnused.m_nAddr = aListHead.m_nAddr;
    aListHead.m_nAddr = rData.m_aDescr.m_nAddr;

    // Save PageHead.
    eErrCode = poke (rData);
    if (eErrCode != store_E_None)
    {
        releaseLock (0, SuperPage::size());
        return eErrCode;
    }

    // Save SuperBlock page.
    m_pSuper->m_aSuperTwo.unusedInsert (aListHead);
    m_pSuper->m_aSuperOne = m_pSuper->m_aSuperTwo;

    eErrCode = m_pSuper->save (*this);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::free(): SuperBlock save failed");

    // Release SuperBlock Lock and finish.
    return releaseLock (0, SuperPage::size());
}

/*
 * load.
 * Precond: initialized, readable.
 */
storeError OStorePageBIOS::load (OStorePageObject& rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;

    // Save PageDescriptor.
    OStorePageData       &rData = rPage.getData();
    OStorePageDescriptor  aDescr (rData.m_aDescr);

    // Read page.
    storeError eErrCode = read (aDescr.m_nAddr, &rData, aDescr.m_nSize);
    if (eErrCode != store_E_None)
    {
        // Restore PageDescriptor.
        rData.m_aDescr = aDescr;
        return eErrCode;
    }

    // Verify page.
    eErrCode = rPage.verify (aDescr);
    if (eErrCode != store_E_None)
    {
        // Restore PageDescriptor.
        rData.m_aDescr = aDescr;
        return eErrCode;
    }

#ifdef OSL_BIGENDIAN
    // Swap to internal representation.
    rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Verify PageDescriptor.
    if (!((aDescr == rData.m_aDescr) ||
          (aDescr <= rData.m_aDescr)    ))
        return store_E_InvalidAccess;

    // Mark page as clean.
    rPage.clean();

    // Done.
    return store_E_None;
}

/*
 * save.
 * Precond: initialized, writeable.
 */
storeError OStorePageBIOS::save (OStorePageObject& rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check precond.
    if (!m_xLockBytes.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Save Page.
    return poke (rPage);
}

/*
 * close.
 * Precond: none.
 */
storeError OStorePageBIOS::close (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);

    // Check access control list.
    if (m_pAcl)
    {
        // Check referer count.
        if (m_pAcl->m_nRefCount)
        {
            // Report remaining referer count.
            sal_uInt32 k = m_pAcl->m_nRefCount;
            OSL_TRACE("OStorePageBIOS::close(): referer count: %d\n", k);
        }
        __STORE_DELETEZ (m_pAcl);
    }

    // Check SuperBlock page.
    if (m_pSuper)
    {
        // Release SuperBlock page.
        m_pSuper->close (*this);
        __STORE_DELETEZ (m_pSuper);
    }

    // Check LockBytes.
    if (m_xLockBytes.is())
    {
        // Release LockBytes.
        m_xLockBytes->flush();
        m_xLockBytes.clear();
    }
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

    // Check mode and state.
    if (!(m_bWriteable && m_bModified))
        return store_E_None;

    // Flush SuperBlock page.
    storeError eErrCode = m_pSuper->flush (*this);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::flush(): SuperBlock flush failed");

    // Flush LockBytes.
    eErrCode = m_xLockBytes->flush();
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageBIOS::flush(): LockBytes flush failed");
    if (eErrCode == store_E_None)
    {
        // Mark not modified.
        m_bModified = sal_False;
    }
    return eErrCode;
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
    return m_xLockBytes->stat (rnSize);
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
    OStorePageData aPageHead (OStorePageData::size());

    // Check context.
    while (rCtx.isValid())
    {
        // Assign next location.
        aPageHead.location (rCtx.m_aDescr.m_nAddr);
        rCtx.m_aDescr.m_nAddr += rCtx.m_aDescr.m_nSize;

        // Load PageHead.
        storeError eErrCode = peek (aPageHead);
        if (eErrCode != store_E_None)
            continue;

        // Check PageHead Magic number.
        if (aPageHead.m_aGuard.m_nMagic != rCtx.m_nMagic)
            continue;

        // Check PageHead Unused link.
        if (aPageHead.m_aUnused.m_nAddr != STORE_PAGE_NULL)
            continue;

        // Load page.
        rPage.location (aPageHead.location());
        eErrCode = OStorePageBIOS::load (rPage);
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
storeError OStorePageBIOS::peek (OStorePageData &rData)
{
    // Save PageDescriptor.
    OStorePageDescriptor aDescr (rData.m_aDescr);

    // Read PageHead.
    storeError eErrCode = read (aDescr.m_nAddr, &rData, rData.size());
    if (eErrCode != store_E_None)
    {
        // Restore PageDescriptor.
        rData.m_aDescr = aDescr;
        return eErrCode;
    }

    // Verify PageHead.
    eErrCode = rData.verify (aDescr);
    if (eErrCode != store_E_None)
    {
        // Restore PageDescriptor.
        rData.m_aDescr = aDescr;
        return eErrCode;
    }

#ifdef OSL_BIGENDIAN
    // Swap to internal representation.
    rData.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Verify PageDescriptor.
    if (!((aDescr == rData.m_aDescr) ||
          (aDescr <= rData.m_aDescr)    ))
        return store_E_InvalidAccess;
    else
        return store_E_None;
}

/*
 * poke (PageHead).
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageBIOS::poke (OStorePageData &rData)
{
    // Save PageDescriptor.
    OStorePageDescriptor aDescr (rData.m_aDescr);

#ifdef OSL_BIGENDIAN
    // Swap to external representation.
    rData.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Guard PageHead.
    rData.guard (aDescr);

    // Write PageHead.
    storeError eErrCode = write (aDescr.m_nAddr, &rData, rData.size());

#ifdef OSL_BIGENDIAN
    // Swap back to internal representation.
    rData.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Done.
    return eErrCode;
}

/*
 * poke (PageObject).
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageBIOS::poke (OStorePageObject &rPage)
{
    // Save PageDescriptor.
    OStorePageData       &rData = rPage.getData();
    OStorePageDescriptor  aDescr (rData.m_aDescr);

#ifdef OSL_BIGENDIAN
    // Swap to external representation.
    rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Guard page.
    rPage.guard (aDescr);

    // Write page.
    storeError eErrCode = write (aDescr.m_nAddr, &rData, aDescr.m_nSize);

#ifdef OSL_BIGENDIAN
    // Swap back to internal representation.
    rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

    // Mark page as clean.
    if (eErrCode == store_E_None)
        rPage.clean();

    // Done.
    return eErrCode;
}

