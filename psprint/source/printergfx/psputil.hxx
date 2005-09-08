/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: psputil.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:47:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PSPRINT_PRINTERUTIL_HXX_
#define _PSPRINT_PRINTERUTIL_HXX_

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

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

