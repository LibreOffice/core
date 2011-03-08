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

#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustring.hxx"
#include <boost/unordered_map.hpp>

#include "dp_identifier.hxx"
#include "dp_persmap.h"

#include "dp_activepackages.hxx"

// Old format of database entry:
//   key: UTF8(filename)
//   value: UTF8(tempname ";" mediatype)
// New format of database entry:
//   key: 0xFF UTF8(identifier)
//   value: UTF8(tempname) 0xFF UTF8(filename) 0xFF UTF8(mediatype)

namespace {

static char const separator = static_cast< char >(
    static_cast< unsigned char >(0xFF));

static char const legacyPrefix[] = "org.openoffice.legacy.";

::rtl::OString oldKey(::rtl::OUString const & fileName) {
    return ::rtl::OUStringToOString(fileName, RTL_TEXTENCODING_UTF8);
}

::rtl::OString newKey(::rtl::OUString const & id) {
    ::rtl::OStringBuffer b;
    b.append(separator);
    b.append(::rtl::OUStringToOString(id, RTL_TEXTENCODING_UTF8));
    return b.makeStringAndClear();
}

::dp_manager::ActivePackages::Data decodeOldData(
    ::rtl::OUString const & fileName, ::rtl::OString const & value)
{
    ::dp_manager::ActivePackages::Data d;
    sal_Int32 i = value.indexOf(';');
    OSL_ASSERT(i >= 0);
    d.temporaryName = ::rtl::OUString(value.getStr(), i, RTL_TEXTENCODING_UTF8);
    d.fileName = fileName;
    d.mediaType = ::rtl::OUString(
        value.getStr() + i + 1, value.getLength() - i - 1,
        RTL_TEXTENCODING_UTF8);
    return d;
}

::dp_manager::ActivePackages::Data decodeNewData(::rtl::OString const & value) {
    ::dp_manager::ActivePackages::Data d;
    sal_Int32 i1 = value.indexOf(separator);
    OSL_ASSERT(i1 >= 0);
    d.temporaryName = ::rtl::OUString(
        value.getStr(), i1, RTL_TEXTENCODING_UTF8);
    sal_Int32 i2 = value.indexOf(separator, i1 + 1);
    OSL_ASSERT(i2 >= 0);
    d.fileName = ::rtl::OUString(
        value.getStr() + i1 + 1, i2 - i1 - 1, RTL_TEXTENCODING_UTF8);
    sal_Int32 i3 = value.indexOf(separator, i2 + 1);

    if (i3 < 0)
    {
        //Before ActivePackages::Data::version was added
        d.mediaType = ::rtl::OUString(
            value.getStr() + i2 + 1, value.getLength() - i2 - 1,
            RTL_TEXTENCODING_UTF8);
    }
    else
    {
        sal_Int32 i4 = value.indexOf(separator, i3 + 1);
        d.mediaType = ::rtl::OUString(
            value.getStr() + i2 + 1, i3 - i2 -1, RTL_TEXTENCODING_UTF8);
        d.version = ::rtl::OUString(
            value.getStr() + i3 + 1, i4 - i3 - 1,
            RTL_TEXTENCODING_UTF8);
        d.failedPrerequisites = ::rtl::OUString(
            value.getStr() + i4 + 1, value.getLength() - i4 - 1,
            RTL_TEXTENCODING_UTF8);
    }
    return d;
}

}

namespace dp_manager {

ActivePackages::ActivePackages() {}

ActivePackages::ActivePackages(::rtl::OUString const & url, bool readOnly):
    m_map(url, readOnly) {}

ActivePackages::~ActivePackages() {}

bool ActivePackages::has(
    ::rtl::OUString const & id, ::rtl::OUString const & fileName) const
{
    return get(NULL, id, fileName);
}

bool ActivePackages::get(
    Data * data, ::rtl::OUString const & id, ::rtl::OUString const & fileName)
    const
{
    ::rtl::OString v;
    if (m_map.get(&v, newKey(id))) {
        if (data != NULL) {
            *data = decodeNewData(v);
        }
        return true;
    } else if (m_map.get(&v, oldKey(fileName))) {
        if (data != NULL) {
            *data = decodeOldData(fileName, v);
        }
        return true;
    } else {
        return false;
    }
}

ActivePackages::Entries ActivePackages::getEntries() const {
    Entries es;
    ::dp_misc::t_string2string_map m(m_map.getEntries());
    for (::dp_misc::t_string2string_map::const_iterator i(m.begin());
         i != m.end(); ++i)
    {
        if (i->first.getLength() > 0 && i->first[0] == separator) {
            es.push_back(
                ::std::make_pair(
                    ::rtl::OUString(
                        i->first.getStr() + 1, i->first.getLength() - 1,
                        RTL_TEXTENCODING_UTF8),
                    decodeNewData(i->second)));
        } else {
            ::rtl::OUString fn(
                ::rtl::OStringToOUString(i->first, RTL_TEXTENCODING_UTF8));
            es.push_back(
                ::std::make_pair(
                    ::dp_misc::generateLegacyIdentifier(fn),
                    decodeOldData(fn, i->second)));
        }
    }
    return es;
}

void ActivePackages::put(::rtl::OUString const & id, Data const & data) {
    ::rtl::OStringBuffer b;
    b.append(
        ::rtl::OUStringToOString(data.temporaryName, RTL_TEXTENCODING_UTF8));
    b.append(separator);
    b.append(::rtl::OUStringToOString(data.fileName, RTL_TEXTENCODING_UTF8));
    b.append(separator);
    b.append(::rtl::OUStringToOString(data.mediaType, RTL_TEXTENCODING_UTF8));
    b.append(separator);
    b.append(::rtl::OUStringToOString(data.version, RTL_TEXTENCODING_UTF8));
    b.append(separator);
    b.append(::rtl::OUStringToOString(data.failedPrerequisites, RTL_TEXTENCODING_UTF8));
    m_map.put(newKey(id), b.makeStringAndClear());
}

void ActivePackages::erase(
    ::rtl::OUString const & id, ::rtl::OUString const & fileName)
{
    m_map.erase(newKey(id), true) || m_map.erase(oldKey(fileName), true);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
