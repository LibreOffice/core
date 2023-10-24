/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include <new>

namespace comphelper
{
template <typename F> OUString dumpXmlToString(F f)
{
    auto const buf = xmlBufferCreate();
    if (buf == nullptr)
    {
        throw std::bad_alloc();
    }
    auto const writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == nullptr)
    {
        throw std::bad_alloc();
    }
    f(writer);
    xmlFreeTextWriter(writer);
    std::string_view s(reinterpret_cast<char const*>(xmlBufferContent(buf)), xmlBufferLength(buf));
    OUString rv = OStringToOUString(s, RTL_TEXTENCODING_ISO_8859_1); //TODO
    xmlBufferFree(buf);
    return rv;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
