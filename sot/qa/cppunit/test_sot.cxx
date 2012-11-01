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
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *  (initial developer)
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

#include <unotest/filters-test.hxx>
#include <unotest/bootstrapfixturebase.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>

using namespace ::com::sun::star;

namespace
{
    class SotTest
        : public test::FiltersTest
        , public test::BootstrapFixtureBase
    {
    public:
        SotTest() {}

        bool checkStream( const SotStorageRef &xObjStor,
                          const String &rStreamName,
                          sal_uLong nSize );
        bool checkStorage( const SotStorageRef &xObjStor );

        virtual bool load(const rtl::OUString &,
            const rtl::OUString &rURL, const rtl::OUString &,
            unsigned int, unsigned int, unsigned int);

        void test();

        CPPUNIT_TEST_SUITE(SotTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    };

    bool SotTest::checkStream( const SotStorageRef &xObjStor,
                               const String &rStreamName,
                               sal_uLong nSize )
    {
        unsigned char *pData = (unsigned char*)malloc( nSize );
        sal_uLong nReadableSize = 0;
        if( !pData )
            return true;

        {   // Read the data in one block
            SotStorageStreamRef xStream( xObjStor->OpenSotStream( rStreamName ) );
            xStream->Seek(0);
            sal_uLong nRemaining = xStream->GetSize() - xStream->Tell();

            CPPUNIT_ASSERT_MESSAGE( "check size", nRemaining == nSize );
            CPPUNIT_ASSERT_MESSAGE( "check size #2", xStream->remainingSize() == nSize );

            // Read as much as we can, a corrupted FAT chain can cause real grief here
            nReadableSize = xStream->Read( (void *)pData, nSize );
//            fprintf(stderr, "readable size %d vs size %d remaining %d\n", nReadableSize, nSize, nReadableSize);
        }
        {   // Read the data backwards as well
            SotStorageStreamRef xStream( xObjStor->OpenSotStream( rStreamName ) );
            for( sal_uLong i = nReadableSize; i > 0; i-- )
            {
                CPPUNIT_ASSERT_MESSAGE( "sot reading error", !xStream->GetError() );
                unsigned char c;
                xStream->Seek( i - 1 );
                CPPUNIT_ASSERT_MESSAGE( "sot storage reading byte",
                                        xStream->Read( &c, 1 ) == 1);
                CPPUNIT_ASSERT_MESSAGE( "mismatching data storage reading byte",
                                        pData[i - 1] == c );
            }
        }

        return true;
    }

    bool SotTest::checkStorage( const SotStorageRef &xObjStor )
    {
        SvStorageInfoList aInfoList;
        xObjStor->FillInfoList( &aInfoList );

        for( SvStorageInfoList::iterator aIt = aInfoList.begin();
             aIt != aInfoList.end(); ++aIt )
        {
            if( aIt->IsStorage() )
            {
                SotStorageRef xChild( xObjStor->OpenSotStorage( aIt->GetName() ) );
                checkStorage( xChild );
            }
            else if( aIt->IsStream() )
                checkStream( xObjStor, aIt->GetName(), aIt->GetSize() );
        }

        return true;
    }

    bool SotTest::load(const rtl::OUString &,
        const rtl::OUString &rURL, const rtl::OUString &,
        unsigned int, unsigned int, unsigned int)
    {
        SvFileStream aStream(rURL, STREAM_READ);
        SotStorageRef xObjStor = new SotStorage(aStream);
        if (!xObjStor.Is() || xObjStor->GetError())
            return false;

        CPPUNIT_ASSERT_MESSAGE("sot storage is not valid", xObjStor->Validate());
        return checkStorage (xObjStor);
    }

    void SotTest::test()
    {
        testDir(rtl::OUString(),
            getURLFromSrc("/sot/qa/cppunit/data/"),
            rtl::OUString());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(SotTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
