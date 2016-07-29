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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGDIR_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGDIR_HXX

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
    StgEntry     m_aSave;                     // original dir entry
    StgDirEntry*  m_pUp;                      // parent directory
    StgDirEntry*  m_pDown;                    // child directory for storages
    StgDirEntry** m_ppRoot;                   // root of TOC tree
    StgStrm*     m_pStgStrm;                  // storage stream
    StgTmpStrm*  m_pTmpStrm;                  // temporary stream
    StgTmpStrm*  m_pCurStrm;                  // temp stream after commit
    sal_Int32    m_nEntry;                    // entry # in TOC stream (temp)
    sal_Int32    m_nPos;                      // current position
    bool         m_bDirty;                    // dirty directory entry
    bool         m_bCreated;                  // newly created entry
    bool         m_bRemoved;                  // removed per Invalidate()
    bool         m_bRenamed;                  // renamed
    void         InitMembers();             // ctor helper
    virtual short Compare( const StgAvlNode* ) const override;
    bool         StoreStream( StgIo& );     // store the stream
    bool         StoreStreams( StgIo& );    // store all streams
    void         RevertAll();               // revert the whole tree
    bool         Strm2Tmp();                // copy stgstream to temp file
    bool         Tmp2Strm();                // copy temp file to stgstream
public:
    StgEntry     m_aEntry;                    // entry data
    sal_Int32        m_nRefCnt;                   // reference count
    StreamMode   m_nMode;                     // open mode
    bool         m_bTemp;                     // true: delete on dir flush
    bool         m_bDirect;                   // true: direct mode
    bool         m_bZombie;                   // true: Removed From StgIo
    bool         m_bInvalid;                  // true: invalid entry
    StgDirEntry(const void* pBuffer, sal_uInt32 nBufferLen,
                sal_uInt64 nUnderlyingStreamSize, bool * pbOk);
    explicit StgDirEntry( const StgEntry& );
    virtual ~StgDirEntry();

    void Invalidate( bool );                // invalidate all open entries
    void Enum( sal_Int32& );                // enumerate entries for iteration
    void DelTemp( bool );                   // delete temporary entries
    bool Store( StgDirStrm& );              // save entry into dir strm
    bool IsContained( StgDirEntry* );       // check if subentry

    void SetDirty()  { m_bDirty = true; }
    bool IsDirty();

    bool Commit();

    void  OpenStream( StgIo& );     // set up an appropriate stream
    void  Close();
    sal_Int32 GetSize();
    bool  SetSize( sal_Int32 );
    sal_Int32 Seek( sal_Int32 );
    sal_Int32 Read( void*, sal_Int32 );
    sal_Int32 Write( const void*, sal_Int32 );
    void  Copy( BaseStorageStream& );
};

class StgDirStrm : public StgDataStrm
{
    friend class StgIterator;
    StgDirEntry* m_pRoot;                         // root of dir tree
    short        m_nEntries;                      // entries per page
    void         SetupEntry( sal_Int32, StgDirEntry* );
public:
    explicit StgDirStrm( StgIo& );
    virtual ~StgDirStrm();
    virtual bool SetSize( sal_Int32 ) override;              // change the size
    bool         Store();
    void*        GetEntry( sal_Int32 n, bool );// get an entry
    StgDirEntry* GetRoot() { return m_pRoot; }
    StgDirEntry* Find( StgDirEntry&, const OUString& );
    StgDirEntry* Create( StgDirEntry&, const OUString&, StgEntryType );
};

class StgIterator : public StgAvlIterator
{
public:
    explicit StgIterator( StgDirEntry& rStg ) : StgAvlIterator( rStg.m_pDown ) {}
    StgDirEntry* First() { return static_cast<StgDirEntry*>( StgAvlIterator::First() ); }
    StgDirEntry* Next()  { return static_cast<StgDirEntry*>( StgAvlIterator::Next() );  }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
