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

// This file reflects the structure of MS file elements.
// It is very sensitive to alignment!

#ifndef _STGELEM_HXX
#define _STGELEM_HXX

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif

#include <sot/stg.hxx>

class StgIo;
class SvStream;
class String;

SvStream& operator>>( SvStream&, ClsId& );
SvStream& operator<<( SvStream&, const ClsId& );

class StgHeader
{
    sal_uInt8   cSignature[ 8 ];            // 00 signature (see below)
    ClsId   aClsId;                     // 08 Class ID
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
    sal_Int32   nMasterFAT[ 109 ];          // 4C first 109 master FAT pages
public:
    StgHeader();
    void  Init();                       // initialize the header
    sal_Bool  Load( StgIo& );
    sal_Bool  Load( SvStream& );
    sal_Bool  Store( StgIo& );
    sal_Bool  Check();                      // check the signature and version
    short GetByteOrder() const          { return nByteOrder;    }
    sal_Int32 GetTOCStart() const           { return nTOCstrm;      }
    void  SetTOCStart( sal_Int32 n );
    sal_Int32 GetDataFATStart() const       { return nDataFAT;      }
    void  SetDataFATStart( sal_Int32 n );
    sal_Int32 GetDataFATSize() const        { return nDataFATSize;  }
    void  SetDataFATSize( sal_Int32 n );
    sal_Int32 GetThreshold() const          { return nThreshold;    }
    short GetPageSize() const           { return nPageSize;     }
    short GetDataPageSize() const       { return nDataPageSize; }
    sal_Int32 GetFATSize() const            { return nFATSize;      }
    void  SetFATSize( sal_Int32 n );
    sal_Int32 GetFATChain() const           { return nMasterChain;  }
    void  SetFATChain( sal_Int32 n );
    sal_Int32 GetMasters() const            { return nMaster;       }
    void  SetMasters( sal_Int32 n );
    short GetFAT1Size() const           { return 109;           }
    const ClsId& GetClassId() const     { return aClsId;        }
    void  SetClassId( const ClsId& );
    sal_Int32 GetFATPage( short ) const;
    void  SetFATPage( short, sal_Int32 );
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

class StgStream;

#define STGENTRY_SIZE 128

class StgEntry {                        // directory enty
    sal_uInt16  nName[ 32 ];                // 00 name as WCHAR
    sal_Int16   nNameLen;                   // 40 size of name in bytes including 00H
    sal_uInt8   cType;                      // 42 entry type
    sal_uInt8   cFlags;                     // 43 0 or 1 (tree balance?)
    sal_Int32   nLeft;                      // 44 left node entry
    sal_Int32   nRight;                     // 48 right node entry
    sal_Int32   nChild;                     // 4C 1st child entry if storage
    ClsId   aClsId;                     // 50 class ID (optional)
    sal_Int32   nFlags;                     // 60 state flags(?)
    sal_Int32   nMtime[ 2 ];                // 64 modification time
    sal_Int32   nAtime[ 2 ];                // 6C creation and access time
    sal_Int32   nPage1;                     // 74 starting block (either direct or translated)
    sal_Int32   nSize;                      // 78 file size
    sal_Int32   nUnknown;                   // 7C unknown
    String  aName;                      // Name as Compare String (ascii, upper)
public:
    sal_Bool    Init();                     // initialize the data
    sal_Bool    SetName( const String& );   // store a name (ASCII, up to 32 chars)
    void    GetName( String& rName ) const;
                                        // fill in the name
    short   Compare( const StgEntry& ) const;   // compare two entries
    sal_Bool    Load( const void* );
    void    Store( void* );
    StgEntryType GetType() const        { return (StgEntryType) cType;  }
    sal_Int32   GetStartPage() const        { return nPage1; }
    void    SetType( StgEntryType t )   { cType = (sal_uInt8) t;                }
    sal_uInt8   GetFlags() const            { return cFlags;                }
    void    SetFlags( sal_uInt8 c )         { cFlags = c;                   }
    sal_Int32   GetSize() const             { return nSize;                 }
    void    SetSize( sal_Int32 n )          { nSize = n;                    }
    const   ClsId& GetClassId() const   { return aClsId;                }
    void    SetClassId( const ClsId& );
    sal_Int32   GetLeaf( StgEntryRef ) const;
    void    SetLeaf( StgEntryRef, sal_Int32 );
    const   sal_Int32* GetTime( StgEntryTime ) const;
    void    SetTime( StgEntryTime, sal_Int32* );
};


#define STG_FREE    -1L                 // page is free
#define STG_EOF     -2L                 // page is last page in chain
#define STG_FAT     -3L                 // page is FAT page
#define STG_MASTER  -4L                 // page is master FAT page

#endif
