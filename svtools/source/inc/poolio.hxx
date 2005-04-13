/*************************************************************************
 *
 *  $RCSfile: poolio.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:03:43 $
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
#include "brdcst.hxx"


#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif


struct SfxPoolVersion_Impl
{
    USHORT          _nVer;
    USHORT          _nStart, _nEnd;
    USHORT*         _pMap;

                    SfxPoolVersion_Impl( USHORT nVer, USHORT nStart, USHORT nEnd,
                                         USHORT *pMap )
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

SV_DECL_PTRARR( SfxPoolItemArrayBase_Impl, SfxPoolItem*, 0, 5 )
SV_DECL_PTRARR_DEL( SfxPoolVersionArr_Impl, SfxPoolVersion_Impl*, 0, 2 );

struct SfxPoolItemArray_Impl: public SfxPoolItemArrayBase_Impl
{
    USHORT  nFirstFree;

    SfxPoolItemArray_Impl (USHORT nInitSize = 0)
        : SfxPoolItemArrayBase_Impl( nInitSize ),
          nFirstFree( 0 )
    {}
};

class SfxStyleSheetIterator;

struct SfxItemPool_Impl
{
    SfxBroadcaster                  aBC;
    SfxPoolItemArray_Impl**         ppPoolItems;
    SfxPoolVersionArr_Impl          aVersions;
    USHORT                          nVersion;
    USHORT                          nLoadingVersion;
    USHORT                          nInitRefCount; // 1, beim Laden ggf. 2
    USHORT                          nVerStart, nVerEnd; // WhichRange in Versions
    USHORT                          nStoringStart, nStoringEnd; // zu speichernder Range
    BYTE                            nMajorVer, nMinorVer; // Pool selbst
    SfxMapUnit                      eDefMetric;
    FASTBOOL                        bInSetItem;
    FASTBOOL                        bStreaming; // in Load() bzw. Store()

    SfxItemPool_Impl( USHORT nStart, USHORT nEnd )
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
        delete[] ppPoolItems; ppPoolItems = 0;
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
    {   USHORT nFileTag; \
        rStream >> nFileTag; \
        if ( nTag != nFileTag ) \
        { \
            DBG_ERROR( #nTag ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = FALSE; \
            return rStream; \
        } \
    }

#define CHECK_FILEFORMAT2( rStream, nTag1, nTag2 ) \
    {   USHORT nFileTag; \
        rStream >> nFileTag; \
        if ( nTag1 != nFileTag && nTag2 != nFileTag ) \
        { \
            DBG_ERROR( #nTag1 ); /*! s.u. */ \
            /*! error-code setzen und auswerten! */ \
            (rStream).SetError(SVSTREAM_FILEFORMAT_ERROR); \
            pImp->bStreaming = FALSE; \
            return rStream; \
        } \
    }

#define SFX_ITEMPOOL_VER_MAJOR          BYTE(2)
#define SFX_ITEMPOOL_VER_MINOR          BYTE(0)

#define SFX_ITEMPOOL_TAG_STARTPOOL_4    USHORT(0x1111)
#define SFX_ITEMPOOL_TAG_STARTPOOL_5    USHORT(0xBBBB)
#define SFX_ITEMPOOL_TAG_ITEMPOOL       USHORT(0xAAAA)
#define SFX_ITEMPOOL_TAG_ITEMS          USHORT(0x2222)
#define SFX_ITEMPOOL_TAG_ITEM           USHORT(0x7777)
#define SFX_ITEMPOOL_TAG_SIZES          USHORT(0x3333)
#define SFX_ITEMPOOL_TAG_DEFAULTS       USHORT(0x4444)
#define SFX_ITEMPOOL_TAG_VERSIONMAP     USHORT(0x5555)
#define SFX_ITEMPOOL_TAG_HEADER         USHORT(0x6666)
#define SFX_ITEMPOOL_TAG_ENDPOOL        USHORT(0xEEEE)
#define SFX_ITEMPOOL_TAG_TRICK4OLD      USHORT(0xFFFF)

#define SFX_ITEMPOOL_REC                BYTE(0x01)
#define SFX_ITEMPOOL_REC_HEADER         BYTE(0x10)
#define SFX_ITEMPOOL_REC_VERSIONMAP     USHORT(0x0020)
#define SFX_ITEMPOOL_REC_WHICHIDS       USHORT(0x0030)
#define SFX_ITEMPOOL_REC_ITEMS          USHORT(0x0040)
#define SFX_ITEMPOOL_REC_DEFAULTS       USHORT(0x0050)

#define SFX_ITEMSET_REC                 BYTE(0x02)

#define SFX_STYLES_REC                  BYTE(0x03)
#define SFX_STYLES_REC_HEADER       USHORT(0x0010)
#define SFX_STYLES_REC_STYLES       USHORT(0x0020)

//========================================================================

inline USHORT SfxItemPool::GetIndex_Impl(USHORT nWhich) const
{
    DBG_CHKTHIS(SfxItemPool, 0);
    DBG_ASSERT(nWhich >= nStart && nWhich <= nEnd, "Which-Id nicht im Pool-Bereich");
    return nWhich - nStart;
}

