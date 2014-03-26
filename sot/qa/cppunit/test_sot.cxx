/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
                          const OUString &rStreamName,
                          sal_uLong nSize );
        bool checkStorage( const SotStorageRef &xObjStor );

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            unsigned int, unsigned int, unsigned int) SAL_OVERRIDE;

        void test();

        CPPUNIT_TEST_SUITE(SotTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST_SUITE_END();
    };

    bool SotTest::checkStream( const SotStorageRef &xObjStor,
                               const OUString &rStreamName,
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

    bool SotTest::load(const OUString &,
        const OUString &rURL, const OUString &,
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
        testDir(OUString(),
            getURLFromSrc("/sot/qa/cppunit/data/"),
            OUString());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(SotTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
