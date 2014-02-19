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
#include <svl/brdcst.hxx>
#include <boost/shared_ptr.hpp>
#include <deque>
#include <vector>

#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif


struct SfxPoolVersion_Impl
{
    sal_uInt16          _nVer;
    sal_uInt16          _nStart, _nEnd;
    const sal_uInt16*         _pMap;

                    SfxPoolVersion_Impl( sal_uInt16 nVer, sal_uInt16 nStart, sal_uInt16 nEnd,
                                         const sal_uInt16 *pMap )
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

typedef std::vector<SfxPoolItem*> SfxPoolItemArrayBase_Impl;

typedef boost::shared_ptr< SfxPoolVersion_Impl > SfxPoolVersion_ImplPtr;
typedef std::deque< SfxPoolVersion_ImplPtr > SfxPoolVersionArr_Impl;

struct SfxPoolItemArray_Impl: public SfxPoolItemArrayBase_Impl
{
    size_t  nFirstFree;

    SfxPoolItemArray_Impl ()
        : nFirstFree( 0 )
    {}
};

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    std::vector<SfxPoolItemArray_Impl*> maPoolItems;
    std::vector<SfxItemPoolUser*> maSfxItemPoolUsers; /// ObjectUser section
    OUString                   aName;
    SfxPoolItem**                   ppPoolDefaults;
    SfxPoolItem**                   ppStaticDefaults;
    SfxItemPool*                    mpMaster;
    SfxItemPool*                    mpSecondary;
    sal_uInt16*                     mpPoolRanges;
    SfxPoolVersionArr_Impl          aVersions;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    sal_uInt16                      mnFileFormatVersion;
    sal_uInt16                          nVersion;
    sal_uInt16                          nLoadingVersion;
    sal_uInt16                          nInitRefCount; // 1, beim Laden ggf. 2
    sal_uInt16                          nVerStart, nVerEnd; // WhichRange in Versions
    sal_uInt16                          nStoringStart, nStoringEnd; // zu speichernder Range
    sal_uInt8                           nMajorVer, nMinorVer; // Pool selbst
    SfxMapUnit                      eDefMetric;
    bool                            bInSetItem;
    bool                            bStreaming; // in Load() bzw. Store()
    bool                            mbPersistentRefCounts;

    SfxItemPool_Impl( SfxItemPool* pMaster, const OUString& rName, sal_uInt16 nStart, sal_uInt16 nEnd )
        : maPoolItems(nEnd - nStart + 1, static_cast<SfxPoolItemArray_Impl*>(NULL))
        , aName(rName)
        , ppPoolDefaults(new SfxPoolItem* [nEnd - nStart + 1])
        , ppStaticDefaults(0)
        , mpMaster(pMaster)
        , mpSecondary(NULL)
        , mpPoolRanges(NULL)
        , mnStart(nStart)
        , mnEnd(nEnd)
        , mnFileFormatVersion(0)
        , nVersion(0)
        , nLoadingVersion(0)
        , nInitRefCount(0)
        , nVerStart(0)
        , nVerEnd(0)
        , nStoringStart(0)
        , nStoringEnd(0)
        , nMajorVer(0)
        , nMinorVer(0)
        , eDefMetric(SFX_MAPUNIT_CM)
        , bInSetItem(false)
        , bStreaming(false)
        , mbPersistentRefCounts(false)
    {
        DBG_ASSERT(mnStart, "Start-Which-Id must be greater 0" );
        memset( ppPoolDefaults, 0, sizeof( SfxPoolItem* ) * (nEnd - nStart + 1));
    }

    ~SfxItemPool_Impl()
    {
        DeleteItems();
    }

    void DeleteItems()
    {
        std::vector<SfxPoolItemArray_Impl*>::iterator itr = maPoolItems.begin(), itrEnd = maPoolItems.end();
        for (; itr != itrEnd; ++itr)
            delete *itr;
        maPoolItems.clear();

        delete[] mpPoolRanges;
        mpPoolRanges = NULL;
        delete[] ppPoolDefaults;
        ppPoolDefaults = NULL;
    }

    void readTheItems(SvStream & rStream, sal_uInt32 nCount, sal_uInt16 nVersion,
                      SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** pArr);
};

// -----------------------------------------------------------------------

// IBM-C-Set mag keine doppelten Defines
#ifdef DBG
#  undef DBG
#endif

#if defined(DBG_UTIL) && defined(_MSC_VER)
#define DBG(x) x
#else
#define DBG(x)
#endif

#define CHECK_FILEFORMAT( rStream, nTag ) \
    {   sal_uInt16 nFileTag; \
        rStream.ReadUInt16( nFileTag ); \
        if ( nTag != nFileTag ) \
        { \
            OSL_FAIL( #nTag ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = false; \
            return rStream; \
        } \
    }

#define CHECK_FILEFORMAT_RELEASE( rStream, nTag, pPointer ) \
   {   sal_uInt16 nFileTag; \
       rStream.ReadUInt16( nFileTag ); \
       if ( nTag != nFileTag ) \
        { \
           OSL_FAIL( #nTag ); /*! s.u. */ \
           /*! error-code setzen und auswerten! */ \
           (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
           pImp->bStreaming = false; \
           delete pPointer; \
            return rStream; \
        } \
    }

#define CHECK_FILEFORMAT2( rStream, nTag1, nTag2 ) \
    {   sal_uInt16 nFileTag; \
        rStream.ReadUInt16( nFileTag ); \
        if ( nTag1 != nFileTag && nTag2 != nFileTag ) \
        { \
            OSL_FAIL( #nTag1 ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = false; \
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
