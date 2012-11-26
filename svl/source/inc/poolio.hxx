/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    int                         bInSetItem; // !CAUTION! This should be a bool, but is used as int in poolio.cxx(205)
    bool                        bStreaming; // in Load() bzw. Store()

    SfxItemPool_Impl( sal_uInt16 nStart, sal_uInt16 nEnd )
        : ppPoolItems (new SfxPoolItemArray_Impl*[ nEnd - nStart + 1])
        , nLoadingVersion(0)
        , nInitRefCount(0)
        , nVerStart(0)
        , nVerEnd(0)
        , nStoringStart(0)
        , nStoringEnd(0)
        , nMajorVer(0)
        , nMinorVer(0)
        , bInSetItem(false)
        , bStreaming(false)
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

