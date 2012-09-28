/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2011 Michael Meeks <michael.meeks@suse.com>
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
#ifdef WNT
    if (pSrcRoot[1] == ':')
        m_aSrcRootURL += rtl::OUString::createFromAscii( "/" );
    if (pSolverRoot[1] == ':')
        m_aSolverRootURL += rtl::OUString::createFromAscii( "/" );
#endif
#else
    const char* pSrcRoot = "/assets";
    const char* pSolverRoot = "/assets";
#endif
    m_aSrcRootPath = rtl::OUString::createFromAscii( pSrcRoot );
    m_aSrcRootURL += m_aSrcRootPath;

    m_aSolverRootPath = rtl::OUString::createFromAscii( pSolverRoot );
    m_aSolverRootURL += m_aSolverRootPath;
}

test::BootstrapFixtureBase::~BootstrapFixtureBase()
{
}

::rtl::OUString test::BootstrapFixtureBase::getURLFromSrc( const char *pPath )
{
    return m_aSrcRootURL + rtl::OUString::createFromAscii( pPath );
}

::rtl::OUString test::BootstrapFixtureBase::getPathFromSrc( const char *pPath )
{
    return m_aSrcRootPath + rtl::OUString::createFromAscii( pPath );
}

::rtl::OUString test::BootstrapFixtureBase::getURLFromSolver( const char *pPath )
{
    return m_aSolverRootURL + rtl::OUString::createFromAscii( pPath );
}

::rtl::OUString test::BootstrapFixtureBase::getPathFromSolver( const char *pPath )
{
    return m_aSolverRootPath + rtl::OUString::createFromAscii( pPath );
}

void test::BootstrapFixtureBase::setUp()
{
    // set UserInstallation to user profile dir in test/user-template
    rtl::Bootstrap aDefaultVars;
    rtl::OUString sUserInstallURL = m_aSolverRootURL + rtl::OUString("/unittest");
    aDefaultVars.set(rtl::OUString("UserInstallation"), sUserInstallURL);

    m_xContext = comphelper::getProcessComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xSFactory = uno::Reference<lang::XMultiServiceFactory>(m_xFactory, uno::UNO_QUERY_THROW);
}

void test::BootstrapFixtureBase::tearDown()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
