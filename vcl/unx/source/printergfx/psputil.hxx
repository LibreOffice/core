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

#ifndef _PSPRINT_PRINTERUTIL_HXX_
#define _PSPRINT_PRINTERUTIL_HXX_

#include "osl/file.hxx"

#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include "rtl/tencinfo.h"
#include "rtl/textcvt.h"

#include <map>

namespace psp {

/*
 *  string convenience routines
 *  sizeof(pBuffer) must be at least 2 Bytes, 0x00 <= nValue <= 0xFF,
 *  effective buffer of get*ValueOf() is NOT NULL-terminated
 */
sal_Int32   getHexValueOf (sal_Int32 nValue, sal_Char* pBuffer);
sal_Int32   getAlignedHexValueOf (sal_Int32 nValue, sal_Char* pBuffer);
sal_Int32   getValueOf    (sal_Int32 nValue, sal_Char* pBuffer);
sal_Int32   appendStr     (const sal_Char* pSrc, sal_Char* pDst);
sal_Int32   appendStr     (const sal_Char* pSrc, sal_Char* pDst, sal_Int32 nBytes);

sal_Bool    WritePS (osl::File* pFile, const sal_Char* pString);
sal_Bool    WritePS (osl::File* pFile, const sal_Char* pString, sal_uInt64 nInLength);
sal_Bool    WritePS (osl::File* pFile, const rtl::OString &rString);
sal_Bool    WritePS (osl::File* pFile, const rtl::OUString &rString);

class ConverterFactory
{

public:
    ConverterFactory();
    ~ConverterFactory();
    rtl_UnicodeToTextConverter  Get (rtl_TextEncoding nEncoding);
    sal_Size                    Convert (const sal_Unicode *pText, int nTextLen,
                                         sal_uChar *pBuffer, sal_Size nBufferSize,
                                         rtl_TextEncoding nEncoding);
private:

    std::map< rtl_TextEncoding, rtl_UnicodeToTextConverter >        m_aConverters;
};

ConverterFactory* GetConverterFactory ();

}  /* namespace psp */

#endif /* _PSPRINT_PRINTERUTIL_HXX_ */

