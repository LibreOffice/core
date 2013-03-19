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

#include <test/bootstrapfixture.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "../../source/update/check/updatecheck.hxx"
#include "../../source/update/check/updateprotocol.hxx"

using namespace com::sun::star;
using namespace com::sun::star::xml;

namespace testupdate {

class Test : public test::BootstrapFixture
{
public:
    virtual void setUp()
    {
        // so that comphelper::getProcessServiceFactory() works, m_xContext is
        // set up, etc.
        test::BootstrapFixture::setUp();

        if ( !m_xProvider.is() )
            m_xProvider = deployment::UpdateInformationProvider::create( m_xContext );

        // repositories that we will be checking
        m_aRepositoryList.realloc( 1 );
        m_aRepositoryList[0] = getURLFromSrc( "/extensions/qa/update/simple.xml" );
    }

    virtual void tearDown()
    {
        m_xProvider.clear();
        m_aRepositoryList.realloc( 0 );
        test::BootstrapFixture::tearDown();
    }

protected:
    // test the getUpdateInformationEnumeration() method
    void testGetUpdateInformationEnumeration()
    {
        ::rtl::OUString aInstallSetID( "TODO" ); // unused when we do not have a 'feed'

        uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            m_xProvider->getUpdateInformationEnumeration( m_aRepositoryList, aInstallSetID );

        if ( !aUpdateInfoEnumeration.is() )
            CPPUNIT_FAIL( "Calling getUpdateInformationEnumeration() with TODO failed." );

        if ( !aUpdateInfoEnumeration->hasMoreElements() )
            CPPUNIT_FAIL( "Should have more elements (this one is 1st)." );

        deployment::UpdateInformationEntry aEntry;
        if ( aUpdateInfoEnumeration->nextElement() >>= aEntry )
        {
            CPPUNIT_ASSERT( aEntry.UpdateDocument->getNodeName() == rtl::OUString( "description" ) );

            uno::Reference< dom::XNodeList> xChildNodes = aEntry.UpdateDocument->getChildNodes();
            CPPUNIT_ASSERT( xChildNodes.is() );
#if 0
            for ( int i = 0; i < xChildNodes->getLength(); ++i )
            {
                fprintf( stderr, "node == %d\n", i );
                uno::Reference< dom::XElement > xChildId( xChildNodes->item( i ), uno::UNO_QUERY );
                if ( xChildId.is() )
                {
                    fprintf( stderr, "Name == %s\n", rtl::OUStringToOString( xChildId->getNodeName(), RTL_TEXTENCODING_UTF8 ).getStr() );
                    fprintf( stderr, "Value == %s\n", rtl::OUStringToOString( xChildId->getNodeValue(), RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
#endif
            CPPUNIT_ASSERT( xChildNodes->getLength() == 13 );

            //uno::Reference< dom::XElement > xChildId( xChildNodes->item( 0 ), uno::UNO_QUERY );
            //CPPUNIT_ASSERT( xChildId.is() );
            //CPPUNIT_ASSERT( xChildId->getNodeValue() == rtl::OUString( "LibreOffice_3.4" ) );
            //fprintf( stderr, "Attribute == %s\n", rtl::OUStringToOString( aEntry.UpdateDocument->getAttribute( rtl::OUString( "test" ) ), RTL_TEXTENCODING_UTF8 ).getStr() );
            //fprintf( stderr, "Value == %s\n", rtl::OUStringToOString( xChildId->getNodeValue(), RTL_TEXTENCODING_UTF8 ).getStr() );
            // TODO check more deeply
        }
        else
            CPPUNIT_FAIL( "Wrong type of the entry." );
    }

    // test the checkForUpdates() method - update is available
    void testCheckUpdateAvailable()
    {
        UpdateInfo aInfo;
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );

        if ( checkForUpdates( aInfo, m_xContext, aController->getInteractionHandler(), m_xProvider,
                    rtl::OUString( "Linux" ),
                    rtl::OUString( "x86" ),
                    m_aRepositoryList,
                    rtl::OUString( "111111-222222-333333-444444" ),
                    rtl::OUString( "InstallSetID" ) ) )
        {
            CPPUNIT_ASSERT( aInfo.Sources.size() == 1 );
            CPPUNIT_ASSERT( aInfo.Sources[0].URL == rtl::OUString( "http://www.libreoffice.org/download/" ) );
        }
        else
            CPPUNIT_FAIL( "Calling checkForUpdates() failed." );
    }

    // test the checkForUpdates() method - we are up-to-date
    void testCheckUpToDate()
    {
        UpdateInfo aInfo;
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );

        if ( checkForUpdates( aInfo, m_xContext, aController->getInteractionHandler(), m_xProvider,
                    rtl::OUString( "Linux" ),
                    rtl::OUString( "x86" ),
                    m_aRepositoryList,
                    rtl::OUString( "123456-abcdef-1a2b3c-4d5e6f" ),
                    rtl::OUString( "InstallSetID" ) ) )
        {
            CPPUNIT_ASSERT( aInfo.Sources.empty() );
        }
        else
            CPPUNIT_FAIL( "Calling checkForUpdates() failed." );
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testGetUpdateInformationEnumeration);
    CPPUNIT_TEST(testCheckUpdateAvailable);
    CPPUNIT_TEST(testCheckUpToDate);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< deployment::XUpdateInformationProvider > m_xProvider;
    uno::Sequence< rtl::OUString > m_aRepositoryList;
};

CPPUNIT_TEST_SUITE_REGISTRATION(testupdate::Test);
} // namespace testupdate

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
