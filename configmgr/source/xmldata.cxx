/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

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
#include "type.hxx"

namespace configmgr {

namespace xmldata {

namespace {

namespace css = com::sun::star;

}

Type parseType(
    xmlreader::XmlReader const & reader, xmlreader::Span const & text)
{
    OSL_ASSERT(text.is());
    sal_Int32 i = rtl_str_indexOfChar_WithLength(text.begin, text.length, ':');
    if (i >= 0) {
        switch (reader.getNamespaceId(xmlreader::Span(text.begin, i))) {
        case ParseManager::NAMESPACE_OOR:
            if (xmlreader::Span(text.begin + i + 1, text.length - (i + 1)).
                equals(RTL_CONSTASCII_STRINGPARAM("any")))
            {
                return TYPE_ANY;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("boolean-list")))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("short-list")))
            {
                return TYPE_SHORT_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("int-list")))
            {
                return TYPE_INT_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("long-list")))
            {
                return TYPE_LONG_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("double-list")))
            {
                return TYPE_DOUBLE_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("string-list")))
            {
                return TYPE_STRING_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("hexBinary-list")))
            {
                return TYPE_HEXBINARY_LIST;
            }
            break;
        case ParseManager::NAMESPACE_XS:
            if (xmlreader::Span(text.begin + i + 1, text.length - (i + 1)).
                equals(RTL_CONSTASCII_STRINGPARAM("boolean")))
            {
                return TYPE_BOOLEAN;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("short")))
            {
                return TYPE_SHORT;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("int")))
            {
                return TYPE_INT;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("long")))
            {
                return TYPE_LONG;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("double")))
            {
                return TYPE_DOUBLE;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("string")))
            {
                return TYPE_STRING;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals(RTL_CONSTASCII_STRINGPARAM("hexBinary")))
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
         text.convertFromUtf8()),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBoolean(xmlreader::Span const & text) {
    OSL_ASSERT(text.is());
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("true"))) {
        return true;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid boolean ")) +
         text.convertFromUtf8()),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
