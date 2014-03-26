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

#ifndef _STGDIR_HXX
#define _STGDIR_HXX

#include "stgavl.hxx"
#include "stgelem.hxx"
#include "stgstrms.hxx"

class StgIo;
class StgEntry;
class StgDirEntry;
class StgDirStrm;

class BaseStorageStream;
class StgDirEntry : public StgAvlNode
{
    friend class StgIterator;
    friend class StgDirStrm;
    StgEntry     aSave;                     // original dir entry
    StgDirEntry*  pUp;                      // parent directory
    StgDirEntry*  pDown;                    // child directory for storages
    StgDirEntry** ppRoot;                   // root of TOC tree
    StgStrm*     pStgStrm;                  // storage stream
    StgTmpStrm*  pTmpStrm;                  // temporary stream
    StgTmpStrm*  pCurStrm;                  // temp stream after commit
    sal_Int32    nEntry;                    // entry # in TOC stream (temp)
    sal_Int32    nPos;                      // current position
    bool         bDirty;                    // dirty directory entry
    bool         bCreated;                  // newly created entry
    bool         bRemoved;                  // removed per Invalidate()
    bool         bRenamed;                  // renamed
    void         InitMembers();             // ctor helper
    virtual short Compare( const StgAvlNode* ) const SAL_OVERRIDE;
    bool         StoreStream( StgIo& );     // store the stream
    bool         StoreStreams( StgIo& );    // store all streams
    void         RevertAll();               // revert the whole tree
    bool         Strm2Tmp();                // copy stgstream to temp file
    bool         Tmp2Strm();                // copy temp file to stgstream
public:
    StgEntry     aEntry;                    // entry data
    sal_Int32        nRefCnt;                   // reference count
    StreamMode   nMode;                     // open mode
    bool         bTemp;                     // true: delete on dir flush
    bool         bDirect;                   // true: direct mode
    bool         bZombie;                   // true: Removed From StgIo
    bool         bInvalid;                  // true: invalid entry
    StgDirEntry( const void* pBuffer, sal_uInt32 nBufferLen, bool * pbOk );
    StgDirEntry( const StgEntry& );
    ~StgDirEntry();

    void Invalidate( bool=false );          // invalidate all open entries
    void Enum( sal_Int32& );                    // enumerate entries for iteration
    void DelTemp( bool );                   // delete temporary entries
    bool Store( StgDirStrm& );              // save entry into dir strm
    bool IsContained( StgDirEntry* );       // check if subentry

    void SetDirty()  { bDirty = true; }
    bool IsDirty();
    void ClearDirty();

    bool Commit();
    bool Revert();

    void  OpenStream( StgIo&, bool=false );     // set up an approbiate stream
    void  Close();
    sal_Int32 GetSize();
    bool  SetSize( sal_Int32 );
    sal_Int32 Seek( sal_Int32 );
    sal_Int32 Tell() { return nPos; }
    sal_Int32 Read( void*, sal_Int32 );
    sal_Int32 Write( const void*, sal_Int32 );
    void  Copy( BaseStorageStream& );
};

class StgDirStrm : public StgDataStrm
{
    friend class StgIterator;
    StgDirEntry* pRoot;                         // root of dir tree
    short        nEntries;                      // entries per page
    void         SetupEntry( sal_Int32, StgDirEntry* );
public:
    StgDirStrm( StgIo& );
    ~StgDirStrm();
    virtual bool SetSize( sal_Int32 ) SAL_OVERRIDE;              // change the size
    bool         Store();
    void*        GetEntry( sal_Int32 n, bool=false );// get an entry
    StgDirEntry* GetRoot() { return pRoot; }
    StgDirEntry* Find( StgDirEntry&, const OUString& );
    StgDirEntry* Create( StgDirEntry&, const OUString&, StgEntryType );
    bool         Remove( StgDirEntry&, const OUString& );
    bool         Rename( StgDirEntry&, const OUString&, const OUString& );
    bool         Move( StgDirEntry&, StgDirEntry&, const OUString& );
};

class StgIterator : public StgAvlIterator
{
public:
    StgIterator( StgDirEntry& rStg ) : StgAvlIterator( rStg.pDown ) {}
    StgDirEntry* First() { return (StgDirEntry*) StgAvlIterator::First(); }
    StgDirEntry* Next()  { return (StgDirEntry*) StgAvlIterator::Next();  }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
