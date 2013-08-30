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
#ifndef _SVX_TXENCBOX_HXX
#define _SVX_TXENCBOX_HXX

#include <vcl/lstbox.hxx>
#include <rtl/textenc.h>
#include "svx/svxdllapi.h"

class SvxTextEncodingTable;

class SVX_DLLPUBLIC SvxTextEncodingBox : public ListBox
{
private:
    const SvxTextEncodingTable*     m_pEncTable;

    SVX_DLLPRIVATE sal_uInt16               EncodingToPos_Impl( rtl_TextEncoding nEnc ) const;

public:
                        SvxTextEncodingBox( Window* pParent, const ResId& rResId );
                        SvxTextEncodingBox( Window* pParent, WinBits nBits );
                        ~SvxTextEncodingBox();

    /** Fill with all known encodings but exclude those matching one or more
        given flags as defined in rtl/tencinfo.h

         <p> If nButIncludeInfoFlags is given, encodings are included even if they
         match nExcludeInfoFlags. Thus it is possible to exclude 16/32-bit
         Unicode with RTL_TEXTENCODING_INFO_UNICODE but to include UTF7 and UTF8
         with RTL_TEXTENCODING_INFO_MIME </p>

        @param bExcludeImportSubsets
            If <TRUE/>, some specific encodings are not listed, as they are a
            subset of another encoding. This is the case for
            RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_GBK,
            RTL_TEXTENCODING_MS_936, which are covered by
            RTL_TEXTENCODING_GB_18030. Normally, this flag should be set to
            <TRUE/> whenever the box is used in import dialogs. */
    void                FillFromTextEncodingTable(
                            sal_Bool bExcludeImportSubsets = sal_False,
                            sal_uInt32 nExcludeInfoFlags = 0,
                            sal_uInt32 nButIncludeInfoFlags = 0
                            );

    /** Fill with all encodings known to the dbtools::OCharsetMap but exclude
        those matching one or more given flags as defined in rtl/tencinfo.h

         <p> If nButIncludeInfoFlags is given, encodings are included even if they
         match nExcludeInfoFlags. Thus it is possible to exclude 16/32-bit
         Unicode with RTL_TEXTENCODING_INFO_UNICODE but to include UTF7 and UTF8
         with RTL_TEXTENCODING_INFO_MIME </p>

        @param bExcludeImportSubsets
            If <TRUE/>, some specific encodings are not listed, as they are a
            subset of another encoding. This is the case for
            RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_GBK,
            RTL_TEXTENCODING_MS_936, which are covered by
            RTL_TEXTENCODING_GB_18030. Normally, this flag should be set to
            <TRUE/> whenever the box is used in import dialogs. */
    void                FillFromDbTextEncodingMap(
                            sal_Bool bExcludeImportSubsets = sal_False,
                            sal_uInt32 nExcludeInfoFlags = 0,
                            sal_uInt32 nButIncludeInfoFlags = 0
                            );

    /** Fill with all known MIME encodings and select the best according to
        <method>GetBestMimeEncoding</method>
     */
    void                FillWithMimeAndSelectBest();

    const SvxTextEncodingTable*     GetTextEncodingTable() const
                            { return m_pEncTable; }

    void                InsertTextEncoding( const rtl_TextEncoding nEnc,
                            sal_uInt16 nPos = LISTBOX_APPEND );

    void                InsertTextEncoding( const rtl_TextEncoding nEnc,
                            const OUString& rEntry,
                            sal_uInt16 nPos = LISTBOX_APPEND );

    void                SelectTextEncoding( const rtl_TextEncoding nEnc,
                            sal_Bool bSelect = sal_True );

    rtl_TextEncoding    GetSelectTextEncoding() const;

    const OUString&     GetSelectTextString() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
