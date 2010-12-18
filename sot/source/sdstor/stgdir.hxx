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
    INT32        nEntry;                    // entry # in TOC stream (temp)
    INT32        nPos;                      // current position
    BOOL         bDirty;                    // dirty directory entry
    BOOL         bCreated;                  // newly created entry
    BOOL         bRemoved;                  // removed per Invalidate()
    BOOL         bRenamed;                  // renamed
    void         InitMembers();             // ctor helper
    virtual short Compare( const StgAvlNode* ) const;
    BOOL         StoreStream( StgIo& );     // store the stream
    BOOL         StoreStreams( StgIo& );    // store all streams
    void         RevertAll();               // revert the whole tree
    BOOL         Strm2Tmp();                // copy stgstream to temp file
    BOOL         Tmp2Strm();                // copy temp file to stgstream
public:
    StgEntry     aEntry;                    // entry data
    INT32        nRefCnt;                   // reference count
    StreamMode   nMode;                     // open mode
    BOOL         bTemp;                     // TRUE: delete on dir flush
    BOOL         bDirect;                   // TRUE: direct mode
    BOOL         bZombie;                   // TRUE: Removed From StgIo
    BOOL         bInvalid;                  // TRUE: invalid entry
    StgDirEntry( const void*, BOOL * pbOk );
    StgDirEntry( const StgEntry& );
    ~StgDirEntry();

    void Invalidate( BOOL=FALSE );          // invalidate all open entries
    void Enum( INT32& );                    // enumerate entries for iteration
    void DelTemp( BOOL );                   // delete temporary entries
    BOOL Store( StgDirStrm& );              // save entry into dir strm
    BOOL IsContained( StgDirEntry* );       // check if subentry

    void SetDirty()  { bDirty = TRUE;     }
    BOOL IsDirty();
    void ClearDirty();

    BOOL Commit();
    BOOL Revert();

    void  OpenStream( StgIo&, BOOL=FALSE );     // set up an approbiate stream
    void  Close();
    INT32 GetSize();
    BOOL  SetSize( INT32 );
    INT32 Seek( INT32 );
    INT32 Tell() { return nPos; }
    INT32 Read( void*, INT32 );
    INT32 Write( const void*, INT32 );
    void  Copy( StgDirEntry& );
    void  Copy( BaseStorageStream& );
};

class StgDirStrm : public StgDataStrm
{
    friend class StgIterator;
    StgDirEntry* pRoot;                         // root of dir tree
    short        nEntries;                      // entries per page
    void         SetupEntry( INT32, StgDirEntry* );
public:
    StgDirStrm( StgIo& );
    ~StgDirStrm();
    virtual BOOL SetSize( INT32 );              // change the size
    BOOL         Store();
    void*        GetEntry( INT32 n, BOOL=FALSE );// get an entry
    StgDirEntry* GetRoot() { return pRoot; }
    StgDirEntry* Find( StgDirEntry&, const String& );
    StgDirEntry* Create( StgDirEntry&, const String&, StgEntryType );
    BOOL         Remove( StgDirEntry&, const String& );
    BOOL         Rename( StgDirEntry&, const String&, const String& );
    BOOL         Move( StgDirEntry&, StgDirEntry&, const String& );
};

class StgIterator : public StgAvlIterator
{
public:
    StgIterator( StgDirEntry& rStg ) : StgAvlIterator( rStg.pDown ) {}
    StgDirEntry* First() { return (StgDirEntry*) StgAvlIterator::First(); }
    StgDirEntry* Next()  { return (StgDirEntry*) StgAvlIterator::Next();  }
    StgDirEntry* Last()  { return (StgDirEntry*) StgAvlIterator::Last();  }
    StgDirEntry* Prev()  { return (StgDirEntry*) StgAvlIterator::Prev();  }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
