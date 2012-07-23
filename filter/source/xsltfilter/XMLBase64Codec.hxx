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
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
//This file is about the conversion of the UOF v2.0 and ODF document format

#ifndef _FILTER_SOURCE_XSLTFILTER_XMLBASE64_CODEC_HXX
#define _FILTER_SOURCE_XSLTFILTER_XMLBASE64_CODEC_HXX

#include <com/sun/star/uno/Sequence.hxx>

namespace rtl
{
class OUString;
class OUStringBuffer;
}

namespace XSLT{

class XMLBase64Codec
{
public:
    static void encodeBase64(rtl::OUStringBuffer& aStrBuffer, const com::sun::star::uno::Sequence<sal_Int8>& aPass);
#if 0
    static void decodeBase64(com::sun::star::uno::Sequence<sal_uInt8>& aPass, const rtl::OUString& sBuffer);
#endif
};

}
#endif
