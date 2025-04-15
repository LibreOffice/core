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

#include <memory>
#include <string_view>
#include <o3tl/string_view.hxx>
#include <rtl/ustring.hxx>
#include <tools/lineend.hxx>
#include <utility>
#include <tools/solar.h>
#include "swdllapi.h"

inline constexpr OUString FILTER_RTF = u"RTF"_ustr;       ///< RTF filter
inline constexpr OUString sRtfWH = u"WH_RTF"_ustr;
inline constexpr OUString FILTER_TEXT = u"TEXT"_ustr; ///< text filter with default codeset
inline constexpr OUString FILTER_BAS = u"BAS"_ustr;       ///< StarBasic (identical to ANSI)
inline constexpr OUString FILTER_WW8 = u"CWW8"_ustr;      ///< WinWord 97 filter
inline constexpr OUString FILTER_TEXT_DLG = u"TEXT_DLG"_ustr;  ///< text filter with encoding dialog
inline constexpr OUString FILTER_XML = u"CXML"_ustr;      ///< XML filter
#define FILTER_XMLV     "CXMLV"     ///< XML filter
#define FILTER_XMLVW    "CXMLVWEB"  ///< XML filter
inline constexpr OUString FILTER_DOCX = u"OXML"_ustr;
inline constexpr OUString FILTER_MD = u"Markdown"_ustr; // markdown filter
inline constexpr OUString sHTML = u"HTML"_ustr;
inline constexpr OUString sWW5 = u"WW6"_ustr;
inline constexpr OUString sWW6 = u"CWW6"_ustr;

inline constexpr OUString sSWRITER = u"swriter"_ustr;
inline constexpr OUString sSWRITERWEB = u"swriter/web"_ustr;

class SfxFilter;
class SfxFilterContainer;
class SotStorage;
class SvStream;
namespace com::sun::star::embed { class XStorage; }
namespace com::sun::star::uno { template <typename> class Reference; }

struct SwIoDetect
{
    const OUString sName;

    SwIoDetect(OUString aN)
        : sName(std::move(aN))
    {
    }

    bool IsFilter( std::u16string_view rNm ) const
    {
        return o3tl::starts_with(rNm, sName);
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
    READER_WRITER_DOCX,
    READER_WRITER_MD,
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
    SW_DLLPUBLIC static std::shared_ptr<const SfxFilter>
        GetFilterOfFormat( std::u16string_view rFormat,
            const SfxFilterContainer* pCnt = nullptr );

    /** Detect for the given file which filter should be used. The filter name
     is returned. If no filter could be found, the name of the ASCII filter
     is returned! */
    static std::shared_ptr<const SfxFilter> GetFileFilter(const OUString& rFileName);

    static bool IsValidStgFilter( SotStorage& , const SfxFilter& );
    static bool IsValidStgFilter( const css::uno::Reference < css::embed::XStorage >& rStg, const SfxFilter& rFilter);

    // tdf#106899 - wrapper around IsDetectableText to retrieve the text encoding for a given stream
    SW_DLLPUBLIC static rtl_TextEncoding GetTextEncoding(SvStream&);
    static bool IsDetectableText( const char* pBuf, sal_uLong &rLen,
            rtl_TextEncoding *pCharSet, bool *pSwap, LineEnd *pLineEnd, bool *pBom);

    static OUString GetSubStorageName( const SfxFilter& rFltr );
};

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL TestImportFODT(SvStream &rStream);
extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL TestPDFExportFODT(SvStream &rStream);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
