/*************************************************************************
 *
 *  $RCSfile: salcvt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    private:

        typedef struct {
            rtl_UnicodeToTextConverter  mpU2T;
            rtl_TextToUnicodeConverter  mpT2U;
            Bool                        mbSingleByteEncoding;
            Bool                        mbValid;
        } ConverterT;

        ConverterT *mpConverter;
};

// wrapper for rtl_convertUnicodeToText that handles the usual cases for
// textconversion in drawtext routines
sal_Size
ConvertStringUTF16( const sal_Unicode *pText, int nTextLen,
        sal_Char *pBuffer, sal_Size nBufferSize,
        rtl_UnicodeToTextConverter aConverter );


#endif /* SAL_CONVERTER_CACHE_HXX_ */

