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

#pragma once

#include <map>
#include <vector>
#include <memory>

#include <sal/types.h>
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
#define PID_CREATE_DTM          0x0c

// DocumentSummaryInformation
#define PID_SLIDECOUNT          0x07
#define PID_HEADINGPAIR         0x0c
#define PID_DOCPARTS            0x0d

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

typedef std::map<OUString,sal_uInt32> PropDictionary;

struct PropEntry
{
    sal_uInt32                    mnId;
    sal_uInt32                    mnSize;
    std::unique_ptr<sal_uInt8[]>  mpBuf;

    PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize );
    PropEntry( const PropEntry& rProp );

    PropEntry& operator=(const PropEntry& rPropEntry);
};

class PropItem : public SvMemoryStream
{
    sal_uInt16      mnTextEnc;

public:
    PropItem()
        : mnTextEnc(RTL_TEXTENCODING_DONTKNOW)
    {
    }
    void            Clear();

    void            SetTextEncoding( sal_uInt16 nTextEnc ){ mnTextEnc = nTextEnc; };
    bool            Read( OUString& rString, sal_uInt32 nType = VT_EMPTY, bool bDwordAlign = true );
    PropItem&       operator=( PropItem& rPropItem );
};

class Section final
{
        sal_uInt16              mnTextEnc;
        std::vector<std::unique_ptr<PropEntry> > maEntries;

        sal_uInt8                   aFMTID[ 16 ];

        void                    AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize );

    public:
                                explicit Section( const sal_uInt8* pFMTID );
                                Section( const Section& rSection );

        Section&                operator=( const Section& rSection );
        bool                    GetProperty( sal_uInt32 nId, PropItem& rPropItem );
        void                    GetDictionary( PropDictionary& rDict );
        const sal_uInt8*        GetFMTID() const { return aFMTID; };
        void                    Read( SotStorageStream* pStrm );
};

class PropRead
{
        bool                mbStatus;
        tools::SvRef<SotStorageStream>      mpSvStream;

        sal_uInt16              mnByteOrder;
        sal_uInt8               mApplicationCLSID[ 16 ];
        std::vector<std::unique_ptr<Section> > maSections;

    public:
                                PropRead( SotStorage& rSvStorage, const OUString& rName );

        PropRead&               operator=( const PropRead& rPropRead );
        const Section*          GetSection( const sal_uInt8* pFMTID );
        bool                    IsValid() const { return mbStatus; };
        void                    Read();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
