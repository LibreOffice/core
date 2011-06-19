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
    sal_Int32        nEntry;                    // entry # in TOC stream (temp)
    sal_Int32        nPos;                      // current position
    sal_Bool         bDirty;                    // dirty directory entry
    sal_Bool         bCreated;                  // newly created entry
    sal_Bool         bRemoved;                  // removed per Invalidate()
    sal_Bool         bRenamed;                  // renamed
    void         InitMembers();             // ctor helper
    virtual short Compare( const StgAvlNode* ) const;
    sal_Bool         StoreStream( StgIo& );     // store the stream
    sal_Bool         StoreStreams( StgIo& );    // store all streams
    void         RevertAll();               // revert the whole tree
    sal_Bool         Strm2Tmp();                // copy stgstream to temp file
    sal_Bool         Tmp2Strm();                // copy temp file to stgstream
public:
    StgEntry     aEntry;                    // entry data
    sal_Int32        nRefCnt;                   // reference count
    StreamMode   nMode;                     // open mode
    sal_Bool         bTemp;                     // sal_True: delete on dir flush
    sal_Bool         bDirect;                   // sal_True: direct mode
    sal_Bool         bZombie;                   // sal_True: Removed From StgIo
    sal_Bool         bInvalid;                  // sal_True: invalid entry
    StgDirEntry( const void*, sal_Bool * pbOk );
    StgDirEntry( const StgEntry& );
    ~StgDirEntry();

    void Invalidate( sal_Bool=sal_False );          // invalidate all open entries
    void Enum( sal_Int32& );                    // enumerate entries for iteration
    void DelTemp( sal_Bool );                   // delete temporary entries
    sal_Bool Store( StgDirStrm& );              // save entry into dir strm
    sal_Bool IsContained( StgDirEntry* );       // check if subentry

    void SetDirty()  { bDirty = sal_True;     }
    sal_Bool IsDirty();
    void ClearDirty();

    sal_Bool Commit();
    sal_Bool Revert();

    void  OpenStream( StgIo&, sal_Bool=sal_False );     // set up an approbiate stream
    void  Close();
    sal_Int32 GetSize();
    sal_Bool  SetSize( sal_Int32 );
    sal_Int32 Seek( sal_Int32 );
    sal_Int32 Tell() { return nPos; }
    sal_Int32 Read( void*, sal_Int32 );
    sal_Int32 Write( const void*, sal_Int32 );
    void  Copy( StgDirEntry& );
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
    virtual sal_Bool SetSize( sal_Int32 );              // change the size
    sal_Bool         Store();
    void*        GetEntry( sal_Int32 n, sal_Bool=sal_False );// get an entry
    StgDirEntry* GetRoot() { return pRoot; }
    StgDirEntry* Find( StgDirEntry&, const String& );
    StgDirEntry* Create( StgDirEntry&, const String&, StgEntryType );
    sal_Bool         Remove( StgDirEntry&, const String& );
    sal_Bool         Rename( StgDirEntry&, const String&, const String& );
    sal_Bool         Move( StgDirEntry&, StgDirEntry&, const String& );
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
