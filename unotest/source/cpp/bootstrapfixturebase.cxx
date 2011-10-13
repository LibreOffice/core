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
#include <ucbhelper/contentbroker.hxx>
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
    : m_aSrcRootURL(RTL_CONSTASCII_USTRINGPARAM("file://"))
{
    const char* pSrcRoot = getenv( "SRC_ROOT" );
    CPPUNIT_ASSERT_MESSAGE("SRC_ROOT env variable not set", pSrcRoot != NULL && pSrcRoot[0] != 0);

#ifdef WNT
    if (pSrcRoot[1] == ':')
        m_aSrcRootURL += rtl::OUString::createFromAscii( "/" );
#endif
    m_aSrcRootPath = rtl::OUString::createFromAscii( pSrcRoot );
    m_aSrcRootURL += m_aSrcRootPath;
}

test::BootstrapFixtureBase::~BootstrapFixtureBase()
{
}

::rtl::OUString test::BootstrapFixtureBase::getURLFromSrc( const char *pPath )
{
  return m_aSrcRootURL + rtl::OUString::createFromAscii( pPath );
}

void test::BootstrapFixtureBase::setUp()
{
    // set UserInstallation to user profile dir in test/user-template
    rtl::Bootstrap aDefaultVars;
    aDefaultVars.set( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UserInstallation") ),
                         getURLFromSrc("/test/user-template"));

    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();
    m_xSFactory = uno::Reference<lang::XMultiServiceFactory> (m_xFactory, uno::UNO_QUERY_THROW);

    // Without this we're crashing because callees are using
    // getProcessServiceFactory.  In general those should be removed in favour
    // of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(m_xSFactory);
}

void test::BootstrapFixtureBase::tearDown()
{
    //    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
