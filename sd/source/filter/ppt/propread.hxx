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

#ifndef _PROPREAD_HXX_
#define _PROPREAD_HXX_

#include <map>
#include <boost/ptr_container/ptr_vector.hpp>

#include <sal/types.h>
#include <tools/solar.h>
#include <sot/storage.hxx>
#include <tools/stream.hxx>

// SummaryInformation
#define PID_TITLE               0x02
#define PID_SUBJECT             0x03
#define PID_AUTHOR              0x04
#define PID_KEYWORDS            0x05
#define PID_COMMENTS            0x06
#define PID_TEMPLATE            0x07
#define PID_LASTAUTHOR          0x08
#define PID_REVNUMBER           0x09
#define PID_EDITTIME            0x0a
#define PID_LASTPRINTED_DTM     0x0b
#define PID_CREATE_DTM          0x0c
#define PID_LASTSAVED_DTM       0x0d

// DocumentSummaryInformation
#define PID_CATEGORY            0x02
#define PID_PRESFORMAT          0x03
#define PID_BYTECOUNT           0x04
#define PID_LINECOUNT           0x05
#define PID_PARACOUNT           0x06
#define PID_SLIDECOUNT          0x07
#define PID_NOTECOUNT           0x08
#define PID_HIDDENCOUNT         0x09
#define PID_MMCLIPCOUNT         0x0a
#define PID_SCALE               0x0b
#define PID_HEADINGPAIR         0x0c
#define PID_DOCPARTS            0x0d
#define PID_MANAGER             0x0e
#define PID_COMPANY             0x0f
#define PID_LINKSDIRTY          0x10

#define VT_EMPTY            0
#define VT_NULL             1
#define VT_I2               2
#define VT_I4               3
#define VT_R4               4
#define VT_R8               5
#define VT_CY               6
#define VT_DATE             7
#define VT_BSTR             8
#define VT_UI4              9
#define VT_ERROR            10
#define VT_BOOL             11
#define VT_VARIANT          12
#define VT_DECIMAL          14
#define VT_I1               16
#define VT_UI1              17
#define VT_UI2              18
#define VT_I8               20
#define VT_UI8              21
#define VT_INT              22
#define VT_UINT             23
#define VT_LPSTR            30
#define VT_LPWSTR           31
#define VT_FILETIME         64
#define VT_BLOB             65
#define VT_STREAM           66
#define VT_STORAGE          67
#define VT_STREAMED_OBJECT  68
#define VT_STORED_OBJECT    69
#define VT_BLOB_OBJECT      70
#define VT_CF               71
#define VT_CLSID            72
#define VT_VECTOR           0x1000
#define VT_ARRAY            0x2000
#define VT_BYREF            0x4000
#define VT_TYPEMASK         0xFFF

// ------------------------------------------------------------------------

typedef std::map<OUString,sal_uInt32> Dictionary;

struct PropEntry
{
    sal_uInt32  mnId;
    sal_uInt32  mnSize;
    sal_uInt16  mnTextEnc;
    sal_uInt8*  mpBuf;

    PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize, sal_uInt16 nTextEnc );
    PropEntry( const PropEntry& rProp );
    ~PropEntry() { delete[] mpBuf; } ;

    PropEntry& operator=(const PropEntry& rPropEntry);
};

class PropItem : public SvMemoryStream
{
        sal_uInt16      mnTextEnc;

    public :
                        PropItem(){};
        void            Clear();

        void            SetTextEncoding( sal_uInt16 nTextEnc ){ mnTextEnc = nTextEnc; };
        sal_Bool        Read( OUString& rString, sal_uInt32 nType = VT_EMPTY, sal_Bool bDwordAlign = sal_True );
        PropItem&       operator=( PropItem& rPropItem );

    using SvStream::Read;
};

// ------------------------------------------------------------------------

class Section
{
        sal_uInt16              mnTextEnc;
        boost::ptr_vector<PropEntry> maEntries;

    protected:

        sal_uInt8                   aFMTID[ 16 ];

        void                    AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize );

    public:
                                Section( const sal_uInt8* pFMTID );
                                Section( const Section& rSection );

        Section&                operator=( const Section& rSection );
        sal_Bool                GetProperty( sal_uInt32 nId, PropItem& rPropItem );
        sal_Bool                GetDictionary( Dictionary& rDict );
        const sal_uInt8*        GetFMTID() const { return aFMTID; };
        void                    Read( SvStorageStream* pStrm );
};

// ------------------------------------------------------------------------

class PropRead
{
        sal_Bool                mbStatus;
        SvStorageStreamRef      mpSvStream;

        sal_uInt16              mnByteOrder;
        sal_uInt16              mnFormat;
        sal_uInt16              mnVersionLo;
        sal_uInt16              mnVersionHi;
        sal_uInt8               mApplicationCLSID[ 16 ];
        boost::ptr_vector<Section> maSections;

        void                    AddSection( Section& rSection );

    public:
                                PropRead( SvStorage& rSvStorage, const OUString& rName );

        PropRead&               operator=( const PropRead& rPropRead );
        const Section*          GetSection( const sal_uInt8* pFMTID );
        sal_Bool                IsValid() const { return mbStatus; };
        void                    Read();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
