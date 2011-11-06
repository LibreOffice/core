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



#ifndef _STGCACHE_HXX
#define _STGCACHE_HXX

#include <osl/endian.h>
#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#include <stgelem.hxx>

class UCBStorageStream;

class StgIo;
class StgPage;
class StgDirEntry;
class StorageBase;

class StgCache {
    StgPage* pCur;                          // top of LRU list
    StgPage* pElem1;                        // top of ordered list
    sal_uLong nError;                           // error code
    sal_Int32 nPages;                           // size of data area in pages
    sal_uInt16 nRef;                            // reference count
    void * pLRUCache;                       // hash table of cached objects
    short nPageSize;                        // page size of the file
    UCBStorageStream* pStorageStream;       // holds reference to UCB storage stream

    void Erase( StgPage* );                 // delete a cache element
    void InsertToLRU( StgPage* );           // insert into LRU list
    void InsertToOrdered( StgPage* );       // insert into ordered list
    StgPage* Create( sal_Int32 );               // create a cached page
protected:
    SvStream* pStrm;                        // physical stream
    sal_Bool  bMyStream;                        // sal_True: delete stream in dtor
    sal_Bool  bFile;                            // sal_True: file stream
    sal_Int32 Page2Pos( sal_Int32 );                // page address --> file position
    sal_Int32 Pos2Page( sal_Int32 );                // file position --> page address
public:
    StgCache();
    ~StgCache();
    void  IncRef()                      { nRef++;           }
    sal_uInt16 DecRef()                     { return --nRef;    }
    void  SetPhysPageSize( short );
    sal_Int32 GetPhysPages()                { return nPages;    }
    short GetPhysPageSize()             { return nPageSize; }
    SvStream* GetStrm()                 { return pStrm;     }
    void  SetStrm( SvStream*, sal_Bool );
    void  SetStrm( UCBStorageStream* );
    sal_Bool  IsWritable()                  { return pStrm->IsWritable(); }
    sal_Bool  Good()                        { return sal_Bool( nError == SVSTREAM_OK ); }
    sal_Bool  Bad()                         { return sal_Bool( nError != SVSTREAM_OK ); }
    sal_uLong GetError()                    { return nError;    }
    void  MoveError( StorageBase& );
    void  SetError( sal_uLong );
    void  ResetError();
    sal_Bool  Open( const String& rName, StreamMode );
    void  Close();
    sal_Bool  Read( sal_Int32 nPage, void* pBuf, sal_Int32 nPages );
    sal_Bool  Write( sal_Int32 nPage, void* pBuf, sal_Int32 nPages );
    sal_Bool  SetSize( sal_Int32 nPages );
    StgPage* Find( sal_Int32 );                 // find a cached page
    StgPage* Get( sal_Int32, sal_Bool );            // get a cached page
    StgPage* Copy( sal_Int32, sal_Int32=STG_FREE ); // copy a page
    sal_Bool Commit( StgDirEntry* = NULL );     // flush all pages
    void Revert( StgDirEntry* = NULL );     // revert dirty pages
    void Clear();                           // clear the cache
};

class StgPage {
    friend class StgCache;
    StgCache* pCache;                       // the cache
    StgPage *pNext1, *pLast1;               // LRU chain
    StgPage *pNext2, *pLast2;               // ordered chain
    StgDirEntry* pOwner;                    // owner
    sal_Int32   nPage;                          // page #
    sal_uInt8*  pData;                          // nPageSize characters
    short   nData;                          // size of this page
    sal_Bool    bDirty;                         // dirty flag
    StgPage( StgCache*, short );
    ~StgPage();
public:
    void  SetDirty()                    { bDirty = sal_True;            }
    sal_Int32 GetPage()                     { return nPage;             }
    void* GetData()                     { return pData;             }
    short GetSize()                     { return nData;             }
    void  SetOwner( StgDirEntry* p )    { pOwner = p;               }
    // routines for accessing FAT pages
    // Assume that the data is a FAT page and get/put FAT data.
    sal_Int32 GetPage( short nOff )
    {
        if( ( nOff >= (short) ( nData / sizeof( sal_Int32 ) ) ) || nOff < 0 )
            return -1;
        sal_Int32 n = ((sal_Int32*) pData )[ nOff ];
#ifdef OSL_BIGENDIAN
        return SWAPLONG(n);
#else
        return n;
#endif
    }
    void  SetPage( short, sal_Int32 );      // put an element
};

#endif
