/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salcvt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:06:55 $
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
#ifndef SAL_CONVERTER_CACHE_HXX_
#define SAL_CONVERTER_CACHE_HXX_

#ifndef _SALUNX_H
#include <salunx.h>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _RTL_TEXTCVT_H
#include <rtl/textcvt.h>
#endif

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

