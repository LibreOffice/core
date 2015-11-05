/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <unotest/bootstrapfixturebase.hxx>
#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <basic/sbstar.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;

namespace {

OUString getFileURLFromSystemPath(OUString const & path) {
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(path, url);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    if (!url.endsWith("/")) {
        url += "/";
    }
    return url;
}

}

// NB. this constructor is called before any tests are run, once for each
// test function in a rather non-intuitive way. This is why all the 'real'
// heavy lifting is deferred until setUp. setUp and tearDown are interleaved
// between the tests as you might expect.
test::BootstrapFixtureBase::BootstrapFixtureBase()
{
#ifndef ANDROID
    const char* pSrcRoot = getenv( "SRC_ROOT" );
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != NULL && pSrcRoot[0] != 0);
    const char* pWorkdirRoot = getenv( "WORKDIR_FOR_BUILD" );
    CPPUNIT_ASSERT_MESSAGE("$WORKDIR_FOR_BUILD env variable not set", pWorkdirRoot != NULL && pWorkdirRoot[0] != 0);
#else
    const char* pSrcRoot = "/assets";
    const char* pWorkdirRoot = "/assets";
#endif
    m_aSrcRootPath = OUString::createFromAscii( pSrcRoot );
    m_aSrcRootURL = getFileURLFromSystemPath(m_aSrcRootPath);

    m_aWorkdirRootPath = OUString::createFromAscii( pWorkdirRoot );
    m_aWorkdirRootURL = getFileURLFromSystemPath(m_aWorkdirRootPath);

}

test::BootstrapFixtureBase::~BootstrapFixtureBase()
{
}

OUString test::BootstrapFixtureBase::getURLFromSrc( const char *pPath )
{
    return m_aSrcRootURL + OUString::createFromAscii( pPath );
}

OUString test::BootstrapFixtureBase::getURLFromSrc( const OUString& rPath )
{
    return m_aSrcRootURL + rPath;
}

OUString test::BootstrapFixtureBase::getPathFromSrc( const char *pPath )
{
    return m_aSrcRootPath + OUString::createFromAscii( pPath );
}

OUString test::BootstrapFixtureBase::getURLFromWorkdir( const char *pPath )
{
    return m_aWorkdirRootURL + OUString::createFromAscii( pPath );
}

#ifdef _WIN32 // ifdef just to keep it out of unusedcode.easy
OUString test::BootstrapFixtureBase::getPathFromWorkdir( const char *pPath )
{
    return m_aWorkdirRootPath + OUString::createFromAscii( pPath );
}
#endif

void test::BootstrapFixtureBase::setUp()
{
    // set UserInstallation to user profile dir in test/user-template
    OUString sUserInstallURL = m_aWorkdirRootURL + "/unittest";
    rtl::Bootstrap::set("UserInstallation", sUserInstallURL);

    m_xContext = comphelper::getProcessComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xSFactory.set(m_xFactory, uno::UNO_QUERY_THROW);
}

void test::BootstrapFixtureBase::tearDown()
{
    StarBASIC::DetachAllDocBasicItems();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
