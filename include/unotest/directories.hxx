/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_DIRECTORIES_HXX
#define INCLUDED_UNOTEST_DIRECTORIES_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotest/detail/unotestdllapi.hxx>

namespace test
{
class OOO_DLLPUBLIC_UNOTEST Directories
{
private:
    OUString m_aSrcRootURL;
    OUString m_aSrcRootPath;
    OUString m_aWorkdirRootURL;
    OUString m_aWorkdirRootPath;

    static void concatEnsureSeparator(OUStringBuffer& buf, std::u16string_view s)
    {
        while (s.starts_with('/'))
            s = s.substr(1);
        assert(!s.empty());
        if (!OUString::unacquired(buf).endsWith("/"))
            buf.append("/");
        buf.append(s);
    }

public:
    Directories();

    // return a URL to a given path from the source directory
    template <typename... Segments> OUString getURLFromSrc(Segments... segments) const
    {
        OUStringBuffer buf(m_aSrcRootURL);
        (..., concatEnsureSeparator(buf, segments));
        return buf.makeStringAndClear();
    }

    // return a Path to a given path from the source directory
    template <typename... Segments> OUString getPathFromSrc(Segments... segments) const
    {
        OUStringBuffer buf(m_aSrcRootPath);
        (..., concatEnsureSeparator(buf, segments));
        return buf.makeStringAndClear();
    }

    // return a URL to a given path from the workdir directory
    OUString getURLFromWorkdir(std::u16string_view rPath) const
    {
        OUStringBuffer buf(m_aWorkdirRootURL);
        concatEnsureSeparator(buf, rPath);
        return buf.makeStringAndClear();
    }

    // return a Path to a given path from the workdir directory
    OUString getPathFromWorkdir(std::u16string_view rPath) const
    {
        OUStringBuffer buf(m_aWorkdirRootPath);
        concatEnsureSeparator(buf, rPath);
        return buf.makeStringAndClear();
    }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
