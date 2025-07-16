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
#include <o3tl/environment.hxx>
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
    : m_aSrcRootPath(o3tl::getEnvironment(u"SRC_ROOT"_ustr))
    , m_aWorkdirRootPath(o3tl::getEnvironment(u"WORKDIR_FOR_BUILD"_ustr))
{
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", !m_aSrcRootPath.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("WORKDIR_FOR_BUILD env variable not set", !m_aWorkdirRootPath.isEmpty());
    m_aSrcRootURL = getFileURLFromSystemPath(m_aSrcRootPath);
    m_aWorkdirRootURL = getFileURLFromSystemPath(m_aWorkdirRootPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
