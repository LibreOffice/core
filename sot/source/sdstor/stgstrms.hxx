/*************************************************************************
 *
 *  $RCSfile: stgstrms.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:52 $
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

#ifndef _STGSTRMS_HXX
#define _STGSTRMS_HXX

#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif

class StgIo;
class StgStrm;
class StgPage;
class StgDirEntry;

// The FAT class performs FAT operations on an underlying storage stream.
// This stream is either the physical FAT stream (bPhys == TRUE ) or a normal
// storage stream, which then holds the FAT for small data allocations.

class StgFAT
{                                       // FAT allocator
    StgStrm& rStrm;                     // underlying stream
    INT32 nMaxPage;                     // highest page allocated so far
    short nPageSize;                    // physical page size
    short nEntries;                     // FAT entries per page
    short nOffset;                      // current offset within page
    INT32 nLimit;                       // search limit recommendation
    BOOL  bPhys;                        // TRUE: physical FAT
    StgPage* GetPhysPage( INT32 nPage );
    BOOL  MakeChain( INT32 nStart, INT32 nPages );
    BOOL  InitNew( INT32 nPage1 );
public:
    StgFAT( StgStrm& rStrm, BOOL bMark );
    INT32 FindBlock( INT32& nPages );
    INT32 GetNextPage( INT32 nPg );
    INT32 AllocPages( INT32 nStart, INT32 nPages );
    BOOL  FreePages( INT32 nStart, BOOL bAll );
    INT32 GetMaxPage() { return nMaxPage; }
    void  SetLimit( INT32 n ) { nLimit = n; }
};

// The base stream class provides basic functionality for seeking
// and accessing the data on a physical basis. It uses the built-in
// FAT class for the page allocations.

class StgStrm {                         // base class for all streams
protected:
    StgIo& rIo;                         // I/O system
    StgFAT* pFat;                       // FAT stream for allocations
    StgDirEntry* pEntry;                // dir entry (for ownership)
    INT32 nStart;                       // 1st data page
    INT32 nSize;                        // stream size in bytes
    INT32 nPos;                         // current byte position
    INT32 nPage;                        // current logical page
    short nOffset;                      // offset into current page
    short nPageSize;                    // logical page size
    BOOL  Copy( INT32 nFrom, INT32 nBytes );
    StgStrm( StgIo& );
public:
    ~StgStrm();
    StgIo&  GetIo()     { return rIo;    }
    INT32   GetPos()    { return nPos;   }
    INT32   GetStart()  { return nStart; }
    INT32   GetSize()   { return nSize;  }
    INT32   GetPage()   { return nPage;  }
    short   GetPageSize() { return nPageSize; }
    INT32   GetPages();
    short   GetOffset() { return nOffset;}
    void    SetEntry( StgDirEntry& );
    virtual BOOL SetSize( INT32 );
    virtual BOOL Pos2Page( INT32 nBytePos );
    virtual INT32 Read( void*, INT32 )        { return 0; }
    virtual INT32 Write( const void*, INT32 ) { return 0; }
    virtual StgPage* GetPhysPage( INT32 nBytePos, BOOL bForce = FALSE );
    virtual BOOL IsSmallStrm() { return FALSE; }
};

// The FAT stream class provides physical access to the master FAT.
// Since this access is implemented as a StgStrm, we can use the
// FAT allocator.

class StgFATStrm : public StgStrm {     // the master FAT stream
    virtual BOOL Pos2Page( INT32 nBytePos );
    BOOL  SetPage( short, INT32 );
public:
    StgFATStrm( StgIo& );
    INT32 GetPage( short, BOOL, USHORT *pnMasterAlloc = 0);
    virtual BOOL SetSize( INT32 );
    virtual StgPage* GetPhysPage( INT32 nBytePos, BOOL bForce = FALSE );
};

// The stream has a size increment which normally is 1, but which can be
// set to any value is you want the size to be incremented by certain values.

class StgDataStrm : public StgStrm      // a physical data stream
{
    short nIncr;                        // size adjust increment
    void Init( INT32 nBgn, INT32 nLen );
public:
    StgDataStrm( StgIo&, INT32 nBgn, INT32 nLen=-1 );
    StgDataStrm( StgIo&, StgDirEntry* );
    void* GetPtr( INT32 nPos, BOOL bForce, BOOL bDirty );
    void SetIncrement( short n ) { nIncr = n ; }
    virtual BOOL SetSize( INT32 );
    virtual INT32 Read( void*, INT32 );
    virtual INT32 Write( const void*, INT32 );
};

// The small stream class provides access to streams with a size < 4096 bytes.
// This stream is a StgStream containing small pages. The FAT for this stream
// is also a StgStream. The start of the FAT is in the header at DataRootPage,
// the stream itself is pointed to by the root entry (it holds start & size).

class StgSmallStrm : public StgStrm     // a logical data stream
{
    StgStrm* pData;                     // the data stream
    void Init( INT32 nBgn, INT32 nLen );
public:
    StgSmallStrm( StgIo&, INT32 nBgn, INT32 nLen );
    StgSmallStrm( StgIo&, StgDirEntry* );
    virtual INT32 Read( void*, INT32 );
    virtual INT32 Write( const void*, INT32 );
    virtual BOOL IsSmallStrm() { return TRUE; }
};

class StgTmpStrm : public SvMemoryStream
{
    String aName;
    SvFileStream* pStrm;
    virtual ULONG GetData( void* pData, ULONG nSize );
    virtual ULONG PutData( const void* pData, ULONG nSize );
    virtual ULONG SeekPos( ULONG nPos );
    virtual void FlushData();

public:
    StgTmpStrm( ULONG=16 );
   ~StgTmpStrm();
    BOOL Copy( StgTmpStrm& );
    void SetSize( ULONG );
    ULONG GetSize();
};

#endif
