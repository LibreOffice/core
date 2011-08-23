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
#ifndef _LAYHELP_HXX
#define _LAYHELP_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BYTES
#define _SVSTDARR_XUB_STRLEN
#include <bf_svtools/svstdarr.hxx>
#endif
#include <swrect.hxx>
class SvStream; 
namespace binfilter {

class SwDoc;
class SwFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SwSectionNode;
class SwLayoutCache;


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
    void Insert( USHORT nType, ULONG nIndex, xub_StrLen nOffset );

public:
    SwLayCacheImpl() : SvULongs( 20, 10 ), aOffset( 20, 10 ), aType( 20, 10 ) {}
    BOOL Read( SvStream& rStream );

    ULONG GetBreakIndex( USHORT nIdx ) const { return GetObject( nIdx ); }
    xub_StrLen GetBreakOfst( USHORT nIdx ) const { return aOffset[ nIdx ]; }
    USHORT GetBreakType( USHORT nIdx ) const { return aType[ nIdx ]; }

    USHORT GetFlyCount() const { return aFlyCache.Count(); }
    SwFlyCache *GetFlyCache( USHORT nIdx ) const { return aFlyCache[ nIdx ]; }

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
    SwSectionFrm	*pSectFrm;
    SwSectionNode	*pSectNode;
public:
    SwActualSection( SwActualSection *pUpper,
                     SwSectionFrm	 *pSect,
                     SwSectionNode	 *pNd );

    SwSectionFrm	*GetSectionFrm()					{ return pSectFrm; }
    void			 SetSectionFrm( SwSectionFrm *p )	{ pSectFrm = p; }
    SwSectionNode   *GetSectionNode()					{ return pSectNode;}
    SwActualSection *GetUpper()							{ return pUpper; }
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
    BOOL &rbBreakAfter;
    SwDoc* pDoc;
    SwLayCacheImpl* pImpl;
    ULONG nMaxParaPerPage;
    ULONG nParagraphCnt;
    ULONG nStartOfContent;
    USHORT nIndex;                      // the index in the page break array
    USHORT nFlyIdx;                     // the index in the fly cache array
    BOOL bFirst : 1;
    void _CheckFlyCache( SwPageFrm* pPage );
public:
    SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
            SwLayoutFrm* &rpL, SwActualSection* &rpA, BOOL &rBrk,
            ULONG nNodeIndex, BOOL bCache );
    ~SwLayHelper();
    ULONG CalcPageCount();
    BOOL CheckInsert( ULONG nNodeIndex );

    BOOL BreakPage( xub_StrLen& rOffs, ULONG nNodeIndex );
    BOOL CheckInsertPage();

    // Look for fresh text frames at this (new) page and set them to the right
    // position, if they are in the fly cache.
    void CheckFlyCache( SwPageFrm* pPage )
    { if( pImpl && nFlyIdx < pImpl->GetFlyCount() ) _CheckFlyCache( pPage ); }

    // Look for this text frame and set it to the right position,
    // if it's in the fly cache.
    static BOOL CheckPageFlyCache( SwPageFrm* &rpPage, SwFlyFrm* pFly );
};

/*************************************************************************
 *                      class SwLayCacheIoImpl
 * contains the data structures that are required to read and write a
 * layout cache.
 *************************************************************************/

#define SW_LAYCACHE_IO_REC_PAGES	'p'
#define SW_LAYCACHE_IO_REC_PARA		'P'
#define SW_LAYCACHE_IO_REC_TABLE	'T'
#define SW_LAYCACHE_IO_REC_FLY      'F'

#define SW_LAYCACHE_IO_VERSION_MAJOR	1
#define SW_LAYCACHE_IO_VERSION_MINOR    1

class SwLayCacheIoImpl
{
    SvBytes		   	aRecTypes;
    SvULongs		aRecSizes;

    SvStream       	*pStream;

    ULONG			nFlagRecEnd;

    USHORT			nMajorVersion;
    USHORT 			nMinorVersion;

    BOOL			bWriteMode : 1;
    BOOL			bError : 1;

public:
    SwLayCacheIoImpl( SvStream& rStrm, BOOL bWrtMd );

    // Get input or output stream
    SvStream& GetStream() const { return *pStream; }

    // Open a record of type "nType"
    BOOL OpenRec( BYTE nType );

    // Close a record of type "nType". This skips any unread data that
    // remains in the record.
    BOOL CloseRec( BYTE nType );

    // Return the number of bytes contained in the current record that
    // haven't been read by now.
    UINT32 BytesLeft();

    // Return the current record's type
    BYTE Peek();

    // Skip the current record

    // Open a flag record for reading. The uppermost four bits are flags,
    // while the lowermost are the flag record's size. Flag records cannot
    // be nested.
    BYTE OpenFlagRec();

    // Open flag record for writing;
    void OpenFlagRec( BYTE nFlags, BYTE nLen );

    // Close a flag record. Any bytes left are skipped.
    void CloseFlagRec();

    BOOL HasError() const { return bError; }

    USHORT GetMajorVersion() const { return nMajorVersion; }
    USHORT GetMinorVersion() const { return nMinorVersion; }
};

// Stored information about text frames:
class SwFlyCache : public SwRect // position and size
{
public:
    ULONG nOrdNum;      // Id to recognize text frames
    USHORT nPageNum;    // page number
    SwFlyCache( USHORT nP, ULONG nO, long nX, long nY, long nW, long nH ) :
        SwRect( nX, nY, nW, nH ), nOrdNum( nO ), nPageNum( nP ){}
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
