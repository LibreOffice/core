/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstddef>

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/deployment/UpdateInformationEntry.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>

#include "../../source/update/check/updatecheck.hxx"
#include "../../source/update/check/updateprotocol.hxx"

using namespace com::sun::star;
using namespace com::sun::star::xml;

namespace testupdate {

class Test : public test::BootstrapFixture
{
public:
    virtual void setUp() override
    {
        // so that comphelper::getProcessServiceFactory() works, m_xContext is
        // set up, etc.
        test::BootstrapFixture::setUp();

        if ( !m_xProvider.is() )
            m_xProvider = deployment::UpdateInformationProvider::create( m_xContext );

        // repositories that we will be checking
        m_aRepositoryList.realloc( 1 );
        m_aRepositoryList[0] = m_directories.getURLFromSrc( "/extensions/qa/update/simple.xml" );
    }

    virtual void tearDown() override
    {
        m_xProvider.clear();
        m_aRepositoryList.realloc( 0 );
        test::BootstrapFixture::tearDown();
    }

protected:
    // test the getUpdateInformationEnumeration() method
    void testGetUpdateInformationEnumeration()
    {
        uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            m_xProvider->getUpdateInformationEnumeration(
                m_aRepositoryList,
                "TODO" ); // unused when we do not have a 'feed'

        if ( !aUpdateInfoEnumeration.is() )
            CPPUNIT_FAIL( "Calling getUpdateInformationEnumeration() with TODO failed." );

        if ( !aUpdateInfoEnumeration->hasMoreElements() )
            CPPUNIT_FAIL( "Should have more elements (this one is 1st)." );

        deployment::UpdateInformationEntry aEntry;
        if ( aUpdateInfoEnumeration->nextElement() >>= aEntry )
        {
            CPPUNIT_ASSERT_EQUAL( OUString("description"), aEntry.UpdateDocument->getNodeName() );

            uno::Reference< dom::XNodeList> xChildNodes = aEntry.UpdateDocument->getChildNodes();
            CPPUNIT_ASSERT( xChildNodes.is() );
#if 0
            for ( int i = 0; i < xChildNodes->(); ++i )
            {
                fprintf( stderr, "node == %d\n", i );
                uno::Reference< dom::XElement > xChildId( xChildNodes->item( i ), uno::UNO_QUERY );
                if ( xChildId.is() )
                {
                    fprintf( stderr, "Name == %s\n", OUStringToOString( xChildId->getNodeName(), RTL_TEXTENCODING_UTF8 ).getStr() );
                    fprintf( stderr, "Value == %s\n", OUStringToOString( xChildId->getNodeValue(), RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
#endif
            CPPUNIT_ASSERT_EQUAL( sal_Int32(13), xChildNodes->getLength() );

            //uno::Reference< dom::XElement > xChildId( xChildNodes->item( 0 ), uno::UNO_QUERY );
            //CPPUNIT_ASSERT( xChildId.is() );
            //CPPUNIT_ASSERT( xChildId->getNodeValue() == "LibreOffice_3.4" );
            //fprintf( stderr, "Attribute == %s\n", OUStringToOString( aEntry.UpdateDocument->getAttribute( OUString( "test" ) ), RTL_TEXTENCODING_UTF8 ).getStr() );
            //fprintf( stderr, "Value == %s\n", OUStringToOString( xChildId->getNodeValue(), RTL_TEXTENCODING_UTF8 ).getStr() );
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
                    "Linux",
                    "x86",
                    m_aRepositoryList,
                    "111111-222222-333333-444444",
                    "InstallSetID" ) )
        {
            CPPUNIT_ASSERT_EQUAL( std::size_t(1), aInfo.Sources.size() );
            CPPUNIT_ASSERT_EQUAL( OUString("http://www.libreoffice.org/download/"), aInfo.Sources[0].URL );
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
                    "Linux",
                    "x86",
                    m_aRepositoryList,
                    "123456-abcdef-1a2b3c-4d5e6f",
                    "InstallSetID" ) )
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
    uno::Sequence< OUString > m_aRepositoryList;
};

CPPUNIT_TEST_SUITE_REGISTRATION(testupdate::Test);
} // namespace testupdate

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
