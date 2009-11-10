/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <climits>
#include <stack>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "parser.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "span.hxx"
#include "type.hxx"
#include "xmlreader.hxx"

namespace configmgr {

namespace xmldata {

namespace {

namespace css = com::sun::star;

}

rtl::OUString convertFromUtf8(Span const & text) {
    OSL_ASSERT(text.is());
    rtl_uString * s = 0;
    if (!rtl_convertStringToUString(
            &s, text.begin, text.length, RTL_TEXTENCODING_UTF8,
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

Type parseType(XmlReader const & reader, Span const & text) {
    OSL_ASSERT(text.is());
    sal_Int32 i = rtl_str_indexOfChar_WithLength(text.begin, text.length, ':');
    if (i >= 0) {
        switch (reader.getNamespace(Span(text.begin, i))) {
        case XmlReader::NAMESPACE_OOR:
            if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                    RTL_CONSTASCII_STRINGPARAM("any")))
            {
                return TYPE_ANY;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("boolean-list")))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("short-list")))
            {
                return TYPE_SHORT_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("int-list")))
            {
                return TYPE_INT_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("long-list")))
            {
                return TYPE_LONG_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("double-list")))
            {
                return TYPE_DOUBLE_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("string-list")))
            {
                return TYPE_STRING_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("hexBinary-list")))
            {
                return TYPE_HEXBINARY_LIST;
            }
            break;
        case XmlReader::NAMESPACE_XS:
            if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                    RTL_CONSTASCII_STRINGPARAM("boolean")))
            {
                return TYPE_BOOLEAN;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("short")))
            {
                return TYPE_SHORT;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("int")))
            {
                return TYPE_INT;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("long")))
            {
                return TYPE_LONG;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("double")))
            {
                return TYPE_DOUBLE;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("string")))
            {
                return TYPE_STRING;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("hexBinary")))
            {
                return TYPE_HEXBINARY;
            }
            break;
        default:
            break;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid type ")) +
         convertFromUtf8(text)),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBoolean(Span const & text) {
    OSL_ASSERT(text.is());
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("true"))) {
        return true;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid boolean ")) +
         convertFromUtf8(text)),
        css::uno::Reference< css::uno::XInterface >());
}

rtl::OUString parseTemplateReference(
    rtl::OUString const & component, bool hasNodeType,
    rtl::OUString const & nodeType, rtl::OUString const * defaultTemplateName)
{
    if (!hasNodeType) {
        if (defaultTemplateName != 0) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing node-type attribute")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return Data::fullTemplateName(component, nodeType);
}

}

}
