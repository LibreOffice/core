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

#ifndef INCLUDED_SW_INC_IODETECT_HXX
#define INCLUDED_SW_INC_IODETECT_HXX

#include <osl/endian.h>
#include <rtl/ustring.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <swdllapi.h>

#define FILTER_RTF      "RTF"       ///< RTF filter
#define sRtfWH          "WH_RTF"
#define FILTER_TEXT     "TEXT"      ///< text filter with default codeset
#define FILTER_BAS      "BAS"       ///< StarBasic (identical to ANSI)
#define FILTER_WW8      "CWW8"      ///< WinWord 97 filter
#define FILTER_TEXT_DLG "TEXT_DLG"  ///< text filter with encoding dialog
#define FILTER_XML      "CXML"      ///< XML filter
#define FILTER_XMLV     "CXMLV"     ///< XML filter
#define FILTER_XMLVW    "CXMLVWEB"  ///< XML filter
#define sHTML           "HTML"
#define sWW5            "WW6"
#define sWW6            "CWW6"

#define sSWRITER        "swriter"
#define sSWRITERWEB     "swriter/web"

struct SwIoDetect
{
    const OUString sName;

    SwIoDetect(const OUString &rN)
        : sName(rN)
    {
    }

    bool IsFilter( const OUString& rNm ) const
    {
        return rNm.startsWith(sName);
    }
};

enum ReaderWriterEnum {
    READER_WRITER_RTF,
    READER_WRITER_BAS,
    READER_WRITER_WW6,
    READER_WRITER_WW8,
    READER_WRITER_RTF_WH,
    READER_WRITER_HTML,
    READER_WRITER_WW5,
    READER_WRITER_XML,
    READER_WRITER_TEXT_DLG,
    READER_WRITER_TEXT,
    MAXFILTER
};

extern SwIoDetect aFilterDetect[];

/** The following class is a wrapper for basic i/o functions of Writer 3.0.
 Everything is static. All filter names mentioned are Writer-internal
 names, i.e. the names in front of the equality sign in INSTALL.INI, like SWG
 or ASCII.*/

class SwIoSystem
{
public:
    /// find for an internal format name the corresponding filter entry
    SW_DLLPUBLIC static const SfxFilter*
        GetFilterOfFormat( const OUString& rFormat,
            const SfxFilterContainer* pCnt = 0 );

    /** Detect for the given file which filter should be used. The filter name
     is returned. If no filter could be found, the name of the ASCII filter
     is returned! */
    static const SfxFilter* GetFileFilter(const OUString& rFileName);

    static bool IsValidStgFilter( SotStorage& , const SfxFilter& );
    static bool IsValidStgFilter( const css::uno::Reference < css::embed::XStorage >& rStg, const SfxFilter& rFilter);

    static bool IsDetectableText( const sal_Char* pBuf, sal_uLong &rLen,
            rtl_TextEncoding *pCharSet=0, bool *pSwap=0, LineEnd *pLineEnd=0, bool bEncodedFilter = false );

    static const OUString GetSubStorageName( const SfxFilter& rFltr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
