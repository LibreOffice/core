/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stgcache.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:40:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STGCACHE_HXX
#define _STGCACHE_HXX

#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif
#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _STGELEM_HXX
#include <stgelem.hxx>
#endif

class UCBStorageStream;

class StgIo;
class StgPage;
class StgDirEntry;
class StorageBase;

class StgCache {
    StgPage* pCur;                          // top of LRU list
    StgPage* pElem1;                        // top of ordered list
    ULONG nError;                           // error code
    INT32 nPages;                           // size of data area in pages
    USHORT nRef;                            // reference count
    void * pLRUCache;                       // hash table of cached objects
    short nPageSize;                        // page size of the file
    UCBStorageStream* pStorageStream;       // holds reference to UCB storage stream

    void Erase( StgPage* );                 // delete a cache element
    void InsertToLRU( StgPage* );           // insert into LRU list
    void InsertToOrdered( StgPage* );       // insert into ordered list
    StgPage* Create( INT32 );               // create a cached page
protected:
    SvStream* pStrm;                        // physical stream
    BOOL  bMyStream;                        // TRUE: delete stream in dtor
    BOOL  bFile;                            // TRUE: file stream
    INT32 Page2Pos( INT32 );                // page address --> file position
    INT32 Pos2Page( INT32 );                // file position --> page address
public:
    StgCache();
    ~StgCache();
    void  IncRef()                      { nRef++;           }
    USHORT DecRef()                     { return --nRef;    }
    void  SetPhysPageSize( short );
    INT32 GetPhysPages()                { return nPages;    }
    short GetPhysPageSize()             { return nPageSize; }
    SvStream* GetStrm()                 { return pStrm;     }
    void  SetStrm( SvStream*, BOOL );
    void  SetStrm( UCBStorageStream* );
    BOOL  IsWritable()                  { return pStrm->IsWritable(); }
    BOOL  Good()                        { return BOOL( nError == SVSTREAM_OK ); }
    BOOL  Bad()                         { return BOOL( nError != SVSTREAM_OK ); }
    ULONG GetError()                    { return nError;    }
    void  MoveError( StorageBase& );
    void  SetError( ULONG );
    void  ResetError();
    BOOL  Open( const String& rName, StreamMode );
    void  Close();
    BOOL  Read( INT32 nPage, void* pBuf, INT32 nPages );
    BOOL  Write( INT32 nPage, void* pBuf, INT32 nPages );
    BOOL  SetSize( INT32 nPages );
    StgPage* Find( INT32 );                 // find a cached page
    StgPage* Get( INT32, BOOL );            // get a cached page
    StgPage* Copy( INT32, INT32=STG_FREE ); // copy a page
    BOOL Commit( StgDirEntry* = NULL );     // flush all pages
    void Revert( StgDirEntry* = NULL );     // revert dirty pages
    void Clear();                           // clear the cache
};

class StgPage {
    friend class StgCache;
    StgCache* pCache;                       // the cache
    StgPage *pNext1, *pLast1;               // LRU chain
    StgPage *pNext2, *pLast2;               // ordered chain
    StgDirEntry* pOwner;                    // owner
    INT32   nPage;                          // page #
    BYTE*   pData;                          // nPageSize characters
    short   nData;                          // size of this page
    BOOL    bDirty;                         // dirty flag
    StgPage( StgCache*, short );
    ~StgPage();
public:
    void  SetDirty()                    { bDirty = TRUE;            }
    INT32 GetPage()                     { return nPage;             }
    void* GetData()                     { return pData;             }
    short GetSize()                     { return nData;             }
    void  SetOwner( StgDirEntry* p )    { pOwner = p;               }
    // routines for accessing FAT pages
    // Assume that the data is a FAT page and get/put FAT data.
    INT32 GetPage( short nOff )
    {
        if( ( nOff >= (short) ( nData / sizeof( INT32 ) ) ) || nOff < 0 )
            return -1;
        INT32 n = ((INT32*) pData )[ nOff ];
#ifdef OSL_BIGENDIAN
        return SWAPLONG(n);
#else
        return n;
#endif
    }
    void  SetPage( short, INT32 );      // put an element
};

#endif
