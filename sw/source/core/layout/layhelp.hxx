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

#include <tools/solar.h>

#include <memory>
#include <vector>
#include <deque>

class SwDoc;
class SwFrame;
class SwLayoutFrame;
class SwPageFrame;
class SwSectionFrame;
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
typedef std::vector<SwFlyCache> SwPageFlyCache;

class SwLayCacheImpl
{
    std::vector<SwNodeOffset> mIndices;
    /// either a textframe character offset, or a row index inside a table
    std::deque<sal_Int32> m_aOffset;
    std::vector<sal_uInt16> m_aType;
    SwPageFlyCache m_FlyCache;
    bool m_bUseFlyCache;
    void Insert( sal_uInt16 nType, SwNodeOffset nIndex, sal_Int32 nOffset );

public:
    inline SwLayCacheImpl();

    size_t size() const { return mIndices.size(); }

    bool Read( SvStream& rStream );

    SwNodeOffset GetBreakIndex( size_t nIdx ) const { return mIndices[ nIdx ]; }
    sal_Int32 GetBreakOfst( size_t nIdx ) const { return m_aOffset[ nIdx ]; }
    sal_uInt16 GetBreakType( size_t nIdx ) const { return m_aType[ nIdx ]; }

    inline size_t GetFlyCount() const;
    inline SwFlyCache& GetFlyCache( size_t nIdx );

    bool IsUseFlyCache() const { return m_bUseFlyCache; }
};

// Helps to create the sectionframes during the InsertCnt_-function
// by controlling nested sections.
class SwActualSection
{
    SwActualSection *m_pUpper;
    SwSectionFrame    *m_pSectFrame;
    SwSectionNode   *m_pSectNode;
public:
    SwActualSection( SwActualSection *pUpper,
                     SwSectionFrame    *pSect,
                     SwSectionNode   *pNd );

    SwSectionFrame    *GetSectionFrame()                    { return m_pSectFrame; }
    void             SetSectionFrame( SwSectionFrame *p )   { m_pSectFrame = p; }
    SwSectionNode   *GetSectionNode()                   { return m_pSectNode;}
    void             SetUpper(SwActualSection *p)       { m_pUpper = p; }
    SwActualSection *GetUpper()                         { return m_pUpper; }
};

/// Helps during the InsertCnt_ function to create new pages.
/// If there's a layout cache available, this information is used.
class SwLayHelper
{
    SwFrame* &mrpFrame;
    SwFrame* &mrpPrv;
    SwPageFrame* &mrpPage;
    SwLayoutFrame* &mrpLay;
    std::unique_ptr<SwActualSection> &mrpActualSection;
    bool mbBreakAfter;
    SwDoc* mpDoc;
    SwLayCacheImpl* mpImpl;
    sal_uLong mnMaxParaPerPage;
    sal_uLong mnParagraphCnt;
    SwNodeOffset mnStartOfContent;
    size_t mnIndex;                          ///< the index in the page break array
    size_t mnFlyIdx;                         ///< the index in the fly cache array
    bool mbFirst : 1;
    void CheckFlyCache_( SwPageFrame* pPage );
public:
    SwLayHelper( SwDoc *pD, SwFrame* &rpF, SwFrame* &rpP, SwPageFrame* &rpPg,
            SwLayoutFrame* &rpL, std::unique_ptr<SwActualSection> &rpA,
            SwNodeOffset nNodeIndex, bool bCache );
    ~SwLayHelper();
    sal_uLong CalcPageCount();
    bool CheckInsert( SwNodeOffset nNodeIndex );

    bool CheckInsertPage();

    /// Look for fresh text frames at this (new) page and set them to the right
    /// position, if they are in the fly cache.
    void CheckFlyCache( SwPageFrame* pPage )
    { if( mpImpl && mnFlyIdx < mpImpl->GetFlyCount() ) CheckFlyCache_( pPage ); }
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
    std::vector<RecTypeSize> m_aRecords;

    SvStream        *m_pStream;

    sal_uLong           m_nFlagRecEnd;

    sal_uInt16          m_nMajorVersion;
    sal_uInt16          m_nMinorVersion;

    bool            m_bWriteMode : 1;
    bool            m_bError : 1;

public:
    SwLayCacheIoImpl( SvStream& rStrm, bool bWrtMd );

    /// Get input or output stream
    SvStream& GetStream() const { return *m_pStream; }

    /// Open a record of type "nType"
    void OpenRec( sal_uInt8 nType );

    /// Close a record. This skips any unread data that
    /// remains in the record.
    void CloseRec();

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

    bool HasError() const { return m_bError; }

    sal_uInt16 GetMajorVersion() const { return m_nMajorVersion; }
    sal_uInt16 GetMinorVersion() const { return m_nMinorVersion; }
};

// Stored information about text frames:
class SwFlyCache : public SwRect // position and size
{
public:
    sal_uLong nOrdNum;      ///< Id to recognize text frames
    sal_uInt16 nPageNum;    ///< page number
    SwFlyCache( sal_uInt16 nP, sal_uLong nO, tools::Long nXL, tools::Long nYL, tools::Long nWL, tools::Long nHL ) :
        SwRect( nXL, nYL, nWL, nHL ), nOrdNum( nO ), nPageNum( nP ){}
};

SwLayCacheImpl::SwLayCacheImpl() : m_bUseFlyCache(false) {}

size_t SwLayCacheImpl::GetFlyCount() const { return m_FlyCache.size(); }

SwFlyCache& SwLayCacheImpl::GetFlyCache( size_t nIdx ) { return m_FlyCache[ nIdx ]; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
