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

#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>

#include "data.hxx"
#include "parsemanager.hxx"
#include "type.hxx"
#include "xmldata.hxx"

namespace configmgr::xmldata {

Type parseType(
    xmlreader::XmlReader const & reader, xmlreader::Span const & text)
{
    assert(text.is());
    sal_Int32 i = rtl_str_indexOfChar_WithLength(text.begin, text.length, ':');
    if (i >= 0) {
        xmlreader::Span token(text.begin + i + 1, text.length - (i + 1));
        switch (reader.getNamespaceId(xmlreader::Span(text.begin, i))) {
        case ParseManager::NAMESPACE_OOR:
            if (token == "any")
            {
                return TYPE_ANY;
            } else if (token == "boolean-list")
            {
                return TYPE_BOOLEAN_LIST;
            } else if (token == "short-list")
            {
                return TYPE_SHORT_LIST;
            } else if (token == "int-list")
            {
                return TYPE_INT_LIST;
            } else if (token == "long-list")
            {
                return TYPE_LONG_LIST;
            } else if (token == "double-list")
            {
                return TYPE_DOUBLE_LIST;
            } else if (token == "string-list")
            {
                return TYPE_STRING_LIST;
            } else if (token == "hexBinary-list")
            {
                return TYPE_HEXBINARY_LIST;
            }
            break;
        case ParseManager::NAMESPACE_XS:
            if (token == "boolean")
            {
                return TYPE_BOOLEAN;
            } else if (token =="short")
            {
                return TYPE_SHORT;
            } else if (token =="int")
            {
                return TYPE_INT;
            } else if (token =="long")
            {
                return TYPE_LONG;
            } else if (token =="double")
            {
                return TYPE_DOUBLE;
            } else if (token =="string")
            {
                return TYPE_STRING;
            } else if (token =="hexBinary")
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
    if (text == "true") {
        return true;
    }
    if (text == "false") {
        return false;
    }
    throw css::uno::RuntimeException(
        "invalid boolean " + text.convertFromUtf8());
}

OUString parseTemplateReference(
    std::u16string_view component, bool hasNodeType,
    std::u16string_view nodeType, OUString const * defaultTemplateName)
{
    if (!hasNodeType) {
        if (defaultTemplateName != nullptr) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            u"missing node-type attribute"_ustr);
    }
    return Data::fullTemplateName(component, nodeType);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
