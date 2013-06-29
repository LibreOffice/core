/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <unotest/bootstrapfixturebase.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;

// NB. this constructor is called before any tests are run, once for each
// test function in a rather non-intuitive way. This is why all the 'real'
// heavy lifting is deferred until setUp. setUp and tearDown are interleaved
// between the tests as you might expect.
test::BootstrapFixtureBase::BootstrapFixtureBase()
    : m_aSrcRootURL("file://"), m_aSolverRootURL( m_aSrcRootURL )
{
#ifndef ANDROID
    const char* pSrcRoot = getenv( "SRC_ROOT" );
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != NULL && pSrcRoot[0] != 0);
    const char* pSolverRoot = getenv( "OUTDIR_FOR_BUILD" );
    CPPUNIT_ASSERT_MESSAGE("$OUTDIR_FOR_BUILD env variable not set", pSolverRoot != NULL && pSolverRoot[0] != 0);
    const char* pWorkdirRoot = getenv( "WORKDIR_FOR_BUILD" );
    CPPUNIT_ASSERT_MESSAGE("$WORKDIR_FOR_BUILD env variable not set", pWorkdirRoot != NULL && pWorkdirRoot[0] != 0);
#ifdef WNT
    if (pSrcRoot[1] == ':')
    {
        m_aSrcRootURL += "/";
    }
    if (pSolverRoot[1] == ':')
    {
        m_aSolverRootURL += "/";
    }
    if (pWorkdirRoot[1] == ':')
    {
        m_aWorkdirRootURL += "/";
    }
#endif
#else
    const char* pSrcRoot = "/assets";
    const char* pSolverRoot = "/assets";
    const char* pWorkdirRoot = "/assets";
#endif
    m_aSrcRootPath = OUString::createFromAscii( pSrcRoot );
    m_aSrcRootURL += m_aSrcRootPath;

    m_aSolverRootPath = OUString::createFromAscii( pSolverRoot );
    m_aSolverRootURL += m_aSolverRootPath;

    m_aWorkdirRootPath = OUString::createFromAscii( pWorkdirRoot );
    m_aWorkdirRootURL += m_aWorkdirRootPath;

}

test::BootstrapFixtureBase::~BootstrapFixtureBase()
{
}

OUString test::BootstrapFixtureBase::getURLFromSrc( const char *pPath )
{
    return m_aSrcRootURL + OUString::createFromAscii( pPath );
}

OUString test::BootstrapFixtureBase::getPathFromSrc( const char *pPath )
{
    return m_aSrcRootPath + OUString::createFromAscii( pPath );
}


OUString test::BootstrapFixtureBase::getURLFromWorkdir( const char *pPath )
{
    return m_aWorkdirRootURL + OUString::createFromAscii( pPath );
}

OUString test::BootstrapFixtureBase::getPathFromWorkdir( const char *pPath )
{
    return m_aWorkdirRootPath + OUString::createFromAscii( pPath );

}

void test::BootstrapFixtureBase::setUp()
{
    // set UserInstallation to user profile dir in test/user-template
    rtl::Bootstrap aDefaultVars;
    OUString sUserInstallURL = m_aSolverRootURL + OUString("/unittest");
    aDefaultVars.set(OUString("UserInstallation"), sUserInstallURL);

    m_xContext = comphelper::getProcessComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xSFactory = uno::Reference<lang::XMultiServiceFactory>(m_xFactory, uno::UNO_QUERY_THROW);
}

void test::BootstrapFixtureBase::tearDown()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
