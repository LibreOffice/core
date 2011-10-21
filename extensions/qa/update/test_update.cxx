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
 * Copyright (C) 2011 Jan Holesovsky <kendy@suse.cz> (initial developer)
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

#include <sal/config.h>
#include <sal/precppunit.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "../../source/update/check/updatecheck.hxx"
#include "../../source/update/check/updateprotocol.hxx"

using namespace com::sun::star;

namespace testupdate {

class Test : public CppUnit::TestFixture
{
public:
    void setUp()
    {
        if (!m_xContext.is())
            m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    }

    void tearDown()
    {
        uno::Reference< lang::XComponent >( m_xContext, uno::UNO_QUERY_THROW)->dispose();
    }

protected:
    // test the checkForUpdates() method
    void testCheckForUpdates()
    {
        UpdateState eUIState = UPDATESTATE_NO_UPDATE_AVAIL;

        UpdateInfo aInfo;
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );
        uno::Reference< deployment::XUpdateInformationProvider > m_xProvider( deployment::UpdateInformationProvider::create( m_xContext ) );

        if ( checkForUpdates( aInfo, m_xContext, aController->getInteractionHandler(), m_xProvider ) )
        {
            aController->setUpdateInfo( aInfo );
            eUIState = aController->getUIState( aInfo );
        }
        else
            CPPUNIT_FAIL("Calling checkForUpdates() failed.");
    }

    CPPUNIT_TEST_SUITE(Test);
    // FIXME CPPUNIT_TEST(testCheckForUpdates);
    CPPUNIT_TEST_SUITE_END();

private:
    static uno::Reference< uno::XComponentContext > m_xContext;
};

uno::Reference< uno::XComponentContext > Test::m_xContext;

CPPUNIT_TEST_SUITE_REGISTRATION(testupdate::Test);
} // namespace testupdate

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
