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
#ifndef _LAYHELP_HXX
#define _LAYHELP_HXX
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BYTES
#define _SVSTDARR_XUB_STRLEN
#include <svl/svstdarr.hxx>
#endif
#include <swrect.hxx>

class SwDoc;
class SwFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SwSectionNode;
class SvStream;

/*************************************************************************
 *                      class SwLayCacheImpl
 * contains the page break information and the text frame positions
 * of the document (after loading)
 * and is used inside the constructor of the layout rootframe to
 * insert content and text frames at the right pages.
 * For every page of the main text (body content, no footnotes, text frames etc.)
 * we have the nodeindex of the first content at the page,
 * the type of content ( table or paragraph )
 * and if it's not the first part of the table/paragraph,
 * the row/character-offset inside the table/paragraph.
 * The text frame positions are stored in the SwPageFlyCache array.
 *************************************************************************/

class SwFlyCache;
typedef SwFlyCache* SwFlyCachePtr;
SV_DECL_PTRARR_DEL( SwPageFlyCache, SwFlyCachePtr, 0, 4 )

class SwLayCacheImpl : public SvULongs
{
    SvXub_StrLens aOffset;
    SvUShorts aType;
    SwPageFlyCache aFlyCache;
    sal_Bool bUseFlyCache;
    void Insert( sal_uInt16 nType, sal_uLong nIndex, xub_StrLen nOffset );

public:
    SwLayCacheImpl() : SvULongs( 20, 10 ), aType( 20, 10 ) {}
    sal_Bool Read( SvStream& rStream );

    sal_uLong GetBreakIndex( sal_uInt16 nIdx ) const { return GetObject( nIdx ); }
    xub_StrLen GetBreakOfst( size_t nIdx ) const { return aOffset[ nIdx ]; }
    sal_uInt16 GetBreakType( sal_uInt16 nIdx ) const { return aType[ nIdx ]; }

    sal_uInt16 GetFlyCount() const { return aFlyCache.Count(); }
    SwFlyCache *GetFlyCache( sal_uInt16 nIdx ) const { return aFlyCache[ nIdx ]; }

    sal_Bool IsUseFlyCache() const { return bUseFlyCache; }
};

/*************************************************************************
 *                      class SwActualSection
 * helps to create the sectionframes during the _InsertCnt-function
 * by controlling nested sections.
 *************************************************************************/

class SwActualSection
{
    SwActualSection *pUpper;
    SwSectionFrm    *pSectFrm;
    SwSectionNode   *pSectNode;
public:
    SwActualSection( SwActualSection *pUpper,
                     SwSectionFrm    *pSect,
                     SwSectionNode   *pNd );

    SwSectionFrm    *GetSectionFrm()                    { return pSectFrm; }
    void             SetSectionFrm( SwSectionFrm *p )   { pSectFrm = p; }
    SwSectionNode   *GetSectionNode()                   { return pSectNode;}
    SwActualSection *GetUpper()                         { return pUpper; }
};

/*************************************************************************
 *                      class SwLayHelper
 * helps during the _InsertCnt-function to create new pages.
 * If there's a layoutcache available, this information is used.
 *************************************************************************/

class SwLayHelper
{
    SwFrm* &rpFrm;
    SwFrm* &rpPrv;
    SwPageFrm* &rpPage;
    SwLayoutFrm* &rpLay;
    SwActualSection* &rpActualSection;
    sal_Bool &rbBreakAfter;
    SwDoc* pDoc;
    SwLayCacheImpl* pImpl;
    sal_uLong nMaxParaPerPage;
    sal_uLong nParagraphCnt;
    sal_uLong nStartOfContent;
    sal_uInt16 nIndex;                      // the index in the page break array
    sal_uInt16 nFlyIdx;                     // the index in the fly cache array
    sal_Bool bFirst : 1;
    void _CheckFlyCache( SwPageFrm* pPage );
public:
    SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
            SwLayoutFrm* &rpL, SwActualSection* &rpA, sal_Bool &rBrk,
            sal_uLong nNodeIndex, sal_Bool bCache );
    ~SwLayHelper();
    sal_uLong CalcPageCount();
    sal_Bool CheckInsert( sal_uLong nNodeIndex );

    sal_Bool BreakPage( xub_StrLen& rOffs, sal_uLong nNodeIndex );
    sal_Bool CheckInsertPage();

    // Look for fresh text frames at this (new) page and set them to the right
    // position, if they are in the fly cache.
    void CheckFlyCache( SwPageFrm* pPage )
    { if( pImpl && nFlyIdx < pImpl->GetFlyCount() ) _CheckFlyCache( pPage ); }

    // Look for this text frame and set it to the right position,
    // if it's in the fly cache.
    static sal_Bool CheckPageFlyCache( SwPageFrm* &rpPage, SwFlyFrm* pFly );
};

/*************************************************************************
 *                      class SwLayCacheIoImpl
 * contains the data structures that are required to read and write a
 * layout cache.
 *************************************************************************/

#define SW_LAYCACHE_IO_REC_PAGES    'p'
#define SW_LAYCACHE_IO_REC_PARA     'P'
#define SW_LAYCACHE_IO_REC_TABLE    'T'
#define SW_LAYCACHE_IO_REC_FLY      'F'

#define SW_LAYCACHE_IO_VERSION_MAJOR    1
#define SW_LAYCACHE_IO_VERSION_MINOR    1

class SwLayCacheIoImpl
{
    SvBytes         aRecTypes;
    SvULongs        aRecSizes;

    SvStream        *pStream;

    sal_uLong           nFlagRecEnd;

    sal_uInt16          nMajorVersion;
    sal_uInt16          nMinorVersion;

    sal_Bool            bWriteMode : 1;
    sal_Bool            bError : 1;

public:
    SwLayCacheIoImpl( SvStream& rStrm, sal_Bool bWrtMd );

    // Get input or output stream
    SvStream& GetStream() const { return *pStream; }

    // Open a record of type "nType"
    sal_Bool OpenRec( sal_uInt8 nType );

    // Close a record of type "nType". This skips any unread data that
    // remains in the record.
    sal_Bool CloseRec( sal_uInt8 nType );

    // Return the number of bytes contained in the current record that
    // haven't been read by now.
    sal_uInt32 BytesLeft();

    // Return the current record's type
    sal_uInt8 Peek();

    // Skip the current record
    void SkipRec();

    // Open a flag record for reading. The uppermost four bits are flags,
    // while the lowermost are the flag record's size. Flag records cannot
    // be nested.
    sal_uInt8 OpenFlagRec();

    // Open flag record for writing;
    void OpenFlagRec( sal_uInt8 nFlags, sal_uInt8 nLen );

    // Close a flag record. Any bytes left are skipped.
    void CloseFlagRec();

    sal_Bool HasError() const { return bError; }

    sal_uInt16 GetMajorVersion() const { return nMajorVersion; }
    sal_uInt16 GetMinorVersion() const { return nMinorVersion; }
};

// Stored information about text frames:
class SwFlyCache : public SwRect // position and size
{
public:
    sal_uLong nOrdNum;      // Id to recognize text frames
    sal_uInt16 nPageNum;    // page number
    SwFlyCache( sal_uInt16 nP, sal_uLong nO, long nXL, long nYL, long nWL, long nHL ) :
        SwRect( nXL, nYL, nWL, nHL ), nOrdNum( nO ), nPageNum( nP ){}
};

#endif
