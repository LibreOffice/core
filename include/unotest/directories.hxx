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

public:
    Directories();

    const OUString& getSrcRootURL() const { return m_aSrcRootURL; }
    const OUString& getSrcRootPath() const { return m_aSrcRootPath; }

    // return a URL to a given path from the source directory
    OUString getURLFromSrc(std::u16string_view rPath) const;

    // return a Path to a given path from the source directory
    OUString getPathFromSrc(std::u16string_view rPath) const;

    // return a URL to a given path from the workdir directory
    OUString getURLFromWorkdir(std::u16string_view rPath) const;

    // return a Path to a given path from the workdir directory
    OUString getPathFromWorkdir(std::u16string_view rPath) const;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
