/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SAL_CONVERTER_CACHE_HXX_
#define SAL_CONVERTER_CACHE_HXX_

#include <rtl/tencinfo.h>
#include <rtl/textcvt.h>

#include <unx/salunx.h>

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

