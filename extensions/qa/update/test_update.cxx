/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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
        
        
        test::BootstrapFixture::setUp();

        if ( !m_xProvider.is() )
            m_xProvider = deployment::UpdateInformationProvider::create( m_xContext );

        
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
    
    void testGetUpdateInformationEnumeration()
    {
        OUString aInstallSetID( "TODO" ); 

        uno::Reference< container::XEnumeration > aUpdateInfoEnumeration =
            m_xProvider->getUpdateInformationEnumeration( m_aRepositoryList, aInstallSetID );

        if ( !aUpdateInfoEnumeration.is() )
            CPPUNIT_FAIL( "Calling getUpdateInformationEnumeration() with TODO failed." );

        if ( !aUpdateInfoEnumeration->hasMoreElements() )
            CPPUNIT_FAIL( "Should have more elements (this one is 1st)." );

        deployment::UpdateInformationEntry aEntry;
        if ( aUpdateInfoEnumeration->nextElement() >>= aEntry )
        {
            CPPUNIT_ASSERT( aEntry.UpdateDocument->getNodeName() == "description" );

            uno::Reference< dom::XNodeList> xChildNodes = aEntry.UpdateDocument->getChildNodes();
            CPPUNIT_ASSERT( xChildNodes.is() );
#if 0
            for ( int i = 0; i < xChildNodes->getLength(); ++i )
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
            CPPUNIT_ASSERT( xChildNodes->getLength() == 13 );

            
            
            
            
            
            
        }
        else
            CPPUNIT_FAIL( "Wrong type of the entry." );
    }

    
    void testCheckUpdateAvailable()
    {
        UpdateInfo aInfo;
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );

        if ( checkForUpdates( aInfo, m_xContext, aController->getInteractionHandler(), m_xProvider,
                    OUString( "Linux" ),
                    OUString( "x86" ),
                    m_aRepositoryList,
                    OUString( "111111-222222-333333-444444" ),
                    OUString( "InstallSetID" ) ) )
        {
            CPPUNIT_ASSERT( aInfo.Sources.size() == 1 );
            CPPUNIT_ASSERT( aInfo.Sources[0].URL == "http:
        }
        else
            CPPUNIT_FAIL( "Calling checkForUpdates() failed." );
    }

    
    void testCheckUpToDate()
    {
        UpdateInfo aInfo;
        rtl::Reference< UpdateCheck > aController( UpdateCheck::get() );

        if ( checkForUpdates( aInfo, m_xContext, aController->getInteractionHandler(), m_xProvider,
                    OUString( "Linux" ),
                    OUString( "x86" ),
                    m_aRepositoryList,
                    OUString( "123456-abcdef-1a2b3c-4d5e6f" ),
                    OUString( "InstallSetID" ) ) )
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
} 

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
