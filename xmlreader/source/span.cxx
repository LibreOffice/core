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



#include "sal/config.h"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "xmlreader/span.hxx"

namespace xmlreader {

namespace {

namespace css = com::sun::star;

}

rtl::OUString Span::convertFromUtf8() const {
    OSL_ASSERT(is());
    rtl_uString * s = 0;
    if (!rtl_convertStringToUString(
            &s, begin, length, RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot convert from UTF-8")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return rtl::OUString(s, SAL_NO_ACQUIRE);
}

}
