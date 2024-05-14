/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/plugin/TestPlugIn.h>
#include <PropfindCache.hxx>

using namespace http_dav_ucp;

namespace
{

    class webdav_propcache_test: public test::BootstrapFixture
    {

    public:
        webdav_propcache_test() : BootstrapFixture( true, true ) {}

        // initialise your test code values here.
        void setUp(  ) override;

        void tearDown(  ) override;

        void PropfindCacheElemTests();
        void PropfindCacheTests();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_propcache_test );
        CPPUNIT_TEST( PropfindCacheElemTests );
        CPPUNIT_TEST( PropfindCacheTests );
        CPPUNIT_TEST_SUITE_END();
    };                          // class webdav_local_test

    // initialise your test code values here.
    void webdav_propcache_test::setUp()
    {
    }

    void webdav_propcache_test::tearDown()
    {
    }

    void webdav_propcache_test::PropfindCacheElemTests( )
    {
        OUString aTheURL( u"http:://server/path/filename.odt"_ustr );
        PropertyNames aPropsNames( aTheURL );

        CPPUNIT_ASSERT_EQUAL( aTheURL, aPropsNames.getURL() );
        CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt32 >(0), aPropsNames.getStaleTime() );

        sal_uInt32 maxTime = static_cast< sal_uInt32 >(std::pow(2,32)-1);

        aPropsNames.setStaleTime( maxTime );
        CPPUNIT_ASSERT_EQUAL( maxTime, aPropsNames.getStaleTime() );

        std::vector < OUString > properties {
                u"DAV:lockdiscovery"_ustr,
                u"DAV:supportedlock"_ustr,
                u"DAV:resourcetype"_ustr,
                u"DAV:displayname"_ustr,
                u"DAV:getlastmodified"_ustr,
                u"DAV:getcontentlength"_ustr,
                u"DAV:creationdate"_ustr,
                u"DAV:getetag"_ustr,
                u"DAV:authticket"_ustr,
                };

        DAVResourceInfo aSingleInfo { properties };
        std::vector< DAVResourceInfo > aProps { aSingleInfo };
        std::vector< DAVResourceInfo > aRetProp;

        aPropsNames.setPropertiesNames( std::vector(aProps) );
        aRetProp = aPropsNames.getPropertiesNames();
        CPPUNIT_ASSERT_EQUAL( true, ( aProps == aRetProp ) );

        aProps[0].properties.emplace_back("DAV:getlastmodified" );
        aRetProp = aPropsNames.getPropertiesNames();
        CPPUNIT_ASSERT_EQUAL( false, ( aProps == aRetProp ) );
    }

    void webdav_propcache_test::PropfindCacheTests( )
    {
        PropertyNamesCache PropCache;
        OUString aTheURL( u"http:://server/path/filename.odt"_ustr );
        PropertyNames aPropsNames( aTheURL );

        // check cache emptiness
        CPPUNIT_ASSERT_EQUAL( false, PropCache.getCachedPropertyNames( aTheURL,  aPropsNames ) );

        std::vector < OUString > properties {
                u"DAV:lockdiscovery"_ustr,
                u"DAV:supportedlock"_ustr,
                u"DAV:resourcetype"_ustr,
                u"DAV:displayname"_ustr,
                u"DAV:getlastmodified"_ustr,
                u"DAV:getcontentlength"_ustr,
                u"DAV:creationdate"_ustr,
                u"DAV:getetag"_ustr,
                u"DAV:authticket"_ustr,
                };

        DAVResourceInfo aSingleInfo { properties };
        std::vector< DAVResourceInfo > aProps { aSingleInfo };

        // add the cache an element
        aPropsNames.setPropertiesNames( std::vector(aProps) );
        PropCache.addCachePropertyNames( aPropsNames, 10 );

        PropertyNames aRetPropsNames;
        //test existence
        CPPUNIT_ASSERT_EQUAL( true, PropCache.getCachedPropertyNames( aTheURL, aRetPropsNames ) );
        //check equality
        std::vector< DAVResourceInfo > aRetProp = aRetPropsNames.getPropertiesNames();
        CPPUNIT_ASSERT_EQUAL( true, ( aProps == aRetProp ) );
        //remove from cache
        PropCache.removeCachedPropertyNames( aTheURL );
        //check absence
        CPPUNIT_ASSERT_EQUAL( false, PropCache.getCachedPropertyNames( aTheURL,  aPropsNames ) );
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_propcache_test );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
