/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <osl/file.hxx>
#include <unotest/directories.hxx>

namespace
{
OUString getFileURLFromSystemPath(OUString const& path)
{
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(path, url);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    if (!url.endsWith("/"))
    {
        url += "/";
    }
    return url;
}
}

test::Directories::Directories()
{
    const char* pSrcRoot = getenv("SRC_ROOT");
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != nullptr);
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot[0] != 0);
    const char* pWorkdirRoot = getenv("WORKDIR_FOR_BUILD");
    CPPUNIT_ASSERT_MESSAGE("$WORKDIR_FOR_BUILD env variable not set", pWorkdirRoot != nullptr);
    CPPUNIT_ASSERT_MESSAGE("$WORKDIR_FOR_BUILD env variable not set", pWorkdirRoot[0] != 0);
    m_aSrcRootPath = OUString::createFromAscii(pSrcRoot);
    m_aSrcRootURL = getFileURLFromSystemPath(m_aSrcRootPath);

    m_aWorkdirRootPath = OUString::createFromAscii(pWorkdirRoot);
    m_aWorkdirRootURL = getFileURLFromSystemPath(m_aWorkdirRootPath);
}

OUString test::Directories::getURLFromSrc(std::u16string_view rPath) const
{
    return m_aSrcRootURL + rPath;
}

OUString test::Directories::getPathFromSrc(std::u16string_view rPath) const
{
    return m_aSrcRootPath + rPath;
}

OUString test::Directories::getURLFromWorkdir(std::u16string_view rPath) const
{
    return m_aWorkdirRootURL + rPath;
}

OUString test::Directories::getPathFromWorkdir(std::u16string_view rPath) const
{
    return m_aWorkdirRootPath + rPath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
