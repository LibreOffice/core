/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salcvt.hxx,v $
 * $Revision: 1.5 $
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
#ifndef SAL_CONVERTER_CACHE_HXX_
#define SAL_CONVERTER_CACHE_HXX_

#include <salunx.h>
#include <rtl/tencinfo.h>
#include <rtl/textcvt.h>

#include <map>

extern "C" const char*
pGetEncodingName( rtl_TextEncoding nEncoding );

//
// Cache TextToUnicode and UnicodeToText converter and conversion info which is
// used in DrawXYZ routines and in the Event loop
//

class SalConverterCache {

    public:
                        SalConverterCache();
                        ~SalConverterCache();
        Bool            EncodingHasChar(
                                rtl_TextEncoding nEncoding, sal_Unicode nChar );
        rtl_UnicodeToTextConverter
                        GetU2TConverter( rtl_TextEncoding nEncoding );
        rtl_TextToUnicodeConverter
                        GetT2UConverter( rtl_TextEncoding nEncoding );
        Bool            IsSingleByteEncoding( rtl_TextEncoding nEncoding );
        sal_Size        ConvertStringUTF16( const sal_Unicode *pText, int nTextLen,
                                sal_Char *pBuffer, sal_Size nBufferSize,
                                rtl_TextEncoding nEncoding);

        static SalConverterCache*
                        GetInstance ();

    private:

        struct ConverterT {
            rtl_UnicodeToTextConverter  mpU2T;
            rtl_TextToUnicodeConverter  mpT2U;
            Bool                        mbSingleByteEncoding;
            Bool                        mbValid;
            ConverterT() :
                    mpU2T( NULL ),
                    mpT2U( NULL ),
                    mbSingleByteEncoding( False ),
                    mbValid( False )
            {
            }
            ~ConverterT()
            {
                if( mpU2T )
                    rtl_destroyUnicodeToTextConverter( mpU2T );
                if( mpT2U )
                    rtl_destroyTextToUnicodeConverter( mpT2U );
            }
        };

    std::map< rtl_TextEncoding, ConverterT >        m_aConverters;
};



#endif /* SAL_CONVERTER_CACHE_HXX_ */

