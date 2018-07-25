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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGELEM_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGELEM_HXX

#include <sot/stg.hxx>

class StgIo;
class SvStream;

SvStream& ReadClsId( SvStream&, ClsId& );
SvStream& WriteClsId( SvStream&, const ClsId& );

class StgHeader
{
    static const sal_uInt8 cFATPagesInHeader = 109;

    sal_uInt8   m_cSignature[ 8 ];            // 00 signature (see below)
    ClsId       m_aClsId;                     // 08 Class ID
    sal_Int32   m_nVersion;                   // 18 version number
    sal_uInt16  m_nByteOrder;                 // 1C Unicode byte order indicator
    sal_Int16   m_nPageSize;                  // 1E 1 << nPageSize = block size
    sal_Int16   m_nDataPageSize;              // 20 1 << this size == data block size
    sal_uInt8   m_bDirty;                     // 22 internal dirty flag (should be
                                            //    bool, but probably required to
                                            //    be exactly one byte)
    sal_uInt8   m_cReserved[ 9 ];             // 23
    sal_Int32   m_nFATSize;                   // 2C total number of FAT pages
    sal_Int32   m_nTOCstrm;                   // 30 starting page for the TOC stream
    sal_Int32   m_nReserved;                  // 34
    sal_Int32   m_nThreshold;                 // 38 minimum file size for big data
    sal_Int32   m_nDataFAT;                   // 3C page # of 1st data FAT block
    sal_Int32   m_nDataFATSize;               // 40 # of data fat blocks
    sal_Int32   m_nMasterChain;               // 44 chain to the next master block
    sal_Int32   m_nMaster;                    // 48 # of additional master blocks
    sal_Int32   m_nMasterFAT[ cFATPagesInHeader ];            // 4C first [cFATPagesInHeader] master FAT pages
public:
    StgHeader();

    void  Init();                       // initialize the header
    bool      Load( StgIo& );
    bool      Load( SvStream& );
    bool      Store( StgIo& );
    bool      Check();                      // check the signature and version
    sal_Int32 GetTOCStart() const           { return m_nTOCstrm; }
    void      SetTOCStart( sal_Int32 n );
    sal_Int32 GetDataFATStart() const       { return m_nDataFAT; }
    void      SetDataFATStart( sal_Int32 n );
    sal_Int32 GetDataFATSize() const        { return m_nDataFATSize; }
    void      SetDataFATSize( sal_Int32 n );
    sal_Int32 GetThreshold() const          { return m_nThreshold; }
    short     GetPageSize() const           { return m_nPageSize; }
    short     GetDataPageSize() const       { return m_nDataPageSize; }
    sal_Int32 GetFATSize() const            { return m_nFATSize; }
    void      SetFATSize( sal_Int32 n );
    sal_Int32 GetFATChain() const           { return m_nMasterChain; }
    void      SetFATChain( sal_Int32 n );
    sal_Int32 GetMasters() const            { return m_nMaster; }
    void      SetMasters( sal_Int32 n );
    static short GetFAT1Size()              { return cFATPagesInHeader; }
    sal_Int32 GetFATPage( short ) const;
    void      SetFATPage( short, sal_Int32 );
};

enum StgEntryType {                     // dir entry types:
    STG_EMPTY     = 0,
    STG_STORAGE   = 1,
    STG_STREAM    = 2,
    STG_ROOT      = 5
};

enum StgEntryRef {                      // reference blocks:
    STG_LEFT      = 0,                  // left
    STG_RIGHT     = 1,                  // right
    STG_CHILD     = 2,                  // child
    STG_DATA      = 3                   // data start
};

#define STGENTRY_SIZE 128

//StructuredStorageDirectoryEntry
class StgEntry
{                        // directory entry
    sal_Unicode m_nName[ 32 ];                // 00 name as WCHAR
    sal_uInt16  m_nNameLen;                   // 40 size of name in bytes including 00H
    sal_uInt8   m_cType;                      // 42 entry type
    sal_uInt8   m_cFlags;                     // 43 0 or 1 (tree balance?)
    sal_Int32   m_nLeft;                      // 44 left node entry
    sal_Int32   m_nRight;                     // 48 right node entry
    sal_Int32   m_nChild;                     // 4C 1st child entry if storage
    ClsId       m_aClsId;                     // 50 class ID (optional)
    sal_Int32   m_nFlags;                     // 60 state flags(?)
    sal_Int32   m_nMtime[ 2 ];                // 64 modification time
    sal_Int32   m_nAtime[ 2 ];                // 6C creation and access time
    sal_Int32   m_nPage1;                     // 74 starting block (either direct or translated)
    sal_Int32   m_nSize;                      // 78 file size
    sal_Int32   m_nUnknown;                   // 7C unknown
    OUString    m_aName;                      // Name as Compare String (ascii, upper)
public:
    void        Init();                     // initialize the data
    void        SetName( const OUString& );   // store a name (ASCII, up to 32 chars)
    void        GetName( OUString& rName ) const;
                                        // fill in the name
    sal_Int32   Compare( const StgEntry& ) const;   // compare two entries
    bool        Load( const void* pBuffer, sal_uInt32 nBufSize, sal_uInt64 nUnderlyingStreamSize );
    void        Store( void* );
    StgEntryType GetType() const            { return static_cast<StgEntryType>(m_cType);  }
    sal_Int32   GetStartPage() const        { return m_nPage1; }
    void        SetType( StgEntryType t )   { m_cType = static_cast<sal_uInt8>(t); }
    sal_Int32   GetSize() const             { return m_nSize; }
    void        SetSize( sal_Int32 n )      { m_nSize = n; }
    const ClsId& GetClassId() const         { return m_aClsId; }
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
