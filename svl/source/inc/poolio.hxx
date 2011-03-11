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
#include <svl/brdcst.hxx>
#include <boost/shared_ptr.hpp>
#include <deque>

#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif


struct SfxPoolVersion_Impl
{
    sal_uInt16          _nVer;
    sal_uInt16          _nStart, _nEnd;
    sal_uInt16*         _pMap;

                    SfxPoolVersion_Impl( sal_uInt16 nVer, sal_uInt16 nStart, sal_uInt16 nEnd,
                                         sal_uInt16 *pMap )
                    :   _nVer( nVer ),
                        _nStart( nStart ),
                        _nEnd( nEnd ),
                        _pMap( pMap )
                    {}
                    SfxPoolVersion_Impl( const SfxPoolVersion_Impl &rOrig )
                    :   _nVer( rOrig._nVer ),
                        _nStart( rOrig._nStart ),
                        _nEnd( rOrig._nEnd ),
                        _pMap( rOrig._pMap )
                    {}
};

typedef std::deque<SfxPoolItem*> SfxPoolItemArrayBase_Impl;

typedef boost::shared_ptr< SfxPoolVersion_Impl > SfxPoolVersion_ImplPtr;
typedef std::deque< SfxPoolVersion_ImplPtr > SfxPoolVersionArr_Impl;

struct SfxPoolItemArray_Impl: public SfxPoolItemArrayBase_Impl
{
    size_t  nFirstFree;

    SfxPoolItemArray_Impl ()
        : nFirstFree( 0 )
    {}
};

class SfxStyleSheetIterator;

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    SfxPoolItemArray_Impl**         ppPoolItems;
    SfxPoolVersionArr_Impl          aVersions;
    sal_uInt16                          nVersion;
    sal_uInt16                          nLoadingVersion;
    sal_uInt16                          nInitRefCount; // 1, beim Laden ggf. 2
    sal_uInt16                          nVerStart, nVerEnd; // WhichRange in Versions
    sal_uInt16                          nStoringStart, nStoringEnd; // zu speichernder Range
    sal_uInt8                           nMajorVer, nMinorVer; // Pool selbst
    SfxMapUnit                      eDefMetric;
    bool                            bInSetItem;
    bool                            bStreaming; // in Load() bzw. Store()

    SfxItemPool_Impl( sal_uInt16 nStart, sal_uInt16 nEnd )
        : ppPoolItems (new SfxPoolItemArray_Impl*[ nEnd - nStart + 1])
    {
        memset( ppPoolItems, 0, sizeof( SfxPoolItemArray_Impl* ) * ( nEnd - nStart + 1) );
    }

    ~SfxItemPool_Impl()
    {
        delete[] ppPoolItems;
    }

    void DeleteItems()
    {
        delete[] ppPoolItems;
        ppPoolItems = 0;
    }
};

// -----------------------------------------------------------------------

// IBM-C-Set mag keine doppelten Defines
#ifdef DBG
#  undef DBG
#endif

#if defined(DBG_UTIL) && defined(MSC)
#define SFX_TRACE(s,p) \
        { \
            ByteString aPtr(RTL_CONSTASCII_STRINGPARAM("0x0000:0x0000")); \
            _snprintf(const_cast< sal_Char *>(aPtr.GetBuffer()), aPtr.Len(), \
                       "%lp", p ); \
            aPtr.Insert(s, 0); \
            DbgTrace( aPtr.GetBuffer() ); \
        }
#define DBG(x) x
#else
#define SFX_TRACE(s,p)
#define DBG(x)
#endif

#define CHECK_FILEFORMAT( rStream, nTag ) \
    {   sal_uInt16 nFileTag; \
        rStream >> nFileTag; \
        if ( nTag != nFileTag ) \
        { \
            DBG_ERROR( #nTag ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = sal_False; \
            return rStream; \
        } \
    }

#define CHECK_FILEFORMAT_RELEASE( rStream, nTag, pPointer ) \
   {   sal_uInt16 nFileTag; \
       rStream >> nFileTag; \
       if ( nTag != nFileTag ) \
        { \
            DBG_ERROR( #nTag ); /*! s.u. */ \
           /*! error-code setzen und auswerten! */ \
           (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
           pImp->bStreaming = sal_False; \
           delete pPointer; \
            return rStream; \
        } \
    }

#define CHECK_FILEFORMAT2( rStream, nTag1, nTag2 ) \
    {   sal_uInt16 nFileTag; \
        rStream >> nFileTag; \
        if ( nTag1 != nFileTag && nTag2 != nFileTag ) \
        { \
            DBG_ERROR( #nTag1 ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = sal_False; \
            return rStream; \
        } \
    }

#define SFX_ITEMPOOL_VER_MAJOR          sal_uInt8(2)
#define SFX_ITEMPOOL_VER_MINOR          sal_uInt8(0)

#define SFX_ITEMPOOL_TAG_STARTPOOL_4    sal_uInt16(0x1111)
#define SFX_ITEMPOOL_TAG_STARTPOOL_5    sal_uInt16(0xBBBB)
#define SFX_ITEMPOOL_TAG_ITEMPOOL       sal_uInt16(0xAAAA)
#define SFX_ITEMPOOL_TAG_ITEMS          sal_uInt16(0x2222)
#define SFX_ITEMPOOL_TAG_ITEM           sal_uInt16(0x7777)
#define SFX_ITEMPOOL_TAG_SIZES          sal_uInt16(0x3333)
#define SFX_ITEMPOOL_TAG_DEFAULTS       sal_uInt16(0x4444)
#define SFX_ITEMPOOL_TAG_VERSIONMAP     sal_uInt16(0x5555)
#define SFX_ITEMPOOL_TAG_HEADER         sal_uInt16(0x6666)
#define SFX_ITEMPOOL_TAG_ENDPOOL        sal_uInt16(0xEEEE)
#define SFX_ITEMPOOL_TAG_TRICK4OLD      sal_uInt16(0xFFFF)

#define SFX_ITEMPOOL_REC                sal_uInt8(0x01)
#define SFX_ITEMPOOL_REC_HEADER         sal_uInt8(0x10)
#define SFX_ITEMPOOL_REC_VERSIONMAP     sal_uInt16(0x0020)
#define SFX_ITEMPOOL_REC_WHICHIDS       sal_uInt16(0x0030)
#define SFX_ITEMPOOL_REC_ITEMS          sal_uInt16(0x0040)
#define SFX_ITEMPOOL_REC_DEFAULTS       sal_uInt16(0x0050)

#define SFX_ITEMSET_REC                 sal_uInt8(0x02)

#define SFX_STYLES_REC                  sal_uInt8(0x03)
#define SFX_STYLES_REC_HEADER       sal_uInt16(0x0010)
#define SFX_STYLES_REC_STYLES       sal_uInt16(0x0020)

//========================================================================

inline sal_uInt16 SfxItemPool::GetIndex_Impl(sal_uInt16 nWhich) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(nWhich >= nStart && nWhich <= nEnd, "Which-Id nicht im Pool-Bereich");
    return nWhich - nStart;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
