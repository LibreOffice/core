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

#include <config_extensions.h>

#include <sal/config.h>

#include <string_view>
#include <utility>

#include <osl/diagnose.h>
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>

#include <dp_identifier.hxx>
#include "dp_activepackages.hxx"

// Old format of database entry:
//   key: UTF8(filename)
//   value: UTF8(tempname ";" mediatype)
// New format of database entry:
//   key: 0xFF UTF8(identifier)
//   value: UTF8(tempname) 0xFF UTF8(filename) 0xFF UTF8(mediatype)

#if HAVE_FEATURE_EXTENSIONS

namespace {

constexpr const char separator[] = "\xff";

OString oldKey(std::u16string_view fileName) {
    return OUStringToOString(fileName, RTL_TEXTENCODING_UTF8);
}

OString newKey(std::u16string_view id) {
    return separator + OUStringToOString(id, RTL_TEXTENCODING_UTF8);
}

::dp_manager::ActivePackages::Data decodeOldData(
    OUString const & fileName, OString const & value)
{
    ::dp_manager::ActivePackages::Data d;
    sal_Int32 i = value.indexOf(';');
    OSL_ASSERT(i >= 0);
    d.temporaryName = OUString(value.getStr(), i, RTL_TEXTENCODING_UTF8);
    d.fileName = fileName;
    d.mediaType = OUString(
        value.getStr() + i + 1, value.getLength() - i - 1,
        RTL_TEXTENCODING_UTF8);
    return d;
}

::dp_manager::ActivePackages::Data decodeNewData(OString const & value) {
    ::dp_manager::ActivePackages::Data d;
    sal_Int32 i1 = value.indexOf(separator);
    OSL_ASSERT(i1 >= 0);
    d.temporaryName = OUString(
        value.getStr(), i1, RTL_TEXTENCODING_UTF8);
    sal_Int32 i2 = value.indexOf(separator, i1 + 1);
    OSL_ASSERT(i2 >= 0);
    d.fileName = OUString(
        value.getStr() + i1 + 1, i2 - i1 - 1, RTL_TEXTENCODING_UTF8);
    sal_Int32 i3 = value.indexOf(separator, i2 + 1);

    if (i3 < 0)
    {
        //Before ActivePackages::Data::version was added
        d.mediaType = OUString(
            value.getStr() + i2 + 1, value.getLength() - i2 - 1,
            RTL_TEXTENCODING_UTF8);
    }
    else
    {
        sal_Int32 i4 = value.indexOf(separator, i3 + 1);
        d.mediaType = OUString(
            value.getStr() + i2 + 1, i3 - i2 -1, RTL_TEXTENCODING_UTF8);
        d.version = OUString(
            value.getStr() + i3 + 1, i4 - i3 - 1,
            RTL_TEXTENCODING_UTF8);
        d.failedPrerequisites = OUString(
            value.getStr() + i4 + 1, value.getLength() - i4 - 1,
            RTL_TEXTENCODING_UTF8);
    }
    return d;
}

}
#endif

namespace dp_manager {

ActivePackages::ActivePackages() {}

ActivePackages::ActivePackages(OUString const & url)
#if HAVE_FEATURE_EXTENSIONS
    : m_map(url)
#endif
{
#if !HAVE_FEATURE_EXTENSIONS
    (void) url;
#endif
}

ActivePackages::~ActivePackages() {}

bool ActivePackages::has(
    OUString const & id, OUString const & fileName) const
{
    return get(nullptr, id, fileName);
}

bool ActivePackages::get(
    Data * data, OUString const & id, OUString const & fileName)
    const
{
#if HAVE_FEATURE_EXTENSIONS
    OString v;
    if (m_map.get(&v, newKey(id))) {
        if (data != nullptr) {
            *data = decodeNewData(v);
        }
        return true;
    } else if (m_map.get(&v, oldKey(fileName))) {
        if (data != nullptr) {
            *data = decodeOldData(fileName, v);
        }
        return true;
    } else {
        return false;
    }
#else
    (void) data;
    (void) id;
    (void) fileName;
    (void) this;
    return false;
#endif
}

ActivePackages::Entries ActivePackages::getEntries() const {
    Entries es;
#if HAVE_FEATURE_EXTENSIONS
    ::dp_misc::t_string2string_map m(m_map.getEntries());
    for (auto const& elem : m)
    {
        if (!elem.first.isEmpty() && elem.first[0] == separator[0]) {
            es.emplace_back(
                    OUString(
                        elem.first.getStr() + 1, elem.first.getLength() - 1,
                        RTL_TEXTENCODING_UTF8),
                    decodeNewData(elem.second));
        } else {
            OUString fn(
                OStringToOUString(elem.first, RTL_TEXTENCODING_UTF8));
            es.emplace_back(
                    ::dp_misc::generateLegacyIdentifier(fn),
                    decodeOldData(fn, elem.second));
        }
    }
#else
    (void) this;
#endif
    return es;
}

void ActivePackages::put(OUString const & id, Data const & data) {
#if HAVE_FEATURE_EXTENSIONS
    OString b =
        OUStringToOString(data.temporaryName, RTL_TEXTENCODING_UTF8) +
        separator +
        OUStringToOString(data.fileName, RTL_TEXTENCODING_UTF8) +
        separator +
        OUStringToOString(data.mediaType, RTL_TEXTENCODING_UTF8) +
        separator +
        OUStringToOString(data.version, RTL_TEXTENCODING_UTF8) +
        separator +
        OUStringToOString(data.failedPrerequisites, RTL_TEXTENCODING_UTF8);
    m_map.put(newKey(id), b);
#else
    (void) id;
    (void) data;
    (void) this;
#endif
}

void ActivePackages::erase(
    OUString const & id, OUString const & fileName)
{
#if HAVE_FEATURE_EXTENSIONS
    m_map.erase(newKey(id)) || m_map.erase(oldKey(fileName));
#else
    (void) id;
    (void) fileName;
    (void) this;
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
