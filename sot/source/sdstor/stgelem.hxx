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

// This file reflects the structure of MS file elements.
// It is very sensitive to alignment!

#ifndef _STGELEM_HXX
#define _STGELEM_HXX

#include <sot/stg.hxx>

class StgIo;
class SvStream;

SvStream& operator>>( SvStream&, ClsId& );
SvStream& WriteClsId( SvStream&, const ClsId& );

class StgHeader
{
    static const sal_uInt8 cFATPagesInHeader = 109;

    sal_uInt8   cSignature[ 8 ];            // 00 signature (see below)
    ClsId       aClsId;                     // 08 Class ID
    sal_Int32   nVersion;                   // 18 version number
    sal_uInt16  nByteOrder;                 // 1C Unicode byte order indicator
    sal_Int16   nPageSize;                  // 1E 1 << nPageSize = block size
    sal_Int16   nDataPageSize;              // 20 1 << this size == data block size
    sal_uInt8   bDirty;                     // 22 internal dirty flag
    sal_uInt8   cReserved[ 9 ];             // 23
    sal_Int32   nFATSize;                   // 2C total number of FAT pages
    sal_Int32   nTOCstrm;                   // 30 starting page for the TOC stream
    sal_Int32   nReserved;                  // 34
    sal_Int32   nThreshold;                 // 38 minimum file size for big data
    sal_Int32   nDataFAT;                   // 3C page # of 1st data FAT block
    sal_Int32   nDataFATSize;               // 40 # of data fat blocks
    sal_Int32   nMasterChain;               // 44 chain to the next master block
    sal_Int32   nMaster;                    // 48 # of additional master blocks
    sal_Int32   nMasterFAT[ cFATPagesInHeader ];            // 4C first [cFATPagesInHeader] master FAT pages
public:
    StgHeader();

    void  Init();                       // initialize the header
    bool      Load( StgIo& );
    bool      Load( SvStream& );
    bool      Store( StgIo& );
    bool      Check();                      // check the signature and version
    short     GetByteOrder() const          { return nByteOrder; }
    sal_Int32 GetTOCStart() const           { return nTOCstrm; }
    void      SetTOCStart( sal_Int32 n );
    sal_Int32 GetDataFATStart() const       { return nDataFAT; }
    void      SetDataFATStart( sal_Int32 n );
    sal_Int32 GetDataFATSize() const        { return nDataFATSize; }
    void      SetDataFATSize( sal_Int32 n );
    sal_Int32 GetThreshold() const          { return nThreshold; }
    short     GetPageSize() const           { return nPageSize; }
    short     GetDataPageSize() const       { return nDataPageSize; }
    sal_Int32 GetFATSize() const            { return nFATSize; }
    void      SetFATSize( sal_Int32 n );
    sal_Int32 GetFATChain() const           { return nMasterChain; }
    void      SetFATChain( sal_Int32 n );
    sal_Int32 GetMasters() const            { return nMaster; }
    void      SetMasters( sal_Int32 n );
    short     GetFAT1Size() const           { return cFATPagesInHeader; }
    const ClsId& GetClassId() const         { return aClsId;        }
    sal_Int32 GetFATPage( short ) const;
    void      SetFATPage( short, sal_Int32 );
};

enum StgEntryType {                     // dir entry types:
    STG_EMPTY     = 0,
    STG_STORAGE   = 1,
    STG_STREAM    = 2,
    STG_LOCKBYTES = 3,
    STG_PROPERTY  = 4,
    STG_ROOT      = 5
};

enum StgEntryRef {                      // reference blocks:
    STG_LEFT      = 0,                  // left
    STG_RIGHT     = 1,                  // right
    STG_CHILD     = 2,                  // child
    STG_DATA      = 3                   // data start
};

enum StgEntryTime {                     // time codes:
    STG_MODIFIED  = 0,                  // last modification
    STG_ACCESSED  = 1                   // last access
};

#define STGENTRY_SIZE 128

//StructuredStorageDirectoryEntry
class StgEntry
{                        // directory enty
    sal_uInt16  nName[ 32 ];                // 00 name as WCHAR
    sal_uInt16  nNameLen;                   // 40 size of name in bytes including 00H
    sal_uInt8   cType;                      // 42 entry type
    sal_uInt8   cFlags;                     // 43 0 or 1 (tree balance?)
    sal_Int32   nLeft;                      // 44 left node entry
    sal_Int32   nRight;                     // 48 right node entry
    sal_Int32   nChild;                     // 4C 1st child entry if storage
    ClsId       aClsId;                     // 50 class ID (optional)
    sal_Int32   nFlags;                     // 60 state flags(?)
    sal_Int32   nMtime[ 2 ];                // 64 modification time
    sal_Int32   nAtime[ 2 ];                // 6C creation and access time
    sal_Int32   nPage1;                     // 74 starting block (either direct or translated)
    sal_Int32   nSize;                      // 78 file size
    sal_Int32   nUnknown;                   // 7C unknown
    OUString    aName;                      // Name as Compare String (ascii, upper)
public:
    bool        Init();                     // initialize the data
    bool        SetName( const OUString& );   // store a name (ASCII, up to 32 chars)
    void        GetName( OUString& rName ) const;
                                        // fill in the name
    short       Compare( const StgEntry& ) const;   // compare two entries
    bool        Load( const void* pBuffer, sal_uInt32 nBufSize );
    void        Store( void* );
    StgEntryType GetType() const            { return (StgEntryType) cType;  }
    sal_Int32   GetStartPage() const        { return nPage1; }
    void        SetType( StgEntryType t )   { cType = (sal_uInt8) t; }
    sal_uInt8   GetFlags() const            { return cFlags; }
    void        SetFlags( sal_uInt8 c )     { cFlags = c; }
    sal_Int32   GetSize() const             { return nSize; }
    void        SetSize( sal_Int32 n )      { nSize = n; }
    const ClsId& GetClassId() const         { return aClsId; }
    void        SetClassId( const ClsId& );
    sal_Int32   GetLeaf( StgEntryRef ) const;
    void        SetLeaf( StgEntryRef, sal_Int32 );
};


#define STG_FREE    -1L                 // page is free
#define STG_EOF     -2L                 // page is last page in chain
#define STG_FAT     -3L                 // page is FAT page
#define STG_MASTER  -4L                 // page is master FAT page

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
