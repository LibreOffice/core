/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cassert>
#include <climits>
#include <stack>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/file.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>

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
#include "xmldata.hxx"

namespace configmgr {

namespace xmldata {

Type parseType(
    xmlreader::XmlReader const & reader, xmlreader::Span const & text)
{
    assert(text.is());
    sal_Int32 i = rtl_str_indexOfChar_WithLength(text.begin, text.length, ':');
    if (i >= 0) {
        switch (reader.getNamespaceId(xmlreader::Span(text.begin, i))) {
        case ParseManager::NAMESPACE_OOR:
            if (xmlreader::Span(text.begin + i + 1, text.length - (i + 1)).
                equals("any"))
            {
                return TYPE_ANY;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("boolean-list"))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("short-list"))
            {
                return TYPE_SHORT_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("int-list"))
            {
                return TYPE_INT_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("long-list"))
            {
                return TYPE_LONG_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("double-list"))
            {
                return TYPE_DOUBLE_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("string-list"))
            {
                return TYPE_STRING_LIST;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("hexBinary-list"))
            {
                return TYPE_HEXBINARY_LIST;
            }
            break;
        case ParseManager::NAMESPACE_XS:
            if (xmlreader::Span(text.begin + i + 1, text.length - (i + 1)).
                equals("boolean"))
            {
                return TYPE_BOOLEAN;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("short"))
            {
                return TYPE_SHORT;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("int"))
            {
                return TYPE_INT;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("long"))
            {
                return TYPE_LONG;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("double"))
            {
                return TYPE_DOUBLE;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("string"))
            {
                return TYPE_STRING;
            } else if (xmlreader::Span(
                           text.begin + i + 1, text.length - (i + 1)).
                       equals("hexBinary"))
            {
                return TYPE_HEXBINARY;
            }
            break;
        default:
            break;
        }
    }
    throw css::uno::RuntimeException(
        "invalid type " + text.convertFromUtf8());
}

bool parseBoolean(xmlreader::Span const & text) {
    assert(text.is());
    if (text.equals("true")) {
        return true;
    }
    if (text.equals("false")) {
        return false;
    }
    throw css::uno::RuntimeException(
        "invalid boolean " + text.convertFromUtf8());
}

OUString parseTemplateReference(
    OUString const & component, bool hasNodeType,
    OUString const & nodeType, OUString const * defaultTemplateName)
{
    if (!hasNodeType) {
        if (defaultTemplateName != nullptr) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            "missing node-type attribute");
    }
    return Data::fullTemplateName(component, nodeType);
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
