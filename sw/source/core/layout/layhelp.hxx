/*************************************************************************
 *
 *  $RCSfile: layhelp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ama $ $Date: 2001-05-29 12:41:00 $
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
#ifndef _LAYHELP_HXX
#define _LAYHELP_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BYTES
#define _SVSTDARR_XUB_STRLEN
#include <svtools/svstdarr.hxx>
#endif

class SwDoc;
class SwFrm;
class SwLayoutFrm;
class SwPageFrm;
class SwSectionFrm;
class SwSectionNode;
class SwLayoutCache;
class SvStream;

/*************************************************************************
 *                      class SwLayCacheImpl
 * contains the page break information of the document (after loading)
 * and is used inside the constructor of the layout rootframe to
 * insert content at the right pages.
 * For every page of the main text (body content, no footnotes, text frames etc.)
 * we have the nodeindex of the first content at the page,
 * the type of content ( table or paragraph )
 * and if it's not the first part of the table/paragraph,
 * the row/character-offset inside the table/paragraph.
 *************************************************************************/

class SwLayCacheImpl : public SvULongs
{
    SvXub_StrLens aOffset;
    SvUShorts aType;

    void Insert( USHORT nType, ULONG nIndex, xub_StrLen nOffset );

public:
    SwLayCacheImpl() : SvULongs( 20, 10 ), aOffset( 20, 10 ), aType( 20, 10 ) {}
    BOOL Read( SvStream& rStream );

    ULONG GetBreakIndex( USHORT nIdx ) const { return GetObject( nIdx ); }
    xub_StrLen GetBreakOfst( USHORT nIdx ) const { return aOffset[ nIdx ]; }
    USHORT GetBreakType( USHORT nIdx ) const { return aType[ nIdx ]; }
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
    BOOL &rbBreakAfter;
    SwDoc* pDoc;
    SwLayCacheImpl* pImpl;
    ULONG nMaxParaPerPage;
    ULONG nParagraphCnt;
    ULONG nStartOfContent;
    USHORT nIndex;
    BOOL bFirst : 1;
public:
    SwLayHelper( SwDoc *pD, SwFrm* &rpF, SwFrm* &rpP, SwPageFrm* &rpPg,
            SwLayoutFrm* &rpL, SwActualSection* &rpA, BOOL &rBrk,
            ULONG nNodeIndex, BOOL bCache );
    ~SwLayHelper();
    ULONG CalcPageCount();
    BOOL CheckInsert( ULONG nNodeIndex );

    BOOL BreakPage( xub_StrLen& rOffs, ULONG nNodeIndex );
    BOOL CheckInsertPage();

};

/*************************************************************************
 *                      class SwLayCacheIoImpl
 * contains the data structures that are required to read and write a
 * layout cache.
 *************************************************************************/

#define SW_LAYCACHE_IO_REC_PAGES    'p'
#define SW_LAYCACHE_IO_REC_PARA     'P'
#define SW_LAYCACHE_IO_REC_TABLE    'T'

#define SW_LAYCACHE_IO_VERSION_MAJOR    1
#define SW_LAYCACHE_IO_VERSION_MINOR    0

class SwLayCacheIoImpl
{
    SvBytes         aRecTypes;
    SvULongs        aRecSizes;

    SvStream        *pStream;

    ULONG           nFlagRecEnd;

    USHORT          nMajorVersion;
    USHORT          nMinorVersion;

    BOOL            bWriteMode : 1;
    BOOL            bError : 1;

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
    void SkipRec();

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

#endif
