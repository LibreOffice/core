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

#ifndef INCLUDED_SW_SOURCE_CORE_LAYOUT_LAYHELP_HXX
#define INCLUDED_SW_SOURCE_CORE_LAYOUT_LAYHELP_HXX

#include <swrect.hxx>
#include <vector>
#include <deque>

class SwDoc;
class SwFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SwSectionNode;
class SvStream;

/*
 * Contains the page break information and the text frame positions
 * of the document (after loading)
 * and is used inside the constructor of the layout rootframe to
 * insert content and text frames at the right pages.
 * For every page of the main text (body content, no footnotes, text frames etc.)
 * we have the nodeindex of the first content at the page,
 * the type of content ( table or paragraph )
 * and if it's not the first part of the table/paragraph,
 * the row/character-offset inside the table/paragraph.
 * The text frame positions are stored in the SwPageFlyCache array.
 */

class SwFlyCache;
typedef boost::ptr_vector<SwFlyCache> SwPageFlyCache;

class SwLayCacheImpl : public std::vector<sal_uLong>
{
    std::deque<sal_Int32> aOffset;
    std::vector<sal_uInt16> aType;
    SwPageFlyCache aFlyCache;
    bool bUseFlyCache;
    void Insert( sal_uInt16 nType, sal_uLong nIndex, sal_Int32 nOffset );

public:
    SwLayCacheImpl() : bUseFlyCache(false) {}
    bool Read( SvStream& rStream );

    sal_uLong GetBreakIndex( sal_uInt16 nIdx ) const { return std::vector<sal_uLong>::operator[]( nIdx ); }
    sal_Int32 GetBreakOfst( size_t nIdx ) const { return aOffset[ nIdx ]; }
    sal_uInt16 GetBreakType( sal_uInt16 nIdx ) const { return aType[ nIdx ]; }

    sal_uInt16 GetFlyCount() const { return aFlyCache.size(); }
    SwFlyCache *GetFlyCache( sal_uInt16 nIdx ) { return &aFlyCache[ nIdx ]; }

    bool IsUseFlyCache() const { return bUseFlyCache; }
};

// Helps to create the sectionframes during the _InsertCnt-function
// by controlling nested sections.
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

// Helps during the _InsertCnt-function to create new pages.
// If there's a layoutcache available, this information is used.
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
    bool bFirst : 1;
    void _CheckFlyCache( SwPageFrm* pPage );
public:
    SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
            SwLayoutFrm* &rpL, SwActualSection* &rpA, sal_Bool &rBrk,
            sal_uLong nNodeIndex, bool bCache );
    ~SwLayHelper();
    sal_uLong CalcPageCount();
    bool CheckInsert( sal_uLong nNodeIndex );

    bool CheckInsertPage();

    /// Look for fresh text frames at this (new) page and set them to the right
    /// position, if they are in the fly cache.
    void CheckFlyCache( SwPageFrm* pPage )
    { if( pImpl && nFlyIdx < pImpl->GetFlyCount() ) _CheckFlyCache( pPage ); }

    /// Look for this text frame and set it to the right position,
    /// if it's in the fly cache.
    static bool CheckPageFlyCache( SwPageFrm* &rpPage, SwFlyFrm* pFly );
};

// Contains the data structures that are required to read and write a layout cache.
#define SW_LAYCACHE_IO_REC_PAGES    'p'
#define SW_LAYCACHE_IO_REC_PARA     'P'
#define SW_LAYCACHE_IO_REC_TABLE    'T'
#define SW_LAYCACHE_IO_REC_FLY      'F'

#define SW_LAYCACHE_IO_VERSION_MAJOR    1
#define SW_LAYCACHE_IO_VERSION_MINOR    1

class SwLayCacheIoImpl
{
private:
    struct RecTypeSize {
        sal_uInt8 type;
        sal_uLong size;
        RecTypeSize(sal_uInt8 typ, sal_uLong siz) : type(typ), size(siz) {}
    };
    std::vector<RecTypeSize> aRecords;

    SvStream        *pStream;

    sal_uLong           nFlagRecEnd;

    sal_uInt16          nMajorVersion;
    sal_uInt16          nMinorVersion;

    bool            bWriteMode : 1;
    bool            bError : 1;

public:
    SwLayCacheIoImpl( SvStream& rStrm, bool bWrtMd );

    /// Get input or output stream
    SvStream& GetStream() const { return *pStream; }

    /// Open a record of type "nType"
    bool OpenRec( sal_uInt8 nType );

    /// Close a record of type "nType". This skips any unread data that
    /// remains in the record.
    bool CloseRec( sal_uInt8 nType );

    /// Return the number of bytes contained in the current record that
    /// haven't been read by now.
    sal_uInt32 BytesLeft();

    /// Return the current record's type
    sal_uInt8 Peek();

    /// Skip the current record
    void SkipRec();

    /// Open a flag record for reading. The uppermost four bits are flags,
    /// while the lowermost are the flag record's size. Flag records cannot
    /// be nested.
    sal_uInt8 OpenFlagRec();

    /// Open flag record for writing;
    void OpenFlagRec( sal_uInt8 nFlags, sal_uInt8 nLen );

    /// Close a flag record. Any bytes left are skipped.
    void CloseFlagRec();

    bool HasError() const { return bError; }

    sal_uInt16 GetMajorVersion() const { return nMajorVersion; }
    sal_uInt16 GetMinorVersion() const { return nMinorVersion; }
};

// Stored information about text frames:
class SwFlyCache : public SwRect // position and size
{
public:
    sal_uLong nOrdNum;      ///< Id to recognize text frames
    sal_uInt16 nPageNum;    ///< page number
    SwFlyCache( sal_uInt16 nP, sal_uLong nO, long nXL, long nYL, long nWL, long nHL ) :
        SwRect( nXL, nYL, nWL, nHL ), nOrdNum( nO ), nPageNum( nP ){}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
