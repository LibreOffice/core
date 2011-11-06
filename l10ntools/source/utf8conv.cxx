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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#include "utf8conv.hxx"

//
// class UTF8Converter
//

#define MAX_CONV_BUFFER_SIZE    0xFF00

#define TO_CVTFLAGS     (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |\
                          RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |\
                          RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT)

#define FROM_CVTFLAGS   (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |\
                             RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |\
                            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |\
                            RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0)

/*****************************************************************************/
void UTF8Converter::Convert( ByteString &rBuffer,
    rtl_TextEncoding nSourceENC, rtl_TextEncoding nDestENC )
/*****************************************************************************/
{
    String sTemp( rBuffer, nSourceENC );
    rBuffer = ByteString( sTemp, nDestENC );
}

/*****************************************************************************/
ByteString UTF8Converter::ConvertToUTF8(
    const ByteString &rASCII, rtl_TextEncoding nEncoding )
/*****************************************************************************/
{
    ByteString sReturn( rASCII );
    Convert( sReturn, nEncoding, RTL_TEXTENCODING_UTF8 );
    return sReturn;
}

/*****************************************************************************/
ByteString UTF8Converter::ConvertFromUTF8(
    const ByteString &rUTF8, rtl_TextEncoding nEncoding )
/*****************************************************************************/
{
    ByteString sReturn( rUTF8 );
    Convert( sReturn, RTL_TEXTENCODING_UTF8, nEncoding );
    return sReturn;
}
